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



#ifndef __MIPS_TX3927CPU_H_INCLUDED
#define __MIPS_TX3927CPU_H_INCLUDED
/*
 *  mips/tx3927cpu.h
 *

 */

#ifdef __ASM__
    #include <mips/cpu.h>
#else
	#ifndef __PLATFORM_H_INCLUDED
	#include <sys/platform.h>
	#endif
	#include _NTO_HDR_(mips/cpu.h)
#endif

/*
** Timer Registers
*/
#define TX3927_TIMERREG(__t,__o)	(0xfffef000 + (__t)*0x100 + __o)

/*
** Timer Control Register
*/
#define TX3927_TMTCRx(__t)	TX3927_TIMERREG(__t,0x00)

#define TX3927_TMTCRx_TMODE_MASK		0x00000003
#define TX3927_TMTCRx_TMODE_SHIFT		0
#define TX3927_TMTCRx_TMODE_INTERVAL	0x00000000
#define TX3927_TMTCRx_TMODE_PULSE		0x00000001
#define TX3927_TMTCRx_TMODE_WATCHDOG	0x00000002
#define TX3927_TMTCRx_TMODE_RESERVED	0x00000003
#define TX3927_TMTCRx_CCS				0x00000004
#define TX3927_TMTCRx_ECES				0x00000008
#define TX3927_TMTCRx_CRE				0x00000020
#define TX3927_TMTCRx_CCDE				0x00000040
#define TX3927_TMTCRx_TCE				0x00000080

/*
** Timer Interrupt Status Register
*/
#define TX3927_TMTISRx(__t)		TX3927_TIMERREG(__t,0x04)

#define TX3927_TMTISRx_TIIS				0x00000001
#define TX3927_TMTISRx_TPIAS			0x00000002
#define TX3927_TMTISRx_TPIBS			0x00000004

/*
** Timer Compare Registers
*/
#define TX3927_TMCPRAx(__t)		TX3927_TIMERREG(__t,0x08)
#define TX3927_TMCPRBx(__t)		TX3927_TIMERREG(__t,0x0c)

/*
** Interval Timer Mode Register
*/
#define TX3927_TMITMRx(__t)		TX3927_TIMERREG(__t,0x10)

#define TX3927_TMITMRx_TZCE				0x00000001
#define TX3927_TMITMRx_TIIE				0x00008000

/*
** Timer Divide Register
*/
#define TX3927_TMCCDRx(__t)		TX3927_TIMERREG(__t,0x20)

/*
** Timer Pulse Generator Mode Register
*/
#define TX3927_TMPGMRx(__t)		TX3927_TIMERREG(__t,0x30)

#define TX3927_TMPGMRx_FFI				0x00000001
#define TX3927_TMPGMRx_TPIAE			0x00004000
#define TX3927_TMPGMRx_TPIBE			0x00008000

/*
** Timer Read Register
*/
#define TX3927_TMTRRx(__t)		TX3927_TIMERREG(__t,0xf0)


/*
** Interrupt Controller Registers
*/
#define TX3927_IR_BASE			0xfffec000

/*
** Interrupt Detection Enable Register
*/
#define TX3927_IRDER_OFFSET		0x00

#define TX3927_IRDER_ICE		0x00000001

/*
** Interrupt Detection Mode Registers 0 & 1
*/
#define TX3927_IRDMR0_OFFSET	0x04
#define TX3927_IRDMR1_OFFSET	0x08

#define TX3927_IRDMRx_LOW(__l)		(0 << ((__l)*2))
#define TX3927_IRDMRx_HIGH(__l)		(1 << ((__l)*2))
#define TX3927_IRDMRx_FALLING(__l)	(2 << ((__l)*2))
#define TX3927_IRDMRx_RISING(__l)	(3 << ((__l)*2))

/*
** Interrupt Level Registers 0 => 7
*/
#define TX3927_IRILR0_OFFSET	0x10
#define TX3927_IRILR1_OFFSET	0x14
#define TX3927_IRILR2_OFFSET	0x18
#define TX3927_IRILR3_OFFSET	0x1c
#define TX3927_IRILR4_OFFSET	0x20
#define TX3927_IRILR5_OFFSET	0x24
#define TX3927_IRILR6_OFFSET	0x28
#define TX3927_IRILR7_OFFSET	0x2c

#define TX3927_IRILRx_MASK			7
#define TX3927_IRILRx_EVEN_SHIFT	0
#define TX3927_IRILRx_ODD_SHIFT		8

/*
** Interrupt Mask Register
*/
#define TX3927_IRIMR_OFFSET		0x40

#define TX3927_IRIMR_IML_MASK	0x7
#define TX3927_IRIMR_IML_SHIFT	0

/*
** Interrupt Status Control Register
*/
#define TX3927_IRSCR_OFFSET		0x60

#define TX3927_IRSCR_ElClr_MASK		0x0000000f
#define TX3927_IRSCR_ElClr_SHIFT	0
#define TX3927_IRSCR_ElClrE			0x00000100

/*
** Interrupt Source Status Register
*/
#define TX3927_IRSSR_OFFSET		0x80

/*
** Interrupt Current Status Register
*/
#define TX3927_IRCSR_OFFSET		0xa0

#define TX3927_IRCSR_IVL_MASK	0x0000000f
#define TX3927_IRCSR_IVL_SHIFT	0
#define TX3927_IRCSR_ILV_MASK	0x00000700
#define TX3927_IRCSR_ILV_SHIFT	8
#define TX3927_IRCSR_IF			0x00010000


#endif /* __MIPS_TX3927CPU_H_INCLUDED */

/* __SRCVERSION("tx3927cpu.h $Rev: 153052 $"); */
