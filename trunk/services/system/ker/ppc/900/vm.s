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
# This file contains 900 series VM related stuff in the kernel. 
#


	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"


.macro save_regs_r123
	mtsprg	0,%r1
	mtsprg	1,%r2
	mtsprg	2,%r3
.endm

.macro restore_regs_r123
	mfsprg	%r3,2
	mfsprg	%r2,1
	mfsprg	%r1,0
.endm

#
# We need to define the variables here, so that the kernel can link on its own.
#

.set NUM_XFER_MAPPINGS,2	# must match defn in mem_virtual.c

	.globl HashTable_base
	.section ".sdata","aw"
	.align 2
	.type	 HashTable_base,@object
	.size	 HashTable_base,4
HashTable_base:
	.long -1

	.globl HashTable_mask
	.align 2
	.type	 HashTable_mask,@object
	.size	 HashTable_mask,4
HashTable_mask:
	.long 0

	.globl ht_slock
	.section ".sdata","aw"
	.align 2
ht_slock:
	.space	4
	.size	 ht_slock,4

#
# These are defined in mem_virtual.c, we put in a weak definition for them here
# just so tnto can link
#
	.weak xfer_rotor
	.section ".sdata","aw"
	.align 2
xfer_rotor:
	.space	4
	.size	 xfer_rotor,4

	.weak xfer_lastaddr
	.section ".sdata","aw"
	.align 2
xfer_lastaddr:
	.space	4*NUM_XFER_MAPPINGS
	.size	 xfer_lastaddr,4*NUM_XFER_MAPPINGS

	.weak xfer_pgdir
	.section ".sdata","aw"
	.align 2
xfer_pgdir:
	.space	4
	.size	 xfer_pgdir,4

	.weak xfer_diff
	.section ".sdata","aw"
	.align 2
xfer_diff:
	.space	4*NUM_XFER_MAPPINGS
	.size	 xfer_diff,4*NUM_XFER_MAPPINGS


	.section ".text"

#
# This is per-cpu info kept in the reserved area
# each CPU gets 0x40 bytes of space (64 bytes). This is
# used to save registers in the hash table refill handler, 
# and to save the VSID, LOWMAP and L1PAGETABLE
#
# Note: this leaves us enough room for 8 CPU's.
#
# On SMP, we use SPRG3 to identify which cpu we are.
#

	.set	RESERVED_BASE,0x1e00
	.set	CPU_OFFSET_SHIFT,0x06		# 0x40 = 1 << 6
	.set	STACK_OFFSET,0x00
	.set	LOWMAP_OFFSET,0x34
	.set	VSID_OFFSET,0x38
	.set	L1PAGETABLE_OFFSET,0x3c


.macro	get_cpu_offset reg
.ifdef VARIANT_smp
	GETCPU  &reg,CPU_OFFSET_SHIFT
.else
	li	&reg,0
.endif
.endm

/*
	First 1G: kernel space, direct map. From 0 to VM_KERN_LOW_SIZE
	From VM_KERN_LOW_SIZE to 1G: xfer buffer
	Next 3G: user space
*/
	
# This routine sets a new active prp (vsid)
# %r3: pgdir to set
# %r4: vsid
#
# IRQ's should always be off.

routine_start set_l1pagetable, 1
	get_cpu_offset %r5
	# Save L1 pagetable
	stw		%r3,RESERVED_BASE+L1PAGETABLE_OFFSET(%r5)

	# Load segment registers
	stw		%r4,RESERVED_BASE+VSID_OFFSET(%r5)

	rlwinm	%r5,%r4,PPC64_SLB0_VSID_SHIFT,0,23
	ori		%r5,%r5,PPC64_SLB0_KS+PPC64_SLB0_KP + (4 << PPC64_SLB0_VSID_SHIFT)
	loadi	%r6,0x40000000+0x4
	bitset	%r4,%r6,PPC64_SLB1_V
	loadi	%r7,0x10000001
	loadi	%r3,0x10-5
	mtctr	%r3
	# Sync, to make sure all old accesses are completed in right context
	sync


