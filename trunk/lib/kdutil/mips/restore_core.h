// Bit of a kludge - this file gets included multiple times
// with various tweaks to the macro definitions to get the right
// code generated for R3K/MIPS32/MIPS64 variants
	.set	noreorder
	
	
   	/* restore TLB entries */
	lw		a1,mips_num_tlbs
   	li		t9,0
	la		t8,32*4(a0)
	mfc0	t0,CP0_SREG		# get status register
	move	t1,t0			# save copy of CPU status register
	ori		t0,t0,MIPS_SREG_IE
	xori	t0,t0,MIPS_SREG_IE
	mtc0	t0,CP0_SREG		# disable interrupts
	 nop
1:
#if defined(VARIANT_r3k)
	sll		t7,t9,MIPS3K_TLB_INX_INDEXSHIFT
	mtc0	t7,CP0_INDEX
#else	 
	mtc0	t9,CP0_INDEX
	lw		t3,0(t8)		
	mtc0	t3,CP0_PAGEMASK
	lw		t3,12(t8)			
	mtc0	t3,CP0_TLB_LO_1
#endif	
	lw		t3,4(t8)			
	mtc0	t3,CP0_TLB_HI
	sw		t3,8(t8)	
	mtc0	t3,CP0_TLB_LO_0	
	ssnop
	ssnop
	ssnop
	ssnop
	ssnop
	ssnop
	tlbwi						# write TLB entry
	addiu	t9,1
	bne		t9,a1,1b
	 addiu	t8,16

	j	ra
	 mtc0	t1,CP0_SREG		# restore original status register

	.set	reorder
