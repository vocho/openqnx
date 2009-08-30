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





/*-
 * Copyright (c) 1982, 1986, 1992, 1993
 *      The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by the University of
 *      California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      @(#)gmon.h      8.2 (Berkeley) 1/4/94
 * $Id: gmon.h 166179 2008-04-11 14:54:30Z cburgess $
 */

#ifndef _SYS_GMON_H_
#define _SYS_GMON_H_

#include <sys/types.h>
#ifdef __GNUC__
#	if __GNUC__ < 3
#		define MCOUNT_ATTR __attribute__((unused))
#	else
#		define MCOUNT_ATTR __attribute__((unused,used))
#	endif
#endif
#include "profile.h"

/*
 * Structure prepended to gmon.out profiling data file.
 */
struct gmonhdr {
        u_long  lpc;            /* base pc address of sample buffer */
        u_long  hpc;            /* max pc address of sampled buffer */
        int     ncnt;           /* size of sample buffer (plus this header) */
        int     version;        /* version number */
        int     profrate;       /* profiling clock rate */
        int     spare[3];       /* reserved */
        /* XXX should record counter size and density */
};
#define GMONVERSION     0x00051879
#define GMON_MAGIC "gmon"
#define GMON_VERSION 1

/*
 * New gmon header that includes identifying magic cookie
 */

struct gmon_hdr {
    char cookie[4];	/* Magic cookie */
    char version[4];	/* Version number */
    char spare[3 * 4];
};

/*
 * Record types for the gmon information
 */

typedef enum {
	GMON_TAG_TIME_HIST = 0,
	GMON_TAG_CG_ARC = 1,
	GMON_TAG_BB_COUNT = 2
} __gmon_tags;

struct gmon_hist_hdr {
    u_long low_pc;       /* base pc address of sample buffer */
    u_long high_pc;      /* max pc address of sampled buffer */
    u_long hist_size;                  /* size of sample buffer */
    u_long prof_rate;                  /* profiling clock rate */
    char dimen[15];                     /* phys. dim., usually "seconds" */
    char dimen_abbrev;                  /* usually 's' for "seconds" */
};

struct gmon_cg_arc_record {
    u_long from_pc;      /* address within caller's body */
    u_long self_pc;      /* address within callee's body */
    u_long count;                      /* number of arc traversals */
};

struct __bb {
	unsigned	zero_word;
	const char	*filename;
	long		*counts;
	long		ncounts;
	struct __bb	*next;
	const unsigned long	*addresses;
};

/*
 * Type of histogram counters used in the kernel.
 */
#ifdef GPROF4
#define HISTCOUNTER     int64_t
#else
#define HISTCOUNTER     unsigned short
#endif

int profil(char *buffer, size_t size, unsigned start, unsigned histfraction);

/*
 * Fraction of text space to allocate for histogram counters.
 * We allocate counters at the same or higher density as function
 * addresses, so that each counter belongs to a unique function.
 * A lower density of counters would give less resolution but a
 * higher density would be wasted.
 */
#define HISTFRACTION    (FUNCTION_ALIGNMENT / sizeof(HISTCOUNTER) == 0 \
                         ? 1 : FUNCTION_ALIGNMENT / sizeof(HISTCOUNTER))

/*
 * Fraction of text space to allocate for from hash buckets.
 * The value of HASHFRACTION is based on the minimum number of bytes
 * of separation between two subroutine call points in the object code.
 * Given MIN_SUBR_SEPARATION bytes of separation the value of
 * HASHFRACTION is calculated as:
 *
 *      HASHFRACTION = MIN_SUBR_SEPARATION / (2 * sizeof(short) - 1);
 *
 * For example, on the VAX, the shortest two call sequence is:
 *
 *      calls   $0,(r0)
 *      calls   $0,(r0)
 *
 * which is separated by only three bytes, thus HASHFRACTION is
 * calculated as:
 *
 *      HASHFRACTION = 3 / (2 * 2 - 1) = 1
 *
 * Note that the division above rounds down, thus if MIN_SUBR_FRACTION
 * is less than three, this algorithm will not work!
 *
 * In practice, however, call instructions are rarely at a minimal
 * distance.  Hence, we will define HASHFRACTION to be 2 across all
 * architectures.  This saves a reasonable amount of space for
 * profiling data structures without (in practice) sacrificing
 * any granularity.
 */
