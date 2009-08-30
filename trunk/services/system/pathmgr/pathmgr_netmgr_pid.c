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

#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include <kernel/nto.h>
#include "pathmgr_node.h"
#include "pathmgr_object.h"
#include "pathmgr_proto.h"

/*
 * Find the process id of the network manager.
 */
pid_t pathmgr_netmgr_pid(void) {
	NODE	*pathnode;
	pid_t	pid;

	pid = 0;
	pathnode = pathmgr_node_lookup(NULL, "dev/netmgr", PATHMGR_LOOKUP_ATTACH, NULL);
	if(pathnode != NULL) {
		if(pathnode->object != NULL) {
			pid = pathnode->object->server.pid;
		}
		pathmgr_node_detach(pathnode);
	}
	return pid;
}

__SRCVERSION("pathmgr_netmgr_pid.c $Rev: 153052 $");
