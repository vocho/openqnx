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





#ifdef __USAGE		/* rmdir.c */
%C - remove directories (POSIX)

%C	[-p] dir...
Options:
 -p  For each directory:
       (1) the directory entry it names is removed.
       (2) if the dir operand includes more than one pathname
           component, and the parent directory of the directory
           that was removed is now empty, it removes that directory
Where:
 dir  is the pathname of an empty directory to be removed.
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.8  2006/04/11 16:16:16  rmansfield
	PR: 23548
	CI: kewarken

	Stage 1 of the warnings/errors cleanup.

	Revision 1.7  2005/06/03 01:37:58  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.6  2003/08/28 20:43:36  martin
	Add QSSL Copyright.
	
	Revision 1.5  1996/01/26 19:37:34  dtdodge
	*** empty log message ***
	
	Revision 1.4  1994/11/17 20:01:47  eric
	fixed exit status to be 0 on total success and
	non-zero if any failure occurred.

 * Revision 1.3  1992/10/27  17:49:49  eric
 * added usage one-liner
 *
 * Revision 1.2  1992/07/09  14:19:21  garry
 * Editted usage
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *

	$Author: coreos $
	
---------------------------------------------------------------------*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifndef TRUE
#	define TRUE  	(1==1)
#	define FALSE 	(1==0)
#endif

int main(int argc, char *argv[])
{
	register int	 i;
	register char   *filename,
					*p;
	int              error = 0,
					 pflag = 0;

	while(( i= getopt( argc, argv, "p")) != -1)
	{
		switch (i) 	
		{
			case 'p':
				pflag++;
				break;

			case '?':
				error++;                
				break;                   
		}
	}

	if (optind >= argc)       /* no directory names specified */
	{
		fprintf( stderr, "no directory specified\n" );
		error++;
	}

	if (error)
		exit( EXIT_FAILURE );

	for( ; optind < argc; optind++ ) 
	{
		filename = argv[optind];
		if(rmdir(filename) == -1) {
			error++;
			perror(filename);
		}

		else if(pflag)
		{
			while((p = strrchr(filename, '/')) != NULL)
			{
				/*
				 *	Stop once you've hit the root.
				 */
				if(p == filename)
					break;

				*p = '\0';
				if(rmdir(filename) == -1)
				{
						perror(filename);
						error++;
						break;
				}
			}
		}
	}

	exit(error?EXIT_FAILURE:EXIT_SUCCESS);
}
