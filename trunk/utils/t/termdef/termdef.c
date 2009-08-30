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





/* define RTERMDEF to make restricted version of this utility */

/*
#ifdef __USAGE
%-termdef
%C - display or set the terminal type (QNX)

%C	[-c command] [-eI] [terminal_type]
%C	-s [terminal_type]
%C	- [terminal_type]
Options:
 -                  : report terminal type to stdout. Do not exec into login.
 -c command         : command to start (default is "/bin/login -p")
 -e                 : use the terminal type currently defined by the
                      TERM environment variable.
 -I                 : don't output terminal initialization strings
 -s                 : output shell commands for setting $TERM to stdout,
                      do not exec into login (or other command via -c).
Where:
 terminal_type      is the name of a terminal type that may be supplied
                    on the command line. When this is supplied a terminal
                    type will not be prompted for and -e is overridden.
Note:
 When the terminal type obtained via -e or a terminal_type specified on
 the command line is invalid, a warning message will be written to standard
 error and the terminal type will be prompted for.
%-rtermdef
%C - display or set the terminal type (QNX)

%C	[-eI] [terminal_type]
%C	-s [terminal_type]
%C	- [terminal_type]
Options:
 -                  : report terminal type to stdout. Do not exec into login.
 -e                 : use the terminal type currently defined by the
                      TERM environment variable.
 -I                 : don't output terminal initialization strings
 -s                 : output shell commands for setting $TERM to stdout,
                      do not exec into login (or other command via -c).
Where:
 terminal_type      is the name of a terminal type that may be supplied
                    on the command line. When this is supplied a terminal
                    type will not be prompted for and -e is overridden.
Note:
 When the terminal type obtained via -e or a terminal_type specified on
 the command line is invalid, a warning message will be written to standard
 error and the terminal type will be prompted for.
#endif
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <process.h>
#include <termios.h>
#include <sys/term.h>
#include <sys/types.h>
#include <fcntl.h>
#ifdef __QNXNTO__
#else
    #include <env.h>
#endif

#define FALSE (1==0)
#define TRUE (1==1)

#define	TERMINFO_DIR	"/usr/lib/terminfo"
#define	TERMINFO		"TERMINFO"
#define	TERM			"TERM"
#define	NUM_NAMES		4

char	name[PATH_MAX];
char	dir[PATH_MAX];
char	tmp[PATH_MAX];
char	logincmd[] = "/bin/login -p";
char	*cmd  = logincmd;		/* Default login command			*/
int		count = 0;
int		flag  = 1;
int		shell_output = FALSE;
int		noexec = FALSE;
int		noinit = FALSE;
FILE    *interactive_out;

int run_cmd(char *sh);
void print_list(void);
void process_dir( char *dirname );
void process_name(char *name );

#ifdef RTERMDEF
#define OPTIONS "esI"
#else
#define OPTIONS "c:esI"
#endif

