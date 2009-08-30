/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

/*
 * mig4nto_init.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/neutrino.h>
#include <mig4nto.h>
#include <mig4nto_procmsg.h>

magic_t	magic;	/* defined in mig4nto.h */

int
mig4nto_init(void)
{
	/* 
	 * this ChannelCreate() must be the first channel created for this
	 * process so that its chid is 1
	 */
	if ((magic.ipc_chid = ChannelCreate(0)) == -1 || magic.ipc_chid != 1)
		return -1;
	if ((magic.procmgr_fd = open(_PROCMGR_PATH, O_RDWR)) == -1) {
		ChannelDestroy(magic.ipc_chid);
		return -1;
	}
	fcntl(magic.procmgr_fd, F_SETFD, FD_CLOEXEC);
	if (ipc_init() == -1 || name_init() == -1) {
		ChannelDestroy(magic.ipc_chid);
		close(magic.procmgr_fd);
		return -1;
	}
	qnx_hint_table_init();
	return 0;
}
