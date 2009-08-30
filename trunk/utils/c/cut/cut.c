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




#ifdef __USAGE		/* cut.c (phase 3) */
%C	- cut out selected fields of each line of a file (POSIX)

%C	-c list [file ...]
%C	-f list [-d delim] [-s] [file ...]
Options:
 -c      Cut out the <list> of fields based on character position.
 -f      Cut out the <list> of fields based on field position.
 -d      Specify the field delimiter character.
 -s      Suppress the output of lines which contain no field delimiter.
Where:
 list    Is a list of character or field positions separated by a <space>
         or a <comma>. A range of positions may be specified using the
         <hyphen>.
Examples:
 cut -c 20-29,40-49 myfile    Output character columns 20 to 29 and 40 to 49 
                              of the file 'myfile'
 cut -f 3,5,7-11 -d| myfile   Output fields numbered 3, 5, and 7 to 11 of the
                              file 'myfile'.  The fields in each input line
                              are separated by the '|' character.
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.14  2006/04/11 16:15:36  rmansfield
	PR: 23548
	CI: kewarken

	Stage 1 of the warnings/errors cleanup.

	Revision 1.13  2006/03/21 16:05:28  rmansfield
	
	PR: 23547
	CI: kewarken
	
	Revision 1.12  2005/06/03 01:37:43  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.11  2003/08/21 20:43:05  martin
	Add QSSL Copyright.
	
	Revision 1.10  1999/11/02 19:56:34  dagibbs
	Fix PR 1731.  Standard fgets programming error, forgot to make sure there
	was a newline before deleting the last character on the line.
	
	Revision 1.9  1997/02/10 20:46:53  eric
	fopen had old mode chars from qnx2 days which are too funky for
	fopen under nto and ignored under qnx4; removed.
	
	Revision 1.8  1996/11/12 18:54:56  eric
	fixed bad fopen() parameters

	Revision 1.7  1996/07/17 19:17:22  glen
	fixed error handling
	bumped field limit

	Revision 1.6  1996/01/28 20:43:30  dtdodge
	*** empty log message ***

	Revision 1.5  1992/08/05 15:22:57  eric
	added usage one-liner

 * Revision 1.4  1991/12/11  21:38:17  ajedgar
 * Fixed a bug with field specs
 *
 * Revision 1.3  1991/12/09  17:09:50  ajedgar
 * Updated usage and remade
 *
 * Revision 1.2  1991/09/03  17:52:33  ajedgar
 * New usage, tested.
 *
 * Revision 1.1  1991/09/03  16:25:13  ajedgar
 * Initial revision
 *
	
---------------------------------------------------------------------*/


/*
-----------------------------------------------------------------------
POSIX UTILITY: CUT

1003.2 -- Draft 9 (Shell and Utilities working group)

cut - Cut out selected fields of each line of a file
-----------------------------------------------------------------------
*/

#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <libgen.h>

/*
 *  TXT(s) provides support for language independence, once we have it...
 */

#define TXT(s)			  s

#define T_ERR_FILE			"%s: error %d %s file '%s'\n"
#define T_OPEN				"opening"
#define T_READ				"reading"
#define T_WRITE				"writing"
#define T_STDOUT			"stdout"
#define T_ERR_FOPT			"%s: the -d and -s options may only be used with -f\n"
#define T_ERR_FLIST			"%s: invalid field list specification\n"

#define E_INVALID_USAGE		-1
#define E_ERR				-1
#define E_OK				0

#define C_RANGE			'-'
#define C_FIELD			','
#define C_SPACE			' '
#define C_NEWLINE		'\n'
#define C_TAB			'\t'

#define STATE_LSPACE	0		/*	Leading space */
#define STATE_TSPACE	1		/*	Trailing space */
#define STATE_TNUM	 	2		/*	Trailing number (MUST be there) */
#define STATE_SNUM	 	3		/*	Starting number (range) */
#define STATE_ENUM	 	4		/*	Ending number (range) */

#ifndef TRUE
#	define TRUE		(1==1)
#	define FALSE	(1==0)
#endif

#define BUFSIZE			LINE_MAX

char *Me;
char *Delim = NULL;
char *FieldList = NULL;
char Field = 0;
char Suppress = 0;

char Ibuf[BUFSIZE];
char Obuf[BUFSIZE];

/*
 *	Function declarations
 */
char *next_range();
int check_flist (char *p);
int extract_fields (char *op, char *ip, char delim);
int extract_chars (char *op, char *ip);
int cut (FILE *fp, char *fname, char delim);
int strccnt (char *str, int c);


