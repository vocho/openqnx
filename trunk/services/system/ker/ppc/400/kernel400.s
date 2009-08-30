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
# to the 400 series.
#
	
	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"
	
	.global __exc_machine_check403
	.global __exc_machine_check405
	.global __exc_alignment400
	.global __exc_program400
	.global __exc_debug400
	.global __exc_data_access400
	.global __exc_instr_access400
	
.section .text_kernel, "ax"


.ifdef PPC_CPUOP_ENABLED
	.cpu 403
.endif


pgm_fault_codes400:
0:	.long SIGSEGV + (SEGV_MAPERR*256) + (FLTPRIV*65536)	
1:	.long SIGSEGV + (SEGV_MAPERR*256) + (FLTBOUNDS*65536)
2:	.long SIGBUS + (BUS_OBJERR*256) + (FLTBUSERR*65536)
3:	.long SIGBUS + (BUS_OBJERR*256) + (FLTBUSTIMOUT*65536)
4:	.long SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)
5:	.long SIGILL + (ILL_ILLOPC*256) + (FLTPRIV*65536)
6:	.long SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)
##NYI: need to fill in these for other 400 series processors
7:	.long 0
8:	.long 0
9:	.long 0
10:	.long 0
11:	.long 0
12:	.long 0
13:	.long 0
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

EXC_COPY_CODE_START __exc_entry_machine_check
	mtsprg	1,%r31
	mtsprg	2,%r30
	mfcr	%r30
	mfspr	%r31,SRR3
	bittst	%r31,%r31,PPC_MSR_IS
	bne		1f
	#
	# Took a Machine Check in exception handler code.
	# Just keep going.
	# 
	li		%r31,0
	mtspr	SRR3,%r31
	mtcr	%r30
	mfsprg	%r30,2
	mfsprg	%r31,1
	rfci
1:
	mtcr	%r30
	mfsprg	%r30,2
	mfsprg	%r31,1
	ENTERKERNEL EK_EXC, SRR2, SRR3, 1
EXC_COPY_CODE_END
	
	
__exc_machine_check405:
	mfspr	%r4,PPC400_SPR_ESR	# see if instr or data machine chk
	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTBUSERR*65536)
	bittst	%r0,%r4,PPC405_ESR_MCI
	beq		1f
	# instruction machine check
	mfspr	%r4,SRR2
	b		__exc
1:
	# data machine check
	#NYI: No doc on what to do in this case
	# 405GP implementation documentation says it's "implementation defined"
	# what happens. If it weren't so sad, it'd be funny :-(.
	li		%r4,0
	b		__exc

__exc_machine_check403:
	mfdcr	%r4,PPC403_DCR_BEAR		# get address responsible
	mfdcr	%r3,PPC403_DCR_BESR		# data or instruction?
	bittst	%r0,%r3,PPC403_BESR_DSES
	beq		ins_machine_check
	# handle a data fault
	li		%r5,0			# clear error condition
	mtdcr	PPC403_DCR_BESR,%r5
	dcbi	0,%r4			# clear the invalid data from the cache
	rlwinm	%r5,%r3,7,28,29	# extract ET field & prepare for indexing	
	lis		%r3,pgm_fault_codes400@ha	
	la		%r3,pgm_fault_codes400@l(%r3)
	lwzx	%r3,%r3,%r5		# get fault code
	b		__exc
	
__exc_program400:
ins_machine_check:
	mfspr	%r4,PPC400_SPR_ESR	# get reason for instruction machine chk/program
	cntlzw	%r4,%r4
	cmplwi	%r4,6
	beq-	__exc_trap
	loadi	%r3,SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)	
	
	cmplwi	%r4,4
	beq-	__exc_emulation
	slwi	%r4,%r4,2
	lis		%r5,pgm_fault_codes400@ha	
	la		%r5,pgm_fault_codes400@l(%r5)
	lwzx	%r3,%r4,%r5
	cmplwi	%r3,0
	bne+	__exc
	b		__hardcrash

