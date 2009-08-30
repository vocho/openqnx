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

#define FREE_ENTRY (~0U)
static struct mdriver_entry	*mdriver_ptr;
static unsigned				mdriver_num;

void
(mdriver_init)(void) {
	mdriver_ptr = SYSPAGE_ENTRY(mdriver);
	mdriver_num = _syspage_ptr->mdriver.entry_size / sizeof(*mdriver_ptr);
	mdriver_check();
}	

void
(mdriver_check)(void) {
	struct mdriver_entry	*md = mdriver_ptr;
	unsigned				num = mdriver_num;
	unsigned				i;
	int						last = -1;

	for(i = 0; i < num; ++i, ++md) {
		if(md->intr != _NTO_INTR_SPARE) {
			if(md->handler(MDRIVER_KERNEL, md->data)) {
				md->intr = _NTO_INTR_SPARE;
			} else {
				last = i;
			}
		}
	}
	mdriver_num = last + 1;
}

static struct sigevent	sigev;
static pthread_attr_t	attr;

static void
kerext_md_detach(void *d) {
	struct mdriver_entry	*md = d;
	unsigned				num;

	if(md->internal != FREE_ENTRY) {
		lock_kernel();
		interrupt_detach_entry(NULL, md->internal);
		md->internal = FREE_ENTRY;
		num = mdriver_num;
		md = &mdriver_ptr[num-1];
		while((num > 0) && (md->intr == _NTO_INTR_SPARE)) {
			--md;
			--num;
		}
		mdriver_num = num;
	}	
}

static void
md_detach(union sigval d) {
	// Do the detaching in the kernel to avoid race conditions
	// with md_intr_attach().
	__Ring0(kerext_md_detach, d.sival_ptr);
}	


static const struct sigevent *
md_intr(void *d, int id) {
	struct mdriver_entry *md = d;
	int					r;

	if(md->intr != _NTO_INTR_SPARE) {
		InterruptDisable();
		r = md->handler(MDRIVER_PROCESS, md->data);
		InterruptEnable();
		if(r != 0) {
			md->intr = _NTO_INTR_SPARE;
			SIGEV_THREAD_INIT(&sigev, md_detach, md, &attr);
			return &sigev;
		}	
	}	
	return NULL;
}

static void
kerext_md_attach(void *d) {
	struct mdriver_entry	*md = d;
	int						level;
	int						id;

	level = get_interrupt_level(NULL, md->intr);
	if(level < 0) {
		crash();
	}
	lock_kernel();
	id = interrupt_attach(level, &md_intr, md, 0);
	if(id < 0) {
		crash();
	}
	md->internal = id;
}

void
(mdriver_process_time)(void) {
	struct mdriver_entry	*md = mdriver_ptr;
	unsigned				num = mdriver_num;
	unsigned				i;
	struct sched_param		param;

	(void)pthread_attr_init(&attr);
	param.sched_priority = 10;
	(void)pthread_attr_setschedparam(&attr, &param);

	// Set mdriver_num to zero temporarily so that mdriver_intr_attach()
	// doesn't try calling the mini-driver handlers for any of these
	// InterruptAttach's.
	mdriver_num = 0; 
	for(i = 0; i < num; ++i, ++md) {
		if(md->intr != _NTO_INTR_SPARE) {
			__Ring0(kerext_md_attach, md);
		}	
	}	
	mdriver_num = num;
}

int
(mdriver_intr_attach)(int intr) {
	struct mdriver_entry	*md = mdriver_ptr;
	unsigned				num = mdriver_num;
	unsigned				i;
	int						r;
	int						last = -1;
	unsigned				count = 0;

	for(i = 0; i < num; ++i, ++md) {
		if((md->intr == intr) && (md->internal != FREE_ENTRY)) {
			InterruptDisable();
			r = md->handler(MDRIVER_INTR_ATTACH, md->data);
			InterruptEnable();
			if(r != 0) {
				interrupt_detach_entry(NULL, md->internal);
				md->intr = _NTO_INTR_SPARE;
				md->internal = FREE_ENTRY;
				++count;
			} else {
				last = i;
			}
		} else if(md->intr != _NTO_INTR_SPARE) {
			last = i;
		}	
	}
	mdriver_num = last + 1;
	return count;
}

__SRCVERSION("mdriver.c $Rev: 153052 $");
