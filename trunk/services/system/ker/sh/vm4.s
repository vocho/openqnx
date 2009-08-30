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
# SH4/SH4A TLB miss exception handlers.
#
# We are executing with bank1 registers (SR.BL=1, SR.RB=1):
#	r0_bank1 - available for use as temp register
#	r1_bank1 - available for use as temp register
#	r2_bank1 - available for use as temp register
#	r3_bank1 - available for use as temp register
#	r4_bank1 - &actives[RUNCPU]
#	r5_bank1 - ker_stack[RUNCPU]
#	r6_bank1 - RUNCPU << 2
#	r7_bank1 - &inkernel (SMP), inkernal value (non-SMP)
#
# WARNING: it is assumed that any code that can cause a TLB miss exception
#          is executing with bank0 registers.
#
# The appropriate handler code is copied to VBR+SH_EXC_TLBMISS by init_vm().
# This code:
# - must be position independent
# - must not exceed 0x200 bytes
#
	
	.include "asmoff.def"
	.include "sh/util.ah"
	
	.extern	__exc_general_start

	.set	debug_base,0xa8000000
	
	.global	__exc_tlb_start
	.global __exc_tlb_end
	.global	__exc_tlb_start_a
	.global __exc_tlb_end_a

#
# TLB miss for SH4 (non-SH4A)
#
# should not exceed 0x200 bytes, as it will be copied to exc area
#
	.align 2
__exc_tlb_start:
	mov.l	__exc_tlb_base,r1
	mov		#-22,r0
	mov.l	@(SH_MMR_CCN_TEA_OFF,r1),r2		!fault address	
	mov		r2,r3
	shld	r0,r2
	mov.l	@(SH_MMR_CCN_TTB_OFF,r1),r0		! L1 page table base
	shll2	r2
	mov.l	@(r0,r2),r0
	mov.l	__exc_tlb_mask1,r2
	tst		r0,r0
	bt		1f
	and		r3,r2
	shlr8	r2
	mov.l	__exc_tlb_v,r3		! check validity
	shlr2	r2
	mov.l	@(r0,r2),r0
	tst		r0,r3
	mov.l	__exc_ptel_mask,r2	
	mov	r0,r3
	and	r2,r0		
	bt		1f		
	mov.l	r0,@(SH_MMR_CCN_PTEL_OFF,r1)

	mov		#-29,r2
	mov		r3,r0
	shld	r2,r3
	mov		#-6,r2
	shld	r2,r0
	and		#8,r0
	or		r3,r0

!msgpass?
	mov.l	__exc_tlb_msgstart,r2			!msgxfer start addr
	mov.l	@(SH_MMR_CCN_TEA_OFF,r1),r3		!fault address	
	cmp/hs	r2,r3
	bt	2f
3:

	mov.l	r0,@(SH_MMR_CCN_PTEA_OFF,r1)
	
	ldtlb
	nop		!manual said there is at least one instruction between ldtlb and rte
	nop
	nop
	rte
	nop	
1:
	mov.l	__exc_tlb_fault,r0
	jmp		@r0
	nop	
2:	!msgxfer
	mov.l	@(SH_MMR_CCN_MMUCR_OFF,r1),r2
	mov.l	__exc_tlb_mask2,r3
	and	r3,r2
	bra	3b
	mov.l	r2,@(SH_MMR_CCN_MMUCR_OFF,r1)

	.align 5	
__exc_tlb_base:
	.long	SH_CCN_BASE				
__exc_tlb_mask1:
	.long	0x003ff000
__exc_tlb_v:
	.long	SH_CCN_PTEL_V			
__exc_tlb_fault:
	.long	__exc_general_start
__exc_tlb_debug:
	.long	debug_base
__exc_ptel_mask:
	.long	~0xe0000200
__exc_tlb_msgstart:
	.long	VM_MSG_XFER_START
__exc_tlb_mask2:
	.long	~0x0000fc00
__exc_tlb_end:

#
# TLB miss (for SH4A)
#
# should not exceed 0x200 bytes, as it will be copied to exc area
#
	.align 2
__exc_tlb_start_a:
	mov.l	__exc_tlb_base_a,r1
	mov		#-22,r0
	mov.l	@(SH_MMR_CCN_TEA_OFF,r1),r2		!fault address	
	mov		r2,r3
	shld	r0,r2
	mov.l	@(SH_MMR_CCN_TTB_OFF,r1),r0		! L1 page table base
	shll2	r2
	mov.l	@(r0,r2),r0
	mov.l	__exc_tlb_mask1_a,r2
	tst		r0,r0
	bt		1f
	and		r3,r2
	shlr8	r2
	mov.l	__exc_tlb_v_a,r3		! check validity
	shlr2	r2
	mov.l	@(r0,r2),r0
	tst		r0,r3
	bt		1f		
	!msgpass?
	mov.l	__exc_tlb_msgstart_a,r2			!msgxfer start addr
	mov.l	@(SH_MMR_CCN_TEA_OFF,r1),r3		!fault address	
	cmp/hs	r2,r3
	bt	2f
3:
	mov.l	r0,@(SH_MMR_CCN_PTEL_OFF,r1)

	ldtlb
	nop		!manual said there is at least one instruction between ldtlb and rte
	nop
	nop
	rte
	nop	
1:
	mov.l	__exc_tlb_fault_a,r0
	jmp		@r0
	nop	
2:	!msgxfer
	mov.l	@(SH_MMR_CCN_MMUCR_OFF,r1),r2
	mov.l	__exc_tlb_mask2_a,r3
	and	r3,r2
	bra	3b
	mov.l	r2,@(SH_MMR_CCN_MMUCR_OFF,r1)

	.align 5	
__exc_tlb_base_a:
	.long	SH_CCN_BASE				
__exc_tlb_mask1_a:
	.long	0x003ff000
__exc_tlb_v_a:
	.long	SH_CCN_PTEL_V			
__exc_tlb_fault_a:
	.long	__exc_general_start
__exc_tlb_msgstart_a:
	.long	VM_MSG_XFER_START
__exc_tlb_mask2_a:
	.long	~0x0000fc00
__exc_tlb_end_a:
