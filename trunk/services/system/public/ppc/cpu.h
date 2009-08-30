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
 *  ppc/cpu.h
 *

 */

#ifndef __PPC_CPU_H_INCLUDED
#define __PPC_CPU_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __PPC_CONTEXT_H_INCLUDED
#include _NTO_HDR_(ppc/context.h)
#endif

#define PPC_STRINGNAME	"ppc"

/*
 * Floating Point Status and Control Register
 */
#define PPC_FPSCR_FX			_ONEBIT32B( 0 )
#define PPC_FPSCR_FEX			_ONEBIT32B( 1 )
#define PPC_FPSCR_VX			_ONEBIT32B( 2 )
#define PPC_FPSCR_OX			_ONEBIT32B( 3 )
#define PPC_FPSCR_UX			_ONEBIT32B( 4 )
#define PPC_FPSCR_ZX			_ONEBIT32B( 5 )
#define PPC_FPSCR_XX			_ONEBIT32B( 6 )
#define PPC_FPSCR_VXSNAN		_ONEBIT32B( 7 )
#define PPC_FPSCR_VXISI			_ONEBIT32B( 8 )
#define PPC_FPSCR_VXIDI			_ONEBIT32B( 9 )
#define PPC_FPSCR_VXZDZ			_ONEBIT32B( 10 )
#define PPC_FPSCR_VXIMZ			_ONEBIT32B( 11 )
#define PPC_FPSCR_VXVC			_ONEBIT32B( 12 )
#define PPC_FPSCR_FR			_ONEBIT32B( 13 )
#define PPC_FPSCR_FI			_ONEBIT32B( 14 )
#define PPC_FPSCR_FPRF_MASK		_BITFIELD32B( 19, 0x1f )
#define PPC_FPSCR_FPRF_SHIFT	(31-19)
#define PPC_FPSCR_VXSOFT		_ONEBIT32B( 21 )
#define PPC_FPSCR_VXSQRT		_ONEBIT32B( 22 )
#define PPC_FPSCR_VXCVI			_ONEBIT32B( 23 )
#define PPC_FPSCR_VE			_ONEBIT32B( 24 )
#define PPC_FPSCR_OE			_ONEBIT32B( 25 )
#define PPC_FPSCR_UE			_ONEBIT32B( 26 )
#define PPC_FPSCR_ZE			_ONEBIT32B( 27 )
#define PPC_FPSCR_XE			_ONEBIT32B( 28 )
#define PPC_FPSCR_RN_MASK		_BITFIELD32B( 31, 0x3 )
#define PPC_FPSCR_RN_NEAREST	_BITFIELD32B( 31, 0 )
#define PPC_FPSCR_RN_ZERO		_BITFIELD32B( 31, 1 )
#define PPC_FPSCR_RN_PINF		_BITFIELD32B( 31, 2 )
#define PPC_FPSCR_RN_NINF		_BITFIELD32B( 31, 3 )

/*
 * Machine Status Register (also SRR1 & SRR3)
 */
