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
 *  sh/ccn.h    SH CCN (Cache and TLB control) definitions
 *

 */

/* registers addresses */
#define	SH_CCN_BASE				0xff000000
#define SH_MMR_CCN_PTEH_OFF		0x0
#define SH_MMR_CCN_PTEL_OFF		0x4
#define SH_MMR_CCN_TTB_OFF		0x8
#define SH_MMR_CCN_TEA_OFF		0xc
#define SH_MMR_CCN_MMUCR_OFF	0x10
#define SH_MMR_CCN_PTEA_OFF		0x34

/* MMUCR */
#define	SH_CCN_MMUCR_LRUI_M		_BITFIELD32L(26, 0x3f)
#define	SH_CCN_MMUCR_LRUI(x)	_BITFIELD32L(26, x)
#define	SH_CCN_MMUCR_URB_M		_BITFIELD32L(18, 0x3f)
#define	SH_CCN_MMUCR_URB(x)		_BITFIELD32L(18, x)
#define	SH_CCN_MMUCR_URC_M		_BITFIELD32L(10, 0x3f)
#define	SH_CCN_MMUCR_URC(x)		_BITFIELD32L(10, x)
#define	SH_CCN_MMUCR_SQMD		_ONEBIT32L(9)
#define	SH_CCN_MMUCR_SV			_ONEBIT32L(8)
#define	SH_CCN_MMUCR_TI			_ONEBIT32L(2)
#define	SH_CCN_MMUCR_AT			_ONEBIT32L(0)

/* PTEH */
#define	SH_CCN_PTEH_VPN_M		0xfffffc00
#define	SH_CCN_PTEH_VPN(x)		(0xffffc00 & (x<<10))
#define	SH_CCN_PTEH_ASID_M		_BITFIELD32L(0, 0xff)
#define	SH_CCN_PTEH_ASID(x)		_BITFIELD32L(0, x)
#define SH_CCN_ASID_FROM_PTEH(x)	( x & SH_CCN_PTEH_ASID_M )

/* PTEL */
#define	SH_CCN_PTEL_PRN_M		0x1ffffc00
#define	SH_CCN_PTEL_PRN(x)		(0xffffc00 & (x<<10))
#define	SH_CCN_PTEL_V			_ONEBIT32L(8)
#define	SH_CCN_PTEL_SZ_M		(_ONEBIT32L(7) | _ONEBIT32L(4))
#define	SH_CCN_PTEL_SZ0			_ONEBIT32L(4)
#define	SH_CCN_PTEL_SZ1			_ONEBIT32L(7)
#define	SH_CCN_PTEL_PR_M		_BITFIELD32L(5, 0x3)
#define	SH_CCN_PTEL_PR(x)		_BITFIELD32L(5, x)
#define	SH_CCN_PTEL_C			_ONEBIT32L(3)
#define	SH_CCN_PTEL_D			_ONEBIT32L(2)
#define	SH_CCN_PTEL_SH			_ONEBIT32L(1)
#define	SH_CCN_PTEL_WT			_ONEBIT32L(0)
#define SH_CCN_SZ_FROM_PTEL(x)	((x & SH_CCN_PTEL_SZ1) >> 6) + ((x & SH_CCN_PTEL_SZ0) >> 4)

/* CCR */
#define	SH_CCN_CCR_EMODE		_ONEBIT32L(31) /* 7750r/7751r only; reserved bit for other SH4 */
#define	SH_CCN_CCR_SNM			_ONEBIT32L(18) /* SMP only */
#define	SH_CCN_CCR_MCP			_ONEBIT32L(17) /* SMP only */
#define	SH_CCN_CCR_CCD			_ONEBIT32L(16) /* SMP only */
#define	SH_CCN_CCR_IIX			_ONEBIT32L(15)
#define	SH_CCN_CCR_ICI			_ONEBIT32L(11)
#define	SH_CCN_CCR_ICE			_ONEBIT32L(8)
#define	SH_CCN_CCR_OIX			_ONEBIT32L(7)
#define	SH_CCN_CCR_ORA			_ONEBIT32L(5)
#define	SH_CCN_CCR_OCI			_ONEBIT32L(3)
#define	SH_CCN_CCR_CB			_ONEBIT32L(2)
#define	SH_CCN_CCR_WT			_ONEBIT32L(1)
#define	SH_CCN_CCR_OCE			_ONEBIT32L(0)

/* QACR0 */
/* QACR1 */
#define	SH_CCN_QACR_AREA_M		_BITFIELD32L(2, 0x7)
#define	SH_CCN_QACR_AREA(x)		_BITFIELD32L(2, x)



// NOTE: we don't support 1K pages (sz0 & sz1 both 0)
#define SH_PTE_PGSIZE(ptel)		((ptel & SH_CCN_PTEL_SZ1)\
										?((ptel & SH_CCN_PTEL_SZ0)\
												?MEG(1)\
												:KILO(64))\
										:KILO(4))
										
// NOTE: we don't support 1K pages (sz0 & sz1 both 0)
#define SH_PTE_PGSIZE_MASK(ptel)	((ptel & SH_CCN_PTEL_SZ1)\
										?((ptel & SH_CCN_PTEL_SZ0)\
												?0xfff00000\
												:0xffff0000)\
										:0xfffff000)



/* __SRCVERSION("ccn.h $Rev: 204740 $"); */
