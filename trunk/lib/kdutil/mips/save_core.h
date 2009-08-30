// Bit of a kludge - this file gets included multiple times
// with various tweaks to the macro definitions to get the right
// code generated for R3K/MIPS32/MIPS64 variants
	.set	noreorder
	
	/* save CP0 state */
	SAVE_ONE   (0, 0)
//	SAVE_ONE   (1, 0)
	SAVE_ONE   (2, 0)
	SAVE_ONE_4K(3, 0)
	SAVE_ONE   (4, 0)
	SAVE_ONE_4K(5, 0)
	SAVE_ONE_4K(6, 0)
//	SAVE_ONE   (7, 0) 
	SAVE_ONE   (8, 0)
	SAVE_ONE_4K(9, 0)
	SAVE_ONE   (10, 0)
	SAVE_ONE   (11, 0)
	SAVE_ONE   (12, 0)
	SAVE_ONE   (13, 0)
	SAVE_ONE   (14, 0)
	SAVE_ONE   (15, 0)
	SAVE_ONE_4K(16, 0)
	SAVE_ONE_4K(17, 0)
	SAVE_ONE_4K(18, 0)
	SAVE_ONE_4K(19, 0)
#if !defined(VARIANT_32) && !defined(VARIANT_r3k)
	dmfc0	t0,CP0_XCONTEXT
	 nop
	sd		t0,20*4(a0)
#endif
//	SAVE_ONE   (21, 4)
//	SAVE_ONE   (22, 4)
//	SAVE_ONE   (23, 4)
//	SAVE_ONE   (24, 4)
//	SAVE_ONE   (25, 4)
	SAVE_ONE_4K(26, 4)
	SAVE_ONE_4K(27, 4)
	SAVE_ONE_4K(28, 4)
	SAVE_ONE_4K(29, 4)
	SAVE_ONE_4K(30, 4)
	
#if defined(VARIANT_sb1)
	mfc0    t0,CP0_IPL_LO
	nop
	sw      t0,(4*31 + 4)(a0)

	mfc0    t0,CP0_IPL_HI
	nop
	sw      t0,(4*31 + 8)(a0)

	mfc0    t0,CP0_INT_CTL
	nop
	sw      t0,(4*31 + 12)(a0)

	mfc0    t0,CP0_ERR_ADDR0
	nop
	sw      t0,(4*31 + 16)(a0)

	mfc0    t0,CP0_ERR_ADDR1
	nop
	sw      t0,(4*31 + 20)(a0)
#endif
   	/* dump out TLB entries */
	lw		a1,mips_num_tlbs
   	li		t9,0
	la		t8,37*4(a0)
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
#endif
	 nop
	tlbr						# read TLB entry
	 nop
	 nop
	 nop
	 nop
#if defined(VARIANT_r3k)
	move	t3,zero
#else
	mfc0	t3,CP0_PAGEMASK		# read page mask
	 nop
#endif
	mfc0	t4,CP0_TLB_HI		# read entry Hi
	 nop
	sw		t3,0(t8)			# tlb->mask = CP0_PAGEMASK
	sw		t4,4(t8)			# tlb->hi = CP0_TLB_HI
	mfc0	t3,CP0_TLB_LO_0		# read entry lo 0
	 nop
#if defined(VARIANT_r3k)
	move	t4,zero
#else
	mfc0	t4,CP0_TLB_LO_1		# read entry lo 1
	 nop
#endif
	sw		t3,8(t8)			# tlb->lo0 = CP0_TLB_LO_0
	sw		t4,12(t8)			# tlb->lo1 = CP0_TLB_LO_1
	addiu	t9,1
	bne		t9,a1,1b
	 addiu	t8,16
	j	ra
	 mtc0	t1,CP0_SREG		# restore original status register

	.set	reorder