#define PPC_MSR_UCLE	_ONEBIT32B( 5 ) /* E500 */
#define PPC_MSR_SPE		_ONEBIT32B( 6 ) /* E500 */
#define PPC_MSR_VA		_ONEBIT32B( 6 ) /* 7400 series */
#define PPC_MSR_APE		_ONEBIT32B( 11 ) /* 400 series */
#define PPC_MSR_APA		_ONEBIT32B( 12 ) /* 400 series */
#define PPC_MSR_WE		_ONEBIT32B( 13 ) /* 400,booke series */
#define PPC_MSR_POW		_ONEBIT32B( 13 ) /* 603e,860 */
#define PPC_MSR_CE		_ONEBIT32B( 14 ) /* 400,booke series */
#define PPC_MSR_TGPR	_ONEBIT32B( 14 ) /* 603e */
#define PPC_MSR_ILE		_ONEBIT32B( 15 ) /* 400 series, 603e,860 */
#define PPC_MSR_EE		_ONEBIT32B( 16 )
#define PPC_MSR_PR		_ONEBIT32B( 17 )
#define PPC_MSR_FP		_ONEBIT32B( 18 ) 
#define PPC_MSR_ME		_ONEBIT32B( 19 )
#define PPC_MSR_FE0		_ONEBIT32B( 20 ) 
#define PPC_MSR_SE		_ONEBIT32B( 21 ) /* 601,603e,860 */
#define PPC_MSR_DWE		_ONEBIT32B( 21 ) /* 405 */
#define PPC_MSR_DE		_ONEBIT32B( 22 ) /* 400,booke series */
#define PPC_MSR_BE		_ONEBIT32B( 22 ) /* 603e,860 */
#define PPC_MSR_FE1		_ONEBIT32B( 23 ) 
#define PPC_MSR_EP  	_ONEBIT32B( 25 ) /* 601, 603e */
#define PPC_MSR_IP  	_ONEBIT32B( 25 ) /* 860 */
#define PPC_MSR_IT		_ONEBIT32B( 26 ) /* 601, 603e */
#define PPC_MSR_IR		_ONEBIT32B( 26 ) /* 400 series,860 */
#define PPC_MSR_IS		_ONEBIT32B( 26 ) /* booke */
#define PPC_MSR_DT		_ONEBIT32B( 27 ) /* 601, 603e */
#define PPC_MSR_DR		_ONEBIT32B( 27 ) /* 400 series,860 */
#define PPC_MSR_DS		_ONEBIT32B( 27 ) /* booke */
#define PPC_MSR_PE		_ONEBIT32B( 28 ) /* 403 */
#define PPC_MSR_PX		_ONEBIT32B( 29 ) /* 403 */
#define PPC_MSR_RI		_ONEBIT32B( 30 ) /* 603e,860 */
#define PPC_MSR_LE		_ONEBIT32B( 31 ) 

#define PPC64_MSR_SF	_ONEBIT64B(  0 )
#define PPC64_MSR_HV	_ONEBIT64B(  3 )

/* Some bits are for 603e others are for 700 */
#define PPC_SRR1_L1IC _ONEBIT32B( 1 ) /* L1 Instruction Cache Error */
#define PPC_SRR1_L1DC _ONEBIT32B( 2 ) /* L1 Data Cache Error */
#define PPC_SRR1_MSS  _ONEBIT32B( 11 ) /* Memory sub system error */
#define PPC_SRR1_MCP  _ONEBIT32B( 12 ) /* Machine Check */
#define PPC_SRR1_TEA  _ONEBIT32B( 13 ) /* Transfer error ack */
#define PPC_SRR1_DPE  _ONEBIT32B( 14 ) /* Data bus parity error */
#define PPC_SRR1_APE  _ONEBIT32B( 15 ) /* Data bus parity error */

/*
 * Processor Version Register
 */
#define PPC_PVR_FAMILY_MEMBER_MASK	_BITFIELD32B( 15, 0xffff )
#define PPC_PVR_FAMILY_MEMBER_SHIFT (31-15)
#define PPC_PVR_FAM_MASK			_BITFIELD32B( 11, 0xfff )
#define PPC_PVR_FAM_SHIFT			(31-11)
#define PPC_PVR_MEMBER_MASK			_BITFIELD32B( 15, 0xf )
#define PPC_PVR_MEMBER_SHIFT		(31-15)
#define PPC_PVR_CL_MASK				_BITFIELD32B( 19, 0xf )
#define PPC_PVR_CL_SHIFT			(31-19)
#define PPC_PVR_CFG_MASK			_BITFIELD32B( 23, 0xf )
#define PPC_PVR_CFG_SHIFT			(31-23)
#define PPC_PVR_MAJ_MASK			_BITFIELD32B( 27, 0xf )
#define PPC_PVR_MAJ_SHIFT			(31-27)
#define PPC_PVR_MIN_MASK			_BITFIELD32B( 31, 0xf )
#define PPC_PVR_MIN_SHIFT			(31-31)
#define PPC_PVR_REVISION_MASK		_BITFIELD32B( 31, 0xffff )
#define PPC_PVR_REVISION_SHIFT		(31-31)

#define PPC_GET_FAMILY( t )		(((t) & PPC_PVR_FAM_MASK) >> PPC_PVR_FAM_SHIFT)
#define PPC_GET_FAM_MEMBER( v ) (((v) & PPC_PVR_FAMILY_MEMBER_MASK) >> PPC_PVR_FAMILY_MEMBER_SHIFT)
#define PPC_GET_REVISION( t )	(((t) & PPC_PVR_REVISION_MASK) >> PPC_PVR_REVISION_SHIFT)

