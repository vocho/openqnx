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
 *  c6x/context.h
 *

 */
#ifndef __C6X_CONTEXT_H_INCLUDED
#define __C6X_CONTEXT_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

typedef struct c6x_cpu_registers {
	_Uint32t	A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A12, A13, A14, A15;
	_Uint32t	B0, B1, B2, B3, B4, B5, B6, B7, B8, B9, B10, B11, B12, B13, B14, B15;
	_Uint32t	AMR, CSR, IFR, ISR, ICR, IER, ISTP, IRP, NRP, PCE;
} C6X_CPU_REGISTERS;

typedef struct c6x_fpu_registers {
	double 	A1A0, A3A2, A5A4, A7A6, A9A8, A11A10, A13A12, A15A14;
	double 	B1B0, B3B2, B5B4, B7B6, B9B8, B11B10, B13B12, B15B14;
} C6X_FPU_REGISTERS;

#endif /* __C6X_CONTEXT_H_INCLUDED */

/* __SRCVERSION("context.h $Rev: 153052 $"); */
