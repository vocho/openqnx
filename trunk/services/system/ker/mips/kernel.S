#
# $QNXLicenseA:
# Copyright 2007, QNX Software Systems. All Rights Reserved.
# 
# You must obtain a written license from and pay applicable license fees to QNX 
# Software Systems before you may reproduce, modify or distribute this software, 
# or any work that includes all or part of this software.   Free development 
# licenses are available for evaluation and non-commercial purposes.  For more 
# information visit http://licensing.qnx.com or email licensing@qnx.com.
#  
# This file may contain contributions from others.  Please review this entire 
# file for other proprietary rights or license notices, as well as the QNX 
# Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
# for other information.
# $
#

/*
 * kernel.s
 *	Functions best implemented in assembly, usually for speed
 */

/*
 * Get global defs.  Remove conflicting names from other architectures
 */
#include "asmoff.def"
#include <mips/asm.h>
#include <mips/cpu.h>
#include <mips/opcode.h>
#include <mips/vm.h>
#include <asm_variant.h>
#include "loadstore.h"
#include "getcpu.h"
#include "dbgmacros.h"


/*
 * Some strangeness explained. The TX79 chip has 128-bit general purpose
 * registers. We've only allocated enough space for 64 bits in the MIPS
 * cpu context structure. To keep that the same so that userland application
 * programs that access the context (e.g. debuggers) can ignore that
 * fact, we're going to store the high portions of the TX79 context
 * _below_ the standard MIPS context area. That means that register context
 * pointers in this file actually point to the _middle_ of the full context
 * structure. Following the pointer is the standard MIPS_CPU_REGISTERS
 * structure. Just in front is the MIPS_ALT_REGISTERS one. Access to the
 * high portions in here are done with negative offsets to the context
 * pointer. Storage for the high area is arranged by locating a
 * MIPS_ALT_REGISTER structure as the last thing in the 'cpu' file of
 * a THREAD structure (and the 'cpu' field has to be just in front of
 * the standard context area (was required for the X86 anyway)). For
 * context's stored on the kernel stack, we just have to allocate more
 * memory and point the register in the middle of it.
 */
#define ABI_CALL_OVERHEAD		16
#define MIPS_CONTEXT_HI_SIZE	MIPS_CONTEXT_LO_START
#define MIPS_CONTEXT_EXTRA_SIZE	(MIPS_CONTEXT_HI_SIZE+ABI_CALL_OVERHEAD)

	/*
	 * Externals that we reference.  Note that we try to identify
	 * short data sizes, to get GP-relative addressing.
	 */
	.extern	_gp
	.extern	queued_event_priority,4
	.extern	xfer_handlers,4*PROCESSORS_MAX
	.extern actives_prp,4*PROCESSORS_MAX
	.extern aspaces_prp,4*PROCESSORS_MAX
	.extern actives_fpu,4*PROCESSORS_MAX
	.extern cpupageptr,4*PROCESSORS_MAX
	.extern alives,1*PROCESSORS_MAX
	.extern intrevent_pending,4
	.extern ker_call_table,4*__KER_BAD
	.extern debug_attach_brkpts,4
	.extern debug_detach_brkpts,4
	.extern run_ker_stack_bot,4
	.extern run_ker_stack_top,4
	.extern	load_linked_addr,4
	.extern __mips_cause2sig_map,4*64
	.extern memmgr,SIZEOF_MEMMGR_ENTRY
	.extern	inspecret,4
	.extern kercallptr,4
	.extern sys_kercallptr,4
	.extern resched,4
#if defined(VARIANT_instr)
	.extern _trace_call_table,4*__KER_BAD
	.extern ker_exit_enable_mask,4
	.extern int_exit_enable_mask,4
	.extern int_enter_enable_mask,4
#endif
	
	.extern emulate_instruction
	.extern usr_fault
	.extern shutdown
	.extern kdebug_callout
	.extern kererr
	.extern intrevent_drain
	.extern clock_load
	.extern begin_fp_emulation
	.extern fpu_emulation_prep

	.set MIPSARCH
	.set noreorder


#if defined(VARIANT_32) || defined(VARIANT_tx79) || defined(VARIANT_r3k)
	#if defined(VARIANT_smp)
		#error SMP needs a 64-bit CONTEXT register
	#endif
	#define GET_L1PAGETABLE_1(reg)	lui		reg,%hi(l1pagetable)
	#define GET_L1PAGETABLE_2(reg)	lw		reg,%lo(l1pagetable)(reg)
	#define SET_L1PAGETABLE(reg)	sw		reg,l1pagetable
#else
	#define GET_L1PAGETABLE_1(reg)	dmfc0	reg,CP0_CONTEXT
	#define GET_L1PAGETABLE_2(reg)  dsra	reg,32
	#define SET_L1PAGETABLE(reg)	dsll    reg,32; dmtc0 reg,CP0_CONTEXT
#endif
	
#if defined(VARIANT_smp)
	.extern	need_to_run,4
	.extern need_to_run_cpu,4
	.extern ker_stack_bot,4
	.extern ker_stack_top,4

	#define BRANCH_INKERNEL(reg,sys_lbl)	\
		bgez	sp,999f;					\
		 lui	reg,%hi(ker_stack_top);		\
		lw		reg,%lo(ker_stack_top)(reg);\
		sltu	reg,sp,reg;					\
		beq		reg,zero,999f;				\
		 lui	reg,%hi(ker_stack_bot);		\
		lw		reg,%lo(ker_stack_bot)(reg);\
		sltu	reg,sp,reg;					\
		beq  	reg,zero,sys_lbl;			\
		999:
		
	#define SMPADDR(areg,cpu_x4)	addu areg,cpu_x4
		
	#define SMPREF(ins,reg,var,cpu_x4)	\
		ins 	reg,var(cpu_x4)		
		
	#define FORCED_KERNEL_RESTORE(tmp,act)	\
		lw		tmp,ARGS_ASYNC_TYPE(act);	\
		SAVE_ONE_REG(tmp,V0,REG_OFF,act);	\
		lw		tmp,ARGS_ASYNC_IP(act);		\
		SAVE_ONE_REG(tmp,EPC,REG_OFF,act)
		
	#define CLEAR_FPUPTR(reg)		\
		ori		reg,(~FPUDATA_MASK & 0xFFFFFFFF);	\
		xori	reg,(~FPUDATA_MASK & 0xFFFFFFFF)
		
	// store in second byte
	#define NEW_INKERNEL_BITS(reg,bits) \
		li	reg,(bits) >> 8;	\
		sb	reg,inkernel+1+MIPS_REGS_LOW_WORD; \
		sync
		
	// store in second byte
	#define NEW_INKERNEL_BITS_REG(tmp,reg) \
		.ifc reg,zero;	\
			sb	zero,inkernel+1+MIPS_REGS_LOW_WORD;	\
		.else;			\
			srl	tmp,reg,8;\
			sb	tmp,inkernel+1+MIPS_REGS_LOW_WORD;	\
		.endif;	\
		sync
		
	#define ACQUIRE_KERNEL(bits,or_in_reg,must_be_usermode)	\
		.if must_be_usermode;								\
			jal		usr_acquire_kernel;						\
			 li		a0,bits;								\
		.else;												\
			la		ra,999f;								\
			beq		s6,zero,sys_acquire_kernel;				\
			 li		a0,bits;								\
			j		usr_acquire_kernel;						\
			 nop;											\
			999:;											\
		.endif
	
	.extern intr_slock,4
	
	#define ACQUIRE_INTR_SLOCK(tmp0,tmp1)	\
		la		tmp0,intr_slock;			\
		999:;								\
		ll		tmp1,(tmp0);				\
		bne		tmp1,zero,999b;				\
		 li		tmp1,1;						\
		sc		tmp1,(tmp0);				\
		beq		tmp1,zero,999b;				\
		 nop
	
	#define RELEASE_INTR_SLOCK	sw zero,intr_slock; sync
	
#else

	#define BRANCH_INKERNEL(reg,sys_lbl) \
		GET_INKERNEL(reg,1); \
		bne  reg,zero,sys_lbl
		
	#define SMPADDR(areg,cpu_x4)	
		
	#define SMPREF(ins,reg,var,cpu_x4)	\
		ins 	reg,var		
		
	#define CLEAR_FPUPTR(reg)		
	
	#define NEW_INKERNEL_BITS(reg,bits) \
		li	reg,(bits);	\
		SET_INKERNEL(reg)
		
	#define NEW_INKERNEL_BITS_REG(tmp,reg) \
		SET_INKERNEL(reg)
		
	#define ACQUIRE_KERNEL(bits,or_in_reg,must_be_usermode)	\
		.ifnc or_in_reg,zero;		\
		   GET_INKERNEL(or_in_reg,0);\
		.endif;						\
		ori	a0,or_in_reg,bits;		\
		SET_INKERNEL(a0)
		
	#define ACQUIRE_INTR_SLOCK(tmp0,tmp1)
	
	#define RELEASE_INTR_SLOCK	
	
#endif

#if defined(SMP_MSGOPT)
	#define MSGOPT_SMPADDR(r1,r2)	SMPADDR(r2,r2)
#else
	#define MSGOPT_SMPADDR(r1,r2)	
#endif
		
//
// If I could figure out a way to ensure that actives & ker_stack are
// in the same 64K block of memory I could spiffy up this sequence a bit.
//		bstecher
//
#define LD_ACTIVE_AND_KERSTACK(act_reg, stk_reg)	\
	lui		stk_reg,%hi(ker_stack);	\
	GETCPU_1(act_reg,2);			\
	addiu	stk_reg,%lo(ker_stack);	\
	GETCPU_2(act_reg,2);			\
	SMPADDR(stk_reg,act_reg);		\
	lw		act_reg,KER_STACK_TO_ACTIVES_DISTANCE(stk_reg); \
	lw		stk_reg,(stk_reg)


//Kill the load-linked instruction address so any SC instructions
//following will fail and be recycled.
#define KILL_LOAD_LINKED(reg) li reg,-1; sw	reg,load_linked_addr

	
.sdata

//
// KLUDGE: Due to a bug in the MIPS assembler, the definition of inkernel
//         MUST be the first thing in the sdata section. See PR-12671
//         for details. Once it's fixed, this comment can be removed.
//
.globl inkernel
inkernel:	.long 0

#if defined(VARIANT_smp)
// Make cpunum be the top byte of inkernel.
.globl cpunum
.equ	cpunum,inkernel+3-(MIPS_REGS_LOW_WORD*3)
#endif

//
// Define actives & ker_stack here so that the assembler knows the
// distance between them as a constant.
//
.globl ker_stack
ker_stack:	.space	4*PROCESSORS_MAX
.globl actives
actives:	.space	4*PROCESSORS_MAX

#define KER_STACK_TO_ACTIVES_DISTANCE (4*PROCESSORS_MAX)

	/*
 	 * Exception Table 
	 */
	.globl	r4k_exception_table

r4k_exception_table:

	.word	0 						/* 0  : interrupt (filled in later)	*/
	.word	r4k_tlb_handler			/* 1  : TLB modification	*/
	.word	r4k_tlb_handler			/* 2  : TLB exception (load)	*/
	.word	r4k_tlb_handler			/* 3  : TLB exception (store)	*/
	.word	r4k_addrerr_handler		/* 4  : Address error (load)	*/
	.word	r4k_addrerr_handler		/* 5  : Address error (store)	*/
	.word	r4k_exception_handler	/* 6  : Bus Error (instr) 	*/
	.word	r4k_exception_handler	/* 7  : Bus Error (data)	*/
	.word	r4k_syscall_handler		/* 8  : System call		*/
	.word	r4k_brkpt_handler		/* 9  : Breakpoint		*/
	.word	r4k_illop_handler		/* 10 : Illegal op		*/
	.word	0						/* 11 : Coprocessor unusable (filled in later)	*/
	.word	r4k_exception_handler	/* 12 : Arithmetic overflow	*/
	.word	r4k_exception_handler	/* 13 : Trap			*/
	.word	r4k_exception_handler	/* 14 : Reserved		*/
	.word	r4k_fpu_exc_handler		/* 15 : Floating FPU exception	*/
	.word	r4k_exception_handler	/* 16 : Reserved		*/
	.word	r4k_exception_handler	/* 17 : Reserved		*/
	.word	r4k_exception_handler	/* 18 : Reserved		*/
	.word	r4k_exception_handler	/* 19 : Reserved 		*/
	.word	r4k_exception_handler	/* 20 : Reserved		*/
	.word	r4k_exception_handler	/* 21 : Reserved		*/
	.word	r4k_exception_handler	/* 22 : Reserved		*/
	.word	r4k_watch_handler		/* 23 : Watchpoint		*/
	.word	r4k_check_handler		/* 24 : machine check	*/
	.word	r4k_exception_handler	/* 25 : Reserved		*/
	.word	r4k_exception_handler	/* 26 : Reserved		*/
	.word	r4k_exception_handler	/* 27 : Reserved		*/
	.word	r4k_exception_handler	/* 28 : Reserved		*/
	.word	r4k_exception_handler	/* 29 : Reserved		*/
	.word	r4k_exception_handler	/* 30 : Reserved		*/
	.word	r4k_exception_handler	/* 31 : Reserved		*/
	.word	r4k_cache_handler		/* 32 : Cache Error		*/
	.word	r4k_exception_handler	/* 33 : TLB exception		*/
	.word	r4k_exception_handler	/* 34 : XTLB exception		*/
	
.text


//
// Mark a block of code to be copied to an exception location
//
.macro EXC_COPY_CODE_START func
	.global	&func
&func:
	.long	1962f - &func - 4
.endm

.macro EXC_COPY_CODE_END
1962:
.endm

	.set	noat
 
/*
 * tlbmiss: 	
 *
 * The tlb refill exception vector. This code does not save registers, 
 * and hence only touches k0 and k1. If successful, it will load the tlb 
 * up with a pair of entries, but it is only guaranteed that one of these
 * entries will be valid (the one satisfying the tlb miss). If the refill
 * is unsuccessful, then it will jump into the exception handler as if
 * it came through the general exception vector.
 */
#define MISS_SB1	1
#define MISS_R7K	2
#define MISS_E9K	3


.macro TLBMISS rtn, var
EXC_COPY_CODE_START	&rtn
	mfc0	k1,CP0_BADVADDR
.ifeq &var - MISS_SB1
	// Deal with the Broadcom SB-1 chip bug "M3" where it might record bad 
	// virtual address information in the EntryHi register. The BadVaddr 
	// register information is good, so we transfer that to EntryHi while 
	// maintaining the ASID portion of the register.
	mfc0	k0,CP0_TLB_HI
	ori		k1,0xfff			// turn off offset bits
	xori	k1,0xfff
	andi	k0,0xff				// isolate ASID portion of entryhi
	or		k0,k1
	mtc0	k0,CP0_TLB_HI		// put proper vaddr & ASID into entryhi
.endif
	GET_L1PAGETABLE_1(k0)			/* get current L1 page table (stage 1) */
	srl 	k1,PT_L1SHIFT - PT_L1INDXSHIFT
	GET_L1PAGETABLE_2(k0)			/* get current L1 page table (stage 2) */
	andi	k1,PT_L1MASK << PT_L1INDXSHIFT
	add		k1,k0,k1
	lw		k0,0(k1)		/* get L2 page table */
	mfc0	k1,CP0_BADVADDR
	beq		k0,zero,1f /* No L2? Head out to general hdlr (saves badva)*/

	 srl 	k1,PT_L2SHIFT - PT_L2INDXSHIFT
#if defined(VARIANT_r3k)
	andi	k1,(PT_L2MASK & ~0) << PT_L2INDXSHIFT
#else
	andi	k1,(PT_L2MASK & ~1) << PT_L2INDXSHIFT
#endif
	add		k1,k0,k1		/* get pointer to pte */
#if defined(VARIANT_r3k)
	lw		k0,0(k1)
	mtc0	k0,CP0_TLB_LO_0
#else	
	lw		k0,4(k1)
	mtc0	k0,CP0_PAGEMASK
	 lw		k0,0(k1)
	lw		k1,8(k1)
	mtc0	k0,CP0_TLB_LO_0
	or		k0,k1
	mtc0	k1,CP0_TLB_LO_1
#endif
	/* PTE(s) zero? Head to general handler (referencing syspage) */
	beq		k0,zero,1f
	 ssnop
