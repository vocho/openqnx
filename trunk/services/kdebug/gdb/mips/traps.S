/*
 * traps.S - GDB exception handling code
 */

/*
 * Include files
 */
#include "asmoff.def"
#include <mips/asm.h>
#include <mips/cpu.h>

/*
 * Deal with load and store issues for MIPS32/MIPS64 architecture
 */
 
#if defined(VARIANT_32) || defined(VARIANT_r3k)
	#define MIPSARCH		mips2

	#define REG_POS_BASE	(MIPS_REGS_LOW_WORD*4)
		
	#define SAVE_ONE_REG_EXPANDED(src, dst, adj, base, tmp)	\
		SAVE_ONE_REG(src, dst, adj, base);	\
		sra tmp,src,31; \
		sw	tmp,MIPS_AREG(MIPS_REG_##dst)+((MIPS_REGS_LOW_WORD^1)*4)+(adj)(base)
		
	#define MV_INSTR(instr)	instr
	
	#define LS_INSTR(ls)	ls##w
#else
	#define MIPSARCH		mips3

	#define REG_POS_BASE	0
		
	#define SAVE_ONE_REG_EXPANDED(src, dst, adj, base, tmp)	\
		SAVE_ONE_REG(src, dst, adj, base)
		
	#define MV_INSTR(instr)	d##instr
	#define LS_INSTR(ls)	ls##d
#endif
	
#define RESTORE_ONE_REG(dst, src, adj, base)	\
	LS_INSTR(l)	dst, MIPS_AREG(MIPS_REG_##src)+REG_POS_BASE+(adj)(base)
	
#define SAVE_ONE_REG(src, dst, adj, base)	\
	LS_INSTR(s)	src,MIPS_AREG(MIPS_REG_##dst)+REG_POS_BASE+(adj)(base)

//To fix some funkiness with C macro expansion & token concatenation	
#define MIPS_REG_MYAT	MIPS_REG_AT

	.extern _stack_top
	.extern _stack_base
	.extern mips_signal_handler
	.extern handle_exception
	
	#define EXCEPT_TABLE_OFFSET	0x1c0
	
	#define R4K_EXCEPTION_TABLE	(MIPS_R4K_K0BASE+EXCEPT_TABLE_OFFSET)
	
#ifdef SAVE_EVERYTHING
	#define STASH_KxREGS	\
		dmtc1   k0,FPU_R1;	\
		dmtc1   k1,FPU_R2
	#define SAVE_KxREGS		\
		sdc1	FPU_R1,MIPS_AREG(MIPS_REG_K0)(k1);	\
		sdc1	FPU_R2,MIPS_AREG(MIPS_REG_K1)(k1)
#else
	#define	STASH_KxREGS
	#define SAVE_KxREGS
#endif
	

	.set MIPSARCH

	.globl exception_code_start
/*
 * exception_code_start[]
 *	Code to handle exceptions of the four basic types
 *
 * This snapshot of exception handling is copied to its real location
 * during startup.
 */
FRAME(exception_code_start,sp,0,ra)
	.set	noreorder
	.set	noat

	.globl	_tlb
_tlb:
	/*
	 * TLB error handler : 0x80000000
	 */
	STASH_KxREGS

	li      k0,R4K_EXCEPTION_TABLE
	lw      k0,0x84(k0)             # get handler address

	j       k0                      # jump to the handler
	 nop

	.org  exception_code_start + 0x80
#if !defined(VARIANT_r3k)
	.globl	_xtlb
_xtlb:
	/*
	 * XTLB error handler : 0x80000080
	 */
	STASH_KxREGS

	li      k0,R4K_EXCEPTION_TABLE
	lw      k0,0x88(k0)             # get handler address
	nop

	j       k0                      # jump to the handler
	 nop

	.org  exception_code_start + 0x100
	.globl	_cache
_cache:
	/*
	 * Cache error handler : 0x80000100
	 */
	STASH_KxREGS

	/*
	 * Must turn off ERL here, otherwise xkuseg becomes a 2 power-of 31
	 * uncached, unmapped address space which will then cause a TLB
	 * miss on the j AT below
	 */
	mfc0    k1,CP0_SREG             # read status register

	addiu   k0,zero,~MIPS_SREG_IE   # get IE mask
	and     k1,k1,k0                # mask off IE
	mtc0    k1,CP0_SREG             # write sreg
	 nop                            # let it settle

#if !defined(VARIANT_r3k)
	addiu   k0,zero,~MIPS_SREG_ERL  # get ERL mask
	and     k1,k1,k0                # mask off ERL
	mtc0    k1,CP0_SREG             # write sreg
	 nop                            # let it settle
#endif
	li      k0,R4K_EXCEPTION_TABLE
	lw      k0,0x80(k0)             # get handler address

	j       k0                      # jump to the handler
	 nop

	.org  exception_code_start + 0x180
#endif
	.globl	_excep
_excep:
	/*
	 * General exception handler : 0x80000180 (0x80000080 for R3K's)
	 */
	STASH_KxREGS

	mfc0    k0,CP0_CAUSE            # read Cause register

	la      k1,R4K_EXCEPTION_TABLE  # get table address
	andi    k0,k0,MIPS_CAUSE_MASK   # get Cause [5..0]
	addu    k1,k1,k0                # get address in table
	lw      k1,0(k1)                # read the handler address
	 nop

	j       k1                      # call the handler
	 nop

	.org  exception_code_start + EXCEPT_TABLE_OFFSET

	/*
 	 * Exception Table : 0x800001C0
	 */
	.globl	r4k_exception_table

r4k_exception_table:

	.word	r4k_exception_handler	/* 0  : interrupt		*/
	.word	r4k_exception_handler	/* 1  : TLB modification	*/
	.word	r4k_exception_handler	/* 2  : TLB exception (load)	*/
	.word	r4k_exception_handler	/* 3  : TLB exception (store)	*/
	.word	r4k_exception_handler	/* 4  : Address error (load)	*/
	.word	r4k_exception_handler	/* 5  : Address error (store)	*/
	.word	r4k_exception_handler	/* 6  : Bus Error (instr) 	*/
	.word	r4k_exception_handler	/* 7  : Bus Error (data)	*/
	.word	r4k_exception_handler	/* 8  : System call		*/
	.word	r4k_exception_handler	/* 9  : Breakpoint		*/
	.word	r4k_exception_handler	/* 10 : Illegal op		*/
	.word	r4k_exception_handler	/* 11 : Coprocessor unusable	*/
	.word	r4k_exception_handler	/* 12 : Arithmetic overflow	*/
	.word	r4k_exception_handler	/* 13 : Trap			*/
	.word	r4k_exception_handler	/* 14 : Reserved		*/
	.word	r4k_exception_handler	/* 15 : Floating FPU exception	*/
	.word	r4k_exception_handler	/* 16 : Reserved		*/
	.word	r4k_exception_handler	/* 17 : Reserved		*/
	.word	r4k_exception_handler	/* 18 : Reserved		*/
	.word	r4k_exception_handler	/* 19 : Reserved 		*/
	.word	r4k_exception_handler	/* 20 : Reserved		*/
	.word	r4k_exception_handler	/* 21 : Reserved		*/
	.word	r4k_exception_handler	/* 22 : Reserved		*/
	.word	r4k_exception_handler	/* 23 : Reserved		*/
	.word	r4k_exception_handler	/* 24 : Reserved		*/
	.word	r4k_exception_handler	/* 25 : Reserved		*/
	.word	r4k_exception_handler	/* 26 : Reserved		*/
	.word	r4k_exception_handler	/* 27 : Reserved		*/
	.word	r4k_exception_handler	/* 28 : Reserved		*/
	.word	r4k_exception_handler	/* 29 : Reserved		*/
	.word	r4k_exception_handler	/* 30 : Reserved		*/
	.word	r4k_exception_handler	/* 31 : Reserved		*/
	.word	r4k_cache_handler	/* 32 : Cache Error		*/
	.word	r4k_tlb_handler		/* 33 : TLB exception		*/
	.word	r4k_xtlb_handler	/* 34 : XTLB exception		*/

	.set 	reorder
	.set	at
ENDFRAME(exception_code_start)

/*
 * Marker of the exception vector code
 */
	.globl	exception_code_end
	.text
	.align 2

exception_code_end:
	.word	0

/*
 * r4k_cache_handler:
 *
 * Handles cache error exceptions
 *
 * Entry:  k0/k1 is saved in FPU_R1/FPU_R2 (maybe)
 */

FRAME(r4k_cache_handler,sp,0,ra)
	.set	noreorder
	.set	noat
	
	/*
	 * Call signal dispatcher with cause code in K0
	 */
	j		r4k_sig_dispatch		# call signal dispatcher
	 li		k0,(FLTCACHERR << 2)	# indicate cache error 

	.set	reorder
	.set	at
ENDFRAME(r4k_cache_handler)

/*
 * r4k_tlb_handler:
 *
 * Handles TLB miss exceptions
 *
 * Entry:  k0/k1 is saved in FPU_R1/FPU_R2 (maybe)
 */

FRAME(r4k_tlb_handler,sp,0,ra)
	.set	noreorder
	.set	noat
	
	j		r4k_sig_dispatch		# call signal dispatcher
	 li		k0,(FLTUTLBREFILL << 2)	# indicate 32-bit TLB refill

	.set	reorder
	.set	at
ENDFRAME(r4k_tlb_handler)

/*
 * r4k_xtlb_handler:
 *
 * Handles xtlb miss exceptions
 *
 * Entry:  k0/k1 is saved in FPU_R1/FPU_R2 (maybe)
 */

FRAME(r4k_xtlb_handler,sp,0,ra)
	.set	noreorder
	.set	noat

	j		r4k_sig_dispatch		# call signal dispatcher
	 li		k0,(FLTXTLBREFILL << 2)	# indicate 64-bit TLB refill

	.set	reorder
	.set	at
ENDFRAME(r4k_xtlb_handler)

/*
 * r4k_exception_handler:
 *
 * Handles all other exceptions
 *
 * Entry:  k0/k1 is saved in FPU_R1/FPU_R2 (maybe)
 */

FRAME(r4k_exception_handler,sp,0,ra)
	.set	noreorder
	.set	noat

	mfc0	k0,CP0_CAUSE			# re-read cause register
	
	j		r4k_sig_dispatch		# call signal dispatcher
	 andi	k0,k0,MIPS_CAUSE_MASK	# code = cause[0..5] (bd slot)

	.set	reorder
	.set	at
ENDFRAME(r4k_exception_handler)

/*
 * r4k_sig_dispatch:
 *
 * Handle strange events that require a signal handler to be
 * dispatched.
 *
 */

FRAME(r4k_sig_dispatch,sp,0,ra)
	.set	noreorder
	.set	noat

	/*
	 * Save complete context
	 */
	lw		k1,_stack_top
	addiu	k1,k1,-(SIZEOF_REG + 7)
	ori		k1,k1,7
	xori	k1,k1,7
	
	SAVE_ONE_REG_EXPANDED(AT,MYAT,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(zero,ZERO,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(v0,V0,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(v1,V1,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(a0,A0,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(a1,A1,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(a2,A2,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(a3,A3,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(t0,T0,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(t1,T1,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(t2,T2,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(t3,T3,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(t4,T4,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(t5,T5,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(t6,T6,0,k1,AT);	

	SAVE_KxREGS
	
	MV_INSTR(mfc0) t0,CP0_EPC		# get EPC reg
	mfc0	t1,CP0_SREG				# get Status reg
	mflo	t2					# get LO
	mfhi	t3					# get HI
//	MV_INSTR(mfc0) t4,CP0_ERR_EPC		# get Error EPC reg
	mfc0	t5,CP0_CAUSE				# get Cause register
	MV_INSTR(mfc0)	t6,CP0_BADVADDR				# get Bad Vaddr

	SAVE_ONE_REG_EXPANDED(t0,EPC,0,k1,AT);	
	sw	t1,MIPS_AREG(MIPS_REG_SREG)+(MIPS_REGS_LOW_WORD*4)(k1) # save Status reg
	SAVE_ONE_REG_EXPANDED(t2,LO,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(t3,HI,0,k1,AT);	
//	SAVE_ONE_REG_EXPANDED(t4,ERR_EPC,0,k1,AT);	
	sw	t5,MIPS_AREG(MIPS_REG_CAUSE)(k1)	# save Cause
	SAVE_ONE_REG_EXPANDED(t6,BADVADDR,0,k1,AT);	

	SAVE_ONE_REG_EXPANDED(t7,T7,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(t8,T8,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(t9,T9,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(ra,RA,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(s0,S0,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(s1,S1,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(s2,S2,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(s3,S3,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(s4,S4,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(s5,S5,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(s6,S6,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(s7,S7,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(s8,S8,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(gp,GP,0,k1,AT);	
	SAVE_ONE_REG_EXPANDED(sp,SP,0,k1,AT);	
	
	move	sp,k1

	/*
	 * Setup the status register. We need to ensure that
	 * both EXL and ERL are de-asserted and IE is off.
	 * Hence we can take recursive exceptions but do not take any
	 * interrupts. t1 has a copy of the status register.
	 * We install Error Level interrupts that allows the 
	 * timer interrupt to be serviced so the watchdog does not bark.
	 *
	 * Note: The code used to de-assert all three bits with one
	 * write to the sreg. This caused the ORION to turn off CP1,
	 * and take a cache error exception (while running uncached
	 * with parity exceptions disabled!). Similar weirdness can probably
	 * occur on the R4400. So three writes to the sreg are used.
	 */
	addiu	t2,zero,~MIPS_SREG_IE		# IE mask
	and		t1,t2,t1			# de-assert IE
	mtc0	t1,CP0_SREG			# write sreg
	nop					# let sreg settle

#if !defined(VARIANT_r3k)
	addiu	t2,zero,~MIPS_SREG_ERL		# ERL mask
	and		t1,t2,t1			# de-assert ERL
	mtc0	t1,CP0_SREG			# write sreg
	nop					# let sreg settle

	addiu	t2,zero,~MIPS_SREG_EXL		# EXL mask
	and		t1,t2,t1			# de-assert EXL
	mtc0	t1,CP0_SREG			# load new status register
	nop					# let sreg settle
#endif

	/*
	 * Setup parameters to the signal handler
	 */
	move	a0,k0				# setup code
	move	a1,k1				# setup context pointer
	
	la		gp,_gp

	/*
	 * Call the signal handler
	 */
	jal		mips_signal_handler		# call signal handler
	 nop					# (bd slot)

r4k_sig_dispatch_return:
	/*
	 * If the handler resumes, or the user types cont at
	 * the monitor prompt, we will come back here.
	 * We restore the stacked status register which guarantees
	 * that EXL/ERL will be asserted through critical section
	 * and the state of the IE bit will be restored
	 *
	 * Note: The R4400 processor does not like s/w to change
	 * the state of EXL with IE enabled, so disable it during change.
	 */
	move	k0,sp
	RESTORE_ONE_REG(s0,S0,0,k0)	
	RESTORE_ONE_REG(s1,S1,0,k0)	
	RESTORE_ONE_REG(s2,S2,0,k0)	
	RESTORE_ONE_REG(s3,S3,0,k0)	
	RESTORE_ONE_REG(s4,S4,0,k0)	
	RESTORE_ONE_REG(s5,S5,0,k0)	
	RESTORE_ONE_REG(s6,S6,0,k0)	
	RESTORE_ONE_REG(s7,S7,0,k0)	
	RESTORE_ONE_REG(s8,S8,0,k0)	
	
	RESTORE_ONE_REG(gp,GP,0,k0)	
	
	RESTORE_ONE_REG(v0,V0,0,k0)	
	RESTORE_ONE_REG(v1,V1,0,k0)	

	RESTORE_ONE_REG(a0,A0,0,k0)	
	RESTORE_ONE_REG(a1,A1,0,k0)	
	RESTORE_ONE_REG(a2,A2,0,k0)	
	RESTORE_ONE_REG(a3,A3,0,k0)	

	lw	t5,MIPS_AREG(MIPS_REG_SREG)+(MIPS_REGS_LOW_WORD*4)(k0)		# load Status reg
	RESTORE_ONE_REG(t0,T0,0,k0)	
	RESTORE_ONE_REG(t1,T1,0,k0)	
	RESTORE_ONE_REG(t2,T2,0,k0)	
	RESTORE_ONE_REG(t3,T3,0,k0)	

	mfc0	t6,CP0_SREG				# read sreg
	li		t4,~MIPS_SREG_IE		# get IE mask

	and		t6,t6,t4				# disable IE bit
	mtc0	t6,CP0_SREG				# re-write sreg (EXL=0,IE=0)

	and		t7,t5,t4				# disable IE in sreg copy
	mtc0	t7,CP0_SREG				# re-write sreg (EXL=1,IE=0)
	nop								# let sreg settle

	mtc0	t5,CP0_SREG				# restore sreg (EXL=1)
	nop								# let sreg settle

	RESTORE_ONE_REG(k1,EPC,0,k0)	
/* Can't use K1 after this point */
	RESTORE_ONE_REG(t6,LO,0,k0)	
	RESTORE_ONE_REG(t7,HI,0,k0)	
//	RESTORE_ONE_REG(t8,ERR_EPC,0,k0)	

	mtlo	t6					# restore LO
	mthi	t7					# restore HI
//	MV_INSTR(mtc0)	t8,CP0_ERR_EPC				# restore Error EPC reg

	RESTORE_ONE_REG(t4,T4,0,k0)	
	RESTORE_ONE_REG(t5,T5,0,k0)	
	RESTORE_ONE_REG(t6,T6,0,k0)	
	RESTORE_ONE_REG(t7,T7,0,k0)	
	RESTORE_ONE_REG(t8,T8,0,k0)	
	RESTORE_ONE_REG(t9,T9,0,k0)	
	
	RESTORE_ONE_REG(ra,RA,0,k0)	

	.set noat
	RESTORE_ONE_REG(AT,MYAT,0,k0)	
	RESTORE_ONE_REG(sp,SP,0,k0)	
	
	/*
	 * dismiss the exception/interrupt
	 */
#if defined(VARIANT_r3k)
	j		k1
	 rfe
#else
	MV_INSTR(mtc0)	k1,CP0_EPC				# restore EPC reg
	nop
	nop
	nop
	nop

	.set mips3
	
	eret						# return
	 nop						# not executed
	.set MIPSARCH
#endif
	 
	 

	.set	reorder
	.set	at
ENDFRAME(r4k_sig_dispatch)

/*
 * ulong_t outside_fault_entry (struct kdebug *entry, ulong_t sigcode, CPU_REGISTERS *ctx)
 *
 * Called by the kernel via syspage kdebug_entry to
 * give GDB a chance to handle the exception. Also
 * called by GDB's own exception handlers when kernel
 * hasn't take over the exceptions yet.
 */

FRAME(outside_fault_entry,sp,0,ra)
	.set	noreorder
	.set	noat

	/*
	 * First turn off the interrupts
	 */
	mfc0    t0,CP0_SREG             # read status register
	 nop
	addiu   t1,zero,~MIPS_SREG_IE   # get IE mask
	and     t0,t0,t1                # mask off IE
	mtc0    t0,CP0_SREG             # write sreg
	 nop                            # let it settle

	/*
	 * Save caller's gp and make sure
	 * gp point to my small data area
	 */
	move	t2, gp
	la      gp,_gp

	/*
	 * Switch to GDB stack
	 */
	move	t3, sp
	lw		sp,_stack_top

	/*
	 * Create a stack frame on GDB stack
	 */
	addiu	sp,sp,-32
	sw		ra, 24(sp)
	sw		t2, 20(sp)
	sw		t3, 16(sp)

	/*
	 * Call C debugger entry
	 */
	jal	handle_exception
	 nop

	lw		ra, 24(sp)
	lw		gp, 20(sp)
	lw		sp, 16(sp)

	jr		ra
	 nop

	.set	reorder
	.set	at
ENDFRAME(outside_fault_entry)

/*
 * int outside_watch_entry(struct kdebug *entry, paddr_t start)
 *  Stop at the given address
 */
FRAME(outside_watch_entry,sp,0,ra)
	.set	noreorder
	/*
	 * Create a stack frame, save old gp and make it point
	 * to kernel debugger's small data area.
	 */
	addiu	sp,sp,-32
	sw		ra, 24(sp)
	sw		gp, 20(sp)

	la      gp,_gp

	/*
	 * Call C debugger entry
	 */
	jal	watch_entry
	nop

	lw		ra, 24(sp)
	lw		gp, 20(sp)

	addiu	sp,sp,+32
	jr		ra
	 nop
ENDFRAME(outside_watch_entry)

/*
 * void outside_msg_entry(const char *msg, unsigned len)
 *  Display the given message
 */
FRAME(outside_msg_entry,sp,0,ra)
	.set	noreorder
	/*
	 * Create a stack frame, save old gp and make it point
	 * to kernel debugger's small data area.
	 */
	addiu	sp,sp,-32
	sw		ra, 24(sp)
	sw		gp, 20(sp)

	la      gp,_gp

	/*
	 * Call C debugger entry
	 */
	jal		msg_entry
	 nop

	lw		ra, 24(sp)
	lw		gp, 20(sp)

	addiu	sp,sp,+32
	jr		ra
	 nop
ENDFRAME(outside_msg_entry)

/*
 * void outside_update_plist(struct kdebug_entry *modified)
 *  A process has been added/deleted to the kernel debugger's list. The
 *  parm is the entry that's been added/deleted.
 */
FRAME(outside_update_plist,sp,0,ra)
	.set	noreorder
	jr		ra
	 nop
ENDFRAME(outside_update_plist)

/*
 * void outside_timer_reload(struct syspage_entry *, struct qtime_entry *)
 *  Hook to check for async stop request
 */
FRAME(outside_timer_reload,sp,0,ra)
	.set	noreorder
	/*
	 * Create a stack frame, save old gp and make it point
	 * to kernel debugger's small data area.
	 */
	addiu	sp,sp,-32
	sw		ra, 24(sp)
	sw		gp, 20(sp)

	la      gp,_gp

	/*
	 * Call C debugger entry
	 */
	jal		kdebug_timer_reload
	 nop

	lw		ra, 24(sp)
	lw		gp, 20(sp)

	addiu	sp,sp,+32
	jr		ra
	 nop
ENDFRAME(outside_timer_reload)