1:
	slbie	%r6		# cause the [I,D]-ERAT caches to invalidate
	slbmte	%r5,%r4
	addi	%r5,%r5,1 << PPC64_SLB0_VSID_SHIFT
	add		%r6,%r6,%r7
	add		%r4,%r4,%r7
	bdnz+ 	1b
.ifdef VARIANT_smp
	# last entry (idx=0xf) loads with a value that's CPU specific
	# so that we can have different translations for the CPU page.
	GETCPU  %r3,PPC64_SLB0_VSID_SHIFT
	add		%r5,%r5,%r3
.endif
	slbie	%r6		# cause the [I,D]-ERAT caches to invalidate
	slbmte	%r5,%r4

#.cpu ppc64bridge
#	mfsr	%r4,0
#	mtsr	0,%r4

	isync

	blr
routine_end set_l1pagetable


routine_start get_l1pagetable, 1
	get_cpu_offset %r5
	lwz		%r3,RESERVED_BASE+L1PAGETABLE_OFFSET(%r5)
	blr
routine_end get_l1pagetable


	
EXC_COPY_CODE_START mmu_on
	get_cpu_offset %r7
# load address boundary
	lwz			%r5,RESERVED_BASE+VSID_OFFSET(%r7)
	
	loadi		%r3,VM_KERN_LOW_SIZE
	stw			%r3,RESERVED_BASE+LOWMAP_OFFSET(%r7)
	
	# re-enable the one-to-one mapping SLB entry 
	loadi	%r3,0x00000580
	loadi	%r4,0x08000000
	slbmte  %r3,%r4

	# re-enable the device mapping SLB entry 
	loadi	%r3,0x00003480
	loadi	%r4,0x38000003
	slbmte  %r3,%r4

# turn on mmu
	mfmsr	%r3							
	ori		%r3,%r3,PPC_MSR_IR + PPC_MSR_DR + PPC_MSR_RI
	rlwinm	%r3,%r3,0,0,31	# turn off MSR[SF] bit
	mtmsrd	%r3
	isync
	# Same context, no need to sync because we previously had 
	# translation disabled, which implies guarded storage access, so 
	# we cannot have speculatively started loading across the above isync
EXC_COPY_CODE_END


EXC_COPY_CODE_START mmu_off
#
# Note: we don't need to turn translation off if we're not changing the SLB
#
	# return to user space	
	# check low address boundry
	get_cpu_offset %r5
	loadi	%r3,0x10000000
	lwz		%r4,LOW_MEM_BOUNDRY(%r31)
	cmplw	%r4,%r3
	stw		%r4,RESERVED_BASE+LOWMAP_OFFSET(%r5)
	bgt		1f
	lwz		%r5,TFLAGS(%r31)
	bittst	%r3,%r5,_NTO_TF_IOPRIV
	beq+	1f

#
# This is a thread with IOPRIV. Need to disable special SLB entries
#
# turn off mmu
	mfmsr	%r3
	rlwinm	%r3,%r3,0,28,25				
	mtmsr	%r3
	isync
	sync
	
	# disable the one-to-one mapping SLB entry 
	loadi	%r3,0x08000000
	slbie	%r3

	# disable the msg pass SLB entries
	loadi	%r3,0x10000000
	slbie	%r3
	loadi	%r3,0x20000000
	slbie	%r3

	# disable the device mapping SLB entry
	loadi	%r3,0x38000000
	slbie	%r3
	isync
1:
EXC_COPY_CODE_END

#
# Fault refill handlers
# 
# Check if we can get a valid mapping from pagetable, and
# install mapping in hash table. This is done with IRQ's disabled, 
# i.e. we do not do a full "enterkernel" for this. The overhead
# of entering the kernel and saving everything is about the same as 
# this handler.....
#

# Save %r1, %r2, %r3 
# Set up stack in %r1, save r0,r4,r5,r6,r7,r8,ctr,CR there
# Set up stack and save further regs

