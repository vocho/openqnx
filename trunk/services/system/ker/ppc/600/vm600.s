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
# This file contains 603/604/700 series VM related stuff in the kernel. 
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


.section .text_kernel, "ax"

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

	.set	SEGMENT_REG,0x60000000

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
# %r5: nx_state
#
# IRQ's should always be off.

routine_start set_l1pagetable, 1
	rlwinm	%r7,%r4,0,12,31
	get_cpu_offset %r6
	# Save L1 pagetable
	stw		%r3,RESERVED_BASE+L1PAGETABLE_OFFSET(%r6)
	loadi	%r3,SEGMENT_REG
	or		%r3,%r3,%r4
	stw		%r7,RESERVED_BASE+VSID_OFFSET(%r6)

	#load segment registers
	addis	%r7,%r3,0x40
	addis	%r6,%r3,0x50
	addis	%r8,%r3,0x60
	addis	%r4,%r3,0x70
	rlwimi	%r7,%r5,24,3,3
	rlwimi	%r6,%r5,23,3,3
	rlwimi	%r8,%r5,22,3,3
	rlwimi	%r4,%r5,21,3,3
	# But make sure we sync so all old accesses are completed in right context
	sync
	mtsr	4, %r7
	mtsr	5, %r6
	mtsr	6, %r8
	mtsr	7, %r4

	addis	%r7,%r3,0x80
	addis	%r6,%r3,0x90
	addis	%r8,%r3,0xa0
	addis	%r4,%r3,0xb0
	rlwimi	%r7,%r5,20,3,3
	rlwimi	%r6,%r5,19,3,3
	rlwimi	%r8,%r5,18,3,3
	rlwimi	%r4,%r5,17,3,3
	mtsr	8, %r7
	mtsr	9, %r6
	mtsr	10, %r8
	mtsr	11, %r4

	addis	%r7,%r3,0xc0
	addis	%r6,%r3,0xd0
	addis	%r8,%r3,0xe0
	addis	%r4,%r3,0xf0
	rlwimi	%r7,%r5,16,3,3
	rlwimi	%r6,%r5,15,3,3
	rlwimi	%r8,%r5,14,3,3
	rlwimi	%r4,%r5,13,3,3
	mtsr	12, %r7
	mtsr	13, %r6
	mtsr	14, %r8
	mtsr	15, %r4

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
	lwz			%r5,RESERVED_BASE+VSID_OFFSET(%r7)
	
# load address boundary
	loadi		%r3,VM_KERN_LOW_SIZE
	stw			%r3,RESERVED_BASE+LOWMAP_OFFSET(%r7)
	
# change BAT registers
	mfspr	%r3,PPC_SPR_IBAT0L	
	andi.	%r4,%r3,0x02			# Test if already r/w
	bne+	1f						# Yes, no need to change BATs
	ori		%r3,%r3,0x2				#w/r
	mtspr	PPC_SPR_IBAT0L,%r3
	mtspr	PPC_SPR_DBAT0L,%r3

	# Set segment register for 768MB-1G to asid 0; no execute permissions
1:	
#@@	loadi	%r3,0x70000000
#@@	mtsr	3, %r3

# turnon mmu
	mfmsr	%r3							
	ori		%r3,%r3,PPC_MSR_IR + PPC_MSR_DR + PPC_MSR_RI
	mtmsr	%r3
	isync
	# Same context, no need to sync, because we previously had 
	# translation disabled, which implies guarded storage access, so 
	# we cannot have speculatively started loading across the above isync
EXC_COPY_CODE_END


EXC_COPY_CODE_START mmu_off
#
# Note: we don't need to turn translation off if we're not changing the BATs
#
	# return to user space	
	# check low address boundry
	get_cpu_offset %r5
	loadi	%r3,0x10000000
	lwz		%r4,LOW_MEM_BOUNDRY(%r31)
	cmplw	%r4,%r3
	stw		%r4,RESERVED_BASE+LOWMAP_OFFSET(%r5)
	bgt		2f
	lwz		%r5,TFLAGS(%r31)
	bittst	%r3,%r5,_NTO_TF_IOPRIV
	beq+	2f

#
# This is a thread with IOPRIV. Need to disable BATs		
#
# turn off mmu
	mfmsr	%r3
	rlwinm	%r3,%r3,0,28,25				
	mtmsr	%r3
	isync
	sync

	# change BAT registers	
	mfspr	%r3,PPC_SPR_IBAT0L	
	rlwinm	%r3,%r3,0,0,29			# clear bit30,31, pp				
	mtspr	PPC_SPR_IBAT0L,%r3
	mtspr	PPC_SPR_DBAT0L,%r3
