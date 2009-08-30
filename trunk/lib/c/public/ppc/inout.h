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
 *  ppc/inline.h
 *

 */

#ifndef _PPC_INOUT_H_INCLUDED
#define _PPC_INOUT_H_INCLUDED

/*
    Define the in/out functions for the PPC
*/

#ifndef __TYPES_H_INCLUDED
#include <sys/types.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef _GULLIVER_H_INCLUDED
#include <gulliver.h>
#endif

__BEGIN_DECLS

#define eieio() __asm__ __volatile__( "eieio" )

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

	--__p;
	while(__len > 0) {
		*++__p = *(volatile _Uint8t *)__addr;
		--__len;
	}
	eieio();
	return(__p+1);
}

static __inline__ void * __attribute__((__unused__))
in16s(void *__buff, unsigned __len, _Uintptrt __addr) {
	_Uint16t	*__p = (_Uint16t *)__buff;

	--__p;
	while(__len > 0) {
		*++__p = *(volatile _Uint16t *)__addr;
		--__len;
	}
	eieio();
	return(__p+1);
}

static __inline__ void * __attribute__((__unused__))
in32s(void *__buff, unsigned __len, _Uintptrt __addr) {
	_Uint32t	*__p = (_Uint32t *)__buff;

	--__p;
	while(__len > 0) {
		*++__p = *(volatile _Uint32t *)__addr;
		--__len;
	}
	eieio();
	return(__p+1);
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
	_Uint8t	*__p = (_Uint8t *)__buff;

	--__p;
	while(__len > 0) {
		*(volatile _Uint8t *)__addr = *++__p;
		--__len;
	}
	eieio();
	return(__p+1);
}

static __inline__ void * __attribute__((__unused__))
out16s(const void *__buff, unsigned __len, _Uintptrt __addr) {
	_Uint16t	*__p = (_Uint16t *)__buff;

	--__p;
	while(__len > 0) {
		*(volatile _Uint16t *)__addr = *++__p;
		--__len;
	}
	eieio();
	return(__p+1);
}

static __inline__ void * __attribute__((__unused__))
out32s(const void *__buff, unsigned __len, _Uintptrt __addr) {
	_Uint32t	*__p = (_Uint32t *)__buff;

	--__p;
	while(__len > 0) {
		*(volatile _Uint32t *)__addr = *++__p;
		--__len;
	}
	eieio();
	return(__p+1);
}

__END_DECLS

static __inline__ _Uint16t __attribute__((__unused__))
__swap_in16(volatile _Uintptrt __addr) {
	int ret;

	__asm__ __volatile__("lhbrx %0,0,%1; eieio" : "=r" (ret) :
			      "r" (__addr), "m" (__addr));
	return ret;
}

static __inline__ void __attribute__((__unused__))
__swap_out16(volatile _Uintptrt __addr, _Uint16t __data) {
	__asm__ __volatile__("sthbrx %1,0,%2; eieio" : "=m" (__addr) :
			      "r" (__data), "r" (__addr));
}

static __inline__ _Uint32t __attribute__((__unused__))
__swap_in32(volatile _Uintptrt __addr) {
	unsigned ret;

	__asm__ __volatile__("lwbrx %0,0,%1; eieio" : "=r" (ret) :
			     "r" (__addr), "m" (__addr));
	return ret;
}

static __inline__ void __attribute__((__unused__))
__swap_out32(volatile _Uintptrt __addr, _Uint32t __data) {
	__asm__ __volatile__("stwbrx %1,0,%2; eieio" : "=m" (__addr) :
			     "r" (__data), "r" (__addr));
}

#ifdef __BIGENDIAN__
	#define inle16(__port)             __swap_in16(__port)
	#define inle32(__port)             __swap_in32(__port)
	#define outle16(__port, __val)     __swap_out16(__port, __val)
	#define outle32(__port, __val)     __swap_out32(__port, __val)
#else
	#define inbe16(__port)             __swap_in16(__port)
	#define inbe32(__port)             __swap_in32(__port)
	#define outbe16(__port, __val)     __swap_out16(__port, __val)
	#define outbe32(__port, __val)     __swap_out32(__port, __val)
#endif

#endif

/* __SRCVERSION("inout.h $Rev: 153052 $"); */
