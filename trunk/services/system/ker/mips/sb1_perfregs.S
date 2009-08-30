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

#define SAVE_CONTROL(save_area,counter_num) \
	MFC0_SEL_OPCODE(8,25,((counter_num)*2)); \
	sw		t0,(counter_num*4)(save_area)
#define SAVE_COUNTER(save_area,counter_num) \
	DMFC0_SEL_OPCODE(8,25,((counter_num)*2)+1); \
	sd		t0,(counter_num*8)(save_area)



#define RESTORE_CONTROL(save_area,counter_num) \
	lw		t0,(counter_num*4)(save_area); \
	MTC0_SEL_OPCODE(8,25,((counter_num)*2))
#define RESTORE_COUNTER(save_area,counter_num) \
	ld		t0,(counter_num*8)(save_area); \
	DMTC0_SEL_OPCODE(8,25,((counter_num)*2)+1)

	.set	mips4
	.set	noreorder

FRAME(save_perfregs_sb1,sp,0,ra)

	addiu	t1,a0,SB1_PERF_CONTROL
	SAVE_CONTROL(t1,0)
	SAVE_CONTROL(t1,1)
	SAVE_CONTROL(t1,2)
	SAVE_CONTROL(t1,3)

	addiu	t1,a0,SB1_PERF_COUNTER
	SAVE_COUNTER(t1,0)
	SAVE_COUNTER(t1,1)
	SAVE_COUNTER(t1,2)
	SAVE_COUNTER(t1,3)

	DMFC0_SEL_OPCODE(8,22,0)
	sd		t0,SB1_PERF_PTR(a0)

	j	ra
	 nop
ENDFRAME(save_perfregs_sb1)

FRAME(restore_perfregs_sb1,sp,0,ra)

	addiu	t1,a0,SB1_PERF_CONTROL
	RESTORE_CONTROL(t1,0)
	RESTORE_CONTROL(t1,1)
	RESTORE_CONTROL(t1,2)
	RESTORE_CONTROL(t1,3)

	addiu	t1,a0,SB1_PERF_COUNTER
	RESTORE_COUNTER(t1,0)
	RESTORE_COUNTER(t1,1)
	RESTORE_COUNTER(t1,2)
	RESTORE_COUNTER(t1,3)

	j	ra
	 nop
ENDFRAME(restore_perfregs_sb1)


/* Set all counting modes to 0 to prevent counter
operation. */
#define DISABLE_COUNTER(counter_num) \
	li 		t1, 0xFFFFFFF0; \
	MFC0_SEL_OPCODE(8,25,((counter_num)*2)); \
	and		t0, t1; \
	MTC0_SEL_OPCODE(8,25,((counter_num)*2))
	

FRAME(disable_counters_sb1, sp, 0, ra)

	/* Disables all event counting modes for all
	control registers. */

	DISABLE_COUNTER(0)
	DISABLE_COUNTER(1)
	DISABLE_COUNTER(2)
	DISABLE_COUNTER(3)
	
	j	ra
	 nop
ENDFRAME(disable_counters_sb1)

