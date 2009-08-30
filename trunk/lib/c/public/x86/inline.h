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
 *  x86/inline.h     some handy pragma's for low-level work: 
 *

 */

#ifndef _X86_INLINE_H_INCLUDED
#define _X86_INLINE_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef _X86_INOUT_INCLUDE
#include <x86/inout.h>
#endif

__BEGIN_DECLS

#if defined(__WATCOMC__)

	/* issues an int 3 to put you in the process debugger */
void	breakpoint(void);

	/* delays approximately 8 clocks * count */
void    delay_8X(unsigned count);

	/* returns the current value of the flags register */
unsigned    pswget(void);

	/* restores psw into flags register */
void    restore(unsigned __psw);

	/* disables interrupts, returns previous value of flags */
unsigned    disable(void);

	/* enables interrupts, returns previous value of flags */
unsigned    enable(void);

	/* sets direction flag to forward, returns previous value of flags */
unsigned    str_forward(void);

	/* sets direction flag to reverse, returns previous value of flags */
unsigned    str_reverse(void);

	/* return the size of the segment */
unsigned segsize(_Uint16t __sel);

	/* verify a pointer/length is readable */
#define verr(p,len) \
    (_verr(FP_SEG(p)) ? segsize(FP_SEG(p)) >= (FP_OFF(p) + len) : 0)
_Uint8t _verr(_Uint16t __seg);
    
	/* verify a pointer/length is writeable */
#define verw(p,len) \
    (_verw(FP_SEG(p)) ? segsize(FP_SEG(p)) >= (FP_OFF(p) + len) : 0)
_Uint8t _verw(int __seg);

   	/* return the current value of the 'cs' register */
_Uint16t    _cs(void);
    
   	/* return the current value of the 'ds' register */
_Uint16t    _ds(void);
    
   	/* return the current value of the 'es' register */
_Uint16t    _es(void);
    
   	/* return the current value of the 'fs' register */
_Uint16t    _fs(void);
    
   	/* return the current value of the 'gs' register */
_Uint16t    _gs(void);
    
   	/* return the current value of the 'ss' register */
_Uint16t    _ss(void);

	/* load segment registers register */
void ldds(unsigned __ds);
void ldes(unsigned __es);
void ldfs(unsigned __fs);
void ldgs(unsigned __gs);

	/* load stack pointer */
void ldesp(unsigned __esp);

	/* does 2 "jmp ." to avoid trashing IO device */
void    io_safe(void);

	/* swaps byte ordering of a word */
_Uint16t    swapw(_Uint16t __val);

	/* swaps byte ordering of a long */
_Uint32t   swapl(_Uint32t __lval);

	/*
    	memcopies by 8,16 and 32 bits resp.
    	note nbytes is BYTES and is expected to be an even
        multiple of the copy size (ie 16 or 32).
	*/
memcpy8(void __far *__dst, const void __far *__src, unsigned __nbytes);
memcpy16(void __far *__dst, const void __far *__src, unsigned __nbytes);
memcpy32(void __far *__dst, const void __far *__src, unsigned __nbytes);

	/* Read the Pentium's real time counter */
void rdtsc(void *__ptr);

	/* Various floating point thingies */
void fninit(void);
int fsw(void);
int fnclex(void);
void fsave(void *__addr);
void fnsave(void *__addr);
void frstor(void *__addr);

	/* Find high order on bit */
int bsr(long unsigned __eax);

	/* Find high order on bit, handling case where nothing's on */
int bsr0(long unsigned __eax);

	/* SMP Atomic or'ing of memory */
unsigned smp_locked_or(unsigned *__addr, unsigned __value);

	/* SMP Atomic anding'ing of memory */
unsigned smp_locked_and(unsigned *__addr, unsigned __value);

#pragma aux breakpoint = "int 0x03" parm nomemory [] modify nomemory exact []

unsigned trppoint(void);
#pragma aux trppoint = "int 020h" parm nomemory [ ] modify exact nomemory [ ]

#pragma aux _verr = ".286p" \
					"verr ax" \
					"lahf" \
					"shr ax,14" \
					"and al,1" \
					parm [ax] value [al]

#pragma aux _verw = ".286p" \
					"verw ax" \
					"lahf" \
					"shr ax,14" \
					"and al,1" \
					parm [ax] value [al]

#pragma aux _cs = "mov ax,cs" parm nomemory [] modify nomemory exact [ax] value [ax]
#pragma aux _ds = "mov ax,ds" parm nomemory [] modify nomemory exact [ax] value [ax]
#pragma aux _es = "mov ax,es" parm nomemory [] modify nomemory exact [ax] value [ax]
#pragma aux _ss = "mov ax,ss" parm nomemory [] modify nomemory exact [ax] value [ax]

