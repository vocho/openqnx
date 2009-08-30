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



#ifndef __NTO_PROFILER_H_INCLUDED
#define __NTO_PROFILER_H_INCLUDED

#include <sys/types.h>
#include <sys/queue.h>
#include <sys/dispatch.h>
#include <sys/iofunc.h>
#include <inttypes.h>


__BEGIN_DECLS

/*
 * Client (process) to profiler agent interface
 */
#define PROF_CAP_SAMPLER	0x00000001
#define PROF_CAP_ARCCNTS	0x00000002
#define PROF_CAP_BBINFO		0x00000004
#define PROF_CAP_THREAD		0x00000008
#define PROF_CAP_SHLIB		0x00000010

#define PROF_CMD_ARCS_2         0x00010000   // 'to' index into 'from' in memory format
#define PROF_CMD_ARCS	        0x00020000   // 'from' index into 'to' in memroy format
#define PROF_CMD_QUERY_THREAD	0x00100000
#define PROF_CMD_QUERY_SHLIB	0x00200000
#define PROF_CMD_REMOVE_MAPPING 0x20000000
#define PROF_CMD_ADD_MAPPING	0x40000000
#define PROF_CMD_INIT		    0x80000000
#define PROF_CMD_MASK		    0xffff0000

/*
 * Structure to pass info from client to profile agent
 * This is used by the devctl's to attach, add the main executable
 * code and add shared libraries.
 *
 * Normally, the executable puts the arc information in a shared memory
 * object that has the form /dev/shmem/prof-pid-%d, where %d is the key.
 * The profiler agent uses the info in shared memory to get call pairs,
 * total call numbers and so on. If the PROF_CAP_THREAD flag is passed, then
 * the library can do thread-level function call accounting.
 *
 * The name of the mapping is passed in for information purposes. This is
 * typically the basename of the executable for the main text segment, and 
 * the soname of libs when passing in shared lib information.
 *
 * The PROF_COMMAND_QUERY_SHLIB and PROF_CMD_QUERY_THREAD can be used by the 
 * library to determine if it should try to profile shared libs and keep
 * track of thread-level profile info.
 *
 */

struct __prof_clientinfo {
	unsigned	cmd;		/* Command for this request */
	unsigned 	cap_flags;	/* capabilities */
	unsigned	version;	/* Version of the profile code */
	int		    reserved;
	_Uintptrt	lowpc;		/* Low address for this mapping */
	_Uintptrt	highpc;		/* High address for this mapping */
	void		*mcounts;	/* Address in process for count array if present */
	void		*arcdata;	/* Address in process for arc data if present */
	void		*bb_head;	/* Address in process for basic block info if present */
	int		from_off;		/* Offset in shared memory for the from structures */
	int		from_size;		/* Size of froms stryuctures in bytes */
	int		tos_off;		/* Offset in shared memory for the tos structure */
	int		tos_size;		/* Size of tos structures in bytes */
	int		hash_frac;		/* Hash fraction for tos structures */
	int		shmem_key;		/* Key used to identify shared memory */
	int		map_name_len;	/* name of mapping identifier if present */
	char	map_name[1];	/* name */
};

__END_DECLS
#endif

/* __SRCVERSION("profiler.h $Rev: 153052 $"); */
