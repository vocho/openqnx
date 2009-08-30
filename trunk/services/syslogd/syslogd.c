/*
 * $QNXtpLicenseC:
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





/*
 * Copyright (c) 1983, 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef lint
char copyright[] =
"@(#) Copyright (c) 1983, 1988 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#if !defined(lint) && defined(NO_SCCSID)
static char sccsid[] = "@(#)syslogd.c	5.45 (Berkeley) 3/2/91";
#endif /* not lint */

/*
 *  syslogd -- log system messages
 *
 * This program implements a system log. It takes a series of lines.
 * Each line may have a priority, signified as "<n>" as
 * the first characters of the line.  If this is
 * not present, a default priority is used.
 *
 * To kill syslogd, send a signal 15 (terminate).  A signal 1 (hup) will
 * cause it to reread its configuration file.
 *
 * Defined Constants:
 *
 * MAXLINE -- the maximimum line length that can be handled.
 * DEFUPRI -- the default priority for user messages
 * DEFSPRI -- the default priority for kernel messages
 *
 * Author: Eric Allman
 * extensive changes by Ralph Campbell
 * more extensive changes by Eric Allman (again)
 */

//	What's different is we accept QNX native msg.  This is done
//	within the select_recieve call.  The original UDP stuff is 
//	still in here, so this will work with original syslog()
//	lib routines.  When hit with SIGHUP, we also try to 
//	establish the UDP port -- we stay up for QNX msgs if
//	we can't do this originally.  This allows syslogd to
//	be started before Socket, and attach to it latter if necessary.
//	All other address families besides AF_INET have been stripped
//	out as well as anything else that doesn't "make sense" for QNX.



#define	MAXLINE		1024		/* maximum line length */
#define	MAXSVLINE	120		/* maximum saved line length */
#define DEFUPRI		(LOG_USER|LOG_NOTICE)
#define DEFSPRI		(LOG_KERN|LOG_CRIT)
#define TIMERINTVL	30		/* interval for checking flush, mark */

#define MSG_BSIZE	(4096 - 3 * sizeof(long))
#define UT_NAMESIZE	14

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/uio.h>

#ifndef __QNXNTO__
#include <sys/errno.h>
#include <sys/un.h>
#include <sys/signal.h>
#include <sys/sidinfo.h>
#include <sys/kernel.h>
#include <sys/osinfo.h>
#include <sys/vc.h>
#include <sys/name.h>
#endif

#include <sys/param.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>

#include <utmp.h>
#include <setjmp.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include "pathnames.h"

#define SYSLOG_NAMES
#include <syslog.h>

#include <unix.h>

#ifdef __QNXNTO__
#define	fsync(x)
#include <sys/resmgr.h>
pthread_mutex_t	synch_lock;
#endif

//char	*LogName = _PATH_LOG;
char	*ConfFile = _PATH_LOGCONF;
char	*PidFile = _PATH_LOGPID;
char	ctty[] = _PATH_CONSOLE;

#define FDMASK(fd)	(1 << (fd))

#define	dprintf		if (Debug) printf

#define MAXUNAMES	20	/* maximum number of user names */

/*
 * Flags to logmsg().
 */

#define IGN_CONS	0x001	/* don't print on console */
#define SYNC_FILE	0x002	/* do fsync on file after printing */
#define ADDDATE		0x004	/* add a date to the message */
#define MARK		0x008	/* this message is a mark */
#define LOG_INTERNAL_ERROR 0x010

/*
 * This structure represents the files that will have log
 * copies printed.
 */

struct filed {
	struct	filed *f_next;		/* next in linked list */
	short	f_type;			/* entry type, see below */
	short	f_file;			/* file descriptor */
	time_t	f_time;			/* time this was last written */
	time_t  f_failed_time;
    int     f_failures;
	u_char	f_pmask[LOG_NFACILITIES+1];	/* priority mask */
	union {
		char	f_uname[MAXUNAMES][UT_NAMESIZE+1];
		struct {
			char	f_hname[MAXHOSTNAMELEN+1];
			struct sockaddr_in	f_addr;
		} f_forw;		/* forwarding address */
		char	f_fname[MAXPATHLEN];
	} f_un;
	char	f_prevline[MAXSVLINE];		/* last message logged */
	char	f_lasttime[16];			/* time of last occurrence */
	char	f_prevhost[MAXHOSTNAMELEN+1];	/* host from which recd. */
	int	f_prevpri;			/* pri of f_prevline */
	int	f_prevlen;			/* length of f_prevline */
	int	f_prevcount;			/* repetition cnt of prevline */
	int	f_repeatcount;			/* number of "repeated" msgs */
};

/*
 * Intervals at which we flush out "message repeated" messages,
 * in seconds after previous message is logged.  After each flush,
 * we move to the next interval until we reach the largest.
 */
int	repeatinterval[] = { 30, 120, 600 };	/* # of secs before flush */
#define	MAXREPEAT ((sizeof(repeatinterval) / sizeof(repeatinterval[0])) - 1)
#define	REPEATTIME(f)	((f)->f_time + repeatinterval[(f)->f_repeatcount])
#define	BACKOFF(f)	{ if (++(f)->f_repeatcount > MAXREPEAT) \
				 (f)->f_repeatcount = MAXREPEAT; \
			}