2:
	# disable access to low vaddrs
	loadi	%r3,0x60000000
	mtsr	1, %r3
	mtsr	2, %r3
	mtsr	3, %r3
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

EXC_COPY_CODE_START __exc_data_access700
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
	loada	%r2,__exc_access700
	mtctr	%r2
	stw		%r7,RESERVED_BASE+STACK_OFFSET+16(%r1)
	lwz		%r2,RESERVED_BASE+VSID_OFFSET(%r1)
	mfdsisr	%r5
	stw		%r8,RESERVED_BASE+STACK_OFFSET+20(%r1)
	mfspr	%r3,PPC_SPR_DAR 	# get address responsible
	bittst	%r8,%r5,0x02000000
	stw		%r6,RESERVED_BASE+STACK_OFFSET+24(%r1)
	beq		1f
	bitset	%r2,%r2,VM_FAULT_WRITE		# indicate a store instruction
1:
	bittst	%r8,%r5,0x0c000000
	beq		1f
	bitset	%r2,%r2,VM_FAULT_WIMG_ERR	# indicate a WIMG error
1:
	bctr
EXC_COPY_CODE_END
	

EXC_COPY_CODE_START __exc_instr_access700
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
	loada	%r2,__exc_access700
	mtctr	%r2
	stw		%r7,RESERVED_BASE+STACK_OFFSET+16(%r1)
	lwz		%r2,RESERVED_BASE+VSID_OFFSET(%r1)
	mfspr	%r5,SRR1
	stw		%r8,RESERVED_BASE+STACK_OFFSET+20(%r1)
	mfspr	%r3,SRR0
	bitset	%r2,%r2,VM_FAULT_INSTR	# say it was an instruction access
	bittst	%r8,%r5,0x18000000
	beq		1f
	bitset	%r2,%r2,VM_FAULT_WIMG_ERR	# indicate a WIMG error
1:	
	stw		%r6,RESERVED_BASE+STACK_OFFSET+24(%r1)
	bctr
EXC_COPY_CODE_END
	
#
# On a fault, this checks to see if we can simply
# pluck a new (valid) mapping in the hash table. This occurs because
# adding new mappings in the upper layer may delete one (hash collisions) 
#
# Entry: from __exc_data_access700 or __exc_instr_access700
#
# Registers:
#				%r2: asid + fault bits
#				%r3: bad vaddr
#				%r0,%r1,%r4,%r5,%r6,%r7,%r8: free regs
#
#				%r1,%r2,%r3 registers are in SPRG's

routine_start __exc_access700,0

	get_cpu_offset %r6
	bittst %r7,%r2,VM_FAULT_WIMG_ERR
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
	rlwinm	%r4,%r3,12,20,29 	# extract L1 page table index bits
	lwzx	%r1,%r1,%r4
	cmplwi	cr0, %r1,0
	beq-	cr0, 663f
	rlwinm	%r4,%r3,22,20,29 	# extract L2 page table index bits
	lwzx	%r1,%r1,%r4
	# have to compare to all paddr tlb bits for XAEN + PP bits so paddr 0 mapping is included
	loadi	%r4, 0xfffffe07
	and	%r4,%r4,%r1		# see if page is valid
	cmplwi	cr0, %r4,0
	beq-	cr0, 663f

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
	andis.	%r4,%r2,16384
	bc		4,2,26f
 	b		2f
26:
	# check the segment register to see if we've done protection override for breakpoints
	mfsrin		%r4,%r3
	bittst		%r4,%r4,0x40000000
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
	mfsr	%r7,2
	lis		%r4,xfer_pgdir@ha
	rlwinm	%r0,%r7,0,8,31
	lis		%r6,xfer_diff@ha
	lwz		%r4,xfer_pgdir@l(%r4)
	cmpwi	cr1,%r0,0		# Check if ASID 0 in seg. reg 2

	cmpwi	cr0,%r4,0
	lwz		%r6,4+xfer_diff@l(%r6) # xfer_diff[1] has high diff
	beq-	cr0,666f
	beq		cr1,666f

	# Looks like a valid msg xfer. 

	sub		%r8,%r3,%r6				# real_addr = vaddr - xfer_diff
	b	659f

