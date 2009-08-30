/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */



#ifdef __USAGE
%C - dump an image file system

%C	[-mvxbz -u file] [-f file] image_file_system_file [files]
 -b       Extract to basenames of files
 -u file  Put a copy of the uncompressed image file here
 -v       Verbose
 -x       Extract files
 -m       Display MD5 Checksum
 -f file  Extract named file
 -z       Disable the zero check while searching for the startup header.
          This option should be avoided as it makes the search for the
          startup header less reliable.
          Note: this may not be supported in the future.
#endif

#include <lib/compat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <utime.h>
#include <sys/stat.h>

#include _NTO_HDR_(sys/elf.h)
#include _NTO_HDR_(sys/startup.h)
#include _NTO_HDR_(sys/image.h)

#include <zlib.h>
#include <lzo1x.h>
#include <ucl/ucl.h>

#include "xplatform.h"
#include "md5.h"


#if defined(__MINGW32__)
#define mkdir(path, mode) mkdir(path)
#endif


char *progname;
int verbose;
unsigned flags;
char **check_files;
char *ucompress_file;
int zero_check_enabled = 1;

int files_to_extract;
int files_left_to_extract;
int processing_done;
struct extract_file {
	struct extract_file *next;
	char *path;
} *extract_files = NULL;

#define FLAG_EXTRACT		0x00000001
#define FLAG_DISPLAY		0x00000002
#define FLAG_BASENAME		0x00000004
#define FLAG_MD5			0x00000008

#define ENDIAN_RET32(x)		((((x) >> 24) & 0xff) | \
							(((x) >> 8) & 0xff00) | \
							(((x) & 0xff00) << 8) | \
							(((x) & 0xff) << 24))

#define ENDIAN_RET16(x)		((((x) >> 8) & 0xff) | \
							(((x) & 0xff) << 8))

#if defined(__LITTLEENDIAN__) || defined(__X86__)
	#define	CROSSENDIAN(big)	(big)
#elif defined(__BIGENDIAN__)
	#define	CROSSENDIAN(big)	(!(big))
#else
	#error Host endianness not defined
#endif

void process(const char *file, FILE *fp);

void display_shdr(FILE *fp, int spos, struct startup_header *hdr);
void display_ihdr(FILE *fp, int ipos, struct image_header *hdr);
void process_file(FILE *fp, int ipos, struct image_file *ent);
void process_dir(FILE *fp, int ipos, struct image_dir *ent);
void process_symlink(FILE *fp, int ipos, struct image_symlink *ent);
void process_device(FILE *fp, int ipos, struct image_device *ent);
void display_file(FILE *fp, int ipos, struct image_file *ent);

#define MD5_LENGTH            16
void compute_md5(FILE *fp, int ipos, struct image_file *ent, unsigned char md5_result[16]);

int zero_ok (struct startup_header *shdr);

#if defined(__QNXNTO__) || defined(__SOLARIS__)

// Get basename()
#include <libgen.h>

#endif


void error(int level, char *p, ...) {
	va_list				ap;

	fprintf(stderr, "%s: ", progname);

	va_start(ap, p);
	vfprintf(stderr, p, ap);
	va_end(ap);

	fputc('\n', stderr);

	if(level == 0) {
		exit(EXIT_FAILURE);
	}
}

int main(int argc, char *argv[]) {
	int					c;
	char				*image;
	FILE				*fp = NULL;
	char				*dest_dir_path = NULL;
	int 				tfd = -1;
	char				tpath[_POSIX_PATH_MAX];
	struct extract_file *ef;

	progname = basename(argv[0]);

	while((c = getopt(argc, argv, "f:d:mvxbu:z")) != -1) {
		switch(c) {

		case 'f':
			ef = malloc( sizeof(*ef) );
			if ( ef == NULL ) {
				perror("recording path");
				break;
			}
			ef->path = strdup(optarg);
			if ( ef->path == NULL ) {
				perror("recording path");
				break;
			}
			ef->next = extract_files;
			extract_files = ef;
			files_to_extract++;
			files_left_to_extract++;
			flags |= FLAG_EXTRACT;
			break;

		case 'd':
			dest_dir_path = strdup(optarg);
			flags |= FLAG_EXTRACT;
			break;

		case 'm':
			flags |= FLAG_MD5;
			break;

		case 'x':
			flags |= FLAG_EXTRACT;
			break;

		case 'b':
			flags |= FLAG_BASENAME;
			break;

		case 'u':
			ucompress_file = optarg;
			break;

		case 'v':
			verbose++;
			break;

		case 'z':
			zero_check_enabled = 0;
			break;
			
		default:
			break;
		}
	}

	if(flags & (FLAG_EXTRACT|FLAG_MD5)) {
		if(verbose > 1) {
			flags |= FLAG_DISPLAY;
		}
	} else {
		flags |= FLAG_DISPLAY;
	}

	if(optind >= argc) {
		error(0, "Missing image file system name\n");
	}
	image = argv[optind++];

	if(optind < argc) {
		check_files = &argv[optind];
	}

	if(strcmp(image, "-") != 0) {
		if(!(fp = fopen(image, "rb"))) {
			error(0, "Unable to open file %s - %s", image, strerror(errno));
		}
	} else if(isatty(fileno(stdin))) {
		error(0, "Must have an image file");
	} else {
		fp = stdin;
		image = "-- stdin --";
	}

	if ( dest_dir_path != NULL ) {
		/* This temp file is created to address the problems of creating a file in /dev/shmem */
		sprintf( tpath, "%s/.dumpifs.%d", dest_dir_path, getpid() );

		tfd = open( tpath, O_CREAT|O_RDONLY, 0644 );
		if ( tfd == -1 ) {
			if ( (access( dest_dir_path, W_OK ) != 0 ) && (mkdir( dest_dir_path, 0755) != 0)) {
				error(0, "Cannot create directory %s to extract %s", dest_dir_path, image);
			}
		}
		if (chdir(dest_dir_path) != 0) {
			error(0, "Cannot cd to directory %s to extract %s", dest_dir_path, image);
		}
	}

	process(image, fp);

	if ( tfd != -1 ) {
		close(tfd);
		unlink( tpath );
	}

	return EXIT_SUCCESS;
}

