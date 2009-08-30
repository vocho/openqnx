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
 * Copyright (c) 1983 Regents of the University of California.
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
"@(#) Copyright (c) 1983 Regents of the University of California.\n\
 All rights reserved.\n";
#endif /* not lint */

#ifndef lint
static char sccsid[] = "@(#)lpd.c	5.12 (Berkeley) 3/7/91";
#endif /* not lint */

/*
 * lpd -- line printer daemon.
 *
 * Listen for a connection and perform the requested operation.
 * Operations are:
 *	\1printer\n
 *		check the queue for jobs and print any found.
 *	\2printer\n
 *		receive a job from another machine and queue it.
 *	\3printer [users ...] [jobs ...]\n
 *		return the current state of the queue (short form).
 *	\4printer [users ...] [jobs ...]\n
 *		return the current state of the queue (long form).
 *	\5printer person [users ...] [jobs ...]\n
 *		remove jobs from the queue.
 *
 * Strategy to maintain protected spooling area:
 *	1. Spooling area is writable only by daemon and spooling group
 *	2. lpr runs setuid root and setgrp spooling group; it uses
 *	   root to access any file it wants (verifying things before
 *	   with an access call) and group id to know how it should
 *	   set up ownership of files in the spooling area.
 *	3. Files in spooling area are owned by root, group spooling
 *	   group, with mode 660.
 *	4. lpd, lpq and lprm run setuid daemon and setgrp spooling group to
 *	   access files and printer.  Users can't get to anything
 *	   w/o help of lpq and lprm programs.
 */

#include "lp.h"
#include "pathnames.h"
#include <unix.h>
#include "protos.h"
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#ifndef __QNXNTO__
#include <sys/signal.h>
#else
#include <signal.h>
#endif

int	lflag = 0;				/* log requests flag */
int	from_remote;			/* from remote socket */
int	debug = 0;
int g_nocheckhost = 0;
int 	timeout = 0;

void mcleanup(), reapchild(),alarm_timeout();

