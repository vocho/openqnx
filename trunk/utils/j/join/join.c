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





#ifdef __USAGE		/* join.c */
%C - relational database operator (POSIX)

%C	[options]... file1 file2
Options:
 -1  field    Join on field number <field> from file 1.
 -2  field    Join on field number <field> from file 2.
 -a  fnum     In addition to the default output, produce a line for
              every unpairable line in the specified file number <fnum> (may
              be 1 or 2). If -a 1 and -a 2 are specified, all unpairable
              lines will be output.
 -e  string   Replace empty output fields with <string>.
 -j  field    Equivalent to '-1 <field> -2 <field>'
 -j1 field    Equivalent to '-1 <field>'
 -j2 field    Equivalent to '-2 <field>'
 -o  list     Specify the ouput field list.  Each output line consists of the
              fields specified in <list>.  Each element of <list> has the
              form <file>.<field> where <file> is a file number (either 1 or
              2) and <field> is a field number.  Elements of list are
              separated with a ','or ' ' character. 
              For example: -o 1.2,2.2,2.3 or -o 1.2 2.2 2.3
              The "join" field is not printed unless specifically listed.
 -t  char     Use character <char> as the field separator.  Every occurence
              of <char> in a line is significant.  The character <char> is
              used as the field separator for both input and output.  By
              default the input separator is white space (spaces and tabs)
              and the output separator is a single space.
 -v  fnum     Instead of the default output, produce a line only for every
              unpairable line in the specified file number (may be 1 or 2).
              If bother -v 1 and -v 2 are specified, all unpairable lines
              will be output.
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.19  2006/04/11 16:15:53  rmansfield
	PR: 23548
	CI: kewarken

	Stage 1 of the warnings/errors cleanup.

	Revision 1.18  2005/06/03 01:37:48  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.17  2003/08/25 21:22:21  martin
	Add QSSL Copyright.
	
	Revision 1.16  1998/09/19 14:52:03  bstecher
	made compile cleanly
	
	Revision 1.15  1998/09/19 14:44:38  eric
	*** empty log message ***

	Revision 1.14  1997/07/31 17:49:55  eric
	fixed SIGSEGV problem when the user-specified output field
	includes a field that does not exist in one of the input
	records from the data being processed

	Revision 1.13  1997/03/10 21:21:36  eric
	removed nto-specific ifdef's; added tmpfile to nto ansi lib

	Revision 1.12  1996/11/12 18:56:56  eric
	fixed nto tmpnam/tempname problem

	Revision 1.11  1996/06/11 14:38:47  eric
	fixed problem with SIGSEGV when files are omitted from cmd line

 * Revision 1.10  1996/06/11  14:31:16  glen
 * blat
 *
 * Revision 1.9  1996/04/15  18:56:56  eric
 * updated usage
 *
 * Revision 1.8  1996/04/15  18:52:55  eric
 * revised usage message
 *
 * Revision 1.7  1996/04/15  18:44:40  eric
 * added -v option, fixed SIGSEGV problem in its 32-bit incarnation.
 *
 * Revision 1.6  1996/01/28  20:44:04  dtdodge
 * *** empty log message ***
 *
	Revision 1.5  1992/10/27 15:47:33  eric
	added usage one-liner

 * Revision 1.4  1992/07/09  09:48:45  garry
 * Editted usage
 *
 * Revision 1.3  1991/12/06  15:53:31  ajedgar
 * Bug fix and removed some debugging code.
 *
 * Revision 1.2  1991/12/04  20:13:03  ajedgar
 * Modified usage message...
 *
	
---------------------------------------------------------------------*/


/*
-----------------------------------------------------------------------
POSIX UTILITY: JOIN

1003.2 -- Draft 11 (Shell and Utilities working group)

join - perform a relational database join operation
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
#include <sys/types.h>
#include <lib/compat.h>

//#define NDEBUG

#ifdef NDEBUG
#	define DBG(x)	/* */
#else
#	define DBG(x)   x
#endif

/*
 *  TXT(s) provides support for language independence, once we have it...
 */

#define TXT(s)			  s

#define T_ERR_FILE			"%s: error %d %s file '%s'\n"