660:
	# In the xfer area...
	mfsr	%r7,1
	lis		%r4,xfer_pgdir@ha
	rlwinm	%r0,%r7,0,8,31
	lis		%r6,xfer_diff@ha
	lwz		%r4,xfer_pgdir@l(%r4)
	cmpwi	cr1,%r0,0		# Check if ASID 0 in seg. reg 1

	cmpwi	cr0,%r4,0
	lwz		%r6,xfer_diff@l(%r6) # xfer_diff[0] has low diff
	beq-	cr0,666f
	beq-		cr1,666f

	# Looks like a valid msg xfer. 

	sub		%r8,%r3,%r6				# real_addr = vaddr - xfer_diff

659:
	rlwinm	%r7,%r8,12,20,29 	# extract L1 page table index bits
	lwzx	%r7,%r7,%r4
	cmpwi	%r7,0
	beq-	666f

	rlwinm	%r6,%r8,22,20,29 	# extract L2 page table index bits
	lwzx	%r7,%r7,%r6
	# have to compare to all paddr tlb bits for XAEN
	loadi	%r4, 0xfffffe04
	and	%r4,%r4,%r7		# see if page is valid
	cmpwi	%r4,0
	beq-	666f

	# %r7 has the pte that we want to pluck in; check perms first
	andi.	%r4,%r7,3
	beq		666f				# no_prot page
	cmpwi	%r4,2
	beq		6590f				# R/W page
	bittst	%r1,%r2,VM_FAULT_WRITE
	bne-	666f				# R/O page, write fault

6590: 

#
# move pte to %r1, address to put mapping in for into %r3
# and replace asid in r2 with the right one...
#
	mr	%r1,%r7
	rlwinm	%r2,%r2,0,0,7
	or	%r2,%r2,%r0
	mr	%r3,%r8 # original address is now gone!!!
	b	2f

#
# Check the other special mappings: syspage and cpupage
#

663:
	rlwinm	%r4,%r3,0,0,19
	bittst	%r1,%r2,VM_FAULT_WRITE
	bne-	666f
	
	loadi	%r1, VM_SYSPAGE_ADDR
	cmpw	%r1,%r4	
	bne		664f
	
	# This is a syspage read fault. Generate the PTE
	lis		%r1,_syspage_ptr@ha
	lwz		%r1,_syspage_ptr@l(%r1)
	rlwinm	%r1,%r1,0,0,19
	ori		%r1,%r1,PPC_TLBLO_PP_RO+PPC_TLBLO_M+PPC_TLBLO_C+PPC_TLBLO_R
	b		2f

664:
.ifdef VARIANT_smp
	# Don't handle any faults for CPUPAGE
	b		666f
.else
	loadi	%r1, VM_CPUPAGE_ADDR
	cmpw	%r1,%r4	
	bne		666f
	
	# This is a cpupage read fault. Generate the PTE
	lis		%r1,_cpupage_ptr@ha
	lwz		%r1,_cpupage_ptr@l(%r1)
	rlwinm	%r1,%r1,0,0,19
	ori		%r1,%r1,PPC_TLBLO_PP_RO+PPC_TLBLO_M+PPC_TLBLO_C+PPC_TLBLO_R
	b		2f
.endif


# got a valid page table entry
# %r1: pte
# %r2: flags + asid
# %r3: vaddr
#

# We know we have a valid translation, just need to set up
# and pluck it in

2:

# Alright, do the refill
# r3: vaddr r2: asid 
# AND in top 4 bits of vaddr into ASID
	rlwinm	%r5,%r3,24,8,11
	or		%r2,%r5,%r2
	rlwinm 	%r4,%r2,0,13,31
	rlwinm 	%r5,%r3,20,16,31
	xor 	%r4,%r4,%r5

# r4 now: primary hash

# %r1 has page table entry (lo ht entry as well)
	lis 	%r8,HashTable_base@ha
	rlwinm 	%r7,%r3,10,26,31
	rlwinm 	%r6,%r2,7,1,24
	or 		%r6,%r6,%r7
	lwz 	%r7,HashTable_base@l(%r8)
	lis 	%r8,HashTable_mask@ha
	oris 	%r5,%r6,0x8000
	
# and r7 now has hi ht entry

#
# r1: ht lo entry
# r2: asid | flags
# r3: vaddr
# r4: hash
# r5: ht hi entry
#
	lwz 	%r6,HashTable_mask@l(%r8)
	andis. 	%r8,%r7,0xf700
	rlwinm 	%r0,%r4,6,7,15
	rlwinm 	%r4,%r4,6,16,25
	andis. 	%r7,%r7,0x8ff
	and 	%r6,%r6,%r0
	or 		%r7,%r7,%r6
	or 		%r8,%r8,%r7
	or 		%r4,%r8,%r4
	li		%r7,8
	mr 		%r8,%r4

