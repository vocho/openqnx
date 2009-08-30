/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */



/*
 *  sh/cpu.h
 *
 * 
 * Note that the original definitions in this file came from the 7750/7751 days.
 * If a constant begins SH_, it might or might not be applicable for later hardware.
 * Look out for SH77xx_ definitions, and also have a look in sh4acpu.h.  Your
 * hardware manual is your friend!
 */

#ifndef __SH_CPU_H_INCLUDED
#define __SH_CPU_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __SH_CONTEXT_H_INCLUDED
#include _NTO_HDR_(sh/context.h)
#endif

#define SH_STRINGNAME	"sh"

/*
 * Floating Point Status and Control Register
 */
#define SH_FPSCR_FR				_ONEBIT32L( 21 )
#define SH_FPSCR_SZ				_ONEBIT32L( 20 )
#define SH_FPSCR_PR				_ONEBIT32L( 19 )
#define SH_FPSCR_DN				_ONEBIT32L( 18 )
#define SH_FPSCR_CAUSE_MASK		_BITFIELD32L( 12, 0x3f )
#define SH_FPSCR_CAUSE_E		_ONEBIT32L( 17 )
#define SH_FPSCR_CAUSE_V		_ONEBIT32L( 16 )
#define SH_FPSCR_CAUSE_Z		_ONEBIT32L( 15 )
#define SH_FPSCR_CAUSE_O		_ONEBIT32L( 14 )
#define SH_FPSCR_CAUSE_U		_ONEBIT32L( 13 )
#define SH_FPSCR_CAUSE_I		_ONEBIT32L( 12 )
#define SH_FPSCR_ENABLE_MASK	_BITFIELD32L( 7, 0x1f )
#define SH_FPSCR_ENABLE_V		_ONEBIT32L( 11 )
#define SH_FPSCR_ENABLE_Z		_ONEBIT32L( 10 )
#define SH_FPSCR_ENABLE_O		_ONEBIT32L( 9 )
#define SH_FPSCR_ENABLE_U		_ONEBIT32L( 8 )
#define SH_FPSCR_ENABLE_I		_ONEBIT32L( 7 )
#define SH_FPSCR_FLAG_MASK		_BITFIELD32L( 2, 0x1f )
#define SH_FPSCR_FLAG_V			_ONEBIT32L( 6 )
#define SH_FPSCR_FLAG_Z			_ONEBIT32L( 5 )
#define SH_FPSCR_FLAG_O			_ONEBIT32L( 4 )
#define SH_FPSCR_FLAG_U			_ONEBIT32L( 3 )
#define SH_FPSCR_FLAG_I			_ONEBIT32L( 2 )
#define SH_FPSCR_RM_MASK		_BITFIELD32L( 0, 0x3 )
#define SH_FPSCR_RM_N			_BITFIELD32L( 0, 0 )
#define SH_FPSCR_RM_Z			_BITFIELD32L( 0, 1 )

/*
 * Status Register (also SRR )
 */
#define SH_SR_MD			_ONEBIT32L( 30 )
#define SH_SR_RB			_ONEBIT32L( 29 )
#define SH_SR_BL			_ONEBIT32L( 28 ) 
#define SH_SR_FD			_ONEBIT32L( 15 )
#define SH_SR_M				_ONEBIT32L( 9 ) 
#define SH_SR_Q				_ONEBIT32L( 8 )
#define SH_SR_IMASK			_BITFIELD32L( 4, 0xf )
#define SH_SR_IMASKL(x)		_BITFIELD32L( 4, x )
#define SH_SR_S				_ONEBIT32L( 1 )
#define SH_SR_T				_ONEBIT32L( 0 )

/*
 * The exception table
 */
/* in P2 */
#define SH_EXC_BASE_RESET		0xa0000000
/* VBR + off */
#define SH_EXC_GENERAL			0x100
#define SH_EXC_TLBMISS			0x400
#define SH_EXC_INTR				0x600
#define SH_EXC_SIZE				0X1000

/*
 * Exception Code
 */

