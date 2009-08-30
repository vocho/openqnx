/*
 * $QNXLicenseC:
 * Copyright 2008, QNX Software Systems. All Rights Reserved.
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

extern void	smp_exc_init(void);
extern void	ppcbke_tlb_init(uint8_t *);

/*
 * Invoked by init_smp() to perform any per-cpu specific initialisation.
 */
void
smp_init_cpu()
{
	smp_exc_init();
	ppcbke_tlb_init(0);
	init_cpu();
}

__SRCVERSION("smp_init_cpu.c $Rev: 199330 $");
