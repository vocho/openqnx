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





#ifdef  	 __USAGE		/* usemsg.c */
%C - change usage message for a command (QNX)

%C	[-c] [-i id[=value]]* [-f info_file]* loadfile [newmsg]
Options:
 -c        Assume newmsg is a C source file even if it does not end in .c.
 -f file   File to read for "id=value" lines to import into ELF loadfile.
 -i id     Add ID to import into ELF loadfile. (Default: NAME, DESCRIPTION,
           COPYRIGHT, DATE, STATE, HOST, USER).
 -i id=val Import "id" as "value" into ELF loadfile (overrides -f entries).
 -l        Use "ldrel" to manipulate ELF loadfile (overrides '-o'). Default.
 -o        Use "ntomulti-objcopy" to manipulate ELF loadfile (overrides '-l').
 -s string Take usage message from #ifdef <string> section in C source
           file (default: __USAGE). Multiple -s options may be specified,
           in which case it will try to locate usage sections in the 
           order specified until it successfully finds one.
 -t        Prints DATE timezone as UTC value instead of machine dependent
           string. 
 loadfile  The name of an executable program.
 newmsg    A text file or a C source file containing a usage message.
Note:
 A newmsg filename of '-' may be used to indicate standard input.
#endif  	

#include <lib/compat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/timeb.h> 

#if defined(__QNXNTO__)
	#include <utime.h>
	#include <libgen.h>
#elif defined(__QNX__)
	#include <process.h>
	#include <utime.h>
	#include <libgen.h>
#elif defined(__SOLARIS__)
	#include <utime.h>
	#include <libgen.h>
	#include <sys/wait.h>
#elif defined(__LINUX__)
	#include <utime.h>
	#include <libgen.h>
	#include <sys/wait.h>
#elif defined(__CYGWIN__)
	#include <process.h>
	#define P_WAIT _P_WAIT
#elif defined(__MINGW32__)
	/* tmpnam from libcompat returns filenames in length up to MAX_PATH */
	#undef L_tmpnam
	#define L_tmpnam MAX_PATH 
#else
	#error not configured for OS
#endif

#include _NTO_HDR_(sys/elf.h)

#include <util/lmf.h>


/*
#define DEBUG
*/
#define MAX_USAGE 400
#define MAX_BLOCK (16*1024)
#define MAX_LINE  1024

/* number of -s options that will be accepted */
#define MAX_USAGE_STRINGS (16)		

#if defined(__BIGENDIAN__)
#define NATIVE_ENDIAN	ELFDATA2MSB
#elif defined(__LITTLEENDIAN__)
#define NATIVE_ENDIAN	ELFDATA2LSB
#else
#error Endian not defined
#endif

#define SWAP16( val ) ( (((val) >> 8) & 0x00ff) | (((val) << 8) & 0xff00) )

void write_error(int errnum);
#define putc_check(c,filep) \
			{ \
			int x,y; \
			x=(c);  \
			y=putc(x,filep); \
			if (x!=y) write_error(errno); \
			}
#define fwrite_check(buf,elsize,nelem,filep) \
			if ((nelem)!=fwrite(buf,elsize,nelem,filep)) \
				write_error(errno);

#define LMF_FILE 0
#define ELF_FILE 1

struct import {
	struct import			*next;
	unsigned				flags;
#define FLAG_REPLACE			0x0001
#define FLAG_UPDATE_ONLY		0x0002
#define FLAG_BOOL				0x0004
	char					*id;
	char					*value;
}						*list;
	
struct _lmf_header		hdr;
struct _lmf_resource	res;
char					buf[MAX_LINE], buf2[MAX_LINE];
char					fname[256];
char					tmpfname[L_tmpnam];
char					*lines[MAX_USAGE];     /* number that 'use' supports */
int						cflag;
int						loadfile_type = LMF_FILE;

