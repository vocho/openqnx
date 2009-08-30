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





/* #define DIAG */
#ifdef __USAGE		/* tr.c */
%C - translate characters (POSIX)

%C	[-cs] [-r <filename>] <string1> <string2>
%C	-s [-c] [-r <filename>] <string1>
%C	-d [-c]  [-r <filename>] <string1>
%C	-ds [-c] <string1> <string2>
Options:
 -c            Complement the chars in <string1> with respect to \0-\255.
 -d            Delete all input characters in <string1>.
 -r<file>      Translate the named file in place (not using stdin, stdout).
 -s            Squeeze output resulting from input characters present in
               <string1> that were repeated in the input, to single instances
               of the corresponding character in <string2>, or if <string2>
               is not specified, to a single instance of the input character
               itself.
 -ds           The combination of -d and -s options causes <string1> to
               be used for the set of characters to delete in the input,
               and <string2> to be used as the set of characters to squeeze
               (in the input -- tr will only delete and squeeze when -ds
               is specified, it will not translate).
Where:
 <string1>     is the 'from' or 'deletion' (-d) set of characters
 <string2>     is the 'to' set of characters
Note:
 The following sequences have special meaning inside <string1> or <string2>:

 \octal        \ followed by 1-3 octal digits.
 \char         \ followed by a character represents that character.
 [c-c]         A range of characters.
 [:class:]     The set of all chars in class where class is one of {alpha,
               upper, lower, digit, xdigit, alnum, space, punct,
               print, graph, cntrl, blank}.
 [.cs.]        Multi-character collating symbol.
 [x*n]         n repeated occurrences of the character or collating
               symbol x. e.g. [[.\\15\\12.]*4] is allowed. When n is not
               specified the sequence will be expanded to grow string2 to
               the size of string1. [x*n] is not allowed in string1.
#endif

/* 
	Current points of concern:

		o	Internationalization

		o	What happens when the same symbol/char is specified TWICE
			in string1?
*/

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.17  2007/04/26 20:18:39  aristovski
	PR: 46523
	CI: rmansfield

	Added include lib/compat.h

	Revision 1.16  2005/06/03 01:38:02  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.15  2005/06/02 14:25:39  dsarrazin
	Add support for the ASCII escape chars. (\n, \r, \t, and the like).  This
	brings us more in-line with POSIX, and corrects a JCI PR.
	
	PR 24740
	
	Revision 1.14  2003/09/02 14:59:26  martin
	Add QSSL Copyright.
	
	Revision 1.13  2000/12/21 18:14:41  seanb
	- exit status should be <2^7.
	
	Revision 1.12  2000/12/21 15:06:24  seanb
	- Incorrect use of tempnam() (neutrino specific code) causing -r option
	  to fail out erroneously.
	
	Revision 1.11  1998/10/22 14:49:03  bstecher
	*** empty log message ***
	
	Revision 1.10  1998/10/22 14:35:45  eric
	*** empty log message ***

	Revision 1.9  1996/09/17 14:50:40  steve
	got rid of silly include files. Fastio is now default.

 * Revision 1.8  1995/04/12  18:43:06  eric
 * (hopefully) amended to allow
 * [a-df-g] instead of [a-d][f-g]
 *
 * Revision 1.7  1995/01/06  16:52:08  eric
 * Upped maximum symbol length to 128 bytes.
 *
 * Revision 1.6  1992/10/27  19:44:39  eric
 * added usage one-liner
 *
 * Revision 1.5  1992/07/09  14:43:06  garry
 * *** empty log message ***
 *
 * Revision 1.4  1992/05/05  17:50:57  eric
 * changed back from private _tmpnam to standard library issue, since wcs
 * fixed theirs to use the TMPDIR envar
 *
 * Revision 1.3  1992/02/05  21:05:45  eric
 * now uses _tmpnam/_tmpfile, tmp file in TMPDIR instead of current
 * directory.
 *
 * Revision 1.2  1991/11/14  22:22:24  eric
 * fixed severe problem with tr -s in absence of 2nd string argument.
 * (didn't work at all - replaced everything in first string with
 * a smiley face. Not funny.)
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *
	
	Revision 1.8 Thu Apr  5 12:42:45 1990 opr
	Bug fix in processing of ring buffer during multi-character
	collation element (token) recognition.
	
	Revision 1.7 Wed Mar 21 09:04:14 1990 ejohnson
	Small changes for watcom
	
	Revision 1.6 Wed Mar  7 14:08:37 1990 ejohnson
	Added prototypes in conditional section. Looks for ANSI.
	Under CI compile it -AS -Za -DANSI tr.c psx_getoptS.obj
	
	Revision 1.5 Tue Mar  6 16:17:27 1990 ejohnson
	changed to use cdefs.h type definitions
	
	Revision 1.4 Wed Jan 10 12:44:45 1990 ejohnson
	Added support for xlation to and from user-declared multi-
	character collating symbols via [.collating_symbol.]
	This involved a change from pascal character strings to
	pascal word-length token strings.
	
	Also the syntax for multiples of collating elements [x*n] now
	also will accept multi-char collating symbols. e.g.
	[[.cs.]*8]
	
	Revision 1.3 Wed Nov 29 14:42:44 1989 ejohnson
	added class support for the following:
	
	[:alpha:]		- set of all alphabetic characters
	[:upper:]		- set of all uppercase characters
	[:lower:]		- set of all lowercase characters
	[:digit:]		- set of all digit characters (0-9)
	[:xdigit:]		- set of all hex digit chars
	[:alnum:]		- union of alpha and digit
	[:space:]		- set of all whitespace characters (space,tab,nl,cr,ff,vt)
	[:blank:]		- tab or space characters
	[:punct:]		- set of all punctuation characters
	[:print:]		- set of all printable characters
	[:cntrl:]		- set of all control characters
	
	These will be expanded in regular ASCII collating sequence (for now).
	
	
	Revision 1.2 Wed Nov 29 11:16:00 1989 ejohnson
	basic working tr, less multi-char collation, less classes, less
	equivalence classes.
	
	Revision 1.1 Tue Aug  1 11:12:02 1989 ejohnson
	 *** QRCS - Initial Revision ***
	
---------------------------------------------------------------------*/

