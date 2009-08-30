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
 *  iomgr.h    Non-portable low-level IO definitions
 *

 */
#ifndef __IOMGR_H_INCLUDED
#define __IOMGR_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

enum _mgr_types {
	_IOMGR_FSYS = 0x02,					/* matches _DCMD_FSYS */
	_IOMGR_TCPIP = 0x06,				/* matches _DCMD_IP */
	_IOMGR_PHOTON = 0x0B,				/* matches _DCMD_PHOTON */
	_IOMGR_CAM = 0x0C,					/* matches _DCMD_CAM */
	_IOMGR_PCI = 0x0d,					/* matches _DCMD_PCI */
	_IOMGR_NETMGR = 0x0e,				/* matches _DCMD_NETMGR */
	_IOMGR_REGISTRY = 0x10,				/* registry */
	_IOMGR_PCCARD = 0x11,				/* PCCARD Manager */
	_IOMGR_USB = 0x12,					/* USB */
	_IOMGR_MEDIA = 0x13,				/* Media manager */
	_IOMGR_PMM = 0x14,					/* Power manager */
	_IOMGR_DISPLAY = 0x15,				/* Display device manager */
	_IOMGR_INPUT = 0x16,				/* Input drivers */
	_IOMGR_PRIVATE_BASE = 0xf000,		/* available for non-registered use */
	_IOMGR_PRIVATE_MAX = 0xffff
};

/* flags definitions */
#define _NOTIFY_COND_EXTEN		0x80000000

/* Same as RDBAND, WRNORM, RDNORM below respectively */
#define _NOTIFY_COND_OBAND		0x40000000	/* Out-of-band data is available */
#define _NOTIFY_COND_OUTPUT		0x20000000	/* Room for more output */
#define _NOTIFY_COND_INPUT		0x10000000	/* Data is available */

/*
 * Masks for (io_notify_t).i.event.sigev_value.
 *
 * Requested conditions, extended or non, set
 * in (io_notify_t).i.flags.  Returned conditions,
 * extended or non, returned at poll in
 * (io_notify_t).o.flags.  If extended conditions
 * are requested, and they need to be returned in an
 * armed event, the negative of the satisfied conditions
 * are returned in (io_notify_t).i.event.sigev_code.
 */
#define _NOTIFY_COND_MASK		0xf0000000	/* Mask for conditions */
#define _NOTIFY_DATA_MASK		(~_NOTIFY_COND_MASK)

/* If _NOTIFY_COND_EXTEN set */
 /* See POLL* flags in <sys/poll.h> */
#define _NOTIFY_CONDE_RDNORM	0x00000001	/* Normal data is available */
#define _NOTIFY_CONDE_WRNORM	0x00000002	/* Room for normal data */
#define _NOTIFY_CONDE_RDBAND	0x00000004	/* Out-of-band data is available */
#define _NOTIFY_CONDE_PRI		0x00000008	/* Priority data is available */
#define _NOTIFY_CONDE_WRBAND	0x00000010	/* Room for OOB data */
#define _NOTIFY_CONDE_ERR		0x00000020
#define _NOTIFY_CONDE_HUP		0x00000040

#define _NOTIFY_CONDE_NVAL		0x00001000
/* endif */

/*
 * If returning an armed event, the negative
 * of the satisfied conditions is returned in
 * a short.  Therefor at max 15 conditions if
 * extended versions are requested.
 */
#define _NOTIFY_MAXCOND 15


#define _NOTIFY_FLAGS_EXTEN_FIRST			0x00000001


/*
 * action definitions
 * 00  Always arm for data transition. Usefull for mq_notify(). [EOK, EBUSY]
 * 01  Arm if no data. Useful in that it may be combined with a read. [EOK, EAGAIN, EBUSY]
 * 10  Poll and disarm. [EOK:bits, EBUSY]
 * 11  Poll and arm if no data. Useful for select(). [EOK:bits, EBUSY]
 *
 * For actions which arm an event it will be disarmed when triggered.
 *
 * POLL       - Never arm. Never trig. Return cond.
 * 
 * POLLARM    - Arm if cond not met. Never trig immed. Return cond.
 * 
 * TRANARM    - Always arm. Never trig immed. Trig only when new data
 *              arrives in an empty buffer/queue. Return 0.
 * 
 * CONDARM    - Arm if cond not met and return -1, errno = EAGAIN. If cond
 *              met return 0.
 */
#define _NOTIFY_ACTION_TRANARM	0x0
#define _NOTIFY_ACTION_CONDARM	0x1
#define _NOTIFY_ACTION_POLL		0x2
#define _NOTIFY_ACTION_POLLARM	0x3
#define _NOTIFY_ACTION_MASK		0x3

/*
 * Passed to iofdinfo()
 */
#define _FDINFO_FLAG_LOCALPATH	0x00000001	/* Used to return smaller path for displaying */

/*
 * Returned in "flags" below
 */
#define _FDINFO_LOCKS			0x00000001	/* There are active locks on the file */
#define _FDINFO_MMAPS			0x00000002	/* There are active mmaps on the file */

struct _fdinfo {
	_Uint32t					mode;	/* File mode */
	_Uint32t					ioflag;	/* Current io flags */
	_Uint64t					offset;	/* Current seek position */
	_Uint64t					size;	/* Current size of file */
	_Uint32t					flags;	/* _FDINFO_* */
	_Uint16t					sflag;	/* Share flags */
	_Uint16t					count;	/* File use count */
	_Uint16t					rcount;	/* File reader count */
	_Uint16t					wcount;	/* File writer count */
	_Uint16t					rlocks;	/* Number of read locks */
	_Uint16t					wlocks;	/* Number of write locks */
	_Uint32t					zero[6];
};

__BEGIN_DECLS
#if 0		/* These are in unistd.h */
extern int readcond(int __fd, void *__buff, int __nbytes, int __min, int __time, int __timeout);
extern int ionotify(int __fd, int __action, int __flags, const struct sigevent *__event);
#endif
extern int iofdinfo(int __fd, unsigned __flags, struct _fdinfo *__info, char *__path, int __maxlen);
__END_DECLS

#endif

/* __SRCVERSION("iomgr.h $Rev: 153052 $"); */
