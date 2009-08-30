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
	
.text
	
/*
 * r4k_update_tlb(hi, lo0, lo1, pgmask)
 *	Update the TLB with an entry, checking to see if there already
 *  one for the virtual address and replacing it if need be - also
 *  allows for non-4K pagemask values.
 *
 */
FRAME(r4k_update_tlb,sp,0,ra)
	DISABLEINTERRUPTS(t0,t1)
	mfc0	t3,CP0_TLB_HI		# save Entry Hi
	mtc0	a0,CP0_TLB_HI		# set Entry Hi register
	 ssnop; ssnop; ssnop; ssnop
	tlbp						# probe tlb
	 ssnop; ssnop
	mfc0	v0,CP0_INDEX		# get index register
	 ssnop
#if !defined(VARIANT_r3k)
	mtc0	a3,CP0_PAGEMASK 	# set new pagemask
	 addiu	a2,0				# sign extend 32=>64 bits
	mtc0	a2,CP0_TLB_LO_1		# write Entry Lo 1 register
	 addiu	a1,0				# sign extend 32=>64 bits
#endif
	mtc0	a1,CP0_TLB_LO_0		# write Entry Lo 0 register
	 or		a1,a2
	mtc0	a0,CP0_TLB_HI		# set new Entry hi register
	 ssnop; ssnop; ssnop
	bltz	v0,1f				# branch if neg => TLB probe failed
	  nop
	tlbwi						# write TLB entry, indexed
	 b		2f
	  nop
1:
	beqz	a1,2f				# skip if just trying to kill entry
	 nop
	tlbwr						# write TLB entry, random
	 ssnop; ssnop
2:
	mtc0	t3,CP0_TLB_HI		# restore original entry hi
	 ssnop
 
	RESTOREINTERRUPTS(t0,t1)
	j	ra			# return
	 nop
ENDFRAME(r4k_update_tlb)
