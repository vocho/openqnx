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
# E500 versions of book E TLB routines. Standards, gotta love 'em :-(
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
.cpu booke
.endif

.text

.set	PERM_ENTRY_SIZE,3*4


# Define routine_start and routine_end macros for the routines that have
# both e500 and e500v2 versions
.ifdef __FOR_E500V2__

.macro routine_start_dual name flags
routine_start &name&v2, &flags
.endm

.macro routine_end_dual name
routine_end &name&v2
.endm

.else

.macro routine_start_dual name flags
routine_start &name, &flags
.endm

.macro routine_end_dual name
routine_end &name
.endm

.endif



#
# FIXME -- the read_entry and write_entry routines are BROKEN for TLB0.
#
# Fortunately they are only currently used for TLB1.  For TLB0, the index
# parameter provided by the calling code is munged with the EPN field,
# which results in writing TLB entries that aren't quite what were specified,
# and reading TLB entries that don't quite match what's actually in the
# MMU.  The calling code cannot know the correct index to provide to the
# read_entry or write_entry routine to avoid this problem without taking
# into account processor-specific information, in particular the number of
# ways in the TLB array.
#
# For now we just avoid using read_entry/write_entry for TLB0 for any
# purpose other than invalidating the TLB entries.
#

routine_start_dual write_entry_e500, 1
#                  ================
	# R3 - tlb
	# R4 - idx
	# R5 - struct ppcbke_tlb *

	cmpwi	%r3,0
	bge		1f
	li		%r3,1
1:
   	cmpwi	%r4,0
	bge		4f
	# We're assuming that we only do random writes
	# to TLB 1, since that's the only one BKE_GET_RANDOM
	# works for.
	BKE_GET_RANDOM %r4,0,%r11		# r11 will be trashed on SMP
4:

	mfmsr	%r11
	lwz		%r9,ppc_ienable_bits@sdarel(%r13)
	andc	%r9,%r11,%r9
	mtmsr	%r9

	lwz		%r9,PPCBKE_TLB_EPN(%r5)

	rlwinm	%r6,%r3,63-35,34-32,35-32	# TSEL

	# With TLB0, we only want to stick the low order bit of idx
	# (low order 2 bits for e500v2) into the MAS0.ESEL field.
	# The next higher 7 bits go into the MAS2.EPN bits 45-51
	# NYI: Can make the decision based on the TLBnCFG.ASSOC field value.
	cmplwi	%r3,0
	bne		1f
	# For TLB0, leave r4 with just the low order bits of idx
	# and prepare r9 with MAS2.EPN.
	# FIXME: see BROKEN comment above
.ifdef __FOR_E500V2__
	# e500v2 -- bits 62-63 of idx in r4, 
	#           bits 55-61 of idx in r9 at EPN (bits 45-51)
	rlwimi	%r9,%r4,61-51,45-32,51-32
	andi.	%r4,%r4,3
.else
	# e500 -- bit 63 of idx in r4, 
	#         bits 56-62 of idx in r9 at EPN (bits 45-51)
	rlwimi	%r9,%r4,62-51,45-32,51-32
	andi.	%r4,%r4,1
.endif

1:
	# Currently r6 contains just the MAS0.TSEL field and r4 contains
	# either the whole idx param (for TLB1) or just the low order bit
	# (e500v1 TLB0) or 2 bits (e500v2 TLB0).  In any case, r4 is correct
	# to define the MAS0.ESEL field so extract the low 4 bits, shift them
	# over and mask them into r6 to complete MAS0.
	rlwimi	%r6,%r4,63-47,36-32,47-32	# ESEL
	mtspr	PPCBKEM_SPR_MAS0,%r6

	lbz		%r6,PPCBKE_TLB_V(%r5)
	rlwinm	%r6,%r6,31,0,0
	lbz		%r7,PPCBKE_TLB_SIZE(%r5)
	rlwimi	%r6,%r7,63-55,52-32,55-32
	lbz		%r7,PPCBKE_TLB_TS(%r5)
	rlwimi	%r6,%r7,63-51,51-32,51-32
	lwz		%r7,PPCBKE_TLB_TID(%r5)
	rlwimi	%r6,%r7,63-47,36-32,47-32
	lhz		%r8,PPCBKE_TLB_ATTR(%r5)
	rlwimi	%r6,%r8,48-33,33-32,33-32	# IPROT bit
	mtspr	PPCBKEM_SPR_MAS1,%r6

	rlwimi	%r9,%r8,0,59-32,63-32		# WIMGE bits	
	rlwimi	%r9,%r8,32-8,57-32,58-32	# X0,X1 bits
	rlwimi	%r9,%r8,32-3,54-32,54-32	# SHAREN bit
	mtspr	PPCBKEM_SPR_MAS2,%r9

