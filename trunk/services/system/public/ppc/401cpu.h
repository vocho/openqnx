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
 *  ppc/401cpu.h
 *
 * Registers specific to the 401 chip
 *

 */

#ifndef __PPC_401CPU_H_INCLUDED
#define __PPC_401CPU_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __PPC_400CPU_H_INCLUDED
#include _NTO_HDR_(ppc/400cpu.h)
#endif

/*
 * Bus Error Syndrome 0 Register Bits
 */
#define PPC401_BESR0_IET_MASK		_BITFIELD32B( 3, 7 )
#define PPC401_BESR0_IET_NOERROR	_BITFIELD32B( 3, 0 )
#define PPC401_BESR0_IET_ACTIVE		_BITFIELD32B( 3, 6 )
#define PPC401_BESR0_DRWS			_ONEBIT32B( 4 )
#define PPC401_BESR0_DET_MASK		_BITFIELD32B( 7, 7 )
#define PPC401_BESR0_DET_NOERROR	_BITFIELD32B( 7, 0 )
#define PPC401_BESR0_DET_ACTIVE		_BITFIELD32B( 7, 6 )

/*
 *  Bus Region Control Register 0-7
 */
#define PPC401_BRCR_TR_MASK			_BITFIELD32B( 27, 7 )
#define PPC401_BRCR_TR_1CYCLE		_BITFIELD32B( 27, 1 )
#define PPC401_BRCR_TR_2CYCLE		_BITFIELD32B( 27, 2 )
#define PPC401_BRCR_TR_3CYCLE		_BITFIELD32B( 27, 3 )
#define PPC401_BRCR_TR_4CYCLE		_BITFIELD32B( 27, 4 )
#define PPC401_BRCR_TR_5CYCLE		_BITFIELD32B( 27, 5 )
#define PPC401_BRCR_TR_6CYCLE		_BITFIELD32B( 27, 6 )
#define PPC401_BRCR_TR_7CYCLE		_BITFIELD32B( 27, 7 )
#define PPC401_BRCR_MB				_ONEBIT32B( 28 )
#define PPC401_BRCR_SLF				_ONEBIT32B( 29 )
#define PPC401_BRCR_BW_MASK			_BITFIELD32B( 31, 3 )
#define PPC401_BRCR_BW_8BIT			_BITFIELD32B( 31, 0 )
#define PPC401_BRCR_BW_16BIT		_BITFIELD32B( 31, 1 )
#define PPC401_BRCR_BW_328BIT		_BITFIELD32B( 31, 2 )

/*
 * Cache Debug Control Register
 */
#define PPC401_CDCR_WOA				_ONEBIT32B( 19 )
#define PPC401_CDCR_LDBE			_ONEBIT32B( 22 )
#define PPC401_CDCR_DLXE			_ONEBIT32B( 23 )
#define PPC401_CDCR_IUXE			_ONEBIT32B( 24 )
#define PPC401_CDCR_DUXE			_ONEBIT32B( 25 )
#define PPC401_CDCR_LKE				_ONEBIT32B( 26 )
#define PPC401_CDCR_CIS				_ONEBIT32B( 27 )
#define PPC401_CDCR_CSS				_ONEBIT32B( 31 )

/*
 * Input/Output Configuration Register
 */
#define PPC401_IOCR_CIL				_ONEBIT32B( 0 )
#define PPC401_IOCR_EIL				_ONEBIT32B( 1 )
#define PPC401_IOCR_MCA				_ONEBIT32B( 23 )
#define PPC401_IOCR_EBS_MASK		_BITFIELD32B( 31, 3 )
#define PPC401_IOCR_EBS_1_1			_BITFIELD32B( 31, 0 )
#define PPC401_IOCR_EBS_1_2			_BITFIELD32B( 31, 1 )
#define PPC401_IOCR_EBS_1_3			_BITFIELD32B( 31, 2 )
#define PPC401_IOCR_EBS_1_4			_BITFIELD32B( 31, 3 )

/*
 * Power Managment Control Register
 */
#define PPC401_PMCR0_DS				_ONEBIT32B( 0 )
#define PPC401_PMCR0_NAP			_ONEBIT32B( 1 )
#define PPC401_PMCR0_CDE			_ONEBIT32B( 2 )
#define PPC401_PMCR0_TDE			_ONEBIT32B( 3 )
#define PPC401_PMCR0_CSC_MASK		_BITFIELD32B( 17, 3 )
#define PPC401_PMCR0_CSC_2_1		_BITFIELD32B( 17, 0 )
#define PPC401_PMCR0_CSC_1_1		_BITFIELD32B( 17, 1 )
#define PPC401_PMCR0_CSC_1_2		_BITFIELD32B( 17, 2 )
#define PPC401_PMCR0_CSC_1_4		_BITFIELD32B( 17, 3 )

/*
 * DCR numbers
 */
#define PPC401_DCR_IOCR		0xa0

#endif

/* __SRCVERSION("401cpu.h $Rev: 153052 $"); */
