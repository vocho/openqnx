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

//
// Define various SMP specific macros that are shared between the kernel
// and proc
//
#if defined(COMPILING_MODULE)
	//Source for modules always has to use the SMP versions of the
	//macros.
	#define WANT_SMP_MACROS
#endif
#if defined(VARIANT_smp)
	#define WANT_SMP_MACROS
#endif
#if defined(WANT_SMP_MACROS)
	#ifndef PROCESSORS_MAX
	   #define PROCESSORS_MAX		8
	#endif
	#define NUM_PROCESSORS			num_processors
	#define RUNCPU					get_cpunum()
	#define SPINLOCK(spin)			do { while((spin)->value) { /* nothing to do */ } } while(_smp_xchg(&(spin)->value, 1) != 0)
	#define SPINUNLOCK(spin)		((spin)->value = 0)
	#define SENDIPI(cpu,cmd)		send_ipi(cpu,cmd)
	#define SMP_FLUSH_TLB()			if(num_processors > 1) smp_flush_tlb()
	#define SMP_SPINVAR(class,var)	class intrspin_t var
	#define INTR_LOCK(s)			InterruptLock(s)
	/* the following ensure we don't enable interrupts during kernel initialization */
	#define INTR_UNLOCK(s)			InterruptUnlock(s)
	#define MEM_BARRIER_RD()		__cpu_membarrier()
	#define MEM_BARRIER_WR()		__cpu_membarrier()
	#define MEM_BARRIER_RW()		__cpu_membarrier()
#else
	#define PROCESSORS_MAX			1
	#define NUM_PROCESSORS			1U
	#define RUNCPU					0
	#define SPINLOCK(spin)			
	#define SPINUNLOCK(spin)		
	#define SENDIPI(cpu,cmd)		
	#define SMP_FLUSH_TLB()		
	#if defined(_lint)
		#define SMP_SPINVAR(class,var) _to_semi
	#else
		#define SMP_SPINVAR(class,var)
	#endif	
	#define INTR_LOCK(s)			InterruptDisable()
	/* the following ensure we don't enable interrupts during kernel initialization */
	#define INTR_UNLOCK(s)			InterruptEnable()
	#define MEM_BARRIER_RD()		
	#define MEM_BARRIER_WR()	
	#define MEM_BARRIER_RW()
#endif

#if PROCESSORS_MAX >= 32
	// Have to use this slightly more complicated formula to calculate
	// the bitmask if we can have 32 CPU's because a lot of processors
	// 'and' the shift count with 0x1f, which makes a shift count of 32 
	// become one of zero.
	#define LEGAL_CPU_BITMASK ((0x2 << (NUM_PROCESSORS-1))-1)
#else
	#define LEGAL_CPU_BITMASK ((0x1 << NUM_PROCESSORS)-1)
#endif

//
// IPI commands for the SMP kernel.
//
#define	IPI_RESCHED			0x00000001
#define IPI_TIMESLICE		0x00000002
#define IPI_TLB_FLUSH		0x00000004
#define IPI_TLB_SAFE		0x00000008
#define IPI_CONTEXT_SAVE	0x00000010
#define IPI_CHECK_INTR      0x00000020
#define IPI_PARKIT			0x00000040
#define IPI_INTR_UNMASK     0x00000080
#define IPI_INTR_MASK  		0x00000100
#define IPI_CLOCK_LOAD 		0x00000200
#define IPI_CPU_SPECIFIC	0xffff0000

/* __SRCVERSION("smpswitch.h $Rev: 174913 $"); */
