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
# to the 800 series.
#
	
	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"
	

	.global __exc_alignment800
	.global __exc_machine_check800
	.global __exc_program800
	.global __exc_trace800
	.global __exc_emulation800
	.global __exc_dbreak800
	.global __exc_ibreak800
	.global __exc_pbreak800
	.global __exc_devport800
	.global __exc_data_access800
	.global __exc_instr_access800

	.extern is_store
	
.section .text_kernel, "ax"
	
__exc_alignment800:
	mfdar	%r4
	b		__exc_alignment
	
__exc_machine_check800:
#	mfspr	%r3,SRR1	MMU safe
	lwz		%r3,REG_MSR(%r30)
	bittst	%r0,%r3,0x2		# is it recoverable?
	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTMACHCHK*65536)
	beq-	__hardcrash		# nope
	
	mfdar	%r4
	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTMACHCHK*65536)
	b 		__exc
	
__exc_program800:
	lwz		%r5,REG_MSR(%r30)
	bittst	%r0, %r5, 0x00020000 			# TRAPFLG 	
	bne		__exc_trap
	bittst	%r0, %r5, 0x00040000 			# privilege instruction 	
	bne		1f
	loadi	%r3,SIGILL + (ILL_PRVOPC*256) + (FLTPRIV*65536)	
	b		__exc
1:
	loadi	%r3,SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)	
	b		__exc
	
__exc_trace800:
	loadi	%r3,SIGTRAP + (TRAP_TRACE*256) + (FLTTRACE*65536)	
	b 		__exc

__exc_emulation800:
	loadi	%r3,SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)	
	b 	__exc_emulation 
	
__exc_dbreak800:
__exc_ibreak800:
__exc_pbreak800:
__exc_devport800:
	mfbar	%r4
	loadi	%r3,SIGCODE_KERNEL + SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)
	b 		__exc


__exc_instr_access800:
	#NYI: If we come here via TLB error, it's a protection violation
    #NYI: and we should set the VM_FAULT_WIMG_ERR bit.
    #NYI: However, the TLB miss handlers in vm800.s will also transfer
    #NYI: here if they don't find a valid PTE and we shouldn't set
    #NYI: VM_FAULT_WIMG_ERR in that case. We should recode
    #NYI: the TLB miss handlers to go to another routine so we
    #NYI: can distinguish between the two cases....
	lwz		%r4,REG_IAR(%r30)
	bitset	%r5,%r5,VM_FAULT_INSTR	# say it was an instruction access
	b 		__exc_access

	
__exc_data_access800:
	#NYI: If we come here via TLB error, it's a protection violation
    #NYI: and we should set the VM_FAULT_WIMG_ERR bit.
    #NYI: However, the TLB miss handlers in vm800.s will also transfer
    #NYI: here if they don't find a valid PTE and we shouldn't set
    #NYI: VM_FAULT_WIMG_ERR in that case. We should recode
    #NYI: the TLB miss handlers to go to another routine so we
    #NYI: can distinguish between the two cases....
	mfspr	%r4,PPC_SPR_DAR 	# get address responsible

	mfspr	%r0,PPC_SPR_DSISR
	bittst	%r3,%r0,PPC_DSISR_NOTRANS
	bne		2f
	bittst	%r3,%r0,PPC_DSISR_STORE
	beq		1f
3:	
	bitset	%r5,%r5,VM_FAULT_WRITE		# indicate a store instruction
1:
	b		__exc_access
2:  #need to check the instruction to see if it is a load or store
	lwz		%r3,REG_IAR(%r30)
	lwz		%r3,0(%r3)
	mr		%r27,%r5
	mr		%r26,%r4
	bl		is_store
	mr		%r5,%r27	
	mr		%r4,%r26	
	and.	%r3,%r3,%r3
	bne		3b
	b		1b	

	.global __halt
routine_start halt, 1
	
	# we could add PM here. For now, just spin on __halt, better than doing
	# constant hook_to_idle() (for latency)
__halt:
	b		__halt
	nop
	blr

routine_end halt
