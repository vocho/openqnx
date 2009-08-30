#ifdef __USAGE
%C - modem handler to answer calls (QNX)

%C	[options]
Options:
 -a string     String to answer the phone. (default: "ATA")
 -b baud       Baud rate. (default: 2400)
 -B baud       Baud rate to assume for "connect" without a rate. (default 1200)
 -c command    Command to start for a data call. (default: /bin/login -p)
 -C            Don''t wait for CTS.
 -d [P]number  Dial this number and attempt to connect to a remote modem.
 -D file       Debug mode.
 -f command    Command to start for a fax call. (default: /bin/fax re)
 -F fax_id     Local fax machine identification. (default: "Unknown")
 -g file       Greeting message to print on connect.
 -i string     Initalize modem. (default: "ATZ|~AT")
 -I file       Identification file for caller-id/callback processing.
 -l            Don''t lock the baud rate.
 -L            Locked baud rate (default).
 -m modem      Serial line to connect to. Only valid with -d option.
               Changes default cmd (-c) to /bin/qtalk -m modem.
 -o            Strict open count checking on the tty
 -p parity     Parity {e|o|m|s|n}, data bits {7|8}, stop bits {1|2}.
               (default: n81)
 -P millisec   Number of milliseconds to wait before responding to a RING
               with the answer string (usually ATA). (default: 50)
 -r rings      Number of rings before answer. (default: 1)
 -R minutes    Number of minutes before resyncing the modem. (default: 15)
 -s startdelay Number of seconds to delay before starting command. (default: 3)
 -t tries      Number of times to try and dialout or callback. (default: 2)
 -T pppd_cmd   Command to start on a ppp call (default: none)
 -U user:pass:cmd  Userid and password to login as. Only valid with -d option.
#endif


/*
 * The whole PAUSE_ON_PICKUP / -P / pickup_pause thing was a wild-guess
 * attempt to fix alow's problem with comm not working with his internal
 * modem. The amazing thing - it worked. He used a value of 100ms.
 * Eric
 */

/*
 * This program was written to be a synchronous design. If you hack at
 * it please try and keep nasty signals and race conditions out of the code!
 */

/*--------------------------------------------------------------------------*/
#include <unistd.h>
#include <time.h>
#include <termios.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <signal.h>
#include <setjmp.h>
#include <process.h>
#include <fcntl.h>
#include <ctype.h>

#ifdef __QNXNTO__

/* sanity */
#if _FILE_OFFSET_BITS - 0 != 32
	#undef _FILE_OFFSET_BITS
	#define _FILE_OFFSET_BITS 32
#endif

#define PULSE_PROXY   _PULSE_CODE_MINAVAIL+1


#include <errno.h>
#include <sys/types.h>
#include <sys/dcmd_chr.h>
#include <sys/iomsg.h>
#include <sys/neutrino.h>

#define MAX_TTY_NAME 32

struct _dev_info_entry {
    short int       tty;
    pid_t           nid;
    short unsigned  driver_pid;
    short unsigned  driver_pid_spare;
    short unsigned  nature;
    short unsigned  attributes;
    short unsigned  capabilities;
    char            driver_type[16];
    char            tty_name[MAX_TTY_NAME];
    short unsigned          unit;
    short unsigned          open_count;
    pid_t          pgrp;
    short unsigned          pgrp_spare;
    pid_t          session;
    short unsigned          session_spare;
    short unsigned          flags;
    short unsigned          major;
};

/*
 * Dev_info flags bits
 */
#define _DEV_IS_READERS         0x0001
#define _DEV_IS_WRITERS         0x0002
#define _DEV_WILL_WINCH         0x0004
#define _DEV_WILL_FWD           0x0008
#define _DEV_NBLOCK_PROXY       0x0010

/*
 * Events recognized by dev_arm() and dev_state()
 */
#define _DEV_EVENT_INPUT        0x0001
#define _DEV_EVENT_DRAIN        0x0002
#define _DEV_EVENT_LOGIN        0x0004
#define _DEV_EVENT_EXRDY        0x0008
#define _DEV_EVENT_OUTPUT       0x0010
#define _DEV_EVENT_TXRDY        0x0020
#define _DEV_EVENT_RXRDY        0x0040
#define _DEV_EVENT_HANGUP       0x0080
#define _DEV_EVENT_INTR         0x0100
#define _DEV_EVENT_WINCH        0x0200

/*
 * Special "proxy" value to disarm pending armed proxies
 **/
#define _DEV_DISARM                     (-1)

/*
 * Modes recognized by dev_mode()
 */
#define _DEV_ECHO       0x0001
#define _DEV_EDIT       0x0002
#define _DEV_ISIG       0x0004
#define _DEV_OPOST      0x0008
#define _DEV_OSFLOW     0x0010
#define _DEV_MODES      (_DEV_ECHO|_DEV_EDIT|_DEV_ISIG|_DEV_OPOST|_DEV_OSFLOW)

int			dev_info( int __fd, struct _dev_info_entry *__info );
int			dev_fdinfo( pid_t __server, pid_t __pid,
				int __fd, struct _dev_info_entry *__info );
int			dev_insert_chars( int __fd, int __n, const char *__buf );
unsigned	dev_state( int __fd, unsigned __bits, unsigned __mask );
unsigned	dev_mode( int __fd, unsigned __mask, unsigned __mode );
int			dev_arm( int __fd, pid_t __proxy, unsigned __events );
int			dev_read( int __fd, void *__buf, unsigned __nbytes,
			unsigned __minimum, unsigned __time, unsigned __timeout,
			pid_t __proxy, int *__triggered );
