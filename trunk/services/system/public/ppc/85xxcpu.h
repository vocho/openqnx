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
 *  ppc/85xxcpu.h
 *
 * Registers specific to the Motorola MPC85xx
 *

 */

#ifndef __PPC_85XXCPU_H_INCLUDED
#define __PPC_85XXCPU_H_INCLUDED

#ifndef __PPC_E500CPU_H_INCLUDED
#include <ppc/e500cpu.h>
#endif


/*
 * DDR Chip Select n Memory Bounds
 */
#define PPC85xx_CSn_BNDS_SA_MASK	_BITFIELD32B(15,0xff)
#define PPC85xx_CSn_BNDS_SA_SHIFT	(31-15)
#define PPC85xx_CSn_BNDS_EA_MASK	_BITFIELD32B(31,0xff)
#define PPC85xx_CSn_BNDS_EA_SHIFT	0

/*
 * DDR Chip Select n Configuration
 */
#define PPC85xx_CSn_CONFIG_CS_EN			_ONEBIT32B(0)
#define PPC85xx_CSn_CONFIG_AP_EN			_ONEBIT32B(8)
#define PPC85xx_CSn_CONFIG_ROW_BITS_MASK	_BITFIELD32B(23,0x7)
#define PPC85xx_CSn_CONFIG_ROW_BITS_SHIFT	(31-23)
#define PPC85xx_CSn_CONFIG_ROW_BITS_12		_BITFIELD32B(23,0x0)
#define PPC85xx_CSn_CONFIG_ROW_BITS_13		_BITFIELD32B(23,0x1)
#define PPC85xx_CSn_CONFIG_ROW_BITS_14		_BITFIELD32B(23,0x2)
#define PPC85xx_CSn_CONFIG_COL_BITS_MASK	_BITFIELD32B(23,0x7)
#define PPC85xx_CSn_CONFIG_COL_BITS_SHIFT	(31-31)
#define PPC85xx_CSn_CONFIG_COL_BITS_8		_BITFIELD32B(31,0x0)
#define PPC85xx_CSn_CONFIG_COL_BITS_9		_BITFIELD32B(31,0x1)
#define PPC85xx_CSn_CONFIG_COL_BITS_10		_BITFIELD32B(31,0x2)
#define PPC85xx_CSn_CONFIG_COL_BITS_11		_BITFIELD32B(31,0x3)

/*
 * POR PLL Status Register
 */
#define PPC85xx_PORPLLSR_E500_MASK		_BITFIELD32B(15,0x1f)
#define PPC85xx_PORPLLSR_E500_SHIFT		(31-15)
#define PPC85xx_PORPLLSR_PLAT_MASK		_BITFIELD32B(30,0x1f)
#define PPC85xx_PORPLLSR_PLAT_SHIFT		(31-30)
#define PPC85xx_PORPLLSR_BYPASS			_ONEBIT32B(31)

/*
 * PIC Feature Reporting Register
 */
#define PPC85xx_FRR_NIRQ_MASK			_BITFIELD32B(15,0x7ff)
#define PPC85xx_FRR_NIRQ_SHIFT			(31-15)
#define PPC85xx_FRR_NCPU_MASK			_BITFIELD32B(23,0x1f)
#define PPC85xx_FRR_NCPU_SHIFT			(31-23)
#define PPC85xx_FRR_VID_MASK			_BITFIELD32B(31,0xff)
#define PPC85xx_FRR_VID_SHIFT			(31-31)

/*
 * PIC Global Configuration Register
 */
#define PPC85xx_GCR_RST					_ONEBIT32B(0)
#define PPC85xx_GCR_M					_ONEBIT32B(2)

/*
 * PIC Vendor Identification Register
 */
#define PPC85xx_VIR_STEP_MASK			_BITFIELD32B(15,0xff)
#define PPC85xx_VIR_STEP_SHIFT			(31-15)
#define PPC85xx_VIR_DEVICE_ID_MASK		_BITFIELD32B(23,0xff)
#define PPC85xx_VIR_DEVICE_ID_SHIFT		(31-23)
#define PPC85xx_VIR_VENDOR_ID_MASK		_BITFIELD32B(31,0xff)
#define PPC85xx_VIR_VENDOR_ID_SHIFT		(31-31)

/*
 * PIC Processor Initialization Register
 */
#define PPC85xx_PIR_P0					_ONEBIT32B(31)

/*
 * PIC IPI/Timer/Message/External/Internal Interrupt Vector/Priority Regster
 */