/* Family values (downshifted). DEPRECATED, DO NOT USE */
#define PPC_600		0x0000
#define PPC_700		0x0000
#define PPC_74XX	0x0800 /* 7410/VGER reports a different family */
#define PPC_400		0x0002
#define PPC_400B	0x0401 /* Silly 405 changed families on us */
#define PPC_400C	0x0403 /* Silly 405 vesta changed families on us */
#define PPC_800		0x0005
#define PPC_82XX	0x0008

/* Family member values (downshifted) */
#define PPC_401		0x0021
#define PPC_403		0x0020
#define PPC_405		0x4011
#define PPC_405H	0x4141
#define PPC_405EP	0x5121
#define PPC_405GPR  0x5091	/* PPC405GPR */
#define PPC_VESTA   0x4031	/* PPC405 core */
#define PPC_XILINX  0x2001  /* Xilinx Virtex II Pro and Virtex IV PPC405 core */
#define PPC_405EX	0x1291

#define PPC_601		0x0001
#define PPC_602		0x0002
#define PPC_603		0x0003
#define PPC_603e	0x0006
#define PPC_603e7	0x0007
#define PPC_604		0x0004
#define PPC_750		0x0008
#define PPC_750FX	0x7000
#define PPC_604e	0x0009
#define PPC_604e5	0x000a
#define PPC_7400	0x000c
#define PPC_7450	0x8000
#define PPC_7455	0x8001
#define PPC_7457	0x8002 /* 7447 also */
#define PPC_7447	0x8003 /* 7447A     */
#define PPC_7448	0x8004
#define PPC_7410	0x800c
#define PPC_8260	0x0081
#define PPC_8245	0x8081
#define PPC_8280	0x8082

#define PPC_8xx		0x0050

#define PPC_440GP	0x4012
#define PPC_440GX	0x51b2
#define PPC_440EP	0x4222
#define PPC_440EPx	0x2162

#define PPC_E500	0x8020
#define PPC_E500V2	0x8021

#define PPC_E300C1	0x8083
#define PPC_E300C2	0x8084
#define PPC_E300C3	0x8085
#define PPC_E300C4	0x8086

#define PPC_970FX	0x003c
#define PPC_PA6T	0x0090

/*
 * Fixed Point Exception Register
 */
#define PPC_XER_SO			_ONEBIT32B( 0 )
#define PPC_XER_OV			_ONEBIT32B( 1 )
#define PPC_XER_CA			_ONEBIT32B( 2 )
#define PPC_XER_TBC_MASK	_BITFIELD32B( 31, 0x7f )
#define PPC_XER_TBC_SHIFT	(31-31)

/*
 * Data Storage Interrupt Status Register
 */
#define PPC_DSISR_DSEE		_ONEBIT32B( 0 )
#define PPC_DSISR_NOTRANS	_ONEBIT32B( 1 )
#define PPC_DSISR_PROT		_ONEBIT32B( 4 )
#define PPC_DSISR_ILLINS	_ONEBIT32B( 5 )
#define PPC_DSISR_STORE		_ONEBIT32B( 6 )
#define PPC_DSISR_DABR		_ONEBIT32B( 9 )
#define PPC_DSISR_NOSEG		_ONEBIT32B( 10 )
#define PPC_DSISR_NOEAR		_ONEBIT32B( 11 )

/*
 * Indexes into the exception table
 */