void
bad_lmf(name)
char *name;
	{

	fprintf(stderr, "Bad format for load module %s.\n", name);
	unlink(tmpfname);
	exit(EXIT_FAILURE);
	}

#define IS_LANG(ptr) ((ptr)[0] == '%'  &&  (ptr)[1] == '=')
#define IS_MULT(ptr) ((ptr)[0] == '%'  &&  (ptr)[1] == '%')

// Because not all os's have strupr we have this....
static char *localstrupr(char *p) {
	char		*save = p;
	while(*p) {
		*p = toupper(*p);
		p++;
	}
	return save;
}
#define strupr localstrupr


void
print_data(FILE *fp, unsigned nbytes)
	{
	int i, nlines;

	if(nbytes == 0) {
		printf("No usage available.\n");
		return;
		}

	for(i = nlines = 0 ; hdr.data_nbytes-- ; )
		if((buf[i++] = getc(fp)) == '\n') {
			buf[i - 1] = '\0';
			lines[nlines++] = strdup(buf);
			i = 0;
			printf("%s\n", buf);
			}
	}


void
write_error(errnum)
int errnum;
	{

	fprintf(stderr, "Write error (%s): %s\n",tmpfname,strerror(errnum));
	unlink(tmpfname);
	exit(EXIT_FAILURE);
	}

static void
make_tmpfname(char *f)
{
	char *ptr = tmpnam(NULL);
	if(ptr)
		strcpy(f, ptr);
	else {
		perror("Unable to create temp filename\n");
		exit(1);
	}
}

