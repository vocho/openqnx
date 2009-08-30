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

#include <stddef.h>
#include <malloc.h>
#include <string.h>
#include <pthread.h>
#include <kernel/nto.h>
#include "pathmgr_node.h"
#include "pathmgr_object.h"
#include "pathmgr_proto.h"

pthread_mutex_t		pathmgr_mutex = PTHREAD_MUTEX_INITIALIZER;

/*
 * Allocate a node entry, initializing it to zero with the name stuffed
 * This should only be called by pathmgr_find() and during pathmgr initialization.
 */
NODE *pathmgr_node_alloc(const char *name, unsigned len) {
	NODE						*nop;

	if((nop = _smalloc(offsetof(NODE, name) + len + 1))) {
		memset(nop, 0x00, offsetof(NODE, name));
		if((nop->len = len)) {
			(void) memccpy(nop->name, name, '\0', len + 1);
		}
		nop->name[len] = '\0';
	}
	return nop;
}

/*
 * This call is used to indicate that we are "reading" 
 * something associated with the node.  No external calls
 * (ie messages, anything which could result in a recursive
 * call back into a node accessing call) can be made between
 * the access and the complete calls.
 */
void pathmgr_node_access(NODE *node) {
	pthread_mutex_lock(&pathmgr_mutex);
}

/*
 * This call is used to indicate that we are finished 
 * doing something associated with the node.
 */
void pathmgr_node_complete(NODE *node) {
	pthread_mutex_unlock(&pathmgr_mutex);
}


/*
 * This call is used to indicate that we're making a copy
 * of a pointer to the node.
 */
NODE *pathmgr_node_clone(NODE *node) {

	pthread_mutex_lock(&pathmgr_mutex);
	CRASHCHECK(node->links == 0);

	++node->links;
	pthread_mutex_unlock(&pathmgr_mutex);
	return node;
}

/*
 * This function will find the matching node for a path. It ignores
 * multiple slashes. If "object" is passed in, the node will be created
 * if it doesn't exist. It will also be linked to the current objects
 * on that node.
 */
NODE *pathmgr_node_lookup(register NODE *nop, register const char *path, unsigned flags, const char **result) {
	register NODE					*n;
	register NODE					*lastnode;
	register const char				*lasttail;

	/* Pick a sensible default */
	if(!nop) {
		nop = sysmgr_prp->root;
	}

	/* Grab the mutex here */
	pthread_mutex_lock(&pathmgr_mutex);
	lastnode = NULL;
	lasttail = NULL;
	while(*path) {
		unsigned						len;

		/* Skip any multiple slashes */
		for(len = 0; path[len] && path[len] != '/'; len++) {
			/* nothing to do */
		}

		/* Search current level for a match */
		for(n = nop->child; n; n = n->sibling) {
			if(len == n->len && !memcmp(path, n->name, len)) {
				break;
			}
		}


		if(!n) {
			/* No match found */
			if(flags & PATHMGR_LOOKUP_CREATE) {
				/* If we are linking create a node */
				if((n = pathmgr_node_alloc(path, len))) {
					/* Link it to the parent */
					n->sibling = nop->child;
					n->parent = nop;
					nop->child = n;
				} else {
					/* If no memory clean up */
					nop->links++;
					pthread_mutex_unlock(&pathmgr_mutex);
					pathmgr_node_detach(nop);
					pthread_mutex_lock(&pathmgr_mutex);
					nop = 0;
					break;
				}
			} else if((flags & PATHMGR_LOOKUP_NOSERVER) || !result) {
				/* If a result was asked for, then return node, and the remaining path */
				nop = 0;
				break;
			} else {
				break;
			}
		}

		/*If we don't want any empties, and this is an empty node, then hope that
		  there is a fallback node that we can connect with in the lastnode.  */
		if ((flags & PATHMGR_LOOKUP_NOEMPTIES) && !n->object && n->child_objects == 0) {
			nop = 0;
			break;
		}

		/* Found matching node */
		nop = n;
		path += len;

		/* Skip slash if present */
		if(*path) {
			path++;
		}

		/* Do not recurse the children */
		if(flags & PATHMGR_LOOKUP_PRUNE) {
			break;
		}

		/* Break out if node has non-server objects */
		if(flags & PATHMGR_LOOKUP_NOSERVER) {
			OBJECT					*object;

			if((object = nop->object)) {
				/* We should have an option for shortest/longest pathname matches 
				   shortest match would break at this point, longest one would continue */
				if(object->hdr.type != OBJECT_NONE && object->hdr.type != OBJECT_SERVER) {
					lasttail = path;
					lastnode = nop;
				}
			}
		}
	}

	/* If we don't have any objects (ie the node was autocreated), then 
	   fall back and use the last node that we think was valid */
	if ((flags & PATHMGR_LOOKUP_NOAUTO) && nop && !nop->object) {
		nop = 0;
		/* Fall through to the next test */
	}

	/* If we don't have a node, and there is a last node then use it */
	if (!nop && lastnode) {
		nop = lastnode;
		path = lasttail;
	}

	/* Store the remaining path if requested */
	if(result) {
		*result = path;
	}

	/* Increment link count */
	if(nop && (flags & PATHMGR_LOOKUP_ATTACH)) {
		nop->links++;
	}

	/* Free the mutex */
	pthread_mutex_unlock(&pathmgr_mutex);

	return nop;
}


/*
 * This will lower the link count on the node. If the links are
 * zero, it will remove the node, and all parents with
 * a link count of zero.
 */
void pathmgr_node_detach_flags(NODE *nop, unsigned flags) {
	/* Protect the node list */
	pthread_mutex_lock(&pathmgr_mutex);

	nop->flags &= ~flags;
	if(nop->links == 0 || --nop->links == 0) {
		CRASHCHECK(nop->object != NULL);

		/* Remove the node */
		while(nop->links == 0 && !nop->child) {
			NODE				*p, **pp;

			CRASHCHECK(nop->parent == NULL);
			/* Unlink from parent or siblings */
			for(pp = &nop->parent->child; (p = *pp); pp = &p->sibling) {
				if(p == nop) {
					*pp = p->sibling;
					break;
				}
			}
			CRASHCHECK(p == NULL);

			/* Rember the parent */
			p = nop->parent;
			
			_sfree(nop, offsetof(NODE, name) + nop->len + 1);

			/* Loop to see if parent also has no children or links */
			nop = p;
		}
	}
	pthread_mutex_unlock(&pathmgr_mutex);
}


void pathmgr_node_detach(NODE *nop) {
	pathmgr_node_detach_flags(nop, 0);
}

__SRCVERSION("pathmgr_node.c $Rev: 159046 $");
