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


//NYI: Hack for SB-1 part
#define SB1_BUG

	.extern	dcache_lines,4
	.extern	dcache_lsize,4
	.extern icache_lines,4
	.extern icache_lsize,4
	.extern icache_lines_per_page,4
	.extern dcache_lines_per_page,4
	
/*
 * For index operations in which the tags are being inspected,
 * have to look at both sets of the caches. The set bit is specified
 * by bit 13 of the virtual address. In order to ensure that both
 * sets of tags are inspected, the virtual address will be XOR-ed
 * with SET_BIT to either set or clear the bit, depending upon the
 * virtual address, if the first set checked failed the compare.
 */
#define SET_BIT         0x2000

#ifdef VARIANT_tx79
	#define	SYNCP			sync.p
	#define	SYNCL			sync.l
	#define I_IDX_INV		7
	#define I_HIT_INV		11
	#define D_HIT_WR_INV	24
	#define D_HIT_INV		26
#else
#ifdef SB1_BUG
	#define	SYNCP	sync
	#define	SYNCL	sync
#else
	#define	SYNCP	
	#define	SYNCL
#endif
	#define I_IDX_INV		0
	#define I_HIT_INV		16
	#define D_HIT_WR_INV	21
	#define D_HIT_INV		17
#endif

/*
 * r4k_purge_icache_full()
 *	Purge all of the icache. 
 */
FRAME(r4k_purge_icache_full,sp,0,ra)

	.set noreorder
	.set mips3

	lw	t1,icache_lines
	lw	t2,icache_lsize
	
   	li	t0,MIPS_R4K_K0BASE
1:
	/* index invalidate, icache */
	SYNCP
	cache	I_IDX_INV,0(t0)
	SYNCP
	nop
	addiu	t1,t1,-1
	bgtz	t1,1b
	 addu	t0,t0,t2
#ifdef SB1_BUG
sync
sync
#endif

	j	ra
	 nop

	.set mips2
	.set reorder

ENDFRAME(r4k_purge_icache_full)


/*
 * r4k_flush_dcache_page()
 *	Flush a page out of the primary cache,
 * 	invalidating all this page's lines and
 *	writing back lines as necessary. This 
 * 	routine takes the KSEG0 address of the
 *	start of the page to be evicted.
 */
FRAME(r4k_flush_dcache_page,sp,0,ra)

	.set noreorder
	.set mips3

	lw	t1,dcache_lines_per_page
	lw	t2,dcache_lsize
	
	/* Save status reg and disable interrupts. */
	DISABLEINTERRUPTS(t9,t8)

1:
	blez	t1,2f
	 nop	

	/* hit writeback invalidate, dcache */
	SYNCL
#ifdef SB1_BUG
lw zero,0(a0)
#endif
	cache	D_HIT_WR_INV,0(a0)
	SYNCL
	nop
	addiu	t1,t1,-1
    b       1b
	 addu	a0,a0,t2
2:	

	/* restore original status register */
	RESTOREINTERRUPTS(t9,t8)
	j	ra
	 nop

	.set mips2
	.set	reorder
ENDFRAME(r4k_flush_dcache_page)

/*
 * r4k_flush_dcache_page_hitwa()
 *	Flush a page out of the primary cache,
 * 	invalidating all this page's lines and
 *	writing back lines as necessary. This 
 * 	routine takes the upper 24 bits of 
 *	the physical address to be evicted since
 *	it has to workaround the chip bug 
 *	involving HIT_* cacheops.
 */
FRAME(r4k_flush_dcache_page_hitwa,sp,0,ra)

	.set noreorder
	.set mips3

	lui	t0,0x8000			# use kseg0 as flush address

	lw	t1,dcache_lines
	lw	t2,dcache_lsize
	
	/* Save status reg and disable interrupts. */
	DISABLEINTERRUPTS(t9,t8)

	/* with interrupts disabled save CP0_TAGLO, ECC[inkernel] */
	mfc0	t7,CP0_TAGLO
	 nop	
	mfc0	t6,CP0_ECC

1:
	blez	t1,4f
	 nop	

	/* index load tag, dcache */
	cache	5,0(t0)
	nop	
	mfc0	t3,CP0_TAGLO
	 nop
	/* restore CP0_TAGLO, ECC[inkernel] */
	mtc0	t7,CP0_TAGLO
	nop
	mtc0	t6,CP0_ECC
	srl	t3,t3,MIPS_TAGLO_PTAGLO_SHIFT
	bne	t3,a0,3f			# ptag mismatch ?
	 addiu	t1,t1,-1
	
2:
	/* got a tag match- writeback-invalidate the line */
	cache	1,0(t0)
	nop
	/* restore CP0_TAGLO, ECC[inkernel] */
	mtc0	t7,CP0_TAGLO
	nop
	mtc0	t6,CP0_ECC
	addu	t0,t0,t2
	b	1b
	 nop
