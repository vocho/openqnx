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




#ifdef __QNXNTO__
// set to make esh be able to take the place of login to provide
// a direct shell. Disables command line option parsing.
#define CAN_PRETEND_LOGIN
#endif

#ifdef __USAGE
%-esh
%C - Embedded shell (QNX)

%C	[-irv] [script_file]
Options:
 -c "cmdline" Take next argument as a command to execute.
 -i           Enter interactive mode after running any script file.
 -r           Run in restricted mode.
 -v           Verbose. Echo commands before executing.

%-fesh
%C - "Fat" Embedded shell (QNX)

%C	[-irv] [script_file]
Options:
 -c "cmdline" Take next argument as a command to execute.
 -i           Enter interactive mode after running any script file.
 -r           Run in restricted mode.
 -v           Verbose. Echo commands before executing.

%-uesh
%C - micro-Embedded shell (QNX)

%C
#endif

#include <util/stdutil.h>
#include <util/util_limits.h>
#include <stdio.h>
#include <errno.h>
#include <fnmatch.h>
#include <string.h>
#include <malloc.h>
#include <unistd.h>
#include <fcntl.h>
#include <process.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <ctype.h>

#ifdef __QNXNTO__
	#include <spawn.h>
	#include <libgen.h>
	#include <sys/statvfs.h>
	#include <sys/netmgr.h>
	#include <sys/mount.h>
#else
	#include <sys/qnx_glob.h>
	#include <sys/dev.h>
	#include <sys/psinfo.h>
	#include <sys/osinfo.h>
	#include <sys/osinfo.h>
	#include <sys/disk.h>
#endif

#ifdef VARIANT_socketpair
/*
 * This variant is for testing socketpair().  This shell will use
 * socketpair(), instead of pipe(), for pipeline IPC.
 */

#include <sys/socket.h>
#endif

#ifdef EMBEDDED
#define MAX_LINELEN		120
#define MAX_TOKENLEN	400
#define MAX_TOKENS		200		
#else
#define MAX_LINELEN		1200
#define MAX_TOKENLEN	8000
#define MAX_TOKENS		800	
#define MAX_LINES		10
#define MAX_ALIAS		20
#endif

#ifdef __QNXNTO__
#define CSI_KEY         0x9b
#define UP_KEY			'A'
#define DN_KEY			'B'
#else
#define CSI_KEY         0xff
#define UP_KEY			0xA1
#define DN_KEY			0xA9
#endif

#ifndef PATH_MAX
#define PATH_MAX UTIL_PATH_MAX
#endif

extern char **environ;
// Global vars.
char	cmdline[MAX_LINELEN];
char	*tokenv[MAX_TOKENS];
char	tokenbuf[max(MAX_TOKENLEN, PATH_MAX)];
int		tokenc;
int		back;
int		execing;
int		laststat;
int		super;
int		iov[3];
int		pfds[2];
int		debug;
int		piping;
FILE	*infp;
#ifdef __QNXNTO__
	struct inheritance inherit;
#else
	/* Prototype for qnx_strtonid */
	nid_t qnx_strtonid(const char *, char **);

	nid_t	target_node;

	#define SPAWN_DEBUG	_SPAWN_DEBUG
	#define SPAWN_HOLD	_SPAWN_HOLD
#endif

#ifndef EMBEDDED
static int		login;
static int		recall_lineno;
static char	recall_buf[MAX_LINES][MAX_LINELEN];
static char	*alias[MAX_ALIAS];
static FILE	*stackfp[8];
static int		stackindex;
#ifdef FAT
static char	workbuf[256];
#endif
#endif

#ifdef __QNXNTO__
long	stackmax;
#endif

// Options.
int restrict;
int verbose;
int interactive;
int optc;


int
redir_space(char *s) {


	if(*s == '<'  || *s == '>' || (*s == '2'  &&  *(s+1) == '>')) {
		while(*++s == '>'  ||  *s == '<')
			;
		return(*s == '\0');
		}

	return(0);
	}


/*
char *
redir_fname(char *name, char **spp) {
	char *s1, *s2;

	while(*++name == '>'  ||  *name == '<')
		;

	if(*name == '\0') {
		for(s = *spp ; *s != ' '  &&  *s ; ++s)
			*s = '\';
		return(*s);
		}

	return(name);
	}
*/

