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



/*
	stresc:	extract next character from a string, translating appropriate
			escape sequences.

	int stresc(char *string, char **endptr)
		stresc starts returns the next character from 'string', updating
		endptr to point at next character to extract.  If the character
		starts with the escape character '\', then the escape sequence is
		extracted and the resultant value is returned.   If the next character
		is end-of-string, the value EOF is returned.  This distinguishes an
		actual end-of-string condition from an interpretation of \x00 or
		\000 ...
		by default the following escape sequences are recognized:
			'\a', '\b', '\f', '\n', '\r', '\t', '\v', '\\', '\"'.
		as well as
			'\[0-7]{1,3}'
		and
			'\x[0-9a-bA-B]{1,2}'
		notice that, in keeping with ansi xj11, 8 & 9 are NOT octal digits.
		The first set of strings may be exchanged using 'set_escape_string'
		function.  If they are set to NULL, no escape character mapping will
		be attempted (although the octal & hex mapping are maintained).
		The above string is scanned first, before looking for octal or hex	
		strings, so putting [xX0-9] in the key string should be used with
		caution, as it will superseed scanning for the numeric sequences.


	set_escape_string(const char *match_string, const char *esc_string)
		set_escape_string controls the escape sequence matching.  The first
		string, match_string, specifies the characters which follow the
		'\' character.  esc_string contains the value to return for it.
		The strings are joined by matching indices.


	Steve McPolin

*/



#include	<stdio.h>
#include	<ctype.h>

#include	<stdlib.h>
#include	<string.h>
#include <util/stdutil.h>


#define	DEFAULT_MATCH_STRING	"abfnrtv\\\'\""
#define	DEFAULT_ESC_STRING		"\a\b\f\n\r\t\v\\\'\""



static	char	*esc_key = DEFAULT_MATCH_STRING,
				*esc_rpl = DEFAULT_ESC_STRING;


#ifdef	__STDC__
int	set_escape_string(char *match, char *xlat)
#else
int	set_escape_string(match, xlat)
char	*match;
char	*xlat;
#endif
{
	esc_key = match;
	esc_rpl = xlat;
	return 1;
}



#define	add_hex(_lval,_h)	(((_lval) << 4) + (_h))
#define	add_oct(_lval,_o)	(((_lval) << 3) + (_o))

#ifdef	__STDC__
static	int	rplexc(int	c)
#else
static	int	rplexc(c)
int	c;
#endif
{

char	*p;

	if (esc_key == NULL)
		return -1;
	for (p=esc_key; *p && *p != c; p++)
		;
	if (!*p)
		return -1;
	return esc_rpl[p-esc_key];
}
	

#ifdef	__STDC__
int stresc(char *s, char **update)
#else
int	stresc(s,update)
char	*s;
char	**update;
#endif
{
int		x;
int		i = 0;	
int		c;
int		ival;

	/*	end of string */
    if (!(c=*s++)) {
		if (update)	*update = s-1;
		return EOF;
	}
	/*	normal character, no escape. */
	if (c != '\\') {
		if (update)	
			*update = s;
		return c;
	}
	if ((ival=rplexc(c=*s++)) > 0) {
		if (update)		
			*update = s;
		return ival;
	}

	if (toupper(c) == 'X') {		/*	try for hex const */
		x =c;
		ival = 0;
		for (i=0; i < 2 && isxdigit(c=s[i]); i++)
			ival = add_hex(ival,c <= '9' ? c-'0' : toupper(c)-'A'+10);
		if (update)	*update = s+i;	
		return i == 0 ? x : ival;		
	}
	if (isdigit(c) && c < '8'){
		s--;
		ival = 0;
		for (i=0; i < 3 && isdigit(c=s[i]) && c < '9'; i++) {
			ival = add_oct(ival,c-'0');
		}
		if (update) *update = s+i;
		return ival;
	}
	/*	just an overly cautious person ;-) */
	if (update)	*update = s;
	return c;
}