#define T_OPEN				"opening"
#define T_READ				"reading"
#define T_WRITE				"writing"
#define T_TRUNCATE          "truncating"
#define T_STDOUT			"stdout"
#define T_TMP_FILE          "temp file"
#define T_ERR_ALLOC			"%s: unable to allocate memory for internal buffers\n"
#define T_ERR_JOPT			"%s: Invalid join field specification\n"
#define T_ERR_LIST			"%s: Invalid field list specification\n"
#define T_ERR_OUTPUTLIST		"%s: Output field list too long\n"

#ifndef TRUE
#	define TRUE		(1==1)
#	define FALSE	(1==0)
#endif

#define BUFSIZE			LINE_MAX

char *Me;
char *DelimPtr = NULL;
char Delim = 0;

/*	POSIX says this should be a TAB, but the SunOS uses a SPACE */
char OutDelim = ' ';
char Suppress = 0;

char *EmptyField = "";
int  AlwaysOutput = 0;
int  Join1 = 1;
int  Join2 = 1;

int dashv = 0;		/* flag for -v mode; disables default output */

char *Buf1, *Buf2;
char *SaveBuf2, *OBuf;
char *CacheBuf;
char *FieldList;


/*
 *	Function declarations
 */
char *getfield (char *buf, int field, int *len);
FILE *open_file (char *);
int output (char *buf1, char *buf2);
int compare (char *buf1, int field1, char *buf2, int field2);
int putfield (char *obuf, char *buf, int field);
int putfield2 (char *obuf, char *buf, int field, int *valid);
void bad_field_list ();


/*
 *	Join - perform a relational database join of two files
 *         (the files are assumed to be collated ASCII text)
 *
 *	Exit status:
 *	   0  - join was successful
 *	  -1  - an error occurred
 */