int find(FILE *fp, const unsigned char *p, int len, int count) {
	int					c;
	int					i;
	int					n;

	i = n = 0;
	while(i < len && (c = fgetc(fp)) != EOF) {
		n++;
		if(c == p[i]) {
			i++;
		} else {
			if(count != -1 && n >= count) {
				break;
			}
			i = c == p[0] ? 1 : 0;
		}
	}
	if(i < len) {
		return -1;
	}
	return ftell(fp) - i;
}

int check(const char *name) {
	char			**p;

	if(!(p = check_files)) {
		return 0;
	}
	name = basename((char *)name);
	while(*p) {
		if(strcmp(name, *p) == 0) {
			return 0;
		}
		p++;
	}
	return -1;
}

char *associate(int ix, char *value)
{
static int	num = 0;
static char	**text = NULL;
char		**ptr;

	if (value != NULL) {
		if (ix >= num) {
			if ((ptr = realloc(text, (ix + 1) * sizeof(char *))) == NULL)
				return(value);
			memset(&ptr[num], 0, (ix - num) * sizeof(char *));
			text = ptr, num = ix + 1;
		}
		text[ix] = strdup(value);
	}
	else {
		value = (ix < num) ? text[ix] : "";
	}
	return(value);
}

void display_script(FILE *fp, int pos, int len) {
	int								off;
	char							buff[512];
	union script_cmd				*hdr = (union script_cmd *)buff;
	int								size;
	int								ext_sched = SCRIPT_SCHED_EXT_NONE;

	for(off = 0; off < len; off += size) {
		fseek(fp, pos + off, SEEK_SET);
		if(fread(hdr, sizeof *hdr, 1, fp) != 1) {
			break;
		}
		if((size = hdr->hdr.size_lo | (hdr->hdr.size_hi << 8)) == 0) {
			break;
		}

		if(size > sizeof *hdr) {
			int							n = min(sizeof buff, size - sizeof *hdr);

			if(fread(hdr + 1, n, 1, fp) != 1) {
				break;
			}
		}
		printf("                       ");
		switch(hdr->hdr.type) {
		case SCRIPT_TYPE_EXTERNAL: {
			char			*cmd, *args, *envs;
			int				i;
			
			cmd = hdr->external.args;
			envs = args = cmd + strlen(cmd) + 1;
			for(i = 0; i < hdr->external.argc; i++) {
				envs = envs + strlen(envs) + 1;
			}
			i = strcmp(basename(cmd), args);
			
			if(i || (hdr->external.flags & (SCRIPT_FLAGS_SESSION | SCRIPT_FLAGS_KDEBUG | SCRIPT_FLAGS_SCHED_SET | SCRIPT_FLAGS_CPU_SET | SCRIPT_FLAGS_EXTSCHED))) {
				printf("[ ");
				if(i) {
					printf("argv0=%s ", args);
				}
				if(hdr->external.flags & SCRIPT_FLAGS_SESSION) {
					printf("+session ");
				}
				if(hdr->external.flags & SCRIPT_FLAGS_KDEBUG) {
					printf("+debug ");
				}
				if(hdr->external.flags & SCRIPT_FLAGS_SCHED_SET) {
					printf("priority=%d", hdr->external.priority);
					switch(hdr->external.policy) {
					case SCRIPT_POLICY_NOCHANGE:
						break;
					case SCRIPT_POLICY_FIFO:
						printf("f");
						break;
					case SCRIPT_POLICY_RR:
						printf("r");
						break;
					case SCRIPT_POLICY_OTHER:
						printf("o");
						break;
					default:
						printf("?%d?", hdr->external.policy);
						break;
				    }
					printf(" ");
				}
				if(hdr->external.flags & SCRIPT_FLAGS_CPU_SET) {
					printf("cpu=%d ", hdr->external.cpu);
				}
				if(hdr->external.flags & SCRIPT_FLAGS_EXTSCHED) {
					printf("sched_aps=%s ", associate(hdr->external.extsched.aps.id, NULL));
				}
				printf("] ");
			}
			for(i = 0; i < hdr->external.envc; i++) {
				printf("%s ", envs);
				envs = envs + strlen(envs) + 1;
			}
			printf("%s", cmd);

			args = args + strlen(args) + 1;
			for(i = 1; i < hdr->external.argc; i++) {
				printf(" %s", args);
				args = args + strlen(args) + 1;
			}
			
			if(hdr->external.flags & SCRIPT_FLAGS_BACKGROUND) {
				printf(" &");
			}
			printf("\n");
			break;
		}
			
		case SCRIPT_TYPE_WAITFOR:
			printf("waitfor");
			goto wait;
		case SCRIPT_TYPE_REOPEN:
			printf("reopen");
wait:	
			if(hdr->waitfor_reopen.checks_lo || hdr->waitfor_reopen.checks_hi) {
				int					checks = hdr->waitfor_reopen.checks_lo | hdr->waitfor_reopen.checks_hi << 8;

				printf(" %d.%d", checks / 10, checks % 10);
			}
			printf(" %s\n", hdr->waitfor_reopen.fname);
			break;

		case SCRIPT_TYPE_DISPLAY_MSG:
			printf("display_msg '%s'\n", hdr->display_msg.msg);
			break;

		case SCRIPT_TYPE_PROCMGR_SYMLINK:
			printf("procmgr_symlink '%s' '%s'\n", hdr->procmgr_symlink.src_dest,
				&hdr->procmgr_symlink.src_dest[strlen(hdr->procmgr_symlink.src_dest)+1]);
			break;

		case SCRIPT_TYPE_EXTSCHED_APS:
			if (ext_sched == SCRIPT_SCHED_EXT_NONE)
				associate(SCRIPT_APS_SYSTEM_PARTITION_ID, SCRIPT_APS_SYSTEM_PARTITION_NAME);
			else if (ext_sched != SCRIPT_SCHED_EXT_APS)
				printf("Invalid combination of SCHED_EXT features\n");
			ext_sched = SCRIPT_SCHED_EXT_APS;
			printf("sched_aps %s %d %d\n", associate(hdr->extsched_aps.id, hdr->extsched_aps.pname), hdr->extsched_aps.budget, hdr->extsched_aps.critical_hi << 8 | hdr->extsched_aps.critical_lo);
			break;

		default:
			printf("Unknown type %d\n", hdr->hdr.type);
		}
	}
}