/* values for f_type */
#define F_UNUSED	0		/* unused entry */
#define F_FILE		1		/* regular file */
#define F_TTY		2		/* terminal */
#define F_CONSOLE	3		/* console terminal */
#define F_FORW		4		/* remote machine */
#define F_USERS		5		/* list of users */
#define F_WALL		6		/* everyone logged on */

char	*TypeNames[7] = {
	"UNUSED",	"FILE",		"TTY",		"CONSOLE",
	"FORW",		"USERS",	"WALL"
};

struct	filed *Files;
struct	filed consfile;

int	Debug;			/* debug flag */
char	LocalHostName[MAXHOSTNAMELEN+1];	/* our hostname */
char	*LocalDomain;		/* our local domain name */
int	InetInuse = 0;		/* non-zero if INET sockets are being used */
int	finet=-1;		/* Internet datagram socket */
int	LogPort;		/* port number for INET connections */
int	Initialized = 0;	/* set when we have initialized ourselves */
int	MarkInterval = 20 * 60;	/* interval between marks in seconds */
int	MarkSeq = 0;		/* mark sequence number */
#ifdef __QNXNTO__
int	threads = 5;
#endif


extern	char *ctime();
extern	int errno;

int	S=0;
void sighup( int signo){
S++;
}

#ifndef __QNXNTO__
int write_messages(nid_t node, struct filed *f, char *msg, int msg_len);
int daemon(int, int);
#endif
int decode(char *name, CODE *codetab);
int usage(void);
void *nto_main(void *x);
int logerror(char *type);
int fprintlog(struct filed *f, int flags, char *msg);
int printline(char *hname, char *msg);
int logmsg(int pri, char *msg, char *from, int flags);
int wallmsg(struct filed *f, struct iovec *iov);
int cfline(char *line, struct filed *f);
  
int
main(argc, argv)
	int argc;
	char **argv;
{
	long inetm=0;
	register int i;
	register char *p;
	int len;
	struct sockaddr_in sin, frominet;
	FILE *fp;
	int ch;
	char line[MSG_BSIZE + 1];
	long omask;
	extern int optind;
	extern char *optarg;
	void die(), domark(), init(), reapchild();

#ifdef __QNXNTO__
	while ((ch = getopt(argc, argv, "df:m:t:")) != EOF)
#else
	while ((ch = getopt(argc, argv, "df:m:")) != EOF)
#endif
		switch((char)ch) {
		case 'd':		/* debug */
			Debug++;
			break;
		case 'f':		/* configuration file */
			ConfFile = optarg;
			break;
		case 'm':		/* mark interval */
			MarkInterval = atoi(optarg) * 60;
			break;
#ifdef __QNXNTO__
		case 't':
			threads = atoi(optarg);
			break;
#endif
		case '?':
		default:
			usage();
		}
	if (argc -= optind)
		usage();

	if (!Debug)
		daemon(0, 0);
	else
		setvbuf(stdout, 0, _IOLBF, 0);

	omask = sigblock(sigmask(SIGHUP)|sigmask(SIGALRM));

	(void) signal(SIGTERM, die);
#ifndef __QNXNTO__
	(void) signal(SIGINT, Debug ? die : SIG_IGN);
#endif
	(void) signal(SIGQUIT, Debug ? die : SIG_IGN);
	(void) signal(SIGCHLD, reapchild);
	(void) signal(SIGALRM, domark);
	(void) alarm(TIMERINTVL);
//	(void) unlink(LogName);

#ifndef __QNXNTO__
	if (qnx_name_attach( 0, "qnx/syslogd" ) == -1){
		perror("qnx_name_attach( qnx/syslogd )");
		exit(1);
		}
#else
	pthread_mutex_init( &synch_lock, NULL );
	pthread_create(NULL,NULL, nto_main, NULL);
#endif

	/* tuck my process id away */
	fp = fopen(PidFile, "w");
	if (fp != NULL) {
		fprintf(fp, "%d\n", getpid());
		(void) fclose(fp);
	}


	signal(SIGHUP, sighup);

sig_HUP:
	sigblock(sigmask(SIGHUP)|sigmask(SIGALRM));

	// got hit with SIGHUP (unless first time thru) so try to
	// establish UDP socket again if first one failed.
if (!InetInuse){
	consfile.f_type = F_CONSOLE;
	(void) strcpy(consfile.f_un.f_fname, ctty);
	(void) gethostname(LocalHostName, sizeof LocalHostName);
	if (p = index(LocalHostName, '.')) {
		*p++ = '\0';
		LocalDomain = p;
	}
	else
		LocalDomain = "";

	bzero((char *)&sin, sizeof(sin));
	finet = socket(AF_INET, SOCK_DGRAM, 0);
	if (finet >= 0) {
		struct servent *sp;

		sp = getservbyname("syslog", "udp");
		if (sp == NULL) {
			errno = 0;
			logerror("syslog/udp: unknown service");
		//	die(0);
			close(finet);
			goto out;
		}
		if (setsockopt(finet, SOL_SOCKET, SO_REUSEADDR,
		    (char *) &finet, sizeof(finet)) < 0)
			logerror("setsockopt");
		sin.sin_family = AF_INET;
		sin.sin_port = LogPort = sp->s_port;
		if (bind(finet, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
			logerror("bind");
			close(finet);
			if (!Debug)
				//die(0);
				goto out;
		} else {
			inetm = FDMASK(finet);
			InetInuse = 1;
			dprintf("socket open...\n");
		}
	}
	} //InetInuse	
	dprintf("off & running....\n");


out:
#ifdef __QNXNTO__
	pthread_mutex_lock( &synch_lock );
#endif
	init();
#ifdef __QNXNTO__
	pthread_mutex_unlock( &synch_lock );
#endif

	for (;;) {
		long readfds = inetm;
		int nfds;

		if (S){
			S--;
			goto sig_HUP;
			}
		errno = 0;
		readfds = inetm;
		dprintf("readfds = %#lx\n", readfds);
		(void) sigsetmask(omask);
		nfds = select(20, (fd_set *) &readfds, (fd_set *) NULL,
		    (fd_set *) NULL, (struct timeval *) NULL);
		if (nfds == 0)
			continue;
		if (nfds < 0) {
			if (errno == EBADF || errno == ESRCH){
				InetInuse = 0;
				inetm=0;
				close(finet);
				dprintf("closing socket...\n");
				//die(0);
				}
			if (errno != EINTR)
				logerror("select");
			if (S){
				S--;
				goto sig_HUP;
				}
			continue;
		}
		sigblock(sigmask(SIGHUP)|sigmask(SIGALRM));
		dprintf("got a message (%d, %#lx)\n", nfds, readfds);

		if (readfds & inetm) {
			len = sizeof frominet;
			i = recvfrom(finet, line, MAXLINE, 0,
			    (struct sockaddr *) &frominet, &len);
			if (i > 0) {
				extern char *cvthname();

				line[i] = '\0';
				printline(cvthname(&frominet), line);
			} else if (i < 0 && errno != EINTR)
				logerror("recvfrom inet");
		} 
	}
	return 0;
}

int
usage(void)
{
	(void) fprintf(stderr,
#ifdef __QNXNTO__

	    "usage: syslogd [-f conffile] [-m markinterval] [-t threads]\n");
#else
	    "usage: syslogd [-f conffile] [-m markinterval] \n");
#endif
	exit(1);
	return 0;
}