# Alright: r4 has the hte base
4:
	mtctr 	%r7
5:
	lwz		%r0,0(%r4)
	bittst 	%r0,%r0,0x80000000
	beq- 	10f
	addi 	%r4,%r4,8
	bdnz+ 	5b

# None found; go to secondary hash function
2:
	addis 	%r7,0,HashTable_mask@ha
	ori 	%r5,%r5,0x0040
	lwz 	%r0,HashTable_mask@l(%r7)
	ori 	%r0,%r0,65535
	li		%r7,8
	xor 	%r4,%r8,%r0
	rlwinm 	%r4,%r4,0,0,25

# Try secondary slots; leave unrolled to maximize speed
	mtctr 	%r7
5:
	lwz		%r0,0(%r4)
	bittst 	%r0,%r0,0x80000000
	beq- 	10f
	addi 	%r4,%r4,8
	bdnz+ 	5b

# No free slots... steal one from the primary slots, use DEC to get
# a random slot.
6:
	mfspr	%r6,PPC_SPR_DEC
	rlwinm 	%r6,%r6,0,26,28		# Mask with 0x38
	add		%r4,%r6,%r8
	bitclr	%r5,%r5,0x0040

10:
.ifdef VARIANT_smp
#	
# With SMP, we protect the hash table update with a spinlock.
#

	li		%r0,0

	lis		%r7,ht_slock@ha
	la		%r7,ht_slock@l(%r7)
12:
	lwarx	%r6,%r0,%r7	
	cmpwi	%r6,0
	bne-	12b
	stwcx.	%r4,%r0,%r7		# We know %r4 is not zero
	bne-	12b

	# Inval old entry
	stw		%r0,0(%r4)
	sync

	# Store new one
	stw 	%r1,4(%r4)
	eieio
	stw 	%r5,0(%r4)
	sync

	# all done; release spinlock
	stw		%r0,0(%r7)
	sync

	
.else

#
# Non-smp case is simpler. IRQ's are off, translation is off, so
# we are guaranteed that the hash table is not being used.
#
	stw		%r1,4(%r4)
	stw		%r5,0(%r4)
	
.endif


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
	mfsprg	%r2,1
	rfi
#
# 666: number of the beast :-)
#
#	looks like real fault, go to fault handler
#
#	%r2 is asid + flags
#	%r3 is vaddr
#

666:
	get_cpu_offset %r1
	lwz 	%r0,RESERVED_BASE+STACK_OFFSET+0(%r1)
	lwz		%r4,RESERVED_BASE+STACK_OFFSET+4(%r1)
	# Stash the vaddr and flags
	andis. %r2,%r2,0xff00
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

__exc_access_700_exit:
	ENTERKERNEL EK_EXC, SRR0, SRR1
	bla		PPC_KERENTRY_COMMON
	get_cpu_offset %r7
	lwz 	%r4,RESERVED_BASE+STACK_OFFSET+0(%r7)
	lwz 	%r3,RESERVED_BASE+STACK_OFFSET+4(%r7)
	
	# The initial R5 value comes from the "mmuon" code that 
	# PPC_KERENTRY_COMMON has tacked on the end of it.
	or 		%r5,%r5,%r3
	
	b		__exc_access
routine_end __exc_access700


#
#
# software TLB routines for the 603
#
#

	
#	add_tlb600(hi, lo, exc_flag, addr)
routine_start add_tlb603, 1
	mfmsr	%r7
	lwz		%r8,ppc_ienable_bits@sdarel(%r13)
	andc	%r8,%r7,%r8
	rlwinm	%r8,%r8,0,28,25					# turn off translation bits (mmu)
	mtmsr	%r8								# disable int and mmu

	cmpwi	%r5,0
	beq		1f	
	mtspr	PPC603_SPR_ICMP,%r3				
	mtspr	PPC603_SPR_RPA,%r4		
	tlbli	%r6
1:	
	mtspr	PPC603_SPR_DCMP,%r3				
	mtspr	PPC603_SPR_RPA,%r4			
	tlbld	%r6
2:	
	mtmsr	%r7
	sync
	isync

	blr
routine_end add_tlb603

