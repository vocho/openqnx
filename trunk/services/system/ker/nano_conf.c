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
#include <confname.h>
#include <sys/conf.h>

static int sc_nprocessors(int name) {
	return NUM_PROCESSORS;
}

const long					 kernel_conf_table[] = {
	_CONF_VALUE_NUM|_CONF_INDIRECT |	_SC_CLK_TCK, 		(intptr_t)&clk_tck,
	_CONF_VALUE_MIN|_CONF_INDIRECT |	_SC_OPEN_MAX, 		(intptr_t)&max_fds,
	_CONF_VALUE_MIN | 					_SC_NGROUPS_MAX, 	NGROUPS_MAX,
	_CONF_VALUE_NUM |					_SC_SAVED_IDS,		1,
	_CONF_VALUE_NUM |					_SC_NZERO,			20,
	_CONF_VALUE_NUM |					_SC_RTSIG_MAX,		(SIGRTMAX - SIGRTMIN) + 1,
	_CONF_VALUE_MIN |					_SC_CHILD_MAX,		PID_MASK - 2,
	_CONF_VALUE_MAX |					_SC_THREAD_STACK_MIN,	PTHREAD_STACK_MIN,
	_CONF_VALUE_NUM |					_SC_THREAD_THREADS_MAX, VECTOR_MAX,
	_CONF_VALUE_NUM|_CONF_FCN |			_SC_NPROCESSORS_CONF,	(intptr_t)sc_nprocessors,
	_CONF_VALUE_NUM|_CONF_FCN |			_SC_NPROCESSORS_ONLN,	(intptr_t)sc_nprocessors,
	_CONF_VALUE_NUM |					_SC_DELAYTIMER_MAX,	DELAYTIMER_MAX,
	_CONF_VALUE_NUM |					_SC_AIO_PRIO_DELTA_MAX,	NUM_PRI - 1,
	_CONF_END
};

__SRCVERSION("nano_conf.c $Rev: 153052 $");
