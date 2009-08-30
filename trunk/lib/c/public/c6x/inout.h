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



/*-
 *  c6x/inout.h
 *

 */

#ifndef __C6X_INOUT_H_INCLUDED
#define __C6X_INOUT_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
#include <sys/types.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

static __inline__ _Uint8t in8( _Uintptrt __addr ) {
	_Uint8t	__data;

	__data = *(volatile _Uint8t *)__addr;
	return( __data );
}

static __inline__ _Uint16t in16( _Uintptrt ____addr ) {
	_Uint16t	____data;

	__data = *(volatile _Uint16t *)__addr;
	return( __data );
}

static __inline__ _Uint16t in32( _Uintptrt __addr ) {
	_Uint32t	__data;

	__data = *(volatile _Uint32t *)__addr;
	return( __data );
}

static __inline__ void out8( _Uintptrt __addr, _Uint8t __data ) {
	*(volatile _Uint8t *)__addr = __data;
}

static __inline__ void out16( _Uintptrt __addr, _Uint16t __data ) {
	*(volatile _Uint16t *)__addr = __data;
}

static __inline__ void out32( _Uintptrt __addr, _Uint32t __data ) {
	*(volatile _Uint32t *)__addr = __data;
}

__END_DECLS

#endif

/* __SRCVERSION("inout.h $Rev: 153052 $"); */