/*
 * Take a raw input line, decode the message, and print the message
 * on the appropriate log files.
 */

int
printline(hname, msg)
	char *hname;
	char *msg;
{
	register char *p, *q;
	register int c;
	char line[MAXLINE + 1];
	int pri;

#ifdef __QNXNTO__
	pthread_mutex_lock( &synch_lock );
#endif
	/* test for special codes */
	pri = DEFUPRI;
	p = msg;
	if (*p == '<') {
		pri = 0;
		while (isdigit(*++p))
			pri = 10 * pri + (*p - '0');
		if (*p == '>')
			++p;
	}
	if (pri &~ (LOG_FACMASK|LOG_PRIMASK))
		pri = DEFUPRI;

	/* don't allow users to log kernel messages */
	if (LOG_FAC(pri) == LOG_KERN)
		pri = LOG_MAKEPRI(LOG_USER, LOG_PRI(pri));

	q = line;

	while ((c = *p++ & 0177) != '\0' &&
	    q < &line[sizeof(line) - 1])
		if (iscntrl(c))
			if (c == '\n')
				*q++ = ' ';
			else if (c == '\t')
				*q++ = '\t';
			else {
				*q++ = '^';
				*q++ = c ^ 0100;
			}
		else
			*q++ = c;
	*q = '\0';

	logmsg(pri, line, hname, 0);
#ifdef __QNXNTO__
	pthread_mutex_unlock( &synch_lock );
#endif
	return 0;
}

/*
 * Take a raw input line from /dev/klog, split and format similar to syslog().
 */

int
printsys(msg)
	char *msg;
{
	register char *p, *q;
	register int c;
	char line[MAXLINE + 1];
	int pri, flags;
	char *lp;

	(void) strcpy(line, "vmunix: ");
	lp = line + strlen(line);
	for (p = msg; *p != '\0'; ) {
		flags = SYNC_FILE | ADDDATE;	/* fsync file after write */
		pri = DEFSPRI;
		if (*p == '<') {
			pri = 0;
			while (isdigit(*++p))
				pri = 10 * pri + (*p - '0');
			if (*p == '>')
				++p;
		} else {
			/* kernel printf's come out on console */
			flags |= IGN_CONS;
		}
		if (pri &~ (LOG_FACMASK|LOG_PRIMASK))
			pri = DEFSPRI;
		q = lp;
		while (*p != '\0' && (c = *p++) != '\n' &&
		    q < &line[MAXLINE])
			*q++ = c;
		*q = '\0';
		logmsg(pri, line, LocalHostName, flags);
	}
	return 0;
}

time_t	now;

/*
 * Log a message to the appropriate log files, users, etc. based on
 * the priority.
 */