#define PPC85xx_xIVPR_MSK				_ONEBIT32B(0)
#define PPC85xx_xIVPR_A					_ONEBIT32B(1)
#define PPC85xx_xIVPR_P					_ONEBIT32B(8)
#define PPC85xx_xIVPR_S					_ONEBIT32B(9)
#define PPC85xx_xIVPR_PRIORITY_MASK		_BITFIELD32B(15,0xf)
#define PPC85xx_xIVPR_PRIORITY_SHIFT	(31-15)
#define PPC85xx_xIVPR_VECTOR_MASK		_BITFIELD32B(31,0xffff)
#define PPC85xx_xIVPR_VECTOR_SHIFT		(31-31)

/*
 * PIC IPI/Timer/Message/External/Internal Interrupt Destination Regster
 */
#define PPC85xx_xIDR_EP					_ONEBIT32B(0)
#define PPC85xx_xIDR_CI					_ONEBIT32B(1)
#define PPC85xx_xIDR_P0					_ONEBIT32B(31)

/*
 * PIC Global Timer Current Count Register
 */
#define PPC85xx_GTCCR_TOG				_ONEBIT32B(0)
#define PPC85xx_GTCCR_COUNT_MASK		_BITFIELD32B(31,0x7fffffff)
#define PPC85xx_GTCCR_COUNT_SHIFT		(31-31)

/*
 * PIC Global Timer Base Count Register
 */
#define PPC85xx_GTBCR_CI				_ONEBIT32B(0)
#define PPC85xx_GTBCR_BASE_CNT_MASK		_BITFIELD32B(31,0x7fffffff)
#define PPC85xx_GTBCR_BASE_CNT_SHIFT	(31-31)

/*
 * PIC Timer Control Register
 */
#define PPC85xx_TCR_ROVR_MASK			_BITFIELD32B(7,0x7)
#define PPC85xx_TCR_ROVR_SHIFT			(31-7)
#define PPC85xx_TCR_ROVR_0				_ONEBIT32B(7)
#define PPC85xx_TCR_ROVR_1				_ONEBIT32B(6)
#define PPC85xx_TCR_ROVR_2				_ONEBIT32B(5)
#define PPC85xx_TCR_CLKR_MASK			_BITFIELD32B(23,0x3)
#define PPC85xx_TCR_CLKR_SHIFT			(31-23)
#define PPC85xx_TCR_CLKR_DIV8			_BITFIELD32B(23,0x0)
#define PPC85xx_TCR_CLKR_DIV16			_BITFIELD32B(23,0x1)
#define PPC85xx_TCR_CLKR_DIV32			_BITFIELD32B(23,0x2)
#define PPC85xx_TCR_CLKR_DIV64			_BITFIELD32B(23,0x3)
#define PPC85xx_TCR_CASC_MASK			_BITFIELD32B(31,0x7)
#define PPC85xx_TCR_CASC_SHIFT			(31-31)
#define PPC85xx_TCR_CASC_NONE			_BITFIELD32B(31,0x0)
#define PPC85xx_TCR_CASC_0AND1			_BITFIELD32B(31,0x1)
#define PPC85xx_TCR_CASC_1AND2			_BITFIELD32B(31,0x2)
#define PPC85xx_TCR_CASC_0AND1AND2		_BITFIELD32B(31,0x3)
#define PPC85xx_TCR_CASC_2AND3			_BITFIELD32B(31,0x4)
#define PPC85xx_TCR_CASC_0AND1_2AND3	_BITFIELD32B(31,0x5)
#define PPC85xx_TCR_CASC_1AND2AND3		_BITFIELD32B(31,0x6)
#define PPC85xx_TCR_CASC_0AND1AND2AND3	_BITFIELD32B(31,0x7)

/*
 * PIC Performance Monitor Mask Register (Even)
 */
#define PPC85xx_PMMRE_IPI(n)			_ONEBIT32B(8+(n))
#define PPC85xx_PMMRE_TIMER(n)			_ONEBIT32B(12+(n))
#define PPC85xx_PMMRE_MSG(n)			_ONEBIT32B(16+(n))
#define PPC85xx_PMMRE_EXT(n)			_ONEBIT32B(20+(n))

/*
 * PIC Performance Monitor Mask Register (Odd)
 */
#define PPC85xx_PMMRO_INT(n)			_ONEBIT32B((n))

/*
 * PIC Message Enable Register
 */
#define PPC85xx_MER_E3					_ONEBIT32B(28)
#define PPC85xx_MER_E2					_ONEBIT32B(29)
#define PPC85xx_MER_E1					_ONEBIT32B(30)
#define PPC85xx_MER_E0					_ONEBIT32B(31)

/*
 * PIC Message Status Register
 */
#define PPC85xx_MSR_S3					_ONEBIT32B(28)
#define PPC85xx_MSR_S2					_ONEBIT32B(29)
#define PPC85xx_MSR_S1					_ONEBIT32B(30)
#define PPC85xx_MSR_S0					_ONEBIT32B(31)

