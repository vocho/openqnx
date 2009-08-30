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




#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <process.h>
#include <spawn.h>
#include <errno.h>
#include "cvtl2v.h"

int execlpe(const char *path, const char *arg0, ...) {
	char		**argv;
	char		**envv;

	CVT_L2V_ENV(arg0, argv, envv);
	return execvpe(path, argv, envv);
}

__SRCVERSION("execlpe.c $Rev: 153052 $");