void
copy_data(fpr, name, offset)
FILE *fpr;
char *name;
long offset;
	{
	FILE			*fpw;
	char			buff[MAX_LINE];
	long			n;
	long			i;
	struct stat		sbuf;
	dev_t			temp_dev;

	make_tmpfname(tmpfname);
	if((fpw = fopen(tmpfname, "wb")) == NULL) {
		fprintf(stderr, "Unable to create temp file(%s).\nSource file %s not changed.\n", tmpfname, name);
		exit(EXIT_FAILURE);
		}

	fseek(fpr, 0L, SEEK_SET);

	/* Copy the definition record */
	fread(&hdr, 1, sizeof(hdr), fpr);
	fwrite_check(&hdr, 1, sizeof(hdr), fpw);

	for(n = 0 ; n < hdr.data_nbytes ; n += i) {
		i = hdr.data_nbytes - n;
		i = (i > sizeof(buff)) ? sizeof(buff) : i;
		fread(buff, 1, i, fpr);
		fwrite_check(buff, 1, i, fpw);
	}
	n += sizeof(hdr);

	/* Insert the new resource record (unless it is zero bytes) */
	memset(&hdr, 0, sizeof(hdr));
	fstat(fileno(stdin), &sbuf);
	hdr.rec_type = _LMF_RESOURCE_REC;
	hdr.data_nbytes = sbuf.st_size + sizeof(res);

	if(sbuf.st_size != 0) {
		long startofrecord, endofrecord;

		startofrecord=ftell(fpw);
		fwrite_check(&hdr, 1, sizeof(hdr), fpw);	/* placeholder */
		memset(&res, 0, sizeof(res));
		fwrite_check(&res, 1, sizeof(res), fpw);

		while((i = fread(buff, 1, sizeof(buff), stdin)) > 0)
			fwrite_check(buff, 1, i, fpw);
			
		endofrecord = ftell(fpw);

		/* now go back and put in our record size - what we really wrote! 
           zzx - I suspect the data_nbytes is not inclusive of hdr, therefore
           our value is too big (since it includes the hdr) and needs to be
           adjusted by sizeof(hdr)  EJ */
		hdr.data_nbytes = endofrecord-startofrecord-sizeof(hdr);

		fseek(fpw,startofrecord,SEEK_SET);
		fwrite_check(&hdr, 1, sizeof(hdr), fpw);	/* real thing this time */
		fseek(fpw,endofrecord,SEEK_SET);
	}


	/* Copy the old data to the point where we hit the old resource record */
	if(offset) {
		for( ; n < offset ; n += i) {
			i = offset - n;
			i = (i > sizeof(buff)) ? sizeof(buff) : i;
			fread(buff, 1, i, fpr);
			fwrite_check(buff, 1, i, fpw);
		}

		/* Skip over the old resource record */
		if(fread(&hdr, 1, sizeof(hdr), fpr) != sizeof(hdr))
			bad_lmf(name);
		fseek(fpr, (long) hdr.data_nbytes, SEEK_CUR);
		}


	/* Copy the rest of the data */
	while((i = fread(buff, 1, sizeof(buff), fpr)) > 0)
		fwrite_check(buff, 1, i, fpw);

	/* read and save away the device of the temp file. We will do
	   an unlink/rename if on the same device */
	fstat(fileno(fpw), &sbuf);
	temp_dev = sbuf.st_dev;

	fstat(fileno(fpr), &sbuf);
	fchmod(fileno(fpw), sbuf.st_mode);
	fchown(fileno(fpw), sbuf.st_uid, sbuf.st_gid);

	fflush(fpw);
#if defined(__QNX__) && !defined(__QNXNTO__)
	{
		struct utimbuf	ubuf;
		ubuf.actime = sbuf.st_atime;
		ubuf.modtime = sbuf.st_mtime;
		__futime(fileno(fpw), &ubuf);
	}
#endif

	fclose(fpr);
	fclose(fpw);

	if(sbuf.st_nlink>1 || (sbuf.st_dev!=temp_dev)) {
		/* We copy the data back to preserve the link */
justcopy:
		if((fpr = fopen(tmpfname, "rb"))) {
			if((fpw = fopen(name, "wb"))) {
				while((i = fread(buff, 1, sizeof(buff), fpr)) > 0)
					fwrite_check(buff, 1, i, fpw);
				unlink(tmpfname);
			} else {
				perror(name);
				exit(EXIT_FAILURE);
			}
		} else {
			perror(tmpfname);
			exit(EXIT_FAILURE);
		}
	} else {
		/* We do a quick rename */
		if(rename(tmpfname, name) == -1) {
#ifdef DEBUG
			perror("rename");
			fprintf(stderr, "errno = %d\n", errno);
			fprintf(stderr, "Unable to rename %s. goto justcopy\n", tmpfname);
			goto justcopy;
#else
			goto justcopy;
#endif
		}
	}

}

#if defined(linux) || defined (__SOLARIS__)
#define P_WAIT      0

// hack spawnvp cover - FIXME should check mode
int spawnvp(int mode, const char *path, char * const args[])
{
	int status;
	pid_t pid;
	pid_t wpid;

	mode = mode; //hush warnings
	pid=fork();
	if(pid == -1)
	{
		perror("usemsg: fork failed in spawvp");
		return(-1);
	}
	if(pid == 0) //child
	{
		if(execvp(path, args) == -1)
		{
			fprintf(stderr,"usemsg: execvp of %s failed: errno %d (%s)\n", args[0], errno, strerror(errno));
			exit(0);
		}
		exit(0);
	}
	else //parent
	{
		do 
		{
			wpid = waitpid(pid, &status, 0);
		} while (WIFEXITED(status) == 0);
	}
	return(status);	
}
#endif

