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
# IBM versions of book E TLB routines. Standards, gotta love 'em :-(
#
# We have a TLB that maps the first 256M in slot 0, TS==0. That is
# never invalidated. The slot 1 TLB also maps the first 256M of memory
# with TS==1. That is made valid while code in procnto is running and
# invalidated when the kernel or process manager isn't active.

# Stay away from SPRG3 - if we start supporting SMP, we'll need it.
	
	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"

.ifdef PPC_CPUOP_ENABLED
.cpu booke32
.endif

.text

.set	PERM_ENTRY_SIZE,3*4


routine_start write_entry_ibm, 1
	# Initial moves because we added an extra parm to the front of
	# the list. If we re-org the remainder of the list, they can be
	# removed
	mr		%r3,%r4
	mr		%r4,%r5
	
   	cmpwi	%r3,0
	bge		4f

	BKE_GET_RANDOM %r3,0,%r5			# r5 will be trashed on SMP
4:
	# get word 1 (RPN) ready
.ifdef __LITTLEENDIAN__
	lwz		%r5,PPCBKE_TLB_RPN+0(%r4)
	lwz		%r6,PPCBKE_TLB_RPN+4(%r4)
.else
	lwz		%r5,PPCBKE_TLB_RPN+4(%r4)
	lwz		%r6,PPCBKE_TLB_RPN+0(%r4)
.endif
	rlwinm	%r5,%r5,0,0,21 
	or		%r5,%r5,%r6	
	
	# get word 2 (attr/access) ready
	lbz		%r6,PPCBKE_TLB_ACCESS(%r4)
	lhz		%r7,PPCBKE_TLB_ATTR(%r4)
	# r7 now holds the attr bits as specified by the PPCBKE_TLB_ATTR_* and PPCBKEM_TLB_ATTR_*
	# flags specified in bookecpu.h.  We need to shift them into the correct position for
	# the IBM-specified TLB word 2 that we're building up in r6.
	#
	#          IBM TLB         tlb->attr
	# WIMGE    bits 20-24      bits 27-31
	# U0-U3    bits 16-19      bits 20-23
	#
	# Insert WIMGE bits, from 27-31 -> 20-24 into r6.
	rlwimi	%r6,%r7,31-24,20,24
	# Insert Ux bits, from 20-23 -> 16-19 into r6.
	rlwimi	%r6,%r7,23-19,16,19 
	
	# get word 0 (epn/v/ts/size/tid) ready
	lwz		%r7,PPCBKE_TLB_EPN(%r4)
	rlwinm	%r7,%r7,0,0,21
	lbz		%r8,PPCBKE_TLB_V(%r4)
	rlwimi	%r7,%r8,31-22,22,22
	lbz		%r8,PPCBKE_TLB_TS(%r4)
	rlwimi	%r7,%r8,31-23,23,23
	lbz		%r8,PPCBKE_TLB_SIZE(%r4)
	rlwimi	%r7,%r8,31-27,24,27
	lwz		%r8,PPCBKE_TLB_TID(%r4)
	
	mfmsr	%r11
	lwz		%r9,ppc_ienable_bits@sdarel(%r13)
	andc	%r9,%r11,%r9
	mtmsr	%r9
	
	mfspr	%r9,PPC440_SPR_MMUCR
	rlwimi	%r9,%r8,0,24,31
	mtspr	PPC440_SPR_MMUCR,%r9
	
   	tlbwe	%r6,%r3,2
   	tlbwe	%r5,%r3,1
   	tlbwe	%r7,%r3,0
	
   	mtmsr	%r11
	isync
	blr
routine_end write_entry_ibm

