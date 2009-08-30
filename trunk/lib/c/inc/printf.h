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
 * printf.h --  definitions needed by callers to internal string
 *              formatter __prtf()
 */
#ifndef _PRINTF_H_INCLUDED_
#define _PRINTF_H_INCLUDED_

int __prt(const char *format_str, va_list arg, size_t len, void *ptr, int (*func)(const char *p, int num, void *ptr));

#endif

/* __SRCVERSION("printf.h $Rev: 153052 $"); */