.if &var - MISS_SB1	
	ssnop
.endif	
.ifeq &var - MISS_E9K	
	ssnop
.endif	
.ifeq &var - MISS_R7K	
	nop		//R7K Hazard
	nop
	nop
.endif
	tlbwr
#if defined(VARIANT_r3k)
	mfc0	k0,CP0_EPC
	 nop
	j		k0
	 rfe
#else
.if &var - MISS_SB1	
	ssnop
.endif	
.ifeq &var - MISS_E9K	
	ssnop; ssnop
.endif	
.ifeq &var - MISS_R7K	
	sync	//R7K bug workaround
.endif	
	.set mips3
	eret
	.set MIPSARCH
#endif
1:
	la		k0,r4k_tlb_handler
	j		k0
	 nop
EXC_COPY_CODE_END
.endm

	TLBMISS exc_tlbmiss_generic, 0

#if !defined(VARIANT_r3k)
	TLBMISS exc_tlbmiss_sb1, MISS_SB1
	TLBMISS exc_tlbmiss_r7k, MISS_R7K
	TLBMISS exc_tlbmiss_e9k, MISS_E9K
#endif	
	
	
EXC_COPY_CODE_START exc_general
	/*
	 * General exception handler : 0x80000180 (0x80000080 for R3k)
	 */
	mfc0	k1,CP0_CAUSE					# Get cause
	lui		k0,%hi(r4k_exception_table)		# Jump table base
	andi	k1,MIPS_CAUSE_MASK				# Extract index
	addu	k0,k1							# Build table addr
	lw		k0,%lo(r4k_exception_table)(k0)	# Get routine
	j		k0								# Go
	 nop
EXC_COPY_CODE_END	

	
#if !defined(VARIANT_r3k)
EXC_COPY_CODE_START exc_cache_generic	
	/*
	 * Cache error handler : 0xa0000100
	 */
	// Do load in two parts since GP isn't set up yet.
	lui		k1,%hi(r4k_exception_table+0x80)
	lw      k1,%lo(r4k_exception_table+0x80)(k1)
	j       k1                      # jump to the handler
	 nop
EXC_COPY_CODE_END	 

EXC_COPY_CODE_START exc_cache_sr7100	

	/* errata prescribes that you should always set ERL in cache handler */
	mfc0	k0,CP0_SREG
	li		k1,MIPS_SREG_ERL
	or		k0,k1,k0
	mtc0	k0,CP0_SREG

	/*
	 * Cache error handler : 0xa0000100
	 */
	// Do load in two parts since GP isn't set up yet.
	lui		k1,%hi(r4k_exception_table+0x80)
	lw      k1,%lo(r4k_exception_table+0x80)(k1)
	j       k1                      # jump to the handler
	 nop
EXC_COPY_CODE_END	 

/*
 * Special verisons of the general exception vector for SR7100 and SB-1 Cores
 *
 * On these cores, if you get a cache and another imprecise exception
 * (bus, address, etc) 'simultaneously' then you can get into a weird state
 * where the cache exception state is setup, but then before excuting any
 * of the cache exceptoin vector the general exception vector is excutied.  This
 * leaves you with:
 *
 * ErrEPC = faulting address
 * EPC = cache exception vector (0xa0000100)
 * ERL+EXL set in the Status Register
 *
 * SB-1:
 * The error is defined as fatal, so we simply jump to the r4k_cache_handler
 * if we detect this condition.
 *
 * SR7100:
 * The error can be recovered - just knock down the ERL bit.
 *
 */
EXC_COPY_CODE_START exc_general_sb1	

	mfc0	k0,CP0_SREG
	li		k1,MIPS_SREG_ERL
	and		k1,k0
	bgtz	k1,handle_errata
	 nop

	/*
	 * General exception handler : 0x80000180
	 */
	mfc0	k1,CP0_CAUSE					# Get cause
	lui		k0,%hi(r4k_exception_table)		# Jump table base
	andi	k1,MIPS_CAUSE_MASK				# Extract index
	addu	k0,k1							# Build table addr
	lw		k0,%lo(r4k_exception_table)(k0)	# Get routine
	j		k0								# Go
	 nop

handle_errata:
	/* Jump to r4k_cache_handler */
	// Do load in two parts since GP isn't set up yet.
	lui		k1,%hi(r4k_exception_table+0x80)
	lw      k1,%lo(r4k_exception_table+0x80)(k1)
	j       k1                      # jump to the handler
	 nop

EXC_COPY_CODE_END	 

EXC_COPY_CODE_START exc_general_sr7100	

	mfc0	k0,CP0_SREG
	li		k1,MIPS_SREG_ERL
	and		k1,k0
	bgtz	k1,handle_sr7100_errata
	 nop

sr7100_errata_return:

	/*
	 * General exception handler : 0x80000180
	 */
	mfc0	k1,CP0_CAUSE					# Get cause
	lui		k0,%hi(r4k_exception_table)		# Jump table base
	andi	k1,MIPS_CAUSE_MASK				# Extract index
	addu	k0,k1							# Build table addr
	lw		k0,%lo(r4k_exception_table)(k0)	# Get routine
	j		k0								# Go
	 nop

handle_sr7100_errata:
	la		k1,sr7100_errata_workaround
	j       k1                      # jump to the handler
	 nop

EXC_COPY_CODE_END	 

/*
 * Cache err code for Broadcom 12500 chip - it's copied to offset 0x100
 * of the exception table, overlaying the _cache_err code if we're on
 * a broadcom chip. Has two purposes, one is to ignore recoverable cache
 * errors and the other is to handle a PASS 1 chip bug where it reports
 * errors that aren't really there. Can't have more than 32 instructions
 * or it overlaps the general exception vector.
 */
EXC_COPY_CODE_START exc_cache_sb1
//SHOWME('=',0x38,0,k0,k1)
	mfc0	k0,$26
	 li		k1,0x40000000
	bltz	k0,2f		// recoverable cache error
	 nop
	bne		k0,k1,1f	// not a data cache error
#if 0	
//DEBUG
li		k1,0xa0000000
sw		k0,0x30c(k1)
MFC0_SEL_OPCODE(26,26,1)
nop
sw		k0,0x304(k1)
MFC0_SEL_OPCODE(26,27,3)
nop
sw		k0,0x308(k1)
	 MFC0_SEL_OPCODE(26,27,1)
sw		k0,0x300(k1)
//END DEBUG
#endif	
	 li		k1,0xffe00000
	and		k0,k1
	li		k1,0x02000000
	beq		k0,k1,2f	// a spurious error
1:
	// Do load in two parts since GP isn't set up yet.
	 lui	k1,%hi(r4k_exception_table+0x80)
	lw      k1,%lo(r4k_exception_table+0x80)(k1)
	j       k1                      # jump to the handler
	 nop
2:
//SHOWME('_',0x38,0,k0,k1)
	mtc0	zero,$26	// unlock cache error regs
	MTC0_SEL_OPCODE(0,27,1)
#if defined(VARIANT_r3k)
	mfc0	k0,CP0_EPC
	 nop
	j		k0
	 rfe
#else
	.set mips3
	eret
	.set MIPSARCH
#endif
EXC_COPY_CODE_END	


EXC_COPY_CODE_START	exc_intr
	/*
	 * Optimized interrupt exception handler : 0x80000200
	 * Used by TX79 and MIPS32/MIPS64 chips.
	 */
	lui		k0,0x0000	# value patched by gen_intr() in interrupt.c
	ori		k0,0x0000	# value patched by gen_intr() in interrupt.c
	j		k0
	 nop
EXC_COPY_CODE_END
#endif

	.set	at

/*
 * STI()
 *	Enable interrupts
 *
 * On return, the current SREG is in t8.
 *
 * This is the Intel instruction, implemented here as several
 * MIPS instructions.  Apologies to Stephen Belair, who considers
 * using Intel instruction names to be--and I quote--"depraved".
 * 					--Andy Valencia 7/15/97
 */
#define STI ENABLEINTERRUPTS(t8,t7)

/*
 * CLI()
 *	Like STI, but disables interrupts
 *
 * The current SREG is returned in t8.
 */
#define CLI DISABLEINTERRUPTS(t8,t8)

/*
 * KERNEL_MODE()
 *	Change CP0 status so we're in kernel mode
 *
 * E[XR]L are off, interrupts are disabled by MIPS_SREG_IE.  Upper level code
 * can sti() them when ready.
 */
#if defined(VARIANT_r3k)
	#define KERNEL_MODE(treg1, treg2)
#else
	#define ON_BITS  (MIPS_SREG_CU1+MIPS_SREG_FR)
	#define OFF_BITS (MIPS_SREG_KSU+MIPS_SREG_ERL+MIPS_SREG_EXL+MIPS_SREG_IE)
	#define KERNEL_MODE(treg1,treg2) \
		mfc0	treg1,CP0_SREG ;\
		 li		treg2,ON_BITS+OFF_BITS ;\
		or      treg1,treg2,treg1 ;\
		xori	treg1,treg1,OFF_BITS ;\
		mtc0	treg1,CP0_SREG ;\
		 nop ;\
		nop
