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


#ifndef _KERNEL_IF_H_
#define _KERNEL_IF_H_

#include <sys/cdefs.h>

#ifdef DECLARE_GLOBALS
#define EXT 
#else
#define EXT extern
#endif

EXT int 		num_kernel_buffers;
EXT tracebuf_t	*kernel_buffers_vaddr;

__BEGIN_DECLS

extern int kernel_attach( int nbufs, int reuse_buffers, paddr_t *kernel_buffers_paddr );
extern int kernel_detach( paddr_t kernel_buffers_paddr, int persist_buffers );

__END_DECLS

#endif //_KERNEL_IF_H_

/* __SRCVERSION("kernel_if.h $Rev: 157117 $"); */
