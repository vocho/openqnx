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




#ifndef __SYS_DCMD_PROF_H_INCLUDED
#define __SYS_DCMD_PROF_H_INCLUDED

#include <sys/profiler.h>
#include <devctl.h>

enum message_type {
	PROF_ATTACH=1,     /* attach a process to profiler */
	PROF_DETACH,       /* detach a process from profiler */
	PROF_QUERY,        /* query if capability is needed */
	PROF_MAPPING_ADD,  /* add a mapping from running process */
	PROF_MAPPING_REM   /* remove a mapping from running process */
};

#define DCMD_PROF_ATTACH          __DIOT(_DCMD_MISC, PROF_ATTACH, struct __prof_clientinfo)
#define DCMD_PROF_DETACH          __DIOT(_DCMD_MISC, PROF_DETACH, NULL)
#define DCMD_PROF_QUERY           __DIOT(_DCMD_MISC, PROF_QUERY, struct __prof_clientinfo)
#define DCMD_PROF_MAPPING_ADD     __DIOT(_DCMD_MISC, PROF_MAPPING_ADD, struct __prof_clientinfo)
#define DCMD_PROF_MAPPING_REM     __DIOT(_DCMD_MISC, PROF_MAPPING_REM, struct __prof_clientinfo)

#endif

/* __SRCVERSION("dcmd_prof.h $Rev: 153052 $"); */