int
main(int argc, char **argv)
{
	char	*termtype=NULL;
	char	*defaulttermtype=NULL;
	char	*term_path;
	int		c, i, numoperands, display_result = FALSE;

	tcflush(0,TCIFLUSH);

	defaulttermtype=getenv("TERM");

	while ((c=getopt(argc,argv,OPTIONS)) != -1) {
		switch (c) {
#ifndef RTERMDEF
		case	'c': cmd = optarg;				break;
#endif
		case 	'e': termtype=defaulttermtype;	break;
		case 	'I': noinit=TRUE;				break;
		case 	's': shell_output=noexec=TRUE;	break;
		case	'?':
		default    : exit(EXIT_FAILURE); break;
		}
	}

	interactive_out=(shell_output?stderr:stdout);
	setvbuf(interactive_out,NULL,_IOLBF,80);

	numoperands=argc-optind;

	if ((numoperands>=1) && (!strcmp(argv[optind],"-") ) ) {
		display_result=TRUE;
		numoperands--;
		optind++;
		noexec=TRUE;
	}

	if (numoperands) termtype=argv[optind];

	if (numoperands>1) 
		fprintf(stderr,"termdef: Warning - >1 terminal specified on cmd line. Setting TERM='%s'.\n",termtype);

	if ((termtype==NULL)&&(noexec)&&(!shell_output)) {
		/* user specified -, did not specify a terminal type */
		termtype=defaulttermtype;
	}

	strcpy( dir, TERMINFO_DIR );

	term_path=getenv(TERMINFO);
	if ( term_path != NULL )
		strncpy( dir, term_path, sizeof(dir) - 1); 
		
	/* put tty into editted mode */
	{
		struct termios tios;
		tcgetattr(0,&tios);
		tios.c_iflag|=BRKINT|ICRNL|IXON;
		tios.c_oflag|=OPOST;
		tios.c_cflag|=CREAD;
		tios.c_lflag|=ECHO|ECHOE|ECHOK|ISIG|ICANON;
		tcsetattr(0,TCSANOW,&tios);
	}

	if (termtype!=NULL) {
		strncpy(tmp,dir, sizeof(tmp) - 4);
		strcat(tmp, "/");
		strncat(tmp, termtype, 1);
		strcat(tmp, "/");
		strncat(tmp, termtype, sizeof(tmp) - strlen(tmp) - 1);
		if (access(tmp,R_OK)==-1) {
			if (numoperands) {
				fprintf(stderr,"termdef: Warning - terminal type set on command line (%s) is unknown.\n",termtype);
			} else {
				fprintf(stderr,"termdef: Warning - terminal type previously set (TERM=%s) is unknown.\n",termtype);
			}

			termtype=NULL;
		} else strncpy(name,termtype, sizeof(name) - 1);
	}
			
	if (termtype==NULL) {
		while( flag ) {
			strncpy(tmp, dir, sizeof(tmp) - 4);
			fprintf(interactive_out,"Enter terminal type : ");
			fflush(interactive_out);			
			if ( (gets(name) != NULL) && (*name) ) {
				strcat(tmp, "/");
				strncat(tmp, name, 1);
				strcat(tmp, "/");
				strncat(tmp, name, sizeof(tmp) - strlen(tmp) - 1);
			}
			if ( (*name == NULL) || (access( tmp, R_OK ) == -1) ) 
				print_list();
			else
				flag = 0;
		}
	}

#ifdef DIAG
   fprintf(stderr,"setting TERM to %s\n", name ); 
#endif

	if ( setenv( TERM, name, 1 ) != 0 ) {
		perror("termdef");
	}

	/* this stuff has to occur before sending init sequences, since
       we have to (potentially) change where stdout goes to to send
       the init sequence */
	if (shell_output) {	/* output shell sequence */
		#ifdef DIAG
			fprintf(stderr,"Writing shell command sequence\n");
		#endif
		fprintf(stdout,"export TERM=%s\n",name);
	}

	if (display_result) fprintf(stdout,"%s\n",name);
	
	fflush(stdout);
	fflush(stderr);

#ifdef DIAG
	fprintf(stderr,"check that stdout is a tty\n");
#endif
	/* make sure stdout is the tty being run on, or at least a tty */
	if (!isatty(STDOUT_FILENO)) {
		int outfd;
#ifdef DIAG
		fprintf(stderr,"use /dev/tty for init sequences\n");
#endif
		/* if stdout is not a tty, use /dev/tty for init sequences etc */
		outfd = open("/dev/tty",O_WRONLY);
		dup2(outfd,STDOUT_FILENO);
	}

#ifdef DIAG
	fprintf(stderr,"Calling __setupterm\n");
#endif
    __setupterm(0,1,&i);
#ifdef DIAG
	fprintf(stderr,"returned from __setupterm\n");
#endif

	setvbuf(stdout,NULL,_IOLBF,512);
	if (i!=1) {
		fprintf(stderr,"termdef: warning - can't get terminal info!\n");
	} else {
		struct termios tios;

#ifdef DIAG
	fprintf(stderr,"sending init sequence\n");
#endif

		/* send init string, put keyboard into appropriate mode */
		if (!noinit) {
			__putp(init_1string);
			__putp(init_2string);
			__putp(keypad_xmit);
		}

		fflush(stdout);
		if (tcgetattr(STDOUT_FILENO,&tios)!=-1) {
			#define NKEYS 8
			char *key[NKEYS];
			int  key_idx[NKEYS] = {VLEFT,VRIGHT,VUP,VDOWN,VINS,VDEL,VHOME,VEND};
			int prefix, suffix;

			key[0] = key_left;
			key[1] = key_right;
			key[2] = key_up;
			key[3] = key_down;
			key[4] = key_ic;
			key[5] = key_dc;
			key[6] = key_home;
			key[7] = key_end;

			for (prefix = 0; prefix < 4; ++prefix) {
				if (key[0][prefix] == 0) break;

				for (i = 0; i < NKEYS; ++i) {
					if(key[i][0] == 0) continue;
					if(key[i][prefix] != key[0][prefix]) break;
				}				
				if(i<NKEYS) break;
			}

			for(suffix = 0; suffix < 4; ++suffix) {
				if(key[0][prefix+1+suffix] == 0) break;
				for(i = 0; i < NKEYS; ++i) {
					if(key[i][0] == 0) continue;
					if(key[i][prefix+1+suffix] != key[0][prefix+1+suffix])
						break;
				}				
				if (i<NKEYS) break;
			}

			for(i = 0; i < 4; ++i) {
				tios.c_cc[VPREFIX+i] = 0;
				tios.c_cc[VSUFFIX+i] = 0;
			}

			for(i=0;i<prefix;++i)	tios.c_cc[VPREFIX+i] = key[0][i];					
			for(i=0;i<NKEYS;++i)	tios.c_cc[key_idx[i]] = key[i][prefix];
			for(i=0;i<suffix;++i)	tios.c_cc[VSUFFIX+i] = key[0][prefix+1+i];

			tios.c_cc[VERASE]	=	0x7f;		/* rubout */
			tios.c_cc[VINTR]	=	0x03;		/* ctrl-c stuff */
			tios.c_cc[VKILL]	=	0x15;		/* ctrl-u */
			tios.c_cc[VQUIT]	=	0x1c;		/* ctrl-\ */
			if (strlen(key_eol)==1) tios.c_cc[VEOL] = *key_eol;
			tios.c_cc[VEOF]		=   0x04;		/* ctrl-d end of file */

			if (xon_xoff) {
				if (*xoff_character) tios.c_cc[VSTOP] = *xoff_character;
				else tios.c_cc[VSTOP] = 0x13;	/* ctrl-s */
				if (*xon_character) tios.c_cc[VSTART] = *xon_character;
				else tios.c_cc[VSTART]= 0x11;	/* ctrl-q */
				tios.c_iflag |= IXON;			/* enable output flow control */
				tios.c_iflag |= IXOFF;			/* enable input flow control */
			}
				
			if (tcsetattr(STDOUT_FILENO,TCSANOW,&tios)==-1)
				perror("termdef: unable to set terminal control settings");
		} else perror("termdef: unable to get/set terminal control settings");
	}
		
#ifndef RTERMDEF
	if (!noexec)	run_cmd(cmd);
#endif

	return (EXIT_SUCCESS);
}