#define SH_EXC_CODE_HDREST		0x000
#define SH_EXC_CODE_SFREST		0x020
#define SH_EXC_CODE_UDIREST		0x000
#define SH_EXC_CODE_TLBMULHIT	0x140
#define SH_EXC_CODE_UBBEFORE	0x1e0
#define SH_EXC_CODE_IADDRERR	0x0e0
#define SH_EXC_CODE_ITLBMISS	0x040
#define SH_EXC_CODE_ITLBPROT	0x0a0
#define SH_EXC_CODE_GILLINST	0x180
#define SH_EXC_CODE_SILLINST	0x1a0
#define SH_EXC_CODE_GFPUDISABLE	0x800
#define SH_EXC_CODE_SFPUDISABLE	0x820
#define SH_EXC_CODE_DADDRREAD	0x0e0
#define SH_EXC_CODE_DADDRWRITE	0x100
#define SH_EXC_CODE_DTLBMISSR	0x040
#define SH_EXC_CODE_DTLBMISSW	0x060
#define SH_EXC_CODE_DTLBPROTR	0x0a0
#define SH_EXC_CODE_DTLBPROTW	0x0c0
#define SH_EXC_CODE_FPU			0x120
#define SH_EXC_CODE_INIWRITE	0x080
#define SH_EXC_CODE_TRAP		0x160
#define SH_EXC_CODE_UBAFTER		0x1e0
#define SH_EXC_CODE_NMI			0x1c0
#define SH_EXC_CODE_IRL0		0x200
#define SH_EXC_CODE_IRL1		0x220
#define SH_EXC_CODE_IRL2		0x240
#define SH_EXC_CODE_IRL3		0x260
#define SH_EXC_CODE_IRL4		0x280
#define SH_EXC_CODE_IRL5		0x2a0
#define SH_EXC_CODE_IRL6		0x2c0
#define SH_EXC_CODE_IRL7		0x2e0
#define SH_EXC_CODE_IRL8		0x300
#define SH_EXC_CODE_IRL9		0x320
#define SH_EXC_CODE_IRLA		0x340
#define SH_EXC_CODE_IRLB		0x360
#define SH_EXC_CODE_IRLC		0x380
#define SH_EXC_CODE_IRLD		0x3a0
#define SH_EXC_CODE_IRLE		0x3c0
#define SH_EXC_CODE_TUNI0		0x400
#define SH_EXC_CODE_TUNI1		0x420
#define SH_EXC_CODE_TUNI2		0x440
#define SH_EXC_CODE_TICPI2		0x460
#define SH_EXC_CODE_ATI			0x480
#define SH_EXC_CODE_PRI			0x4a0
#define SH_EXC_CODE_CUI			0x4c0
#define SH_EXC_CODE_SCIERI		0x4e0
#define SH_EXC_CODE_SCIRXI		0x500
#define SH_EXC_CODE_SCITXI		0x520
#define SH_EXC_CODE_SCITEI		0x540
#define SH_EXC_CODE_ITI			0x560
#define SH_EXC_CODE_RCMI		0x580
#define SH_EXC_CODE_ROVI		0x5a0
#define SH_EXC_CODE_UDI			0x600
#define SH_EXC_CODE_GPIO		0x620
#define SH_EXC_CODE_DMTE0		0x640
#define SH_EXC_CODE_DMTE1		0x660
#define SH_EXC_CODE_DMTE2		0x680
#define SH_EXC_CODE_DMTE3		0x6a0
#define SH_EXC_CODE_DMAE		0x6c0
#define SH_EXC_CODE_SCIFERI		0x700
#define SH_EXC_CODE_SCIFRXI		0x720
#define SH_EXC_CODE_SCIFBRI		0x740
#define SH_EXC_CODE_SCIFTXI		0x760
/* SH7751 only */
#define SH_EXC_CODE_PCISERR		0xa00
#define SH_EXC_CODE_PCIDMA3		0xa20
#define SH_EXC_CODE_PCIDMA2		0xa40
#define SH_EXC_CODE_PCIDMA1		0xa60
#define SH_EXC_CODE_PCIDMA0		0xa80
#define SH_EXC_CODE_PCIPWON		0xaa0
#define SH_EXC_CODE_PCIPWDWN	0xac0
#define SH_EXC_CODE_PCIERR 		0xae0
#define SH_EXC_CODE_TUNI3		0xb00
#define SH_EXC_CODE_TUNI4		0xb80
/* SH7760 only */
#define SH7760_EXC_CODE_SCIFERI0	0x880
#define SH7760_EXC_CODE_SCIFRXI0	0x8A0
#define SH7760_EXC_CODE_SCIFBRI0	0x8C0
#define SH7760_EXC_CODE_SCIFTXI0	0x8E0
#define SH7760_EXC_CODE_SCIFERI1	0xB00
#define SH7760_EXC_CODE_SCIFRXI1	0xB20
#define SH7760_EXC_CODE_SCIFBRI1	0xB40
#define SH7760_EXC_CODE_SCIFTXI1	0xB60
#define SH7760_EXC_CODE_SCIFERI2	0xB80
#define SH7760_EXC_CODE_SCIFRXI2	0xBA0
#define SH7760_EXC_CODE_SCIFBRI2	0xBC0
#define SH7760_EXC_CODE_SCIFTXI2	0xBE0
	
