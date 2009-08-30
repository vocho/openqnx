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
# This file contains entry points for the 74xx machine check exception
#
	
	.include "asmoff.def"
	.include "ppc/util.ah"
	.include "../context.ah"

.section .text_kernel, "ax"

	.global __exc_machine_check74xx
__exc_machine_check74xx: 
	mfspr	%r4,SRR1

	loadi	%r3, PPC_SRR1_L1DC     # Crash if L1DC
	and. 	%r5,%r3,%r4
	bne		3f

	loadi	%r3, PPC_SRR1_L1IC     # check if L1IC error
	and.	%r5,%r3,%r4
	beq		1f

	mfspr	%r3, PPC700_SPR_HID0  # L1IC Flush cachE
	ori		%r5,%r3,PPC700_SPR_HID0_ICE+PPC700_SPR_HID0_ICFI
	mtspr	PPC700_SPR_HID0,%r5
	isync
	sync
	mtspr	PPC700_SPR_HID0,%r3   # restore HID0
	isync
	sync
	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTMACHCHK*65536)
	b	__exc         # Done send signal/crash

1:
	mfspr	%r5,PPC700_SPR_L3CR   # Check if L3 is set to I only
	loadi	%r3,0x400000          # L3IO
	and.	%r5,%r3,%r5
	beq		3f                    # Crash if I only is not set

	loadi	%r3,PPC_SRR1_MSS      # Crash if MSS not set
	and.	%r5,%r3,%r4
	beq             3f

	mfspr	%r5,PPC700_SPR_MSSSR0 # Check if L3 error
	loadi	%r3,0x18000           # L3 Data or Tag
	and.	%r5,%r3,%r5
	beq             3f            # Crash if not L3 error

	mfspr	%r5,PPC700_SPR_L3CR   # It is L3 Flush the cache
	ori		%r5,%r5,0x400         # L3I
	mtspr	PPC700_SPR_L3CR,%r5
2:
	mfspr	%r5,PPC700_SPR_L3CR
	andi.	%r5,%r5,0x400
	bne		2b                    # Loop until L3I is clear

	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTMACHCHK*65536)
	b	__exc
3:
	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTMACHCHK*65536)
	b	__hardcrash
