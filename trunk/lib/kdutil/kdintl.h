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

#include <stddef.h>
#include <stdint.h>
#include <sys/syspage.h>
#include <kernel/kdutil.h>
#include "cpu_kdintl.h"

extern unsigned	__cpu_flags;

extern int mmu_needs_restoring;

extern unsigned	cpu_get_cpunum(void);


#define CPU_MAP_COLOUR_NONE -1

