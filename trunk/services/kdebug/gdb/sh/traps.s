#
# traps.s - GDB exception handling code
#

	.include "asmoff.def"
	.include "sh/util.ah"

	.extern _stack_top
	.extern _stack_base
	.extern watch_entry
	.extern msg_entry
	.extern handle_exception
	.extern kerdebug_reg
	
#	bank regs (0 - 7)
#	temp reg (r0,r1,r2), shared var and const	
#		context_off,		r3_bank
#		actives_addr,		r4_bank
#		ker_stack_addr,		r5_bank
#		atomic_flag,		r6_bank
#		inkernel,			r7_bank	
#	common regs (8 - 15)	
#		regsave,			r11
#		active,				r12
#		org_inkern,			r13
#		sp,					r15
	
#
# On exit from this macro:
#			R15	- debugger stack pointer
#			R11 - register save area
#	The register bank is switched to bank0
#
# NOTE: Don't damage r0_bank - it might be passing the syspage pointer to
# the kernel...
#
.macro ENTERDBG
	mov.l	kerdebug_reg_addr,r1
	add		#SIZEOF_REG,r1
# save context
# inverse order of the context	sh_cpu_registers 	
	sts.l	pr,@-r1
	sts.l	macl,@-r1
	sts.l	mach,@-r1
	stc.l	gbr,@-r1
	stc.l	spc,@-r1
	stc.l	ssr,@-r1
	mov.l	r15,@-r1	
	mov.l	r14,@-r1	
	mov.l	r13,@-r1	
	mov.l	r12,@-r1	
	mov.l	r11,@-r1	
	mov.l	r10,@-r1	
	mov.l	r9,@-r1	
	mov.l	r8,@-r1	
	stc.l	r7_bank,@-r1
	stc.l	r6_bank,@-r1
	stc.l	r5_bank,@-r1
	stc.l	r4_bank,@-r1
	stc.l	r3_bank,@-r1
	stc.l	r2_bank,@-r1
	stc.l	r1_bank,@-r1
	stc.l	r0_bank,@-r1
# load stack, and register save area
	mov		r1,r11
	mov.l	stack_top_addr,r15
# switch to reg bank 0	
	mov.l	sh_sr_rb,r1
	stc		sr,r2
	and		r1,r2
	mov		#0x78,r3
	shll	r3
	or		r3,r2
	ldc		r2,sr
.endm
	
#interrupt is off, reg bank0
.macro EXITDBG
# restore context
#  r11 point to the context save area
	mov.l	kerdebug_reg_addr,r11
	mov.l	@r11+,r0
	mov.l	@r11+,r1	
	mov.l	@r11+,r2
	mov.l	@r11+,r3
	mov.l	@r11+,r4
	mov.l	@r11+,r5
	mov.l	@r11+,r6
	mov.l	@r11+,r7
	mov.l	@r11+,r8	
	mov.l	@r11+,r9	
	mov.l	@r11+,r10	
	ldc.l	@r11+,r1_bank
	mov.l	@r11+,r12	
	mov.l	@r11+,r13	
	mov.l	@r11+,r14	
	mov.l	@r11+,r15
	ldc.l	@r11+,ssr
	ldc.l	@r11+,spc
	ldc.l	@r11+,gbr
	lds.l	@r11+,mach
	lds.l	@r11+,macl
	lds.l	@r11+,pr
	stc		r1_bank,r11
	rte
	nop
.endm

#
# exceptions.
#

# entry for general exceptions
.global	exc_general_start
exc_general_start:
.align 2
/*
# test
mov.l test_0,r1
mov	 #44,r0
mov.b r0,@r1
*/
#syspageptr might be in r0_bank1
mov	r0,r2

	mov.l	exc_general_expevt,r1
	mov.l	@r1,r0
	shlr2	r0
	shlr	r0
	and		#0x3c,r0
	mov		r0,r1
	mova	exc_general_exptable,r0
	mov.l	@(r0,r1),r1

mov	r2,r0
	jmp		@r1
	nop
	.align 2
test_0:
	.long	0xffe8000c	
exc_general_expevt:
	.long	SH_MMR_CCN_EXPEVT		
exc_general_exptable:
	.long	exc_unexpected	/* 0x800 */
	.long	exc_unexpected	/* 0x820 */
	.long	exc_unexpected	 	
	.long	exc_unexpected	 	
	.long	exc_unexpected	/* 0x080 */
	.long	exc_unexpected	/* 0x0a0 */
	.long	exc_unexpected	/* 0x0c0 */	
	.long	exc_unexpected	/* 0x0e0 */
	.long	exc_unexpected	/* 0x100 */
	.long	exc_unexpected	/* 0x120 */
	.long	exc_unexpected	 	
	.long	exc_unexpected	 	/* 0x160 */
	.long	exc_ill_instr		/* 0x180 */
	.long	exc_ill_instr_slot/* 0x1a0 */	
	.long	exc_unexpected	/* 0x1e0 */
	.long	exc_unexpected		


