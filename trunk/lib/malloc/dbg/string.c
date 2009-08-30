/*
 * $QNXtpLicenseC:
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
 * (c) Copyright 1990, 1991 Conor P. Cahill (uunet!virtech!cpcahil).  
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */

/*
 * NOTE: if you have problems compiling this file, the first thing to try is
 * to take out the include of string.h.  This is due to the fact that some
 * systems (like ultrix) have conflicting definitions and some (like aix)
 * even set up some of these functions to be in-lined.
 */

#ifdef __QNXNTO__
#define _STRINGS_H_INCLUDED
#endif
#include <stdio.h>
#if ! defined(_IBMR2) && ! defined(ultrix)
#include <string.h>
#endif
#include <sys/types.h>

#include "malloc-lib.h"
#include "mallocint.h"


/*
 * strcat - concatenate a string onto the end of another string
 */
char *
strcat(char *str1, const char *str2)
{
	int line = libmalloc_caller();
	return( DBstrcat((char *)NULL,line,str1,str2) );
}

char *
DBstrcat(const char *file, int line, register char *str1, register const char *str2)
{
	char			* rtn;
	int			  len;

	/* 
	 * check pointers agains malloc region.  The malloc* functions
	 * will properly handle the case where a pointer does not
	 * point into malloc space.
	 */
	if (str1 == NULL || str2 == NULL) {
		malloc_errno = M_CODE_NPD;
		malloc_warning("strcat",file,line,NULL);
	}

	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		set_in_string(1);

		len = strlen(str2);
		malloc_check_str("strcat", file, line, str2);

		len += strlen(str1) + 1;
		set_in_string(0);

		malloc_check_data("strcat", file, line, str1, len);
		LEAVE();
	}

	rtn = str1;

	while( *str1 ) {
		str1++;
	}
	
	while( (*str1 = *str2) != '\0' ) {
		str1++;
		str2++;
	}
	
	return(rtn);
}

/*
 * strdup - duplicate a string
 */
char *
strdup(const char *str1)
{
	int line = libmalloc_caller();
	return( DBstrdup((char *)NULL, line, str1) );
}

char *
DBstrdup(const char *file, int line, register const char *str1)
{
	char			* rtn;
	register char		* str2;

	if (str1 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strdup",file,line,NULL);
	}
	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_str("strdup", file, line, str1);
		LEAVE();
	}

	ENTER();
	set_in_string(1);
	rtn = str2 = malloc((size_t)strlen(str1)+1);
	set_in_string(0);
	LEAVE();

	if( rtn != (char *) 0) {
		while( (*str2 = *str1) != '\0' ) {
			str1++;
			str2++;
		}
	}

	return(rtn);
}

/*
 * strncat - concatenate a string onto the end of another up to a specified len
 */
char *
strncat(char *str1, const char *str2, size_t len)
{
	int line = libmalloc_caller();
	return( DBstrncat((char *)NULL, line, str1, str2, len) );
}

char *
DBstrncat(const char *file, int line,
	register char *str1, register const char *str2, register size_t len)
{
	int 		  len1;
	int 		  len2;
	char		* rtn;

	if ((len == 0) && (!malloc_verify_access_level)) {
		return(str1);
	}
	if (str1 == NULL || str2 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strncat",file,line,NULL);
	}
	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_strn("strncat", file, line, str2, len);

		set_in_string(1);

		len2 = strlen(str2) + 1;
		len1 = strlen(str1);

		set_in_string(0);

		if( (len+1) < len2 ) {
			len1 += len + 1;
		} else {
			len1 += len2;
		}
		malloc_check_data("strncat", file, line, str1, len1);
		LEAVE();
	}

	rtn = str1;

	while( *str1 ) {
		str1++;
	}

	while( len && ((*str1++ = *str2++) != '\0') ) {
		len--;
	}
	
	if( ! len ) {
		*str1 = '\0';
	}

	return(rtn);
}

/*
 * strcmp - compare two strings
 */
int
strcmp(register const char *str1, register const char *str2)
{
	int line = libmalloc_caller();
	return( DBstrcmp((char *) NULL, line, str1, str2) );
}

