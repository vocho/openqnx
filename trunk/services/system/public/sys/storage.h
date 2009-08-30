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
 *  sys/storage.h
 *

 */

#ifndef __STORAGE_H_INCLUDED
#define __STORAGE_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include _NTO_HDR_(_pack64.h)

__BEGIN_DECLS

/*
 * Thread local storage. This data is at the top of each threads stack.
 */
struct _thread_local_storage {
	void						(*__exitfunc)(void *);
	void						*__arg;
	int							*__errptr;
	int							__errval;		/* Not used for main() thread. */
	unsigned					__flags;
	int							__pid;
	int							__tid;
	unsigned					__owner;
	void						*__stackaddr;
	unsigned					__reserved1;
	unsigned					__numkeys;
	void						**__keydata;	/* Indexed by pthread_key_t */
	void						*__cleanup;
	void						*__fpuemu_data;
	void						*__reserved2[2];
};

/*
 * Routine to return pointer to thread local storage structure.
 */
extern struct _thread_local_storage		*__tls(void) __attribute__((__const__));


/*
 * Process local storage.
 */
struct _process_local_storage {
	void						*(*__getgot)(void *__pltaddr);
	void						(*__mathemulator)(unsigned __sigcode, void **__pdata, void *__regs);
	void						*__dll_list;
	void						*__reserved[6];
	void						(*__threadwatch)(int __tid);
};

__END_DECLS

#include _NTO_HDR_(_packpop.h)

#endif

/* __SRCVERSION("storage.h $Rev: 153052 $"); */