.ifdef __LITTLEENDIAN__
	lwz		%r6,PPCBKE_TLB_RPN+0(%r5)
.else
	lwz		%r6,PPCBKE_TLB_RPN+4(%r5)
.endif
	rlwimi	%r6,%r8,32-2,54-32,57-32	# U0-U3 bits
	lbz		%r7,PPCBKE_TLB_ACCESS(%r5)
	rlwimi	%r6,%r7,0,63-32,63-32		# SR
	rlwimi	%r6,%r7,1,61-32,61-32		# SW
	rlwimi	%r6,%r7,2,59-32,59-32		# SX
	rlwimi	%r6,%r7,30,62-32,62-32		# UR
	rlwimi	%r6,%r7,31,60-32,60-32		# UW
	rlwimi	%r6,%r7,0,58-32,58-32		# UX
	mtspr	PPCBKEM_SPR_MAS3,%r6

.ifdef __FOR_E500V2__
.ifdef __LITTLEENDIAN__
	lwz		%r6,PPCBKE_TLB_RPN+4(%r5)
.else
	lwz		%r6,PPCBKE_TLB_RPN+0(%r5)
.endif
	# This assumes we don't need to mask off the reserved bits.  They should be zero already.
	mtspr	PPCBKEM_SPR_MAS7,%r6
.endif
	
	tlbwe	%r0,%r0,%r0
	
	isync

	mtmsr	%r11
	blr
routine_end_dual write_entry_e500

routine_start_dual read_entry_e500, 1
#                  ===============
	# R3 - tlb
	# R4 - idx
	# R5 - struct ppcbke_tlb *


	mfmsr	%r11
	lwz		%r9,ppc_ienable_bits@sdarel(%r13)
	andc	%r9,%r11,%r9
	mtmsr	%r9

	mr		%r9,%r4

	# r6 will be used for MAS0 -- clear it and set TSEL
	rlwinm	%r6,%r3,63-35,34-32,35-32	# TSEL

	# Is it TLB0 (2- or 4-way set associative) or TLB1 (fully associative)?
	# NYI: Could make the decision based on the TLBnCFG.ASSOC field value.
	# For TLB1 we skip the setup of MAS2.
	cmplwi	%r3,0
	bne		1f
	
	# With TLB0, we only want to stick the low order bit of idx
	# (low order 2 bits for e500v2) into the MAS0.ESEL field.
	# The next higher 7 bits go into the MAS2.EPN bits 45-51.
	# We'll use r7 for MAS2.  We'll pare off the upper bits of
	# idx so that when we set MAS0.ESEL below we only grab the
	# necessary bit or bits.
.ifdef __FOR_E500V2__
	# e500v2 -- bits 62-63 of idx in r9, 
	#           bits 55-61 of idx in r7 at EPN (bits 45-51)
	rlwinm	%r7,%r4,61-51,45-32,51-32
	andi.	%r9,%r4,3
.else
	# e500 -- bit 63 of idx in r9, 
	#         bits 56-62 of idx in r7 at EPN (bits 45-51)
	rlwinm	%r7,%r4,62-51,45-32,51-32
	andi.	%r9,%r4,1
.endif
	mtspr	PPCBKEM_SPR_MAS2,%r7

1:
	# Currently r6 = MAS0.TSEL.  Mask in ESEL from r9 and set it.
	rlwimi	%r6,%r9,63-47,36-32,47-32	# ESEL
	mtspr	PPCBKEM_SPR_MAS0,%r6

	tlbre	%r0,%r0,%r0		# operands ignored

	mfspr	%r6,PPCBKEM_SPR_MAS1
	mfspr	%r7,PPCBKEM_SPR_MAS2
	mfspr	%r8,PPCBKEM_SPR_MAS3
.ifdef __FOR_E500V2__
	# This assumes we don't need to mask off the reserved bits.  They should be zero already.
	mfspr	%r10,PPCBKEM_SPR_MAS7
