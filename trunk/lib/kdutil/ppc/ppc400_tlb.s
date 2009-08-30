	.include "asmoff.def"
	.include "ppc/util.ah"
	
.macro TLBWE RS, RA, WS
	.long	(31 << 26) + (&RS << 21) + (&RA << 16) + (&WS << 11) + (978 << 1)
.endm
	
.macro TLBRE RS, RA, WS
	.long	(31 << 26) + (&RS << 21) + (&RA << 16) + (&WS << 11) + (946 << 1)
.endm

routine_start get_tlb400, 1
	TLBRE	5,3,1
	stw		%r5,0(%r4)
	mfmsr	%r6
	rlwinm	%r7,%r6,0,17,15
	mtmsr	%r7
	mfspr	%r0,PPC400_SPR_PID
	TLBRE	5,3,0
	stw		%r5,4(%r4)
	mfspr	%r5,PPC400_SPR_PID
	mtspr	PPC400_SPR_PID,%r0
	mtmsr	%r6
	stw		%r5,8(%r4)
	blr
routine_end get_tlb400

routine_start clr_tlb400, 1
	li		%r0,0
	mtspr	PPC400_SPR_PID,%r0
	TLBWE	0,3,1
	TLBWE	0,3,0
	blr
routine_end clr_tlb400
