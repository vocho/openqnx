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






#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <sys/netmgr.h>
#include <spawn.h>
#include <libgen.h>
#include <sys/shutdown.h>


int spawn_remote_shutdown(char *node_name, char **argv)
{
	char path[PATH_MAX];
	struct inheritance inherit;

	if(netmgr_path(node_name, NULL, path, PATH_MAX) == -1){
		snprintf(path, PATH_MAX, "Invalid node '%s': %s", node_name, strerror(errno));
		shutdown_error(path);
		return -1;
	}
	
	if(chroot(path) != 0){
		snprintf(path, PATH_MAX, "Cannot chroot to node '%s': %s", node_name, strerror(errno));
		shutdown_error(path);
		return -1;
	}
	
	inherit.flags = SPAWN_NOZOMBIE | SPAWN_SETND;
	
	if((inherit.nd = netmgr_strtond(optarg, 0)) == -1){
		snprintf(path, PATH_MAX, "Unable to convert '%s' to node descriptor: %s", node_name, strerror(errno));
		shutdown_error(path);
		return -1;
	}

	return spawnp(basename(argv[0]), 0, NULL, &inherit, argv, NULL);
}
