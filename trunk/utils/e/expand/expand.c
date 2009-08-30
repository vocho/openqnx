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




#ifdef __USAGE		/* expand.c (phase 4) */
%C - convert tabs to spaces (POSIX)

%C	[-t tablist] [file ...]
Options:
 -t tablist   List of tab stops  ('4' or '4,12,64,...')
Where:
 tablist  consists of positive decimal integers, in ascending order,
          seperated by single commas.  If a single number is specified,
          tabs will be set tablist columns apart.  If multiple numbers
          are given, tabs will be set at those specific columns.
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.9  2006/04/11 16:15:44  rmansfield
	PR: 23548
	CI: kewarken

	Stage 1 of the warnings/errors cleanup.

	Revision 1.8  2005/06/03 01:37:45  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.7  2003/08/25 16:00:17  martin
	Add QSSL Copyright.
	
	Revision 1.6  1994/12/02 14:33:20  steve
	oops...
	
 * Revision 1.5  1994/11/30  19:14:59  steve
 * logic for scanning the tab string was broken.  It would scan a null pointer
 * for a ',' before exitting set_tabs().
 *
 * Revision 1.4  1992/10/27  19:57:34  eric
 * added usage one-liner
 *
 * Revision 1.3  1992/07/06  16:29:12  garry
 * Edit Usage.
 *
 * Revision 1.2  1991/11/25  21:48:50  eric
 * fixed problem with lack of range checking on -t tabstop options.
 * Previous version could cause Proc crash!!(!!!!!). Also fixed
 * error message for invalid options (was saying the invalid options
 * was a smiley face)
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *
	
---------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>

extern void	process( FILE * );
extern long	space_fill( long );
extern void	set_tabs( char * );

#define LINE_MAX	2048
#define IOBUF_SIZE	10240

int		tabs[LINE_MAX], tabcnt = 0;
int		tabsize = 8;

#define TXT(s)		s
#define T_NO_OPEN	"expand:  Unable to open '%s'\n"
#define T_TOO_FEW	"expand:  Too few argument passed on command line\n"
#define T_INVALID	"Invalid option ('%c')\n"

int main( int argc, char *argv[] )
	{
	FILE	*fp;
	char	*p = "8";	/* tabsize */
	int		i;

	opterr = 0;
	while( ( i = getopt( argc, argv, "t:" ) ) != -1 ) {
		switch( i ) {
			case 't':	p = optarg;				break;
			default:
						if ( isdigit( optopt ) )
							p = strchr( argv[optind-1], optopt );
						else {
							fprintf( stderr, TXT( T_INVALID ), optopt );
							exit( EXIT_FAILURE );
							}
			}
		}

	set_tabs( p );

	if ( optind < argc )
		for( ; optind < argc; optind++ ) {
			if ( !( fp = fopen( argv[optind], "r" ) ) ) {
				fprintf( stderr, TXT( T_NO_OPEN ), argv[optind] );
				continue;
				}
			process( fp );
			fclose( fp );
			}
	else
		process( stdin );

	exit( EXIT_SUCCESS );
	}

void
set_tabs( tlist )
	char	*tlist;
	{
	int	i;

	memset( tabs, 0, sizeof( tabs ) );
	tabcnt = 0;

	while (*tlist) {
		long        ltabsize;
		ltabsize = strtol(tlist, &tlist, 10);
		if (*tlist) {
			if (*tlist != ',') {
				fprintf(stderr,"expand: invalid character '%c'(%x) in tab spec\n",
					*tlist, *tlist);
				exit(EXIT_FAILURE);
			}
			tlist++;
		}
		if (ltabsize<1 || ltabsize>LINE_MAX) {
			fprintf(stderr,"expand: illegal tab size/stop (%ld)\n",ltabsize);
			exit(EXIT_FAILURE);
		}
		tabsize=(int)ltabsize;
		tabs[tabsize] = 1;
		tabcnt++;
	}
	if ( tabcnt == 1 ) { /* Only one argument for tablist */
		for( i = tabsize; i < LINE_MAX; i += tabsize, tabcnt++ )
			tabs[i] = 1;
		}
#ifdef DBUG
	for( i = 0; i < 132; i++ )
		if ( tabs[i] == 1 )
			fprintf( stderr, "tabstop at column %d\n", i );
#endif
	}

void
process( FILE *fp )
	{
	long	pos = 0L;
	int		c;

	setvbuf( fp, NULL, _IOFBF, IOBUF_SIZE );
	while( ( c = getc( fp ) ) != EOF ) {
		switch( c ) {
/* TAB		*/	case '\t':	pos = space_fill( pos );			break;
/* Newline	*/	case '\n':	pos = 0L;	putchar( c );			break;
/* Bkspace	*/	case '\b':	if ( pos - 1 > 0 ) pos--;			break;
/* Other	*/	default:	pos++;		putchar( c );			break;
			}
		}
	}

long
space_fill( long pos )
	{
	long	i;
	int		j;

	/* output atleast 1 space for a tab.. */
	for( i = pos+1, j = 1; i < LINE_MAX  &&  tabs[i] != 1; i++ )
		j++;
	if ( i == LINE_MAX ) {
		/* no more tab stops on this line, output a space */
		j = 1;
		}
	pos += j;
#ifdef DBUG
	printf( "[output %d spaces]", j );
#endif
	for( i = 0; i < j; i++ )
		putchar( ' ' );
	return( pos );
	}
