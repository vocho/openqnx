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
# Optimized memcpy for message xfer. Assumes that aligned copies are done, 
# or that the hardware supports unaligned accesses. Also store # of 
# transferred bytes for premmption
#
# 
# %r3: char *dst
# %r4: char *src
# %r5: int nbytes
#
# We save in %r6 the dst pointer. On preemption, %r3-%r6 equals the number
# of bytes already transferred.
#

	.globl	_xfer_cpy_start
	.globl	_xfer_cpy_end

routine_start _xfer_cpy,1
	mr		%r6,%r3
	andi.	%r7,%r5,0x0f
	srwi	%r8,%r5,4
	cmpwi	%r8,0
	beq-	2f
_xfer_cpy_start:

1:
	lwz		%r9,0(%r4)
	addi	%r8,%r8,-1
	lwz		%r10,4(%r4)
	lwz		%r11,8(%r4)
	cmpwi	%r8,0
	stw		%r9,0(%r3)
	lwz		%r12,12(%r4)
	stw		%r10,4(%r3)
	addi	%r4,%r4,16
	stw		%r11,8(%r3)
	stw		%r12,12(%r3)
	addi	%r3,%r3,16
	bne+	1b
2:
	srwi	%r8,%r7,2
	cmpwi	%r8,0
	beq-	4f
3:
	lwz		%r9,0(%r4)
	addi	%r8,%r8,-1
	addi	%r4,%r4,4
	cmpwi	%r8,0
	stw		%r9,0(%r3)
	addi	%r3,%r3,4
	bne+	3b
4:
	andi.	%r7,%r5,0x03
	cmpwi	%r7,0
	beq+	6f
5:
	lbz		%r9,0(%r4)
	addi	%r7,%r7,-1
	addi	%r4,%r4,1
	cmpwi	%r7,0
	stb		%r9,0(%r3)
	addi	%r3,%r3,1
	bne+	5b
6:
_xfer_cpy_end:
	sub	%r3,%r3,%r3
	blr
routine_end _xfer_cpy
