#
# traps.s - GDB exception handling code
#

	.include "asmoff.def"
	.include "ppc/util.ah"

.ifdef PPC_CPUOP_ENABLED
	.cpu 403
.endif

	.extern _stack_top
	.extern _stack_base
	.extern ppc_signal_handler
	.extern watch_entry
	.extern msg_entry
	.extern handle_exception
	.extern kerdebug_reg
	.extern spinlock
	.extern msr_bits_off
	
	.set SRR0,	0x01a
	.set SRR1,	0x01b
	.set SRR2,	0x3de
	.set SRR3,	0x3df

	.set SRR1_TRAP_FLG, 0x00020000	 	
	.set SRR1_PRIVINSTRUCTION_FLG, 0x00010000	 	
	.set SRR1_ILLINSTRUCTION_FLG, 0x00008000	 	
	.set SRR1_FLOATINPOINT_FLG, 0x00004000	 	
	
.macro	SPINLOCK,spin,reg1,reg2
	lis		&reg1,&spin@ha
	la		&reg1,&spin@l(&reg1)
	# Note: test spinlock first without using lwarx, to avoid jamming the bus
1:
	lwz		&reg2,0(&reg1)
	cmpwi	&reg2,0
	bne-	1b
	lwarx	&reg2,0,&reg1
	cmpwi	&reg2,0
	bne-	1b
	li		&reg2,1
	stwcx.	&reg2,0,&reg1
	bne-	1b
.endm

.macro	SPINUNLOCK,spin,reg1,reg2
	lis		&reg1,&spin@ha
	li		&reg2,0
	stw		&reg2,&spin@l(&reg1)
	sync
.endm

#
# On exit from this macro:
#			R1	- debugger stack pointer
#			R30 - register save area
#
# NOTE: Don't damage SPRG3 - it has the CPU number on an SMP system.
#
# spinlock is used to allow only one CPU into the kernel debugger at a time
#
.macro ENTERDBG, iar, msr, sf
	mtsprg	0,%r30
	mtsprg	1,%r31
	mfcr	%r30
	mtsprg	2,%r30
	SPINLOCK spinlock,%r30,%r31
	lis		%r30,kerdebug_reg@ha
	la		%r30,kerdebug_reg@l(%r30)
	stw		%r1,REG_GPR+1*PPCINT(%r30)
	lis		%r1,_stack_top@ha	
	lwz		%r1,_stack_top@l(%r1)
3:
	stw		%r2,REG_GPR+2*PPCINT(%r30)
	mfsprg	%r2,0
	stw		%r2,REG_GPR+30*PPCINT(%r30)
	mfsprg	%r2,1
	stw		%r2,REG_GPR+31*PPCINT(%r30)
	mfsprg	%r2,2
	stw		%r2,REG_CR(%r30)
	mfspr	%r2,&iar
	stw		%r2,REG_IAR(%r30)
	mfspr	%r2,&msr
	stw		%r2,REG_MSR(%r30)
.if	&sf
 	rldicl	%r2,%r2,32,32
 	stw		%r2,REG_MSR_U(%r30)
.endif
	stw		%r0,REG_GPR+0*PPCINT(%r30)
	stw		%r3,REG_GPR+3*PPCINT(%r30)
	stw		%r4,REG_GPR+4*PPCINT(%r30)
	stw		%r5,REG_GPR+5*PPCINT(%r30)
	stw		%r6,REG_GPR+6*PPCINT(%r30)
	stw		%r7,REG_GPR+7*PPCINT(%r30)
	stw		%r8,REG_GPR+8*PPCINT(%r30)
	stw		%r9,REG_GPR+9*PPCINT(%r30)
	stw		%r10,REG_GPR+10*PPCINT(%r30)
	stw		%r11,REG_GPR+11*PPCINT(%r30)
	stw		%r12,REG_GPR+12*PPCINT(%r30)
	stw		%r13,REG_GPR+13*PPCINT(%r30)
	stw		%r14,REG_GPR+14*PPCINT(%r30)
	stw		%r15,REG_GPR+15*PPCINT(%r30)
	stw		%r16,REG_GPR+16*PPCINT(%r30)
	stw		%r17,REG_GPR+17*PPCINT(%r30)
	stw		%r18,REG_GPR+18*PPCINT(%r30)
	stw		%r19,REG_GPR+19*PPCINT(%r30)
	stw		%r20,REG_GPR+20*PPCINT(%r30)
	stw		%r21,REG_GPR+21*PPCINT(%r30)
	stw		%r22,REG_GPR+22*PPCINT(%r30)
	stw		%r23,REG_GPR+23*PPCINT(%r30)
	stw		%r24,REG_GPR+24*PPCINT(%r30)
	stw		%r25,REG_GPR+25*PPCINT(%r30)
	stw		%r26,REG_GPR+26*PPCINT(%r30)
	stw		%r27,REG_GPR+27*PPCINT(%r30)
	stw		%r28,REG_GPR+28*PPCINT(%r30)
	stw		%r29,REG_GPR+29*PPCINT(%r30)
	mfctr	%r3
	stw		%r3,REG_CTR(%r30)
	mflr	%r3
	stw		%r3,REG_LR(%r30)
	mfxer	%r3
	stw		%r3,REG_XER(%r30)
