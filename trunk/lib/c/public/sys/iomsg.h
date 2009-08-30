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
 *  iomsg.h    Non-portable low-level IO definitions
 *

 */
#ifndef __IOMSG_H_INCLUDED
#define __IOMSG_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef _STDINT_H_INCLUDED
#include <stdint.h>
#endif

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#ifndef __STAT_H_INCLUDED
 #include <sys/stat.h>
#endif

#ifndef __STATVFS_H_INCLUDED
 #include <sys/statvfs.h>
#endif

#ifndef __NEUTRINO_H_INCLUDED
 #include <sys/neutrino.h>
#endif

#ifndef _UTIME_H_INCLUDED
 #include <utime.h>
#endif

#ifndef __FTYPE_H_INCLUDED
 #include <sys/ftype.h>
#endif

#ifndef _LIMITS_H_INCLUDED
 #include <limits.h>
#endif

#ifndef __MOUNT_H_INCLUDED
 #include <sys/mount.h>
#endif

#ifndef __IOMGR_H_INCLUDED
 #include <sys/iomgr.h>
#endif

#ifndef	__PM_H_INCLUDED
 #include <sys/pm.h>
#endif

__BEGIN_DECLS

#include <_pack64.h>

/*
 *  Message types
 */
enum _msg_bases {
	_IO_BASE = 0x100,
	_IO_MAX = 0x1FF
};

enum _io__Uint16types {
	_IO_CONNECT = _IO_BASE,
	_IO_READ,
	_IO_WRITE,
	_IO_RSVD_CLOSE_OCB,		/* Place holder in jump table */
	_IO_STAT,
	_IO_NOTIFY,
	_IO_DEVCTL,
	_IO_RSVD_UNBLOCK,		/* Place holder in jump table */
	_IO_PATHCONF,
	_IO_LSEEK,
	_IO_CHMOD,
	_IO_CHOWN,
	_IO_UTIME,
	_IO_OPENFD,
	_IO_FDINFO,
	_IO_LOCK,
	_IO_SPACE,
	_IO_SHUTDOWN,
	_IO_MMAP,
	_IO_MSG,
	_IO_RSVD,
	_IO_DUP,
	_IO_CLOSE,
	_IO_RSVD_LOCK_OCB,		/* Place holder in jump table */
	_IO_RSVD_UNLOCK_OCB,	/* Place holder in jump table */
	_IO_SYNC,
	_IO_POWER
};


enum _io_msg_xtypes {
	_IO_XTYPE_NONE,
	_IO_XTYPE_READCOND,
	_IO_XTYPE_MQUEUE,
	_IO_XTYPE_TCPIP,
	_IO_XTYPE_TCPIP_MSG,
	_IO_XTYPE_OFFSET,
	_IO_XTYPE_REGISTRY,
	_IO_XTYPE_MASK =			0x000000ff,

	/*
	 * The _IO_XFLAG_DIR_EXTRA_HINT flag is only valid when
	 * reading from a directory. The filesystem should normally
	 * return extra directory information when it is easy to get.
	 * If this flag is set, it is a hint to the filesystem
	 * to try harder (possibly causing media lookups) to return
	 * the extra information. The most common use would be to
	 * return _DTYPE_LSTAT information.
	 */
	_IO_XFLAG_DIR_EXTRA_HINT =	0x00000100,

	_IO_XFLAG_NONBLOCK =		0x00004000,
	_IO_XFLAG_BLOCK =			0x00008000
	/* Upper 16 bits are for use by the specific xtype */
};

struct _xtype_readcond {
	_Int32t			min;
	_Int32t			time;
	_Int32t			timeout;
};

struct _xtype_offset {
	off64_t		offset;
};

/***********************************************************************
 * IO message types which are pathname based                           *
 ***********************************************************************/

/*
 * Message of _IO_CONNECT
 */
struct _io_connect {
	_Uint16t				type;
	_Uint16t				subtype;		/* _IO_CONNECT_? */
	_Uint32t				file_type;		/* _FTYPE_? in sys/ftype.h */
	_Uint16t				reply_max;
	_Uint16t				entry_max;
	_Uint32t				key;
	_Uint32t				handle;
	_Uint32t				ioflag;			/* O_? in fcntl.h & _IO_FLAG_? */
	_Uint32t				mode;			/* S_IF? in sys/stat.h */
	_Uint16t				sflag;			/* SH_? in share.h */
	_Uint16t				access;			/* S_I in sys/stat.h */
	_Uint16t				zero;
	_Uint16t				path_len;
	_Uint8t					eflag;			/* _IO_CONNECT_EFLAG_? */
	_Uint8t					extra_type;		/* _IO_CONNECT_EXTRA_? */
	_Uint16t				extra_len;
	char					path[1];		/* path_len, null, extra_len */
};