/*
 * PIC Interprocessor Interrupt Dispatch Register
 */
#define PPC85xx_IPIDR_P0				_ONEBIT32B(31)

/*
 * ECB CCB Port Configuration register
 */
#define PPC85xx_EEBPCR_CPU1_EN			_ONEBIT32B(6)
#define PPC85xx_EEBPCR_CPU0_EN			_ONEBIT32B(7)

/*
 * Boot Page Translation Register
 */
#define PPC85xx_BPTR_EN					_ONEBIT32B(0)

/*
 * CCSR Offsets of memory mapped registers
 */
#define PPC85xx_CCSR_OFF_CCSRBAR		0x00000
#define PPC85xx_CCSR_OFF_ALTCBAR		0x00008
#define PPC85xx_CCSR_OFF_ALTCSR			0x00010
#define PPC85xx_CCSR_OFF_BPTR			0x00020
#define PPC85xx_CCSR_OFF_EEBPCR			0x01010
#define PPC85xx_CCSR_OFF_PVR			0xe00a0
#define PPC85xx_CCSR_OFF_SVR			0xe00a4
/* Memory controller */
#define PPC85xx_CCSR_OFF_CSn_BNDS(n)	(0x02000 + (n)*8)
#define PPC85xx_CCSR_OFF_CSn_CONFIG(n)	(0x02080 + (n)*4)
/* Clock stuff */
#define PPC85xx_CCSR_OFF_PORPLLSR		0xe0000
#define PPC85xx_CCSR_OFF_PORBMSR		0xe0004
#define PPC85xx_CCSR_OFF_PORIMPSCR		0xe0008
#define PPC85xx_CCSR_OFF_PORDEVSR		0xe000c
#define PPC85xx_CCSR_OFF_PORDBGMSR		0xe0010
#define PPC85xx_CCSR_OFF_GPPORCR		0xe0020
#define PPC85xx_CCSR_OFF_GPIOCR			0xe0030
#define PPC85xx_CCSR_OFF_GPOUTDR		0xe0040
#define PPC85xx_CCSR_OFF_GPINDR			0xe0050
#define PPC85xx_CCSR_OFF_PMUXCR			0xe0060
#define PPC85xx_CCSR_OFF_DEVDISR		0xe0070
#define PPC85xx_CCSR_OFF_POWMGTCSR		0xe0080
#define PPC85xx_CCSR_OFF_MCPSUMR		0xe0090
#define PPC85xx_CCSR_OFF_RSTCR			0xe00b0
#define PPC85xx_CCSR_OFF_CLKOCR			0xe0e00
#define PPC85xx_CCSR_OFF_DDRDLLCR		0xe0e10
#define PPC85xx_CCSR_OFF_LBCDLLCR		0xe0e20
/* Interrupt controller */
#define PPC85xx_CCSR_OFF_IPIDR(n)		(0x40040 + (n)*0x10)
#define PPC85xx_CCSR_OFF_CTPR			0x40080
#define PPC85xx_CCSR_OFF_WHOAMI			0x40090
#define PPC85xx_CCSR_OFF_IACK			0x400a0
#define PPC85xx_CCSR_OFF_EOI			0x400b0
#define PPC85xx_CCSR_OFF_FRR			0x41000
#define PPC85xx_CCSR_OFF_GCR			0x41020
#define PPC85xx_CCSR_OFF_VIR			0x41080
#define PPC85xx_CCSR_OFF_PIR			0x41090
#define PPC85xx_CCSR_OFF_IPIVPR(n)		(0x410a0 + (n)*0x10)
#define PPC85xx_CCSR_OFF_PIC_SVR		0x410e0
#define PPC85xx_CCSR_OFF_TFRR			0x410f0
#define PPC85xx_CCSR_OFF_GTCCR(n)		(0x41100 + (n)*0x40)
#define PPC85xx_CCSR_OFF_GTBCR(n)		(0x41110 + (n)*0x40)
#define PPC85xx_CCSR_OFF_GTVPR(n)		(0x41120 + (n)*0x40)
#define PPC85xx_CCSR_OFF_GTDR(n)		(0x41130 + (n)*0x40)
#define PPC85xx_CCSR_OFF_TCR			0x41300
#define PPC85xx_CCSR_OFF_IRQSR0			0x41310
#define PPC85xx_CCSR_OFF_IRQSR1			0x41320
#define PPC85xx_CCSR_OFF_CIRSR0			0x41330
#define PPC85xx_CCSR_OFF_CIRSR1			0x41340
#define PPC85xx_CCSR_OFF_PMMR_even(n)	(0x41350+ (n)*0x10) /* n = 0,2,4,6 */
#define PPC85xx_CCSR_OFF_PMMR_odd(n)	(0x41350+ (n)*0x10) /* n = 1,3,5,7 */
#define PPC85xx_CCSR_OFF_MSGR(n)		(0x41400 + (n)*0x10)
#define PPC85xx_CCSR_OFF_MER			0x41500
#define PPC85xx_CCSR_OFF_MSR			0x41510
#define PPC85xx_CCSR_OFF_EIVPR(n)		(0x50000 + (n)*0x20)
#define PPC85xx_CCSR_OFF_EIDR(n)		(0x50010 + (n)*0x20)
#define PPC85xx_CCSR_OFF_IIVPR(n)		(0x50200 + (n)*0x20)
#define PPC85xx_CCSR_OFF_IIDR(n)		(0x50210 + (n)*0x20)
#define PPC85xx_CCSR_OFF_MIVPR(n)		(0x51600 + (n)*0x20)
#define PPC85xx_CCSR_OFF_MIDR(n)		(0x51610 + (n)*0x20)
#define PPC85xx_CCSR_OFF_MSIVPR(n)		(0x51c00 + (n)*0x20)
#define PPC85xx_CCSR_OFF_MSIDR(n)		(0x51C10 + (n)*0x20)
#define PPC85xx_CCSR_OFF_PC_IPIDR(n)	(0x60040 + (n)*0x10)
#define PPC85xx_CCSR_OFF_PC_CTPTR		0x60080
#define PPC85xx_CCSR_OFF_PC_WHOAMI		0x60090
#define PPC85xx_CCSR_OFF_PC_IACK		0x600a0
#define PPC85xx_CCSR_OFF_PC_EOI			0x600b0


