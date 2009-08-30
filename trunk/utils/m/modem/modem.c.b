#ifdef __USAGE
%C - modem handler to answer calls (QNX)

%C	[options]
Options:
 -a string     String to answer the phone. (default: "ATA")
 -b baud       Baud rate. (default: 2400)
 -B baud       Baud rate to assume for "connect" without a rate. (default 1200)
 -c command    Command to start for a data call. (default: /bin/login -p)
 -C            Don't wait for CTS.
 -d [P]number  Dial this number and attempt to connect to a remote modem.
 -D file       Debug mode.
 -f command    Command to start for a fax call. (default: /bin/fax re)
 -F fax_id     Local fax machine identification. (default: "Unknown")
 -g file       Greeting message to print on connect.
 -i string     Initalize modem. (default: "ATZ|~AT")
 -I file       Identification file for call-id/callback processing.
 -L            Locked baud rate.
 -m modem      Serial line to connect to. Only valid with -d option.
               Changes default cmd (-c) to /bin/qtalk -m modem.
 -p parity     Parity {e|o|m|s|n}, data bits {7|8}, stop bits {1|2}.
               (default: n81)
 -P millisec   Number of milliseconds to wait before responding to a RING
               with the answer string (usually ATA). (default: 50)
 -r rings      Number of rings before answer. (default: 1)
 -R minutes    Number of minutes before resyncing the modem. (default: 15)
 -s startdelay Number of seconds to delay before starting command. (default: 3)
 -t tries      Number of times to try and dialout or callback. (default: 2)
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
#include <env.h>
#include <ctype.h>

#include <sys/stat.h>
#include <sys/dev.h>
#include <sys/qioctl.h>
#include <sys/timers.h>
#include <sys/proxy.h>
#include <sys/kernel.h>

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
#define MODEM_MOPEN		13	// Error opening -m modem

char	*cmd = "/bin/login -p";			/* Default login command			*/
char	*faxcmd = "/bin/fax";			/* Default fax command				*/
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
char	lockedbaud	= FALSE;			/* Not a locked baudrate modem		*/
char	recycle		= TRUE;				/* Recycle the modem if inactive	*/
struct _dev_info_entry devinfo;			/* Used to hold device name			*/

long	pickup_pause = 50L;				/* #millisec to pause b4 picking up */

/*-- Prototypes ------------------------------------------------------------*/
time_t		get_modem(unsigned, unsigned);
void		put_modem(char *);
void		set_line(speed_t, char *, int);
speed_t		extract_baud(char *);
long		number(char *, char *);
void		debugmsg(const char *, ...);
void		sleepms(long msec);
void		proxyms(long msec);
char	  **parse(char *);
int			callback_lookup(char *, int);