int			dev_ischars( int __fd );
int			dev_size( int __fd, int __set_rows, int __set_columns,
				int *__rows, int *__cols );
int			dev_osize(int __fd, int __size);
int			dev_readex(int __fd, char *__buf, int __nbytes);
void		proxy_emulate(int signo);
void 		hold_off_modem(int signo);
void		strip_nl(char *buf);
int			go_raw(int fd);
int			go_edit(int fd);
int			cts(void);
int			carrier(void);
int			wait_for_proxy(void);
void		logmsg(char *file, char *str);

#else
#include <env.h>
#include <sys/dev.h>
#include <sys/qioctl.h>
#include <sys/timers.h>
#include <sys/proxy.h>
#include <sys/kernel.h>
#endif


#include <sys/stat.h>


/*-- General Definitions ---------------------------------------------------*/
#define	TRUE	1
#define	FALSE	0

// Exit status codes
// Note that codes >= 10 usually indicate that something must be done before
// retrying.
//
#define MODEM_OK		0	// All pages sent/received successfully
#define MODEM_BUSY		1	// The called number was busy
#define MODEM_INUSE		2	// The modem device is in use
#define MODEM_NOCARRIER	3	// No carrier
#define MODEM_TIMEOUT	8	// A timeout waiting for an expected modem response
#define MODEM_CMDLINE	10	// An error processing the command line
#define MODEM_PROXY		11	// Unable to attach a proxy
#define MODEM_EXEC		12	// Unable to exec
#define MODEM_DEVERR	13	// Error geting status from device manager
#define MODEM_MOPEN		14	// Error opening -m modem
#define MODEM_DOPEN		15	// Error opening -m modem

#ifdef __QNXNTO__
static char modem_version[] = "modem version: 0.1b EXPERIMENTAL";
static unsigned proxy_count = 0;
static struct sigevent ev;
static unsigned dev_events = 0;
static int chid;
static int coid;

/* String constants can't be modified (which parse() seems to do...grrr) */
char	*cmd;
char	*faxcmd;
int		notify_fd;
#else
char	*cmd = "/bin/login -p";			/* Default login command			*/
char	*faxcmd = "/bin/fax";			/* Default fax command				*/
#endif
char	*pppcmd;						/* pppd command						*/
char	callcmd[200];					/* Default callcmd					*/
jmp_buf	 env;							/* Environment for setjmp/longjmp	*/

char	*greeting_fname;				/* File containing greeting screen	*/
char	*hayes_answer = "ATA";			/* String to make modem answer		*/
char	*hayes_init = "ATZ|~AT";		/* Hayes init string				*/
char	*parity		= "n81";			/* Parity, data bits, stop bits		*/
char	*callback_fname;				/* File of id's and callback numbers*/
char	*userid;						/* Userid to login as				*/
char	*password;						/* Password to login as				*/
char	*remcmd;						/* A remote command to execute		*/
char	buf[200];						/* Scratch buffer for modem I/O		*/
char	callbuf[90];					/* Used to dial out callback number */
char	term;							/* Additional terminator for input	*/
int		calling;						/* We are calling back				*/
int		rings		= 1;				/* Answer on first ring				*/
int		tries		= 2;				/* Try callback or callout 2 times	*/
int		ring_cnt;						/* Number of rings pending			*/
int		startdelay	= 3;				/* Seconds after connect before login*/
int		got_caller_id;					/* We got and matched a caller id	*/
int		isfax;
int		isppp;
int		modem_fd = 0;					/* Used with -m and -d only			*/
char	*faxid;							/* Modem fax id (usually a phone #)	*/
pid_t	proxy;							/* Proxy used for dev_arm			*/
FILE	*debug_fp;						/* Send debug info out here			*/
unsigned resync		= 15;				/* Resync every 15 minutes			*/
speed_t	baud;							/* Connect baud rate				*/
speed_t	startbaud	= 2400;				/* Default baud rate				*/
speed_t	assumebaud	= 1200;				/* Assume "connect" baud rate		*/
char	cts_chk		= TRUE;				/* Default to a CTS check			*/
char	fidonet		= FALSE;			/* Fidonet node is running this comm*/
char	lockedbaud	= TRUE;				/* Not a locked baudrate modem		*/
char	recycle		= TRUE;				/* Recycle the modem if inactive	*/
struct _dev_info_entry devinfo;			/* Used to hold device name			*/

long	pickup_pause = 50L;				/* #millisec to pause b4 picking up */
int		strict_open_count = FALSE;		/* Strict opencount checking on tty */

/*-- Prototypes ------------------------------------------------------------*/
time_t		get_modem(unsigned, unsigned);
void		put_modem(char *);
void		set_line(speed_t, char *, unsigned);
speed_t		extract_baud(char *);
long		number(char *, char *);
void		debugmsg(const char *, ...);
void		sleepms(long msec);
void		proxyms(long msec);
char	  **parse(char *);
int			callback_lookup(char *, int);

