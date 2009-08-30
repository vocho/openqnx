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




#ifndef	_fregex_h
#define	_fregex_h

#include	<limits.h>
#include	<regex.h>
#include	"bits.h"

#ifdef __cplusplus
extern "C" {
#endif
#define	FAILED	-1
#define	EQUIV(f_,c_) ((f_)->fr_equiv[(c_)])
#define	INIT_STTAB   64
#define	ST_INCR	     64
#define	DTRANS(f_,i_,j_) ((f_)->fr_dtrans[(i_)*(f_)->fr_nchars+(j_)])


/* exact match flag, defeats failure function!?! */
#define FREG_EXACT	0x1000

#if 0
struct {			/* these are for allocating [char][tx] pairs */
	int           **itab;	/* pointer table of pair lists */
	int             fr_nuse;/* number of 'full' entries */
	int             fr_ntab;/* index of next entry to use */
}               fr_alloc;
#endif

typedef struct {
	int	**fr_dtrans;
#if 0
	struct fr_xtion {
		int             fr_default;	/* default state -- for error
						 * id */
		int             fr_nentry;	/* number of state entries	 */
		int            *fr_trans;	/* transitions
						 * [char][tx][char][tx]... */
	}              *fr_dtrans;
#endif
	int             fr_maxst;	/* last state allocated */
	int             fr_nst;	/* number of states */
	int             fr_fst;	/* start state */
	int            *fr_fail;/* failure function */
	BitVect         fr_final;	/* set of 'final' states */
	int             fr_nstrings;	/* the number of strings in table */
	int            *fr_which;	/* which string was matched */
	int             fr_cflags;	/* flags compiled with */
	int		*fr_equiv;
	int		fr_nchars;
} fregex_t;



#ifdef	__STDC__

/* init feg structure */
int             freginit(fregex_t * feg, int nchars, int cflags);
/* add string to feg */
int             fregadd(fregex_t * feg, char *str);
/* complete feg compile */
int             fregdone(fregex_t * feg);
int             fregcomp(fregex_t * feg, char **pats, int cflags);
int 
fregexec(fregex_t * feg, char *string, int nmatch,
	 regmatch_t * pmatch, int eflags);
void            fregfree(fregex_t * feg);

#endif

#ifdef __cplusplus
};
#endif

#endif
