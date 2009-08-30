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
# This file contains 800 series VM related stuff in the kernel. 
#

	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"

#	.set	LOWMAP,0x2ff4

	.set	CACHED_REGION_BOUN,	0x30000000

.set CHIP_BUG,0

/*
	First 1G: kernel space, direct map. From 0 to VM_KERN_LOW_SIZE
		0x0 -- 0x2fffffff		Kernel cached region
		0x30000000 - VM_KERN_LOW_SIZE-1	Kernel uncached region, page size 512k (for I/O, eg timer, int)
		
	From VM_KERN_LOW_SIZE to 1G: xfer buffer
	Next 3G: user space
*/

.if CHIP_BUG
#code for working around the 823 MMU bug
#use r3 to pass the data, r4 to hold the address
.macro mmu_bug val
loadi	%r3,&val
stw %r3,0(%r0)
lwz %r3,0(%r0)	
.endm
.macro mmu_bug9 val
loadi	%r9,&val
stw %r9,0(%r0)
lwz %r9,0(%r0)	
.endm
.else
.macro mmu_bug val
.endm
.macro mmu_bug9 val
.endm
.endif

routine_start set_l1pagetable, 1
	mtspr	PPC800_SPR_M_TWB,%r3
	mtspr	PPC800_SPR_M_CASID,%r4
	blr
routine_end set_l1pagetable

routine_start get_l1pagetable, 1
	mfspr	%r3,PPC800_SPR_M_TWB
	rlwinm	%r3,%r3,0,0,19 	
	blr
routine_end get_l1pagetable

#	add_tlb800(epn, twc, rpn,  i_tlb)
routine_start add_tlb800, 1
	lis		%r8,ppc_ienable_bits@ha
	mfmsr	%r7
	lwz		%r8,ppc_ienable_bits@l(%r8)
	andc	%r8,%r7,%r8
	mtmsr	%r8								#disable int 

	cmplwi	%r6, 0
	beq		1f	

	mmu_bug9 0x2980 #addr for MI_EPN

	mtspr	PPC800_SPR_MI_EPN,%r3				

	mmu_bug9 0x2b80	# addr for MI_TWC

	mtspr	PPC800_SPR_MI_TWC,%r4			

	mmu_bug9 0x2d80		# addr for MI_RPN

	mtspr	PPC800_SPR_MI_RPN,%r5			
1:	
	mmu_bug9 0x3780			#addr for MD_EPN
	
	mtspr	PPC800_SPR_MD_EPN,%r3				

	mmu_bug9 0x3b80			#addr for MD_TWC

	mtspr	PPC800_SPR_MD_TWC,%r4			

	mmu_bug9 0x3d80			#addr for MD_RPN

	mtspr	PPC800_SPR_MD_RPN,%r5			
2:	
	mtmsr	%r7

	blr
routine_end add_tlb800

.set	SYS800_TLB_TWC,(PPC800_TWC_PS_8M + PPC800_TWC_V) 			
#.set	SYS800_TLB_RPN,(PPC800_RPN_V + PPC800_RPN_LPS + (0x2 << PPC800_RPN_PP1_SHIFT) + (0x1 << PPC800_RPN_PP2_SHIFT) + 0xf0 )			
.set	SYS800_TLB_RPN_KER,(PPC800_RPN_V + PPC800_RPN_LPS + PPC800_RPN_SH + (0x1 << PPC800_RPN_PP2_SHIFT) + 0xf0 )			
.set	SYS800_TLB_TWC_NC,(PPC800_TWC_PS_512K + PPC800_TWC_V) 			
#.set	SYS800_TLB_RPN_NC,(PPC800_RPN_V + PPC800_RPN_LPS + (0x2 << PPC800_RPN_PP1_SHIFT) + (0x1 << PPC800_RPN_PP2_SHIFT) + 0xf2 )			
.set	SYS800_TLB_RPN_NC_KER,(PPC800_RPN_V + PPC800_RPN_LPS +  PPC800_RPN_SH + (0x1 << PPC800_RPN_PP2_SHIFT) + 0xf2 )			
	
EXC_COPY_CODE_START __exc_dtlb
#
# Data TLB miss
#

.if CHIP_BUG
# mmu bug
	mtsprg	2,%r3
	mtsprg	3,%r4