routine_start read_entry_ibm, 1
	# Initial moves because we added an extra parm to the front of
	# the list. If we re-org the remainder of the list, they can be
	# removed
	mr		%r3,%r4
	mr		%r4,%r5
	
   	tlbre	%r5,%r3,0
	rlwinm	%r8,%r5,28,28,31
	stb		%r8,PPCBKE_TLB_SIZE(%r4)
	rlwinm	%r8,%r5,24,31,31
	stb		%r8,PPCBKE_TLB_TS(%r4)
	rlwinm	%r8,%r5,24,31,31
	stb		%r8,PPCBKE_TLB_TS(%r4)
	rlwinm	%r8,%r5,23,31,31
	stb		%r8,PPCBKE_TLB_V(%r4)
	rlwinm	%r8,%r5,0,0,21
	stw		%r8,PPCBKE_TLB_EPN(%r4)
	mfspr	%r8,PPC440_SPR_MMUCR
	rlwinm	%r8,%r8,0,24,31
	stw		%r8,PPCBKE_TLB_TID(%r4)
	
   	tlbre	%r5,%r3,1
	rlwinm	%r8,%r5,0,0,21
	rlwinm	%r7,%r5,0,28,31
.ifdef __LITTLEENDIAN__
	stw		%r8,PPCBKE_TLB_RPN+0(%r4)
	stw		%r7,PPCBKE_TLB_RPN+4(%r4)
.else
	stw		%r8,PPCBKE_TLB_RPN+4(%r4)
	stw		%r7,PPCBKE_TLB_RPN+0(%r4)
.endif

   	tlbre	%r5,%r3,2
	rlwinm	%r8,%r5,0,26,31
	stb		%r8,PPCBKE_TLB_ACCESS(%r4)
	# r5 holds the TLB word 2.  We need to extract the flags from the tlb to create the 
	# tlb->attr field.  TLB is defined by the IBM booke spec.  The tlb->attr field is
	# defined by the PPCBKE_TLB_ATTR_* and PPCBKEM_TLB_ATTR_* flags in bookecpu.h
	#
	#          IBM TLB         tlb->attr
	# WIMGE    bits 20-24      bits 27-31
	# U0-U3    bits 16-19      bits 20-23
	#
	# Insert WIMGE bits, from TLB bits[20-24] -> attr bits[27-31].
	rlwinm	%r8,%r5,32+24-31,27,31
	# Insert Ux bits, from TLB bits[16-19] -> attr bits[20-23]
	rlwimi	%r8,%r5,32+19-23,20,23 
	# and store it in the tlb->attr field
	sth		%r8,PPCBKE_TLB_ATTR(%r4)
	
	blr
routine_end read_entry_ibm

routine_start read_raw_ibm,1
	tlbre	%r3,%r4,0
	stw		%r3,0(%r5)
	tlbre	%r3,%r4,1
	stw		%r3,4(%r5)
	tlbre	%r3,%r4,2
	stw		%r3,8(%r5)
	li		%r3,PERM_ENTRY_SIZE
	blr
routine_end read_raw_ibm

routine_start flush_all_ibm, 1
	# have to flush the entries one by one :-(.
	lwz		%r3,tlbinfo@sdarel(%r13)
	lhz		%r3,NUM_ENTRIES(%r3)

	# get the number of wired entries
.ifdef VARIANT_smp
	GETCPU	%r6,0
	slwi	%r6,%r6,PPCBKE_RANDOM_SHIFT
	lbz		%r6,PPCBKE_RANDOM_BASE(%r6)
.else
	lbz		%r6,PPCBKE_RANDOM_BASE(%r0)
.endif
	
	mfmsr	%r11
	lwz		%r9,ppc_ienable_bits@sdarel(%r13)
	andc	%r9,%r11,%r9
	mtmsr	%r9
	
1:
	addi	%r3,%r3,-1
	tlbre	%r4,%r3,0
	rlwinm	%r5,%r4,24,30,31
	cmplwi	%r5,3		# V==1 && TS==1
	bne		2f
	
	rlwinm	%r5,%r4,0,23,21	# turn off V
	tlbwe	%r5,%r3,0
2:
	cmplw	%r3,%r6	# never invalidate the wired entries
	bgt		1b
	
   	mtmsr	%r11
	isync
	blr
routine_end flush_all_ibm

routine_start flush_asid_ibm, 1
	# have to flush the entries one by one :-(.
	lwz		%r4,tlbinfo@sdarel(%r13)
	loadi	%r12,-1			# stepping backwards through TLB
	lhz		%r4,NUM_ENTRIES(%r4)
	addi	%r4,%r4,-1
	
	mfmsr	%r11
	lwz		%r9,ppc_ienable_bits@sdarel(%r13)
	andc	%r9,%r11,%r9
	mtmsr	%r9
	
