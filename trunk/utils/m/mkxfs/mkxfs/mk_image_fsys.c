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



#include <lib/compat.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <stddef.h>
#include <time.h>
#include <sys/stat.h>
#include "struct.h"
#include <zlib.h>
#include <lzo1x.h>
#include <ucl/ucl.h>
#include "xplatform.h"


struct soname_entry {
	struct soname_entry	*next;
	struct file_entry	*fip;
	int					make_other_the_targpath;
	char				other_name[1];
};

struct keep_section {
	struct keep_section	*next;
	char				*name;
	Elf32_Shdr			shdr;
};

struct soname_entry	*soname_head;
char 				copybuf[4096];
unsigned	 		image_cksum = 0;
unsigned	 		image_offset;		// uncompressed image offset
unsigned	 		cimage_offset;		// compressed image offset
unsigned	 		ram_offset;
void				*compress_fp;
char				*compress_name;
struct name_list	*section_list;

#if !(defined(__QNX__) || defined(__QNXNTO__))
	#define PT_SEGREL	0x4001
#endif


#define EHost32( m, v )		swap32((m)->big_endian, v)
#define EHost16( m, v )		swap16((m)->big_endian, v)
#define ETarget32( m, v )	swap32((m)->big_endian, v)
#define ETarget16( m, v )	swap16((m)->big_endian, v)

#define	K					* 1024
#define M					K K

#define MAX(_a,_b)			(((_a) > (_b)) ? (_a) : (_b))
#define MIN(_a,_b)			(((_a) < (_b)) ? (_a) : (_b))

#define NUM_ELTS( array )	(sizeof(array) / sizeof((array)[0]))

//
//  +--------------+
//  | boot prefix  | Provided as a file by virtual= or physical= attribute
//  +--------------+	-		-
//  | startup hdr  |    |    	|
//  +--------------+  startup  Only present for a bootable image
//  | startup      |  chksum    |
//  +--------------+    |		|
//  | startup tlr  |    |		|
//  +--------------+    -       -
//  | img header   |    |
//  +--------------+    |
//  |              |    |
//  | directory    |    |
//  |              |    |
//  +--------------+  image
//  | file 1       |  chksum
//  +--------------+    |
//  | file 2       |    |
//  +--------------+    |
//  | file n       |    |
//  +--------------+    |
//  | img trailer  |    |
//  +--------------+    -
//  | boot postfix | Applied by another program if needed (mkifsf_elf)
//  +--------------+
//
//
// boot prefix - On a PC BIOS boot it gathers info via BIOS calls and
//               switches into 32 bit protected mode.
//             - On some systems it and the boot postfix allow network boot
//
// startup hdr - Stuff needed mostly for a ROM/FLASH boot. Contains type/value
//               pairs for stuff like where the memory is.
//
//
//

void
ifs_section(struct name_list **owner, const char *name) {
	struct name_list	*kn;

	if(owner == NULL) owner = &section_list;
	kn = malloc(sizeof(*kn) + strlen(name));
	if(kn == NULL) {
		error_exit("No memory for section list information.\n");
	}
	kn->next = *owner;
	strcpy(kn->name, name);
	*owner = kn;
}

static void
fill_addr_defaults(struct addr_space *space, struct addr_space *def) {
	if(space->addr == 0) space->addr = def->addr;
	if(space->maxsize == 0) space->maxsize = def->maxsize ? def->maxsize : ~0UL;
	if(space->endaddr == 0) space->endaddr = def->endaddr ? def->endaddr : ~0UL;
	if(space->totalsize == 0) space->totalsize = def->totalsize;
	if(space->align == 0) space->align = (def->align >= 4) ? def->align : 4;
};

/*
Congruence truth table (care of bstecher)
               u_flags
            CD  CD  UD  UD
            CC  UC  CC  UC
            --- -X- W-- WX-
    ---      0   0   0   0

 p  --R      0   x   0   x
 _  -X-      0   x   0   x
 f  -XR      0   x   0   x
 l
 a  W--      0   0   x   x
 g  W-R      0   0   x   x
 s  WX-      0   0   x   x
    WXR      0   0   x   x
*/
unsigned
seg_uip_congruence(struct file_entry *fip, Elf32_Phdr *phdr) {
	unsigned	p_flags;
	unsigned	u_flags;

	p_flags = EHost32(fip, phdr->p_flags);
	if(p_flags == 0) return(0);
	if(p_flags & PF_W) p_flags &= ~PF_X;  // In case data segment is execute
	if(fip->flags & FILE_FLAGS_STARTUP) return(~0UL);
	u_flags = fip->attr->uip_flags;
	if(split_image && !(fip->flags & FILE_FLAGS_BOOT)) u_flags &= ~PF_W;
	if(fip->flags & FILE_FLAGS_SO) u_flags |= PF_W;

	if(!(u_flags & p_flags)) return(0);
	if(booter.virtual) {
		if((fip->flags & FILE_FLAGS_BOOT) && (booter.vboot_addr == 0)) {
			return(~0UL);
		}
		return(EHost32(fip, phdr->p_align)-1);
	}
	if(fip->flags & (FILE_FLAGS_BOOT|FILE_FLAGS_RELOCATED)) return(~0UL);
	return(0);
}


int
expand_out(int congruence, struct file_entry *fip) {
	// This function is attempting to determine if we need to pad out
	// the binary up the phdr->p_memsz, rather than just writing out
	// for a length of phdr->p_filesz. Only the first test should be
	// required, but because of a bug in the procnto executable loader
	// and also in lib/c/ldd.c where they alway mmap() for phdr->p_memsz
	// rather then phdr->p_filesz and then a seperate anonymous mmap()
	// for "phdr->p_memsz - phdr->p_filesz", we have to make sure that
	// we pad out non-shared object binaries. The bug in
	// procnto & ldd.c will be fixed in the 6.4.0 release, so the release
	// after that we should be able to remove this kludge.
	if(congruence == ~0L) return 1;
	if(fip->flags & FILE_FLAGS_SO) return 0;
	return 1;
}


int
ropen(struct file_entry *fip) {
	int			 fd;
	static char	*lastcd;

	if(*fip->hostpath != '/'  &&  fip->attr  &&  lastcd != fip->attr->cd)
		chdir(lastcd = fip->attr->cd);

	fd = open(fip->hostpath, O_RDONLY);
	if(fd == -1) {
		error_exit("Unable to open %s : %s\n", fip->hostpath, strerror(errno));
	}

	MAKE_BINARY_FD(fd);
	return(fd);
}

static void
check_over(char *fname, char *sname, struct addr_space *space, unsigned offset) {

	if(offset > space->maxsize) {
		if(fname != NULL) fprintf(stderr, "%s: ", fname);
		error_exit("%s area exceeds 0x%x maximum size.\n", sname, space->maxsize);
	}
	if((space->addr + offset) > space->endaddr) {
		if(fname != NULL) fprintf(stderr, "%s: ", fname);
		error_exit("%s area exceeds 0x%x end address.\n", sname, space->endaddr);
	}
}

#define COMPRESS_ENUM(type)	\
	COMPRESS_##type= (STARTUP_HDR_FLAGS1_COMPRESS_##type >> STARTUP_HDR_FLAGS1_COMPRESS_SHIFT)

enum {
	COMPRESS_ENUM(ZLIB),
	COMPRESS_ENUM(LZO),
	COMPRESS_ENUM(UCL)
};

#define BUFFSIZE_LZO	0x10000
struct compress_lzo {
	FILE			*fp;
	unsigned		in_off;
	unsigned char	work[LZO1X_999_MEM_COMPRESS];
	unsigned char	in[BUFFSIZE_LZO];
	unsigned char	out[BUFFSIZE_LZO+(BUFFSIZE_LZO/64 + 16 + 3)];
};