int main (int argc, char *argv[])
{
	register FILE *fp1, *fp2, *fptmp, *fpinput;
	register int i;
	int opt, t;
	int match, last_match;
	char *fname1, *fname2, *p;
	char read1, read2;
	char match1, match2;
	char saved_position = 0, first = 1;

	/*	Parse command line arguments */
	Me = basename( argv[0] );

	if ((Buf1 = calloc(6, BUFSIZE)) == NULL) {
		fprintf(stderr, TXT(T_ERR_ALLOC), Me);
		exit(1);
	}
	Buf2 = Buf1 + BUFSIZE;
	SaveBuf2 = Buf2 + BUFSIZE;
	OBuf = SaveBuf2 + BUFSIZE;
	CacheBuf = OBuf + BUFSIZE;
	FieldList = CacheBuf + BUFSIZE;

	*Buf1 = *Buf2 = '\0';
	*FieldList = '\0';

	while ((opt = getopt(argc,argv,"v:a:e:j:o:t:1:2:")) != -1)
	{
		switch(opt)
		{
			/*	Output lines which don't match? */
			case 'v':
				dashv++;
				/* fall thru to -a; -v is -a without the default output */
			case 'a':
				t = atoi(optarg);
				if (t<1 || t>2)
					t=3;
				AlwaysOutput |= t;
				break;

			/*	Empty field output */
			case 'e':
				EmptyField = optarg;
				break;

			/*	Join field */
			case '1':
				Join1 = atoi(optarg);
				break;

			case '2':
				Join2 = atoi(optarg);
				break;

			/*
			 *	Support old style "join field" spec
			 */
			case 'j':
				while (*(argv[--optind]+1) != 'j')
					;
				t = *(argv[optind]+2) - '0';
				if ( *(p=argv[++optind]) < '1' || *p > '9' ) {
					fprintf(stderr, T_ERR_JOPT, Me);
					exit(1);
				}
				i = atoi(argv[optind++]);
				if (t<1 || t>2)
					Join1 = Join2 = i;	/*	File was not specified */
				else
					if (t == 1)
						Join1 = i;
					else
						Join2 = i;
				break;

			/*	Output field specification */
			case 'o':
				if (strlen(optarg) >= BUFSIZE) 	 {
					fprintf(stderr, T_ERR_OUTPUTLIST, optarg);
					exit(EXIT_FAILURE);
				}
				strncpy(FieldList, optarg, BUFSIZE - 1);
				p = FieldList;
				/*
				 * Support POSIX ',' separator for -o fields
				 * by converting them to the old ' ' separator.
				 */
				for (;;) {
					if ((p = strchr(p, ',')) == NULL)
						break;
					*p++ = ' ';
				}
				/*
				 *	Support old style output field "arguments"
				 */
				for ( ; argv[optind] && (*(p=argv[optind]) == '1' || *p == '2' && *(p+1) == '.') && (strlen(FieldList) + strlen(p) + 1 < BUFSIZE); ++optind) {
					strcat(FieldList, " ");
					strcat(FieldList, p);
				}
				break;

			/*	Delimiter */
			case 't':
				DelimPtr = optarg;
				Delim = *optarg;
				break;

			/*	Suppress output of lines with no delimited fields */
			case 's':
				Suppress = 1;
				break;

		}
	}

	if ((optind+2)>argc) {
		fprintf(stderr,"join: error - two filenames must be supplied on the command line\n");
		exit(1);
	}

	/*	Open the files */
	fp1 = open_file(fname1 = argv[optind++]);
	fp2 = fpinput = open_file(fname2 = argv[optind++]);
	if (! fp1 || ! fp2)
		exit(1);

	/*	Open a temporary file */
	if ((fptmp = tmpfile()) == NULL) {
		/*	If there's an error, display it... */
		fprintf(stderr, TXT(T_ERR_FILE), Me, errno, TXT(T_OPEN), T_TMP_FILE);
		exit(1);
	}

	read1 = read2 = TRUE;
	match1 = match2 = FALSE;
	match = 0;

	while(1) {
		i = 0;
    	last_match = match;

		/*	Read the first file */
		if (read1 && fp1 != NULL) {
			DBG(fprintf(stderr, "read: file 1\n"));
			match1 = FALSE;
			read1 = FALSE;
			*Buf1 = '\0';
			if (fgets(Buf1, BUFSIZE-1, fp1) == NULL) {
				int saved_errno=errno;

				if (! feof(fp1)) {
					fprintf(stderr, TXT(T_ERR_FILE), Me, saved_errno, TXT(T_READ), fname1);
					DBG(fprintf(stderr, "read: error %d on file 1\n", saved_errno));
				}

				if (fp1 != stdin)
					fclose(fp1);
				fp1 = NULL;
			}
			else {
				/*
				*  If lines containing identical fields have been cached and
				*  the next line from file 1 matches the previous line from
				*  file 2 then start reading from the cache, if the match fails
				*  disable caching and restore the input pointer for file 2.
				*/
				DBG(fprintf(stderr, "compare: file 1 to previous line from file 2\n"));
				if (!first && compare(Buf1, Join1, SaveBuf2, Join2) == 0) {
					if (saved_position == 1 && fp2 != fptmp) {
						DBG(fprintf(stderr, "cache: switching to cache\n"));
						rewind(fptmp);
						fp2 = fptmp;
						strcpy(CacheBuf, Buf2);
						read2 = TRUE;
						continue;
					}
					else {
						output(Buf1, SaveBuf2);
						read1 = TRUE;
						continue;
					}
				}
				else {
					saved_position = 0;
					fp2 = fpinput;
				}
			}
		}

		/*	Read the second file */
		if (read2 && fp2 != NULL) {
			DBG(fprintf(stderr, "read: file 2\n"));
			strcpy(SaveBuf2, Buf2);
			*Buf2 = '\0';
			match2 = FALSE;
			if (fgets(Buf2, BUFSIZE-1, fp2) == NULL) {
				int saved_errno=errno;
				if (fp2 == fptmp) {
					if (! feof(fp2)) {
						fprintf(stderr, TXT(T_ERR_FILE), Me, saved_errno, TXT(T_READ), T_TMP_FILE);
						exit(1);
					}
					DBG(fprintf(stderr, "cache: switching back to file input\n"));
					fp2 = fpinput;
					strcpy(Buf2, CacheBuf);
				}
				else {
					if (! feof(fp2)) {
						fprintf(stderr, TXT(T_ERR_FILE), Me, saved_errno, TXT(T_READ), fname2);
						DBG(fprintf(stderr, "read: error %d on file 2\n", saved_errno));
					}

					if (fpinput != stdin)
						fclose(fpinput);
					fp2 = fpinput = NULL;
				}
			}
			read2 = FALSE;
		}

		/*	All done ? */
		if (fp1 == NULL && fp2 == NULL)
			break;
		first=0;

		DBG(fprintf(stderr, "compare: main compare\n"));
		if (fp1!=NULL && fp2!=NULL) {
			match = compare(Buf1, Join1, Buf2, Join2);
		} else match=0;
	
		if (match == 0 && fp1 != NULL && fp2 != NULL) {
			match1 = match2 = TRUE;

			/*
			 *	If there's a match and subsequent lines containing matching
			 *  fields are not already being cached to a temporary file and
			 *  that same temporary file is not currently being used for
			 *  input then enable caching (rewind & truncate the temp file). 
			 */
			DBG(fprintf(stderr, "compare: start caching?\n"));
			if (saved_position == 0 && fp2 != fptmp && compare(Buf2, Join2, SaveBuf2, Join2) == 0) {
				DBG(fprintf(stderr, "cache: rewind temp file and start caching\n"));

				saved_position = 1;
				if ( ltrunc( fileno(fptmp), (off_t)0, SEEK_SET) == -1 ) {
					fprintf(stderr, TXT(T_ERR_FILE), Me, errno, TXT(T_TRUNCATE), T_TMP_FILE);
					exit(1);
				}
				rewind(fptmp);

				DBG(fprintf(stderr, "cache: \"%s\" previous line to temp file\n", SaveBuf2));
 				if (EOF==fputs(SaveBuf2, fptmp)) {
					fprintf(stderr, TXT(T_ERR_FILE), Me, errno, TXT(T_WRITE), T_TMP_FILE);
					exit(1);
				}
			}
			if (saved_position == 1 && fp2 != fptmp) {
				DBG(fprintf(stderr, "cache: \"%s\" to temp file\n", Buf2));
 				if (EOF==fputs(Buf2, fptmp)) {
					fprintf(stderr, TXT(T_ERR_FILE), Me, errno, TXT(T_WRITE), T_TMP_FILE);
					exit(1);
				}
			}

			output(Buf1, Buf2);
			read2 = TRUE;
			continue;
		}

		if ((match < 0 && fp1 != NULL) || fp2 == NULL) {
			/*	No match: field is only in file 1 */
			if (AlwaysOutput & 0x01 && match1 == FALSE)
				output(Buf1, "");
			read1 = TRUE;
			continue;
		}

		if ((match > 0 && fp2 != NULL) || fp1 == NULL) {
			/*	No match: field is only in file 2 */
			if (AlwaysOutput & 0x02 && match2 == FALSE)
				output("", Buf2);
			read2 = TRUE;
			continue;
		}

	}

	return(0);
}

