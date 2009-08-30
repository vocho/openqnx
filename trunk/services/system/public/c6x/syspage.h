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
 *  c6x/syspage.h
 *

 */

#ifdef __C6X__

/*
 * Pointer to per-thread storage, actually located at the top of each
 * thread's stack.
 */

extern struct cpupage_entry *_cpupage_ptr;
#define ThreadLocalStorage()	(_cpupage_ptr->tls)

#endif /* __C6X__ */
 
struct c6x_syspage_entry {
	syspage_entry_info	kerinfo;
};

struct c6x_kernel_entry {
	unsigned long	code[2];
};

/* __SRCVERSION("syspage.h $Rev: 153052 $"); */