3:
	/* first set missed, check the second */
	xori    t0,t0,SET_BIT
	cache   5,0(t0)
	nop
	mfc0    t3,CP0_TAGLO
	 nop
	/* restore CP0_TAGLO, ECC[inkernel] */
	mtc0	t7,CP0_TAGLO
	nop
	mtc0	t6,CP0_ECC
	srl     t3,t3,MIPS_TAGLO_PTAGLO_SHIFT
	beq     t3,a0,2b                        # ptag match ?
	 nop
	b       1b
	 addu   t0,t0,t2
4:	
	/* restore CP0_TAGLO */
	mtc0	t7,CP0_TAGLO
	nop
	mtc0	t6,CP0_ECC

	/* restore original status register */
	RESTOREINTERRUPTS(t9,t8)
	j	ra	
	 nop

	.set mips2
	.set	reorder
ENDFRAME(r4k_flush_dcache_page_hitwa)


/*
 * r4k_purge_dcache_page()
 *	Purge a page out of the primary cache,
 * 	invalidating all this page's lines. This
 * 	routine takes the KSEG0 address of the
 *	page to be evicted.
 */
FRAME(r4k_purge_dcache_page,sp,0,ra)

	.set noreorder
	.set mips3

	lw	t1,dcache_lines_per_page
	lw	t2,dcache_lsize
	
	/* Save status reg and disable interrupts. */
	DISABLEINTERRUPTS(t9,t8)

1:
	blez	t1,2f
	 nop	

	/* hit invalidate, dcache */
	SYNCL
#ifdef SB1_BUG
lw zero,0(a0)
#endif
	cache	D_HIT_INV,0(a0)
	SYNCL
	nop	
	addiu	t1,t1,-1
	b       1b
	 addu	a0,a0,t2
2:	

	/* restore original status register */
	RESTOREINTERRUPTS(t9,t8)
	j	ra	
	 nop

	.set mips2
	.set	reorder
ENDFRAME(r4k_purge_dcache_page)

/*
 * r4k_purge_dcache_page_hitwa()
 *	Purge a page out of the primary cache,
 * 	invalidating all this page's lines. This
 * 	routine takes the upper 24 bits of 
 *	the physical address to be evicted
 *	since it has to manually inspect the
 *	tags because of the HIT_* chip bug.
 */
FRAME(r4k_purge_dcache_page_hitwa,sp,0,ra)

	.set noreorder
	.set mips3

	lui	t0,0x8000			# use kseg0 as flush address

	lw	t1,dcache_lines
	lw	t2,dcache_lsize
	
	/* Save status reg and disable interrupts. */
	DISABLEINTERRUPTS(t9,t8)

	mtc0	zero,CP0_TAGHI			# for store tag cacheops

	/* with interrupts disabled save CP0_TAGLO, ECC[inkernel] */
	mfc0	t7,CP0_TAGLO
	 nop	
	mfc0	t6,CP0_ECC

1:
	blez	t1,4f
	 nop	

	/* index load tag, dcache */
	cache	5,0(t0)
	nop	
	mfc0	t3,CP0_TAGLO
	nop
	/* restore CP0_TAGLO, ECC[inkernel] */
	mtc0	t7,CP0_TAGLO
	nop
	mtc0	t6,CP0_ECC
	srl	t3,t3,MIPS_TAGLO_PTAGLO_SHIFT
	bne	t3,a0,3f			# ptag mismatch ?
	 addiu	t1,t1,-1
	
2:
	/* got a tag match- manually invalidate the tags */
	lw	zero,0(t0)		# Cause TLB miss while TAGLO/ECC good
	mtc0	zero,CP0_TAGLO
	nop
	cache	9,0(t0)			# index store tags, dcache
	nop
	/* restore CP0_TAGLO, ECC[inkernel] */
	mtc0	t7,CP0_TAGLO
	nop
	mtc0	t6,CP0_ECC
	addu	t0,t0,t2
	b	1b
	 nop
3:
	/* first set missed- check the second */
	xori    t0,t0,SET_BIT
	cache   5,0(t0)
	nop
	mfc0    t3,CP0_TAGLO
	 nop
	/* restore CP0_TAGLO, ECC[inkernel] */
	mtc0	t7,CP0_TAGLO
	nop
	mtc0	t6,CP0_ECC
	srl     t3,t3,MIPS_TAGLO_PTAGLO_SHIFT
	beq     t3,a0,2b                        # ptag match ?
	 nop
	b       1b
	 addu   t0,t0,t2

4:	
	/* restore CP0_TAGLO, ECC[inkernel] */
	mtc0	t7,CP0_TAGLO
	nop
	mtc0	t6,CP0_ECC

	/* restore original status register */
	RESTOREINTERRUPTS(t9,t8)
	j	ra	
	 nop
	
	.set mips2
	.set	reorder

ENDFRAME(r4k_purge_dcache_page_hitwa)