.endif

	mmu_bug 0x3f80			#addr for M_TW
	
	mtspr	PPC800_SPR_M_TW,%r1
	mtsprg	0,%r2
	mfcr	%r2
	mtsprg	1,%r2
	
	mfspr	%r1,PPC800_SPR_MD_EPN
	rlwinm	%r1,%r1,0,0,21
#	lwz		%r2,LOWMAP(0)
loadi	%r2,VM_KERN_LOW_SIZE
	cmplw	%r1,%r2
	bge		1f						#user space
	
	#kernel space
	loadi	%r2,CACHED_REGION_BOUN
	cmplw	%r1,%r2
	bge		4f						#no cache space
	
	loadi	%r2,(SYS800_TLB_TWC+0x0)	
	
	mmu_bug	0x3b80			#addr for MD_TWC
	
	mtspr	PPC800_SPR_MD_TWC,%r2			

	rlwinm	%r1,%r1,0,0,8
	ori		%r2,%r1,(SYS800_TLB_RPN_KER+0)
	
	mmu_bug	0x3d80			#addr for MD_RPN
	
	mtspr	PPC800_SPR_MD_RPN,%r2	

	b		2f		

4:	#kernel space, no cache, page size 512k	
	loadi	%r2,SYS800_TLB_TWC_NC	
	
	mmu_bug	0x3b80			#addr for MD_TWC
	
	mtspr	PPC800_SPR_MD_TWC,%r2			

	rlwinm	%r1,%r1,0,0,8
	ori		%r2,%r1,SYS800_TLB_RPN_NC_KER
	
	mmu_bug	0x3d80			#addr for MD_RPN

	
	mtspr	PPC800_SPR_MD_RPN,%r2	

	b		2f		


1:	#user space
	mfspr	%r1, PPC800_SPR_M_TWB	#load level 1 pointer
	lwz		%r1, 0(%r1)				#load level 1 page entry

	mr.		%r1,%r1					#check validity
	beq		3f
		
	mmu_bug	0x3b80			#addr for MD_TWC
	
	mtspr	PPC800_SPR_MD_TWC, %r1	#save level 2 base pointer and level 1 attributes		
	mfspr	%r1, PPC800_SPR_MD_TWC	#load level 2 pointer
	lwz		%r1, 0(%r1)				#load level 2 page entry

	andi.	%r2,%r1,0x0ffd			#check validity
	beq		3f
	
	mmu_bug	0x3d80			#addr for MD_RPN
	
	mtspr	PPC800_SPR_MD_RPN, %r1	#write TLB entry
2:	
.if CHIP_BUG
	mfsprg	%r3,2
	mfsprg	%r4,3
.endif

	mfsprg	%r2,1
	mtcr	%r2
	mfsprg	%r2,0
	mfspr	%r1,PPC800_SPR_M_TW		
	rfi
	
3:
# error	
# load DSISR with error type
	mfspr	%r1,PPC_SPR_DSISR
	bitset	%r1,%r1,PPC_DSISR_NOTRANS
	mtspr	PPC_SPR_DSISR,%r1

	# Chip doesn't set up DAR register for data TLB misses :-(
	mfspr	%r1,PPC800_SPR_MD_EPN
	rlwinm	%r1,%r1,0,0,21
	mtspr	PPC_SPR_DAR,%r1
	
.if CHIP_BUG
	mfsprg	%r3,2
	mfsprg	%r4,3
.endif

	mfsprg	%r2,1
	mtcr	%r2
	mfsprg	%r2,0
	mfspr	%r1,PPC800_SPR_M_TW		
	ba	PPC_EXC_DATA_ACCESS*4
EXC_COPY_CODE_END	


EXC_COPY_CODE_START __exc_itlb
#
# Instruction TLB miss
#

.if CHIP_BUG
# mmu bug
	mtsprg	2,%r3
	mtsprg	3,%r4
.endif
	
	mmu_bug 0x3f80			#addr for M_TW
	
	mtspr	PPC800_SPR_M_TW,%r1
	mtsprg	0,%r2
	mfcr	%r2
	mtsprg	1,%r2
	
	mfspr	%r1,SRR0				#load missed effective address
	rlwinm	%r1,%r1,0,0,21
