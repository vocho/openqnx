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
#include "dbgmacros.h"

	.set	noreorder

/*
 * r4k_settlb(hi, lo0, lo1, pagemask, idx):
 *
 * Does a tlb write pair into a passed in tlb index, with the passed
 * in pagemask value. If the top bit is on in the idx parm, check for
 * a matching entry in the TLB and kill it, if found.
 *
 * On the Orion chip, the entry_hi,lo0,lo1 registers are all 64bit.
 */
FRAME(r4k_settlb,sp,8,ra)
	lw		t4,16(sp)			# get passed in index
	DISABLEINTERRUPTS(t1,t0)
	mfc0	t0,CP0_TLB_HI		# Save ASID
	bgez	t4,1f
	 mtc0	a0,CP0_TLB_HI		# write Entry Hi register

	// Check for an entry to be flushed
	andi	t4,0xffff			# remove top bit from index
	ssnop; ssnop; ssnop; ssnop
	tlbp
	 ssnop; ssnop;
	mfc0	v0,CP0_INDEX
	 ssnop
	bltz	v0,1f 
	 sll   t1,v0,13				# got a matching entry, have to kill it
	mtc0    zero,CP0_TLB_LO_0
	mtc0    t1,CP0_TLB_HI
#if !defined(VARIANT_r3k)
	mtc0    zero,CP0_TLB_LO_1
	mtc0    zero,CP0_PAGEMASK
#endif
	ssnop; ssnop; ssnop; ssnop
	tlbwi                       # kill the entry
	ssnop; ssnop
	mtc0	a0,CP0_TLB_HI		# re-write Entry Hi register
	b 3f
	 nop
1:
#ifdef TLB_SANITY_CHECK
	ssnop; ssnop; ssnop; ssnop
	tlbp
	 ssnop; ssnop;
	mfc0	v0,CP0_INDEX
	 ssnop
	bltz	v0,3f 
	 nop
	beq		v0,t4,3f
	 nop
	DEBUGKDBREAK
#endif
3:
	mtc0	a1,CP0_TLB_LO_0		# write Entry Lo 0 register
#if !defined(VARIANT_r3k)
	mtc0	a2,CP0_TLB_LO_1		# write Entry Lo 1 register
 	mtc0	a3,CP0_PAGEMASK		# set page mask
#endif
	mtc0	t4,CP0_INDEX		# TLB index = index
	 ssnop
 	 ssnop; ssnop; ssnop; ssnop
	tlbwi				# write TLB entry
	 ssnop; ssnop
	mtc0	t0,CP0_TLB_HI		# restore ASID
	RESTOREINTERRUPTS(t1,t0)

	j	ra			# return
	 nop
ENDFRAME(r4k_settlb)
