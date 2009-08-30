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
#include <string.h>
#include <spawn.h>


pid_t spawnp(const char *file, int fd_count, const int fd_map[],
		const struct inheritance *inherit, char * const argv[], char * const envp[]) {
	struct inheritance			attr;

	if(inherit) {
		attr = *inherit;
	} else {
		memset(&attr, 0x00, sizeof attr);
	}	

	if(!strchr(file, '/')) {
		attr.flags |= SPAWN_SEARCH_PATH;
	}

	attr.flags |= SPAWN_CHECK_SCRIPT;

	return spawn(file, fd_count, fd_map, &attr, argv, envp);
}

__SRCVERSION("spawnp.c $Rev: 153052 $");