.else
	li		%r10,0
.endif

	mtmsr	%r11

	cmplwi	%r3,0
	bne		1f
	# Make sure the EPN address bits that control which entry the
	# thing goes into match up with the requested index
	# FIXME: see BROKEN comment above
	rlwimi	%r7,%r4,11,45-32,51-32
1:

	rlwinm	%r9,%r6,1,31,31
	stb		%r9,PPCBKE_TLB_V(%r5)
	rlwinm	%r9,%r6,47-31,31-12,31
	stw		%r9,PPCBKE_TLB_TID(%r5)
	rlwinm	%r9,%r6,51-31,31,31
	stb		%r9,PPCBKE_TLB_TS(%r5)
	rlwinm	%r9,%r6,55-31,28,31
	stb		%r9,PPCBKE_TLB_SIZE(%r5)
	rlwinm	%r9,%r8,0,32-32,51-32
.ifdef __LITTLEENDIAN__
	stw		%r9,PPCBKE_TLB_RPN+0(%r5)
	stw		%r10,PPCBKE_TLB_RPN+4(%r5)
.else
	stw		%r9,PPCBKE_TLB_RPN+4(%r5)
	stw		%r10,PPCBKE_TLB_RPN+0(%r5)
.endif
	rlwinm	%r9,%r7,0,32-32,51-32
	stw		%r9,PPCBKE_TLB_EPN(%r5)
	rlwinm	%r9,%r7,0,27,31			# WIMGE bits
	rlwimi	%r9,%r8,2,20,23			# U0-U3 bits
	rlwimi	%r9,%r7,3,19,19			# SHAREN bit
	rlwimi	%r9,%r7,8,17,18			# X0,X1 bits
	rlwimi	%r9,%r6,17,16,16		# IPROT bit
	sth		%r9,PPCBKE_TLB_ATTR(%r5)

	rlwinm	%r9,%r8,0,31,31			# SR bit
	rlwimi	%r9,%r8,31,30,30		# SW bit
	rlwimi	%r9,%r8,30,29,29		# SX bit
	rlwimi	%r9,%r8,2,28,28			# UR bit
	rlwimi	%r9,%r8,1,27,27			# UW bit
	rlwimi	%r9,%r8,0,26,26			# UX bit
	stb		%r9,PPCBKE_TLB_ACCESS(%r5)

	blr
routine_end_dual read_entry_e500

routine_start_dual read_raw_e500, 1
#                  =============
	# R3: tlb
	# R4: idx
	# R5: void *

	rlwinm	%r6,%r3,63-35,34-32,35-32	# TSEL

	# Is it TLB0 (2- or 4-way set associative) or TLB1 (fully associative)?
	# NYI: Could make the decision based on the TLBnCFG.ASSOC field value.
	# For TLB1 we skip the setup of MAS2.
	cmplwi	%r3,0
	bne		1f
	
	# With TLB0, we only want to stick the low order bit of idx
	# (low order 2 bits for e500v2) into the MAS0.ESEL field.
	# The next higher 7 bits go into the MAS2.EPN bits 45-51.
	# We'll use r7 for MAS2.  We'll pare off the upper bits of
	# idx so that when we set MAS0.ESEL below we only grab the
	# necessary bit or bits.
.ifdef __FOR_E500V2__
	# e500v2 -- bits 62-63 of idx in r9, 
	#           bits 55-61 of idx in r7 at EPN (bits 45-51)
	rlwinm	%r7,%r4,61-51,45-32,51-32
	andi.	%r4,%r4,3
.else
	# e500 -- bit 63 of idx in r9, 
	#         bits 56-62 of idx in r7 at EPN (bits 45-51)
	rlwinm	%r7,%r4,62-51,45-32,51-32
	andi.	%r4,%r4,1
.endif
	mtspr	PPCBKEM_SPR_MAS2,%r7

1:
	rlwimi	%r6,%r4,63-47,36-32,47-32	# ESEL
	mtspr	PPCBKEM_SPR_MAS0,%r6

	tlbre	%r0,%r0,%r0		# operands ignored

	mfspr	%r3,PPCBKEM_SPR_MAS1
	stw		%r3,0(%r5)
	mfspr	%r3,PPCBKEM_SPR_MAS2
	stw		%r3,4(%r5)
	mfspr	%r3,PPCBKEM_SPR_MAS3
	stw		%r3,8(%r5)
