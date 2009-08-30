#include <mips/asm.h>
#include <mips/cpu.h>

	.set	noreorder
	
.text
	
/*
 * r4k_update_tlb(hi, lo0, lo1, pgmask)
 *	Update the TLB with an entry, checking to see if there already
 *  one for the virtual address and replacing it if need be - also
 *  allows for non-4K pagemask values.
 *
 */
FRAME(r4k_update_tlb,sp,0,ra)
	DISABLEINTERRUPTS(t0,t1)
	mfc0	t3,CP0_TLB_HI		# save Entry Hi
	mtc0	a0,CP0_TLB_HI		# set Entry Hi register
	 ssnop; ssnop; ssnop; ssnop
	tlbp						# probe tlb
	 ssnop; ssnop
	mfc0	v0,CP0_INDEX		# get index register
	 ssnop
#if !defined(VARIANT_r3k)
	mtc0	a3,CP0_PAGEMASK 	# set new pagemask
	 ssnop
	mtc0	a2,CP0_TLB_LO_1		# write Entry Lo 1 register
	 or		t6,a1,a2
#endif
	mtc0	a1,CP0_TLB_LO_0		# write Entry Lo 0 register
	 ssnop
	mtc0	a0,CP0_TLB_HI		# set new Entry hi register
	 ssnop; ssnop; ssnop
	bltz	v0,1f				# branch if neg => TLB probe failed
	  nop
	tlbwi						# write TLB entry, indexed
	 b		2f
	  nop
1:
#if defined(VARIANT_r3k)
	beqz	a1,2f				# skip if just trying to kill entry
#else
	beqz	t6,2f				# skip if just trying to kill entry
#endif
	 nop
	tlbwr						# write TLB entry, random
	 ssnop; ssnop
2:
	mtc0	t3,CP0_TLB_HI		# restore original entry hi
	 ssnop
 
	RESTOREINTERRUPTS(t0,t1)
	j	ra			# return
	 nop
ENDFRAME(r4k_update_tlb)
