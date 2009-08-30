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

/*
 * xfer_cpy.S
 */
#include <mips/asm.h>
#include "loadstore.h"

	.set	noreorder
	.set	MIPSARCH

	.global _xfer_cpy_end
	.global _xfer_cpy_start

/*
 * int xfer_cpy(char *dst, char *src, unsigned nbytes)
 *
 * a0: dst
 * a1: src
 * a2: nbytes
 * a3: original dst. a0 - a3 = bytes xfered.
 * 
 * Very fast byte copy.  Will handle misaligned sources and
 * destinations, although "double long" aligned (i.e. 64 bit boundary)
 * source and destination addresses will increase speed significantly.
 *
 *        This routine does not check for overlapping source and
 *        destination address ranges.
 */
FRAME(xfer_cpy,sp,0,ra)
	move	 a3,a0

_xfer_cpy_start:
#if defined(VARIANT_32) || defined(VARIANT_r3k)

#ifdef __BIGENDIAN__
#  define LWHI	lwl
#  define LWLO	lwr
#  define SWHI	swl
#  define SWLO	swr
#else
#  define LWHI	lwr
#  define LWLO	lwl
#  define SWHI	swr
#  define SWLO	swl
#endif

	slt     t2, a2, 12              # check for small copy

	bne     t2, zero, smallcpy      # do a small bcopy
	 xor    t9, a1, a0              # compare low two bits of addresses
	and     t9, t9, 3
	subu    t8, zero, a0            # compute # bytes to word align address
	beq     t9, zero, aligned       # addresses can be word aligned
	 and    t8, t8, 3

	beq     t8, zero, 1f
	 subu   a2, a2, t8              # subtract from remaining count
	LWHI    t9, 0(a1)               # get next 4 bytes (unaligned)
	LWLO    t9, 3(a1)
	addu    a1, a1, t8
	SWHI    t9, 0(a0)               # store 1, 2, or 3 bytes to align a0
	addu    a0, a0, t8
1:
	and     t9, a2, 3               # compute number of words left
	subu    t8, a2, t9
	move    a2, t9
	addu    t8, t8, a1              # compute ending address
2:
	LWHI    t9, 0(a1)               # copy words a1 unaligned, a0 aligned
	LWLO    t9, 3(a1)
	addu    a1, a1, 4
	sw     t9, 0(a0)
	bne     a1, t8, 2b
	 addu   a0, a0, 4
	b       smallcpy
	 nop
aligned:
	beq     t8, zero, 1f
	 subu   a2, a2, t8              # subtract from remaining count
	LWHI    t9, 0(a1)               # copy 1, 2, or 3 bytes to align
	addu    a1, a1, t8
	SWHI    t9, 0(a0)
	addu    a0, a0, t8
1:
	and     t9, a2, 3               # compute number of whole words left
	subu    t8, a2, t9
	move    a2, t9
	addu    t8, t8, a1              # compute ending address
2:
	lw      t9, 0(a1)               # copy words
	addu    a1, a1, 4
	sw      t9, 0(a0)
	bne     a1, t8, 2b
	 addu   a0, a0, 4
smallcpy:
	ble     a2, zero, bc_exit
	 addu   t8, a2, a1              # compute ending address
1:
	lbu     t9, 0(a1)               # copy bytes
	addu    a1, a1, 1
	sb      t9, 0(a0)
	bne     a1, t8, 1b
	 addu   a0, a0, 1

#elif defined(VARIANT_be)

	move	t0,a2			# copy count

	andi	t4,a0,0x7		# t4 = dst & 0x7 (bd slot)
	bnez	t4,bc_odd_dst		# dst ptr not double-word aligned
	 andi	t5,a1,0x7		# t5 = src & 0x7 (bd slot)
	bnez	t5,bc_odd_src		# src ptr not double-word aligned
	 slti	t3,t0,32		# do we have 32 bytes?

	/* both src and dst are double word aligned. */
	bnez	t3,bc_even_16		# try copying 16 
	 nop
	addiu	t0,t0,-32		# decrement count

