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

#ifndef __KERNEL_DEBUG
#define __KERNEL_DEBUG

#include <sys/debug.h>

//Internal data structure used between procfs and kerext
typedef struct debug_breaklist {
	uint32_t		offset;			//Offset at which to start 
	uint32_t		count;			//IN: Number to fill OUT: Number filled	
	uint32_t		total_count;	//Total number of breakpoints (only filled in for offset == 0)
	debug_break_t	*breakpoints;	//Space for at least count breakpoints
} debug_breaklist_t;
	

union nto_debug_data {
	debug_process_t					process;
	debug_thread_t					thread;
	debug_greg_t					greg;
	debug_fpreg_t					fpreg;
	debug_altreg_t					altreg;
	debug_perfreg_t					perfreg;
	debug_run_t						run;
	debug_break_t					brk;
	debug_breaklist_t				brklist;
	uint32_t						flags;
};
	
enum nto_debug_request {
	NTO_DEBUG_PROCESS_INFO,			// pid,na,debug_process_t
	NTO_DEBUG_THREAD_INFO,			// pid,tid,debug_thread_t
	NTO_DEBUG_GET_GREG,				// pid,tid,debug_greg_t
	NTO_DEBUG_SET_GREG,				// pid,tid,debug_greg_t
	NTO_DEBUG_GET_FPREG,			// pid,tid,debug_fpreg_t
	NTO_DEBUG_SET_FPREG,			// pid,tid,debug_fpreg_t
	NTO_DEBUG_STOP,					// pid,na,debug_thread_t
	NTO_DEBUG_RUN,					// pid,na,debug_run_t
	NTO_DEBUG_CURTHREAD,			// pid,tid,NULL
	NTO_DEBUG_FREEZE,				// pid,tid,NULL
	NTO_DEBUG_THAW,					// pid,tid,NULL
	NTO_DEBUG_BREAK,				// pid,na,debug_break_t
	NTO_DEBUG_SET_FLAG,				// pid,na,uint32_t
	NTO_DEBUG_CLEAR_FLAG,			// pid,na,uint32_t
	NTO_DEBUG_GET_ALTREG,			// pid,tid,debug_altreg_t
	NTO_DEBUG_SET_ALTREG,			// pid,tid,debug_altreg_t
	NTO_DEBUG_GET_PERFREG,			// pid,tid,
	NTO_DEBUG_SET_PERFREG,			// pid,tid,
	NTO_DEBUG_GET_BREAKLIST,		// pid,na,debug_break_list_t
};

#endif

/* __SRCVERSION("debug.h $Rev: 153052 $"); */
