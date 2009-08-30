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

/*
 * r4k_gettlb:
 *
 * Get index TLB entry. Return 0 for success, 1 for fail
 * On the Orion chip, the entry_hi,lo0,lo1 registers are all 64bit.
 *
 */

 #
 # int r4k_gettlb (struct tlb_t *tlb, int index)
 #
FRAME(r4k_gettlb,sp,0,ra)
	.set	noreorder

	DISABLEINTERRUPTS(t0,t1)
	mfc0	t2,CP0_TLB_HI	# get current ASID
	 ssnop
#if !defined(VARIANT_r3k)
	mfc0	t5,CP0_PAGEMASK	# get current pagemask
	 ssnop
#endif
	mtc0	a1,CP0_INDEX	# write TLB index
	 ssnop; ssnop; ssnop; ssnop
	tlbr					# read TLB entry
	 ssnop; ssnop
#if defined(VARIANT_r3k)
	move	t3,zero
#else
	mfc0	t3,CP0_PAGEMASK	# read page mask
	 ssnop
#endif
	mfc0	t4,CP0_TLB_HI	# read entry Hi
	 ssnop
	sw		t3,0(a0)		# tlb->pmask = CP0_PAGEMASK
	sw		t4,4(a0)		# tlb->hi = CP0_TLB_HI
	mfc0	t3,CP0_TLB_LO_0	# read entry lo 0
#if defined(VARIANT_r3k)
	move	t4,zero
#else
	mfc0	t4,CP0_TLB_LO_1	# read entry lo 1
	 ssnop
#endif
	sw		t3,8(a0)		# tlb->lo0 = CP0_TLB_LO_0
	sw		t4,12(a0)		# tlb->lo1 = CP0_TLB_LO_1
	mtc0	t2,CP0_TLB_HI	# restore current ASID
#if !defined(VARIANT_r3k)
	mtc0	t5,CP0_PAGEMASK	# reset pagemask
#endif
	RESTOREINTERRUPTS(t0,t1)

	j		ra				# return
	 move	v0,zero			# return success (bd slot)

	.set 	reorder
ENDFRAME(r4k_gettlb)
