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
 *  ppc/603cpu.h
 *
 * Registers specific to the 600 series and 603
 *

 */

#ifndef __PPC_603CPU_H_INCLUDED
#define __PPC_603CPU_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif


/*
 * SPR registers
 */
#define	PPC603_SPR_DMISS		976
#define	PPC603_SPR_DCMP			977
#define	PPC603_SPR_HASH1		978
#define	PPC603_SPR_HASH2		979
#define	PPC603_SPR_IMISS		980
#define	PPC603_SPR_ICMP			981
#define	PPC603_SPR_RPA			982
#define	PPC603_SPR_HID0			1008
#define	PPC603_SPR_HID1			1009
#define	PPC603_SPR_IABR			1010

/*
 * Exception table vectors
 */
#define PPC603_EXC_ITMISS		(0x1000/sizeof(uint32_t)) 
#define PPC603_EXC_DLTMISS		(0x1100/sizeof(uint32_t)) 
#define PPC603_EXC_DSTMISS		(0x1200/sizeof(uint32_t)) 
#define PPC603_EXC_IABREAKPOINT	(0x1300/sizeof(uint32_t)) 
#define PPC603_EXC_SYSTEM_MGT	(0x1400/sizeof(uint32_t)) 

/*
 * HID0
 */
#define	PPC603_SPR_HID0_RECP	_ONEBIT32B(0)
#define	PPC603_SPR_HID0_EBA		_ONEBIT32B(2)
#define	PPC603_SPR_HID0_EBD		_ONEBIT32B(3)
#define	PPC603_SPR_HID0_SBCLK	_ONEBIT32B(4)
#define	PPC603_SPR_HID0_EICE	_ONEBIT32B(5)
#define	PPC603_SPR_HID0_ECLK	_ONEBIT32B(6)
#define	PPC603_SPR_HID0_PAR		_ONEBIT32B(7)
#define	PPC603_SPR_HID0_DOZE	_ONEBIT32B(8)
#define	PPC603_SPR_HID0_NAP		_ONEBIT32B(9)
#define	PPC603_SPR_HID0_SLEEP	_ONEBIT32B(10)
#define	PPC603_SPR_HID0_DPM		_ONEBIT32B(11)
#define	PPC603_SPR_HID0_RISEG	_ONEBIT32B(12)
#define	PPC603_SPR_HID0_NHR		_ONEBIT32B(15)
#define	PPC603_SPR_HID0_ICE		_ONEBIT32B(16)
#define	PPC603_SPR_HID0_DCE		_ONEBIT32B(17)
#define	PPC603_SPR_HID0_ILOCK	_ONEBIT32B(18)
#define	PPC603_SPR_HID0_DLOCK	_ONEBIT32B(19)
#define	PPC603_SPR_HID0_ICFI	_ONEBIT32B(20)
#define	PPC603_SPR_HID0_DCFI	_ONEBIT32B(21)
#define	PPC603_SPR_HID0_IFEM	_ONEBIT32B(24)
#define	PPC603_SPR_HID0_FBIOB	_ONEBIT32B(27)
#define	PPC603_SPR_HID0_ABE		_ONEBIT32B(28)
#define	PPC603_SPR_HID0_NOOPTI	_ONEBIT32B(31)

/*
 * MMU
 */
 
#endif

/* __SRCVERSION("603cpu.h $Rev: 153052 $"); */
