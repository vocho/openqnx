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
 *  regex.h     Regular expression handler structures
 *

 */
#ifndef _EREGEX_H_INCLUDED
#define	_EREGEX_H_INCLUDED

#include <sys/types.h>
#include <limits.h>

#include <regex.h>


#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
        short        opcode;            
        short        x1;                
        short        x2;                
/*    filler here, handy for indexing (4 shorts vs 3 shorts) */
        short        fill1;
    } __mach_t;

typedef struct {
        short        first;
        short        final;
        short        last;
        short        fill2;
    } __mach_opt;

typedef struct    {
        size_t          re_nsub;        /* number of sub-expressions */
        short           re_start;       /* start state */
        short           re_flags;       /* compilation flags */
        short           re_bol   : 1,   /* match beginning of line */
                        re_eol   : 1,   /* match end of line */
                        re_empty : 1;   /* match empty string */
        short           re_max_st;      /* maximum number of states avail */
        short           re_last_st;     /* highest number nfa in use */
        short           re_ewidth;      /* width of the ecl table */
        short           re_accnum;      /* number of accepting states */
        __mach_t        *re_mach;       /* machine itself */
        __mach_opt      *re_opt;        /* extra info, for building dfa */
        unsigned char   *re_accno;      /* set of accepting states */
        unsigned char   *re_eclos;      /* epsilon closure of re_mach */
        unsigned char   *re_stch;       /* start chars -- for quick scan */
        unsigned char   **re_csets;     /* char sets, referenced by re_mach */
        short           re_max_cset;    /* maximum number of char sets */
        short           re_last_cset;   /* last cset used */
        short           *re_sub_st;     /* table of sub pattern "starts" */
        short           *re_sub_end;    /* table of sub pattern "ends" */
} eregex_t;


#ifdef __STDC__

extern  int     eregcomp( eregex_t *__preg, const char *__pattern, int __cflags );
extern  int     eregexec( const eregex_t *__preg, const char *__str,
                         size_t __nmatch, regmatch_t *__pmatch, int __eflags );
extern  void    eregfree( eregex_t *__preg );
							
#else

extern  int     eregcomp();
extern  int     eregexec();
extern  void    eregfree();

#endif

#ifdef __cplusplus
};
#endif
#endif