#pragma aux io_safe = 0xeb 0x00 0xeb 0x00 parm nomemory [] modify nomemory exact []

#pragma aux swapw = "xchg al,ah" parm nomemory [ax] modify nomemory exact [ax] value [ax]

#pragma aux fninit = ".8087" "fninit" parm nomemory [ ] modify exact nomemory [ ]
#pragma aux fnclex = ".8087" "fnclex" modify exact nomemory [ ];

#ifdef __386__

#pragma aux _fs = ".386" "mov ax,fs" parm nomemory [] modify nomemory exact [ax] value [ax]
#pragma aux _gs = ".386" "mov ax,gs" parm nomemory [] modify nomemory exact [ax] value [ax]

#pragma aux ldds = \
			"mov ds,ax" \
			parm nomemory [ eax ] \
			modify exact nomemory [ ];

#pragma aux ldes = \
			"mov es,ax" \
			parm nomemory [ eax ] \
			modify exact nomemory [ ];

#pragma aux ldfs = \
			"mov fs,ax" \
			parm nomemory [ eax ] \
			modify exact nomemory [ ];

#pragma aux ldgs = \
			"mov gs,ax" \
			parm nomemory [ eax ] \
			modify exact nomemory [ ];

#pragma aux ldesp = \
			"mov esp,eax" \
			parm nomemory [ eax ] \
			modify exact nomemory [ ];

#pragma aux delay_8X = "here: loop here" parm nomemory [ecx] modify nomemory exact [ecx];

#pragma aux pswget = ".386" "pushfd" "pop eax" parm nomemory [] modify nomemory [] value  [eax];
#pragma aux restore = ".386" "popfd" parm routine nomemory [] modify nomemory exact [];
#pragma aux disable = ".386" "pushfd" "pop eax" "cli" parm nomemory [] modify exact [eax] value [eax]
#pragma aux enable = ".386" "pushfd" "sti" "pop eax" parm nomemory [] modify exact [eax] value [eax]
#pragma aux str_forward = ".386" "pushfd" "cld" "pop eax" parm nomemory [] modify exact [eax] value [eax]
#pragma aux str_reverse = ".386" "pushfd" "std" "pop eax" parm nomemory [] modify exact [eax] value [eax]

#pragma aux segsize = ".386p" "lsl eax,ax" parm nomemory [ax] modify exact nomemory [eax] value [eax]
            
#pragma aux swapl = ".386" "xchg al,ah" "ror eax,16" "xchg al,ah" parm nomemory [eax] modify nomemory [] value [eax]

#pragma aux rdtsc = \
			"db 0fh,31h" \
			"mov 0[ebx],eax" \
			"mov 4[ebx],edx" \
			parm nomemory [ebx] modify exact [eax edx];

#pragma aux fsw = \
			"xor eax,eax" \
			0xdf 0xe0						 /* fnstsw ax */ \
			modify exact nomemory [ eax ];

#pragma aux fsave = \
			".8087" \
			"fsave [eax]" \
			parm nomemory [ eax ] \
			modify exact [ ];

#pragma aux fnsave = \
			".8087" \
			"fnsave [eax]" \
			parm nomemory [ eax ] \
			modify exact [ ];

#pragma aux frstor = \
			".8087" \
			"frstor [eax]" \
			parm [ eax ] \
			modify exact nomemory [ ];

#pragma aux bsr =\
			"bsr eax,eax"\
			parm nomemory [ eax ]\
			modify exact nomemory [ eax ];

#pragma aux bsr0 =\
			"bsr eax,eax"\
			"jnz l1"\
			"xor eax,eax"\
			"l1:"\
			parm nomemory [ eax ]\
			modify exact nomemory [ eax ];

#pragma aux smp_locked_or =\
			"lock or [eax],edx"\
			parm [eax] [edx]\
			modify exact [ ];

#pragma aux smp_locked_and =\
			"lock and [eax],edx"\
			parm [eax] [edx]\
			modify exact [ ];

#else

#pragma aux delay_8X = "here: loop here" parm nomemory [cx] modify nomemory exact [cx]

#pragma aux pswget = "pushf" "pop ax" parm nomemory [] modify nomemory [] value  [ax]
#pragma aux restore = "popf" parm routine nomemory [] modify nomemory exact [];
#pragma aux disable = "pushf" "pop ax" "cli" parm nomemory [] modify exact [ax] value [ax]
#pragma aux enable = "pushf" "sti" "pop ax" parm nomemory [] modify exact [ax] value [ax]
#pragma aux str_forward = "pushf" "cld" "pop ax" parm nomemory [] modify exact [ax] value [ax]
#pragma aux str_reverse = "pushf" "std" "pop ax" parm nomemory [] modify exact [ax] value [ax]