void print_list(void)
{
	DIR	*dirp;
	struct	dirent	*dp;

	if ((dirp=opendir( dir )) == NULL ) {
		perror("termdef");
		return;
	}
	
	do {
		dp = readdir ( dirp );
		if ( dp != NULL )
			process_name( &dp->d_name[0] );
	} while ( dp != NULL );
	closedir( dirp );
	if ( count ) {
		fprintf(interactive_out,"\n\n");
		count = 0;
	}
}

void process_name(char *name )
{
	char			buf[PATH_MAX];

	if ( (strcmp(name, ".") == 0) || ( strcmp(name, "..") == 0) )
		return;

	strcpy( buf, dir );
	strcat( buf, "/" );
	strcat( buf, name );
	process_dir(buf);					
}


void process_dir( char *dirname )
{
	DIR	*dirp;
	struct	dirent	*dp;

	if ((dirp=opendir( dirname )) == NULL )	{
		if ( errno == ENOENT )		/* In case this is a file, not a dir. */
			return;
		else {
			perror("termdef");
			return;
		}
	}
	do {
		dp = readdir ( dirp );
		if ( dp != NULL ) {
			if ( (strcmp(&dp->d_name[0], ".") == 0) ||
				 (strcmp(&dp->d_name[0], "..") == 0) )
				continue;
			fprintf(interactive_out,"%-15s\t", &dp->d_name[0] );
			count++;
			if ( count == NUM_NAMES ) {
				fprintf(interactive_out,"\n" );
				count = 0;
			}
		}
	} while ( dp != NULL );
	closedir( dirp );
}

#ifndef RTERMDEF
	int run_cmd(char *sh)
	{
		int	i;
		#define MAX_ARGV_ITEMS (256)
		char	**argv;
		char	*delims= " \t\n";
	
		if (NULL==(argv=calloc(1024,sizeof(char *)) )) {
			fprintf(stderr,"termdef: Sorry, insufficient memory\n");
			return 0;
		}
	
		if (NULL==(argv[0] = strtok(sh,delims))) {
			fprintf(stderr,"termdef: bad command for -c option\n");
			return 0;
		}
	
	
		for (i=1;i<MAX_ARGV_ITEMS && NULL!= (argv[i]=strtok(NULL,delims));i++) {
			;
		}
	
		argv[i] = NULL;
		execve(sh, argv, environ);
	    fprintf(stderr,"termdef: failed to exec (%s)\n",strerror(errno));
	    return 0;
	}
#endif

