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
 *  x86/fault.h    X86 specific fault definitions
 *

 */

/*
 * FLTILL		1	exc6  Illegal instruction
 * FLTPRIV		2	excD  Privileged instruction
 * FLTBPT		3	exc3  Breakpoint instruction
 * FLTTRACE		4	exc1  Trace trap (single-step)
 * FLTACCESS	5	exc11 Memory access (e.g. alignment)
 * FLTBOUNDS	6	exc5  Memory bounds (invalid address)
 * FLTIOVF		7	exc4  Integer overflow
 * FLTIZDIV		8	exc0  Integer zero divide
 * FLTFPE		9	exc10 Floating-point exception
 * FLTSTACK		10	excC  Irrecoverable stack fault
 * FLTPAGE		11	excE  Recoverable page fault (no associated sig)
 */

#define FLTNMI		(_FIRST_CPU_FAULT+0) /* exc2  Non-Maskable Interrupt */
#define FLTDBLFLT	(_FIRST_CPU_FAULT+1) /* exc8  Double Fault (hardcrash) */
#define FLTOLDFPE	(_FIRST_CPU_FAULT+2) /* exc9  Old Floating-Point execption (hardcrash) */
#define FLTTSS		(_FIRST_CPU_FAULT+3) /* excA  Invalid TSS (hardcrash) */
#define FLTSEG		(_FIRST_CPU_FAULT+4) /* excB  Segment Not Present (hardcrash) */
#define FLTRSVD		(_FIRST_CPU_FAULT+6) /* excF  Intel Reserved (hardcrash) */
#define FLTNOFPU	(_FIRST_CPU_FAULT+7) /* exc7  No Floating Point Device */
#define FLTMACHCHK	(_FIRST_CPU_FAULT+8) /* exc12 Machine Check */

/* __SRCVERSION("fault.h $Rev: 153052 $"); */
