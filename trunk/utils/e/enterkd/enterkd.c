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





#ifdef __USAGE
%C - Start a program and cause it to enter the kernel debugger

%C	[executable [command_line]]

If no executable is specifed, we just enter the kernel debugger directly.

NOTE: This utility is for internal use only. Do not ship to customers.
#endif

#include <stdio.h>
#include <spawn.h>
#include <unistd.h>
#include <string.h>
#include <sys/neutrino.h>

//
// start a process and make it go into the kernel debugger
//

int
main(int argc, char *argv[]) {
	struct inheritance	inherit;

	if(argc <= 1) {
		DebugKDBreak();
	} else {
		memset(&inherit, 0, sizeof(inherit));
		inherit.flags = SPAWN_DEBUG | SPAWN_EXEC;
		if(spawnp(argv[1], 0, NULL, &inherit, &argv[1], environ) == -1) {
			perror( "spawn failed");
		}
	}
	return 0;
}