/*
 *	cut - Cut out selected fields of each line of a file
 *
 *	Exit status:
 *	   0  - all files processed successfully
 *	  -n  - number of files that encountered errors * -1
 */
int main (int argc, char *argv[])
{
	FILE *ifp;
	int nfiles;
	int opt, error = 0;
	char *curfile, delim = C_TAB;
	

	/*	Parse command line arguments */
	error = 0;
	Me = basename( argv[0] );

	while ((opt = getopt(argc,argv,"c:f:d:s")) != -1)
	{
		switch(opt)
		{
			/*	Fields are character positions */
			case 'c':
				FieldList = optarg;
				Field = 0;
				break;

			/*	Fields are delimited strings */
			case 'f':
				FieldList = optarg;
				Field = 1;
				break;

			/*	Delimiter */
			case 'd':
				Delim = optarg;
				delim = *Delim;
				break;

			/*	Suppress output of lines with no delimited fields */
			case 's':
				Suppress = 1;
				break;

			/*	User wants a usage message or bad option */
			case '?':
				error++;
				break;
		}
	}

	if (optind < 2)
		++error;

	/*	Check for mutually exclusive options -c and -f */
	if (!Field && (Delim || Suppress)) {
		fprintf(stderr, TXT(T_ERR_FOPT), Me);		
		++error;
	}

	/*	Check field list */
	if (!FieldList || check_flist(FieldList)) {
		fprintf(stderr, TXT(T_ERR_FLIST), Me);		
		++error;
	}

	/*	Check for correct usage */
	if (error)
		exit( EXIT_FAILURE );

	/*	Process each file */
	error = 0;
	for (nfiles=0 ; (curfile=argv[optind]); ++optind, ++nfiles)	{
		/*	If the string "-" is given then use stdin otherwise open the file */
		if (strcmp(curfile, "-")) {
			/*	Open the file */
			if (!(ifp = fopen(curfile, "r"))) {
				/*	If there's an error, display it... */
				fprintf(stderr, TXT(T_ERR_FILE), Me, errno, TXT(T_OPEN), curfile);
				/*	and continue with rest of files */
				error -= 1;
				continue;
			}
		}
		else {
			curfile = "stdin";
			ifp = stdin;
		}
		error += cut(ifp, curfile, delim);

		/*	Close the file */
		if (ifp != stdin)
			fclose(ifp);
	}	
	if (! nfiles)
		error += cut(stdin, "stdin", delim);

	exit(error);
}


/*
 *	Extract fields from a file
 */
int cut (FILE *fp, char *fname, char delim)
{
	register int d;
	int last_c;

	while (fgets(Ibuf, BUFSIZE-1, fp)) {
		/*	Get rid of the newline, if it exists */
		last_c = strlen(Ibuf) -1;
		if (Ibuf[last_c] == '\n')
			Ibuf[last_c] = '\0';
		d = 1;
		if (Field)
			d = extract_fields(Obuf, Ibuf, delim);
		else
			extract_chars(Obuf, Ibuf);
		errno = 0;
		if (! (Suppress && ! d)) {
			fputs(Obuf, stdout);
			putc(C_NEWLINE, stdout);
		}
		if (ferror(stdout)) {
			fprintf(stderr, TXT(T_ERR_FILE), Me, errno, TXT(T_WRITE), TXT(T_STDOUT));
			return(-1);
		}
	}
	if (ferror(fp)) {
		fprintf(stderr, TXT(T_ERR_FILE), Me, errno, TXT(T_READ), fname);
		return(-1);
	}
	fflush(stdout);

	return(0);
}


/*
 *	Extract fields
 *
 *	Return 0 if there were no delimiters otherwise 1
 */
int extract_fields (char *op, char *ip, char delim)
{
	char *p;
	int a, b, c, i, max_pos;
	char *field[512];
	
	/*	Max position does not include the <newline> */
	max_pos = strccnt(ip, delim);

	/*	If there are no delimiters then pass the string thru untouched */
	if (max_pos < 1) {
		strcpy(op, ip);
		return(0);
	}

	/*	Find the beginning of each field */
	field[0] = ip;
	for (i=1; (field[i] = strchr(field[i-1], delim)); ++i) 
		field[i]++;
	field[i] = &ip[strlen(ip)];

	for (*op = NULL, p=FieldList; (p=next_range(p, &a, &b)); ) {
		--a;
		--b;
		if (a > max_pos && b > max_pos)
			continue;
		if (a > max_pos)
			a = max_pos;
		if (b > max_pos)
			b = max_pos;
		c = abs(a-b)+1;
		if (a > b) {
			for(i=a; i>=b; --i) {
				for(ip=field[i]; *ip && *ip != delim; )
					*op++ = *ip++;
				*op++ = delim;
			}
		}
		else {
			for(i=a; i<=b; ++i) {
				for(ip=field[i]; *ip && *ip != delim; )
					*op++ = *ip++;
				*op++ = delim;
			}
		}
	}
	*--op = NULL;

	return(1);
}

