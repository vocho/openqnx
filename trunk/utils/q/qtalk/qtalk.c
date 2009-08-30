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
%C	[options]* [system]
Options:
 -b [baud|data|parity|stop][,...]
                         Change serial port to this baud, parity, stop bits,
                         and/or data bits. Values of 1-2 are interpreted as
                         stop bits, 7-8 as data bits, 'none', 'even', 'odd',
                         'mark' and 'space' as parity, all other numbers as
                         baud. Order is not significant e.g. -b 9600,8,n,1
 -c <cmd>                Command char.
 -d <del>                Delete char.
 -D delay                Delay this many 1/20th second periods reading from
                         the port after emitting the dialling string, before
                         running the -x command. If -x is not specified,
                         there is never any delay and you will be immediately
                         given keyboard control after the dialling string is
                         emitted.
 -e                      Local echo on.
 -h                      Hang up when new system is dialled.
 -l <logfile>            Path to log file.
 -m <modem>[,initstring] Device or modem pool to use. More than
                         one may be specified. If an initstring
                         is specified, it will be emitted to the
                         modem before the dial string, if this
                         modem or a modem from this pool is selected.
 -O                      Ignore the open count, and open anyways
 -o protocol=string      Transfer protocol options. Protocol must be one of:
                            qcp_se    : command to run to do a qcp send
                            qcp_re    : command to run to do a qcp receive
                            zmodem_se : command to run to do a zmodem send
                            zmodem_re : command to run to do a zmodem receive
                            other_se  : command to run to do 'other' send
                            other_re  : command to run to do 'other' receive
            			 The string is a command to be run to do the file
                         transfer. The command will be run by the shell, and
                         the macro $MODEM will be set to the pathname of the
                         modem device, and $FILENAME will be set to the
                         filename to be transmitted in the case of a send.
                         Note: automatic invocation of zmodem or qcp receive
                               can be disabled by setting the respective
                               receive protocol to a null string ("").
 -P                      Strip parity.
 -q                      Quiet. When qtalk goes into command mode, -q
                         suppresses the banner and displays only a minimal
                         prompt instead.
 -s system_directory     Check systems in this dialling directory first before
                         falling back to $HOME/.qtalk and /etc/config/qtalk.
 -t xfer_protocol        Current transfer protocol. May be 'qcp', 'zmodem' or
                         'other'. (Use -o to change the command that will be
                         run to perform the file transfer.)
 -T terminal_type        Terminal emulation to be used for screen output.
                         Will only function when running qtalk on a QNX
                         Dev.con or Dev.ansi device. Terminal type must be
                         'qnx', 'qnxs', 'ansi' or 'pcansi'. 'qnxs' differs from
                         'qnx' in that it will not send leading FFs for
                         function keys. 'pcansi' differs from 'ansi' in that
                         the upper part of the character set will be set to
                         IBM PC characters vs ISO Latin-1. This is done by
                         sending an escape sequence when the terminal type is
                         selected.
 -x "command"            Run this command after emitting the dialling string
                         for the system named. The command will be run through
                         a shell, and the environment variable $MODEM will
                         be set to the pathname of the device being used.
Where:
 system          Is the name of the system you want qtalk to call.
                 (systems are defined in $HOME/.qtalk or /etc/config/qtalk)
Note:
 For HELP, press Ctrl-A followed by ? within Qtalk.

 -m <modem> may be either an actual pathname of a device, or it may be the
 name of a modem pool. Modem pools are defined in the file
 /etc/config/qtalk.modems.
#endif

#ifdef __USAGENTO
%C	[options]* [system]
Options:
 -b [baud|data|parity|stop][,...]
                         Change serial port to this baud, parity, stop bits,
                         and/or data bits. Values of 1-2 are interpreted as
                         stop bits, 7-8 as data bits, 'none', 'even', 'odd',
                         'mark' and 'space' as parity, all other numbers as
                         baud. Order is not significant e.g. -b 9600,8,n,1
 -c <cmd>                Command char.
 -d <del>                Delete char.
 -D delay                Delay this many 1/20th second periods reading from
                         the port after emitting the dialling string, before
                         running the -x command. If -x is not specified,
                         there is never any delay and you will be immediately
                         given keyboard control after the dialling string is
                         emitted.
 -e                      Local echo on.
 -h                      Hang up when new system is dialled.
 -l <logfile>            Path to log file.
 -m <modem>[,initstring] Device or modem pool to use. More than
                         one may be specified. If an initstring
                         is specified, it will be emitted to the
                         modem before the dial string, if this
                         modem or a modem from this pool is selected.
 -O                      Ignore the open count, and open anyways
 -o protocol=string      Transfer protocol options. Protocol must be one of:
                            qcp_se    : command to run to do a qcp send
                            qcp_re    : command to run to do a qcp receive
                            zmodem_se : command to run to do a zmodem send
                            zmodem_re : command to run to do a zmodem receive
                            other_se  : command to run to do 'other' send
                            other_re  : command to run to do 'other' receive
            			 The string is a command to be run to do the file
                         transfer. The command will be run by the shell, and
                         the macro $MODEM will be set to the pathname of the
                         modem device, and $FILENAME will be set to the
                         filename to be transmitted in the case of a send.
                         Note: automatic invocation of zmodem or qcp receive
                               can be disabled by setting the respective
                               receive protocol to a null string ("").
 -P                      Strip parity.
 -q                      Quiet. When qtalk goes into command mode, -q
                         suppresses the banner and displays only a minimal
                         prompt instead.
 -s system_directory     Check systems in this dialling directory first before
                         falling back to $HOME/.qtalk and /etc/config/qtalk.
 -t xfer_protocol        Current transfer protocol. May be 'qcp', 'zmodem' or
                         'other'. (Use -o to change the command that will be
                         run to perform the file transfer.)
 -x "command"            Run this command after emitting the dialling string
                         for the system named. The command will be run through
                         a shell, and the environment variable $MODEM will
                         be set to the pathname of the device being used.
Where:
 system          Is the name of the system you want qtalk to call.
                 (systems are defined in $HOME/.qtalk or /etc/config/qtalk)
Note:
 For HELP, press Ctrl-A followed by ? within Qtalk.

 -m <modem> may be either an actual pathname of a device, or it may be the
 name of a modem pool. Modem pools are defined in the file
 /etc/config/qtalk.modems.
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#ifndef __QNXNTO__
#include <sys/dev.h>
#include <sys/kernel.h>
#include <sys/proxy.h>
#include <sys/console.h>
#include <process.h>
#include <env.h>
#else
#include <sys/neutrino.h>
#include <sys/select.h>
//#include <i86.h>
#include <util/qnx4dev.h>
#endif

#include <sys/wait.h>
#include <termios.h>
#include <malloc.h>
#include <signal.h>
#include <string.h>
#include <setjmp.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

#ifdef __QNXNTO__
/* For "devctl()" */
#include <devctl.h>
#include <sys/dcmd_chr.h>
#endif


/* =============================== CONFIGURABLE MANIFESTS FOR COMPILATION */

#ifndef __QNXNTO__
#define TERM_TYPE_SUPPORTED
#endif

#ifdef TERM_TYPE_SUPPORTED
#define TERM_OPTION "ET:"
#else
#define TERM_OPTION
#endif


#define FORCE_ALLOWED

#ifdef FORCE_ALLOWED
#define FORCE_ALLOWED_OPTION "F"
#else
#define FORCE_ALLOWED_OPTION
#endif

/* ============================== END OF CONFIGURATION */


void mdm_write( int upload_flag );
int a_dev_ischars(int fd);

unsigned a_dev_mode(int fd, unsigned mode, unsigned mask);

#define old_mode()	{a_dev_mode(kbd, okbd_mode, _DEV_MODES);\
					 a_dev_mode(mdm, omdm_mode, _DEV_MODES);}

#define new_mode()	{a_dev_mode(kbd,  kbd_mode, _DEV_MODES);\
					 a_dev_mode(mdm,  mdm_mode, _DEV_MODES);}

#define VERSION				"[4.81]"
#define MDM_VERSION			"q10"
#define BUFFER_SIZE			4000
#define DEFAULT_CMD_CHAR	0x01
#define ENQ					0x05
#define SYN					0x16
#define ESC					0x1B
#define RUB					0x7F

#define MODEM_POOL_DIR		"/etc/config/qtalk.modems"
#define DIAL_DIR			"/etc/config/qtalk"
#define OLD_DIAL_DIR		"/etc/config/dial.dir"

#define LINE_LENGTH				1024
#define OPT_LENGTH				LINE_LENGTH
#define CMDLINE_LENGTH			LINE_LENGTH
#define TOKEN_MAXLEN			LINE_LENGTH
#define MAX_POOLS				128					/* max # of separate -m options allowed */
#define SIMULATED_ARGV_MAXITEMS 64					/* max # cmd line options which may appear
                                                       in a dialling entry */

#ifdef TERM_TYPE_SUPPORTED
#define INIT_PCANSI "\033*U\033>\033[?1l"	/* ansi - PC charset, ansi keypad, ansi cursor pad */
#define INIT_LATIN  "\033*A\033>\033[?1l"   /* ansi - latin chars, ansi keypad, ansi cursor pad */
#endif

#define PARITY_NOT_SET   (0)
#define NO_PARITY		 (1)
#define EVEN_PARITY		 (2)
#define ODD_PARITY		 (3)
#define MARK_PARITY		 (4)
#define SPACE_PARITY	 (5)

struct envstruct {
	char		systemname[LINE_LENGTH+1];		/* name of system */
	char		dialstring[LINE_LENGTH];		/* dial string */
	char		execute_cmd[CMDLINE_LENGTH];	/* command to execute after dialling */
	int			execute_delay;					/* how long to pause collecting chars before
                                    	    	   running the execute command */
	char		*pools[MAX_POOLS];				/* modem pool/device table */
	char		device[PATH_MAX];		/* device being used */
    char		initstring[LINE_LENGTH];		/* init string to emit to modem being used */

	long		baud;							/* try to change baud of port to this */
	char		parity;							/* try to change parity to this */
	char		databits;						/* try to change databits to this */
	char		stopbits;						/* try to change stopbits to this */

#ifdef TERM_TYPE_SUPPORTED
	int			terminal_type;					/* QNX_TERM or ANSI_TERM */
#endif
	char		cmd_char;
	char		del_char;
	char		pause_char;
	char		kick_char;

#define PROTOCOL_QCP 0
#define PROTOCOL_ZMODEM 1
#define PROTOCOL_OTHER 2
#define RX_PROTOCOL 0
#define TX_PROTOCOL 1

#define DEFAULT_QCP_RE_PROTOCOL "qcp $MODEM re"
#define DEFAULT_QCP_SE_PROTOCOL "qcp $MODEM se $FILENAME"
#define DEFAULT_ZMODEM_RE_PROTOCOL "rz <$MODEM >$MODEM"
#define DEFAULT_ZMODEM_SE_PROTOCOL "sz $FILENAME <$MODEM >$MODEM"
#define DEFAULT_OTHER_RE_PROTOCOL  ""
#define DEFAULT_OTHER_SE_PROTOCOL  ""

	char		protocol_strings[3][2][LINE_LENGTH+1]; /* index 0=qcp,1=zmodem,2=other;0=re,1=se;char array */
	int			protocol;		/* one of PROTOCOL_QCP, PROTOCOL_ZMODEM, PROTOCOL_OTHER */

	char		transfer_options[ OPT_LENGTH + 1 ];
	int			local_echo;
	int			strip_parity;
#ifdef TERM_TYPE_SUPPORTED
	int			qnxs;
	int 		pc;
#endif
	int			hangup_on_switch;
	int			quiet;

#ifdef FORCE_ALLOWED	
	unsigned			force;
#endif
};

struct envstruct start_env;		/* starting cmd line specs which will be used as a baseline
                                   for other information in dialling entries. This allows one
                                   to override the defaults */
struct envstruct current_env;	/* current cmd line specs & qtalk environment, as spec'd by
                                   a dialling entry. Will start off the same as start_env
                                   if no system is dialled */

/* modem pool vars */
#define MAX_LEVELS			(4)
#define NAMEBUFFER_LENGTH   (PATH_MAX + LINE_LENGTH + 1)
char		namebuffer[MAX_LEVELS][NAMEBUFFER_LENGTH];
char		*nameptr[MAX_LEVELS];
FILE 		*poolfile[MAX_LEVELS];
char		*working_on[MAX_LEVELS];
int			level=0;

/* general vars */
char					buf[ BUFFER_SIZE ];
char					buf2[ BUFFER_SIZE ];
char					line[ LINE_LENGTH + 1 ];

int						mdm;
int						scr;
int						kbd;
int						log_file = -1;
int						mdmflag, kbdflag;
pid_t					pid;
unsigned				mproxy = 0, kproxy = 1, vmproxy = 0, vkproxy = 1;

int						diagflag=0;

#define	diag	if (diagflag)
//#define	diag	if (1)

