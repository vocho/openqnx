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
# This file contains 400 series VM related stuff in the kernel. 
#
	
	.include "asmoff.def"
	.include "ppc/util.ah"
   	.include "../context.ah"

.ifdef PPC_CPUOP_ENABLED
.cpu 403
.endif

	# Note that these addresses are at the end of the ITLB miss
	# handler range. If the ITLB miss handler gets longer than
	# 0xf0 bytes, things ain't going to work.
	.set	USRPID,0x12f0
	.set	TLBROTOR,0x12f4
	.set	LOWMAP,0x12f8
	.set	L1PAGETABLE,0x12fc

routine_start set_l1pagetable, 1
	stw		%r3,L1PAGETABLE(0)
	stw		%r4,USRPID(0)
tlbia; isync
	blr
routine_end set_l1pagetable

routine_start get_l1pagetable, 1
	lwz		%r3,L1PAGETABLE(0)
	blr
routine_end get_l1pagetable

routine_start add_tlb400, 1
	# Strictly speaking, we should atomically update TLBROTOR, but it
	# doesn't really matter if we lose the operation. We're just trying
	# to randomly pick a TLB entry anyway.
	lwz		%r8,TLBROTOR(0)
	addi	%r8,%r8,1
	andi.	%r8,%r8,0x3f
	stw		%r8,TLBROTOR(0)
	lis		%r6,ppc_ienable_bits@ha
	mfspr	%r0,PPC400_SPR_PID
	lwz		%r6,ppc_ienable_bits@l(%r6)
	mfmsr	%r7
	ori		%r6,%r6,PPC_MSR_IR+PPC_MSR_DR
	andc	%r6,%r7,%r6
	mtmsr	%r6
	isync
	mtspr	PPC400_SPR_PID,%r5
	tlbwe	4,8,1
	tlbwe	3,8,0
	mtspr	PPC400_SPR_PID,%r0
	mtmsr	%r7
	isync
	blr
routine_end add_tlb400

	
#
# Handle TLB miss exceptions for the 400 series. Copied into low
# memory:
#

.set SYS400_TLBLO,PPC400_TLBLO_EX + PPC400_TLBLO_WR + (1 << PPC400_TLBLO_ZONE_SHIFT)

.macro tlb_miss addr, fault
	mtsprg	0,%r31
	mtsprg	1,%r30
	mtsprg	2,%r29
	mfcr	%r29
	mtsprg	3,%r29
	
	mfspr	%r29,&addr			# get address responsible

	lwz		%r30,LOWMAP(0)
	cmplw	%r29,%r30
	blt		2f
	lwz		%r30,L1PAGETABLE(0)
	rlwinm	%r31,%r29,12,20,29 	# extract L1 page table index bits
	lwzx	%r30,%r30,%r31
	cmplwi	%r30,0
	beq-	1f
	rlwinm	%r31,%r29,22,20,29 	# extract L2 page table index bits
	lwzx	%r30,%r30,%r31
	andi.	%r31,%r30,0xffa
	beq-	1f
	#
	# Work around a 403 chip bug. An "icbi" instruction is supposed to only
	# require read access to a page, but it really needs write access.
	# Since we only do icbi's while in the kernel, we can deal with the
	# problem by always turning on the WR bit when loading an entry for
	# kernel access.
	#
.if 0	
	# Comment out the workaround, because it screws up permission checking
	# and we don't support the 403 anymore.
	mfspr	%r31,PPC400_SPR_PID
	cmpwi	%r31,1
	bne+	4f
	ori		%r30,%r30,PPC400_TLBLO_WR
4:
.endif
	# got a valid page table entry
	lwz		%r31,TLBROTOR(0)
	addi	%r31,%r31,1
	andi.	%r31,%r31,0x3f
	stw		%r31,TLBROTOR(0)
	tlbwe	30,31,1
	rlwinm	%r30,%r29,0,0,19
	ori		%r30,%r30,PPC400_TLBHI_SIZE_4K+PPC400_TLBHI_VALID
3:
	tlbwe	30,31,0
	mfsprg	%r29,3
	mtcr	%r29
	mfsprg	%r29,2
	mfsprg	%r30,1
	mfsprg	%r31,0
	rfi
1:
	# No mapping
	mfsprg	%r29,3
	mtcr	%r29
	mfsprg	%r29,2
	mfsprg	%r30,1
	mfsprg	%r31,0
	ba		&fault
2:
	# system area one-to-one mapping
	rlwinm	%r30,%r29,0,0,7
	ori		%r30,%r30,SYS400_TLBLO
	lwz		%r31,TLBROTOR(0)
	addi	%r31,%r31,1
	andi.	%r31,%r31,0x3f
	stw		%r31,TLBROTOR(0)
	tlbwe	30,31,1
	rlwinm	%r30,%r29,0,0,7
	ori		%r30,%r30,PPC400_TLBHI_SIZE_16M+PPC400_TLBHI_VALID
	b		3b
.endm
	
#
# Data TLB miss
#
EXC_COPY_CODE_START __exc_dtlb_400
	tlb_miss PPC400_SPR_DEAR, PPC_EXC_DATA_ACCESS*4
EXC_COPY_CODE_END

#
# Instruction TLB miss
#
EXC_COPY_CODE_START __exc_itlb_400
	tlb_miss PPC_SPR_SRR0, PPC_EXC_INSTR_ACCESS*4
EXC_COPY_CODE_END

EXC_COPY_CODE_START mmu_on
	loadi	%r3,VM_KERN_LOW_SIZE
	mfspr	%r5,PPC400_SPR_PID	# need for virtual_fault() handling
	stw		%r3,LOWMAP(0)
	loadi	%r3,1
	mtspr	PPC400_SPR_PID,%r3
	mfmsr	%r3
	ori		%r3,%r3,PPC_MSR_IR+PPC_MSR_DR
	mtmsr	%r3
	isync
EXC_COPY_CODE_END

EXC_COPY_CODE_START mmu_off
	mfmsr	%r3
	rlwinm	%r3,%r3,0,28,25		# turn off translation bits
	mtmsr	%r3
	lwz		%r4,LOW_MEM_BOUNDRY(%r31)
	cmplwi	%r4,0
	bne-	1f
	lwz		%r3,USRPID(0)
	mtspr	PPC400_SPR_PID,%r3
1:
	stw		%r4,LOWMAP(0)
	isync
EXC_COPY_CODE_END