.endm

.macro EXITDBG, sf
	lis		%r30,kerdebug_reg@ha
	la		%r30,kerdebug_reg@l(%r30)
   	lwz		%r3,REG_IAR(%r30)
	mtspr	SRR0,%r3
	lwz		%r3,REG_MSR(%r30)
.if &sf
	lwz		%r4,REG_MSR_U(%r30)
	rldicr	%r4,%r4,32,31
	or		%r3,%r3,%r4
.endif	
	mtspr	SRR1,%r3
	lwz		%r3,REG_CTR(%r30)
	mtctr	%r3
	lwz		%r3,REG_LR(%r30)
	mtlr	%r3
	lwz		%r3,REG_XER(%r30)
	mtxer	%r3
	lwz		%r0,REG_GPR+0*PPCINT(%r30)
	lwz		%r1,REG_GPR+1*PPCINT(%r30)
	lwz		%r2,REG_GPR+2*PPCINT(%r30)
	lwz		%r3,REG_GPR+3*PPCINT(%r30)
	lwz		%r4,REG_GPR+4*PPCINT(%r30)
	lwz		%r5,REG_GPR+5*PPCINT(%r30)
	lwz		%r6,REG_GPR+6*PPCINT(%r30)
	lwz		%r7,REG_GPR+7*PPCINT(%r30)
	lwz		%r8,REG_GPR+8*PPCINT(%r30)
	lwz		%r9,REG_GPR+9*PPCINT(%r30)
	lwz		%r10,REG_GPR+10*PPCINT(%r30)
	lwz		%r11,REG_GPR+11*PPCINT(%r30)
	lwz		%r12,REG_GPR+12*PPCINT(%r30)
	lwz		%r13,REG_GPR+13*PPCINT(%r30)
	lwz		%r14,REG_GPR+14*PPCINT(%r30)
	lwz		%r15,REG_GPR+15*PPCINT(%r30)
	lwz		%r16,REG_GPR+16*PPCINT(%r30)
	lwz		%r17,REG_GPR+17*PPCINT(%r30)
	lwz		%r18,REG_GPR+18*PPCINT(%r30)
	lwz		%r19,REG_GPR+19*PPCINT(%r30)
	lwz		%r20,REG_GPR+20*PPCINT(%r30)
	lwz		%r21,REG_GPR+21*PPCINT(%r30)
	lwz		%r22,REG_GPR+22*PPCINT(%r30)
	lwz		%r23,REG_GPR+23*PPCINT(%r30)
	lwz		%r24,REG_GPR+24*PPCINT(%r30)
	lwz		%r25,REG_GPR+25*PPCINT(%r30)
	lwz		%r26,REG_GPR+26*PPCINT(%r30)
	lwz		%r27,REG_GPR+27*PPCINT(%r30)
	lwz		%r28,REG_GPR+28*PPCINT(%r30)
	lwz		%r29,REG_GPR+29*PPCINT(%r30)
	lwz		%r31,REG_GPR+31*PPCINT(%r30)
	mtsprg	1,%r31
	lwz		%r31,REG_CR(%r30)
	mtcrf	0xff,%r31
	# Can't modify condition codes after this point...
	lwz		%r30,REG_GPR+30*PPCINT(%r30)
	mtsprg	2,%r30								
	SPINUNLOCK	spinlock,%r30,%r31
	mfsprg	%r31,1
	mfsprg	%r30,2
