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
 *  sh/sci.h    SH sci serial port definitions
 *

 */

/* registers addresses */
#define	SH_SCI_BASE				0xffe00000
#define	SH_SCI_SCSMR_OFF		0x0
#define	SH_SCI_SCBRR_OFF		0x4
#define	SH_SCI_SCSCR_OFF		0x8
#define	SH_SCI_SCTDR_OFF		0xc
#define	SH_SCI_SCSSR_OFF		0x10
#define	SH_SCI_SCRDR_OFF		0x14
#define	SH_SCI_SCSCMR_OFF		0x18
#define	SH_SCI_SCSPTR_OFF		0x1c

/* SCSMR */
#define	SH_SCI_SCSMR_CA			_ONEBIT8L(7)
#define	SH_SCI_SCSMR_CHR		_ONEBIT8L(6)
#define	SH_SCI_SCSMR_PE			_ONEBIT8L(5)
#define	SH_SCI_SCSMR_OE			_ONEBIT8L(4)
#define	SH_SCI_SCSMR_STOP		_ONEBIT8L(3)
#define	SH_SCI_SCSMR_MP			_ONEBIT8L(2)
#define	SH_SCI_SCSMR_CKS_M		_BITFIELD8L(0,3)
#define	SH_SCI_SCSMR_CKS_0		_BITFIELD8L(0,0)
#define	SH_SCI_SCSMR_CKS_4		_BITFIELD8L(0,1)
#define	SH_SCI_SCSMR_CKS_16		_BITFIELD8L(0,2)
#define	SH_SCI_SCSMR_CKS_64		_BITFIELD8L(0,3)

/* SCSCR */
#define	SH_SCI_SCSCR_TIE		_ONEBIT8L(7)
#define	SH_SCI_SCSCR_RIE		_ONEBIT8L(6)
#define	SH_SCI_SCSCR_TE			_ONEBIT8L(5)
#define	SH_SCI_SCSCR_RE			_ONEBIT8L(4)
#define	SH_SCI_SCSCR_MPIE		_ONEBIT8L(3)
#define	SH_SCI_SCSCR_TEIE		_ONEBIT8L(2)
#define	SH_SCI_SCSCR_CKE1		_ONEBIT8L(1)
#define	SH_SCI_SCSCR_CKE0		_ONEBIT8L(0)

/* SCSSR */
#define	SH_SCI_SCSSR_TDRE		_ONEBIT8L(7)
#define	SH_SCI_SCSSR_RDRF		_ONEBIT8L(6)
#define	SH_SCI_SCSSR_ORER		_ONEBIT8L(5)
#define	SH_SCI_SCSSR_FER		_ONEBIT8L(4)
#define	SH_SCI_SCSSR_PER		_ONEBIT8L(3)
#define	SH_SCI_SCSSR_TEND		_ONEBIT8L(2)
#define	SH_SCI_SCSSR_MPB		_ONEBIT8L(1)
#define	SH_SCI_SCSSR_MPBT		_ONEBIT8L(0)

/* SCSPTR */
#define	SH_SCI_SCSPTR_EIO		_ONEBIT8L(7)
#define	SH_SCI_SCSPTR_SPB1IO	_ONEBIT8L(3)
#define	SH_SCI_SCSPTR_SPB1DT	_ONEBIT8L(2)
#define	SH_SCI_SCSPTR_SPB0IO	_ONEBIT8L(1)
#define	SH_SCI_SCSPTR_SPB0DT	_ONEBIT8L(0)


/* __SRCVERSION("sci.h $Rev: 153052 $"); */