void process(const char *file, FILE *fp) {
	struct startup_header		shdr = { STARTUP_HDR_SIGNATURE };
	int							spos;
	struct image_header			ihdr = { IMAGE_SIGNATURE };
	int							ipos;
	int							dpos;
	static char					buf[0x10000];
	static char 				out_buf[0x10000];

	spos = -1;
	if((ipos = find(fp, ihdr.signature, sizeof ihdr.signature, 0)) == -1) {
		rewind(fp);
		//find startup signature and verify its validity
		while(1){
			if((spos = find(fp, (char *)&shdr.signature, sizeof shdr.signature, -1)) == -1) {
				shdr.signature = ENDIAN_RET32(shdr.signature);
				rewind(fp);
				if((spos = find(fp, (char *)&shdr.signature, sizeof shdr.signature, -1)) == -1) {
					error(1, "Unable to find startup header in %s", file);
					return;
				}
			}
			if(fread((char *)&shdr + sizeof shdr.signature, sizeof shdr - sizeof shdr.signature, 1, fp) != 1) {
				error(1, "Unable to read image %s", file);
				return;
			}
			// if we are cross endian, flip the shdr members
			{
				// flip shdr members
			}

			//check the zero member of startup, one more check to confirm
			//we found the correct signature
			if (!zero_ok(&shdr)) {
				if (zero_check_enabled)
					continue;
				if (verbose)
					printf("Warning: Non zero data in zero fields ignored\n");
				break;
			} else {
				break;
			}
		}

		// If the image is compressed we need to uncompress it into a
		// tempfile and restart.
		if((shdr.flags1 & STARTUP_HDR_FLAGS1_COMPRESS_MASK) != 0) {
			FILE	*fp2;
			int		n;

			// Create a file to hold uncompressed image.
			if(ucompress_file) {
				fp2 = fopen(ucompress_file, "w+");
			} else {
				fp2 = tmpfile();
			}
		
			if(fp2 == NULL) {
				error(1, "Unable to create a file to uncompress image.");
				return;
			}

			// Copy non-compressed part.
			rewind(fp);
			if(CROSSENDIAN(shdr.flags1 & STARTUP_HDR_FLAGS1_BIGENDIAN))
				for(n = 0 ; n < spos + ENDIAN_RET32(shdr.startup_size); ++n) {
					putc(getc(fp), fp2);
				}
			else
				for(n = 0 ; n < spos + shdr.startup_size; ++n) {
					putc(getc(fp), fp2);
				}
			fflush(fp2);

			// Uncompress compressed part
			switch(shdr.flags1 & STARTUP_HDR_FLAGS1_COMPRESS_MASK) {
			case STARTUP_HDR_FLAGS1_COMPRESS_ZLIB:
				{
					int			fd;
					gzFile			*zin;

					// We need an fd for zlib, and we cannot count on the fd
					// position being the same as the FILE *'s, so fileno() 
					// and lseek.
					fd = fileno(fp);
					lseek(fd, ftell(fp), SEEK_SET);
					if((zin = gzdopen(fd, "rb")) == NULL) {
						error(1, "Unable to open decompression stream.");
						return;
					}
					while((n = gzread(zin, buf, sizeof(buf))) > 0) {
						fwrite(buf, n, 1, fp2);
					}
				}
				break;
			case STARTUP_HDR_FLAGS1_COMPRESS_LZO:
				{
					unsigned	len;
					lzo_uint	out_len;
					int			status;
				
					if(lzo_init() != LZO_E_OK) {
						error(1,"decompression init failure");
						return;
					}
					for(;;) {
						len = getc(fp) << 8;
						len += getc(fp);
						if(len == 0) break;
						fread(buf, len, 1, fp);
						status = lzo1x_decompress(buf, len, out_buf, &out_len, NULL);
						if(status != LZO_E_OK) {
							error(1, "decompression failure");
							return;
						}
						fwrite(out_buf, out_len, 1, fp2);
					}
				}
				break;
			case STARTUP_HDR_FLAGS1_COMPRESS_UCL:
				{
					unsigned	len;
					ucl_uint	out_len;
					int			status;

					for(;;) {
						len = getc(fp) << 8;
						len += getc(fp);
						if(len == 0) break;
						fread(buf, len, 1, fp);
						status = ucl_nrv2b_decompress_8(buf, len, out_buf, &out_len, NULL);
						if(status != 0) {
							error(1, "decompression failure");
							return;
						}
						fwrite(out_buf, out_len, 1, fp2);
					}
				}
				break;
			default:
				error(1, "Unsupported compression type.");
				return;
			}

			fclose(fp);
			fp = fp2;
			rewind(fp2);
		} 

		if(CROSSENDIAN(shdr.flags1 & STARTUP_HDR_FLAGS1_BIGENDIAN)) {
			unsigned long			*p;

			shdr.version = ENDIAN_RET16(shdr.version);
			shdr.machine = ENDIAN_RET16(shdr.machine);
			shdr.header_size = ENDIAN_RET16(shdr.header_size);
			shdr.preboot_size = ENDIAN_RET16(shdr.preboot_size);
			for(p = (unsigned long *)&shdr.startup_vaddr; (unsigned char *)p < (unsigned char *)&shdr + sizeof shdr; p++) {
				if(p != (unsigned long *)&shdr.preboot_size) {
					*p = ENDIAN_RET32(*p);
				}
			}
		}
		fseek(fp, spos + shdr.startup_size, SEEK_SET);
		if((ipos = find(fp, ihdr.signature, sizeof ihdr.signature, -1)) == -1) {
			error(1, "Unable to find image header in %s", file);
			return;
		}
	}
	if(fread((char *)&ihdr + sizeof ihdr.signature, sizeof ihdr - sizeof ihdr.signature, 1, fp) != 1) {
		error(1, "Unable to read image %s", file);
		return;
	}
	if(CROSSENDIAN(ihdr.flags & IMAGE_FLAGS_BIGENDIAN)) {
		unsigned long			*p;

		for(p = (unsigned long *)&ihdr.image_size; (unsigned char *)p < (unsigned char *)&ihdr + offsetof(struct image_header, mountpoint); p++) {
			*p = ENDIAN_RET32(*p);
		}
	}

	dpos = ipos + ihdr.dir_offset;

	if(flags & FLAG_DISPLAY) {
		printf("   Offset     Size  Name\n");
		if(spos != -1) {
			if(spos) {
				if(check("*.boot") == 0) {
					printf(" %8x %8x  %s\n", 0, spos, "*.boot");
				}
			}
			display_shdr(fp, spos, &shdr);
			if(check("startup.*") == 0) {
				printf(" %8x %8lx  %s\n", spos + sizeof shdr, shdr.startup_size - sizeof shdr, "startup.*");
			}
		}
		display_ihdr(fp, ipos, &ihdr);
		if(check("Image-directory") == 0) {
			printf(" %8x %8lx  %s\n", dpos, ihdr.hdr_dir_size - ihdr.dir_offset, "Image-directory");
		}
	}

	while(!processing_done) {
		char						buff[1024];
		union image_dirent			*dir = (union image_dirent *)buff;

		fseek(fp, dpos, SEEK_SET);
		if(fread(&dir->attr, sizeof dir->attr, 1, fp) != 1) {
			error(1, "Early end reading directory");
			break;
		}
		if(CROSSENDIAN(ihdr.flags & IMAGE_FLAGS_BIGENDIAN)) {
			unsigned long			*p;

			dir->attr.size = ENDIAN_RET16(dir->attr.size);
			dir->attr.extattr_offset = ENDIAN_RET16(dir->attr.extattr_offset);
			for(p = (unsigned long *)&dir->attr.ino; (unsigned char *)p < (unsigned char *)dir + sizeof dir->attr; p++) {
				*p = ENDIAN_RET32(*p);
			}
		}
		if(dir->attr.size < sizeof dir->attr) {
			if(dir->attr.size != 0) {
				error(1, "Invalid dir entry");
			}
			break;
		}
		if(dir->attr.size > sizeof dir->attr) {
			if(fread((char *)dir + sizeof dir->attr, min(sizeof buff, dir->attr.size) - sizeof dir->attr, 1, fp) != 1) {
				error(1, "Error reading directory");
				break;
			}
		}
		dpos += dir->attr.size;

		switch(dir->attr.mode & S_IFMT) {
		case S_IFREG:
			if(CROSSENDIAN(ihdr.flags & IMAGE_FLAGS_BIGENDIAN)) {
				dir->file.offset = ENDIAN_RET32(dir->file.offset);
				dir->file.size = ENDIAN_RET32(dir->file.size);
			}
			process_file(fp, ipos, &dir->file);
			if(dir->attr.ino == ihdr.script_ino && verbose > 1) {
				display_script(fp, ipos + dir->file.offset, dir->file.size);
			}
			break;
		case S_IFDIR:
			process_dir(fp, ipos, &dir->dir);
			break;
		case S_IFLNK:
			if(CROSSENDIAN(ihdr.flags & IMAGE_FLAGS_BIGENDIAN)) {
				dir->symlink.sym_offset = ENDIAN_RET16(dir->symlink.sym_offset);
				dir->symlink.sym_size = ENDIAN_RET16(dir->symlink.sym_size);
			}
			process_symlink(fp, ipos, &dir->symlink);
			break;
		case S_IFCHR:
		case S_IFBLK:
		case S_IFIFO:
		case S_IFNAM:
			if(CROSSENDIAN(ihdr.flags & IMAGE_FLAGS_BIGENDIAN)) {
				dir->device.dev = ENDIAN_RET32(dir->device.dev);
				dir->device.rdev = ENDIAN_RET32(dir->device.rdev);
			}
			process_device(fp, ipos, &dir->device);
			// dir->device;
			break;
		default:
			error(1, "Unknown type\n");
			break;
		}
	}
	if(flags & FLAG_DISPLAY) {
		struct image_trailer	itlr;
		struct startup_trailer	stlr;

		fseek(fp, ipos + ihdr.image_size-sizeof(itlr), SEEK_SET);
		if(fread(&itlr, sizeof(itlr), 1, fp) != 1) {
			error(1, "Early end reading image trailer");
			return;
		}
		if(CROSSENDIAN(shdr.flags1 & STARTUP_HDR_FLAGS1_BIGENDIAN))
			printf("Checksums: image=%#lx", ENDIAN_RET32(itlr.cksum));
		else
			printf("Checksums: image=%#lx", itlr.cksum);
		if(spos != -1) {
			fseek(fp, spos + shdr.startup_size-sizeof(stlr), SEEK_SET);
			if(fread(&stlr, sizeof(stlr), 1, fp) != 1) {
				printf("\n");
				error(1, "Early end reading startup trailer");
				return;
			}
			if(CROSSENDIAN(shdr.flags1 & STARTUP_HDR_FLAGS1_BIGENDIAN))
				printf(" startup=%#lx", ENDIAN_RET32(stlr.cksum));
			else
				printf(" startup=%#lx", stlr.cksum);
		}
		printf("\n");
	}
}