int
logmsg(pri, msg, from, flags)
	int pri;
	char *msg, *from;
	int flags;
{
	register struct filed *f;
	int fac=0, prilev, msglen;
	char *timestamp;
	time_t time();

	dprintf("logmsg: pri %o, flags %x, from %s, msg %s\n",
	    pri, flags, from, msg);

	/*
	 * Check to see if msg looks non-standard.
	 */
	msglen = strlen(msg);
	if (msglen < 16 || msg[3] != ' ' || msg[6] != ' ' ||
	    msg[9] != ':' || msg[12] != ':' || msg[15] != ' ')
		flags |= ADDDATE;

	(void) time(&now);
	if (flags & ADDDATE)
		timestamp = ctime(&now) + 4;
	else {
		timestamp = msg;
		msg += 16;
		msglen -= 16;
	}

	/* extract facility and priority level */
	if (flags & MARK)
		fac = LOG_NFACILITIES;
	else
		fac = LOG_FAC(pri);
	prilev = LOG_PRI(pri);

	/* log the message to the particular outputs */
	if (!Initialized) {
		f = &consfile;
		f->f_file = open(ctty, O_WRONLY, 0);

		if (f->f_file >= 0) {
			fprintlog(f, flags, msg);
			(void) close(f->f_file);
		}
		return 0;
	}
	for (f = Files; f; f = f->f_next) {
		/* skip messages that are incorrect priority */
		if (f->f_pmask[fac] < prilev ||
		    f->f_pmask[fac] == INTERNAL_NOPRI)
			continue;

		if (f->f_type == F_CONSOLE && (flags & IGN_CONS))
			continue;

		/* don't output marks to recently written files */
		if ((flags & MARK) && (now - f->f_time) < MarkInterval / 2)
			continue;

		/*
		 * suppress duplicate lines to this file
		 */
		if ((flags & MARK) == 0 && msglen == f->f_prevlen &&
		    !strcmp(msg, f->f_prevline) &&
		    !strcmp(from, f->f_prevhost)) {
			(void) strncpy(f->f_lasttime, timestamp, 15);
			f->f_prevcount++;
			dprintf("msg repeated %d times, %d sec of %d\n",
			    f->f_prevcount, now - f->f_time,
			    repeatinterval[f->f_repeatcount]);
			/*
			 * If domark would have logged this by now,
			 * flush it now (so we don't hold isolated messages),
			 * but back off so we'll flush less often
			 * in the future.
			 */
			if (now > REPEATTIME(f)) {
				fprintlog(f, flags, (char *)NULL);
				BACKOFF(f);
			}
		} else {
			/* new line, save it */
			if (f->f_prevcount)
				fprintlog(f, 0, (char *)NULL);
			f->f_repeatcount = 0;
			(void) strncpy(f->f_lasttime, timestamp, 15);
			(void) strncpy(f->f_prevhost, from,
					sizeof(f->f_prevhost));
			if (msglen < MAXSVLINE) {
				f->f_prevlen = msglen;
				f->f_prevpri = pri;
				(void) strcpy(f->f_prevline, msg);
				fprintlog(f, flags, (char *)NULL);
			} else {
				f->f_prevline[0] = 0;
				f->f_prevlen = 0;
				fprintlog(f, flags, msg);
			}
		}
	}
	return 0;
}

int
fprintlog(f, flags, msg)
	register struct filed *f;
	int flags;
	char *msg;
{
	struct iovec iov[6];
	register struct iovec *v;
	register int l;
	int reopencnt, sigrepcnt, i;
	char line[MAXLINE + 1], repbuf[80], greetings[200];

	v = iov;
	memset( v, 0, sizeof( struct iovec ) * 6 );
	if ((f->f_type == F_WALL) || (f->f_type == F_USERS )) {
		v->iov_base = greetings;
		v->iov_len = sprintf(greetings,
		    "\r\n\7Message from syslogd@%s at %.24s ...\r\n",
		    f->f_prevhost, ctime(&now));
		v++;
		v->iov_base = "";
		v->iov_len = 0;
		v++;
	} else {
		v->iov_base = f->f_lasttime;
		v->iov_len = 15;
		v++;
		v->iov_base = " ";
		v->iov_len = 1;
		v++;
	}
	v->iov_base = f->f_prevhost;
	v->iov_len = strlen(v->iov_base);
	v++;
	v->iov_base = " ";
	v->iov_len = 1;
	v++;

	if (msg) {
		v->iov_base = msg;
		v->iov_len = strlen(msg);
	} else if (f->f_prevcount > 1) {
		v->iov_base = repbuf;
		v->iov_len = sprintf(repbuf, "last message repeated %d times",
		    f->f_prevcount);
	} else {
		v->iov_base = f->f_prevline;
		v->iov_len = f->f_prevlen;
	}
	v++;

	dprintf("Logging to %s", TypeNames[f->f_type]);
	f->f_time = now;

		if (flags & LOG_INTERNAL_ERROR &&
            f->f_failed_time == time(NULL) ) {
			dprintf(" **** recursion detected: can't log [%s]\n", (char *)iov[4].iov_base);
			return 0;
		}

	switch (f->f_type) {
	case F_UNUSED:
		dprintf("\n");
		break;

	case F_FORW:
		dprintf(" %s\n", f->f_un.f_forw.f_hname);
		l = sprintf(line, "<%d>%.15s %s", f->f_prevpri,
		    (char *)iov[0].iov_base, (char *)iov[4].iov_base);
		if (l > MAXLINE)
			l = MAXLINE;

       i= 0;
       do{
		if (sendto(finet, line, l, 0,
		    (struct sockaddr *)&f->f_un.f_forw.f_addr,
		    sizeof f->f_un.f_forw.f_addr) != l) {
			f->f_failed_time= time(NULL);
			f->f_failures++;
			logerror("sendto");
			errno = 0;
			logerror("possible duplicate message");
		} else break;
       } while( i++ < 4 );		
     
     //if ( i >= 4 )
     //  f->f_type= F_UNUSED;
		break;

	case F_CONSOLE:
		if (flags & IGN_CONS) {
			dprintf(" (ignored)\n");
			break;
		}
		/* FALLTHROUGH */

	case F_TTY:
	case F_FILE:
		dprintf(" %s\n", f->f_un.f_fname);
		if (f->f_type != F_FILE) {
			v->iov_base = "\r\n";
			v->iov_len = 2;
		} else {
			v->iov_base = "\n";
			v->iov_len = 1;
		}
		reopencnt= 0;
		sigrepcnt= 0;
	again:
		if (writev(f->f_file, iov, 6) < 0) {
			int e = errno;
			if (e != EINTR || ++sigrepcnt > 3) {
				(void) close(f->f_file);
			} else {
			    f->f_failed_time= time(NULL);
			    f->f_failures++;
				logerror(f->f_un.f_fname);
				errno = 0;
				logerror("possible duplicate message");
				goto again; /* try again */
			}
			/*
			 * Check for errors on TTY's due to loss of tty
			 */
			if ((e == EIO || e == EBADF) && f->f_type != F_FILE) {
				reopencnt++;
				f->f_file = open(f->f_un.f_fname,
				    O_WRONLY|O_APPEND, 0);
				if (f->f_file < 0) {
					f->f_type = F_UNUSED;
					logerror(f->f_un.f_fname);
				} else if (reopencnt < 3)
					goto again;
			} else {
				f->f_type = F_UNUSED;
				errno = e;
				logerror(f->f_un.f_fname);
			}
		} else if (flags & SYNC_FILE)
			fsync(f->f_file);
		break;
#ifndef __QNXNTO__
	case F_USERS:
	case F_WALL:
		dprintf("\n");
		v->iov_base = "\r\n";
		v->iov_len = 2;
		wallmsg(f, iov);
		break;
#endif
	}
	f->f_prevcount = 0;
	return 0;
}