/*
 *	Extract characters
 */
int extract_chars (char *op, char *ip)
{
	char *p, *ibuf = ip;
	int a, b, c, i, max_pos;

	max_pos = strlen(ip)-1;
	for (p=FieldList; (p=next_range(p, &a, &b)); ) {
		--a;
		--b;
		if (a > max_pos && b > max_pos)
			continue;
		if (a > max_pos)
			a = max_pos;
		if (b > max_pos)
			b = max_pos;
		c = abs(a-b)+1;
		if (a > b) {
			for(ip=&ibuf[a], i=0; i<c; ++i)
				*op++ = *ip--;
		}
		else {
			for(ip=&ibuf[a], i=0; i<c; ++i)
				*op++ = *ip++;
		}
	}
	*op++ = NULL;

	return(0);
}

/*
 *	Check the field list 'p' for validity
 *
 *	Valid ranges are:
 *		4			- the 4th field
 *		3-7			- from the 3rd to the 7th field
 *		-10			- equivalent to 1-10
 *		30-			- equivalent to 30-<last_field>
 *
 *	Valid field lists are ranges separated by commas or spaces:
 *		1,2,3
 *		1-5,12-17
 *		-5, 12-17 , 21-
 */
int check_flist (char *p)
{
	register int state = STATE_LSPACE;

	for ( ; *p ; ++p)  {
		/*	Leading space */
		if (state == STATE_LSPACE) {
			if (isdigit(*p)) {
				/*	Field 0 is not allowed... */
				if (! atoi(p))
					return(E_ERR);
				state = STATE_SNUM;
				continue;
			}
			if (*p == C_RANGE) {
				state = STATE_TNUM;
				continue;
			}
			if (*p == C_SPACE) {
				continue;
			}
			return(E_ERR);
		}
		/*	Trailing space */
		if (state == STATE_TSPACE) {
			if (*p == C_FIELD) {
				state = STATE_LSPACE;
				continue;
			}
			if (*p == C_SPACE) {
				continue;
			}
			return(E_ERR);
		}
		/*
		 *	Trailing number - if there was a leading range delimiter there
		 *	MUST be a following number
		 */
		if (state == STATE_TNUM) {
			if (isdigit(*p)) {
				state = STATE_ENUM;
				continue;
			}
			return(E_ERR);
		}
		/*	Starting number (of a range) */
		if (state == STATE_SNUM) {
			if (isdigit(*p)) {
				continue;
			}
			if (*p == C_RANGE) {
				state = STATE_ENUM;
				continue;
			}
			if (*p == C_FIELD) {
				state = STATE_LSPACE;
				continue;
			}
			return(E_ERR);
		}
		/*	Ending number (of a range) */
		if (state == STATE_ENUM) {
			if (isdigit(*p)) {
				/*	Field 0 is not allowed... */
				if (*(p-1) == C_RANGE && (! atoi(p)))
					return(E_ERR);
				continue;
			}
			if (*p == C_FIELD) {
				state = STATE_LSPACE;
				continue;
			}
			if (*p == C_SPACE) {
				state = STATE_TSPACE;
				continue;
			}
			return(E_ERR);
		}
	}

	/*	These are invalid states to be in at this point... */
	if (state == STATE_TNUM || state == STATE_LSPACE)
		return(E_ERR);

	return(E_OK);
}


/*
 *	Get the next range from the field list 'p'.  Put the starting field position
 *	in 'sf' and the ending field in 'ef'.
 */
char *next_range(p, sf, ef)
register char *p;
int *sf, *ef;
{
	if (! *p)
		return(NULL);

	/*	Skip leading white space */
	for ( ; *p == C_SPACE; ++p)
		;
	if (*p == C_RANGE)
		*sf = *ef = 1;
	else
		*sf = *ef = atoi(p);

	/*	Is it a range ? */
	for ( ; *p && *p != C_FIELD; ++p)
		if (*p == C_RANGE) {
			if (*(p+1) == 0)
				*ef = BUFSIZE-1;
			else
				*ef = atoi(p+1);
		}

	if (*p)
		++p;
	return(p);
}

/*
 *	Count the number of occurences of 'c' in 'str'
 */
int strccnt (char *str, int c)
{
	int i;

	for (i=0; *str; ++str)
		if (*str == (char)c)
			++i;

	return(i);
}