#define PPC85xx_SPEFSCR_SOVH		_ONEBIT32B(0)
#define PPC85xx_SPEFSCR_OVH			_ONEBIT32B(1)
#define PPC85xx_SPEFSCR_FGH			_ONEBIT32B(2)
#define PPC85xx_SPEFSCR_FXH			_ONEBIT32B(3)
#define PPC85xx_SPEFSCR_FINVH		_ONEBIT32B(4)
#define PPC85xx_SPEFSCR_FDBZH		_ONEBIT32B(5)
#define PPC85xx_SPEFSCR_FUNFH		_ONEBIT32B(6)
#define PPC85xx_SPEFSCR_FOVFH		_ONEBIT32B(7)
#define PPC85xx_SPEFSCR_FINXS		_ONEBIT32B(10)
#define PPC85xx_SPEFSCR_FINVS		_ONEBIT32B(11)
#define PPC85xx_SPEFSCR_FDBZS		_ONEBIT32B(12)
#define PPC85xx_SPEFSCR_FUNFS		_ONEBIT32B(13)
#define PPC85xx_SPEFSCR_FOVFS		_ONEBIT32B(14)
#define PPC85xx_SPEFSCR_MODE		_ONEBIT32B(15)
#define PPC85xx_SPEFSCR_SOV			_ONEBIT32B(16)
#define PPC85xx_SPEFSCR_OV			_ONEBIT32B(17)
#define PPC85xx_SPEFSCR_FG			_ONEBIT32B(18)
#define PPC85xx_SPEFSCR_FX			_ONEBIT32B(19)
#define PPC85xx_SPEFSCR_FINV		_ONEBIT32B(20)
#define PPC85xx_SPEFSCR_FDBZ		_ONEBIT32B(21)
#define PPC85xx_SPEFSCR_FUNF		_ONEBIT32B(22)
#define PPC85xx_SPEFSCR_FOVF		_ONEBIT32B(23)
#define PPC85xx_SPEFSCR_FINXE		_ONEBIT32B(25)
#define PPC85xx_SPEFSCR_FINVE		_ONEBIT32B(26)
#define PPC85xx_SPEFSCR_FDBZE		_ONEBIT32B(27)
#define PPC85xx_SPEFSCR_FUNFE		_ONEBIT32B(28)
#define PPC85xx_SPEFSCR_FOVFE		_ONEBIT32B(29)
#define PPC85xx_SPEFSCR_FRMC		_ONEBIT32B(30)
#define PPC85xx_SPEFSCR_FRMC_MASK	_BITFIELD32B( 31, 0x3 )
#define PPC85xx_SPEFSCR_FRMC_NEAREST _BITFIELD32B( 31, 0 )
#define PPC85xx_SPEFSCR_FRMC_ZERO	_BITFIELD32B( 31, 1 )
#define PPC85xx_SPEFSCR_FRMC_PINF	_BITFIELD32B( 31, 2 )
#define PPC85xx_SPEFSCR_FRMC_NINF	_BITFIELD32B( 31, 3 )



#endif

/* __SRCVERSION("85xxcpu.h $Rev: 165241 $"); */