sigjmp_buf				jmpbuf;
unsigned				kbd_mode, mdm_mode, okbd_mode, omdm_mode;
#ifndef __QNXNTO__
struct	_dev_info_entry	kbd_entry, mdm_entry, newmdm_entry;
#endif
long					omdm_ispeed, omdm_ospeed;
tcflag_t				ocflag;
struct termios			mdm_tios;
int						old_terminal_type=-1;
int						open_override = 0;
timer_t					global_timerid=-1;

char		*dialling_files[] = {NULL,DIAL_DIR,OLD_DIAL_DIR,NULL};
char	    personal_dialdir[PATH_MAX];


#ifdef __QNXNTO__
#include <unistd.h>
#include <termios.h>
#if  _NTO_VERSION < 100
#include <sys/dev.h>
#endif
#endif


int put_console( char *buf, int n );
void sighandler(int sig_no);
int put_logfile( char *buf, int n );
int parse_options(int argc, char **argv, struct envstruct *env);
void command(void);
void dup_env(struct envstruct *to, struct envstruct *from);

/*-------------------------------------------------- IO COVER FNS
        They abort when they get 'significant' errors.
*/

void a_abrt(int err, char *fn, int fd)
{
	fprintf(stderr,"\r\nqtalk: (%s) %s\r\n",fn,strerror(err));
	if (mdm!=-1 && fd!=mdm) tcdropline(mdm,500);
    if (kbd!=-1 && fd!=kbd) dev_mode(kbd, okbd_mode, _DEV_MODES);
#ifdef TERM_TYPE_SUPPORTED
	if (old_terminal_type!=-1 && old_terminal_type!=current_env.terminal_type) {
		struct _console_ctrl *cc;		
		
#ifdef TERM_TYPE_DEBUG
	{
		char *op, *np;

		if (old_terminal_type==_CON_PROT_QNX4) op="qnx";
		else if (old_terminal_type==_CON_PROT_ANSI) op="ansi";
		else op="unknown";

		if (current_env.terminal_type==_CON_PROT_QNX4) np="qnx";
		else if (current_env.terminal_type==_CON_PROT_ANSI) np="ansi";
		else np="unknown";

		fprintf(stderr,"abort: old terminal type %d (%s) != current terminal type %d (%s)\r\n",
				old_terminal_type, op, current_env.terminal_type, np);
		fprintf(stderr,"abort: resetting console to old type\n");

	}
#endif

		if ((cc=console_open(kbd,O_RDWR))!=NULL) {
			console_protocol(cc, 0, old_terminal_type);
			if (old_terminal_type==_CON_PROT_ANSI) {
				fflush(stdout); fflush(stderr);
				a_write(kbd,INIT_LATIN,sizeof(INIT_LATIN)-1);
			}
			console_close(cc);
		}
	}		
#endif

	if (mdm!=-1 && fd!=mdm) {
		dev_mode(mdm, omdm_mode, _DEV_MODES);

		/* restore old baud if we have changed it */
		if (current_env.baud||current_env.parity||current_env.stopbits||current_env.databits) {
			if (tcgetattr(mdm, &mdm_tios)!=-1) {
				if (current_env.baud) {
					mdm_tios.c_ospeed	=	omdm_ispeed;
					mdm_tios.c_ispeed	=	omdm_ospeed;
				}

				if (current_env.parity||current_env.stopbits||current_env.databits) {
					mdm_tios.c_cflag = ocflag;
				}

				tcsetattr(mdm,TCSANOW,&mdm_tios);
			}
		}
	}

	if (err==ESRCH || err==EBADF) {
		if (fd==mdm) fprintf(stderr,"qtalk: May have lost network VC to modem device.\r\n");
	}

	exit (EXIT_FAILURE);
}

void a_exit(int rc)
{
    if (kbd!=-1) dev_mode(kbd, okbd_mode, _DEV_MODES);
	if (mdm!=-1) {
		dev_mode(mdm, omdm_mode, _DEV_MODES);

#ifdef TERM_TYPE_SUPPORTED
		if (old_terminal_type!=-1 && old_terminal_type!=current_env.terminal_type) {
			struct _console_ctrl *cc;		

#ifdef TERM_TYPE_DEBUG
	{
		char *op, *np;

		if (old_terminal_type==_CON_PROT_QNX4) op="qnx";
		else if (old_terminal_type==_CON_PROT_ANSI) op="ansi";
		else op="unknown";

		if (current_env.terminal_type==_CON_PROT_QNX4) np="qnx";
		else if (current_env.terminal_type==_CON_PROT_ANSI) np="ansi";
		else np="unknown";

		fprintf(stderr,"exit: restoring old terminal type %d (%s) [!= current terminal type %d (%s)]\r\n",
				old_terminal_type, op, current_env.terminal_type, np);

	}
#endif
			
			if ((cc=console_open(kbd,O_RDWR))!=NULL) {
				console_protocol(cc, 0, old_terminal_type);
				if (old_terminal_type==_CON_PROT_ANSI) {
					fflush(stdout); fflush(stderr);
					a_write(kbd,INIT_LATIN,sizeof(INIT_LATIN)-1);
				}
				console_close(cc);
			}
		}		
#endif

		/* restore old baud if we have changed it */
		if (current_env.baud||current_env.parity||current_env.stopbits||current_env.databits) {
			if (tcgetattr(mdm, &mdm_tios)!=-1) {
				if (current_env.baud) {
					mdm_tios.c_ospeed	=	omdm_ispeed;
					mdm_tios.c_ispeed	=	omdm_ospeed;
				}

				if (current_env.parity||current_env.stopbits||current_env.databits) {
					mdm_tios.c_cflag = ocflag;
				}

				tcsetattr(mdm,TCSANOW,&mdm_tios);
			}
		}

	}

	exit (rc);
}

ssize_t a_read(int fildes, void *buf, size_t nbyte)
{
	ssize_t rc;	

	if ((rc = read(fildes,buf,nbyte))==-1) {
		switch (errno) {
			case EAGAIN:
			case EINTR:
			case EIO:		/* Old MacDonald had a farm, EI EIO... */
				break;
			default:		/* Old MacDonald bought the farm... */
				a_abrt(errno,"read",fildes);
		}
	}	
	return rc;
}

ssize_t a_write(int fildes, const void *buf, size_t nbyte)
{
	ssize_t rc;

	if ((rc = write(fildes,buf,nbyte))==-1) {
		switch (errno) {
			case EAGAIN:
			case EINTR:
			case EIO:
				rc=-1;
				break;
			default:
				a_abrt(errno,"write",fildes);
		}
	}

	return rc;
}

void emit_string ( int fd, char *string) 
{
#ifdef __QNXNTO__
/* zzx fcntl to block for IO */
#endif

	for (;*string;string++) {
		switch(*string) {
			case '\\':
				{
					int n, digits;

					if (*(string+1)=='x' || (*(string+1)=='0' && *(string+2)=='x') ) {
						if (*(string+1)=='0') string++;
						string++;
						/* hex */
						for (n=0,digits=0;
                             digits<2 &&
                             ( *(string+1)<'8' && *(string+1)>='0') ||
                             ( *(string+1)<='f' && *(string+1)>='a') ;
                             digits++)
                        {
							n=n*16;
							string++;
							if (*string>='a') n+=10+(*string-'a');
							else n+=*string-'0';
						}

						if (n) a_write( fd, &n, 1);
					} else if (*(string+1)<'8' && *(string+1)>='0') {
						/* octal */
		
						for (n=0,digits=0;digits<3 && *(string+1)<'8' && *(string+1)>='0';digits++) {
							n=n*8;
							n+=(*(string+1)-'0');
							string++;
						}

						if (n) a_write( fd, &n, 1);
					} else a_write( fd, string, 1);
				}
				break;
			case '|':	a_write( fd, "\r", 1 );		break;
			case '~':	delay(1000);			break;
			case '\'':	delay(100);			break;
			case '^':   tcdropline(fd,1000); 		break;
			case '!':	tcsendbreak(fd, 500);		break;
			default: 	a_write( fd, string, 1 );	break;
		}
	}
	a_write(fd,"\r",1);
#ifdef __QNXNTO__
/* zzx fcntl back to nonblk for IO */
#endif
}

int a_dev_ischars(int fd)
{
	int rc;

#ifdef __QNXNTO__
	if ((rc = tcischars(fd))==-1)
#else
	if ((rc = dev_ischars(fd))==-1)
#endif
		a_abrt(errno,"dev_ischars",fd);
	return rc;                      
}


#ifndef __QNXNTO__
int a_dev_arm(int fd, pid_t proxy, unsigned events)
{
	int rc;

	if ((rc = dev_arm(fd,proxy,events))==-1) 
		a_abrt(errno,"dev_arm",fd);

	return rc;
}
#endif


unsigned a_dev_mode(int fd, unsigned mode, unsigned mask)
{
	unsigned rc;

	if ((rc = dev_mode(fd,mode,mask))==-1) 
		a_abrt(errno,"dev_mode",fd);
	return rc;
}

#ifndef __QNXNTO__
int a_dev_read(int fd, void *buf, unsigned n, unsigned min, unsigned time,
               unsigned timeout, pid_t proxy, int *armed)
{
	int rc;

	if ((rc = dev_read(fd,buf,n,min,time,timeout,proxy,armed))==-1) {
		switch (rc) {
			case EINTR:
			case EIO:
				break;
			default:
				a_abrt(errno,"dev_read",fd);
		}
	}
	return rc;
}
#endif

/* 
	This function should have a timeout around the tcdrain() call..
	if we call tcdrain() and we're paged we could get stuck here forever
*/
int a_tcdrain(int fildes)				
{
	int rc;

	if ((rc = tcdrain(fildes))==-1) {
		switch(rc) {
			case EINTR:
				break;
			default:
				a_abrt(errno,"tcdrain",fildes);
		}
	}
	return rc;
}

/* end of IO cover fns ------------------------------------------------ */				

/* Modem pool functions - set_pool(name) to initialize, get_poolmember(env)
   to return device names one at a time to try. Names returned in
   env->device with any initstring in env->initstring */


char *skip_white(char *pointer)
{
	while(isspace(*pointer) && (*pointer)) pointer++;
	return pointer;
}	

char *skip_alpha(char *pointer)
{
	int inside_quote = 0, inside_dblquote = 0;

	while (*pointer) {
		if (inside_quote) {
			if (*pointer == '\'') inside_quote = 0;
        	if (*pointer == '\\') pointer++;
			if (*pointer) pointer++;
			continue;
		}
		if (inside_dblquote) {
			if (*pointer == '"') inside_dblquote = 0;
        	if (*pointer == '\\') pointer++;
			if (*pointer) pointer++;
			continue;
		}
		
		switch (*pointer) {
			case '\r':
			case '\t':
			case '\n':
			case ' ':
				return pointer;

			case '"':	inside_dblquote = 1; break;
			case '\'':	inside_quote = 1; break;
		}

		pointer++;
	}

	return pointer;
}	

void clean_quotes(char *pointer)
{
	int inside_quote = 0, inside_dblquote = 0;
    char *r, *w;

	if (pointer==NULL || *pointer!=',') return;
	
	r=w=pointer;
	diag fprintf(stderr,"DIAG: cleaning quotes from '%s'\r\n",r); fflush(stderr);

	while (*r) {
		if (*r=='\\' && !inside_quote) {
			/* if not inside single quotes, allow \octal */
			if (*(r+1)<'8' && *(r+1)>='0') {
				int n, digits;

				r++;
				for (n=0,digits=0;digits<3 && *r<'8' && *r>='0';digits++) {
					n=n*8;
					n+=(*r-'0');
					r++;
				}
				if (n) {
					*w = n;
					w++;
				}
				continue;
			} 
		}
		if (*r=='\\' && !inside_quote) {
			/* if not inside single quotes, allow \octal */
			if (*(r+1)<'8' && *(r+1)>='0') {
				int n, digits;

				r++;
				for (n=0,digits=0;digits<3 && *r<'8' && *r>='0';digits++) {
					n=n*8;
					n+=(*r-'0');
					r++;
				}
				if (n) {
					*w = n;
					w++;
				}
				continue;
			} 
		}
						
		if (inside_quote) {
			if (*r == '\'') {
				inside_quote = 0;
				r++;
				continue;
			}

			*w = *r;
			w++;
			if (*r) r++;
			continue;
		}

		if (inside_dblquote) {
			if (*r == '"') {
				inside_dblquote = 0;
				r++;
				continue;
			}

        	if (*r == '\\') r++;
			*w = *r;
			w++;
			if (*r) r++;
			continue;
		}
		
		switch (*r) {
			case '"':	inside_dblquote = 1; break;
			case '\'':	inside_quote = 1; break;
			default: *w = *r; w++; break;
		}

		r++;
	}

	*w=0;

	diag	fprintf(stderr,"DIAG: cleaned verssion = '%s'\r\n",pointer); fflush(stderr);
}	

