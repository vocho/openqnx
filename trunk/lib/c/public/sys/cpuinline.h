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
 *  sys/cpuinline.h
 *

 *
 *  All symbols defined through this header file must not polute the namespace.
 *  i.e. the symbols will start with two underscores or one underscore and
 *  a capital letter.
 */
#ifndef __CPUINLINE_H_INCLUDED
#define __CPUINLINE_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include _NTO_CPU_HDR_(cpuinline.h)

#endif

/* __SRCVERSION("cpuinline.h $Rev: 153052 $"); */