int main( argc, argv )
	int		argc;
	char	*argv[];
	{
	extern void sigterm();
	int		opt, c, i;
	FILE	*fp;

#ifdef __QNXNTO__
	cmd = strdup("/bin/login -p");
	faxcmd = strdup("/bin/fax");
#endif

	while((opt = getopt(argc, argv, "a:b:B:c:Cd:D:f:F:g:i:I:Lm:op:r:R:s:P:t:T:U:")) != -1) {
		switch(opt) {
		case 'a':
			hayes_answer = optarg;
			break;
		case 'b':
			startbaud = number("-b", optarg);
			break;
		case 'B':
			assumebaud = number("-B", optarg);
			break;
		case 'c':
			strcpy(callcmd, cmd = optarg);
			break;
		case 'C': 
			cts_chk = FALSE;
			break;
		case 'd':
			sprintf(callbuf, "ATDT%s", optarg);
			calling = tries + 1;
			break;
		case 'D':
			if(strcmp(optarg, "-") == 0)
				debug_fp = stderr;
			else {
				if((debug_fp = fopen(optarg, "a")) == NULL) {
					fprintf(stderr, "Unable to open %s: %s\n", optarg, strerror(errno));
					exit(MODEM_DOPEN);
					}
				fcntl(fileno(debug_fp), F_SETFD, FD_CLOEXEC);
				}
			break;
		case 'f':
			faxcmd = optarg;
			break;
		case 'F':
			faxid = optarg;
			break;
		case 'g':
			greeting_fname = optarg;
			break;
		case 'i':
			hayes_init = optarg;
			break;
		case 'I':
			callback_fname = optarg;
			break;
		case 'l':
			lockedbaud = FALSE;
			break;
		case 'L':
			lockedbaud = TRUE;
			break;
		case 'm':
			modem_fd = open(optarg, O_RDWR, 0);
			if(modem_fd == -1) {
				fprintf(stderr, "Unable to open %s: %s\n", optarg, strerror(errno));
				exit(MODEM_MOPEN);
				}
			fcntl(modem_fd, F_SETFD, FD_CLOEXEC);
			if(callcmd[0] == '\0')
				sprintf(callcmd, "/bin/qtalk -m %s", optarg);
			break;
		case 'o':
			strict_open_count = 1;
			break;
		case 'p':
			parity = optarg;
			break;
		case 'P':
			pickup_pause = atol(optarg);
			break;
		case 'r':
			rings = number("-r", optarg);
			break;
		case 'R':
			resync = number("-R", optarg);
			break;
		case 's':
			startdelay = number("-s", optarg);
			break;
		case 't':
			tries = number("-t", optarg);
			break;
		case 'T':
			pppcmd = optarg;
			break;
		case 'U':
			userid = optarg;
			if(password = strchr(userid, ':'))
			{
				*password++ = '\0';
				if(remcmd = strchr(password, ':'))
					*remcmd++ = '\0';
				else
					remcmd = "";
			}
			else
				password = remcmd = "";
			break;
		default:
			exit(MODEM_CMDLINE);
			}
		}

	// Callback info comes after the first ring.
	if(callback_fname  &&  rings < 2)
		rings = 2;

	setsid();			/* Make sure we are a session leader */

#ifdef __QNXNTO__
	SIGEV_SIGNAL_INIT(&ev, SIGUSR1);
	if((chid = ChannelCreate(0)) == -1) {
		if (debug_fp) debugmsg("Channel: create failure %d\n", errno);
		exit(MODEM_PROXY);
	}
	if((coid = ConnectAttach(0, getpid(), chid, 0|_NTO_SIDE_CHANNEL, _NTO_COF_CLOEXEC)) == -1) {
		if (debug_fp) debugmsg("Connect: attach failure %d\n", errno);
		exit(MODEM_PROXY);
	}
	signal(SIGUSR1, proxy_emulate);
	signal(SIGUSR2, hold_off_modem);
#else
	if((proxy = qnx_proxy_attach(0, 0, 0, -1)) == -1) {
		int en = errno;
		if(debug_fp)
			debugmsg("PROXY: Unable to attach - [%d] %s\n",en, strerror(en));
		exit(MODEM_PROXY);
		}
#endif

	for (i = 15; i; i--) {
		if (dev_info(modem_fd, &devinfo) == -1) {
			if (debug_fp)
				debugmsg("DEVINFO: Unable to query\n");
			exit(MODEM_DEVERR);
			}
		/* make sure nobody else is still using the port */
#ifdef __QNXNTO__
		else if ((devinfo.open_count > 1) && strict_open_count)
#else
		else if (devinfo.open_count > 1)
#endif
		{
			if (debug_fp) debugmsg("Open count is %d...waiting for release..\n"
							,devinfo.open_count);
			sleep(2);
		} else
			break;
	}
	if (i == 0)
		exit(MODEM_INUSE);

	/*
	 * In case the modem bounces its CD line or sends a burst of junk
	 * we ignore these signals. Just before we exec we will re-enable
	 * them and poll the CD line to make sure we still have carrier.
	 */
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	/* Signal to terminate modem */
	signal(SIGTERM, &sigterm);

restart1:
	setjmp(env);
	if(debug_fp)
		debugmsg("STATUS: Restart\n");

	dev_mode(modem_fd, 0, _DEV_MODES);		/* Go raw						*/
	set_line(startbaud, parity, lockedbaud);/* Set the serial line parms	*/
	tcdropline(modem_fd, 2000);				/* Drop the line for 2 seconds	*/
	sleep(1);
	tcflush(modem_fd, TCIOFLUSH);			/* Flush all I/O				*/
	term = 0;

#ifdef __QNXNTO__
	proxy_count = 0;
	dev_events &= ~_DEV_EVENT_LOGIN;
#endif

	if(cts_chk) {
		int off = FALSE;

		while(!cts()) {
			sleep(1);
			if(debug_fp  &&  off)
				debugmsg("STATUS: No CTS\n");
			off = TRUE;
			}

		if(debug_fp  &&  off)
			debugmsg("STATUS: CTS on\n");
		}

	for(i = 0 ; i < 5 ; ++i) {				/* Get modems attention			*/
#ifndef use_put_modem
		write(modem_fd, "\r", 1);	sleepms(100L);
		write(modem_fd, "A", 1);	sleepms(100L);
		write(modem_fd, "T", 1); 	sleepms(100L);
		write(modem_fd, "\r", 1);	sleepms(100L);
#else
		put_modem("\rAT");
#endif
		get_modem(40, 0);					/* Wait up to 4 seconds for "ok"*/
		if(strstr(buf, "ok"))
			break;
		}

	if(debug_fp  &&  i >= 5)
		debugmsg("STATUS: Did not get OK response from modem\n");

	put_modem(hayes_init);
	get_modem(10, 0);					/* Wait up to 1 second for "ok"*/
	sleep(1);
	if(faxid) {
		sprintf(buf, "AT+FCLASS=2|AT+FAA=%d +FCR=1 +FLID=\"%-20.20s\"",
					*cmd ? 1 : 0, faxid);
		put_modem(buf);
		}
	get_modem(10, 0);					/* Wait up to 1 second for "ok"*/
	sleep(1);
	tcflush(modem_fd, TCIFLUSH);
	if(calling == 0) {
		debugmsg("Waiting for proxy\n");
		wait_for_proxy();
	}

restart2:
	got_caller_id = 0;
	if(calling  &&  --calling == 0)
		exit(MODEM_BUSY);

	if(calling)
		put_modem(callbuf);

	for(ring_cnt = 0 ; rings && !calling;) {/* Wait for rings...			*/
		get_modem(200, 1);					/* Read modem msg				*/

		if(strstr(buf, "ring")) {
			if(++ring_cnt >= rings) {
				break;
				}
			}
		else if(callback_fname  &&  (strstr(buf, "nmbr ="))) {
			if(callback_lookup(strchr(buf, '=') + 1, 1)) {
				got_caller_id = 1;
				break;
				}
			}
		else if(strstr(buf, "no carrier"))
			goto restart1;
		}

	if (pickup_pause)
		sleepms(pickup_pause);
	if(!calling)
		put_modem(hayes_answer);

	for(baud = 0 ;;) {
		get_modem(400, 1);					/* Read modem msg				*/

		if(strstr(buf, "connect")) {
			if(baud == 0)
				baud = extract_baud(buf);
			if(!lockedbaud)
				set_line(baud, parity, lockedbaud);
			break;
			}
		else if(strstr(buf, "+fcon")) {
			isfax = 1;
			break;
			}
		else if(strstr(buf, "no carrier"))
			goto restart1;
		else if(strstr(buf, "no answer"))
			goto restart1;
		else if(strstr(buf, "no dialtone"))
			goto restart1;
		else if(strstr(buf, "+fhng"))
			goto restart1;
		else if(strstr(buf, "carrier"))
			baud = extract_baud(buf);
		else if(!calling  &&  strstr(buf, "ring"))
			goto restart2;
		else if(strstr(buf, "busy"))
			goto restart1;
		}

	sleep(startdelay);
	if(!isfax  &&  !calling) {
		if(pppcmd) {
			static unsigned char ppphdr[3] ={ 0x7e, 0xff, 0x7d };
			unsigned char c;
			int match = 0;

			for(i = 0 ; i < 15;)
				if(dev_read(modem_fd, &c, 1, 1, 0, 1, 0, 0) == 1) {
					if(debug_fp)
						debugmsg("STATUS: PPP check saw %2.2x\n", c);
					if(c == ppphdr[match]) {
						if(++match == 3) {
							isppp = 1;
							break;
							}
						}
					else
						match = 0;
					}
				else {
					++i;	// Allow 10 timeouts then give up.
					if(debug_fp)
						debugmsg("STATUS: PPP check timeout (%d)\n", i);
					}
		}

		tcflush(0, TCIFLUSH);	/* Flush all input						*/

		if(!isppp  &&  greeting_fname)
			if(fp = fopen(greeting_fname, "r")) {
				while(carrier()  &&  (c = getc(fp)) != EOF) {
					if(c == '\n')
						putchar(0x0d);	/* Expand newline into CR LF	*/
					putchar(c);
					}
				fflush(stdout);
				fclose(fp);
				}
	}

	signal(SIGHUP, SIG_DFL);				/* Restore default			*/
	signal(SIGINT, SIG_DFL);				/* Restore default			*/

	if(!carrier()) {
		if(debug_fp)
			debugmsg("STATUS: Lost carrier\n");
		goto restart1;
		}

	sprintf(buf, "%ld", baud);
	setenv("BAUD", buf, 1);

	if(calling)
		sprintf(buf, "%10ld CA %s %s\n",
				(long)time(NULL), ctermid(NULL), callbuf);
	else
		sprintf(buf, "%10ld MO %s %ld\n",
				(long)time(NULL), ctermid(NULL), baud);
	logmsg("/etc/acclog", buf);

	debugmsg("Doing login stuff\n");
	term = ':';
	while(calling  &&  userid) {
		char c = '\n';

		get_modem(200, 1);				/* Read modem msg				*/

		if(strstr(buf, "login")) {
			write(modem_fd, userid, strlen(userid));
			write(modem_fd, &c, 1);
			if (*password == '\0') {
				if (*remcmd == '\0')
					break;
/*
	If we do get prompted for a passwd, the following may cause get_modem to
	time out.  Guess this is ok since a passwd wasn't provided.
*/
				term = ' ';
				}
			}
		else if(strstr(buf, "password")) {
			write(modem_fd, password, strlen(password));
			write(modem_fd, &c, 1);
			if (*remcmd == '\0')
				break;
/* 
	Since the passwd prompt doesn't end with a space (login prompt does
   at time of writing), if the passwd is wrong, the following should 
   cause get_modem() to time out on next passwd prompt (can't change 
   it on the fly anyway).
*/
			term = ' ';  
			}
/* 
   Check for the standard prompts (space stripped of by get_modem()) 
*/
		else if(term == ' ' && (buf[strlen(buf)-1] == '#' || buf[strlen(buf)-1] == '$')) { 
			write(modem_fd, remcmd, strlen(remcmd));
			write(modem_fd, &c, 1);
			break;
			}
		else if(strstr(buf, "no carrier"))
			goto restart1;
	}
	debugmsg("Login stuff done\n");
	if(!isfax  &&  !isppp)
		dev_mode(modem_fd, _DEV_MODES, _DEV_MODES);	/* Go edited	*/

	if(!isppp  &&  !calling  &&  !got_caller_id  &&  callback_fname) {
		int trys = 2;
		for(;;) {
			if(trys-- == 0)
				goto restart1;
			printf("Callback userid: ");
			fflush(stdout);
			fgets(buf, sizeof(buf), stdin);
			strip_nl(buf);
			if(callback_lookup(buf, 0)) {
				if(strcmp(callbuf, "*") == 0)	/* Wildcard in file lets you in */
					break;
				calling = tries + 1;
				signal(SIGHUP, SIG_IGN);
				signal(SIGINT, SIG_IGN);
				goto restart1;
				}
			}
		}
	debugmsg("Finishing up...\n");
	if(isfax)
		argv = parse(faxcmd);
	else if(isppp)
		argv = parse(pppcmd);
	else {
		debugmsg("parsing...\n");
		argv = parse(calling &&  callcmd[0] ? callcmd : cmd);
	}
	debugmsg("execing...\n");
	execvp(argv[0], argv);
	debugmsg("Oppps\n");

	if(debug_fp)
		debugmsg("STATUS: Exec failed\n");

	exit(MODEM_EXEC);
	}


