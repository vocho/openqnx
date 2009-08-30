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
 *  sh/CPG.h    SH CPG (clock circuit & watchdog timer) definitions
 *

 */


/* FRQCR */
#define	SH_CPG_FRQCR_CKOEN		_ONEBIT16L(11)
#define	SH_CPG_FRQCR_PLL1EN		_ONEBIT16L(10)
#define	SH_CPG_FRQCR_PLL2EN		_ONEBIT16L(9)
#define	SH_CPG_FRQCR_IFC_M		_BITFIELD16L( 6, 0x7)
#define	SH_CPG_FRQCR_IFC_1		_BITFIELD16L( 6, 0x0)
#define	SH_CPG_FRQCR_IFC_2		_BITFIELD16L( 6, 0x1)
#define	SH_CPG_FRQCR_IFC_3		_BITFIELD16L( 6, 0x2)
#define	SH_CPG_FRQCR_IFC_4		_BITFIELD16L( 6, 0x3)
#define	SH_CPG_FRQCR_IFC_6		_BITFIELD16L( 6, 0x4)
#define	SH_CPG_FRQCR_IFC_8		_BITFIELD16L( 6, 0x5)
#define	SH_CPG_FRQCR_BFC_M		_BITFIELD16L( 3, 0x7)
#define	SH_CPG_FRQCR_BFC_1		_BITFIELD16L( 3, 0x0)
#define	SH_CPG_FRQCR_BFC_2		_BITFIELD16L( 3, 0x1)
#define	SH_CPG_FRQCR_BFC_3		_BITFIELD16L( 3, 0x2)
#define	SH_CPG_FRQCR_BFC_4		_BITFIELD16L( 3, 0x3)
#define	SH_CPG_FRQCR_BFC_6		_BITFIELD16L( 3, 0x4)
#define	SH_CPG_FRQCR_BFC_8		_BITFIELD16L( 3, 0x5)
#define	SH_CPG_FRQCR_PFC_M		_BITFIELD16L( 0, 0x7)
#define	SH_CPG_FRQCR_PFC_2		_BITFIELD16L( 0, 0x0)
#define	SH_CPG_FRQCR_PFC_3		_BITFIELD16L( 0, 0x1)
#define	SH_CPG_FRQCR_PFC_4		_BITFIELD16L( 0, 0x2)
#define	SH_CPG_FRQCR_PFC_6		_BITFIELD16L( 0, 0x3)
#define	SH_CPG_FRQCR_PFC_8		_BITFIELD16L( 0, 0x4)


/* __SRCVERSION("cpg.h $Rev: 153052 $"); */