/*
 *  WALLMSG -- Write a message to the world at large
 *
 *	Write the specified message to either the entire
 *	world, or a list of approved users.
 */

#ifndef __QNXNTO__
int
wallmsg(f, iov)
	register struct filed *f;
	struct iovec *iov;
{
	static int reenter;			/* avoid calling ourselves */
	register int i;

	char *message;
	int	message_len = 0;
	struct _osinfo osinfo;
	int	max_node = 0;
	char	*nid_stat;

	qnx_osinfo( 0, &osinfo );
	max_node = osinfo.max_nodes + 1;


	if (reenter++)
		return 0;

	for (i=0; i<6; i++){
		message_len += iov[i].iov_len;
		}

	if ((message = malloc(message_len+2)) == NULL){
		logerror("wallmsg: malloc()");
//fprintf(stderr, "msg_len[%d]\n", message_len);
		reenter = 0;
		return 0;
		}

	memset( message, 0, message_len );
	for (i=0; i<6; i++){
		strncat( message, iov[i].iov_base, iov[i].iov_len );
//fprintf(stderr,"message[%s]\n", message);
		}

	if ((nid_stat = calloc( max_node, 1)) == NULL){
		logerror("wallmsg: calloc()");
		reenter = 0;
		free(message);
		return 0;
		}

	qnx_net_alive( nid_stat, max_node );

	if (fork() == 0){
		for(i=1; i<=max_node; i++)
			if (nid_stat[i])
				write_message( (nid_t)i, (f->f_type == F_WALL) ? NULL : f, message, message_len );
		exit(0);
		}	
	else{
		reenter = 0;
		free(message);
		free(nid_stat);
		return 0;
		}
	return 0;
}
#endif

#ifndef __QNXNTO__
int write_message(nid_t node, struct filed *f, char *msg, int msg_len){
	int errs=0,i;
	nid_t nid = node;
    pid_t vid;
    int sid;
    FILE *tfp;
    struct _sidinfo sidinfo;
	char *tty;
    static char usr_msgs[256];
	struct _osinfo osinfo;
	int match = 0;
	int test_mode = 0;

	if ((vid = qnx_vc_attach(nid, PROC_PID, 0, 0)) == -1)
	   return(errno);

	qnx_osinfo( vid, &osinfo );

		/* skip session zero (System) */
	for (sid = 1; sid <= osinfo.num_sessions; ++sid) {
		match = 0;

	    if ((sid = qnx_sid_query(vid, sid, &sidinfo)) == -1)
			break;
	
	    tty = sidinfo.tty_name;
	    if (*tty == 0 || strcmp(tty, "none") == 0)
			continue;

		/* should we send the message to this user? */
		if (f){
			for (i = 0; i < MAXUNAMES; i++) {
				if (!f->f_un.f_uname[i][0])
					break;
				if (!strncmp(f->f_un.f_uname[i], sidinfo.name, 15)) {
					match++;
					break;
					}
				}
			}
		else	// f was NULL -- send to all users
			match++;
	
		if (!match){
//fprintf(stderr,"m[%s] node[%d] sid[%d]\n", msg, nid, sid); 
			continue;
			}

//fprintf(stderr,"MATCH: m[%s] node[%d] sid[%d]\n", msg, nid, sid); 

	    if ((tfp = fopen(tty, "w")) == 0) {
			i=sprintf(usr_msgs, "%s: cannot send to %s (%s)\n", "write_message()", sidinfo.name, tty);
			logerror(usr_msgs);
			continue;
		    }
	
		if (!test_mode) {
		    fcntl(fileno(tfp), F_SETFL, O_NONBLOCK | fcntl(fileno(tfp), F_GETFL));
		 //   fprintf(tfp, "\a\aBroadcast message from %s (%s) at %s...\r\n", user, term, stamp);
		    fprintf(tfp, msg);
		    fputs("EOT\r\n", tfp);
			} 

		fclose(tfp);
    	}
	qnx_vc_detach(vid);
	return (errs?-1:0);	
}
#endif