#pragma aux segsize = "lsl ax,ax" parm nomemory [ax] modify exact nomemory [ax] value [ax]
            
#pragma aux swapl = \
            "xchg al,ah" \
            "xchg dl,dh" \
            "xchg ax,dx" \
            parm nomemory [dx ax] \
            modify exact nomemory [dx ax] \
			value [dx ax]

#pragma aux memcpy8 =\
			"push ds" \
			"mov ds,dx" \
			"rep movsb" \
			"pop ds" \
			parm [es di] [dx si] [cx] \
			modify exact [si di cx]
    
#pragma aux memcpy16 =  \
			"push ds" \
			"mov ds,dx" \
			"shr cx,1" \
			"rep movsw" \
			"pop ds" \
			parm [es di] [dx si] [cx] \
			modify exact [si di cx]
    
#pragma aux memcpy32 =  \
			".386p" \
			"push ds" \
			"mov ds,dx" \
			"shr cx,2" \
			"rep movsd" \
			"pop ds" \
			parm [es di] [dx si] [cx] \
			modify exact [si di cx]
#endif

#elif defined(__GNUC__) || defined(__INTEL_COMPILER)

#define breakpoint()	({ __asm__ __volatile__ ("int $0x03": :); })
#define trppoint()		({ __asm__ __volatile__ ("int $0x20": :); })

#define segsize(__seg)	({ register _Uint32t __ret; __asm__( \
						"lsll %1,%0" \
						: "=r" (__ret) : "r" ((unsigned)__seg)); __ret; })
						
#define _cs() ({register _Uint16t __seg; __asm__ ( "movw %%cs,%0": "=qm" (__seg):); __seg; })
#define _ds() ({register _Uint16t __seg; __asm__ ( "movw %%ds,%0": "=qm" (__seg):); __seg; })
#define _es() ({register _Uint16t __seg; __asm__ ( "movw %%es,%0": "=qm" (__seg):); __seg; })
#define _fs() ({register _Uint16t __seg; __asm__ ( "movw %%fs,%0": "=qm" (__seg):); __seg; })
#define _gs() ({register _Uint16t __seg; __asm__ ( "movw %%gs,%0": "=qm" (__seg):); __seg; })
#define _ss() ({register _Uint16t __seg; __asm__ ( "movw %%ss,%0": "=qm" (__seg):); __seg; })

#define _verr(__seg)	({ register _Uint8t __ret; _Uint16t __tmp = __seg; __asm__( \
						"verr %w1\n\t" \
						"sete %b0" \
						: "=q" (__ret) : "q" (__tmp)); __ret; })

#define verr(__p,__len)	(_verr(_ds()) ? segsize(_ds()) >= ((_Intptrt)(__p) + (__len)) : 0)

#define _verw(__seg)	({ register _Uint8t __ret; _Uint16t __tmp = __seg; __asm__( \
						"verr %w1\n\t" \
						"sete %b0" \
						: "=q" (__ret) : "q" (__tmp)); __ret; })

#define verw(__p,__len)	(_verw(_ds()) ? segsize(_ds()) >= ((_Intptrt)(__p) + (__len)) : 0)

#define io_safe()		({ __asm__ __volatile__("jmp 1f\n1:\tjmp 1f\n1:"); })

#define swapw(__val)	({ _Uint16t __tmp = __val; __asm__("xchgb %b0, %h0": "=q" (__tmp): "0" (__tmp)); __tmp; })
#define swapl(__val)	({ _Uint32t __tmp = __val; __asm__("xchgb %b0, %h0\n\trorl $16,%0\n\txchgb %b0,%h0": "=q" (__tmp): "0" (__tmp)); __tmp; })

#define fninit()		({ __asm__ __volatile__ ("fninit": :); })
#define fnclex()		({ __asm__ __volatile__ ("fnclex": :); })

#define ldds(__seg)		(({_Uint16t __tmp = (__seg); __asm__ ( "movw %0,%%ds": :"rm" (__tmp)); }))
#define ldes(__seg)		(({_Uint16t __tmp = (__seg); __asm__ ( "movw %0,%%es": :"rm" (__tmp)); }))
#define ldfs(__seg)		(({_Uint16t __tmp = (__seg); __asm__ ( "movw %0,%%fs": :"rm" (__tmp)); }))
#define ldgs(__seg)		(({_Uint16t __tmp = (__seg); __asm__ ( "movw %0,%%gs": :"rm" (__tmp)); }))
#define ldesp(__esp)	(({__asm__ ( "movl %0,%%esp": :"g" (__esp)); }))