//
// Take a cmdline and tokenize it into another buffer with an argv
// array pointing to each token.
//
int
tokenize(char *cmd) {
	int err;
	unsigned n;
	char *s1, *s2, quoting, pattern, next;

	iov[0] = iov[1] = iov[2] = -1;

#ifdef __QNXNTO__
	debug = 0;
	stackmax = 0;
#else
	target_node = 0;
#endif
	piping = 0;
	if(pfds[0]) {
		piping = 1;
		iov[0] = pfds[0];
		pfds[0] = 0;
		}

	for(s1 = cmd, s2 = tokenbuf, err = n = back = 0 ; *s1 ; ++s1) {
		if(*s1 == ' ')
			continue;

		tokenv[n] = s2;
getarg:
		quoting = pattern = 0;

 		while((next = *s1) && (quoting || next != ' ')) {
 			if((!quoting && next == '\\')&& *(s1+1)) {
				*s2++ = *++s1;
				}

#ifndef EMBEDDED

			else if(!quoting && next == '$') {
				char *s_dollar = s1;
				char *s3, term;
				int get_laststat = 0;
				term = ' ';
				if(*++s1 == '{') {
					term = '}';
					++s1;
					}
				if (*s1 == '?') {
					/* Special case of ${?} or $? */
					s3 = s1;
					s1 ++;
					get_laststat = 1;
				} else {
					for(s3 = s1; *s1 && *s1 != term; ++s1)
					{	
						// break on ':', '." and others 
						// non-alphanumeric characters PR5778
						if ( !isalnum(*s1) && *s1 != '_'){ 
							break;
						}
					}
				}
				if ((term == '}' && *s1 != term) ||
					s1 == s3) {
					/* Not a properly terminated variable, or all that
					 * is left is "$" or "${", so it isn't treated as
					 * a variable, and the text is just transferred to
					 * s2... */
					memcpy(s2, s_dollar, (s1-s_dollar)*sizeof(char));
					s2+=(s1-s_dollar);
				} else {
					if (get_laststat)
						s2 += sprintf(s2, "%d", laststat);
					else {
						char saved=*s1;
						*s1=0;
						if(s3 = getenv(s3)) {
							strcpy(s2, s3);
							s2 += strlen(s3);
						}
						*s1=saved;
					}
					if (term == '}')
						s1++;	/* eat up the '}' */
				}
				s1--;
			}
#endif
#ifdef FAT
			// Recognise '|' '||' '<' '<<' '>' '>>' without leading
			// space, and '2>' only with.
			else if(!quoting && (next == '|' || next == '<' || next == '>')) {
				if((s2 == tokenv[n] || next == tokenv[n][0])
				  || (tokenv[n][0] == '2' && next == '>')) {
					*s2++ = next;
					}
				else {
					--s1;
					break;
					}
				}
#endif
			else if((next == '"')||(next==39))
				quoting = !quoting;
			else {
				*s2++ = next;
#ifndef EMBEDDED
				if(!quoting  &&  (next == '*'  ||  next == '?'))
					pattern = 1;
#endif
				}
			if(next)
				++s1;
			}

		*s2 = '\0';

		// Check for space after redirection. IE:  > fname   or  < fname
		// and remove it.
		if(redir_space(tokenv[n])) {
			++s1;
			goto getarg;
			}
		++s2;

#ifdef __QNXNTO__
		// stackmax flags
		if(n == 0 && strncmp(tokenv[n], "-S", 2) == 0) {
			stackmax = atoi(&tokenv[n][2]);
			s2 = tokenv[n];
			}

		// debug flags
		else if(n == 0 && strcmp(tokenv[n], "-D") == 0) {
			s2 = tokenv[n];
			debug |= SPAWN_DEBUG;
			}
#else
		// Remote process creation.
		if(n == 0  &&  tokenv[0][0] == '/'  &&  tokenv[0][1] == '/'  &&
		   strchr(&tokenv[0][2], '/') == NULL) {
			if(restrict) {
				err = EPERM;
				break;
				}
			s2 = tokenv[n];
			target_node = qnx_strtonid(s2 + 2,NULL);

			}
#endif

		// hold process
		else if(n == 0 && strcmp(tokenv[n], "-H") == 0) {
			s2 = tokenv[n];
			debug |= SPAWN_HOLD;
			}

		// IO redirection.
		else if(strcmp(tokenv[n], "&") == 0) {
			s2 = tokenv[n];
			++back;
			}

		else if(tokenv[n][0] == '<'  &&  iov[0] == -1) {
			s2 = tokenv[n];
			if((iov[0] = open(s2 + 1, O_RDONLY)) == -1) {
				err = errno;
				break;
				}
			}

		else if(tokenv[n][0] == '>'  &&  iov[1] == -1) {
			s2 = tokenv[n];
			iov[1] = (tokenv[n][1] == '>')	? open(s2 + 2, O_WRONLY | O_APPEND | O_CREAT, 0666)
											: creat(s2 + 1, 0666);
			if(iov[1] == -1) {
				err = errno;
				break;
				}
			}

		else if(tokenv[n][0] == '2'  &&  tokenv[n][1] == '>'  &&  iov[2] == -1) {
			s2 = tokenv[n];
			iov[2] = (tokenv[n][2] == '>')	? open(s2 + 3, O_WRONLY | O_APPEND | O_CREAT, 0666)
											: creat(s2 + 2, 0666);
			if(iov[2] == -1) {
				err = errno;
				break;
				}
			}
#ifndef EMBEDDED

		// Pipe. 
		else if(tokenv[n][0] == '|') {
#ifdef VARIANT_socketpair
			if(socketpair(AF_UNIX, SOCK_STREAM, 0, pfds)) {
#else
			if(pipe(pfds)) {
#endif
				err = errno;
				break;
				}
			iov[1] = pfds[1];
			pfds[1] = 0;
			back = 1;
			optc = 1;

			// Keep chars following pipe.
			strcpy(cmd, &tokenv[n][1]);
			strcat(cmd, s1);
			break;
			}

		// Filename expansion.
		else if(pattern) {
				DIR *dirp;
				struct dirent *dp;
				char *stem = ".", *pat, gotone=0;

				if(pat = strrchr(tokenv[n], '/')) {
					*pat++ = '\0';
					stem = tokenv[n];
					}
				else
					pat = tokenv[n];

				if((dirp = opendir(stem)) == NULL) {
					err = errno;
					break;
					}

				while(dp = readdir(dirp)) {
					if(dp->d_name[0] == '.')
						continue;
					if(fnmatch(pat, dp->d_name, 1) == 0) {
						int tlen;
						tlen = strlen(stem)+strlen(dp->d_name)+2; /* 1 for / and 1 for null */
						if ( (n<MAX_TOKENS)  && 
						   (s2+tlen <= (tokenbuf+sizeof(tokenbuf) )) ){
						    tokenv[n++] = strcpy(s2, stem);
						    strcat(s2, "/");
						    strcat(s2, dp->d_name);
						    s2 += tlen;
						    gotone = 1;
						} else {
		                                    errno = E2BIG; err=errno;
						    break;
						}
						
					}
				}
				closedir(dirp);

				if(gotone == 0) {
					tokenv[n++] = strcpy(s2, pat);
					s2 += strlen(s2) + 1;
					}
				}
#endif
		else {
			++n;
			}

		if(*s1 == '\0') --s1;
		}

	tokenv[n] = NULL;
	tokenc = n;

	return(err);
	}

#ifndef EMBEDDED
int
stack(char *path) {
	FILE *fp;

	if((fp = fopen(path, "r")) == NULL) {
		fprintf(stderr, "%s: %s\n", strerror(errno), path);
		return(-1);
		}

	stackfp[stackindex++] = infp;
	infp = fp;
	return(0);
	}


char *
recall_line(int c, char *buf) {

	if((c == UP_KEY  &&  recall_lineno < MAX_LINES-1)  ||
	   (c == DN_KEY  &&  recall_lineno > 1))
		recall_lineno += (c == UP_KEY) ? 1 : -1;

	strcpy(buf, recall_buf[MAX_LINES - recall_lineno]);

#ifdef __QNXNTO__
	tcinject(fileno(infp), buf, strlen(buf));
#else
	dev_insert_chars(fileno(infp), strlen(buf), buf);
#endif
	return(buf);
	}



int
aliascmd(char *cmd, int query) {
	int i;
	char *ap, *cp, *dp;

	for(i = 0 ; i < MAX_ALIAS ; ++i) {
		if(alias[i] == NULL)
			continue;
		for(ap = alias[i], cp = cmd ; *ap  &&  *ap == *cp ; ++ap, ++cp)
			if(query  &&  *ap == '=')
				return(i);

		if(*ap == '='  &&  (*cp == '\0'  ||  *cp == ' ')) {
			++ap;
			strcpy(cmd, cp);	// Remove alias match.
			i = strlen(cmd);
			for(cp = cmd + i, dp = cp + strlen(ap) ; i-- >= 0 ; --cp, --dp)
				*dp = *cp;
			memcpy(cmd, ap, strlen(ap));
			break;
			}
		}

	return(i);
	}
#endif

	
#ifdef FAT

#ifndef __QNXNTO__
void
print_einfo() {
	long unsigned freepmem, totpmem;
	struct _osinfo osdata;

	qnx_osinfo(0, &osdata);

	freepmem = osdata.freepmem;
	if((totpmem = osdata.totpmem) == 0) {
		totpmem = osdata.totmemk * 1000L;
		freepmem = osdata.freememk * 1000L;
		}

	printf(" Node    CPU    Machine Speed     Memory    Ticksize\n");
	printf("%5ld %4ld/%-4ld %7.7s %5u %5luk/%5luk %2u.%ums\n\n",
			osdata.nodename,
			osdata.cpu,
			osdata.fpu,
			osdata.machine,
			osdata.cpu_speed,
			freepmem/1000,
			totpmem/1000,
			(osdata.tick_size + 10)/1000,
			((osdata.tick_size + 1)/100)%10
			);

	printf("Hands Names Sessions Procs Timers Nodes Virtual\n");
	printf("%5u %5u %8u %5u %6u %5u",
			osdata.num_handlers,
			osdata.num_names,
			osdata.num_sessions,
			osdata.num_procs,
			osdata.num_timers,
			osdata.max_nodes
			);

	if(osdata.totvmem)
		printf(" %5luM/%5luM\n", osdata.freevmem/1000000, osdata.totvmem/1000000);
	else
		printf("   ---\n");
	}
#endif

static int els_col;

void
print_els(char *fname, struct stat *sbuf) {

	if(fname[0] == '.'  &&  fname[1] == '/') 
		fname += 2;

	if(sbuf) {
#if _FILE_OFFSET_BITS-0 == 64
		printf("%4.4o %6d %6d %8llu %s\n",
#else
		printf("%4.4o %6d %6d %8u %s\n",
#endif
			sbuf->st_mode & 0xfff, sbuf->st_uid, 
			sbuf->st_gid, sbuf->st_size, fname);
	} else {
		char format[8];

		int len = (strlen(fname) >> 4) + 1;

		if((els_col += len) > 4) {
			printf("\n");
			els_col = len;
			}

		sprintf(format, "%%-%ds", len << 4);
		printf(format, fname);
		}

	}


int
ecp(char *sname, char *dname, int verbose) {
	FILE *src, *dst;
	int c;
	struct stat sbuf;

	if(verbose)
		printf("copying %s to %s\n", sname, dname);

	if((src = fopen(sname, "r")) == NULL)
		return(errno);

	fstat(fileno(src), &sbuf);
	if(S_ISDIR(sbuf.st_mode)) {
		fclose(src);
		printf("Skipping directory %s\n", sname);
		return(EOK);
		}

	if(dname == NULL)
		dst = stdout;
	else if((dst = fopen(dname, "w")) == NULL) {
		fclose(src);
		return(errno);
		}

	while((c = getc(src)) != EOF)
		putc(c, dst);

	if(dname)
		fclose(dst);
	fclose(src);

	return(EOK);
	}
#endif


//
// Handle a bunch of local commands.
//
int
localcmd(void) {
	char **argv = tokenv;
	unsigned n;

	if(argv[0] == NULL  ||  strlen(argv[0]) == 0)
		return(EOK);

#ifndef EMBEDDED
	if(strcmp(argv[0], "alias") == 0) {

		if(argv[1] == NULL) {
			for(n = 0 ; n < MAX_ALIAS ; ++n)
				if(alias[n])
					printf("%s\n", alias[n]);
			return(EOK);
			}
			
		if((n = aliascmd(argv[1], 1)) == MAX_ALIAS)
			for(n = 0 ; n < MAX_ALIAS ; ++n)	// Its a new one.
				if(alias[n] == NULL)
					break;

		if(n >= MAX_ALIAS)
			return(ENOMEM);

		if(alias[n])
			free(alias[n]);

		alias[n] = strdup(argv[1]);
		return(EOK);
		}
#endif

	if(strcmp(argv[0], "cd") == 0) {
#ifndef __QNXNTO__
		if(restrict  &&  argv[1]  &&
		   argv[1][0] == '/'  &&  argv[1][1] == '/')
			return(EPERM);
#endif
		if(argv[1] == NULL  &&  (argv[1] = getenv("HOME")) == NULL)
			return(ENOENT);
		if(chdir(argv[1]) == -1)
			return(errno);
		return(EOK);
		}

	if(strcmp(argv[0], "exec") == 0) {
#ifndef EMBEDDED
		if (restrict && strchr(argv[1], '/'))
			return(EPERM);
#endif
#ifdef __QNXNTO__
		execing = 1;
#else
		execing = P_OVERLAY;
#endif
        if (argv[1] == NULL)    /* exec with no argument */
            return EINVAL;
		for(n = 0 ; n < tokenc ; ++n)
			tokenv[n] = tokenv[n + 1];
		if (tokenv[0] == NULL) {
			return (EINVAL);
		}
		--tokenc;
		return(-1);
		}

	if(strcmp(argv[0], "exit") == 0)
		exit(argv[1] ? atoi(argv[1]) : laststat);

	if(strcmp(argv[0], "export") == 0) {
		if(argv[1] == NULL  ||  argv[1][0] == '-')
dump_env:
			for(n = 0 ; environ[n] ; ++n)
				printf("%s\n", environ[n]);
		else {
			if(restrict)
				return(EPERM);
			putenv(strdup(argv[1]));
			}
		return(EOK);
		}

	if(strcmp(argv[0], "set") == 0) {
		if(argv[1]) {
			if(argv[1][0] == '-')
				verbose = strchr(argv[1], 'v') != 0;
			return(EOK);
			}
		else
			goto dump_env;
		}

	if(strcmp(argv[0], "unset") == 0) {
		if(restrict)
			return(EPERM);
		unsetenv(argv[1]);
		return(EOK);
		}

	if(strcmp(argv[0], "ewaitfor") == 0) {
		int			msec = 100;

		if(argv[1] == 0)
			return(EOK);

		if(argv[3])
			msec = atoi(argv[3]);

		if(argv[2])
			n = atoi(argv[2]) * 1000;
		else
			n = 1000;

		if ( waitfor( argv[1], n, msec ) == -1 ) {
				return(ENOENT);
			}

		return(EOK);
		}

#ifdef __QNXNTO__
	if(strcmp(argv[0], "emount") == 0) {
		char	*type = QNX4_FS_TYPE;

		if(argv[3])
			type = argv[3];

		if(mount(argv[1], argv[2], 0, type, "", -1) == -1)
			return(errno);

		return(EOK);
	}
#endif

#ifndef EMBEDDED
#ifdef FAT
	if(strcmp(argv[0], "pwd") == 0 || strcmp(argv[0], "epwd") == 0) {
#else
	if(strcmp(argv[0], "pwd") == 0) {
#endif
		if(getcwd(cmdline, UTIL_PATH_MAX) == NULL)
			return(errno);

		printf("%s\n", cmdline);
		return(EOK);
		}

#ifdef FAT
	if(strcmp(argv[0], "print") == 0 || strcmp(argv[0], "eecho") == 0) {
#else
	if(strcmp(argv[0], "print") == 0) {
#endif
		for(n = 1 ; argv[n] ; ++n)
			printf("%s%s", n > 1 ? " " : "", argv[n]);
		printf("\n");

		return(EOK);
		}

	if(strcmp(argv[0], "reopen") == 0) {
		int save;

		if(restrict)
			return(EPERM);

		save = dup(0);
		close(0);
#ifdef __QNXNTO__
		if(open(argv[1] ? argv[1] : "/dev/console", O_RDWR) == -1) {
#else
		if(open(argv[1] ? argv[1] : "//0/dev/con1", O_RDWR) == -1) {
#endif
			dup(save);
			printf("Unable to reopen\n");
			}
		else {
			close(1);
			dup(0);
			close(2);
			dup(0);
			}

		if(save > 2)
			close(save);

		return(EOK);
		}

	if(strcmp(argv[0], ".") == 0) {
		if (argv[1] == NULL)
			return(ENOEXEC);
		if(stack(argv[1]) != 0)
			return(errno);

		fcntl(fileno(infp), F_SETFD, FD_CLOEXEC);
		return(EOK);
		}

	if(strcmp(argv[0], "kill") == 0)
		return argv[1] ? (kill(atoi(argv[1]), atoi(argv[1]) ? SIGTERM : 0) == -1 ? errno : EOK) : ESRCH;

	if(strcmp(argv[0], "ontty") == 0) {
#ifdef __QNXNTO__
		inherit.flags |= SPAWN_SETSID | SPAWN_SETGROUP | SPAWN_TCSETPGROUP;
		inherit.pgroup = SPAWN_NEWPGROUP;
#else
		qnx_spawn_options.flags |= _SPAWN_NEWPGRP|_SPAWN_TCSETPGRP|_SPAWN_SETSID;
		qnx_spawn_options.ctfd = 0;
#endif
		if((iov[0] = iov[1] = iov[2] = open(argv[1], O_RDWR)) == -1)
			return(errno);
		for(n = 0 ; n < tokenc - 1; ++n)
			tokenv[n] = tokenv[n + 2];
		tokenc -= 2;
		back = 1;
		return(-1);
		}
#endif

#ifdef FAT
	// A bunch of built-ins to avoid external utilities like ls
	if(strcmp(argv[0], "els") == 0) {
		int i, verbose = 0, n = 1;
		DIR *dirp;
		char *path;
		struct dirent *dp;
		struct stat sbuf;

		if(argv[1]  &&  strcmp(argv[1], "-l") == 0) {
			verbose = 1;
			n = 2;	
			}

		if(argv[n] == NULL) {
			argv[n] = ".";
			argv[n + 1] = NULL;
			}

		for(i = n ; argv[i] ; ++i) {

			path = argv[i];
			if(stat(path, &sbuf) == -1) {
				els_col = 0;
				return(errno);
				}


			if(!S_ISDIR(sbuf.st_mode)) {
				print_els(path, verbose ? &sbuf : NULL);
				}
			}

		for(i = n ; argv[i] ; ++i) {

			path = argv[i];
			if(stat(path, &sbuf) == -1)
				return(errno);

			if(!S_ISDIR(sbuf.st_mode))
				continue;

			if(argv[n+1])  // There is more than one argument
				printf("\n\n%s:\n", path);

			// Restart the collum count
			els_col = 0;

			if((dirp = opendir(path)) == NULL)
				return(errno);

			while(dp = readdir(dirp)) {
				if(verbose) {
					strcpy(workbuf, path);
					if(strcmp(path, "/") != 0) strcat(workbuf, "/");
					strcat(workbuf, dp->d_name);
					if(stat(workbuf, &sbuf) != -1) {
						print_els(dp->d_name, &sbuf);
						continue;
						}
					}

				print_els(dp->d_name, NULL);
				}

			closedir(dirp);
			}

		if(els_col) {
			printf("\n");
			els_col = 0;
			}


		return(EOK);
		}

	if(strcmp(argv[0], "erm") == 0) {
		int i;

		for(i = 1 ; argv[i] ; ++i)
			if(unlink(argv[i]) != 0)
				return(errno);

		return(EOK);
		}

	if(strcmp(argv[0], "emkdir") == 0) {
		if(mkdir(argv[1], 0666) != 0)
			return(errno);

		return(EOK);
		}

	if(strcmp(argv[0], "ermdir") == 0) {
		if(rmdir(argv[1]) != 0)
			return(errno);

		return(EOK);
		}

	if(strcmp(argv[0], "ecp") == 0) {
		int i, n, status, verbose;
		struct stat sbuf;
		char * tail;
		char * filename;

		// Find last arg
		for(n = 1 ; argv[n] ; ++n)
			;

		// one arg is copied to stdout.
		verbose = --n && !strcmp(argv[1], "-v");
		if(n - verbose == 0) 
			return(EINVAL);

		// If not a directory check for less than 3 args and copy
		if((stat(argv[n], &sbuf) == -1 || !S_ISDIR(sbuf.st_mode))) {
			if(n - verbose > 2) 
				return(ENOTDIR);
			else
				return ecp(argv[verbose + 1], argv[verbose + 2], verbose);
		}

		// optimise for more than one files
		strcat(strcpy(workbuf, argv[n]), "/");
		tail = workbuf + strlen(workbuf);

		for(i = verbose + 1 ; i < n ; ++i) {
			/*
			 * Use only the filename portion of the from-file, and
			 * append it to the to-dir.
			 * 
			 * e.g.:
			 *   # ecp dir1/dir2/file dir3
			 *  is equivalent to:
			 *   # ecp dir1/dir2/file dir3/file
			 * and:
			 *   # ecp file dir
			 *  is equivalent to:
			 *   # ecp file dir/file
			 */
			filename=strrchr(argv[i],'/');
			if (filename)
				filename ++;	/* skip the '/' */
			else
				filename=argv[i];
			strcpy(tail, filename);
			if((status = ecp(argv[i], workbuf, verbose)) != EOK)
				return(status);
			}

		return(EOK);
		}

#ifndef __QNXNTO__
	if(strcmp(argv[0], "esin") == 0) {
		struct _psinfo data;
		
		for(n = 1 ; qnx_psinfo(0, n, &data, 0, 0) != -1 ; n = data.pid + 1)
			if(data.flags & _PPF_MID)
				printf("Proxy   %5d\n", data.pid);
			else if(data.flags & _PPF_VID)
				printf("VC      %5d\n", data.pid);
			else
				printf("Process %5d %s\n", data.pid, data.un.proc.name);

		return(EOK);
		}

	if(strcmp(argv[0], "einfo") == 0) {
		print_einfo();

		return(EOK);
		}

	if(strcmp(argv[0], "edf") == 0) {
		int fd;
		struct _disk_entry d_bfr;
		long total, free, used;

		errno = ENOENT;
		if(argv[1]  &&  (fd = open(argv[1], 0)) != -1) {
			if(disk_get_entry(fd, &d_bfr) == 0  &&  disk_space(fd, &free, &total) == 0) {
				used = total - free;
				printf("Name                 Total      Used      Free  Percent\n");
				printf("%-16s %8dK %8dK %8dK    %d%%\n",
						argv[1], total/2, used/2, free/2, (used*100)/total);

				close( fd );
				return(EOK);
				}

			close( fd );
			}

		return(errno);
		}
#else
	if(strcmp(argv[0], "edf") == 0) {
		struct statvfs		buff;

		errno = ENOENT;
		if(!argv[1])
			argv[1]=".";
			
		if(argv[1]  &&  (statvfs(argv[1], &buff)) != -1) {
			fsblkcnt_t			used;

			if(buff.f_frsize >= 1024) {
				buff.f_blocks = buff.f_blocks * (buff.f_frsize / 1024); 
				buff.f_bfree = buff.f_bfree * (buff.f_frsize / 1024); 
			} else {
				buff.f_blocks = buff.f_blocks / (1024 / buff.f_frsize); 
				buff.f_bfree = buff.f_bfree / (1024 / buff.f_frsize); 
			}
			used = buff.f_blocks - buff.f_bfree;

			printf("Name                 Total      Used      Free  Percent   Type\n");
#if _FILE_OFFSET_BITS-0 == 64
			printf("%-16s %8lldK %8lldK %8lldK    %3lld%%    %s\n",
#else
			printf("%-16s %8dK %8dK %8dK    %3d%%    %s\n",
#endif
					argv[1], buff.f_blocks, used, buff.f_bfree, (used*100)/buff.f_blocks, buff.f_basetype);
			return(EOK);
			}

		return(errno);
		}
#endif
#endif

#ifndef EMBEDDED
	// Check for restricted executables
	if(restrict  &&  strchr(argv[0], '/'))
		return(EPERM);
#endif

	return(-1);
	}


//
// Return a null terminated command line with the trailing \n removed.
// Lines which are too long are consumed and ignored with a warning.
//
int
fgetline(rbuff, n, stream)
int		n;					// Maximum number of characters to read
unsigned char	*rbuff;				// Array to place read characters in
FILE	*stream;			// Stream to read data from
	{
	extern char *recall_line(int, char *);
	int			c=0;		// Return from getc() calls
	unsigned char *cs;		// Pointer into character array returned

	cs = rbuff;
#ifndef EMBEDDED
	recall_lineno = 0;
#endif
	while(--n > 0) {
		c = getc(stream);
		if (c == EOF && cs == rbuff) {
			/* EOF, and empty buffer */
			return -1;
		}
		if (c != EOF && c != '\n') {
			*cs++ = (unsigned char) c;
		} else {
			/* EOF or \n: both of which are valid end-of-line */
			*cs = '\0';
#ifndef EMBEDDED
			if((cs > rbuff)  &&  strcmp(recall_buf[MAX_LINES - 1], rbuff) != 0) {
				memcpy(recall_buf[0], recall_buf[1], (MAX_LINES-1)*sizeof(recall_buf[0]));
				strcpy(recall_buf[MAX_LINES - 1], rbuff);
				}
#endif
			return(cs - rbuff);
			}
#ifndef EMBEDDED
		if((cs - rbuff) >= 2  &&  *(cs-2) == CSI_KEY  &&  (c == UP_KEY  || c == DN_KEY))
			cs = recall_line(c, rbuff);
#endif
		}

	fprintf(stderr, "Line too long : %s\n", rbuff);

	while((c = getc(stream)) != '\n')
		if (c == EOF)
			return -1;

	return(0);
	}


int main(int argc, char *argv[]) {
	int		n, status;
	pid_t	pid;
#ifndef EMBEDDED
	int		opt = 0;
	char	*p;
#endif
#ifdef CAN_PRETEND_LOGIN
	int islogin=0;
#endif

#ifdef CAN_PRETEND_LOGIN
	if (!strcmp("login",basename(argv[0]))) {
		islogin++;                                                             
#ifndef EMBEDDED
		login = 1;
#endif
    }        
#endif

	infp = stdin;

	signal(SIGINT, SIG_IGN);
#ifdef __QNXNTO__
	if(tcgetpgrp(0) == 0) {
		tcsetpgrp(0, getpid());
		}
#ifdef NOT_DEFINED
	if(tcgethup(0) == 0) {
		tcsethup(0, getpid());
		}
#endif
#endif

#ifndef EMBEDDED
	super = geteuid() == 0;

	if(readlink(p = argv[0], tokenbuf, PATH_MAX) != -1)
		p = tokenbuf;
	if(strcmp(p + strlen(p) - 4, "resh") == 0)
		restrict = 1;
	if(*p == '-')
		login = restrict + 1;

#ifdef CAN_PRETEND_LOGIN
	if (!islogin) {
#endif
	while ((opt = getopt(argc, argv, "c:irv")) != -1) {
		switch(opt)	{
			case 'c':
				optc = 1;
				interactive = -1;
				strcpy(cmdline, optarg);
				break;

			case 'i':
				interactive = 1;
				break;

			case 'r':
				restrict = 1;
				break;

			case 'v':
				verbose = 1;
				break;
			}
		}
#ifdef CAN_PRETEND_LOGIN
	} else {		// else it IS running as login
		argv=argv;
		optind=argc;	// ignore arguments; will cause shell to be
                        // interactive
	}
#endif
#else
	argv = argv;		// To disable compiler warning about unreferenced.
	optind = 1;
#endif

	if(optind < argc) {
#ifndef EMBEDDED
		if(stack(argv[optind]) != 0)
			exit(1);
		fcntl(fileno(infp), F_SETFD, FD_CLOEXEC);
#else
		if((infp = fopen(argv[1], "r")) == NULL) {
			fprintf(stderr, "%s: %s\n", strerror(errno), argv[1]);
			exit(1);
			}
#endif
		}
	else
		if(!optc)
			interactive = 1;

#ifndef EMBEDDED
	if(login  &&  (opt = stack("/etc/esh")) == 0)
		restrict = 0;
	else if(login && opt == -1 && restrict) { /* stack() failed, and we're restricted */
		sleep(1); /* wait for error message to make it out to modem/telnet session */
		exit(1);
	}
#endif

	if(getenv("PATH") == NULL) {
#ifdef __QNXNTO__
		strcpy(tokenbuf, "PATH=");
		confstr(_CS_PATH, tokenbuf + sizeof "PATH=" - 1, sizeof tokenbuf - sizeof "PATH=" - 1);
		putenv(strdup(tokenbuf));
#else
		putenv("PATH=/bin:/usr/bin");
#endif
	}

	if(getenv("SYSNAME") == NULL)
#ifdef __QNXNTO__
		putenv("SYSNAME=nto");
#else
		putenv("SYSNAME=qnx4");
#endif

	for(;;) {
#ifndef EMBEDDED
		while((pid = waitpid(-1, &status, WNOHANG)) > 0) {
			if(interactive == 1) {
				if(WIFSIGNALED(status))
					printf("%d terminated with signal %d\n", pid, WTERMSIG(status));
				else if(WIFEXITED(status))
					printf("%d exited with status %d\n", pid, WEXITSTATUS(status));
				}
			}

		if(optc) {
			optc = 0;
			goto doit;
			}

		if(interactive == 1  &&  isatty(fileno(infp))) {
			struct termios tios;

#ifndef ECHOKE
#define ECHOKE	0
#endif
#ifndef ONLCR
#define ONLCR	0
#endif
#ifndef ECHOCTL
#define ECHOCTL	0
#endif
			if(tcgetattr(fileno(infp), &tios) != -1) {
				tios.c_lflag |= ECHO | ECHOE | ECHOK | ECHOKE | ECHONL | ICANON | IEXTEN | ISIG | ECHOCTL;
				tios.c_iflag |= ICRNL|BRKINT;
				tios.c_oflag |= OPOST|ONLCR;
				tios.c_cflag |= CREAD;
				tcsetattr(fileno(infp), TCSANOW, &tios);
				}
			printf(super ? "# " : "$ ");
			fflush(stdout);
			}
#else
		pid = 0;		// Stop compiler complaining...
		if(interactive == 1) {
			printf(super ? "# " : "$ ");
			fflush(stdout);
			}
#endif

		if(interactive == -1)
			break;

		if((n = fgetline(cmdline, sizeof(cmdline), infp)) == -1) {
#ifndef EMBEDDED
			if(stackindex) {
				fclose(infp);
				infp = stackfp[--stackindex];
				if(stackindex == 0) {
					if(!interactive)
						break;
					if(login) {
						restrict = login - 1;
						login = 0;
						}
					}
				continue;
				}
#endif
			break;
			}

		if(n == 0)
			continue;

		// Strip leading white space in a heavy handed way.
		while(*cmdline == ' '  ||  *cmdline == '\t')
			strcpy(cmdline, cmdline + 1);

		if(*cmdline == '#')
			continue;

#ifndef EMBEDDED
		aliascmd(cmdline, 0);
#endif

		if(verbose) {
			fprintf(stdout, "%s\n", cmdline);
			fflush(stdout);
			}

#ifndef EMBEDDED
doit:
#endif
		if(status = tokenize(cmdline)) {
			fprintf(stderr, "%s\n", strerror(status));
			continue;
			}

#ifdef DEBUG
		for(n = 0 ; n < tokenc ; ++n)
			printf("\"%s\"\n", tokenv[n]);
#else

#ifdef __QNXNTO__
		execing = 0;

		sigfillset(&inherit.sigdefault);
		inherit.flags = SPAWN_SETSIGDEF;

		if(back) {
			sigemptyset(&inherit.sigignore);
			sigaddset(&inherit.sigignore, SIGINT);
			sigaddset(&inherit.sigignore, SIGQUIT);
			inherit.flags |= SPAWN_SETSIGIGN;
			}
#else
		execing = P_NOWAIT;
		qnx_spawn_options.node = target_node;
		qnx_spawn_options.flags = _SPAWN_SIGCLR;
		qnx_spawn_options.ctfd = (char) -1;
		qnx_spawn_options.flags |= debug;
		if(back)
			qnx_spawn_options.flags |= _SPAWN_BGROUND;
#endif

		if((status = localcmd()) >= 0) {
			back = 1;
			if(status)
				fprintf(stderr, "%s: %s\n", tokenv[0], strerror(status));
			}
		else {
#ifdef __QNXNTO__
			int			fd[3];

			fd[0] = iov[0] == -1 ? STDIN_FILENO : iov[0];
			fd[1] = iov[1] == -1 ? STDOUT_FILENO : iov[1];
			fd[2] = iov[2] == -1 ? STDERR_FILENO : iov[2];
			if(execing)
				inherit.flags |= SPAWN_EXEC;
			inherit.flags |= debug;

			/* fix for P.R. 4286 */
			if(back && !piping && interactive == 1){
				inherit.flags |= SPAWN_SETGROUP;
				inherit.pgroup = SPAWN_NEWPGROUP;
			}
			
			if(stackmax) {
				inherit.flags |= SPAWN_SETSTACKMAX;
				inherit.stack_max = stackmax;
				}
			pid = spawnp(tokenv[0], 3, fd, &inherit, tokenv, environ);
#else
			for(n = 0 ; n < 3 ; ++n)
				qnx_spawn_options.iov[n] = iov[n];
			pid = spawnvp(execing, tokenv[0], tokenv);
#endif
			if((back || debug)  &&  interactive == 1) {
				if(pid == -1)
					printf("Unable to start %s: %s\n", tokenv[0], strerror(errno));
				else
					printf("%d running %s\n", pid, tokenv[0]);
				}
			}

		for(n = 0 ; n < 3 ; ++n)
			if(iov[n] != -1)
				close(iov[n]);

		if(!back) {
			if(pid == -1) {
				fprintf(stderr, "%s\n", strerror(errno));
				laststat = EXIT_FAILURE;
#ifndef EMBEDDED
				// constant signals will abort "." scripts
				if(errno == EINTR && stackindex) {
					fclose(infp);
					infp = stackfp[--stackindex];
					}
#endif
				continue;
				}

			waitpid(pid, &status, 0);

			if(WIFSIGNALED(status) && interactive == 1)
				printf("%s terminated with signal %d\n", tokenv[0], WTERMSIG(status));

			status = WEXITSTATUS(status);
			}

		laststat = status;
#endif
		}

	return laststat;
	}