.if &sf
	rfid
.else
	rfi
.endif
.endm

pgm_fault_codes400:
0:	.long SIGSEGV + (SEGV_MAPERR*256) + (FLTPRIV*65536)	
1:	.long SIGSEGV + (SEGV_MAPERR*256) + (FLTBOUNDS*65536)
2:	.long SIGBUS + (BUS_OBJERR*256) + (FLTBUSERR*65536)
3:	.long SIGBUS + (BUS_OBJERR*256) + (FLTBUSTIMOUT*65536)
4:	.long SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)
5:	.long SIGILL + (ILL_ILLOPC*256) + (FLTPRIV*65536)
6:	.long SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)
##NYI: need to fill in these for other 400 series processors
7:	.long 0
8:	.long 0
9:	.long 0
10:	.long 0
11:	.long 0
12:	.long 0
13:	.long 0
14:	.long 0
15:	.long 0
16:	.long 0
17:	.long 0
18:	.long 0
19:	.long 0
20:	.long 0
21:	.long 0
22:	.long 0
23:	.long 0
24:	.long 0
25:	.long 0
26:	.long 0
27:	.long 0
28:	.long 0
29:	.long 0
30:	.long 0
31:	.long 0
	
	.global exc_alignment
exc_alignment:
	ENTERDBG SRR0,SRR1,0

	loadi	%r3,SIGBUS + (BUS_ADRALN*256) + (FLTACCESS*65536)
	b		exc
	
	
	.global exc_machine_check800
exc_machine_check800:
	ENTERDBG SRR0,SRR1,0
	#
	# About all we can do is guess that it was a bad address :-(
	#
	loadi	%r3,SIGSEGV + (SEGV_MAPERR*256) + (FLTBOUNDS*65536)
	b exc
	
	.global exc_dbreak800
exc_dbreak800:
	.global exc_ibreak800
exc_ibreak800:
	.global exc_pbreak800
exc_pbreak800:
	.global exc_devport800
exc_devport800:
	.global exc_ibreak603
exc_ibreak603:
	ENTERDBG SRR0,SRR1,0
	mfbar	%r4
	loadi	%r3,SIGCODE_KERNEL + SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)
	b exc
	
	.global exc_trace603
exc_trace603:
	.global exc_trace800
exc_trace800:
	ENTERDBG SRR0,SRR1,0
	loadi	%r3,SIGTRAP + (TRAP_TRACE*256) + (FLTTRACE*65536)	
	b		exc

	.global exc_program800
exc_program800:
	.global exc_program603
exc_program603:
	ENTERDBG SRR0,SRR1,0
	lwz		%r4,REG_MSR(%r30)
	bittst	%r3, %r4, SRR1_TRAP_FLG 	
	bne	1f
	bittst	%r3, %r4, SRR1_PRIVINSTRUCTION_FLG		
	bne 2f
	bittst	%r3, %r4, SRR1_ILLINSTRUCTION_FLG
	bne 3f
	bittst	%r3, %r4, SRR1_FLOATINPOINT_FLG
	bne 4f
1:	#trap
	loadi	%r3,SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)	
	b		exc
2:	#privileged instruction 
	loadi	%r3,SIGILL + (5*256) + (FLTPRIV*65536)
	b		exc
3:	#illlegal instruction
	loadi	%r3,SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)
	b		exc
4:	#Floating Point
	loadi	%r3,SIGFPE + (FPE_NOFPU*256) + (FLTFPE*65536)
	b		exc

	.global exc_data_access400
exc_data_access400:
	.global exc_data_access_booke
exc_data_access_booke:
	ENTERDBG SRR0,SRR1,0
	loadi	%r3,SIGSEGV + (ILL_ILLOPC *256) + (FLTILL *65536)	
	b		exc
	
	.global exc_program400
exc_program400:
	ENTERDBG SRR0,SRR1,0