/*#define OPCODE_BREAK	0x7fe00008*/


/*
 * Memory Mapped Register addresses
 */
 
#define SH_MMR_CCN_PTEH			0xff000000
#define SH_MMR_CCN_PTEL			0xff000004
#define SH_MMR_CCN_TTB			0xff000008
#define SH_MMR_CCN_TEA			0xff00000c
#define SH_MMR_CCN_MMUCR		0xff000010
#define SH_MMR_CCN_BASRA		0xff000014
#define SH_MMR_CCN_BASRB		0xff000018
#define SH_MMR_CCN_CCR			0xff00001c
#define SH_MMR_CCN_TRA			0xff000020
#define SH_MMR_CCN_EXPEVT		0xff000024
#define SH_MMR_CCN_INTEVT		0xff000028
#define SH_MMR_CCN_PTEA			0xff000034
#define SH_MMR_CCN_QACR0		0xff000038
#define SH_MMR_CCN_QACR1		0xff00003c
#define SH_MMR_PVR				0xff000030

#define SH_MMR_UBC_BARA			0xff200000
#define SH_MMR_UBC_BAMRA		0xff200004
#define SH_MMR_UBC_BBRA			0xff200008
#define SH_MMR_UBC_BARB			0xff20000c
#define SH_MMR_UBC_BAMRB		0xff200010
#define SH_MMR_UBC_BBRB			0xff200014
#define SH_MMR_UBC_BDRB			0xff200018
#define SH_MMR_UBC_BDMRB		0xff20001c
#define SH_MMR_UBC_BRCR			0xff200020

#define SH_MMR_BSC_BCR1			0xff800000
#define SH_MMR_BSC_BCR2			0xff800004
#define SH_MMR_BSC_WCR1			0xff800008
#define SH_MMR_BSC_WCR2			0xff80000c
#define SH_MMR_BSC_WCR3			0xff800010
#define SH_MMR_BSC_MCR			0xff800014
#define SH_MMR_BSC_PCR			0xff800018
#define SH_MMR_BSC_RTCSR		0xff80001c
#define SH_MMR_BSC_RTCNT		0xff800020
#define SH_MMR_BSC_RTCOR		0xff800024
#define SH_MMR_BSC_RFCR			0xff800028
#define SH_MMR_BSC_PCTRA		0xff80002c
#define SH_MMR_BSC_PDTRA		0xff800030
#define SH_MMR_BSC_PCTRB		0xff800040
#define SH_MMR_BSC_PDTRB		0xff800044
#define SH_MMR_BSC_GPIOIC		0xff800048
#define SH_MMR_BSC_SDMR2		0xff900000
#define SH_MMR_BSC_SDMR3		0xff940000