1:
	tlbre	%r5,%r4,0
	mfspr	%r6,PPC440_SPR_MMUCR
	andi.	%r6,%r6,0xff
	cmplw	%r6,%r3
	bne		2f
	
	rlwinm	%r5,%r5,0,23,21	# turn off V
	tlbwe	%r5,%r4,0
2:
	add.	%r4,%r4,%r12
	bge		1b
	
   	mtmsr	%r11
	isync
	blr
routine_end flush_asid_ibm

routine_start flush_vaddr_ibm, 1
	mfmsr	%r11
	lwz		%r9,ppc_ienable_bits@sdarel(%r13)
	andc	%r9,%r11,%r9
	mtmsr	%r9
	mfspr	%r8,PPC440_SPR_MMUCR
	rlwimi	%r4,%r8,0,0,23
	bitset	%r4,%r4,1<<PPC440_MMUCR_STS_SHIFT
	mtspr	PPC440_SPR_MMUCR,%r4

# Deal with a bug in the assembler - Book E doesn't specify how many
# operands "tlbsx." has, but gas insists that there are only two of them

.ifdef PPC_CPUOP_ENABLED
.cpu 403
.endif
	tlbsx.	%r3,0,%r3
.ifdef PPC_CPUOP_ENABLED
.cpu booke32
.endif
	bne		1f
	tlbre	%r4,%r3,0
	rlwinm	%r4,%r4,0,23,21	# turn off V
	tlbwe	%r4,%r3,0
1:
   	mtmsr	%r11
	isync
	blr
routine_end flush_vaddr_ibm
	
routine_start check_perm_ibm,0
	# This code is invoked from the TLB miss handler.
	# Registers 29-31 have already been saved. R29 has the virtual
	# address that caused the fault.
	mtspr	PPCBKE_SPR_SPRG7,%r3
	mtspr	PPCBKE_SPR_SPRG6,%r4
	
	loada	%r31,perm_tlb_mapping-PERM_ENTRY_SIZE
1:
	addi	%r31,%r31,PERM_ENTRY_SIZE
	lwz		%r30,0(%r31)
	cmplwi	%r30,0
	beq		99f		# no match
	rlwinm	%r3,%r30,29,27,30	# extract size field (times two)
	loadi	%r4,0x400
	slw		%r4,%r4,%r3
	subi	%r4,%r4,1
	andc	%r3,%r29,%r4
	andc	%r4,%r30,%r4
	cmplw	%r3,%r4
	bne		1b
	# found a match
	BKE_GET_RANDOM %r29,0,%r3		# r3 will be trashed on SMP
	
	mfspr	%r3,PPC440_SPR_MMUCR
	rlwinm	%r3,%r3,0,0,23
	mtspr	PPC440_SPR_MMUCR,%r3	# make sure TID==0
	
	tlbwe	%r30,%r29,0
	lwz		%r30,4(%r31)
	tlbwe	%r30,%r29,1
	lwz		%r30,8(%r31)
	tlbwe	%r30,%r29,2
	
	mfspr	%r31,PPCBKE_SPR_SPRG5RO
	mtlr	%r31
	mfspr	%r3,PPCBKE_SPR_SPRG7RO
	mfspr	%r4,PPCBKE_SPR_SPRG6RO
	mfsprg	%r29,3
	mtcr	%r29
	mfsprg	%r29,2
	mfsprg	%r30,1
	mfsprg	%r31,0
	rfi
99:	# no match
	mfspr	%r3,PPCBKE_SPR_SPRG7RO
	mfspr	%r4,PPCBKE_SPR_SPRG6RO
	blr
routine_end check_perm_ibm
	
#
# Handle TLB miss exceptions. Copied into low memory:
#

