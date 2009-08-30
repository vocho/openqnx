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
 *  sys/target_qnx.h
 *

 *
 *  The only symbols made visible by this header are
 *  OS/compiler reserved symbols.
 */
#ifndef __TARGET_QNX_H_INCLUDED
#define __TARGET_QNX_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#error sys/target_qnx.h should not be included directly.
#endif

/* Just a stub configuration for right now */

#if defined(_POSIX_C_SOURCE) && _POSIX_C_SOURCE < 199303
#error Version of POSIX_C_SOURCE Not supported
#endif

#endif


/* __SRCVERSION("target_qnx.h $Rev: 153052 $"); */
