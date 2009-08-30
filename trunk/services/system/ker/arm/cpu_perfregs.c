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
#include <string.h>
#include <sys/perfregs.h>

/* global functions */
_Uint32t cpu_perfreg_id(void)
{
	return 0;
}

void rdecl cpu_free_perfregs( THREAD *thp )
{
}

void rdecl cpu_save_perfregs(void *vpcr)
{
}

void rdecl cpu_restore_perfregs(void *vpcr)
{
}

int	rdecl cpu_debug_set_perfregs(THREAD *thp, debug_perfreg_t *regs)
{
	return ENXIO;
}

int	rdecl cpu_debug_get_perfregs(THREAD *thp, debug_perfreg_t *regs)
{
	return ENXIO;
}

__SRCVERSION("cpu_perfregs.c $Rev: 153052 $");
