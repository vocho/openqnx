/*
 *  regex.h     Regular Expression Matching
 *
 *	Copyright 1992, 1993, 1994 Henry Spencer.  All rights reserved.
 *	This software is not subject to any license of the American Telephone
 *	and Telegraph Company or of the Regents of the University of California.
 *
 *	Permission is granted to anyone to use this software for any purpose on
 *	any computer system, and to alter it and redistribute it, subject
 *	to the following restrictions:
 *
 *	1. The author is not responsible for the consequences of use of this
 *	   software, no matter how awful, even if they arise from flaws in it.
 *
 *	2. The origin of this software must not be misrepresented, either by
 *	   explicit claim or by omission.  Since few users ever read sources,
 *	   credits must appear in the documentation.
 *
 *	3. Altered versions must be plainly marked as such, and must not be
 *	   misrepresented as being the original software.  Since few users
 *	   ever read sources, credits must appear in the documentation.
 *
 *	4. This notice may not be removed or altered.
 *
 */
 
#ifndef _REGEX_H_INCLUDED
#define _REGEX_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

_C_STD_BEGIN

#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif

_C_STD_END

/*
 * regcomp() cflags argument
 */
#define	REG_EXTENDED	0001
#define	REG_ICASE	0002
#define	REG_NOSUB	0004
#define	REG_NEWLINE	0010

#if defined(__EXT_QNX)
#define	REG_BASIC	0000
#define	REG_NOSPEC	0020
#define	REG_PEND	0040
#define	REG_DUMP	0200
#endif

/*
 * regcomp() and regexec() return values
 */
#define	REG_NOMATCH	 1
#define	REG_BADPAT	 2
#define	REG_ECOLLATE	 3
#define	REG_ECTYPE	 4
#define	REG_EESCAPE	 5
#define	REG_ESUBREG	 6
#define	REG_EBRACK	 7
#define	REG_EPAREN	 8
#define	REG_EBRACE	 9
#define	REG_BADBR	10
#define	REG_ERANGE	11
#define	REG_ESPACE	12
#define	REG_BADRPT	13

#if defined(__EXT_XOPEN_EX)
#define REG_ENOSYS	17
#endif

#if defined(__EXT_QNX)
#define REG_OK		0
#define	REG_EMPTY	14
#define	REG_ASSERT	15
#define	REG_INVARG	16
#define	REG_ATOI	255
#define	REG_ITOA	0400
#endif

/*
 * regexec() eflags argument
 */
#define	REG_NOTBOL	00001
#define	REG_NOTEOL	00002

#if defined(__EXT_QNX)
#define	REG_STARTEND	00004
#define	REG_TRACE	00400
#define	REG_LARGE	01000
#define	REG_BACKR	02000
#endif


#include <_pack64.h>

typedef _Int32t regoff_t;

typedef struct {
	int re_magic;
	_CSTD size_t re_nsub;
	__const char *re_endp;
	struct re_guts *re_g;
} regex_t;

typedef struct {
	regoff_t rm_so;
	regoff_t rm_eo;
} regmatch_t;

#include <_packpop.h>


__BEGIN_DECLS
extern  int     regcomp( regex_t *__preg, __const char *__pattern, int __cflags );
extern  int     regexec( __const regex_t *__preg, __const char *__str,
                         _CSTD size_t __nmatch, regmatch_t *__pmatch, int __eflags );
extern  _CSTD size_t  regerror( int __errcode, __const regex_t *__preg, char *__errbuf,
                            _CSTD size_t __errbuf_size);
extern  void    regfree( regex_t *__preg );
__END_DECLS

#endif

/* __SRCVERSION("regex.h $Rev: 153052 $"); */
