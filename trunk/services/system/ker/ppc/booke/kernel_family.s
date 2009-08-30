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

#
# This file contains all the entry points to the kernel that are specific
# to book E series CPU's.
#
	
	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"
	
	.global __exc_machine_check_440gp
	.global __exc_machine_check_e500
	.global __exc_alignment_booke
	.global __exc_program_booke
	.global __exc_debug_booke
	.global __exc_data_access_booke
	.global __exc_instr_access_booke

.ifdef PPC_CPUOP_ENABLED
.cpu booke32
.endif
	
.section .text_kernel, "ax"

EXC_COPY_CODE_START	ctx_save_usprg0
	mfspr	%r4,PPCBKE_SPR_USPRG0
	stw		%r4,REG_USPRG0(%r30)
EXC_COPY_CODE_END

EXC_COPY_CODE_START	ctx_restore_usprg0
	lwz		%r20,REG_USPRG0(%r30)
	mtspr	PPCBKE_SPR_USPRG0,%r20
EXC_COPY_CODE_END

pgm_fault_codes_booke:
0:	.long 0
1:	.long 0
2:	.long 0
3:	.long 0
4:	.long SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)
5:	.long SIGILL + (ILL_ILLOPC*256) + (FLTPRIV*65536)
6:	.long SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)
7:	.long SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)
8:	.long 0
9:	.long 0
10:	.long 0
11:	.long 0
12:	.long SIGILL + (ILL_COPROC*256) + (FLTILL*65536)
13:	.long SIGILL + (ILL_COPROC*256) + (FLTILL*65536)
14:	.long 0
15:	.long 0
16:	.long 0
17:	.long 0
18:	.long 0
19:	.long 0
20:	.long 0
21:	.long 0
22:	.long 0
23:	.long 0
24:	.long 0
25:	.long 0
26:	.long 0
27:	.long 0
28:	.long 0
29:	.long 0
30:	.long 0
31:	.long 0

#
# IBM used the same SPR values for MCSRR0/1 & instruction encoding for
# "rfmci" that Motorola did. And there was rejoicing. In the future,
# think about combining the __exc_entry_machine_check_440gx & _e500 routines.
#
EXC_COPY_CODE_START	__exc_entry_machine_check_440gx
	mtsprg	2,%r31
	mtspr	PPCBKE_SPR_SPRG7,%r30
	mfcr	%r30
	mfspr	%r31,PPC440GX_SPR_MCSRR1
	bittst	%r31,%r31,PPC_MSR_IS
	bne		1f
	#
	# Took a machine check in exception handler code.
	# Just keep going.
	# 
	li		%r31,0
	mtspr	PPC440GX_SPR_MCSRR1,%r31
	mtcr	%r30
	mfspr	%r30,PPCBKE_SPR_SPRG7
	mfsprg	%r31,2
	# Switch to "rfmci" instruction when assembler supports it.
	.long	(19 << 26) + (38 << 1)
1:
	mtcr	%r30
	mfspr	%r30,PPCBKE_SPR_SPRG7
	mfsprg	%r31,2
	ENTERKERNEL EK_EXC, PPC440GX_SPR_MCSRR0, PPC440GX_SPR_MCSRR1, 2
EXC_COPY_CODE_END

EXC_COPY_CODE_START	__exc_entry_machine_check_e500
	mtsprg	2,%r31
	mtspr	PPCBKE_SPR_SPRG7,%r30
	mfcr	%r30
	mfspr	%r31,PPCE500_SPR_MCSRR1
	bittst	%r31,%r31,PPC_MSR_IS
	bne		1f
	#
	# Took a machine check in exception handler code.
	# Just keep going.
	# 
	li		%r31,0
	mtspr	PPCE500_SPR_MCSRR1,%r31
	mtcr	%r30
	mfspr	%r30,PPCBKE_SPR_SPRG7
	mfsprg	%r31,2
	# Switch to "rfmci" instruction when assembler supports it.
	.long	0b01001100000000000000000001001100
1:
	mtcr	%r30
	mfspr	%r30,PPCBKE_SPR_SPRG7
	mfsprg	%r31,2
	ENTERKERNEL EK_EXC, PPCE500_SPR_MCSRR0, PPCE500_SPR_MCSRR1, 2
EXC_COPY_CODE_END

EXC_COPY_CODE_START	__exc_entry_machine_check_booke
	mtsprg	1,%r31
	li		%r31,0
	mtmsr	%r31
	mtsprg	2,%r30
	mfcr	%r30
	mfspr	%r31,PPCBKE_SPR_CSRR1
	bittst	%r31,%r31,PPC_MSR_IS
	bne		1f
	#
	# Took a machine check in exception handler code.
	# Just keep going.
	# 
	li		%r31,0
	mtspr	PPCBKE_SPR_CSRR1,%r31
	mtcr	%r30
	mfsprg	%r30,2
	mfsprg	%r31,1
	rfci
1:
	mtcr	%r30
	mfsprg	%r30,2
	mfsprg	%r31,1
	ENTERKERNEL EK_EXC, PPCBKE_SPR_CSRR0, PPCBKE_SPR_CSRR1, 1
EXC_COPY_CODE_END
	
