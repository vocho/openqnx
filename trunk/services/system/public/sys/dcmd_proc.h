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


#ifndef __DCMD_PROC_H_INCLUDED
#define __DCMD_PROC_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef _DEVCTL_H_INCLUDED
#include _NTO_HDR_(devctl.h)
#endif

__BEGIN_DECLS

#include _NTO_HDR_(_pack64.h)

typedef struct _dcmd_memmgr_memobj_old {
	off64_t						offset;
	_Uint64t					size;
	_Int32t						flags;
}							dcmd_memmgr_memobj_old;

typedef struct _dcmd_memmgr_memobj {
	off64_t						offset;
	_Uint64t					size;
	_Int32t						flags;
	_Uint32t					special;
	_Uint32t					reserved[2];
}							dcmd_memmgr_memobj;


#define __PROC_SUBCMD_PROCFS	0
#define __PROC_SUBCMD_MEMMGR	100
#define __PROC_SUBCMD_USER      200


/* This call is made to change attributes of a memory object
 associated with the file descriptor.
 Args: A dcmd_memmgr_memobj structure with the new attributes is
 provided as input.
 Notes: This function is for internal use and should not be called
 directly.  Instead use the shm_ctl() cover function.  */
#define DCMD_MEMMGR_MEMOBJ_OLD	__DIOT(_DCMD_PROC, __PROC_SUBCMD_MEMMGR + 0, dcmd_memmgr_memobj_old)
#define DCMD_MEMMGR_MEMOBJ		__DIOT(_DCMD_PROC, __PROC_SUBCMD_MEMMGR + 0, dcmd_memmgr_memobj)

#include _NTO_HDR_(_packpop.h)

__END_DECLS

#endif

/* __SRCVERSION("dcmd_proc.h $Rev: 154853 $"); */