/* struct _io_connect subtype */
enum _io_connect_subtypes {
	_IO_CONNECT_COMBINE,		/* Combine with IO msg */
	_IO_CONNECT_COMBINE_CLOSE,	/* Combine with IO msg and always close */
	_IO_CONNECT_OPEN,
	_IO_CONNECT_UNLINK,
	_IO_CONNECT_RENAME,
	_IO_CONNECT_MKNOD,
	_IO_CONNECT_READLINK,
	_IO_CONNECT_LINK,
	_IO_CONNECT_RSVD_UNBLOCK,	/* Place holder in jump table */
	_IO_CONNECT_MOUNT
};

/* struct _io_connect extra_type */
enum _io_connect_extra_type {
	_IO_CONNECT_EXTRA_NONE,
	_IO_CONNECT_EXTRA_LINK,
	_IO_CONNECT_EXTRA_SYMLINK,
	_IO_CONNECT_EXTRA_MQUEUE,
	_IO_CONNECT_EXTRA_PHOTON,
	_IO_CONNECT_EXTRA_SOCKET,
	_IO_CONNECT_EXTRA_SEM,
	_IO_CONNECT_EXTRA_RESMGR_LINK,
	_IO_CONNECT_EXTRA_PROC_SYMLINK,
	_IO_CONNECT_EXTRA_RENAME,
	_IO_CONNECT_EXTRA_MOUNT,
	_IO_CONNECT_EXTRA_MOUNT_OCB,
	_IO_CONNECT_EXTRA_TYMEM
};

/* struct _io_connect ioflag (non-masked values match O_? in fcntl.h) */
#define _IO_FLAG_RD					0x00000001  /* read permission */
#define _IO_FLAG_WR					0x00000002  /* write permission */
#define _IO_FLAG_MASK				0x00000003  /* permission mask */

/* struct _io_connect eflag */
#define _IO_CONNECT_EFLAG_DIR		0x01	/* Path referenced a directory    */
#define _IO_CONNECT_EFLAG_DOT		0x02	/* Last component was . or ..     */
#define _IO_CONNECT_EFLAG_DOTDOT	0x04	/* Last component was ..          */

/*
 * return status from connect (These are continuation cases that
 * always negative (RET_FLAG is set). To avoid conflicting io msgs
 * must never return a negative status
 */
#define _IO_CONNECT_RET_UMASK		0x00020000	/* umask field in link reply is valid */
#define _IO_CONNECT_RET_NOCTTY		0x00040000	/* No controling terminal defined  */
#define _IO_CONNECT_RET_CHROOT		0x00080000	/* chroot_len field in link reply is valid */
#define _IO_CONNECT_RET_MSG			0x00100000	/* Connect to server and send new message */

#define _IO_CONNECT_RET_TYPE_MASK	0x0001e000	/* Mask for returned file type */
#define _IO_CONNECT_RET_FTYPE		0x00008000	/* File type was matched, _io_connect_ftype_reply expected */
#define _IO_CONNECT_RET_LINK		0x00010000  /* Not fully resolved, follow link */

#define _IO_CONNECT_RET_FLAG		0x80000000	/* Must be set to signify connect is returing */

#define _IO_SET_CONNECT_RET(_c, _s)	_RESMGR_STATUS(_c, _IO_CONNECT_RET_FLAG | (_s))		/* Sets the connect return code */

/* _io_connect reply redirecting resolution to other entries */
struct _io_connect_link_reply {
	_Uint32t					reserved1;
    _Uint32t                    file_type;      /*_FTYPE_? in sys/ftype.h */
	_Uint8t						eflag;          /* _IO_CONNECT_EFLAG_? */
	_Uint8t						reserved2[1];
	_Uint16t					chroot_len;		/* len of chroot in returned path*/
	_Uint32t					umask;			/* S_IF? in sys/stat.h */
	_Uint16t					nentries;		/* If zero, path is a symbolic link */
	_Uint16t					path_len;       /* len includs null, if zero, path is null terminated */
/*
	struct _io_connect_entry	server[nentries];
	char						path[path_len];
or
	struct _server_info			info;
	io_?_t						msg;
*/
};

