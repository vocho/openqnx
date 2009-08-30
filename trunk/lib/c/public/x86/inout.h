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
 *  x86/inout.h     Define the in and out functions for the X86
 *

 */

#ifndef _X86_INOUT_H_INCLUDED
#define _X86_INOUT_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

#if defined(__WATCOMC__)

	/* input a byte from a port */
_Uint8t  in8(_Uint16t __port);

	/* input a word from a port */
_Uint16t  in16(_Uint16t __port);

	/* input a long from a port */
_Uint32t  in32(_Uint16t __port);

	/* input a byte string from a port */
void	*in8s(void *__buff, unsigned __len, _Uint16t __port);

	/* input a word string from a port */
void	*in16s(void *__buff, unsigned __len, _Uint16t __port);

	/* input a long string from a port */
void	*in32s(void *__buff, unsigned __len, _Uint16t __port);

	/* output a byte to a port */
void	out8(_Uint16t __port, _Uint8t __val);

	/* output a word to a port */
void	out16(_Uint16t __port, _Uint16t __val);

	/* output a long to a port */
void	out32(_Uint16t __port, _Uint32t __val);

	/* output a byte string to a port */
void	*out8s(const void *__buff, unsigned __len, _Uint16t __port);

	/* output a word string to a port */
void	*out16s(const void *__buff, unsigned __len, _Uint16t __port);

	/* output a long string to a port */
void	*out32s(const void *__buff, unsigned __len, _Uint16t __port);

#pragma aux in8  = "in al,dx" parm nomemory [dx] modify nomemory exact [al] value [al]
#pragma aux in16 = "in ax,dx" parm nomemory [dx] modify nomemory exact [ax] value [ax]
#pragma aux in32 = ".386" "in eax,dx" parm nomemory [dx] modify nomemory exact [eax] value [eax]

#pragma aux in8s = "rep insb" parm [edi] [ecx] [dx] modify exact [ecx edi] value [edi]
#pragma aux in16s = "rep insw" parm [edi] [ecx] [dx] modify exact [ecx edi] value [edi]
#pragma aux in32s = ".386" "rep insd" parm [edi] [ecx] [dx] modify exact [ecx edi] value [edi]

#pragma aux out8  = "out dx,al" parm nomemory [dx] [al] modify nomemory exact []
#pragma aux out16 = "out dx,ax" parm nomemory [dx] [ax] modify nomemory exact []
#pragma aux out32 = ".386" "out dx,eax" parm nomemory [dx] [eax] modify nomemory exact []

#pragma aux out8s = "rep outsb" parm [esi] [ecx] [dx] modify nomemory exact [ecx esi] value [esi]
#pragma aux out16s = "rep outsw" parm [esi] [ecx] [dx] modify nomemory exact [ecx esi] value [esi]
#pragma aux out32s = ".386" "rep outsd" parm [esi] [ecx] [dx] modify nomemory exact [ecx esi] value [esi]

#elif defined(__INTEL_COMPILER)

static __inline__ _Uint8t __attribute__((__unused__)) in8(_Uint16t __port) {
	_Uint8t __v;
	__asm__ __volatile__ ("inb %w1,%0" : "=a" (__v) : "d" (__port));
	return __v;
} 

static __inline__ _Uint16t __attribute__((__unused__)) in16(_Uint16t __port) {
	_Uint16t __v;
	__asm__ __volatile__ ("inw %w1,%0" : "=a" (__v) : "d" (__port));
	return __v;
} 

static __inline__ _Uint32t __attribute__((__unused__)) in32(_Uint16t __port) {
	_Uint32t __v;
	__asm__ __volatile__ ("inl %w1,%0" : "=a" (__v) : "d" (__port));
	return __v;
} 