/*
 *	Open a file
 */
FILE *open_file (char *fname)
{
	FILE *fp;

	/*	If fname is "-" then use stdin otherwise open the file */
	if (fname[0] == '-' && fname[1] == '\0') {
		fp = stdin;
	}
	else {
		/*	Open the file */
		if (!(fp = fopen(fname, "r"))) {
			/*	If there's an error, display it... */
			fprintf(stderr, TXT(T_ERR_FILE), Me, errno, TXT(T_OPEN), fname);
			return(NULL);
		}
	}

	return fp;
}

int compare (char *buf1, int field1, char *buf2, int field2)
{
	char *p1, *p2;
	int l1, l2, rc;

	DBG(fprintf(stderr, "compare: buf1='%s', field1=%d, buf2='%s', field2=%d\n",
                          buf1, field1, buf2, field2));

	p1 = getfield(buf1, field1, &l1);
	p2 = getfield(buf2, field2, &l2);


	DBG(fprintf(stderr, "compare: \"%-*s\" (%2d)  \"%-*s\" (%2d)  ", 
		l1, p1, l1, l2, p2, l2));

	for ( ; l1 && l2 && *p1 == *p2; ++p1, ++p2, --l1, --l2)
		;

	if (l1 == 0 && l2 == 0)
		rc = 0;
	else if (l1 == 0)
		rc = -1;
	else if (l2 == 0)
		rc = 1;
	else 
		rc = (*p1 - *p2) < 0 ? -1 : 1;

	DBG(fprintf(stderr, "match=%d\n", rc));

	return rc;
}