.ifdef __FOR_E500V2__
	mfspr	%r3,PPCBKEM_SPR_MAS7
	stw		%r3,12(%r5)
.endif
	li		%r3,PERM_ENTRY_SIZE
	blr
routine_end_dual read_raw_e500

routine_start_dual flush_all_e500, 1
#                  ==============
	lwz		%r5,tlbinfo@sdarel(%r13)
	lwz		%r12,num_tlb@sdarel(%r13)

	li		%r4,0
	
98:
	# have to flush the entries one by one :-(.
	lhz		%r6,NUM_ENTRIES(%r5)
	
	mfmsr	%r11
	lwz		%r9,ppc_ienable_bits@sdarel(%r13)
	andc	%r9,%r11,%r9
	mtmsr	%r9
	
99:
	addi	%r6,%r6,-1

	rlwinm	%r7,%r4,63-35,34-32,35-32	# TSEL
	mr		%r9,%r6

	# Is it TLB0 (2- or 4-way set associative) or TLB1 (fully associative)?
	# NYI: Could make the decision based on the TLBnCFG.ASSOC field value.
	# For TLB1 we skip the setup of MAS2.
	cmplwi	%r4,0
	bne		1f
	
	# With TLB0, we only want to stick the low order bit of idx
	# (low order 2 bits for e500v2) into the MAS0.ESEL field.
	# The next higher 7 bits go into the MAS2.EPN bits 45-51.
	# We'll use r7 for MAS2.  We'll pare off the upper bits of
	# idx so that when we set MAS0.ESEL below we only grab the
	# necessary bit or bits.
.ifdef __FOR_E500V2__
	# e500v2 -- bits 62-63 of idx in r9, 
	#           bits 55-61 of idx in r7 at EPN (bits 45-51)
	rlwinm	%r7,%r6,61-51,45-32,51-32
	andi.	%r9,%r6,3
.else
	# e500 -- bit 63 of idx in r9, 
	#         bits 56-62 of idx in r7 at EPN (bits 45-51)
	rlwinm	%r7,%r6,62-51,45-32,51-32
	andi.	%r9,%r6,1
.endif
	mtspr	PPCBKEM_SPR_MAS2,%r7

1:
	rlwimi	%r7,%r9,63-47,36-32,47-32	# ESEL
	mtspr	PPCBKEM_SPR_MAS0,%r7

	tlbre	%r0,%r0,%r0		# operands ignored

	mfspr	%r7,PPCBKEM_SPR_MAS1
	rlwinm	%r8,%r7,2,30,31
	cmplwi	%r8,2		# V==1 && IPROT=0
	bne		1f
	
	rlwinm	%r7,%r7,0,1,31	# turn off V
	mtspr	PPCBKEM_SPR_MAS1,%r7
	tlbwe	%r0,%r0,%r0		# operands ignored

1:
	cmplwi	%r6,0
	bgt		99b
	
   	mtmsr	%r11

	addi	%r4,%r4,1
	addi	%r5,%r5,SIZEOF_TLBINFO
	cmplw	%r4,%r12
	blt		98b

	isync
	blr
routine_end_dual flush_all_e500

routine_start_dual flush_asid_e500, 1
#                  ===============
	lwz		%r5,tlbinfo@sdarel(%r13)
	lwz		%r12,num_tlb@sdarel(%r13)

	li		%r4,0
	
98:
	# have to flush the entries one by one :-(.
	lhz		%r6,NUM_ENTRIES(%r5)
	
	mfmsr	%r11
	lwz		%r9,ppc_ienable_bits@sdarel(%r13)
	andc	%r9,%r11,%r9
	mtmsr	%r9
	
99:
	addi	%r6,%r6,-1

	rlwinm	%r7,%r4,63-35,34-32,35-32	# TSEL

	mr		%r9,%r6

	# Is it TLB0 (2- or 4-way set associative) or TLB1 (fully associative)?
	# NYI: Could make the decision based on the TLBnCFG.ASSOC field value.
	# For TLB1 we skip the setup of MAS2.
	cmplwi	%r4,0
	bne		1f
	
	# With TLB0, we only want to stick the low order bit of idx
	# (low order 2 bits for e500v2) into the MAS0.ESEL field.
	# The next higher 7 bits go into the MAS2.EPN bits 45-51.
	# We'll use r7 for MAS2.  We'll pare off the upper bits of
	# idx so that when we set MAS0.ESEL below we only grab the
	# necessary bit or bits.