# Careful: Only 0x80 bytes of storage available for this exception block
EXC_COPY_CODE_START __exc_data_access900
	mtsprg	0,%r1
	mtsprg	1,%r2
	get_cpu_offset %r1
	stw		%r0,RESERVED_BASE+STACK_OFFSET+0(%r1)
	mtsprg	2,%r3
	stw		%r4,RESERVED_BASE+STACK_OFFSET+4(%r1)
	mfcr	%r3
	stw		%r5,RESERVED_BASE+STACK_OFFSET+8(%r1)
	stw		%r3,RESERVED_BASE+STACK_OFFSET+28(%r1)
	stw		%r6,RESERVED_BASE+STACK_OFFSET+12(%r1)
	mfctr	%r6
	loada	%r2,__exc_access900
	mtctr	%r2
	stw		%r7,RESERVED_BASE+STACK_OFFSET+16(%r1)
	li		%r2,0
	mfdsisr	%r5
	stw		%r8,RESERVED_BASE+STACK_OFFSET+20(%r1)
	mfspr	%r3,PPC_SPR_DAR 	# get address responsible
	bittst	%r8,%r5,0x02000000
	stw		%r6,RESERVED_BASE+STACK_OFFSET+24(%r1)
	stw		%r9,RESERVED_BASE+STACK_OFFSET+32(%r1)
	beq		1f
	bitset	%r2,%r2,VM_FAULT_WRITE		# indicate a store instruction
1:
	bittst	%r8,%r5,0x04000000
	beq		1f
	bitset	%r2,%r2,VM_FAULT_WIMG_ERR	# indicate a WIMG error
1:
	bctr
EXC_COPY_CODE_END
	

# Careful: Only 0x80 bytes of storage available for this exception block
EXC_COPY_CODE_START __exc_instr_access900
# Save %r1, %r2, %r3 
# Save r0,r4,r5,r6,r7,r8,ctr,CR in stack area of CPU low mem region
	mtsprg	0,%r1
	mtsprg	1,%r2
	get_cpu_offset %r1
	stw		%r0,RESERVED_BASE+STACK_OFFSET+0(%r1)
	mtsprg	2,%r3
	stw		%r4,RESERVED_BASE+STACK_OFFSET+4(%r1)
	mfcr	%r3
	stw		%r5,RESERVED_BASE+STACK_OFFSET+8(%r1)
	stw		%r3,RESERVED_BASE+STACK_OFFSET+28(%r1)
	stw		%r6,RESERVED_BASE+STACK_OFFSET+12(%r1)
	mfctr	%r6
	loada	%r2,__exc_access900
	mtctr	%r2
	stw		%r7,RESERVED_BASE+STACK_OFFSET+16(%r1)
	mfspr	%r5,SRR1
	stw		%r8,RESERVED_BASE+STACK_OFFSET+20(%r1)
	mfspr	%r3,SRR0
	loadi	%r2,VM_FAULT_INSTR	# say it was an instruction access
	bittst	%r8,%r5,0x18000000
	beq		1f
	bitset	%r2,%r2,VM_FAULT_WIMG_ERR	# indicate a WIMG error
1:	
	stw		%r6,RESERVED_BASE+STACK_OFFSET+24(%r1)
	stw		%r9,RESERVED_BASE+STACK_OFFSET+32(%r1)
	bctr
EXC_COPY_CODE_END
	
#
# On a fault, this checks to see if we can simply
# pluck a new (valid) mapping in the hash table. This occurs because
# adding new mappings in the upper layer may delete one (hash collisions) 
#
# Entry: from __exc_data_access900 or __exc_instr_access900
#
# Registers:
#				%r2: fault bits
#				%r3: bad vaddr
#				%r0,%r1,%r4,%r5,%r6,%r7,%r8: free regs
#
#				%r1,%r2,%r3 registers are in SPRG's

