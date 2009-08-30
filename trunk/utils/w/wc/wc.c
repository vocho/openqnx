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
%C - word, line, and byte count (POSIX)

%C	[-lwcht] [file ...]
Options:
 -l       Line count
 -w       Word count
 -c       Byte count
 -m       Character count
Where:
  file    (optional) one or more files to be processed. When no
          file is specified, stdin is used.
Note:
  The order of the options 'l', 'w', 'm' and 'c', if specified, orders
  the output.  If not specified, the default ordering is 'lwc'.

Neutrino-specific Options:
 -h       Display a header
 -t       Do not display totals

Note: we do not support these environment variables:
LANG, LC_ALL, LC_CTYPE, LC_MESSAGES.

#endif

/*---------------------------------------------------------------------

	$Header$

	$Log$
	Revision 1.12  2005/06/03 01:38:04  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.11  2003/09/24 00:19:59  jgarvey
	Make "wc" utility large-file-aware.  Change counts from int/long to
	off_t (so will resize according to _FILE_OFFSET_BITS compilation) and
	also allow for this in output formatting (printf).
	
	Revision 1.10  2003/09/02 18:07:11  martin
	Update QSSL Copyright.
	
	Revision 1.9  1999/07/29 18:27:36  spuri
	- I added support for the '-m' option
	
	I made the following changes even though they are not bugs:
	- added return values to all the functions and moved then around to get
	  rid of warnings about missing declarations and unspecified return types
	- I fixed the usage to indicate which command line options are Neutrino
	  specific and that we don't support any environment vars. I also added
	  a note on the '-m' option.
	
	Revision 1.8  1998/09/15 17:52:45  steve
	*** empty log message ***
	
 * Revision 1.7  1995/08/10  21:30:54  waflowers
 * Fix to ignore file size for a FIFO (or pipe).
 * You have to really read the bytes and count them.
 *
 * Revision 1.6  1993/04/07  15:08:06  steve
 * Removed the <fstdio.h> defines.  Watcom does these better now.
 *
 * Revision 1.5  1992/10/27  20:18:02  eric
 * added usage one-liner
 *
 * Revision 1.4  1991/10/23  19:50:05  eric
 * now uses Steve's fstio.h stuff.
 *
 * Revision 1.3  1991/09/12  19:26:14  brianc
 * Optimized to simply stat() file if only character count is requested
 *
 * Revision 1.2  1991/09/07  00:32:28  brianc
 * Right align counts in fixed-size fields (always)
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *

	$Author: coreos $
	
---------------------------------------------------------------------*/
/*********************************************************************/
/*                                                                   */
/* The WC utility reads one or more input text files, and, by default*/
/* writes the number of lines, words, and bytes contained in each    */
/* input file to the standard output.                                */
/*                                                                   */
/* The utility also writes a total count for all named files, if     */
/* more than one input file is specified.  If no file is specified   */
/* standard input is used and no filename is printed out.            */
/*                                                                   */
/* The WC utility considers a word to be a maximal string of         */
/* characters delimited by white space.                              */
/*                                                                   */
/* USAGE : wc [-clw] [file...]                                       */
/*                                                                   */
/* OPTIONS: The order the opetions are specified determines the order*/
/*          that the number of lines, words, and/or characters are   */
/*          written.                                                 */
/*                                                                   */
/*    -c    causes wc to write to the standard output the number of  */
/*          bytes in each input file.  A byte is considered to be    */
/*          character.                                               */
/*    -m    causes wc to write to the standard output the number of  */
/*          characters in each input file.                           */
/*    -l    causes wc to write to the standard output the number of  */
/*          lines in each input file.                                */
/*    -w    causes wc to write to the standard output the number of  */
/*          words in each input file.                                */
/*                                                                   */
/*********************************************************************/
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>


#include "util/stdutil.h"

/* Global Declarations */

extern int	 getopt();
extern int	 optind;
extern int	 opterr;
extern char	*optarg;

#define NUM_FIELDS 4	// max fields we can output

int		 hdr=0,	 totals = 1;
off_t	 nc, nw, nl;
char 	 num_args[NUM_FIELDS];

/* This function prints to standard out, the number of words, lines and  */
/* bytes in a given file.  If no file is specified, no file is printed.  */
/* They are printed out in the order the options were entered.  If no    */
/* options were specified, the default order is written out.             */

