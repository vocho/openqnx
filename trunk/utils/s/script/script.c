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



/*	$OpenBSD: script.c,v 1.12 2000/04/16 20:28:54 espie Exp $	*/
/*	$NetBSD: script.c,v 1.3 1994/12/21 08:55:43 jtc Exp $	*/

/*
 * Copyright (c) 1980, 1992, 1993
 *	The Regents of the University of California.  All rights reserved.
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
#ifdef __GNUC__
static char __attribute__ ((unused)) copyright[] =
"@(#) Copyright (c) 1980, 1992, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#else 
static char copyright[] =
"@(#) Copyright (c) 1980, 1992, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif 
#endif /* not lint */

#ifndef lint
#if 0
static char sccsid[] = "@(#)script.c	8.1 (Berkeley) 6/6/93";
#endif
#ifdef __GNUC__
static char __attribute__ ((unused)) rcsid[] = "$OpenBSD: script.c,v 1.12 2000/04/16 20:28:54 espie Exp $";
#else 
static char copyright[] =
"@(#) Copyright (c) 1980, 1992, 1993\n\
	The Regents of the University of California.  All rights reserved.\n";
#endif
#endif /* not lint */

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>

#include <errno.h>
#include <fcntl.h>
#include <paths.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#ifndef __QNX__
#include <tzfile.h>
#endif
#include <unistd.h>

#ifdef __QNX__
#include <unix.h>
#define SECSPERMIN 60
#else
#include <util.h>
#endif
#include <err.h>

FILE	*fscript;
int	master, slave;
int	child, subchild;
int	outcc;
char	*fname;
int 	time_it;
time_t	last_time;
int		saw_nl;

struct	termios tt;

__dead	void done __P((int));
	void dooutput __P((void));
	void doshell __P((void));
	void fail __P((void));
	void finish __P((int));
	void scriptflush __P((int));
	void handlesigwinch __P((int));


int
main(argc, argv)
	int argc;
	char *argv[];
{
	register int cc;
	struct termios rtt;
	struct winsize win;
	int aflg, ch;
	char ibuf[BUFSIZ];

	aflg = 0;
	while ((ch = getopt(argc, argv, "at")) != -1)
		switch(ch) {
		case 'a':
			aflg = 1;
			break;
		case 't':
			time_it	= 1;
			break;
		case '?':
		default:
			(void)fprintf(stderr, "usage: script [-a] [file]\n");
			exit(1);
		}
	argc -= optind;
	argv += optind;

	if (argc > 0)
		fname = argv[0];
	else
		fname = "typescript";

	if ((fscript = fopen(fname, aflg ? "a" : "w")) == NULL)
		err(1, fname);

	(void)tcgetattr(STDIN_FILENO, &tt);
	(void)ioctl(STDIN_FILENO, TIOCGWINSZ, &win);
	if (openpty(&master, &slave, NULL, &tt, &win) == -1)
		err(1, "openpty");

	(void)printf("Script started, output file is %s\n", fname);
	rtt = tt;
	cfmakeraw(&rtt);
	rtt.c_lflag &= ~ECHO;
	(void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &rtt);

	(void)signal(SIGWINCH, handlesigwinch);
	(void)signal(SIGCHLD, finish);
	child = fork();
	if (child < 0) {
		perror("fork");
		fail();
	}
	if (child == 0) {
		subchild = child = fork();
		if (child < 0) {
			perror("fork");
			fail();
		}
		if (child)
			dooutput();
		else
			doshell();
	}

	(void)fclose(fscript);
	while ((cc = read(STDIN_FILENO, ibuf, BUFSIZ)) > 0)
		(void)write(master, ibuf, cc);
	done(0);
	return 0;
}

void
finish(signo)
	int signo;
{
	register int die, pid;
	int save_errno = errno;
	int status, e;

	die = e = 0;
	while ((pid = wait3(&status, WNOHANG, 0)) > 0)
		if (pid == child) {
			die = 1;
			if (WIFEXITED(status))
                                e = WEXITSTATUS(status);
                        else
                                e = 1;
		}

	if (die)
		done(e);
	errno = save_errno;
}

void
handlesigwinch(signo)
	int signo;
{
	struct winsize win;
	pid_t pgrp;
	int save_errno = errno;

	if (ioctl(STDIN_FILENO, TIOCGWINSZ, &win) != -1) {
	    ioctl(slave, TIOCSWINSZ, &win);
	    if (ioctl(slave, TIOCGPGRP, &pgrp) != -1)
	    	killpg(pgrp, SIGWINCH);
	}
	errno = save_errno;
}

void
dooutput()
{
	struct itimerval value;
	register int cc;
	time_t tvec;
	char obuf[BUFSIZ];

	(void)close(STDIN_FILENO);
	tvec = time(NULL);
	(void)fprintf(fscript, "Script started on %s", ctime(&tvec));

	(void)signal(SIGALRM, scriptflush);
	value.it_interval.tv_sec = SECSPERMIN / 2;
	value.it_interval.tv_usec = 0;
	value.it_value = value.it_interval;
	(void)setitimer(ITIMER_REAL, &value, NULL);
	last_time = time(NULL);
	saw_nl = 1;
	for (;;) {
		cc = read(master, obuf, sizeof (obuf));
		if (cc == 0)
			break;
		if (cc == -1) {
			if (errno == EINTR)
				continue;
			break;
		}
		(void)write(1, obuf, cc);
		if(time_it) {
			char		*start;
			char		*end;
			char		*curr;
			time_t		curr_time;
			
			end = &obuf[cc];
			start = obuf;
			for(curr = start; curr < end; ++curr) {
				if(*curr == '\n') {
					saw_nl = 1;
				} else if(saw_nl) {
					saw_nl = 0;
					curr_time = time(NULL);
					(void)fwrite(start, 1, curr - start, fscript);
					start = curr;
					fprintf(fscript, "[%lu]", (unsigned long) (curr_time - last_time));
					last_time = curr_time;
				}
			}
			if(start < curr) {
				(void)fwrite(start, 1, curr - start, fscript);
			}
		} else {
			(void)fwrite(obuf, 1, cc, fscript);
		}
		outcc += cc;
	}
	done(0);
}

void
scriptflush(signo)
	int signo;
{
	int save_errno = errno;

	if (outcc) {
		(void)fflush(fscript);
		outcc = 0;
	}
	errno = save_errno;
}

void
doshell()
{
	char *shell;

	shell = getenv("SHELL");
	if (shell == NULL)
		shell = _PATH_BSHELL;

	(void)close(master);
	(void)fclose(fscript);
	login_tty(slave);
	execl(shell, shell, "-i", NULL);
	perror(shell);
	fail();
}

void
fail()
{

	(void)kill(0, SIGTERM);
	done(1);
}

void
done(eval)
	int eval;
{
	time_t tvec;

	if (subchild) {
		tvec = time(NULL);
		(void)fprintf(fscript,"\nScript done on %s", ctime(&tvec));
		(void)fclose(fscript);
		(void)close(master);
	} else {
		(void)tcsetattr(STDIN_FILENO, TCSAFLUSH, &tt);
		(void)printf("Script done, output file is %s\n", fname);
	}
 	exit(eval);
}

