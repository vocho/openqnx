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



#ifndef __SYSMACROS_H_INCLUDED
#define __SYSMACROS_H_INCLUDED

#if defined(__DEV_T)
typedef __DEV_T		dev_t;
#undef __DEV_T
#endif

#ifndef minor
#define minor(device)                   ((int)((device) & 0x3ff))
#endif

#ifndef major
#define major(device)                   ((int)(((device) >> 10) & 0x3f))
#endif

#ifndef makedev
#define makedev(node,major,minor)       ((dev_t)(((node) << 16) | ((major) << 10) | (minor)))
#endif

#endif

/* __SRCVERSION("sysmacros.h $Rev$"); */
