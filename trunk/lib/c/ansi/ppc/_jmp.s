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

#
#
# setjmp.s
#	Routines for saving and restoring a context
#

#################
################# NYI: What to do about F.P. registers 14 -> 31???
#################

#
# jmp_buf:
#
#      ----------------
#      |       LR      |    jmp_buf[0]
#      -----------------
#      |       SP (R1) |           [1]
#      -----------------
#      |       R14     |           [2]
#      -----------------
#      |       R15     |           [3]
#      -----------------
#      |       R16     |           [4]
#      -----------------
#      |       ...     |           ...
#      -----------------
#      |       R31     |           [19]
#      -----------------
#      |       CR      |           [20]
#      -----------------
#
 
	.global _setjmp
	.global _longjmp
	.section ".text"
	
 #
 # int _setjmp (jmp_buf *ptr)
 #
_setjmp:
		mflr	%r0
		stw		%r1,4(%r3)
		stw		%r0,0(%r3)
		mfcr	%r0
		stmw	%r14,8(%r3)
		stw		%r0,80(%r3)
		li		%r3,0
		blr

	.type _setjmp,@function
	.size _setjmp,.-_setjmp
 
 #
 # void _longjmp (jmp_buf *ptr, int ret_val)
 #
_longjmp:
		lwz		%r0,0(%r3)
		lmw		%r14,8(%r3)
		mtlr	%r0
		lwz		%r1,4(%r3)
		lwz		%r0,80(%r3)
		mr.		%r3,%r4
		mtcrf	0x38,%r0
### Workaround for PPC405 errata #41 (fixed in rev D)
###		bnelr+
		bnelr
#end workaround
		li		%r3,1
		blr

	.type _longjmp,@function
	.size _longjmp,.-_longjmp
