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


.ifdef PPC_CPUOP_ENABLED
	.cpu booke32
.endif

#
# IBM versions of book E TLB routines. Standards, gotta love 'em :-(
#

routine_start ppcbke_tlb_write_ibm, 1
	# Initial moves because we added an extra parm to the front of
	# the list. If we re-org the remainder of the list, they can be
	# removed
	mr		%r3,%r4
	mr		%r4,%r5
	
	# get word 1 (RPN) ready
.ifdef __LITTLEENDIAN__
	lwz		%r5,PPCBKE_TLB_RPN+0(%r4)
	lwz		%r6,PPCBKE_TLB_RPN+4(%r4)
.else
	lwz		%r5,PPCBKE_TLB_RPN+4(%r4)
	lwz		%r6,PPCBKE_TLB_RPN+0(%r4)
.endif
	rlwinm	%r5,%r5,0,0,21 
	or		%r5,%r5,%r6	
	
	# get word 2 (attr/access) ready
	lbz		%r6,PPCBKE_TLB_ACCESS(%r4)
	lhz		%r7,PPCBKE_TLB_ATTR(%r4)
	# r7 now holds the attr bits as specified by the PPCBKE_TLB_ATTR_* and PPCBKEM_TLB_ATTR_*
	# flags specified in bookecpu.h.  We need to shift them into the correct position for
	# the IBM-specified TLB word 2 that we're building up in r6.
	#
	#          IBM TLB         tlb->attr
	# WIMGE    bits 20-24      bits 27-31
	# U0-U3    bits 16-19      bits 20-23
	#
	# Insert WIMGE bits, from 27-31 -> 20-24 into r6.
	rlwimi	%r6,%r7,31-24,20,24
	# Insert Ux bits, from 20-23 -> 16-19 into r6.
	rlwimi	%r6,%r7,23-19,16,19 
	
	# get word 0 (epn/v/ts/size/tid) ready
	lwz		%r7,PPCBKE_TLB_EPN(%r4)
	rlwinm	%r7,%r7,0,0,21
	lbz		%r8,PPCBKE_TLB_V(%r4)
	rlwimi	%r7,%r8,31-22,22,22
	lbz		%r8,PPCBKE_TLB_TS(%r4)
	rlwimi	%r7,%r8,31-23,23,23
	lbz		%r8,PPCBKE_TLB_SIZE(%r4)
	rlwimi	%r7,%r8,31-27,24,27
	lwz		%r8,PPCBKE_TLB_TID(%r4)
	
	mfspr	%r9,PPC440_SPR_MMUCR
	rlwimi	%r9,%r8,0,24,31
	mtspr	PPC440_SPR_MMUCR,%r9
	
   	tlbwe	%r6,%r3,2
   	tlbwe	%r5,%r3,1
   	tlbwe	%r7,%r3,0
	
	isync
	blr
routine_end ppcbke_tlb_write_ibm

routine_start ppcbke_tlb_read_ibm, 1
	# Initial moves because we added an extra parm to the front of
	# the list. If we re-org the remainder of the list, they can be
	# removed
	mr		%r3,%r4
	mr		%r4,%r5
	
   	tlbre	%r5,%r3,0
	rlwinm	%r8,%r5,28,28,31
	stb		%r8,PPCBKE_TLB_SIZE(%r4)
	rlwinm	%r8,%r5,24,31,31
	stb		%r8,PPCBKE_TLB_TS(%r4)
	rlwinm	%r8,%r5,24,31,31
	stb		%r8,PPCBKE_TLB_TS(%r4)
	rlwinm	%r8,%r5,23,31,31
	stb		%r8,PPCBKE_TLB_V(%r4)
	rlwinm	%r8,%r5,0,0,21
	stw		%r8,PPCBKE_TLB_EPN(%r4)
	mfspr	%r8,PPC440_SPR_MMUCR
	rlwinm	%r8,%r8,0,24,31
	stw		%r8,PPCBKE_TLB_TID(%r4)
	
   	tlbre	%r5,%r3,1
	rlwinm	%r8,%r5,0,0,21
	rlwinm	%r7,%r5,0,28,31
.ifdef __LITTLEENDIAN__
	stw		%r8,PPCBKE_TLB_RPN+0(%r4)
	stw		%r7,PPCBKE_TLB_RPN+4(%r4)
.else
	stw		%r8,PPCBKE_TLB_RPN+4(%r4)
	stw		%r7,PPCBKE_TLB_RPN+0(%r4)
.endif

   	tlbre	%r5,%r3,2
	rlwinm	%r8,%r5,0,26,31
	stb		%r8,PPCBKE_TLB_ACCESS(%r4)
	# r5 holds the TLB word 2.  We need to extract the flags from the tlb to create the 
	# tlb->attr field.  TLB is defined by the IBM booke spec.  The tlb->attr field is
	# defined by the PPCBKE_TLB_ATTR_* and PPCBKEM_TLB_ATTR_* flags in bookecpu.h
	#
	#          IBM TLB         tlb->attr
	# WIMGE    bits 20-24      bits 27-31
	# U0-U3    bits 16-19      bits 20-23
	#
	# Insert WIMGE bits, from TLB bits[20-24] -> attr bits[27-31].
	rlwinm	%r8,%r5,32+24-31,27,31
	# Insert Ux bits, from TLB bits[16-19] -> attr bits[20-23]
	rlwimi	%r8,%r5,32+19-23,20,23 
	# and store it in the tlb->attr field
	sth		%r8,PPCBKE_TLB_ATTR(%r4)
	
	blr
routine_end ppcbke_tlb_read_ibm