int copy(FILE *from, FILE *to, int nbytes) {
	char			buff[4096];
	int				n;

	while(nbytes > 0) {
		n = min(sizeof buff, nbytes);
		if(fread(buff, n, 1, from) != 1) {
			return -1;
		}
		if(fwrite(buff, n, 1, to) != 1) {
			return -1;
		}
		nbytes -= n;
	}
	return 0;
}

void display_shdr(FILE *fp, int spos, struct startup_header *hdr) {
	if(check("Startup-header") != 0) {
		return;
	}
	printf(" %8x %8x  %s flags1=%#x flags2=%#x paddr_bias=%#lx\n",
				spos, hdr->header_size, "Startup-header",
				hdr->flags1, hdr->flags2,
				hdr->paddr_bias);
	if(verbose) {
		printf("                       preboot_size=%#lx\n", (unsigned long)hdr->preboot_size);
		printf("                       image_paddr=%#lx stored_size=%#lx\n", (unsigned long)hdr->image_paddr, (unsigned long)hdr->stored_size);
		printf("                       startup_size=%#lx imagefs_size=%#lx\n", (unsigned long)hdr->startup_size, (unsigned long)hdr->imagefs_size);
		printf("                       ram_paddr=%#lx ram_size=%#lx\n", (unsigned long)hdr->ram_paddr, (unsigned long)hdr->ram_size);
		printf("                       startup_vaddr=%#lx\n", (unsigned long)hdr->startup_vaddr);
	}
}

