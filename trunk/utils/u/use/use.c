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





/* if you want this to compile with full MORE prompts, #define FULL_MORE here */
#ifdef __USAGE		/* usage.c */
%C - print a usage message (QNX)

%C	[-a] [-d <directory>] [-e] [-f <filelist>] [-i] files
Options:
 -a    Extracts all usage information from the load module in its source
       form, suitable for piping into usemsg.
 -d    Recursively display info for all files under <directory>.
 -e    Only consider ELF files.
 -f    Read list of files, one per line, from file <filelist>, displaying
       usage info for each.
 -i    Displays information from load module (ELF only).
 -s    Displays .ident strings (ELF only).
 -r    Displays information about running process and libraries (native only).
Where:
 files A list of executable load modules or shell scripts that contain
       usage messages (see printed documentation for use and usemsg
       for details).
#endif

#include <lib/compat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(__QNXNTO__)
	#include <process.h>
	#include <libgen.h>
	#include <sys/ioctl.h>
	#include <termios.h>
	#include <sys/wait.h>
	#include <sys/neutrino.h>
	#include <sys/procfs.h>
	#include <sys/mman.h>
	#include <dirent.h>
#elif defined(__QNX__)
	#include <process.h>
	#include <sys/ioctl.h>
	#include <sys/fd.h>
	#include <sys/console.h>
	#include <sys/dev.h>
	#include <termios.h>
	#include <sys/wait.h>
#elif defined(__SOLARIS__)
	#include <libgen.h>
	#include <termios.h>
	#include <sys/wait.h>
#elif defined(__LINUX__)
	#include <libgen.h>
	#include <termios.h>
	#include <sys/wait.h>
#elif defined(__MINGW32__)
#elif defined(__CYGWIN__)
#include _NTO_HDR_(process.h)
#include <termios.h>
#elif defined(__NT__)
//	#define SIGPIPE 13
#else
	#error Not configured for system.
#endif

#include <ftw.h>

#include _NTO_HDR_(sys/elf.h)

/*
 This doesn't have to come from the Neutrino headers,
 it comes from the installed lib/util headers.
*/
#include <util/qnx4dev.h>
#include <util/lmf.h>

/* #define  ALLOW_SIGINT */ /* we disallow this because we don't want
                               more to die leaving the tty attributes
                               screwed up */

#define BUF_SIZE 78
#define MAX_USAGE 400
#define DEFAULT_LINES (24)

int raw(int fd);
int unraw(int fd);
int elf_getsecinfo(FILE *fp,char *secname,long int *size, long int *offset);
void bad_lmf(char *fname,FILE *fp, int numlines);
void print_data( FILE *fp, char *full_fname, long nbytes );
void setup_paging(void);
int use_guy(const char *argv);

int	allflag = 0;   /* all flag -- extract source for the usage message */
int infoflag = 0;	/* info flag -- extract info instead of usemsg */
#ifdef __QNXNTO__
int runflag = 0;	/* run flag -- display usage of running processes. */
#endif
struct _lmf_header		hdr;
struct _lmf_resource	res;
char					buf[BUF_SIZE+1];
char					fname[256];
char					*lines[MAX_USAGE+1]; /* _should_ malloc as needed */
int						elfonly;
int						numlines = DEFAULT_LINES;
int						nl = DEFAULT_LINES; /* used at top to hold info obtained
                                    when checking console size & setting
                                    LINES envar for MORE to use since more
                                    does not check the console size. We
                                    use it later on to set numlines to. */
pid_t					more_pid=-1;
int						exit_val = EXIT_SUCCESS; /* one failure causes
											return value to be EXIT_FAILURE */												
extern pid_t            _my_pid;

static struct stat statbuf;

void exec_searchenv( const char *name, const char *env_var, char *buffer );

#ifdef ALLOW_SIGINT
break_out(int sig_number)
{
	printf("break??\n");
	if (more_pid!=-1) kill(more_pid,SIGQUIT); /* kill child */
	exit(EXIT_FAILURE);
}
#endif

