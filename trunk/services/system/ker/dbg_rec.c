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

#include "externs.h"

// This file is only used for kernel debugging purposes


#define RSIZE 0x100
int rbuff[RSIZE];
unsigned ridx;
#define ADV(r,i)	(((r) + (i)) & (RSIZE-1))

static 
#if defined(__GNUC__)
inline 
#endif
unsigned
get_ridx(unsigned inc) {
	unsigned 	result;

#if defined(VARIANT_smp)	
	unsigned	new;

	do {
		result = ridx;
		new = ADV(result, inc);
	} while(_smp_cmpxchg(&ridx, result, new) != result);
#else
	unsigned	prev;

	prev = InterruptStatus();
	InterruptDisable();
	result = ridx;
	ridx = ADV(result, inc);
	if(prev){ 
		InterruptEnable();
	}
#endif
	return(result);
}
			
void
rec(int num) {
	rbuff[get_ridx(1)] = num;
}
			
void
rec_who(int num) {
	unsigned			my_cpu;
	unsigned			i;
	PROCESS				*prp;

	my_cpu = RUNCPU;
	prp = actives[my_cpu]->aspace_prp;
	if(prp == NULL) prp = actives[my_cpu]->process;
	i = get_ridx(2);
	rbuff[i] = num;
	i = ADV(i, 1);
	rbuff[i] = (my_cpu << 28) + ((prp->pid & 0xffff) << 4) + actives[my_cpu]->tid;
}

void
rec_who2(int num, int num2) {
	unsigned			my_cpu;
	PROCESS				*prp;
	unsigned			i;

	my_cpu = RUNCPU;
	prp = actives[my_cpu]->aspace_prp;
	if(prp == NULL) prp = actives[my_cpu]->process;
	i = get_ridx(3);
	rbuff[i] = num;
	i = ADV(i, 1);
	rbuff[i] = num2;
	i = ADV(i, 1);
	rbuff[i] = (my_cpu << 28) + ((prp->pid & 0xffff) << 4) + actives[my_cpu]->tid;
}

void
rec_who3(int num, int num2, int num3) {
	unsigned			my_cpu;
	PROCESS				*prp;
	unsigned			i;

	my_cpu = RUNCPU;
	prp = actives[my_cpu]->aspace_prp;
	if(prp == NULL) prp = actives[my_cpu]->process;
	i = get_ridx(4);
	rbuff[i] = num;
	i = ADV(i, 1);
	rbuff[i] = num2;
	i = ADV(i, 1);
	rbuff[i] = num3;
	i = ADV(i, 1);
	rbuff[i] = (my_cpu << 28) + ((prp->pid & 0xffff) << 4) + actives[my_cpu]->tid;
}

__SRCVERSION("dbg_rec.c $Rev: 199078 $");
