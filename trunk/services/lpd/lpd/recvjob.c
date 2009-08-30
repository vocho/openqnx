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
static char sccsid[] = "@(#)recvjob.c	5.15 (Berkeley) 5/4/91";
#endif /* not lint */

/*
 * Receive printer jobs from the network, queue them and
 * start the printer daemon.
 */

#include "lp.h"
#include "pathnames.h"
#include "protos.h"
#include <string.h>
#include <stdlib.h>
#ifdef __QNXNTO__
#include <sys/statvfs.h>
#endif

char	*sp = "";
#define ack()	(void) write(1, sp, 1);

extern int lflag;
char    tfname[40];		/* tmp copy of cf before linking */
char    dfname[40];		/* data files */
int	minfree;		/* keep at least minfree blocks available */

void	rcleanup();

void
recvjob()
{
	struct stat stb;
	char *bp = pbuf;
	int status;

	/*
	 * Perform lookup for printer name or abbreviation
	 */
	if ((status = pgetent(line, printer)) < 0)
		frecverr("cannot open printer description file");
	else if (status == 0)
		frecverr("unknown printer %s", printer);
	if ((LF = pgetstr("lf", &bp)) == NULL)
		LF = _PATH_CONSOLE;
	if ((SD = pgetstr("sd", &bp)) == NULL)
		SD = _PATH_DEFSPOOL;
	if ((LO = pgetstr("lo", &bp)) == NULL)
		LO = DEFLOCK;

	(void) close(2);			/* set up log file */
	if (open(LF, O_WRONLY|O_APPEND, 0664) < 0) {
		syslog(LOG_ERR, "%s: %m", LF);
		(void) open(_PATH_DEVNULL, O_WRONLY);
	}

	if (chdir(SD) < 0)
		frecverr("%s: %s: %m", printer, SD);
	if (stat(LO, &stb) == 0) {
		if (stb.st_mode & 010) {
			/* queue is disabled */
			putchar('\1');		/* return error code */
			exit(1);
		}
	} else if (stat(SD, &stb) < 0)
		frecverr("%s: %s: %m", printer, SD);
	minfree = 2 * read_number("minfree");	/* scale KB to 512 blocks */
	signal(SIGTERM, (void *)rcleanup);
	signal(SIGPIPE, (void *)rcleanup);

	if (readjob())
		printjob();
}

/*
 * Read printer jobs sent by lpd and copy them to the spooling directory.
 * Return the number of jobs successfully transfered.
 */
readjob()
{
	register int nfiles;
	long size;
	register char *cp;

	ack();
	nfiles = 0;
	for (;;) {
		/*
		 * Read a command to tell us what to do
		 */
		cp = line;
		do {
			if ((size = read(1, cp, 1)) != 1) {
				if (size < 0)
					frecverr("%s: Lost connection",printer);
				return(nfiles);
			}
		} while (*cp++ != '\n');
		*--cp = '\0';
		cp = line;
		switch (*cp++) {
		case '\1':	/* cleanup because data sent was bad */
			rcleanup();
			continue;

		case '\2':	/* read cf file */
			size = 0;
			while (*cp >= '0' && *cp <= '9')
				size = size * 10 + (*cp++ - '0');
			if (*cp++ != ' ')
				break;
			/*
			 * host name has been authenticated, we use our
			 * view of the host name since we may be passed
			 * something different than what gethostbyaddr()
			 * returns
			 */
			strcpy(cp + 6, from);
			strcpy(tfname, cp);
			tfname[0] = 't';
			if (!chksize(size)) {
				(void) write(1, "\2", 1);
				continue;
			}
			if (!readfile(tfname, size)) {
				rcleanup();
				continue;
			}
			if (NULL == getenv("DONT_USE_LINK_UNLINK")) {
				if (link(tfname, cp) < 0)
					frecverr("%s: %m", tfname);
				(void) unlink(tfname);
			} else {
				if (rename(tfname, cp) < 0)
					frecverr("%s: %m", tfname);
			}
			tfname[0] = '\0';
			nfiles++;
			continue;

		case '\3':	/* read df file */
			size = 0;
			while (*cp >= '0' && *cp <= '9')
				size = size * 10 + (*cp++ - '0');
			if (*cp++ != ' ')
				break;
			if (!chksize(size)) {
				(void) write(1, "\2", 1);
				continue;
			}
			(void) strcpy(dfname, cp);
			if (index(dfname, '/'))
				frecverr("readjob: %s: illegal path name",
					dfname);
			(void) readfile(dfname, size);
			continue;
		}
frecverr("%20c", cp-10);	
		frecverr("protocol screwup");
	}
}

