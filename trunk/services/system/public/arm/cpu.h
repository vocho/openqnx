/*
 * $QNXLicenseC: 
 * Copyright 2007, QNX Software Systems.  
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */


/*
 *  arm/cpu.h
 *

 */

#ifndef __ARM_CPU_H_INCLUDED
#define __ARM_CPU_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __ARM_CONTEXT_H_INCLUDED
#include _NTO_HDR_(arm/context.h)
#endif

#define ARM_STRINGNAME	"arm"

/*
 * Exception vector offsets
 */
#define ARM_EXC_RST				0x00
#define ARM_EXC_UND				0x04
#define ARM_EXC_SWI				0x08
#define ARM_EXC_PRF				0x0c
#define ARM_EXC_ABT				0x10
#define ARM_EXC_IRQ				0x18
#define ARM_EXC_FIQ				0x1c
#define ARM_EXC_END				0x20

/*
 * Program Status Register CPSR
 */
#define ARM_CPSR_MODE_MASK		0x1f
#define ARM_CPSR_MODE_USR		0x10
#define ARM_CPSR_MODE_FIQ		0x11
#define ARM_CPSR_MODE_IRQ		0x12
#define ARM_CPSR_MODE_SVC		0x13
#define ARM_CPSR_MODE_ABT		0x17
#define ARM_CPSR_MODE_UND		0x1b
#define ARM_CPSR_MODE_SYS		0x1f
#define ARM_CPSR_T				(1 << 5)
#define ARM_CPSR_F				(1 << 6)
#define ARM_CPSR_I				(1 << 7)
#define	ARM_CPSR_A				(1 << 8)
#define	ARM_CPSR_E				(1 << 9)
#define	ARM_CPSR_GE_MASK		0x000f0000
#define	ARM_CPSR_GE0			(1 << 16)
#define	ARM_CPSR_GE1			(1 << 17)
#define	ARM_CPSR_GE2			(1 << 18)
#define	ARM_CPSR_GE3			(1 << 19)
#define	ARM_CPSR_J				(1 << 24)
#define	ARM_CPSR_Q				(1 << 27)
#define ARM_CPSR_COND_MASK		0xf0000000
#define ARM_CPSR_V				(1 << 28)
#define ARM_CPSR_C				(1 << 29)
#define ARM_CPSR_Z				(1 << 30)
#define ARM_CPSR_N				(1 << 31)

#endif /* __ARM_CPU_H_INCLUDED */

/* __SRCVERSION("cpu.h $Rev: 160127 $"); */
