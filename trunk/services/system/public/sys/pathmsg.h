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
 *  sys/pathmsg.h
 *

 */
#ifndef __PATHMSG_H_INCLUDED
#define __PATHMSG_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
#pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __SYSMSG_H_INCLUDED
#include _NTO_HDR_(sys/sysmsg.h)
#endif

#ifndef __IOMSG_H_INCLUDED
#include _NTO_HDR_(sys/iomsg.h)
#endif

#define PATHMGR_PID				SYSMGR_PID
#define PATHMGR_CHID			SYSMGR_CHID
#define PATHMGR_COID			SYSMGR_COID
#define PATHMGR_HANDLE			SYSMGR_HANDLE
#define PATHMGR_HANDLE_REMOTE	(SYSMGR_HANDLE+1)

enum {
	_PATH_RESOLVE = _PATHMGR_BASE,
	_PATH_CHDIR,
	_PATH_CHROOT
};

#endif

/* __SRCVERSION("pathmsg.h $Rev: 153052 $"); */