bc_even_loop:
	ld      t4,0(a1)        	# read *(src)
	ld      t5,8(a1)        	# read *(src + 8)
	ld      t6,16(a1)       	# read *(src + 16)
	ld      t7,24(a1)       	# read *(src + 24)
	sd      t4,0(a0)        	# save *(dst)
	sd      t5,8(a0)        	# save *(dst + 8)
	sd      t6,16(a0)       	# save *(dst + 16)
	sd      t7,24(a0)       	# save *(dst + 24)

	addiu	a0,a0,32		# dst += 32
	slti	t3,t0,32		# 32 bytes left?
	bnez	t3,bc_even_16	
	 addiu	a1,a1,32		# src += 32 (bd slot)

	b	bc_even_loop		# do 32 byte loop again
	 addiu	t0,t0,-32		# decrement real count 

bc_even_16:
	slti    t3,t0,16
	bnez	t3,bc_even_min		# are there at least 16 bytes left?
	 nop
	addiu	t0,t0,-16

	ld      t4,0(a1)        	# read *(src)
	ld      t5,8(a1)        	# read *(src + 8)
	sd      t4,0(a0)        	# save *(dst)
	sd      t5,8(a0)        	# save *(dst + 8)

	addiu	a0,a0,16		# dst += 16
	b	bc_even_min
	 addiu	a1,a1,16		# src += 16 (bd slot)

bc_even_min:

	slti    t3,t0,4                 # any word copies possible?
	bnez	t3,bc_min		# go do byte copies 
	 nop			

	lw	t4,0(a1)		# read *(src)
	addiu	a1,a1,4		
	sw	t4,0(a0)		# save *(dst)
	addiu	t0,t0,-4
	b	bc_even_min
	 addiu	a0,a0,4		

bc_odd_src:
	bnez	t3,bc_odd_src_16	# do we have 32 bytes?
	 nop
	addiu	t0,t0,-32		# decrement count

bc_odd_src_loop:
	/*
	 * We have at least 32 bytes, count has been pre-decremented
	 * and dst is double-word-aligned and src is not. 
	 */
	ldl     t4,0(a1)        	# read *(src)
	ldr     t4,7(a1)
	ldl     t5,8(a1)        	# read *(src + 8)
	ldr     t5,15(a1)
	ldl     t6,16(a1)       	# read *(src + 16)
	ldr     t6,23(a1)
	ldl     t7,24(a1)       	# read *(src + 24)
	ldr     t7,31(a1)

	sd      t4,0(a0)        	# save *(dst)
	sd      t5,8(a0)        	# save *(dst + 8)
	sd      t6,16(a0)       	# save *(dst + 16)
	sd      t7,24(a0)       	# save *(dst + 24)

	addiu	a0,a0,32		# dst += 32
	slti	t3,t0,32		# 32 bytes left?
	bnez	t3,bc_odd_src_16
	 addiu	a1,a1,32		# src += 32 (bd slot)
	b	bc_odd_src_loop
	 addiu	t0,t0,-32		# decrement real count (bd slot)

bc_odd_src_16:
	slti	t3,t0,16		# are there at least 16 bytes left?
	bnez	t3,bc_odd_src_min
	 nop
	addiu	t0,t0,-16

	ldl     t4,0(a1)        	# read *(src)
	ldr     t4,7(a1)
	ldl     t5,8(a1)        	# read *(src + 8)
	ldr     t5,15(a1)
	sd      t4,0(a0)        	# save *(dst)
	sd      t5,8(a0)        	# save *(dst + 8)

	addiu	a0,a0,16		# dst += 16
	addiu	a1,a1,16		# src += 16 (bd slot)