void display_ihdr(FILE *fp, int ipos, struct image_header *hdr) {
	if(check("Image-header") != 0) {
		return;
	}
	printf(" %8x %8x  %s", ipos, sizeof *hdr, "Image-header");
	if(hdr->mountpoint[0]) {
		char				buff[512];

		fseek(fp, ipos + offsetof(struct image_header, mountpoint), SEEK_SET);
		if(fread(buff, min(sizeof buff, hdr->dir_offset - offsetof(struct image_header, mountpoint)), 1, fp) != 1) {
			error(1, "Unable to read image mountpoint");
		} else {
			printf(" mountpoint=%s", buff);
		}
	}
	printf("\n");
	if(verbose) {
		int						i;

		printf("                       flags=%#x", hdr->flags);
		if(hdr->chain_paddr) {
			printf(" chain=%#lx", hdr->chain_paddr);
		}
		if(hdr->script_ino) {
			printf(" script=%ld", hdr->script_ino);
		}
		for(i = 0; i < sizeof hdr->boot_ino / sizeof hdr->boot_ino[0]; i++) {
			if(hdr->boot_ino[i]) {
				printf(" boot=%ld", hdr->boot_ino[i]);
				for(i++; i < sizeof hdr->boot_ino / sizeof hdr->boot_ino[0]; i++) {
					if(hdr->boot_ino[i]) {
						printf(",%ld", hdr->boot_ino[i]);
					}
				}
				break;
			}
		}
		for(i = 0; i < sizeof hdr->spare / sizeof hdr->spare[0]; i++) {
			if(hdr->spare[i]) {
				printf(" spare=%#lx", hdr->spare[0]);
				for(i = 1; i < sizeof hdr->spare / sizeof hdr->spare[0]; i++) {
					printf(",%ld", hdr->spare[i]);
				}
				break;
			}
		}
		printf(" mntflg=%#lx\n", hdr->mountflags);
	}
}

