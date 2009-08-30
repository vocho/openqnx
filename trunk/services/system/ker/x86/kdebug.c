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

#include "externs.h"

/*
 * kdebug_callout
 *
 * It's a wrapper for the kdebug_enter call. It's called
 * when an exception occurred in kernel context.
 */
unsigned rdecl
kdebug_callout(unsigned sigcode, CPU_REGISTERS *reg) {
	return kdebug_enter((sigcode & SIGCODE_INTR) ? aspaces_prp[KERNCPU] : 0, sigcode, reg);
}

__SRCVERSION("kdebug.c $Rev: 153052 $");