.global	exc_general_end
# exc code in upper 16 bits, int code in lower 16 bits
exc_general_end:
	 .type exc_general_start,@function
	 .type exc_general_end,@function

.global	exc_unexpected
exc_unexpected:
	ENTERDBG	
	mov.l	exc_unexpected_1,r1
	mov.l	@r1,r0
	mov.l	exc_unexpected_2,r1
	shll16	r0
	mov.l	@r1,r4
	bsr		exc
	or		r0,r4

#general ill instruction
exc_ill_instr:
	ENTERDBG	
	bsr		exc_trap
	nop

#slot ill instruction
exc_ill_instr_slot:
	ENTERDBG	
	bsr		exc_trap
	nop

exc_trap:
	mov.l	exc_trap_01,r4
# falling through
exc:
	mov.l	exc_1,r5
	mov.l	handle_exception_addr,r0
	or		r4,r5
	mov		r11,r6
	jsr		@r0
	mov		#0,r4
	EXITDBG 

	
#/*
# * ulong_t outside_fault_entry (struct kdebug *entry, ulong_t sigcode, CPU_REGISTERS *ctx)
# *
# * Called by the kernel via syspage kdebug_entry to
# * give GDB a chance to handle the exception. Also
# * called by GDB's own exception handlers when kernel
# * hasn't take over the exceptions yet.
# */
	.global	outside_fault_entry
outside_fault_entry:
	mov.l	r13,@-r15
	mov.l	r14,@-r15
	sts.l	pr,@-r15
#	mov.l	outside_fault_entry_1,r0
	stc		sr,r13
	mov		r15,r14
#	or		r13,r0	
	mov		r13,r0
	or		#0xf0,r0
	ldc		r0,sr
	mov.l	handle_exception_addr,r0
	mov.l	stack_top_addr,r15
	jsr		@r0
	nop
	mov		r14,r15
	ldc		r13,sr
	lds.l	@r15+,pr
	mov.l	@r15+,r14
	mov.l	@r15+,r13
	rts
	nop

#/*
# * int outside_watch_entry(struct kdebug *kdebug, paddr_t start)
# *  Stop at the given address
# */
	.global outside_watch_entry
outside_watch_entry:
	mov.l	r14,@-r15
	sts.l	pr,@-r15
	mov		r15,r14
	mov.l	outside_watch_entry_0,r0
	mov.l	stack_top_addr,r15
	jsr		@r0
	nop
	mov		r14,r15
	lds.l	@r15+,pr
	mov.l	@r15+,r14
	rts
	nop
	
#/*
# * int outside_msg_entry(const char *msg, unsigned len)
# *  Display the given message
# */
	.global outside_msg_entry
outside_msg_entry:
	mov.l	r14,@-r15
	sts.l	pr,@-r15
	mov		r15,r14
	mov.l	outside_msg_entry_0,r0
	mov.l	stack_top_addr,r15
	jsr		@r0
	nop
	mov		r14,r15
	lds.l	@r15+,pr
	mov.l	@r15+,r14
	rts
	nop
	
#/*
# * int outside_timer_reload(struct syspage_entry *, struct qtime_entry *)
# *  Check for an async stop. Run on kernel stack.
# */
	.global outside_timer_reload
outside_timer_reload:
	sts.l	pr,@-r15
	mov.l	outside_timer_reload_0,r0
	jsr		@r0
	nop
	lds.l	@r15+,pr
	rts
	nop

	.align 2	
kerdebug_reg_addr:
	.long	kerdebug_reg
stack_top_addr:
	.long	_stack_top
sh_sr_rb:
	.long	~(SH_SR_RB | SH_SR_BL)
handle_exception_addr:
	.long	handle_exception
	.align 2
exc_unexpected_1:
	.long	SH_MMR_CCN_EXPEVT		
exc_unexpected_2:
	.long	SH_MMR_CCN_INTEVT		
exc_trap_01:
	.long	SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)
exc_1:
	.long	SIGCODE_FATAL
#outside_fault_entry_1:
#	.long	SH_SR_BL	
outside_watch_entry_0:
	.long	watch_entry
outside_msg_entry_0:
	.long	msg_entry
outside_timer_reload_0:
	.long	kdebug_timer_reload
	
	
#/*
# * void outside_update_plist(struct kdebug_entry *modified)
# *  A process has been added/deleted to the kernel debugger's list. The
# *  parm is the entry that's been added/deleted.
# */
	.global outside_update_plist
outside_update_plist:
	rts
	nop

