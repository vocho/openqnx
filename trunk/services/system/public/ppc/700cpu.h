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
 *  ppc/700cpu.h
 *
 * Registers specific to the 604/620, 740/750 and 7400 series
 *

 */

#ifndef __PPC_700CPU_H_INCLUDED
#define __PPC_700CPU_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif


/*
 * SPR registers
 */
#define	PPC7400_SPR_VRSAVE		256
#define	PPC700_SPR_PMC1			953
#define	PPC700_SPR_PMC2			954
#define	PPC700_SPR_PMC3			957
#define	PPC700_SPR_PMC4			958
#define	PPC700_SPR_MMCR0		952
#define	PPC700_SPR_MMCR1		956
#define	PPC700_SPR_SDA			959
#define	PPC700_SPR_SIA			955
#define	PPC700_SPR_HID0			1008
#define	PPC700_SPR_HID1			1009
#define	PPC700_SPR_IABR			1010
#define	PPC700_SPR_ICTRL		1011
#define	PPC700_SPR_DABR			1013
#define	PPC700_SPR_MSSCR0		1014
#define	PPC700_SPR_MSSSR0		1015
#define	PPC700_SPR_LDSTCR		1016
#define	PPC700_SPR_L2CR			1017
#define	PPC700_SPR_L3CR			1018
#define	PPC700_SPR_PIR			1023

/*
 * Exception table vectors
 */
#define PPC700_EXC_ITMISS		(0x1000/sizeof(uint32_t)) 
#define PPC700_EXC_DLTMISS		(0x1100/sizeof(uint32_t)) 
#define PPC700_EXC_DSTMISS		(0x1200/sizeof(uint32_t)) 
#define PPC700_EXC_IABREAKPOINT	(0x1300/sizeof(uint32_t)) 
#define PPC700_EXC_SYSTEM_MGT	(0x1400/sizeof(uint32_t)) 

/*
 * HID0
 */
#define	PPC700_SPR_HID0_RECP	_ONEBIT32B(0)
#define	PPC700_SPR_HID0_EBA		_ONEBIT32B(2)
#define	PPC700_SPR_HID0_EBD		_ONEBIT32B(3)
#define	PPC700_SPR_HID0_SBCLK	_ONEBIT32B(4)
#define	PPC700_SPR_HID0_EICE	_ONEBIT32B(5)
#define	PPC700_SPR_HID0_ECLK	_ONEBIT32B(6)
#define	PPC700_SPR_HID0_PAR		_ONEBIT32B(7)
#define	PPC700_SPR_HID0_DOZE	_ONEBIT32B(8)
#define	PPC700_SPR_HID0_HBE		_ONEBIT32B(8)
#define	PPC700_SPR_HID0_NAP		_ONEBIT32B(9)
#define	PPC700_SPR_HID0_SLEEP	_ONEBIT32B(10)
#define	PPC700_SPR_HID0_DPM		_ONEBIT32B(11)
#define	PPC700_SPR_HID0_RISEG	_ONEBIT32B(12)
#define	PPC700_SPR_HID0_NHR		_ONEBIT32B(15)
#define	PPC700_SPR_HID0_ICE		_ONEBIT32B(16)
#define	PPC700_SPR_HID0_DCE		_ONEBIT32B(17)
#define	PPC700_SPR_HID0_ILOCK	_ONEBIT32B(18)
#define	PPC700_SPR_HID0_DLOCK	_ONEBIT32B(19)
#define	PPC700_SPR_HID0_ICFI	_ONEBIT32B(20)
#define	PPC700_SPR_HID0_DCFI	_ONEBIT32B(21)
#define	PPC700_SPR_HID0_SPD		_ONEBIT32B(22)
#define	PPC700_SPR_HID0_IFEM	_ONEBIT32B(23)
#define	PPC700_SPR_HID0_SGE		_ONEBIT32B(24)
#define	PPC700_SPR_HID0_DCFA	_ONEBIT32B(25)
#define	PPC700_SPR_HID0_BTIC	_ONEBIT32B(26)
#define	PPC700_SPR_HID0_FBIOB	_ONEBIT32B(27)
#define	PPC700_SPR_HID0_ABE		_ONEBIT32B(28)
#define	PPC700_SPR_HID0_BHT		_ONEBIT32B(29)
#define	PPC700_SPR_HID0_BTAC	_ONEBIT32B(30)
#define	PPC700_SPR_HID0_NOOPTI	_ONEBIT32B(31)

