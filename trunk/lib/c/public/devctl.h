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
 * devctl.h
 *

 */

#ifndef _DEVCTL_H_INCLUDED
#define _DEVCTL_H_INCLUDED


#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#ifndef _LIMITS_H_INCLUDED
 #include <limits.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif


/*
 * Flag bits applied to dcmd parameter of devctl()
 * Note that we limit the cmd portion to the low order 16 bits for
 * compatibility with UNIX ioctl's.
 */

#define _POSIX_DEVDIR_NONE		0
#define _POSIX_DEVDIR_TO		0x80000000
#define _POSIX_DEVDIR_FROM		0x40000000
#define _POSIX_DEVDIR_TOFROM	(_POSIX_DEVDIR_TO | _POSIX_DEVDIR_FROM)
#define _POSIX_DEVDIR_CMD_MASK	0x0000FFFF

#define DEVDIR_NONE				_POSIX_DEVDIR_NONE
#define DEVDIR_TO				_POSIX_DEVDIR_TO
#define DEVDIR_FROM				_POSIX_DEVDIR_FROM
#define DEVDIR_TOFROM			_POSIX_DEVDIR_TOFROM
#define DEVDIR_CMD_MASK			_POSIX_DEVDIR_CMD_MASK

__BEGIN_DECLS


struct iovec;
extern int devctl(int fd, int dcmd, void *dev_data_ptr, _CSTD size_t nbytes, int *dev_info_ptr);
extern int devctlv(int fd, int dcmd, int sparts, int rparts, const struct iovec *sv, const struct iovec *rv, int *dev_info_ptr);
extern int get_device_command(int command);
extern int get_device_direction(int command);
extern int set_device_direction(int command, int direction);

#define get_device_command(_cmd)			((_cmd)&DEVDIR_CMD_MASK)
#define get_device_direction(_cmd)			((_cmd)&DEVDIR_TOFROM)
#define set_device_direction(_cmd, _dir)	((_cmd)|(_dir))


/*
 * ioctl is built() on top of devctl().
 * Both devcmd() and ioctl() encode the direction in the command using
 * the same 2 high order bits.
 *
 * All QNX devctl's are defined in this file. Other codes may exist for
 * 3rd party products in files provided by the 3rd party.
 *
 *
 * The dcmd codes are subdivided into the following classes:
 *
 *      QNX reserved            0x000 - 0x0ff
 *      common (all io servers) 0x100 - 0x1ff
 *      Fsys/Block              0x200 - 0x2ff 
 *      Character               0x300 - 0x3ff
 *      Network                 0x400 - 0x4ff
 *      Misc                    0x500 - 0x5ff
 *      IP                  	0x600 - 0x6ff
 *      Mixer					0x700 - 0x7ff
 *		Proc					0x800 - 0x8ff
 *      Memory card				0x900 - 0x9ff
 *      Input devices           0xA00 - 0xAff
 *      CAM                     0xC00 - 0xCff
 *      USB                     0xD00 - 0xDff
 *      MEDIA                   0xE00 - 0xEff
 *      CAM - SIM               0xF00 - 0xFff
 *
 * Add new ranges here and create a header file in <sys/dcmd_???.h>
 * which you include at the bottom of this file.
 *
 *      QNX reserved            0x000 - 0xfff
 */
#define _DCMD_ALL		0x01
#define _DCMD_FSYS		0x02
#define _DCMD_BLK		_DCMD_FSYS
#define _DCMD_CHR		0x03
#define _DCMD_NET		0x04
#define _DCMD_MISC		0x05
#define _DCMD_IP		0x06
#define _DCMD_MIXER		0x07
#define _DCMD_PROC		0x08
#define _DCMD_MEM		0x09
#define _DCMD_INPUT		0x0A
#define _DCMD_PHOTON	0x0B
#define _DCMD_CAM		0x0C
#define _DCMD_USB		0x0D
#define _DCMD_MEDIA     0x0E
#define _DCMD_CAM_SIM	0x0F
#define _DCMD_MEMCLASS	0x10
#define _DCMD_PARTITION	0x11

#define __DIOF(class, cmd, data)	((sizeof(data)<<16) + ((class)<<8) + (cmd) + _POSIX_DEVDIR_FROM)
#define __DIOT(class, cmd, data)	((sizeof(data)<<16) + ((class)<<8) + (cmd) + _POSIX_DEVDIR_TO)
#define __DIOTF(class, cmd, data)	((sizeof(data)<<16) + ((class)<<8) + (cmd) + _POSIX_DEVDIR_TOFROM)

#define __DION(class, cmd)			(((class)<<8) + (cmd) + _POSIX_DEVDIR_NONE)
#define _DEVCTL_DATA(msg)			((void *)(sizeof(msg) + (char *)(&msg)))

#if defined(__EXT_QNX)
#define _DEVCTL_FLAG_NORETVAL	0x00000001
#define _DEVCTL_FLAG_NOTTY		0x00000002
#define _DEVCTL_FLAG_NOCANCEL	0x00000004

extern int _devctl(int fd, int dcmd, void *data_ptr, _CSTD size_t nbytes, unsigned flags);
#endif

__END_DECLS

#endif

/* __SRCVERSION("devctl.h $Rev: 168446 $"); */