#endif
	
	
#if defined(VARIANT_tx79)

	/* define some TX79 instructions that the assembler doesn't support */
	#define RINSTR(op, rs, rt, rd, sa, func) \
				.word (((op)<<26)|((rs)<<21)|((rt)<<16)|((rd)<<11)|((sa)<<6)|(func))
	#define MFHI1(rd)			RINSTR(0x1c,0,0,rd,0,0x10)
	#define MFLO1(rd)			RINSTR(0x1c,0,0,rd,0,0x12)
	#define MFSA(rd)			RINSTR(0x00,0,0,rd,0,0x28)
	#define MTHI1(rs)			RINSTR(0x1c,rs,0,0,0,0x11)
	#define MTLO1(rs)			RINSTR(0x1c,rs,0,0,0,0x13)
	#define MTSA(rs)			RINSTR(0x00,rs,0,0,0,0x29)
	#define PCPYLD(rd,rs,rt)	RINSTR(0x1c,rs,rt,rd,0x0e,0x09)
	#define PCPYUD(rd,rs,rt)	RINSTR(0x1c,rs,rt,rd,0x0e,0x29)
		
	#define XREGNAME(tmp)	$##tmp
	#define REGNAME(tmp)	XREGNAME(tmp)
		
		
	#define SAVE_ONE_REG_GPR(src, dst, adj, base, tmp)	\
		SAVE_ONE_REG(src, dst, adj, base);				\
		PCPYUD(MIPS_REG_AT,MIPS_REG_##dst,MIPS_REG_ZERO);	\
		sd	AT,(-MIPS_CONTEXT_HI_SIZE)+MIPSTX79_CONTEXT_GPR_HI+(MIPS_REG_##dst*8)+(adj)(base); 

	#define RESTORE_ONE_REG_GPR(dst, src, adj, base)	\
		ld	AT,(-MIPS_CONTEXT_HI_SIZE)+MIPSTX79_CONTEXT_GPR_HI+(MIPS_REG_##src*8)+(adj)(base); \
		PCPYLD(MIPS_REG_##src,MIPS_REG_AT,MIPS_REG_ZERO);	\
		RESTORE_ONE_REG(dst, src, adj, base)
		
	#define SAVE_ONE_REG_CP0(src, dst, adj, base, tmp)	\
		SAVE_ONE_REG(src, dst, adj, base)

	#define RESTORE_ONE_REG_CP0(dst, src, adj, base)	\
		RESTORE_ONE_REG(dst, src, adj, base)

	#define SAVE_ONE_REG_HL(src,dst,adj,base,tmp) \
		SAVE_ONE_REG(src, dst, adj, base)
		
	#define RESTORE_ONE_REG_HL(dst,src,adj,base) \
		RESTORE_ONE_REG(dst, src, adj, base)
	
	#define SAVE_TX79_REGS(adj,base,tmp)		\
		MFSA(tmp);								\
		sw	REGNAME(tmp),(-MIPS_CONTEXT_HI_SIZE)+MIPSTX79_CONTEXT_SA+(adj)(base); \
		MFLO1(tmp); 							\
		sd	REGNAME(tmp),(-MIPS_CONTEXT_HI_SIZE)+MIPSTX79_CONTEXT_LO1+(adj)(base); \
		MFHI1(tmp);								\
		sd	REGNAME(tmp),(-MIPS_CONTEXT_HI_SIZE)+MIPSTX79_CONTEXT_HI1+(adj)(base); 
	
	#define RESTORE_TX79_REGS(adj,base,tmp)	\
		lw	REGNAME(tmp),(-MIPS_CONTEXT_HI_SIZE)+MIPSTX79_CONTEXT_SA+(adj)(base); \
		MTSA(tmp);								\
		ld	REGNAME(tmp),(-MIPS_CONTEXT_HI_SIZE)+MIPSTX79_CONTEXT_LO1+(adj)(base); \
		MTLO1(tmp); 							\
		ld	REGNAME(tmp),(-MIPS_CONTEXT_HI_SIZE)+MIPSTX79_CONTEXT_HI1+(adj)(base); \
		MTHI1(tmp);					
	
#elif defined(VARIANT_32) || defined(VARIANT_r3k)
		
	#define SAVE_ONE_REG_GPR(src, dst, adj, base, tmp)	\
		SAVE_ONE_REG(src, dst, adj, base);	\
		sra tmp,src,31; \
		sw	tmp,MIPS_AREG(MIPS_REG_##dst)+((MIPS_REGS_LOW_WORD^1)*4)+(adj)(base)

	#define RESTORE_ONE_REG_GPR(dst, src, adj, base)	\
		RESTORE_ONE_REG(dst, src, adj, base)
		
	#define SAVE_ONE_REG_CP0(src, dst, adj, base, tmp)	\
		SAVE_ONE_REG_GPR(src, dst, adj, base, tmp)
		
	#define RESTORE_ONE_REG_CP0(dst, src, adj, base)	\
		RESTORE_ONE_REG(dst, src, adj, base)

	#define SAVE_ONE_REG_HL(src,dst,adj,base,tmp) \
		SAVE_ONE_REG_GPR(src, dst, adj, base, tmp)
		
	#define RESTORE_ONE_REG_HL(dst,src,adj,base) \
		RESTORE_ONE_REG_GPR(dst, src, adj, base)
	
	#define SAVE_TX79_REGS(adj,base,tmp)
	
	#define RESTORE_TX79_REGS(adj,base,tmp)

#else
		
	#define SAVE_ONE_REG_GPR(src, dst, adj, base, tmp)	\
		SAVE_ONE_REG(src, dst, adj, base)

	#define RESTORE_ONE_REG_GPR(dst, src, adj, base)	\
		RESTORE_ONE_REG(dst, src, adj, base)
		
	#define SAVE_ONE_REG_CP0(src, dst, adj, base, tmp)	\
		SAVE_ONE_REG(src, dst, adj, base)

	#define RESTORE_ONE_REG_CP0(dst, src, adj, base)	\
		RESTORE_ONE_REG(dst, src, adj, base)

	#define SAVE_ONE_REG_HL(src,dst,adj,base,tmp) \
		SAVE_ONE_REG(src, dst, adj, base)
		
	#define RESTORE_ONE_REG_HL(dst,src,adj,base) \
		RESTORE_ONE_REG(dst, src, adj, base)
	
	#define SAVE_TX79_REGS(adj,base,tmp)
	
	#define RESTORE_TX79_REGS(adj,base,tmp)
	
#endif



#ifdef	VARIANT_instr

/* Note that performance registers are only saved on first
entry into the kernel.  Unlike GPRs, perf regs aren't re-saved
if the kernel itself gets pre-empted since they're disabled
by the save routine anyway (and hence won't be changing within
the kernel/interrupt context).
*/

#define SAVE_PERFREGS(act_reg, lng_jmp)						\
	la		a1,disabled_perfregs;							\
	lw		a0,CPUDATA(act_reg);							\
	beq		a0,a1,1111f;									\
	 move	s1,v0;											\
	.if lng_jmp;											\
	la		a1, cpu_save_perfregs;							\
	jalr	a1;												\
	 nop;													\
	.else;													\
	jal		cpu_save_perfregs;								\
	 nop;													\
	.endif;													\
	move	v0,s1;											\
1111:														

#else

#define SAVE_PERFREGS(act_reg, lng_jmp)

#endif



/*
 * SAVE_REGS(doing_syscall)
 *	Dump all registers onto a save area based at k0
 *
 * This macro must preserve the contents of v0, to optimize
 * system call handling.
 */
#define SAVE_REGS(doing_syscall)\
	SAVE_ONE_REG_GPR(AT,MYAT,0,k0,AT);		\
	SAVE_ONE_REG_GPR(sp,SP,0,k0,AT);		\
	SAVE_ONE_REG_GPR(v0,V0,0,k0,AT);		\
	SAVE_ONE_REG_GPR(v1,V1,0,k0,AT);		\
	SAVE_ONE_REG_GPR(a0,A0,0,k0,AT);		\
	SAVE_ONE_REG_GPR(a1,A1,0,k0,AT);		\
	SAVE_ONE_REG_GPR(a2,A2,0,k0,AT);		\
	SAVE_ONE_REG_GPR(a3,A3,0,k0,AT);		\
	SAVE_ONE_REG_GPR(s0,S0,0,k0,AT);		\
	SAVE_ONE_REG_GPR(s1,S1,0,k0,AT);		\
	SAVE_ONE_REG_GPR(s2,S2,0,k0,AT);		\
	SAVE_ONE_REG_GPR(s3,S3,0,k0,AT);		\
	SAVE_ONE_REG_GPR(s4,S4,0,k0,AT);		\
	SAVE_ONE_REG_GPR(s5,S5,0,k0,AT);		\
	SAVE_ONE_REG_GPR(s6,S6,0,k0,AT);		\
	SAVE_ONE_REG_GPR(s7,S7,0,k0,AT);		\
	SAVE_ONE_REG_GPR(s8,S8,0,k0,AT);		\
	SAVE_ONE_REG_GPR(t0,T0,0,k0,AT);		\
	SAVE_ONE_REG_GPR(t1,T1,0,k0,AT);		\
	SAVE_ONE_REG_GPR(t2,T2,0,k0,AT);		\
	SAVE_ONE_REG_GPR(t3,T3,0,k0,AT);		\
	SAVE_ONE_REG_GPR(t4,T4,0,k0,AT);		\
	MV_INSTR(mfc0) t0,CP0_EPC;	\
	mflo	t2;					\
	mfc0	t1,CP0_SREG;		\
	mfhi	t3;					\
	mfc0	t4,CP0_CAUSE;		\
	.if doing_syscall;			\
	addiu	t0,t0,4;			\
	.endif;						\
	SAVE_ONE_REG_CP0(t0,EPC,0,k0,AT);		\
	sw		t1,MIPS_AREG(MIPS_REG_SREG)+(MIPS_REGS_LOW_WORD*4)(k0);\
	sw		t4,MIPS_AREG(MIPS_REG_CAUSE)+(MIPS_REGS_LOW_WORD*4)(k0);\
	SAVE_ONE_REG_HL(t2,LO,0,k0,AT);			\
	SAVE_ONE_REG_HL(t3,HI,0,k0,AT);			\
	MV_INSTR(mfc0) t0,CP0_BADVADDR; 		\
	SAVE_ONE_REG_GPR(gp,GP,0,k0,AT);	\
	SAVE_ONE_REG_GPR(t5,T5,0,k0,AT);	\
	SAVE_ONE_REG_GPR(t6,T6,0,k0,AT);	\
	SAVE_ONE_REG_GPR(t7,T7,0,k0,AT);	\
	SAVE_ONE_REG_GPR(t8,T8,0,k0,AT);	\
	SAVE_ONE_REG_GPR(t9,T9,0,k0,AT);	\
	SAVE_ONE_REG_CP0(t0,BADVADDR,0,k0,AT);	\
	SAVE_ONE_REG_GPR(ra,RA,0,k0,AT);	\
	SAVE_TX79_REGS(0,k0,MIPS_REG_AT)
	
/*
 * RESTORE_REGS()
 *	Restore all registers from a save area based at 'base_reg'
 *
 * IE and EXL (when R4K) are also cleared as a part of restoring CP0 status.
 * For R3K systems, the EPC value is left in K1.
 */

#if defined(VARIANT_r3k)
	#define RESTORE_EPC(reg)	// nothing to do
	#define RESTORE_SREG_ON_BITS	MIPS_SREG_IMASK
#else
	#define RESTORE_EPC(reg) MV_INSTR(mtc0) reg,CP0_EPC
	#define RESTORE_SREG_ON_BITS	(MIPS_SREG_IMASK|MIPS_SREG_EXL)
#endif
	
#define RESTORE_REGS(base_reg) \
	RESTORE_TX79_REGS(0,base_reg,MIPS_REG_AT);	\
	RESTORE_ONE_REG_GPR(ra,RA,0,base_reg);	\
	RESTORE_ONE_REG_GPR(t9,T9,0,base_reg);	\
	RESTORE_ONE_REG_GPR(t8,T8,0,base_reg);	\
	RESTORE_ONE_REG_HL(a0,LO,0,base_reg);		\
	RESTORE_ONE_REG_HL(a1,HI,0,base_reg);		\
	RESTORE_ONE_REG_GPR(t7,T7,0,base_reg);	\
	RESTORE_ONE_REG_GPR(t6,T6,0,base_reg);	\
	RESTORE_ONE_REG_GPR(t5,T5,0,base_reg);	\
	mtlo a0;				\
	RESTORE_ONE_REG_GPR(t4,T4,0,base_reg);	\
	RESTORE_ONE_REG_GPR(t3,T3,0,base_reg);	\
	RESTORE_ONE_REG_GPR(s8,S8,0,base_reg);	\
	RESTORE_ONE_REG_GPR(s7,S7,0,base_reg);	\
	mthi a1;				\
	RESTORE_ONE_REG_GPR(s6,S6,0,base_reg);	\
	RESTORE_ONE_REG_GPR(s5,S5,0,base_reg);	\
	RESTORE_ONE_REG_GPR(s4,S4,0,base_reg);	\
	RESTORE_ONE_REG_GPR(s3,S3,0,base_reg);	\
	RESTORE_ONE_REG_GPR(s2,S2,0,base_reg);	\
	RESTORE_ONE_REG_GPR(s1,S1,0,base_reg);	\
	RESTORE_ONE_REG_GPR(s0,S0,0,base_reg);	\
	RESTORE_ONE_REG_GPR(a3,A3,0,base_reg);	\
	RESTORE_ONE_REG_GPR(a2,A2,0,base_reg);	\
	RESTORE_ONE_REG_GPR(a1,A1,0,base_reg);	\
	RESTORE_ONE_REG_GPR(a0,A0,0,base_reg);	\
	RESTORE_ONE_REG_GPR(v0,V0,0,base_reg);	\
	RESTORE_ONE_REG_GPR(v1,V1,0,base_reg);	\
	RESTORE_ONE_REG_CP0(k1,EPC,0,base_reg);	\
	/* Can't use K1 register from here on down... */ \
	lw		t1,MIPS_AREG(MIPS_REG_SREG)+(MIPS_REGS_LOW_WORD*4)(base_reg);\
	RESTORE_EPC(k1); \
	lw		t0,__shadow_imask; \
	ori		t1,t1,RESTORE_SREG_ON_BITS; \
	xori	t1,MIPS_SREG_IMASK; \
	lw		t2,(t0); \
	RESTORE_ONE_REG_GPR(t0,T0,0,base_reg);	\
	or    	t1,t2;\
	mtc0	t1,CP0_SREG;	\
	RESTORE_ONE_REG_GPR(t1,T1,0,base_reg);	\
	RESTORE_ONE_REG_GPR(t2,T2,0,base_reg);	\
	RESTORE_ONE_REG_GPR(gp,GP,0,base_reg);	\
	RESTORE_ONE_REG_GPR(sp,SP,0,base_reg);	\
	RESTORE_ONE_REG_GPR(AT,MYAT,0,base_reg); /* AT must be last register restored */

/* Put an indicator in context about which hardcrash happened */
#define IDHARDCRASH(ctx,temp)	\
	li		temp,__LINE__; \
	sw		temp,MIPS_AREG(MIPS_REG_CAUSE)+((MIPS_REGS_LOW_WORD^1)*4)(ctx)

/*
 * r4k_cache_handler:
 *	Handle cache error exceptions
 */
FRAME(r4k_cache_handler,sp,0,ra)
	.set	noat

//SHOWME('@',0x38,0,k0,k1)
	/*
	 * Call for help.  This one does not return (he's dead, Jim).
	 */

	BRANCH_INKERNEL(k1,1f)
	/*
	 * Nope, this is our first time in. Save the registers to
	 * the thread register save area and start the kernel stack off fresh.
	 */
	 LD_ACTIVE_AND_KERSTACK(k0,k1)
	b		2f
	 addiu	k0,k0,REG_OFF
	 
1:
	 /*
	  * We're nested, lop off some more stack.
	  */
	addiu	k0,sp,-(MIPS_CONTEXT_SIZE + 7)
	ori		k0,k0,7
	xori	k0,k0,7
	addiu	k1,k0,-MIPS_CONTEXT_EXTRA_SIZE

2:	
	SAVE_REGS(0)
	
	move	sp,k1
	.set  at
	/*
	* Get cache error value, and call for help.  This one
	* does not return (he's dead, Jim).
	*/
	move  a0,k0
	j cache_error
	 nop
ENDFRAME(r4k_cache_handler)

/*
 * r4k_check_handler:
 *	Handle machine check exceptions
 */
FRAME(r4k_check_handler,sp,0,ra)
	.set	noat

	/*
	 * Call for help.  This one does not return (he's dead, Jim).
	 */
	 
	 // Fall into hardcrash1 handler

	.set	at
ENDFRAME(r4k_check_handler)

/*
 * hardcrash1()
 *	Save register set, *then* toss our cookies
 */
	.globl hardcrash1
FRAME(hardcrash1,sp,0,ra)
	.set	noat
	addiu	k0,sp,-(MIPS_CONTEXT_SIZE + 7)
	ori		k0,k0,7
	xori	k0,k0,7
	SAVE_REGS(0)
	addiu	sp,k0,-MIPS_CONTEXT_EXTRA_SIZE
	IDHARDCRASH(k0,t0)

	.set	at
	/*
	 * Fall into hardcrash()
	 */
ENDFRAME(hardcrash1)

/*
 * hardcrash()
 *	Toss our cookies
 */
	.globl hardcrash
FRAME(hardcrash,sp,0,ra)

	/*
	 * Exception occured in either interrupt or lock
	 * phase. Need to call kdebug_enter with SIGCODE_FATAL.
	 */
	mfc0	t0,CP0_CAUSE		# read cause register
	addiu	a1,sp,MIPS_CONTEXT_EXTRA_SIZE # context is in sp
hardcrash2:
	sw		t0,MIPS_AREG(MIPS_REG_CAUSE)+(MIPS_REGS_LOW_WORD*4)(a1);\
	andi	t1,t0,MIPS_CAUSE_MASK
	lw		a0,__mips_cause2sig_map(t1)	# get signal code
	bgez	t0,hardcrash3
	 li		t0,SIGCODE_BDSLOT
	or		a0,t0
hardcrash3:
	jal	kdebug_callout
	 li		a2,SIGCODE_FATAL

	beq	v0,zero,__keriret	# is excp handled by kdebugger?
	 addiu k0,sp,MIPS_CONTEXT_EXTRA_SIZE

	addiu	a1,sp,MIPS_CONTEXT_EXTRA_SIZE # context is in sp
	j	shutdown
	 ori	a0,v0,0x80 # so we can tell we came from hardcrash in the dump

ENDFRAME(hardcrash)


/*
 * r4k_tlb_handler:
 *	Handle TLB exceptions
 */
FRAME(r4k_tlb_handler,sp,0,ra)
	.set	noat
	/*
	 * See if we are in the kernel already
	 */
	BRANCH_INKERNEL(k1,1f)
	/*
	 * Nope, this is our first time in. Save the registers to
	 * the thread register save area and start the kernel stack off fresh.
	 */
	 LD_ACTIVE_AND_KERSTACK(k0,k1)
	b		2f
	 addiu	k0,k0,REG_OFF
	 
1:
	 /*
	  * We're nested, lop off some more stack.
	  */
	addiu	k0,sp,-(MIPS_CONTEXT_SIZE + 7)
	ori		k0,k0,7
	xori	k0,k0,7
	addiu	k1,k0,-MIPS_CONTEXT_EXTRA_SIZE

2:	
	SAVE_REGS(0)
	
	move	sp,k1
	
   	move	s5,k0		// remember save area
	addiu	k0,-MIPS_CONTEXT_EXTRA_SIZE
	subu	s6,k0,k1	// set S6: in system => 0, in user => !0 
	
	.set	at

	/*
	 * Our registers are saved and our SP is valid
	 * Move CPU mode to kernel, interrupts still disabled
	 */
	KERNEL_MODE(t0,t1)

	GETCPU_1(s8,2)
	la		gp,_gp
	GETCPU_2(s8,2)

	beq		s6,zero,31f		// Only save if in user
	 addiu	s0,s5,-REG_OFF	// Top of actives
	SAVE_PERFREGS(s0,0)  	
31:	

	mfc0	t0,CP0_CAUSE		# Get cause
	mfc0	s2,CP0_BADVADDR		# get address

	SMPREF(lw,s3,aspaces_prp,s8)

	// If not SMP, acquire the kernel here (so BRANCH_INKERNEL() can tell).
	// We can't do it yet for SMP because we might spinlock waiting to
	// acquire, and that can cause a deadlock. It's OK though, because
	// the SMP version of BRANCH_INKERNEL() doesn't need the inkernel
	// bits on to tell that we're on the kernel stack. We do, however,
	// have to pretend that we're in an interrupt so that we don't ever
	// preempt this code when coming out from an interrupt.
#if defined(VARIANT_smp)	
	SMPREF(lw,s4,cpupageptr,s8)
	li		t1,1
	lw		s7,CPUPAGE_STATE(s4)
	sw		t1,CPUPAGE_STATE(s4)
	sync
#else
	ACQUIRE_KERNEL(INKERNEL_NOW+INKERNEL_LOCK,s7,0);
#endif	
	
	STI
	
	andi	t4,t0,MIPS_CAUSE_MASK	# Extract exception cause index 

	SMPREF(lw,s0,actives,s8)

	lw		s1,__mips_cause2sig_map(t4) # convert to signal code
	bgez	t0,1f
	 li		t0,SIGCODE_BDSLOT
	or		s1,t0
	li		t0,0
1:	 
	bne		s6,zero,1f
	 la		t2,xfer_handlers
	li		t0,SIGCODE_KERNEL >> 24 
	MSGOPT_SMPADDR(t2,s8)
	lw		t1,(t2)
	beq		t1,zero,2f
	 nop
	ori		t0,SIGCODE_INXFER >> 24
2:	
	GET_INKERNEL(t1,0) 			
	andi	t1,INKERNEL_EXIT
	beq		t1,zero,1f
	 nop
	ori		t0,SIGCODE_KEREXIT >> 24
1:	 
	sll		t0,24
	// Don't want the SIGCODE_KERNEL, INXFER, KEREXIT bits on in 
	// S1 - it'll confuse other code since those bits have multiple
	// meanings
	or		t0,s1
	addiu	sp,-SIZEOF_FAULT_INFO
	lw		t4,MEMMGR_FAULT+memmgr
	la		a0,ABI_CALL_OVERHEAD(sp)
//	sw		s6,FI_CPU_IN_USER(a0)
	sw		s3,FI_PRP(a0)
	sw		s2,FI_VADDR(a0)
	sw		t0,FI_SIGCODE(a0)

	jal		t4
	 sw		s5,FI_CPU_REGS(a0)
	 
	CLI

#if defined(VARIANT_smp)	
	// undo the 'in interrupt' indication
	sw		s7,CPUPAGE_STATE(s4)
#endif	
	

	bgt		v0,zero,corrected	// Was the fault corrected?
	 nop

// Since we haven't acquired the kernel yet in the SMP case, do it now
#if defined(VARIANT_smp)	
	// NOTE: the acquire kernel code must not modify V0
	ACQUIRE_KERNEL(INKERNEL_NOW+INKERNEL_LOCK,s7,0);
#endif	

	beq		v0,zero,deferred	// Was the fault deferred?
	 nop

	// Fault needs to be signalled 
	
signalled:	 
	beq		s6,zero,sys_tlb
	 move	v0,s1				// V0 = S1 for sys_tlb processing below....
	 
	/*
	 * TLB exception in user process 
	 */

	la		ra,__ker_exit
	move	a0,s1
	move	a1,s0
	j		usr_fault
	 move	a2,s2

	 // Fault has been corrected
corrected:
#if !defined(VARIANT_smp)
	// Restore the inkernel state
	sw		s7,inkernel
#endif	

	beq		s6,zero,__keriret	// if in kernel, just iret
	 move	k0,s5

// Since we haven't acquired the kernel yet in the SMP case, do it now	
#if defined(VARIANT_smp)	
	ACQUIRE_KERNEL(INKERNEL_NOW+INKERNEL_LOCK,s7,0);
#endif	
	b		__ker_exit
	 nop

	 // We need to PageWait the process to figure out what to do
deferred:

	STI
	jal		PageFaultWait
	 la		a0,ABI_CALL_OVERHEAD(sp)
	CLI

	bne		v0,zero,signalled	// If PageFaultWait couldn't, report failure
	 nop

	bne		s6,zero,__ker_exit	// if user, get back into execution
	 li		v0,0				// V0 = 0 for sys_tlb processing below....
	 
	/* 
	 * TLB exception in system 
	 */
sys_tlb:
	addiu	sp,SIZEOF_FAULT_INFO
	
	andi	t0,s7,INKERNEL_LOCK /* are we locked? */
	bne		t0,zero,tlb_maybecrash
	
	 andi	t0,s7,INKERNEL_INTRMASK /* are we in an interrupt handler? */
	bne		t0,zero,tlb_intr
	
sys_tlb_not_intr:
	 
	/*
	 * If INKERNEL_SPECRET is set then it has to be cleared. 
	 */
	 andi	t0,s7,INKERNEL_SPECRET
	bne		t0,zero,tlb_specret
	 xor	t0,s7,t0				# turn off SPECRET bit (if it was on)

	/*
	 * page fault, no specret. If an error was 
	 * returned by memmgr.fault, need to return
	 * EFAULT for kernel call.
	 */
	bne		v0,zero,fixup_kcall 
	 
	/*
	 * page fault, no specret, no error. 
	 * The faulting thread is blocked now. When faulting
	 * in the kernel the kernel call has to be restarted,
	 * so back the pc up unless we're in kernel exit processing. 
	 */
	andi	t0,s7,INKERNEL_EXIT
	bne		t0,zero,__ker_exit
	 RESTORE_ONE_REG(t0,EPC,REG_OFF,s0)
	addiu	t0,t0,-4
	b		__ker_exit
	 SAVE_ONE_REG(t0,EPC,REG_OFF,s0)
	
   	/*
	 * TLB exception during interrupt handling.
	 */
tlb_intr:
#if defined(VARIANT_smp)
	SMPREF(lw,t1,cpupageptr,s8)
	lw		t1,CPUPAGE_STATE(t1)
	beq		t1,zero,sys_tlb_not_intr
	 // first instr of CLI macro below is in branch shadow.
#endif
	
   	/*
	 * TLB exception while kernel was locked (or in an interrupt)
	 */
tlb_maybecrash:
	CLI
	# restore original inkernel value
	NEW_INKERNEL_BITS_REG(t0,s7)
	
// Don't think these next two lines should be here. If we were locked and
// The fault wasn't corrected, we can't just return to that code, even
// if vmm_fault() thinks it can fix things up later.	
//	beq		v0,zero,__keriret
//	 addiu	k0,sp,MIPS_CONTEXT_EXTRA_SIZE
	/*
	 * Check if we were in an xfer handler; if so, fixup.
	 */
	la	t9,xfer_handlers
	MSGOPT_SMPADDR(t9,s8)
	lw	t0,(t9)
	/*
	 * If no xfer handlers, hardcrash
	 */
	IDHARDCRASH(k0,t1)
	beq		t0,zero,hardcrash
	 srl	a2,s1,MIPS_CAUSE_SHIFT

	sw		zero,(t9)
	lw		t0,0(t0)
	move	a0,s0
	jal		t0
	 addiu	a1,sp,MIPS_CONTEXT_EXTRA_SIZE

tlb_specret:
	/* 
	 * If memmgr.fault returned an error,
	 * then return EFAULT for kernel call.
	 * Otherwise, return through __ker_exit 
	 * and let someone else run. 
	 */ 
	
	bne		v0,zero,fixup_specret
	 nop
	NEW_INKERNEL_BITS_REG(t0,t0) 		# clear in INKERNEL_SPECRET bit
	 
#if defined(SMP_MSGOPT)
	SMPREF(sw,zero,xfer_handlers,s8)
#else
	sw		zero,xfer_handlers
#endif
	j		__ker_exit
	 nop
	
ENDFRAME(r4k_tlb_handler)


/*
 * r4k_brkpt_handler:
 *	Handle breakpoint exceptions
 */
FRAME(r4k_brkpt_handler,sp,0,ra)
	.set	noat

	/*
	 * See if we're in the kernel already.
	 */
	BRANCH_INKERNEL(k1,sys_brkpt)
	/*
	 * Coming from user mode.  Save user registers, and get
	 * a fresh kernel stack.
	 */
	 LD_ACTIVE_AND_KERSTACK(k0,k1)
	addiu	k0,k0,REG_OFF

	SAVE_REGS(0)

	.set	at
	
	addiu	s0,k0,-REG_OFF
	move	sp,k1

	/*
	 * Move CPU mode to kernel, interrupts still disabled
	 */
	KERNEL_MODE(t0,t1)

	GETCPU_1(s8,2)
	la		gp,_gp
	GETCPU_2(s8,2)

	SAVE_PERFREGS(s0,0)

	/*
	 * Check for Debug*() opcodes
	 */
	RESTORE_ONE_REG(s2,EPC,REG_OFF,s0)
	mfc0	t4,CP0_CAUSE
	 li		t3,MIPS_CAUSE_BD_SLOT
	and		t3,t4
	beq		t3,zero,1f
	 nop
	addiu	s2,4	//Instruction we want is in branch delay slot
1:

	//
	// You might think that you don't have to load S1 here and instead
	// could transfer control to the "deliver_user" label below and
	// let it determine the signal code from the CAUSE register. You'd
	// be wrong. On the Broadcom SB-1, we can get an icache hit without
	// a TLB mapping for the region (because of the virtual icache).
	// That means the "lw" instruction just below can get a TLB miss
	// exception, which will change the ExcCode field from a 9 to a
	// 2 and make "deliver_user" report a SIGSEGV rather than a SIGTRAP.
	//
	li		s1,SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)

	lw		t1,(s2)
	li		t2,OPCODE_BREAKX(8)
	beq		t1,t2,kd_break
	 li		t2,OPCODE_BREAKX(9)
	beq 	t1,t2,kd_break
	 li		t2,OPCODE_BREAKX(6)		//Overflow breakpoint
	beq		t1,t2,int_overflow
	 li		t2,OPCODE_BREAKX(7)		//Div-by-zero breakpoint
	beq		t1,t2,div_zero
	 li		t2,OPCODE_BREAKX(10)
	bne 	t1,t2,deliver_user_synth
	/* DebugBreak() opcode */
	 addiu	s2,s2,4
	b		deliver_user_synth
	 SAVE_ONE_REG(s2,EPC,REG_OFF,s0)

int_overflow:
div_zero:
	/* Report correct exception code */
	li		s1,SIGFPE + (FPE_INTOVF*256) + (FLTIOVF*65536)
	b		deliver_user_synth
	 nop
	 
kd_break:
	/* DebugKD*() opcode */
	li		a2,SIGCODE_KERNEL
	move	a0,s1
	jal		kdebug_callout
	 addiu	a1,s0,REG_OFF		# point at register set context
	bne		v0,zero,deliver_user_synth
	 move	s1,v0
	 
#if defined(VARIANT_smp)
   	ACQUIRE_KERNEL(INKERNEL_NOW+INKERNEL_EXIT,zero,1)
#endif
	b		__ker_exit
	 nop
	 
	
sys_brkpt:
	.set	noat
	
	/*
	 * breakpoint happened during kernel code (or ISR).  We'll need
	 * to nest onto the existing kernel stack.
	 */
	addiu	k0,sp,-(MIPS_CONTEXT_SIZE + 7)
	ori		k0,k0,7
	xori	k0,k0,7
	
	SAVE_REGS(0)

	.set	at

	la		gp,_gp
	IDHARDCRASH(k0,t0)
	b		hardcrash	# let the kernel debugger take it
	 addiu	sp,k0,-MIPS_CONTEXT_EXTRA_SIZE
	
ENDFRAME(r4k_brkpt_handler)


/*
 * r4k_addrerr_handler:
 *	Handle Address Error exceptions
 */
FRAME(r4k_addrerr_handler,sp,0,ra)
	.set	noat

#if !defined(VARIANT_r3k)
	/*
	 * See if we're accessing a supervisor address space when in user mode.
     * May want to allow it - only virtual fault knows for sure.
	 */
	mfc0	k0,CP0_BADVADDR
	li		k1,0xe0000000
	and		k0,k1
	li		k1,0xc0000000
	bne		k0,k1,1f
	 mfc0	k0,CP0_SREG		/* it was a KSSEG addr */
      nop
	andi	k1,k0,0x18
    li		k0,0x10
    beq		k0,k1,r4k_tlb_handler	/* and we're in user mode */
      nop
1:
#endif
	GETCPU_1(k1,2)
	lui		k0,%hi(actives)
	GETCPU_2(k1,2)
	SMPADDR(k0,k1)
	lw		k0,%lo(actives)(k0)
	li		k1,_NTO_TF_ALIGN_FAULT
	lw		k0,TFLAGS(k0)
	and		k0,k1
	bne		zero,k0,r4k_exception_handler	/* fault alignment errors */
	 nop
	 
	/* fall through to r4k_illop_handler (try and emulate instr) */
	 
	.set	at
ENDFRAME(r4k_addrerr_handler)


/*
 * r4k_illop_handler:
 *	Handle illegal opcode exceptions (and try to fix up alignment errors)
 */
FRAME(r4k_illop_handler,sp,0,ra)
	.set	noat

	/*
	 * See if we're in the kernel already.
	 */
	BRANCH_INKERNEL(k1,sys_illop)
	/*
	 * Coming from user mode.  Save user registers, and get
	 * a fresh kernel stack.
	 */
	 LD_ACTIVE_AND_KERSTACK(k0,k1)
	b		1f
	 addiu	k0,k0,REG_OFF

sys_illop:
	/*
	 * illegal op happened during kernel code (or ISR).  We'll need
	 * to nest onto the existing kernel stack.
	 */
	addiu	k0,sp,-(MIPS_CONTEXT_SIZE + 7)
	ori		k0,k0,7
	xori	k0,k0,7
	addiu	k1,k0,-MIPS_CONTEXT_EXTRA_SIZE

1:
	SAVE_REGS(0)
	
	move	sp,k1
	
	.set	at
	
   	GETCPU_1(s8,2)
	la		gp,_gp
	GETCPU_2(s8,2)

	move	s0,k0	// remember reg storage area
	
	addiu	k0,-MIPS_CONTEXT_EXTRA_SIZE 
	subu	s6,k0,k1	// set S6: in system => 0, in user => !0 
	
	/*
	 * Move CPU mode to kernel, interrupts still disabled
	 */
	KERNEL_MODE(t0,t1)
	
	beq		s6,zero,2f		// Only save if in user
	 addiu	s2,s0,-REG_OFF	// Point to actives
	SAVE_PERFREGS(s2,0) 	
2:

	/* 
	 * emulate_instruction needs the BD bit of the cause register.
	 * We have to save it here since an interrupt could come along and 
	 * hammer the contents after they're enabled.
	 */
try_emulation:
	mfc0	a1,CP0_CAUSE	
	
   	ACQUIRE_KERNEL(INKERNEL_NOW+INKERNEL_EXIT,s7,0)
	 
	ori		t0,s7,INKERNEL_NOW+INKERNEL_EXIT
	NEW_INKERNEL_BITS_REG(t0,t0)
#ifdef TXHACK
mfc0	t0,CP0_SREG
li		t1,~0x00010000
and		t0,t0,t1
mtc0    t0,CP0_SREG
#endif
	
   	/*
	 * Enable interrupts again, if they were enabled in the original pgm.
	 * Also pass the original state to emulate_instruction.
	 */
   	mfc0	t0,CP0_SREG
	 lw		a2,MIPS_AREG(MIPS_REG_SREG)+(MIPS_REGS_LOW_WORD*4)(s0) 

#if defined(VARIANT_r3k)
	// Need to use the IEp bit to see if intrs were enabled previously
	srl		a2,2
#endif
	 
	ori		t0,MIPS_SREG_IE
	andi	a2,MIPS_SREG_IE
	xori	t0,MIPS_SREG_IE
	or		t0,a2
	mtc0	t0,CP0_SREG
	 
	jal		emulate_instruction
	 move	a0,s0
	 
	CLI
	
	bne		v0,zero,1f
	 move	k0,s0				# point at saved register set
	 
	# we successfully emulated instruction
	
	beq		s6,zero,2f			# if in kernel, just return
	 SMPREF(lw,s0,actives,s8)
	 
	NEW_INKERNEL_BITS(t0,INKERNEL_NOW+INKERNEL_LOCK+INKERNEL_EXIT)
	b		__ker_emu_restart	# do a little more work if user
	 nop
2:	
	NEW_INKERNEL_BITS_REG(zero,s7)
	b		__keriret	
	 nop
	
1:
#if defined(VARIANT_r3k)
	// Check for instructions that R3K's don't handle, but we want to use
	// the floating point emulator for.
	SMPREF(lw,s0,actives,s8)
	li		t0,SIGILL + (ILL_PRVOPC*256) + (FLTPRIV*65536)
	li		t1,MIPS_CAUSE_CP_UNUSABLE << MIPS_CAUSE_SHIFT
	beq		v0,t0,do_fp_emulation
	 sw		t1,MIPS_AREG(MIPS_REG_CAUSE)+(MIPS_REGS_LOW_WORD*4)+REG_OFF(s0) 
#endif

	# we did NOT successfully emulate instruction
	
	beq		s6,zero,deliver_system	# handle fault in system space
	 move	s1,v0					# put sigcode where deliver_system expects
	
	b		deliver_user_synth2
	 move	s2,zero

ENDFRAME(r4k_illop_handler)


/*
 * r4k_nocoproc_handler:
 *	Handle coprocessor unusable exceptions when we're doing floating point 
 *  emulation.
 */
FRAME(r4k_nocoproc_handler,sp,0,ra)
	.set	noat

	/*
	 * See if we're in the kernel already.
	 */
	BRANCH_INKERNEL(k1,sys_exc)
	/*
	 * Coming from user mode.  Save user registers, and get
	 * a fresh kernel stack.
	 */
	 LD_ACTIVE_AND_KERSTACK(k0,k1)
	addiu	k0,k0,REG_OFF

	SAVE_REGS(0)

	.set	at
	
	addiu	s0,k0,-REG_OFF					// save active pointer
	move	sp,k1

	/*
	 * Move CPU mode to kernel, interrupts still disabled
	 */
	KERNEL_MODE(t0,t1)
	
	mfc0	t0,CP0_CAUSE
	
	GETCPU_1(s8,2)
	la		gp,_gp
	GETCPU_2(s8,2)

	SAVE_PERFREGS(s0,0)

//	sw		t0,MIPS_AREG(MIPS_REG_CAUSE)+(MIPS_REGS_LOW_WORD*4)+REG_OFF(s0) 
	srl		t0,t0,28				// get the coproc unit number
	andi	t0,t0,0x3				// ...
	beq		t0,zero,check_mfc0_count2 // no emulation of coproc zero
do_fp_emulation:
	 lw		t0,PROCESS(s0)
	 
	lw		t0,PLS(t0)
	lw		s1,PLS_MATHEMULATOR(t0)	// have we got a math emulator?
	beq		s1,zero,deliver_user
	 nop
	
	/*
	 * We're going to try to emulate the instruction
	 */
	ACQUIRE_KERNEL(INKERNEL_NOW+INKERNEL_EXIT,zero,1)
	
	STI
	
	move	a1,s0
	li		a2,SIZEOF_REG
	jal		fpu_emulation_prep
	 addiu	a0,s0,REG_OFF
	// Where registers got stored comes back in V0

	lw		t0,MIPS_AREG(MIPS_REG_CAUSE)+(MIPS_REGS_LOW_WORD*4)+REG_OFF(s0) 
	andi	a0,t0,MIPS_CAUSE_MASK	# Extract index
	lw		s2,__mips_cause2sig_map(a0)
	bgez	t0,1f
	 li		t0,SIGCODE_BDSLOT
	or		a0,t0
1:	
	 
	// invoke user handler 
	NEW_INKERNEL_BITS(t0,INKERNEL_NOW+INKERNEL_LOCK+INKERNEL_EXIT)
	
	lw		t0,TLS(s0)
	SAVE_ONE_REG(s1,EPC,REG_OFF,s0)
	SAVE_ONE_REG(v0,A2,REG_OFF,s0)
	addi	v0,-16
	addiu	t0,TLS_FPUEMU_DATA
	SAVE_ONE_REG(s2,A0,REG_OFF,s0)
	SAVE_ONE_REG(t0,A1,REG_OFF,s0)
	SAVE_ONE_REG(v0,SP,REG_OFF,s0)
	
   	// Suppress any single stepping going on.
	jal		begin_fp_emulation
   	 move	a0,s0
	beq		v0,zero,__ker_exit
	 li		t0,SIGCODE_SSTEP
	or		s2,t0
	SAVE_ONE_REG(s2,A0,REG_OFF,s0)
	 
	b		__ker_exit
	 nop
	
ENDFRAME(r4k_nocoproc_handler)


/*
 * r4k_fpu_handler:
 *	Handle coprocessor unusable exceptions when we're doing floating point 
 *  hardware support.
 */
FRAME(r4k_fpu_handler,sp,0,ra)
	.set	noat

	/*
	 * See if we're in the kernel already.
	 */
	BRANCH_INKERNEL(k1,sys_exc)
	/*
	 * Coming from user mode.  Save user registers, and get
	 * a fresh kernel stack.
	 */
	 LD_ACTIVE_AND_KERSTACK(k0,k1)
	addiu	k0,k0,REG_OFF

	SAVE_REGS(0)

	.set	at
	
	addiu	s0,k0,-REG_OFF					// save active pointer
	move	sp,k1

	/*
	 * Move CPU mode to kernel, interrupts still disabled
	 */
	KERNEL_MODE(t0,t1)
	
	mfc0	t0,CP0_CAUSE
	
	GETCPU_1(s8,2)
	la		gp,_gp
	GETCPU_2(s8,2)

	SAVE_PERFREGS(s0,0)

//	sw		t0,MIPS_AREG(MIPS_REG_CAUSE)+(MIPS_REGS_LOW_WORD*4)+REG_OFF(s0) 
	srl		t0,t0,28				// get the coproc unit number
	andi	t0,t0,0x3				// ...
	li		t1,1
	bne		t0,t1,check_mfc0_count	// only interested in CP1
	 lw		s1,FPUDATA(s0)
	 
	bne		s1,zero,1f
	 // Branch delay instr below
	/*
	 * Allocate an FP save area 
	 */
#if defined(VARIANT_smp)
2:
	 ll		t0,ATFLAGS(s0)
	ori		t0,_NTO_ATF_FPUSAVE_ALLOC
	sc		t0,ATFLAGS(s0)
	beq		t0,zero,2b
	 nop
	ACQUIRE_KERNEL(INKERNEL_NOW+INKERNEL_EXIT,zero,1)
	b		__ker_exit
	 nop
#else
	 lw		t0,ATFLAGS(s0)
	ori		t0,_NTO_ATF_FPUSAVE_ALLOC
	b		__ker_exit
	 sw		t0,ATFLAGS(s0)
#endif
	 
1:
	ACQUIRE_KERNEL(INKERNEL_NOW+INKERNEL_EXIT,zero,1)
	
	la		s3,actives_fpu
	SMPADDR(s3,s8)
	lw		t0,(s3)
	STI
	
	beq		t0,s0,2f	// active fpu is us - just enable FPU access
	 nop
	
	beq		t0,zero,1f	// no active FPU, just have to load our context.
	 nop
	.set mips3
	lw		t2,FPUDATA(t0)
	CLEAR_FPUPTR(t2)
	cfc1	t1,$31
	sdc1	 $f0, 0*8(t2)
	sdc1	 $f1, 1*8(t2)
	sdc1	 $f2, 2*8(t2)
	sdc1	 $f3, 3*8(t2)
	sdc1	 $f4, 4*8(t2)
	sdc1	 $f5, 5*8(t2)
	sdc1	 $f6, 6*8(t2)
	sdc1	 $f7, 7*8(t2)
	sdc1	 $f8, 8*8(t2)
	sdc1	 $f9, 9*8(t2)
	sdc1	$f10,10*8(t2)
	sdc1	$f11,11*8(t2)
	sdc1	$f12,12*8(t2)
	sdc1	$f13,13*8(t2)
	sdc1	$f14,14*8(t2)
	sdc1	$f15,15*8(t2)
	sdc1	$f16,16*8(t2)
	sdc1	$f17,17*8(t2)
	sdc1	$f18,18*8(t2)
	sdc1	$f19,19*8(t2)
	sdc1	$f20,20*8(t2)
	sdc1	$f21,21*8(t2)
	sdc1	$f22,22*8(t2)
	sdc1	$f23,23*8(t2)
	sdc1	$f24,24*8(t2)
	sdc1	$f25,25*8(t2)
	sdc1	$f26,26*8(t2)
	sdc1	$f27,27*8(t2)
	sdc1	$f28,28*8(t2)
	sdc1	$f29,29*8(t2)
	sdc1	$f30,30*8(t2)
	sdc1	$f31,31*8(t2)
	sw		t1,REG_FPCR31(t2)
#if defined(VARIANT_smp)
	sw		t2,FPUDATA(t0)	// clear FPUDATA_BUSY, CPU # indicators
#endif
	sw		zero,(s3)
1:
#if defined(VARIANT_smp)
	srl		s2,s8,2
	and		t0,s1,FPUDATA_BUSY
	beq		t0,zero,not_busy
	 and	a0,s1,FPUDATA_CPUMASK
	beq		a0,s2,not_busy
	 li		a1,IPI_CONTEXT_SAVE
	jal		send_ipi
	 nop
	// wait for context to be saved
1:
	lw		t0,FPUDATA(s0)
	andi	t0,FPUDATA_BUSY
	bne		t0,zero,1b
	 nop
not_busy:
#endif

	CLEAR_FPUPTR(s1)
	lw		t1,REG_FPCR31(s1)
	cfc1	zero,$31	// Make sure no FP instructions executing
	li		t2,~MIPS_FCR31_CAUSE_MASK
	and		t1,t2				//Make sure no cause bits are on
	ctc1	t1,$31
	ldc1	 $f0, 0*8(s1)
	ldc1	 $f1, 1*8(s1)
	ldc1	 $f2, 2*8(s1)
	ldc1	 $f3, 3*8(s1)
	ldc1	 $f4, 4*8(s1)
	ldc1	 $f5, 5*8(s1)
	ldc1	 $f6, 6*8(s1)
	ldc1	 $f7, 7*8(s1)
	ldc1	 $f8, 8*8(s1)
	ldc1	 $f9, 9*8(s1)
	ldc1	$f10,10*8(s1)
	ldc1	$f11,11*8(s1)
	ldc1	$f12,12*8(s1)
	ldc1	$f13,13*8(s1)
	ldc1	$f14,14*8(s1)
	ldc1	$f15,15*8(s1)
	ldc1	$f16,16*8(s1)
	ldc1	$f17,17*8(s1)
	ldc1	$f18,18*8(s1)
	ldc1	$f19,19*8(s1)
	ldc1	$f20,20*8(s1)
	ldc1	$f21,21*8(s1)
	ldc1	$f22,22*8(s1)
	ldc1	$f23,23*8(s1)
	ldc1	$f24,24*8(s1)
	ldc1	$f25,25*8(s1)
	ldc1	$f26,26*8(s1)
	ldc1	$f27,27*8(s1)
	ldc1	$f28,28*8(s1)
	ldc1	$f29,29*8(s1)
	ldc1	$f30,30*8(s1)
	ldc1	$f31,31*8(s1)
#if defined(VARIANT_smp)
	ori		t0,s1,FPUDATA_BUSY
	or		t0,s2			// or in the CPU number that we're running on
	sw		t0,FPUDATA(s0)
#endif
	sw		s0,(s3)
	.set MIPSARCH
2:
   	/* OK for this thread to use the F.P. registers now */
	lw		t0,MIPS_AREG(MIPS_REG_SREG)+(MIPS_REGS_LOW_WORD*4)+REG_OFF(s0)
	li		t1,MIPS_SREG_CU1
	or		t0,t1
	b		__ker_exit
	 sw		t0,MIPS_AREG(MIPS_REG_SREG)+(MIPS_REGS_LOW_WORD*4)+REG_OFF(s0)
	 
ENDFRAME(r4k_fpu_handler)

/*
 * Check to see if instruction is "mfc0	??,CP0_COUNT" and let it happen
 * if so (used by ClockCycles() API).
 */
check_mfc0_count:
	bne		t0,zero,deliver_user	//Make sure it's CP0
	 nop
	/* 
	 * S0 is pointing at the active thread
	 */
check_mfc0_count2:
	// using S2 to line up with deliver_user_synth requirements
	RESTORE_ONE_REG(s2,EPC,REG_OFF,s0)
	li		t2,0xffe0ffff
	lw		t1,(s2)						// get instruction
	// load up S1 assuming that we're going to branch to deliver_user_synth
	li		s1,SIGILL + (ILL_PRVOPC*256) + (FLTPRIV*65536)
	and		t3,t1,t2					// mask out dest register
	li		t4,0x40004800				// "mfc0 ??,CP0_COUNT"
#if defined(VARIANT_r3k)
	bne		t3,t4,try_ll_sc
#else
	bne		t3,t4,deliver_user_synth
#endif
	 srl	t2,t1,(16-3)
	andi	t1,t2,(0x1f << 3)				// get offset to dest register
#if defined(VARIANT_r3k)
//R3K: CP0_COUNT emulation
	move	t2,zero
#else
	mfc0	t2,CP0_COUNT				// get count register value
#endif
	 la		k0,REG_OFF(s0)
	add		s2,4						// advance EPC
	add		t1,k0
	SAVE_ONE_REG(s2,EPC,REG_OFF,s0)
	b		__keriret
	 LS_INSTR(s) t2,REG_POS_BASE(t1)	// save count reg value
	 
#if defined(VARIANT_r3k)
// Workaround for TX39 - it reports a coprocessor unusable exception
// for LL & SC conditional instructions when in user mode with CP0 access
// disallowed (silly chip :-().
try_ll_sc:
	addiu	s0,REG_OFF		// try_emulation expects S0 to point to reg set
	li		s6,1			// so that try_emulation knows we were in user mode
	li		t2,0xfc000000
	and		t3,t1,t2
	li		t4,6<<29	//LL
	beq     t3,t4,try_emulation
	 li		t4,7<<29	//SC
	beq		t3,t4,try_emulation
	 nop
	b		deliver_user_synth
	 nop
#endif

fpe_codes:
	.long SIGFPE + (FPE_FLTRES*256) + (FLTFPE*65536)	//I
	.long SIGFPE + (FPE_FLTUND*256) + (FLTFPE*65536)	//U
	.long SIGFPE + (FPE_FLTOVF*256) + (FLTFPE*65536)	//O
	.long SIGFPE + (FPE_FLTDIV*256) + (FLTFPE*65536)	//Z
	.long SIGFPE + (FPE_FLTINV*256) + (FLTFPE*65536)	//V
	.long SIGFPE + (FPE_NOFPU*256)  + (FLTFPE*65536)	//E

/*
 * r4k_fpu_exc_handler:
 *	Handle FPU exception error (hardware FPU support)
 */
FRAME(r4k_fpu_exc_handler,sp,0,ra)
	.set	noat

	/*
	 * See if we're in the kernel already.
	 */
	BRANCH_INKERNEL(k1,hardcrash1)
	/*
	 * Coming from user mode.  Save user registers, and get
	 * a fresh kernel stack.
	 */
	 LD_ACTIVE_AND_KERSTACK(k0,k1)
	addiu	k0,k0,REG_OFF

	SAVE_REGS(0)

	.set	at
	
	addiu	s0,k0,-REG_OFF					// save active pointer
	move	sp,k1

	GETCPU_1(s8,2)
	la		gp,_gp
	GETCPU_2(s8,2)
	
	/*
	 * Move CPU mode to kernel, interrupts still disabled
	 */
	KERNEL_MODE(t0,t1)

	ACQUIRE_KERNEL(INKERNEL_NOW+INKERNEL_EXIT,zero,1)

	SAVE_PERFREGS(s0,0)

	/* User fault expects a1 to contain the active pointer. 
	SAVE_PERFERGS uses a1, so assign it here. */
	move 	a1, s0
	
	STI
	
	lw		t0,FPUDATA(a1)	//Must have allocated save area by now
	CLEAR_FPUPTR(t0)
	
   	.set mips3
	/*
	 * Save state for any user exception handler
	 */
	cfc1	t1,$31
	sdc1	 $f0, 0*8(t0)
	sdc1	 $f1, 1*8(t0)
	sdc1	 $f2, 2*8(t0)
	sdc1	 $f3, 3*8(t0)
	sdc1	 $f4, 4*8(t0)
	sdc1	 $f5, 5*8(t0)
	sdc1	 $f6, 6*8(t0)
	sdc1	 $f7, 7*8(t0)
	sdc1	 $f8, 8*8(t0)
	sdc1	 $f9, 9*8(t0)
	sdc1	$f10,10*8(t0)
	sdc1	$f11,11*8(t0)
	sdc1	$f12,12*8(t0)
	sdc1	$f13,13*8(t0)
	sdc1	$f14,14*8(t0)
	sdc1	$f15,15*8(t0)
	sdc1	$f16,16*8(t0)
	sdc1	$f17,17*8(t0)
	sdc1	$f18,18*8(t0)
	sdc1	$f19,19*8(t0)
	sdc1	$f20,20*8(t0)
	sdc1	$f21,21*8(t0)
	sdc1	$f22,22*8(t0)
	sdc1	$f23,23*8(t0)
	sdc1	$f24,24*8(t0)
	sdc1	$f25,25*8(t0)
	sdc1	$f26,26*8(t0)
	sdc1	$f27,27*8(t0)
	sdc1	$f28,28*8(t0)
	sdc1	$f29,29*8(t0)
	sdc1	$f30,30*8(t0)
	sdc1	$f31,31*8(t0)
	.set MIPSARCH
	sw		t1,REG_FPCR31(t0)
	li		t0,MIPS_FCR31_CAUSE_MASK
	and		t2,t1,t0		//Extract exception cause bits
	addiu	t3,t2,-1		//Isolate low order bit
	and		t4,t3,t2	
	xor		t4,t2
	li		t5,MIPS_FCR31_ENABLE_MASK
	and		t6,t1,t5		//Extract enabled exception bits
	ori		t6,MIPS_FCR31_ENABLE_V << 1 // for turning off the E cause bit
	sll		t6,5
	not		t6
	and		t1,t6			//Turn off any enabled cause bits
	
	cfc1	zero,$31	// Make sure no FP instructions executing
	ctc1	t1,$31
	
	srl		t4,11			//get the exception bit to the bottom of reg
	
	la		t0,fpe_codes-4	//turn exception bit into signal code
1:
	srl		t4,1
	andi	a0, t4, 1		//Check this exception bit
	beq		a0,zero,1b
	 addiu	t0,4			
	lw		a0,(t0)
	
	la		ra,__ker_exit
	j		usr_fault
	 move	a2,zero
	 
ENDFRAME(r4k_fpu_exc_handler)


FRAME(sb1_bus_handler,sp,0,ra)
	.set	noat
	/*
	 * See if we are in the kernel already
	 */
	BRANCH_INKERNEL(k1,1f)
	/*
	 * Nope, this is our first time in. Save the registers to
	 * the thread register save area and start the kernel stack off fresh.
	 */
	 LD_ACTIVE_AND_KERSTACK(k0,k1)
	b		2f
	 addiu	k0,k0,REG_OFF
	 
1:
	 /*
	  * We're nested, lop off some more stack.
	  */
	addiu	k0,sp,-(MIPS_CONTEXT_SIZE + 7)
	ori		k0,k0,7
	xori	k0,k0,7
	addiu	k1,k0,-MIPS_CONTEXT_EXTRA_SIZE

2:	
	SAVE_REGS(0)
	
	move	sp,k1
	
   	move	s5,k0		// remember save area
	addiu	k0,-MIPS_CONTEXT_EXTRA_SIZE
	subu	s6,k0,k1	// set S6: in system => 0, in user => !0 
	
	.set	at

	/*
	 * Our registers are saved and our SP is valid
	 * Move CPU mode to kernel, interrupts still disabled
	 */
	KERNEL_MODE(t0,t1)

	GETCPU_1(s8,2)
	la		gp,_gp
	GETCPU_2(s8,2)

	la		ra,sys_exc1
	beq		s6,zero,31f		// Only save if in user
	 addiu	s0,s5,-REG_OFF	// Top of actives
	SAVE_PERFREGS(s0,0)  	
	la		ra,deliver_user
31:	
	.set mips64
	dmfc0	a1,$26,1	// Get buserr-DPA
	 nop
	mtc0	zero,$26,1	// Unlock buserr-DPA for the next time 

	 // We have to make sure that the bus error didn't 'infect' the
	 // cache with an entry that will cause an error when it gets tossed
	 // out. We've already set up the return address register above with 
	 // the right place for the subroutine to come back to, so we don't 
	 // bother loading it here with a 'jal'
	j		sb1_cache_clean
	 dsrl	a0,a1,32	// get high order portion in A0, low order in A1

	.set MIPSARCH

ENDFRAME(sb1_bus_handler)


FRAME(r4k_watch_handler,sp,0,ra)
	// Make sure to turn off the Watch Pending bit in the CAUSE register
	mfc0	k0,CP0_CAUSE
	 li		k1,~MIPS_CAUSE_WP
	and		k0,k1
	mtc0	k0,CP0_CAUSE
	 // delay slot instruction below
	 
	// fall through to r4k_exception_handler
ENDFRAME(r4k_watch_handler)
/*
 * r4k_exception_handler:
 *	Handle all other exceptions
 */
FRAME(r4k_exception_handler,sp,0,ra)
	.set	noat
	
	/*
	 * See if we're in the kernel already.
	 */
	BRANCH_INKERNEL(k1,sys_exc)
	/*
	 * Coming from user mode.  Save user registers, and get
	 * a fresh kernel stack.
	 */
	 LD_ACTIVE_AND_KERSTACK(k0,k1)
	addiu	k0,k0,REG_OFF

	SAVE_REGS(0)

	.set	at
	
	move	sp,k1

	/*
	 * Move CPU mode to kernel, interrupts still disabled
	 */
	KERNEL_MODE(t0,t1)

	GETCPU_1(s8,2)
	la		gp,_gp
	GETCPU_2(s8,2)

	addiu	s0,k0,-REG_OFF	// Back to start
	SAVE_PERFREGS(s0,0)

	
deliver_user:
	mfc0	t8,CP0_CAUSE
	mfc0	s2,CP0_BADVADDR
	andi	s1,t8,MIPS_CAUSE_MASK	# Extract index
	lw		s1,__mips_cause2sig_map(s1)
	bgez	t8,deliver_user_synth
	 li		t8,SIGCODE_BDSLOT
	or		s1,t8

deliver_user_synth:
	/*
	 * S1 - signal code fields
	 * S2 - faulting address (if applicable)
	 * 
	 */
	ACQUIRE_KERNEL(INKERNEL_NOW+INKERNEL_LOCK,zero,1)
deliver_user_synth2:
	STI
	SMPREF(lw,a1,actives,s8)
	la		ra,__ker_exit
	move	a0,s1
	j		usr_fault
	 move	a2,s2

sys_exc:
	.set	noat
	/*
	 * Exception happened during kernel code (or ISR).  We'll need
	 * to nest onto the existing kernel stack.
	 */
	addiu	k0,sp,-(MIPS_CONTEXT_SIZE + 7)
	ori		k0,k0,7
	xori	k0,k0,7
	SAVE_REGS(0)
	addiu	sp,k0,-MIPS_CONTEXT_EXTRA_SIZE

	.set	at
	
sys_exc1:	
	mfc0 	a0,CP0_CAUSE		# Get cause
	
   	GETCPU_1(s8,2)
	la		gp,_gp
   	GETCPU_2(s8,2)

	andi	t0,a0,MIPS_CAUSE_MASK	
	lw		s1,__mips_cause2sig_map(t0)	# get signal code
	bgez	a0,1f
	 li		a0,SIGCODE_BDSLOT
	or		s1,a0
1:	
	GET_INKERNEL(s7,0) 			# remember original inkernel value

deliver_system:
	
	SMPREF(lw,s0,actives,s8)
	
	/*
	 * Special return?
	 */
#ifdef TXHACK
mfc0	t0,CP0_SREG
li		t1,0x00010000
or		t0,t0,t1
mtc0	t0,CP0_SREG
#endif
	SMPREF(lw,t0,cpupageptr,s8)
	IDHARDCRASH(k0,t1)
	lw		t0,CPUPAGE_STATE(t0)
	bne		t0,zero,hardcrash
	 andi	k0,s7,INKERNEL_SPECRET
	bne		k0,zero,fixup_specret
	 /* the "li a2,SIGCODE_USER" below is in the branch delay slot */
	
fixup_kcall:
	/*
	 * 	S0 - actives
	 *  S1 - sigcode 
	 *  S7 - original inkernel value
	 */
	 li		a2,SIGCODE_USER

	/*
	 * Exception occured in kernel, pass sigcode register
	 * and context to kdebug_callout which will call
	 * kdebug_enter with SIGCODE_USER as fault code.
	 */
	move	a0,s1
	jal		kdebug_callout
	 addiu	a1,sp,MIPS_CONTEXT_EXTRA_SIZE		# get context pointer
	 
	beq	v0,zero,__keriret	# is excp handled by kdebugger?
	 addiu	k0,sp,MIPS_CONTEXT_EXTRA_SIZE	# yes, restore regs and return
	 
	andi	t0,s7,INKERNEL_LOCK
	IDHARDCRASH(k0,t1)
	move	a0,s1
	bne		t0,zero,hardcrash3
	 addiu	a1,sp,MIPS_CONTEXT_EXTRA_SIZE	# get ctx ptr (for hardcrash)
	la		t9,xfer_handlers
	MSGOPT_SMPADDR(t9,s8)
	lw		t0,(t9)
	beq		t0,zero,no_xfer
	 move	a2,s1
	/*
	 * Hook to branch out to special handling code when a
	 * fault occurs during a transfer.
	 */
	sw		zero,(t9)
	lw		t0,0(t0)
	move	a0,s0
	jal		t0
	 addiu	a1,sp,MIPS_CONTEXT_EXTRA_SIZE
	
no_xfer:
	/*
	 * Clear timeout flags
	 */
	sw		zero,TIMEOUT_FLAGS(s0)

	/*
	 * Tell him about the fault
	 */
	li		t1,ERRNO_EFAULT
	SAVE_ONE_REG(t1,V0,REG_OFF,s0)
	lw		t1,TFLAGS(s0)
	li		t7,_NTO_TF_KERERR_SET
	or		t6,t1,t7
	sw		t6,TFLAGS(s0)

	/*
	 * Advance the PC, but only the first time we see he
	 * needs to receive an error.
	 */
	and		t1,t1,t7
	bne		t1,zero,__ker_exit
	 nop
	RESTORE_ONE_REG(t1,EPC,REG_OFF,s0)
	addiu	t1,t1,KERERR_SKIPAHEAD
	j		__ker_exit
	 SAVE_ONE_REG(t1,EPC,REG_OFF,s0)
	 
fixup_specret:
	/*
	 * Lock us in the kernel, clear specret
	 */
	ori		t1,s7,INKERNEL_SPECRET+INKERNEL_LOCK
	xori	t1,t1,INKERNEL_SPECRET
	NEW_INKERNEL_BITS_REG(t1,t1)

	/* if we took a WATCH fault processing the _NTO_TF_SHORT_MSG then we simply
	 * note the fact by setting _NTO_ATF_WATCHPOINT, and allow the message copy
	 * to complete
	 */
	andi	t1, s1, 0xff
	li		t0, SIGTRAP
	bne 	t0, t1, 2f
	 lw		t2, PROCESS(s0)

	lw		t1,	debug_detach_brkpts
	jalr	t1
	 lw		a0, DEBUGGER(t2)

#if defined(VARIANT_smp)
1:
	ll		t0,ATFLAGS(s0)
	ori		t0,_NTO_ATF_WATCHPOINT
	sc		t0,ATFLAGS(s0)
	beq 	t0,zero,1b
	 nop
#else
	lw		t0,ATFLAGS(s0)
	ori		t0,_NTO_ATF_WATCHPOINT
	sw		t0,ATFLAGS(s0)
#endif
	j		__ker_exit
	 nop

 2:
	/*
	 * Mask out the thread flag SPECRET bit which
	 * is currently being processed.
	 */
	lw		t7,inspecret
	lw		t1,TFLAGS(s0)
	nor		t7,zero,t7
	and		t1,t7,t1
	j		fixup_kcall
	 sw		t1,TFLAGS(s0)

ENDFRAME(r4k_exception_handler)

/*
 * ker_start()
 *	Set up the environment and jump into the scheduler to run processes
 */
FRAME(ker_start,sp,0,ra)
#if defined(VARIANT_smp)
// 
// The X86 version has looping to make sure that multiple people
// don't get into the kernel at the same time, but I don't think it's
// needed since idle() cycles the CPU's into the system one at a time
// and doesn't let anybody do anything until all the CPU's are up
// and running. - bstecher
//
	GETCPU_1(s8,2)
	lui		t3,%hi(ker_stack)
	GETCPU_2(s8,2)
	addu	t3,s8
	lw		sp,%lo(ker_stack)(t3)
	lw		t0,run_ker_stack_top
	lw		t1,run_ker_stack_bot
	sw		t0,ker_stack_top
	j		__ker_exit_start
	 sw		t1,ker_stack_bot
#else
	j		__ker_exit_start
	 lw		sp,ker_stack
#endif
ENDFRAME(ker_start)

/*
 * r4k_syscall_handler()
 *	Streamlined path for our most common operation--kernel calls
 */
FRAME(r4k_syscall_handler,sp,0,ra)
	.set	noat

	/*
	 * Coming from user mode.  Save user registers, and get
	 * a fresh kernel stack.  Move GP to our own short data
	 * area.
	 */
	LD_ACTIVE_AND_KERSTACK(k0,k1)
	addiu	k0,k0,REG_OFF

	SAVE_REGS(1)

	.set	at

	move	sp,k1
	addiu	s0,k0,-REG_OFF	/* put actives in a non-volatile register */

	/*
	 * Move CPU mode to kernel, interrupts still disabled
	 */
	KERNEL_MODE(t0,t1)

	GETCPU_1(s8,2)
	la		gp,_gp
	GETCPU_2(s8,2)

	ACQUIRE_KERNEL(INKERNEL_NOW,zero,1)

	SAVE_PERFREGS(s0,0)

	/*
	 * Interrupts are now OK again
	 */
	STI

	/*
	 * Clear the "saw error" flag
	 */
	lw		t0,TFLAGS(s0)
	li		t1,~(_NTO_TF_KERERR_SET | _NTO_TF_BUFF_MSG)
	and		t0,t0,t1
	sw		t0,TFLAGS(s0)
	/*
	 * Kernel call number should still be intact in v0.
	 * Save the kernel call number.
	 */
	sw		v0,SYSCALL(s0)

	/*
	 * Make sure it's a valid syscall number.
	 */
	sltiu	t0,v0,__KER_BAD
	beq		t0,zero,bad_func
#if defined(VARIANT_instr)
	 la		t1,_trace_call_table
#else
	 la		t1,ker_call_table
#endif

	/*
	 * Index the call table and run the C code
	 */
	sll		v0,v0,2
	addu	t1,v0
	move	a0,s0
	lw		t0,0(t1)
	jal		t0
	 addiu	a1,s0,REG_OFF+MIPS_AREG(MIPS_REG_A0)
	bgez 	v0,set_err
	 nop
enoerror:
	/*
	 * Fall through to __ker_exit()
	 */
ENDFRAME(r4k_syscall_handler)

/*
 * __ker_exit()
 *	Return to previously executing space
 *
 * Honors a bunch of edge conditions for pending/special actions
 */
FRAME(__ker_exit,sp,0,ra)
	.set	at
	/*
	 * Set inkernel to locked/now
	 */
#if 0
//DEBUG: code to test to make sure we've already acquired the kernel
//and are on the right CPU in an SMP system.
lw t0,inkernel
andi t1,t0,0xff00
beq t1,zero,1f
srl	t2,t0,24-2
beq t2,s8,2f
nop
1:
DEBUGKDBREAK
2:
#endif
__ker_exit_start:
	GETCPU_1(s8,2)
	NEW_INKERNEL_BITS(t3,INKERNEL_NOW+INKERNEL_LOCK+INKERNEL_EXIT);
	GETCPU_2(s8,2)
	/*
	 * s0 will point to the active process for the duration
	 * of this function.
	 */
	SMPREF(lw,s0,actives,s8)
	
	/*
	 * We can take interrupts now, even if we were called from
	 * an interrupt handler.
	 */
	STI

	/*
	 * Flush pending events.  We'll flush again with interrupts
	 * locked out, but this'll get most of them.
	 */
	lw	t4,intrevent_pending
	bne	t4,zero,ker_intrevent
	 lw	t1,PROCESS(s0)
	/*
	 * Check for a process switch since we may need to remove breakpoints
	 */
	SMPREF(lw,t2,actives_prp,s8)
	bne t1,t2,dbg_check
	 nop
dbgret:

	/*
	 * Check for a aspace switch since we may need to change aspace mappings
	 */
 	la	a1,aspaces_prp 	/* aspace_switch uses A0, A1 */
	SMPADDR(a1,s8)
	lw	a0,ASPACE_PRP(s0)
	lw	t1,(a1)
	bne	a0,t1,aspace_switch
	 nop
aspaceret:

	/*
	 * Check for a process switch since we may need to add breakpoints 
	 * and change PLS
	 */
	lw	t1,PROCESS(s0)
	SMPREF(lw,t2,actives_prp,s8)
	bne t1,t2,prp_switch
	 nop
prpret:

	/*
	 * We continue back here after letting the memory manager
	 * decide whether (or not) to switch the address space.  Things
	 * like proc manager threads will not require such a switch.
	 */
	lw	t3,TFLAGS(s0)
	li	t1,_NTO_TF_SPECRET_MASK
	and	t3,t1,t3
	bne	t3,zero,ker_specialret
	 nop

	/*
	 * Save/Restore FPU
	 */
	SMPREF(lw,t0,actives_fpu,s8)
	bne	t0,s0,fpusave
	 /* first instruction of CLI is in branch shadow */
fpuret:

#if defined(VARIANT_instr)
	lw	t0,ker_exit_enable_mask
	beq	t0,zero,1f
	  nop
	jal	_trace_ker_exit
	 move	a0,s0
1:
#endif

kerret:
	/*
	 * No more interrupts.  Check for a couple special cases,
	 * otherwise we're out of here.
	 */
	CLI
	lw	t4,intrevent_pending
	bne	t4,zero,ker_intrevent
	 nop
	sw	zero,queued_event_priority
	
	KILL_LOAD_LINKED(t0)
	
__ker_emu_restart:
#if defined(VARIANT_smp)
1:
	ll		s1,ATFLAGS(s0)
	li		t5,0
	sc		t5,ATFLAGS(s0)
	beq 	t5,zero,1b
	 nop
#else
	lw		s1,ATFLAGS(s0)
	sw		zero,ATFLAGS(s0)
#endif
	bne		s1,zero,ker_atflags
	 nop
	SMPREF(lw,t1,cpupageptr,s8)
 	lw	t4,TLS(s0)
	/*
	 * Refresh the system segment pointer and update TLS variable in it
	 */
	sw	t4,CPUPAGE_TLS(t1)

	NEW_INKERNEL_BITS_REG(zero,zero)

__ker_exit2:
#if defined(VARIANT_smp)

// If need_to_run is set and we're the target cpu, we force
// whoever is executing to make a null syscall so that we'll
// eventually return through the scheduler and take care of
// whoever it is that needs to run.
//
	lw		t0,need_to_run
	beq		t0,zero,11f
	 srl	t1,s8,2
	lw		t0,need_to_run_cpu
	bne		t1,t0,11f
	 nop

	li		t2,_NTO_ATF_FORCED_KERNEL
	ori		t3,t2,_NTO_ATF_SMP_RESCHED
1:
	ll		t0,ATFLAGS(s0)
	or		t1,t0,t3
	sc		t1,ATFLAGS(s0)
	beq		t1,zero,1b
	 and 	t1,t0,t2
	bne		t1,zero,11f
     // force this thread to do a null syscall so we'll resched
	 RESTORE_ONE_REG(t0,V0,REG_OFF,s0)
	RESTORE_ONE_REG(t1,EPC,REG_OFF,s0)
	sw		t0,ARGS_ASYNC_TYPE(s0)
	sw		t1,ARGS_ASYNC_IP(s0)
	li		t0,__KER_NOP
	lw		t1,kercallptr
	SAVE_ONE_REG(t0,V0,REG_OFF,s0)
	SAVE_ONE_REG(t1,EPC,REG_OFF,s0)

11:	

// Sometimes we aren't able to drain the interrupt event queue
// so here we check if we've got anything pending that hasn't 
// been processed yet.  If there are some interrupt events, we
// send an IPI to the next cpu numerically and hope that they're
// able to take care of it.
// 
// note: there is another check, identical to this one down in
//       case4/5 of intr_done.
//          
	lw		t0,intrevent_pending
	beq		t0,zero,9f

    // tell another cpu to deal with this.  we pick another cpu
    // by incrementing our cpu number and doing a modulo on the
    // number of cpu's present.
	 srl	a0,s8,2
	lw		t0,num_processors
	addiu	a0,1
	bne		a0,t0,1f
	 nop
	li		a0,0
1:
	jal		send_ipi
	 li		a1,IPI_CHECK_INTR						

9:
#endif
#ifdef 	VARIANT_instr
restore_perfregs:
	/* save performance counter registers, if necessary */
	SMPREF(sw,zero,actives_pcr,s8)
	lw		a0,CPUDATA(s0)
	la		a1,disabled_perfregs
	beq		a0,a1,dont_restore_perfregs
	 nop
	jal		cpu_restore_perfregs
	 nop
	SMPREF(sw,s0,actives_pcr,s8)
dont_restore_perfregs:
#endif
	/*
	 * Point to the register save area, pop their values,
	 * and return from whence we came.
	 */
	addiu	k0,s0,REG_OFF

//SHOWPROGRESS(0x20,1,t0,t1)
__keriret:
//SHOWPROGRESS(0x30,1,t0,t1)
#if 0
//DEBUG: code to hang the other CPU when one goes into the kernel debugger
.data
.global iret_regs
iret_regs: .long 0
	.long 0
.text
la t1,iret_regs(s8)
1:
lw t0,kd_slock
bne t0,zero,1b
 sw k0,(t1)
sw zero,(t1)
#endif
#if 0
//DEBUG: code to stop a particular CPU
.data
.global stopme
stopme: .long -1
.text
lw t0,stopme
bne t0,s8,1f
 move t1,k0
// k0 saved in t1 so we can see it in the kernel debugger and
// so it can be restored after we return from kdebug.
DEBUGKDBREAK
move k0,t1
1:
#endif

#ifdef TXHACK
mfc0	t0,CP0_SREG
li		t1,0x00010000
or		t0,t0,t1
mtc0	t0,CP0_SREG
#endif

	.set	noat

	RESTORE_REGS(k0)

#if defined(VARIANT_r3k)
	// RESTORE_REGS put the EPC value into K1
	j	k1
	 rfe
#else
	sync	//R7K bug workaround
	.set mips3
	eret
	 nop
	.set MIPSARCH
#endif

	/*
	 * We need to set up to drain off pending interrupt events.
	 * We cobble up the return address so the C function will
	 * come back to run at the regular __ker_exit().
	 */
	.set	at
ker_intrevent:
	la	ra,__ker_exit
	j	intrevent_drain
	 move	a0,s0

	/* 
	 * take out breakpoints for old process
	 */
dbg_check:
	beq		t2,zero,dbgret
	 nop
	lw		s1,DEBUGGER(t2)
	beq		s1,zero,dbgret
	 la 	a1,aspaces_prp
	SMPADDR(a1,s8)
	lw		t0,0(a1)
	beq 	t2,t0,1f
	 lw		t4,MEMMGR_ASPACE+memmgr
	jal		t4
	 move 	a0,t2
1:
	lw		t1,debug_detach_brkpts	
	la		ra,dbgret
	j		t1
	 move	a0,s1
	 
	/*
	 * Ask the memory manager if he'd like to switch processes,
	 * as aspaces_prp no longer matches.  He may switch it or not,
	 * and will return to continue the switch out to the active
	 * task. (A0 & A1 have already been set by __ker_exit code)
	 */
aspace_switch:
	beq	a0,zero,aspaceret
	 lw	t4,MEMMGR_ASPACE+memmgr
	/*
	 * map_address(actives->aspace_prp, &aspaces_prp)
	 */
	la	ra,aspaceret
	j	t4
	 nop
	 
	/*
	 * set actives_prp, put in breakpoints for new process
	 */
prp_switch:
	SMPREF(sw,t1,actives_prp,s8)
	SMPREF(lw,t0,cpupageptr,s8)
	lw		t2,PLS(t1)
	sw		t2,CPUPAGE_PLS(t0)
	lw		a0,DEBUGGER(t1)
	beq		a0,zero,prpret
	 la		ra,prpret
	lw		t1,debug_attach_brkpts
	j		t1
	 nop

	/*
	 * Take special actions, then continue at __ker_exit
	 */
ker_specialret:

	/*
	 * Reset the kernel stack; the old context is not needed, and a
	 * sequence of specialret's intermingled with pagewaits can
	 * cause the stack to grow indefinitely. Do the load explicitly 
	 * since a "lw sp,ker_stack" may be split into two instructions
	 * and the stack pointer won't be valid after the first one. 
	 */
	lui		t0,%hi(ker_stack)
	SMPADDR(t0,s8)
	lw		sp,%lo(ker_stack)(t0)
#if defined(VARIANT_smp)
	li		t2,_NTO_ATF_FORCED_KERNEL
	CLI
1:
	ll		t0,ATFLAGS(s0)
	and		t1,t0,t2
	beq		t1,zero,2f
	 xor	t0,t1
	sc		t0,ATFLAGS(s0)
	beq		t0,zero,1b
	//
	// have to restore original registers - specialret looks at them
	//
	 FORCED_KERNEL_RESTORE(t0,s0)
2:
	STI
#endif
	
	/*
	 * Call specialret() C code, arrange for return to re-run
	 * __ker_exit().
	 */
	la		ra,__ker_exit
	j		specialret
	 move	a0,s0

	/*
	 * Deal with async thread flags
	 */
ker_atflags:
#if defined(VARIANT_smp)
	li		t0,_NTO_ATF_FORCED_KERNEL
	and		t0,s1
	beq		t0,zero,1f
	 FORCED_KERNEL_RESTORE(t0,s0)
#endif
1:
	STI
#ifdef TXHACK
	mfc0	t0,CP0_SREG
	li		t1,0x00010000
	or		t0,t1,t1
	mtc0	t0,CP0_SREG
#endif
	andi 	t0,s1,_NTO_ATF_FPUSAVE_ALLOC
	beq		t0,zero,1f
	 nop
	jal		fpusave_alloc
	 nop
1:
	andi 	t0,t4,_NTO_ATF_WATCHPOINT
	beq		t0,zero,1f
	 nop
	lw		t1,PROCESS(s0)
	lw		t0,debug_attach_brkpts
	jalr	t0
	 lw		a0,DEBUGGER(t1)

1:
	andi 	t0,s1,_NTO_ATF_TIMESLICE+_NTO_ATF_SMP_RESCHED
	beq		t0,zero,1f
	 nop
2:
#if defined(VARIANT_smp)
	lw		t4,resched
	jal		t4
#else
	lw		t4,resched
	jal		t4
1:
#endif
	 nop
	j		__ker_exit
	 nop
	 
#if defined(VARIANT_smp)
1:
	lw		t0,need_to_run
	beq		t0,zero,__ker_exit
	 srl	t0,s8,2
	lw		t1,need_to_run_cpu
	bne		t0,t1,__ker_exit
	 nop
	b		2b
	 nop
#endif
	 
	/*
	 * Switch FP context. We don't actually do that here - just disable
	 * access to CU1 so that it happens at next FP instruction.
	 */
fpusave:
	lw		t0,MIPS_AREG(MIPS_REG_SREG)+(MIPS_REGS_LOW_WORD*4)+REG_OFF(s0)
	li		t1,~MIPS_SREG_CU1
	and		t0,t1
	j		fpuret
	 sw		t0,MIPS_AREG(MIPS_REG_SREG)+(MIPS_REGS_LOW_WORD*4)+REG_OFF(s0)
	 
set_err:
	/*
	 * NOTE: We're setting the error on the initial active thread (in s0),
	 * not on what's in actives[KERNCPU] after the call.
	*/
	bne 	v0,zero,real_err
	 NEW_INKERNEL_BITS(t3,INKERNEL_NOW+INKERNEL_LOCK+INKERNEL_EXIT)
	SAVE_ONE_REG(zero,V0,REG_OFF,s0)
	b		enoerror
	 nop

bad_func:
	/*
	 * Invalid system call number.  Bad programmer, no biscuit.
	 */
	 li		v0,ENOSYS
	 /* fall through */
real_err:
	move	a1,v0
	move	a0,s0
	la		ra,__ker_exit
	j		kererr
	 nop
	 
ENDFRAME(__ker_exit)

/*
 * r4k_interrupt_handler()
 *	Overall interrupt handler for R4k systems.
 *  This code is not used in place. Rather, it is copied elsewhere and
 *  intermixed with interrupt controller bursts from the startup module.
 *  Make sure that only position independent code transfers are used here
 *  (referencing absolute data addresses is OK (required, actually)).
 */

	.globl	intr_entry_start
intr_entry_start:
	.set	noat

	/*
	 * Bump the interrupt level
	 */
	ADJUST_INKERNEL(k0,k1,k1,1,1)
#if defined(VARIANT_smp)
	BRANCH_INKERNEL(k1,1f)
#else
	addiu	k1,-1
	bne		k1,zero,1f
#endif
	/*
	 * Nope, this is our first time in. Save the registers to
	 * the thread register save area and start the kernel stack off fresh.
	 */
	 LD_ACTIVE_AND_KERSTACK(k0,k1)
	b		2f
	 addiu	k0,k0,REG_OFF
	
1:
	 /*
	  * We're nested, lop off some more stack.
	  */
	addiu	k0,sp,-(MIPS_CONTEXT_SIZE + 7)
	ori		k0,k0,7
	xori	k0,k0,7
	addiu	k1,k0,-MIPS_CONTEXT_EXTRA_SIZE

2:	
	SAVE_REGS(0)
	
	move	sp,k1
	
    addiu	k0,-MIPS_CONTEXT_EXTRA_SIZE
	subu	s6,k0,k1	// set S6: in system => 0, in user => !0 
	
	.set	at

	/*
	 * Our registers are saved and our SP is valid
	 * Move CPU mode to kernel, interrupts still disabled
	 */
	KERNEL_MODE(t0,t1)

	GETCPU_1(s8,2)
	la		gp,_gp
	GETCPU_2(s8,2)

	beq		s6, zero, 31f	// only save if in user
	 addiu	s0,k0,MIPS_CONTEXT_EXTRA_SIZE-REG_OFF	// Start of actives
	SAVE_PERFREGS(s0,1)		
31:

   	ACQUIRE_INTR_SLOCK(t0,t1)

	/*
	 * Fall into startup/generated interrupt level identification code.
	 * This is the end point of the code that gets copied.
	 */

	.global intr_entry_end
intr_entry_end:
	 .type intr_entry_start,@function
	 .type intr_entry_end,@function

FRAME(intr_process_queue,sp,0,ra)
	/*
	 * Virtually speaking, the interrupt queue is processed
	 * at this point. Later on, interrupt() should be coded in assembly
	 * and moved here for efficiency's sake.
	 */
	 
	# generated code puts interrupt level ptr in S1

	KILL_LOAD_LINKED(t0)
#if defined(VARIANT_smp)
	RELEASE_INTR_SLOCK
	move	s7,ra
	jal		interrupt
	 move	a0,s1
	ACQUIRE_INTR_SLOCK(t0,t1)
	j		s7
	 nop
#else
	j	interrupt
	 move	a0,s1		
#endif
ENDFRAME(intr_process_queue)

FRAME(intr_done_chk_fault,sp,0,ra)
	/*
	 * All done dispatching this interrupt. An EOI has been sent to the
	 * interrupt controller by the startup/generated code.
	 * See if we've got a CPU exception and, if so, handle it. If not,
	 * figure out how to continue the interrupted context.
	 * Interupts are disabled.
	 */
	 
	 RELEASE_INTR_SLOCK
	/*
	 * Lower the inkernel count
	 */
	ADJUST_INKERNEL(t2,s7,t0,-1,0)

	beq		a0,zero,intr_done2
	 // delay slot in instruction below

	/*
	 * exception in user code (and not an interrupt handler)
	 * so deliver the exception, no questions asked.
	 */
	 move	s1,a0				// put sigcode in correct register
	bne		s6,zero,deliver_user_synth
	 move	s2,a1				// put reference addr in correct reg
	 
	// exception in system code or interrupt handler:
	b		deliver_system
	 mtc0	a1,CP0_BADVADDR		// in case someone wants the reference addr
ENDFRAME(intr_done_chk_fault)

FRAME(intr_done,sp,0,ra)
	/*
	 * All done dispatching this interrupt. An EOI has been sent to the
	 * interrupt controller by the startup/generated code.
	 * Figure out how to continue the interrupted context.
	 * Interupts are disabled.
	 */

	RELEASE_INTR_SLOCK
	
	/*
	 * Lower the inkernel count
	 */
	ADJUST_INKERNEL(t0,s7,t1,-1,0)

intr_done2:
	KILL_LOAD_LINKED(t0)
	
//
// We have 6 cases to consider (3 in non-smp).
//
//          This CPU      Another CPU    Action
//			-------------------------
// case1    from user     in user        Become the kernel
// case2    from user     in kernel      Check preempt and maybe ipi (SMP)
// case3    from kernel   in user        Check preempt and maybe become kernel
// case4    from intr     in user        Return to user
// case5    from intr     in kernel      Return to user (SMP)
// case6    kacquire spin *              Try to become kernel, else return (SMP)
//
// We can examine the previous privity, inkernel, and cpunum
// to figure out where we came from and where to go
//

	beq		s6,zero,from_kerorintr
	 // branch delay instruction from below
	
case1:
#if defined(VARIANT_smp)
	 sll	t3,s8,24-2
	li		t4,0x00ffffff
1:
	ll		t0,inkernel
	andi	t1,t0,INKERNEL_NOW+INKERNEL_LOCK
	bne		t1,zero,case2
	 ori	t2,t0,INKERNEL_NOW+INKERNEL_LOCK
	and		t2,t4
	or		t2,t3
	sc		t2,inkernel
	beq		t2,zero,1b
#endif
	 nop
	b		__ker_exit
	 nop
	 
#if defined(VARIANT_smp)
case2:
	lw		s0,actives(s8)
	andi	t1,t0,INKERNEL_LOCK+INKERNEL_SPECRET
	bne		t1,zero,__ker_exit2
	 lbu	t2,PRIORITY(s0)
	lw		t3,queued_event_priority
	sltu	t2,t3,t2
	bne		t2,zero,__ker_exit2
	// We've got something queued that's a higher priority than us,
	// Send an IPI to the guy in the kernel so that he knows to
	// reschedule things.
	 li		a1,IPI_CHECK_INTR					
	la		ra,__ker_exit2
	j		send_ipi
	 lbu	a0,cpunum
#endif
	
	
from_kerorintr:
#if defined(VARIANT_smp)
	andi	t1,s7,INKERNEL_LOCK
	bne		t1,zero,__keriret
	 addiu	k0,sp,MIPS_CONTEXT_EXTRA_SIZE
	SMPREF(lw,t0,cpupageptr,s8)
	lw		t0,CPUPAGE_STATE(t0)
	bne		t0,zero,__keriret
	 la		t1,beg_acquire_kernel_attempt
	RESTORE_ONE_REG(t0,EPC,MIPS_CONTEXT_EXTRA_SIZE,sp)
	sltu	t2,t0,t1
	bne		t2,zero,case3
	 addiu	t1,(end_acquire_kernel_attempt-beg_acquire_kernel_attempt)
	sltu	t2,t0,t1
	beq		t2,zero,case3
	 //Branch delay instr below

case6:
	la		t1,sys_acquire_kernel
	sltu	t1,t0,t1
	beq		t1,zero,__keriret	# Waiting in system to become kernel
	 addiu	k0,sp,MIPS_CONTEXT_EXTRA_SIZE
	RESTORE_ONE_REG(t2,A0,MIPS_CONTEXT_EXTRA_SIZE,sp)
	li		t1,INKERNEL_NOW
	bne		t2,t1,1f
	 nop
	 // Waiting for entry on syscall - have to back up EPC 
	 // to re-execute.
	lw		s0,actives(s8)
	RESTORE_ONE_REG(t3,EPC,REG_OFF,s0)
	addiu	t3,-KER_ENTRY_SIZE
	SAVE_ONE_REG(t3,EPC,REG_OFF,s0)
1:
	lui		t3,%hi(ker_stack)
	addu	t3,s8
	b		case1
	 lw		sp,%lo(ker_stack)(t3)
#else
	andi	t1,s7,INKERNEL_LOCK+INKERNEL_INTRMASK
	bne		t1,zero,case4
	 //Branch delay instr below
#endif

case3:
	 SMPREF(lw,s0,actives,s8)
	lw		t3,queued_event_priority
	lbu		t2,PRIORITY(s0)
	sltu	t2,t2,t3
	bne		t2,zero,preempt
	 nop
	
case4:
case5:
#if defined(VARIANT_smp)
//
// Similar to before, if we get here and there's an interrupt
// event pending that we haven't handled then we fire off an
// ipi to another cpu to get them to handle it.
//
	lw		t0,intrevent_pending
	beq		t0,zero,9f
	 
    // if we're here it means there's an interrupt event pending
	// but we're not going to drain it.  so we tell another cpu 
	// to deal with it so that the event gets drained in a timely
	// fashion.
    //
	// we pick another cpu by incrementing our cpu number and doing 
    // a modulo on the number of cpu's present.

	 srl	a0,s8,2
	lw		t0,num_processors
	addiu	a0,1
	bne		a0,t0,1f
	 nop
	li		a0,0
1:
	jal		send_ipi
	 li		a1,IPI_CHECK_INTR						
9:
#endif
	b		__keriret
	 addiu	k0,sp,MIPS_CONTEXT_EXTRA_SIZE

preempt:
#if 0
//DEBUG: code to test to make sure we've already acquired the kernel
//and are on the right CPU in an SMP system.
andi t1,s7,0xff00
beq t1,zero,1f
srl	t2,s7,24-2
beq t2,s8,2f
nop
1:
DEBUGKDBREAK
2:
#endif
	andi	t1,s7,INKERNEL_EXIT
	bne		t1,zero,1f
	 RESTORE_ONE_REG(t1,EPC,REG_OFF,s0)
	addiu	t1,-KER_ENTRY_SIZE
	SAVE_ONE_REG(t1,EPC,REG_OFF,s0)
1:
	ori		t0,s7,INKERNEL_NOW+INKERNEL_LOCK+INKERNEL_SPECRET+INKERNEL_EXIT
	xori	t0,INKERNEL_SPECRET
	NEW_INKERNEL_BITS_REG(t0,t0)
	
1:
	la		t1,xfer_handlers
	MSGOPT_SMPADDR(t1,s8)
	lw		t0,(t1)
	beq		t0,zero,__ker_exit
	 nop

	/*
	 * Yup, there was a transfer handler.  Get it and clear
	 * the flag.
	 */
	lw		t0,4(t0)
	sw		zero,(t1)

	/*
	 * Get the restart code, and run it unless it's the NULL pointer.
	 */
	beq		t0,zero,__ker_exit
	 move 	a0,s0
	la		ra,__ker_exit
	j		t0
	 addiu	a1,sp,MIPS_CONTEXT_EXTRA_SIZE

	.set	at
ENDFRAME(intr_done)


FRAME(set_l1pagetable,sp,0,ra)
	
   	KILL_LOAD_LINKED(t0)
	SET_L1PAGETABLE(a0)
	j		ra
	 mtc0	a1,CP0_TLB_HI
	 
ENDFRAME(set_l1pagetable)


FRAME(cpu_force_fpu_save,sp,0,ra)
	.set mips3
	lw		t0,FPUDATA(a0)
	CLEAR_FPUPTR(t0)
	cfc1	t1,$31
	sdc1	 $f0, 0*8(t0)
	sdc1	 $f1, 1*8(t0)
	sdc1	 $f2, 2*8(t0)
	sdc1	 $f3, 3*8(t0)
	sdc1	 $f4, 4*8(t0)
	sdc1	 $f5, 5*8(t0)
	sdc1	 $f6, 6*8(t0)
	sdc1	 $f7, 7*8(t0)
	sdc1	 $f8, 8*8(t0)
	sdc1	 $f9, 9*8(t0)
	sdc1	$f10,10*8(t0)
	sdc1	$f11,11*8(t0)
	sdc1	$f12,12*8(t0)
	sdc1	$f13,13*8(t0)
	sdc1	$f14,14*8(t0)
	sdc1	$f15,15*8(t0)
	sdc1	$f16,16*8(t0)
	sdc1	$f17,17*8(t0)
	sdc1	$f18,18*8(t0)
	sdc1	$f19,19*8(t0)
	sdc1	$f20,20*8(t0)
	sdc1	$f21,21*8(t0)
	sdc1	$f22,22*8(t0)
	sdc1	$f23,23*8(t0)
	sdc1	$f24,24*8(t0)
	sdc1	$f25,25*8(t0)
	sdc1	$f26,26*8(t0)
	sdc1	$f27,27*8(t0)
	sdc1	$f28,28*8(t0)
	sdc1	$f29,29*8(t0)
	sdc1	$f30,30*8(t0)
	sdc1	$f31,31*8(t0)
	.set MIPSARCH
	sw		t1,REG_FPCR31(t0)
	j		ra
	 sw		t0,FPUDATA(a0)
ENDFRAME(cpu_force_fpu_save)

#if defined(VARIANT_smp)

// 
// A0 - has bits to turn on in 'inkernel' variable.
//
// Return original inkernel value in S7
//
usr_acquire_kernel:

beg_acquire_kernel_attempt:
	STI
1:
	lw		t0,need_to_run_cpu
	sll		t0,2
	beq		s8,t0,2f
	 lw		t0,need_to_run
	bne		t0,zero,1b
	 nop
2:
   	ll		s7,inkernel
	andi	t1,s7,INKERNEL_NOW
	bne		t1,zero,1b
	 nop

	CLI
	li		t1,0x00ffffff
	and		t0,t1,s7
	sll		t1,s8,24-2
	or		t0,a0
	or		t0,t1
	sc		t0,inkernel
	beq		t0,zero,beg_acquire_kernel_attempt
	 nop
	jr		ra
	 sync
		   
			 
sys_acquire_kernel:
	STI
1:
	ll		s7,inkernel
	andi	t0,s7,INKERNEL_NOW
	beq		t0,zero,2f
	 srl	t0,s7,24-2
	bne		t0,s8,1b
2:
	 CLI
end_acquire_kernel_attempt:
	li		t1,0x00ffffff
	and		t0,t1,s7
	sll		t1,s8,24-2
	or		t0,a0
	or		t0,t1
	sc		t0,inkernel
	beq		t0,zero,sys_acquire_kernel
	 nop
	j		ra
	 sync
	
	
//
// SMP inter-processor interrupt handler
//
// Can't use S0-S3,S6,S8 in this routine without saving/restoring them - they
// are expected to be stable from the id to eoi callout.
//

	.global	intr_process_ipi
intr_process_ipi:
	move	s7,ra

	RELEASE_INTR_SLOCK
	
	SMPREF(lw, t0,cpupageptr,s8)
	li		t1,1
	lw		s4,CPUPAGE_STATE(t0)
	sw		t1,CPUPAGE_STATE(t0)
	sync

	la		t3,ipicmds(s8)
1:
	ll		s5,(t3)
	li		t1,0
	sc		t1,(t3)
	beq		t1,zero,1b
	 andi	t1,s5,IPI_TLB_SAFE
	beq		t1,zero,1f
	 nop

	bne		s4,zero,1f		// Don't set if we're in an interrupt handler
	 nop

	jal		set_safe_aspace
	 srl	a0,s8,2
	CLI // Interrupts might have been turned on by aspace switch
1:
	andi	t1,s5,IPI_PARKIT
	beq		t1,zero,1f
	 srl	t0,s8,2
	lb		t1,alives(t0)
	ori		t1,0x2
	sb		t1,alives(t0)	// Mark the CPU as parked

	// Freeze the system, we've got a kernel dump happening 
2:	b		2b
	 nop

1:	
	andi	t1,s5,IPI_CLOCK_LOAD
	beq		t1,zero,1f
	 nop
	jal		clock_load
	 nop
	CLI // Interrupts are turned on by clock_load()
1:
	andi	t1,s5,IPI_INTR_MASK
	beq		t1,zero,1f
	 nop
	jal		interrupt_smp_sync
	 li		a0,INTR_FLAG_SMP_BROADCAST_MASK
1:
	andi	t1,s5,IPI_INTR_UNMASK
	beq		t1,zero,1f
	 nop
	jal		interrupt_smp_sync
	 li		a0,INTR_FLAG_SMP_BROADCAST_UNMASK
1:
	andi	t1,s5,IPI_TLB_FLUSH
	beq		t1,zero,1f
	 nop
	SMPREF(lw,a0,aspaces_prp,s8)
	//KLUDGE here. Direct referencing a function in procnto memory
	//manager. Means we can't build a standalone SMP kernel.
	jal		smp_sync_tlb 
	 nop
	CLI		//might have been enabled by function call
1:
	li		a0,0
	andi	t1,s5,IPI_RESCHED
	beq		t1,zero,1f
	 nop
	ori		a0,_NTO_ATF_SMP_RESCHED
1:
	andi	t1,s5,IPI_TIMESLICE
	beq		t1,zero,1f
	 nop
	ori		a0,_NTO_ATF_TIMESLICE
1:
	andi	t1,s5,IPI_CONTEXT_SAVE
	beq		t1,zero,1f
	 nop
	la		t9,actives_fpu
	SMPADDR(t9,s8)
	lw		t0,(t9)
	
	beq		t0,zero,1f	
	 nop
	.set mips3
	lw		t2,FPUDATA(t0)
	CLEAR_FPUPTR(t2)
	cfc1	t1,$31
	sdc1	 $f0, 0*8(t2)
	sdc1	 $f1, 1*8(t2)
	sdc1	 $f2, 2*8(t2)
	sdc1	 $f3, 3*8(t2)
	sdc1	 $f4, 4*8(t2)
	sdc1	 $f5, 5*8(t2)
	sdc1	 $f6, 6*8(t2)
	sdc1	 $f7, 7*8(t2)
	sdc1	 $f8, 8*8(t2)
	sdc1	 $f9, 9*8(t2)
	sdc1	$f10,10*8(t2)
	sdc1	$f11,11*8(t2)
	sdc1	$f12,12*8(t2)
	sdc1	$f13,13*8(t2)
	sdc1	$f14,14*8(t2)
	sdc1	$f15,15*8(t2)
	sdc1	$f16,16*8(t2)
	sdc1	$f17,17*8(t2)
	sdc1	$f18,18*8(t2)
	sdc1	$f19,19*8(t2)
	sdc1	$f20,20*8(t2)
	sdc1	$f21,21*8(t2)
	sdc1	$f22,22*8(t2)
	sdc1	$f23,23*8(t2)
	sdc1	$f24,24*8(t2)
	sdc1	$f25,25*8(t2)
	sdc1	$f26,26*8(t2)
	sdc1	$f27,27*8(t2)
	sdc1	$f28,28*8(t2)
	sdc1	$f29,29*8(t2)
	sdc1	$f30,30*8(t2)
	sdc1	$f31,31*8(t2)
	sw		t1,REG_FPCR31(t2)
	sw		zero,(t9)
	sync
	// clear FPUDATA_BUSY, CPU # indicators, releases CPU waiting for save
	sw		t2,FPUDATA(t0)	
1:

	// force the thread in to the kernel, if need be
	beq		a0,zero,4f	// No reason to force kernel entry
	 li		t8,_NTO_ATF_FORCED_KERNEL
	 
	li		t6,0
	move	t7,s6

	lw		t5,actives(s8)
	bne		s6,zero,2f 
	 la		t1,beg_acquire_kernel_attempt
	RESTORE_ONE_REG(t0,EPC,MIPS_CONTEXT_EXTRA_SIZE,sp)
	sltu	t2,t0,t1
	bne		t2,zero,3f
	 addiu	t1,(end_acquire_kernel_attempt-beg_acquire_kernel_attempt)
	sltu	t2,t0,t1
	beq		t2,zero,3f
	la		t1,sys_acquire_kernel
	sltu	t1,t0,t1
	beq		t1,zero,3f	# Waiting in system to become kernel
	 RESTORE_ONE_REG(t2,A0,MIPS_CONTEXT_EXTRA_SIZE,sp)
	li		t1,INKERNEL_NOW
	bne		t2,t1,3f			

#if 1
	// We were spining waiting for the kernel. Set up to pretend like we
	// were in userland
	 li		t6,-KER_ENTRY_SIZE
	li		t7,-1
#else
	 nop
	b	3f
	 nop
#endif	 

2:
	or		a0,t8
3:
	ll		t0,ATFLAGS(t5)
	or		t1,t0,a0
	sc		t1,ATFLAGS(t5)
	beq		t1,zero,3b

	 and	t0,t8
	bne		t0,zero,4f
	 and	a0,t8
	beq		a0,zero,4f 

     // force this thread to do a null syscall so we'll process the flags
	 RESTORE_ONE_REG(t0,V0,REG_OFF,t5)
	RESTORE_ONE_REG(t1,EPC,REG_OFF,t5)
	sw		t0,ARGS_ASYNC_TYPE(t5)
	// adjust values to pretend we were in userland if we were spining
	// waiting for kernel
	addu	t1,t6
	move	s6,t7
	sw		t1,ARGS_ASYNC_IP(t5)
	bltz	t1,5f
	 li		t0,__KER_NOP
	b		6f 
	 lw		t1,kercallptr
5:
	// If the instruction pointer was in the KSEG region, use the
	// KSEG0 address of the kercall instuction - we might now have an
	// address space and the user address for it won't work then.
	lw		t1,sys_kercallptr
6:	
	
	SAVE_ONE_REG(t0,V0,REG_OFF,t5)
	SAVE_ONE_REG(t1,EPC,REG_OFF,t5)

4:	
	SMPREF(lw, t2,cpupageptr,s8)
	sw		s4,CPUPAGE_STATE(t2)
	
	ACQUIRE_INTR_SLOCK(t0,t1)
	j		s7
	 sync
	 
#endif

#if !defined(VARIANT_r3k)
sr7100_errata_workaround:
	/* k0 still holds the CP0_SREG value */
	li	k1,~MIPS_SREG_ERL
	and	k1,k0,k1
	mtc0	k0,CP0_SREG
	/* 32 nops as prescibed by errata doc */
	nop; nop; nop; nop; nop; nop; nop; nop;
	nop; nop; nop; nop; nop; nop; nop; nop;
	nop; nop; nop; nop; nop; nop; nop; nop;
	nop; nop; nop; nop; nop; nop; nop; nop;
	j sr7100_errata_return
	 nop
#endif