# tlb miss exc handlers
#
# Note that on the 603e, PPC_MSR_TGPR gets set on any TLB miss exception,
# Swapping out the first four GPR's with temporary ones, so we don't
# have to bother saving any registers in the SPGR's.
#
.macro tlb_miss addr, write_inst
	mfcr	%r0
	mfspr	%r1,&addr			# get address responsible

	lwz		%r2,RESERVED_BASE+LOWMAP_OFFSET(0)
	cmplw	%r1,%r2
	blt		2f
	#user address space
	lwz		%r2,RESERVED_BASE+L1PAGETABLE_OFFSET(0)
	rlwinm	%r3,%r1,12,20,29 	# extract L1 page table index bits
	lwzx	%r2,%r2,%r3
	cmplwi	%r2,0
	beq-	1f
	rlwinm	%r3,%r1,22,20,29 	# extract L2 page table index bits
	lwzx	%r2,%r2,%r3
	cmplwi	%r2,0
	beq-	1f
3:	
	# got a valid page table entry
	mtspr	PPC603_SPR_RPA, %r2
	&write_inst	%r1	

#	ret successfully
	mtcr	%r0
	rfi
2:
	# system area one-to-one mapping
	rlwinm	%r2,%r1,0,0,19
	ori		%r2,%r2,PPC_TLBLO_PP_RW+PPC_TLBLO_M+PPC_TLBLO_C+PPC_TLBLO_R	#r/w
	b	3b
1:
	mtcr	%r0
.endm

EXC_COPY_CODE_START __exc_dtlbr_603
#
# Data TLB miss (read)
#

	tlb_miss	PPC603_SPR_DMISS, tlbld
	mfspr	%r1,PPC603_SPR_DMISS		# get address responsible
	mtspr	PPC_SPR_DAR,%r1
	ori		%r3,%r1,0
	mfdsisr %r1
	rlwinm	%r1,%r1,0,7,5
	mtdsisr %r1
	mfctr	%r1
	loada	%r2,__exc_access603
	mtctr	%r2
	loadi	%r2,0
	bctr
EXC_COPY_CODE_END


EXC_COPY_CODE_START __exc_dtlbw_603
#
# Data TLB miss (write)
#

	tlb_miss	PPC603_SPR_DMISS, tlbld
	mfspr	%r1,PPC603_SPR_DMISS		# get address responsible
	mtspr	PPC_SPR_DAR,%r1
	ori		%r3,%r1,0
	mfdsisr %r1
	bitset	%r1,%r1,PPC_DSISR_STORE
	mtdsisr %r1
	mfctr	%r1
	loada	%r2,__exc_access603
	mtctr	%r2
	loadi	%r2,VM_FAULT_WRITE
	bctr
EXC_COPY_CODE_END

EXC_COPY_CODE_START __exc_itlb_603
#
# Instruction TLB miss
#

	tlb_miss	PPC603_SPR_IMISS, tlbli
	mfctr	%r1
	loada	%r2,__exc_access603
	mtctr	%r2
	loadi	%r2,VM_FAULT_INSTR
	mfspr	%r3,PPC603_SPR_IMISS
	bctr
EXC_COPY_CODE_END

#
# exception that cannot be resolved from the pagetable. We 
# handle the syspage, cpupage and msg pass faults here; if not, then
# we let it go to __exc_access
#
# On entry: 
#			%r2: flags (+ asid?)
#			%r3: bad vaddr
#			%r1: original CTR
#
#
__exc_access603:
	# restore original CTR value
	mtctr	%r1
	# Save some regs
	mfcr	%r0
	stw		%r0,RESERVED_BASE+STACK_OFFSET+8(0)
	stw		%r2,RESERVED_BASE+STACK_OFFSET+4(0)
	stw		%r3,RESERVED_BASE+STACK_OFFSET+0(0)

#
# Allright. Check if we can do something with vaddr
#

#
# First check for a msg pass
# Note: this relies on the msg. pass area being from 0x10000000 to 0x30000000
#
	rlwinm 	%r0,%r3,0,0,3
	loadi	%r1,VM_MSG_XFER_START
	cmpw	%r1,%r0
	beq		690f		
	loadi	%r1,VM_MSG_XFER_START+0x10000000
	cmpw	%r1,%r0
	bne	710f		

691:
	# In the xfer area, 2nd slot...
	mfsr	%r0,2
	rlwinm	%r0,%r0,0,8,31
	cmpwi	%r0,0		# Check if ASID 0 in seg. reg 1
	beq-	766f
	lis	%r2,xfer_diff@ha
	lwz	%r2,4+xfer_diff@l(%r2) # xfer_diff[1] has high diff

	lis	%r1,xfer_pgdir@ha
	lwz	%r1,xfer_pgdir@l(%r1)
	cmpwi	%r1,0
	beq-	766f

	# Looks like a valid msg xfer. 

	sub	%r3,%r3,%r2				# real_addr = vaddr - xfer_diff
	b	699f