routine_start __exc_access900,0
	rlwinm	%r0,%r3,4,28,31
	get_cpu_offset %r6
	slbmfev	%r0,%r0
	bittst	%r7,%r2,VM_FAULT_WIMG_ERR
	rlwimi	%r2,%r0,32-PPC64_SLB0_VSID_SHIFT,PPC64_SLB0_VSID_SHIFT,31
	bne-	666f

	#
	# Check for a msg_xfer area match first
	#
	rlwinm	%r7,%r3,0,0,3
	loadi	%r1,VM_MSG_XFER_START
	loadi	%r8,VM_MSG_XFER_START+0x10000000
	cmpw	%r1,%r7
	beq		660f
	cmpw	%r8,%r7
	beq		658f

	#
	# Now check if boundary off
	#
	lwz		%r1,RESERVED_BASE+LOWMAP_OFFSET(%r6)
	cmplw	cr0, %r3,%r1
	blt		cr0, 666f

	#
	# user address space
	#

	#
	# Now check the current pagetable
	#
	lwz		%r1,RESERVED_BASE+L1PAGETABLE_OFFSET(%r6)
	rlwinm	%r4,%r3,13,19,29 	# extract L1 page table index bits
	lwzx	%r1,%r1,%r4
	cmplwi	cr0, %r1,0
	beq-	cr0, 663f
	rlwinm	%r4,%r3,23,20,28 	# extract L2 page table index bits
	ldx		%r1,%r1,%r4
	loadi	%r4, 0xfff
	andc.	%r4,%r1,%r4			# see if page is valid
	beq-	663f

	# Test if write fault and write protection
	bittst	%r4,%r2,VM_FAULT_INSTR
	beq		1f
	# Executing guarded memory
	bittst	%r4,%r1,PPC_TLBLO_G
	bne-	666f	
1:
	andi.	%r4,%r1,3
	bc		12,2,666f
	cmpwi	1,%r4,1
	bc		4,6,2f
	# Write fault, R/O page ?
	bittst	%r4,%r2,VM_FAULT_WRITE
	beq		2f

	# check the SLB entry to see if we've done protection override 
	# for breakpoints
	bittst	%r4,%r0,PPC64_SLB0_KS

	beq		2f
	b		666f
  
#
# Check for a match with one of the special areas (msg, cpupage, syspage)
#
# Note that for message xfer, we hardcode the size of the area here. Change
# here if we change the definition in kernel/cpu_ppc.h
#

#
#
#

658:
	# In the xfer area...
	lis		%r4,xfer_pgdir@ha
	lis		%r6,xfer_diff@ha
	lwz		%r4,xfer_pgdir@l(%r4)

	cmpwi	cr0,%r4,0
	lwz		%r6,4+xfer_diff@l(%r6) # xfer_diff[1] has high diff
	beq-	cr0,666f

	# Looks like a valid msg xfer. 

	sub		%r8,%r3,%r6				# real_addr = vaddr - xfer_diff
	b		659f

660:
	# In the xfer area...
	lis		%r4,xfer_pgdir@ha
	lis		%r6,xfer_diff@ha
	lwz		%r4,xfer_pgdir@l(%r4)

	cmpwi	cr0,%r4,0
	lwz		%r6,xfer_diff@l(%r6) # xfer_diff[0] has low diff
	beq-	cr0,666f

	# Looks like a valid msg xfer. 

	sub		%r8,%r3,%r6				# real_addr = vaddr - xfer_diff

659:
	rlwinm	%r7,%r8,13,19,29 	# extract L1 page table index bits
	lwzx	%r7,%r7,%r4
	cmpwi	%r7,0
	beq-	666f

	rlwinm	%r6,%r8,23,20,28 	# extract L2 page table index bits
	ldx		%r1,%r7,%r6
	loadi	%r4, 0xfff
	andc.	%r4,%r1,%r4			# see if page is valid
	beq-	666f

	# %r7 has the pte that we want to pluck in; check perms first
	andi.	%r4,%r1,3
	beq		666f				# no_prot page
	cmpwi	%r4,2
	beq		6590f				# R/W page
	bittst	%r7,%r2,VM_FAULT_WRITE
	bne-	666f				# R/O page, write fault

6590: 

#
# move address to put mapping in for into %r3
#
	mr		%r3,%r8 			# original address is now gone!!!
	b		2f

#
# Check the other special mappings: syspage and cpupage
#