static int add_import(const char *id, const char *value, unsigned flags) {
	struct import				*p, **pp;

	for(pp = &list; (p = *pp); pp = &p->next) {
		if(!strcmp(p->id, id)) {
			break;
		}
	}
	if(!p) {
		if(flags & FLAG_UPDATE_ONLY) {
			return 0;
		}
		if(!(p = malloc(sizeof *p))) {
			errno = ENOMEM;
			return -1;
		}
		memset(p, 0x00, sizeof *p);
		if(!(p->id = strdup(id))) {
			free(p);
			errno = ENOMEM;
			return -1;
		}
		p->flags = flags & FLAG_BOOL;
	}

	if(value) {
		if((flags & FLAG_REPLACE) || (p->flags & FLAG_REPLACE) == 0 || !p->value) {
			char						*str = "";

			if((((flags ^ p->flags) & FLAG_BOOL) && p->value) || !(str = strdup(value))) {
				if(!*pp) {
					free(p->id);
					free(p);
				}
				errno = str ? EINVAL : ENOMEM;
				return -1;
			}

			if(p->value) {
				free(p->value);
			}

			p->value = str;
		}
		p->flags |= flags;		// to get FLAG_REPLACE
	}

	*pp = p;

	return 1;
}
	
		
static int read_import_file(char *fname) {
	FILE				*fp;
	int					line;

	if(!(fp = fopen(fname, "r"))) {
		return -1;
	}
	line = 0;
	buf[sizeof buf - 1] = '\0';
	while(fgets(buf, sizeof buf, fp)) {
		char				*p;
		char				*id;
		char				*value;
		unsigned			flags;

		// Check if last char is line terminator
		p = buf + (sizeof buf - 1);
		if(*p && *p != '\n' && *p != '\r') {
			*p = '\0';
			fprintf(stderr, "Line too long, max=%d, line %d, file %s.\n", sizeof buf - 1, line, fname);
			continue;
		}

		line++;

		// Skip white space
		for(p = buf; *p && isspace(*p); p++);
		if(p[0] == '\0' || p[0] == '#' || (p[0] == '/' && p[1] == '/') || p[0] == ';') {
			continue;
		}

		// Check for identifier
		if(!isalpha(*p) && *p != '_') {
			fprintf(stderr, "Invalid identifier start, line %d, file %s.\n", line, fname);
			continue;
		}

		// Find end of identifier
		for(id = p++; *p && (isalnum(*p) || *p == '_'); p++);
		if(*p && isspace(*p)) {
			*p++ = '\0';
		}
		while(*p && isspace(*p)) {
			p++;
		}		

		// Check for '=' assignment operator
		value = p;
		if(*p) {
			char				*last;

			if(*p != '=') {
				fprintf(stderr, "Invalid identifier before '=', line %d, file %s.\n", line, fname);
				continue;
			}
			*p++ = '\0';

			// Skip the white space
			while(*p && isspace(*p)) {
				p++;
			}		

			if(!*p) {
				fprintf(stderr, "Missing content after '=', line %d, file %s.\n", line, fname);
				continue;
			}
			value = p;

			// Trim off the trailing whitespace
			for(last = ++p; *p; p++) {
				if(!isspace(*p)) {
					last = p + 1;
				}
			}
			*last = '\0';
		}

		// Add the entry...
		strupr(id);
		flags = FLAG_UPDATE_ONLY;
		if(value[0] == '\0') {
			flags |= FLAG_BOOL;
			value = id;
		}
		if(add_import(id, value, flags) == -1) {
			if(errno == ENOMEM) {
				fprintf(stderr, "Out of memory.\n");
				exit(EXIT_FAILURE);
			}
			fprintf(stderr, "Mismatch on line %d, file %s.\n", line, fname);
			continue;
			
		}
	}
	fclose(fp);

	return 0;
}


/* func() to check for all supported extensions */
int ext_check (char *cp)
{
	char *ext[] = { ".c", ".cc", ".cpp", ".C", ".cxx", NULL };
	int count = 0;
	while (ext[count]){
		if (strcmp (cp, ext[count]) == 0)
			return 0;
		count++;
	}
	return 1;
}


