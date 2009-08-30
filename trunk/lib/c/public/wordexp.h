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
 * wordexp.h
 *

 *
 */
#ifndef	_WORDEXP_H_INCLUDED
#define	_WORDEXP_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

_C_STD_BEGIN

#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif

_C_STD_END

__BEGIN_DECLS

#define WRDE_DOOFFS	0x00000001
#define WRDE_APPEND	0x00000002
#define WRDE_NOCMD	0x00000004
#define WRDE_REUSE	0x00000008
#define WRDE_SHOWERR	0x00000010
#define WRDE_UNDEF	0x00000020
#define __WRDE_FLAGS	(WRDE_DOOFFS | WRDE_APPEND | WRDE_NOCMD | WRDE_REUSE | WRDE_SHOWERR | WRDE_UNDEF)

#define WRDE_NOSYS	-1
#define WRDE_NOSPACE	1
#define WRDE_BADCHAR	2
#define WRDE_BADVAL	3
#define WRDE_CMDSUB	4
#define WRDE_SYNTAX	5

#include <_pack64.h>

typedef struct {
	_CSTD size_t		we_wordc;
	char				**we_wordv;
	_CSTD size_t		we_offs;
}					wordexp_t;

#include <_packpop.h>

extern int wordexp(const char *__words, wordexp_t *__pwordexp, int __flags);
extern void wordfree(wordexp_t *__pwordexp);

__END_DECLS

#endif

/* __SRCVERSION("wordexp.h $Rev: 153052 $"); */
