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
 *  sys/mount.h
 *

 */
#ifndef __MOUNT_H_INCLUDED
#define __MOUNT_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __STATVFS_H_INCLUDED
 #include <sys/statvfs.h>
#endif

/* These flags match with the iofunc mount flags and can be masked directly */
#define _MOUNT_READONLY	ST_RDONLY	/* read only */
#define _MOUNT_NOEXEC	ST_NOEXEC	/* can't exec from filesystem */
#define _MOUNT_NOSUID	ST_NOSUID	/* don't honor setuid bits on fs */
#define _MOUNT_NOCREAT	ST_NOCREAT	/* don't allow creat on this fs */
#define _MOUNT_OFF32	ST_OFF32	/* Limit off_t to 32 bits */
#define _MOUNT_NOATIME	ST_NOATIME	/* don't update times if only atime is dirty */

/* These flags match with the RESMGR flags if you shift by 32 bits */
#define _MOUNT_BEFORE   0x00010000  /* call pathname attach with RESMGR_FLAG_BEFORE */
#define _MOUNT_AFTER    0x00020000  /* call pathname attach with RESMGR_FLAG_AFTER */
#define _MOUNT_OPAQUE   0x00040000  /* call pathname attach with RESMGR_FLAG_OPAQUE */

/* These flags are mount specific */
#define _MOUNT_UNMOUNT  0x00001000  /* Unmount this path */
#define _MOUNT_REMOUNT  0x00002000  /* This path is already mounted, perform an update */
#define _MOUNT_FORCE	0x00004000  /* Force an unmount or a remount change */
#define _MOUNT_ENUMERATE	0x00008000  /* Auto-detect on this device */
#define _MOUNT_IMPLIED	0x00080000  /* The mountpoint is unspecified by client */


#define _MFLAG_OCB      0x80000000  /* Attempt to open the device and send and ocb to server */
#define _MFLAG_SPEC     0x40000000  /* Send the special device string to the server */
#define _MFLAG_STRUCT   0x20000000  /* The data is not a string, but a structure and datalen is defined */

__BEGIN_DECLS

/* 
 If spec starts with a leading / then the device will be
 opened and an ocb send along with the mount request.

 If datalen is < 0 then it is assumed that data contains
 a null terminated string.
*/
int mount(const char *spec, const char *dir, int flags, 
          const char *type, const void *data, int datalen);

/*
 There are currently no flags defined for umount
*/
int umount(const char *dir, int flags);

__END_DECLS

/*
 This is a helper routine for argument parsing
*/
#define _MOPTION_RDONLY		"ro"
#define _MOPTION_RDWR		"rw"
#define _MOPTION_EXEC		"exec"
#define _MOPTION_NOEXEC		"noexec"
#define _MOPTION_SUID		"suid"
#define _MOPTION_NOSUID		"nosuid"
#define _MOPTION_ATIME		"atime"
#define _MOPTION_NOATIME	"noatime"
#define _MOPTION_REMOUNT	"remount"
#define _MOPTION_ENUMERATE	"enumerate"
#define _MOPTION_BEFORE		"before"	
#define _MOPTION_AFTER		"after"
#define _MOPTION_OPAQUE		"opaque"
#define _MOPTION_NOSTAT		"nostat"	
char * mount_parse_generic_args(char *options, int *flags);

/*
 * Commonly used system mount strings
 */

#define NFS_FS_TYPE				"nfs"
#define CIFS_FS_TYPE			"cifs"
#define QNX4_FS_TYPE			"qnx4"
#define EXT2_FS_TYPE			"ext2"
#define DOS_FS_TYPE				"dos"
#define CD_FS_TYPE				"cd"
#define FLASH_FS_TYPE			"flash"

#endif

/* __SRCVERSION("mount.h $Rev: 153052 $"); */