/* _io_connect reply indicating a change/reply of a certain ftype and errno */
struct _io_connect_ftype_reply {
        _Uint16t                                        status;                 /* Typically an errno */
        _Uint16t                                        reserved;
        _Uint32t                                        file_type;              /* _FTYPE_? in sys/ftype.h */
};

/* used in _io_connect_link_reply */
struct _io_connect_entry {
	_Uint32t						nd;
	_Int32t							pid;
	_Int32t							chid;
	_Uint32t						handle;
	_Uint32t						key;
	_Uint32t						file_type;
	_Uint16t						prefix_len;
	_Uint16t						zero[3];
};

typedef union {
	struct _io_connect					connect;
	struct _io_connect_link_reply		link_reply;
	struct _io_connect_ftype_reply		ftype_reply;
} io_open_t;


typedef union {
	struct _io_connect					connect;
	struct _io_connect_link_reply		link_reply;
	struct _io_connect_ftype_reply		ftype_reply;
} io_unlink_t;


typedef union {
	struct _io_connect					connect;
	struct _io_connect_link_reply		link_reply;
	struct _io_connect_ftype_reply		ftype_reply;
} io_rename_t;

typedef union _io_rename_extra {
	char								path[1];
} io_rename_extra_t;


typedef union {
	struct _io_connect					connect;
	struct _io_connect_link_reply		link_reply;
	struct _io_connect_ftype_reply		ftype_reply;
} io_mknod_t;


typedef union {
	struct _io_connect					connect;
	struct _io_connect_link_reply		link_reply;
	struct _io_connect_ftype_reply		ftype_reply;
} io_readlink_t;


typedef union {
	struct _io_connect					connect;
	struct _io_connect_link_reply		link_reply;
	struct _io_connect_ftype_reply		ftype_reply;
} io_link_t;

struct _io_resmgr_link_extra {
	_Uint32t							nd;
	_Int32t								pid;
	_Int32t								chid;
	_Uint32t							handle;
	_Uint32t							flags;
	_Uint32t							file_type;
	_Uint32t							reserved[2];
};

typedef union _io_link_extra {
	struct _msg_info					info;		/* EXTRA_LINK (from client) */
	void								*ocb;		/* EXTRA_LINK (from resmgr functions) */
	char								path[1];	/* EXTRA_SYMLINK */
	struct _io_resmgr_link_extra		resmgr;		/* EXTRA_RESMGR_LINK */
} io_link_extra_t;
 
typedef union {
	struct _io_connect					connect;
	struct _io_connect_link_reply		link_reply;
	struct _io_connect_ftype_reply		ftype_reply;
} io_mount_t;

typedef struct _io_mount_extra {
	_Uint32t			flags;				/* _MOUNT_? or ST_? flags above */
	_Uint32t			nbytes;				/* size of entire structure */
	_Uint32t			datalen;			/* length of the data structure following */
	_Uint32t			zero[1];
	union {									/* if EXTRA_MOUNT_PATHNAME these set*/
		struct {							/* Sent from client to resmgr framework */
			struct _msg_info	info;		/* special info on first mount, path info on remount */
		}					cl;
		struct {							/* Server receives this structure filled in */
			void				*ocb;		/* OCB to the path (remount) or special (first) */
			void				*data;		/* Server specific data of len datalen */
			char				*type;		/* Character string with type information */
			char				*special;	/* Optional special device info */
			void				*zero[4];	/* Padding */
		}					srv;
	}					extra;
} io_mount_extra_t;


/***********************************************************************
 * IO message types which are fd based                                 *
 ***********************************************************************/

/*
 * Common header for combining io messages
 */
struct _io_combine {
	_Uint16t					type;
	_Uint16t					combine_len;
};

#define _IO_COMBINE_FLAG		0x8000		/* Ored with combine len to cause combine */


/*
 * Message of _IO_CLOSE
 */
struct _io_close {
	_Uint16t					type;
	_Uint16t					combine_len;
};

typedef union {
	struct _io_close			i;
} io_close_t;

/*
 * Message of _IO_WRITE
 */
struct _io_write {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Int32t						nbytes;
	_Uint32t					xtype;
	_Uint32t					zero;
/*	unsigned char				data[nbytes];	*/
};