#define PPC_EXC_SYSTEM_RESET	(0x0100/sizeof(uint32_t)) /* 603e,800 */
#define PPC_EXC_CRITICAL_INTR	(0x0100/sizeof(uint32_t)) /* 400 */
#define PPC_EXC_MACHINE_CHECK	(0x0200/sizeof(uint32_t))
#define PPC_EXC_DATA_ACCESS		(0x0300/sizeof(uint32_t))
#define PPC_EXC_DATA_SEGMENT	(0x0380/sizeof(uint32_t)) /* PPC64 */
#define PPC_EXC_INSTR_ACCESS	(0x0400/sizeof(uint32_t)) /* 603e,800 */
#define PPC_EXC_INSTR_SEGMENT	(0x0480/sizeof(uint32_t)) /* PPC64 */
#define PPC_EXC_EXTERNAL_INTR	(0x0500/sizeof(uint32_t))
#define PPC_EXC_ALIGNMENT		(0x0600/sizeof(uint32_t))
#define PPC_EXC_PROGRAM			(0x0700/sizeof(uint32_t))
#define PPC_EXC_FPU_UNAVAILABLE	(0x0800/sizeof(uint32_t)) /* 603e,800 */
#define PPC_EXC_DECREMENTER		(0x0900/sizeof(uint32_t)) /* 603e,800 */
#define PPC_EXC_HYPER_DECR		(0x0980/sizeof(uint32_t)) /* PPC64 */
#define PPC_EXC_SYSTEM_CALL		(0x0c00/sizeof(uint32_t))
#define PPC_EXC_TRACE			(0x0d00/sizeof(uint32_t)) /* 800 */
#define PPC_EXC_FPU_ASSIST		(0x0e00/sizeof(uint32_t)) /* 800 */
#define PPC64_EXC_HYPER_DATA_ACCESS		(0x0e00/sizeof(uint32_t)) /* PPC64 */
#define PPC64_EXC_HYPER_INSTR_ACCESS	(0x0e10/sizeof(uint32_t)) /* PPC64 */
#define PPC64_EXC_HYPER_DATA_SEGMENT	(0x0e20/sizeof(uint32_t)) /* PPC64 */
#define PPC64_EXC_HYPER_INSTR_SEGMENT	(0x0e30/sizeof(uint32_t)) /* PPC64 */
#define PPC_EXC_PMI				(0x0f00/sizeof(uint32_t)) /* 600+ */
#define PPC_EXC_VMX_UNAVAILABLE	(0x0f20/sizeof(uint32_t)) /* 7400 - VMX */
#define PPC_EXC_VMX_ASSIST		(0x1600/sizeof(uint32_t)) /* 7400 - VMX */
#define PPC64_EXC_VMX_ASSIST	(0x1700/sizeof(uint32_t)) /* 970 - VMX */

#define PPC_EXC_SIZE			0x3000

/*
 * SPR numbers
 */
#define PPC_SPR_XER		1
#define PPC_SPR_LR		8
#define PPC_SPR_CTR		9
#define PPC_SPR_DSISR	18
#define PPC_SPR_DAR		19
#define PPC_SPR_DEC		22
#define PPC_SPR_SDR1	25
#define PPC_SPR_SRR0	26
#define PPC_SPR_SRR1	27
#define PPC_SPR_ACCR	29
#define PPC_SPR_CTRL	152
#define PPC_SPR_SPRG0	272
#define PPC_SPR_SPRG1	273
#define PPC_SPR_SPRG2	274
#define PPC_SPR_SPRG3	275
#define PPC_SPR_SPRG4	276
#define PPC_SPR_SPRG5	277
#define PPC_SPR_SPRG6	278
#define PPC_SPR_SPRG7	279
#define PPC_SPR_SPRG4_RD 260
#define PPC_SPR_SPRG5_RD 261
#define PPC_SPR_SPRG6_RD 262
#define PPC_SPR_SPRG7_RD 263
#define PPC_SPR_ASR		280
#define PPC_SPR_EAR		282
#define PPC_SPR_TBL		268
#define PPC_SPR_TBU		269
#define PPC_SPR_TBLW	284
#define PPC_SPR_TBHW	285
#define PPC_SPR_PVR		287
#define PPC_SPR_HDEC	310
#define PPC_SPR_IBAT0U	528
#define PPC_SPR_IBAT0L	529
#define PPC_SPR_IBAT1U	530
#define PPC_SPR_IBAT1L	531
#define PPC_SPR_IBAT2U	532
#define PPC_SPR_IBAT2L	533
#define PPC_SPR_IBAT3U	534
#define PPC_SPR_IBAT3L	535
#define PPC_SPR_DBAT0U	536
#define PPC_SPR_DBAT0L	537
#define PPC_SPR_DBAT1U	538
#define PPC_SPR_DBAT1L	539
#define PPC_SPR_DBAT2U	540
#define PPC_SPR_DBAT2L	541
#define PPC_SPR_DBAT3U	542
#define PPC_SPR_DBAT3L	543
#define PPC_SPR_DABR	1013
#define PPC64_SPR_MMCR0			795
#define PPC64_SPR_MMCR0_USER	779
#define PPC64_SPR_MMCR1			798
#define PPC64_SPR_MMCR1_USER	782
#define PPC64_SPR_MMCRA			786
#define PPC64_SPR_MMCRA_USER	770
#define PPC64_SPR_PMC1			787
#define PPC64_SPR_PMC1_USER		771
#define PPC64_SPR_PMC2			788
#define PPC64_SPR_PMC2_USER		772
#define PPC64_SPR_PMC3			789
#define PPC64_SPR_PMC3_USER		773
#define PPC64_SPR_PMC4			790
#define PPC64_SPR_PMC4_USER		774
#define PPC64_SPR_PMC5			791
#define PPC64_SPR_PMC5_USER		775
#define PPC64_SPR_PMC6			792
#define PPC64_SPR_PMC6_USER		776
#define PPC64_SPR_PMC7			793
#define PPC64_SPR_PMC7_USER		777
#define PPC64_SPR_PMC8			794
#define PPC64_SPR_PMC8_USER 	778
#define PPC64_SPR_SIAR			796
#define PPC64_SPR_SIAR_USER		780
#define PPC64_SPR_SDAR			797
#define PPC64_SPR_SDAR_USER		781
#define PPC64_SPR_HSPRG0		304
#define PPC64_SPR_HSPRG1		305
#define PPC64_SPR_HIOR			311
#define PPC64_SPR_HSSR0			314
#define PPC64_SPR_HSSR1			315
#define PPC64_SPR_LPCR		 	318  
#define PPC64_SPR_RMOR			312   
#define PPC64_SPR_HRMOR			313 
#define PPC64_SPR_LPIDR		 	319  
#define PPC64_SPR_PPR		 	896  
#define PPC64_SPR_PURR		 	309  
#define PPC64_SPR_DABRX			1015 
#define PPC64_SPR_VRSAVE		256    

