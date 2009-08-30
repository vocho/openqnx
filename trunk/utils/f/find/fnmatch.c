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

    fnmatch.c:	File name pattern matching.

    description:

	derived from QnxC's 'fmatch' routine, this performs a very
	limited regular expression match.  The pattern matching
	capability is equivalent to the posix shell's.
	This mechanism is relatively slow, and should not be used for
	general regular expression matching, the <regexp> library is
	far more suitable.


    stevem.

*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <util/defns.h>
#include <util/util_limits.h>
#include <ctype.h>
#include <fnmatch.h>
#include "fnmatch_local.h"

#ifdef __STDC__
 static char	*ccl_match(const char *, int );
 static int	 gmatch(const char *expr, const char *s);
 static int	 amatch(const char *expr, const char *s);
#else
 static char	*ccl_match();
 static int		 gmatch(), amatch(), fnmatch();
#endif

static int Case_Insensitive;

#define NEG_CHR     '!'
#define HIDE_CHR    '.'
#define OPT_CHR     '?'
#define CLS_CHR     '*'
#define CCL_CHR     '['
#define CCLE_CHR    ']'
#define ESC_CHR     '\\'

#ifdef __STDC__
static	char	*ccl_match(const char *expr, int c)
#else
static	char *ccl_match(expr,c)
register char *expr;
register    int c;
#endif
{
int	rc = 0;
int	lc = 0;
int	state = 0;

    if (*expr++ != CCL_CHR) {
	return NULL;		/*  uhuh. */
    }
    while (1) {
	if (*expr == ']') {
	    return( rc ? (char *)expr + 1 : NULL );
	}
	switch (state) {
	case	0:
	    if (*expr == ESC_CHR) {
		expr++;
	    }
		if (Case_Insensitive) {
		    if (toupper(*expr) == toupper(c)) {
			rc = 1;
		    }
			/* lower character if this is a range */
		    lc = (int)toupper(*expr++);
		} else {
		    if (*expr == c) {
			rc = 1;
		    }
			/* lower character if this is a range */
		    lc = (int) *expr++;
		}

	    state = 1;
	    break;
	case	1:
	    if (state = (*expr == '-') ? 2 : 0) {
		expr++;
	    }
	    break;
	case	2:	
	    if (*expr == ESC_CHR)
		expr++;

		/* comparing ranges is different for case-sensitive vs
           case-insensitive */
		if (Case_Insensitive) {
		    if (lc <= toupper(c) && toupper(c) <= toupper(*expr)) {
			rc = 1;
		    }
		} else {
		    if (lc <= c && c <= *expr) {
			rc = 1;
		    }
		}
	    expr++;	
	    state = 0;
	    break;
	default:
#ifdef DEBUGGING
	    fprintf(stderr,"ccl_match: state failed %d\n",state);
#else
/*  
    yes, i know, this is analogous to removing your parachute when
    the plane leaves the ground, but I don't want this module 
    dragging in any extra libs.  Right now, the only function it 
    invokes is 'strrchr'.
*/
	    return NULL;    
#endif
	}
    }
}
	

#ifdef __STDC__
static int amatch(const char *expr, const char *s)
#else
static int amatch(expr,s) 
register    char *s, *expr;
#endif
{
    while( *expr && *s ) {
	switch( *expr ) {
	case OPT_CHR:
	    expr++; s++;
	    break;
	case CCL_CHR:
	    if ((expr = ccl_match(expr,*s)) == NULL) {
		return 0;
	    }	
	    s++;
	    break;
	case CLS_CHR:
	    return gmatch(expr,s);
	case ESC_CHR:
	    if (!*++expr)	/* Fall through to next clause	*/
		break;
	default:
		if (Case_Insensitive) {
		    if (toupper(*expr) != toupper(*s))
			return 0 ;
		    expr++;
		    s++;
		} else {
		    if (*expr != *s)
			return 0 ;
		    expr++;
		    s++;
		}
	}
    }
    return (*expr == 0 || (*expr == CLS_CHR && *(expr+1) == 0)) && *s == 0;
}


#ifdef __STDC__
static int gmatch(const char *expr, const char *s)
#else
static	gmatch(expr,s)
register char *expr, *s;
#endif
{
    if (*expr == CLS_CHR) {
	if (*++expr == '\0') return 1;

	while (*s)
	    if (amatch(expr,s++)) return 1;
	return 0; 
    }
    return amatch(expr,s);
}


#ifdef __STDC__
int fnmatch(const char *pattern, const char *string, int flag)
#else
int fnmatch(pattern,string,flag)
register    char    *pattern;
register    char    *string;
int flag;
#endif
{
int	negate_flag = 0; 

	Case_Insensitive = FALSE;

    if (flag == FNM_PATHNAME) {
	const char *t;
	if ((t=strrchr(string,'/')) != NULL)
	    string = t+1;
    } else if (*pattern == '.') {
	if (*string != '.')
	    return !0;
	string++; pattern++;
    } else if (negate_flag = (*pattern == NEG_CHR)) {
	pattern++;
    }
    return negate_flag == (gmatch(pattern,string) != 0);    
}

#ifdef __STDC__
int ifnmatch(const char *pattern, const char *string, int flag)
#else
int ifnmatch(pattern,string,flag)
register    char    *pattern;
register    char    *string;
int flag;
#endif
{
int	negate_flag = 0; 

	Case_Insensitive = TRUE;

    if (flag == FNM_PATHNAME) {
	const char *t;
	if ((t=strrchr(string,'/')) != NULL)
	    string = t+1;
    } else if (*pattern == '.') {
	if (*string != '.')
	    return !0;
	string++; pattern++;
    } else if (negate_flag = (*pattern == NEG_CHR)) {
	pattern++;
    }
    return negate_flag == (gmatch(pattern,string) != 0);    
}



#ifdef DEBUGGING

#ifdef __STDC__
main(int argc, char **argv)
#else
main(argc,argv)
int argc;
char **argv;
#endif
{
char	buffer[100];

    if (argc < 2) {
	fprintf(stderr,"usage: fnmatch pattern < string_list\n");
	exit(-1);
    }
    while (gets(buffer)) {
	if (fnmatch(argv[1],buffer,argc > 2))
	    printf("%s fnmatches %s\n",buffer,argv[1]);
    }
}


#endif