.macro tlb_miss_ibm addr, transbit, salt
	mtsprg	0,%r31
	li		%r31,0
	mtmsr	%r31	# can't handle critical intrs/machine checks here
	mtsprg	1,%r30
	mtsprg	2,%r29
	mfcr	%r29
	mtsprg	3,%r29
	
   	# set the TID to be used when writing the TLB entry
	mfspr	%r29,PPCBKE_SPR_PID
	mfspr	%r30,PPC440_SPR_MMUCR
	rlwimi	%r30,%r29,0,24,31 
	mtspr	PPC440_SPR_MMUCR,%r30
	
	mfspr	%r29,&addr			# get address responsible
	
	mfspr	%r31,PPC_SPR_SRR1	# check the translation space
	bittst	%r30,%r31,&transbit
	beq-	91f

	mfspr	%r30,PPCBKE_SPR_L1PAGETABLE_RD
	rlwinm	%r31,%r29,13,19,29 	# extract L1 page table index bits
	lwzx	%r31,%r30,%r31
	cmplwi	%r31,0
	beq-	90f
	rlwimi	%r31,%r29,23,20,28 	# extract L2 page table index bits
	lwz		%r30,4(%r31)
	cmplwi	%r30,0
	beq-	90f
	###
	### got a valid page table entry
	###
	
	lwz		%r29,0(%r31)		# Get RPN

	BKE_GET_RANDOM %r31,&salt,%r4,6

	tlbwe	%r29,%r31,1
	
	rlwinm	%r29,%r30,7,20,24	# Get WIMGE bits in right spot
	rlwimi	%r29,%r30,27,29,31	# Get S? access bits in the right spot
	rlwimi	%r29,%r30,30,26,28	# Get U? access bits in the right spot
	tlbwe	%r29,%r31,2
	
	mfspr	%r29,&addr				# get address responsible (again)
	rlwinm	%r29,%r29,0,0,19
	rlwimi	%r29,%r30,28,24,27		# get TLB size in right spot
	ori		%r29,%r29,(1<<9)+(1<<8) # Turn on V, TS bits
	tlbwe	%r29,%r31,0
	
	mfsprg	%r29,3
	mtcr	%r29
	mfsprg	%r29,2
	mfsprg	%r30,1
	mfsprg	%r31,0
	rfi
91:
	####
	#### TS==0, check for permanent mappings
	####
	
	mflr	%r31
	mtspr	PPCBKE_SPR_SPRG5,%r31
	loada	%r31,check_perm_ibm
	mtlr	%r31
	blrl
	mfspr	%r31,PPCBKE_SPR_SPRG5RO
	mtlr	%r31
	# fall through
90:
	####
	#### No mapping, report fault
	####
	
	mfsprg	%r29,3
	mtcr	%r29
	mfsprg	%r29,2
	mfsprg	%r30,1
	mfsprg	%r31,0
	ba		0			# patched when copied to low memory
.endm
	
#
# Data TLB miss
#
EXC_COPY_CODE_START dmiss_ibm
	tlb_miss_ibm PPCBKE_SPR_DEAR, PPC_MSR_DS, 0
EXC_COPY_CODE_END

#
# Instruction TLB miss
#
EXC_COPY_CODE_START imiss_ibm
	tlb_miss_ibm PPC_SPR_SRR0, PPC_MSR_IS, 0xff
EXC_COPY_CODE_END
  
#
# Turn MMU on/off (actually just change translation space). 
# Copied to low memory:
#

EXC_COPY_CODE_START mmu_on_ibm
	# Enable TLB entry #1
	mfspr	%r5,PPC440_SPR_MMUCR
	rlwinm	%r5,%r5,0,0,23
	mtspr	PPC440_SPR_MMUCR,%r5	# make sure TID==0
	li		%r5,(1 << 9) | (1 << 8) | (PPCBKE_TLB_SIZE_256M << 4)
	li		%r3,1
	tlbwe	%r5,%r3,0
	mfmsr	%r3
	ori		%r3,%r3,PPC_MSR_IS+PPC_MSR_DS
	mtmsr	%r3
EXC_COPY_CODE_END

EXC_COPY_CODE_START mmu_off_ibm
	mfmsr	%r3
	rlwinm	%r3,%r3,0,28,25		# turn off translation bits
	mtmsr	%r3
	lwz		%r3,LOW_MEM_BOUNDRY(%r31)
	cmplwi	%r3,0
	bne-	1f
	# Disable TLB entry #1
	li		%r0,(0 << 9) | (1 << 8) | (PPCBKE_TLB_SIZE_256M << 4)
	li		%r3,1
	tlbwe	%r0,%r3,0
1:
EXC_COPY_CODE_END