bc_odd_src_min:

	slti    t3,t0,4                 # any word copies possible?
	bnez	t3,bc_min		# go do byte copies 
	 nop			

	lwl     t4,0(a1)        	# read *(src)
	lwr     t4,3(a1)
	addiu	a1,a1,4		
	sw	t4,0(a0)		# save *(dst)
	addiu	t0,t0,-4
	b	bc_odd_src_min
	 addiu	a0,a0,4		

bc_odd_dst:
	bnez	t5,bc_odd_src_dst
	 nop

	slti	t3,t0,32		# do we have 32 bytes?
	bnez	t3,bc_odd_dst_16	
	 nop
	addiu	t0,t0,-32		# decrement count

bc_odd_dst_loop:
	/*
	 * We have at least 32 bytes, count has been pre-decremented
	 * src is double-word-aligned and dst is not. 
	 */
	ld      t4,0(a1)        	# read *(src)
	ld      t5,8(a1)        	# read *(src + 8)
	ld      t6,16(a1)       	# read *(src + 16)
	ld      t7,24(a1)       	# read *(src + 24)

	sdl     t4,0(a0)        	# save *(dst)
	sdr     t4,7(a0)
	sdl     t5,8(a0)        	# save *(dst + 8)
	sdr     t5,15(a0)
	sdl     t6,16(a0)       	# save *(dst + 16)
	sdr     t6,23(a0)
	sdl     t7,24(a0)       	# save *(dst + 24)
	sdr     t7,31(a0)

	addiu	a0,a0,32		# dst += 32
	slti	t3,t0,32		# 32 bytes left?
	bnez	t3,bc_odd_dst_16
	 addiu	a1,a1,32		# src += 32 (bd slot)

	b	bc_odd_dst_loop
	 addiu	t0,t0,-32		# decrement real count (bd slot)

bc_odd_dst_16:
	slti	t3,t0,16		# are there at least 16 bytes left?
	bnez	t3,bc_odd_dst_min
	 nop
	addiu	t0,t0,-16

	ld      t4,0(a1)        	# read *(src)
	ld      t5,8(a1)        	# read *(src + 8)
	sdl     t4,0(a0)        	# save *(dst)
	sdr     t4,7(a0)
	sdl     t5,8(a0)        	# save *(dst + 8)
	sdr     t5,15(a0)

	addiu	a0,a0,16		# dst += 16
	addiu	a1,a1,16		# src += 16 (bd slot)

bc_odd_dst_min:

	slti    t3,t0,4         # any word copies possible?
	bnez	t3,bc_min		# go do byte copies 
	 nop			

	lw	t4,0(a1)			# read *(src)
	addiu	a1,a1,4		
	swl     t4,0(a0)       	# save *(dst)
	swr     t4,3(a0)
	addiu	t0,t0,-4
	b	bc_odd_dst_min
	 addiu	a0,a0,4		

bc_odd_src_dst:
	slti	t3,t0,32		# do we have 32 bytes?
	bnez	t3,bc_odd_src_dst_16	
	 nop
	addiu	t0,t0,-32		# decrement count

bc_odd_src_dst_loop:
	/*
	 * We have at least 32 bytes, count has been pre-decremented
	 * and both source and destination pointers are misaligned
	 */
	ldl     t4,0(a1)        	# read *(src)
	ldr     t4,7(a1)
	ldl     t5,8(a1)        	# read *(src + 8)
	ldr     t5,15(a1)
	ldl     t6,16(a1)       	# read *(src + 16)
	ldr     t6,23(a1)
	ldl     t7,24(a1)       	# read *(src + 24)
	ldr     t7,31(a1)

	sdl     t4,0(a0)        	# save *(dst)
	sdr     t4,7(a0)
	sdl     t5,8(a0)        	# save *(dst + 8)
	sdr     t5,15(a0)
	sdl     t6,16(a0)       	# save *(dst + 16)
	sdr     t6,23(a0)
	sdl     t7,24(a0)       	# save *(dst + 24)
	sdr     t7,31(a0)

	addiu	a0,a0,32		# dst += 32
	slti	t3,t0,32		# 32 bytes left?
	bnez	t3,bc_odd_src_dst_16
	 addiu	a1,a1,32		# src += 32 (bd slot)

	b	bc_odd_src_dst_loop
	 addiu	t0,t0,-32		# decrement real count (bd slot)