typedef union {
	struct _io_write			i;
/*	nbytes is returned with MsgReply */
} io_write_t;

#define _IO_SET_WRITE_NBYTES(_c, _s)	_RESMGR_STATUS(_c, _s)

/*
 * Message of _IO_READ
 */
struct _io_read {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Int32t						nbytes;
	_Uint32t					xtype;
	_Uint32t					zero;
};

typedef union {
	struct _io_read				i;
/*	unsigned char				data[nbytes];	*/
/*	nbytes is returned with MsgReply */
} io_read_t;

#define _IO_SET_READ_NBYTES(_c, _s)	_RESMGR_STATUS(_c, _s)

/*
 * Message of _IO_STAT
 */
struct _io_stat {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Uint32t					zero;
};

typedef union {
	struct _io_stat				i;
	struct stat					o;
} io_stat_t;


/*
 * Message of _IO_NOTIFY
 */
struct _io_notify {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Int32t						action;
	_Int32t						flags;
	struct sigevent				event;

	/* Following fields only valid if (flags & _NOTIFY_COND_EXTEN) */
	_Int32t						mgr[2];    /* For use by manager */
	_Int32t						flags_extra_mask;
	_Int32t						flags_exten;
	_Int32t						nfds;
	_Int32t						fd_first;
	_Int32t						nfds_ready;
	_Int64t						timo;
/*	struct pollfd				fds[nfds]; */
};

struct _io_notify_reply {
	_Uint32t					zero;
	_Uint32t					flags;    /* action */

	/* Following fields only updated by new managers (if valid) */
	_Int32t						flags2;   /* flags above */
	struct sigevent				event;
	_Int32t						mgr[2];
	_Int32t						flags_extra_mask;
	_Int32t						flags_exten;
	_Int32t						nfds;
	_Int32t						fd_first;
	_Int32t						nfds_ready;
	_Int64t						timo;
/*	struct pollfd				fds[nfds]; */
};

typedef union {
	struct _io_notify			i;
	struct _io_notify_reply		o;
} io_notify_t;


/*
 * Message of _IO_DEVCTL
 */
struct _io_devctl {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Int32t						dcmd;
	_Int32t						nbytes;
	_Int32t						zero;
/*	char						data[nbytes]; */
};

struct _io_devctl_reply {
	_Uint32t					zero;
	_Int32t						ret_val;
	_Int32t						nbytes;
	_Int32t						zero2;
/*	char						data[nbytes]; */
    } ;

typedef union {
	struct _io_devctl			i;
	struct _io_devctl_reply		o;
} io_devctl_t;


/*
 * Message of _IO_PATHCONF
 */
struct _io_pathconf {
	_Uint16t					type;
	_Uint16t					combine_len;
	short						name;
	_Uint16t					zero;
};

typedef union {
	struct _io_pathconf			i;
/*	value is returned with MsgReply */
} io_pathconf_t;

#define _IO_SET_PATHCONF_VALUE(_c, _s)	_RESMGR_STATUS(_c, _s)


/*
 * Message of _IO_LSEEK
 */
struct _io_lseek {
	_Uint16t					type;
	_Uint16t					combine_len;
	short						whence;
	_Uint16t					zero;
	_Uint64t					offset;
};

typedef union {
	struct _io_lseek			i;
	_Uint64t					o;
} io_lseek_t;


/*
 * Message of _IO_CHMOD
 */
struct _io_chmod {
	_Uint16t					type;
	_Uint16t					combine_len;
	mode_t						mode;
};

typedef union {
	struct _io_chmod			i;
} io_chmod_t;


/*
 * Message of _IO_CHOWN
 */
struct _io_chown {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Int32t						gid;
	_Int32t						uid;
};

typedef union {
	struct _io_chown			i;
} io_chown_t;


/*
 * Message of _IO_UTIME
 */
struct _io_utime {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Int32t						cur_flag;		/*  If set, ignore times and set to "now"   */
	struct utimbuf				times;
};

typedef union {
	struct _io_utime			i;
} io_utime_t;


/*
 * Message of _IO_OPENFD
 */

enum _io_openfd_xtypes {
	_IO_OPENFD_NONE,
	_IO_OPENFD_PIPE,
	_IO_OPENFD_KQUEUE,
	_IO_OPENFD_ACCEPT,
	_IO_OPENFD_SCTP_PEELOFF
};

