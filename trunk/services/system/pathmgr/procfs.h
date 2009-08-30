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

#ifndef PROCFS_H
#define PROCFS_H

#include "externs.h"

#define PROCFS_FLAG_AS			0x0001
#define PROCFS_FLAG_SELF		0x0002
#define PROCFS_FLAG_LINK		0x0004		/* Procmgr symlink information */
#define PROCFS_FLAG_MOUNT		0x0008		/* Procmgr mount information */
#define PROCFS_FLAG_DIR			0x0010		/* This entry a dir (for mount) */
#define PROCFS_FLAG_PROC		0x0020		/* This is the "/proc" entry */
#define PROCFS_FLAG_MASK		0x00fc		/* mask to see if it references a process */


struct procfs_ocb {
	struct procfs_ocb			*next;
	pid_t						pid;
	int							ioflag;
	unsigned					flags;
	int							tid;
	uintptr_t					offset;
	int							rcvid;
	struct sigevent				event;
	char						tail[1];
};

struct procfs_waiting {
	struct procfs_waiting		*next;
	int							rcvid;
};

#endif

/* __SRCVERSION("procfs.h $Rev: 0 $"); */