int main(int argc, char *argv[])
	{
	FILE				*fp, *fpw;
	char				*cp;
	char 				*objcopy = NULL;
	int				modify = 0, import = 0, opt, nbytes = 0, use_objcopy = 0;
	long				offset = 0;
	int				num_usage_strings=0;
	char				*usage_string[MAX_USAGE_STRINGS];
	char			use_utc_timezone=0;
    struct timeb timebuf; 
	unsigned			flags;
	struct files {
		struct files			*next;
		char				*fname;
	}					*files = 0, *p, **last = &files;


	add_import("NAME", "", 0);
	add_import("DESCRIPTION", "", 0);
	add_import("COPYRIGHT", "", 0);
	add_import("DATE", "", 0);
	add_import("STATE", "", 0);
	add_import("HOST", "", 0);
	add_import("USER", "", 0);

	while ((opt = getopt(argc, argv, "cf:i:lots:")) != -1) {
		switch(opt) {
		case 'c':
			cflag = 1;
			break;
		case 'f':
			import = 1;
			if(!(p = malloc(sizeof *p))) {
				fprintf(stderr, "Out of memory\n");
				exit(EXIT_FAILURE);
			}
			p->next = 0;
			p->fname = optarg;
			*last = p;
			last = &p->next;
			break;
		case 'i':
			import = 1;
			flags = FLAG_REPLACE;
			if(*optarg == '+' || *optarg == '-') {
				flags |= FLAG_BOOL;
				if(strchr(optarg, '=')) {
					fprintf(stderr, "Invalid argument \"%s\"\n", optarg);
					exit(EXIT_FAILURE);
				}
				cp = *optarg == '+' ? optarg : "";
				optarg++;
			} else if((cp = strchr(optarg, '='))) {
				*cp++ = '\0';
			}
			strupr(optarg);
			if(add_import(optarg, cp, flags) == -1) {
				if(errno == ENOMEM) {
					fprintf(stderr, "Out of memory\n");
				} else {
					fprintf(stderr, "Invalid argument \"%s\"\n", optarg);
				}
				exit(EXIT_FAILURE);
			}
			break;
		case 'l':
			use_objcopy = 0;  // Use ldrel to add sections
			break;
		case 'o':
			use_objcopy = 1;  // Use objcopy to add sections
			break;
		case 's':
			usage_string[num_usage_strings++]=optarg;
			break;
		case 't':
			use_utc_timezone = 1;
			break;
		}
	}

	if(optind == argc) {
		fprintf(stderr, "You must specify a loadfile.\n");
		exit(EXIT_FAILURE);
	}

	if((argc - optind) > 2) {
		fprintf(stderr, "Too many file arguments.\n");
		exit(EXIT_FAILURE);
	}

	if (num_usage_strings==0) {
		usage_string[num_usage_strings++]="__USAGE";
	}

	cp = 0;
	if((argc - optind) == 2) {
        cp=argv[optind+1];
		if (strcmp(cp, "-") != 0) {
			if(freopen(cp, "r", stdin) == NULL) {
				perror(cp);
				exit(EXIT_FAILURE);
			}
			if((cp = strrchr(cp, '.'))  &&  ext_check(cp) == 0)
				cflag = 1;
			else cp=argv[optind+1];
		} 
		modify = 1;
	}

/*
	searchenv(argv[optind], "PATH", fname);
*/
	strcpy(fname, argv[optind]);
	if(fname[0] == '\0') {
		fprintf(stderr, "Unable to locate %s.\n", argv[optind]);
		exit(EXIT_FAILURE);
	}

	if((fp = fopen(fname, "rb")) == NULL) {
		perror(fname);
		exit(EXIT_FAILURE);
	}

	if(import) {
		struct stat		statbuf;

		add_import("NAME", basename(strcpy(buf, fname)), 0);
#ifdef MAXHOSTNAMELEN
		if(gethostname(buf, sizeof buf) != -1) {
			add_import("HOST", buf, 0);
		}
#endif

		if (! use_utc_timezone ) {		
			if(fstat(fileno(fp), &statbuf) != -1 && strftime(buf, sizeof buf, "%Y-%m-%d%Z-%H:%M:%S", localtime(&statbuf.st_mtime))) {
				add_import("DATE", buf, 0);
			}
		} else
		{  // create a UTC timezone string instead of local machine depended string
			fstat(fileno(fp), &statbuf);
			strftime(buf, sizeof buf, "%Y-%m-%d UTC", localtime(&statbuf.st_mtime) );
			    ftime( &timebuf );
			    timebuf.timezone = timebuf.timezone * -1;
			    if ( timebuf.timezone/60 > 0)
					strcat( buf, "+" );
		 	 	if ( timebuf.timezone/60 != 0) {
			 	 	sprintf( buf2, "%d", timebuf.timezone/60 );
					strcat( buf, buf2 );
				}
			 	strftime(buf2, sizeof buf2, " %H:%M:%S", localtime(&statbuf.st_mtime) );
		 		strcat( buf, buf2 );
				add_import("DATE", buf, 0);
		}

		for(p = files; p; p = p->next) {	
			if(read_import_file(p->fname) == -1) {
				fprintf(stderr, "Unable to process %s\n", p->fname);
				exit(EXIT_FAILURE);
			}
		}
	}

	/* determine type of load file: check for valid LMF or ELF file */
	{
		Elf32_Ehdr ehdr;
		int cross_endian; 	

		if (1==fread(&ehdr,sizeof(ehdr),1,fp)) {
			/* check ELF header fields */
			if (ehdr.e_ident[0]==0x7f &&			
				ehdr.e_ident[1]=='E' &&
				ehdr.e_ident[2]=='L' &&
				ehdr.e_ident[3]=='F') 
			{
				/* it's a ELF file */
				if (ehdr.e_ident[4]!=ELFCLASS32) {
					fprintf(stderr,"ELF file is not 32-bit. Can't add usage.\n");
					exit(EXIT_FAILURE);
				}
				loadfile_type = ELF_FILE;

				switch(ehdr.e_ident[EI_DATA]) {
				case ELFDATA2MSB:
				case ELFDATA2LSB:
					cross_endian = (ehdr.e_ident[EI_DATA] != NATIVE_ENDIAN);
					break;
				default:
					fprintf(stderr, "ELF file has invalid endian type\n");
					exit(EXIT_FAILURE);
				}
				if (cross_endian)
					ehdr.e_machine = SWAP16(ehdr.e_machine);

				switch(ehdr.e_machine) {
				case EM_386: 
					objcopy="ntox86-objcopy";
					break;
				case EM_ARM: 
					objcopy="ntoarm-objcopy"; 
					break;
				case EM_MIPS:
					objcopy="ntomips-objcopy";
					break;
				case EM_PPC: 
					objcopy="ntoppc-objcopy"; 
					break;
				case EM_SH:  
					objcopy="ntosh-objcopy"; 
					break;
				default: 
					fprintf(stderr, "ELF file has unknown architecture.\n");
					exit(EXIT_FAILURE);
				}
			}
		}

		/* restore file position to beginning of file */
		fseek(fp,0L,SEEK_SET);
	}

	if (modify) {
		if( cflag || !strcmp(cp,"-") ) {
			char *bptr=buf;
			const char *delim=" \b\t\r\n";
			int nlines=0, gotit=0;
			int usage_string_idx=0;

			cp = 0;
			make_tmpfname(tmpfname);
			if((fpw = fopen(tmpfname, "wb")) == NULL) {
				fprintf(stderr, "Unable to create temp file(%s).\nSource file %s not changed.\n", tmpfname, fname);
				exit(EXIT_FAILURE);
			}

			if(cflag) {
				for (usage_string_idx=0;usage_string_idx<num_usage_strings;usage_string_idx++) 
				{
					if (usage_string_idx!=0) {
						if (fseek(stdin,0L,SEEK_SET)) {
							fprintf(stderr,"Cannot seek to beginning of file (stdin); unable to check\n");
							fprintf(stderr,"for alternate usage section(s) specified via -s\n");
							exit(EXIT_FAILURE);
							break;
						}
					}
	
					while(fgets(buf, MAX_LINE, stdin)) {
						bptr = strtok(buf,delim);
						if (bptr!=NULL && strcmp(bptr,"#ifdef")==0) {
							bptr=strtok(NULL,delim);
							if (bptr!=NULL && strcmp(bptr,usage_string[usage_string_idx])==0) {
								gotit++;
								break;
							}
						}
					}
		
					if (gotit) break;
				}
	
				if (usage_string_idx==num_usage_strings) {
					//fprintf(stderr,"usage_string_idx=%d, num_usage_strings=%d\n",
					//		usage_string_idx, num_usage_strings);
					exit(EXIT_FAILURE);
				}
	
				while(fgets(buf, MAX_LINE, stdin)) {
					buf[strcspn(buf,"\r\n")]=0;
					bptr = buf+strspn(buf,delim);
					if (strncmp(bptr,"#endif",6)) {
						fprintf(fpw,"%s\n",buf);
						nlines++;
					}
					else break;
				}
			}
			else {
				while(fgets(buf,MAX_LINE,stdin)){
					nlines++;
					fputs(buf,fpw);
				}
			}

			if (nlines>MAX_USAGE) {
				fclose(fpw);
				unlink(tmpfname);
				fprintf(stderr, "Usage exceeds MAX_USAGE %d lines. Source file %s not changed.\n", (int)MAX_USAGE, fname);
				exit(EXIT_FAILURE);
			}

			fclose(fpw);
			if (loadfile_type==LMF_FILE) {
				freopen(tmpfname, "r", stdin);
                unlink(tmpfname);
			} else {
				cp = tmpfname;
			}
		}
	}

	if (loadfile_type==ELF_FILE) {
		char		import_fname[L_tmpnam] = "";

		if(import) {
			struct import	*p;
			FILE			*fp;

			
			make_tmpfname(import_fname);
			if(!(fp = fopen(import_fname, "wb"))) {
				fprintf(stderr, "Unable to create temp file(%s).\nSource file %s not changed.\n", import_fname, fname);
				unlink(tmpfname);
				exit(EXIT_FAILURE);
			}

			for(p = list; p; p = p->next) {
				if(p->value) {
					if(p->flags & FLAG_BOOL) {
						fprintf(fp, "%s\n", p->id);
					} else {
						if(p->value[0]) {
							fprintf(fp, "%s=%s\n", p->id, p->value);
						}
					}
				}
			}
			fclose(fp);
		}
		fclose(fp);
		if (modify || import) {
			/* cp points to the filename containing usage data */
			/* spawn: ldrel -r -s* <loadfile> usemsg <cp> */
			/* or spawn objcopy --remove-section <section_name> */
			/*                  --add-section <section_name>=<cp> <loadfile> */
			char commandbuf1[LINE_MAX+1];
			char commandbuf2[LINE_MAX+1];
			char *args[11], **argp = &args[0];
			int ret, reterr = EOK;

			if(use_objcopy && objcopy)
			    *argp++ = objcopy;
			else	
			{
			    *argp++ = "ldrel";
			    *argp++ = "-r";
			    *argp++ = "-s*";
			}

			if(modify && cp) {
				if(use_objcopy) {
				    *argp++ = "--remove-section";
				    *argp++ = "QNX_usage";
				    *argp++ = "--add-section";
				    sprintf(*argp++ = commandbuf2,"QNX_usage=%s", cp);
				} else {
#ifdef __CYGWIN32__
				    char win32_path[PATH_MAX];
				    cygwin_conv_to_win32_path(cp, win32_path);
				    sprintf(*argp++ = commandbuf2,"-sQNX_usage=%s", win32_path);
#else
				    sprintf(*argp++ = commandbuf2,"-sQNX_usage=%s", cp);
#endif
				}
			}
			if(import && import_fname) {
				if(use_objcopy) {
				    *argp++ = "--remove-section";
				    *argp++ = "QNX_info";
				    *argp++ = "--add-section";
				    sprintf(*argp++ = commandbuf1,"QNX_info=%s", import_fname);
				} else {
#ifdef __CYGWIN32__
				    char win32_path[PATH_MAX];
				    cygwin_conv_to_win32_path(import_fname, win32_path);
				    sprintf(*argp++ = commandbuf1,"-sQNX_info=%s", win32_path);
#else
				    sprintf(*argp++ = commandbuf1,"-sQNX_info=%s", import_fname);
#endif
				}
			}
#ifdef __CYGWIN32__
			if (use_objcopy) {
				*argp++ = fname;
			} else {
				char win32_path[PATH_MAX];
				cygwin_conv_to_win32_path(fname, win32_path);
				*argp++ = win32_path;
			}
#else
			*argp++ = fname;
#endif
			*argp++ = 0;
			ret = spawnvp(P_WAIT, args[0], args);
			if(ret == -1)
				reterr = errno;
			else
				reterr = ret;

			// Clean up tmp files
			if(*import_fname) {
				unlink(import_fname);
				*import_fname = 0;
			}
			unlink(tmpfname);

			// Exit if an error occured
			if(ret == -1) {
				errno = reterr;
				if (use_objcopy) {
					perror("spawn ntomulti-objcopy"); 
				} else {
					perror("spawn ldrel");
				}
			}
			exit(ret);
		} else {
			/* extract contents of usage from load file */
			/* spawn: elfman <loadfile> use - */
			/* spawn: use -a filename will do just as well */

			char *args[4], **argp = &args[0];
			int ret;
			*argp++ = strdup("use");
			*argp++ = strdup("-a");
			*argp++ = strdup(fname);
			*argp++ = 0;

			if((ret = spawnvp(P_WAIT, args[0], args))==-1)
			{
				perror("spawn use");
				return(EXIT_FAILURE);
			}
			exit(ret);
		}
	}

	if(import) {
		fprintf(stderr, "Importing values only works with ELF files.\n");
		unlink(tmpfname);
		exit(EXIT_FAILURE);
	}

	if(fread(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr))
		bad_lmf(fname);

	if(hdr.rec_type != _LMF_DEFINITION_REC)
		bad_lmf(fname);

	fseek(fp, (long) hdr.data_nbytes, SEEK_CUR);

	while(fread(&hdr, 1, sizeof(hdr), fp) == sizeof(hdr)) {
		switch(hdr.rec_type) {
		case _LMF_RESOURCE_REC:
			if(fread(&res, 1, sizeof(res), fp) != sizeof(res))
				bad_lmf(fname);
			hdr.data_nbytes -= sizeof(res);
			if(res.resource_type != 0) {
				fseek(fp, (long) hdr.data_nbytes, SEEK_CUR);
				continue;
				}

			if(modify)
				offset = ftell(fp) - (sizeof(res) + sizeof(hdr));
			else
				nbytes = hdr.data_nbytes;
			break;

		case _LMF_COMMENT_REC:
		case _LMF_DATA_REC:
		case _LMF_FIXUP_SEG_REC:
		case _LMF_FIXUP_80X87_REC:
		case _LMF_ENDDATA_REC:
		case _LMF_FIXUP_LINEAR_REC:
			fseek(fp, (long) hdr.data_nbytes, SEEK_CUR);
			continue;

		case _LMF_EOF_REC:
			break;

		case _LMF_DEFINITION_REC:
		default:
			bad_lmf(fname);
			}

		break;
		}

	if(modify)
		copy_data(fp, fname, offset);
	else
		print_data(fp, nbytes);

	unlink(tmpfname);
	return EXIT_SUCCESS;
}