#if 0
wallmsg(f, iov)
	register struct filed *f;
	struct iovec *iov;
{
	static int reenter;			/* avoid calling ourselves */
	register FILE *uf;
	register int i;
	struct utmp ut;
	char *p, *ttymsg();

	if (reenter++)
		return;
	if ((uf = fopen(_PATH_UTMP, "r")) == NULL) {
		logerror(_PATH_UTMP);
		reenter = 0;
		return;
	}
	/* NOSTRICT */
	while (fread((char *) &ut, sizeof ut, 1, uf) == 1) {
		if (ut.ut_name[0] == '\0')
			continue;
		if (f->f_type == F_WALL) {
			if (p = ttymsg(iov, 6, ut.ut_line, 1)) {
				errno = 0;	/* already in msg */
				logerror(p);
			}
			continue;
		}
		/* should we send the message to this user? */
		for (i = 0; i < MAXUNAMES; i++) {
			if (!f->f_un.f_uname[i][0])
				break;
			if (!strncmp(f->f_un.f_uname[i], ut.ut_name,
			    UT_NAMESIZE)) {
				if (p = ttymsg(iov, 6, ut.ut_line, 1)) {
					errno = 0;	/* already in msg */
					logerror(p);
				}
				break;
			}
		}
	}
	(void) fclose(uf);
	reenter = 0;
}
#endif



void
reapchild()
{
	int status;

	while (waitpid(-1, (int *)&status, WNOHANG) > 0)
		;
}

/*
 * Return a printable representation of a host address.
 */
char *
cvthname(f)
	struct sockaddr_in *f;
{
	struct hostent *hp;
	register char *p;
	extern char *inet_ntoa();

	dprintf("cvthname(%s)\n", inet_ntoa(f->sin_addr));

	if (f->sin_family != AF_INET) {
		dprintf("Malformed from address\n");
		return ("???");
	}
	hp = gethostbyaddr((char *)&f->sin_addr,
	    sizeof(struct in_addr), f->sin_family);
	if (hp == 0) {
		dprintf("Host name for your address (%s) unknown\n",
			inet_ntoa(f->sin_addr));
		return (inet_ntoa(f->sin_addr));
	}
	if ((p = index(hp->h_name, '.')) && strcmp(p + 1, LocalDomain) == 0)
		*p = '\0';
	return (hp->h_name);
}

void
domark()
{
	register struct filed *f;
	time_t time();

	now = time((time_t *)NULL);
	MarkSeq += TIMERINTVL;
	if (MarkSeq >= MarkInterval) {
		logmsg(LOG_INFO, "-- MARK --", LocalHostName, ADDDATE|MARK);
		MarkSeq = 0;
	}

	for (f = Files; f; f = f->f_next) {
		if (f->f_prevcount && now >= REPEATTIME(f)) {
			dprintf("flush %s: repeated %d times, %d sec.\n",
			    TypeNames[f->f_type], f->f_prevcount,
			    repeatinterval[f->f_repeatcount]);
			fprintlog(f, 0, (char *)NULL);
			BACKOFF(f);
		}
	}
	(void) alarm(TIMERINTVL);
}

/*
 * Print syslogd errors some place.
 */
int
logerror(type)
	char *type;
{
	char buf[100], *strerror();

	if (errno)
		(void) sprintf(buf, "syslogd: %s: %s", type, strerror(errno));
	else
		(void) sprintf(buf, "syslogd: %s", type);
	errno = 0;
	dprintf("%s\n", buf);
	logmsg(LOG_SYSLOG|LOG_ERR, buf, LocalHostName, ADDDATE|LOG_INTERNAL_ERROR);
	return 0;
}

void
die(sig)
{
	register struct filed *f;
	char buf[100];

	for (f = Files; f != NULL; f = f->f_next) {
		/* flush any pending output */
		if (f->f_prevcount)
			fprintlog(f, 0, (char *)NULL);
	}
	if (sig) {
		dprintf("syslogd: exiting on signal %d\n", sig);
		(void) sprintf(buf, "exiting on signal %d", sig);
		errno = 0;
		logerror(buf);
	}
	//(void) unlink(LogName);
	exit(0);
}

/*
 *  INIT -- Initialize syslogd from configuration table
 */

