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
 * Copyright (c) 1983, 1992, 1993
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
 */

#if !defined(lint) && !defined(KERNEL) && defined(LIBC_SCCS)
#if 0
static char sccsid[] = "@(#)mcount.c    8.1 (Berkeley) 6/4/93";
#endif
static const char rcsid[] =
        "$Id: mcount.c 159982 2007-12-10 18:55:43Z MYeung $";
#endif

#define __INLINE_FUNCTIONS__

#include <stdio.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/neutrino.h>
#include <pthread.h>
#include "cpucfg.h"

#define DEF_MCOUNT
#include "gmon.h"   // For now

static struct gmon_arc_param *p;
static pthread_mutex_t __profile_mutex = PTHREAD_MUTEX_INITIALIZER;
/*
 * mcount is called on entry to each function compiled with the profiling
 * switch set.  _mcount(), which is declared in a machine-dependent way
 * with _MCOUNT_DECL, does the actual work and is either inlined into a
 * C routine or called by an assembly stub.  In any case, this magic is
 * taken care of by the MCOUNT definition in <machine/profile.h>.
 *
 * _mcount updates data structures that represent traversals of the
 * program's call graph edges.  frompc and selfpc are the return
 * address and function address that represents the given call graph edge.
 *
 * Note: the original BSD code used the same variable (frompcindex) for
 * both frompcindex and frompc.  Any reasonable, modern compiler will
 * perform this optimization.
 */
_MCOUNT_DECL(frompc, selfpc)    /* _mcount; may be static, inline, etc */
        register uintfptr_t frompc, selfpc;
{
    register fptrdiff_t pci;
    register uintfptr_t pc;
	register u_short *pcindex;
	register struct pc_struct *pc_s, *prev_pc_s;
	register long index;
	int tid = 0;
	struct gmonparam *gmon = &_gmonparam;
	
	/*
	 * Check out the locking -- in most cases, we don't need to grab the mutex
	 * We also won't be called recursively, so the test below should be sufficient.
	 */

	if (gmon->state != GMON_PROF_ON || in_interrupt())
		return;
	

	if (p == NULL) {
		register struct gmon_arc_param *param = &_gmonparam.arc_param;
		do {
			if (param->lowpc < selfpc && param->highpc > selfpc) {
				break;
			}
			param = param->next;
		} while(param != NULL);
		if (param == NULL) {
			p = (void *)-1;
			return;
		}
		p = param;
	} else if (p == (void*)-1) {
		return;
	}

	if (GET_GMON_STATE(p->state_flags) != GMON_PROF_ON) {
		return;
	}

	if (gmon->flags & GMON_PARAM_FLAGS_THREAD) {
		tid = pthread_self();
	}

	if (gmon->flags & GMON_PARAM_FLAGS_FROM_TO_ARCS) {
		pci = frompc - p->lowpc;
		pc = selfpc;
	} else {
		pci = selfpc - p->lowpc;
		pc = frompc;
	}
	pcindex = &p->pc_index[pci / (p->hashfraction * sizeof(*p->pc_index))];
	index = *pcindex;

	if (index == 0) {
		/*
		 *      first time traversing this arc
		 */
		while(_mutex_lock(&__profile_mutex) == EINTR)
			;

		index = ++p->pc[0].link;
		if (index >= p->pc_limit) {
			p->pc[0].link--;
			goto overflow;
		}
		*pcindex = index;
		pc_s = &p->pc[index];
		pc_s->pc = pc;
		pc_s->count = 1;
		pc_s->tid = tid;
		pc_s->link = 0;			
		goto done_w_unlock;
	}
	pc_s = &p->pc[index];
	if ((pc_s->pc == pc) && (pc_s->tid == tid)) {
		/*
		 * arc at front of chain; usual case.
		 * this is the fast path without the need for mutexing
		 */
		pc_s->count++;
		return;
	}
	/*
	 * have to go looking down chain for it.
	 * top points to what we are looking at,
	 * prevtop points to previous top.
	 * we know it is not at the head of the chain.
	 */
	while(_mutex_lock(&__profile_mutex) == EINTR)
		;
	for (; /* goto done */; ) {
		if (pc_s->link == 0) {
			/*
			 * pc is end of the chain and none of the chain
			 * had pc->pc == pc.
			 * so we allocate a new pc_struct
			 * and link it to the head of the chain.
			 */
			index = ++p->pc[0].link;
			if (index >= p->pc_limit) {
				p->pc[0].link--;
				goto overflow;
			}
			
			pc_s = &p->pc[index];
			pc_s->pc = pc;
			pc_s->count = 1;
			pc_s->tid = tid;
			pc_s->link = *pcindex;
			*pcindex = index;
			goto done_w_unlock;
		}
		/*
		 * otherwise, check the next arc on the chain.
		 */
		prev_pc_s = pc_s;
		pc_s = &p->pc[pc_s->link];
		if ((pc_s->pc == pc) && (pc_s->tid == tid)) {
			/*
			 * there it is.
			 * increment its count
			 * move it to the head of the chain.
			 */
			pc_s->count++;
			index = prev_pc_s->link;
			prev_pc_s->link = pc_s->link;
			pc_s->link = *pcindex;
			*pcindex = index;
			goto done_w_unlock;
		}
		
	}	
done_w_unlock:
	while(_mutex_unlock(&__profile_mutex) == EINTR)
		;
	return;
overflow:
	while(_mutex_unlock(&__profile_mutex) == EINTR)
		;
	if (GET_GMON_STATE(p->state_flags) != GMON_PROF_MCOUNT_OVERFLOW) {
		fprintf(stderr, "mcount: overflow profiling module: %s\n",p->name != NULL ? p->name : "unknown");
	}
	SET_GMON_STATE(p->state_flags, GMON_PROF_MCOUNT_OVERFLOW);
	*((unsigned char *)p->pc_index + p->index_size) = p->state_flags;
	return;
}

/*
 * Actual definition of mcount function.  Defined in <machine/profile.h>,
 * which is included by <sys/gmon.h>.
 */
MCOUNT

__SRCVERSION("mcount.c $Rev: 159982 $");