static __inline__ void * __attribute__((__unused__)) in8s(void *__addr, unsigned __count, _Uint16t __port) {
	__asm__ __volatile__("rep; insb" : "=D" (__addr), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __addr;
} 

static __inline__ void * __attribute__((__unused__)) in16s(void *__addr, unsigned __count, _Uint16t __port) {
	__asm__ __volatile__("rep; insw" : "=D" (__addr), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __addr;
} 

static __inline__ void * __attribute__((__unused__)) in32s(void *__addr, unsigned __count, _Uint16t __port) {
	__asm__ __volatile__("rep; insl" : "=D" (__addr), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __addr;
} 



static __inline__ void __attribute__((__unused__)) out8(_Uint16t __port, _Uint8t __value) {
	__asm__ __volatile__ ("outb %b1,%w0" : : "d" (__port), "a" (__value));
}

static __inline__ void __attribute__((__unused__)) out16(_Uint16t __port, _Uint16t __value) {
	__asm__ __volatile__ ("outw %w1,%w0" : : "d" (__port), "a" (__value));
}

static __inline__ void __attribute__((__unused__)) out32(_Uint16t __port, _Uint32t __value) {
	__asm__ __volatile__ ("outl %1,%w0" : : "d" (__port), "a" (__value));
}

static __inline__ void * __attribute__((__unused__)) out8s(const void *__addr, unsigned long __count, _Uint16t __port) {
	register void *__result;
	__asm__ __volatile__ ("rep; outsb" : "=S" (__result), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __result;
} 

static __inline__ void * __attribute__((__unused__)) out16s(const void *__addr, unsigned long __count, _Uint16t __port) {
	register void *__result;
	__asm__ __volatile__ ("rep; outsw" : "=S" (__result), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __result;
} 

static __inline__ void * __attribute__((__unused__)) out32s(const void *__addr, unsigned long __count, _Uint16t __port) {
	register void *__result;
	__asm__ __volatile__ ("rep; outsl" : "=S" (__result), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __result;
} 

#elif defined(__GNUC__)

static __inline__ _Uint8t __attribute__((__unused__)) in8(_Uint16t __port) {
	_Uint8t __v;
	__asm__ __volatile__ ("inb %w1,%0" : "=a,a" (__v) : "N,d" (__port));
	return __v;
} 

static __inline__ _Uint16t __attribute__((__unused__)) in16(_Uint16t __port) {
	_Uint16t __v;
	__asm__ __volatile__ ("inw %w1,%0" : "=a,a" (__v) : "N,d" (__port));
	return __v;
} 

static __inline__ _Uint32t __attribute__((__unused__)) in32(_Uint16t __port) {
	_Uint32t __v;
	__asm__ __volatile__ ("inl %w1,%0" : "=a,a" (__v) : "N,d" (__port));
	return __v;
} 

static __inline__ void * __attribute__((__unused__)) in8s(void *__addr, unsigned __count, _Uint16t __port) {
	__asm__ __volatile__("rep; insb" : "=D" (__addr), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __addr;
} 

static __inline__ void * __attribute__((__unused__)) in16s(void *__addr, unsigned __count, _Uint16t __port) {
	__asm__ __volatile__("rep; insw" : "=D" (__addr), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __addr;
} 

static __inline__ void * __attribute__((__unused__)) in32s(void *__addr, unsigned __count, _Uint16t __port) {
	__asm__ __volatile__("rep; insl" : "=D" (__addr), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __addr;
} 



static __inline__ void __attribute__((__unused__)) out8(_Uint16t __port, _Uint8t __value) {
	__asm__ __volatile__ ("outb %b1,%w0" : : "N,d" (__port), "a,a" (__value));
}

static __inline__ void __attribute__((__unused__)) out16(_Uint16t __port, _Uint16t __value) {
	__asm__ __volatile__ ("outw %w1,%w0" : : "N,d" (__port), "a,a" (__value));
}

static __inline__ void __attribute__((__unused__)) out32(_Uint16t __port, _Uint32t __value) {
	__asm__ __volatile__ ("outl %1,%w0" : : "N,d" (__port), "a,a" (__value));
}

static __inline__ void * __attribute__((__unused__)) out8s(const void *__addr, unsigned long __count, _Uint16t __port) {
	register void *__result;
	__asm__ __volatile__ ("rep; outsb" : "=S" (__result), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __result;
} 

static __inline__ void * __attribute__((__unused__)) out16s(const void *__addr, unsigned long __count, _Uint16t __port) {
	register void *__result;
	__asm__ __volatile__ ("rep; outsw" : "=S" (__result), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __result;
} 

static __inline__ void * __attribute__((__unused__)) out32s(const void *__addr, unsigned long __count, _Uint16t __port) {
	register void *__result;
	__asm__ __volatile__ ("rep; outsl" : "=S" (__result), "=c" (__count) : "d" (__port),"0" (__addr),"1" (__count): "memory");
	return __result;
} 

#elif defined __MWERKS__

#define __MWINL  inline __declspec(naked)

static __MWINL _Uint8t in8(_Uint16t __port) {
	asm {
		push edx
		mov  dx,__port
		in   al,dx
		pop edx
	}
}

static __MWINL _Uint16t in16(_Uint16t __port) {
	asm {
		push edx
		mov  dx,__port
		in   ax,dx
		pop edx
	}
}
static __MWINL _Uint32t in32(_Uint16t __port) {
	asm {
		push edx
		mov  dx,__port
		in   eax,dx
		pop edx
	}
}

static __MWINL void * in8s(void *__addr, unsigned __count, _Uint16t __port) {
	asm {
		push ecx
		push edi
		push edx
		mov edi, __addr
		mov ecx, __count
		mov dx, __port
		rep: insb
		pop edx
		pop edi
		pop ecx
	}
}
static __MWINL void * in16s(void *__addr, unsigned __count, _Uint16t __port) {
	asm {
		push ecx
		push edi
		push edx
		mov edi, __addr
		mov ecx, __count
		mov dx, __port
		rep: insw
		pop edx
		pop edi
		pop ecx
	}
}

static __MWINL void * in32s(void *__addr, unsigned __count, _Uint16t __port) {
	asm {
		push ecx
		push edi
		push edx
		mov edi, __addr
		mov ecx, __count
		mov dx, __port
		rep: insd
		pop edx
		pop edi
		pop ecx
	}
}

static __MWINL void out8(_Uint16t __port, _Uint8t __val) {
	asm {
		push edx
		mov  dx,__port
		mov  eax,__val
		out  dx,al
		pop edx
	}
}
static __MWINL void out16(_Uint16t __port, _Uint16t __val) {
	asm {
		push edx
		mov  dx,__port
		mov  eax,__val
		out  dx,ax
		pop edx
	}
}
static __MWINL void out32(_Uint16t __port, _Uint32t __val) {
	asm {
		push edx
		mov  dx,__port
		mov  eax,__val
		out  dx,eax
		pop edx
	}
}

static __MWINL void * out8s(const void *__addr, unsigned __count, _Uint16t __port) {
	asm {
		push ecx
		push edi
		push edx
		mov esi, __addr
		mov ecx, __count
		mov dx, __port
		rep: outsb
		pop edx
		pop edi
		pop ecx
	}
}
static __MWINL void * out16s(const void *__addr, unsigned __count, _Uint16t __port) {
	asm {
		push ecx
		push edi
		push edx
		mov esi, __addr
		mov ecx, __count
		mov dx, __port
		rep: outsw
		pop edx
		pop edi
		pop ecx
	}
}
static __MWINL void * out32s(const void *__addr, unsigned __count, _Uint16t __port) {
	asm {
		push ecx
		push edi
		push edx
		mov esi, __addr
		mov ecx, __count
		mov dx, __port
		rep: outsd
		pop edx
		pop edi
		pop ecx
	}
}

#undef __MWINL
#else
#error Compiler not defined
#endif


__END_DECLS

#endif

/* __SRCVERSION("inout.h $Rev: 207552 $"); */
