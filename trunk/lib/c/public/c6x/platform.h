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
 *  c6x/platform.h
 *

 */

#ifndef _C6X_PLATFORM_H_INCLUDED
#define _C6X_PLATFORM_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#error c6x/platform.h should not be included directly.
#endif

#define BITFIELD32( start_bit, value )	((value) << (start_bit))
#define BITFIELD16( start_bit, value )	((value) << (start_bit))
#define BITFIELD8( start_bit, value )	((value) << (start_bit))


#if defined(__QNXNTO__)

typedef char *__NTO_va_list;
 
/********************************************************************/
/* WARNING - va_arg will not work for "float" type, must use double */
/* ALSO NOTE THAT DOUBLES MUST BE DOUBLE WORD ALIGNED               */
/********************************************************************/
#define __NTO_va_end(_ap)
#define __NTO_va_copy(__d,__s)	((__d)=(__s))
  
#define __NTO_va_start(_ap, _parmN) \
         (_ap = ((char *)&(_parmN)) + (sizeof(_parmN) < 4 ? 4 : sizeof(_parmN)))

#ifdef _TMS320C6200
#define __NTO_va_arg(_ap, _type)                                       \
         (sizeof(_type) == sizeof(double)                        \
             ? ((_ap += 8), (*(_type *)(_ap - 8)))               \
	     : ((_ap += 4), (*(_type *)(_ap - 4))))
#else
#define __NTO_va_arg(_ap, _type)                                       \
         ((sizeof(_type) == sizeof(double)                       \
             ? ((_ap = (void *)(((int)_ap + 7) & ~7)),           \
	        (_ap += 8), (*(_type *)(_ap - 8)))               \
	     : ((_ap += 4), (*(_type *)(_ap - 4)))))
#endif

/* @@c6x jmpbufsize & align: not too sure about this  */
#define __JMPBUFSIZE	32
typedef unsigned		__jmpbufalign;

#else
#error Not configured for target
#endif

#endif

/* __SRCVERSION("platform.h $Rev: 164949 $"); */
