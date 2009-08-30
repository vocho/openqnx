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
#include <unistd.h>
#include <process.h>
#include <spawn.h>
#include <errno.h>
#include <sys/wait.h>

int execve(const char *path, char * const *argv, char * const *envp) {
	spawn_inheritance_type		attr;

	attr.flags = SPAWN_EXEC;
	return spawn(path, 0, 0, &attr, argv, envp);
}

int execv(const char *path, char * const *argv) {
	return execve(path, argv, 0);
}

__SRCVERSION("execve.c $Rev: 153052 $");
