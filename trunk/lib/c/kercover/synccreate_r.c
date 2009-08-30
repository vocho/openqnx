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




#include <sys/neutrino.h>

/*
 * This function is for backwards compatibility. The two arguments
 * "sync" and "attr" are passed to the kernel as the first two
 * arguments of SyncTypeCreate. The kernel looks at the first
 * argument of SyncTypeCreate and if it is a valid pointer it
 * treats the call like it is the older SyncCreate function.
 */

int SyncCreate_r(sync_t *sync, const struct _sync_attr *attr) {
	return SyncTypeCreate_r((unsigned)sync, (sync_t *)attr, 0);
}

__SRCVERSION("synccreate_r.c $Rev: 153052 $");
