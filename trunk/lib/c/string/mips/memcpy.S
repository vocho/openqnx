/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */




/*
 * Get global defs.  Remove conflicting names from other architectures
 */
#include <mips/asm.h>
#include "asmoff.def"

	.set	noreorder
	
	.extern	__cpu_flags,4

#if defined(__PIC__) || defined (__DLL)
.data
#else
.sdata
#endif
mc: 	.long mc_chk

.text

FRAME(__reset_mc,sp,0,ra)
#ifdef __PIC__
	move	t9,s7
	move	t8,ra
	.cpload	ra				# We need to temporarily load $s7
	move	ra,t8
#endif
	la		t0,mc_chk
	sw		t0,mc
#ifdef __PIC__
	move	s7,t9
#endif
	j	ra
	 nop
ENDFRAME(__reset_mc);

/*
 * memcpy:
 *
 * Very fast byte copy.  Will handle misaligned sources and
 * destinations, although "double long" aligned (i.e. 64 bit boundary)
 * source and destination addresses will increase speed significantly.
 *
 *        This routine does not check for overlapping source and
 *        destination address ranges.
 */
FRAME(memcpy,sp,0,ra)
#ifdef __PIC__
#ifdef __MIPS_ABICALLS__
	.cpload t9
	move    t9,s7
	lw      t0,mc
	move    t7,s7           # in case we go to mc_chk
	jr      t0
	move   s7,t9
#else /* __MIPS_ABICALLS__  */
	move	t9,s7
	move	t8,ra
	.cpload	ra				# We need to temporarily load $s7
	move	ra,t8
	lw		t0,mc
	move	t7,s7			# in case we go to mc_chk
	jr		t0
	 move	s7,t9
#endif /* __MIPS_ABICALLS__  */
#else
	lw		t0,mc
	jr		t0
	 nop
#endif
	
mc_chk:
#ifdef __PIC__
	move	s7,t7
#endif
   	lw		t0,__cpu_flags
	andi	t0,MIPS_CPU_FLAG_64BIT
	la		t1,_32bit
	bnez	t0,1f
	 nop
	sw		t1,mc
	b		_32bit
#ifdef __PIC__
	 move	s7,t9
#else
	 nop
#endif
1:
	la		t0,_64bit
	sw		t0,mc
	// fall through to _64bit
#ifdef __PIC__
	 move	s7,t9
#endif

_64bit:
	.set	mips3
	
	move	v0,a0			# return pointer to start of dst buffer 
#if defined(__BIGENDIAN__)

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

bc_exit:
	j	ra
	 nop

#endif /* not defined(VARIANT_be) -- little endian */


_32bit: /* Do memcpy only using 32-bit instructions */
	.set	mips2

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
	move	v0,a0					# return pointer to start of dst buffer 

	bne     t2, zero, smallcpy      # do a small bcopy
	 xor    t9, a1, a0              # compare low two bits of addresses
	and     t9, t9, 3
	subu    a3, zero, a0            # compute # bytes to word align address
	beq     t9, zero, aligned       # addresses can be word aligned
	 and    a3, a3, 3

	beq     a3, zero, 1f
	 subu   a2, a2, a3              # subtract from remaining count
	LWHI    t9, 0(a1)               # get next 4 bytes (unaligned)
	LWLO    t9, 3(a1)
	addu    a1, a1, a3
	SWHI    t9, 0(a0)               # store 1, 2, or 3 bytes to align a0
	addu    a0, a0, a3
1:
	and     t9, a2, 3               # compute number of words left
	subu    a3, a2, t9
	move    a2, t9
	addu    a3, a3, a1              # compute ending address
2:
	LWHI    t9, 0(a1)               # copy words a1 unaligned, a0 aligned
	LWLO    t9, 3(a1)
	addu    a1, a1, 4
	addu    a0, a0, 4
	bne     a1, a3, 2b
	 sw     t9, -4(a0)
	b       smallcpy
	 nop
aligned:
	beq     a3, zero, 1f
	 subu   a2, a2, a3              # subtract from remaining count
	LWHI    t9, 0(a1)               # copy 1, 2, or 3 bytes to align
	addu    a1, a1, a3
	SWHI    t9, 0(a0)
	addu    a0, a0, a3
1:
	and     t9, a2, 3               # compute number of whole words left
	subu    a3, a2, t9
	move    a2, t9
	addu    a3, a3, a1              # compute ending address
2:
	lw      t9, 0(a1)               # copy words
	addu    a1, a1, 4
	addu    a0, a0, 4
	bne     a1, a3, 2b
	 sw     t9, -4(a0)
smallcpy:
	ble     a2, zero, 2f
	 addu   a3, a2, a1              # compute ending address
1:
	lbu     t9, 0(a1)               # copy bytes
	addu    a1, a1, 1
	addu    a0, a0, 1
	bne     a1, a3, 1b
	 sb     t9, -1(a0)
2:
	j       ra
	 nop

ENDFRAME(memcpy)
