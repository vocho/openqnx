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
#include <stdarg.h>
#include <sys/wait.h>

int spawnve(int mode, const char *path, char * const argv[], char * const envp[]) {
	pid_t						pid;
	spawn_inheritance_type		attr;

	switch(mode) {
	case P_WAIT:
	case P_NOWAIT:
		attr.flags = 0;
		break;

	case P_OVERLAY:
		attr.flags = SPAWN_EXEC;
		break;

	case P_NOWAITO:
		attr.flags = SPAWN_NOZOMBIE;
		break;

	default:
		errno = EINVAL;
		return -1;
	}

	if((pid = spawn(path, 0, 0, &attr, argv, envp)) != -1) {
		if(mode == P_WAIT) {
			if(waitpid(pid, &pid, 0) == -1) {
				return -1;
			}
		}
	}

	return pid;
}

int spawnv(int mode, const char *path, char * const *argv) {
	return spawnve(mode, path, argv, 0);
}

__SRCVERSION("spawnve.c $Rev: 153052 $");