void display_attr(struct image_attr *attr) {
	printf("                       gid=%ld uid=%ld mode=%#lo ino=%ld mtime=%lx\n", attr->gid, attr->uid, attr->mode & ~S_IFMT, attr->ino, attr->mtime);
}

void display_dir(FILE *fp, int ipos, struct image_dir *ent) {
	if(check(ent->path[0] ? ent->path : "Root-dirent") != 0) {
		return;
	}
	printf("     ----     ----  %s\n", ent->path[0] ? ent->path : "Root-dirent");
	if(verbose) {
		display_attr(&ent->attr);
	}
}

void process_dir(FILE *fp, int ipos, struct image_dir *ent) {
	if(flags & FLAG_DISPLAY) {
		display_dir(fp, ipos, ent);
	}
}

void display_symlink(FILE *fp, int ipos, struct image_symlink *ent) {
	if(check(ent->path) != 0) {
		return;
	}
	printf("     ---- %8x  %s -> %s\n", ent->sym_size, ent->path, &ent->path[ent->sym_offset]);
	if(verbose) {
		display_attr(&ent->attr);
	}
}

void process_symlink(FILE *fp, int ipos, struct image_symlink *ent) {
	if(flags & FLAG_DISPLAY) {
		display_symlink(fp, ipos, ent);
	}
}

void display_device(FILE *fp, int ipos, struct image_device *ent) {
	if(check(ent->path) != 0) {
		return;
	}
	printf("     ----     ----  %s dev=%ld rdev=%ld\n", ent->path, ent->dev, ent->rdev);

	if(verbose) {
		display_attr(&ent->attr);
	}
}

void process_device(FILE *fp, int ipos, struct image_device *ent) {
	if(flags & FLAG_DISPLAY) {
		display_device(fp, ipos, ent);
	}
}

void extract_file(FILE *fp, int ipos, struct image_file *ent) {
	char			*name;
	FILE			*dst;
	struct utimbuf	buff;
	struct extract_file *ef;

	if(check(ent->path) != 0) {
		return;
	}

	name = (flags & FLAG_BASENAME) ? basename(ent->path) : ent->path;

	if ( extract_files != NULL ) {
		for ( ef = extract_files; ef != NULL; ef = ef->next ) {
			if ( !strcmp( ef->path, name ) ) {
				break;
			}
		}
		if ( ef == NULL ) {
			return;
		}
		if ( --files_left_to_extract == 0 ) {
			processing_done = 1;
		}
	}

	if(!(dst = fopen(name, "wb"))) {
		error(0, "Unable to open %s: %s\n", name, strerror(errno));
	}

	fseek(fp, ipos + ent->offset, SEEK_SET);
	ftruncate(fileno(dst), ent->size); /* pregrow the dst file */
	if(copy(fp, dst, ent->size) == -1) {
		unlink(name);
		error(0, "Unable to create file %s: %s\n", name, strerror(errno));
	}
	fchmod(fileno(dst), ent->attr.mode & 07777);
	fchown(fileno(dst), ent->attr.uid, ent->attr.gid);
	fclose(dst);
	buff.actime = buff.modtime = ent->attr.mtime;
	utime(name, &buff);
	if(verbose) {
		printf("Extracted %s\n", name);
	}
}

void process_file(FILE *fp, int ipos, struct image_file *ent) {
	if(flags & FLAG_EXTRACT) {
		extract_file(fp, ipos, ent);
	}
	if(flags & (FLAG_MD5|FLAG_DISPLAY)) {
		display_file(fp, ipos, ent);
	}
}

const char *ehdr_type[] = {
	"ET_NONE",
	"ET_REL",
	"ET_EXEC",
	"ET_DYN",
	"ET_CORE"
};
	
const char *ehdr_machine[] = {
	"EM_NONE",
	"EM_M32",
	"EM_SPARC",
	"EM_386",
	"EM_68k",
	"EM_88k",
	"EM_486",
	"EM_860",
	"EM_MIPS",
	"EM_UNKNOWN9",
	"EM_MIPS_RS3_LE",
	"EM_RS6000",
	"EM_UNKNOWN12",
	"EM_UNKNOWN13",
	"EM_UNKNOWN14",
	"EM_PA_RISC",
	"EM_nCUBE",
	"EM_VPP500",
	"EM_SPARC32PLUS",
	"EM_UNKNOWN19",
	"EM_PPC",
	"EM_UNKNOWN21",
	"EM_UNKNOWN22",
	"EM_UNKNOWN23",
	"EM_UNKNOWN24",
	"EM_UNKNOWN25",
	"EM_UNKNOWN26",
	"EM_UNKNOWN27",
	"EM_UNKNOWN28",
	"EM_UNKNOWN29",
	"EM_UNKNOWN30",
	"EM_UNKNOWN31",
	"EM_UNKNOWN32",
	"EM_UNKNOWN33",
	"EM_UNKNOWN34",
	"EM_UNKNOWN35",
	"EM_UNKNOWN36",
	"EM_UNKNOWN37",
	"EM_UNKNOWN38",
	"EM_UNKNOWN39",
	"EM_ARM",
	"EM_UNKNOWN41",
	"EM_SH",
};