690:
	# In the xfer area...
	mfsr	%r0,1
	rlwinm	%r0,%r0,0,8,31
	cmpwi	cr1,%r0,0		# Check if ASID 0 in seg. reg 1
	beq-	cr1,766f
	lis	%r2,xfer_diff@ha
	lwz	%r2,xfer_diff@l(%r2) # xfer_diff[0] has low diff

	lis	%r1,xfer_pgdir@ha
	lwz	%r1,xfer_pgdir@l(%r1)
	cmpwi	%r1,0
	beq-	766f

	# Looks like a valid msg xfer. 

	sub	%r3,%r3,%r2				# real_addr = vaddr - xfer_diff

699:
	rlwinm	%r2,%r3,12,20,29		# L1 index bits
	lwzx	%r2,%r2,%r1
	cmpwi	%r2,0
	beq-	766f
	rlwinm	%r1,%r3,22,20,29		# L2 index bits
	lwzx	%r2,%r2,%r1

	rlwinm	%r1,%r2,0,0,19		# see if page is valid
	cmpwi	%r1,0
	beq-	766f
			
	# %r2 has the tlb that we want to pluck in; check perms first
	lwz		%r3,RESERVED_BASE+STACK_OFFSET+0(0)
	andi.	%r0,%r2,3
	beq		766f				# no_prot page
	lwz		%r1,RESERVED_BASE+STACK_OFFSET+4(0)
	cmpwi	%r0,2
	beq		701f				# R/W page
	bittst	%r1,%r1,VM_FAULT_WRITE
	bne-	766f				# R/O page, write fault

701:
	# %r2 has our hte; just pluck it in, and we're out.
	mtspr	PPC603_SPR_RPA,%r2
	# Remove old mapping (note ordering: inval is before new entry is written)
	tlbld	%r3
	rlwinm	%r3,%r3,0,0,19
	b		720f

710:
	bittst	%r1,%r2,VM_FAULT_WRITE
	bne-	766f
	rlwinm	%r0,%r3,0,0,19
	
	loadi	%r1, VM_SYSPAGE_ADDR
	cmpw	%r1,%r0
	bne		712f

	# Syspage read fault. Generate the TLB entry	
	lis		%r1,_syspage_ptr@ha
	lwz		%r1,_syspage_ptr@l(%r1)
	rlwinm	%r1,%r1,0,0,19
	ori		%r1,%r1,PPC_TLBLO_PP_RO+PPC_TLBLO_M+PPC_TLBLO_C+PPC_TLBLO_R
	mtspr	PPC603_SPR_RPA, %r1
	bittst	%r1,%r2,VM_FAULT_INSTR
	bne 800f	
	tlbld	%r3
	b		720f
800:	
	tlbli	%r3
	b		720f

712:
	bittst	%r1,%r2,VM_FAULT_INSTR
	bne-	766f
	loadi	%r1,VM_CPUPAGE_ADDR
	cmpw	%r1,%r0
	bne-	766f
	
	# This is a cpupage read fault. Generate the TLB
	lis		%r1,_cpupage_ptr@ha
	lwz		%r1,_cpupage_ptr@l(%r1)
	rlwinm	%r1,%r1,0,0,19
	ori		%r1,%r1,PPC_TLBLO_PP_RO+PPC_TLBLO_M+PPC_TLBLO_C+PPC_TLBLO_R
	mtspr	PPC603_SPR_RPA, %r1
	tlbld	%r3
	b		720f

	# We have successfully written the TLB entry. Restore and exit.
720:
	lwz		%r0,RESERVED_BASE+STACK_OFFSET+8(0)
	mtcr	%r0
	rfi

#
# Looks like a real fault. Restore regs and go to generic routine
#

766:
	lwz		%r0,RESERVED_BASE+STACK_OFFSET+8(0)
	mtcr	%r0
	mfmsr	%r0					# switch back to gpr
	rlwinm	%r0,%r0,0,15,13				
	mtmsr	%r0
	# We've saved the faulting address and flags in our save area
	b		__exc_access_700_exit

#
# TLB reload routines for the 755. 
#

.macro tlb_miss_755 addr, write_inst
	mfcr	%r3
	mfspr	%r1,&addr			# get address responsible

	lwz		%r2,RESERVED_BASE+LOWMAP_OFFSET(0)
	cmplw	%r1,%r2
	blt		2f
	#user address space
	lwz		%r2,RESERVED_BASE+L1PAGETABLE_OFFSET(0)
	rlwinm	%r1,%r1,12,20,29 	# extract L1 page table index bits
	lwzx	%r2,%r2,%r1
	cmplwi	%r2,0
	beq-	1f
	mfspr	%r1,&addr			# get address responsible (again)
	rlwinm	%r1,%r1,22,20,29 	# extract L2 page table index bits
	lwzx	%r2,%r2,%r1
	cmplwi	%r2,0
	beq-	1f