#	lwz		%r2,LOWMAP(0)
loadi	%r2,VM_KERN_LOW_SIZE
	cmplw	%r1,%r2
	bge		1f						#user space
	
	#kernel space
	loadi	%r2,CACHED_REGION_BOUN
	cmplw	%r1,%r2
	bge		4f						#no cache space
	
	loadi	%r2,SYS800_TLB_TWC	
	
	mmu_bug 0x2b80			#addr for MI_TWC
	
	mtspr	PPC800_SPR_MI_TWC,%r2			
	rlwinm	%r1,%r1,0,0,8
	ori		%r2,%r1,SYS800_TLB_RPN_KER	
	
	mmu_bug	0x2d80			#addr for MI_RPN
	
	mtspr	PPC800_SPR_MI_RPN,%r2	
	b		2f	
		
4:	#kernel space, no cache, page size 512k	
	loadi	%r2,SYS800_TLB_TWC_NC	
	
	mmu_bug	0x2b80			#addr for MI_TWC
	
	mtspr	PPC800_SPR_MI_TWC,%r2			

	rlwinm	%r1,%r1,0,0,8
	ori		%r2,%r1,SYS800_TLB_RPN_NC_KER
	
	mmu_bug	0x2d80			#addr for MI_RPN
	
	mtspr	PPC800_SPR_MI_RPN,%r2	

	b		2f		

1:	#user space	
	mmu_bug	0x3780			#addr for MD_EPN
	
	mtspr	PPC800_SPR_MD_EPN,%r1	#save the miss address for level 1 table walk
	mfspr	%r1,PPC800_SPR_M_TWB	#load level 1 pointer
	lwz		%r1,0(%r1)				#load level 1 page entry
	
	mr.		%r1,%r1					#check validity
	beq		3f
	
	#we do not want the guided property inside i-tlb right now
	rlwinm	%r1,%r1,0,28,26
	
	mmu_bug	0x2b80			#addr for MI_TWC
	
	mtspr	PPC800_SPR_MI_TWC,%r1	#save level 1 attributes		
	
	mmu_bug 0x3b80			#addr for MD_TWC
	
	mtspr	PPC800_SPR_MD_TWC,%r1	#save level 2 base pointer 
	mfspr	%r1,PPC800_SPR_MD_TWC	#load level 2 pointer
	lwz		%r1,0(%r1)				#load level 2 page entry
	
	andi.	%r2,%r1,0x0ffd			#check validity
	beq		3f
	
	mmu_bug	0x2d80			#addr for MI_RPN
	
	mtspr	PPC800_SPR_MI_RPN,%r1	#write TLB entry
2:
.if CHIP_BUG
	mfsprg	%r3,2
	mfsprg	%r4,3
.endif
	
	mfsprg	%r2,1
	mtcr	%r2
	mfsprg	%r2,0
	mfspr	%r1,PPC800_SPR_M_TW		
	rfi

3:	
.if CHIP_BUG
#error
	mfsprg	%r3,2
	mfsprg	%r4,3
.endif
	
	mfsprg	%r2,1
	mtcr	%r2
	mfsprg	%r2,0
	mfspr	%r1,PPC800_SPR_M_TW		
	ba	PPC_EXC_INSTR_ACCESS*4
EXC_COPY_CODE_END	
	
	
EXC_COPY_CODE_START mmu_on
	mfspr	%r5,PPC800_SPR_M_CASID		# need for virtual_fault() handling
	andi.	%r5,%r5,0xf					
	
#	loadi	%r3,VM_KERN_LOW_SIZE
#	stw		%r3,LOWMAP(0)
	
	mmu_bug	0x3380			#addr for M_CASID
	
	mfmsr	%r3							
	# turnon mmu
	ori		%r3,%r3,PPC_MSR_IR+PPC_MSR_DR+PPC_MSR_RI
	mtmsr	%r3
	isync
EXC_COPY_CODE_END

EXC_COPY_CODE_START mmu_off
.if 0
#	lwz		%r5,LOW_MEM_BOUNDRY(%r31)
#	stw		%r5,LOWMAP(0)

	# since we're not changing anything, no need to turn off translation 
	# to do a kernel exit
	mfmsr	%r3
	rlwinm	%r3,%r3,0,28,25				# turn off translation bits (mmu)
	mtmsr	%r3
	isync
.endif
EXC_COPY_CODE_END