663:
	rlwinm	%r4,%r3,0,0,19
	bittst	%r1,%r2,VM_FAULT_WRITE
	bne-	666f
	
	loadi	%r1,VM_SYSPAGE_ADDR
	cmpw	%r1,%r4	
	bne		664f
	
	# This is a syspage read fault. Generate the PTE
	lis		%r1,_syspage_ptr@ha
	lwz		%r1,_syspage_ptr@l(%r1)
	rlwinm	%r1,%r1,0,0,19
	ori		%r1,%r1,PPC_TLBLO_PP_RO+PPC_TLBLO_M+PPC_TLBLO_C+PPC_TLBLO_R
	b		2f

664:
	loadi	%r1, VM_CPUPAGE_ADDR
	cmpw	%r1,%r4	
	bne		666f
	
	# This is a cpupage read fault. Generate the PTE
	lis		%r1,cpupageptr@ha
.ifdef VARIANT_smp
	GETCPU	%r8,2
	add		%r1,%r1,%r8
.endif
	lwz		%r1,cpupageptr@l(%r1)
	rlwinm	%r1,%r1,0,0,19
	ori		%r1,%r1,PPC_TLBLO_PP_RO+PPC_TLBLO_M+PPC_TLBLO_C+PPC_TLBLO_R
	b		2f


# got a valid page table entry
# %r1: pte (64-bit)
# %r2: flags + vsid
# %r3: vaddr
#

# We know we have a valid translation, just need to set up
# and pluck it in

2:

# Alright, do the refill
# r3: vaddr r2: vsid 
	rlwinm 	%r4,%r2,0,13,31
	rlwinm 	%r5,%r3,20,16,31

	xor 	%r4,%r4,%r5

# r4 now: primary hash

# %r1 has page table entry (lo ht entry as well)
	lis 	%r8,HashTable_base@ha
	rlwinm 	%r7,%r3,9,27,31 		# r7: vaddr page index >> 11
	rlwinm 	%r6,%r2,5,3,26			# r6: vsid << 5
	or 		%r6,%r6,%r7				# r6: avpn
	sldi	%r6,%r6,PPC64_TLBHI_AVPN_SHIFT # form PTE hi
	lwz 	%r7,HashTable_base@l(%r8)
	lis 	%r8,HashTable_mask@ha
	ori  	%r5,%r6,PPC64_TLBHI_V	# turn on valid bit
	
#
# r1: ht lo entry (64 bits)
# r2: vsid | flags
# r3: vaddr
# r4: hash
# r5: ht hi entry (64 bits)
#
	lwz 	%r6,HashTable_mask@l(%r8)
	andis. 	%r8,%r7,0xf700
	rlwinm 	%r0,%r4,7,6,13
	rlwinm 	%r4,%r4,7,14,24
	andis. 	%r7,%r7,0x8ff
	and 	%r6,%r6,%r0
	or 		%r7,%r7,%r6
	or 		%r8,%r8,%r7
	or 		%r4,%r8,%r4
	li		%r7,8
	mr 		%r8,%r4

.ifdef VARIANT_g
	li		%r6,0
.endif
# Alright: r4 has the hte base
4:
	mtctr 	%r7
5:
	ld		%r0,0(%r4)			
	bittst 	%r7,%r0,PPC64_TLBHI_V
	beq- 	10f
.ifdef VARIANT_g
	cmpd	%r0,%r5
	bne+	99f
	# duplicate entry present
	addi	%r6,%r6,1
99:	
.endif
	addi 	%r4,%r4,16
	bdnz+ 	5b

# None found; go to secondary hash function
2:
	addis 	%r7,0,HashTable_mask@ha
	ori 	%r5,%r5,PPC64_TLBHI_H
	lwz 	%r0,HashTable_mask@l(%r7)
	ori 	%r0,%r0,0xffff
	oris	%r0,%r0,0x3
	li		%r7,8
	xor 	%r4,%r8,%r0
	rlwinm 	%r4,%r4,0,0,24

# Try secondary slots; leave unrolled to maximize speed
	mtctr 	%r7
