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





#ifdef __USAGE		/* sleep.c */
%C - suspend execution for an interval (POSIX)

%C	time
Where:
 time  is the number of seconds to sleep (0 <= time <= 4294967295).
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.13  2007/04/30 20:01:44  aristovski
	PR: 46523
	CI: rmansfield

	added win32 dir. structure
	Added WIN32_ENVIRON
	Added #include <lib/compat.h>
	added __MINGW32__ ifdef cases,
	code for mingw32

	Revision 1.12  2005/06/03 01:38:00  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.11  2003/08/29 20:29:28  martin
	Update QSSL Copyright.
	
	Revision 1.10  1999/07/06 18:00:22  bstecher
	Wasn't checking proper value to determine error return from alarm() call.
	
	Revision 1.9  1998/09/25 20:46:27  eric
	minor error checking tweak
	
	Revision 1.8  1998/08/19 17:13:07  bstecher
	*** empty log message ***

	Revision 1.7  1998/08/19 17:09:58  eric
	/

	Revision 1.6  1996/10/30 18:48:40  eric
	removed reference to stdutil.h

	Revision 1.5  1996/09/16 18:52:55  eric
	fixed for nto (removed need_usage)

	Revision 1.4  1996/09/16 18:48:55  steve
	(eric) checking in - dunno what changes steve made

 * Revision 1.3  1993/08/18  19:31:53  eric
 * fixed buf where sleep 0 would return failure (should always succeed)
 *
 * fixed buf where sleep would never return if it was unable to
 * set an alarm (no timers); will now print an error msg to
 * standard error and terminate with a failure.
 *
 * Revision 1.2  1992/10/27  17:55:58  eric
 * added usage one-liner
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *

	$Author: coreos $
	
---------------------------------------------------------------------*/
#ifdef __MINGW32__
#include <windows.h>
#include <lib/compat.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

void	sigalarm( int );
static int do_pause(int seconds);
static void install_sh();

int
main( argc, argv )
int			argc;
char		*argv[];
{
	unsigned long nsec;
	int errflag = 0;
	int c;
	char *endptr;

	errno = 0;	/* cuz entry wont do it for me */

	while ((c=getopt(argc,argv,"")) != -1) 
		errflag++;

	if (!errflag && (optind==argc-1)) {
		errno=0;
		nsec = strtoul(argv[optind],&endptr,10);
	} else {
		fprintf(stderr,"sleep: Invalid number of operands.\n");
		errflag++;
		errno=0;
		nsec = 0;
	}

	if (errflag || errno!=0 || *endptr ) {
		if (!errflag) fprintf(stderr,"sleep: Invalid number of seconds ('%s') Must be 0<=n<=4294967295\n",argv[optind]);

		exit(EXIT_FAILURE);
	}
	
	if (nsec==0) exit(EXIT_SUCCESS);
	
	// setup signal handler:
	install_sh();
	
	return do_pause(nsec);
}

void sigalarm( int sig_number ) 
{
  #ifndef __MINGW32__ // Unix behaviour:
	exit( (sig_number==SIGALRM)?EXIT_SUCCESS:EXIT_FAILURE );
  #else // win32
	if (sig_number == SIGINT) {
	    exit(EXIT_FAILURE);
	} else {
	    exit(EXIT_SUCCESS);
	}
  #endif
}

void install_sh() {
  #ifndef __MINGW32__ 
    signal( SIGALRM, sigalarm );
  #else // win32:
    signal(SIGINT, sigalarm); // we catch Ctrl-C signal on win32
  #endif
}

int do_pause(int seconds) {
  #ifndef __MINGW32__
    if (alarm(seconds)!=((unsigned)-1)) pause();
	else fprintf(stderr,"sleep: can't set timer (%s)\n",strerror(errno));
	return EXIT_FAILURE;
  #else // we are in __MINGW32__ environ
    Sleep(seconds * 1000); // Win32 function Sleep takes number of milliseconds as argument.
    return EXIT_SUCCESS;
  #endif	
}