int callback_lookup(char *str, int callid) {
	char *cp, term;
	FILE *fp;

	if((fp = fopen(callback_fname, "r")) == NULL) {
		if(debug_fp)
			debugmsg("CALLBACK: Unable to open callback file.\n");
		return(0);
		}

	if(callid)
		while(*str == ' '  ||  *str == '\t')
			++str;		// Skip over leading white space

	while(fgets(callbuf, sizeof(callbuf), fp)) {
		strip_nl(callbuf);
		for(cp = callbuf ; (term = *cp)  &&  *cp != ':' ; ++cp)
			;
		*cp = '\0';
		if(term)
			while(*++cp == ' '  ||  *cp == '\t')
				;	// Skip over leading white space

		if(callid) {
			while(*cp  && (*cp != ' '))
				++cp;	// Skip over to the number area (separated by a space)
			if(*cp)
				++cp;	// Skip over space
			}

		if(strcmp(str, callid ? cp : callbuf) == 0) {
			strcpy(callbuf, cp);
			fclose(fp);
{
char *buf = alloca(strlen(callback_fname) + 4 + 1);

strcat(strcpy(buf, callback_fname), ".bad");
if(fp = fopen(buf, "a")) {
	fprintf(fp, "%d %s\n", callid, str);
	fclose(fp);
	}
}
			return(1);
			}
		}

	fclose(fp);

	{
	char *buf = alloca(strlen(callback_fname) + 4 + 1);

	strcat(strcpy(buf, callback_fname), ".bad");
	if(fp = fopen(buf, "a")) {
		fprintf(fp, "%d %s\n", callid, str);
		fclose(fp);
		}
	else
		if(debug_fp) debugmsg("Unable to open %s : %s\n", buf, strerror(errno));
	}

	return(0);
	}



