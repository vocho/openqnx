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
 *  sh/scif.h    SH scif serial port definitions
 *

 */

/* registers addresses */
#define	SH_SCIF_BASE			0xffe80000
#define	SH_SCIF_SCSMR_OFF		0x0
#define	SH_SCIF_SCBRR_OFF		0x4
#define	SH_SCIF_SCSCR_OFF		0x8
#define	SH_SCIF_SCFTDR_OFF		0xC
#define	SH_SCIF_SCFSR_OFF		0x10
#define	SH_SCIF_SCFRDR_OFF		0x14
#define	SH_SCIF_SCFCR_OFF		0x18
#define	SH_SCIF_SCFDR_OFF		0x1c
#define	SH_SCIF_SCSPTR_OFF		0x20
#define	SH_SCIF_SCLSR_OFF		0x24

/* SCSMR */
#define	SH_SCIF_SCSMR_CHR		_ONEBIT16L(6)
#define	SH_SCIF_SCSMR_PE		_ONEBIT16L(5)
#define	SH_SCIF_SCSMR_OE		_ONEBIT16L(4)
#define	SH_SCIF_SCSMR_STOP		_ONEBIT16L(3)
#define	SH_SCIF_SCSMR_CKS_M		_BITFIELD16L(0,3)
#define	SH_SCIF_SCSMR_CKS_0		_BITFIELD16L(0,0)
#define	SH_SCIF_SCSMR_CKS_4		_BITFIELD16L(0,1)
#define	SH_SCIF_SCSMR_CKS_16	_BITFIELD16L(0,2)
#define	SH_SCIF_SCSMR_CKS_64	_BITFIELD16L(0,3)

/* SCSCR */
#define	SH_SCIF_SCSCR_TIE		_ONEBIT16L(7)
#define	SH_SCIF_SCSCR_RIE		_ONEBIT16L(6)
#define	SH_SCIF_SCSCR_TE		_ONEBIT16L(5)
#define	SH_SCIF_SCSCR_RE		_ONEBIT16L(4)
#define	SH_SCIF_SCSCR_REIE		_ONEBIT16L(3)
#define	SH_SCIF_SCSCR_CKE1		_ONEBIT16L(1)

/* SCFSR */
#define	SH_SCIF_SCFSR_PERF(x)	((x>>12)&0xf)
#define	SH_SCIF_SCFSR_FERF(x)	((x>>8)&0xf)
#define	SH_SCIF_SCFSR_ER		_ONEBIT16L(7)
#define	SH_SCIF_SCFSR_TEND		_ONEBIT16L(6)
#define	SH_SCIF_SCFSR_TDFE		_ONEBIT16L(5)
#define	SH_SCIF_SCFSR_BRK		_ONEBIT16L(4)
#define	SH_SCIF_SCFSR_FER		_ONEBIT16L(3)
#define	SH_SCIF_SCFSR_PER		_ONEBIT16L(2)
#define	SH_SCIF_SCFSR_RDF		_ONEBIT16L(1)
#define	SH_SCIF_SCFSR_DR		_ONEBIT16L(0)

/* SCFDR */
#define	SH_SCIF_SCFDR_TX(x)		((x>>8)&0x1f)
#define	SH_SCIF_SCFDR_RX(x)		(x&0x1f)

/* SCSPTR */
#define	SH_SCIF_SCSPTR_RTSIO	_ONEBIT16L(7)
#define	SH_SCIF_SCSPTR_RTSDT	_ONEBIT16L(6)
#define	SH_SCIF_SCSPTR_CTSIO	_ONEBIT16L(5)
#define	SH_SCIF_SCSPTR_CTSDT	_ONEBIT16L(4)
#define	SH_SCIF_SCSPTR_SPB2IO	_ONEBIT16L(1)
#define	SH_SCIF_SCSPTR_SPB2DT	_ONEBIT16L(0)

/* SCLSR */
#define	SH_SCIF_SCLSR_ORER		_ONEBIT16L(0)

/* SCFCR */
#define	SH_SCIF_SCFCR_RTRG_M	_BITFIELD16L(6,3)
#define	SH_SCIF_SCFCR_RTRG_1	_BITFIELD16L(6,0)
#define	SH_SCIF_SCFCR_RTRG_4	_BITFIELD16L(6,1)
#define	SH_SCIF_SCFCR_RTRG_8	_BITFIELD16L(6,2)
#define	SH_SCIF_SCFCR_RTRG_E	_BITFIELD16L(6,3)
#define	SH_SCIF_SCFCR_TTRG_E	_BITFIELD16L(4,3)
#define SH_SCIF_SCFCR_TTRG_M    _BITFIELD16L(4,3)
#define SH_SCIF_SCFCR_TTRG_1    _BITFIELD16L(4,3)
#define SH_SCIF_SCFCR_TTRG_2    _BITFIELD16L(4,2)
#define SH_SCIF_SCFCR_TTRG_4    _BITFIELD16L(4,1)
#define SH_SCIF_SCFCR_TTRG_8    _BITFIELD16L(4,0)
#define	SH_SCIF_SCFCR_MCE		_ONEBIT16L(3)
#define	SH_SCIF_SCFCR_TFRST		_ONEBIT16L(2)
#define	SH_SCIF_SCFCR_RFRST		_ONEBIT16L(1)
#define	SH_SCIF_SCFCR_LOOP		_ONEBIT16L(0)