struct _io_openfd {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Uint32t					ioflag;			/* O_? in fcntl.h & _IO_FLAG_? */
	_Uint16t					sflag;			/* SH_? in share.h */
	_Uint16t					xtype;
	struct _msg_info			info;
	_Uint32t					reserved2;
	_Uint32t					key;
};

typedef union {
	struct _io_openfd			i;
} io_openfd_t;


/*
 * Message of _IO_FDINFO
 */
struct _io_fdinfo {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Uint32t					flags;
	_Int32t						path_len;
	_Uint32t					reserved;
};

struct _io_fdinfo_reply {
	_Uint32t					zero[2];
	struct _fdinfo				info;
/*	char						path[path_len + 1];	 */
};

typedef union {
	struct _io_fdinfo			i;
	struct _io_fdinfo_reply		o;
} io_fdinfo_t;

#define _IO_SET_FDINFO_LEN(_c, _s)	_RESMGR_STATUS(_c, _s)

/*
 * Message of _IO_LOCK
 */
struct _io_lock {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Uint32t					subtype;
	_Int32t						nbytes;
/*	char        				data[1];	*/		/* for F_*LK this will be flock_t */
};

struct _io_lock_reply {
	_Uint32t					zero[3];
/*	char        				data[1];	*/		/* for F_*LK this will be flock_t */
};

typedef union {
	struct _io_lock				i;
	struct _io_lock_reply		o;
} io_lock_t;


/*
 * Message of _IO_SPACE
 */
struct _io_space {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Uint16t					subtype;			/* F_ALLOCSP or F_FREESP */
	short						whence;
	_Uint64t					start;
	_Uint64t					len;				/* zero means to end of file */
};

typedef union {
	struct _io_space			i;
	_Uint64t					o;				/* file size */
} io_space_t;


/*
 * Message of _IO_SHUTDOWN
 */
struct _io_shutdown {
	_Uint16t					type;
	_Uint16t					combine_len;
};

struct _io_shutdown_reply {
	_Uint32t					zero;
};

typedef union {
	struct _io_shutdown			i;
	struct _io_shutdown_reply	o;
} io_shutdown_t;


/*
 * Message of _IO_MMAP
 */
struct _io_mmap {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Uint32t					prot;
	_Uint64t					offset;
	struct _msg_info			info;
	_Uint32t					zero[6];
};

struct _io_mmap_reply {
	_Uint32t					zero;
	_Uint32t					flags;
	_Uint64t					offset;
	_Int32t						coid;
	_Int32t						fd;
};

typedef union {
	struct _io_mmap				i;
	struct _io_mmap_reply		o;
} io_mmap_t;


/*
 * Message of _IO_DUP
 */
struct _io_dup {
	_Uint16t					type;
	_Uint16t					combine_len;
	struct _msg_info			info;
	_Uint32t					reserved;
	_Uint32t					key;
};

typedef union {
	struct _io_dup				i;
} io_dup_t;


/*
 * Message of _IO_MSG
 */
struct _io_msg {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Uint16t					mgrid;		/* manager id (sys/iomgr.h) */
	_Uint16t					subtype;	/* manager specific subtype */
};

typedef union {
	struct _io_msg				i;
} io_msg_t;


/*
 * Message of _IO_SYNC
 */
struct _io_sync {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Uint32t					flag;			/* O_?SYNC in fcntl.h */
};

typedef union {
	struct _io_sync				i;
} io_sync_t;


/*
 * Message of _IO_POWER
 */
struct _io_power {
	_Uint16t					type;
	_Uint16t					combine_len;
	_Uint32t					subtype;
	_Uint32t					flags;
	_Int32t						mode;
};

/* struct _io_power subtype */
enum _io_power_subtypes {
	_IO_POWER_GET,
	_IO_POWER_SET,
	_IO_POWER_MODES,
	_IO_POWER_MODEATTR
};

typedef union {
	struct _io_power			i;
	pm_power_attr_t				o;				/* for _IO_POWER_GET	*/
/*  pm_power_mode_t				modes[mode];       for _IO_POWER_MODES	*/
/*  pmd_mode_attr_t				modes[mode];       for _IO_POWER_MODES	*/
} io_power_t;

#include <_packpop.h>

__END_DECLS


#endif

/* __SRCVERSION("iomsg.h $Rev: 153052 $"); */