const char *phdr_type[] = {
	"PT_NULL",					/* Program header table entry unused */
	"PT_LOAD",					/* Loadable program segment */
	"PT_DYNAMIC",				/* Dynamic linking information */
	"PT_INTERP",				/* Program interpreter */
	"PT_NOTE",					/* Auxiliary information */
	"PT_SHLIB",					/* Reserved, unspecified semantics */
	"PT_PHDR",					/* Entry for header table itself */
};

const char *phdr_flags[] = {
	"---",
	"--X",
	"-W-",
	"-WX",
	"R--",
	"R-X",
	"RW-",
	"RWX"
};

void dsp_index(const char *strv[], unsigned max, int index) {
	if(index < 0 || index >= max || !strv[index]) {
		printf("Unknown(%d)", index);
	} else {
		printf("%s", strv[index]);
	}
}

void display_elf(FILE *fp, int pos, int size, char *name) {
	Elf32_Ehdr			ehdr;

	fseek(fp, pos, SEEK_SET);
	if(fread(&ehdr, sizeof ehdr, 1, fp) != 1) {
		return;
	}
	if(memcmp(ehdr.e_ident, ELFMAG, SELFMAG)) {
		return;
	}
	if(verbose <= 2) {
		printf("                       ");
	}
	printf("----- %s ", name);
	if(ehdr.e_ident[EI_VERSION] != EV_CURRENT) {
		printf("ELF version %d -----\n", ehdr.e_ident[EI_VERSION]);
		return;
	}
	switch(ehdr.e_ident[EI_CLASS]) {
	case ELFCLASS32:
		printf("- ELF32");
		break;
	case ELFCLASS64:
		printf("- ELF64");
		break;
	default:
		printf("ELF executable - invalid class -----\n");
		return;
	}

	switch(ehdr.e_ident[EI_DATA]) {
	case ELFDATA2LSB:
		printf("LE ");
		break;
	case ELFDATA2MSB:
		printf("BE ");
		break;
	default:
		printf(" executable - unknown endian -----\n");
		return;
	}
	if(ehdr.e_ident[EI_CLASS] == ELFCLASS64) {
		printf("-----\n");
		return;
	}

	if(CROSSENDIAN(ehdr.e_ident[EI_DATA] == ELFDATA2MSB)) {
		ehdr.e_type = ENDIAN_RET16(ehdr.e_type);
		ehdr.e_machine = ENDIAN_RET16(ehdr.e_machine);
		ehdr.e_version = ENDIAN_RET32(ehdr.e_version);
		ehdr.e_entry = ENDIAN_RET32(ehdr.e_entry);
		ehdr.e_phoff = ENDIAN_RET32(ehdr.e_phoff);
		ehdr.e_shoff = ENDIAN_RET32(ehdr.e_shoff);
		ehdr.e_flags = ENDIAN_RET32(ehdr.e_flags);
		ehdr.e_ehsize = ENDIAN_RET16(ehdr.e_ehsize);
		ehdr.e_phentsize = ENDIAN_RET16(ehdr.e_phentsize);
		ehdr.e_phnum = ENDIAN_RET16(ehdr.e_phnum);
		ehdr.e_shentsize = ENDIAN_RET16(ehdr.e_shentsize);
		ehdr.e_shnum = ENDIAN_RET16(ehdr.e_shnum);
		ehdr.e_shstrndx = ENDIAN_RET16(ehdr.e_shstrndx);
	}

	if(ehdr.e_ehsize < sizeof ehdr) {
		printf("-----\n");
		error(1, "ehdr too small");
		return;
	}

	dsp_index(ehdr_type, sizeof ehdr_type / sizeof *ehdr_type, ehdr.e_type);
	printf(" ");
	dsp_index(ehdr_machine, sizeof ehdr_machine / sizeof *ehdr_machine, ehdr.e_machine);
	printf(" -----\n");
	if(verbose <= 2) {
		return;
	}
	if(ehdr.e_ehsize != sizeof ehdr) {
		printf(" e_ehsize             : %d\n", ehdr.e_ehsize);
	}
	printf(" e_flags              : %#lx\n", (unsigned long)ehdr.e_flags);
	if(ehdr.e_entry || ehdr.e_phoff || ehdr.e_phnum || ehdr.e_phentsize) {
		printf(" e_entry              : %#lx\n", (unsigned long)ehdr.e_entry);
		printf(" e_phoff              : %ld\n", (unsigned long)ehdr.e_phoff);
		printf(" e_phentsize          : %d\n", ehdr.e_phentsize);
		printf(" e_phnum              : %d\n", ehdr.e_phnum);
	} else if(ehdr.e_entry) {
		printf(" e_entry              : %#lx\n", (unsigned long)ehdr.e_entry);
	}
	if(ehdr.e_shoff || ehdr.e_shnum || ehdr.e_shentsize || ehdr.e_shstrndx) {
		printf(" e_shoff              : %ld\n", (unsigned long)ehdr.e_shoff);
		printf(" e_shentsize          : %d\n", ehdr.e_shentsize);
		printf(" e_shnum              : %d\n", ehdr.e_shnum);
	}
	if(ehdr.e_shstrndx) {
		printf(" e_shstrndx           : %d\n", ehdr.e_shstrndx);
	}
	if(ehdr.e_phnum && ehdr.e_phentsize >= sizeof(Elf32_Phdr)) {
		int				n;
		for(n = 0; n < ehdr.e_phnum; n++) {
			Elf32_Phdr			phdr;

			fseek(fp, pos + ehdr.e_phoff + n * ehdr.e_phentsize, SEEK_SET);
			if(fread(&phdr, sizeof phdr, 1, fp) != 1) {
				error(1, "Unable to read phdr %d\n", n);
				break;
			}
			printf(" segment %d\n", n);
			if(CROSSENDIAN(ehdr.e_ident[EI_DATA] == ELFDATA2MSB)) {
				phdr.p_type = ENDIAN_RET32(phdr.p_type);
				phdr.p_offset = ENDIAN_RET32(phdr.p_offset);
				phdr.p_vaddr = ENDIAN_RET32(phdr.p_vaddr);
				phdr.p_paddr = ENDIAN_RET32(phdr.p_paddr);
				phdr.p_filesz = ENDIAN_RET32(phdr.p_filesz);
				phdr.p_memsz = ENDIAN_RET32(phdr.p_memsz);
				phdr.p_flags = ENDIAN_RET32(phdr.p_flags);
				phdr.p_align = ENDIAN_RET32(phdr.p_align);
			}
			printf("   p_type               : ");
			switch(phdr.p_type) {
#ifndef PT_COMPRESS
#define PT_COMPRESS		0x4000
#endif
			case PT_COMPRESS:
				printf("PT_COMPRESS");
				break;
#ifndef PT_SEGREL
#define PT_SEGREL		0x4001
#endif
			case PT_SEGREL:
				printf("PT_SEGREL");
				break;

			default:
				dsp_index(phdr_type, sizeof phdr_type / sizeof *phdr_type, phdr.p_type);
				break;
			}
			printf("\n");
			printf("   p_offset             : %ld\n", (unsigned long)phdr.p_offset);
			printf("   p_vaddr              : 0x%lX\n", (unsigned long)phdr.p_vaddr);
			printf("   p_paddr              : 0x%lX\n", (unsigned long)phdr.p_paddr);
			printf("   p_filesz             : %ld\n", (unsigned long)phdr.p_filesz);
			printf("   p_memsz              : %ld\n", (unsigned long)phdr.p_memsz);
			printf("   p_flags              : ");
			dsp_index(phdr_flags, sizeof phdr_flags / sizeof *phdr_flags, phdr.p_flags);
			printf("\n");
			printf("   p_align              : %ld\n", (unsigned long)phdr.p_align);
		}
	}
	printf("----------\n");
}

