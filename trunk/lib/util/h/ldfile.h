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



/*-
 * ldfile.h: extract info from qnx load modules
 */

#ifndef ldfile_h
#define ldfile_h

#ifndef __TYPES_H_INCLUDED
#include <sys/types.h>
#endif

enum ldf_typeflags {
	_SY_STATIC = 001,
	_SY_DATA   = 002,
	_SY_CODE   = 004
};

typedef
struct LdfName {
	char	*name;
	int	type;
	int	segno;
	ulong_t	offset;
	struct LdfName *next;
} LdfName;

typedef
struct LdInfo {
	ulong_t   ld_vaddr;
	ulong_t   ld_tsize;
	ulong_t   ld_dsize;
	ulong_t   ld_bsize;
	ulong_t   ld_ssize;
} LdInfo;

extern int ldf_ismagic(char *fname);
extern LdfName *ldf_nlist(char *fname);
extern int ldf_del_nlist(LdfName *nl);
extern int ldf_walk_nlist(LdfName *nl, int (*f)(LdfName *nl, void *p), void *p);
extern int ldf_seglen(char *fname, LdInfo *ldi);

#endif