time_t
get_modem(timeout, doreset)
unsigned timeout, doreset;
	{
	time_t t = time(NULL);
	char *s = buf;
	int n;

	for(;;) {
		n= dev_read(modem_fd, s, 1, 1, 0, timeout, 0, 0);
		if(n == 0) {
			if(doreset)
				longjmp(env, 0);
			else
				break;
			}

		/* Strip top bit and check for CR	*/
		if((*s &= ~0x80) == 0x0d  ||  (term  &&  *s == term)) {
			if(s == buf)			/* Ignore empty lines				*/
				continue;
			break;
			}
		if(*s < ' ')				/* Ignore control characters		*/
			continue;
		*s = tolower(*s);			/* Convert to a common lower case	*/

		if(s < &buf[sizeof buf])
			++s;
		}

	*s = '\0';						/* Terminate buffer					*/

	if(debug_fp)
		debugmsg("INPUT:  \"%s\" %ld\n", buf, time(NULL) - t);

	return(time(NULL) - t);			/* Return elapsed time				*/
	}



void
put_modem(str)
char *str;
	{
	char c;
	int stop = FALSE;

	if(debug_fp)
		debugmsg("EMIT:   \"%s\"\n", str);

	while(!stop) {
		switch(c = *str++) {
			case '\0':	stop = TRUE;					/* fall through */
			case '|':	c = '\r';						break;
			case '\\':	c = *str++;						break;
			case '~':	sleepms(1000L);					continue;
			case '^':	tcdropline(modem_fd, 1000);		continue;
			case '!':	tcsendbreak(modem_fd, 500);		continue;
			case '-':	sleepms(100L);					continue;
			}

		write(modem_fd, &c, 1);
		if(c != 0x0d) {				/* Attempt to read echo of character	*/
			dev_read(modem_fd, &c, 1, 1, 0, 3, 0, 0);	/* Wait up to .3 sec		*/
			tcflush(modem_fd, TCIFLUSH);	/* Flush all input						*/
			}
		else {
			sleep(1);				/* Pause 1 second before looping		*/
			if(!stop  &&  *str)		/* Flush input if more commands			*/
				tcflush(modem_fd, TCIFLUSH);
			}
		}
	}