#define SH_MMR_DMAC_SAR0		0xffa00000
#define SH_MMR_DMAC_DAR0		0xffa00004
#define SH_MMR_DMAC_DMATCR0		0xffa00008
#define SH_MMR_DMAC_CHCR0		0xffa0000c
#define SH_MMR_DMAC_SAR1		0xffa00010
#define SH_MMR_DMAC_DAR1		0xffa00014
#define SH_MMR_DMAC_DMATCR1		0xffa00018
#define SH_MMR_DMAC_CHCR1		0xffa0001c
#define SH_MMR_DMAC_SAR2		0xffa00020
#define SH_MMR_DMAC_DAR2		0xffa00024
#define SH_MMR_DMAC_DMATCR2		0xffa00028
#define SH_MMR_DMAC_CHCR2		0xffa0002c
#define SH_MMR_DMAC_SAR3		0xffa00030
#define SH_MMR_DMAC_DAR3		0xffa00034
#define SH_MMR_DMAC_DMATCR3		0xffa00038
#define SH_MMR_DMAC_CHCR3		0xffa0003c
#define SH_MMR_DMAC_DMAOR		0xffa00040

#define SH_MMR_CPG_FRQCR		0xffc00000
#define SH_MMR_CPG_STBCR		0xffc00004
#define SH_MMR_CPG_WTCNT		0xffc00008
#define SH_MMR_CPG_WTCSR		0xffc0000c
#define SH_MMR_CPG_STBCR2		0xffc00010

#define SH7760_MMR_CPG_CLKSTP00 	0xfe0a0000
#define SH7760_MMR_CPG_CLKSTPCLR00 	0xfe0a0010

#define SH_MMR_RTC_R64CNT		0xffc80000
#define SH_MMR_RTC_RSECCNT		0xffc80004
#define SH_MMR_RTC_RMINCNT		0xffc80008
#define SH_MMR_RTC_RHRCNT		0xffc8000c
#define SH_MMR_RTC_RWKCNT		0xffc80010
#define SH_MMR_RTC_RDAYCNT		0xffc80014
#define SH_MMR_RTC_RMONCNT		0xffc80018
#define SH_MMR_RTC_RYRCNT		0xffc8001c
#define SH_MMR_RTC_RSECAR		0xffc80020
#define SH_MMR_RTC_RMINAR		0xffc80024
#define SH_MMR_RTC_RHRAR		0xffc80028
#define SH_MMR_RTC_RWKAR		0xffc8002c
#define SH_MMR_RTC_RDAYAR		0xffc80030
#define SH_MMR_RTC_RMONAR		0xffc80034
#define SH_MMR_RTC_RCR1			0xffc80038
#define SH_MMR_RTC_RCR2			0xffc8003c

#define SH_MMR_INTC_ICR			0xffd00000
#define SH_MMR_INTC_IPRA		0xffd00004
#define SH_MMR_INTC_IPRB		0xffd00008
#define SH_MMR_INTC_IPRC		0xffd0000c
/* SH7751 only */
#define SH_MMR_INTC_IPRD		0xffd00010
#define SH_MMR_INTC_INTPRI00	0xfe080000
#define SH_MMR_INTC_INTREQ00	0xfe080020
#define SH_MMR_INTC_INTMSK00	0xfe080040
#define SH_MMR_INTC_INTMSKCLR00	0xfe080060

#define SH4_MMR_TMU_BASE_ADDRESS	0xffd80000
#define SH_MMR_TMU_TOCR				0xffd80000
#define SH_MMR_TMU_TSTR				0xffd80004
#define SH_MMR_TMU_TCOR0			0xffd80008
#define SH_MMR_TMU_TCNT0			0xffd8000c
#define SH_MMR_TMU_TCR0				0xffd80010
#define SH_MMR_TMU_TCOR1			0xffd80014
#define SH_MMR_TMU_TCNT1			0xffd80018
#define SH_MMR_TMU_TCR1				0xffd8001c
#define SH_MMR_TMU_TCOR2			0xffd80020
#define SH_MMR_TMU_TCNT2			0xffd80024
#define SH_MMR_TMU_TCR2				0xffd80028
#define SH_MMR_TMU_TCPR2			0xffd8002c

