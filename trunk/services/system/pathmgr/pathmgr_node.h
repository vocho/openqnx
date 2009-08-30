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

#ifndef PATHMGR_NODE_H
#define PATHMGR_NODE_H

#include <inttypes.h>
#include <kernel/proctypes.h>

#define INODE_XOR(value) ((int)value ^ 0x19071975)

struct node_entry {
	NODE						*parent;
	NODE						*sibling;
	NODE						*child;
	OBJECT						*object;
	uint16_t					links;			/* Number of current access to this node */
	uint16_t					child_objects;	/* Number of children with objects */
	uint16_t					flags;
	uint16_t					len;
	char						name[1];
};

#define NODE_FLAG_NOCHILD		0x0001			/* No children allowed on this node */
#define NODE_FLAG_NONSERVER		0x0002			/* There is a non-server object on this node */
#define NODE_FLAG_UNLINKING		0x0004			/* The node is being unlinked */
#define NODE_FLAG_DEFERRED		0x0008			/* The node has a deferred open */

#define PATHMGR_LOOKUP_ATTACH	0x00000001		/* increment link count if found */
#define PATHMGR_LOOKUP_CREATE	0x00000002		/* create entries if they don't exist */
#define PATHMGR_LOOKUP_PRUNE	0x00000004		/* only search first path entry */
#define PATHMGR_LOOKUP_NOSERVER	0x00000008		/* Stop at first non-server object */
#define PATHMGR_LOOKUP_NOEMPTIES	0x00000010		/* Avoid reporting empty objects if possible */
#define PATHMGR_LOOKUP_NOAUTO	0x00000020		/* Avoid reporting autocreated objects if possible */

extern pthread_mutex_t			pathmgr_mutex;

#endif

/* __SRCVERSION("pathmgr_node.h $Rev: 159046 $"); */
