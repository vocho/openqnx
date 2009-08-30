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
 *  ftype.h    File types.
 *

 */
#ifndef __FTYPE_H_INCLUDED
#define __FTYPE_H_INCLUDED

enum _file_type {
	_FTYPE_MATCHED = -1,
	_FTYPE_ALL = -1,
	_FTYPE_ANY = 0,
	_FTYPE_FILE,
	_FTYPE_LINK,
	_FTYPE_SYMLINK,
	_FTYPE_PIPE,
	_FTYPE_SHMEM,
	_FTYPE_MQUEUE,
	_FTYPE_SOCKET,
	_FTYPE_SEM,
	_FTYPE_PHOTON,
	_FTYPE_DUMPER,
	_FTYPE_MOUNT,
	_FTYPE_NAME,
	_FTYPE_TYMEM
};

#define _MAJOR_PATHMGR		"pathmgr"	/* Use by path manager only */
#define _MAJOR_DEV			"dev"		/* Devices in /dev with only one instance (/dev/tty) */
#define _MAJOR_BLK_PREFIX	"blk-"		/* All block devices (/dev/hd[0-9]* would be "blk-hd") */
#define _MAJOR_CHAR_PREFIX	"char-"		/* All char devices (/dev/ser[0-9]* would be "char-ser") */
#define _MAJOR_FSYS			"fsys"		/* All filesystems */

#endif

/* __SRCVERSION("ftype.h $Rev: 153052 $"); */
