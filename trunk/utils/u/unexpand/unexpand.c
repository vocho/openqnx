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





#ifdef __USAGE		/* unexpand.c */
%C - convert spaces to tabs (POSIX)

%C	[-a] [-t tabsize] [file ...]
Options:
 -a            Two to eight spaces preceeding a tab stop are converted to a tab.
 -t tabsize    A single positive decimal integer indicates tab size otherwise
               the list is interpreted as locations for tab stops.
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.7  2006/04/11 16:16:35  rmansfield
	PR: 23548
	CI: kewarken

	Stage 1 of the warnings/errors cleanup.

	Revision 1.6  2005/06/03 01:38:03  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.5  2003/09/02 16:27:18  martin
	Update QSSL Copyright.
	
	Revision 1.4  1996/06/25 18:47:55  glen
	*** empty log message ***
	
	Revision 1.3  1992/10/27 19:56:44  eric
	added usage one-liner

 * Revision 1.2  1992/07/09  14:47:35  garry
 * *** empty log message ***
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *
	
---------------------------------------------------------------------*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

extern void	process( FILE *fp );
extern void	set_tabs( char *tlist );
int next_tstop( int pos );

#define LINE_MAX	2048

unsigned char	tabs[LINE_MAX];
int				tabcnt = 0;
char			line[LINE_MAX], *p;
int				tabsize = 8;
int				aflag;

#define TXT(s)		s
#define T_NO_OPEN	"UNEXPAND:  Unable to open '%s'\n"

int main(int argc, char **argv )
	{
	FILE	*fp;
	char	*p = "8";
	int		i, error = 0;

	while( ( i = getopt( argc, argv, "at:" ) ) != -1 ) {
		switch( i ) {
			case 'a':	aflag = 1;				break;
			case 't':	p = optarg;				break;
			default:	error++;				break;
			}
		}

	if ( error )
		exit( EXIT_FAILURE );

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
process( fp )
	FILE	*fp;
	{
	int		pos, spaces, tabx, nxt;

	while( fgets( line, LINE_MAX, fp ) != NULL ) {
		for( pos = 0, p = &line[0]; *p != NULL; ) {
			tabx = spaces = 0;
			for( ; *p == ' '; p++, spaces++ );
			for ( ;; ) {
				nxt = next_tstop( pos );
				if ( nxt > spaces  ||  spaces < 2 ) break;
				putchar( '\t' );
				spaces -= nxt;
				pos += nxt;
				tabx = 1;
				}
			while( spaces > 0 ) {
				putchar( ' ' );
				pos++;
				spaces--;
				}
			if ( !aflag ) {
				for( ; *p != NULL; p++ )
					putchar( *p );
				break;
				}
			if ( *p == '\t' )		pos += next_tstop( pos );
			else if ( *p == '\b' ) {if ( pos - 1 > 0 ) pos--;}
			else			  		pos++;
			putchar( *p++ );
			}
		}
	}

void
set_tabs( char *tlist )
	{
	char	*p, *pp;
	int		i;

	memset( tabs, 0, sizeof( tabs ) );
	tabcnt = 0;
	tabs[0] = 1;
	for( p = tlist - 1; p != NULL; p = strchr( p, ',' ) ) {
		if ( ( pp = strchr( ++p, ',' ) ) != NULL )
			*pp = '\0';
		tabs[(tabsize = atoi(p))] = 1;
		tabcnt++;
		if ( pp )
			*pp = ',';
		else
			break;
		}
	if ( tabcnt == 1 ) { /* Only one argument for tablist */
		for( i = 0; i < LINE_MAX; i += tabsize, tabcnt++ )
			tabs[i] = 1;
		}
	}

int next_tstop( int pos )
	{
	int		i;

	for( i = 0; !tabs[pos++]; i++ );
	return( i ? i : tabsize );
	}
