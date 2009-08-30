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





#ifdef __USAGE
%C - find printable strings in files (POSIX)

%C	[-a] [-f] [-n len] [-[Ot] format] file...
Options:
 -a            Scan file in entirety.
 -f            Print strings >= min length, regardless of null, newline
               delimiter
 -n len        Minimum string length.
 -O {o|x|d}    Format of address (octal, hexadecimal, decimal).
 -t {o|x|d}    Format of address (octal, hexadecimal, decimal).
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.8  2006/04/11 16:16:28  rmansfield
	PR: 23548
	CI: kewarken

	Stage 1 of the warnings/errors cleanup.

	Revision 1.7  2005/06/03 01:38:01  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.6  2003/08/29 21:04:17  martin
	Add QSSL Copyright.
	
	Revision 1.5  1993/08/04 20:37:13  steve
	Added a (non-standard) option -f to print strings which are NOT terminated
	by null or newline.
	
 * Revision 1.4  1993/07/14  20:46:02  root
 * *** empty log message ***
 *
 * Revision 1.3  1993/04/27  20:44:02  steve
 * There were a multitude of little problems.   It didn't get all the strings
 * from a file.  The offsets it printed were meaningless (well, not meaningless
 * they actually were the number of printable characters encountered thus far
 * in the file, rather than the offset of the printed string).  It allways
 * insisted on printing the offset (only with -t|-O).
 * It seems fixed now, although the "diff" list is pretty high here....
 *
 * Revision 1.2  1992/10/27  19:01:27  eric
 * added usage one-liner
 *
 * Revision 1.1  1991/08/27  21:39:02  brianc
 * Initial revision
 *
	
	Revision 1.1 Wed Mar 14 13:29:22 1990 glen
	 *** QRCS - Initial Revision ***
	
---------------------------------------------------------------------*/

#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

#ifdef __GNUC__
static char __attribute__ ((unused)) rcsid[] = "$Id: strings.c 153052 2008-08-13 01:17:50Z coreos $";
#else 
static char rcsid[] = "$Id: strings.c 153052 2008-08-13 01:17:50Z coreos $";
#endif 

#define	STRSIZE		10240
#define TXT(s)				s
#define T_BAD_FORMAT		"strings:  Invalid format option\n"
#define T_NO_MEMORY			"strings:  Out of memory\n"
#define T_UNABLE_TO_OPEN	"strings:  Unable to open '%s'\n"

int strings(FILE *inf, FILE *outf, int minlength);

int	entire = 1;
char	*format = 0;
int	nonewline = 0;

void putline(FILE *f, long offs, char *string)
{
	if (format) {
		fprintf(f,format,offs,string);
	} else {
		fputs(string,f); fputc('\n',f);
	}
}



int main(int argc, char *argv[])
{
	int	errors = 0;
	int	i;
	int	minlength = 4;
	while ((i=getopt(argc,argv,"afn:O:t:")) != -1) {
		switch (i) {
		case 'a': entire = 1; break;
		case 'f': nonewline = 1; break;
		case 't':
		case 'O':
			switch( *optarg ) {
			case 'o': format="%lo %s\n"; break;
			case 'x': format="%lx %s\n"; break;
			case 'd': format="%ld %s\n"; break;
			default:
				fprintf( stderr, TXT( T_BAD_FORMAT ) );
				errors++;
			}
			break;
		case 'n':
			minlength = strtol(optarg,0,0);
			if (minlength >= STRSIZE) {
				minlength = STRSIZE;
				fprintf(stderr,"%s: minlength truncated to %d\n",
					argv[0], minlength);
			}
			break;
		default:
			errors++;
			break;
		}
	}
	if (errors) return EXIT_FAILURE;


	if (optind == argc) {
		strings(stdin,stdout, minlength);
	} else while (optind < argc) {
		if (strcmp(argv[optind],"-") == 0) {
			strings(stdin, stdout, minlength);
		} else {
			FILE *fp;
			if ((fp=fopen(argv[optind],"r")) == 0) {
				fprintf(stderr,"%s: cannot open '%s':%s\n",
					argv[0], argv[optind],
					strerror(errno));
				return EXIT_FAILURE;
			}
			strings(fp, stdout, minlength);
			fclose(fp);
		}
		optind++;
	}
	return EXIT_SUCCESS;
}

int strings(FILE *inf, FILE *outf, int minlength)
{
	int	count = 0;
	long	cpos = 0;
	int	c;
	char	*buf;
	if ((buf=malloc(STRSIZE)) == 0) {
		fprintf(stderr, TXT(T_NO_MEMORY));
		exit(EXIT_FAILURE);
	}
	while ((c=getc(inf)) != EOF) {
		if (c == '\0' || c == '\n') {
			if (count >= minlength) {
				buf[count] = '\0';
				putline(outf, cpos-count, buf);
			}
			count = 0;
		} else if (isprint(c) || c == ' ' || c == '\t') {
			if (count == STRSIZE-1) {
				buf[count] = '\0';
				putline(outf, cpos-count, buf);
				count = 0;
			} else {
				buf[count++] = c;
			}
		} else {
			if (nonewline && count >= minlength) {
				buf[count] = '\0';
				putline(outf, cpos-count, buf);
			}
			count = 0;
		}
		cpos++;
	}
	if (count >= minlength) {
		buf[count] = 0;
		putline(outf, cpos-count, buf);
	}
	free(buf);
	return 0;
}


#if 0
	for( ; optind < argc; optind++ ) {
		FILE *fp;

		if ( nofile )
			fd = STDIN_FILENO;
		else if ( ( fd = open( argv[optind], O_RDONLY ) ) == -1 ) {
			fprintf( stderr, TXT( T_UNABLE_TO_OPEN ), argv[optind] );
			continue;
			}

		for ( pos = 0, i = 1; i > 0; i = read( fd, inbuf, BUFSIZE ) ) {
			for ( c = inbuf, bptr = 0; --i > 0 ; ) {
				while ( isprint( *c ) ) {
					buf[bptr++] = *c++;
					pos++;
					if ( !--i ) break;
					}
				if ( bptr >= minlength ) {
					buf[bptr] = '\0';	/* Null at end of string */
					printf( format, pos, buf );
					}
				if ( !bptr )	c++;
				bptr = 0;
				}
			}
		fflush( stdout );
		close( fd );
		}
	exit( EXIT_SUCCESS );
	}
#endif
