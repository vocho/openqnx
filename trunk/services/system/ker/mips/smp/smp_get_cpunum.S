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

#include "asmoff.def"
#include <mips/asm.h>
#include <getcpu.h>

	.set noreorder
	.set mips3

/*
 * smp_get_cpunum.S
 *  Get the CPU number of the processor that we're running on. _smpstart.S
 *  stored it in the top bits of the CP0_XCONTEXT register so that
 *  we can handle different SMP implementations.
 */
 
FRAME(get_cpunum,sp,0,ra)
	GETCPU_1(v0,0)
	j	ra
	 GETCPU_2(v0,0)
ENDFRAME(get_cpunum)
