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

.include "ppc/util.ah"
.include "asmoff.def"
	.text

#
# Optimized memcpy for message xfer, using the altivec registers. 
# 
# on entry: 
# %r3: char *dst
# %r4: char *src
# %r5: int nbytes
# %r6: vmx save area
#
# We save in %r0 the dst pointer. On preemption, %r3-%r0 equals the number
# of bytes already transferred.
#
# If this routine is called, it's because the copy is large, so we
# can assume nbytes > 16
#

	.globl	_xfer_cpy_vmx_start
	.globl	_xfer_cpy_vmx_end

routine_start _xfer_cpy_vmx,1
	mfmsr	%r8
	bitset	%r8,%r8,PPC_MSR_VA
	mr.		%r7,%r6
	mtmsr	%r8
	isync
	beq		1f
					# We need to save some VMX registers
	stvx	%v0,0,%r7
	addi	%r7,%r7,16
	stvx	%v1,0,%r7
	addi	%r7,%r7,16
	stvx	%v2,0,%r7
	addi	%r7,%r7,16
	stvx	%v3,0,%r7
					# OK, now we've got 4 registers to play with...
1:
					# Now we must align the dst pointer to a 16 byte boundary
	# Don't touch %r0 from now on, it has the original dst ptr.
	mr		%r0,%r3
	andi.	%r7,%r3,0x0f
	beq		..dst_align
	subfic	%r7,%r7,16
	srwi	%r8,%r7,2
2:
	cmpwi	%r8,0
	beq		3f
	lwz		%r9,0(%r4)
	addi	%r4,%r4,4
	stw		%r9,0(%r3)
	addi	%r8,%r8,-1
	addi	%r3,%r3,4
	b		2b
3:
	andi.	%r8,%r7,0x03
	beq		4f
	mtxer	%r8
	lswx	%r9,0,%r4
	stswx	%r9,0,%r3
	add		%r4,%r4,%r8
	add		%r3,%r3,%r8
4:
	subf	%r5,%r7,%r5

	#
	# Now dst is aligned. We have two further cases:
	# - either src is aligned as well, which makes it easy
	# - or src is unaligned, in which case we'll use vmx to load/rotate
	#   src before storing
	#

..dst_align:
	li		%r10,16
	li		%r11,32
	andi.	%r7,%r4,0x0f
	li		%r12,48
	beq		..all_align

_xfer_cpy_vmx_start:
..src_unaligned:
	#
	# We use the vector select functions to load both low and
	# high parts of the unaligned memory, and stick it in dst.
	# On entry to the loop, v0 contains the high part of 
	# the string (lowest address), v1 contains the permute mask
	# 
	#
	srwi	%r8,%r5,4
	lvxl	%v0,0,%r4
	lvsl	%v1,0,%r4
1:
	cmpwi	%r8,1
	ble		2f
	lvxl	%v2,%r10,%r4
	lvxl	%v3,%r11,%r4
	addi	%r4,%r4,32
	vperm	%v0,%v0,%v2,%v1
	vperm	%v2,%v2,%v3,%v1
	addi	%r8,%r8,-2
	stvx	%v0,0,%r3
	vor		%v0,%v3,%v3	
	stvx	%v2,%r10,%r3
	addi	%r3,%r3,32
	b		1b

2:					# zero or 1 left
	cmpwi	%r8,0
	beq		..done_16s
					# do last one
	lvxl	%v2,%r10,%r4
	addi	%r4,%r4,16
	vperm	%v0,%v0,%v2,%v1
	stvx	%v0,0,%r3
	addi	%r3,%r3,16
	b		..done_16s

..all_align:
	srwi	%r8,%r5,4
	cmpwi	%r8,3
	ble-	1f
2:						# Do 64 bytes per loop
	lvxl	%v0,0,%r4
	lvxl	%v1,%r10,%r4
	addi	%r8,%r8,-4
	lvxl	%v2,%r11,%r4
	stvx	%v0,0,%r3
	lvxl	%v3,%r12,%r4
	addi	%r4,%r4,64
	stvx	%v1,%r10,%r3
	cmpwi	%r8,3
	stvx	%v2,%r11,%r3
	stvx	%v3,%r12,%r3
	addi	%r3,%r3,64
	bgt+	2b
1:
	cmpwi	%r8,0
	beq		2f
	lvxl	%v0,0,%r4
	addi	%r4,%r4,16
	stvx	%v0,0,%r3
	addi	%r8,%r8,-1
	addi	%r3,%r3,16
	b		1b
2:
..done_16s:			# no more 16-byte multiples to do...
	andi.	%r7,%r5,0x0f
	beq		..done
	srwi	%r8,%r7,2
2:
	cmpwi	%r8,0
	beq		3f
	lwz		%r9,0(%r4)
	addi	%r4,%r4,4
	stw		%r9,0(%r3)
	addi	%r8,%r8,-1
	addi	%r3,%r3,4
	b		2b
3:
	andi.	%r8,%r7,0x03
	beq		4f
	mtxer	%r8
	lswx	%r9,0,%r4
	stswx	%r9,0,%r3
					# We're now all done!
	add		%r3,%r3,%r8
4:
..done:
_xfer_cpy_vmx_end:
	mr.		%r7,%r6
	sub		%r3,%r3,%r3
	beqlr+
					# We need to restore some VMX registers
	lvx		%v0,0,%r7
	addi	%r7,%r7,16
	lvx		%v1,0,%r7
	addi	%r7,%r7,16
	lvx		%v2,0,%r7
	addi	%r7,%r7,16
	lvx		%v3,0,%r7
	blr
routine_end _xfer_cpy_vmx

#
# restore the registers we've borrowed to do our copy; r3 has the VMX
# save area
#
routine_start _xfer_cpy_vmx_restore,1
	# Make sure we can use Altivec instructions (we might be
	# preempting from an interrupt, which will have cleared the VA
	# bit).
	mfmsr	%r8
	bitset	%r8,%r8,PPC_MSR_VA
	mtmsr	%r8
	isync
	
	lvx		%v0,0,%r3
	addi	%r3,%r3,16
	lvx		%v1,0,%r3
	addi	%r3,%r3,16
	lvx		%v2,0,%r3
	addi	%r3,%r3,16
	lvx		%v3,0,%r3
	blr
routine_end _xfer_cpy_vmx_restore