int output (char *buf1, char *buf2)
{
	char *p, *op;
	int file, field, first;
	int valid;

	/* one of buf1 or buf2 will point to an empty string if this is a
       non-matching line. -v suppresses the default (matching) output.
       Move this to after the else statement, below to make -v suppress
       output only when -o hasn't been specified (interpretive issue) */
	if (dashv && buf1[0] && buf2[0]) return 0;

	op = OBuf;

	if (*FieldList != '\0') {
		for (first=1, p=FieldList; *p; ) {
			while ((*p == ' ' || *p == '\t' || *p == '\n') && *p != '\0')
				++p;
			file = *p - '0';
			if (file != 1 && file != 2)
				bad_field_list();			/*  This will exit() */
			if ((*++p != '.') || (*(p+1) < '0' || *(p+1) > '9'))
				bad_field_list();
			field = atoi(++p);
			while (*p >= '0' && *p <= '9')
				++p;

			if (first)
				first=0;
			else
				*op++ = Delim == '\0' ? OutDelim : Delim;

			if (file == 1)
				op += putfield(op, buf1, field);
			else
				op += putfield(op, buf2, field);
			}
	}
	else {
		/*
		 *  Default output: match field (field 1 from both files), followed
		 *	by the rest of the line from file 1 and then the rest of the line
		 *	from file 2.
		 */
		op += putfield2(op, buf1, 1, &valid);
		if (!valid)
			op += putfield2(op, buf2, 1, &valid);

		for (field=2; field; ++field) {
			*op++ = Delim == '\0' ? OutDelim : Delim;
			op += putfield2(op, buf1, field, &valid);
			if (!valid) {
				*--op = '\0';
				break;
			}
		}
		for (field=2; field; ++field) {
			*op++ = Delim == '\0' ? OutDelim : Delim;
			op += putfield2(op, buf2, field, &valid);
			if (!valid) {
				*--op = '\0';
				break;
			}
		}

	}

	DBG(fprintf(stderr, "output: %s\n", OBuf));

	puts(OBuf);

	return 0;
}


/*
 *	Put the field <field> from <buf> into the output buffer <obuf> and
 *  return the number of characters transferred.
 */
int putfield (char *obuf, char *buf, int field)
{
	char *p;
	int i, l;

	p = getfield(buf, field, &l);
	if (p==NULL || l == 0)
		l = strlen(p=EmptyField);

	for (i=0; i<l; ++i)
		*obuf++ = *p++;
	*obuf = '\0';

	return l;
}


/*
 *	Put the field <field> from <buf> into the output buffer <obuf> and
 *  return the number of characters transferred. Set the <valid> flag
 *  to TRUE if the field exists, otherwise set it to FALSE.
 */
int putfield2 (char *obuf, char *buf, int field, int *valid)
{
	char *p;
	int i, l;

	p = getfield(buf, field, &l);

	if (p == NULL) {
		*valid = FALSE;
		return 0;
	}
	else
		*valid = TRUE;

	if (l == 0)
		l = strlen(p=EmptyField);

	for (i=0; i<l; ++i)
		*obuf++ = *p++;
	*obuf = '\0';

	return l;
}


/*
 *	Find the beginning of field number <field> within <buf>, return a
 *  pointer to it and stuff <len> with its length.  Return NULL if the
 *  field doesn't exist.
 */
char *getfield (char *buf, int field, int *len)
{
	char *sp = NULL;
	char *p = buf;
	int i, l = 0;

	if (field < 1)
		field = 1;

	if (Delim) {
		for (i=0; i<field && *p != '\0' && *p != '\n'; ++i) {
			sp = p;
			for (l=0; *p != Delim && *p != '\0' && *p != '\n'; ++p, ++l)
				;
			if (*p == Delim)
				++p;
		}
	}
	else {
		while (*p == ' ' || *p == '\t' && *p != '\0' && *p != '\n')
			++p;
		for (i=0; i<field && *p != '\0' && *p != '\n'; ++i) {
			sp = p;
			for (l=0; (! (*p == ' ' || *p == '\t')) && *p != '\0' && *p != '\n'; ++p, ++l)
				;
			while (*p == ' ' || *p == '\t' && *p != '\0' && *p != '\n')
				++p;
		}
	}

	*len = l;

	if (i == field)
		return sp;
	else
		return NULL;
}


void bad_field_list ()
{
	fprintf(stderr, T_ERR_LIST, Me);
	exit(1);
}