void strip_nl(char *buf) {
	int i;

	if(buf[i = strlen(buf) - 1] == '\n')
		buf[i] = '\0';
	}



speed_t
extract_baud(buf)
char *buf;
	{
	speed_t baud;

	debugmsg("extract_baud");
	while (!isdigit(*buf)) buf++;
	baud = strtol(buf, &buf, 10);
	if (baud == 0) {
		if (strstr(buf, "fast"))
			return B19200;
		return assumebaud;
		}
	return baud;
	}


void
set_line(speed_t baud, char* par, unsigned hflow)
	{
	struct termios termio;

	tcgetattr(modem_fd, &termio);
	termio.c_cflag &= ~(PARENB|PARODD|PARSTK|CSIZE|CSTOPB);

	switch(par[0]) {
		case 'e':	termio.c_cflag |= PARENB;
					break;
		case 'o':	termio.c_cflag |= PARENB | PARODD;
					break;
		case 'm':	termio.c_cflag |= PARENB | PARODD | PARSTK;
					termio.c_lflag |= IEXTEN;
					break;
		case 's':	termio.c_cflag |= PARENB | PARSTK;
					termio.c_lflag |= IEXTEN;
					break;
		case 'n':	break;
		}

	switch(par[1]) {
		case '7':	termio.c_cflag |= CS7;
					break;
		case '8':	termio.c_cflag |= CS8;
					break;
		}

	switch(par[2]) {
		case '1':	break;
		case '2':	termio.c_cflag |= CSTOPB;
					break;
		}

	if(hflow) {
		termio.c_cflag |= (IHFLOW|OHFLOW);
		termio.c_lflag |= IEXTEN;
		}

	cfsetispeed(&termio, baud);
	cfsetospeed(&termio, baud);

	tcsetattr(modem_fd, TCSANOW, &termio);
	}


int carrier() {
#ifdef __QNXNTO__
	unsigned msr;
	int retval;

	retval = devctl(modem_fd, DCMD_CHR_LINESTATUS, &msr, sizeof(msr), NULL);
	if (retval != EOK) {
		errno = retval;
		return -1;
	}

	return (msr & _LINESTATUS_SER_CD);
#else
	int i;
	long bits[2];

	for(i = 0 ; i < 10 ; ++i) {	/* Wait up to 5 seconds for carrier	*/
		bits[0] = bits[1] = 0;	/* Don't change anything			*/
		qnx_ioctl(modem_fd, QCTL_DEV_CTL, bits, sizeof(bits), bits, sizeof(bits));
		if(bits[0] & (1L << 23))
			return(1);
		sleepms(500L);
		}

	return(0);
#endif
	}


int cts() {
#ifdef __QNXNTO__
	unsigned msr;
	int retval;

	
	retval = devctl(modem_fd, DCMD_CHR_LINESTATUS, &msr, sizeof(msr), NULL);
	if (retval != EOK) {
		errno = retval;
		return -1;
	}
	
	return (msr & _LINESTATUS_SER_CTS);
	
#else
	long bits[2];

	bits[0] = bits[1] = 0;	/* Don't change anything */
	qnx_ioctl(modem_fd, QCTL_DEV_CTL, bits, sizeof(bits), bits, sizeof(bits));

	return((bits[0] & (1L << 20)) != 0);
#endif
	}


void
sleepms(long msec)
	{
#ifdef __QNXNTO__
	struct timespec timer;

	timer.tv_sec     = msec/1000L;
	timer.tv_nsec    = (msec%1000L) * 1000000L;

	/* NYI: 	
			what if a signal/timeout happens... 
			should we resleep for the rest?  Probably not.
	*/
	nanosleep(&timer, NULL);
#else
	timer_t tid;
	struct itimerspec timer;

	tid = mktimer(TIMEOFDAY, _TNOTIFY_SLEEP, 0);
	timer.it_value.tv_sec     = msec/1000L;
	timer.it_value.tv_nsec    = (msec%1000L) * 1000000L;
	timer.it_interval.tv_sec  = 0L;
	timer.it_interval.tv_nsec = 0L;

	reltimer(tid, &timer, NULL);
#endif
	}


void
proxyms(long msec)
	{
	static timer_t tid;
	struct itimerspec timer;
#ifdef __QNXNTO__
	timer_create(CLOCK_REALTIME, &ev, &tid);
#else
	struct itimercb timercb;

	/*
	 *  NOTE: Setting it_value to 0 cancels the timer
	 * [921006 gbbell]
	 */

	if(tid == 0) {
		timercb.itcb_event.evt_value = proxy;
		tid = mktimer(TIMEOFDAY, _TNOTIFY_PROXY, &timercb);
		}
#endif
	timer.it_value.tv_sec     = msec/1000L;
	timer.it_value.tv_nsec    = (msec%1000L) * 1000000L;
	timer.it_interval.tv_sec  = 0L;
	timer.it_interval.tv_nsec = 0L;
#ifdef __QNXNTO__
	timer_settime(tid, 0, &timer, NULL);
#else
	reltimer(tid, &timer, NULL);
#endif
	}