/* TMU has moved!  The TMU has moved to a different location on later hardware
 * Since we refer to the TMU in kernel code, we set up an asinfo record in
 * startup to define the TMU location.
 * During kernel initialization, we find the asinfo record and initialize
 * sh_mmr_tmu_base_address appropriately.
 * References to TMU registers in the kernel are then done through
 * *(sh_mmr_tmu_base_address + OFFSET).
 * 
 * TMU register offsets are (mostly) unchanged, so we can use constants for those.
 */
#define SH_X3P_MMR_TMU_BASE_ADDRESS 0xffc10000

#define SH_MMR_TMU_TSTR_OFFSET    (SH_MMR_TMU_TSTR - SH4_MMR_TMU_BASE_ADDRESS)
#define SH_MMR_TMU_TCOR0_OFFSET   (SH_MMR_TMU_TCOR0 - SH4_MMR_TMU_BASE_ADDRESS)
#define SH_MMR_TMU_TCNT0_OFFSET   (SH_MMR_TMU_TCNT0 - SH4_MMR_TMU_BASE_ADDRESS)
#define SH_MMR_TMU_TCR0_OFFSET    (SH_MMR_TMU_TCR0 - SH4_MMR_TMU_BASE_ADDRESS)
#define SH_MMR_TMU_TCOR1_OFFSET   (SH_MMR_TMU_TCOR1 - SH4_MMR_TMU_BASE_ADDRESS)
#define SH_MMR_TMU_TCNT1_OFFSET   (SH_MMR_TMU_TCNT1 - SH4_MMR_TMU_BASE_ADDRESS)
#define SH_MMR_TMU_TCR1_OFFSET    (SH_MMR_TMU_TCR1 - SH4_MMR_TMU_BASE_ADDRESS)
#define SH_MMR_TMU_TCOR2_OFFSET   (SH_MMR_TMU_TCOR2 - SH4_MMR_TMU_BASE_ADDRESS)
#define SH_MMR_TMU_TCNT2_OFFSET   (SH_MMR_TMU_TCNT2 - SH4_MMR_TMU_BASE_ADDRESS)
#define SH_MMR_TMU_TCR2_OFFSET    (SH_MMR_TMU_TCR2 - SH4_MMR_TMU_BASE_ADDRESS)
#define SH_MMR_TMU_TCPR2_OFFSET   (SH_MMR_TMU_TCPR2 - SH4_MMR_TMU_BASE_ADDRESS)

#define SH_MMR_SCI_SCSMR1		0xffe00000
#define SH_MMR_SCI_SCBRR1		0xffe00004
#define SH_MMR_SCI_SCSCR1		0xffe00008
#define SH_MMR_SCI_SCTDR1		0xffe0000c
#define SH_MMR_SCI_SCSSR1		0xffe00010
#define SH_MMR_SCI_SCRDR1		0xffe00014
#define SH_MMR_SCI_SCSCMR1		0xffe00018
#define SH_MMR_SCI_SCSPTR1		0xffe0001c

#define SH_MMR_SCIF_SCSMR2		0xffe80000
#define SH_MMR_SCIF_SCBRR2		0xffe80004
#define SH_MMR_SCIF_SCSCR2		0xffe80008
#define SH_MMR_SCIF_SCFTDR2		0xffe8000c
#define SH_MMR_SCIF_SCFSR2		0xffe80010
#define SH_MMR_SCIF_SCFRDR2		0xffe80014
#define SH_MMR_SCIF_SCFCR2		0xffe80018
#define SH_MMR_SCIF_SCFDR2		0xffe8001c
#define SH_MMR_SCIF_SCSPTR2		0xffe80020
#define SH_MMR_SCIF_SCLSR2		0xffe80024

#define SH_MMR_UDI_SDIR			0xfff00000
#define SH_MMR_UDI_SDDR			0xfff00008

#define P4_TO_A7(x)			(x & 0x1fffffff)

/*
 * Virtual Address spaces
 */