/*
-----------------------------------------------------------------------
POSIX UTILITY: TR

1003.2 -- Draft 9 (Shell and Utilities working group)

-----------------------------------------------------------------------
*/

/*
-------------------------------------------------------- INCLUDES --------
*/

#ifdef __MINGW32__
#include <lib/compat.h>
#endif

#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <malloc.h>
#include <inttypes.h>

/* my extensions */
#include <util/stdutil.h>
#include <util/defns.h>


#ifdef __STDC__
#define ANSI
#endif

#ifdef __QNXNTO__
#include <unix.h>
#endif

/*
-------------------------------------------------------- MANIFESTS -------
*/

#define READWRITE_ERR_RC  1
#define SUCCESS_RC		  0
#define INVALID_USAGE_RC  -1
#define CANT_EXPAND		  -1		/* rc from strnexpand */
#define PASS_AGAIN		  -2		/* rc from strnexpand */

#define TXT(s)			  s

#define T_NSUPPORTED			"tr: Equivalence classes not supported."
#define T_UNKNOWN_CLASS			"tr: Unknown class specification [:class:]"
#define T_MISSING_CLASS_END		"tr: Bad class specification - expected closing ':]'"
#define T_COL_ELS_ONLY			"tr: Only [ convention allowed as x in [x*n] is [.cs.]"
#define T_EXPECTING_DOTBRACKET  "tr: Bad collating element definition - expected closing '.]'"
#define T_UNEXPECTED_BRACKET 	"tr: Unexpected end bracket."
#define T_MISSING_BRACKET		"tr: Missing end bracket."
#define T_STRING_TOO_BIG		"tr: Expanded string too large."
#define T_UNKNOWN_CONV			"tr: Unrecognized bracket convention."
#define T_XN_NOT_ALLOWED		"tr: [x*n] convention not allowed in <string1>."
#define T_CANT_OPEN_FILE		"tr: Can't open file '%s'."
#define T_WRITE_ERROR			"tr: Error writing file "
#define T_MUST_SUPPLY_FILENAME  "tr: Must supply filename for -r option."
#define T_TOO_MANY_OPERANDS		"tr: Too many operands.\n"
#define T_CANT_SQUEEZE_AND_DELETE "tr: Can't squeeze (-s) and delete (-d) at the same time.\n"
#define T_MISSING_STRING1		"tr: Missing <string1>\n"
#define T_MISSING_STRING2		"tr: Missing <string2>\n"
#define T_STRING2_NOT_REQUIRED  "tr: delete option (-d) does not use <string2>\n"


/* Manifests for values in the action table. Even though they are
   bit fields, DONT_XLAT, DELETE_CHAR, XLAT_SYM are mutually
   exclusive. SQUEEZE may be set on any of them.
*/
#define DONT_XLAT	1
#define DELETE_CHAR 2
#define XLAT_SYM 	4
#define SQUEEZE		16

/* The minimum size in our symbolic buffer before we reallocate */
#define REALLOC_THRESHOLD	5

#define MAX_SYMBOLS 512

#define isoctal(c) (c>='0' && c<= '7')

/*
---------------------------------------------------------- STRUCTS ---------
*/

/* use pascal-style strings so we can xlate to and from NULL bytes */
typedef struct Spascalstring {
	uint16_t len;
	uint8_t  *dat;
} pascalstring;

typedef struct Ssymbolstring {
	uint16_t len; 
	uint16_t max;
	uint16_t *sym;				/* sym points to WORD data - symbols */
} symbolstring;

/*
---------------------------------------------------- PROTOTYPES --------
*/

#ifdef __STDC__
	uint8_t	octalorliteral	(pascalstring*, uint16_t*);
	uint16_t	octalordecimal	(pascalstring*, uint16_t*);
	int16_t	col_el_parse	(pascalstring*, uint16_t*);
	int16_t	strnexpand		(pascalstring*,
							 symbolstring*,
							 int16_t, bool, int16_t);
	void	expandbuffer(symbolstring *, int);
	void	init_symbols	(void);
	int16_t	find_symbol		(int16_t, uint8_t*);
	int16_t	make_symbol		(int16_t, uint8_t*);
	int16_t	put_symbol		(uint16_t);
	int16_t	get_symbol		(void);
#endif

/*
---------------------------------------------------- GLOBALS -----------
*/

uint16_t top_symbol;					/*	number of the highest symbol present in
								   		symbol[] */
									
pascalstring symbol[MAX_SYMBOLS];	/*	allow basic 256 character symbols
							   			plus another 256 multi-character
										collating symbols defined in the
										strings passed on the command line */

pascalstring cstring1, cstring2;	/*	null term strings in p-structs */
symbolstring sstring1, sstring2;	/*	symbolized versions of same */

int16_t  squeeze, delete, complement;	/* set when cmd line options are parsed */

uint8_t  action[MAX_SYMBOLS];	/* action for each incoming symbol, | 16 means squeeze */
uint16_t xlat[MAX_SYMBOLS];	/* output_symbols */

char *filename;
#ifndef __QNXNTO__
char outfilename[_POSIX_PATH_MAX+1];
#else
char *outfilename;
#endif
FILE *input_fp, *output_fp;

int replace;				/* flag for translate-in-place */
int errflag=0;				/* flag for error occurring in get_symbol() */

/*
------------------------------------------------- octalorliteral() ---------
octalorliteral(pascalstring *string,uint16_t *index)

	For use in translating "\something" to a single character.
	It is assumed that the '\' has been parsed by the fn calling this
	one. string->dat[*index] should be the first character of 'something'.
	If the first character is not an octal digit, this fn returns that
	character ('literal'). If it is an octal digit, this fn will convert
	that plus up to two more octal digits as an octal number and will
	return the byte equivalent of that number. 

	e.g. "66fred" would return (char) 0x36

	a pointer to index is passed so the calling fn can resume parsing
	from the point AFTER the octal digits.

	Up to three characters are read. 

----------------------------------------------------------------------------
*/

