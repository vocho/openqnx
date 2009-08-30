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
 * latest member of the regex() family, a boyer-moore derived string searcher
 */
 
#ifndef _bmex_h_included
#define _bmex_h_included

#ifndef _REGEX_H_INCLUDED
#include <regex.h>
#endif

#ifndef _LIMITS_H_INCLUDED
#include <limits.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
typedef struct bmtab {
     int len;               /* length of the search string */
     int flags;             /* compiled in flags */
     char *s;               /* copy of the string */
     int dist[UCHAR_MAX+1]; /* distances of each char to begging of string */
} bmex_t;
/*
 * 'compile' a pattern into a bmex_t
 */
int bmcomp(bmex_t *b, const char *s, int bmflags);
/*
 * compare a string against a bmex_t
 */
int bmexec(const bmex_t *b, const char *str, int nmatch,
           regmatch_t *nm, int eflags);
/*
 * release resources alloc'd for bmex_t
 */
int bmfree(bmex_t *bm);

#ifdef __cplusplus
};
#endif
#endif