5:
	ld		%r0,0(%r4)
	bittst 	%r7,%r0,PPC64_TLBHI_V
	beq- 	10f
.ifdef VARIANT_g
	cmpd	%r0,%r5
	bne+	99f
	# duplicate entry present
	addi	%r6,%r6,1
99:	
.endif
	addi 	%r4,%r4,16
	bdnz+ 	5b

.ifdef VARIANT_g
	# it is ok for there to be some duplicate entries,
	# but if we have too many, time to kick into the kernel
	# debugger/dumper - we are in a fault loop
	cmplwi	%r6,9
	ble+	99f
#	mfsrr0	%r0
#	std		%r0,0(%r0)
#	mfsrr1	%r0
#	std		%r1,8(%r0)
	tw		31,%r1,%r1
99:	
.endif	

# Nothing free... use DEC to get a random slot, always use primary
	mfspr	%r6,PPC_SPR_DEC
	xori	%r5,%r5,PPC64_TLBHI_H
	rlwinm 	%r6,%r6,4,25,27		# Mask with 0x70
	add		%r4,%r6,%r8

# Check to make sure we're not overwriting a permanent entry.
# Startup put all of them at the end of the appropriate PTE group, so we can
# just back up until we find an entry that doesn't have the indicator bit on.
6:	
	lwz		%r6,4(%r4)
	bittst	%r0,%r6,1 << PPC64_TLBHI_SW_SHIFT
	beq+	10f
	addi	%r4,%r4,-0x10
	b		6b

10:
.ifdef VARIANT_smp
#	
# With SMP, we protect the hash table update with a spinlock.
#

	li	%r0,0

	lis	%r7,ht_slock@ha
	la	%r7,ht_slock@l(%r7)
12:
	lwarx	%r6,%r0,%r7	
	cmpwi	%r6,0
	bne-	12b
	stwcx.	%r4,%r0,%r7		# We know %r4 is not zero
	bne-	12b

	# Inval old entry
	stw		%r0,4(%r4)
	ptesync

	# Store new one
	std 	%r1,8(%r4)
	eieio
	std 	%r5,0(%r4)

	# all done; release spinlock
	stw		%r0,0(%r7)
	sync

.else

#
# Non-smp case is simpler. IRQ's are off, translation is off, so
# we are guaranteed that the hash table is not being used.
#
	std		%r1,8(%r4)
	std 	%r5,0(%r4)
	
.endif

	ptesync

#
# We did it...
# restore regs, exit from handler
#

20:
	get_cpu_offset %r2
	lwz		%r3,RESERVED_BASE+STACK_OFFSET+28(%r2)
	lwz		%r0,RESERVED_BASE+STACK_OFFSET+0(%r2)
	lwz		%r4,RESERVED_BASE+STACK_OFFSET+4(%r2)
	lwz		%r5,RESERVED_BASE+STACK_OFFSET+8(%r2)
	mtcr	%r3
	lwz		%r3,RESERVED_BASE+STACK_OFFSET+24(%r2)
	mfsprg	%r1,0
	lwz		%r6,RESERVED_BASE+STACK_OFFSET+12(%r2)
	mtctr	%r3
	lwz		%r7,RESERVED_BASE+STACK_OFFSET+16(%r2)
	mfsprg	%r3,2
	lwz		%r8,RESERVED_BASE+STACK_OFFSET+20(%r2)
	lwz		%r9,RESERVED_BASE+STACK_OFFSET+32(%r2)
	mfsprg	%r2,1
	rfid
#
# 666: number of the beast :-)
#
#	looks like real fault, go to fault handler
#
#	%r2 is flags
#	%r3 is vaddr
#

