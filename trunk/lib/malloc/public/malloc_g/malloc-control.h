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

#include <errno.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/iofunc.h>
#include <sys/dispatch.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <limits.h>
#include <inttypes.h>
#include <devctl.h>

#ifndef _DBGM_CONTROL_H_
#define _DBGM_CONTROL_H_

#define MDBG_PREFIX "/dev/malloc_g"

enum  {
	SUBCMD_TRACE_MIN = 1,
	SUBCMD_TRACE_MAX,
	SUBCMD_TRACE_RANGE,
	SUBCMD_BTDEPTH, 		/* SUBCMD_EVENTBTDEPTH */
	SUBCMD_TRACEDEPTH,
	SUBCMD_NOBT,			/* SUBCMD_EVENTNOBT*/
	SUBCMD_NOTRACEDEPTH,
	SUBCMD_TRACE_FILE,
	SUBCMD_NOTRACE_FILE,
	SUBCMD_DUMP_UNREF,
	SUBCMD_DUMP_ALLOC_STATE,
	SUBCMD_DUMP_ALLOC_STATE_NODETAIL,
	SUBCMD_GET_STATE,
	SUBCMD_CKALLOC,
	SUBCMD_CKACCESS,
	SUBCMD_CKCHAIN,
	SUBCMD_CKBOUNDS,
	SUBCMD_EVENT_ACTION,
	SUBCMD_VERIFY,
	SUBCMD_EVENT_FILE,
	SUBCMD_NOEVENT_FILE,
	SUBCMD_VERBOSE,
	SUBCMD_ERR_FILE,
	SUBCMD_NOERR_FILE,
	SUBCMD_USE_DLADDR,
	SUBCMD_MAX
};

typedef struct no_arg_msg {
	int cmd;
} __dbgm_na_msg;

typedef struct one_int_msg {
	int cmd;
	int val;
} __dbgm_one_int_msg;

typedef struct two_int_msg {
	int cmd;
	int val1;
	int val2;
} __dbgm_two_int_msg;

typedef struct str_msg {
	int cmd;
	char str[_POSIX_PATH_MAX];
} __dbgm_str_msg;

typedef struct get_state_msg {
	int tmin;
	int tmax;
	int btdepth;
	int tdepth;
	char tfile[_POSIX_PATH_MAX];
} __dbgm_get_state_msg;

#define DBGM_CMD_CODE      1
#define DBGM_DEVCTL_TRACEMIN __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 0, __dbgm_one_int_msg)
#define DBGM_DEVCTL_TRACEMAX __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 1, __dbgm_one_int_msg)
#define DBGM_DEVCTL_TRACERANGE __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 2, __dbgm_two_int_msg)
#define DBGM_DEVCTL_BTDEPTH __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 3, __dbgm_one_int_msg)
#define DBGM_DEVCTL_TRACEDEPTH __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 4, __dbgm_one_int_msg)
#define DBGM_DEVCTL_NOBT __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 5, __dbgm_na_msg)
#define DBGM_DEVCTL_NOTRACEDEPTH __DIOT(_DCMD_MISC, DBGM_CMD_CODE + 6, __dbgm_na_msg)
#define DBGM_DEVCTL_TRACEFILE __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 7, __dbgm_str_msg)
#define DBGM_DEVCTL_NOTRACEFILE __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 8, __dbgm_na_msg)
#define DBGM_DEVCTL_DUMPUNREF __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 9, __dbgm_str_msg)
#define DBGM_DEVCTL_DUMPALLOCSTATE __DIOT(_DCMD_MISC, DBGM_CMD_CODE + 10, __dbgm_str_msg)
#define DBGM_DEVCTL_DUMPALLOCSTATE_NODETAIL __DIOT(_DCMD_MISC, DBGM_CMD_CODE + 11, __dbgm_na_msg)
#define DBGM_DEVCTL_GET_STATE __DIOF(_DCMD_MISC, DBGM_CMD_CODE + 12, __dbgm_get_state_msg)


#define DBGM_DEVCTL_CKALLOC __DIOT(_DCMD_MISC, DBGM_CMD_CODE + 13, __dbgm_one_int_msg)
#define DBGM_DEVCTL_CKACCESS __DIOT(_DCMD_MISC, DBGM_CMD_CODE + 14, __dbgm_one_int_msg)
#define DBGM_DEVCTL_CKCHAIN __DIOT(_DCMD_MISC, DBGM_CMD_CODE + 15, __dbgm_one_int_msg)
#define DBGM_DEVCTL_CKBOUNDS __DIOT(_DCMD_MISC, DBGM_CMD_CODE + 16, __dbgm_one_int_msg)
#define DBGM_DEVCTL_EVENT_ACTION __DIOT(_DCMD_MISC, DBGM_CMD_CODE + 17, __dbgm_one_int_msg)
#define DBGM_DEVCTL_VERIFY __DIOT(_DCMD_MISC, DBGM_CMD_CODE + 18, __dbgm_na_msg)
#define DBGM_DEVCTL_EVENTFILE __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 19, __dbgm_str_msg)
#define DBGM_DEVCTL_NOEVENTFILE __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 20, __dbgm_na_msg)
#define DBGM_DEVCTL_VERBOSE __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 21, __dbgm_one_int_msg)
#define DBGM_DEVCTL_ERRFILE __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 22, __dbgm_str_msg)
#define DBGM_DEVCTL_NOERRFILE __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 23, __dbgm_na_msg)
#define DBGM_DEVCTL_USE_DLADDR __DIOT(_DCMD_MISC,  DBGM_CMD_CODE + 24, __dbgm_one_int_msg)

#endif
