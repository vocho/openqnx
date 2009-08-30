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
 *  ppc/86xxcpu.h
 *
 * Registers specific to the Motorola MPC86xx
 *
 */

#ifndef __PPC_86XXCPU_H_INCLUDED
#define __PPC_86XXCPU_H_INCLUDED

#ifndef __PPC_E500CPU_H_INCLUDED
#include <ppc/e500cpu.h>
#endif


/*
 * DDR Chip Select n Memory Bounds
 */
#define PPC86xx_CSn_BNDS_SA_MASK	_BITFIELD32B(15,0xff)
#define PPC86xx_CSn_BNDS_SA_SHIFT	(31-15)
#define PPC86xx_CSn_BNDS_EA_MASK	_BITFIELD32B(31,0xff)
#define PPC86xx_CSn_BNDS_EA_SHIFT	0

/*
 * DDR Chip Select n Configuration
 */
#define PPC86xx_CSn_CONFIG_CS_EN			_ONEBIT32B(0)
#define PPC86xx_CSn_CONFIG_AP_EN			_ONEBIT32B(8)
#define PPC86xx_CSn_CONFIG_ROW_BITS_MASK	_BITFIELD32B(23,0x7)
#define PPC86xx_CSn_CONFIG_ROW_BITS_SHIFT	(31-23)
#define PPC86xx_CSn_CONFIG_ROW_BITS_12		_BITFIELD32B(23,0x0)
#define PPC86xx_CSn_CONFIG_ROW_BITS_13		_BITFIELD32B(23,0x1)
#define PPC86xx_CSn_CONFIG_ROW_BITS_14		_BITFIELD32B(23,0x2)
#define PPC86xx_CSn_CONFIG_COL_BITS_MASK	_BITFIELD32B(23,0x7)
#define PPC86xx_CSn_CONFIG_COL_BITS_SHIFT	(31-31)
#define PPC86xx_CSn_CONFIG_COL_BITS_8		_BITFIELD32B(31,0x0)
#define PPC86xx_CSn_CONFIG_COL_BITS_9		_BITFIELD32B(31,0x1)
#define PPC86xx_CSn_CONFIG_COL_BITS_10		_BITFIELD32B(31,0x2)
#define PPC86xx_CSn_CONFIG_COL_BITS_11		_BITFIELD32B(31,0x3)
 
/*
 * POR PLL Status Register
 */
#define PPC86xx_PORPLLSR_E500_MASK		_BITFIELD32B(15,0x1f)
#define PPC86xx_PORPLLSR_E500_SHIFT		(31-15)
#define PPC86xx_PORPLLSR_PLAT_MASK		_BITFIELD32B(30,0x1f)
#define PPC86xx_PORPLLSR_PLAT_SHIFT		(31-30)
#define PPC86xx_PORPLLSR_BYPASS			_ONEBIT32B(31)

/*
 * PIC Feature Reporting Register
 */
#define PPC86xx_FRR_NIRQ_MASK			_BITFIELD32B(15,0x7f)
#define PPC86xx_FRR_NIRQ_SHIFT			(31-15)
#define PPC86xx_FRR_NCPU_MASK			_BITFIELD32B(23,0x1f)
#define PPC86xx_FRR_NCPU_SHIFT			(31-23)
#define PPC86xx_FRR_VID_MASK			_BITFIELD32B(31,0xff)
#define PPC86xx_FRR_VID_SHIFT			(31-31)

/*
 * PIC Global Configuration Register
 */
#define PPC86xx_GCR_RST					_ONEBIT32B(0)
#define PPC86xx_GCR_M					_ONEBIT32B(2)

/*
 * PIC Vendor Identification Register
 */
#define PPC86xx_VIR_STEP_MASK			_BITFIELD32B(15,0xff)
#define PPC86xx_VIR_STEP_SHIFT			(31-15)
#define PPC86xx_VIR_DEVICE_ID_MASK		_BITFIELD32B(23,0xff)
#define PPC86xx_VIR_DEVICE_ID_SHIFT		(31-23)
#define PPC86xx_VIR_VENDOR_ID_MASK		_BITFIELD32B(31,0xff)
#define PPC86xx_VIR_VENDOR_ID_SHIFT		(31-31)

/*
 * PIC Processor Initialization Register
 */
#define PPC86xx_PIR_P0					_ONEBIT32B(31)