/*
 * XXX I think the above analysis completely misses the point.  I think
 * the point is that addresses in different functions must hash to
 * different values.  Since the hash is essentially division by
 * sizeof(unsigned short), the correct formula is:
 *
 *      HASHFRACTION = MIN_FUNCTION_ALIGNMENT / sizeof(unsigned short)
 *
 * Note that he unsigned short here has nothing to do with the one for
 * HISTFRACTION.
 *
 * Hash collisions from a two call sequence don't matter.  They get
 * handled like collisions for calls to different addresses from the
 * same address through a function pointer.
 */
#define HASHFRACTION    (FUNCTION_ALIGNMENT / sizeof(unsigned short) == 0 \
                         ? 1 : FUNCTION_ALIGNMENT / sizeof(unsigned short))

/*
 * percent of text space to allocate for tostructs with a minimum.
 */
#define ARCDENSITY      5 // was 2 now with shared lib profiling need more space for call info
#define MINARCS         50
/*
 * Limit on the number of arcs to so that arc numbers can be stored in
 * `*froms' and stored and incremented without overflow in links.
 */
#define MAXARCS         (((u_long)1 << (8 * sizeof(u_short))) - 2)

/*
 * Rounding functions.
 */
#define ROUNDUP(x,y)	((((x)+(y)-1)/(y))*(y))
#define ROUNDDOWN(x,y)	(((x)/(y))*(y))


struct pc_struct {
	u_long pc;
	u_long count;
	u_short link;
	u_short tid;
};

/*
 * a raw arc, with pointers to the calling site and
 * the called site and a count.
 */
struct rawarc {
        u_long  raw_frompc;
        u_long  raw_selfpc;
        long    raw_count;
};

/*
 * The profiling data structures are housed in this structure.
 */
struct gmon_arc_param {
	struct gmonparam *gmon;
	u_short          *pc_index;
	u_long           index_size;
	struct pc_struct *pc;
	u_long           pc_size;
	long             pc_limit;
	uintfptr_t       lowpc;
	uintfptr_t       highpc;
	u_long           textsize;
	u_long           hashfraction;
	char             *name;
	int              state_flags;	
	struct gmon_arc_param *next;
};

#define GMON_PARAM_FLAGS_THREAD 0x1  // enable thread level arc counting
#define GMON_PARAM_FLAGS_FROM_TO_ARCS 0x2 //arc format is from(indexed) self(stored) else its the other way around (new style) 

struct gmonparam {
	int             state;
	HISTCOUNTER     *kcount;
	u_long          kcountsize;
	int             flags;
	int             profrate;       /* XXX wrong type to match gmonhdr */
	void            *ldd_handle;
	struct gmon_arc_param  arc_param;  // arc info for program.
};
	
extern struct gmonparam _gmonparam;

/*
 * Possible states of profiling.
 */
#define GMON_PROF_ON    0
#define GMON_PROF_BUSY  1
#define GMON_PROF_MCOUNT_OVERFLOW 2
#define GMON_PROF_OFF   3

#define GMON_STATE_MASK 0xff
#define GET_GMON_STATE(x)   (x & GMON_STATE_MASK)
#define SET_GMON_STATE(x, y) (x = ((x & ~GMON_STATE_MASK) | y))

#define GMON_FLAGS_MASK 0xff00
#define GET_GMON_FLAGS(x) ((x & GMON_FLAGS_MASK) >> 8) 


#endif /* !_SYS_GMON_H_ */


/* __SRCVERSION("gmon.h $Rev: 166179 $"); */
