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
 * fsys_get_mount_dev.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <fcntl.h>
#include <share.h>
#include <string.h>
#include <sys/dcmd_blk.h>
#include <sys/iomsg.h>
#include <mig4nto.h>

int fsys_get_mount_dev(const char *path, char *device)
{
	union {
		struct {
			struct _io_devctl		devctl;
		}	i;
		struct {
			struct _io_devctl_reply	devctl;
			char					path[256];
		}	o;
	}	msg;

	msg.i.devctl.type = _IO_DEVCTL;
	msg.i.devctl.combine_len = sizeof(msg.i);
	msg.i.devctl.dcmd = DCMD_FSYS_MOUNTED_ON;
	msg.i.devctl.nbytes = DCMD_FSYS_MOUNTED_ON >> 16;
	msg.i.devctl.zero = 0;
	if (_connect_combine(path, 0, O_ACCMODE, SH_DENYNO, 0, _FTYPE_ANY, sizeof(msg.i), &msg.i, sizeof(msg.o), &msg.o) == -1)
		return(-1);
	strcpy(device, msg.o.path);
	return(0);
}