void leave(int exit_code)
{
	int stat;

	if (more_pid!=-1) {
		close(1);	/* so that more sees an end to the tunnel */
#if defined(__NT__) || defined(__MINGW32__) || defined (__CYGWIN__)
		stat = 0;
		exit(exit_code?exit_code:stat);
#else
		/* We might have already waited for it */
		waitpid(more_pid,&stat,0);
		exit(exit_code?exit_code:WEXITSTATUS(stat));
#endif
	} else {
		exit(exit_code);
	}
}

int
is_elf(FILE *fp)
{
	Elf32_Ehdr ehdr;

	if (-1==fseek(fp,0L, SEEK_SET)) return 0;

	if (1==fread(&ehdr,sizeof(ehdr),1,fp) &&
		ehdr.e_ident[0]==0x7f &&
		ehdr.e_ident[1]=='E' &&
		ehdr.e_ident[2]=='L' &&
		ehdr.e_ident[3]=='F')
			return 1;
	return 0;
}

int
map_fn(const char *filename, const struct stat *buf, int flags)
{
	if(flags == FTW_F){ /* S_ISDIR(buf->st_mode)) */
		if(use_guy(filename) > 0) {
			printf("\n"); 
		} else exit_val = EXIT_FAILURE;
	}
	return 0;
}

#ifdef __QNXNTO__

static struct name_list{
	struct name_list *next;
	char name[_POSIX_PATH_MAX + 1];
} name_list;

static char *
my_basename(char *path)
{
	char *last = path;
	char *c = path;

	while(*c){
		if((*c == '/' || *c == '\\') && *(c + 1))
			last = c + 1;
		c++;
	}
	return last;
}

static int
add_name(char *name){
	struct name_list *nl = name_list.next;
	struct name_list *new_name;
	
	while(nl){
		if(strcmp(name, nl->name) == 0)
			return 1;
		nl = nl->next;
	}
	if((new_name = malloc(sizeof(*new_name))) == NULL)
		return 0;
	strcpy(new_name->name, name);
	new_name->next = name_list.next;
	name_list.next = new_name;
	return 1;
}

int
add_solibs(procfs_info *i, int fd)
{
	int				num, j, retval = 0;
	procfs_mapinfo	*m, *mapinfo = 0;
	struct {
		procfs_debuginfo	info;
		char				buff[_POSIX_PATH_MAX];
	}				map;
	int				num_maps = 0;

	// get mapping info
	for (num = num_maps + 1; num > num_maps;) {
		if (num > num_maps) {
			if (!(m = realloc(mapinfo, (num + 10) * sizeof *mapinfo))) {
				free(mapinfo);
				return 1;
			}
			mapinfo = m;
			num_maps = num + 10;
		}
		if (devctl(fd, DCMD_PROC_PAGEDATA, mapinfo, sizeof *mapinfo * num_maps, &num) != EOK) {
			retval = 1;
			free(mapinfo);
			return 0;
		}
	}
	
	for (m = mapinfo, j = num; j--; m++) {
		if (m->ino == 0) {
			continue;
		}
		map.info.vaddr = m->vaddr;
		if (devctl(fd, DCMD_PROC_MAPDEBUG, &map, sizeof map, 0) != EOK) {
			continue;
		}
		
		/* Skip if not executable. */
		if (map.info.vaddr == m->vaddr && !(m->flags & MAP_ELF))
			continue;
		add_name(my_basename(map.info.path));
	}

	return 1;
}