int name_match(char *string,char *name)
{
    while(!(isspace(*string)) && *string!=',' && (*string) && (*name) && (*name!=',') && (*string==*name)) {
		string++;
		name++;
	}

	/* if we hit end of name, we succeeded */
	if ((isspace(*string)||*string==','||(!string[0])) && (!name[0] || *name ==',')) return 1;
	else return 0;
}

	
int set_pool(char *buffer)
{
	int	name_found;
	char	*temp = NULL;

	if (level==MAX_LEVELS) {
		fprintf(stderr,"qtalk: modem pool contains entry too many levels deep\r\n");
		fprintf(stderr,"qtalk: %s<<",buffer);
		return -1;
	}

	working_on[level] = buffer;
	
	if (strchr(buffer,'/')) {
		/* a single device name, has to be, don't bother looking it up. Note that this may
           contain an init string - will be dealt with later */
		strcpy(namebuffer[level],buffer);
		nameptr[level] = working_on[level] = namebuffer[level];
		poolfile[level] = NULL;		/* flag which means this is not from the pool file */
		level++;
		return 0;
	}

	if ((poolfile[level]=fopen(MODEM_POOL_DIR,"r"))==NULL) {
		fprintf(stderr,"qtalk: Can't open modem pool file '%s'\r\n",MODEM_POOL_DIR);
		return -1;
	}
			
	/* name_found will be 2 when we have found the name and are scanning forward
       past synonymous pool names for the actual pool definition; it will be
       1 when we have the actual definition, 0 if we haven't hit the name at all
       yet */

	name_found = 0;
	
	while (name_found!=1) {
		temp = fgets( namebuffer[level], NAMEBUFFER_LENGTH, poolfile[level] );
		if (temp == NULL) {
			fclose(poolfile[level]);
			break;
		}

		if (name_found==2 || name_match(namebuffer[level],buffer)) {
			if (name_found) {
				if (isspace(namebuffer[level][0])) {
					/* if the line starts with white space, this is the actual definition */
					nameptr[level] = skip_white(namebuffer[level]);
				} else {
					/* otherwise, this is another synonym. Check to see if it has an
                       actual definition sitting past it */
 					nameptr[level] = skip_alpha(nameptr[level]);
					nameptr[level] = skip_white(nameptr[level]);
				}
			} else {
				/* found the match */
				nameptr[level] = skip_alpha(namebuffer[level]);
				nameptr[level] = skip_white(nameptr[level]);
			}

			if (!*(nameptr[level])) {
				/* uh oh - no more items on this line, set a flag and scan forward for
                   the actual definition */
				name_found = 2;
			} else name_found = 1;
		}
    }

	if (temp==NULL) {
		fprintf(stderr,"qtalk: pool '%s' not found in '%s'\r\n",buffer,MODEM_POOL_DIR);
		return -1;
	}

	level++;		/* change level */

	return 0;		/* Success */
}

void close_pool(void)
{

	diag	fprintf(stderr,"DIAG: close pool; working_on = '%s', level %d\r\n",level?working_on[level-1]:"<nothing>", level); fflush(stderr);

	if (level) {
		level--;
	
		if (poolfile[level]!=NULL) {
			fclose(poolfile[level]);
		}
		poolfile[level] = NULL;
		working_on[level] = "";
	}
}

void close_all_pools(void)
{
	diag	fprintf(stderr,"DIAG: closing all open pools\r\n"); fflush(stderr);
	while(level) close_pool();
}


char *_get_poolmember(struct envstruct *env)
{
	char	*mem, *temp;

	diag	fprintf(stderr,"DIAG: _get_poolmember - level %d\r\n",level); fflush(stderr);

	if (!level) return NULL;

	/* if there is an initstring in the namebuffer for this level, isolate it from
       the name */

	if (poolfile[level-1]==NULL) {
		int n;
		char    *w;

    	/* poolfile will only be NULL if this is an actual device i.e. contains / */

		diag fprintf(stderr,"DIAG: _get_poolmember - actual device, nameptr->'%s'\r\n",nameptr[level-1]); fflush(stderr);

		if (!*nameptr[level-1]) return NULL;

		env->initstring[0] = 0;

		/* find last initstring which applies */
		for (n=level-1;n>=0;n--) {
			diag fprintf(stderr,"DIAG: checking for level %d initstring in '%s'\r\n",n+1,working_on[n]); fflush(stderr);

			if ((w=strchr(working_on[n],','))!=NULL) {
				w++;
				strcpy(env->initstring,w);
				diag fprintf(stderr,"DIAG; got initstring = '%s'\r\n",env->initstring); fflush(stderr);
       			break;
			}
		}

		/* if this specific item had an initstring, null terminate the device where the
           comma for the initstring was (we don't want the initstring as part of the
           device name */
		if ((w=strchr(namebuffer[level-1],','))) *w=0;

		strcpy(env->device,namebuffer[level-1]);

		/* make sure nameptr points to null so that this routine returns null on the
           next call */
		nameptr[level-1] = "";

		return env->device;
	}

	if (!(*(nameptr[level-1]))) {
		/* no more names in this line! */
		temp = fgets(namebuffer[level-1],NAMEBUFFER_LENGTH,poolfile[level-1]);
		if (temp == NULL) return NULL;

		/* 1st char of line must be white space, otherwise this is not a continuation of
           the entry */
 		if (!(isspace(namebuffer[level-1][0]))) return NULL;

		/* skip ahead to non-white space */
		nameptr[level-1] = skip_white(namebuffer[level-1]);

		/* if this was a blank line (all white space), then this was not a continuation of
		   the entry */
		if (!(nameptr[level-1][0])) return NULL;		/* this line is blank ! */
	}

	/* nameptr[level-1] now points to the device[,initstring] next in list */

	mem = nameptr[level-1];		/* mem points to start of modem name */

	/* advance nameptr to end of modem name */
	nameptr[level-1] = skip_alpha(nameptr[level-1]);

	/* if the end is not the end of the line, make it null so the modem name
       is null-terminated, and advance the pointer past the null */

	if (nameptr[level-1][0]) {
		nameptr[level-1][0] = (char) 0x00;
		nameptr[level-1]++;
	} 

	/* advance nameptr to the next entry in the line, for the next time this routine is
       called */
 	nameptr[level-1] = skip_white(nameptr[level-1]);

	working_on[level] = mem;

	clean_quotes(strchr(mem,','));	/* will get rid of extra quote chars */
	if (set_pool(mem)==-1) return NULL;

	return _get_poolmember(env);
}

char *get_poolmember(struct envstruct *env)
{
	char *rc;

	diag fprintf(stderr,"DIAG: get_poolmember level = %d\r\n",level); fflush(stderr);

	for (rc=NULL; rc==NULL && level ;) {
		if ((rc=_get_poolmember(env))==NULL)	close_pool();
	}

	return rc;
}


/* GOOBER ------------------------------------------------------------ */

char *get_name( char *prompt )
{
	static char	name[ LINE_LENGTH + 1 ];
	int		i;
	int 		savedflags;

	name[0] = '\0';

#ifdef __QNXNTO__
	/* 
         Turn on O_NONBLOCK while we prompt the user for input,
         this is likely not just a Neutrino requirement.
	*/
    	savedflags = fcntl(fileno(stdin),F_GETFL);
	if((savedflags != -1) && (savedflags & O_NONBLOCK)) {
		if(fcntl(fileno(stdin),F_SETFL, savedflags & ~O_NONBLOCK) == -1) {
			printf("Can't turn off O_NONBLOCK flag\n");
		}
	}
#endif

	old_mode();

	a_write( kbd, prompt, strlen( prompt ) );
	if(fgets( name, LINE_LENGTH, stdin ) == NULL) {
		printf("Error reading input: %s \n", strerror(errno));
	}

	new_mode();

#ifdef __QNXNTO__
    	if(savedflags != -1 && savedflags & O_NONBLOCK) {
		if(fcntl(fileno(stdin),F_SETFL, savedflags) == -1) {
			printf("Can't restore blocking flag\n");
		}
	}
#endif

	i = strlen(name)-1;
	if ( name[i] == '\n' )
		name[i] = '\0';

	return( name );
}

void gotsig( int sig )
{
#ifdef DEBUG
printf("gotsig: Got signal %d\n", sig);
#endif

	if ( sig == SIGINT ) {
		tcsendbreak( mdm, 300 );
		//siglongjmp( &jmpbuf, 0 );
		siglongjmp( jmpbuf, 0 );
	}
	old_mode();
	exit( sig );
}

void mdm_handler( char *bp, int size
#ifndef __QNXNTO__
					,int *armed
#endif
				)
{
	static int			 numsyns, esc_seen;
	static	char		 zmodemsequence[6]={ 0x2a, 0x2a, 0x18, 'B','0','0' };
	static  char		 syns[1] = { SYN };
	static 	int			 numzmhdr=0;
	char				*p;
	int					 n, xfer;
	int					 header_index=0;

	if (bp==NULL) {
		/* reset protocol init sequence stuff! */
		if (numzmhdr) {
			/* this means that we were holding off output of this
               data from a previous call - must write out the 
               difference now since the data is not contained
               in our current chunk of data */
			put_console(zmodemsequence,numzmhdr);
			numzmhdr = 0;
		}
		
		if (numsyns) {	/* numsyns can only be 1 or 0 */
			put_console(syns,1);
			numsyns=0;
		}

		return;
	}

	/*
	 * pre-process the line
	 */
	p = bp;
	xfer = 0;

	for(n = 0; n < size; ++n) {
		/* Respond to zmodem header sequence */

		if (current_env.protocol_strings[PROTOCOL_ZMODEM][RX_PROTOCOL][0] 
			&& *p == zmodemsequence[numzmhdr])
		{
			if (numzmhdr==0) header_index = n;

			numzmhdr++;

			if (numzmhdr==sizeof(zmodemsequence)) {
				/* engage rz */

				xfer++;
				sighandler(SIGUSR1);

#ifdef __QNXNTO__
				/* neutrino - flush any waiting data on the fd (O_NONBLK is set) */
				do {
					n=a_read(mdm,buf,BUFFER_SIZE);
				} while (n!=0 && n!=-1);
#else
				/* turn off modem proxies */
				a_dev_arm(mdm,-1,_DEV_EVENT_INPUT);

				/* flush any pending modem proxies */
				while (Creceive(mproxy,bp,0)!=-1);

				/* read and discard any pending modem data */
				while (n=a_dev_ischars(mdm)) {
					if (n==-1) break;
					/* have BUFFER_SIZE to play with */
					a_read(mdm,bp,n>BUFFER_SIZE?BUFFER_SIZE:n);
				}
#endif
				old_mode();
				system( current_env.protocol_strings[PROTOCOL_ZMODEM][RX_PROTOCOL] );
				new_mode();
				numzmhdr = 0;

#ifndef __QNXNTO__
				/* turn modem proxies back on */
				a_dev_arm(mdm,vmproxy,_DEV_EVENT_INPUT);

				*armed = 0;	/* ready to go back to receive */
#endif

				errno=0;
				break;	/* ignore the rest of the line */
			}
		} else {
			if (numzmhdr>n) {
				/* cancel timer */
				numzmhdr-=n; // will be reset to zero anyway; relates to
                             // commented-out lines below.
				sighandler(SIGUSR1);

				/* this means that we were holding off output of this
                   data from a previous call - must write out the 
                   difference now since the data is not contained
                   in our current chunk of data */
				// sighandler(SIGUSR1) will have reset numzmhdr to 0!!
				// put_console(zmodemsequence,numzmhdr-n);
			}
			numzmhdr = 0;
		}		

		/* Respond to 2 consecutively received SYN characters */
		if (current_env.protocol_strings[PROTOCOL_QCP][RX_PROTOCOL][0] 
			&& *p == SYN)
		{
			if(++numsyns == 2 ) {
				/*	After 2 in a row, invoke QCP. */
		
				xfer++;
#ifdef __QNXNTO__
				/* neutrino - flush any waiting data on the fd (O_NONBLK is set) */
				do {
					n=a_read(mdm,buf,BUFFER_SIZE);
				} while (n!=0 && n!=-1);
#else
				/* turn off modem proxies */
				a_dev_arm(mdm,-1,_DEV_EVENT_INPUT);

				/* flush any pending modem proxies */
				while (Creceive(mproxy,bp,0)!=-1);

				/* read and discard any pending modem data */
				while (n=a_dev_ischars(mdm)) {
					if (n==-1) break;
					/* have BUFFER_SIZE to play with */
					a_read(mdm,bp,n>BUFFER_SIZE?BUFFER_SIZE:n);
				}
#endif
		
				old_mode();
				system( current_env.protocol_strings[PROTOCOL_QCP][RX_PROTOCOL] );
				new_mode();
				numsyns = 0;

#ifndef __QNXNTO__
				/* turn modem proxies back on */
				a_dev_arm(mdm,vmproxy,_DEV_EVENT_INPUT);

				*armed = 0;	/* ready to go back to receive */
#endif

				errno=0;
				break;	/* ignore the rest of the line */
			}
		} else {
			if (numsyns>n) {
				/* cancel timer */
				sighandler(SIGUSR1);
				put_console(syns,1);
			}

			numsyns = 0;	/* should really also write out one SYN */
		}

		/*
		 * Write back ID after <Esc><Enq>
		 */
		if(esc_seen && *p == ENQ) {
			a_write( mdm, MDM_VERSION, 3 );
		}

		if(*p == ESC) esc_seen = 1;
		else esc_seen = 0;

		++p;
	}


	if (!xfer && size) {
		/* numzmhdr!=0 means that we are in the middle of a zmodem header,
           but haven't got the whole thing yet (xfer not set) - so print out
           the data up to the start of the header but don't print any of the
           header. */
		if (numzmhdr) {
			if (header_index) put_console(bp, header_index);

			/* set a timer to signal ourselves (SIGUSR1) in 40ms; deadline
               for receiving the rest of the init sequence */
			if (global_timerid==-1) {
				struct sigevent event;

				event.sigev_signo = SIGUSR1;
#ifdef __QNXNTO__
				event.sigev_notify = SIGEV_SIGNAL;
				timer_create(CLOCK_REALTIME,&event,&global_timerid);
#else
				global_timerid = timer_create(CLOCK_REALTIME,&event);
#endif

				if (global_timerid!=-1) {
					struct itimerspec timer;

					timer.it_value.tv_sec = 0L;
					timer.it_value.tv_nsec = 400000000L;	/* .4 seconds */
					timer.it_interval.tv_sec = 0L;
					timer.it_interval.tv_nsec = 0L;
#ifdef __QNXNTO__
					timer_settime(global_timerid,0,&timer,NULL);
#else
					timer_settime(global_timerid,TIMER_ADDREL,&timer,NULL);
#endif
				}
			} /* otherwise there is already a timer pending */
		} else put_console( bp, size);
	}

}