bc_odd_src_dst_16:
	slti	t3,t0,16		# are there at least 16 bytes left?
	bnez	t3,bc_odd_src_dst_min	
	 nop
	addiu	t0,t0,-16

	ldl     t4,0(a1)        	# read *(src)
	ldr     t4,7(a1)
	ldl     t5,8(a1)        	# read *(src + 8)
	ldr     t5,15(a1)
	sdl     t4,0(a0)        	# save *(dst)
	sdr     t4,7(a0)
	sdl     t5,8(a0)        	# save *(dst + 8)
	sdr     t5,15(a0)

	addiu	a0,a0,16		# dst += 16
	addiu	a1,a1,16		# src += 16 (bd slot)

bc_odd_src_dst_min:

	slti    t3,t0,4                 # any word copies possible?
	bnez	t3,bc_min		# go do byte copies 
	 nop			

	lwl     t4,0(a1)        	# read *(src)
	lwr     t4,3(a1)
	addiu	a1,a1,4		
	swl     t4,0(a0)        	# save *(dst)
	swr     t4,3(a0)
	addiu	t0,t0,-4
	b	bc_odd_src_dst_min
	 addiu	a0,a0,4		

bc_min:

	/* byte copy, less than 4 bytes left */
	addiu	t0,t0,-1		# decrement count
	bltz	t0,bc_exit		# any more ? no, exit
	 nop			

	lb	t3,0(a1)		# load a byte
	addiu	a1,a1,1			# increment src pointer
	sb	t3,0(a0)		# save a byte

	b	bc_min
	 addiu	a0,a0,1			# increment dst pointer (bd slot)

#else /* 64-bit little endian copy code */

	or		t1,a0,a1

	andi	v0,t1,31
	bne		v0,zero,1200f	
	 sltiu	v0,a2,32
	bne		v0,zero,1200f
	 nop
1201: ld	t1,0(a1)
	ld		t3,8(a1)
	ld		t4,16(a1)
	ld		t5,24(a1)
	addiu	a2,a2,-32 
	addiu	a1,a1,32
	sltiu	v0,a2,32 
	sd		t1,0(a0) 
	sd		t3,8(a0) 
	sd		t4,16(a0) 
	sd		t5,24(a0) 
	beq 	v0,zero,1201b
	 addiu	a0,a0,32 
	
	or		t1,a0,a1
1200:	
	
	sltiu	v0,a2,8
	bne		v0,zero,1100f
	 andi	v0,t1,7
	bne		v0,zero,1100f	
	 nop
1101: ld	t1,0(a1)
	addiu	a2,a2,-8 
	addiu	a1,a1,8 
	sltiu	v0,a2,8 
	sd		t1,0(a0) 
	beq 	v0,zero,1101b
	 addiu	a0,a0,8 
	
	or		t1,a0,a1
1100:	
	
	sltiu	v0,a2,4
	bne		v0,zero,1002f
	 andi	t1,3 
	bne		t1,zero,1002f
	 nop 
1001: lw	t1,0(a1)
	addiu	a2,a2,-4 
	addiu	a1,a1,4 
	sltiu	v0,a2,4 
	sw		t1,0(a0) 
	beq 	v0,zero,1001b
	 addiu	a0,a0,4 
1002: beq		a2,zero,1004f
	nop
1003: lbu		t1,0(a1)
	addiu	a2,a2,-1 
	addiu	a1,a1,1 
	sb		t1,0(a0)	
	bne		a2,zero,1003b
	 addiu	a0,a0,1 
1004: 

#endif /* not defined(VARIANT_be) -- little endian */

bc_exit:
_xfer_cpy_end:
	j	ra
	 li	v0,0
ENDFRAME(xfer_cpy)
