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
*  sys/rsrcdbmsg.h
*

*/

#ifndef __RSRCDBMSG_H_INCLUDED
#define __RSRCDBMSG_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __SYSMSG_H_INCLUDED
#include _NTO_HDR_(sys/sysmsg.h)
#endif

#define RSRCDBMGR_COID  SYSMGR_COID

enum _rsrc_command {
	RSRCDBMGR_RSRC_CMD	= _RSRCDBMGR_BASE
};

/*
 Resource Request Commands
*/
#define RSRCDBMGR_REQ_MASK	0xfe00
enum _rsrc_req {
	RSRCDBMGR_REQ_CREATE	= (2 << 8),
	RSRCDBMGR_REQ_DESTROY	= (4 << 8),
	RSRCDBMGR_REQ_ATTACH	= (6 << 8),
	RSRCDBMGR_REQ_DETACH	= (8 << 8),
	RSRCDBMGR_REQ_QUERY		= (10 << 8),
	RSRCDBMGR_RESERVED   	= (12 << 8),
	RSRCDBMGR_REQ_QUERY_NAME= (14 << 8)
};


/*
 Structures used to communicate w/ server. 
*/
struct _rsrc_cmd {
	_Uint16t		type;
	_Uint16t		subtype;
	_Uint32t		pid;
	_Uint32t		nbytes;			/* Size of the message to follow  */
	_Uint32t		count;			/* Number of elements in the message */
	_Uint32t		index;			/* Used for query operation, start index */
};

struct _rsrc_cmd_reply {
	_Uint8t			res[sizeof(struct _rsrc_cmd)];
};

typedef union {
	struct _rsrc_cmd		i;
	struct _rsrc_cmd_reply	o;
} rsrc_cmd_t;

#endif

/* __SRCVERSION("rsrcdbmsg.h $Rev: 153052 $"); */
