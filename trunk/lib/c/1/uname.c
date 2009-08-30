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




#include <unistd.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <sys/utsname.h>

int uname(struct utsname *name) {
	if(		confstr(_CS_SYSNAME, name->sysname, sizeof name->sysname) == 0 ||
			confstr(_CS_HOSTNAME, name->nodename, sizeof name->nodename) == 0 ||
			confstr(_CS_RELEASE, name->release, sizeof name->release) == 0 ||
			confstr(_CS_VERSION, name->version, sizeof name->version) == 0 ||
			confstr(_CS_MACHINE, name->machine, sizeof name->machine) == 0) {
		return -1;
	}
	return 0;
}

__SRCVERSION("uname.c $Rev: 153052 $");
