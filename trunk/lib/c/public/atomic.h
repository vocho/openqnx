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
 *  atomic.h
 *

 */

#ifndef _ATOMIC_H_INCLUDED
#define _ATOMIC_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

extern void atomic_add(volatile unsigned *__loc, unsigned __incr);
extern void atomic_clr(volatile unsigned *__loc, unsigned __bits);
extern void atomic_set(volatile unsigned *__loc, unsigned __bits);
extern void atomic_sub(volatile unsigned *__loc, unsigned __decr);
extern void atomic_toggle(volatile unsigned *__loc, unsigned __bits);

/*
 * These return the previous value of '*__loc', but may be slower than
 * the above functions on some machines. Don't use them unless you
 * need the return value.
 */
extern unsigned atomic_add_value(volatile unsigned *__loc, unsigned __incr);
extern unsigned atomic_clr_value(volatile unsigned *__loc, unsigned __bits);
extern unsigned atomic_set_value(volatile unsigned *__loc, unsigned __bits);
extern unsigned atomic_sub_value(volatile unsigned *__loc, unsigned __decr);
extern unsigned atomic_toggle_value(volatile unsigned *__loc, unsigned __bits);

__END_DECLS

#endif

/* __SRCVERSION("atomic.h $Rev: 153052 $"); */