#define	PPC7450_SPR_HID0_TBEN	_ONEBIT32B(5)
#define	PPC7450_SPR_HID0_XAEN	_ONEBIT32B(14)
#define	PPC7450_SPR_HID0_LRSTK	_ONEBIT32B(27)

#define	PPC7450_SPR_HID1_SYNCBE	_ONEBIT32B(20)
#define	PPC7450_SPR_HID1_ABE	_ONEBIT32B(21)

/* 
 * 7450 series performance counters
 */
#define PPC7450_SPR_PMC1	PPC700_SPR_PMC1
#define PPC7450_SPR_PMC2	PPC700_SPR_PMC2
#define PPC7450_SPR_PMC3	PPC700_SPR_PMC3
#define PPC7450_SPR_PMC4	PPC700_SPR_PMC4
#define PPC7450_SPR_PMC5	945
#define PPC7450_SPR_PMC6	946

#define PPC7450_SPR_MMCR0	PPC700_SPR_MMCR0
#define PPC7450_SPR_MMCR1	PPC700_SPR_MMCR1
#define PPC7450_SPR_MMCR2	944

#define	PPC7450_SPR_SIA		PPC700_SPR_SIA

/*
 * L2CR (700/7400 series only)
 */
#define	PPC700_SPR_L2CR_L2E		_ONEBIT32B(0)
#define	PPC700_SPR_L2CR_L2PE	_ONEBIT32B(1)
#define	PPC700_SPR_L2CR_L2DO	_ONEBIT32B(9)
#define	PPC700_SPR_L2CR_L2I		_ONEBIT32B(10)
#define	PPC700_SPR_L2CR_CTL		_ONEBIT32B(11)
#define	PPC700_SPR_L2CR_L2WT	_ONEBIT32B(12)
#define	PPC700_SPR_L2CR_L2DF	_ONEBIT32B(17)
#define	PPC700_SPR_L2CR_L2IP	_ONEBIT32B(31)

#define	PPC700_SPR_L2CR_256K	_BITFIELD32B(3,1)
#define	PPC700_SPR_L2CR_512K	_BITFIELD32B(3,2)
#define	PPC700_SPR_L2CR_1M		_BITFIELD32B(3,3)
#define	PPC700_SPR_L2CR_2M		_BITFIELD32B(3,0)

#define	PPC700_SPR_L2CR_CLK1	_BITFIELD32B(6,1)
#define	PPC700_SPR_L2CR_CLK15	_BITFIELD32B(6,2)
#define	PPC700_SPR_L2CR_CLK2	_BITFIELD32B(6,4)
#define	PPC700_SPR_L2CR_CLK25	_BITFIELD32B(6,5)
#define	PPC700_SPR_L2CR_CLK3	_BITFIELD32B(6,6)
#define	PPC700_SPR_L2CR_CLK35	_BITFIELD32B(6,3)
#define	PPC700_SPR_L2CR_CLK4	_BITFIELD32B(6,7)

#define	PPC700_SPR_L2CR_PBSRAM	_BITFIELD32B(8,2)
#define	PPC700_SPR_L2CR_PLWSRAM	_BITFIELD32B(8,3)

#define	PPC700_SPR_L2CR_OH05	_BITFIELD32B(15,0)
#define	PPC700_SPR_L2CR_OH10	_BITFIELD32B(15,1)

/*
 * MMU
 */
 
#endif

/* __SRCVERSION("700cpu.h $Rev: 169331 $"); */
