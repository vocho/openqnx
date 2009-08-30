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




#ifdef __USAGE		/* uname.c */
%C - return system name (POSIX)

%C	[-amnrsv]
Options:
 -a       All options (as though all the options (-mnrsv) were specified).
 -b       OS native bit size (16 or 32) (write to standard output).
 -m       Machine name (write to standard output).
 -n       Node name (write to standard output).
 -r       Release level (write to standard output).
 -s       System (OS) name (write to standard output).
 -v       Version (write to standard output).
Note:
 If no options are specified, the uname utility writes the System name,
 as if the -s option had been specified.
#endif
	
/*---------------------------------------------------------------------



	$Header$

	$Source$

	$Log$
	Revision 1.8  2005/06/03 01:38:03  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.7  2004/04/17 17:58:51  thomasf
	Update licensing
	
	Revision 1.6  2001/05/15 20:12:40  bstecher
	Added licensing.
	
	Revision 1.5  1995/01/26 23:57:21  glen
	The extra spaces that were being output were breaking configure scripts.
	
 * Revision 1.4  1992/10/27  19:55:30  eric
 * added usage one-liner
 *
 * Revision 1.3  1992/10/02  18:11:24  dtdodge
 * Added a -b option.
 *
 * Revision 1.2  1992/07/09  14:46:13  garry
 * Editted usage
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *

	$Author: coreos $
	
---------------------------------------------------------------------*/
/*
 * Include declarations:
 */
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/utsname.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * Define declarations:
 */
#define TRUE	1
#define FALSE	0

#define MACHINE 1
#define NODE    2
#define RELEASE 4
#define OSFLAG  8
#define VERSION 16
#define BITS	32
#define ALL (MACHINE|NODE|RELEASE|OSFLAG|VERSION|BITS)

/*
 * Global Declarations
 */
int create_mask;



/*	Uname writes the current system name to standard output.  When options
 *	are specified, symbols representing one or more system characteristics are 
 *	written to the stdandard output.  
 */
main( argc, argv )
int argc;
char *argv[];
{
	static struct utsname name;
	int  result, i, error;
	int flags=0;

	result=error= FALSE;

	while(( i= getopt( argc, argv, "abmnrsv")) != -1)
		switch ( i ){
			case 'a':	flags |= ALL;
						break;
			case 'b':	flags |= BITS;
						break;
			case 'm':	flags |= MACHINE;   /* hardware machine name */
						break;
			case 'n':	flags |= NODE;      /* system node name */
						break;
			case 'r':	flags |= RELEASE;   /* OS release level */
						break;
			case 's':	flags |= OSFLAG;    /* OS name - also default */
						break;            
			case 'v':	flags |= VERSION;   /* OS version */
						break;
			case '?':	error = TRUE;       /* error */
						break;
			}

	if (flags == 0) 
		flags |= OSFLAG;

	if ( error )
		exit(EXIT_FAILURE);
	
	if (uname(&name)==-1) {
		perror("uname");
		exit(EXIT_FAILURE);
	}

#define OUT(flag, var) if(flags & flag) { printf("%s", var);\
										  if(flags & ~flag) printf(" "); }
	OUT(OSFLAG, name.sysname);
	OUT(NODE, name.nodename);
	OUT(RELEASE, name.release);
	OUT(VERSION, name.version);
	OUT(MACHINE, name.machine);
	OUT(BITS, name.bits);
	printf("\n");

	return(EXIT_SUCCESS);
}