/*
 * Extended BAT Registers
 */
#define PPC_SPR_IBAT4U  560
#define PPC_SPR_IBAT4L  561
#define PPC_SPR_IBAT5U  562
#define PPC_SPR_IBAT5L  563
#define PPC_SPR_IBAT6U  564
#define PPC_SPR_IBAT6L  565
#define PPC_SPR_IBAT7U  566
#define PPC_SPR_IBAT7L  567
#define PPC_SPR_DBAT4U  568
#define PPC_SPR_DBAT4L  569
#define PPC_SPR_DBAT5U  570
#define PPC_SPR_DBAT5L  571
#define PPC_SPR_DBAT6U  572
#define PPC_SPR_DBAT6L  573
#define PPC_SPR_DBAT7U  574
#define PPC_SPR_DBAT7L  575


#define OPCODE_BREAK	0x7fe00008

/*
 * MMU
 */

/* Segment registers */
#define PPC_SR_T			_ONEBIT32B(0)
#define PPC_SR_KS			_ONEBIT32B(1)
#define PPC_SR_KP			_ONEBIT32B(2)
#define PPC_SR_N			_ONEBIT32B(3)
#define PPC_SR_VSID_MASK	_BITFIELD32B(31, 0x00ffffff)
#define PPC_SR_VSID_SHIFT	0

/* BAT registers */
#define PPC_BATU_BEPI_MASK	_BITFIELD32B( 14, 0x7fff )
#define PPC_BATU_BEPI(x)	_BITFIELD32B( 14, x )
#define PPC_BATU_BL_MASK	_BITFIELD32B( 29, 0x7ff )
#define PPC_BATU_BL_128K	_BITFIELD32B( 29, 0x0 )
#define PPC_BATU_BL_256K	_BITFIELD32B( 29, 0x1 )
#define PPC_BATU_BL_512K	_BITFIELD32B( 29, 0x3 )
#define PPC_BATU_BL_1M		_BITFIELD32B( 29, 0x7 )
#define PPC_BATU_BL_2M		_BITFIELD32B( 29, 0xf )
#define PPC_BATU_BL_4M		_BITFIELD32B( 29, 0x1f )
#define PPC_BATU_BL_8M		_BITFIELD32B( 29, 0x3f )
#define PPC_BATU_BL_16M		_BITFIELD32B( 29, 0x7f )
#define PPC_BATU_BL_32M		_BITFIELD32B( 29, 0xff )
#define PPC_BATU_BL_64M		_BITFIELD32B( 29, 0x1ff )
#define PPC_BATU_BL_128M	_BITFIELD32B( 29, 0x3ff )
#define PPC_BATU_BL_256M	_BITFIELD32B( 29, 0x7ff )
#define PPC_BATU_VS			_ONEBIT32B( 30 )
#define PPC_BATU_VP			_ONEBIT32B( 31 )
#define PPC_BATL_BRPN_MASK	_BITFIELD32B( 14, 0x7fff )
#define PPC_BATL_BRPN(x)	_BITFIELD32B( 14, x )
#define PPC_BATL_BRXN_MASK	_BITFIELD32B( 22, 0x7 )
#define PPC_BATL_BXPN(x)	_BITFIELD32B( 22, x )
#define PPC_BATL_W 			_ONEBIT32B( 25 )
#define PPC_BATL_I 			_ONEBIT32B( 26 )
#define PPC_BATL_M 			_ONEBIT32B( 27 )
#define PPC_BATL_G 			_ONEBIT32B( 28 )
#define PPC_BATL_BX			_ONEBIT32B( 29 )
#define PPC_BATL_PP_MASK	_BITFIELD32B( 31, 0x3 )
#define PPC_BATL_PP_NA		_BITFIELD32B( 31, 0x0 )
#define PPC_BATL_PP_RW		_BITFIELD32B( 31, 0x2 )
#define PPC_BATL_PP_R		_ONEBIT32B( 31 )