ins_mach_check:
	mfspr	%r4,PPC400_SPR_ESR			# get reason for instruction machine chk/program
	cntlzw	%r4,%r4
	slwi	%r4,%r4,2
	lis		%r5,pgm_fault_codes400@ha	
	la		%r5,pgm_fault_codes400@l(%r5)
	lwzx	%r3,%r4,%r5
	b		exc
	
	.global exc_debug400
exc_debug400:
	ENTERDBG SRR2,SRR3,0
	lwz		%r3,REG_MSR(%r30)
	bitclr	%r3,%r3,PPC_MSR_DE
	stw		%r3,REG_MSR(%r30)
	#
	# Have to spiffy this code up if we start using the debug control
	# register for things other than single stepping
	#
	loadi	%r3,SIGTRAP + (TRAP_TRACE*256) + (FLTTRACE*65536)	
	b		exc

	.global exc_machine_check403
exc_machine_check403:
	ENTERDBG SRR2,SRR3,0
	mfdcr	%r4,PPC400_DCR_BEAR		# get address responsible
	mfdcr	%r3,PPC400_DCR_BESR		# data or instruction?
	bittst	%r0,%r3,PPC403_BESR_DSES
	beq		ins_mach_check
	# handle a data fault
	li		%r5,0			# clear error condition
	mtdcr	PPC400_DCR_BESR,%r5
	dcbi	0,%r4			# clear the invalid data from the cache
	rlwinm	%r3,%r3,7,28,29	# extract ET field & prepare for indexing	
	lis		%r5,pgm_fault_codes400@ha	
	la		%r5,pgm_fault_codes400@l(%r4)
	lwzx	%r3,%r3,%r5		# get fault code
	b		exc
	
	.global exc_debug_booke
exc_debug_booke:
	ENTERDBG PPCBKE_SPR_CSRR0,PPCBKE_SPR_CSRR1,0
	lwz		%r3,REG_MSR(%r30)
	bitclr	%r3,%r3,PPC_MSR_DE
	stw		%r3,REG_MSR(%r30)
	#
	# Have to spiffy this code up if we start using the debug control
	# register for things other than single stepping
	#
	loadi	%r3,SIGTRAP + (TRAP_TRACE*256) + (FLTTRACE*65536)	
	b		exc


pgm_fault_codes_booke:
0:	.long 0
1:	.long 0
2:	.long 0
3:	.long 0
4:	.long SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)
5:	.long SIGILL + (ILL_ILLOPC*256) + (FLTPRIV*65536)
6:	.long SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)
7:	.long SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)
8:	.long 0
9:	.long 0
10:	.long 0
11:	.long 0
12:	.long SIGILL + (ILL_COPROC*256) + (FLTILL*65536)
13:	.long SIGILL + (ILL_COPROC*256) + (FLTILL*65536)
14:	.long 0
15:	.long 0
16:	.long 0
17:	.long 0
18:	.long 0
19:	.long 0
20:	.long 0
21:	.long 0
22:	.long 0
23:	.long 0
24:	.long 0
25:	.long 0
26:	.long 0
27:	.long 0
28:	.long 0
29:	.long 0
30:	.long 0
31:	.long 0
	
	.global exc_machine_check_e500
exc_machine_check_e500:
	ENTERDBG PPCE500_SPR_MCSRR0,PPCE500_SPR_MCSRR1,0
	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTBUSERR*65536)
	b		exc
	
	.global exc_machine_check_booke
exc_machine_check_booke:
	ENTERDBG PPCBKE_SPR_CSRR0,PPCBKE_SPR_CSRR1,0
	loadi	%r3,SIGBUS + (BUS_OBJERR*256) + (FLTBUSERR*65536)
	b		exc
	
	.global exc_program_booke
exc_program_booke:
	ENTERDBG SRR0,SRR1,0
	mfspr	%r4,PPCBKE_SPR_ESR	# get reason for instruction machine chk/program
	cntlzw	%r4,%r4
	slwi	%r6,%r4,2
	lis		%r5,pgm_fault_codes_booke@ha	
	la		%r5,pgm_fault_codes_booke@l(%r5)
	lwzx	%r3,%r6,%r5
	b		exc
	
	