void sighandler(int sig_no)
{
#ifdef DEBUG
printf("sighandler: Got signal %d\n", sig_no);
#endif
	if (sig_no==SIGUSR1) {
		if (global_timerid!=-1) {
			/* on success, make global_timer be -1 */
			if (timer_delete(global_timerid)!=-1) global_timerid = -1;

			/* reset state of init sequence recognition */
			mdm_handler(NULL,0
#ifndef __QNXNTO__
						,NULL
#endif
						);
		}
	}
}

int put_console( char *buf, int n )
{
	int	 i;
	char *p = buf;

	if (current_env.strip_parity)	for(i=0; i<n; ++p,++i) *p &= 0x7F;

#ifdef TERM_TYPE_SUPPORTED
	if (current_env.qnxs) {
		char *buf2=buf;	/* current print position */
		char *replace_with;
		int  numtoprint=0;

		for (i=0;i<n;++i,++numtoprint) {
			switch(*p) {
				case 0xa6:	replace_with = "c"; break; /* right arrow */
				case 0xa4:	replace_with = "d"; break; /* left arrow */
				case 0xa1:	replace_with = "a"; break; /* up arrow */
				case 0xa9:	replace_with = "b"; break; /* down arrow */
				case 0xa0:	replace_with = "H"; break; /* home key */
				default: replace_with = NULL; break;
			}

			if (replace_with!=NULL) {
				a_write(kbd,buf2,numtoprint);	/* write line to this point */
				if (log_file != -1) put_logfile( buf2, numtoprint );
				buf2+=numtoprint+1;			/* increment 'write from' position */
				a_write(kbd,replace_with,2);	/* write replacement for ctrl sequence */
				if (log_file != -1) put_logfile( replace_with, 2 );
				numtoprint=-1;				/* will be inc'ed to 0 as we loop */
			}
		}
		if (numtoprint) a_write(kbd,buf2,numtoprint);
		if (log_file != -1) put_logfile( buf2, numtoprint );
	} else {
#endif
		a_write( kbd, buf, n );
		if (log_file != -1) put_logfile( buf, n );
#ifdef TERM_TYPE_SUPPORTED
	}
#endif

	return( n );
}

int put_logfile( char *buf, int n )
{
	register char	*p1,
					*p2,
					*endp;
	register int	 count = 0;

	for( p1 = p2 = buf, endp = p1 + n; p1 < endp; ++p1 ) {
		if(*p1 != '\r') {							/* Strip CRs */
			*p2++ = *p1;
			++count;
		}
	}

	write( log_file, buf, count );

	return  n;
}

#ifdef __QNXNTO__
int port_check(int fd)
{
    struct _ttyinfo info;

    devctl(fd, DCMD_CHR_TTYINFO, &info, sizeof(info), NULL);

    return (!open_override && (info.opencount > 1)) ? 1 : 0;
}
#endif


