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





#ifdef __USAGE		/* pwd.c */
%C - print working directory name (POSIX)
    
%C [-w]
Options:
 -w    Separate paths with backslashes (\) rather than front (/) (Windows only)
Note:
   pwd is also a shell builtin.
#endif

/*---------------------------------------------------------------------


	$Header$

	$Source$

	$Log$
	Revision 1.15  2007/02/06 15:22:16  seanb
	- Back out conditionally terminating output with '\n'
	  which was added for PR 19924 as it breaks POSIX
	  conformance.
	PR:44519
	CI:skittur

	Revision 1.14  2006/04/11 16:16:08  rmansfield
	
	PR: 23548
	CI: kewarken
	
	Stage 1 of the warnings/errors cleanup.
	
	Revision 1.13  2005/06/03 01:37:57  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.12  2004/04/20 15:03:17  kewarken
	More fixes for PR:19924.  Make CR echo conditional on tty state.  CR: TF
	
	Revision 1.11  2004/04/15 15:40:46  kewarken
	Added -w option for display of windows style paths, made unix style default.
	CR: TF
	
	Revision 1.10  2003/08/28 15:49:45  martin
	Add QSSL Copyright.
	
	Revision 1.9  2002/01/11 20:13:48  bstecher
	Clean up a warning message.
	
	Revision 1.8  1999/03/11 23:13:33  cburgess
	added win32 build target and fixed pwd to cleanup backslashes for make.
	
	Revision 1.7  1998/09/16 14:48:34  builder
	gcc cleanup
	
	Revision 1.6  1997/03/14 18:16:00  eric
	removed nto ifdefs; corrected qnx4 version to not use
	print_usage

	Revision 1.5  1996/10/31 15:18:10  kirk
	added some #ifdefs so that this code will work this Neutrino

	Revision 1.4  1996/10/31 15:14:31  steve
	checked in by kirk
	looks like steve changed the source in include head stdutil.h instead
	of printmsg.h

 * Revision 1.3  1992/10/27  17:42:51  eric
 * added usage one-liner
 *
 * Revision 1.2  1992/07/09  14:13:45  garry
 * *** empty log message ***
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *

	$Author: CHarris@qnx.com $
	
---------------------------------------------------------------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <lib/compat.h>

#if defined(__NT__) || defined(__MINGW32__)
#include <fcntl.h>
#endif

/* Defines */

#define ERR   1
#define NOERR 0



int main( int argc, char * argv[])
{
	char path[PATH_MAX];
	int windows_path=0;

	if(argc > 1 && strcmp(argv[1], "-w") == 0)
		windows_path = 1;

	if (getcwd(path,PATH_MAX) != NULL) {
#if defined(__NT__) || defined(__MINGW32__)
		int i;
		MAKE_BINARY_FP(stdout);
		if(!windows_path)
			for ( i = 0; path[i]; i++ )
				if ( path[i] == '\\' )
					path[i] = '/';
#endif
		fprintf(stdout,"%s\n",path);

		exit(EXIT_SUCCESS);
	} else {
		perror("pwd");
		exit(EXIT_FAILURE);
	}

	return(EXIT_SUCCESS);
}
