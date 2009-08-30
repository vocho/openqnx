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
 *  stdbool.h   Macros for boolean tests
 *

 */
#ifndef _STDBOOL_H_INCLUDED
#define _STDBOOL_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __cplusplus
_C_STD_BEGIN
#if (defined(__STDC_VERSION__) && __STDC_VERSION__ < 199901L) || \
    (defined(__GNUC__) && __GNUC__ < 3)
typedef char	_Bool;
#endif
_C_STD_END

#define bool	_Bool
#define false	0
#define true	1

#endif

#define __bool_true_false_are_defined	1

#endif

/* __SRCVERSION("stdbool.h $Rev: 157817 $"); */