uint8_t octalorliteral (pascalstring *string,uint16_t *index)
{
	uint8_t rc;
	uint16_t numdigit;

	/* read	in octal sequence */
	for (	rc = 0,	numdigit=0;
			isoctal(string->dat[*index]) && (numdigit<3);
			numdigit++
		) {
		rc *= (uint8_t) 8;
		rc += (uint8_t) (string->dat[(*index)++] - '0');
	}

	/* if it wasn't an octal sequence, take next char literal */
	if (!numdigit) rc = string->dat[(*index)++];

	/* PR 24740 - Have to check for "\r", "\n", "\t", etc. */
	switch (rc){
		case '\\':
			rc = '\\';
			break;
		case 'a':
			rc = '\a';
			break;
		case 'b':
			rc = '\b';
			break;
		case 'f':
			rc = '\f';
			break;
		case 'n':
			rc = '\n';
			break;
		case 'r':
			rc = '\r';
			break;
		case 't':
			rc = '\t';
			break;
		case 'v':
			rc = '\v';
			break;
	}

	/* return rc, string->dat[*index] is now pointing just past the
	   stuff we processed */

	#ifdef DIAG
		fprintf(stderr,"Octal %d\n",(int) rc);
	#endif

	return(rc);
}

/*
------------------------------------------------- octalordecimal() ---------
octalordecimal(pascalstring *string,uint16_t *index)

	For use in reading an octal or decimal number from a pascal string.
	string->dat[*index] should be the first character of the number.
	If the first character is a '0', the number will be read as octal.
	Otherwise, it will be read as decimal. Interpretation will stop
	as soon as the first non-decimal (or non-octal) number is encountered.
	The number read will be returned (uint16_t) and index will be
	for the char which caused the routine to stop.

	e.g. "123]ed" would return (uint16_t) 123 decimal

----------------------------------------------------------------------------
*/

uint16_t octalordecimal (pascalstring *string,uint16_t *index)
{
	uint16_t rc;
	uint16_t numdigit;

	rc = 0;
	numdigit = 0;

	if (string->dat[*index] == '0') {
		/* read	in numerical sequence */
		for (;isoctal(string->dat[*index]) && (numdigit<5);numdigit++) {
			rc *= 8;
			rc += string->dat[(*index)++] - (uint16_t)'0';
		}

		#ifdef DIAG
			fprintf(stderr,"Octal %d\n",rc);
		#endif
	} else {
		/* read	in numerical sequence */
		for (;isdigit(string->dat[*index]) && (numdigit<4);numdigit++) {
			rc *= 10;
			rc += string->dat[(*index)++] - (uint16_t)'0';
		}
		#ifdef DIAG
			fprintf(stderr,"Decimal %d\n",rc);
		#endif
	}

	if (!numdigit) rc = 0;
	/* return rc, string->dat[*index] is now pointing just past the
	   stuff we processed */

	return(rc);
}

/*
----------------------------------------------------------------------------
col_el_parse(pascalstring *string,uint16_t *index)

    string->data[*index] is supposed to point just past a starting [. --
	the start of a collating sequence definition encountered during 
    higher level parsing. This fn will convert the remainder of the
    collating sequence to a symbol (word-length handle). A new
	symbol may be added in the process to the symbol table if no
	matching symbol was already defined. *index will be left pointing
	just past the end of the closing .].

	Returns:
	
		>0 = symbol equivalent to the collating element passed.
		-1 = Error in syntax or in creation of a new symbol.
----------------------------------------------------------------------------
*/

int16_t col_el_parse (pascalstring *string,uint16_t *index)
{
	int16_t openbrackets,s;
	int8_t  *string_start;
	uint16_t start_index;

	/* remember location of element start */
	string_start = &(string->dat[*index]);
	start_index = *index;
	
	/* skip ahead to corresponding .] */
	for (openbrackets=1;openbrackets && (*index<string->len);(*index)++) {
		if (string->dat[*index] == ']')	openbrackets--;
	}

	if (openbrackets) {
		fprintf(stderr,"%s (%s)\n",TXT(T_MISSING_BRACKET),string->dat);
		return(-1);
	}
		
	if (string->dat[*index-2]!='.') {
		fprintf(stderr,"%s (%s)\n",TXT(T_EXPECTING_DOTBRACKET),string->dat);
	    return(-1);
	}


	/* null terminate string just before the .] */
	string->dat[*index-2] = (int8_t) 0;

	/* interpret any \xxx present in the
	   string. Put data into buffer, then
	   make the symbol */
	{
		uint8_t buffer[128];
		int16_t bufind = 0;

		for (;start_index<(*index-2);start_index++) {
			if (string->dat[start_index]=='\\') {
				start_index++;
				buffer[bufind++] = octalorliteral(string,&start_index);
				start_index--;
			} else buffer[bufind++] = string->dat[start_index];
		}
	
		/* if symbol is already present, this
		   will return that symbol, otherwise it
		   will create the new symbol and return
		   it */
	    s = make_symbol(bufind,buffer);
	}

	string->dat[*index-2]='.'; /* restore . */

	return(s);	/* return symbol or err code from make_symbol */
}

/*
----------------------------------------------------------------------------
expandbuffer(symbolstring *dest, int size)

	Expand the symbol buffer to fit more data, and copy the old
	data into the new buffer.  Make sure size is always greater
	than dest->max.  If dest->max equals size, then a new copy
	will be created and the data will be moved.  The old symbol
	buffer will be freed at the end of this operation.

----------------------------------------------------------------------------
*/
void expandbuffer(symbolstring *dest, int size)
{	
	int16_t i;
	uint16_t *oldsym;
	if (size < dest->max){
		return;
	}

	#ifdef DIAG
	fprintf(stderr,"Expanding buffer to %d (%d was exhausted, len = %d)\n",size, dest->max, dest->len);
	#endif
	oldsym = dest->sym;
	dest->sym = malloc(size * sizeof(dest->sym));
	if (dest->sym == NULL){
		exit(EXIT_FAILURE);
	}
	for(i = 0; i < dest->len; i++){
		dest->sym[i] = oldsym[i];
	}
	free(oldsym);
	dest->max = size;
}

