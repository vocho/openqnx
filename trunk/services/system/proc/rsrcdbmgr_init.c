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

/*
 Neutrino Resource Manager 
 - Maintains a database of resources which can be requested
   by other processes.
*/
#include <errno.h>
#include "rsrcdbmgr.h"
#include <sys/sysmsg.h>
#include <sys/iofunc.h>


int  rsrcdbmgr_handler(message_context_t *ctp, int code, unsigned flags, void *handle);
void rsrcdbmgr_seed(void);

/*
 Globals
*/
pthread_mutex_t					g_rsrc_mutex = PTHREAD_MUTEX_INITIALIZER;
rsrc_root_node_t				*g_rsrc_root;		

void rsrcdbmgr_init() {
	//Initialize all the list heads to NULL
	g_rsrc_root = NULL;

	rsrcdbmgr_seed();
	
	//Tell proc to send resource database messages our way
	message_attach(dpp, NULL, _RSRCDBMGR_BASE, _RSRCDBMGR_MAX, rsrcdbmgr_handler, NULL);
}


/*
 Called to destroy a process's resource allocations 
*/
void rsrcdbmgr_destroy_process(PROCESS *prp) {
	rsrc_list_array_t *pidrsrc;
	int				   i;

	if(!prp || !(pidrsrc = prp->rsrc_list)) {
		return;
	}

	//Release the resources held & allocated by this process
	if(pidrsrc->haversrc) {
		(void)_rsrcdbmgr_pid_clear(prp->pid);
	}

	//Release all of the devnos this device has
	for (i = 0; i < pidrsrc->dev_count; i++) {
		if((int)pidrsrc->dev_list[i] != -1) {
			int major, minor;
			major = major(pidrsrc->dev_list[i]);
			minor = minor(pidrsrc->dev_list[i]);
			(void)_rsrc_minor(NULL, &major, &minor, RSRCDBMGR_REQ_DETACH);
		}
	}
	if (pidrsrc->dev_count) {
		_sfree(pidrsrc->dev_list, pidrsrc->dev_count * sizeof(dev_t));
		pidrsrc->dev_count = 0;
	}

	//Get rid of the resource pointer itself
	if (prp->rsrc_list) {
		_sfree(prp->rsrc_list, sizeof(rsrc_list_array_t));
	}

	pthread_mutex_unlock(&g_rsrc_mutex);
}



__SRCVERSION("rsrcdbmgr_init.c $Rev: 153052 $");