.ifdef __FOR_E500V2__
	# e500v2 -- bits 62-63 of idx in r9, 
	#           bits 55-61 of idx in r7 at EPN (bits 45-51)
	rlwinm	%r7,%r6,61-51,45-32,51-32
	andi.	%r9,%r6,3
.else
	# e500 -- bit 63 of idx in r9, 
	#         bits 56-62 of idx in r7 at EPN (bits 45-51)
	rlwinm	%r7,%r6,62-51,45-32,51-32
	andi.	%r9,%r6,1
.endif
	mtspr	PPCBKEM_SPR_MAS2,%r7

1:
	rlwimi	%r7,%r9,63-47,36-32,47-32	# ESEL
	mtspr	PPCBKEM_SPR_MAS0,%r7

	tlbre	%r0,%r0,%r0		# operands ignored

	mfspr	%r7,PPCBKEM_SPR_MAS1
	
	rlwinm	%r9,%r7,16,23,31	# Check the ASID matches.
	cmplw	%r9,%r3
	bne		1f
	
	rlwinm	%r7,%r7,0,1,31	# turn off V
	mtspr	PPCBKEM_SPR_MAS1,%r7
	tlbwe	%r0,%r0,%r0		# operands ignored

1:
	cmplwi	%r6,0
	bgt		99b
	
   	mtmsr	%r11

	addi	%r4,%r4,1
	addi	%r5,%r5,SIZEOF_TLBINFO
	cmplw	%r4,%r12
	blt		98b

	isync
	blr
routine_end_dual flush_asid_e500

# Note: we don't need a separate e500v2 version
.ifndef __FOR_E500V2__
routine_start flush_vaddr_e500, 1
#             ================
	# E500 ignores PID for the tlbivax instruction, so we don't have
	# to worry about transfering the R4 value into anything
	rlwinm	%r3,%r3,0,0,19	# E500 puts flags in bottom bits...

	tlbivax	%r0,%r3			# Invalidate from TLB0
	bitset	%r3,%r3,0x8
	tlbivax	%r0,%r3			# Invalidate from TLB1
	blr
routine_end flush_vaddr_e500
.endif # ifndef e500v2


# Think about using end of PPC_KEREXIT_COMMON sequence for save area...
.set SAVE_BASE,	(PPC_KERENTRY_COMMON-(8*4))

# It turns out that it's faster to use memory to save/restore registers
# on the E500 than it is to use the SPRG's, though not by much (1% best
# case). Swapping the commented out lines in the two macros below will
# switch back to SPRG's, though there's a bug in the 6.3.0 assembler that
# needs to be worked around (it won't allow SPRG4-7 in the m[t,f]sprg instr).

.macro REG_SAVE sprg,reg
.ifdef VARIANT_smp
	mtsprg	&sprg,&reg
.else
	stw		&reg, &sprg*4+SAVE_BASE(0)
.endif
.endm

.macro REG_RESTORE reg, sprg
.ifdef VARIANT_smp
	mfsprg	&reg,&sprg	
.else
	lwz		&reg, &sprg*4+SAVE_BASE(0)
.endif
.endm	
	
routine_start_dual check_perm_e500,0
#                  ===============
	# This code is invoked from the TLB miss handler.
	# Registers 29-31 have already been saved. R29 has the virtual
	# address that caused the fault.

	## We're going to make the assumption that all the permanent entries
	## are in TLB1.

	REG_SAVE	6,%r4
	
	loada	%r31,perm_tlb_mapping-PERM_ENTRY_SIZE
