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

#ifndef _PPC_INLINE_H_INCLUDED
#define _PPC_INLINE_H_INCLUDED

/*
    some handy pragma's for low-level work: 
*/

#ifndef _PPC_INOUT_INCLUDED
#include <ppc/inout.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

	/* returns the current value of the MSR register */
#define get_msr()	({unsigned msr; __asm__ __volatile__(	"mfmsr	%0"	: "=r" (msr) ); msr; })

	/* sets the MSR resgister */
static __inline__ void __attribute__((__unused__))
set_msr(unsigned __msr) {
	__asm__ __volatile__( "mtmsr	%0"	: : "r" (__msr) );
}

	/* Get or set a Special Purpose Register */
#define get_spr(spr)	({unsigned __val; __asm__ __volatile__( "mfspr %0,%1" : "=r" (__val) : "i" (spr) ); __val; })
#define set_spr(spr,val) __asm__ __volatile__( "mtspr %0,%1" : : "i" (spr), "r" (val) )

	/* Get or set a Segment Register */
#define get_sreg(index)          	\
	({unsigned __val;               \
	__asm__ __volatile__(           \
	".ifdef PPC_CPUOP_ENABLED;"     \
	".cpu ppc;"                     \
	".endif;"                       \
	"mfsrin %0,%1" :                \
	"=r" (__val) : "r" (index) );   \
	__val; })

#define set_sreg(index,val)	        \
	__asm__ __volatile__(           \
	".ifdef PPC_CPUOP_ENABLED;"     \
	".cpu ppc;"                     \
	".endif;"                       \
	"mtsrin %1,%0" :                \
	: "r" (index), "r" (val) )

	/* Get or set a Device Control Register */
#define get_dcr(dcr)						\
		({unsigned __val; 					\
		__asm__ __volatile__(				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu 403;"						\
			".endif;"						\
			"mfdcr %0,%1" 					\
			: "=r" (__val) : "i" (dcr) );	\
		__val; })
#define set_dcr(dcr,val) 					\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu 403;"						\
			".endif;"						\
			"mtdcr %0,%1" 					\
			: : "i" (dcr), "r" (val) )

	/* Get the Processor Version Register */
#define get_pvr()	({unsigned __pvr; __asm__( "mfpvr %0": "=r" (__pvr) ); __pvr; })

	/* load up the stack pointer */
#define ldesp(stack) __asm__ __volatile__( "mr %%r1,%0" : : "r" (stack))

#define ppc_eieio() __asm__ __volatile__( "eieio" )
#define ppc_tlbia() __asm__ __volatile__( "tlbia" )
#define ppc_tlbie(addr) __asm__ __volatile__( "tlbie %0" : : "r"(addr) )
#define ppc_tlbsync() __asm__ __volatile__( "tlbsync" )
#define ppc_sync() __asm__ __volatile__( "sync" )
#define ppc_isync() __asm__ __volatile__( "isync" )
#define ppc_lwsync()						\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu ppc64;"					\
			".endif;"						\
			"lwsync")			
#define ppc_ptesync()						\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu ppc64;"					\
			".endif;"						\
			"ptesync")			

#define icache_flush(addr) __asm__ __volatile__( "dcbst 0,%0; sync; icbi 0,%0; isync" : : "r" (addr) )

#define _cntlzw(val)	({int __lz; __asm__( "cntlzw %0,%1" : "=r" (__lz) : "r" (val) ); __lz; })

#define bsr(v)		(31 - _cntlzw(v))
#define bsr0(v)		(((v) == 0) ? 0 : bsr(v))

#if __INT_BITS__ == 32
#define get_spr64(spr)						\
		({unsigned  __val0; 				\
		 unsigned	__val1;					\
		 unsigned	__msr;					\
		 unsigned	__msrnew;				\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu ppc64;"					\
			".endif;"						\
			"mfmsr	%2;"					\
			"rlwinm	%3,%2,0,17,15;"			\
			"mtmsr	%3;"					\
			"mfspr	%0,%4;"					\
			"rldicl	%1,%0,32,32;"			\
			"mtmsr	%2;"					\
			: "=r" (__val0), "=r" (__val1), "=&r" (__msr), "=&r" (__msrnew) \
			: "i" (spr) ); 					\
		__val0 | (((_Uint64t)(__val1)) << 32); })
#define set_spr64(spr,v)					\
		({unsigned	__msr;					\
		 unsigned	__msrnew;				\
		 unsigned long long val = (v);		\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu ppc64;"					\
			".endif;"						\
			"mfmsr	%0;"					\
			"rlwinm	%1,%0,0,17,15;"			\
			"mtmsr	%1;"					\
			"rldicr	%4,%4,32,31;"			\
			"rldimi %4,%3,0,32;"			\
			"mtspr	%2,%4;"					\
			"mtmsr	%0;"					\
			: "=&r" (__msr), "=&r" (__msrnew) \
			: "i" (spr), "r" ((unsigned)(val)), "r" ((unsigned)((val) >> 32)));\
		})
#define get_msr64()							\
		({unsigned  __val0; 				\
		 unsigned	__val1;					\
		 unsigned	__msrnew;				\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu ppc64;"					\
			".endif;"						\
			"mfmsr	%0;"					\
			"rlwinm	%2,%0,0,17,15;"			\
			"mtmsr	%2;"					\
			"mfmsr  %2;"					\
			"rldicl	%1,%2,32,32;"			\
			"mtmsr	%0;"					\
			: "=r" (__val0), "=r" (__val1), "=&r" (__msrnew)); \
		__val0 | (((_Uint64t)(__val1)) << 32); })
#define set_msr64(v)						\
		({unsigned	__msrnew;				\
		 unsigned long long val = (v);		\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu ppc64;"					\
			".endif;"						\
			"mfmsr	%0;"					\
			"rlwinm	%0,%0,0,17,15;"			\
			"mtmsr	%0;"					\
			"rldicr	%2,%2,32,31;"			\
			"rldimi %2,%1,0,32;"			\
			"mtmsrd	%2;"					\
			: "=&r" (__msrnew) 				\
			: "r" ((unsigned)(val)), "r" ((unsigned)((val) >> 32)));\
		})
#else
#define get_spr64(spr)		get_spr(spr)		
#define set_spr64(spr,val)	set_spr(spr,val)		
#define get_msr64()			get_msr()	
#define set_msr64(val)		set_msr(val)	
#endif

#define ppc_slbia()							\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu ppc64;"					\
			".endif;"						\
			"slbia");

#define ppc_slbie(rb)						\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu ppc64;"					\
			".endif;"						\
			"slbie %0"						\
			:: "r" ((unsigned)(rb)));

#define ppc_slbmte(rs, rb)					\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu ppc64;"					\
			".endif;"						\
			"slbmte %0, %1"					\
			:: "r" ((unsigned)(rs)), "r" ((unsigned)(rb)));

#define ppc_slbmfev(rs, rb)					\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu ppc64;"					\
			".endif;"						\
			"slbmfev %0, %1"				\
			: "=r" ((rs))					\
			: "r" ((unsigned)(rb)));

#define ppc_slbmfee(rs, rb)					\
		__asm__ __volatile__( 				\
			".ifdef PPC_CPUOP_ENABLED;"		\
			".cpu ppc64;"					\
			".endif;"						\
			"slbmfee %0, %1"				\
			: "=r" ((rs))					\
			: "r" ((unsigned)(rb)));

__END_DECLS

#endif

/* __SRCVERSION("inline.h $Rev: 169331 $"); */
