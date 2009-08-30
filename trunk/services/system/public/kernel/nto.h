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

#ifndef __KERNEL_NTO_H_INCLUDED
#define __KERNEL_NTO_H_INCLUDED

#if defined(rdecl) || defined(KERHDR)
#error Not to be mixed with other kernel/ includes
#endif

#include <stddef.h>
#include <inttypes.h>
#include <sys/neutrino.h>
#include <kernel/types.h>

#if defined(__X86__)
	#include <kernel/cpu_x86.h>
#elif defined(__PPC__)
	#include <kernel/cpu_ppc.h>
#elif defined(__MIPS__)
	#include <kernel/cpu_mips.h>
#elif defined(__SH__)
	#include <kernel/cpu_sh.h>
#elif defined(__ARM__)
	#include <kernel/cpu_arm.h>
#else
    #error not configured for system
#endif

#include <sys/fault.h>
#include <sys/debug.h>
#include <kernel/macros.h>
#include <sys/kdebug.h>
#include <kernel/debug.h>
#include <kernel/objects.h>
#include <kernel/kerext.h>
#include <kernel/proto.h>
#include <kernel/query.h>
// FIX ME #include <sys/memclass.h>
#include "kernel/memclass.h"

extern SOUL					process_souls;
extern VECTOR				process_vector;
extern PROCMGR				procmgr;
extern MEMMGR				memmgr;
extern unsigned				num_processors;
extern PROCESS				*procnto_prp;
extern memclass_id_t		sys_memclass_id;	// generic system ram memory class
extern VECTOR				mempart_vector;
extern VECTOR				schedpart_vector;

extern unsigned				user_boundry_addr;

extern const long			kernel_conf_table[];

extern int					ker_verbose;
extern int					munmap_flags_default;
extern mode_t				procfs_umask;

#endif

/* __SRCVERSION("nto.h $Rev: 168445 $"); */
