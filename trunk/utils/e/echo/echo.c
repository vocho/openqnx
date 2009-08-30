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




#ifdef __USAGE		/* echo.c */
%C - write arguments to standard output (POSIX)

%C	[-n] [string...]
Where:
 -n 	Do not print a trailing newline.

 string is the string to be echoed to standard output.
Note:
   The following backslash sequences are processed within strings:

   \0nnn   nnn is an octal representation of a character
   \a      Bell (alert)
   \b      Backspace
   \c      End Of String with no newline
   \f      Form Feed
   \n      Newline
   \r      Carriage Return
   \t      Horizontal Tab
   \v      Vertical Tab
   \\      A real backslash
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.10  2006/04/11 16:15:42  rmansfield
	PR: 23548
	CI: kewarken

	Stage 1 of the warnings/errors cleanup.

	Revision 1.9  2005/06/03 01:37:45  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.8  2003/11/28 21:27:37  kewarken
	win32 variant + port
	
	Revision 1.7  2003/08/25 15:35:06  martin
	Update QSSL Copyright.
	
	Revision 1.6  1999/08/09 14:33:43  spuri
	Added the -n option to suppress the trailing newline.
	
	Revision 1.5  1998/05/01 16:03:24  eric
	reorg for gcc, fixed \v
	
	Revision 1.4  1997/02/12 18:39:06  eric
	got rid of stdutil.h ifdef

	Revision 1.3  1996/10/02 15:31:08  eric
	nto port

 * Revision 1.2  1992/10/26  21:40:55  eric
 * added usage one-liner
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *

	$Author: coreos $
	
---------------------------------------------------------------------*/

/*
 * Purpose:
 *				Writes its arguments to standard out.  If the -n is specified,
 *				the newline character is suppressed.
 *
 * Note:		See Draft 8 of December 5, 1988 Section 4.19.1
 *
 *
 * Include declarations:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <util/stdutil.h>

/*
 * Define declarations:
 */

#ifndef TRUE
#	define TRUE  	(1==1)
#	define FALSE 	(1==0)
#endif

/*
 * Global declarations:
 */

int  sum;

/*
 *- ---------------------------------------------------------------------------
 */

int octalnum( c )						/* converts octal to decimal */
char c;
	{
	if( (c >= '0') && (c <= '7') )
		{
		sum *= 8;
		sum += (c - '0');
		return( TRUE );
		}
	else
		return( FALSE );
	}


int main( argc, argv )
int argc;
char *argv[];
	{
	register char *p;
	register int   i = 1;
	int  nlflag = 1;

	// if the 1st option is "-n", don't print a newline
	if ((argc >= 2) && (strcmp (argv[1], "-n") == 0))
	{
		i++;	// skip the 1st option
		nlflag = 0;
	}

	for (; i < argc;  )
		{
		for( p = argv[i++]; *p; ++p )
			{
			if( *p == '\\' )
				{
				switch( *++p )
					{
					case 'a':
						putchar('\a');
						break;
					case 'b':
						putchar('\b');
						break;
					case 'c':
						exit( 0 );
						break;
					case 'f':
						putchar('\f');
						break;
					case 'n':
						putchar('\n');
						break;
					case 'r':
						putchar('\r');
						break;
					case 't':
						putchar('\t');
						break;
					case 'v':
						putchar('\v');
						break;
					case '\\':
						putchar('\\');
						break;
					case '0':
							{
							int count = 0;
							for( count=0, sum=0; (count<=3) && ((octalnum(*++p) != FALSE)); ++count )
								;
							}
						putchar( sum );
						break;
					case 0:
						break;				/* hit end of string */
					default:
						putchar( *p );
						break;
					}
				}
			else
				putchar( *p );
			}

		if( i < argc )
			putchar(' ');
		}

	if( nlflag )
		putchar( '\n' );

	return 0;
	}

