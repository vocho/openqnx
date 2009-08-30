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
 * disk_get_entry.c - QNX 4 to QNX Neutrino migration functions
 */
 
#include <devctl.h>
#include <errno.h>
#include <string.h>
#include <sys/cam_device.h>
#include <sys/dcmd_blk.h>
#include <sys/dcmd_cam.h>
#include <sys/disk.h>
#include <mig4nto.h>

int disk_get_entry(int fd, struct _disk_entry *entry)
{
	static const int		typemappings[D_MASK] = { _HARD, _TAPE, _PRINTER, _PROCESSOR, _WORM, _CDROM, _SCANNER, _OPTICAL, _MEDIA_CHG, _COMMS };
	struct cam_devinfo		cam;
	struct partition_entry	part;
	int						result;

	if ((result = devctl(fd, DCMD_CAM_DEVINFO, &cam, sizeof(cam), NULL)) != EOK || (result = devctl(fd, DCMD_BLK_PARTENTRY, &part, sizeof(part), NULL)) != EOK)
		return(errno = result, -1);
	entry->blk_offset = part.part_offset;
	entry->num_sectors = part.part_size;
	entry->disk_sectors = cam.num_sctrs;
	entry->cylinders = cam.cylinders;
	entry->heads = cam.heads;
	entry->track_sectors = cam.tracks;
	entry->disk_type = (cam.flags & DEV_REMOVABLE) ? _REMOVABLE : typemappings[cam.type & D_MASK];
	entry->disk_drv = 0;
	memset(entry->reserved, 0, sizeof(entry->reserved));
	strncpy(entry->driver_name, "NTO2 devb-*", _DRIVER_NAME_LEN);
	return(0);
}