3:	
	# got a valid page table entry
	mtspr	PPC603_SPR_RPA, %r2
	mfspr	%r1,&addr			# get address responsible (again)
	&write_inst	%r1	

#	ret successfully
	mtcr	%r3
	restore_regs_r123
	rfi
2:
	# system area one-to-one mapping
	rlwinm	%r2,%r1,0,0,19
	ori		%r2,%r2,PPC_TLBLO_PP_RW+PPC_TLBLO_M+PPC_TLBLO_C+PPC_TLBLO_R	#r/w
	b		3b
1:
	mtcr	%r3
.endm

EXC_COPY_CODE_START __exc_dtlbr_755
#
# Data TLB miss (read)
#

	save_regs_r123
	tlb_miss_755 PPC603_SPR_DMISS, tlbld
	mfspr	%r1,PPC603_SPR_DMISS		# get address responsible
	mtspr	PPC_SPR_DAR,%r1
	ori		%r3,%r1,0
	mfdsisr %r1
	rlwinm	%r1,%r1,0,7,5
	mtdsisr %r1
	mfctr	%r1
	loada	%r2,__exc_access755
	mtctr	%r2
	loadi	%r2,0
	bctr	
EXC_COPY_CODE_END

EXC_COPY_CODE_START __exc_dtlbw_755
#
# Data TLB miss (write)
#

	save_regs_r123
	tlb_miss_755 PPC603_SPR_DMISS, tlbld
	mfspr	%r1,PPC603_SPR_DMISS		# get address responsible
	mtspr	PPC_SPR_DAR,%r1
	ori		%r3,%r1,0
	mfdsisr %r1
	bitset	%r1,%r1,PPC_DSISR_STORE
	mtdsisr %r1
	mfctr	%r1
	loada	%r2,__exc_access755
	mtctr	%r2
	loadi	%r2,VM_FAULT_WRITE
	bctr
EXC_COPY_CODE_END

EXC_COPY_CODE_START __exc_itlb_755
#
# Instruction TLB miss
#

	save_regs_r123
	tlb_miss_755 PPC603_SPR_IMISS, tlbli
	mfctr	%r1
	loada	%r2,__exc_access755
	mtctr	%r2
	loadi	%r2,VM_FAULT_INSTR
	mfspr	%r3,PPC603_SPR_IMISS
	bctr
EXC_COPY_CODE_END

#
# I think this can be combined with the 603 in some way, but for now just
# to test, let it be this way.
# 
__exc_access755:
	# restore original CTR value
	mtctr	%r1
	# Save some regs
	mfcr	%r1
	stw		%r0,RESERVED_BASE+STACK_OFFSET+8(0)
	stw		%r2,RESERVED_BASE+STACK_OFFSET+4(0)
	stw		%r3,RESERVED_BASE+STACK_OFFSET+0(0)
	stw		%r1,RESERVED_BASE+STACK_OFFSET+12(0)

#
# Alright. Check if we can do something with vaddr
#

#
# First check for a msg pass
# Note: this relies on the msg. pass area being from 0x38000000 to 0x40000000
#
	rlwinm 	%r0,%r3,0,0,4
	loadi	%r1,VM_MSG_XFER_START
	cmpw	%r1,%r0
	bne		710f		
	mfsr	%r0,3
	lis		%r1,xfer_pgdir@ha
	rlwinm	%r0,%r0,0,8,31
	lwz		%r1,xfer_pgdir@l(%r1)
	cmpwi	%r0,0
	bne-	766f
	cmpwi	%r1,0
	beq-	766f

	# xfer looks valid. Save faulting address in save area (cannot use r4-r31)
	lis		%r2,xfer_diff@ha
	lwz		%r2,xfer_diff@l(%r2)
	sub		%r3,%r3,%r2
	rlwinm	%r2,%r3,12,20,29		# L1 index bits
	lwzx	%r2,%r2,%r1
	cmpwi	%r2,0
	beq-	766f
	rlwinm	%r1,%r3,22,20,29		# L2 index bits
	lwzx	%r2,%r2,%r1

	rlwinm	%r1,%r2,0,0,19		# see if page is valid
	cmpwi	%r1,0
	beq-	766f
			
	# %r2 has the tlb that we want to pluck in; check perms first
	lwz		%r3,RESERVED_BASE+STACK_OFFSET+0(0)
	andi.	%r0,%r2,3
	beq		766f				# no_prot page
	lwz		%r1,4(%r1)
	cmpwi	%r0,2
	beq		701f				# R/W page
	bittst	%r1,%r1,VM_FAULT_WRITE
	bne-	766f				# R/O page, write fault