int
DBstrcmp(const char *file, int line, register const char *str1, register const char *str2)
{
	if (str1 == NULL || str2 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strcmp",file,line,NULL);
	}
	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_str("strcmp", file, line, str1);
		malloc_check_str("strcmp", file, line, str2);
		LEAVE();
	}

	while( *str1 && (*str1 == *str2) ) {
		str1++;
		str2++;
	}


	/*
	 * in order to deal with the case of a negative last char of either
	 * string when the other string has a null
	 */
	if( (*str2 == '\0') && (*str1 == '\0') ) {
		return(0);
	} else if( *str2 == '\0' ) {
		return(1);
	} else if( *str1 == '\0' ) {
		return(-1);
	}
	
	return( (int)((unsigned char)*str1 - (unsigned char)*str2) );
}

/*
 * strncmp - compare two strings up to a specified length
 */
int
strncmp(register const char *str1, register const char *str2, register size_t len)
{
	int line = libmalloc_caller();
	return( DBstrncmp((char *)NULL, line, str1, str2, len) );
}

int
DBstrncmp(const char *file, int line,register const char *str1, 
	register const char *str2, register size_t len)
{
	if ((len == 0) && (!malloc_verify_access_level)) {
		return(0);
	}
	if (str1 == NULL || str2 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strncmp",file,line,NULL);
	}
	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_strn("strncmp", file, line, str1, len);
		malloc_check_strn("strncmp", file, line, str2, len);
		LEAVE();
	}

	while( len > 0 && *str1 && (*str1 == *str2) ) {
		len--;
		str1++;
		str2++;
	}

	if( len == 0 ) {
		return(0);
	}
	/*
	 * in order to deal with the case of a negative last char of either
	 * string when the other string has a null
	 */
	if( (*str2 == '\0') && (*str1 == '\0') ) {
		return(0);
	} else if( *str2 == '\0' ) {
		return(1);
	} else if( *str1 == '\0' ) {
		return(-1);
	}
	
	return( (int)((unsigned char)*str1 - (unsigned char)*str2) );
}

/*
 * strcpy - copy a string somewhere else
 */
char *
strcpy(register char *str1, register const char *str2)
{
	int line = libmalloc_caller();
	return( DBstrcpy((char *)NULL, line, str1, str2) );
}

char *
DBstrcpy(const char *file, int line, register char *str1, register const char *str2)
{
	char		* rtn;
	int		  len;

	if (str1 == NULL || str2 == NULL) {
		malloc_errno = M_CODE_NPD;
		malloc_warning("strcpy",file,line,NULL);
	}

	ENTER();
	set_in_string(1);
	len = strlen(str2) + 1;
	set_in_string(0);
	LEAVE();

	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_data("strcpy", file, line, str1, len);
		malloc_check_data("strcpy", file, line, str2, len);
		LEAVE();
	}

	rtn = str1;

	while( (*str1++ = *str2++) != '\0') {
	}

	return(rtn);
}

/*
 * strncpy - copy a string upto a specified number of chars somewhere else
 */
char *
strncpy(register char *str1, register const char *str2, register size_t len)
{
	int line = libmalloc_caller();
	return( DBstrncpy((char *)NULL, line, str1, str2, len) );
}

char *
DBstrncpy(const char *file, int line,
	register char *str1, register const char *str2, register size_t len)
{
	char		* rtn;

	if ((len == 0) && (!malloc_verify_access_level)) {
		return(str1);
	}
	if (str1 == NULL || str2 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strncpy",file,line,NULL);
	}

	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_data("strncpy", file, line, str1, len);
		malloc_check_strn("strncpy", file, line, str2, len);
		LEAVE();
	}

	rtn = str1;

	while((len > 0) && (*str1 = *str2) != '\0') {
		str1++;
		str2++;
		len--;
	}
	while( (len > 0) ) {
		*str1++ = '\0';
		len--;
	}

	return(rtn);
}

/*
 * strlen - determine length of a string
 */
size_t
strlen(const char *str1)
{
	int line = libmalloc_caller();
	return( DBstrlen((char *) NULL, line, str1) );
}

size_t
DBstrlen(const char *file, int line, register const char *str1)
{
	register const char	* s;

	if (str1 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strlen",file,line,NULL);
	}

	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_str("strlen", file, line, str1);
		LEAVE();
	}

	for( s = str1; *s; s++) {
	}

	return( s - str1 );
}