exc:
	mfmsr	%r5
	bitset	%r5,%r5,PPC_MSR_IR|PPC_MSR_DR
	lis		%r4,msr_bits_off@ha
	lwz		%r4,msr_bits_off@l(%r4)
	andc	%r5,%r5,%r4
	mtmsr	%r5
	isync
	lis		%r13,_SDA_BASE_@ha	
	lis		%r2,_SDA2_BASE_@ha	
	la		%r13,_SDA_BASE_@l(%r13)
	la		%r2,_SDA2_BASE_@l(%r2)
	bitset	%r4,%r3,SIGCODE_FATAL
	mr		%r5,%r30
	li		%r3,0
	bl 		handle_exception
	EXITDBG 0
	

.ifdef PPC_CPUOP_ENABLED
	.cpu ppc64
.endif


	.global exc_alignment900
exc_alignment900:
	ENTERDBG SRR0,SRR1,1
	loadi	%r3,SIGBUS + (BUS_ADRALN*256) + (FLTACCESS*65536)
	b		exc64
	
	.global exc_trace900
exc_trace900:
	ENTERDBG SRR0,SRR1,1
	loadi	%r3,SIGTRAP + (TRAP_TRACE*256) + (FLTTRACE*65536)	
	b		exc64

	.global exc_program900
exc_program900:
	ENTERDBG SRR0,SRR1,1
	lwz		%r4,REG_MSR(%r30)
	bittst	%r3, %r4, SRR1_TRAP_FLG 	
	bne	1f
	bittst	%r3, %r4, SRR1_PRIVINSTRUCTION_FLG		
	bne 2f
	bittst	%r3, %r4, SRR1_ILLINSTRUCTION_FLG
	bne 3f
	bittst	%r3, %r4, SRR1_FLOATINPOINT_FLG
	bne 4f
1:	#trap
	loadi	%r3,SIGTRAP + (TRAP_BRKPT*256) + (FLTBPT*65536)	
	b		exc64
2:	#privileged instruction 
	loadi	%r3,SIGILL + (5*256) + (FLTPRIV*65536)
	b		exc64
3:	#illlegal instruction
	loadi	%r3,SIGILL + (ILL_ILLOPC*256) + (FLTILL*65536)
	b		exc64
4:	#Floating Point
	loadi	%r3,SIGFPE + (FPE_NOFPU*256) + (FLTFPE*65536)
	b		exc64
	
	
exc64:
	mfmsr	%r5
	bitset	%r5,%r5,PPC_MSR_IR|PPC_MSR_DR
	lis		%r4,msr_bits_off@ha
	lwz		%r4,msr_bits_off@l(%r4)
	andc	%r5,%r5,%r4
	rlwinm	%r5,%r5,0,0,31	# turn off SF bit
	mtmsrd	%r5
	isync
	lis		%r13,_SDA_BASE_@ha	
	lis		%r2,_SDA2_BASE_@ha	
	la		%r13,_SDA_BASE_@l(%r13)
	la		%r2,_SDA2_BASE_@l(%r2)
	bitset	%r4,%r3,SIGCODE_FATAL
	mr		%r5,%r30
	li		%r3,0
	bl 		handle_exception
	EXITDBG 1

#/*
# * ulong_t outside_fault_entry (struct kdebug *entry, ulong_t sigcode, CPU_REGISTERS *ctx)
# *
# * Called by the kernel via syspage kdebug_entry to
# * give GDB a chance to handle the exception. 
# */
	.global	outside_fault_entry
outside_fault_entry:
	stwu 	%r1,-32(%r1)
	mflr 	%r0
	stw 	%r2,12(%r1)
	stw 	%r13,16(%r1)
	stw 	%r31,20(%r1)
	mfmsr	%r31
	stw		%r31,24(%r1)
	lis		%r2,msr_bits_off@ha
	lwz		%r2,msr_bits_off@l(%r2)
	andc	%r31,%r31,%r2
	mtmsr	%r31
	isync
	stw 	%r0,36(%r1)
	mr		%r31,%r1
	lis		%r1,_stack_top@ha
	lwz		%r1,_stack_top@l(%r1)
	lis		%r13,_SDA_BASE_@ha				# load up small data area ptrs
	lis		%r2,_SDA2_BASE_@ha
	la		%r13,_SDA_BASE_@l(%r13)
	la		%r2,_SDA2_BASE_@l(%r2)
	SPINLOCK spinlock,%r6,%r7
	bl 		handle_exception
	SPINUNLOCK spinlock,%r6,%r7
	lwz 	%r11,0(%r31)
	lwz 	%r0,4(%r11)
	mtlr 	%r0
	lwz 	%r2,-20(%r11)
	lwz 	%r13,-16(%r11)
	lwz 	%r31,-12(%r11)
	lwz 	%r0,-8(%r11)
	mtmsr	%r0
	isync
	mr 		%r1,%r11
	blr