void
print_out ( num_chars, num_words, num_lines, name)
off_t num_chars, num_words, num_lines;
char *name;
{	int i;
#if _FILE_OFFSET_BITS - 0 == 64
	char *fmt = "%9lld";
#else
	char *fmt = "%9d";
#endif

	for (i=0; i < NUM_FIELDS && num_args[i]; i++)
	{	switch(num_args[i])
		{
			case 'c':	printf(fmt, num_chars * sizeof (char)); break;
			case 'm':	printf(fmt, num_chars); break;
			case 'l':	printf(fmt, num_lines); break;
			case 'w':	printf(fmt, num_words); break;
		}
	}
	if ( name )            /* if filename was specified, stdin is not printed */
		printf(" %s", name);
	putchar( '\n' );
}

/* This function counts the number of lines, chars, and words in a given */
/* filename.  A word is considered to be a maximal string of characters  */
/* delimited by white space.                                             */

void
count(fp, name)
FILE *fp;
char *name; 
{
	register int c;
	int inword = 0;
	struct stat sb;
	
	nc = nl = nw = 0;
	if (((num_args[0] == 'c') || (num_args[0] == 'm')) && num_args[1] == 0 && 
		fstat(fileno(fp), &sb) == 0 && !S_ISFIFO(sb.st_mode))
		nc = sb.st_size;
	else
	while (( c = getc(fp)) != EOF )
	{	++nc;
		if ( isspace( c ))     /* checks for space, tab, and newline as */
			inword = 0;        /* word separators */
		else 	
			if ( inword == 0)
			{	inword = 1;
				++nw;
			}
		if ( c == '\n' )       /* if the file does not end with a newline */
			++nl;              /* it is not counted */
	}
	print_out( nc, nw, nl, name );
}

/* This function prints to standard out, the headers                     */
void
print_hdr()
{	int i;

	for (i=0; i < NUM_FIELDS && num_args[i]; i++)
	{	switch(num_args[i])
		{	
			case 'c':	printf("%9s", "bytes");		break;
			case 'm':	printf("%9s", "chars");		break;
			case 'l':	printf("%9s", "lines");		break;
			case 'w':	printf("%9s", "words");		break;
		}
	}
	printf (" file\n");
}

int
main(argc,argv)
int argc;
char *argv[];
{
	register char *filename;
	FILE *fp, *fopen();	
	int		 i,
			 filecount = 0,
			 status = 0,
			 error = 0,
			 index = 0;
	off_t	 total_lines = 0,
			 total_words = 0,
			 total_chars = 0;

	while(( i= getopt( argc, argv, "cmlw**ht")) != -1)
	{	switch (i) 	
		{
			case 'c': 
				if (index < NUM_FIELDS)
					num_args[index++] = 'c';  /* specifies which order */
				break;                    /* the options were entered */
			case 'm': 
				if (index < NUM_FIELDS)
					num_args[index++] = 'm';  /* specifies which order */
				break;                    /* the options were entered */
			case 'l': 
				if (index < NUM_FIELDS)
					num_args[index++] = 'l';
				break;
			case 'w': 
				if (index < NUM_FIELDS)
					num_args[index++] = 'w';
				break;
			case 'h': ++hdr;
					  break;
			case 't': --totals;
					  break;
			case '?': error++;
					  break;
		}
	}

	if( error ) exit(1);

	if (!index)                /* if no options were specified */
	{	num_args[0] = 'l';     /* fill with default order */
		num_args[1] = 'w';
		num_args[2] = 'c';
		num_args[3] = 0;
	}

	if( hdr ) print_hdr();
	if (optind >= argc)     
		count( stdin, 0 );     /* no filename specified, use stdin */
	else
	{
		for( ; optind < argc; optind++ )    /* count for each filename */
		{	filename = argv[optind];
			if (( fp = fopen( filename, "r")) !=0) 
			{ 	count( fp, filename );
				fclose(fp);
				++filecount;
				total_chars += nc;
				total_lines += nl;
				total_words += nw;
			}
			else
			{   fprintf(stderr, "cannot open file '%s'\n", filename);
				status = 1; 
			}
		}
	}
	if ((filecount > 1) && (totals > 0))
		print_out (total_chars, total_words, total_lines, "total");
	exit(status);
}