/*
------expand_range(orig,dest,oind,xn_allowed,open_bracket,
                                            length_difference,xsplat,max) ---
int16_t expand_range (	pascalstring *orig,
				 		symbolstring *dest,
						uint16_t *oind,
						bool xn_allowed,
						bool open_bracket,
						int16_t length_difference,
						int *xsplat,
						int16_t max)
{

	Expands the form c1-c2 or x*n to their strings of symbols.  Cut out of 
original strnexpand function when adding support for c1-c2 without brackets to
avoid redundant code.

If xn_allowed is TRUE, attempts to resolve x*n form as well

If open_bracket is TRUE, then the expression legally must resolve to 
c1-c2 or x*n forms, or an error message is output and CANT_EXPAND is returned.

If open_bracket is FALSE, then no error messages will be output, and the
return value indicates success or failure of the function, allowing
normal processing of the character string where neither form exists.
	
Returns:

	SUCCESS_RC on success
	CANT_EXPAND on failure

----------------------------------------------------------------------------
*/

int16_t expand_range (	pascalstring *orig,
				 		symbolstring *dest,
						uint16_t *oind,
						bool xn_allowed,
						bool open_bracket,
						int16_t length_difference,
						int *xsplat,
						int16_t max)
{
	int c1,c2,nextc;
	uint16_t n;

	do {
		if ((c1=(int16_t)orig->dat[(*oind)++]) == '\\') {
			/* char is a literal or octal */
			c1 = (int16_t)octalorliteral(orig,oind);
		} else if (c1 == '[') {
			if (orig->dat[(*oind)]=='.') {
				/* its a collating element def'n */
				(*oind)++;
				c1=col_el_parse(orig,oind);
				if (c1==-1) return(CANT_EXPAND);
			} else {
				if (open_bracket)
					fprintf(stderr,"%s (%s)\n",TXT(T_COL_ELS_ONLY),orig->dat);
				return(CANT_EXPAND);
			}
		}
			
		switch(orig->dat[(*oind)]) {
			case '-':		/* char range [c-c] */
				if ((c2=(int16_t)orig->dat[++(*oind)]) == '\\') {
					/* char is a literal or octal */
					(*oind)++;
					c2 = (int16_t)octalorliteral(orig,oind);
					(*oind)--;
				} else if (c2==']') {
					/* Error */
					if (open_bracket)
						fprintf(stderr,"%s (%s)\n",TXT(T_UNEXPECTED_BRACKET),orig->dat);
					return(CANT_EXPAND);
				} else if (c2 == '[') {
					if ((orig->dat[(*oind)]=='.')) {
						/* its a collating element def'n */
						(*oind)++;
						c2=col_el_parse(orig,oind);
						if (c2==-1) return(CANT_EXPAND);
						(*oind)--;
					} else {
						if (open_bracket)
							fprintf(stderr,"%s (%s)\n",TXT(T_COL_ELS_ONLY),orig->dat);
						return(CANT_EXPAND);
					}
				}


				nextc=orig->dat[++(*oind)];
#ifdef OLD
				if ((nextc=orig->dat[++(*oind)]) != ']') {
					/* Error */
					fprintf(stderr,"%s (%s)\n",TXT(T_MISSING_BRACKET),orig->dat);
					return(CANT_EXPAND);
				}
#endif

				/* copy range of characters to string */
				{
					uint16_t i;

					if (c1<=c2){
						for (i=(uint16_t)c1;i<=(uint16_t)c2;i++){
							if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
							dest->sym[dest->len++] = i;
						}
					}else{ 
						for (i=(uint16_t)c1;i>=(uint16_t)c2;i--){
							if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
							dest->sym[dest->len++] = i;
						}
					}
				}								
					
#ifdef OLD
				(*oind)++;
#endif

				break;

			case '*':		/* multiples of 'c' [x*n] */
				if (!xn_allowed) {
					if (open_bracket)
						fprintf(stderr,"%s (%s)\n",TXT(T_XN_NOT_ALLOWED),orig->dat);
					return(CANT_EXPAND);
				}

				(*oind)++;
				if (orig->dat[(*oind)] == ']') {
					n = 0;
                } else {
					/* convert this to a nummer */
					n = octalordecimal(orig,oind);
					if (orig->dat[(*oind)]!=']') {
						/* Error */
						if (open_bracket)
							fprintf(stderr,"%s (%s)\n",TXT(T_MISSING_BRACKET),orig->dat);
						return(CANT_EXPAND);
					}
				}


				if (!n) {
					for (;length_difference;length_difference--){
						if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
						dest->sym[dest->len++] = (uint16_t) c1;
					}
					(*xsplat)++;
				} else {
					for (;n;n--) {
						if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
						if (dest->len<max)
							dest->sym[dest->len++] = (uint16_t) c1;
						else {
							if (open_bracket)
								fprintf(stderr,"%s (%s)\n",TXT(T_STRING_TOO_BIG),orig->dat);
							return(CANT_EXPAND);
						}
					}
				}
#ifdef OLD
                (*oind)++;
#endif
				break;

			default:	/* Not c1-c2 OR x*n */
				if (open_bracket)
					fprintf(stderr,"%s (%s)\n",TXT(T_UNKNOWN_CONV),orig->dat);
				return(CANT_EXPAND);
				break;
		}
	} while (nextc && nextc!=']');

	return SUCCESS_RC;

}

/*
---------------------------------------------- strnexpand(orig,dest,max) ---
int16_t strnexpand(pascalstring *orig,
				 symbolstring *dest,
				 int16_t max,
				 bool  xn_allowed,
				 int16_t length_difference);

	Expands the string passed in orig into dest. max is the maximum
size that dest can hold. orig->dat is expected to contain a NULL-TERMINATED
string. The result in dest will NOT be null-terminated. Its length is
determined by the value of dest->len. The destination string is a string
of WORD-LENGTH symbol HANDLES, *NOT* character data. The symbols must
be looked up to obtain actual data for each element.

	The conversions supported are:

\octal		-	\nc, \nnc, \nnnx	where n is octal digit, c is other
									char not octal, x is don't care.

\character	-	\c	literal character 'c' e.g. \\ is '\'

[c-c]		-	range of characters. either char may be specified using
				\octal or \character in addition to plain text.

IF xn_allowed is TRUE:

[x*n]		-	expands to n occurrances of char 'x'. x may be specified
				as \octal, \character, or plain. n will be interpreted
				as octal if there is a leading '0', otherwise decimal
				will be assumed.

For x*0 or x* to work, strnexpand must be called with 
length_difference set to the difference in length between the two strings
if NO occurrances of x were inserted. i.e. when processing string1, xn_allowed
should be FALSE. Then process string2 once with xn_allowed TRUE but
length_difference 0 (so that x* results in 0 chars). Then process string2
again, with xn_allowed TRUE, and length_difference set to the difference in 
length between string1 and the result of the first pass on string2.

Returns:

	Positive size of dest on success. (may be 0)
	Negative error code on failure.

----------------------------------------------------------------------------
*/