int select_modem(struct envstruct *env)
{
	int newmdm=-1, poolentry;
	struct stat statbuf;
	//dev_t mdm_dev;
	//ino_t mdm_ino;
	dev_t mdm_dev = 0;
	ino_t mdm_ino = 0;
	int err;

	/* select device from the list of devices or modem pools in
	env->pools[] */

	if (mdm!=-1) {
		fstat(mdm,&statbuf);
		mdm_dev = statbuf.st_dev;
		mdm_ino = statbuf.st_ino;
	}

	for (err=0,poolentry=0;newmdm==-1 && poolentry<MAX_POOLS;poolentry++) {
		if (env->pools[poolentry]==NULL) continue;

		diag fprintf(stderr,"DIAG: trying pool %d = '%s'\r\n",poolentry,env->pools[poolentry]); fflush(stderr);

		if (set_pool(env->pools[poolentry])!=-1) {

			diag fprintf(stderr,"DIAG: newmdm=%d\r\n",newmdm); fflush(stderr);

			while (newmdm==-1 && get_poolmember(env)!=NULL) {
				newmdm = open(env->device,O_RDWR);
                if (newmdm==-1) {
    				if (errno==ENOENT) {
						fprintf(stderr,"qtalk: Modem device '%s' does not exist.\r\n",env->device);
						err++;
					}
					continue;
				}

#ifdef __QNXNTO__
                if (port_check(newmdm)) {
                        fprintf(stderr, "qtalk: Modem in use - %s\r\n",env->device );
                        close (newmdm);
                        a_exit(EXIT_FAILURE);
                }
#endif

				/* this paragraph causes us to reuse our current mdm if we hit upon it
                   while scanning the pool */
				if (mdm!=-1 && fstat(newmdm,&statbuf)!=-1) {
					if ((statbuf.st_dev==mdm_dev)&&(statbuf.st_ino==mdm_ino)) {
            			continue;
					}
				}

				fcntl(newmdm,F_SETFD,FD_CLOEXEC);
#ifndef __QNXNTO__
				dev_info( newmdm, &newmdm_entry );

				#ifndef FORCE_ALLOWED
				if(newmdm_entry.open_count > 1) {
				#else
				if(newmdm_entry.open_count > 1 && !env->force) {
				#endif
					diag fprintf(stderr,"DIAG: open count for %s was %d\r\n",env->device,newmdm_entry.open_count); fflush(stderr);
					close(newmdm);
					newmdm=-1;
				}
#endif
			}
			diag if (newmdm==-1) fprintf(stderr,"DIAG: get_poolmember() returned NULL\r\n"); fflush(stderr);

			close_all_pools();
		} else diag fprintf(stderr,"DIAG: could not open pool '%s'\r\n",env->pools[poolentry]); fflush(stderr);
	}

	if (newmdm==-1) {
		int num;

		for (num=0,poolentry=0;poolentry<MAX_POOLS;poolentry++) if (env->pools[poolentry]) num++;

		if (num>1) {
			fprintf(stderr, "qtalk: No modems available amongst");
		} else {
			if (!strchr(env->pools[0],'/')) fprintf(stderr,"qtalk: All modems currently in use in pool");
			else {
				if (!err) fprintf(stderr, "qtalk: Modem in use -");
				else fprintf(stderr,"qtalk: Modem not available -");
			}
		}

		for (poolentry=0;poolentry<MAX_POOLS;poolentry++) {
			if (env->pools[poolentry])
				fprintf(stderr," %s",env->pools[poolentry]);		
		}
		fprintf(stderr,".\r\n");
	}

	return newmdm;
}

int connect(struct envstruct *env)
{
	int newfd, n;

	/* select & open modem */
	if ((newfd=select_modem(env))==-1) return -1;

	fcntl(newfd,F_SETFD,FD_CLOEXEC);
diag perror("fcntl");

	/* restore old settings for current mdm fd, if fd is in use */
	if (mdm!=-1) {
#ifndef __QNXNTO__
		/* turn off modem proxies */
		a_dev_arm(mdm,-1,_DEV_EVENT_INPUT);

		/* flush any pending modem proxies */
		while (Creceive(mproxy,buf,0)!=-1);

		/* read and discard any pending modem data */
		while (n=a_dev_ischars(mdm)) {
			if (n==-1) break;
			/* have BUFFER_SIZE to play with */
			a_read(mdm,buf,n>BUFFER_SIZE?BUFFER_SIZE:n);
		}
#else 
		/* neutrino - flush any waiting data on the fd (O_NONBLK is set) */
		do {
			n=a_read(mdm,buf,BUFFER_SIZE);
		} while (n!=0 && n!=-1);
#endif

		a_dev_mode(mdm, omdm_mode, _DEV_MODES);

#ifdef TERM_TYPE_SUPPORTED
		if (old_terminal_type!=-1 && old_terminal_type!=current_env.terminal_type) {
			struct _console_ctrl *cc;		

#ifdef TERM_TYPE_DEBUG
	{
		char *op, *np;

		if (old_terminal_type==_CON_PROT_QNX4) op="qnx";
		else if (old_terminal_type==_CON_PROT_ANSI) op="ansi";
		else op="unknown";

		if (current_env.terminal_type==_CON_PROT_QNX4) np="qnx";
		else if (current_env.terminal_type==_CON_PROT_ANSI) np="ansi";
		else np="unknown";

		fprintf(stderr,"connect: restoring old terminal type %d (%s) [!= current terminal type %d (%s)]\r\n",
				old_terminal_type, op, current_env.terminal_type, np);

	}
#endif
			
			if ((cc=console_open(kbd,O_RDWR))!=NULL) {
				console_protocol(cc, 0, old_terminal_type);
				if (old_terminal_type==_CON_PROT_ANSI) {
					fflush(stdout); fflush(stderr);
					/* emit_string(kbd,INIT_LATIN); */
					a_write(kbd,INIT_LATIN,sizeof(INIT_LATIN)-1);
				}
				console_close(cc);
			}
		}		
#endif

		/* restore old baud if we have changed it */
		if (current_env.baud||current_env.parity||current_env.stopbits||current_env.databits) {
			if (tcgetattr(mdm, &mdm_tios)!=-1) {
				if (current_env.baud) {
					mdm_tios.c_ospeed	=	omdm_ispeed;
					mdm_tios.c_ispeed	=	omdm_ospeed;
				}

				if (current_env.parity||current_env.stopbits||current_env.databits) {
					mdm_tios.c_cflag = ocflag;
				}

				tcsetattr(mdm,TCSANOW,&mdm_tios);
			}
		}
		
#ifndef __QNXNTO__
		/* detach old proxies */
		if ( qnx_proxy_rem_detach( mdm_entry.nid, vmproxy ) == -1 ) {
			fprintf(stderr, "qtalk: Unable to detach remote modem proxy\r\n" );
			a_exit(EXIT_FAILURE);
		}
		if ( qnx_proxy_detach( mproxy ) == -1 ) {
			fprintf(stderr, "qtalk: Unable to detach modem proxy\r\n" );
			a_exit(EXIT_FAILURE);
		}
#endif

		/* close old mdm fd */
		close(mdm);
	}

	/* set mdm to the new modem fd */
		
	mdm = newfd;

	//Use the recursive version since we can control the buffer size.
	if(ttyname_r(mdm, current_env.device, sizeof(current_env.device)) != EOK) {
		strcpy(current_env.device, "unknown");
	}

diag {
fprintf(stderr, "ttyname %s\r\n", ttyname(mdm));
fflush(stderr) ; sleep(1);
perror("ttyname");
fprintf(stderr, "\r\n"); fflush(stderr) ; sleep(1);
}
	printf( "Using modem %s\r\n", current_env.device );

	/* set the envar */
	setenv("MODEM",current_env.device,1);

#ifndef __QNXNTO__
	dev_info( mdm, &mdm_entry );
	if ( ( mproxy = qnx_proxy_attach( 0, 0, 0, -1 ) ) == -1 ) {
		fprintf(stderr, "qtalk: Unable to create modem proxy\r\n" );
		a_exit(EXIT_FAILURE);
	}

	if ( ( vmproxy = qnx_proxy_rem_attach( mdm_entry.nid, mproxy ) ) == -1 ) {
		fprintf(stderr, "qtalk: Unable to create remote modem proxy\r\n" );
		a_exit(EXIT_FAILURE);
	}
#endif

	signal( SIGQUIT, &gotsig );
	signal( SIGINT, &gotsig );
	signal( SIGTERM, &gotsig );

	/* Set the modem completely RAW */
	mdm_mode = 0;
	omdm_mode = a_dev_mode( mdm, mdm_mode, _DEV_MODES);

	/* flush all the stuff there to start with */
	tcflush( mdm, TCIOFLUSH );

	/* perform any stty modifications to selected & opened port */
	if (env->baud||env->parity||env->stopbits||env->databits) {
		if (tcgetattr(mdm, &mdm_tios)!=-1) {
			if (env->baud) {
				omdm_ispeed			=   mdm_tios.c_ispeed;
				omdm_ospeed			= 	mdm_tios.c_ospeed;
				mdm_tios.c_ospeed	=	env->baud;
				mdm_tios.c_ispeed	=	env->baud;
			}

			/* save cflag and lflag if we are going to be setting them */
			if (env->parity||env->stopbits||env->databits) {
				ocflag=mdm_tios.c_cflag;
			}

			if (env->parity) {
				mdm_tios.c_cflag&=~(PARENB|PARODD|PARSTK);
				switch(env->parity) {
					case NO_PARITY:   break;
					case EVEN_PARITY: mdm_tios.c_cflag|=PARENB; break;
					case ODD_PARITY: mdm_tios.c_cflag|=PARENB|PARODD; break;
					case MARK_PARITY: mdm_tios.c_cflag|=PARENB|PARODD|PARSTK; break;
					case SPACE_PARITY: mdm_tios.c_cflag|=PARENB|PARSTK; break;
					default: mdm_tios.c_cflag=ocflag; break;
				}
			}

			if (env->stopbits==1) mdm_tios.c_cflag&=~CSTOPB;
			else if (env->stopbits==2) mdm_tios.c_cflag|=CSTOPB;

            if (env->databits) {
				mdm_tios.c_cflag&=~CSIZE;
				switch(env->databits) {
					case 5: mdm_tios.c_cflag|=CS5; break;
					case 6: mdm_tios.c_cflag|=CS6; break;
					case 7: mdm_tios.c_cflag|=CS7; break;
					case 8: mdm_tios.c_cflag|=CS8; break;
				}
			}
			
			if (tcsetattr(mdm,TCSANOW,&mdm_tios)==-1) {
				if (env->baud) {
					fprintf(stderr,"qtalk: unable to change baud to %ld - %s\n",env->baud,strerror(errno));
					mdm_tios.c_ospeed	=	omdm_ispeed;
					mdm_tios.c_ispeed	=	omdm_ospeed;
				}
				if (env->parity||env->stopbits||env->databits) {
					fprintf(stderr,"qtalk: unable to change tty control(parity/stopbits/databits)\n");
					mdm_tios.c_cflag = ocflag;
				}			

				tcsetattr(mdm,TCSANOW,&mdm_tios);
				/* we've restored it now (if possible), so don't bother later on exit */
				env->baud=env->parity=env->stopbits=env->databits=0;
			}
		} else env->baud=env->parity=env->stopbits=env->databits=0;
				/* so that we don't try to change it BACK */

		/* will only attempt to restore them later if env->baud etc are not 0 */
	}

#ifdef TERM_TYPE_SUPPORTED
	if (env->terminal_type!=-1) {
		struct _console_ctrl *cc;		

#ifdef TERM_TYPE_DEBUG
	{
		char *np;

		if (current_env.terminal_type==_CON_PROT_QNX4) np="qnx";
		else if (current_env.terminal_type==_CON_PROT_ANSI) np="ansi";
		else np="unknown";

		fprintf(stderr,"connect: setting new terminal type %d (%s)\r\n",
				current_env.terminal_type, np);

	}
#endif
		
		if ((cc=console_open(kbd,O_RDWR))!=NULL) {
			old_terminal_type = console_protocol(cc, 0, env->terminal_type);
			if (old_terminal_type==-1) {
				fprintf(stderr,"qtalk: unable to set terminal type (%s)\n",strerror(errno));
				env->terminal_type=-1;
			} else if (env->terminal_type == _CON_PROT_ANSI) {
				fflush(stdout); fflush(stderr);
				if (env->pc) a_write(kbd,INIT_PCANSI,sizeof(INIT_PCANSI)-1);
				else a_write(kbd,INIT_LATIN,sizeof(INIT_LATIN)-1);
			}

			console_close(cc);
		} else {
			env->terminal_type=old_terminal_type=-1;
		}
	}		
#endif

	/* write initstring, if there is one */
	if (env->initstring[0]) {
		diag fprintf(stderr,"DIAG: emitting initstring '%s'\r\n",env->initstring); fflush(stderr);
		emit_string(mdm,env->initstring);
	}

	/* write dialstring, if there is one */
	if (env->dialstring[0]) {
		diag fprintf(stderr,"DIAG: emitting dialstring '%s'\r\n",env->dialstring); fflush(stderr);
		emit_string(mdm,env->dialstring);
	}

	/* run program, if there is one */
	if (env->execute_cmd[0]) {
		if (env->execute_delay) {
			/* delay this many ticks */
			delay(env->execute_delay * 50);
		}

		/* read and discard any pending modem data */
#ifdef __QNXNTO__
	    fcntl(mdm,F_SETFL,O_NONBLOCK);
		
		/* neutrino - flush any waiting data on the fd (O_NONBLK is set) */
		do {
			n=a_read(mdm,buf,BUFFER_SIZE);
		} while (n!=0 && n!=-1);
#else
		while (n=a_dev_ischars(mdm)) {
			if (n==-1) break;
			/* have BUFFER_SIZE to play with */
			n=a_read(mdm,buf,n>BUFFER_SIZE?BUFFER_SIZE:n);
			a_write(kbd,buf,n);
		}
#endif

		diag fprintf(stderr,"DIAG: executing '%s'\r\n",env->execute_cmd); fflush(stderr);

		old_mode();
		system(env->execute_cmd);
		new_mode();
	}

    delay(200);

#ifdef __QNXNTO__
	/* set up for non-blocking operation */
    fcntl(mdm,F_SETFL,O_NONBLOCK);
#else
	/*
	 * Arm the proxies by reading zero bytes
	 */
	a_dev_read( kbd, &buf, 0, 1, 0, 0, vkproxy, 0);
	a_dev_read( mdm, &buf, 0, 1, 0, 0, vmproxy, 0);
#endif

	return 0;
}

#define CONNECT 1
#define NOCONNECT 0

int dial_system( char *system, int doconnect )
{
	static char	tokenbuf[TOKEN_MAXLEN];
	FILE	*fp;
	char	*p;
	int		n;
	char	*dialling_file;
	int		simulated_argc;
	char	*simulated_argv[SIMULATED_ARGV_MAXITEMS];
	int		file_index;

	for (file_index=0;dialling_files[file_index]!=NULL;file_index++) {
		dialling_file = dialling_files[file_index];

		if (!*dialling_file) continue;

		diag fprintf(stderr,"DIAG: checking dial file '%s'\r\n",dialling_file); fflush(stderr);

	  	if (NULL==(fp=fopen(dialling_file,"r"))) continue;

		diag fprintf(stderr,"DIAG: file opened...\r\n"); fflush(stderr);

		/* CODE FOR PRODUCING LIST OF SYSTEMS DEFINED IN DIALLING DIRS */
		if ( strcmp( system, "?" ) == 0 ) {
			fprintf(stderr,"\r\n--------------- %s ---------------\r\n",dialling_file);

			while( 1 ) {
				p = fgets( line, LINE_LENGTH, fp );

				if(p == NULL) break;
				n = strlen(p);
				if(n<1)  break;

				if (isspace(line[0])) continue;

				line[n-1] = '\0';
				fprintf( stderr, "%s\r\n", line );
			}
		} else while( 1 ) {
			p = fgets( line, LINE_LENGTH, fp );
			if(p == NULL) break;
			n = strlen( p );
			if( n < 1) continue;
			if (line[0]=='#') continue;	/* comment line */

			/* trim trailing newline from line, if one exists */
			if (line[n-1]=='\r' || line[n-1]=='\n')	line[n - 1] = '\0';

			/* if a line starts with white space it is either blank or
               a continuation of the previous line */
			if (isspace(*line)) continue;

			/* make p point to the first white space on the line */
			for(p=line; *p; ++p) if (*p==' '||*p=='\t') break;

			/* if (*p == '\0') continue; */		/*	Check for a dialling string	*/

			/* null terminate - line now contains null-terminated system name,
               p points to dialling entry */

			if (*p!=0)
				*p++ = '\0';

			/* if this isn't the line we are looking for, continue.
               If the line started with white space, line will start
               with a null and thus we will also continue */
			if ( strcmp( line, system ) != 0 ) continue;
                            	
			diag fprintf(stderr,"DIAG: found system '%s'\r\n",system); fflush(stderr);

			/* hang up if we're supposed to hang up on a system switch */
			if (current_env.hangup_on_switch)	if (mdm!=-1) {
				tcdropline(mdm, 500);
				delay(700);	/* give modem time to reset if it is configured to do so on break */
			}

			/* reset current_env to start_env */
			dup_env(&current_env,&start_env);

			/*	Skip leading whitespace before the dialling string */
			while(*p && (*p==' ' || *p=='\t')) ++p;
			if (*p == '\0') {
				current_env.dialstring[0] = 0;	/* no dialstring */
			} else strcpy(current_env.dialstring,p);

			strcpy(current_env.systemname,system);

			diag fprintf(stderr,"DIAG: systemname='%s', dialstring='%s'\r\n",current_env.systemname,current_env.dialstring); fflush(stderr);
            
			/* build the command line into an argv-like array */
			simulated_argc=1;
			for (n=0;n<SIMULATED_ARGV_MAXITEMS;n++) simulated_argv[n] = NULL;
			simulated_argv[0] = "qtalk";

			while ((p=fgets(line,LINE_LENGTH,fp))!=NULL) {
				int inside_quote = 0, inside_dblquote = 0;
				
				n = strlen( p );
				if(n<1) break;

				if (*p=='#') continue;				/* # comment lines */

				if (*p!=' ' && *p!='	') break;	/* must start with whitespace */

				line[n - 1] = '\0';

				diag fprintf(stderr,"DIAG: argvizing line '%s'\r\n",line); fflush(stderr);
								
				inside_quote = inside_dblquote = 0;
				for (;*p;p++) {
					char *w;

					while(isspace(*p)) p++;

					tokenbuf[0] = 0;
					for (w=tokenbuf;*p;p++) {
						if (!inside_quote && !inside_dblquote && isspace(*p))
							break;

						if (!inside_quote && !inside_dblquote && *p=='\\') {
							/* backslash something */
							p++;
							if (!*p) break;
						} else if (*p=='\'') {
							if (!inside_dblquote) inside_quote=!inside_quote;
							continue;
						} else if (*p=='\"') {
							if (!inside_quote) inside_dblquote=!inside_dblquote;
							continue;
						}

						*w = *p;
						w++;
					}

					*w=0;

					if (tokenbuf[0]) {
						simulated_argv[simulated_argc] = malloc(strlen(tokenbuf)+1);
						strcpy(simulated_argv[simulated_argc],tokenbuf);
						simulated_argc++;
					}
				} /* parse tokens in this line */
				if (inside_quote || inside_dblquote) {
				 if (inside_quote) {
					fprintf(stderr,"qtalk: no ending \', system '%s' in %s\r\n",system,dialling_file);
				 }
				 if (inside_dblquote) {
					fprintf(stderr,"qtalk: no ending \"m system '%s' in %s\r\n",system,dialling_file);
				 }
				 fclose( fp );
				 return -1;
				}
			} /* get all lines of additional cmd line args */

			/* close the dialling dir */
			fclose( fp );


			/* parse command line options */
			parse_options(simulated_argc,simulated_argv,&current_env);

			if (doconnect) return connect(&current_env);
			else return 0;
		} /* else while (1) */

		fclose( fp );
	}	/* for each dialling file */

	if ( strcmp( system, "?" ) == 0 ) return 1;

	if (doconnect) 
		fprintf( stderr, "Unable to dial '%s': no dialing entry\r\n", system );
	
	return -1;
}

void kbd_handler( char *bp, int size )
{
	char	*p, *p1;
	unsigned char c;
	int		n=0;
	int command_flag = 0;
	int i;

	p = p1 = bp;

	for(i = 0; i < size; ++i) {
		c = *p++;
		if(c == current_env.cmd_char) {
			++command_flag;
			break; /* Ignore the rest of the data */
		}
#ifdef TERM_TYPE_SUPPORTED
		if (current_env.qnxs && c==0xFF) continue;
#endif
		if (current_env.del_char && c==RUB)	c=current_env.del_char;
		*p1++ = c;
		++n;
	}

	if(n > 0) {
		a_write( mdm, bp, n );
		if (current_env.local_echo) put_console( bp, n );
	}

	if(command_flag) command();

}


void command(void)
{
	register char	*p;
	char			 c;
	int 			rc;

	/* ignore SIGINT while in command mode */
	signal(SIGINT,SIG_IGN);

	if (!current_env.quiet) {
info:
		printf( "\r\nQtalk %s\r\n",VERSION);
		printf( "-------------------------------------------------------------------------------\r\n");
		printf( "Modem         : %-.62s\r\n",current_env.device);
	
		if (current_env.systemname[0] && strcmp(current_env.systemname,"defaults"))
			printf( "System        : %-.62s\r\n",current_env.systemname);
	
		printf( "Local echo    : %-9s       Logging       : %-9s\r\n"
				,current_env.local_echo?"enabled":"disabled",
				log_file!=-1?"enabled":"disabled");
	
		printf( "Top bit       : %-9s       Xfer protocol : %-9s\r\n",
				current_env.strip_parity?"stripped":"displayed",
	            (current_env.protocol==PROTOCOL_QCP)?"qcp":(current_env.protocol==PROTOCOL_ZMODEM)?"zmodem":"other");
	
		printf( "Command char  : 0x%02x",current_env.cmd_char);
		if (current_env.cmd_char>0 && current_env.cmd_char<('z'-'a'+1))	printf(" (^%c) ",'a'+current_env.cmd_char-1);
		else printf("      ");
		printf( "      Delete char   : 0x%02x",current_env.del_char);
		if (current_env.del_char>0 && current_env.del_char<('z'-'a'+1))	printf(" (^%c)",'a'+current_env.del_char-1);
		printf("\r\n");
	
#ifdef TERM_TYPE_SUPPORTED
		if (current_env.terminal_type == -1)
			printf("Terminal type : <not set>\r\n");
		else if (current_env.terminal_type == _CON_PROT_QNX4) {
			if (current_env.qnxs) 
				printf("Terminal type : qnxs\r\n");
			else printf("Terminal type : qnx\r\n");
		} else if (current_env.terminal_type == _CON_PROT_ANSI) {
			if (current_env.pc) printf("Terminal type : pcansi\r\n");
			else printf("Terminal type : ansi\r\n");
		}

#ifdef TERM_TYPE_DEBUG
		if (old_terminal_type == -1)
			printf("Old Terminal type : <not set>\r\n");
		else if (old_terminal_type == _CON_PROT_QNX4) {
			printf("Old Terminal type : qnx\r\n");
		} else if (old_terminal_type == _CON_PROT_ANSI) {
			printf("Old Terminal type : ansi\r\n");
		}
#endif
#endif
		printf( "-------------------------------------------------------------------------------\r\n");
		printf( "b)rk, c)d, d)ial, h)angup, l)og, o)protocol options, q)uit w/hangup, r)eceive\r\n");
		printf( "s)end,  t)ransfer protocol, "
                "w)rite,  "
                "x)exit w/o hangup, "
#ifdef TERM_TYPE_SUPPORTED
                "T)erm emul "
#endif
                "?)more cmds\r\n");
	}

again:
	printf( "\r\nCommand(?): ");
	fflush( stdout );
	
	do {
		if (-1==(rc=read( kbd, &c, 1 ))) {
		#ifdef DEBUG
			fprintf(stderr,"read() returned -1 (%s)\n",strerror(errno));
		#endif
		}
	} while (rc==-1);

	if (isalnum(c) || c=='?') putc( c, stdout );

	printf( "\r\n" );
	switch(c) {
		case 'b':	tcsendbreak( mdm, 300 );	break;

		case 'c':	// getcwd( &line, LINE_LENGTH );
				getcwd( line, LINE_LENGTH );
					//printf( "Current Directory is: %s\r\n", &line);
					printf( "Current Directory is: %s\r\n", line);
					p = get_name( "Directory? " );
					if (strlen(p) > 0) {
						if ( chdir( p ) != 0 )
							printf( "Unable to change to directory %s\r\n", p );
					} else printf("Directory not changed.\r\n");
					break;

		case 'C':   printf( "Current command character is 0x%x",current_env.cmd_char);
					if (current_env.cmd_char>0 && current_env.cmd_char<('z'-'a'+1)) 
						printf(" (^%c)",'a'+current_env.cmd_char-1);
					printf("\r\n");
					p=get_name("New command character? (in hex or use ^char) ");

					if (strlen(p)>0) {
						while (isspace(*p)) p++;
						if (*p=='^') {
							p++;
							*p=tolower(*p);
							if (*p<'a' || *p>'z') {
								printf("Unable to change command char - ^%c not valid.\r\n",*p);
							} else {
								current_env.cmd_char = *p - 'a' + 1;

								printf( "New command character is 0x%x",current_env.cmd_char);
								if (current_env.cmd_char>0 && current_env.cmd_char<('z'-'a'+1)) 
									printf(" (^%c)",'a'+current_env.cmd_char-1);
								printf("\r\n");
							}
						} else if(isdigit(*p)) {
							long n;      

							n = strtol(p,NULL,16);
							if (n>0 && n<128) {
								current_env.cmd_char = (unsigned char) n;

								printf( "New command character is 0x%x",current_env.cmd_char);
								if (current_env.cmd_char>0 && current_env.cmd_char<('z'-'a'+1)) 
									printf(" (^%c)",'a'+current_env.cmd_char-1);
								printf("\r\n");
							} else printf("Unable to change command char - must be 0<x<0x80\r\n");
						} else printf("Must be either a hex number of ^ctrl_char e.g. 0x01, ^a\r\n");
					} else printf("Command character not changed\r\n");

                    break;

		case 'D':   printf( "Current delete character is 0x%x",current_env.del_char);
					if (current_env.del_char>0 && current_env.del_char<('z'-'a'+1)) 
						printf(" (^%c)",'a'+current_env.del_char-1);
					printf("\r\n");
					p=get_name("New delete character? (in hex or use ^char) ");

					if (strlen(p)>0) {
						while (isspace(*p)) p++;
						if (*p=='^') {
							p++;
							*p=tolower(*p);
							if (*p<'a' || *p>'z') {
								printf("Unable to change delete char - ^%c not valid.\r\n",*p);
							} else {
								current_env.del_char = *p - 'a' + 1;

								printf( "New delete character is 0x%x",current_env.del_char);
								if (current_env.del_char>0 && current_env.del_char<('z'-'a'+1)) 
									printf(" (^%c)",'a'+current_env.del_char-1);
								printf("\r\n");
							}
						} else if(isdigit(*p)) {
							long n;

							n = strtol(p,NULL,16);
							if (n>0 && n<128) {
								current_env.del_char = (unsigned char) n;

								printf( "New delete character is 0x%x",current_env.del_char);
								if (current_env.del_char>0 && current_env.del_char<('z'-'a'+1)) 
									printf(" (^%c)",'a'+current_env.del_char-1);
								printf("\r\n");
							} else printf("Unable to change delete char - must be 0<x<0x80\r\n");
						} else printf("Must be either a hex number of ^ctrl_char e.g. 0x01, ^a\r\n");
					} else printf("Delete character not changed\r\n");

                    break;

		case 'd':	do {
						p = get_name("System? (? for list) ");
						if (*p=='\0') {
							printf("No system selected.\r\n");
							break;
						}
					} while( dial_system( p, CONNECT ) == 1 );
					break;

		case 'e':	current_env.local_echo=!current_env.local_echo;
					printf("Local echo %s\r\n",current_env.local_echo?"enabled":"disabled");
					break;

		case 'I':   goto info;

		case 'l':	if ( log_file != -1 ) {	/*	toggle log status	*/
						close( log_file );
						log_file = -1;
						printf( "Log file closed\r\n");
					} else {
						char *str;
						log_file=open(str=get_name("Log file?"),O_WRONLY|O_APPEND|O_CREAT,0666);
						if (*str) {
							if (log_file==-1) printf("Cannot open log file '%s' : %s\r\n",str,strerror(errno));
							else fcntl(log_file,F_SETFD,FD_CLOEXEC);
						} else printf("No log file opened.\r\n");
					}
					break;

		case 'o':	
					{
					int i=-1,j=-1;

					p = get_name( "Protocol options for which protocol? (qcp, zmodem or other): " );
					if (strlen(p)>0) {
						if (!strncmp(p,"qcp",strlen(p))) {strcpy(p,"qcp"); i = PROTOCOL_QCP;}
						else if (!strncmp(p,"zmodem",strlen(p))) {strcpy(p,"zmodem"); i = PROTOCOL_ZMODEM;}
						else if (!strncmp(p,"other",strlen(p))) {strcpy(p,"other"); i = PROTOCOL_OTHER;}
						else printf("Unknown protocol (must be qcp, zmodem or other). Not changed.\r\n");
					} else	printf("Protocol options not changed.\r\n");

					if (i==-1) break;

					printf("Protocol: %s\r\n",p);
					printf("Send = '%s'\r\nReceive = '%s'\r\n",current_env.protocol_strings[i][TX_PROTOCOL],current_env.protocol_strings[i][RX_PROTOCOL]);

					p = get_name( "Which do you wish to change? (send or receive): " );
					if (strlen(p)>0) {
						if (!strncmp(p,"send",strlen(p))) j = TX_PROTOCOL;
						else if (!strncmp(p,"receive",strlen(p))) j = RX_PROTOCOL;
						else printf("Unknown protocol (must be send or receive). Not changed.\r\n");
					} else	printf("Protocol options not changed.\r\n");
					if (j==-1) break;

					printf("Current setting: '%s'\r\n",current_env.protocol_strings[i][j] );
					p = get_name( "New setting: " );
					if(strlen(p)>0) {
						if (!strcmp(p,"\"\"") || !strcmp(p,"''")) current_env.protocol_strings[i][j][0]=0;
						else strcpy( current_env.protocol_strings[i][j], p );
					} else printf("Transfer options not changed.\r\n");
					}
					break;

		case 't':	printf("Current protocol is \"%s\"\r\n",
						(current_env.protocol==PROTOCOL_QCP)?"qcp":(current_env.protocol==PROTOCOL_ZMODEM)?"zmodem":"other");
					p = get_name( "Protocol (choose 'qcp', 'zmodem' or 'other'): " );
					if (strlen(p)>0) {
						if (!strncmp(p,"qcp",strlen(p))) current_env.protocol = PROTOCOL_QCP;
						else if (!strncmp(p,"zmodem",strlen(p))) current_env.protocol = PROTOCOL_ZMODEM;
						else if (!strncmp(p,"other",strlen(p))) current_env.protocol = PROTOCOL_OTHER;
						else printf("Unknown protocol (must be qcp, zmodem, or other). Not changed.\r\n");
					} else printf("Current protocol not changed.\r\n");
					break;

		case 'p':	current_env.strip_parity=!current_env.strip_parity;
					printf("Parity bit %s\r\n",current_env.strip_parity?"stripped":"displayed");
					break;

		case 'r':
					old_mode();
					system( current_env.protocol_strings[current_env.protocol][RX_PROTOCOL] );
					new_mode();
					break;

		case 's':	p=get_name( "Send file(s)? " );
					if (strlen(p)<1) {
						printf("Not sent - no file specified.\r\n");
						break;
					}
					setenv("FILENAME",p,1);
					old_mode();
					system( current_env.protocol_strings[current_env.protocol][TX_PROTOCOL] );
					new_mode();
					setenv("FILENAME",NULL,1);
					break;

#ifdef TERM_TYPE_SUPPORTED
		case 'T':
			if (current_env.terminal_type == -1)
				printf("Current terminal type is not set.\r\n");
			else if (current_env.terminal_type == _CON_PROT_QNX4) {
				if (current_env.qnxs) 
					printf("Current terminal type is 'qnxs'.\r\n");
				else
					printf("Current terminal type is 'qnx'.\r\n");
			} else if (current_env.terminal_type == _CON_PROT_ANSI) {
				if (current_env.pc) 
					printf("Current terminal type is 'pcansi'.\r\n");
				else
					printf("Current terminal type is 'ansi'.\r\n");
			}

			p = get_name( "Choose qnx, qnxs, ansi or pcansi: " );
			if (strlen(p)<1) break;

			if (!strncmp(p,"qnx",strlen(p))) {
				current_env.terminal_type = _CON_PROT_QNX4;
				current_env.qnxs = 0;
				current_env.pc = 0;
			} else if (!strcmp(p,"qnxs")) {
				current_env.terminal_type = _CON_PROT_QNX4;
				current_env.qnxs = 1;
				current_env.pc = 0;
			} else if (!strncmp(p,"ansi",strlen(p))) {
				current_env.terminal_type = _CON_PROT_ANSI;
				current_env.qnxs = 0;
				current_env.pc = 0;
			} else if (!strncmp(p,"pcansi",strlen(p))) {
				current_env.terminal_type = _CON_PROT_ANSI;
				current_env.qnxs = 0;
				current_env.pc = 1;
			} else {
				fprintf(stderr,"qtalk: unknown terminal type '%s'\r\n",p);
				break;
			}

			if (old_terminal_type!=-1) {
				struct _console_ctrl *cc;		

#ifdef TERM_TYPE_DEBUG
	{
		char *op, *np;

		if (old_terminal_type==_CON_PROT_QNX4) op="qnx";
		else if (old_terminal_type==_CON_PROT_ANSI) op="ansi";
		else op="unknown";

		if (current_env.terminal_type==_CON_PROT_QNX4) np="qnx";
		else if (current_env.terminal_type==_CON_PROT_ANSI) np="ansi";
		else np="unknown";

		fprintf(stderr,"command 'T': restoring old terminal type %d (%s) != current terminal type %d (%s)\r\n",
				old_terminal_type, op, current_env.terminal_type, np);

	}
#endif
				
				if ((cc=console_open(kbd,O_RDWR))!=NULL) {
					console_protocol(cc, 0, old_terminal_type);
					if (old_terminal_type==_CON_PROT_ANSI) {
						fflush(stdout); fflush(stderr);
						a_write(kbd,INIT_LATIN,sizeof(INIT_LATIN)-1);
					}
					console_close(cc);
				}
			}		

			{
				struct _console_ctrl *cc;		
				
#ifdef TERM_TYPE_DEBUG
	{
		char *np;

		if (current_env.terminal_type==_CON_PROT_QNX4) np="qnx";
		else if (current_env.terminal_type==_CON_PROT_ANSI) np="ansi";
		else np="unknown";

		fprintf(stderr,"command 'T': setting new terminal type %d (%s)\r\n",
				current_env.terminal_type, np);

	}
#endif
				if ((cc=console_open(kbd,O_RDWR))!=NULL) {
					old_terminal_type = console_protocol(cc, 0, current_env.terminal_type);
					if (old_terminal_type==-1) {
						fprintf(stderr,"qtalk: unable to set terminal type (%s)\r\n",strerror(errno));
						current_env.terminal_type=-1;
					} else {
						/* success */
						if (current_env.terminal_type==_CON_PROT_QNX4) {
							if (current_env.qnxs)
								printf("Terminal type set to 'qnxs'.\r\n");
							else 
								printf("Terminal type set to 'qnx'.\r\n");
						} else {
							if (current_env.pc) {
								fflush(stdout); fflush(stderr);
								a_write(kbd,INIT_PCANSI,sizeof(INIT_PCANSI)-1);
								printf("Terminal type set to 'pcansi'.\r\n");
							} else {		
								fflush(stdout); fflush(stderr);
								a_write(kbd,INIT_LATIN,sizeof(INIT_LATIN)-1);
								printf("Terminal type set to 'ansi'.\r\n");
							}
						}
					}
					console_close(cc);
				} else {
					fprintf(stderr,"qtalk: can't set term type - console_open() %s\n",strerror(errno));
					current_env.terminal_type=old_terminal_type=-1;
				}
			}		
			break;
#endif

		case 'u':	mdm_write( 1 );	break;

		case 'w':	mdm_write( 0 );	break;

		case 'h':	tcdropline(mdm, 500);	delay(700); break;

		case 'q':	tcdropline( mdm, 500 );	/* fall thru */
		case 'x':	a_exit(EXIT_SUCCESS);

		case '!':	p = get_name("Enter command: ");
					if(strlen(p) > 0) {
						old_mode();
						system( p );
						new_mode();
					}
					break;
		default:	if( c == current_env.cmd_char ) {
						a_write( mdm, &c, 1 );
						break;
					}
					if (isalnum(c)||c=='?') {
						printf( "%s%s%s%s%s%s%s%s%s%s",
								"b - Send Break              q - Quit & Hangup\r\n",
								"c - Change directory        r - Receive a file\r\n",
								"d - Dial System             s - Send File\r\n",
								"e - Toggle Local Echo       t - Select transfer protocol\r\n",
								"h - Hangup                  w - Write File to Modem\r\n",
								"l - Log File On/Off         x - Exit w/o Hangup\r\n",
								"o - Modify protocol options ! - Escape to shell\r\n",
								"p - Toggle Parity Filter    I - Show current info\r\n",
								"C - Command Character       D - Delete Character\r\n",
#ifdef TERM_TYPE_SUPPORTED
                                "T - Switch terminal emulation between QNX4, QNXS, and ANSI\r\n"
#else
                                ""
#endif
						);
						goto again;
					}
	}
	printf("Returning to interactive mode. (%s)\r\n",current_env.device);
	signal(SIGINT,&gotsig);
	return;
}

/* this is used only by u and w commands */
void mdm_write( int upload_flag )
{
	int		fd, n, i;
	char	c, kick_seen;
	char    *name;

	name=get_name("Upload file? "); 

	if (strlen(name)<1) return;

	if ( ( fd = open( name, O_RDONLY ) ) == -1 ) {
		printf("Could not open '%s' for read (%s)\r\n", name, strerror(errno));
		return;
	}

	while( a_read( fd, &c, 1 ) == 1 ) {
		if ( current_env.local_echo ) {
			if (c=='\n') put_console("\r\n", 2);
			else put_console( &c, 1 );
		}

		if (c=='\n') c='\r';

		a_write( mdm, &c, 1 );

		kick_seen = (upload_flag && !current_env.local_echo	|| current_env.pause_char && current_env.kick_char
					 && c==current_env.pause_char)? 0 : 1 ;
		/*
		 *	Only flush if you are about to wait for echo or kick.
		 *  99% of the time a <W>rite won't go through this code.
		 */
		if (!kick_seen) a_tcdrain(mdm);

		while(!kick_seen) {
#ifdef DEVARM
			if ( !mdmflag ) {
				a_dev_arm( mdm, mproxy, _DEV_EVENT_INPUT );
				mdmflag = 1;
			}
			if ( !kbdflag ) {
				a_dev_arm( kbd, kproxy, _DEV_EVENT_INPUT );
				kbdflag = 1;
			}
			pid = Receive( 0, 0, 0 );
#else
			for( ;; ) {
				if ( a_dev_ischars( mdm ) ) {
					pid = mproxy;
					break;
				}
				else if ( a_dev_ischars( kbd ) ) {
					pid = kproxy;
					break;
				}
			}
#endif

			if ( pid == kproxy ) {
				printf( "Upload aborted.\r\n" );
				goto end;
			} else if ( pid == mproxy ) {
				n = a_read( mdm, &buf, a_dev_ischars( mdm ) );
				put_console( buf, n );
				if ( current_env.pause_char  && current_env.kick_char  &&  c == current_env.pause_char ) {
					for( kick_seen = i = 0; !kick_seen  &&  i < n; ++i ) {
						if (buf[i]==current_env.kick_char)	kick_seen = 1;
					}
				} else kick_seen = 1;
			} else {
				fprintf(stderr, "qtalk: Received an unexpected message from pid %d\r\n", pid );
			}
		}
	}

	a_tcdrain( mdm );
	printf( "Upload completed\r\n");
	fflush( stdout );
end:
	close( fd );
}

void dup_env(struct envstruct *to, struct envstruct *from) 
{
	int n;

	*to = *from;

	/* duplicate modem pool entries */
	for (n=0;n<MAX_POOLS;n++) {
		if (from->pools[n]!=NULL) {
			to->pools[n] = malloc(strlen(from->pools[n])+1);
			strcpy(to->pools[n],from->pools[n]);
		}
	}
}


void clear_pools (struct envstruct *env)
{
	int n;

	for (n=0;n<MAX_POOLS;n++) {
		if (env->pools[n]!=NULL) {
			free(env->pools[n]);
			env->pools[n] = NULL;
		}
	}
}

int add_pool(struct envstruct *env, char *new_pool) 
{
	int n;

	for (n=0;n<MAX_POOLS && env->pools[n]!=NULL;n++) ;

	if (n==MAX_POOLS) {
		/* outta da pool, bub */
	} else {
		env->pools[n] = malloc(strlen(new_pool)+1);
		strcpy(env->pools[n],new_pool);
	}		
	
	return 0;
}

int parse_options(int argc, char **argv, struct envstruct *env) 
{
	int opt, error=0, pool_specd=0;
	char *p;

	{
		int n;

		diag fprintf(stderr,"DIAG; argc=%d\r\n",argc); fflush(stderr);
		diag	for (n=0;n<=argc;n++) {
					fprintf(stderr,"DIAG: argv[%d] = '%s'\r\n",n,argv[n]); fflush(stderr);
				}
	}

# if _NTO_VERSION < 110
	optind = 0;
# else
	optind = 1;
# endif

	while((opt=getopt(argc,argv,"qhs:m:x:D:l:ePOo:c:d:b:t:Z" FORCE_ALLOWED_OPTION TERM_OPTION ))!=-1) {

		diag fprintf(stderr,"DIAG; opt=%c(%x)\r\n",opt, opt); fflush(stderr);

		switch(opt) {
			case 'q':   ++env->quiet; break;

			case 'Z':   diagflag++;	break;

			case 's':  	dialling_files[0] = optarg;	break;

			case 'h':	++env->hangup_on_switch;			break;

 			case 'm':	if (!pool_specd) {
							clear_pools(env);
							pool_specd = 1;
						}
diag { fprintf(stderr, "-m option, dev %s\n", optarg); fflush(stderr); }
						add_pool(env,optarg);
						break;
 			case 'l':	log_file=open(optarg,O_WRONLY|O_APPEND|O_CREAT,0666);
						if (log_file!=-1) fcntl(log_file,F_SETFD,FD_CLOEXEC);
						break;
 			case 'e':	++env->local_echo;					 	break;
 			case 'P':	++env->strip_parity;				 	break;
/* deprecated */
#ifdef TERM_TYPE_SUPPORTED
 			case 'E':	++env->qnxs; env->terminal_type = _CON_PROT_QNX4; break;
#endif
			case 'O':	open_override = 1; break;
 			case 'o':	/* -o protocol="string" */
						{
							char *p;
							int n, i;

							if (!strncmp(optarg,"qcp_re=",7)) {
								p=&optarg[7];
								n=PROTOCOL_QCP;
								i=RX_PROTOCOL;
							} else if (!strncmp(optarg,"qcp_se=",7)) {
								p=&optarg[7];
								n=PROTOCOL_QCP;
								i=TX_PROTOCOL;
							} else if (!strncmp(optarg,"zmodem_re=",10)) {
								p=&optarg[10];
								n=PROTOCOL_ZMODEM;
								i=RX_PROTOCOL;
							} else if (!strncmp(optarg,"zmodem_se=",10)) {
								p=&optarg[10];
								n=PROTOCOL_ZMODEM;
								i=TX_PROTOCOL;
							} else if (!strncmp(optarg,"other_re=",9)) {
								p=&optarg[9];
								n=PROTOCOL_OTHER;
								i=RX_PROTOCOL;
							} else if (!strncmp(optarg,"other_se=",9)) {
								p=&optarg[9];
								n=PROTOCOL_OTHER;
								i=TX_PROTOCOL;
							} else {
								fprintf(stderr,"qtalk -o: Unknown protocol (%s)\r\n",optarg);
								fprintf(stderr,"qtalk -o: 'use qtalk' for list of valid protocols\n");
							    error++;
								break;				
							}

							strcpy( env->protocol_strings[n][i], p);
						}
						break;

 			case 'c':	env->cmd_char = (char)atoh( optarg ); 	break;
 			case 'd':	env->del_char = (char)atoh( optarg ); 	break;
 			case 'b':	for (p=strtok(optarg,",");p; p=strtok(NULL,",")) {
							if (isdigit(*p)) {
								long n;
								n=strtol(p,NULL,10);
							    if (n==1 || n==2) env->stopbits=(char) n;
								else if (n==7 || n==8) env->databits=(char) n;
								else env->baud=n;
							} else {
								/* parity */
								switch(*p) {
									case 'n': env->parity=NO_PARITY; break;
									case 'e': env->parity=EVEN_PARITY; break;
									case 'o': env->parity=ODD_PARITY; break;
									case 'm': env->parity=MARK_PARITY; break;
									case 's': env->parity=SPACE_PARITY; break;
									default:
										fprintf(stderr,"qtalk -b: invalid specification (%s)\n",optarg); 
										break;
								}
							}
						}
						break;

			case 'x':   strcpy(env->execute_cmd, optarg);	break;
			case 'D':   env->execute_delay=atoh(optarg);	break;
			case 't':	if (!strncmp(optarg,"qcp",strlen(optarg))) env->protocol = PROTOCOL_QCP;
						else if (!strncmp(optarg,"zmodem",strlen(optarg))) env->protocol = PROTOCOL_ZMODEM;
						else if (!strncmp(optarg,"other",strlen(optarg))) env->protocol = PROTOCOL_OTHER;
						else {
							printf("qtalk -t: Unknown protocol (must be qcp, zmodem or other).\r\n");
							error++;
						}
						break;

#ifdef FORCE_ALLOWED
			case 'F':	env->force = 1;						break;
#endif

#ifdef TERM_TYPE_SUPPORTED
			case 'T':   if (!strcmp("qnx",optarg)) {
							env->terminal_type = _CON_PROT_QNX4; 
							env->qnxs = 0;
							env->pc = 0;
						} else if (!strcmp("qnxs",optarg)) {
							env->terminal_type = _CON_PROT_QNX4;
							env->qnxs = 1;
							env->pc = 0;
						} else if (!strcmp("ansi",optarg)) {
							env->terminal_type = _CON_PROT_ANSI;
							env->pc = 0;
							env->qnxs=0;
						} else if (!strcmp("pcansi",optarg)) {
							env->terminal_type = _CON_PROT_ANSI;
							env->pc = 1;
							env->qnxs=0;
						} else fprintf(stderr,"qtalk: invalid terminal type -T %s\r\n",optarg);
						break;
#endif
					
			default:	error++;							break;
		}
	}
	diag fprintf(stderr,"DIAG; opt=%c(%x) before return\r\n",opt, opt); fflush(stderr);
	diag fprintf(stderr,"DIAG: parse_options optind = %d\r\n", optind); fflush(stderr);
	
	return error?-1:0;
}

int main( int argc, char *argv[] )
{
	unsigned  			 n;
#ifndef __QNXNTO__
	int					 armed;
#endif
	char				*cp;

	signal( SIGUSR1, &sighandler);

	/**** determine name of the personal dialling directory - 
     ****
     **** $HOME/.qtalk      used if $HOME envar exists
     ****
     **** cuserid()/.qtalk  used if $HOME envar does not exist
     **** 
     **** If cuserid() fails, no personal dialling directory will be
     **** checked for.
     ****/

	dialling_files[0] = personal_dialdir;

	if ((cp=getenv("HOME"))!=NULL) {
		strcpy(personal_dialdir,cp);
#ifndef __QNXNTO__
	} else if (cp=cuserid(NULL)) {
		sprintf(personal_dialdir,"/home/%s",cp);
#endif
	} else personal_dialdir[0] = 0;

	if (personal_dialdir[0]) {
		if (strcmp(personal_dialdir,"/")) strcat(personal_dialdir,"/");
		strcat(personal_dialdir,".qtalk");
		/* Note that we do not care about existence of this file at startup,
           user is free to create it for use while qtalk is running. */
	} 

	/**** set startup defaults, put into the start_env environment. */

	start_env.systemname[0]			= 0;
	start_env.dialstring[0]			= 0;
	start_env.execute_cmd[0]		= 0;
	start_env.execute_delay			= 3;	/* 3 x 1/20th second */
	start_env.device[0]				= 0;
	start_env.initstring[0]			= 0;
#ifdef TERM_TYPE_SUPPORTED
	start_env.terminal_type			= -1;	/* means 'leave it alone' */
#endif
	start_env.cmd_char				= DEFAULT_CMD_CHAR;
	start_env.del_char				= 0x7f;	/* translate KBD RUB to RUB */
	start_env.pause_char			= 0;
	start_env.kick_char				= 0;
	start_env.baud					= 0;	/* this does not mean 0 baud; it means 'do not attempt
                                               to change the baud of the selected port' */
	start_env.parity                = 0;	/* likewise, 0 means 'unchanged' */
	start_env.databits				= 0;    /* ditto */
	start_env.stopbits				= 0;    /* ditto */

	start_env.protocol = PROTOCOL_QCP;
	strcpy(start_env.protocol_strings[PROTOCOL_QCP][RX_PROTOCOL],DEFAULT_QCP_RE_PROTOCOL);
	strcpy(start_env.protocol_strings[PROTOCOL_QCP][TX_PROTOCOL],DEFAULT_QCP_SE_PROTOCOL);
	strcpy(start_env.protocol_strings[PROTOCOL_ZMODEM][RX_PROTOCOL],DEFAULT_ZMODEM_RE_PROTOCOL);
	strcpy(start_env.protocol_strings[PROTOCOL_ZMODEM][TX_PROTOCOL],DEFAULT_ZMODEM_SE_PROTOCOL);
	strcpy(start_env.protocol_strings[PROTOCOL_OTHER][RX_PROTOCOL],DEFAULT_OTHER_RE_PROTOCOL);
	strcpy(start_env.protocol_strings[PROTOCOL_OTHER][TX_PROTOCOL],DEFAULT_OTHER_SE_PROTOCOL);

	for (n=0;n<MAX_POOLS;n++) start_env.pools[n] = NULL;
	add_pool(&start_env,"/dev/ser1");

	start_env.transfer_options[0]	= 0;
	start_env.local_echo			= 0;
	start_env.strip_parity			= 0;
	start_env.quiet					= 0;
#ifdef TERM_TYPE_SUPPORTED
	start_env.qnxs					= 0;
	start_env.pc					= 0;	/* pc-ansi flag */
#endif
	start_env.hangup_on_switch		= 0;

	#ifdef FORCE_ALLOWED	
		start_env.force				= 0;
	#endif

	dup_env(&current_env, &start_env);
	if (dial_system("defaults",NOCONNECT)!=-1) dup_env(&start_env, &current_env);

	/* parse command line options, put settings into start_env since
       these options are specified on the cmd line from the shell, they
       will override other defaults */

	if (parse_options(argc,argv,&start_env)==-1) exit(EXIT_FAILURE);

	/* make our current environment our start environment */
	dup_env(&current_env, &start_env);

	/* set our kbd file descriptor to stdin, and scr fd to stdout */

	kbd = 0;
	scr = 1;

	/* no modem yet - setting mdm to -1 lets other parts of the program be
       aware of this - many are called both before an original connect and
       also while a system is currently connected (such as is the situation
       when dialling a new system when already connected to a system) */

	mdm = -1;

	/* print out Qtalk message so's people know what they're runnin' */
	printf( "Qtalk %s\n", VERSION );

	/* set the keyboard raw */
	kbd_mode = 0;
	okbd_mode = a_dev_mode( kbd, kbd_mode, _DEV_MODES);

#ifndef __QNXNTO__
	if ( ( kproxy = qnx_proxy_attach( 0, 0, 0, -1 ) ) == -1 ) {
		fprintf(stderr, "qtalk: Unable to create keyboard proxy\n" );
		a_exit(EXIT_FAILURE);
	}
	dev_info( kbd, &kbd_entry );
	if ( ( vkproxy = qnx_proxy_rem_attach( kbd_entry.nid, kproxy ) ) == -1 ) {
		fprintf(stderr, "qtalk: Unable to create remote keyboard proxy\n" );
		a_exit(EXIT_FAILURE);
	}
#endif

	tcflush( kbd, TCIFLUSH );

#ifndef __QNXNTO__
	a_dev_read( kbd, &buf, 0, 1, 0, 0, vkproxy, 0);
#endif

	/* either dial a system or connect right now to the default or selected modem
       port (or pool) */

	diag fprintf(stderr,"DIAG: optind = %d, argc = %d\r\n",optind, argc); fflush(stderr);

	if (optind<argc) {
		diag fprintf(stderr,"DIAG: calling dial_system(%s)\r\n",argv[optind]); fflush(stderr);
		if (-1==dial_system(argv[optind],CONNECT))	a_exit(EXIT_FAILURE);
	} else {
		if (-1==connect(&current_env))		a_exit(EXIT_FAILURE);
	}

	sigsetjmp( jmpbuf, 1 );	/* If we get a BREAK, send one and return here */
#ifndef __QNXNTO__
	/* this is necessary to make sure that dev is armed if we arrive here
       due to being ripped out of some random operation due to a SIGINT */
	a_dev_arm(kbd,vkproxy,_DEV_EVENT_INPUT);
	a_dev_arm(mdm,vmproxy,_DEV_EVENT_INPUT);
#endif

#ifdef DEBUG
printf("Starting...\n");
#endif

		printf( "-------------------------------------------------------------------------------\r\n");
		printf( "Modem         : %-.62s\r\n",current_env.device);
	
		if (current_env.systemname[0] && strcmp(current_env.systemname,"defaults"))
			printf( "System        : %-.62s\r\n",current_env.systemname);
	
		printf( "Local echo    : %-9s       Logging       : %-9s\r\n"
				,current_env.local_echo?"enabled":"disabled",
				log_file!=-1?"enabled":"disabled");
	
		printf( "Top bit       : %-9s       Xfer protocol : %-9s\r\n",
				current_env.strip_parity?"stripped":"displayed",
	            (current_env.protocol==PROTOCOL_QCP)?"qcp":(current_env.protocol==PROTOCOL_ZMODEM)?"zmodem":"other");
	
		printf( "Command char  : 0x%02x",current_env.cmd_char);
		if (current_env.cmd_char>0 && current_env.cmd_char<('z'-'a'+1))	printf(" (^%c) ",'a'+current_env.cmd_char-1);
		else printf("      ");
		printf( "      Delete char   : 0x%02x",current_env.del_char);
		if (current_env.del_char>0 && current_env.del_char<('z'-'a'+1))	printf(" (^%c)",'a'+current_env.del_char-1);
		printf("\r\n");
	
#ifdef TERM_TYPE_SUPPORTED
		if (current_env.terminal_type == -1)
			printf("Terminal type : <not set>\r\n");
		else if (current_env.terminal_type == _CON_PROT_QNX4) {
			if (current_env.qnxs) 
				printf("Terminal type : qnxs\r\n");
			else printf("Terminal type : qnx\r\n");
		} else if (current_env.terminal_type == _CON_PROT_ANSI) {
			if (current_env.pc) printf("Terminal type : pcansi\r\n");
			else printf("Terminal type : ansi\r\n");
		}

#ifdef TERM_TYPE_DEBUG
		if (old_terminal_type == -1)
			printf("Old Terminal type : <not set>\r\n");
		else if (old_terminal_type == _CON_PROT_QNX4) {
			printf("Old Terminal type : qnx\r\n");
		} else if (old_terminal_type == _CON_PROT_ANSI) {
			printf("Old Terminal type : ansi\r\n");
		}
#endif
#endif
		printf( "-------------------------------------------------------------------------------\r\n");

#ifndef __QNXNTO__
	for( ;; ) {
		pid = Receive( 0, &buf, 0 );
		if ( pid == mproxy ) {
			armed = 0;
			while(!armed) {
				n = a_dev_read( mdm, &buf[0], BUFFER_SIZE, 1, 0, 0, vmproxy, &armed);
				strncpy(buf2,buf,n+1);
				buf2[n+1]=0;	
				if (n==-1) perror("dev_read(mdm...)");
				mdm_handler( &buf[0], n, &armed );
			}
		} else if ( pid == kproxy ) {
			armed = 0;
			while(!armed) {
				n = a_dev_read( kbd, &buf[0], BUFFER_SIZE, 1, 0, 0, vkproxy, &armed);
				kbd_handler( &buf[0], n );
			}
		} else if ( pid != -1 ) {
			Reply(pid,"\131",1);	/* reply ENOSYS */
		}
	}
#else
	/* neutrino version */
	{
	int errs=0;
	for (;;) {
		fd_set readfds;

		FD_ZERO(&readfds);
		FD_SET(kbd, &readfds);
		FD_SET(mdm, &readfds);
		if (select(1+max(kbd,mdm),&readfds,NULL, NULL, NULL)!=-1) {
			errs=0;
			if (FD_ISSET(mdm,&readfds)) {
				/* handle modem input */
				n=a_read(mdm,buf, BUFFER_SIZE);
				if (n>0) mdm_handler(buf,n);
			} else if (FD_ISSET(kbd, &readfds)) {
				/* handle keyboard input */
				n=a_read(kbd,buf, BUFFER_SIZE);
				if (n>0) kbd_handler(buf,n);
			}
		} else {
			if (errno!=EINTR) {
				fprintf(stderr,"[Select returned %s]\r\n",strerror(errno));
				if (++errs>100) a_exit(EXIT_FAILURE);
			}
		}
	}
	}
#endif
	return 0;
}

#ifdef __QNXNTO__ 
#if _NTO_VERSION < 110

#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <process.h>
#include <spawn.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

int system (char *cmd)
{
	int						pid, status;
	struct sigaction		sa, savintr, savequit, savewinch;
	char					*argv[4];
	spawn_inheritance_type	inherit;

	// Setup arguments for spawn.
	argv[0] = "/bin/sh";
	argv[1] = "-c";
	argv[2] = cmd;
	argv[3] = NULL;

	// If cmd is NULL we do an existance check on the shell.
	if(cmd == NULL) {
		struct stat			sbuf;
		return(stat(argv[0], &sbuf) == 0);
		}

	// Ignore SIGINT,SIGQUIT,SIGWINCH and mask SIGCHLD on parent.
	sa.sa_handler = SIG_IGN;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, &savintr);
	sigaction(SIGQUIT, &sa, &savequit);
	sigaction(SIGWINCH, &sa, &savewinch);
// comment out because of NTO bug which sets action to kill as side-effect?
  	sigaddset(&sa.sa_mask, SIGCHLD); 
	sigprocmask(SIG_BLOCK, &sa.sa_mask, &inherit.sigmask);

	// Inialize inheritance structure for spawn.
	sigfillset(&inherit.sigdefault);
	inherit.flags = SPAWN_SETSIGDEF | SPAWN_SETSIGMASK;

	// POSIX 1003.1d implementation.
	pid = spawn(argv[0], 0, NULL, &inherit, argv, NULL);
	if (pid==-1) fprintf(stderr,"Spawn failed (%s)\r\n",strerror(errno));
	else {
		while(waitpid(pid, &status, 0) == -1)
			if(errno != EINTR) {
				status = -1;
				break;
				}
	}

	// restore SIGINT, SIGQUIT, SIGCHLD.
	sigaction(SIGINT, &savintr, NULL);
	sigaction(SIGQUIT, &savequit, NULL);
	sigaction(SIGWINCH, &savewinch, NULL);
	sigprocmask(SIG_SETMASK, &inherit.sigmask, NULL);

//	if(status == -1) errno = err;

	return(status);
}

#endif
#endif
