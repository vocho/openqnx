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
 *  sh/inout.h
 *

 */

#ifndef _SH_INOUT_H_INCLUDED
#define _SH_INOUT_H_INCLUDED

/*
    Define the in/out functions for the SH
*/

#ifndef __TYPES_H_INCLUDED
#include <sys/types.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

#define eieio() 

#define DEV_OUT( addr, type, val )	(*(volatile type *)(addr) = (val))

static __inline__ _Uint8t __attribute__((__unused__))
in8(_Uintptrt __addr) {
	_Uint8t	__data;

	__data = *(volatile _Uint8t *)__addr;
	eieio();
	return(__data);
}

static __inline__ _Uint16t __attribute__((__unused__))
in16(_Uintptrt __addr) {
	_Uint16t	__data;

	__data = *(volatile _Uint16t *)__addr;
	eieio();
	return(__data);
}

static __inline__ _Uint32t __attribute__((__unused__))
in32(_Uintptrt __addr) {
	_Uint32t	__data;

	__data = *(volatile _Uint32t *)__addr;
	eieio();
	return(__data);
}

static __inline__ void * __attribute__((__unused__))
in8s(void *__buff, unsigned __len, _Uintptrt __addr) {
	_Uint8t	*__p = (_Uint8t *)__buff;

	while(__len > 0) {
		*__p++ = *(volatile _Uint8t *)__addr;
		--__len;
	}
	eieio();
	return(__p);
}

static __inline__ void * __attribute__((__unused__))
in16s(void *__buff, unsigned __len, _Uintptrt __addr) {
	_Uint16t	*__p = (_Uint16t *)__buff;

	while(__len > 0) {
		*__p++ = *(volatile _Uint16t *)__addr;
		--__len;
	}
	eieio();
	return(__p);
}

static __inline__ void * __attribute__((__unused__))
in32s(void *__buff, unsigned __len, _Uintptrt __addr) {
	_Uint32t	*__p = (_Uint32t *)__buff;

	while(__len > 0) {
		*__p++ = *(volatile _Uint32t *)__addr;
		--__len;
	}
	eieio();
	return(__p);
}

static __inline__ void __attribute__((__unused__))
out8(_Uintptrt __addr, _Uint8t __data) {
	*(volatile _Uint8t *)__addr = __data;
	eieio();
}

static __inline__ void __attribute__((__unused__))
out16(_Uintptrt __addr, _Uint16t __data) {
	*(volatile _Uint16t *)__addr = __data;
	eieio();
}

static __inline__ void __attribute__((__unused__))
out32(_Uintptrt __addr, _Uint32t __data) {
	*(volatile _Uint32t *)__addr = __data;
	eieio();
}

static __inline__ void * __attribute__((__unused__))
out8s(const void *__buff, unsigned __len, _Uintptrt __addr) {
	union {
		const _Uint8t	*__p1;
		_Uint8t			*__p2;
	} __p;
	__p.__p1 = (const _Uint8t *)__buff;

	while(__len > 0) {
		*(volatile _Uint8t *)__addr = *__p.__p1++;
		--__len;
	}
	eieio();
	return(__p.__p2);
}

static __inline__ void * __attribute__((__unused__))
out16s(const void *__buff, unsigned __len, _Uintptrt __addr) {
	union {
		const _Uint16t	*__p1;
		_Uint16t		*__p2;
	} __p;
	__p.__p1 = (const _Uint16t *)__buff;

	while(__len > 0) {
		*(volatile _Uint16t *)__addr = *__p.__p1++;
		--__len;
	}
	eieio();
	return(__p.__p2);
}

static __inline__ void * __attribute__((__unused__))
out32s(const void *__buff, unsigned __len, _Uintptrt __addr) {
	union {
		const _Uint32t *__p1;
		_Uint32t *__p2;
	} __p;
	__p.__p1 = (const _Uint32t *)__buff;

	while(__len > 0) {
		*(volatile _Uint32t *)__addr = *__p.__p1++;
		--__len;
	}
	eieio();
	return(__p.__p2);
}

__END_DECLS

#endif

/* __SRCVERSION("inout.h $Rev: 153052 $"); */