/*
 * Read files send by lpd and copy them to the spooling directory.
 */
readfile(file, size)
	signed char *file;
	long size;
{
	register char *cp;
	char buf[BUFSIZ];
	long i, j, amt;
	int fd, err;

	if (lflag > 1) 
		syslog(LOG_INFO, "Received file %s of size %d", file, size);
	fd = open(file, O_CREAT|O_EXCL|O_WRONLY, FILMOD);
	if (fd < 0)
		frecverr("readfile: %s: illegal path name: %m", file);
	ack();
	err = 0;
	for (i = 0; i < size; i += BUFSIZ) {
		amt = BUFSIZ;
		cp = buf;
		if (i + amt > size)
			amt = size - i;
		do {
			j = read(1, cp, amt);
			if (j <= 0)
				frecverr("Lost connection");
			amt -= j;
			cp += j;
		} while (amt > 0);
		amt = BUFSIZ;
		if (i + amt > size)
			amt = size - i;
		if (write(fd, buf, amt) != amt) {
			err++;
			frecverr("Write error");
			break;
		}
	}
	(void) close(fd);
	if (err)
		frecverr("%s: write error", file);
	if (noresponse()) {		/* file sent had bad data in it */
		(void) unlink(file);
		return(0);
	}
	ack();
	return(1);
}

noresponse()
{
	char resp;

	if (read(1, &resp, 1) != 1)
		frecverr("Lost connection");
	if (resp == '\0')
		return(0);
	return(1);
}

/*
 * Check to see if there is enough space on the disk for size bytes.
 * 1 == OK, 0 == Not OK.
 */
chksize(size)
	long size;
{
	int spacefree;
#ifndef __QNXNTO__
	struct /* statfs */
	{
		long f_bavail;
		long f_fsize;
	} sfb;
#else
	struct statvfs sfb;
#endif
	int fd;

	if ((fd = open(SD, O_RDONLY)) == -1) {
		perror("the open failed");
		syslog(LOG_ERR, "%s: %m", "statfs(\".\")");
		return (1);
	}
#ifdef __QNXNTO__
        if (statvfs(".",&sfb) <0) {
   		perror("statfs failed");
		syslog(LOG_ERR, "%s: %m","statvfs(\".\")");
		return(1);
        }
        spacefree = sfb.f_bavail * (sfb.f_frsize/512);

#else
	if (disk_space(fd, &sfb.f_bavail, &sfb.f_fsize) == -1) {
//	if (statfs(., &sfb) < 0) {
		perror("the disk_space failed");
		syslog(LOG_ERR, "%s: %m", "statfs(\".\")");
		return (1);
	}
//	spacefree = sfb.f_bavail * (sfb.f_fsize / 512);
	spacefree = sfb.f_bavail;
#endif
	size = (size + 511) / 512;
	if (minfree + size > spacefree)
		return(0);
	return(1);
}

read_number(fn)
	signed char *fn;
{
	char lin[80];
	register FILE *fp;

	if ((fp = fopen(fn, "r")) == NULL)
		return (0);
	if (fgets(lin, 80, fp) == NULL) {
		fclose(fp);
		return (0);
	}
	fclose(fp);
	return (atoi(lin));
}

/*
 * Remove all the files associated with the current job being transfered.
 */
void
rcleanup()
{

	if (tfname[0])
		(void) unlink(tfname);
	if (dfname[0])
		do {
			do
				(void) unlink(dfname);
			while (dfname[2]-- != 'A');
			dfname[2] = 'z';
		} while (dfname[0]-- != 'd');
	dfname[0] = '\0';
}

#include <stdarg.h>
//void
void frecverr(signed char *msg, ... ) 
//	signed char *msg;
{
	va_list ap;
	rcleanup();
va_start(ap, msg);
	syslog(LOG_ERR, msg,va_arg(ap, int), va_arg(ap, int));
	putchar('\1');		/* return error code */
	exit(1);
}