/*
 * PIC IPI/Timer/Message/External/Internal Interrupt Vector/Priority Regster
 */
#define PPC86xx_xIVPR_MSK				_ONEBIT32B(0)
#define PPC86xx_xIVPR_A					_ONEBIT32B(1)
#define PPC86xx_xIVPR_P					_ONEBIT32B(8)
#define PPC86xx_xIVPR_S					_ONEBIT32B(9)
#define PPC86xx_xIVPR_PRIORITY_MASK		_BITFIELD32B(15,0xf)
#define PPC86xx_xIVPR_PRIORITY_SHIFT	(31-15)
#define PPC86xx_xIVPR_VECTOR_MASK		_BITFIELD32B(31,0xffff)
#define PPC86xx_xIVPR_VECTOR_SHIFT		(31-31)

/*
 * PIC IPI/Timer/Message/External/Internal Interrupt Destination Regster
 */
#define PPC86xx_xIDR_EP					_ONEBIT32B(0)
#define PPC86xx_xIDR_CI					_ONEBIT32B(1)
#define PPC86xx_xIDR_P0					_ONEBIT32B(31)

/*
 * PIC Global Timer Current Count Register
 */
#define PPC86xx_GTCCR_TOG				_ONEBIT32B(0)
#define PPC86xx_GTCCR_COUNT_MASK		_BITFIELD32B(31,0x7fffffff)
#define PPC86xx_GTCCR_COUNT_SHIFT		(31-31)

/*
 * PIC Global Timer Base Count Register
 */
#define PPC86xx_GTBCR_CI				_ONEBIT32B(0)
#define PPC86xx_GTBCR_BASE_CNT_MASK		_BITFIELD32B(31,0x7fffffff)
#define PPC86xx_GTBCR_BASE_CNT_SHIFT	(31-31)

/*
 * PIC Timer Control Register
 */
#define PPC86xx_TCR_ROVR_MASK			_BITFIELD32B(7,0x7)
#define PPC86xx_TCR_ROVR_SHIFT			(31-7)
#define PPC86xx_TCR_ROVR_0				_ONEBIT32B(7)
#define PPC86xx_TCR_ROVR_1				_ONEBIT32B(6)
#define PPC86xx_TCR_ROVR_2				_ONEBIT32B(5)
#define PPC86xx_TCR_CLKR_MASK			_BITFIELD32B(23,0x3)
#define PPC86xx_TCR_CLKR_SHIFT			(31-23)
#define PPC86xx_TCR_CLKR_DIV8			_BITFIELD32B(23,0x0)
#define PPC86xx_TCR_CLKR_DIV16			_BITFIELD32B(23,0x1)
#define PPC86xx_TCR_CLKR_DIV32			_BITFIELD32B(23,0x2)
#define PPC86xx_TCR_CLKR_DIV64			_BITFIELD32B(23,0x3)
#define PPC86xx_TCR_CASC_MASK			_BITFIELD32B(31,0x7)
#define PPC86xx_TCR_CASC_SHIFT			(31-31)
#define PPC86xx_TCR_CASC_NONE			_BITFIELD32B(31,0x0)
#define PPC86xx_TCR_CASC_0AND1			_BITFIELD32B(31,0x1)
#define PPC86xx_TCR_CASC_1AND2			_BITFIELD32B(31,0x2)
#define PPC86xx_TCR_CASC_0AND1AND2		_BITFIELD32B(31,0x3)
#define PPC86xx_TCR_CASC_2AND3			_BITFIELD32B(31,0x4)
#define PPC86xx_TCR_CASC_0AND1_2AND3	_BITFIELD32B(31,0x5)
#define PPC86xx_TCR_CASC_1AND2AND3		_BITFIELD32B(31,0x6)
#define PPC86xx_TCR_CASC_0AND1AND2AND3	_BITFIELD32B(31,0x7)

/*
 * PIC Performance Monitor Mask Register (Even)
 */
#define PPC86xx_PMMRE_IPI(n)			_ONEBIT32B(8+(n))
#define PPC86xx_PMMRE_TIMER(n)			_ONEBIT32B(12+(n))
#define PPC86xx_PMMRE_MSG(n)			_ONEBIT32B(16+(n))
#define PPC86xx_PMMRE_EXT(n)			_ONEBIT32B(20+(n))

/*
 * PIC Performance Monitor Mask Register (Odd)
 */
#define PPC86xx_PMMRO_INT(n)			_ONEBIT32B((n))