static struct compress_lzo *
lzoopen(const char *name) {
	struct compress_lzo	*lzo;

	lzo = malloc(sizeof(*lzo));
	if(lzo == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	lzo->fp = fopen(name, "wb");
	if(lzo->fp == NULL) {
		return NULL;
	}
	if(lzo_init() != LZO_E_OK) {
		errno = EDOM; //Strange error so we know what failed.
		return NULL;
	}
	lzo->in_off = 0;
	return lzo;
}

static int
lzoflush(struct compress_lzo *lzo) {
	int			status;
	lzo_uint	out_len;
	unsigned char		*buf;
	unsigned	len;

	buf = lzo->in;
	len = lzo->in_off;

	while(lzo->in_off != 0) {
		status = lzo1x_999_compress(buf, len, lzo->out, &out_len, lzo->work);
		if(status != LZO_E_OK) {
			errno = EDOM;
			return 0;
		}
		if(out_len >= 0x10000) {
			//Didn't compress, try smaller block value so we can use
			//2 byte lengths in file
			len -= 0x1000;
			continue;
		}
		//write out block
		clearerr(lzo->fp);
		putc(out_len >> 8, lzo->fp);
		putc(out_len & 0xff, lzo->fp);
		if((fwrite(lzo->out, 1, out_len, lzo->fp) != out_len) || ferror(lzo->fp)) {
			return 0;
		}
		buf += len;
		lzo->in_off -= len;
		len = lzo->in_off;
	}
	return 1;
}

static int
lzowrite(struct compress_lzo *lzo, const void *buf, size_t len) {
	size_t	add;

	for( ;; ) {
		add = len;
		if((add + lzo->in_off) < sizeof(lzo->in)) break;
		add = sizeof(lzo->in) - lzo->in_off;
		memcpy(&lzo->in[lzo->in_off], buf, add);
		lzo->in_off += add;
		len -= add;
		buf = (unsigned char *)buf + add;
		if(lzoflush(lzo) == 0) return 0;
	}
	memcpy(&lzo->in[lzo->in_off], buf, add);
	lzo->in_off += add;
	return 1;
}

static int
lzoclose(struct compress_lzo *lzo) {
	int		status = 1;

	if(lzo->in_off != 0) {
		status = lzoflush(lzo);
	}
	//Mark end of compression
	putc(0, lzo->fp);
	putc(0, lzo->fp);
	fclose(lzo->fp);
	free(lzo);
	return status;
}

#define BUFFSIZE_UCL	0x10000
struct compress_ucl {
	FILE			*fp;
	unsigned		in_off;
	unsigned char	in[BUFFSIZE_UCL];
	unsigned char	out[BUFFSIZE_UCL+(BUFFSIZE_UCL/8 + 256)];
};

static struct compress_ucl *
uclopen(const char *name) {
	struct compress_ucl	*ucl;

	ucl = malloc(sizeof(*ucl));
	if(ucl == NULL) {
		errno = ENOMEM;
		return NULL;
	}
	ucl->fp = fopen(name, "wb");
	if(ucl->fp == NULL) {
		return NULL;
	}
	ucl->in_off = 0;
	return ucl;
}

static int
uclflush(struct compress_ucl *ucl) {
	int				status;
	unsigned		out_len;
	unsigned char	*buf;
	unsigned		len;

	buf = ucl->in;
	len = ucl->in_off;

	while(ucl->in_off != 0) {
		status = ucl_nrv2b_99_compress(buf, len, ucl->out, &out_len, NULL, 9, NULL, NULL);
		if(status != 0) {
			errno = EDOM;
			return 0;
		}
		if(out_len >= 0x10000) {
			//Didn't compress, try smaller block value so we can use
			//2 byte lengths in file
			len -= 0x1000;
			continue;
		}
		//write out block
		clearerr(ucl->fp);
		putc(out_len >> 8, ucl->fp);
		putc(out_len & 0xff, ucl->fp);
		if((fwrite(ucl->out, 1, out_len, ucl->fp) != out_len) || ferror(ucl->fp)) {
			return 0;
		}
		buf += len;
		ucl->in_off -= len;
		len = ucl->in_off;
	}
	return 1;
}

static int
uclwrite(struct compress_ucl *ucl, const void *buf, size_t len) {
	size_t	add;

	for( ;; ) {
		add = len;
		if((add + ucl->in_off) < sizeof(ucl->in)) break;
		add = sizeof(ucl->in) - ucl->in_off;
		memcpy(&ucl->in[ucl->in_off], buf, add);
		ucl->in_off += add;
		len -= add;
		buf = (unsigned char *)buf + add;
		if(uclflush(ucl) == 0) return 0;
	}
	memcpy(&ucl->in[ucl->in_off], buf, add);
	ucl->in_off += add;
	return 1;
}

static int
uclclose(struct compress_ucl *ucl) {
	int		status = 1;

	if(ucl->in_off != 0) {
		status = uclflush(ucl);
	}
	//Mark end of compression
	putc(0, ucl->fp);
	putc(0, ucl->fp);
	fclose(ucl->fp);
	free(ucl);
	return status;
}

static void
compress_start(void) {
	compress_name = mk_tmpfile();
	switch(compressed) {
	case COMPRESS_ZLIB:
		if((compress_fp = gzopen(compress_name, "wb")) == NULL) {
			error_exit("Error opening compression stream.\n");
		}
		break;
	case COMPRESS_LZO:
		if((compress_fp = lzoopen(compress_name)) == NULL) {
			error_exit("Error opening compression stream: %s.\n", strerror(errno));
		}
		break;
	case COMPRESS_UCL:
		if((compress_fp = uclopen(compress_name)) == NULL) {
			error_exit("Error opening compression stream: %s.\n", strerror(errno));
		}
		break;
	default:
		error_exit("Unsupported compression type %d.\n", compressed);
		break;
	}
}

static void
compress_stop(void) {
	switch(compressed) {
	case COMPRESS_ZLIB:
		gzclose(compress_fp);
		break;
	case COMPRESS_LZO:
		lzoclose(compress_fp);
		break;
	case COMPRESS_UCL:
		uclclose(compress_fp);
		break;
	default:
		//Should never happen
		error_exit("Unsupported compression type %d - 2.\n", compressed);
		break;
	}
	compress_fp = NULL;
}

void
iwrite(void *buf, int nbytes, FILE *dst_fp, char *fname) {
	char	*cp;

	if(compress_fp != NULL) {
		switch(compressed) {
		case COMPRESS_ZLIB:
			if(gzwrite(compress_fp, buf, nbytes) == 0) {
				error_exit("Error writing compression file: %s.\n", strerror(errno));
			}
			break;
		case COMPRESS_LZO:
			if(lzowrite(compress_fp, buf, nbytes) == 0) {
				error_exit("Error writing compression file: %s.\n", strerror(errno));
			}
			break;
		case COMPRESS_UCL:
			if(uclwrite(compress_fp, buf, nbytes) == 0) {
				error_exit("Error writing compression file: %s.\n", strerror(errno));
			}
			break;
		default:
			//Should never happen
			error_exit("Unsupported compression type %d - 3.\n", compressed);
			break;
		}
	} else {
		if(fwrite(buf, 1, nbytes, dst_fp) != nbytes) {
			error_exit("Error writing image: %s.\n", strerror(errno));
		}
	}

	for(cp = buf ; nbytes ; --nbytes, ++cp, ++image_offset) {
		static unsigned char	hold_cksum[4];
		unsigned 				index = image_offset & 0x3;

		hold_cksum[index] = *cp;
		if(index == 0x3) {
			if(target_endian) {
				image_cksum +=
					  ((unsigned long)hold_cksum[0] << 24)
					+ ((unsigned long)hold_cksum[1] << 16)
					+ ((unsigned long)hold_cksum[2] <<  8)
					+ ((unsigned long)hold_cksum[3]);
			} else {
				image_cksum +=
					  ((unsigned long)hold_cksum[3] << 24)
					+ ((unsigned long)hold_cksum[2] << 16)
					+ ((unsigned long)hold_cksum[1] <<  8)
					+ ((unsigned long)hold_cksum[0]);
			}
		}
	}

	check_over(fname, "image", &image, image_offset);
}


void
padfile(FILE *dst_fp, unsigned off, char *fname) {
	static char		zeros[1024];
	int				nbytes, n;

	if(image_offset > off) {
		error_exit("%s: internal error in function padfile (%x>%x).\n", fname, image_offset, off);
	}

	for(nbytes = off - image_offset ; nbytes ; nbytes -= n) {
		n = nbytes;
		if(n > sizeof(zeros)) n = sizeof(zeros);
		iwrite(zeros, n, dst_fp, fname);
	}
}


void
copy_boot(int fd, FILE *dst_fp, int nbytes, struct file_entry *fip) {
	int		size, n;
	char	*buf, *p;

	buf = malloc(nbytes);
	if(read(fd, buf, nbytes) != nbytes) {
		error_exit("Error reading %s\n", fip->hostpath);
	}

	for(p = buf, n = nbytes ; n ; --n, ++p) {
		if(p[0] == 'd'  &&  p[1] == 'd'
		&& p[2] == 'p'  &&  p[3] == 'v'
		&& p[4] == 'b'  &&  p[5] == 's'
		&& p[6] == 'k'  &&  p[7] == 'r') {
			size = fip->bootargs->size_lo + (fip->bootargs->size_hi << 8);
			memcpy(p, fip->bootargs, size);
			break;
		}
	}

	iwrite(buf, nbytes, dst_fp, fip->hostpath);

	free(buf);
}


void
copy_data(int fd, FILE *dst_fp, int nbytes, struct file_entry *fip) {
	int n;

	if(fip->bootargs) {
		copy_boot(fd, dst_fp, nbytes, fip);
		return;
	}

	while((n = read(fd, copybuf, MIN(nbytes, sizeof(copybuf)))) > 0) {
		iwrite(copybuf, n, dst_fp, fip->hostpath);
		nbytes -= n;
	}
}

static char init_strtab[] = { "\0.shstrtab" };

static unsigned
section_keep(struct file_entry *fip, int fd, Elf32_Ehdr *ehdr) {
	unsigned 	i, num, shoff, shstrndx, strtab_size;
	char 		*strtab;
	unsigned	size;
	Elf32_Shdr	shdr;
	struct name_list	*list;

	list = fip->attr->keepsection;
	if(list == NULL) {
		if(fip->flags & FILE_FLAGS_BOOT) {
			return 0;
		}
		list = section_list;
	}

	shstrndx = EHost16(fip, ehdr->e_shstrndx);
	if(shstrndx == SHN_UNDEF) {
		return 0; /* no string table */
	}

	shoff = EHost32(fip, ehdr->e_shoff);
	if(shoff == 0) {
		return 0; /* no section headers */
	}

	/* get section header for string table */
	if(lseek(fd, shoff + sizeof(shdr) * shstrndx, SEEK_SET) == -1 ||
	  read(fd, &shdr, sizeof(shdr)) != sizeof(shdr)){
		fprintf(stderr, "Warning: Failed reading string table section header in %s.\n", fip->hostpath);
		return 0;
	}

	strtab_size = EHost32(fip, shdr.sh_size);
	if(strtab_size == 0) {
		return 0; /* empty string table */
	}
	
	strtab = malloc(strtab_size);
	if(strtab == NULL) {
		return 0;
	}

	size = 0;
		
	/* read entire string table */
	if(lseek(fd, EHost32(fip, shdr.sh_offset), SEEK_SET) == -1 ||
	  read(fd, strtab, strtab_size) != strtab_size){
		fprintf(stderr, "Warning: Failed reading string table in %s.\n", fip->hostpath);
		goto cleanup;
	}

	num = EHost16(fip, ehdr->e_shnum);

	if(lseek(fd, EHost32(fip, ehdr->e_shoff), SEEK_SET) == -1){
		fprintf(stderr, "Warning: Failed reading section header table in %s.\n", fip->hostpath);
		goto cleanup;
	}

	strtab_size = sizeof(init_strtab);
	
	for(i = 0; i < num; i++) {
		char				*name;
		struct name_list	*chk;
		struct keep_section	*ks;

		read(fd, &shdr, sizeof(shdr));
		name = &strtab[EHost32(fip, shdr.sh_name)];
		for(chk = list; chk != NULL; chk = chk->next) {
			if(strcmp(name, chk->name) == 0) {
				ks = malloc(sizeof(*ks));
				if(ks == NULL) {
					error_exit("No memory for keep section information.\n");
				}
				ks->next = fip->sect;
				ks->name = chk->name;
				ks->shdr = shdr;
				fip->sect = ks;
				size += sizeof(shdr) + EHost32(fip, shdr.sh_size);
				strtab_size += strlen(name) + 1;
				break;
			}
		}
	}
	// Allow for string table and empty section header
	if(size != 0) {
		size += 2*sizeof(shdr) + strtab_size;
	}
cleanup:
	free(strtab);
	return size;
}

static void
section_prep(struct file_entry *fip, Elf32_Ehdr *ehdr, unsigned off) {
	unsigned	num;
	struct keep_section	*ks;

	if(fip->sect != NULL) {
		num = 2;
		for(ks = fip->sect; ks != NULL; ks = ks->next) {
			++num;
		}

		ehdr->e_shoff = ETarget32(fip, off);
		ehdr->e_shentsize = ETarget16(fip, sizeof(fip->sect->shdr));
		ehdr->e_shnum = ETarget16(fip, num);
		ehdr->e_shstrndx = ETarget16(fip, num-1);
	}
}

static void
section_write(struct file_entry *fip, int fd, Elf32_Ehdr *ehdr, FILE *dst_fp) {
	Elf32_Shdr			shdr;
	unsigned			off;
	unsigned			strtab_size;
	unsigned			num;
	struct keep_section	*ks;

	if(fip->sect != NULL) {
		num = 2;
		for(ks = fip->sect; ks != NULL; ks = ks->next) {
			++num;
		}

		off = EHost32(fip, ehdr->e_shoff);
		padfile(dst_fp, off+fip->file_offset, fip->hostpath);

		off += num * sizeof(shdr);

		// Write out section headers

		/* write a blank header */
		memset(&shdr, 0, sizeof(shdr));
		shdr.sh_type = SHT_NULL;
		shdr.sh_link = SHN_UNDEF;
		iwrite(&shdr, sizeof(shdr), dst_fp, fip->hostpath);
		strtab_size = sizeof(init_strtab);
		for(ks = fip->sect; ks != NULL; ks = ks->next) {
			shdr = ks->shdr;
			shdr.sh_offset = ETarget32(fip, off);
			shdr.sh_name = ETarget32(fip, strtab_size);
			iwrite(&shdr, sizeof(shdr), dst_fp, fip->hostpath);
			off += EHost32(fip, shdr.sh_size);
			strtab_size += strlen(ks->name) + 1;
		}
		/* create a string table header */
		shdr.sh_name = ETarget32(fip, 1);
		shdr.sh_size = ETarget32(fip, strtab_size);
		shdr.sh_offset = ETarget32(fip, off);
		shdr.sh_type = ETarget32(fip, SHT_STRTAB);
		shdr.sh_addralign = ETarget32(fip, 1);
		iwrite(&shdr, sizeof(shdr), dst_fp, fip->hostpath);

		// Write out section data
		for(ks = fip->sect; ks != NULL; ks = ks->next) {
			lseek(fd, EHost32(fip, ks->shdr.sh_offset), SEEK_SET);
			copy_data(fd, dst_fp, EHost32(fip, ks->shdr.sh_size), fip);
		}

		// Write out string table
		iwrite(init_strtab, sizeof(init_strtab), dst_fp, fip->hostpath);
		for(ks = fip->sect; ks != NULL; ks = ks->next) {
			iwrite(ks->name, strlen(ks->name)+1, dst_fp, fip->hostpath);
		}
	}
}

void
copy_elf(int fd, FILE *dst_fp, struct file_entry *fip) {
	Elf32_Ehdr			ehdr;
	Elf32_Phdr			*phdrv, *phdr;
	struct {
		unsigned	off;
		unsigned	size;
		unsigned	hdr_adjust;
	}					*sinfo;
	unsigned			i;
	unsigned			j;
	unsigned			num;
	unsigned			size;
	unsigned			segsize;
	unsigned			off;
	unsigned			ehdr_size;
#ifdef COPY_BOOTFILES
	unsigned			ram_loc;
#endif

	if(read(fd, &ehdr, sizeof(ehdr)) != sizeof(ehdr)) {
		error_exit("Failed reading ELF header in %s.\n", fip->hostpath);
	}

	num = EHost16(fip, ehdr.e_phnum);
	size = num * EHost16(fip, ehdr.e_phentsize);
	phdrv = alloca(size);
	sinfo = alloca(num * sizeof(*sinfo));
	if(lseek(fd, EHost32(fip, ehdr.e_phoff), SEEK_SET) == -1 || read(fd, phdrv, size) != size) {
		error_exit("Failed reading program header table in %s.\n", fip->hostpath);
	}

	ehdr_size = sizeof(ehdr) + size;
	off = 0;
	if(!(fip->flags & FILE_FLAGS_STARTUP)) {
		off = ehdr_size;
		ehdr.e_shoff = 0;
		ehdr.e_shentsize = 0;
		ehdr.e_shnum = 0;
		ehdr.e_shstrndx = 0;
		ehdr.e_phoff = ETarget32(fip, sizeof(ehdr));
	}
#ifdef COPY_BOOTFILES
	ram_loc = fip->ram_offset;
#endif

	// Trim out segments which we don't want to copy
	for(i = 0; i < num; ++i) {
		phdr = &phdrv[i];
		sinfo[i].off = EHost32(fip, phdr->p_offset);
		sinfo[i].size = 0;
		sinfo[i].hdr_adjust = 0;
		switch(EHost32(fip, phdr->p_type)) {
		case PT_DYNAMIC:
		case PT_PHDR:
		case PT_INTERP:
			//NYI: have to shift starting offsets as appropriate
			break;
		case PT_NOTE:
			// Major league kludge here. Sometimes PT_NOTE's are contained
			// within a PT_LOAD segment, sometimes not. Really we should
			// make a map of all the segments that we want included in
			// the destination file and copy the bits identified by that
			// data structure. That would deal with all cases of overlap.
			// For right now, we'll just scan the this PT_NOTE's range
			// and see if it overlaps with a PT_LOAD's :-(.
			for(j = 0; j < num; ++j) {
				Elf32_Phdr	*p;
				unsigned	l_start;
				unsigned	l_end;

				p = &phdrv[j];
				if(EHost32(fip, p->p_type) == PT_LOAD) {
					l_start = EHost32(fip, p->p_offset);
					l_end   = l_start + EHost32(fip, phdr->p_filesz) - 1;
					if((sinfo[i].off >= l_start) && (sinfo[i].off <= l_end)) {
						// already covered by a PT_LOAD, we don't
						// have to write it out seperately
						continue;
					}
				}
			}
			// We need to write this segment out on it's own.
			goto do_load;

		case PT_SEGREL:
			if(fip->flags & FILE_FLAGS_STRIP_RELOCS) {
				phdr->p_type = ETarget32(fip, PT_NULL);
				break;
			}
			/* fall through */
		case PT_LOAD:
do_load:			
			segsize = EHost32(fip, phdr->p_filesz);
			if(segsize != 0) {
				unsigned	congruence;
				unsigned	vaddr;

				// Get the p_offset field pointing at the right spot
				sinfo[i].size = segsize;
				phdr->p_paddr = 0;
				vaddr = EHost32(fip, phdr->p_vaddr);
				if(phdr->p_offset == 0) {
					//
					// Load segment contains ehdr information. Got to
					// play some tricks when writing it out, since we're
					// going to output the new ehdr separately (or not
					// at all in the case of startup).
					//
					if(fip->flags & FILE_FLAGS_STARTUP) {
						sinfo[i].hdr_adjust = fip->run_offset - vaddr;
						vaddr = fip->run_offset;
					} else {
						sinfo[i].hdr_adjust = ehdr_size;
					}
				}

				congruence = seg_uip_congruence(fip, phdr);
				if(congruence > 0) {
					unsigned	disp;
					unsigned	adjusted_off;
#ifdef COPY_BOOTFILES
					unsigned	tst_flag;
#endif

					adjusted_off = off;
					if(!(fip->flags & FILE_FLAGS_STARTUP)) {	
						adjusted_off -= sinfo[i].hdr_adjust;
					}
					disp = (vaddr - fip->run_offset) & congruence;
					disp |= (adjusted_off & ~congruence);
					if(disp < adjusted_off) disp += congruence+1;
					adjusted_off = disp;
					if(expand_out(congruence, fip)) {
						phdr->p_filesz = phdr->p_memsz;
						segsize = EHost32(fip, phdr->p_memsz);
					}
#ifdef COPY_BOOTFILES
					if(EHost32(fip, phdr->p_flags) & PF_W) {
						tst_flag = FILE_FLAGS_DATA_IN_RAM;
					} else {
						tst_flag = FILE_FLAGS_CODE_IN_RAM;
					}
					if(fip->flags & tst_flag) {
						phdr->p_paddr = ETarget32(fip, ram_loc);
						ram_loc += segsize;
					} else {
						phdr->p_paddr = ETarget32(fip,
							adjusted_off + fip->file_offset + image.addr + booter.paddr_bias);
					}
#else
					phdr->p_paddr = ETarget32(fip,
						adjusted_off + fip->file_offset + image.addr + booter.paddr_bias);
#endif
					off = adjusted_off + sinfo[i].hdr_adjust;
				}
				phdr->p_offset = ETarget32(fip, off - sinfo[i].hdr_adjust);
				off += segsize - sinfo[i].hdr_adjust;
				if(fip->flags & FILE_FLAGS_STARTUP) {	
					sinfo[i].off  += sinfo[i].hdr_adjust;
					sinfo[i].size -= sinfo[i].hdr_adjust;
					sinfo[i].hdr_adjust = 0;
				}
				break;
			}
			/* fall through */
		default:
			phdr->p_type = ETarget32(fip, PT_NULL);
			break;
		}
	}

	if(!(fip->flags & FILE_FLAGS_STARTUP)) {
		section_prep(fip, &ehdr, off);
		iwrite(&ehdr, sizeof(ehdr), dst_fp, fip->hostpath);
		iwrite(phdrv, size, dst_fp, fip->hostpath);
	}

	for(i = 0; i < num; ++i) {
		phdr = &phdrv[i];
		if(sinfo[i].size != 0) {
//printf( "seg:%u, off:%8.8lx, size:%8.8x, file %s\n", i, EHost32(fip,phdr->p_offset), size, fip->targpath);
			off = EHost32(fip, phdr->p_offset) + sinfo[i].hdr_adjust + fip->file_offset;
			padfile(dst_fp, off, fip->hostpath);
			
			if(lseek(fd, sinfo[i].off + sinfo[i].hdr_adjust, SEEK_SET) == -1) {
				error_exit("Failed reading program segment %u in %s.\n", i, fip->hostpath);
			}

			copy_data(fd, dst_fp, sinfo[i].size - sinfo[i].hdr_adjust, fip);
		}
	}
	if(!(fip->flags & FILE_FLAGS_STARTUP)) {
		section_write(fip, fd, &ehdr, dst_fp);
	}
}

static int
match_class(struct elf_class *class, unsigned match) {
	unsigned	i;

	if(class->num == 0) return(1);
	for(i = 0; i < class->num; ++i) {
		if(class->list[i] == match) return(1);
	}
	return(0);
}

static char *
find_linker(unsigned machine, unsigned e_type, unsigned p_type) {
	struct linker_entry	*lnk;

	for(lnk = booter.linker; lnk != NULL; lnk = lnk->next) {
		if(match_class(&lnk->class[CLASS_MACHINE], machine)
		 &&match_class(&lnk->class[CLASS_EHDR], e_type)
		 &&match_class(&lnk->class[CLASS_PHDR], p_type)) return(lnk->spec);
	}
	return(NULL);
}

static unsigned
ram_start(struct file_entry *fip) {
	if((fip->ram_offset == 0) && split_image) {
		fip->ram_offset = ram.addr + ram_offset;
	}
	return(fip->ram_offset);
}
	
static void
bump_ram(struct file_entry *fip, unsigned size) {
	ram_start(fip);
	ram_offset += RUP(size, ram.align);
	check_over(fip->hostpath, "ram", &ram, ram_offset);
}

static void
add_soname(struct file_entry *fip, char *soname) {
	struct soname_entry	*new;
	int					other_is_real;
	int					len;
	char				ch;

//printf("Found SONAME in %s => '%s'\n", fip->hostpath, soname);
	if(strcmp(fip->targpath, soname) == 0) {
		ch = '\0';
		len = strlen(soname);
		for( ;; ) {
			--len;
			if(len < 0) break;
			ch = soname[len];
			if(!((ch == '.') || isdigit(ch))) break;
		}
		if((len < 0) || ch == '/') {
			return;
		}
		soname[len+1] = '\0';
		other_is_real = 0;
	} else {
		other_is_real = 1;
	}
	new = malloc(sizeof(struct soname_entry) + strlen(soname));
	if(new == NULL) {
		error_exit("No memory for SONAME structure.\n");
	}
	new->fip = fip;
	new->make_other_the_targpath = other_is_real;
	strcpy(new->other_name, soname);
	new->next = soname_head;
	soname_head = new;
}
//
// Detect elf executables. If it is a relocatable elf module invoke ld.
// We stuff fip->size to be the size of the file in the image. For an
// elf executable this will not in general be the size of the file since
// we may need to expand the data area with zeros when we lay it down.
// Return the amount of virtual address space that the executable will
// use when running.
// 
unsigned
classify_file(struct file_entry *fip) {
	int					fd;
	unsigned			i;
	unsigned			j;
	unsigned			num_phdrs;
	unsigned			size;
	unsigned			vstart;
	unsigned			vend;
	unsigned			vaddr;
	unsigned			off;
	unsigned			e_type;
	unsigned			p_type;
	Elf32_Ehdr			ehdr;
	Elf32_Phdr			*phdrv, *phdr;
	Elf32_Phdr			*pad_phdr;
	struct stat			sbuf;
	
	fip->size = 0;
	fip->linker = NULL;
	if(fip->attr->mode != S_IFREG) return(0);

	fd = ropen(fip);

	lseek (fd, 0L, SEEK_SET);
	// Is it an elf file?
	if (fip->attr->compress
	 || fip->attr->raw
	 || read (fd, &ehdr, sizeof ehdr) != sizeof ehdr
	 || memcmp (ehdr.e_ident, ELFMAG, SELFMAG) != 0) {

		// Not elf
		fstat(fd, &sbuf);
		//NYI: what to do if not a plain old data file (link, fifo, etc)?
		fip->size = sbuf.st_size;
		close(fd);
		if( (fip->flags & FILE_FLAGS_BOOT) && (fip->flags & FILE_FLAGS_STARTUP)) {
			error_exit("Couldn't find linker spec for boot/startup file: %s\n", fip->hostpath);
		}
		return(0);
	}

#ifndef HOST_HAS_EXECUTE_PERM
	// Don't have any execute permission on host system.
	// Make sure executables are marked execute in image file system.
	fip->host_perms |= S_IXUSR|S_IXGRP|S_IXOTH;
#endif

	fip->big_endian = (ehdr.e_ident[EI_DATA] != ELFDATA2LSB);
	if(target_endian < 0) target_endian = fip->big_endian;
	e_type = EHost16(fip, ehdr.e_type);
	switch(e_type) {
	case ET_DYN:
		fip->flags |= FILE_FLAGS_SO;
		break;
	case ET_EXEC:
		fip->flags |= FILE_FLAGS_EXEC;
		if(!(fip->flags & FILE_FLAGS_BOOT)) break;
		/* fall through */
	case ET_REL:
		fip->flags |= FILE_FLAGS_MUST_RELOC;
		break;
	}
	fip->machine = EHost16(fip, ehdr.e_machine);
	fip->entry = EHost32(fip, ehdr.e_entry);

	size = EHost16(fip, ehdr.e_phnum) * EHost16(fip, ehdr.e_phentsize);
	phdrv = alloca(size);
	if(lseek (fd, EHost32(fip, ehdr.e_phoff), SEEK_SET) == -1 || read (fd, phdrv, size) != size)
		error_exit("Failed reading program header table in %s.\n", fip->hostpath);

	off = 0;
	if(!(fip->flags & FILE_FLAGS_STARTUP)) {
		off = size + sizeof(Elf32_Ehdr);
	}

	vstart = ~0L;
	vend   = 0L;
	pad_phdr = NULL;
	num_phdrs = EHost16(fip, ehdr.e_phnum);
	for(i = 0; i < num_phdrs; ++i) {
		unsigned	congruence;
		unsigned	memsize;

		phdr = &phdrv[i];
		size = EHost32(fip, phdr->p_filesz);
		p_type = EHost32(fip, phdr->p_type);
		if(fip->linker == NULL) {
			fip->linker = find_linker(fip->machine, e_type, p_type);
		}
		switch(p_type) {
		case PT_NOTE:
			// Major league kludge here. Sometimes PT_NOTE's are contained
			// within a PT_LOAD segment, sometimes not. Really we should
			// make a map of all the segments that we want included in
			// the destination file and copy the bits identified by that
			// data structure. That would deal with all cases of overlap.
			// For right now, we'll just scan the this PT_NOTE's range
			// and see if it overlaps with a PT_LOAD's :-(.
			for(j = 0; j < num_phdrs; ++j) {
				Elf32_Phdr	*p;
				unsigned	l_start;
				unsigned	l_end;
				unsigned	n_start;

				p = &phdrv[j];
				if(EHost32(fip, p->p_type) == PT_LOAD) {
					n_start = EHost32(fip, phdr->p_offset);
					l_start = EHost32(fip, p->p_offset);
					l_end   = l_start + EHost32(fip, phdr->p_filesz) - 1;
					if((n_start >= l_start) && (n_start <= l_end)) {
						// already covered by a PT_LOAD, we don't
						// have to write it out seperately
						continue;
					}
				}
			}
			// We need to write this segment out on it's own.
			goto do_load;

		case PT_SEGREL:
			if(fip->flags & FILE_FLAGS_STRIP_RELOCS) break;
			/* fall through */
		case PT_LOAD:
do_load:			
			if(size == 0) break;
			pad_phdr = NULL;
			memsize = EHost32(fip, phdr->p_memsz);
			congruence = seg_uip_congruence(fip, phdr);
			if(congruence > 0) {
				unsigned	disp;
				uint32_t	vaddr;

				vaddr = EHost32(fip, phdr->p_vaddr);

				if((vaddr < fip->run_offset) && (fip->flags & FILE_FLAGS_STARTUP)) {
					//Somebody specified a -Ttext=? on the linker command
					//line where the linker script is set up to include
					//the elf file and program headers in the text segment
					//in such a way that it pushes the starting vaddr
					//back from what we expect. Since it's just the headers
					//that are in that first bit, we can just leave them 
					//off.
					disp = fip->run_offset - vaddr;
					memsize -= disp;
					vaddr += disp;
				}
				disp = (vaddr - fip->run_offset) & congruence;
				disp |= (off & ~congruence);
				if(disp < off) disp += congruence+1;
				off = disp;
				if(congruence == ~0UL) {
					fip->flags |= FILE_FLAGS_MUST_RELOC;
				}
				if(expand_out(congruence, fip)) {
					size = memsize;
				}
				pad_phdr = phdr;

				if(EHost32(fip, phdr->p_flags) & PF_W) {
					/* We can only run the executable once */
					fip->flags |= FILE_FLAGS_RUNONCE;
				}
			}
			if(split_image) {
				unsigned	tst_flag;

				if(EHost32(fip, phdr->p_flags) & PF_W) {
					tst_flag = FILE_FLAGS_DATA_IN_RAM;
				} else {
					tst_flag = FILE_FLAGS_CODE_IN_RAM;
				}
				if(fip->flags & tst_flag) {
					size = memsize;
					bump_ram(fip, size);
				}
			}
			if(phdr->p_flags != 0) {
				vaddr = EHost32(fip, phdr->p_vaddr);
				if(vaddr < vstart) vstart = vaddr;
				vaddr += EHost32(fip, phdr->p_memsz);
				if(vaddr > vend) vend = vaddr;
			}
				
			off += size;
			break;
		case PT_DYNAMIC:
			/* Find the True SO name for a DLL (Hi Peter!) */
			if((e_type == ET_DYN) && fip->attr->autolink) {
				unsigned		soname_off = -1;
				unsigned		strtab_ptr = -1;
				unsigned		seg_off;
				#define MAX_DYN 10
				Elf32_Dyn		dynamic[MAX_DYN];
				unsigned		num_dyn;
				unsigned		j;

				seg_off = EHost32(fip, phdr->p_offset);
				if(lseek (fd, seg_off, SEEK_SET) == -1) {
					error_exit("Failed seeking to segment in %s.\n", fip->hostpath);
				}
				j = MAX_DYN;
				num_dyn = EHost32(fip, phdr->p_filesz) / sizeof(Elf32_Dyn);
				for( ;; ) {
					if(num_dyn == 0) break;
					if(j >= MAX_DYN) {
						unsigned	dyn_size;

						dyn_size = num_dyn * sizeof(Elf32_Dyn);
						if(dyn_size > sizeof(dynamic)) dyn_size = sizeof(dynamic);
						if(read(fd, dynamic, dyn_size) != dyn_size) {
							error_exit("Failed reading dynamic information in %s.\n", fip->hostpath);
						}
						j = 0;
					}
					switch(EHost32(fip, dynamic[j].d_tag)) {
					case DT_SONAME:
						soname_off = EHost32(fip, dynamic[j].d_un.d_val);
						break;
					case DT_STRTAB:
						strtab_ptr = EHost32(fip, dynamic[j].d_un.d_ptr);
						break;
					}
					++j;
					--num_dyn;
					if(soname_off != -1 && strtab_ptr != -1) {
						for(j = 0; j < num_phdrs; ++j) {
							unsigned	targ_dir_len;
							char		*p;
							unsigned	base = EHost32(fip, phdrv[j].p_vaddr);
							unsigned	size = EHost32(fip, phdrv[j].p_memsz);

							if(strtab_ptr >= base && strtab_ptr < (base+size)) {
								soname_off += (strtab_ptr - base)
										+ EHost32(fip, phdrv[j].p_offset);
								if(lseek (fd, soname_off, SEEK_SET) == -1) {
									error_exit("Failed seeking to SONAME in %s.\n", fip->hostpath);
								}
								p = strrchr(fip->targpath, '/');
								if(p == NULL) {
									p = fip->targpath - 1;
								}
								targ_dir_len = p - fip->targpath + 1;
								memcpy(copybuf, fip->targpath, targ_dir_len);

								#define MAX_SONAME	256
								if(read(fd, &copybuf[targ_dir_len], MAX_SONAME) == -1) {
									error_exit("Failed reading SONAME in %s.\n", fip->hostpath);
								}
								/* only add soname if we managed to read one */
								if( copybuf[targ_dir_len] != '\0' )
									add_soname(fip, copybuf);
								break;
							}
						}
						break;
					}
				}
			}
			break;
		}
	}
	if(fip->linker == NULL) {
		//In case there's no program segment headers
		fip->linker = find_linker(fip->machine, e_type, PT_NULL);
		if(fip->linker == NULL
		 && !(fip->flags & FILE_FLAGS_RELOCATED)
		 && (fip->flags & FILE_FLAGS_MUST_RELOC)) {
			error_exit("Can not find required linker for '%s'.\n", fip->hostpath);
		}
	}

	
	if((fip->flags & FILE_FLAGS_RELOCATED)
	  || !(fip->flags & FILE_FLAGS_MUST_RELOC)) {
		off += section_keep(fip, fd, &ehdr);
	}

	fip->size = off;
	if(pad_phdr != NULL) {

		off = RUP(off, booter.pagesize);
		if(split_image) {
			unsigned	tst_flag;

			if(EHost32(fip, pad_phdr->p_flags) & PF_W) {
				tst_flag = FILE_FLAGS_DATA_IN_RAM;
			} else {
				tst_flag = FILE_FLAGS_CODE_IN_RAM;
			}
			if(fip->flags & tst_flag) {
				bump_ram(fip, off - fip->size);
			}
		}
		fip->size = off;
	}

	close(fd);
	return(RUP(vend, booter.pagesize) - (vstart & ~(booter.pagesize-1)));
}

//
// For relocatable elf files, invoke the linker to lock it down.
//

struct repeat_entry {
	struct repeat_entry	*prev;
	struct name_list	*name;
	char				*start;
};

unsigned
relocate(struct file_entry *fip, int text_addr, int data_addr, int ehdr_size, char* destname) {
	char			*linked_name;
	char			*name;
	char			*p;
	char			*fmt;
	unsigned		skip;
	unsigned long	var;
	unsigned long	value;
	enum {
		COND_EQ	= 0x01,
		COND_GT = 0x02,
		COND_LT = 0x04
	}				cond, cmp;
	struct repeat_entry		*rep;
	struct repeat_entry		*new;
	struct name_list		*list;
	int						want_dir;

	rep = NULL;

	if(fip->attr->keep_linked) {
		name = basename(fip->hostpath);
		linked_name = malloc(strlen(name) + (symfile_suffix ? strlen(symfile_suffix)+1:0) + 5);
		if(linked_name == NULL) {
			error_exit("No memory for link name.\n");
		}
		sprintf(linked_name, "%s%s%s.sym", name, symfile_suffix ? ".":"", symfile_suffix ?: "");
	} else {
		linked_name = mk_tmpfile();
	}

	/*
	 * Build the linker command line
	 */
	skip = 0;
	p = copybuf;
	fmt = fip->linker;
	while(*fmt != '\0') {
		if(*fmt == '%') {
			++fmt;
			if(skip > 0) {
				switch(*fmt) {
				case '[':	
				case '(':
					++skip;
					break;
				case ')':
				case ']':
					--skip;
					break;
				}
			} else {
				want_dir = 0;
				if(*fmt == '^') {
					want_dir = 1;
					++fmt;
				}
				switch(*fmt) {
				case 'h':
					p += sprintf(p, "%x", text_addr);
					break;
				case 't':
					p += sprintf(p, "%x", text_addr + ehdr_size);
					break;
				case 'd':
					p += sprintf(p, "%x", data_addr);
					break;
				case 'o':
					name = linked_name;
					if(want_dir) name = dirname(name);
					p += sprintf(p, "\"%s\"", name);
					break;
				case 'i':
					name = fip->hostpath;
					if(want_dir) name = dirname(name);
					p += sprintf(p, "\"%s\"", name);
					break;
				case '(':
					switch( *++fmt ) {
					case 'm':
						var = fip->machine;
						break;
					case 'e':
						var = fip->big_endian;
						break;
					case 'd':
						var = data_addr;
						break;
					case 'h':
						var = text_addr;
						break;
					case 'f':
						if(fip->flags & FILE_FLAGS_STARTUP) {
							var = 0;
						} else if(fip->flags & FILE_FLAGS_BOOT) {
							var = 1;
						} else {
							var = 2;
						}
						break;
					case 'v':
						var = (booter.virtual && !(fip->flags & FILE_FLAGS_STARTUP));
						break;
					case 'V':
						var = booter.virtual;
						break;
					default:
						error_exit("Unknown conditional variable '%c'\n", *fmt);
						var = 0;
						break;
					}
					cond = 0;
					switch(*++fmt) {
					case '!':
					    if(fmt[1] == '=') {
							cond = COND_GT | COND_LT;
							++fmt;
						}
						break;
					case '=':
					    if(fmt[1] == '=') {
							cond = COND_EQ;
							++fmt;
						}
						break;
					case '>':
						cond = COND_GT;
					    if(fmt[1] == '=') {
							cond |= COND_EQ;
							++fmt;
						}
						break;
					case '<':
						cond = COND_LT;
					    if(fmt[1] == '=') {
							cond |= COND_EQ;
							++fmt;
						}
						break;
					}
					if(cond == 0) {
						error_exit("Unknown conditional type '%c%c'\n", fmt[0], fmt[0]);
					}
					value = strtoul(fmt + 1, &fmt, 0);
					if(var == value) {
						cmp = COND_EQ;
					} else if(var > value) {
						cmp = COND_GT;
					} else {
						cmp = COND_LT;
					}
					if(!(cmp & cond)) ++skip;
					if(*fmt != ',') --fmt;
					break;
				case ')':
					/* nothing to do */
					break;
				case '[':
					switch(*++fmt) {
					case 'M':
						list = fip->attr->module_list;
						break;
					default:	
						list = NULL;
						error_exit("Unknown repeat variable '%c'\n", *fmt);
					}
					if(list != NULL) {
						new = malloc(sizeof(*new));
						if(new == NULL) {
							error_exit("No memory for repeat entry\n");
						}
						new->prev = rep;
						rep = new;
						rep->start = fmt;
						rep->name = list;
					} else {
						++skip;
					}
					break;
				case ']':
					rep->name = rep->name->next;
					if(rep->name != NULL) {
						fmt = rep->start;
					} else {
						new = rep;
						rep = rep->prev;
						free(new);
					}
					break;
				case 'n':
					if(rep == NULL) {
						error_exit("%%n used in non-repeated section.\n");
					}
					value = sprintf(p, "%s", rep->name->name);
					if(want_dir) {
						char 	*dash;

						dash = strchr(p, '-');
						if(dash != NULL) {
							value = dash - p;
						}
					}
					p += value;
					break;
				default:
					*p++ = *fmt;
					break;
				}
			}
		} else if(skip == 0) {
			*p++ = *fmt;
		}
		++fmt;
	}
	*p = '\0';

#if defined (__WIN32__) || defined(__NT__)
	fixenviron(copybuf, sizeof(copybuf));
#endif

	if(verbose >= 3) {
		fprintf(debug_fp, "Execute: %s\n", copybuf);
	}
	if(system(copybuf) != 0) {
		if(unlink(destname) != 0) {
			fprintf(stderr, "unlink of %s failed : %d (%s)\n",destname, errno, strerror(errno));
		}
		error_exit("Unable to link relocatable elf file %s.\n", fip->hostpath);
	}

	fip->hostpath = linked_name;
	fip->flags |= FILE_FLAGS_RELOCATED;
	// Recalc size of file in image....
	return(classify_file(fip));
}



//
// We classify an image filesystem into 3 types based upon how the executables
// are run.
// 
// CPY CODE - Both code and data is copied into ram. The image is probably
// CPY DATA  in paged or very slow rom/flash.
//
// UIP CODE - The code is used in place in rom/flash and data copied to ram.
// CPY DATA   The image is probably mapped into the linear address space.
//
// UIP CODE - The code and data is used in place in ram. Loading an image
// UIP DATA   over the network or from disk fits this model.
//
// Image layout of executables is as follows:
// Where:  E=elf header  C=code  D=data  Z=zeros  |=page boundry
//
// |    |    |    |    |   C   D
//   EECCCCDDDD           CPY CPY           - no alignment needed
// EECCCCDDDD             UIP CPY           - align hdr
// EECCCCDDDDZ            UIP UIP +virtual  - align hdr, zero fill last page
// EECCCCDDDDZZZZZZZZZZZ  UIP UIP -virtual  - align hdr, zero fill BSS to page
//
struct file_entry *
locate_files(struct file_entry *list, int offset, char *destname) {
	struct file_entry	*datalist;
	struct file_entry	*data;
	struct file_entry	*fip;
	struct file_entry	**owner;
	struct file_entry	**downer;
	unsigned			align, vsize;
	unsigned			ioffset;
	unsigned			group_address;
	int					group_id, new_group;

	//
	// Split elf and data files.
	// Elf and spacer files are kept in their original order.
	// Data files are sorted from biggest to smallest.
	//
	// We place elf files first.
	// Since they are often constrained to begin on a page boundry
	// this will leave some holes which we try and fill with data files later.
	//
	datalist = NULL;
	owner = &list;
	group_id = 0;
	for( ;; ) {
		fip = *owner;
		if(fip == NULL) break;
		if(fip->attr == NULL) {
			//Skip over the image directory entry
			owner = &fip->next;
		} else if(fip->linker != NULL || (fip->flags & (FILE_FLAGS_EXEC|FILE_FLAGS_SO))) {

			/* alignment should be based on the image load address */
			ioffset = offset + image.addr;

			if (fip->attr->uip_flags & PF_X) {
				// If the code is executed in place it must be on a page boundry.
				align = max(fip->attr->phys_align,booter.pagesize);
			} else {
				align = fip->attr->phys_align;
			}

			if(align) {
				new_group = 0;
				if ( fip->attr->phys_align ) {
					if ( fip->attr->phys_align_group ) {
						new_group = (group_id != fip->attr->phys_align_group);
					}
					else {
						new_group = 1;
					}
				}

				if ( new_group ) {
					// starting a new alignment group
					ioffset = RUP(ioffset, align);

					group_id = fip->attr->phys_align_group;
					group_address = ioffset;

					fip->file_offset = ioffset - image.addr;
				} else {
					if (ioffset + fip->size > (group_address + fip->attr->phys_align)) {
						// if we would straddle the alignment group, start a new group
						ioffset = RUP(ioffset, align);
						offset = fip->file_offset = ioffset - image.addr;
						group_id = 0;
					} else {
						// Otherwise continue the group
						if (fip->attr->uip_flags & PF_X) {
							// If the code is executed in place it must be on a page boundry.
							ioffset = RUP(ioffset, booter.pagesize);
						}
						offset = fip->file_offset = ioffset - image.addr;
					}
				}
			} else {
				fip->file_offset = offset;
			}

			if((fip->flags & FILE_FLAGS_BOOT) && (booter.vboot_addr != 0) ) {
				fip->run_offset = RUP(booter.vboot_addr, booter.pagesize);
			} else if(fip->flags & FILE_FLAGS_CODE_IN_RAM) {
				fip->run_offset = ram_start(fip);
			} else {
				fip->run_offset = fip->file_offset + image.addr;
			}
	
			// For relocatable elf files, invoke the linker to lock it down.
			if(fip->linker != NULL
			 && ((fip->flags & FILE_FLAGS_MUST_RELOC)
			     || !(fip->flags & FILE_FLAGS_EXEC))) {
				unsigned	data_start;
				
				data_start = 0;
				if(!split_image) {
					//Not a split image
				} else if(fip->flags & FILE_FLAGS_CODE_IN_RAM) {
					//Data still follows code
					fip->flags |= FILE_FLAGS_DATA_IN_RAM;
				} else if(fip->flags & FILE_FLAGS_BOOT) {
					// Bootstrap file
					if((booter.vboot_addr == 0) || !booter.virtual) {
						data_start = ram_start(fip);
						fip->flags |= FILE_FLAGS_DATA_IN_RAM;
					}
				} else {
					// Normal file
					if(!booter.virtual) {
						data_start = ram_start(fip);
						fip->flags |= FILE_FLAGS_DATA_IN_RAM;
					}
				}
				vsize = relocate(fip, fip->run_offset, data_start, booter.pagesize, destname);
				if((fip->flags & FILE_FLAGS_BOOT) && (booter.vboot_addr != 0) ) {
					booter.vboot_addr += vsize;
				}
			}
			offset = fip->file_offset + fip->size;
			owner = &fip->next;
		} else if(fip->attr->page_align) {
			fip->file_offset = RUP(offset, booter.pagesize);
			offset = fip->file_offset + fip->size;
			owner = &fip->next;
		} else {
			*owner = fip->next;
			// Put the data file on its own sorted list, we'll deal with
			// them later;
			downer = &datalist;
			for( ;; ) {
				data = *downer;
				if(data == NULL) break;
				if(data->size < fip->size) break;
				downer = &data->next;
			}
			fip->next = *downer;
			*downer = fip;
		}
	}

	//
	// We now place data files. We try and place them in holes
	// at the end of executables if possible.
	//
	while((data = datalist)) {
		unsigned	hole;

		datalist = data->next;
		owner = &list;
		for( ;; ) {
			fip = *owner;
			offset = fip->file_offset + fip->size;
			owner = &fip->next;
			if(*owner == NULL) break;
			hole = (*owner)->file_offset - offset;
			if(data->size <= hole) break;
		}
		data->next = *owner;
		*owner = data;
		data->file_offset = offset;
	}
	return(list);
}

static void
add_solink(struct file_entry *list, struct soname_entry *so) {
	struct file_entry		*fip;
	struct file_entry		*new;
	struct attr_file_entry	*attr;
	char					*p;

	// We have a SO name - see if someone else is using it.
	fip = list;
	for( ;; ) {
		if(fip == NULL) break;
		if(strcmp(so->other_name, fip->targpath) == 0) return;
		fip = fip->next;
	}
	//no one using the name, add the link
	new = malloc(sizeof(*new));
	attr = malloc(sizeof(*attr));
	if(new == NULL || attr == NULL) {
		error_exit("No memory for SONAME link entry.\n");
	}
	fip = so->fip;
	*new = *fip;
	*attr = *fip->attr;
	new->attr = attr;
	attr->mode &= ~S_IFMT;
	attr->mode |= S_IFLNK;
	if(so->make_other_the_targpath) {
		fip->targpath = strdup(so->other_name);
	} else {
		new->targpath = strdup(so->other_name);
	}
	p = strrchr(fip->targpath, '/');
	if(p == NULL) p = fip->targpath - 1;
	new->hostpath = p + 1;
	fip->next = new;
}

int
ifs_need_seekable(struct file_entry *list) {

	return compressed ? 1 : 0;
}

void
fixup_ram_offsets(struct file_entry *list)
{
	for ( ; list != NULL; list = list->next ) {
		if ( (list->flags &(FILE_FLAGS_SO|FILE_FLAGS_EXEC)) &&
				!(list->flags & FILE_FLAGS_RELOCATED) ) {
			list->ram_offset = list->ram_offset + ram_offset;
		}
	}
}

unsigned
ifs_make_fsys(FILE *dst_fp, struct file_entry *list, char *mountpoint, char *destname) {
	int						fd;
	int						n;
	unsigned				bsize;
	unsigned				ssize;
	unsigned				hsize;
	unsigned				dsize;
	unsigned				fsize;
	unsigned				tsize;
	unsigned				isize;
	unsigned				totalsize;
	unsigned				inode;
	struct file_entry		**owner;
	struct file_entry		*fip;
	struct file_entry		*startup;
	struct file_entry		fent;
	struct image_header		ihdr;
	struct image_trailer	itlr;
	struct startup_header	shdr;
	struct startup_trailer	stlr;
	union image_dirent		*dent;
	unsigned				ihdr_offset;
	int						shdr_file_offset = 0;
	int						stlr_file_offset = 0;
	int						stlr_cksum = 0;

	if(compressed && split_image) {
		error_exit( "You can't compress a split image (image=xxxx ram=xxx and +compress).\n");
	}

	// If no "-s" options were specified, use defaults.
	if(section_list == NULL) {
		ifs_section(NULL, "QNX_Phab");
		ifs_section(NULL, "QNX_info");
		ifs_section(NULL, "QNX_usage");
	}

	// KUDGE:
	// This isn't really right, but it'll work for now. We're assuming
	// that secondary (non-bootable) image file systems will only be
	// used on virtual systems (we have to turn on the virtual flag so
	// that UIP stuff gets done properly).
	if(booter.name == NULL) booter.virtual = 1;

	if(booter.pagesize == 0) booter.pagesize = 4 K;
	fill_addr_defaults(&ram, &default_ram);
	fill_addr_defaults(&image, &default_image);

	if(verbose) {
		fprintf(debug_fp, "  Offset   Size    Entry   Ramoff Target=Host\n");
	}

	tsize = sizeof(itlr);

	//
	// Calculate the size of each file and classify it as elf or not.
	//
	startup = NULL;
	inode = 1;
	owner = &list;
	for( ;; ) {
		fip = *owner;
		if(fip == NULL) break;
		//
		// Decide on whether we need to keep relocation information
		// in the executable or not.
		//
		if(fip->flags & FILE_FLAGS_BOOT) {
			fip->flags |= FILE_FLAGS_STRIP_RELOCS;
			if(split_image) {
				fip->flags |= FILE_FLAGS_DATA_IN_RAM;
#ifdef COPY_BOOTFILES
				if(!(fip->attr->uip_flags & PF_X) || (fip->flags & FILE_FLAGS_STARTUP)) {
#else
				if(fip->flags & FILE_FLAGS_STARTUP) {
#endif
					fip->flags |= FILE_FLAGS_CODE_IN_RAM;
				}
			}
#if 0	//Enable this later on
		} else if(booter.virtual) {
			if(!fip->attr->keep_relocs) {
				fip->flags |= FILE_FLAGS_STRIP_RELOCS;
			}
#endif
		} else if(fip->attr->strip_relocs) {
			fip->flags |= FILE_FLAGS_STRIP_RELOCS;
		}
		classify_file(fip);
		if(fip->flags & FILE_FLAGS_STARTUP) {
			// Remove startup program from normal image fsys
			startup = fip;
			*owner = fip->next;
		} else {
			fip->inode = ++inode;
			owner = &fip->next;
			while(soname_head != NULL) {
				struct soname_entry		*tmp;

				add_solink(list, soname_head);
				tmp = soname_head;
				soname_head = tmp->next;
				free(tmp);
			}
		}
	}
	if(!booter.virtual) booter.vboot_addr = 0;

	if(mountpoint == NULL) {
		mountpoint = (startup != NULL) ? "/" : "";
	}

	bsize = 0;
	ssize = 0;
	ihdr_offset = 0;
	//
	// If there is a boot header supplied we sneak it in right up front
	//
	if(booter.name != NULL) {
		if(startup == NULL) {
			error_exit( "No startup program found while creating bootable image\n");
		}

		iwrite(booter.data, booter.data_len, dst_fp, booter.name);
		bsize = booter.data_len;

		//
		// Pad out the file to what was asked and make sure it is a multiple
		// of 4 bytes so the startup header which follows is dword aligned.
		//
		while(bsize < booter.boot_len  ||  (bsize & 0x03)) {
			if(putc(0, dst_fp) == -1)
				error_exit("Error writing image file: %s .\n", strerror(errno));
			++bsize;
		}

		if(booter.notloaded_len > bsize) booter.notloaded_len = bsize;
		bsize -= booter.notloaded_len;

		if((bsize > 0) && verbose) {
			fprintf(debug_fp, "%8x %6x %8x      --- %s\n",
						image.addr, bsize, 0, booter.name);
		}
		image_cksum = 0;
		image_offset = bsize;

		//
		// Figure out where the startup program is going for a bootable image.
		//
		ram_offset = startup->file_offset = bsize + sizeof(shdr);
		if(split_image) {
			startup->run_offset = ram.addr + startup->file_offset;
		} else {
			startup->run_offset = image.addr + startup->file_offset;
		}
		relocate(startup, startup->run_offset, 0, 0, destname);
		ihdr_offset = RUP(bsize + sizeof(shdr) + startup->size + sizeof(stlr), 8);
		ssize = ihdr_offset - bsize;
		//
		// For split images, we want the ram_offset to begin after the startup
		// code + data
		//
		if(split_image) {
			ram_offset = ssize;
		}
	} else if(compressed != 0) {
		// We're going to write out a dummy startup header so we
		// can get the compression type recorded.
		ihdr_offset = ssize = sizeof(shdr) + sizeof(stlr);
	}

	//
	// If nobody's set or complained about the target endianness by this
	// point, just write stuff out in host endian format.
	//
	if(target_endian < 0) target_endian = host_endian;

	//
	// Calculate the size of the image header.
	// Stuff some of the image header in target endian order. We do more below.
	//
	memset(&ihdr, 0, sizeof(ihdr));
	memcpy(ihdr.signature, "imagefs", sizeof(ihdr.signature));
	hsize = RUP(offsetof(struct image_header, mountpoint) + 1 + strlen(mountpoint), 4);
	ihdr.flags = 0;
	ihdr.mountflags = 0;

	//
	// Calculate the size of all image directory entries.
	//
	dsize = RUP(offsetof(struct image_dir, path) + 1, 4);
	for(fip = list ; fip ; fip = fip->next) {
		switch(fip->attr->mode & S_IFMT) {
		case S_IFLNK:
			n = offsetof(struct image_symlink, path) + strlen(fip->targpath) + strlen(fip->hostpath) + 2;
			break;
		case S_IFREG:
			n = offsetof(struct image_file, path) + strlen(fip->targpath) + 1;
			break;
		case S_IFDIR:
			n = offsetof(struct image_dir, path) + strlen(fip->targpath) + 1;
			break;
		default:
			n = offsetof(struct image_device, path) + strlen(fip->targpath) + 1;
			break;
		}
		dsize += RUP(n, 4);	// Round up to 4 byte boundry
	}
	// We end the list with a size entry (of 0 size) so reserve space for it.
	dsize += RUP(sizeof(dent->attr.size), 4);

	// Make up a space entry for the directory. This allows locate_files()
	// to pack data files after it and before a page aligned executable.
	fent.flags = 0;
	fent.attr = NULL;
	fent.file_offset = 0;
	fent.size = bsize + ssize + hsize + dsize;
	fent.next = list;
	list = &fent;

	//
	// If required, adjust the vaddr to take startup into account
	//
	if((booter.vboot_addr != 0) && booter.rsvd_vaddr) { 
		booter.vboot_addr += bsize + ssize + hsize + dsize;
	}
	//
	// Sort and locate files in the image.
	//
	list = locate_files(list, bsize + ssize + hsize + dsize, destname);

	//
	// For split images - check if startup files and proc were relocated
	// All other files will need their ram_offset nudged up if this is
	// true
	//
	if(split_image) {
		fixup_ram_offsets(list);
	}

	//
	// Find the space reserved for files in the image. The fsize is assumed
	// to start right after the directory (hsize + dsize).
	// Look for special files and stuff their position (inode) in the header.
	// Remove the 'fent' entry from the list.
	//
	owner = &list;
	for( ;; ) {
		fip = *owner;
		if(fip == &fent) {
			*owner = fip->next;
		} else {
			owner = &fip->next;
		}
		if(fip->flags & (FILE_FLAGS_EXEC|FILE_FLAGS_SO)) {
			fip->inode |= IFS_INO_PROCESSED_ELF;
			if(fip->flags & FILE_FLAGS_RUNONCE) {
				fip->inode |= IFS_INO_RUNONCE_ELF;
			}
		}

		// Look for special boot files
		if(fip->flags & FILE_FLAGS_BOOT) {
			fip->inode |= IFS_INO_BOOTSTRAP_EXE;
			if(fip->bootargs) {
				fip->bootargs->shdr_addr = swap32(target_endian, image.addr + bsize);
			}
			//After a suitable period of time, we can remove the
			//setting of the boot_ino array and just assume that all
			//the startups have been upgraded to the new style.
			//	bstecher, 2004/06/17
			n = 0;
			for( ;; ) {
				if(n >= sizeof(ihdr.boot_ino)/sizeof(ihdr.boot_ino[0])) {
					if(new_style_bootstrap == 0) {
						error_exit("%s: Maximum of %d bootstrap files.\n", fip->hostpath, n);
					}
					if(new_style_bootstrap == 1) {
						fprintf(stderr, "%s: Old startups may not work with more than %d bootstrap files.\n", fip->hostpath, n);
					}
					break;
				}
				if(ihdr.boot_ino[n] == 0) {
					ihdr.boot_ino[n] = swap32(target_endian, fip->inode);
					break;
				}
				++n;
			}
		} else if(fip->flags & FILE_FLAGS_SCRIPT) {
			// Look for script file
			ihdr.script_ino = swap32(target_endian, fip->inode);
		}
		if(*owner == NULL) break;
	}
	fsize = RUP(fip->file_offset + fip->size, 4) - (bsize + ssize + hsize + dsize);

	totalsize = bsize+ssize+hsize+dsize+fsize+tsize;
	if(image.totalsize != 0) {
		if(totalsize > image.totalsize) {
			error_exit("Image size of 0x%x bytes exceeds total size of 0x%x.\n", totalsize, image.totalsize);
		}
		totalsize = image.totalsize;
	}
	totalsize = RUP(totalsize, image.align);
	isize = totalsize - (bsize+ssize);
	tsize = isize - (hsize+dsize+fsize);

	if((startup != NULL) || (compressed != 0)) {
		//
		// Put out the startup header
		//
		memset(&shdr, 0, sizeof(shdr));
		if(target_endian) shdr.flags1 |= STARTUP_HDR_FLAGS1_BIGENDIAN;
		shdr.signature = swap32(target_endian, STARTUP_HDR_SIGNATURE);
		shdr.version = swap16(target_endian, STARTUP_HDR_VERSION);
		shdr.header_size = swap16(target_endian, sizeof(shdr));
		if(startup != NULL) {
			if(booter.virtual) shdr.flags1 |= STARTUP_HDR_FLAGS1_VIRTUAL;
			shdr.machine = swap16(target_endian, startup->machine);
			shdr.startup_vaddr = swap32(target_endian, startup->entry);
			shdr.image_paddr = swap32(target_endian, image.addr + bsize + booter.paddr_bias);
			shdr.paddr_bias = swap32(target_endian, -booter.paddr_bias);
			if(split_image) {
				shdr.ram_paddr = swap32(target_endian, ram.addr + bsize + booter.paddr_bias);
				if(ram.totalsize != 0) {
					if(ram_offset > ram.totalsize) {
						error_exit("Ram size of 0x%x bytes exceeds total size of 0x%x.\n", ram_offset, ram.totalsize);
					}
					ram_offset = ram.totalsize;
				}
				shdr.ram_size = swap32(target_endian, RUP(ram_offset, ram.align));
			} else {
				shdr.ram_paddr = shdr.image_paddr;
				shdr.ram_size = swap32(target_endian, ssize+isize);
			}
		}
		shdr.startup_size = swap32(target_endian, ssize);
		shdr.stored_size = swap32(target_endian, ssize+isize);
		shdr.imagefs_size = swap32(target_endian, isize);
		shdr.preboot_size = swap16(target_endian, bsize);

		// For a compressed file we need to patch stored_size
		// after we figure out how much the image was compressed.
		if(compressed != 0) {
			shdr.stored_size = 0;
			shdr_file_offset = ftell(dst_fp);
			shdr.flags1 |= compressed << STARTUP_HDR_FLAGS1_COMPRESS_SHIFT;
		}
		iwrite(&shdr, sizeof(shdr), dst_fp, "Startup-header");

		if(startup != NULL) {
			if(verbose)
				fprintf(debug_fp, "%8x %6x     ----      --- Startup-header\n",
							image.addr + bsize, sizeof(shdr));
				
			if(startup->bootargs) {
				startup->bootargs->shdr_addr = swap32(target_endian, (split_image ? ram.addr : image.addr) + bsize);
			}
			fd = ropen(startup);
			copy_elf(fd, dst_fp, startup);
			padfile(dst_fp, ihdr_offset-sizeof(stlr), "Startup-trailer");
			close(fd);
		}

		if(compressed) {
			stlr_file_offset = ftell(dst_fp);
			stlr_cksum = image_cksum;
		}
		stlr.cksum = swap32(target_endian, -image_cksum);
		iwrite(&stlr, sizeof(stlr), dst_fp, "Startup-trailer");

		if((startup != NULL) && verbose) {
			fprintf(debug_fp, "%8x %6x %8x      --- %s",
						image.addr + bsize + sizeof(shdr), image_offset - sizeof(shdr), startup->entry,
						startup->hostpath);
			if(verbose>=2 && (fip->flags & FILE_FLAGS_CRC_VALID)) {
				fprintf(debug_fp, " (%u)", startup->host_file_crc);
			}
			fprintf(debug_fp, "\n");
		}
		image_cksum = 0;

	}

	//
	// Put out the image header
	//
	ihdr.image_size = swap32(target_endian, isize);
	ihdr.hdr_dir_size = swap32(target_endian, hsize + dsize);
	ihdr.dir_offset = swap32(target_endian, hsize);
	ihdr.chain_paddr = swap32(target_endian, chain_paddr);

	ihdr.flags |= IMAGE_FLAGS_INO_BITS;
	if(target_endian != 0) ihdr.flags |= IMAGE_FLAGS_BIGENDIAN;
	if(split_image) ihdr.flags |= IMAGE_FLAGS_READONLY;

	if(compressed) {
		cimage_offset = image_offset;	// Remember offset
		compress_start();
	}
	iwrite(&ihdr, n = offsetof(struct image_header, mountpoint), dst_fp, "Image-header");
	{
		/* hsize is rounded up to 4 bytes, and hsize includes the nul terminating
		 * character.  So, there's 1 to 4 bytes of padding to write... */
		int len = strlen(mountpoint);
		static char padding[4]={0,0,0,0};
		iwrite(mountpoint, len, dst_fp, "Image-header");
		iwrite(padding, hsize - n - len, dst_fp, "Image-header");
	}

	//
	// Put out the directory entries
	//
	dent = (void *)copybuf;
	dent->dir.path[0] = '\0';
	n = RUP(offsetof(struct image_dir, path) + 1, 4);
	dent->attr.size = swap16(target_endian, n);
	dent->attr.extattr_offset = 0;
	dent->attr.ino = swap32(target_endian, 1);
	dent->attr.mode = swap32(target_endian, S_IFDIR | S_IRWXU | S_IRGRP | S_IXGRP | S_IROTH | S_IXOTH);
	dent->attr.mtime = no_time ? 0 : swap32(target_endian, time(NULL));
	dent->attr.gid = 0;
	dent->attr.uid = 0;
	iwrite(dent, n, dst_fp, "Image-directory");
	for(fip = list; fip ; fip = fip->next) {
		unsigned 	n;
		unsigned	mode;
		int			val;

		mode = fip->attr->mode;
		switch(mode) {
		case S_IFLNK:
			n = offsetof(struct image_symlink, path);
			val = sprintf(&copybuf[n], "%s", fip->targpath) + 1;
			dent->symlink.sym_offset = swap16(target_endian, val);
			n += val;
			val = sprintf(&copybuf[n], "%s", fip->hostpath);
			dent->symlink.sym_size = swap16(target_endian, val);
			n += val + 1;
			break;
		case S_IFREG:
			n = offsetof(struct image_file, path);
			n += sprintf(&copybuf[n], "%s", fip->targpath) + 1;
			dent->file.offset = swap32(target_endian, fip->file_offset - ihdr_offset);
			dent->file.size = swap32(target_endian, fip->size);
			break;
		case S_IFDIR:
			n = offsetof(struct image_dir, path);
			n += sprintf(&copybuf[n], "%s", fip->targpath) + 1;
			break;
		default:
			n = offsetof(struct image_device, path);
			n += sprintf(&copybuf[n], "%s", fip->targpath) + 1;
			dent->device.dev = 0;
			dent->device.rdev = 0;
			break;
		}

		memset(&copybuf[n], 0, 4);
		n = RUP(n, 4);

		dent->attr.size = swap16(target_endian, n);
		dent->attr.extattr_offset = 0;
		dent->attr.ino = swap32(target_endian, fip->inode);
		dent->attr.mtime = no_time > 1 ? 0 : swap32(target_endian, fip->host_mtime);
		val = fip->host_perms;
		if(fip->flags & FILE_FLAGS_RELOCATED) {
			//If the file was relocatable, we've linked it, so make sure
			//that the execute permissions are on.
			val |= S_IXUSR | S_IXGRP | S_IXOTH;
		}
		if(fip->attr->mode == S_IFDIR){
			val &= fip->attr->dperms_mask;
			val |= fip->attr->dperms_set;
		} else {
			val &= fip->attr->perms_mask;
			val |= fip->attr->perms_set;
			if((fip->flags & FILE_FLAGS_EXEC) || (val & (S_IXUSR|S_IXGRP|S_IXOTH))) {
				//If the file is an executable, ignore the host sticky bit
				val &= ~S_ISVTX;
				if(!(fip->flags & FILE_FLAGS_RUNONCE)) val |= S_ISVTX;
			}
		}
		dent->attr.mode = swap32(target_endian, mode | val);
		val = (fip->attr->inherit_gid) ? fip->host_gid : fip->attr->gid;
		dent->attr.gid = swap32(target_endian, val);
		val = (fip->attr->inherit_uid) ? fip->host_uid : fip->attr->uid;
		dent->attr.uid = swap32(target_endian, val);
		iwrite(dent, n, dst_fp, "Image-directory");
	}
	// Put out the 0 size entry
	dent->attr.size = 0;
	iwrite(&dent->attr.size, sizeof(dent->attr.size), dst_fp, "Image-directory");

	//
	// Put out each file
	//
	if(verbose) {
		fprintf(debug_fp, "%8x %6x     ----      --- Image-header\n", image.addr + bsize + ssize, hsize);
		fprintf(debug_fp, "%8x %6x     ----      --- Image-directory\n", image.addr + bsize + ssize + hsize, dsize);
	}

	for(fip = list; fip ; fip = fip->next) {
		switch(fip->attr->mode) {
		case S_IFREG:
			padfile(dst_fp, fip->file_offset, fip->hostpath);
			fd = ropen(fip);

			if(fip->flags & (FILE_FLAGS_EXEC|FILE_FLAGS_SO)) {
				copy_elf(fd, dst_fp, fip);
			} else {
				copy_data(fd, dst_fp, fip->size, fip);
			}
			close(fd);
			break;
		}

		if(verbose && fip->targpath[0] != '\0') {
			switch(fip->attr->mode) {
			case S_IFREG:
				fprintf(debug_fp, "%8x %6x ",
						image.addr + fip->file_offset, fip->size);
				if(fip->flags & (FILE_FLAGS_EXEC|FILE_FLAGS_SO)) {
					fprintf(debug_fp, "%8x ", fip->entry);
					if(fip->ram_offset != 0) {
						fprintf(debug_fp, "%8x", fip->ram_offset);
					} else {
						fprintf(debug_fp, "     ---");
					}
				} else {
					fprintf(debug_fp, "    ----      ---");
				}
				break;
			default:
				fprintf(debug_fp, "    ----    ---     ----      ---" );
				break;
			}
			fprintf(debug_fp, " %s", fip->targpath);
			switch(fip->attr->mode) {
			case S_IFREG:
			case S_IFLNK:
				fprintf(debug_fp, "=%s", fip->hostpath);
				break;
			}
			if(verbose>=2 && (fip->flags & FILE_FLAGS_CRC_VALID)) {
				fprintf(debug_fp," (%u)", fip->host_file_crc);
			}
			fprintf(debug_fp, "\n");
		}
	}

	// Pad file out
	padfile(dst_fp, totalsize-sizeof(itlr), "Image-trailer");

	//
	// Put out the image trailer
	//
	if(verbose) {
		fprintf(debug_fp, "%8x %6x     ----      --- Image-trailer\n",
			image.addr + bsize + ssize + hsize + dsize + fsize, tsize);
	}
	itlr.cksum = swap32(target_endian, -image_cksum);
	iwrite(&itlr, sizeof(itlr), dst_fp, "Image-trailer");

	if(totalsize != image_offset) {
		error_exit("Internal error in size calc (%x!=%x).\n", totalsize, image_offset);
	}

	// Copy the compressed image to the image file and
	// correct the stored size if compressed
	if(compressed) {
		FILE	*fp;
		int		nbytes;

		compress_stop();
		// Open the file with the compressed data
		if((fp = fopen(compress_name, "rb")) == NULL) {
			error_exit("Unable to open compression file: %s\n", strerror(errno));
		}

		// Append the compressed data to the image file
		image_cksum = 0;
		image_offset = cimage_offset;	// For checksum calculation
		while((nbytes = fread(copybuf, 1, sizeof(copybuf), fp)) > 0) {
			iwrite(copybuf, nbytes, dst_fp, "compression-file");
		}

		// Pad file out to multiple of trailer checksum (4 bytes).
		padfile(dst_fp, RUP(image_offset, sizeof(itlr)), "Image-trailer");

		itlr.cksum = swap32(target_endian, -image_cksum);
		iwrite(&itlr, sizeof(itlr), dst_fp, "Image-trailer");
		fclose(fp);

		// Fix the stored_size
		shdr.stored_size = swap32(target_endian, nbytes = ftell(dst_fp) - bsize);
		fseek(dst_fp, shdr_file_offset, SEEK_SET);
		fwrite(&shdr, sizeof(shdr), 1, dst_fp);

		// Fix the checksum
		fseek(dst_fp, stlr_file_offset, SEEK_SET);
		stlr.cksum = swap32(target_endian, - (stlr_cksum + nbytes));
		fwrite(&stlr, sizeof(stlr), 1, dst_fp);
	}

	return(bsize+booter.notloaded_len);
}

__SRCVERSION("mk_image_fsys.c $Rev: 203655 $");