#define	SH_SCIF_FIFO_LEN		16

/* Different SH7760 base addresses */
#define SH7760_SCIF_BASE0 		0xfe600000
#define SH7760_SCIF_BASE1 		0xfe610000
#define SH7760_SCIF_BASE2 		0xfe620000

/* Different SH7760 register offsets */
#define SH7760_SCIF_SCTFDR_OFF 	0x1c
#define SH7760_SCIF_SCRFDR_OFF 	0x20
#define SH7760_SCIF_SCSPTR_OFF	0x24
#define SH7760_SCIF_SCLSR_OFF 	0x28

/* New SH7760 registers */
#define SH7760_SCIF_SCRER_OFF 	0x2C

/* New SH7760 bits */
#define SH7760_SCIF_SCSMR_CA        _ONEBIT16L(7)
#define SH7760_SCIF_SCSCR_CKE0      _ONEBIT16L(0)
#define SH7760_SCIF_SCSPTR_SCKIO    _ONEBIT16L(3)
#define SH7760_SCIF_SCSPTR_SCKDT    _ONEBIT16L(2)
#define	SH7760_SCIF_SCFCR_RSTRG_127	_BITFIELD16L(8,0)
#define SH7760_SCIF_SCFCR_RSTRG_1	_BITFIELD16L(8,1)
#define SH7760_SCIF_SCFCR_RSTRG_16	_BITFIELD16L(8,2)
#define SH7760_SCIF_SCFCR_RSTRG_32	_BITFIELD16L(8,3)
#define SH7760_SCIF_SCFCR_RSTRG_64	_BITFIELD16L(8,4)
#define SH7760_SCIF_SCFCR_RSTRG_96	_BITFIELD16L(8,5)
#define SH7760_SCIF_SCFCR_RSTRG_108	_BITFIELD16L(8,6)
#define SH7760_SCIF_SCFCR_RSTRG_120	_BITFIELD16L(8,7)

/* Different SH7760 FIFO Length */
#define SH7760_SCIF_FIFO_LEN        128

/* Different SH7770 base addresses */
#define SH7770_SCIF_BASE0 		0xff923000
#define SH7770_SCIF_BASE1 		0xff924000
#define SH7770_SCIF_BASE2 		0xff925000
#define SH7770_SCIF_BASE3 		0xff926000
#define SH7770_SCIF_BASE4 		0xff927000
#define SH7770_SCIF_BASE5 		0xff928000
#define SH7770_SCIF_BASE6 		0xff929000
#define SH7770_SCIF_BASE7 		0xff92a000
#define SH7770_SCIF_BASE8 		0xff92b000
#define SH7770_SCIF_BASE9 		0xff92c000

/* New SH7770 bits */
#define	SH7770_SCIF_SCFCR_RSTRG_15	_BITFIELD16L(8, 0)
#define	SH7770_SCIF_SCFCR_RSTRG_1	_BITFIELD16L(8, 1)
#define	SH7770_SCIF_SCFCR_RSTRG_4	_BITFIELD16L(8, 2)
#define	SH7770_SCIF_SCFCR_RSTRG_6	_BITFIELD16L(8, 3)
#define	SH7770_SCIF_SCFCR_RSTRG_8	_BITFIELD16L(8, 4)
#define	SH7770_SCIF_SCFCR_RSTRG_10	_BITFIELD16L(8, 5)
#define	SH7770_SCIF_SCFCR_RSTRG_12	_BITFIELD16L(8, 6)
#define	SH7770_SCIF_SCFCR_RSTRG_14	_BITFIELD16L(8, 7)

/* New SH7780 bits */
#define	SH7780_SCIF_SCFCR_RSTRG_63	_BITFIELD16L(8, 0)
#define	SH7780_SCIF_SCFCR_RSTRG_1	_BITFIELD16L(8, 1)
#define	SH7780_SCIF_SCFCR_RSTRG_8	_BITFIELD16L(8, 2)
#define	SH7780_SCIF_SCFCR_RSTRG_16 _BITFIELD16L(8, 3)
#define	SH7780_SCIF_SCFCR_RSTRG_32 _BITFIELD16L(8, 4)
#define	SH7780_SCIF_SCFCR_RSTRG_48 _BITFIELD16L(8, 5)
#define	SH7780_SCIF_SCFCR_RSTRG_54 _BITFIELD16L(8, 6)
#define	SH7780_SCIF_SCFCR_RSTRG_60 _BITFIELD16L(8, 7)

/* Different SH_X3P base addresses */
#define	SH_X3P_SCIF_BASE0	0xffc30000
#define	SH_X3P_SCIF_BASE1	0xffc40000
#define	SH_X3P_SCIF_BASE2	0xffc50000
#define	SH_X3P_SCIF_BASE3	0xffc60000

/* __SRCVERSION("scif.h $Rev: 153052 $"); */
