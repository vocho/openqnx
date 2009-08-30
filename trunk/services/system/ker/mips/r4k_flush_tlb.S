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

#include <mips/asm.h>
#include <mips/cpu.h>

	.set	noreorder
	
/*
 * r4k_flush_tlb(hi)
 *	Update the TLB that matches 'hi' so that it points in the
 *  dead area.
 *
 */
FRAME(r4k_flush_tlb,sp,0,ra)
	DISABLEINTERRUPTS(t0,t1)
	mfc0	t3,CP0_TLB_HI		# save Entry Hi
	 ssnop
	mtc0	a0,CP0_TLB_HI		# set Entry Hi register
	 ssnop; ssnop; ssnop; ssnop
	tlbp						# probe tlb
	 ssnop; ssnop
	mfc0	v0,CP0_INDEX		# get index register
	 ssnop
	bltz	v0,1f				# branch if neg => TLB probe failed
	 sll		t2,v0,13		# Multiply index by (__PAGESIZE*2)
#if defined(VARIANT_r3k)
	li 		t4,8
#else	
	mfc0	t4,CP0_WIRED		# See if it is a wired entry
#endif	
	 li		t5,MIPS_R4K_K0BASE
	sltu	t4,v0,t4
	bnez	t4,1f				# wired entry => we do not flush
	 add	t2,t5
	mtc0	t2,CP0_TLB_HI
	mtc0	zero,CP0_TLB_LO_0
#if !defined(VARIANT_r3k)
	mtc0	zero,CP0_TLB_LO_1
	mtc0	zero,CP0_PAGEMASK
#endif
	 ssnop; ssnop; ssnop; ssnop
	tlbwi						# kill the entry
	 ssnop; ssnop
1:
	mtc0	t3,CP0_TLB_HI		# restore original entry hi
	 ssnop
 
	RESTOREINTERRUPTS(t0,t1)
	j	ra			# return
	 nop
ENDFRAME(r4k_flush_tlb)
