/*
 * $QNXLicenseC:
 * Copyright 2007,2008, QNX Software Systems. All Rights Reserved.
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

#ifndef _GET_BACKTRACE_H_INCLUDED
#define _GET_BACKTRACE_H_INCLUDED

#include <inttypes.h>
#ifndef _BT_LIGHT
#include <sys/procfs.h>
#endif
#include "mem_reader.h"

#ifndef _BT_LIGHT

// Taken from pidin_proc.c...
#ifdef __MIPS__
#ifdef __BIGENDIAN__
#define REG_LOW_WORD 1
#else
#define REG_LOW_WORD 0
#endif
#define GETREG(_ptr,_idx)   ((uint32_t*)(_ptr))[(((_idx)*2)+REG_LOW_WORD)]
#define NREGS(_size)        ((_size)/(sizeof(uint32_t)*2))
#else
#define GETREG(_ptr,_idx)   ((uint32_t*)(_ptr))[(_idx)]
#define NREGS(_size)        ((_size)/sizeof(uint32_t))
#endif

int _bt_get_greg(int fd, bt_accessor_t *acc, procfs_greg *greg, int *greg_size);
#endif

#include "gather.h"


#endif