/* tlb lo */
#define PPC_TLBLO_RPN_MASK	_BITFIELD32B( 19, 0xfffff )
#define PPC_TLBLO_R			_ONEBIT32B( 23 )
#define PPC_TLBLO_C			_ONEBIT32B( 24 )
#define PPC_TLBLO_W			_ONEBIT32B( 25 )
#define PPC_TLBLO_I			_ONEBIT32B( 26 )
#define PPC_TLBLO_M			_ONEBIT32B( 27 )
#define PPC_TLBLO_G			_ONEBIT32B( 28 )
#define PPC_TLBLO_PP_MASK	_BITFIELD32B( 31, 0x3 )
#define PPC_TLBLO_PP_NA		_BITFIELD32B( 31, 0x0 )
#define PPC_TLBLO_PP_RO		_BITFIELD32B( 31, 0x1 )
#define PPC_TLBLO_PP_RW		_BITFIELD32B( 31, 0x2 )
#define PPC_TLBLO_PP_ROA	_BITFIELD32B( 31, 0x3 )
#define PPC64_TLBLO_N			_ONEBIT64B( 61 )
#define PPC64_TLBLO_AC			_ONEBIT64B( 54 )
#define PPC64_TLBLO_RPN_MASK	_BITFIELD64B( 51, 0x3ffffffffffff )

/* tlb hi */
#define PPC64_TLBHI_AVPN_MASK	_BITFIELD64B( 56, 0x1ffffffffffffff )
#define PPC64_TLBHI_AVPN_SHIFT	(63-56)
#define PPC64_TLBHI_SW_MASK		_BITFIELD64B( 60, 0xf )
#define PPC64_TLBHI_SW_SHIFT	(63-60)
#define PPC64_TLBHI_L			_ONEBIT64B( 61 )
#define PPC64_TLBHI_H			_ONEBIT64B( 62 )
#define PPC64_TLBHI_V			_ONEBIT64B( 63 )

/* SLB */
#define PPC64_SLB0_VSID_MASK	_BITFIELD64B(51, 0xfffffffffffff)
#define PPC64_SLB0_VSID_SHIFT	(63-51)
#define PPC64_SLB0_KS			_ONEBIT64B(52)
#define PPC64_SLB0_KP			_ONEBIT64B(53)
#define PPC64_SLB0_N			_ONEBIT64B(54)
#define PPC64_SLB0_L			_ONEBIT64B(55)
#define PPC64_SLB0_C			_ONEBIT64B(56)

#define PPC64_SLB1_ESID_MASK	_BITFIELD64B(35,0xfffffffff)
#define PPC64_SLB1_ESID_SHIFT	(63-35)
#define PPC64_SLB1_V			_ONEBIT64B(36)
#define PPC64_SLB1_INDEX_MASK	_BITFIELD64B(63,0xfff)
#define PPC64_SLB1_INDEX_SHIFT	(63-63)


/*
 * Bits for special parameter of shm_ctl_special()
 */
#define PPC_SPECIAL_E		0x0001
#define PPC_SPECIAL_G		0x0002
#define PPC_SPECIAL_M		0x0004
#define PPC_SPECIAL_I		0x0008
#define PPC_SPECIAL_W		0x0010

#endif /* __PPC_CPU_H_INCLUDED */

/* __SRCVERSION("cpu.h $Rev: 212521 $"); */