int
pretend_to_be_pidin()
{
	DIR *dp = opendir("/proc");
	struct dirent *dirent;

	if(!dp)
		return 0;

	while((dirent = readdir(dp))){
		char aspath[20];
		int pid = atoi(dirent->d_name);
		int fd;
		procfs_info info;
		struct {
			procfs_debuginfo info;
			char buf[_POSIX_PATH_MAX + 1];
		} name;

		if(!pid)
			continue;

		sprintf(aspath, "/proc/%d/as", pid);
		if((fd = open(aspath, O_RDONLY)) == -1){
			fprintf(stderr, "Warning: could not open %s: %s\n", aspath, strerror(errno));
			continue;
		}

		if (devctl(fd, DCMD_PROC_INFO, &info, sizeof info, 0) != EOK) {
			fprintf(stderr, "Warning: could not fill info %s: %s\n", aspath, strerror(errno));
			close(fd);
			continue;
		}
		
		if (devctl(fd, DCMD_PROC_MAPDEBUG_BASE, &name, sizeof name, 0) != EOK) {
			name.info.vaddr = 0;
			if (info.pid == SYSMGR_PID) {
			        strcpy(name.info.path, "procnto");
			} else {
				if(!(info.flags & _NTO_PF_LOADING)) //Don't warn if process not fully loaded.
					fprintf(stderr, "Warning: no name info for %s: %s\n", aspath, strerror(errno));
				close(fd);
				continue;
			}
		}
		add_name(my_basename(name.info.path));
		add_solibs(&info, fd);
	}

	{
		struct name_list *nl = name_list.next;
		while(nl){
			if ( (use_guy(nl->name)) < 1) 
				exit_val = EXIT_FAILURE;
			nl = nl->next;
		}
	}

	return 1;
}
#endif /* __QNXNTO__ */

