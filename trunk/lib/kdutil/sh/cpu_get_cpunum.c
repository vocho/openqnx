/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

#include "kdintl.h"
#include "sh/sh4acpu.h"

unsigned
cpu_get_cpunum(void) {
	// This will only be called (by current_cpunum) if _syspage_ptr->num_cpu > 1,
	// so we don't need to worry about refering to a register that doesn't exist
	// on older platforms.
	return SH4A_MMR_CPIDR_CPU(sh_in32(SH4A_MMR_CPIDR));
}