#define SH_P0BASE	     (0x00000000)
#define SH_P0SIZE	     (0x80000000)
#define SH_P1BASE	     (0x80000000)
#define SH_P1SIZE	     (0x20000000)
#define SH_P2BASE	     (0xA0000000)
#define SH_P2SIZE	     (0x20000000)
#define SH_P3BASE	     (0xC0000000)
#define SH_P3SIZE	     (0x20000000)
#define SH_P4BASE	     (0xE0000000)
#define SH_P4SIZE	     (0x20000000)
#define SH_U0BASE	     (0x00000000)
#define SH_U0SIZE	     (0x80000000)



/* Macros for converting between physical addresses and various virtual memory windows.
 * 
 * Note that in these definitions we assume the 29-bit physical address space of
 * the SH-4, which means a conversion to a physical address just involves clearing the
 * upper 3 bits of an address.  With the SH-4a architecture, we run in 32-bit mode,
 * which means that these routines are only safe to use if the physical address is
 * less than 512MB (0x20000000).  Since we require SH-4a boards to have sufficient
 * RAM below 0x20000000 to support procnto, we can still make use of these routines,
 * though we have to be careful.  If a piece of memory is allocated using the restrict_proc
 * restriction, we are guaranteed that the memory is safe for these routines.  So generally
 * procnto only needs to be careful with memory addresses that were allocated by the user.
 * 
 * Also note that for accessing the memory mapped control register area from user mode,
 * there are differences between the SH-4 and SH-4a architectures.  For privileged mode,
 * both architectures access the control register area through the P4 window of
 * virtual addresses.  For user-mode, the physical addresses of the control register
 * area must be mapped into virtual memory in the U0 area.  In the SH-4 architecture,
 * the physical addresses of the control register area are located in the A7 section of
 * the physical address space (0x1c000000-0x1fffffff).  In the SH-4a architecture, the
 * physical addresses of the control register area are the same as the virtual P4 area
 * (0xfc000000-0xffffffff).
 */
#define SH_PHYS_TO_P1(x)   (SH_P1BASE | (_Uint32t)(x))
#define SH_P1_TO_PHYS(x)   ((_Uint32t)(x) & ~SH_P1BASE)
#define SH_IS_P1(x)  	(((x) >= SH_P1BASE) && 	\
			  	((x) <= (SH_P1BASE + SH_P1SIZE - 1)))

#define SH_PHYS_TO_P2(x)   (SH_P2BASE | (_Uint32t)(x))
#define SH_P2_TO_PHYS(x)   ((_Uint32t)(x) & ~SH_P2BASE)
#define SH_IS_P2(x)  	(((x) >= SH_P2BASE) && 	\
			  	((x) <= (SH_P2BASE + SH_P2SIZE - 1)))

#define SH_IS_P3(x)  	(((x) >= SH_P3BASE) && 	\
			  	((x) <= (SH_P3BASE + SH_P3SIZE - 1)))

#define SH_IS_P4(x)  	(((x) >= SH_P4BASE) && 	\
			  	((x) <= (SH_P4BASE + SH_P4SIZE - 1)))
#define SH_P4_TO_PHYS(x)   ((_Uint32t)(x) & ~SH_P4BASE)
#define SH_PHYS_TO_P4(x)   ((_Uint32t)(x) | SH_P4BASE)

#define SH_IS_P0(x)	(((x) >= SH_P0BASE) &&    \
				((x) < (SH_P0BASE + SH_P0SIZE)))

#define SH_IS_U0(x)	(((x) >= SH_U0BASE) &&    \
				((x) < (SH_U0BASE + SH_U0SIZE)))

#define SH_PHYS_IN_1TO1(x) (x < SH_P1SIZE)


/*
 * Cache
 */
#define SH4_ICACHE_SIZE				0x2000
#define SH4_DCACHE_SIZE				0x4000
#define SH4_ICACHE_LINESIZE			0x10
#define SH4_DCACHE_LINESIZE			0x10

/*
 * MMU
 */
#define SH4_ITLB_SIZE				0x4
#define SH4_UTLB_SIZE				0x40
 
/*
 * Memory Mapped control address
 */
