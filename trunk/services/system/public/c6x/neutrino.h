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
 *  c6x/neutrino.h
 *

 */
#ifndef __C6X_NEUTRINO_H_INCLUDED
#define __C6X_NEUTRINO_H_INCLUDED

extern cregister volatile unsigned int CSR;
#define GIE 1 /*Global Interrupt Enable: bit 0 of the Control Status Register*/

__BEGIN_DECLS

extern _Uint64t ClockCycles(void);

#define CLOCKCYCLES_INCR_BIT	0

/*
 * __inline_InterruptDisable()
 *	Disable the maskable processor interrupts.
 */
static inline void
__inline_InterruptDisable(void)
{
	CSR |= GIE;
}

/*
 * __inline_InterruptEnable()
 *	Enable maskable interrupts
 */
static inline void
__inline_InterruptEnable(void)
{
	CSR &= ~GIE;
}

__END_DECLS

#endif /* __C6X_NEUTRINO_H_INCLUDED */

/* __SRCVERSION("neutrino.h $Rev: 153052 $"); */