/*
 * strchr - find location of a character in a string
 */
char *
strchr(const char *str1, int c)
{
	int line = libmalloc_caller();
	return( DBstrchr((char *)NULL,line,str1,c) );
}

char *
DBstrchr(const char *file, int line, const char *str1, int c)
{
	return( DBFstrchr("strchr",file,line,str1,c) );
}

char *
DBFstrchr(const char *func, const char *file, int line,
	register const char *str1, register int c)
{
	if (str1 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strchr",file,line,NULL);
	}

	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_str(func, file, line, str1);
		LEAVE();
	}

	while( *str1 && (*str1 != (char) c) ) {
		str1++;
	}

	if(*str1 != (char) c) {
		str1 = (char *) 0;
	}

	return((char *)str1);
}

/*
 * strrchr - find rightmost location of a character in a string
 */

char *
strrchr(const char *str1, int c)
{
	int line = libmalloc_caller();
	return( DBstrrchr( (char *)NULL, line, str1, c) );
}

char *
DBstrrchr(const char *file, int line, const char *str1, int c)
{
	return( DBFstrrchr("strrchr",file,line,str1,c) );
}

char *
DBFstrrchr(const char *func, const char *file, int line,
	register const char *str1, register int c)
{
	register const char	* rtn = (char *) 0;

	if (str1 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strrchr",file,line,NULL);
	}

	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_str(func, file, line, str1);
		LEAVE();
	}

	while( *str1 ) {
		if(*str1 == (char) c ) {
			rtn = str1;
		}
		str1++;
	}

	if( *str1 == (char) c) {
		rtn = str1;
	}

	return((char *)rtn);
}

/*
 * index - find location of character within string
 */
char *
index(const char *str1, int c)
{
	int line = libmalloc_caller();
	return( DBindex((char *) NULL, line, str1, c) );
}
char *
DBindex(const char *file, int line, const char *str1, int c)
{
	return( DBFstrchr("index",file,line,str1,c) );
}

/*
 * rindex - find rightmost location of character within string
 */
char *
rindex(const char *str1, int c)
{
	int line = libmalloc_caller();
	return( DBrindex((char *)NULL, line, str1, c) );
}

char *
DBrindex(const char *file, int line, const char *str1, int c)
{
	return( DBFstrrchr("rindex",file,line,str1,c) );
}

/*
 * strpbrk - find the first occurance of any character from str2 in str1
 */
char *
strpbrk(const char *str1, const char *str2)
{
	int line = libmalloc_caller();
	return( DBstrpbrk((char *)NULL, line, str1, str2) );
}

char *
DBstrpbrk(const char *file, int line,
	register const char *str1, register const char *str2)
{
	register const char	* tmp;

	if (str1 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strpbrk",file,line,NULL);
	}

	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_str("strpbrk", file, line, str1);
		malloc_check_str("strpbrk", file, line, str2);
		LEAVE();
	}

	while(*str1) {
		for( tmp=str2; *tmp && *tmp != *str1; tmp++) {
		}
		if( *tmp ) {
			break;
		}
		str1++;
	}

	if( ! *str1 ) {
		str1 = (char *) 0;
	}

	return( (char *) str1);
}

/*
 * strspn - get length of str1 that consists totally of chars from str2
 */
size_t
strspn(const char *str1, const char *str2)
{
	int line = libmalloc_caller();
	return( DBstrspn((char *)NULL, line, str1, str2) );
}

size_t
DBstrspn(const char *file, int line,
	register const char *str1, register const char *str2)
{
	register const char	* tmp;
	const char		* orig = str1;

	if (str1 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strspn",file,line,NULL);
	}

	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_str("strspn", file, line, str1);
		malloc_check_str("strspn", file, line, str2);
		LEAVE();
	}

	while(*str1) {
		for( tmp=str2; *tmp && *tmp != *str1; tmp++) {
		}
		if(! *tmp ) {
			break;
		}
		str1++;
	}

	return( (size_t) (str1 - orig) );
}

/*
 * strcspn - get lenght of str1 that consists of no chars from str2
 */