666:
	get_cpu_offset %r1
	lwz 	%r0,RESERVED_BASE+STACK_OFFSET+0(%r1)
	lwz		%r4,RESERVED_BASE+STACK_OFFSET+4(%r1)
	# Stash the vaddr and flags
	andis.	%r2,%r2,0xff00
	lwz		%r5,RESERVED_BASE+STACK_OFFSET+8(%r1)
	stw 	%r2,RESERVED_BASE+STACK_OFFSET+4(%r1)
	lwz		%r6,RESERVED_BASE+STACK_OFFSET+12(%r1)
	mfsprg	%r2,1
	lwz		%r7,RESERVED_BASE+STACK_OFFSET+16(%r1)
	stw 	%r3,RESERVED_BASE+STACK_OFFSET+0(%r1)
	lwz		%r8,RESERVED_BASE+STACK_OFFSET+20(%r1)
	lwz		%r3,RESERVED_BASE+STACK_OFFSET+24(%r1)
	mtctr	%r3
	lwz		%r3,RESERVED_BASE+STACK_OFFSET+28(%r1)
	mfsprg	%r1,0
	mtcr	%r3
	mfsprg	%r3,2

#
# We've restored our regs to what they were at the fault;
# Do a "normal" kernel entry now.
# Note that we've stashed away both the bad vaddr and the 
# fault flags in our reserved area.
#	

__exc_access_900_exit:
	ENTERKERNEL EK_EXC, SRR0, SRR1
	bla		PPC_KERENTRY_COMMON
	get_cpu_offset %r7
	lwz 	%r4,RESERVED_BASE+STACK_OFFSET+0(%r7)
	lwz 	%r3,RESERVED_BASE+STACK_OFFSET+4(%r7)
	
	# The initial R5 value comes from the "mmu_on" code that 
	# PPC_KERENTRY_COMMON has tacked on the end of it.
	or 		%r5,%r5,%r3
	
	b		__exc_access
routine_end __exc_access900


# Careful: Only 0x80 bytes of storage available for this exception block
EXC_COPY_CODE_START __exc_data_segment900
	mtsprg	0,%r1
	mtsprg	1,%r2
	get_cpu_offset %r1

	mfctr	%r2
	stw 	%r2,RESERVED_BASE+STACK_OFFSET+8(%r1)

	loada	%r2,__exc_segment900
	mtctr	%r2

	mfdar	%r2
	stw 	%r2,RESERVED_BASE+STACK_OFFSET+0(%r1)

	# We can't really tell whether it was a read or write that
	# caused the problem without decoding the instruction and
	# so can't set VM_FAULT_WRITE properly, but it doesn't really
	# matter since we're always going to SIGSEGV the process
	# if we come in here.
	li		%r1,0
	stw 	%r2,RESERVED_BASE+STACK_OFFSET+4(%r1)

	bctr
EXC_COPY_CODE_END


# Careful: Only 0x80 bytes of storage available for this exception block
EXC_COPY_CODE_START __exc_instr_segment900
	mtsprg	0,%r1
	mtsprg	1,%r2
	get_cpu_offset %r1

	mfctr	%r2
	stw 	%r2,RESERVED_BASE+STACK_OFFSET+8(%r1)

	loada	%r2,__exc_segment900
	mtctr	%r2

	mfsrr0	%r2
	stw 	%r2,RESERVED_BASE+STACK_OFFSET+0(%r1)

	# say it was an instruction access
	loadi	%r2,VM_FAULT_INSTR	
	stw 	%r2,RESERVED_BASE+STACK_OFFSET+4(%r1)

	bctr
EXC_COPY_CODE_END


routine_start __exc_segment900,0
	mfsprg	%r1,0
	mfsprg	%r2,0
	ENTERKERNEL EK_EXC, SRR0, SRR1
	get_cpu_offset %r3

	# restore original CTR register value for saving by PPC_KERENTRY_COMMON
	lwz		%r4,RESERVED_BASE+STACK_OFFSET+8(%r3)
	mtctr	%r4	

	bla		PPC_KERENTRY_COMMON

	lwz 	%r4,RESERVED_BASE+STACK_OFFSET+0(%r3)
	lwz 	%r3,RESERVED_BASE+STACK_OFFSET+4(%r3)
	
	# The initial R5 value comes from the "mmu_on" code that 
	# PPC_KERENTRY_COMMON has tacked on the end of it.
	or 		%r5,%r5,%r3
	
	b		__exc_access
routine_end __exc_segment900