#define SH4_ICACHE_ADDRESS_ARRAY	0xf0000000
#define SH4_ICACHE_DATA_ARRAY		0xf1000000
#define SH4_ITLB_ADDRESS_ARRAY		0xf2000000
#define SH4_ITLB_DATA_ARRAY1		0xf3000000
#define SH4_ITLB_DATA_ARRAY2		0xf3800000
#define SH4_DCACHE_ADDRESS_ARRAY	0xf4000000
#define SH4_DCACHE_DATA_ARRAY		0xf5000000
#define SH4_UTLB_ADDRESS_ARRAY		0xf6000000
#define SH4_UTLB_DATA_ARRAY1		0xf7000000
#define SH4_UTLB_DATA_ARRAY2		0xf7800000

/*
 * PVR
 */
#define SH4_PVR_MASK(x)				((x) & 0xffffff00)
#define SH4_PVR_7750_2_4			0x04020400
#define SH4_PVR_7750_2_5			0x04020500
#define SH4_PVR_7750S				0x04020600
#define SH4_PVR_7751_1_2			0x04100000
#define SH4_PVR_7751_3_4_5			0x04110000
#define SH4_PVR_7751R				0x04050000
#define SH4_PVR_7760				0x04050100
#define SH4_PVR_7770				0x10200400
#define SH4_PVR_7780				0x10200600
#define SH4_PVR_7780_2				0x10200a00
#define SH4_PVR_X3P					0x10400100
#define SH4_PVR_7786_90				0x10400200
#define SH4_PVR_FAM(x)				((x) >> 24)
#define SH4_PVR_SH4 				0x04
#define SH4_PVR_SH4A				0x10

/* PFC (Pin Function Controller on SH7760) */
#define SH7760_PFC_PHCR				0xfe40001c

/*
 * BSC Register Bits
 */
#define SH_BSC_MCR_RMODE 			_ONEBIT32L( 1 )
#define SH_BSC_MCR_RFSH 			_ONEBIT32L( 2 )

/*
 * STBCR & STBCR2 Register Bits
 */
#define SH_CPG_STBCR_STBY			_ONEBIT8L( 7 )
#define SH_CPG_STBCR_PHZ			_ONEBIT8L( 6 )
#define SH_CPG_STBCR_PPU			_ONEBIT8L( 5 )
#define SH_CPG_STBCR_MSTP4			_ONEBIT8L( 4 )
#define SH_CPG_STBCR_MSTP3			_ONEBIT8L( 3 )
#define SH_CPG_STBCR_MSTP2			_ONEBIT8L( 2 )
#define SH_CPG_STBCR_MSTPl			_ONEBIT8L( 1 )
#define SH_CPG_STBCR_MSTP0			_ONEBIT8L( 0 )
#define SH_CPG_STBCR2_DSLP			_ONEBIT8L( 7 )
#define SH_CPG_STBCR2_STHZ			_ONEBIT8L( 6 )
#define SH_CPG_STBCR2_MSTP6			_ONEBIT8L( 1 )
#define SH_CPG_STBCR2_MSTP5			_ONEBIT8L( 0 )

/*
 * CLKSTP00/CLKSTPCLR00 Register Bits
 */
#define SH7760_CPG_CLKSTP00_SCIF2 	_ONEBIT32L(29)
#define SH7760_CPG_CLKSTP00_SCIF1 	_ONEBIT32L(28)
#define SH7760_CPG_CLKSTP00_SCIF0 	_ONEBIT32L(27)

/*
 * GPIOIC Register Bits (SH7760)
 */
#define SH7760_BSC_GPIOIC_STBRT8 	_ONEBIT16L(8)
#define SH7760_BSC_GPIOIC_STBRT7 	_ONEBIT16L(7)
#define SH7760_BSC_GPIOIC_STBRT6 	_ONEBIT16L(6)

/*
 * BCR2 Register Bits (SH7760)
 */
#define SH7760_BSC_BCR2_STBIRLEN	_ONEBIT16L(0)

/*
 * PHCR Register Bits (SH7760)
 */
#define SH7760_PFC_PHCR_PH5MD1		_ONEBIT16L(11)
#define SH7760_PFC_PHCR_PH5MD0		_ONEBIT16L(10)


#endif /* __SH_CPU_H_INCLUDED */

/* __SRCVERSION("cpu.h $Rev: 169667 $"); */
