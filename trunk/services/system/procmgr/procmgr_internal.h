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

#ifndef PROCMGR_INTERNAL_H
#define PROCMGR_INTERNAL_H

#define PROC_CODE_TERM			(SI_MINAVAIL + 1)
#define PROC_CODE_STOP			(SI_MINAVAIL + 2)
#define PROC_CODE_CONT			(SI_MINAVAIL + 3)
	
struct wait_entry {
	struct wait_entry				*next;
	int								rcvid;
	id_t							id;
	short 							idtype;
	short							options;
};

struct vfork_info {
	int								rcvid;
	struct _thread_local_storage	tls;
	void							*frame_base;
	unsigned						frame_size;
	unsigned						frame[1];
};

extern SOUL	wait_souls;

#include "procmgr_proto.h"

extern void	procmgr_context_init(void);

#endif

/* __SRCVERSION("procmgr_internal.h $Rev: 164352 $"); */