main( argc, argv )
	int		argc;
	char	*argv[];
	{
	extern void sigterm();
	int		opt, c, i;
	FILE	*fp;

	while((opt = getopt(argc, argv, "a:b:B:c:Cd:D:f:F:g:i:I:Lm:p:r:R:s:P:t:U:")) != -1) {
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
				debug_fp = fopen(optarg, "a");
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
		case 'L':
			lockedbaud = TRUE;
			break;
		case 'm':
			modem_fd = open(optarg, O_RDWR, 0);
			fcntl(modem_fd, F_SETFD, FD_CLOEXEC);
			if(modem_fd == -1) {
				fprintf(stderr, "Unable to open %s : %s\n", optarg, strerror(errno));
				exit(MODEM_MOPEN);
				}
			if(callcmd[0] == '\0')
				sprintf(callcmd, "/bin/qtalk -m %s", optarg);
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
		case 'U':
			userid = optarg;
			if(password = strchr(userid, ':'))   
				*password++ = '\0';
			if(remcmd = strchr(password, ':'))
				*remcmd++ = '\0';
			break;
		default:
			exit(MODEM_CMDLINE);
			}
		}

	setsid();			/* Make sure we are a session leader */
	if((proxy = qnx_proxy_attach(0, 0, 0, -1)) == -1) {
		if(debug_fp)
			debugmsg("PROXY: Unable to attach\n");
		exit(MODEM_PROXY);
		}

	for (i = 15; i; i--) {
		if (dev_info(modem_fd, &devinfo) == -1) {
			if (debug_fp)
				debugmsg("DEVINFO: Unable to query\n");
			exit(MODEM_DEVERR);
			}
		/* make sure nobody else is still using the port */
		else if (devinfo.open_count != 1)
			sleep(2);
		else
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

	if(cts_chk) {
		int off = FALSE;

		while(!cts()) {
			sleep(1);
			if(debug_fp  &&  off)
				debugmsg("STATUS: No CTS\n");
			off = TRUE;
			}

		if(off)
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
	if(calling == 0)
		wait_for_proxy();

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
		else if(callback_fname  &&  strstr(buf, "caller number:")) {
			if(callback_lookup(strchr(buf, ':') + 1, 1)) {
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
		tcflush(0, TCIFLUSH);	/* Flush all input						*/

		if(greeting_fname)
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
				time(NULL), ctermid(NULL), callbuf);
	else
		sprintf(buf, "%10ld MO %s %ld\n",
				time(NULL), ctermid(NULL), baud);
	logmsg("/etc/acclog", buf);

	while(calling  &&  userid) {
		char c = '\n';

		term = ':';
		get_modem(200, 1);				/* Read modem msg				*/

		if(strstr(buf, "login")) {
			write(modem_fd, userid, strlen(userid));
			write(modem_fd, &c, 1);
			}
		else if(strstr(buf, "password")) {
			if(*password == '\0')
				break;
			write(modem_fd, password, strlen(password));
			write(modem_fd, &c, 1);
			}
		else if(strstr(buf, "# ") || strstr(buf, "$ ")) {
			if(*remcmd == '\0')
				break;
			write(modem_fd, remcmd, strlen(remcmd));
			write(modem_fd, &c, 1);
			break;
			}
		else if(strstr(buf, "no carrier"))
			goto restart1;
		}

	if(!isfax)
		dev_mode(modem_fd, _DEV_MODES, _DEV_MODES);	/* Go edited	*/

	if(!calling  &&  !got_caller_id  &&  callback_fname) {
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

	argv = parse(isfax ? faxcmd : calling &&  callcmd[0] ? callcmd : cmd);
	execvp(argv[0], argv);

	if(debug_fp)
		debugmsg("STATUS: Exec failed\n");

	exit(MODEM_EXEC);
	}


callback_lookup(char *str, int callid) {
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

		if(strcmp(str, callid ? cp : callbuf) == 0) {
			strcpy(callbuf, cp);
			fclose(fp);
			return(1);
			}
		}

	fclose(fp);
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
set_line(baud, par, hflow)
speed_t baud;
char *par;
unsigned hflow;
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


carrier() {
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
	}


cts() {
	long bits[2];

	bits[0] = bits[1] = 0;	/* Don't change anything */
	qnx_ioctl(modem_fd, QCTL_DEV_CTL, bits, sizeof(bits), bits, sizeof(bits));

	return((bits[0] & (1L << 20)) != 0);
	}


void
sleepms(long msec)
	{
	timer_t tid;
	struct itimerspec timer;

	tid = mktimer(TIMEOFDAY, _TNOTIFY_SLEEP, 0);
	timer.it_value.tv_sec     = msec/1000L;
	timer.it_value.tv_nsec    = (msec%1000L) * 1000000L;
	timer.it_interval.tv_sec  = 0L;
	timer.it_interval.tv_nsec = 0L;

	reltimer(tid, &timer, NULL);
	}


void
proxyms(long msec)
	{
	static timer_t tid;
	struct itimerspec timer;
	struct itimercb timercb;

	/*
	 *  NOTE: Setting it_value to 0 cancels the timer
	 * [921006 gbbell]
	 */

	if(tid == 0) {
		timercb.itcb_event.evt_value = proxy;
		tid = mktimer(TIMEOFDAY, _TNOTIFY_PROXY, &timercb);
		}

	timer.it_value.tv_sec     = msec/1000L;
	timer.it_value.tv_nsec    = (msec%1000L) * 1000000L;
	timer.it_interval.tv_sec  = 0L;
	timer.it_interval.tv_nsec = 0L;

	reltimer(tid, &timer, NULL);
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
				time(NULL), opt);
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

	fprintf(debug_fp, "%8.8lX %s ", time(NULL), ctermid(NULL));

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
 */

wait_for_proxy() {

again:
//	fchown(0, 0, 0);
//	fchmod(0, 0666);
	dev_arm(0, proxy, _DEV_EVENT_LOGIN);

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

	if(dev_info(0, &devinfo) == -1  ||  devinfo.open_count != 1) {
		dev_arm(0, _DEV_DISARM, _DEV_EVENT_LOGIN);
		if (debug_fp) debugmsg("PROXY: Device '%s' in use.\n", devinfo.tty_name);
		goto again;
		}

	/*
	 * Once stdin/stdout/stderr are successfully re-opened, reenable
	 * breaks and hangups making the tty a controlling tty (again)
	 * [921006 gbbell]
	 */

	tcsetpgrp( 0, getpgrp() );
	tcsetct( 0, getpid() );

	if((dev_state(0, 0, _DEV_EVENT_LOGIN) & _DEV_EVENT_LOGIN) == 0) {
		dev_arm(0, _DEV_DISARM, _DEV_EVENT_LOGIN);
		if(debug_fp) debugmsg("PROXY: Resync timeout\n");
		longjmp(env, 0);
		}

	dev_mode(0, 0, _DEV_MODES);					/* Go raw		*/
	dev_arm(0, _DEV_DISARM, _DEV_EVENT_LOGIN);
	}


void
sigterm() {

	open(devinfo.tty_name, O_RDWR, 0);	/* Incase stdin was closed */
	dev_arm(0, _DEV_DISARM, _DEV_EVENT_LOGIN);

	exit(MODEM_OK);
	}