#define delay_8X(__num)	({register _Uint32t __tmp; __asm__ __volatile__("0: loop 0b": "=c" (__tmp) : "0" (__num)); })
		
#define pswget()		({register _Uint32t __flags; __asm__ __volatile__ ( \
						"pushfl\n\tpop %0" \
						: "=r" (__flags) :); __flags; }) 

#define restore(__fl)	(({__asm__ __volatile__ ("push %0\n\tpopfl": : "rm" (__fl): "cc"); }))

#define disable()		({register _Uint32t __flags; __asm__ __volatile__ ( \
						"pushfl\n\t" \
						"pop %0\n\t" \
						"cli\n\t" \
						: "=r" (__flags) : : "memory"); __flags; })

#define enable()		({register _Uint32t __flags; __asm__ __volatile__ ( \
						"pushfl\n\t" \
						"sti\n\t" \
						"pop %0\n\t" \
						: "=r" (__flags) : : "memory"); __flags; })

#define str_forward()	({register _Uint32t __flags; __asm__ __volatile__ ( \
						"pushfl\n\t" \
						"cld\n\t" \
						"pop %0\n\t" \
						: "=r" (__flags) : : "memory"); __flags; })

#define str_reverse()	({register _Uint32t __flags; __asm__ __volatile__ ( \
						"pushfl\n\t" \
						"std\n\t" \
						"pop %0\n\t" \
						: "=r" (__flags) : : "memory"); __flags; })

static __inline__ void __attribute__((__unused__)) rdtsc(_Uint64t *__ptr) {
	struct __hilo { unsigned __lo, __hi; } *__num = (struct __hilo *)__ptr;
	__asm__ __volatile__ (
		"rdtsc\n\t"
		"movl %%eax,%0\n\t"
		"movl %%edx,%1\n\t"
		: "=m" (__num->__lo), "=m" (__num->__hi): :"eax", "edx");
}

#define fsw()			({ register _Uint32t __st; __asm__("xorl %0,%0\n\tfnstsw %0": "=a" (__st):); __st; })
#define fsave(__p)		({ __asm__("fsave %0": "=m" (__p):); })
#define fnsave(__p)		({ __asm__("fnsave %0": "=m" (__p):); })
#define frstor(__p)		({ __asm__("frstor %0": "=m" (__p):); })

#define bsr(__num)		({ _Uint32t __tmp = __num; __asm__("bsr %0,%0": "=r" (__tmp): "0" (__tmp)); __tmp; })
#define bsr0(__num)		({ _Uint32t __tmp = __num; __asm__("bsr %0,%0\n\tjnz 0f\n\txor %0,%0\n0:": "=r" (__tmp): "0" (__tmp)); __tmp; })

/*
 * Make sure gcc doesn't try to be clever and move things around
 * on us. We need to use _exactly_ the address the user gave us,
 * not some alias that contains the same information.
 */
#ifndef __atomic_fool_gcc
struct __gcc_fool { int __fool[100]; };
#define __atomic_fool_gcc(__x) (*(volatile struct __gcc_fool *)__x)
#endif

static __inline void __attribute__((__unused__)) smp_locked_or(volatile unsigned *__dst, unsigned __value) {
	__asm__ __volatile__(
		"lock; orl %2,%0"
		:"=m" (__atomic_fool_gcc(__dst))
		:"m" (__atomic_fool_gcc(__dst)), "ir" (__value)
		:"memory");
}

static __inline void __attribute__((__unused__)) smp_locked_and(volatile unsigned *__dst, unsigned __value) {
	__asm__ __volatile__(
		"lock; andl %2,%0"
		:"=m" (__atomic_fool_gcc(__dst))
		:"m" (__atomic_fool_gcc(__dst)), "ir" (__value)
		:"memory");
}

#elif defined(__MWERKS__)

#define __MWINL inline __declspec(naked)

static __MWINL void breakpoint(void) { asm { int 0x3 } }

static __MWINL void trppoint(void) { asm { int 0x20 } }