/*
 * PIC Message Enable Register
 */
#define PPC86xx_MER_E3					_ONEBIT32B(28)
#define PPC86xx_MER_E2					_ONEBIT32B(29)
#define PPC86xx_MER_E1					_ONEBIT32B(30)
#define PPC86xx_MER_E0					_ONEBIT32B(31)

/*
 * PIC Message Status Register
 */
#define PPC86xx_MSR_S3					_ONEBIT32B(28)
#define PPC86xx_MSR_S2					_ONEBIT32B(29)
#define PPC86xx_MSR_S1					_ONEBIT32B(30)
#define PPC86xx_MSR_S0					_ONEBIT32B(31)

/*
 * PIC Interprocessor Interrupt Dispatch Register
 */
#define PPC86xx_IPIDR_P0				_ONEBIT32B(31)


/*
 * CCSR Offsets of memory mapped registers
 */


/* MPX Coherency Module */
#define PPC86xx_CCSR_OFF_MPX			0x1000


#define PPC86xx_CCSR_OFF_CCSRBAR		0x00000
#define PPC86xx_CCSR_OFF_ALTCBAR		0x00008
#define PPC86xx_CCSR_OFF_ALTCSR			0x00010
#define PPC86xx_CCSR_OFF_BPTR			0x00020
#define PPC86xx_CCSR_OFF_PVR			0xe00a0
#define PPC86xx_CCSR_OFF_SVR			0xe00a4
/* Memory controller */
#define PPC86xx_CCSR_OFF_CSn_BNDS(n)	(0x02000 + (n)*8)
#define PPC86xx_CCSR_OFF_CSn_CONFIG(n)	(0x02080 + (n)*4)
/* Clock stuff */
#define PPC86xx_CCSR_OFF_PORPLLSR		0xe0000
#define PPC86xx_CCSR_OFF_PORBMSR		0xe0004
#define PPC86xx_CCSR_OFF_PORIMPSCR		0xe0008
#define PPC86xx_CCSR_OFF_PORDEVSR		0xe000c
#define PPC86xx_CCSR_OFF_PORDBGMSR		0xe0010
#define PPC86xx_CCSR_OFF_GPPORCR		0xe0020
#define PPC86xx_CCSR_OFF_GPIOCR			0xe0030
#define PPC86xx_CCSR_OFF_GPOUTDR		0xe0040
#define PPC86xx_CCSR_OFF_GPINDR			0xe0050
#define PPC86xx_CCSR_OFF_PMUXCR			0xe0060
#define PPC86xx_CCSR_OFF_DEVDISR		0xe0070
#define PPC86xx_CCSR_OFF_POWMGTCSR		0xe0080
#define PPC86xx_CCSR_OFF_MCPSUMR		0xe0090
#define PPC86xx_CCSR_OFF_CLKOCR			0xe0e00
#define PPC86xx_CCSR_OFF_DDRDLLCR		0xe0e10
#define PPC86xx_CCSR_OFF_LBCDLLCR		0xe0e20
/* Interrupt controller */
#define PPC86xx_CCSR_OFF_IPIDR(n)		(0x40040 + (n)*0x10)
#define PPC86xx_CCSR_OFF_CTPR			0x40080
#define PPC86xx_CCSR_OFF_WHOAMI			0x40090
#define PPC86xx_CCSR_OFF_IACK			0x400a0
#define PPC86xx_CCSR_OFF_EOI			0x400b0
#define PPC86xx_CCSR_OFF_FRR			0x41000
#define PPC86xx_CCSR_OFF_GCR			0x41020
#define PPC86xx_CCSR_OFF_VIR			0x41080
#define PPC86xx_CCSR_OFF_PIR			0x41090
#define PPC86xx_CCSR_OFF_IPIVPR(n)		(0x410a0 + (n)*0x10)
#define PPC86xx_CCSR_OFF_PIC_SVR		0x410e0
#define PPC86xx_CCSR_OFF_TFRR			0x410f0
#define PPC86xx_CCSR_OFF_GTCCRA(n)		(0x41100 + (n)*0x40)
#define PPC86xx_CCSR_OFF_GTBCRA(n)		(0x41110 + (n)*0x40)
#define PPC86xx_CCSR_OFF_GTVPRA(n)		(0x41120 + (n)*0x40)
#define PPC86xx_CCSR_OFF_GTDRA(n)		(0x41130 + (n)*0x40)
#define PPC86xx_CCSR_OFF_GTCCRB(n)		(0x42100 + (n)*0x40)
#define PPC86xx_CCSR_OFF_GTBCRB(n)		(0x42110 + (n)*0x40)
#define PPC86xx_CCSR_OFF_GTVPRB(n)		(0x42120 + (n)*0x40)
#define PPC86xx_CCSR_OFF_GTDRB(n)		(0x42130 + (n)*0x40)
#define PPC86xx_CCSR_OFF_TCR			0x41300
#define PPC86xx_CCSR_OFF_IRQSR0			0x41310
#define PPC86xx_CCSR_OFF_IRQSR1			0x41320
#define PPC86xx_CCSR_OFF_CIRSR0			0x41330
#define PPC86xx_CCSR_OFF_CIRSR1			0x41340
#define PPC86xx_CCSR_OFF_PMMR_even(n)	(0x41350+ (n)*0x10) /* n = 0,2,4,6 */
#define PPC86xx_CCSR_OFF_PMMR_odd(n)	(0x41350+ (n)*0x10) /* n = 1,3,5,7 */
#define PPC86xx_CCSR_OFF_MSGR(n)		(0x41400 + (n)*0x10)
#define PPC86xx_CCSR_OFF_MER			0x41500
#define PPC86xx_CCSR_OFF_MSR			0x41510
#define PPC86xx_CCSR_OFF_EIVPR(n)		(0x50000 + (n)*0x20)
#define PPC86xx_CCSR_OFF_EIDR(n)		(0x50010 + (n)*0x20)
#define PPC86xx_CCSR_OFF_IIVPR(n)		(0x50200 + (n)*0x20)
#define PPC86xx_CCSR_OFF_IIDR(n)		(0x50210 + (n)*0x20)
#define PPC86xx_CCSR_OFF_MIVPR(n)		(0x51600 + (n)*0x20)
#define PPC86xx_CCSR_OFF_MIDR(n)		(0x51610 + (n)*0x20)
#define PPC86xx_CCSR_OFF_PC_IPIDR(n)	(0x60040 + (n)*0x10)
#define PPC86xx_CCSR_OFF_MSIVPR(n)		(0x51C00 + (n)*0x20)
#define PPC86xx_CCSR_OFF_MSIDR(n)		(0x51C10 + (n)*0x20)
#define PPC86xx_CCSR_OFF_PC_CTPTR		0x60080
#define PPC86xx_CCSR_OFF_PC_WHOAMI		0x60090
#define PPC86xx_CCSR_OFF_PC_IACK		0x600a0
#define PPC86xx_CCSR_OFF_PC_EOI			0x600b0
/* LAWs */
#define PPC86xx_CCSR_OFF_LAWBAR(n)		(0xc08 + (n)*0x20)  /* n = 0-9 */
#define PPC86xx_CCSR_OFF_LAWAR(n)		(0xc10 + (n)*0x20)  /* n = 0-9 */ 
/* PEX 1*/
#define PPC86xx_CCSR_OFF_PEX1_CFG_ADDR      0x8000
#define PPC86xx_CCSR_OFF_PEX1_CFG_DATA      0x8004
#define PPC86xx_CCSR_OFF_PEX1_PEXOTAR(n)    (0x8c00 + (n)*0x20) /*n= 0-3*/
#define PPC86xx_CCSR_OFF_PEX1_PEXOWBAR(n)   (0x8c08 + (n)*0x20) /*n= 1-3*/
#define PPC86xx_CCSR_OFF_PEX1_PEXOWAR(n)    (0x8c10 + (n)*0x20) /*n= 0-3*/
/* PEX 2*/
#define PPC86xx_CCSR_OFF_PEX2_CFG_ADDR      0x9000
#define PPC86xx_CCSR_OFF_PEX2_CFG_DATA      0x9004
#define PPC86xx_CCSR_OFF_PEX2_PEXOTAR(n)    (0x9c00 + (n)*0x20) /*n= 0-3*/
#define PPC86xx_CCSR_OFF_PEX2_PEXOWBAR(n)   (0x9c08 + (n)*0x20) /*n= 1-3*/
#define PPC86xx_CCSR_OFF_PEX2_PEXOWAR(n)    (0x9c10 + (n)*0x20) /*n= 0-3*/




#endif
/* __SRCVERSION("86xxcpu.h $Rev: 153052 $"); */