1:
	addi	%r31,%r31,PERM_ENTRY_SIZE
	lwz		%r30,0(%r31)
	cmplwi	%r30,0
	beq		99f		# no match
	rlwinm	%r30,%r30,56-32+1,27,30	# extract size field (times two)
	loadi	%r4,0x400
	slw		%r4,%r4,%r30
	lwz		%r30,4(%r31)
	subi	%r4,%r4,1
	andc	%r30,%r30,%r4
	andc	%r4,%r29,%r4
	cmplw	%r30,%r4
	bne		1b
	# found a match
	BKE_GET_RANDOM %r29,0,%r30			# r30 will be trashed on SMP

	rlwinm	%r29,%r29,16,44-32,47-32
	bitset	%r29,%r29,0x10000000		# select TLB1
	mtspr	PPCBKEM_SPR_MAS0,%r29
	
	lwz		%r30,0(%r31)
	mtspr	PPCBKEM_SPR_MAS1,%r30
	lwz		%r30,4(%r31)
	mtspr	PPCBKEM_SPR_MAS2,%r30
	lwz		%r30,8(%r31)
	mtspr	PPCBKEM_SPR_MAS3,%r30
.ifdef __FOR_E500v2__
	lwz		%r30,12(%r31)
	mtspr	PPCBKEM_SPR_MAS7,%r30
.endif

	tlbwe	%r0,%r0,%r0		# operands ignored
	
	REG_RESTORE	%r31,5
	mtlr	%r31
	REG_RESTORE	%r4,6
	REG_RESTORE	%r29,7
	mtcr	%r29
	REG_RESTORE	%r29,2
	REG_RESTORE	%r30,1
	REG_RESTORE	%r31,0
	rfi
99:	# no match
	REG_RESTORE	%r4,6
	blr
routine_end_dual check_perm_e500
	
#
# Handle TLB miss exceptions. Copied into low memory:
#

.macro tlb_miss_e500 addr, transbit, salt
	##NYI: E500
	REG_SAVE	0,%r31
	li		%r31,0
	mtmsr	%r31	# can't handle critical intrs/machine checks here
	REG_SAVE	1,%r30
	REG_SAVE	2,%r29
	mfcr	%r29
	REG_SAVE	7,%r29
	
	mfspr	%r29,&addr			# get address responsible
	
	mfspr	%r31,PPC_SPR_SRR1	# check the translation space
	bittst	%r30,%r31,&transbit
	beq-	91f

	mfspr	%r30,PPCBKE_SPR_L1PAGETABLE_RD
	rlwinm	%r31,%r29,13,19,29 	# extract L1 page table index bits
	lwzx	%r31,%r30,%r31
	cmplwi	%r31,0
	beq-	90f
	rlwimi	%r31,%r29,23,20,28 	# insert L2 page table index bits
	lwz		%r30,4(%r31)
	cmplwi	%r30,0
	beq-	90f
	###
	### got a valid page table entry
	###

	# check for a non-4K page. The following test isn't quite correct,
	# since 1K pages get treated the same as 4K, but we don't use
	# 1K pages anywhere and this is faster than doing the correct test.
	bittst	%r29,%r30,(0xe << 8)
	beq+ 	89f
	# non-4K page
	mfspr	%r29,PPCBKEM_SPR_MAS1
	rlwimi	%r29,%r30,0,20,23			# put page size into right spot
	mtspr	PPCBKEM_SPR_MAS1,%r29

	BKE_GET_RANDOM %r29,&salt,%r4,6
	rlwinm	%r29,%r29,16,44-32,47-32
	bitset	%r29,%r29,0x10000000		# select TLB1
	mtspr	PPCBKEM_SPR_MAS0,%r29
89:	

	mfspr	%r29,PPCBKEM_SPR_MAS2
	rlwimi	%r29,%r30,0,27,31		# Insert WIMGE bits
	mtspr	PPCBKEM_SPR_MAS2,%r29

	rlwinm	%r29,%r30,32-5,31,31	# Get SR access bit in the right spot
	rlwimi	%r29,%r30,32-4,29,29	# Get SW access bit in the right spot
	rlwimi	%r29,%r30,32-3,27,27	# Get SX access bit in the right spot
	rlwinm	%r30,%r29,1,0,30		# Get U? access bits in the right spot
	or		%r29,%r29,%r30

	lwz		%r30,0(%r31)
	rlwimi	%r29,%r30,0,0,19		# Get low order RPN
	mtspr	PPCBKEM_SPR_MAS3,%r29
	
.ifdef __FOR_E500V2__
	rlwinm	%r29,%r30,0,20,31
	mtspr	PPCBKEM_SPR_MAS7,%r29
.endif

	tlbwe	%r0,%r0,%r0				# operands ignored
	
	REG_RESTORE	%r29,7
	mtcr	%r29
	REG_RESTORE	%r29,2
	REG_RESTORE	%r30,1
	REG_RESTORE	%r31,0
	rfi
