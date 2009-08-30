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
 *  sys/conf.h
 *

 */
#ifndef __CONF_H_INCLUDED
#define __CONF_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

_C_STD_BEGIN
#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif
_C_STD_END

/* Functions */
#define _CONF_NOP				(0U << 16)	/* Do nothing entry */
#define _CONF_LINK				(1U << 16)	/* table continues here */
#define _CONF_CALL				(2U << 16)	/* See if function will match */
#define _CONF_VALUE				(3U << 16)	/* Contains a value to match */
#define _CONF_END				(4U << 16)	/* End of table */
#define _CONF_CMD_MASK			(0xfU << 16)

/* Conditions (bits) */
#define _CONF_STR				(0x1U << 20)	/* Only use entry if checking for string */
#define _CONF_NUM				(0x2U << 20)	/* Only use entry if checking for number */
#define _CONF_COND_MASK			(0xfU << 20)

/* Value modifiers (bits) */
#define _CONF_NUM_MIN			(1U << 24)
#define _CONF_NUM_MAX			(2U << 24)
#define _CONF_NUM_ADD			(3U << 24)
#define _CONF_MOD_MASK			(0xfU << 24)

/* Flags (bits) */
#define _CONF_STICKY			(0x1U << 28)	/* Used by library to make conf outlive the process */
#define _CONF_INDIRECT			(0x2U << 28)	/* Indirect pointer */
#define _CONF_FCN				(0x4U << 28)	/* Call function */
#define _CONF_FLAGS_MASK		(0xfU << 28)

/* Short name mask */
#define	_CONF_NAME_MASK			(0xffffU)
#define _CONF_NAME_LONG			(0xffffU)	/* Name too long, stored in next entry */

/* Some usefull common combinations */
#define _CONF_LINK_FCN			(_CONF_FCN | _CONF_LINK)
#define _CONF_CALL_FCN			(_CONF_FCN | _CONF_CALL)
#define _CONF_VALUE_FCN			(_CONF_FCN | _CONF_VALUE)
/*	int (*link_call_fcn)(int name, long *value, char *str);		- similar to _conf_get() */
/*	size_t (*value_str_fcn)(int name, char *buff, size_t len);	- similar to confstr() */
/*	long (*value_num_fcn)(int name);							- similar to sysconf() */

#define _CONF_LINK_PTR			(_CONF_INDIRECT | _CONF_LINK)
#define _CONF_CALL_PTR			(_CONF_INDIRECT | _CONF_CALL)
#define _CONF_VALUE_PTR			(_CONF_INDIRECT | _CONF_VALUE)

/* Values must have STR or NUM conditions, so make some common entries */
#define _CONF_VALUE_STR			(_CONF_STR | _CONF_VALUE)
#define _CONF_VALUE_NUM			(_CONF_NUM | _CONF_VALUE)
#define _CONF_VALUE_MIN			(_CONF_NUM | _CONF_VALUE | _CONF_NUM_MIN)
#define _CONF_VALUE_MAX			(_CONF_NUM | _CONF_VALUE | _CONF_NUM_MAX)
#define _CONF_VALUE_ADD			(_CONF_NUM | _CONF_VALUE | _CONF_NUM_ADD)

__BEGIN_DECLS
extern long *_confstr_list;
extern long *_sysconf_list;
extern void _conf_destroy(long *__list);
extern int _conf_get(const long *__list, int __name, long *__value, char *__str);
extern int _conf_set(long **__list, int __cmd, int __name, long __value, const char *__str);
__END_DECLS

#endif

/* __SRCVERSION("conf.h $Rev: 153052 $"); */