void
init()
{
	register int i;
	register FILE *cf;
	register struct filed *f, *next, **nextp;
	register char *p;
	char cline[BUFSIZ];

	dprintf("init\n");

	/*
	 *  Close all open log files.
	 */
	Initialized = 0;
	for (f = Files; f != NULL; f = next) {
		/* flush any pending output */
		if (f->f_prevcount)
			fprintlog(f, 0, (char *)NULL);

		switch (f->f_type) {
		  case F_FILE:
		  case F_TTY:
		  case F_CONSOLE:
		  case F_FORW:
			(void) close(f->f_file);
			break;
		}
		next = f->f_next;
		free((char *) f);
	}
	Files = NULL;
	nextp = &Files;

	/* open the configuration file */
	if ((cf = fopen(ConfFile, "r")) == NULL) {
		dprintf("cannot open %s\n", ConfFile);
		*nextp = (struct filed *)calloc(1, sizeof(*f));
		cfline("*.ERR\t/dev/console", *nextp);
		(*nextp)->f_next = (struct filed *)calloc(1, sizeof(*f));
		cfline("*.PANIC\t*", (*nextp)->f_next);
		Initialized = 1;
		return;
	}

	/*
	 *  Foreach line in the conf table, open that file.
	 */
	f = NULL;
	while (fgets(cline, sizeof cline, cf) != NULL) {
		/*
		 * check for end-of-section, comments, strip off trailing
		 * spaces and newline character.
		 */
		for (p = cline; isspace(*p); ++p);
		if (*p == NULL || *p == '#')
			continue;
		for (p = index(cline, '\0'); isspace(*--p););
		*++p = '\0';
		f = (struct filed *)calloc(1, sizeof(*f));
		*nextp = f;
		nextp = &f->f_next;
		cfline(cline, f);
	}

	/* close the configuration file */
	(void) fclose(cf);

	Initialized = 1;

	if (Debug) {
		for (f = Files; f; f = f->f_next) {
			for (i = 0; i <= LOG_NFACILITIES; i++)
				if (f->f_pmask[i] == INTERNAL_NOPRI)
					printf("X ");
				else
					printf("%d ", f->f_pmask[i]);
			printf("%s: ", TypeNames[f->f_type]);
			switch (f->f_type) {
			case F_FILE:
			case F_TTY:
			case F_CONSOLE:
				printf("%s", f->f_un.f_fname);
				break;

			case F_FORW:
				printf("%s", f->f_un.f_forw.f_hname);
				break;

			case F_USERS:
				for (i = 0; i < MAXUNAMES && *f->f_un.f_uname[i]; i++)
					printf("%s, ", f->f_un.f_uname[i]);
				break;
			}
			printf("\n");
		}
	}

	logmsg(LOG_SYSLOG|LOG_INFO, "syslogd: restart", LocalHostName, ADDDATE);
	dprintf("syslogd: restarted\n");
}

/*
 * Crack a configuration file line
 */

int
cfline(line, f)
	char *line;
	register struct filed *f;
{
	register char *p;
	register char *q;
	register int i;
	char *bp;
	int pri;
	struct hostent *hp;
	char buf[MAXLINE], ebuf[100];

	dprintf("cfline(%s)\n", line);

	errno = 0;	/* keep strerror() stuff out of logerror messages */

	/* clear out file entry */
	bzero((char *) f, sizeof *f);
	for (i = 0; i <= LOG_NFACILITIES; i++)
		f->f_pmask[i] = INTERNAL_NOPRI;

	/* scan through the list of selectors */
	for (p = line; *p && *p != '\t';) {

		/* find the end of this facility name list */
		for (q = p; *q && *q != '\t' && *q++ != '.'; )
			continue;

		/* collect priority name */
		for (bp = buf; *q && !index("\t,;", *q); )
			*bp++ = *q++;
		*bp = '\0';

		/* skip cruft */
		while (index(", ;", *q))
			q++;

		/* decode priority name */
		if (*buf == '*')
			pri = LOG_PRIMASK + 1;
		else {
			pri = decode(buf, prioritynames);
			if (pri < 0) {
				(void) sprintf(ebuf,
				    "unknown priority name \"%s\"", buf);
				logerror(ebuf);
				return -1;
			}
		}

		/* scan facilities */
		while (*p && !index("\t.;", *p)) {
			for (bp = buf; *p && !index("\t,;.", *p); )
				*bp++ = *p++;
			*bp = '\0';
			if (*buf == '*')
				for (i = 0; i < LOG_NFACILITIES; i++)
					f->f_pmask[i] = pri;
			else {
				i = decode(buf, facilitynames);
				if (i < 0) {
					(void) sprintf(ebuf,
					    "unknown facility name \"%s\"",
					    buf);
					logerror(ebuf);
					return -1;
				}
				f->f_pmask[i >> 3] = pri;
			}
			while (*p == ',' || *p == ' ')
				p++;
		}

		p = q;
	}

	/* skip to action part */
	while (*p == '\t')
		p++;

	switch (*p)
	{
	case '@':
		if (!InetInuse)
			break;
		(void) strcpy(f->f_un.f_forw.f_hname, ++p);
		hp = gethostbyname(p);
		if (hp == NULL) {
			char buf[80];
			sprintf(buf, "Resolver error %d", (u_int)h_errno);
			logerror(buf);
			break;
		}
		bzero((char *) &f->f_un.f_forw.f_addr,
			 sizeof f->f_un.f_forw.f_addr);
		f->f_un.f_forw.f_addr.sin_family = AF_INET;
		f->f_un.f_forw.f_addr.sin_port = LogPort;
		bcopy(hp->h_addr, (char *) &f->f_un.f_forw.f_addr.sin_addr, hp->h_length);
		f->f_type = F_FORW;
		break;

	case '/':
		(void) strcpy(f->f_un.f_fname, p);
		if ((f->f_file = open(p, O_WRONLY|O_APPEND, 0)) < 0) {
			f->f_file = F_UNUSED;
			logerror(p);
			break;
		}
		if (isatty(f->f_file))
			f->f_type = F_TTY;
		else
			f->f_type = F_FILE;
		if (strcmp(p, ctty) == 0)
			f->f_type = F_CONSOLE;
		break;

	case '*':
		f->f_type = F_WALL;
		break;

	default:
		for (i = 0; i < MAXUNAMES && *p; i++) {
			for (q = p; *q && *q != ','; )
				q++;
			(void) strncpy(f->f_un.f_uname[i], p, UT_NAMESIZE);
			if ((q - p) > UT_NAMESIZE)
				f->f_un.f_uname[i][UT_NAMESIZE] = '\0';
			else
				f->f_un.f_uname[i][q - p] = '\0';
			while (*q == ',' || *q == ' ')
				q++;
			p = q;
		}
		f->f_type = F_USERS;
		break;
	}
	return 0;
}