void
main(argc, argv)
	int argc;
        signed char **argv;
{
	int f, funix, finet, options = 0, fromlen;
	long defreadfds;
	struct sockaddr_un sun, fromunix;
	struct sockaddr_in sin, frominet;
	int omask, lfd;
	char *cp;

	memset(&sin, 0, sizeof sin);
	gethostname(host, sizeof(host));
	name = argv[0];

	while (--argc > 0) {
		argv++;
		if (argv[0][0] == '-')
			switch (argv[0][1]) {
			case 'd':
				debug = 1;
//				options |= SO_DEBUG;
				break;
			case 'l':
				lflag++;
				break;
			case 'n':				// Added by bobbyc for MTX
				g_nocheckhost = 1;  // So that it is not necessary to.
				break;
			case 't':
				cp=&argv[0][1];
				if(!*++cp){
					argc--,argv++;
					if((argc<=0)||*argv[0]== '-'){
						syslog(LOG_ERR,"Invalid -t argument");
						exit(-1);
					}else{
						timeout=atoi(argv[0]);
					}
				} else {
					timeout = atoi(cp);
				}
				break;
			}
	}

	/*
	 * Set up standard environment by detaching from the parent.
	 */
	daemon(0, 0);
	if (debug) raise(SIGSTOP);

	openlog("lpd", LOG_CONS | LOG_PID, LOG_LPR);
	(void) umask(0);
	lfd = open(_PATH_MASTERLOCK, O_WRONLY|O_CREAT, 0644);
	if (lfd < 0) {
		syslog(LOG_ERR, "%s: %m", _PATH_MASTERLOCK);
		exit(1);
	}
	if (flock(lfd, LOCK_EX|LOCK_NB) < 0) {
		if (errno == EWOULDBLOCK){	/* active deamon present */
			syslog(LOG_ERR, "%s: failed to obtain lock", _PATH_MASTERLOCK);
			exit(0);
			}
		syslog(LOG_ERR, "%s: %m", _PATH_MASTERLOCK);
		exit(1);
	}
	ftruncate(lfd, 0);
#define	mask(s)	(1 << ((s) - 1))
	omask = sigblock(mask(SIGHUP)|mask(SIGINT)|mask(SIGQUIT)|mask(SIGTERM));
	sigblock(mask(SIGCHLD));
	signal(SIGCHLD, reapchild);
        if((int)signal(SIGALRM,alarm_timeout)<0)
		syslog(LOG_ERR,"signal SIGALRM timeout: %m");
	/*
	 * Restart all the printers.
	 */
	startup();
	/*
	 * write process id for others to know
	 */
	sprintf(line, "%u\n%u\n", getpid(),getnid());
	f = strlen(line);
	if (write(lfd, line, f) != f) {
		syslog(LOG_ERR, "%s: %m", _PATH_MASTERLOCK);
		exit(1);
	}
#if 0
	(void) unlink(_PATH_SOCKETNAME);
	funix = socket(AF_UNIX, SOCK_STREAM, 0);
	if (funix < 0) {
		syslog(LOG_ERR, "socket: %m");
		exit(1);
	}
#endif
	signal(SIGHUP, mcleanup);
	signal(SIGINT, mcleanup);
	signal(SIGQUIT, mcleanup);
	signal(SIGTERM, mcleanup);
#if 0
	sun.sun_family = AF_UNIX;
	strcpy(sun.sun_path, _PATH_SOCKETNAME);
	if (bind(funix,
	     (struct sockaddr *)&sun, strlen(sun.sun_path) + 2) < 0) {
		syslog(LOG_ERR, "ubind: %m");
		exit(1);
	}
	defreadfds = 1 << funix;
	listen(funix, 5);
#endif
	finet = socket(AF_INET, SOCK_STREAM, 0);
	if (finet >= 0) {
		struct servent *sp;

		if (options & SO_DEBUG)
			if (setsockopt(finet, SOL_SOCKET, SO_DEBUG, 0, 0) < 0) {
				syslog(LOG_ERR, "setsockopt (SO_DEBUG): %m");
				mcleanup();
			}
		sp = getservbyname("printer", "tcp");
		if (sp == NULL) {
			syslog(LOG_ERR, "printer/tcp: unknown service");
			mcleanup();
		}
		sin.sin_family = AF_INET;
		sin.sin_port = sp->s_port;
		if (bind(finet, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
			syslog(LOG_ERR, "bind: %m");
			mcleanup();
		}
		defreadfds = 1 << finet;
		listen(finet, 5);
	}
	sigsetmask(omask);
	/*
	 * Main loop: accept, do a request, continue.
	 */
	for (;;) {
		int domain, nfds, s;
		long readfds = defreadfds;

		nfds = select(20, &readfds, 0, 0, 0);
		
		if (nfds <= 0) {
			if (nfds < 0 && (errno == ESRCH || errno == EBADF)) {
				syslog(LOG_WARNING, "select: %m");
				exit(1);
			} 
			continue;
		}
#if 0
		if (readfds & (1 << funix)) {
			domain = AF_UNIX, fromlen = sizeof(fromunix);
			s = accept(funix,
			    (struct sockaddr *)&fromunix, &fromlen);
		} else 
#endif
			if ((int)readfds & (1 << finet)) {
			domain = AF_INET, fromlen = sizeof(frominet);
			s = accept(finet,
			    (struct sockaddr *)&frominet, &fromlen);
		}
		if (s < 0) {
			if (errno != EINTR)
				syslog(LOG_WARNING, "accept: %m");
			continue;
		}
		if (fork() == 0) {
			if (debug) raise(SIGSTOP);
			signal(SIGCHLD, SIG_DFL);
			signal(SIGHUP, SIG_IGN);
			signal(SIGINT, SIG_IGN);
			signal(SIGQUIT, SIG_IGN);
			signal(SIGTERM, SIG_IGN);
#if 0
			(void) close(funix);
#endif
			(void) close(finet);
			dup2(s, 1);
			(void) close(s);
#if 0
			if (domain == AF_INET) {
				from_remote = 1;
				chkhost(&frominet);
			} else
				from_remote = 0;
#endif
		if(frominet.sin_addr.s_addr == 0x100007f) /* localhost */
				from_remote = 0;
			else{
				from_remote = 1;
				chkhost(&frominet);
				}
			doit();
			exit(0);
		}
		(void) close(s);
	}
}

void
reapchild()
{
	int status;

	while (waitpid(-1, &status, WNOHANG) > 0)
		;
}

void
mcleanup()
{
	if (lflag)
		syslog(LOG_INFO, "exiting");
	unlink(_PATH_SOCKETNAME);
	exit(0);
}

void
alarm_timeout()
{

}

/*
 * Stuff for handling job specifications
 */
char	*user[MAXUSERS];	/* users to process */
int	users;			/* # of users in user array */
int	requ[MAXREQUESTS];	/* job number of spool entries */
int	requests;		/* # of spool requests */
char	*person;		/* name of person doing lprm */

char	fromb[32];	/* buffer for client's machine name */
char	cbuf[BUFSIZ];	/* command line buffer */
char	*cmdnames[] = {
	"null",
	"printjob",
	"recvjob",
	"displayq short",
	"displayq long",
	"rmjob"
};

void
doit()
{
	register char *cp;
	register int n;

	for (;;) {
		cp = cbuf;
		do {
			if (cp >= &cbuf[sizeof(cbuf) - 1])
				fatal("Command line too long");
			if ((n = read(1, cp, 1)) != 1) {
				if (n < 0)
					fatal("Lost connection");
				return;
			}
		} while (*cp++ != '\n');
		*--cp = '\0';
		cp = cbuf;
		if (lflag) {
			if (*cp >= '\1' && *cp <= '\5')
				syslog(LOG_INFO, "%s requests %s %s",
					from, cmdnames[*cp], cp+1);
			else
				syslog(LOG_INFO, "bad request (%d) from %s",
					*cp, from);
		}
		switch (*cp++) {
		case '\1':	/* check the queue and print any jobs there */
			printer = cp;
			printjob();
			break;
		case '\2':	/* receive files to be queued */
			if (!from_remote) {
				syslog(LOG_INFO, "illegal request (%d)", *cp);
				exit(1);
			}
			printer = cp;
			recvjob();
			break;
		case '\3':	/* display the queue (short form) */
		case '\4':	/* display the queue (long form) */
			printer = cp;
			while (*cp) {
				if (*cp != ' ') {
					cp++;
					continue;
				}
				*cp++ = '\0';
				while (isspace(*cp))
					cp++;
				if (*cp == '\0')
					break;
				if (isdigit(*cp)) {
					if (requests >= MAXREQUESTS)
						fatal("Too many requests");
					requ[requests++] = atoi(cp);
				} else {
					if (users >= MAXUSERS)
						fatal("Too many users");
					user[users++] = cp;
				}
			}
			displayq(cbuf[0] - '\3');
			exit(0);
		case '\5':	/* remove a job from the queue */
			if (!from_remote) {
				syslog(LOG_INFO, "illegal request (%d)", *cp);
				exit(1);
			}
			printer = cp;
			while (*cp && *cp != ' ')
				cp++;
			if (!*cp)
				break;
			*cp++ = '\0';
			person = cp;
			while (*cp) {
				if (*cp != ' ') {
					cp++;
					continue;
				}
				*cp++ = '\0';
				while (isspace(*cp))
					cp++;
				if (*cp == '\0')
					break;
				if (isdigit(*cp)) {
					if (requests >= MAXREQUESTS)
						fatal("Too many requests");
					requ[requests++] = atoi(cp);
				} else {
					if (users >= MAXUSERS)
						fatal("Too many users");
					user[users++] = cp;
				}
			}
			rmjob();
			break;
		}
		fatal("Illegal service request");
	}
}

/*
 * Make a pass through the printcap database and start printing any
 * files left from the last time the machine went down.
 */
void
startup()
{
	char buf[BUFSIZ];
	register char *cp;
	int pid;
	int pos;

	int ret, i = 0;
	

	printer = buf;

	/*
	 * Restart the daemons.
	 */
	while ((ret = (getprent(buf))) > 0) {
		for (cp = buf; *cp; cp++)
			if (*cp == '|' || *cp == ':') {
				*cp = '\0';
				break;
			}

		if ((pid = fork()) < 0) {
			syslog(LOG_WARNING, "startup: cannot fork");
			mcleanup();
		}
		if (!pid) {
			raise(SIGSTOP);
			endprent();
			printjob();
		}
	}
	kill(0, SIGCONT);
}

#define DUMMY ":nobody::"

/*
 * Check to see if the from host has access to the line printer.
 */
void
chkhost(f)
	struct sockaddr_in *f;
{
	register struct hostent *hp;
	register FILE *hostf;
	register char *cp, *sp;
	char ahost[50];
	int first = 1;
	extern char *inet_ntoa();
	int baselen = -1;

	if (g_nocheckhost) return;
	
	f->sin_port = ntohs(f->sin_port);
	if (f->sin_family != AF_INET || f->sin_port >= IPPORT_RESERVED)
		fatal("Malformed from address");
	hp = gethostbyaddr((char *)&f->sin_addr,
	    sizeof(struct in_addr), f->sin_family);
	if (hp == 0)
		fatal("Host name for your address (%s) unknown",
			inet_ntoa(f->sin_addr));

	strcpy(fromb, hp->h_name);
	from = fromb;
	if (!strcmp(from, host))
		return;

	sp = fromb;
	cp = ahost;
	while (*sp) {
		if (*sp == '.') {
			if (baselen == -1)
				baselen = sp - fromb;
			*cp++ = *sp++;
		} else {
			*cp++ = isupper(*sp) ? tolower(*sp++) : *sp++;
		}
	}
	*cp = '\0';
	hostf = fopen(_PATH_HOSTSEQUIV, "r");
again:
	if (hostf) {
//		if (!__ivaliduser(hostf, ahost, DUMMY, DUMMY, baselen)) {
		if (!__ivaliduser(hostf, f->sin_addr.s_addr, DUMMY, DUMMY)) {
			(void) fclose(hostf);
			return;
		}
		(void) fclose(hostf);
	}
	if (first == 1) {
		first = 0;
		hostf = fopen(_PATH_HOSTSLPD, "r");
		goto again;
	}
	fatal("Your host %s does not have line printer access", from);
	syslog(LOG_INFO,"Your host %s does not have line printer access",from);
}