void morekiller(){
#ifndef WIN32
	if (more_pid!=-1) kill(more_pid,SIGQUIT); /* kill child */
#else
	/* Not applicable to Win32 */
#endif
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	int	c;
	char 	*directory = NULL, *filelist = NULL;

	if(argc < 2) {
		fprintf(stderr,"Program name missing; use\nuse [-ai] progname\n");
		exit(EXIT_FAILURE);
	}

#ifdef __QNXNTO__
	{
		/* Search LD_LIBRARY_PATH when self hosted. */
		char *path = getenv("PATH");
		char *libpath = getenv("LD_LIBRARY_PATH");
		char *newpath;

		if(!libpath){
			int len = confstr(_CS_LIBPATH, NULL, 0);
			newpath = malloc(strlen(path) + len + 7);
			sprintf(newpath, "PATH=%s:", path);
			confstr(_CS_LIBPATH, newpath + strlen(newpath), len);
		}
		else{
			newpath = malloc(strlen(path) + strlen(libpath) + 7);
			sprintf(newpath, "PATH=%s:%s", path, libpath);
		}
		putenv(newpath);
	}

	while ((c=getopt(argc,argv,"ad:ef:irs")) != -1) {
#else
	while ((c=getopt(argc,argv,"ad:ef:is")) != -1) {
#endif
		switch(c){
		case 'a':
			allflag = 1;
			break;
		case 'd':
			directory = optarg;
			break;
		case 'e':
			elfonly = 1;
			break;
		case 'f':
			filelist = optarg;
			break;
		case 'i':
			if(infoflag != 0) {
				fprintf(stderr, "Can not specify both -i and -s at the same time.\n");
				return EXIT_FAILURE;
			}
			infoflag = 1;
			break;
		case 's':
			if(infoflag != 0) {
				fprintf(stderr, "Can not specify both -i and -s at the same time.\n");
				return EXIT_FAILURE;
			}
			infoflag = 2;
			break;
#ifdef __QNXNTO__
		case 'r':
			runflag = 1;
			break;
#endif
		default:
			fprintf(stderr, "Invalid argument %c\n", c);
			return EXIT_FAILURE;
		}
	}

#ifdef __QNXNTO__
	if(argc - optind > 1 || directory || filelist || runflag)
#else
	if(argc - optind > 1 || directory || filelist)
#endif
			numlines=0;
	else
			setup_paging(); /* Page if there is only one file.  */

	while(optind < argc){
			if(use_guy(argv[optind]) > 0) {
				printf("\n");
			} else exit_val = EXIT_FAILURE;
			optind++;
	}

#ifdef __QNXNTO__
	if(runflag){
		if(!pretend_to_be_pidin())
			fprintf(stderr, "Unable to list processes: %s", strerror(errno));
	}
#endif

	if(directory)
		ftw(directory, map_fn, 20);

#define TMPBUF 1024
	if(filelist){
		char tmp[TMPBUF];
		FILE *fp = fopen(filelist, "r");
		
		if(!fp){
			fprintf(stderr, "Unable to open %s for reading: %s",
					filelist, strerror(errno));
			return 1;
		}
		while(fgets(tmp, TMPBUF - 1, fp)){
			tmp[strlen(tmp) - 1] = '\0';
			if(use_guy(tmp) > 0) {
				printf("\n");
			} else exit_val = EXIT_FAILURE;
		}
	}
#undef TMPBUF

	return exit_val;
}

void
setup_paging()
{
	char					*lp;

	if (!isatty(1)){
			numlines = 0;	/* no more prompts when stdout not a tty */
			return;
	}

	if ((lp = getenv("LINES"))!= NULL) {
		nl = atoi(lp);
		if ((nl<3)||(nl>100)) nl=0;	/* turn off more prompts */
	} else {
#if defined(__QNX__) && !defined(__QNXNTO__)
		dev_size(1, -1, -1, &nl, (int *) 0);
#else
#if defined(__QNXNTO__) || defined(__SOLARIS__)
		struct winsize wsz;
		nl = ioctl(1, TIOCGWINSZ, &wsz) == -1 ? 0 : wsz.ws_row;
#else
		nl = 0;
#endif
#endif
		if (nl==0) 
			nl=DEFAULT_LINES;
	}

	/* pipe our output through real 'more' if possible */
#ifdef FULL_MORE
	if (numlines && (getenv("TERM")!=NULL) ) {
		int pipfd[2];

		if (pipe(pipfd)!=-1) {
			char *args[]={"more",NULL};
			char iov[10]={-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

			iov[0]=pipfd[0];

			/* must prevent MORE from holding a write end of the pipe open! */
			if (fcntl(pipfd[1],F_SETFD,fcntl(pipfd[1],F_GETFD)|FD_CLOEXEC)==-1)
				perror("fcntl");

			/* close the read end too since be will be handing it to more as
               its stdin, don't need a 2nd fd for the read end floating about! */
			if (fcntl(pipfd[0],F_SETFD,fcntl(pipfd[0],F_GETFD)|FD_CLOEXEC)==-1)
				perror("fcntl");

#ifdef ALLOW_SIGINT
			signal(SIGINT,break_out);
#else
			signal(SIGINT,SIG_IGN);
#endif

			if ((more_pid=qnx_spawn(0,NULL,0,-1,-1,0,"/bin/more",args,arge,iov,-1))!=-1) {
				fflush(stdout);
				if (dup2(pipfd[1],1)!=-1) numlines = 0;
				if (close(0)==-1) perror("close0");	/* we don't need stdin */
				if (close(pipfd[0])==-1) perror("close1");	/* we don't need the read end */
				if (close(pipfd[1])==-1) perror("close2");	/* we've dup2ed the write end to stdout */
			} else {
				signal(SIGINT,SIG_DFL);
			}
		}
	}
#endif

	if (numlines) numlines=nl;	/* calculated up top */
}

int
use_guy(const char *argv)
{
	FILE					*fp;
	long					nbytes = 0L;

	fname[0]=0;
	if(strpbrk(argv, "\\/")) {
	    /* check to see if file exists */
		if (stat(argv,&statbuf)!=-1) {
			if (!S_ISDIR(statbuf.st_mode)) {
				if( access( argv, R_OK ) == 0 ) {
					strcpy(fname, argv);
				}
			}
		}
	} else  {
		if (strlen(argv) > sizeof(fname)) {
			fprintf(stderr, "%s: %s.\n", argv, strerror(ENAMETOOLONG));
			return 0;
		}
		exec_searchenv(argv, "PATH", fname);
	}
	if(*fname == 0) {
		fprintf(stderr, "'%s' is not an executable file in PATH: '%s'.\n", argv, getenv("PATH"));
		return 0;
	}

	if((fp = fopen(fname, "rb")) == NULL) {
		perror(fname);
		return 0;
	}

	if(elfonly && !is_elf(fp))
		goto exit_use_guy;

#if !defined(__NT__) && !defined(__MINGW32__)
	setegid(getgid());
	seteuid(getuid());
#endif
	switch(infoflag) {
	case 1:	
		// "use -i"
		if (-1==elf_getsecinfo(fp,"QNX_info",&nbytes,NULL) || nbytes == 0) {
			fprintf(stderr,"No info available in %s.\n", fname);
			goto exit_use_guy;
		}
		allflag = 1;	/* Force RAW */
		break;
	case 2:	
		// "use -s"
		if (-1==elf_getsecinfo(fp,".ident",&nbytes,NULL) || nbytes == 0) {
			fprintf(stderr,"No info available in %s.\n", fname);
			goto exit_use_guy;
		}
		allflag = 1;	/* Force RAW */
		break;
	default:
		if (-1==elf_getsecinfo(fp,"QNX_usage",&nbytes,NULL)) {
			/* reset back to beginning of file */
			fseek(fp, 0L, SEEK_SET);

			if(fread(&hdr, 1, sizeof(hdr), fp) != sizeof(hdr)){
				bad_lmf(fname,fp,numlines);
				goto exit_use_guy;
			}
		
			if(hdr.rec_type != _LMF_DEFINITION_REC){
				bad_lmf(fname,fp,numlines);
				goto exit_use_guy;
			}
		
			fseek(fp, (long) hdr.data_nbytes, SEEK_CUR);
		
			while(fread(&hdr, 1, sizeof(hdr), fp) == sizeof(hdr)) {
				switch(hdr.rec_type) {
				case _LMF_RESOURCE_REC:
					if(fread(&res, 1, sizeof(res), fp) != sizeof(res)){
						bad_lmf(fname,fp,numlines);
						goto exit_use_guy;
					}
					hdr.data_nbytes -= sizeof(res);
					if(res.resource_type != 0) {
						fseek(fp, (long) hdr.data_nbytes, SEEK_CUR);
						continue;
						}
		
					nbytes = hdr.data_nbytes;
					break;
		
				case _LMF_COMMENT_REC:
				case _LMF_DATA_REC:
				case _LMF_FIXUP_SEG_REC:
				case _LMF_FIXUP_80X87_REC:
				case _LMF_ENDDATA_REC:
					fseek(fp, (long) hdr.data_nbytes, SEEK_CUR);
					continue;

				case _LMF_EOF_REC:
					break;

				case _LMF_DEFINITION_REC:
				default:
					bad_lmf(fname,fp,numlines);
					goto exit_use_guy;
				}
		
				break;
			}
			goto exit_use_guy;
		}
		break;
	}
	if(nbytes == 0)
		fprintf(stderr,"No usage available in %s.\n", fname);
	else
		print_data(fp,fname,nbytes);

exit_use_guy:
	fclose(fp);
	return nbytes;
}


#define IS_LANG(ptr) ((ptr)[0] == '%'  &&  (ptr)[1] == '=')
#define IS_MULT(ptr) ((ptr)[0] == '%'  &&  (ptr)[1] == '-')

#if !defined(__NT__) && !defined(__MINGW32__)
/* open pipe from filedes (1 or 2) to cmd. */
FILE *
open_help(char *cmd, int filedes)
{
	int fds[2];
	FILE *fp;
	char *argv[4];
	int		old;

	argv[0] = "sh";
	argv[1] = "-c";
	argv[2] = cmd;
	argv[3] = NULL;

	if(pipe(fds) == -1){
		perror("pipe:");
		return NULL;
	}
	fp = fdopen(fds[0], "r");

	old = dup(filedes);
	dup2(fds[1], filedes);
		
#if defined(__SOLARIS__) || defined(__LINUX__)
	switch(fork()) {
	case -1:
		perror("fork:");
		exit(EXIT_FAILURE);
		break;
	case 0:
		//child
		execvp("sh", argv);
		perror("execvp:");
		exit(EXIT_FAILURE);
	default:
		//parent
		break;
	}
#else
	if(spawnvp(P_NOWAITO, "sh", argv) == -1){
		perror("spawnvp:");
		exit(EXIT_FAILURE);
	}
#endif
	dup2(old,filedes);
	close(fds[1]);

	return fp;
}
#endif

void print_data( FILE *fp, char *full_fname, long nbytes )
	{
	char *fname=basename(full_fname);
	int i = 0, nlines = 0, cmdlen = strlen(fname), c;
	char *lang = getenv("LANG");

	while(nbytes >= 0) {
		buf[i] = getc(fp);
		if((infoflag == 2) && (buf[i] == '\0')) buf[i] = '\n';
		if(buf[i] == '\n' || nbytes == 0 || i == BUF_SIZE) {
			if(i == BUF_SIZE) /* Last char read was valid. */
				buf[i+1] = '\0';
			else
				buf[i] = '\0';
			lines[nlines++] = strdup(buf);
			if (nlines>MAX_USAGE) {
				char  *p = getenv("PAGER");
				int    j;
				FILE  *pf;
				int pid;
				int pfiled[2];
				int stat;

#if !defined(__NT__) && !defined(__MINGW32__)
				if (p == 0)
					p = "more";

				pipe(pfiled);
				pid = fork();
				switch(pid) {
				case -1:
					perror("fork:");
					exit(EXIT_FAILURE);
					break;
				case 0:
					//child
					close(0);
					dup(pfiled[0]);
					close(pfiled[1]);
					execlp(p, p, (char*) 0);
					perror("execlp");
					exit(EXIT_FAILURE);
				default:
#ifdef ALLOW_SIGINT
					signal(SIGINT,break_out);
#else
					signal(SIGINT,SIG_IGN);
#endif
					atexit(morekiller);
					more_pid = pid;
					close(pfiled[0]);
					//parent
					break;
				}

				signal(SIGPIPE, leave);
				
				for (j=0; j < nlines; j++) {
					write(pfiled[1], lines[j], strlen(lines[j]));
					write(pfiled[1], "\n", 1);
				}
				while (nbytes--) {
					char	c = getc(fp);
					if((infoflag == 2) && (c == '\0')) c = '\n';
					write(pfiled[1], &c, 1);
				}
				close(pfiled[1]);
				waitpid(pid, &stat, WUNTRACED);
				leave(EXIT_SUCCESS);
#else
				pf = stdout;
				for (j=0; j < nlines; j++) {
					fputs(lines[j], pf);
					putc('\n', pf);
				}
				while (nbytes--) {
					int	c = getc(fp);
					if((infoflag == 2) && (c == '\0')) c = '\n';
					putc(c, pf);
				}
#endif
			}
			i = -1;
		} 
		i++;
		nbytes--;
	}
	if (allflag) {
		for (i=0; i < nlines; i++) {
			puts(lines[i]);
		}
		return;
	}
	/*
	 * If multiple usage messages are present we locate the correct one.
	 * If we can't find one we use the first.
	 */
	if(IS_MULT(lines[0])) {
		for(i = 0 ; i < nlines ; ++i)
			if(IS_MULT(lines[i])  &&  strcmp(&lines[i][2], fname) == 0) {
				memcpy(&lines[0], &lines[i], (nlines -= i)*sizeof(char *));
				break;
				}

		memcpy(&lines[0], &lines[1], --nlines*sizeof(char *));
		}

	if(!IS_LANG(lines[0]))
		lang = NULL;	/* No multi-language support */

	/*
	 * Search for language. If we can't find it we use the first.
	 */
	if(lang)
		for(i = 0 ; i < nlines ; ++i)
			if(IS_LANG(lines[i])  &&  strcmp(&lines[i][2], lang) == 0) {
            	memcpy(&lines[0], &lines[i], (nlines -= i)*sizeof(char *));
				break;
				}

	for(i = IS_LANG(lines[0]) ? 1 : 0 ; i < nlines ; ++i) {
		if(numlines && i && !(i%(numlines-2))) {
			printf("More (Y/n)? ");
			fflush(stdout);
			raw(0);
			c = getchar() | ' ';
			unraw(0);
			printf("\r            \r");
			fflush(stdout);
			c=toupper(c);
			if (c=='N' || c=='Q') break;
		}

		if(lines[i][0] == '\t')
			printf("%*s %s\n", cmdlen, "", &lines[i][1]);
		else if(lines[i][0] == '%') {
			if(lines[i][1] == 'C')
				printf("%s %s\n", fname, (lines[i][2]) ? &lines[i][3] : "");
			else if(lines[i][1] == '='  ||  lines[i][1] == '-')
				break;
#if !defined(__NT__) && !defined(__MINGW32__)
			/* Ugliness but consistent with what was here already.  Recognize
			 * options like %n> cmd|%C [args] where n is 1 or 2 for stdout or
			 * stderr and %C is the name of the executable. */
			else if((lines[i][1] == '1' || lines[i][1] == '2') &&
			lines[i][2] == '>'){
				char buf[90];
				int pid;
				char *p;
				int pfiled[2];
				int stat;

				pipe(pfiled);
				pid = fork();
				switch(pid) {
				case -1:
					perror("fork:");
					exit(EXIT_FAILURE);
					break;
				case 0:
					//child
					close(0);
					dup(pfiled[0]);
					close(pfiled[1]);
					execlp("more", "more", (char*) 0);
					perror("execlp");
					exit(EXIT_FAILURE);
				default:
					atexit(morekiller);
#ifdef ALLOW_SIGINT
					signal(SIGINT,break_out);
#else
					signal(SIGINT, SIG_IGN);
#endif
					more_pid = pid;
					close(pfiled[0]);
					//parent
					break;
				}				

				fclose(fp);
				p = &lines[i][3];
				while(*p == ' ') ++p;
				
				if(p[0] == '%' && p[1] == 'C'){
					char *buf = malloc(strlen(full_fname) + strlen(lines[i]));
					if(!buf){
						fprintf(stderr, "out of memory");
						exit(EXIT_FAILURE);
					}
					strcpy(buf, full_fname);
					if(p[2]) /* we have arguments */
						strcat(buf, &p[2]);
					fp = open_help(buf, lines[i][1] - '0');
					free(buf);
				}
				else{
					fp = open_help(p, lines[i][1] - '0');
				}
				if(!fp){
					fprintf(stderr, "No use message available\n");
					exit(EXIT_FAILURE);
				}
				if(pid == -1)
					pfiled[1] = 1;
				while(fgets(buf, 90, fp)){
					if(feof(fp))
						break;
					write(pfiled[1], buf, strlen(buf));
				}
				fclose(fp);
				if(pfiled[1] != 1)
					close(pfiled[1]);
				waitpid(pid, &stat, WUNTRACED);
			}
#endif
			else
				printf("%s\n", &lines[i][1]);
			}
		else
			printf("%s\n", &lines[i][0]);
		}

	}


void
bad_lmf(char *fname,FILE *fp, int numlines)
{
	char *b, *p;
	char buf[133];
	int  i=0;
	int  cmdlen;

	rewind(fp);

	/* bad load module - maybe its a shell script. If it is, we want
       to extract the usage from its text */

	while ((b=fgets(buf,sizeof(buf),fp))) {
		if (*b=='#' && strstr(b,"__USAGE")!=NULL) break;
	}

	if (b) {
		fname = basename(fname);
		cmdlen = strlen(fname);

		/* found comment with __USAGE in it */
		while ((b=fgets(buf,sizeof(buf),fp))) {
			if ((p=strchr(buf,'\n'))) *p=0;	/* strip newlines */

       		if (*b!='#') break;				/* end if we run out of comment */
			b++;							/* skip past '#'				*/
			if (!strncmp(b,"endif",5)) break;	/* end on #endif				*/

			if(numlines && i && !(i%(numlines-2))) {
				int c;

				printf("More (Y/n)? ");
				fflush(stdout);
				raw(0);
				c = getchar() | ' ';
				unraw(0);
				printf("\r            \r");
				fflush(stdout);
				c=toupper(c);
				if (c=='N' || c=='Q') break;
			}

			if(*b == '\t') printf("%*s %s\n", cmdlen, "", b+1);
			else if(*b == '%') {
				if(b[1] == 'C') printf("%s %s\n", fname, (b[2]) ? &b[3] : "");
				else if(b[1]=='=' || b[1] == '-')	break;
				else printf("%s\n", &b[1]);
			} else printf("%s\n", b);
			i++;
		}
    } else {
		fprintf(stderr, "There is no usage message in %s.\n", fname);
		return;
	}
	return;
}

#if defined(__QNX__) || defined(__SOLARIS__) || defined(__LINUX__)
 #define PATH_SEP '/'
 #define LIST_SEP ':'
#else
 #define PATH_SEP '\\'
 #define LIST_SEP ';'
#endif

void
exec_searchenv( const char *name, const char *env_var, char *buffer ) {
	char		*path;
	char		*p;

	path = getenv(env_var);
	if(path != NULL) {
		p = buffer;
		/* scan through (semi)colon separated pathname components */
		for( ;; ) {
			if((*path == '\0') || (*path == LIST_SEP)) {
				if((p != buffer) && (p[-1] != PATH_SEP)) {
					*p++ = PATH_SEP;
				}
				strcpy(p, name);
				if(access(buffer, X_OK) == 0) return;
				p = buffer;
				if(*path == '\0') break;
				++path;
				continue;
			}
			*p++ = *path++;
		}
	}
	buffer[0] = '\0';
}

#if defined(__NT__) || defined(__MINGW32__) 

int raw(int fd)
{
	return 0;
}

int unraw(int fd)
{
	return 0;
}

#else

int raw(int fd)
{
#if defined(__QNX__) && !defined(__QNXNTO__)
    return(dev_mode(fd, 0, (_DEV_ECHO|_DEV_EDIT)));
#else
    struct termios termios_p;

    if( tcgetattr( fd, &termios_p ) )
        return( -1 );

    termios_p.c_cc[VMIN]  =  1;
    termios_p.c_cc[VTIME] =  0;
    termios_p.c_lflag &= ~( ECHO|ICANON|ECHOE|ECHOK|ECHONL );
    termios_p.c_oflag &= ~( OPOST );

    return( tcsetattr( fd, TCSADRAIN, &termios_p ) );
#endif
}

int unraw(int fd)
{
#if defined(__QNX__) && !defined(__QNXNTO__)
    dev_mode(fd, _DEV_MODES, _DEV_MODES);
	return 0;
#else
    struct termios termios_p;

    if( tcgetattr( fd, &termios_p ) )
        return( -1 );

    termios_p.c_lflag |= ( ECHO|ICANON|ISIG|ECHOE|ECHOK|ECHONL );
    termios_p.c_oflag |= ( OPOST );

    return( tcsetattr( fd, TCSADRAIN, &termios_p ) );
#endif
}
#endif


__SRCVERSION("use.c $Rev: 211168 $");