701:
	# %r2 has our hte; just pluck it in, and we're out.
	mtspr	PPC603_SPR_RPA,%r2
	lis		%r1,xfer_rotor@ha
	lwz		%r2,xfer_rotor@l(%r1)
	addi	%r2,%r2,1
	rlwinm	%r2,%r2,0,31,31
	stw		%r2,xfer_rotor@l(%r1)
	lis		%r1,xfer_lastaddr@ha
	rlwinm	%r2,%r2,2,0,31
	la		%r1,xfer_lastaddr@l(%r1)
	add		%r1,%r2,%r1
	lwz		%r2,0(%r1)
	# Remove old mapping (note ordering: inval is before new entry is written)
	tlbie	%r2
	sync
	tlbld	%r3
	rlwinm	%r3,%r3,0,0,19
	stw		%r3,0(%r1)
	b		720f

710:
	bittst	%r1,%r2,VM_FAULT_WRITE
	bne-	766f
	rlwinm	%r0,%r3,0,0,19
	
	loadi	%r1, VM_SYSPAGE_ADDR
	cmpw	%r1,%r0
	bne		712f

	# Syspage read fault. Generate the TLB entry	
	lis		%r1,_syspage_ptr@ha
	lwz		%r1,_syspage_ptr@l(%r1)
	rlwinm	%r1,%r1,0,0,19
	ori		%r1,%r1,PPC_TLBLO_PP_RO+PPC_TLBLO_M+PPC_TLBLO_C+PPC_TLBLO_R
	mtspr	PPC603_SPR_RPA, %r1
	bittst	%r1,%r2,VM_FAULT_INSTR
	bne 800f	
	tlbld	%r3
	b		720f
800:	
	tlbli	%r3
	b		720f

712:
	bittst	%r1,%r2,VM_FAULT_INSTR
	bne-	766f
	loadi	%r1,VM_CPUPAGE_ADDR
	cmpw	%r1,%r0
	bne-	766f
	
	# This is a cpupage read fault. Generate the TLB
	lis		%r1,_cpupage_ptr@ha
	lwz		%r1,_cpupage_ptr@l(%r1)
	rlwinm	%r1,%r1,0,0,19
	ori		%r1,%r1,PPC_TLBLO_PP_RO+PPC_TLBLO_M+PPC_TLBLO_C+PPC_TLBLO_R
	mtspr	PPC603_SPR_RPA, %r1
	tlbld	%r3
	b		720f

	# We have successfully written the TLB entry. Restore and exit.
720:
	lwz		%r1,RESERVED_BASE+STACK_OFFSET+12(0)
	lwz		%r0,RESERVED_BASE+STACK_OFFSET+8(0)
	mtcr	%r1
	restore_regs_r123
	rfi

#
# Looks like a real fault. Restore regs and go to generic routine
#

766:
	lwz		%r1,RESERVED_BASE+STACK_OFFSET+12(0)
	lwz		%r0,RESERVED_BASE+STACK_OFFSET+8(0)
	mtcr	%r1
	restore_regs_r123
	# We've saved the faulting address and flags in our save area
	b		__exc_access_700_exit



.global __exc_instr_access603
__exc_instr_access603:
	# R5 set by mmu_on code
	bitset	%r5,%r5,VM_FAULT_INSTR	# say it was an instruction access
	mfspr	%r4,SRR1
	bittst	%r0,%r4,0x18000000
	beq		1f
	bitset	%r5,%r5,VM_FAULT_WIMG_ERR	# indicate a WIMG error
1:	
	lwz		%r4,REG_IAR(%r30)
	b 		__exc_access


.global __exc_data_access603
__exc_data_access603:  
	# R5 set by mmu_on code
	mfdsisr	%r0
	bittst	%r4,%r0,0x0c000000
	beq		1f
	bitset	%r5,%r5,VM_FAULT_WIMG_ERR	# indicate a WIMG error
1:	
	mfspr	%r4,PPC_SPR_DAR 	# get address responsible
	bittst	%r0,%r0,0x02000000
	beq		__exc_access
	bitset	%r5,%r5,VM_FAULT_WRITE		# indicate a store instruction
	b		__exc_access
