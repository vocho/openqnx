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



#ifndef __MIPS_41XXCPU_H_INCLUDED
#define __MIPS_41XXCPU_H_INCLUDED
/*
 *  mips/41xxcpu.h
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

#define MIPS41XX_UNIT_BASE	0x0b000000

/*
** Bus Control Unit Registers
*/
#define MIPS41XX_BCU_CNTREG1		0x0000
#define MIPS41XX_BCU_CNTREG2		0x0002
#define MIPS41XX_BCU_SPEEDREG		0x000a
#define MIPS41XX_BCU_ERRSTREG		0x000c
#define MIPS41XX_BCU_RFCNTREG		0x000e
#define MIPS41XX_BCU_REVIDREG		0x0010
#define MIPS41XX_BCU_RFCOUNTREG		0x0012
#define MIPS41XX_BCU_CLKSPEEDREG	0x0014
#define MIPS41XX_BCU_CNTREG3		0x0016
#define MIPS41XX_BCU_CACHECNTREG	0x0018

/*
** Interrupt Control Unit Registers
*/
#define MIPS41XX_ICU_SYSINT1	0x0080
#define MIPS41XX_ICU_PIUINT		0x0082
#define MIPS41XX_ICU_AIUINT		0x0084
#define MIPS41XX_ICU_KIUINT		0x0086
#define MIPS41XX_ICU_GIUINT		0x0088
#define MIPS41XX_ICU_DSIUINT	0x008a
#define MIPS41XX_ICU_MSYSINT1	0x008c
#define MIPS41XX_ICU_MPIUINT	0x008e
#define MIPS41XX_ICU_MAIUINT	0x0090
#define MIPS41XX_ICU_MKIUINT	0x0092
#define MIPS41XX_ICU_MGIUINT	0x0094
#define MIPS41XX_ICU_MDSIUINT	0x0096
#define MIPS41XX_ICU_NMI		0x0098
#define MIPS41XX_ICU_SOFTINT	0x009a
#define MIPS41XX_ICU_SYSINT2	0x0200
#define MIPS41XX_ICU_GIUINTH	0x0202
#define MIPS41XX_ICU_FIRINT		0x0204
#define MIPS41XX_ICU_MSYSINT2	0x0206
#define MIPS41XX_ICU_MGIUINTH	0x0208
#define MIPS41XX_ICU_MFIRINT	0x020a

/*
** Deadman's Switch Unit Registers
*/
#define MIPS41XX_DSU_CNT		0x00e0
#define MIPS41XX_DSU_SET		0x00e2
#define MIPS41XX_DSU_CLR		0x00e4
#define MIPS41XX_DSU_TIM		0x00e6

/*
** General Purpose I/O Unit
*/
#define MIPS41XX_GIU_IOSELL		0x0100
#define MIPS41XX_GIU_IOSELH		0x0102
#define MIPS41XX_GIU_PIODL		0x0104
#define MIPS41XX_GIU_PIODH		0x0106
#define MIPS41XX_GIU_INTSTATL	0x0108
#define MIPS41XX_GIU_INTSTATH	0x010a
#define MIPS41XX_GIU_INTENL		0x010c
#define MIPS41XX_GIU_INTENH		0x010e
#define MIPS41XX_GIU_INTTYPL	0x0110
#define MIPS41XX_GIU_INTTYPH	0x0112
#define MIPS41XX_GIU_INTALSELL	0x0114
#define MIPS41XX_GIU_INTALSELH	0x0116
#define MIPS41XX_GIU_INTHTSELL	0x0118
#define MIPS41XX_GIU_INTHTSELH	0x011a
#define MIPS41XX_GIU_PODATL		0x011c
#define MIPS41XX_GIU_PODATH		0x011e
#define MIPS41XX_GIU_USEUPDN	0x02e0
#define MIPS41XX_GIU_TERMUPDN	0x02e2

/*
** Clock Mask Unit
*/
#define MIPS41XX_CMU_CLKMSK		0x0060

#define _R41XXUNIT(unit,reg,size)	(*(volatile uint##size##_t *)((MIPS_R4K_K1BASE+MIPS41XX_UNIT_BASE+MIPS41XX_##unit##_##reg)))
#define R41XXUNIT(unit,reg)			_R41XXUNIT(unit, reg, 16)

#endif /* __MIPS_41XXCPU_H_INCLUDED */

/* __SRCVERSION("41xxcpu.h $Rev: 153052 $"); */