size_t
strcspn(const char *str1, const char *str2)
{
	int line = libmalloc_caller();
	return( DBstrcspn((char *)NULL,line,str1,str2) );
}

size_t
DBstrcspn(const char *file, int line,
	register const char *str1, register const char *str2)
{
	register const char	* tmp;
	const char	* orig = str1;

	if (str1 == NULL || str2 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strcspn",file,line,NULL);
	}

	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_str("strcspn", file, line, str1);
		malloc_check_str("strcspn", file, line, str2);
		LEAVE();
	}

	while(*str1) {
		for( tmp=str2; *tmp && *tmp != *str1; tmp++) {
		}
		if( *tmp ) {
			break;
		}
		str1++;
	}

	return( (int) (str1 - orig) );
}

/*
 * strstr - locate string within another string
 */
char *
strstr(const char *str1, const char *str2)
{
	int line = libmalloc_caller();
	return( DBstrstr((char *)NULL, line, str1, str2) );
}

char *
DBstrstr(const char *file, int line, const char *str1, const char *str2)
{
	register const char	* s;
	register const char	* t;

	if (str1 == NULL || str2 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strstr",file,line,NULL);
	}
	
	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		malloc_check_str("strstr", file, line, str1);
		malloc_check_str("strstr", file, line, str2);
		LEAVE();
	}

	/*
	 * until we run out of room in str1
	 */
	while( *str1 != '\0' ) {
		/*
		 * get tmp pointers to both strings
		 */
		s = str2;
		t = str1;

		/*
		 * see if they match
		 */
		while( *s &&  (*s == *t) ) {
			s++;
			t++;
		}

		/*
		 * if we ran out of str2, we found the match,
		 * so return the pointer within str1.
		 */
		if( ! *s ) {
			return( (char *) str1);
		}
		str1++;
	}

	if( *str2 == '\0' ) {
		return( (char *) str1);
	}
	return(NULL);
}

/*
 * strtok() source taken from that posted to comp.lang.c by Chris Torek
 * in Jan 1990.
 */

/*
 * Copyright (c) 1989 The Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

/*
 * Get next token from string s (NULL on 2nd, 3rd, etc. calls),
 * where tokens are nonempty strings separated by runs of
 * chars from delim.  Writes NULs into s to end tokens.  delim need not
 * remain constant from call to call.
 *
 * Modified by cpc: 	changed variable names to conform with naming
 *			conventions used in rest of code.  Added malloc pointer
 *			check calls.
 */

char *
strtok(char *str1, const char *str2)
{
	int line = libmalloc_caller();
	return( DBstrtok( (char *)NULL, line, str1, str2) );
}

char *
DBstrtok(const char *file, int line, char *str1, const char *str2)
{
	static char 	* last;
	char		* strtoken();

	if (str2 == NULL) {
		malloc_errno = M_CODE_NPD;
	    malloc_warning("strtok",file,line,NULL);
	}

	if( !in_string() && !in_malloc() && malloc_verify_access ) {
		ENTER();
		if( str1 ) {
			malloc_check_str("strtok", file, line, str1);
			last = str1;
		}
		malloc_check_str("strtok", file, line, str2);
		LEAVE();
	}

	if(str1) {
		last = str1;
	}
	return (strtoken(&last, str2, 1));
}


/*
 * Get next token from string *stringp, where tokens are (possibly empty)
 * strings separated by characters from delim.  Tokens are separated
 * by exactly one delimiter iff the skip parameter is false; otherwise
 * they are separated by runs of characters from delim, because we
 * skip over any initial `delim' characters.
 *
 * Writes NULs into the string at *stringp to end tokens.
 * delim will usually, but need not, remain constant from call to call.
 * On return, *stringp points past the last NUL written (if there might
 * be further tokens), or is NULL (if there are definitely no more tokens).
 *
 * If *stringp is NULL, strtoken returns NULL.
 */