#/*
# * int outside_watch_entry(struct kdebug *kdebug, paddr_t start)
# *  Stop at the given address
# */
	.global outside_watch_entry
outside_watch_entry:
	stwu 	%r1,-32(%r1)
	mflr 	%r0
	stw 	%r2,12(%r1)
	stw 	%r13,16(%r1)
	stw 	%r31,20(%r1)
	stw 	%r0,36(%r1)
	mr		%r31,%r1
	lis		%r1,_stack_top@ha
	lwz		%r1,_stack_top@l(%r1)
	lis		%r13,_SDA_BASE_@ha				# load up small data area ptrs
	lis		%r2,_SDA2_BASE_@ha
	la		%r13,_SDA_BASE_@l(%r13)
	la		%r2,_SDA2_BASE_@l(%r2)
	SPINLOCK spinlock,%r6,%r7
	bl 		watch_entry
	SPINUNLOCK spinlock,%r6,%r7
	lwz 	%r11,0(%r31)
	lwz 	%r0,4(%r11)
	mtlr 	%r0
	lwz 	%r2,-20(%r11)
	lwz 	%r13,-16(%r11)
	lwz 	%r31,-12(%r11)
	mr 		%r1,%r11
	blr


#/*
# * int outside_msg_entry(const char *msg, unsigned len)
# *  Display the given message
# */
	.global outside_msg_entry
outside_msg_entry:
	stwu 	%r1,-32(%r1)
	mflr 	%r0
	stw 	%r2,12(%r1)
	stw 	%r13,16(%r1)
	stw 	%r31,20(%r1)
	stw 	%r0,36(%r1)
	mr		%r31,%r1
	lis		%r1,_stack_top@ha
	lwz		%r1,_stack_top@l(%r1)
	lis		%r13,_SDA_BASE_@ha				# load up small data area ptrs
	lis		%r2,_SDA2_BASE_@ha
	la		%r13,_SDA_BASE_@l(%r13)
	la		%r2,_SDA2_BASE_@l(%r2)
	SPINLOCK spinlock,%r6,%r7
	bl 		msg_entry
	SPINUNLOCK spinlock,%r6,%r7
	lwz 	%r11,0(%r31)
	lwz 	%r0,4(%r11)
	mtlr 	%r0
	lwz 	%r2,-20(%r11)
	lwz 	%r13,-16(%r11)
	lwz 	%r31,-12(%r11)
	mr 		%r1,%r11
	blr

#/*
# * void outside_update_plist(struct kdebug_entry *modified)
# *  A process has been added/deleted to the kernel debugger's list. The
# *  parm is the entry that's been added/deleted.
# */
	.global outside_update_plist
outside_update_plist:
	blr


#/*
# * int outside_timer_reload(struct syspage_entry *, struct qtime_entry *)
# *  Check for an async stop. Run on kernel stack.
# */
	.global outside_timer_reload
outside_timer_reload:
	stwu 	%r1,-16(%r1)
	mflr 	%r0
	stw 	%r2,8(%r1)
	stw 	%r13,12(%r1)
	stw 	%r0,20(%r1)
	lis		%r13,_SDA_BASE_@ha				# load up small data area ptrs
	lis		%r2,_SDA2_BASE_@ha
	la		%r13,_SDA_BASE_@l(%r13)
	la		%r2,_SDA2_BASE_@l(%r2)
	bl 		kdebug_timer_reload
	lwz 	%r0,20(%r1)
	mtlr 	%r0
	lwz 	%r2,8(%r1)
	lwz 	%r13,12(%r1)
	addi	%r1,%r1,16
	blr