EXC_COPY_CODE_START __common_exc_entry_405workaround
# Workaround for PPC405 bug (fixed in rev D)
.set DEAR_SAVE_OFF,0x3fc
mtsprg	0,%r31
mfspr	%r31,PPC400_SPR_DEAR
stw		%r31,DEAR_SAVE_OFF(0)
mfsprg	%r31,0
#end workaround 

	ENTERKERNEL EK_EXC, SRR0, SRR1
EXC_COPY_CODE_END


__exc_instr_access400:
	#NYI: If we come here directly, it's a protection violation
    #NYI: and we should set the VM_FAULT_WIMG_ERR bit.
    #NYI: However, the TLB miss handlers in vm400.s will also transfer
    #NYI: here if they don't find a valid PTE and we shouldn't set
    #NYI: VM_FAULT_WIMG_ERR in that case. We should recode
    #NYI: the TLB miss handlers to go to another routine so we
    #NYI: can distinguish between the two cases....
	lwz		%r4,REG_IAR(%r30)
	bitset	%r5,%r5,VM_FAULT_INSTR	# say it was an instruction access
	b 		__exc_access


__exc_data_access400:
	
# Workaround for PPC405 bug (fixed in rev D)
###	mfspr	%r4,PPC400_SPR_DEAR 	# get address responsible
lwz %r4,DEAR_SAVE_OFF(0)
#end workaround 

	#NYI: If we come here directly, it's a protection violation
    #NYI: and we should set the VM_FAULT_WIMG_ERR bit.
    #NYI: However, the TLB miss handlers in vm400.s will also transfer
    #NYI: here if they don't find a valid PTE and we shouldn't set
    #NYI: VM_FAULT_WIMG_ERR in that case. We should recode
    #NYI: the TLB miss handlers to go to another routine so we
    #NYI: can distinguish between the two cases....

	mfspr	%r0,PPC400_SPR_ESR
	bittst	%r0,%r0,PPC400_ESR_DST
	beq		__exc_access
	bitset	%r5,%r5,VM_FAULT_WRITE		# indicate a store instruction
	b 		__exc_access

__exc_alignment400:
	
# Workaround for PPC405 bug (fixed in rev D)
###	mfspr	%r4,PPC400_SPR_DEAR 	# get address responsible
lwz %r4,DEAR_SAVE_OFF(0)
#end workaround 
	b		__exc_alignment
	
EXC_COPY_CODE_START __exc_entry_debug
	mtsprg	1,%r31
	li		%r31,0
	mtmsr	%r31
	mtsprg	2,%r30
	mfcr	%r30
	mfspr	%r31,SRR3
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
	mtspr	SRR3,%r31
	li		%r31,-1
	mtspr	PPC400_SPR_DBSR,%r31
	mtcr	%r30
	mfsprg	%r30,2
	mfsprg	%r31,1
	rfci
1:
	mtcr	%r30
	mfsprg	%r30,2
	mfsprg	%r31,1
	
	ENTERKERNEL EK_EXC, SRR2, SRR3, 1
EXC_COPY_CODE_END
	
__exc_debug400:
	#
	# Have to spiffy this code up if we start using the debug control
	# register for things other than single stepping
	#
	lwz		%r3,REG_MSR(%r30)
	bitclr	%r3,%r3,PPC_MSR_DE
	stw		%r3,REG_MSR(%r30)
	li		%r31,-1
	mtspr	PPC400_SPR_DBSR,%r31
	loadi	%r3,SIGTRAP + (TRAP_TRACE*256) + (FLTTRACE*65536)	
	b 		__exc_debug


#	Can't turn on the wait enable bit because the silly chip leaves
#	it on in the saved image of the MSR when it takes an exception.
#	That means that it'll turn back on when we run this thread again,
#	meaning that we never get out of this code, meaning we'll never
#	run __ker_exit(), meaning we'll never process the intrevents_pending
#	queue, meaning the whole system just sits around doing nothing.
#	Grrr.
#	set_msr(get_msr() | PPC_MSR_WE);

	.global __halt
routine_start halt, 1
	lwz		%r9,actives@sdarel(%r13)
	lwz		%r0,REG_OFF+REG_MSR(%r9)
	bitset	%r0,%r0,PPC_MSR_WE
	stw 	%r0,REG_OFF+REG_MSR(%r9)
__halt:
	# dummy instruction
	nop
	blr
routine_end halt