int zero_ok (struct startup_header *shdr) {
	return(shdr->zero[0] == 0 &&
           shdr->zero[1] == 0 &&
           shdr->zero[2] == 0 );
}

void display_file(FILE *fp, int ipos, struct image_file *ent) {
	if(check(ent->path) != 0) {
		return;
	}
	printf(" %8lx %8lx  %s", ipos + ent->offset, ent->size, ent->path);
	if (flags & FLAG_MD5) {
		unsigned char	md5_result[MD5_LENGTH];
		int				i;

		compute_md5(fp, ipos, ent, md5_result);
		printf(" ");
		for ( i = 0; i < MD5_LENGTH; i++ ) {
			printf("%02x", md5_result[i] );
		}
	}
	printf("\n");
	if(verbose) {
		display_attr(&ent->attr);
	}
	if(verbose > 1) {
		display_elf(fp, ipos + ent->offset, ent->size, basename(ent->path));
	}
}

void compute_md5(
	FILE *fp,
	int ipos,
	struct image_file *ent,
	unsigned char *md5_result)
{
	unsigned char buff[4096];
	int nbytes, n;
	MD5_CTX md5_ctx;

	if (fseek(fp, ipos + ent->offset, SEEK_SET) != 0) {
		error(0, "fseek on source file for MD5 calculation failed: %s\n", strerror(errno));
	}

	memset((unsigned char*)&md5_ctx, 0, sizeof(md5_ctx));
	MD5Init(&md5_ctx);

	nbytes = ent->size;

	while (nbytes > 0) {
		n = min(sizeof buff, nbytes);
		if(fread(buff, n, 1, fp) != 1) {
			error(0, "Error reading %d bytes for MD5 calculation: %s\n", n, strerror(errno));
		}

		MD5Update(&md5_ctx, buff, n);
		nbytes -= n;
	}

	MD5Final(md5_result, &md5_ctx);
}

#ifdef __QNXNTO__
__SRCVERSION("dumpifs.c $Rev: 207305 $");
#endif