/*
 *  Decode a symbolic name to a numeric value
 */

int
decode(name, codetab)
	char *name;
	CODE *codetab;
{
	register CODE *c;
	register char *p;
	char buf[40];

	if (isdigit(*name))
		return (atoi(name));

	(void) strcpy(buf, name);
	for (p = buf; *p; p++)
		if (isupper(*p))
			*p = tolower(*p);
	for (c = codetab; c->c_name; c++)
		if (!strcmp(buf, c->c_name))
			return (c->c_val);

	return (-1);
}

#if (defined(__QNX__)) && (!defined(__QNXNTO__))
#include <sys/psinfo.h>
#include <sys/kernel.h>

char *
pidtoname(pid_t pid) {
	struct _psinfo ps;
	static char ret[20];

	if(( qnx_psinfo( PROC_PID, pid, &ps, 0, 0 ) == -1 ) || (pid != ps.pid) )
		return "node<<???>>";
	if( ps.flags & _PPF_VID )
		sprintf( ret, "node<<%d>>", ps.un.vproc.remote_nid );
	else
		sprintf( ret, "node<<%d>>", getnid());
	return ret;
}

pid_t
_select_receive(pid_t proxy) {
	char line[MSG_BSIZE + 1];
	pid_t pid;

	memset( line, 0, sizeof line );
	while( (pid = Receive( 0, &line, sizeof line - 1 )) != -1 ) {
		if( pid == proxy ) break;
		Reply( pid, NULL, 0 );
dprintf("got msg...\n");
		/* sanity check the line */
		if( line[0] != '<' || !isdigit(line[1]) )
			continue;
		printline(pidtoname(pid), line);
		memset( line, 0, sizeof line );
	}
	return pid;
}
#endif

#ifdef __QNXNTO__
#include <sys/dispatch.h>
#include <sys/iofunc.h>
#include <sys/neutrino.h>


int io_write(resmgr_context_t *ctp, io_write_t *msg, void *notused){
char *p;
int len;

	p= (char *) ((char *)&(msg->i) + sizeof(msg->i));
	len= msg->i.nbytes;
	if(len >= ctp->msg_max_size - sizeof(msg->i))
	{
		len = ctp->msg_max_size - sizeof(msg->i);
		if(p[len-1] != '\0')
		{
			p[len-1] = '\0';
			len--;
		}
	}
	else
		p[len] = '\0';

	MsgReplyv( ctp->rcvid, len, NULL, 0);
	printline( "nto", p );
	return(_RESMGR_NOREPLY);
}

iofunc_attr_t                   attr;

void *
nto_main(void *x){
	int	major= 1;
	resmgr_connect_funcs_t iofunc_connect_funcs_default;
	resmgr_io_funcs_t iofunc_io_funcs_default;
	resmgr_attr_t resmgr_attr;
	thread_pool_attr_t pool_attr;
	dispatch_t *dpp;
	thread_pool_t *tpp;

	x = x; /* not ref'd */
	memset(&resmgr_attr, 0, sizeof(resmgr_attr));
	resmgr_attr.nparts_max = 1;
	resmgr_attr.msg_max_size = 2048;

	dpp = dispatch_create();
	if (dpp == NULL) {
		perror("dispatch_create");
		exit(1);
	}

	memset(&pool_attr, 0, sizeof(pool_attr));
	pool_attr.handle = dpp;
	// We are only doing resmgr-type attach
	pool_attr.context_alloc = resmgr_context_alloc;
	pool_attr.block_func = resmgr_block;
	pool_attr.handler_func = resmgr_handler;
	pool_attr.context_free = resmgr_context_free;
	pool_attr.lo_water = 2;
	pool_attr.hi_water = 15;
	pool_attr.increment = 1;
	pool_attr.maximum= threads;

	if((tpp = thread_pool_create(&pool_attr, POOL_FLAG_USE_SELF)) == NULL) {
		perror("thread_pool_create");
		exit(1);
	}

	iofunc_func_init(_RESMGR_CONNECT_NFUNCS, &iofunc_connect_funcs_default,
		_RESMGR_IO_NFUNCS, &iofunc_io_funcs_default);
	iofunc_attr_init(&attr, 0666 | S_IFCHR, 0, 0);
	attr.rdev= makedev(rand(), major, 1);
	iofunc_io_funcs_default.write= io_write;

	if( resmgr_attach( dpp, &resmgr_attr, _PATH_LOG, _FTYPE_ANY,
		   0, &iofunc_connect_funcs_default, 
		   &iofunc_io_funcs_default, &attr) == -1) {
		perror("resmgr_attach");
		exit(1);
	}

	return (void *)thread_pool_start(tpp);
}
#endif
