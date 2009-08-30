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
 *  sh/tmu.h    SH tmu (timer) definitions
 *

 */


/* TOCR */
#define	SH_TMU_TOCR_TCOE		_ONEBIT8L(0)

/* TSTR */
#define	SH_TMU_TSTR_STR0		_ONEBIT8L(0)
#define	SH_TMU_TSTR_STR1		_ONEBIT8L(1)
#define	SH_TMU_TSTR_STR2		_ONEBIT8L(2)

/* TCR */
#define	SH_TMU_TCR_UNF			_ONEBIT16L(8)
#define	SH_TMU_TCR_UNIE			_ONEBIT16L(5)
#define	SH_TMU_TCR_CKEG1		_ONEBIT16L(4)
#define	SH_TMU_TCR_CKEG0		_ONEBIT16L(3)
#define	SH_TMU_TCR_TPSC_M		_BITFIELD16L( 0, 0x7)
#define	SH_TMU_TCR_TPSC_4		_BITFIELD16L( 0, 0x0)
#define	SH_TMU_TCR_TPSC_16		_BITFIELD16L( 0, 0x1)
#define	SH_TMU_TCR_TPSC_64		_BITFIELD16L( 0, 0x2)
#define	SH_TMU_TCR_TPSC_256		_BITFIELD16L( 0, 0x3)
#define	SH_TMU_TCR_TPSC_1024	_BITFIELD16L( 0, 0x4)
#define	SH_TMU_TCR_TPSC_RTC		_BITFIELD16L( 0, 0x6)
#define	SH_TMU_TCR_TPSC_EXT		_BITFIELD16L( 0, 0x7)
/* below is for tmu ch2 only */
#define	SH_TMU_TCR_ICPF			_ONEBIT16L(9)
#define	SH_TMU_TCR_ICPE1		_ONEBIT16L(7)
#define	SH_TMU_TCR_ICPE0		_ONEBIT16L(6)


/* __SRCVERSION("tmu.h $Rev: 153052 $"); */
