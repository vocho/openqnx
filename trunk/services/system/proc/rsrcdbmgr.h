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
 This are internal things for the resource manager
*/
#ifndef __RSRCDBMGR_PRIVATE_H_INCLUDED
#define __RSRCDBMGR_PRIVATE_H_INCLUDED

#include <pthread.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/rsrcdbmsg.h>
#include <sys/rsrcdbmgr.h>
#include "externs.h"

/*
 Resource Request Commands
 Even #'s so we can take advantage of the RSRC_USED flag
*/
#define RSRCDB_REQ_MASK			0xfe00
enum _rsrc_req_private {
	RSRCDBMGR_REQ_COUNT   = (12 << 8)
};

/*
 Private Resource Types
*/
#define	RSRCDBMGR_MINOR_NUM	 RSRCDBMGR_TYPE_COUNT

/*
 This generic block is a construct which is used so that we
can loop through either create blocks or resource blocks in
a generic fashion.  This layout (start, end, next) should be
at the head of all of the blocks.
*/
#define GENERIC_BLOCK(nexttype, othertype, othermember) \
				uint64_t	start;	\
				uint64_t	end;	\
				nexttype	*next;	\
				othertype	*othermember; 
typedef struct _generic_block {
	GENERIC_BLOCK(struct _generic_block, void, unused)
} generic_block_t;

/*
These blocks are maintained in an active (ie free and used) 
resource listing. We intermingle free blocks with used block 
which are cross threaded back to the process they came from.  

This way we can easily query the free list, the taken list, 
a pid's list or the whole list of resources.  The downside
is that the lookup is more complicated.
*/
typedef struct _rsrc_block {
	GENERIC_BLOCK(struct _rsrc_block, struct _rsrc_block, prev)
	/*
	struct _rsrc_block	**prevtaken, *nexttaken;
	*/
	uint32_t			flags;
	pid_t				owner;
} rsrc_block_t;

/*
These blocks are maintained in a created resource listing.
We maintain a linear list of these blocks sorted by start
point which makes priority queries easier.
*/
typedef struct _rsrc_create {
	GENERIC_BLOCK(struct _rsrc_create, struct _rsrc_node, node)
	pid_t				creator;	
	uint16_t			flags;
	uint16_t			zero;	
} rsrc_create_t;

/*
This is the structure of a node, it contains the priority (ie the level
in the tree) that it was created at for convenient access.  The actual
data allocated components point back to this node.
*/
typedef struct _rsrc_node {
	struct _rsrc_node	*sibling;	//Root node uses this as a next pointer
	struct _rsrc_node	*child;
	uint8_t				 zero[2]; 
	uint8_t				 priority; 
	char				 name[1];	//Null terminated name allocated at creation
} rsrc_node_t;

typedef struct _rsrc_root_node {
	rsrc_block_t		*active;
	rsrc_create_t		*created;
	union {
		struct _rsrc_root_node	*_next;
		rsrc_node_t				_head;		
	} data;
#define next_root data._next
#define head_node data._head
} rsrc_root_node_t;

/*
 MAJOR/MINOR Resource data structures
*/
#define MAX_NUM_MINORS		1024				/* 2^10 from sys/types.h */
#define MAX_NUM_MAJORS		64					/* 2^6  from sys/types.h */

/* 
 This structure contains the minor number mappings.
*/
typedef struct _rsrc_devno {				/* Major # is index to an array of these */
	char	*name;							/* Major #'s name */
	struct _devno_minors {
		uint8_t		count;					/* Number of valid elements in the array */
		uint8_t		bits[1];				/* Bits indicating which minors are taken */
	} *minors;
} rsrc_devno_t;

/*
 This is the per-process data structure which tracks resource usage. 
 TERRIBLE NAME: Rename this later
*/
typedef struct _rsrc_list_array {
	uint16_t			haversrc;				/* Indicates that resources were accessed */
	uint16_t			dev_count;				/* Number of elements in the dev_t array */
	dev_t				*dev_list;				/* Array of dev_t's used by this process */
} rsrc_list_array_t;

/*
 Function prototypes and globals for the resource tracking system.
*/
extern pthread_mutex_t		g_rsrc_mutex;
//extern rsrc_devno_array_t	g_rsrc_devno;	
extern rsrc_root_node_t		*g_rsrc_root;

/* Prototypes */
int _rsrc_minor(const char *name, int *major, int *minor_request, int flags);

int _rsrcdbmgr_create(rsrc_alloc_t *rsrc, pid_t pid);
int _rsrcdbmgr_destroy(rsrc_alloc_t *rsrc, pid_t pid);
int _rsrcdbmgr_find(rsrc_request_t *rsrc, pid_t pid, int *priority);
int _rsrcdbmgr_remove(rsrc_request_t *rsrc, pid_t pid, int *priority);
int _rsrcdbmgr_attach(rsrc_request_t *rsrc, pid_t pid);
int _rsrcdbmgr_detach(rsrc_request_t *rsrc, pid_t pid);

int rsrc_find_range(rsrc_root_node_t *root, uint64_t length, 
					uint64_t *start, uint64_t *end, 
					uint64_t alignment, pid_t owner, 
					unsigned flags, int *priority);

int _rsrcdbmgr_pid_clear(pid_t pid);
int _rsrcdbmgr_pid_mark(pid_t pid);

#define FLAG_QUERY_NEXT     0x1
#define FLAG_QUERY_COUNT    0x2
#define FLAG_QUERY_CREATE   0x4
int _rsrcdbmgr_query(const char *name, pid_t pid, unsigned flags, int start, 
						rsrc_alloc_t *array, uint32_t nbytes, char **newname);

void dump_list(char *name);

/* Not for general consumption */
int rsrcmgr_handle_minor(resmgr_context_t *ctp, rsrc_cmd_t *msg, rsrc_minor_request_t *data);

#endif

/* __SRCVERSION("rsrcdbmgr.h $Rev: 153052 $"); */