long
number(opt, str)
char *opt, *str;
	{
	long n;
	char *endptr;

	n = strtol(str, &endptr, 10);

	if(*endptr != '\0') {
		sprintf(buf, "%10ld modem: Invalid numeric argument to %s.\n",
				(long)time(NULL), opt);
		logmsg("/etc/syslog", buf);
		exit(MODEM_CMDLINE);
		}

	return(n);
	}


void
logmsg(file, str)
char *file, *str;
	{
	int fd;

	if((fd = open(file, O_WRONLY|O_APPEND)) != -1) {
		write(fd, str, strlen(str));
		close(fd);
		}
	}


void
debugmsg(const char *format, ...) {
	va_list arglist;

	fprintf(debug_fp, "%8.8lX %s ", (long)time(NULL), ctermid(NULL));

	va_start(arglist, format);
	vfprintf(debug_fp, format, arglist);

	fflush(debug_fp);
	}


char **
parse(p)
char *p;
{
	char **argv = 0;
	int i = 0;

	while (*p) {
		char delim = (*p == '"') ? *p++ : ' ';

		argv = realloc(argv, (i+2) * sizeof *argv);
		argv[i++] = p;
		while (*p && *p != delim) p++;	/* collect arg */
		if (*p) *p++ = 0;				/* nul delimit */
		if (debug_fp) debugmsg("argv[%d] is '%s'\n", i-1, argv[i-1]);
		while (*p == ' ') p++;			/* skip spaces */
		}
	argv[i] = 0;
	return argv;
	}


/*
 * QNX4:
 * We arm Dev to kick us with a proxy on a login event. A login event
 * is the arrival of data when nobody has the device open. We then
 * close the device and wait for the proxy to kick us. Why not just
 * issue a read and wait you say? Well, this method allows programs
 * like qtalk and uucp (outgoing) to open the device (which will stop
 * modem from getting kicked thus stealing the port) and checking for
 * an open count of 1 (meaning qtalk/uucp is the only program using the
 * device). The net effect means that you can share a single line for
 * incomming (modem) and outgoing (qtalk/uucp) calls. The programs
 * effectively semiphore the line using Dev's open count and the login
 * event processing.
 *
 * QNX6:
 * We arm the TTY to return an event (SIGUSR1) which will trigger a
 * pulse onto ourselves (fake proxy).  We don't have the ability to
 * receive incomming notification of data, without an open connection
 * to the TTY (ie. serial device etc).  This means the open count will
 * be none zero if another program attempts to access the device at the
 * same time.
 */

int wait_for_proxy() {

again:
//	fchown(0, 0, 0);
//	fchmod(0, 0666);

	dev_arm(0, proxy, _DEV_EVENT_LOGIN);

#ifdef __QNXNTO__
	{
	struct _pulse proxy_pulse;
	int retval;
	
	while ((retval = MsgReceivePulse(chid, &proxy_pulse, sizeof proxy_pulse, NULL)) == -1) {
		if (errno == ETIMEDOUT) {
			if(debug_fp) debugmsg("PROX: Timeout resync\n");
			longjmp(env,0);
		} else if (errno == EINTR) {
			if(debug_fp) debugmsg("PROX: Signal wacked!\n");
			continue;
		} else {
			if (debug_fp) debugmsg("PROX: Unblocked error (%d)\n", errno);
		}
	}
	proxy_count-= (proxy_count) ? 1 : 0;
	
	}
#else
	/*
	 * Setup overall timeout
	 */
	proxyms(resync * 60000L);
	close(0); close(1); close(2);
	Receive(proxy, 0, 0);
	proxyms(0L);  /* Cancel timer [921006 gbbell] */

	if(open(devinfo.tty_name, O_RDWR, 0) == -1	/* Open stdin	*/
	|| dup(0) == -1								/* Dup stdout	*/
	|| dup(0) == -1) {							/* Dup stderr	*/
		dev_arm(0, _DEV_DISARM, _DEV_EVENT_LOGIN);
		exit(MODEM_DEVERR);
		}
#endif

	if(dev_info(0, &devinfo) == -1  ||  (strict_open_count && (devinfo.open_count > 1))) {
		dev_arm(0, _DEV_DISARM, _DEV_EVENT_LOGIN);
		if (debug_fp) debugmsg("PROXY: Device '%s' in use %d.\n", devinfo.tty_name,devinfo.open_count);
		goto again;
		}

	/*
	 * Once stdin/stdout/stderr are successfully re-opened, reenable
	 * breaks and hangups making the tty a controlling tty (again)
	 * [921006 gbbell]
	 */

	tcsetpgrp( 0, getpgrp() );
#ifdef __QNXNTO__
	tcsetsid(0, getpid() );
#else
	tcsetct( 0, getpid() );

	if((dev_state(0, 0, _DEV_EVENT_LOGIN) & _DEV_EVENT_LOGIN) == 0) {
		dev_arm(0, _DEV_DISARM, _DEV_EVENT_LOGIN);
		if(debug_fp) debugmsg("PROXY: Resync timeout\n");
		longjmp(env, 0);
		}
#endif

	dev_mode(0, 0, _DEV_MODES);					/* Go raw		*/
	dev_arm(0, _DEV_DISARM, _DEV_EVENT_LOGIN);
	return 0;
	}


