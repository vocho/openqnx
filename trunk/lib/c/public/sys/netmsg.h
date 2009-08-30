/*
 * $QNXLicenseC: $
*/


#ifndef _NETMSG_H_INCLUDED
#define _NETMSG_H_INCLUDED

#ifndef __IOMSG_H_INCLUDED
#include <sys/iomsg.h>
#endif

#ifndef __IOMGR_H_INCLUDED
#include <sys/iomgr.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

enum _netmgr_subtypes {
	_NETMGR_CTL,
	_NETMGR_REMOTE_ND,
	_NETMGR_STRTOND,
	_NETMGR_NDTOSTR,
	_NETMGR_QOS_BASE = 0x0100,
	_NETMGR_QOS_FLUSH = _NETMGR_QOS_BASE,
	_NETMGR_QOS_MAX
};

/*
 * _NETMGR_CTL
 */
struct _netmgr_ctl {
	struct _io_msg			hdr;
	_Uint32t				nd;
	_Int32t					op;
};

typedef union {
	struct _netmgr_ctl			i;
} netmgr_ctl_t;


/*
 * _NETMGR_REMOTE_ND
 */
struct _netmgr_remote_nd {
	struct _io_msg			hdr;
	_Uint32t				remote_nd;
	_Uint32t				local_nd;
};

typedef union {
	struct _netmgr_remote_nd	i;
} netmgr_remote_nd_t;


/*
 * _NETMGR_STRTOND
 */
struct _netmgr_strtond {
	struct _io_msg			hdr;
	_Uint32t				len;
	_Uint32t				zero;
/*	char					path[len];	*/
};

struct _netmgr_strtond_reply {
	_Uint32t				zero;
	_Uint32t				len;
	_Uint32t				nd;
};

typedef union {
	struct _netmgr_strtond			i;
	struct _netmgr_strtond_reply	o;
} netmgr_strtond_t;


/*
 * _NETMGR_NDTOSTR
 */
struct _netmgr_ndtostr {
	struct _io_msg			hdr;
	_Uint32t				len;
	_Uint32t				flags;
	_Uint32t				nd;
};

typedef union {
	struct _netmgr_ndtostr			i;
} netmgr_ndtostr_t;
	
#endif