91:
	####
	#### TS==0, check for permanent mappings
	####
	
	mflr	%r31
	REG_SAVE	5,%r31
.ifdef __FOR_E500V2__
	loada	%r31,check_perm_e500v2
.else
	loada	%r31,check_perm_e500
.endif
	mtlr	%r31
	blrl
	REG_RESTORE	%r31,5
	mtlr	%r31
	# fall through
90:
	####
	#### No mapping, report fault
	####
	
	REG_RESTORE	%r29,7
	mtcr	%r29
	REG_RESTORE	%r29,2
	REG_RESTORE	%r30,1
	REG_RESTORE	%r31,0
	ba		0			# patched when copied to low memory
.endm
	
	
.ifdef __FOR_E500V2__
#
# Data TLB miss
#
EXC_COPY_CODE_START dmiss_e500v2
	tlb_miss_e500 PPCBKE_SPR_DEAR, PPC_MSR_DS, 0
EXC_COPY_CODE_END

#
# Instruction TLB miss
#
EXC_COPY_CODE_START imiss_e500v2
	tlb_miss_e500 PPC_SPR_SRR0, PPC_MSR_IS, 0xff
EXC_COPY_CODE_END
.else
#
# Data TLB miss
#
EXC_COPY_CODE_START dmiss_e500
	tlb_miss_e500 PPCBKE_SPR_DEAR, PPC_MSR_DS, 0
EXC_COPY_CODE_END

#
# Instruction TLB miss
#
EXC_COPY_CODE_START imiss_e500
	tlb_miss_e500 PPC_SPR_SRR0, PPC_MSR_IS, 0xff
EXC_COPY_CODE_END
.endif
  
#
# Turn MMU on/off (actually just change translation space). 
# Copied to low memory:
#

# Note that we don't need different versions of mmu_on and mmu_off for e500v2
.ifndef __FOR_E500V2__
EXC_COPY_CODE_START mmu_on_e500
	# Enable TLB entry #1
	loadi	%r3,(1 << (63-35)) | (1 << (63-47))
	mtspr	PPCBKEM_SPR_MAS0,%r3
	tlbre	%r0,%r0,%r0			# operands ignored
	mfspr	%r3,PPCBKEM_SPR_MAS1
	bitset	%r3,%r3,0x80000000
	mtspr	PPCBKEM_SPR_MAS1,%r3
	tlbwe	%r0,%r0,%r0			# operands ignored
	mfmsr	%r3
	ori		%r3,%r3,PPC_MSR_IS+PPC_MSR_DS
# WORKAROUND for errata CPU29	
.if 0	
	mtmsr	%r3
.else	
	# NOTE: This code is assuming that it is going to be
    # immediately followed by a "BLR" instruction that transfers
    # to the second stage exception handler code that's actually
    # in the procnto executable
	mtsrr1	%r3
	mflr	%r3
	mtsrr0	%r3
	rfi
.endif
# end WORKAROUND for errata CPU29	
EXC_COPY_CODE_END

EXC_COPY_CODE_START mmu_off_e500
	mfmsr	%r3
	rlwinm	%r3,%r3,0,28,25		# turn off translation bits
# WORKAROUND for errata CPU29	
.if 0		
	mtmsr	%r3
.else
	mtsrr1	%r3
	bl		next_addr
next_addr:
	mflr	%r3
	addi	%r3,%r3,rfi_addr-next_addr
	mtsrr0	%r3
	rfi
rfi_addr:	
.endif
# end WORKAROUND for errata CPU29	
	lwz		%r3,LOW_MEM_BOUNDRY(%r31)
	cmplwi	%r3,0
	bne-	1f
	# Disable TLB entry #1
	loadi	%r3,(1 << (63-35)) | (1 << (63-47))
	mtspr	PPCBKEM_SPR_MAS0,%r3
	tlbre	%r0,%r0,%r0			# operands ignored
	mfspr	%r3,PPCBKEM_SPR_MAS1
	bitclr	%r3,%r3,0x80000000
	mtspr	PPCBKEM_SPR_MAS1,%r3
	tlbwe	%r0,%r0,%r0			# operands ignored
1:
EXC_COPY_CODE_END

.endif # e500v2
