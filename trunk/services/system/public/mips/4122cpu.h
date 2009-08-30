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



#ifndef __MIPS_4122CPU_H_INCLUDED
#define __MIPS_4122CPU_H_INCLUDED
/*
 *  mips/4122cpu.h
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

#define MIPS4122_UNIT_BASE	0x0f000000

/*
** Bus Control Unit Registers
*/
#define MIPS4122_BCU_CNTREG1		0x0000
#define MIPS4122_BCU_ROMSIZEREG		0x0004
#define MIPS4122_BCU_ROMSPEEDREG	0x0006
#define MIPS4122_BCU_IO0SPEEDREG	0x0008
#define MIPS4122_BCU_IO1SPEEDREG	0x000a
#define MIPS4122_BCU_REVIDREG		0x0010
#define MIPS4122_BCU_CLKSPEEDREG	0x0014
#define MIPS4122_BCU_CNTREG3		0x0016

/*
** Interrupt Control Unit Registers
*/
#define MIPS4122_ICU_SYSINT1	0x0080
#define MIPS4122_ICU_GIUINTL	0x0088
#define MIPS4122_ICU_DSIUINT	0x008a
#define MIPS4122_ICU_MSYSINT1	0x008c
#define MIPS4122_ICU_MGIUINTL	0x0094
#define MIPS4122_ICU_MDSIUINT	0x0096
#define MIPS4122_ICU_NMI		0x0098
#define MIPS4122_ICU_SOFTINT	0x009a
#define MIPS4122_ICU_SYSINT2	0x00a0
#define MIPS4122_ICU_GIUINTH	0x00a2
#define MIPS4122_ICU_FIRINT		0x00a4
#define MIPS4122_ICU_MSYSINT2	0x00a6
#define MIPS4122_ICU_MGIUINTH	0x00a8
#define MIPS4122_ICU_MFIRINT	0x00aa
#define MIPS4122_ICU_PCIINT		0x00ac
#define MIPS4122_ICU_SCUINT		0x00ae
#define MIPS4122_ICU_CSIINT		0x00b0
#define MIPS4122_ICU_MPCIINT	0x00b2
#define MIPS4122_ICU_MSCUINT	0x00b4
#define MIPS4122_ICU_MCSIINT	0x00b6
#define MIPS4122_ICU_BCUINT		0x00b8
#define MIPS4122_ICU_MBCUINT	0x00ba

/*
** General Purpose I/O Unit
*/
#define MIPS4122_GIU_IOSELL		0x0140
#define MIPS4122_GIU_IOSELH		0x0142
#define MIPS4122_GIU_PIODL		0x0144
#define MIPS4122_GIU_PIODH		0x0146
#define MIPS4122_GIU_INTSTATL	0x0148
#define MIPS4122_GIU_INTSTATH	0x014a
#define MIPS4122_GIU_INTENL		0x014c
#define MIPS4122_GIU_INTENH		0x014e
#define MIPS4122_GIU_INTTYPL	0x0150
#define MIPS4122_GIU_INTTYPH	0x0152
#define MIPS4122_GIU_INTALSELL	0x0154
#define MIPS4122_GIU_INTALSELH	0x0156
#define MIPS4122_GIU_INTHTSELL	0x0158
#define MIPS4122_GIU_INTHTSELH	0x015a
#define MIPS4122_GIU_PODATEN	0x015c
#define MIPS4122_GIU_PODATL		0x015e

/*
** Clock Mask Unit
*/
#define MIPS4122_CMU_CLKMSK		0x0060


/*
** SDRAM Control Unit
*/
#define MIPS4122_SDRAMU_MODEREG		0x0400
#define MIPS4122_SDRAMU_CNTREG		0x0402
#define MIPS4122_SDRAMU_RFCNTREG  	0x0404
#define MIPS4122_SDRAMU_RFCOUNTREG	0x0406
#define MIPS4122_SDRAMU_RAMSIZEREG	0x0408

/*
** PCIU Control Unit
*/
#define MIPS4122_PCIU_PCIMMAW1REG		0x0c00
#define MIPS4122_PCIU_PCIMMAW2REG		0x0c04
#define MIPS4122_PCIU_PCITAW1REG		    0x0c08
#define MIPS4122_PCIU_PCITAW2REG		    0x0c0c
#define MIPS4122_PCIU_PCIMIOAWREG		0x0c10
#define MIPS4122_PCIU_PCICONFDREG		0x0c14
#define MIPS4122_PCIU_PCICONFAREG		0x0c18
#define MIPS4122_PCIU_PCIMAILREG		    0x0c1c
#define MIPS4122_PCIU_BUSERRADREG		0x0c24
#define MIPS4122_PCIU_INTCNTSTAREG		0x0c28
#define MIPS4122_PCIU_PCIEXACCREG		0x0c2c
#define MIPS4122_PCIU_PCIRECONTREG		0x0c30
#define MIPS4122_PCIU_PCIENREG  		    0x0c34
#define MIPS4122_PCIU_PCICLKSELREG		0x0c38
#define MIPS4122_PCIU_PCITRDYVREG		0x0c3c
#define MIPS4122_PCIU_PCICLKRUNREG		0x0c60

/*
** SCU Control Unit
*/
#define MIPS4122_SCU_TIMOUTCNTREG		0x1000
#define MIPS4122_SCU_TIMOUTCOUNTREG		0x1002
#define MIPS4122_SCU_ERRLADDRESSREG		0x1004
#define MIPS4122_SCU_ERRHADDRESSREG		0x1006
#define MIPS4122_SCU_SCUINTRREG		    0x1008



#define _R4122UNIT(unit,reg,size)	(*(volatile uint##size##_t *)((MIPS_R4K_K1BASE+MIPS4122_UNIT_BASE+MIPS4122_##unit##_##reg)))
#define R4122UNIT(unit,reg)			_R4122UNIT(unit, reg, 16)
#define R4122UNIT32(unit,reg)		_R4122UNIT(unit, reg, 32)


#endif /* __MIPS_4122CPU_H_INCLUDED */

/* __SRCVERSION("4122cpu.h $Rev: 153052 $"); */
