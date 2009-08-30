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





#ifdef __USAGE		/* time.c */
%C - determine the execution time of a command (UNIX)

%C [-p] command_to_time [command_arguments]*
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.8  2005/06/03 01:38:02  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.7  2004/08/03 22:22:10  jgarvey
	CLK_TCK is actually a sysconf() call under QNX6, so do it once and
	cache the answer to save overheads.
	
	Revision 1.6  2004/08/03 01:29:23  jgarvey
	Bring time into line with POSIX2 specification: add a '-p' option
	which has a well-specified output format.  Utility now matches the
	behaviour of the pdksh builtin.
	
	Revision 1.5  2003/09/02 14:39:34  martin
	Add QSSL Copyright.
	
	Revision 1.4  2002/04/02 21:49:31  kewarken
	fixed mixing of stderr and stdout on output
	
	Revision 1.3  2000/11/29 16:14:02  kewarken
	Fixed time to account for different CLK_TCKs
	
	Revision 1.2  1998/11/25 22:50:36  eric
	ported to GNU, added target dirs
	
	Revision 1.1  1998/10/23 20:53:55  trs
	initial checkin
	
	Revision 1.4  1998/10/23 20:36:20  trs
	*** empty log message ***

 * Revision 1.3  1992/10/27  19:30:29  eric
 * added usage one-liner
 *
 * Revision 1.2  1992/07/09  14:39:04  garry
 * *** empty log message ***
 *
 * Revision 1.1  1991/07/11  13:40:54  brianc
 * Initial revision
 *
	
	Revision 1.2 Sun Apr  1 18:11:48 1990 opr
	Output to stderr instead of stdout
	
	Revision 1.1 Wed Mar 14 13:28:33 1990 glen
	 *** QRCS - Initial Revision ***
	
---------------------------------------------------------------------*/

#include <time.h>
#include <sys/times.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <process.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>

#include <util/stdutil.h>
#include <util/util_limits.h>

long		clk_tck;		/* Local/Cached CLK_TCK (sysconf() call) */

/*
 * print out time in hh:mm:ss.ms or POSIX format (seconds) given the
 * real/user/system times in clock ticks (CLK_TCK)
 */
#define TCK_PER_SEC		clk_tck
#define TCK_PER_MIN		(60L * TCK_PER_SEC)
#define TCK_PER_HOUR	(60L * TCK_PER_MIN)
static void print_time(char *hdr, clock_t tck) {
	unsigned hh, mm, ss;
	float ms;

	if ((hh = tck / TCK_PER_HOUR) != 0) {
		fprintf(stderr, "%02u:", hh);
		tck %= TCK_PER_HOUR;
	}
	if ((mm = tck / TCK_PER_MIN) != 0 || hh != 0) {
		fprintf(stderr, "%02u:", mm);
		tck %= TCK_PER_MIN;
	}
	ss = tck / TCK_PER_SEC;
	ms = (tck % clk_tck) * 1000.0/clk_tck;
	fprintf(stderr, "%02u.%03us %s", ss, (unsigned)ms, hdr);
}
void print_posix(char *hdr, clock_t tck) {
int		precision;

	if (clk_tck <= 10)			precision = 1;
	else if (clk_tck <= 100)	precision = 2;
	else						precision = 3;
	fprintf(stderr, "%s %8.*f\n", hdr, precision, (double)tck / clk_tck);
}

void catch(int signo) {
	fprintf(stderr, "time: command terminated abnormally (%d)\n",signo);
}

int main(int argc, char **argv) {
	char	resolve[UTIL_PATH_MAX + 1];
	struct	tms t1, t2;				/* user/system times */
	clock_t	c1, c2;					/* elapsed times */
	int		rc;						/* exit status */
	int		opt, posix;

	posix = 0, clk_tck = CLK_TCK;
	while ((opt = getopt(argc, argv, ":p")) != -1) {
		switch (opt) {
		case 'p':
			posix = !0;
			break;
		case ':':
			fprintf(stderr,"time: missing argument for '-%c'\n", optopt);
			exit(EXIT_FAILURE);
		case '?':
			fprintf(stderr,"time: unknown option '-%c'\n", optopt);
			exit(EXIT_FAILURE);
		}
	}

	if (optind >= argc) {
		fprintf(stderr,"time: must specify a command to be timed\n");
		exit(EXIT_FAILURE);
	}

	if (strlen(argv[optind]) >= sizeof(resolve)) {
		fprintf(stderr,"time: %s: %s\n", argv[optind], strerror(ENAMETOOLONG));
		exit(EXIT_FAILURE);
	}
	searchenv(argv[optind], "PATH", resolve);
	if (resolve[0]) {
		/* don't ignore as child will inherit */
		signal(SIGINT, catch), signal(SIGQUIT, catch);
#ifdef __QNXNTO__
		_fullpath(resolve, resolve, sizeof(resolve));
#else
		qnx_fullpath(resolve, resolve);
#endif
		c1 = times(&t1);
		rc = spawnv(P_WAIT, resolve, &argv[optind]);
		c2 = times(&t2);

		if (c1 == (clock_t)-1 || c2 == (clock_t)-1) {
			fprintf(stderr, "time: unable to read system time-stamps\n");
		}
		else if (posix) {
			print_posix("real", c2 - c1);
			print_posix("user", t2.tms_cutime - t1.tms_cutime);
			print_posix("sys ", t2.tms_cstime - t1.tms_cstime);
		}
		else {
			print_time("real   ", c2 - c1);
			print_time("user   ", t2.tms_cutime - t1.tms_cutime);
			print_time("system\n", t2.tms_cstime - t1.tms_cstime);
		}
	}
	else {
		fprintf(stderr, "time: %s (%s)\n", strerror(errno), argv[optind]);
		rc = EXIT_FAILURE;
	}

	return rc;
}