int16_t strnexpand (pascalstring *orig,
				  symbolstring *dest,
				  int16_t max,
				  bool	xn_allowed,
				  int16_t	length_difference)
{
	uint16_t oind;	/* index into original string */
	uint16_t n;
	uint16_t  len_tmp, oind_tmp; /* temps in case of failed attempt */
	int16_t  i;		/* general purpose int variable */
	int	   xsplat = 0;	/* flag saying we encountered an xsplat */

	dest->len = 0;

	oind = 0;

	#ifdef DIAG
	fprintf(stderr,"in strnexpand, length_difference = %d\n",length_difference);
	#endif

	/* for each character in the original */
	while (oind < orig->len) {
		if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
		/* copy char to dest - increment oind & switch on char's value */
		switch(dest->sym[dest->len] = (uint16_t) orig->dat[oind++]) {

			case '\\':	/* octal sequence or literal char */
				/* symbol handles for single-character symbols are always the
				   same as the word-length version of the data they represent */
				dest->sym[dest->len] = (uint16_t) octalorliteral(orig,&oind);
				dest->len++;
				break;

			case '[':	/* all other expansions */
				switch(orig->dat[oind]) {
					case ':':	/* class */
						switch(orig->dat[++oind]) {
							case 'a':		/* alpha or alnum */
								if (!strncmp(&orig->dat[oind],"alpha",5)) {
									oind+=5;
									for (i=0;i<256;i++)
										if (isalpha(i)){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								} else if (!strncmp(&orig->dat[oind],"alnum",5)) {
									oind+=5;
									for (i=0;i<256;i++)
										if (isalnum(i)){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								}   /* fall through on error */

							case 'b':		/* blank */
								if (!strncmp(&orig->dat[oind],"blank",5)) {
									oind+=5;
									for (i=0;i<256;i++)
										if (i=='	'||i==' '){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								}	/* fall through on error */

							case 'c':		/* cntrl */
								if (!strncmp(&orig->dat[oind],"cntrl",5)) {
									oind+=5;
									for (i=0;i<256;i++)
										if (iscntrl(i)){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								}	/* fall through on error */

							case 'd':		/* digit */								
								if (!strncmp(&orig->dat[oind],"digit",5)) {
									oind+=5;
									for (i=0;i<256;i++)
										if (isdigit(i)){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								}	/* fall through on error */

							case 'l':		/* lower */
								if (!strncmp(&orig->dat[oind],"lower",5)) {
									oind+=5;
									for (i=0;i<256;i++)
										if (islower(i)){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								}	/* fall through on error */

							case 'p':		/* print or punct */
								if (!strncmp(&orig->dat[oind],"print",5)) {
									oind+=5;
									for (i=0;i<256;i++)
										if (isprint(i)){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								} else if (!strncmp(&orig->dat[oind],"punct",5)) {
									oind+=5;
									for (i=0;i<256;i++)
										if (ispunct(i)){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								}	/* fall through on error */

							case 's':		/* space */
								if (!strncmp(&orig->dat[oind],"space",5)) {
									oind+=5;
									for (i=0;i<256;i++)
										if (isspace(i)){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								}	/* fall through on error */

							case 'u':		/* upper */
								if (!strncmp(&orig->dat[oind],"upper",5)) {
									oind+=5;
									for (i=0;i<256;i++)
										if (isupper(i)){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								}	/* fall through on error */

							case 'x':		/* xdigit */
								if (!strncmp(&orig->dat[oind],"xdigit",6)) {
									oind+=6;
									for (i=0;i<256;i++)
										if (isxdigit(i)){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								}	/* fall through on error */

							case 'g':		/* graph */
								if (!strncmp(&orig->dat[oind],"graph",5)) {
									oind+=5;
									for (i=0;i<256;i++)
										if (isgraph(i)){
											if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
											dest->sym[dest->len++] = (uint16_t)i;
										}
									break;
								}	/* fall through on error */

							default:
								fprintf(stderr,"%s (%s)\n",TXT(T_UNKNOWN_CLASS),orig->dat);
								return(CANT_EXPAND);
								break;
						} /* switch for character classes */

						/* check for trailing :] */
						if (strncmp(&orig->dat[oind],":]",2)) {
							fprintf(stderr,"%s (%s)\n",TXT(T_MISSING_CLASS_END),orig->dat);
							return(CANT_EXPAND);
						}
						oind+=2; /* skip past end */

						break;	/* [: - class specifier */

					case '.':	/* collating element */
						{
							int16_t s;

							oind++;
							s = col_el_parse(orig,&oind);
							if (s==-1) return(CANT_EXPAND);
							if (dest->max - dest->len < REALLOC_THRESHOLD) expandbuffer(dest, dest->max*2);
							dest->sym[dest->len++] = (uint16_t) s;
						}
                        			break; /* [. collating element */
					default:	/* [x*n] || [c1-c2] */
						if ( (i=expand_range (orig, dest, &oind, xn_allowed, 
						TRUE, length_difference, &xsplat, max)) != SUCCESS_RC)
							return i;
		
						if (orig->dat[oind] != ']') {
							/* Error */
							fprintf(stderr,"%s (%s)\n",TXT(T_MISSING_BRACKET),orig->dat);
							return(CANT_EXPAND);
						} else oind++;
		
						break;	/* default case */

				} /* switch expansions startin with [ */	
				break; /* case [... */

			default:	/* not \anything, nor [anything; might still be c1-c2*/
				/*We can't have a c1-c2 without at least 3 characters left.*/
				/*As well as we can't be at the beginning here*/
				if (orig->len - oind >= 2 && oind > 0){
					/* save current dest->len in case of failure */
					len_tmp = dest->len;
					oind_tmp = oind--; /* need to pass in the previous index */
					if ( (i=expand_range (orig, dest, &oind, FALSE, FALSE,
						length_difference, &xsplat, max)) != SUCCESS_RC) {
							dest->len = len_tmp;
							oind = oind_tmp;
						} else if (orig->dat[oind] == ']') {
							/* Error */
							fprintf(stderr,"%s (%s)\n",TXT(T_UNEXPECTED_BRACKET),orig->dat);
							return(CANT_EXPAND);
						}
				}


			dest->len++;
			break;	
		} /* SWITCH 1 */
	}	/* WHILE */

	#ifdef DIAG
	fprintf(stderr,"xsplat = %d\n",xsplat);
	#endif
	return (xsplat?PASS_AGAIN:dest->len);
}



	
/*
------------------------------------------------------------- main ---------

	See POSIX 1003.2 draft 8 for functional description of utility.

	tr -cs string1 string2
	tr -d string1

	-c = complement (string1 = set of chars not in string1 passed)
	-s = squeeze output (multiple sequential instances in the output
		 of chars present in string2 are not output)
	-d = delete chars in string1, copy non-matching chars

----------------------------------------------------------------------------
*/

int
main (int argc,char **argv)
{
	int   opt, i, diff;


	#ifdef DIAG
		fprintf(stderr,"DIAGNOSTICS ON\n");
	#endif

	/* parse cmd line arguments */

	while ((opt = getopt(argc,argv,"sdc**r:")) != -1) {
		switch(opt) {
			case 's':	squeeze		=	TRUE;	break;
			case 'd':	delete		=	TRUE;	break;
 			case 'c':	complement	=	TRUE;	break;

 			case 'r':	replace		=	TRUE;
						filename    =   optarg;
						break;

			default:	errflag++;	break;
		}
	}

	if (optind<argc) cstring1.dat = argv[optind++]; else cstring1.dat = NULL;
	if (optind<argc) cstring2.dat = argv[optind++]; else cstring2.dat = NULL;

	/* check for too many operands */
	if (optind<argc) { fprintf(stderr,TXT(T_TOO_MANY_OPERANDS)); errflag++; };	if (squeeze&&delete) { fprintf(stderr,TXT(T_CANT_SQUEEZE_AND_DELETE)); errflag++; };

	/* check for no string1 */
	if (!cstring1.dat) { fprintf(stderr,TXT(T_MISSING_STRING1));errflag++; };

	/* must have a string2 unless doing a delete or squeeze operation */
	if (!(delete||squeeze||cstring2.dat)) {
		fprintf(stderr,TXT(T_MISSING_STRING2));
        errflag++;
    };

	/* if doing a delete, but not in combination with squeeze,
	   string2 is not required */
	if (delete&&!squeeze&&cstring2.dat) { fprintf(stderr,TXT(T_STRING2_NOT_REQUIRED));errflag++;};

	if (errflag) exit(EXIT_FAILURE);

	#ifdef DIAG
		fprintf(stderr,"%s %s %s\n",squeeze?"squeeze":"",delete?"delete":"",complement?"complement":"");
		fprintf(stderr,"str1 = '%s' str2 = '%s'\n",cstring1.dat?cstring1.dat:"",cstring2.dat?cstring2.dat:"");
	#endif

	init_symbols();		/* initialize single-character symbol definitions */

	{
		int err = 0;

		if (cstring1.dat) {
			cstring1.len = strlen(cstring1.dat);
			sstring1.len = 0;
			/* If we don't allocate + REALLOC_THRESHOLD + REALLOC_THRESHOLD + 1, */
			/* then we will just trigger a reallocation inside strnexpand */
			sstring1.max = cstring1.len + REALLOC_THRESHOLD + REALLOC_THRESHOLD + 1;
			sstring1.sym = malloc(sstring1.max * sizeof(sstring1.sym));
			if (sstring1.sym == NULL){
				exit(EXIT_FAILURE);
			}
			if (strnexpand(&cstring1,&sstring1,512,FALSE,0)==CANT_EXPAND) err++;
	
			#ifdef DIAG
				else {
					fprintf(stderr,"cstring1 = ");
					fwrite(cstring1.dat,cstring1.len,1,stderr);
					fprintf(stderr,"\n");
				
					/* Can't do this anymore (symbols, not chars)
						fprintf(stderr,"\npstring1 = ");
						fwrite(pstring1.dat,pstring1.len,1,stderr);
					*/
				}
			#endif
		}

		/* complement string1 - ANY MULTI-CHARACTER SYMBOLS WILL BE THROWN
		   AWAY IN THE COMPLEMENT OPERATION, SINCE NONE ARE IN THE BASIC
		   CHARACTER SET


		   complement here so string2 can be expanded to the right
		   length (if necessary)
		*/
	
		if (complement) {
			int8_t  comp[257];
			int16_t i;
	
			/* set comp all true */
			for (i=0;i<256;i++) comp[i] = (int8_t) 1;
	
			/* knock down items in comp covered by sstring1's symbols.
			   Make use of the fact that symbol #s < 256 are the same
			   as the character they represent. */
	
			for (i=0;i<sstring1.len;i++) {
				if (sstring1.sym[i]<256) comp[sstring1.sym[i]] = (int8_t) 0;
			}
	
			/* build new sstring1 from comp */
			for (sstring1.len=i=0;i<256;i++){
				if (sstring1.max - sstring1.len < REALLOC_THRESHOLD) expandbuffer(&sstring1, sstring1.max*2);
				if (comp[i]){
					 sstring1.sym[sstring1.len++] = (uint16_t)i;
				}
			}
		}
	
		if (cstring2.dat) {
			cstring2.len = strlen(cstring2.dat);
			sstring2.max = cstring2.len + REALLOC_THRESHOLD + REALLOC_THRESHOLD + 1;
			sstring2.sym = malloc(sstring2.max * sizeof(sstring2.sym));
			
			if (sstring2.sym == NULL){
				exit (EXIT_FAILURE);
			}
			diff = 0;

PASS_STRING2_AGAIN:
			sstring2.len = 0;

			switch (strnexpand(&cstring2,&sstring2,512,TRUE,diff)) {
				case CANT_EXPAND:	err++;	break;
				case PASS_AGAIN:
					#ifdef DIAG
						fprintf(stderr,"Got PASS_AGAIN back from strnexpand\n");
					#endif
					if (!diff) {
						diff = sstring1.len - sstring2.len;
						if (diff>0) goto PASS_STRING2_AGAIN;
					}
					/* fall through */					
				default:					
					for (;sstring2.len<=sstring1.len;sstring2.len++){
						if (sstring2.max - sstring2.len < REALLOC_THRESHOLD) expandbuffer(&sstring2, sstring2.max*2);
						sstring2.sym[sstring2.len] = sstring2.sym[sstring2.len-1];
					}
					#ifdef DIAG
						fprintf(stderr,"cstring2 = ");
						fwrite(cstring2.dat,cstring2.len,1,stderr);
					#endif
					break;
			}
		}

		if (err) exit(EXIT_FAILURE);
	}


	if (delete) {
		for (i=0;i<MAX_SYMBOLS;i++) action[i] = (int8_t) DONT_XLAT;

		/* for each symbol in sstring1 */
		for (i=0;i<sstring1.len;i++) {
           	action[sstring1.sym[i]] = DELETE_CHAR;
		}
	} else {
		/* erase table (make every entry its identity (no xlation)) */
		for (i=0;i<MAX_SYMBOLS;i++) action[i] = (int8_t) DONT_XLAT;

		if (!sstring2.len && squeeze) {
			sstring2 = sstring1;
		}

		if (sstring2.len) {
			for (i=0;i<sstring1.len;i++) {
				action[sstring1.sym[i]] = XLAT_SYM;
				xlat[sstring1.sym[i]] = sstring2.sym[i];
			}
		}
	}

	if (squeeze) {
		for (i=0;i<sstring2.len;i++) {
			action[sstring2.sym[i]] |= SQUEEZE;
		}
	}

	if (replace) {
		/* set input_fp, output_fp */
		if ((input_fp = fopen(filename,"r"))==NULL) {
			prerror(TXT(T_CANT_OPEN_FILE),filename);
			exit(EXIT_FAILURE);
		}

		#ifdef DIAG
			fprintf(stderr,"INPUT FILE = %s\n",filename);
		#endif
	
#ifndef __QNXNTO__
		tmpnam(outfilename);
#else
		outfilename = tempnam(NULL,"tr");
		if(outfilename == NULL) {
			fprintf(stderr, "Unable to create tmpfile\n");
			exit(EXIT_FAILURE);
		}
		
#endif

		output_fp = fopen(outfilename,"w+");

		if (output_fp == NULL) {
			prerror(TXT(T_CANT_OPEN_FILE),outfilename);
			exit(EXIT_FAILURE);
		}

		/* unlink it but keep it open */
		if (unlink(outfilename)==-1) prerror("couldn't unlink %s",outfilename);
		#ifdef DIAG
		fprintf(stderr,"INPUT FILE = %s\n",filename);
		#endif
	} else {
    	input_fp  = stdin;
		output_fp = stdout;	
	}

	/* set buffer to 4k to improve net throughput */
	setvbuf(input_fp,NULL,_IOFBF,4096);

	#ifdef DIAG
		fprintf(stderr,"doing the conversion\n");
	    if (replace) fprintf(stderr,"replace still set\n");
	#endif

	/* DO IT! */
	{
		uint16_t last_output;	/* symbol last output */
		int16_t s;

		/* make sure that last_output is set to something that
		   will *NOT* be the same as what we will first process! */
		last_output = 0xffff;

		if (top_symbol>255) {
			for (errflag=0;( (s=get_symbol()) !=EOF) && (!errflag);) {
				switch(action[s]&(XLAT_SYM|DONT_XLAT|DELETE_CHAR)) {
					case XLAT_SYM:
						s = xlat[s];
					case DONT_XLAT:
						if (!((last_output == s) && (action[s]&SQUEEZE))) {
							if (put_symbol(s)==EOF) {
								fprintf(stderr,TXT(T_WRITE_ERROR));
								exit(READWRITE_ERR_RC);
							}
	
							last_output = s;
						}
						break;
					default: break;
				}				
			}
		} else {
#ifdef DIAG
			int was;
			for (was=0;was<=top_symbol;was++) {
				fprintf(stderr,"'%c' (%d) %04x '%c' (%d)\n",
						was,was,action[was],xlat[was],xlat[was]);
			}
#endif
					
			/* no multi-character collating elements defined - can use
               quick and dirty approach */
			for (errflag=0;( (s=getc(input_fp)) !=EOF) && (!errflag);) {
				switch(action[s]&(XLAT_SYM|DONT_XLAT|DELETE_CHAR)) {
					case XLAT_SYM:
						s = xlat[s];
					case DONT_XLAT:
						if (!((last_output == s) && (action[s]&SQUEEZE))) {
#ifdef DIAG
							if (!s) {
								fprintf(stderr,"action['%c']=%d\n",was,action[was]);
								fprintf(stderr,"xlat['%c']='%c'(%d)\n",was,xlat[was],xlat[was]);
							}
#endif

							if (putc(s,output_fp)==EOF) {
								fprintf(stderr,TXT(T_WRITE_ERROR));
								exit(READWRITE_ERR_RC);
							}
	
							last_output = s;
						}
						break;
					default: break;
				}				
			}
			if (!feof(input_fp)) {	
				perror("tr: getc");		
				errflag++;
			}
		}
		if (errflag) exit(READWRITE_ERR_RC);
	}

	if (replace) {
		int c;

		fclose(input_fp);
		if (!(input_fp = fopen(filename,"w"))) {
			prerror(T_CANT_OPEN_FILE,filename);
			exit(EXIT_FAILURE);
		}
		rewind(output_fp);

		while ((c=getc(output_fp))!=EOF) if (putc(c,input_fp)==EOF) {
			prerror("%s %s",TXT(T_WRITE_ERROR),filename);
			exit(EXIT_FAILURE);
		}

		fclose(input_fp);
		fclose(output_fp);
#ifdef __QNXNTO__
		free(outfilename);
#endif
	}

	return(SUCCESS_RC);
}


/*
--------------------------------------------------------- init_symbols ------
--------------------------------------------------------- make_symbol  ------
*/

/*
------------ initialize single-character symbols ---
*/

void init_symbols ()
{
	static uint8_t x1st256[256];
	int16_t i;

	for (i=0;i<255;i++) {
		symbol[i].len = 1;
		symbol[i].dat = &x1st256[i];
		x1st256[i] = (uint8_t) i;
	}

	top_symbol = 255;
}

/*
------------ find a matching symbol ------------
*/
int16_t find_symbol (int16_t length,uint8_t *pointer)
{
	int16_t i;

	if (length<2) return((int16_t) *pointer);

	for (i=top_symbol;i>255;i--) {
		if (symbol[i].len == length) {
			if (!strncmp(pointer,symbol[i].dat,length)) {
				return(i);
			}
		}
	}

	return(-1);
}


/* 
------------ create multi-character symbol -----
returns -1 on failure, symbol # on success
*/

int16_t make_symbol (int16_t length,uint8_t *pointer)
{
	int16_t match;

	/* scan for matching symbol */
	if ((match = find_symbol(length,pointer))!=-1) {
		#ifdef DIAG
		printf("Symbol already present ('%s'=%d)\n",pointer,match);
		#endif
		return(match);
	}

	/* add symbol */
	symbol[++top_symbol].len = length;

	if (!(symbol[top_symbol].dat = malloc(length))) {
		return(-1);
	}

	memcpy(symbol[top_symbol].dat,pointer,length);

	#ifdef DIAG
	printf("Created symbol ('%s'=%d) len=%d\n",pointer,top_symbol,symbol[top_symbol].len);
	#endif

    return(top_symbol);
}

/*
----------------------------------------------------------- put_symbol ------
	outputs the symbol passed to stdout
-----------------------------------------------------------------------------
*/

int16_t put_symbol (uint16_t sym)
{
	int16_t i;
	int8_t  *c;

	i=symbol[sym].len;
	c=symbol[sym].dat;

	for (i=symbol[sym].len,c=symbol[sym].dat;i;i--) {
		if (putc(*c++,output_fp)==EOF)
			if (errno) {
				perror("tr: putc to stdout");
				return(EOF);
			}
	}
	return(sym);
}

/*
----------------------------------------------------------- get_symbol ------
	gets symbols from stdin
-----------------------------------------------------------------------------
*/

int16_t get_symbol ()
{
	/* INPUT_RING_SIZE should be a power of two, otherwise this gets even less
	   efficient... */
	#define INPUT_RING_SIZE 256

	static uint8_t ring[INPUT_RING_SIZE]; /* ring look-ahead buffer */
	static int16_t ring_size = 0;			/* # characters in the ring */
	static int16_t ring_index = 0;		/* cur ptr at offset 0 */
	static int16_t nomoredata = 0;		/* ran out of input data flag */

	int16_t bestsym; /* best symbol match from available input */
	int16_t i;


	/* special case for NO special symbols - don't do anything fancy,
	   just get the character and return. */

/* relocated special case outside loop for better efficiency.
   This is the 99% case

	if (top_symbol<256) {
        if ((i=getc(input_fp))== EOF) {
			if (!feof(input_fp)) {
				perror("tr: getc");
				errflag++;
			}
		}
		return(i);
	}
*/

	#ifdef DIAG
		printf("starting element recognition process\n");
	#endif

	/* damnation. The user has specified multi-character collating elements
		(symbols). We will have to buffer our input ourselves in a ring
		buffer. ELEMENT() is an expensive macro which makes the ring 
    	buffer look flat. */

	/* macro for flattening the ring */
	#define ELEMENT(n) ring[(ring_index+n)%INPUT_RING_SIZE]
	
	#ifdef DIAG
		printf("Filling input buffer\n");
		printf("ring_index = %d, ring_size = %d, nomoredata = %s\n",ring_index, ring_size, nomoredata?"TRUE":"FALSE");
	#endif

	/* fill the input buffer */	
	{
		int fill_index = ring_size; /* start filling from end */

		while (ring_size<INPUT_RING_SIZE && !nomoredata) {
			i = getc(input_fp);
			if (i==EOF) {
				if (!feof(input_fp)) {
					perror("tr: getc");
					errflag++;
				} else nomoredata++;
			} else {
				ELEMENT(fill_index++) = (uint8_t) i; 
				ring_size++;
			}
		}
	}

	#ifdef DIAG
		printf("ring_index = %d, ring_size = %d, nomoredata = %s\n",ring_index, ring_size, nomoredata?"TRUE":"FALSE");
	#endif

	if (!ring_size) return(EOF);	/* oops, no data available */

	#ifdef DIAG
		printf("Checking for matches amongst symbols\n");
	#endif

	/* check for matches amongst symbols */		
	{
		int16_t bestlen,sym,ci;

		bestlen = 1;	/* wost case is that we will only be able to match
						   a normal 1 byte collating element */
		bestsym = 0;

		/* doing dat linear scan thing through the symbols. This could
		   be faster. However, the user could also use TR a little bit
		   more sanely. I don't scan 1 byte symbols. Their indexes are
		   equal to their values. */

		#ifdef DIAG
			printf("bestlen = %d, bestsym = %d\n",bestlen,bestsym);
	        printf("top_symbol = %d\n",top_symbol);
		#endif

		for (sym=top_symbol;sym>255;sym--) {
			#ifdef DIAG
				printf("symbol %d = %c%c... len=%d\n",sym,symbol[sym].dat[0],symbol[sym].dat[1],symbol[sym].len);
			#endif

			/* only compare if we have enough chars in the buffer */
			if (symbol[sym].len <= ring_size) {
				for (ci=0;
					 (ci<symbol[sym].len) && 
					 (symbol[sym].dat[ci]==ELEMENT(ci) );
					 ci++);
		
				/* wasn't a full match */
				if (ci!=symbol[sym].len) continue;
		
				if (ci>bestlen) {
					bestlen = ci;
					bestsym = sym;
				}
			}
		}

		if (!bestsym) {		/* no best match found, must be single char */
			bestsym = (uint16_t) ELEMENT(0);
			/* bestlen must still be 1 */
		}

		/* adjust buffer. This means we have essentially withdrawn this
		   symbol from the buffer */

		ring_index+=bestlen;
		ring_index %= INPUT_RING_SIZE;
		ring_size-=bestlen;

		#ifdef DIAG
		printf("ring_index = %d, ring_size = %d, nomoredata = %s\n",ring_index, ring_size, nomoredata?"TRUE":"FALSE");
		if (bestlen>1) printf("FOUND SYMBOL %d, length=%d\n",bestsym,bestlen);
		#endif
	}

  	return(bestsym);
}