void
sigterm() {

	open(devinfo.tty_name, O_RDWR, 0);	/* Incase stdin was closed */
	dev_arm(0, _DEV_DISARM, _DEV_EVENT_LOGIN);

	exit(MODEM_OK);
	}

#ifdef __QNXNTO__
/* Neutrino Support functions */


void proxy_emulate(int signo) {
	proxy_count++;
	/* assume since we got wacked it was for a login event */
	dev_events |= _DEV_EVENT_LOGIN;
//	sem_post(&proxy_sem);
	if (debug_fp) debugmsg("PROXY: Got a proxy!\n");
	MsgSendPulse(coid,getprio(getpid()), PULSE_PROXY, _DEV_EVENT_LOGIN);
}

void hold_off_modem(int signo) {
	sigset_t sig_set;
	int retsig;

	close(0);
	close(1);
	close(2);
	sigemptyset(&sig_set);
	sigaddset(&sig_set, SIGUSR2);	
	if (debug_fp) debugmsg("HELD: trigger again to restart.\n");
	while ((retsig=sigwaitinfo(&sig_set, NULL)) != SIGUSR2) {
		if (debug_fp) debugmsg("HELD: Still waiting %d\n", retsig);
	}
	if (debug_fp) debugmsg("HELD: Modem hold released\n");
	exit(MODEM_OK);
}

int go_raw(int fd) {
	struct termios termios_p;

	if( tcgetattr( fd, &termios_p ) )
		return( -1 );

	termios_p.c_cc[VMIN]  =  1;
	termios_p.c_cc[VTIME] =  0;
	termios_p.c_lflag &= ~( ECHO|ICANON|ISIG|
		ECHOE|ECHOK|ECHONL );
	termios_p.c_oflag &= ~(OPOST|ONLCR);
	termios_p.c_iflag &= ~(ICRNL);
	return( tcsetattr( fd, TCSADRAIN, &termios_p ) );
}

int go_edit(int fd) {
	struct termios termios_p;

	if( tcgetattr( fd, &termios_p ) )
		return( -1 );

	termios_p.c_lflag |= ( ECHO|ICANON|ISIG|
		ECHOE|ECHOK|ECHONL );
	termios_p.c_oflag |= (OPOST|ONLCR);
	termios_p.c_iflag |= ICRNL;
	return( tcsetattr( fd, TCSADRAIN, &termios_p ) );
}

int dev_info( int __fd, struct _dev_info_entry *__info ) {
	int retval;
	struct _ttyinfo tinfo;
	struct _fdinfo info;

	retval = devctl(__fd, DCMD_CHR_TTYINFO, &tinfo, sizeof(tinfo), NULL);

	if (retval != EOK) {
		errno = retval;
		return -1;
	}
	retval = iofdinfo(__fd, 0, &info, __info->tty_name, MAX_TTY_NAME);

	//__info->open_count = tinfo.opencount;
	strcpy(__info->tty_name, tinfo.ttyname);
	__info->open_count = max(info.rcount, info.wcount);

	return 0;
}

int dev_arm( int __fd, pid_t __proxy, unsigned __events ) {
/* dev_arm(0, proxy, _DEV_EVENT_LOGIN); */
/* dev_arm(0, _DEV_DISARM, _DEV_EVENT_LOGIN); */
	int retval;
	if (__proxy == _DEV_DISARM) {
		retval = ionotify(__fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_INPUT, NULL);
		if (retval == -1) {
			if (debug_fp) debugmsg("ionotify disarm failed %d",errno);
			exit(1);
		}
		dev_events &= ~_DEV_EVENT_LOGIN;
	
	} else {
		retval = ionotify(__fd, _NOTIFY_ACTION_POLLARM, _NOTIFY_COND_INPUT, &ev);
	}
	return retval;
}
int dev_read( int __fd, void *__buf, unsigned __nbytes,
		unsigned __minimum, unsigned __time, unsigned __timeout,
		pid_t __proxy, int *__triggered ) {

		/* not supported */
		if (__proxy || __triggered) {
			errno = ENOSYS;
			return -1;
		}

		dev_events &= ~_DEV_EVENT_LOGIN; /* turn off the LOGIN event bit on the device */
		return readcond(__fd, __buf, __nbytes, __minimum, __time, __timeout);
}
unsigned dev_state( int __fd, unsigned __bits, unsigned __mask ) {
/* dev_state(0, 0, _DEV_EVENT_LOGIN) */
	if (__mask != _DEV_EVENT_LOGIN) {
		if (debug_fp) debugmsg("dev_state misuse for NTO\n");
		perror("Bad use, breaks assumptions in emulation");
		exit(1);
	}
	return dev_events;
}

unsigned dev_mode( int __fd, unsigned __mask, unsigned __mode ) {
	/* returns old mode */
	/*	need to support only 'go raw' and 'go edit' via
		dev_mode(fd, 0, _DEV_MODES); == raw
		dev_mode(fd, _DEV_MODES, _DEV_MODES); == edit
	*/
	if (__mode == _DEV_MODES) {
		if (__mask == _DEV_MODES) {
			go_edit(__fd);
		} else if (__mask == 0) {
			go_raw(__fd);
		}
	} else { 
		if (debug_fp) debugmsg("dev_mode misuse for NTO\n");
		perror("Bad use of emulated dev_mode");
		exit(1);
	}

	return 0;
}
#endif