__exc_machine_check_440gp:
	mfspr	%r4,PPCBKE_SPR_ESR	# see if instr or data machine chk
	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTBUSERR*65536)
	bittst	%r0,%r4,PPC440_ESR_MCI
	mfspr	%r4,PPCBKE_SPR_CSRR0
	bne		__exc # instruction machine check
	
	# data machine check
	#
	# Can't tell what instruction caused the problem, can't tell
	# what address caused the problem. Yuck.
	#
	li		%r4,0
	b		__exc
	
__exc_machine_check_e500:
	##NYI: E500: Should look at MCSR and do appropriate action(s)
	mfspr	%r4,PPCBKE_SPR_ESR	# see if instr or data machine chk
	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTBUSERR*65536)
	mfspr	%r4,PPCE500_SPR_MCAR
	b		__exc
	
__exc_program_booke:
	mfspr	%r4,PPCBKE_SPR_ESR	# get reason for instruction machine chk/program
	cntlzw	%r4,%r4
	slwi	%r6,%r4,2
	lis		%r5,pgm_fault_codes_booke@ha	
	la		%r5,pgm_fault_codes_booke@l(%r5)
	lwzx	%r3,%r6,%r5
	cmplwi	%r4,4
	beq-	__exc_emulation
	cmplwi	%r4,6
	beq-	__exc_trap
	cmplwi	%r4,7
	beq-	__exc_fpu
	cmplwi	%r3,0
	bne+	__exc
	b		__hardcrash

__exc_instr_access_booke:
	#NYI: If we come here via TLB error, it's a protection violation
    #NYI: and we should set the VM_FAULT_WIMG_ERR bit.
    #NYI: However, the TLB miss handlers in vm*.s will also transfer
    #NYI: here if they don't find a valid PTE and we shouldn't set
    #NYI: VM_FAULT_WIMG_ERR in that case. We should recode
    #NYI: the TLB miss handlers to go to another routine so we
    #NYI: can distinguish between the two cases....
	mfspr	%r5,PPCBKE_SPR_PID	# need for virtual_fault() handling
	lwz		%r4,REG_IAR(%r30)
	bitset	%r5,%r5,VM_FAULT_INSTR	# say it was an instruction access
	b 		__exc_access

__exc_data_access_booke:
	#NYI: If we come here via TLB error, it's a protection violation
    #NYI: and we should set the VM_FAULT_WIMG_ERR bit.
    #NYI: However, the TLB miss handlers in vm*.s will also transfer
    #NYI: here if they don't find a valid PTE and we shouldn't set
    #NYI: VM_FAULT_WIMG_ERR in that case. We should recode
    #NYI: the TLB miss handlers to go to another routine so we
    #NYI: can distinguish between the two cases....
	mfspr	%r5,PPCBKE_SPR_PID	# need for virtual_fault() handling
	mfspr	%r4,PPCBKE_SPR_DEAR	# get address responsible
	mfspr	%r0,PPCBKE_SPR_ESR
	bittst	%r0,%r0,PPCBKE_ESR_ST
	beq		__exc_access
	bitset	%r5,%r5,VM_FAULT_WRITE		# indicate a store instruction
	b 		__exc_access
	
EXC_COPY_CODE_START __exc_entry_debug
	mtsprg	1,%r31
	li		%r31,0
	mtmsr	%r31
	mtsprg	2,%r30
	mfcr	%r30
	mfspr	%r31,PPCBKE_SPR_CSRR1
	bittst	%r31,%r31,PPC_MSR_IS
	bne		1f
	loadwz	%r31,alives
	cmplwi	%r31,0
	beq		1f
	#
	# Took a single step at the first instruction of an exception (interrupt)
	# handler. Just keep going.
	# 
	li		%r31,0
	mtspr	PPCBKE_SPR_CSRR1,%r31
	li		%r31,-1
	mtspr	PPCBKE_SPR_DBSR,%r31
	mtcr	%r30
	mfsprg	%r30,2
	mfsprg	%r31,1
	rfci
1:
	mtcr	%r30
	mfsprg	%r30,2
	mfsprg	%r31,1
	
	ENTERKERNEL EK_EXC, PPCBKE_SPR_CSRR0, PPCBKE_SPR_CSRR1, 1
EXC_COPY_CODE_END
	
__exc_debug_booke:
	mfspr	%r3,PPCBKE_SPR_DBSR
	stw		%r3,CPU_DBSR_OFF(%r31)
	li		%r3,-1
	mtspr	PPCBKE_SPR_DBSR,%r3
	#
	# Have to spiffy this code up if we start using the debug control
	# register for things other than single stepping
	#
	loadi	%r3,SIGTRAP + (TRAP_TRACE*256) + (FLTTRACE*65536)	
	b 		__exc_debug

__exc_alignment_booke:
	mfspr	%r4,PPCBKE_SPR_DEAR
	b		__exc_alignment


	.global __halt
routine_start halt, 1
	lwz 	%r9,actives@sdarel(%r13)
	lwz 	%r0,REG_OFF+REG_MSR(%r9)
	bitset	%r0,%r0,PPC_MSR_WE
	stw 	%r0,REG_OFF+REG_MSR(%r9)
__halt:
	# dummy instruction
	ori		%r0,%r0,%r0
	blr
routine_end halt