static __MWINL _Uint32t segsize(unsigned __seg) {
	asm {
		mov ax, __seg
		lsl eax, ax
	}
}
static __MWINL _Uint16t _cs(void) { asm { mov ax,cs } }
static __MWINL _Uint16t _ds(void) { asm { mov ax,ds } }
static __MWINL _Uint16t _es(void) { asm { mov ax,es } }
static __MWINL _Uint16t _fs(void) { asm { mov ax,fs } }
static __MWINL _Uint16t _gs(void) { asm { mov ax,gs } }
static __MWINL _Uint16t _ss(void) { asm { mov ax,ss } }
static __MWINL _Uint8t  _verr(unsigned __seg) {
	asm {
		mov ax, __seg
		verr ax
		sete al
	}
}
static __MWINL _Uint8t  _verw(unsigned __seg) {
	asm {
		mov ax, __seg
		verw ax
		sete al
	}
}

#define verr(__p,__len)	(_verr(_ds()) ? segsize(_ds()) >= ((_Intptrt)(__p) + (__len)) : 0)
#define verw(__p,__len)	(_verw(_ds()) ? segsize(_ds()) >= ((_Intptrt)(__p) + (__len)) : 0)
static __MWINL void io_safe(void) {
	asm {
		jmp L1
	L1: jmp L2
	L2:
	}
}
static __MWINL _Uint16t swapw(_Uint16t __val) {
	asm {
		mov ax, __val
		xchg ah,al
	}
}

static __MWINL _Uint32t swapl(_Uint32t __val) {
	asm {
		mov eax, __val
		xchg ah,al
		ror  eax,16
		xchg ah,al	
	}
}

static __MWINL void fninit(void) { asm { fninit } }
static __MWINL void fnclex(void) { asm { fnclex } }

static __MWINL void ldds(unsigned __seg) {
   asm {
  	 mov ax, __seg
  	 mov ds, ax
   }
}
static __MWINL void ldes(unsigned __seg) {
   asm {
  	 mov ax, __seg
  	 mov es, ax
   }
}
static __MWINL void ldfs(unsigned __seg) {
   asm {
  	 mov ax, __seg
  	 mov fs, ax
   }
}
static __MWINL void ldgs(unsigned __seg) {
   asm {
  	 mov ax, __seg
  	 mov gs, ax
   }
}
static __MWINL void ldesp(unsigned __nsp) { asm { mov esp, __nsp } }
static void __MWINL delay_8X(unsigned __n) {
	asm {
		push ecx
		mov ecx,__n
		L1: loop L1
		pop ecx
	}
}
static __MWINL _Uint32t pswget(void) {
	asm {
		pushf
		pop eax
	}
}
static __MWINL void restore(_Uint32t __fl) {
	asm {
		push __fl
		popf
	}
}
static __MWINL _Uint32t disable(void) {
	asm {
		pushf
		pop  eax
		cli
	}
}
static __MWINL _Uint32t enable(void) {
	asm {
		pushf
		sti
		pop eax
	}
}
static __MWINL _Uint32t str_forward(void) {
	asm {
		pushf
		cld
		pop eax
	}
}
static __MWINL _Uint32t str_reverse(void) {
	asm {
		pushf
		std
		pop eax
	}
}
static __MWINL void rdtsc(_Uint64t *__ptr) {
	asm {
		push ebx
		push edx
		rdtsc
		mov ebx,__ptr
		mov [ebx], eax
		mov 4[ebx], edx
		pop edx
		pop ebx
	}
}
static __MWINL _Uint32t fsw(void) {
	asm {
		xor eax,eax
		fnstsw eax
	}
}
static __MWINL void fsave(void *__fsav) {
	asm {
		mov eax, __fsav
		fsave eax
	}
}
static __MWINL void fnsave(void *__fsav) {
	asm {
		mov eax, __fsav
		fnsave eax
	}
}
static __MWINL void frstor(void *__fsav) {
	asm {
		mov eax, __fsav
		frstor eax
	}
}
static __MWINL _Uint32t bsr(_Uint32t __v) {
	asm {
		bsr eax,__v
	}
}
static __MWINL _Uint32t bsr0(_Uint32t __v) {
	asm {
		bsr eax, __v
		jnz L1
		xor eax,eax
		L1:
	}
}

static __MWINL void smp_locked_or(volatile unsigned *__vp, unsigned __value) {
	asm {
		push edx
		mov eax, __vp
		mov edx, __value
		lock: or [eax],edx
		pop edx
	}
}

static __MWINL void smp_locked_and(volatile unsigned *__vp, unsigned __value) {
	asm {
		push edx
		mov eax, __vp
		mov edx, __value
		lock: and [eax],edx
		pop edx
	}
}
#undef __MWINL

#else
#error Compiler not defined.
#endif


__END_DECLS

#endif

/* __SRCVERSION("inline.h $Rev: 200568 $"); */
