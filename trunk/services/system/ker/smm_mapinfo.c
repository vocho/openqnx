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

#include "externs.h"

size_t
smm_mapinfo(PROCESS *prp, uintptr_t vaddr, struct _procfs_map_info *mip, 
				struct _procfs_debug_info *mdp, size_t dbginfo_pathlen, OBJECT **obpp, int *fdp, 
				size_t *contigp) {
	return 0;
}

__SRCVERSION("smm_mapinfo.c $Rev: 153052 $");