char *
strtoken(register char **stringp, register const char *delim, int skip)
{
	register char *s;
	register const char *spanp;
	register int c, sc;
	char *tok;

	if ((s = *stringp) == NULL)
		return (NULL);

	if (skip) {
		/*
		 * Skip (span) leading delimiters (s += strspn(s, delim)).
		 */
	cont:
		c = *s;
		for (spanp = delim; (sc = *spanp++) != 0;) {
			if (c == sc) {
				s++;
				goto cont;
			}
		}
		if (c == 0) {		/* no token found */
			*stringp = NULL;
			return (NULL);
		}
	}

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (tok = s;;) {
		c = *s++;
		spanp = delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*stringp = s;
				return( (char *) tok );
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

/*
 * $Log$
 * Revision 1.12  2006/12/19 20:23:23  elaskavaia
 * - fixed problem with random errors message for npd problem
 * PR: 43567
 * CI: alain@qnx.com
 * CI: cburgess@qnx.com
 *
 * Revision 1.11  2006/09/28 19:05:57  alain
 * PR:41782
 * CI: shiv@qnx.com
 * CI: cburgess@qnx.com
 *
 * Commiting the work done for IGT-CE on the head.
 *
 * Revision 1.10.2.1  2006/04/17 15:07:27  alain
 *
 * Fix the indentation so it is consistent to the rest of the code.
 *
 * Revision 1.10  2005/06/03 01:22:48  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.9  2005/03/29 18:22:44  shiv
 * PR24134
 * PR24010
 * PR24008
 * PR24184
 * The malloc lib used to report errors when mem* and str* functions
 * were called (for those that take a length parameter) with
 * a length of zero, if the other arguments are not valid..
 * In general these would not cause errors, since
 * no actual data is moved. But since the errors being reported could
 * be useful, the option to increase the level of verbosity for this
 * has been provided. the environment variable
 * MALLOC_CKACCESS_LEVEL can be used or the mallopt call
 * with the option mallopt(MALLOC_CKACCESS_LEVEL, arg)
 * can be used. By default the level is zero, a non-zero
 * level will turn on strict checking and reporting of errors
 * if the arguments are not valid.
 * Also fixed PR24184 which had an incorrect function name
 * being passed in (strchr instead of strrchr... thanx kirk)
 * Modified Files:
 * 	mallocint.h dbg/m_init.c dbg/malloc_chk.c dbg/malloc_g.c
 * 	dbg/mallopt.c dbg/memory.c dbg/string.c
 * 	public/malloc_g/malloc-lib.h
 *
 * Revision 1.8  2005/02/13 23:15:40  shiv
 * some more cleanup.
 *
 * Revision 1.7  2005/01/16 20:38:45  shiv
 * Latest DBG malloc code. Lots of cleanup/optimistions
 * Modified Files:
 * 	common.mk mallocint.h common/tostring.c dbg/analyze.c
 * 	dbg/calloc.c dbg/dump.c dbg/free.c dbg/m_init.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc_g/malloc-lib.h public/malloc_g/malloc.h
 * 	std/calloc.c std/free.c std/m_init.c std/malloc_wrapper.c
 * 	std/mtrace.c std/realloc.c
 * Added Files:
 * 	dbg/dl_alloc.c dbg/malloc_control.c dbg/malloc_debug.c
 * 	dbg/new.cc public/malloc_g/malloc-control.h
 * 	public/malloc_g/malloc-debug.h
 *
 * Revision 1.6  2004/02/12 15:43:17  shiv
 * Updated copyright/licenses
 * Modified Files:
 * 	common.mk debug.h malloc-lib.h mallocint.h tostring.h
 * 	common/tostring.c dbg/analyze.c dbg/calloc.c dbg/context.h
 * 	dbg/crc.c dbg/dump.c dbg/free.c dbg/m_init.c dbg/m_perror.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc/malloc.h public/malloc_g/malloc-lib.h
 * 	public/malloc_g/malloc.h public/malloc_g/prototypes.h
 * 	std/calloc.c std/context.h std/free.c std/m_init.c
 * 	std/malloc_wrapper.c std/mtrace.c std/realloc.c test/memtest.c
 * 	test/mtrace.c
 *
 * Revision 1.5  2003/09/25 19:06:49  shiv
 * Fixed several things in the malloc code including the
 * leak detection for both small and large blocks. re-arranged
 * a lot code, and removed pieces that were not necessary after the
 * re-org. Modified the way in which the elf sections are read to
 * determine where heap references could possibly be stored.
 * set the optimisation for the debug variant at -O0, just so
 * so that debugging the lib itself is a little easier.
 * Modified Files:
 * 	common.mk mallocint.h dbg/dump.c dbg/malloc_chk.c
 * 	dbg/malloc_chn.c dbg/malloc_g.c dbg/malloc_gc.c dbg/mtrace.c
 * 	dbg/process.c dbg/string.c
 *
 * Revision 1.4  2002/01/19 00:32:22  sebastien
 * Fixed crash in strtok
 * Fix bad warning when passing NULL to strtok -- that's quite OK.
 *
 * Revision 1.3  2001/02/05 22:07:12  furr
 * Minor fix to mtrace code (correctly print symbol offsets)
 * Warn about NULL pointers to str functions
 * Add stack tracebacks to warnings.
 * Added missing context file.
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	dbg/m_init.c dbg/malloc_g.c dbg/malloc_gc.c dbg/mtrace.c
 *  	dbg/string.c dbg/tostring.c public/malloc_g/malloc.h
 *  	public/malloc_g/prototypes.h
 *  Added Files:
 *  	dbg/context.h
 *
 * Revision 1.2  2000/02/08 19:16:23  furr
 * Fix up guard code problems re. underlying implementation
 * Fixed up problems for Java such as locking on mem functions
 *
 *  Modified Files:
 *  	dbg/free.c dbg/malloc_chk.c dbg/memory.c dbg/realloc.c
 *  	dbg/string.c
 *
 * Revision 1.1  2000/01/31 19:03:31  bstecher
 * Create libmalloc.so and libmalloc_g.so libraries for everything. See
 * Steve Furr for details.
 *
 * Revision 1.1  2000/01/28 22:32:44  furr
 * libmalloc_g allows consistency checks and bounds checking of heap
 * blocks allocated using malloc.
 * Initial revision
 *
 *  Added Files:
 *  	Makefile analyze.c calloc_g.c crc.c dump.c free.c m_init.c
 *  	m_perror.c malloc-config.c malloc_chk.c malloc_chn.c
 *  	malloc_g.c malloc_gc.c mallopt.c memory.c process.c realloc.c
 *  	string.c tostring.c inc/debug.h inc/mallocint.h inc/tostring.h
 *  	inc/malloc_g/malloc inc/malloc_g/malloc.h
 *  	inc/malloc_g/prototypes.h test/memtest.C test/memtest.c
 *  	x86/Makefile x86/so/Makefile
 *
 * Revision 1.2  1996/08/18 21:01:33  furr
 * print the caller return address on errors
 *
 * Revision 1.1  1996/07/24 18:06:30  furr
 * Initial revision
 *
 * Revision 1.14  1991/12/04  09:23:44  cpcahil
 * several performance enhancements including addition of free list
 *
 * Revision 1.13  91/12/02  19:10:14  cpcahil
 * changes for patch release 5
 * 
 * Revision 1.12  91/11/25  14:42:05  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.11  91/11/24  16:56:43  cpcahil
 * porting changes for patch level 4
 * 
 * Revision 1.10  91/11/24  00:49:32  cpcahil
 * first cut at patch 4
 * 
 * Revision 1.9  91/11/20  11:54:11  cpcahil
 * interim checkin
 * 
 * Revision 1.8  91/05/30  12:07:04  cpcahil
 * added fix to get the stuff to compile correctly on ultirx and aix systems.
 * 
 * Revision 1.7  90/08/29  22:24:19  cpcahil
 * added new function to check on strings up to a specified length 
 * and used it within several strn* functions.
 * 
 * Revision 1.6  90/07/16  20:06:56  cpcahil
 * fixed several minor bugs found with Henry Spencer's string/mem function
 * tester program.
 * 
 * Revision 1.5  90/06/10  14:59:49  cpcahil
 * Fixed a couple of bugs in strncpy & strdup
 * 
 * Revision 1.4  90/05/11  00:13:10  cpcahil
 * added copyright statment
 * 
 * Revision 1.3  90/02/24  21:50:32  cpcahil
 * lots of lint fixes
 * 
 * Revision 1.2  90/02/24  17:29:40  cpcahil
 * changed $Header to $Id so full path wouldnt be included as part of rcs 
 * id string
 * 
 * Revision 1.1  90/02/22  23:17:44  cpcahil
 * Initial revision
 * 
 */
