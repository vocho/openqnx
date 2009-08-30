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
 This file contains the main interpretive interface to the resource database
 layer from outside to in.
*/

/*****/
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

#include "rsrcdbmgr.h"

#define CONTAINSE(s1, e1, p)	 ((s1) <= (p) && (p) <= (e1))
#define OVERLAPE(s1, e1, s2, e2) (CONTAINSE((s1), (e1), (s2)) || \
								  CONTAINSE((s2), (e2), (s1)))

/***
 Allocation Functions
***/
rsrc_create_t *rsrc_create_allocate() {
	return calloc(1, sizeof(rsrc_create_t));
}
void rsrc_create_free(rsrc_create_t *create) {
	free(create);
}

rsrc_block_t *rsrc_block_allocate() {
	return calloc(1, sizeof(rsrc_block_t));
}
void rsrc_block_free(rsrc_block_t *blockp) {
	free(blockp);
}

/***
 Searching Functions 
***/
/*
 This will return the root node for a given class.  If NULL is passed
 in then it will return the head of the list of roots.
*/
rsrc_root_node_t *rsrc_root_locate(const char *name, int create) {
	rsrc_root_node_t *root;
	int				  len;

	if(!name) {
		return g_rsrc_root;
	}

	for(len=0; name[len] && name[len] != '/'; len++) { ; }

	for(root = g_rsrc_root; root; root = root->next_root) {
		if(strncmp(root->head_node.name, name, len) == 0 &&
		   strlen(root->head_node.name) == len) {
			break;
		}
	}

	if(!root && create) {
		//TODO: Replace with a pool call?
		if(!(root = calloc(1, sizeof(*root) + len))) {
			errno = ENOMEM;
			return NULL;
		}
		memcpy(root->head_node.name, name, len);
		root->next_root = g_rsrc_root;
		g_rsrc_root = root;
	}

	if(!root) {
		errno = EINVAL;
	}

	return root;
}

/*
 This will return the node which is responsible for this name
*/
rsrc_node_t *rsrc_node_locate(const char *name, int create) {
	rsrc_root_node_t *root;
	rsrc_node_t		 *parent, *node;
	const char       *start, *end;
	int				  level;
	uint8_t			  len;

	if(!(root = rsrc_root_locate(name, create))) {
		return NULL;
	}
	node = NULL;

	//Move to the first "/", if it isn't there return w/ root
	end = start = name;
	while(*start && *start != '/') { start++; }
	while(*start && *start == '/') { start++; }

	if(!*start) {
		return &root->head_node;
	}

	for(level=1, parent=&root->head_node; parent; level++, parent=node) {
		//Eat any extra slashes in the name, then find the end 
		while(*start && *start == '/') { start++; }
		end = start;
		while(*end && *end != '/') { end++; }

		//No more name to process, return this node
		if((len = end - start) == 0) {
			break;
		}

		for(node = parent->child; node; node = node->sibling) {
			if(strncmp(node->name, start, len) == 0 && strlen(node->name) == len) {
				break;
			}
		}

		//Create ourselves a node if that is required
		if(!node && create && (node = calloc(1, sizeof(*node) + len))) {
			memcpy(node->name, start, len);
			node->priority = level;
			node->sibling = parent->child;
			parent->child = node;
		} else {
			errno = (!node && create) ? ENOMEM : EINVAL;
		}

		start = end;
	}

	return node;
}


/*
 Return the priority information about this range (max/?min?)
*/
static int16_t find_range_priority(rsrc_root_node_t *root, 
								   uint64_t start, uint64_t end, 
								   unsigned flags) {
	rsrc_create_t *create;
	int16_t priority;

	priority = 0;
	for (create = root->created; create; create = create->next) {
		if (create->start > end) {
			break;
		}

		if ((create->start <= start && start <= create->end) || 
		    (start <= create->start && create->start <= end)) {
			//HACK: We need to support the RSVP flags in some way
			priority = __max(create->node->priority + (create->flags & RSRCDBMGR_FLAG_RSVP) ? 1 : 0, priority);
		}
	}

	return(priority);
}

/* 
 Find the priority of this block.  Then find the next lowest priority
 block that starts before (backward = 1) or starts after (backwards = 0).
 We then stuff the appropriate end point (backward = 1) or startpoint
 (backwards = 0) in value when we find it.  If the priority is 0, then
 we stuff it with the start/end values respectively
*/
static int find_next_priority(rsrc_root_node_t *root,
							  uint64_t start, uint64_t end, 
							  int backward, uint64_t *value) {
	rsrc_create_t *create;
	uint16_t curpriority, priority;

	
	curpriority = find_range_priority(root, start, end, 0);
	*value = (backward) ? ((start == 0) ? 0 : start-1)
                        : ((end == ULONGLONG_MAX) ? ULONGLONG_MAX : end+1);

	if(curpriority == 0) {
		return (backward) ? (*value != end) : (*value != start);
	}

	priority = 0;
	for(create = root->created; create; create = create->next) {
		if(create->start > end) {
			break;
		}

		if((create->start <= start && start <= create->end) || 
		   (start <= create->start && create->start <= end)) {
			//HACK: Support the RSVP flag
			uint16_t nodeprio;

			nodeprio = create->node->priority + (create->flags & RSRCDBMGR_FLAG_RSVP) ? 1 : 0;
			priority = __max(nodeprio, priority);

			if(priority >= curpriority && nodeprio >= curpriority) {
				*value = (backward) ? __min(*value, create->start-1) 
                                    : __max(*value, create->end+1);
			}
		}
	}

	return (backward) ? (*value != end) : (*value != start);
}

/*
 Find the lowest priority resource based on a set of parameters.
 Optional parameters, other than length which is required, include:
 - The block must have a certain alignment (default 1)
 - The search should continue from the top of the range down (default up)
 - The block may be sharable (default not shared)

 TODO: There is a limitation with this code in that it will only fit
       shared blocks inside a totally free range or inside an exising
	   share block which is greater than the request.  This means that
	   a request in the following situation will fail:
	   5-10 -> 11s-15s -> 16-20
	   A share request similar to 10-11 or 15-16 would fail, even though
	   technically it should succeed.  This is a very infrequent case
	   however since most shares contain the same range, but should be
	   addressed at some point in the future.
*/
#define BLOCK_CONTAINS_PT(blk, pt)	((blk)->start <= (pt) && (pt) <= (blk)->end)
#define RSRC_FIND_BACK	RSRCDBMGR_FLAG_TOPDOWN
#define RSRC_FIND_SHARE	RSRCDBMGR_FLAG_SHARE

#define BLOCK_SHARE_COMPATABLE(blk, flags, owner)  \
		(((blk)->owner == owner) || \
		 ((blk)->owner && (blk)->flags & RSRC_FIND_SHARE && \
		  (flags) & RSRC_FIND_SHARE))

int rsrc_find_range(rsrc_root_node_t *root, uint64_t length, 
					uint64_t *start, uint64_t *end, 
					uint64_t alignment, pid_t owner,
					unsigned flags, int *priority) {
	uint64_t		limit_start, limit_end;
	uint64_t		tmp_start, tmp_end, extraamount;
	uint64_t		savedstart, savedend;
	rsrc_block_t	*curblock, *tmpblock;	
	int16_t			savedprio, curprio;
	uint8_t			forward;


	//Save the limiting start and end points
	limit_end = *end;
	limit_start = *start;

	//Indicate that the savedrange is not valid
	savedstart = savedend = 0;
	savedprio = -1;

	//Where are we looking forward or back?
	forward = (flags & RSRC_FIND_BACK) ? 0 : 1;

	/* Get us to a point where we can start looking for blocks */
	tmpblock = NULL;
	for(curblock = root->active; curblock; curblock = curblock->next) {
		if(forward && (BLOCK_CONTAINS_PT(curblock, limit_start) || limit_start < curblock->start)) {
			tmpblock = curblock;
			break;
		} else if(!forward && (limit_end >= curblock->start)) {
			tmpblock = curblock;
		} else if(!forward) {
			break;
		}
	}

	if(!(curblock = tmpblock)) {
		return -1;
	}

	/* While we have a valid range iterate through the possibilities */
	while(curblock && curblock->start <= limit_end) {
		/* Determine the real range that we have to work with */
		tmp_start = __max(curblock->start, limit_start);
		tmp_end = __min(curblock->end, limit_end);

restart:

		/* Look for something that is valid and might fit */
		if(BLOCK_SHARE_COMPATABLE(curblock, flags, owner) && 
			tmp_start <= tmp_end && length <= ((tmp_end - tmp_start) + 1)) {

			tmp_start = (forward) ? tmp_start : tmp_end - (length - 1);

			/* Attempt to fit ourselves to the aligment we need */
			if((extraamount = (tmp_start % alignment)) != 0) {
				tmp_start += (forward) ? alignment - extraamount : -extraamount;
			}

			tmp_end = tmp_start + (length - 1);

			/* Re-check the limits after a possible alignment adjust */
			if(/*extraamount && We iterate below ... so always check */
			   (tmp_start < __max(curblock->start, limit_start) || 
			    tmp_end   > __min(curblock->end, limit_end))) {
				;		//Invalid block ... just continue
			} else { 
				/* Check range priority against saved block */
				curprio = find_range_priority(root, tmp_start, tmp_end, 0);	
				if(curprio == 0 || savedprio == -1 || curprio < savedprio) {
					savedstart = tmp_start;
					savedend = tmp_end;
					savedprio = curprio;
					if(curprio == 0) {
						break;
					}
				} 

				/* Find the next possible starting block based on the priority */
				if(find_next_priority(root, tmp_start, tmp_end, !forward, &extraamount)) {
					if(forward) {
						tmp_start = extraamount;
						tmp_end =  __min(curblock->end, limit_end);
					} else {
						tmp_start = __max(curblock->start, limit_start);
						tmp_end = extraamount;
					}
					goto restart;
				}
			}
		}

		curblock = (forward) ? curblock->next : curblock->prev;
	}
		
	/* Check and see if what was found was better than what was saved */
	if(savedprio != -1) {
		*start = savedstart;
		*end = savedend;
		if(priority) {
			*priority = savedprio;
		}
		return EOK;
	}

	errno = ERANGE;	
	return -1;
}

/***
 Insertion Functions
***/
#if 0
/*
 Verifies that a block list is in sorted order.
 */
void sort_check (rsrc_block_t *head)
{
	rsrc_block_t	*curblock;
	uint64_t		last;

	last = 0;
	for(curblock = head; curblock; curblock = curblock->next) {
		if (last > curblock->start) {
			kprintf("Out of order\n");
		}
		last = curblock->start;
	}
}
#endif

rsrc_create_t *rsrc_insert_create(rsrc_create_t **head, rsrc_create_t *create) {
	rsrc_create_t **prev;

	prev = head;	
	while(*prev && (*prev)->start <= create->start) {
		prev = &((*prev)->next);
	}
	create->next = *prev;
	*prev = create;

	return create;
}

/*
 Insert a new block (start, end) into the list before/after curblk.
*/
rsrc_block_t *rsrc_insert_block(rsrc_block_t **head, rsrc_block_t *curblk, 
								uint64_t start, uint64_t end, 
								pid_t owner, unsigned flags, int after) {
	rsrc_block_t *newblk;

	if (!(newblk = rsrc_block_allocate())) {
		return NULL;
	}
	newblk->start = start;
	newblk->end = end;
	newblk->owner = owner;
	newblk->flags = flags;

	//We are the first block in the list
	if(!curblk || (!after && !curblk->prev)) {
		newblk->prev = NULL;
		if((newblk->next = *head)) {
			newblk->next->prev = newblk;
		}
		*head = newblk;
	} else {
		//newblk->prev should alway be set to something 
		newblk->prev = (after) ? curblk : curblk->prev;
		if((newblk->next = newblk->prev->next)) {
			newblk->next->prev = newblk;
		}
		newblk->prev->next = newblk;
	}

	// Make sure the list is still in sorted order
	//sort_check (*head);

	return newblk;
}

/*
 Add a new allocation block into the single linked list of existing
 allocated blocks. 
 Then add the intersection of this block and existing blocks into
 the active pool of resources (make this another function?).
*/

//Block are compatible (ie can be joined) if they have the same owner and if
//they have the same share flags (ie can be shared, or can't be shared)
/*
#define JOIN_COMPATIBLE(blk, pid, flags)	((blk)->owner == (pid) && \
										    ((blk)->flags & RSRCDBMGR_FLAG_SHARE) == (flags & RSRCDBMGR_FLAG_SHARE))
*/
/* Shared blocks should always get their own blocks, even if they overlap ? */
#define JOIN_COMPATIBLE(blk, pid, flags)	((blk)->owner == (pid) && \
										    !((blk)->flags & RSRCDBMGR_FLAG_SHARE) && !(flags & RSRCDBMGR_FLAG_SHARE))

/*TODO:
  We need to roll these block_add_range* functions into one!
*/
int rsrc_block_add_range_pid(rsrc_root_node_t *root, 
							 uint64_t newstart, uint64_t newend, pid_t pid, unsigned flags) {
	rsrc_block_t	*curblk, *absorb;

	absorb = NULL;
	//TODO: Find an early out condition here
	for(curblk = root->active; curblk; curblk = curblk->next)
	{
		if(newstart <= curblk->end + 1) {
			uint8_t overlaps, canjoin;

			overlaps = OVERLAPE((newstart != 0) ? newstart-1 : newstart, 
							    (newend != ULONGLONG_MAX) ? newend+1 : newend, 
								curblk->start, curblk->end);
			canjoin = JOIN_COMPATIBLE(curblk, pid, flags);

			/* If we start before this current block then create a new block which
			   will then absorb in any overlapping blocks which arise in the future */
			if(!absorb && newstart < curblk->start) {
				absorb = rsrc_insert_block(&root->active, curblk, newstart, newend, pid, flags, 0);
				/* TODO: If we are maintaining a taken list, then we need to make
				         sure that things are properly marked here.
				*/
			}

			/* Absorb or extend this block as required */
			if(canjoin && overlaps) {
				newstart = __min(newstart, curblk->start);
				newend = __max(newend, curblk->end);
				if(!absorb) {
					absorb = curblk;
				} else {
					if((absorb->next = curblk->next)) {
						absorb->next->prev = absorb;
					}
					rsrc_block_free(curblk);
					curblk = absorb;
				}
				absorb->start = newstart;
				absorb->end = newend;
			}
		}

		if(!curblk->next) {
			break;
		}
	}

	//If we still have an outstanding block, create it at the end of the list
	if(!absorb && newstart <= newend) {
		absorb = rsrc_insert_block(&root->active, curblk, newstart, newend, pid, flags, 1);
	}

	return 0;
}

/*
 This function should have no intelligence other than determining that you want to
 insert a block in a "start point first" scenario or that you want to insert a block
 to be merged with other like minded blocks.  Currently this routine does a lot of
 extra work to limit ranges and the like.  This should be done by an external 
 function.
*/
int rsrc_block_add_range(rsrc_root_node_t *root, 
						 uint64_t newstart, uint64_t newend, pid_t pid, unsigned flags) {
	rsrc_block_t	*curblk, *absorb;

	/* Determine the overlap with existing blocks and add the missing bits */
	absorb = NULL;
	for(curblk = root->active; curblk && newstart <= newend; curblk = curblk->next) {
		uint8_t overlaps;

		//Skip blocks that aren't even close to us, 
		if(curblk->end + 1 < newstart) {
			if(!curblk->next) {
				break;
			}
			continue;
		}

		//Create a new block if it ends before the next block
		if(newend + 1 < curblk->start) {
			if(!absorb) {
				absorb = rsrc_insert_block(&root->active, curblk, newstart, newend, pid, flags, 0);
			}
			newstart = newend + 1;
			break;
		}

		overlaps = OVERLAPE((newstart != 0) ? newstart-1 : newstart, 
						    (newend != ULONGLONG_MAX) ? newend+1 : newend, 
							curblk->start, curblk->end);

		//If the next block has an owner and we overlap then create a new block before 
		if(curblk->owner && overlaps) {
			if(newstart < curblk->start) {
				if(absorb) {
					absorb->end = curblk->start-1;
				} else {
					absorb = rsrc_insert_block(&root->active, curblk, newstart, curblk->start-1, pid, flags, 0);
				}
			}
			if(curblk->end >= newend) {		//Handle ULONGLONG_MAX case
				break;
			}
			newstart = curblk->end + 1;
			absorb = NULL;					
		} else if(overlaps) {
			newstart = __min(newstart, curblk->start);
			newend = __max(newend, curblk->end);
			if(!absorb) {
				absorb = curblk;
			} else {
				if((absorb->next = curblk->next)) {
					absorb->next->prev = absorb;
				}
				rsrc_block_free(curblk);
				curblk = absorb;
			}
			absorb->start = newstart;
			absorb->end = newend;
		} else {
			//I don't know what condition got us here?
			kprintf("WUZZUP!\n");
		}

		if(!curblk->next) {
			break;
		}
	}

	//If we still have an outstanding block, create it here
	if(!absorb && newstart <= newend) {
		absorb = rsrc_insert_block(&root->active, curblk, newstart, newend, pid, flags, 1);
	}

	return 0;
}

/*
 Options for removal are:
 1- Just delete the blocks owned by pid in this range (pid != -1)
 2- Just delete the blocks not owned in this range (pid == 0)
*/
int rsrc_block_remove_range(rsrc_root_node_t *root, uint64_t newstart, uint64_t newend, pid_t pid) {
	rsrc_block_t	*target, **curblk;

	/* Determine the overlap with existing blocks and add the missing bits */
	curblk = &root->active;
	while((target = *curblk) && target->start <= newend) {
		if(target->owner != pid || !(OVERLAPE(newstart, newend, target->start, target->end))) {
			curblk = &(*curblk)->next;
			continue;
		}

		//Remove entire block, split the block or shrink block
		if(newstart <= target->start && target->end <= newend) {
			if(target->next) {
				target->next->prev = target->prev;
			}
			*curblk = target->next;
			rsrc_block_free(target);
		} else if(target->start < newstart && newend < target->end) {
			if(!(target = rsrc_block_allocate())) {
				return ENOMEM;
			}
			//Duplicate this block and insert w/ adjusted end point
			memcpy(target, *curblk, sizeof(*target));
			target->next = *curblk;
			target->next->prev = target;
			
			/*
			TODO: If the block is owned/shared then we need
			      to do some extra fiddling.
			if(target->owner) {
				target->prevtaken = (*curblk)->prevtaken;
				target->nexttaken = *curblk;
				(*curblk)->prevtaken = &target->nexttaken;
				*target->prevtaken = target;
			}
			*/

			*curblk = target;		//target->prev->next

			target->end = newstart-1;
			target->next->start = newend + 1;
			//Advance the pointer to the next,next block
			curblk = &target->next->next;
		} else {
			target->start = (newstart <= target->start) ? newend + 1 : target->start; 
			target->end = (newend >= target->end) ? newstart - 1 : target->end; 
			curblk = &target->next;
		}
	}
	return 0;
}

/***
 Middle Layer Helper Functions 
***/
int _remove_callout(rsrc_root_node_t *root, uint64_t start, uint64_t end, void *data) {
	return rsrc_block_remove_range(root, start, end, 0);
}

int _add_callout(rsrc_root_node_t *root, uint64_t start, uint64_t end, void *data) {
	return rsrc_block_add_range(root, start, end, 0, 0);
}

/*
 This function is used when you want to do some sort of limiting on a range
 and apply a function to that range (ie add/remove etc).
*/
int _rsrc_block_limit_op(rsrc_root_node_t *root, uint64_t start, uint64_t end,
                        void *odata, 
						int (* overlap)(rsrc_root_node_t *root, uint64_t start, uint64_t end, void *data),
                        void *nodata, 
						int (* nonoverlap)(rsrc_root_node_t *root, uint64_t start, uint64_t end, void *data)) {
	rsrc_create_t *tmpinfo;
	uint64_t	  overlapstart, overlapend;
	uint64_t	  purgestart, purgeend;
    int           ret, overlapvalid;

	overlapstart = overlapend = 0;
    purgestart = start;
    purgeend = end;
    overlapvalid = 0;
	ret = EOK;

    for (tmpinfo = root->created; tmpinfo && purgestart <= purgeend; tmpinfo = tmpinfo->next) {
        //If this block doesn't overlap, then don't bother looking at it
        if (!OVERLAPE(tmpinfo->start, tmpinfo->end, purgestart, purgeend)) {
            continue;
        }

        /* If this info block overlaps and we have previous overlap, callout */
        if (overlapvalid && !OVERLAPE(tmpinfo->start, tmpinfo->end, overlapstart, overlapend)) {
            if (overlap) {
                ret = overlap(root, overlapstart, overlapend, odata);
            }
            overlapvalid = 0;
        }

        if (!overlapvalid) {
            overlapstart = __max(tmpinfo->start, purgestart);
            overlapend = __min(tmpinfo->end, purgeend);
        }
        else {
            overlapend = __max(overlapend, __min(purgeend, tmpinfo->end));
        }
        overlapvalid = 1;

        //If we start before the purge block we move our start point forward
        if (tmpinfo->start <= purgestart) {
            purgestart = (tmpinfo->end != ULONGLONG_MAX) ? tmpinfo->end+1 : ULONGLONG_MAX;
        }
        //If we start after the purge block, we should purge the range and continue
        else {
            if (nonoverlap) {
                ret = nonoverlap(root, purgestart, (tmpinfo->start) ? tmpinfo->start-1 : 0, nodata);
            }
            purgestart = (tmpinfo->end != ULONGLONG_MAX) ? tmpinfo->end+1 : ULONGLONG_MAX;
        }
    }

	if (overlapvalid && overlap) {
        ret = overlap(root, overlapstart, overlapend, odata);
    }

    if (purgestart <= purgeend && nonoverlap) {
        ret = nonoverlap(root, purgestart, purgeend, nodata);
    }

    return ret;
}


/* Determine if this node is a child of the parent */
static int _rsrc_is_child(rsrc_node_t *parent, rsrc_node_t *target) {
	rsrc_node_t *child;

	if(!parent) {
		return 0;
	}

	if(target == parent) {
		return 1;
	}

	for(child=parent->child; child; child = child->sibling) {
		if(child == target) {
			return 1;
		}
	}

	for(child=parent->child; child; child = child->sibling) {
		if(_rsrc_is_child(child, target) == 0) {
			return 1;
		}
	}

	return 0;
}
 
/*
 This function is called when you want to perform some sort of search/removal
 operation on a set of blocks.

 rsrc_request_t *req --> The request which has been made from the user
 pid_t           pid --> The process id to set/search on (based on flags)
 unsigned      flags --> Remove the entry or just query
                         Removed entry from pid
						 Removed entry should be assigned to pid
 int		   *prio --> Priority of the located block.
*/
#define FLAG_ATTACH_RM			0x1
#define FLAG_ATTACH_RMTOPID		0x2
#define FLAG_ATTACH_RMFROMPID	0x4
#define FLAG_ATTACH_DONTLOOK	0x8
int _rsrc_block_search_op(rsrc_request_t *rsrc, pid_t pid, unsigned flags, int *priority) {
	uint64_t		alignment, curstart, curend;
	rsrc_root_node_t *root;
	rsrc_node_t		 *node;
	rsrc_create_t	*target;
	rsrc_request_t lastmatch;
	int			   lastprio, curprio;
	uint8_t			curvalid;

	/* Validate the resource entry being attached */
	if(!rsrc || !rsrc->name || 
	  ((rsrc->flags & RSRCDBMGR_FLAG_RANGE) && rsrc->start > rsrc->end)) {
		return EINVAL;
	}

	/* Find the node for this resource */
	if(!(node = rsrc_node_locate(rsrc->name, 0))) {
		return errno;
	}

	/* Find the root for this resource */
	if(!(root = rsrc_root_locate(rsrc->name, 0))) {
		return errno;
	}

	/* 
	 Iterate now over all of the entries and if not successfull, then over 
	 all of the child entries, looking for the best match that we can offer.
	*/
	memset(&lastmatch, 0, sizeof(lastmatch));
	lastprio = INT_MAX;

	if(flags & FLAG_ATTACH_DONTLOOK) {
		lastmatch.start = rsrc->start;
		lastmatch.end = rsrc->end;
		lastprio = 0;
	} else {
		pid_t			frompid;

		//OPTIMIZE: If we are at the head, then the range is 0-ULONGLONG_MAX rather than
		//restricting the range to what is in the create list.  
		if((rsrc->flags & RSRCDBMGR_FLAG_ALIGN) && rsrc->align) {
			alignment = rsrc->align;
		} else {
			alignment = 1;
		}
		frompid = (flags & FLAG_ATTACH_RMFROMPID) ? pid : 0;

		curvalid = 0;
		curstart = curend = 0;
		for(target = root->created; target && lastprio >= node->priority; target = target->next) {
			//Only look at items which are less than or equal to this node
			if(node != &root->head_node && !_rsrc_is_child(node, target->node)) {
				continue;
			}

			//Check the co-alesced creation ranges
			if(curvalid && target->start && target->start-1 > curend) {
				if(rsrc->flags & RSRCDBMGR_FLAG_RANGE) {
					curstart = __max(rsrc->start, curstart);
					curend = __min(rsrc->end, curend);
				}
	
				if(rsrc_find_range(root, rsrc->length, &curstart, &curend, 
									alignment, frompid, rsrc->flags, &curprio) != -1) {
					if(curprio < lastprio || 
					   (rsrc->flags & RSRCDBMGR_FLAG_TOPDOWN && curprio <= lastprio)) {
						lastmatch.start = curstart;
						lastmatch.end = curend;
						lastprio = curprio;
					}
				}
				curvalid = 0;
			}
	
			if(curvalid) {
				curstart =  __min(curstart, target->start);
				curend = __max(curend, target->end);
			} else {
				curstart = target->start;
				curend = target->end;
			}
			curvalid = 1;
		}
	
		//One last check on exit if we missed any
		if(curvalid) {
			if(rsrc->flags & RSRCDBMGR_FLAG_RANGE) {
				curstart = __max(rsrc->start, curstart);
				curend = __min(rsrc->end, curend);
			}

			if(rsrc_find_range(root, rsrc->length, &curstart, &curend, 
								alignment, frompid, rsrc->flags, &curprio) != -1) {
				if(curprio < lastprio) {
					lastmatch.start = curstart;
					lastmatch.end = curend;
					lastprio = curprio;
				}
			}
		}
	
		//We didn't find a matching entry, invalid arguments assumed
		if(lastprio == INT_MAX) {
			return EINVAL;
		}
	}

	if(flags & FLAG_ATTACH_RMTOPID) {			/* Remove and assign to pid */
		(void)rsrc_block_remove_range(root, lastmatch.start, lastmatch.end, 0);
		(void)rsrc_block_add_range_pid(root, lastmatch.start, lastmatch.end, 
								 pid, rsrc->flags & RSRCDBMGR_FLAG_SHARE);
	} else if(flags & FLAG_ATTACH_RMFROMPID) {	/* Remove from the pid */
		(void)rsrc_block_remove_range(root, lastmatch.start, lastmatch.end, pid);
		if(!(flags & FLAG_ATTACH_RM)) {
			_rsrc_block_limit_op(root, lastmatch.start, lastmatch.end, 
								 NULL, _add_callout,	/* Overlap ranges */	
								 NULL, NULL);			/* Non-overlap ranges */
		}
	}

	rsrc->start = lastmatch.start;
	rsrc->end = lastmatch.end;
	if(priority) {
		*priority = lastprio;
	}
	return EOK;
}

/*** INTERFACE FUNCTIONS ***/
int _rsrcdbmgr_create(rsrc_alloc_t *rsrc, pid_t pid) {
	rsrc_root_node_t *root;
	rsrc_node_t		 *node;
	rsrc_create_t    *create;

	/* Validate the resource entry being attached */
	if(!rsrc || !rsrc->name || rsrc->start > rsrc->end) {
		return EINVAL;
	}
	pid = (rsrc->flags & RSRCDBMGR_FLAG_NOREMOVE) ? 0 : pid;

	/* Find the node for this resource */
	if(!(node = rsrc_node_locate(rsrc->name, 1))) {
		return errno;
	}

	/* Find the root for this resource */
	if(!(root = rsrc_root_locate(rsrc->name, 0))) {
		return errno;
	}

	/* Create a new resource allocation entry */
	if(!(create = rsrc_create_allocate())) {
		return errno;
	}
	create->start = rsrc->start;
	create->end = rsrc->end;
	create->flags = rsrc->flags;
	create->creator = pid;
	create->node = node;

	/* Insert the data blocks before we insert the create block */
	_rsrc_block_limit_op(root, create->start, create->end, 
						 NULL, NULL,			/* Overlap ranges */	
						 NULL, _add_callout);	/* Non-overlap ranges */

	/* Insert the create block into the list ordered by start fields */
	rsrc_insert_create(&root->created, create);

	return EOK;
}

int _rsrcdbmgr_destroy(rsrc_alloc_t *rsrc, pid_t pid) {
	rsrc_root_node_t *root;
	rsrc_node_t		 *node;
	rsrc_create_t	 **prev, *target;

	/* Validate the resource entry being attached */
	if(!rsrc || !rsrc->name || rsrc->start > rsrc->end) {
		return EINVAL;
	}
	pid = (rsrc->flags & RSRCDBMGR_FLAG_NOREMOVE) ? 0 : pid;

	/* Find the node for this resource */
	if(!(node = rsrc_node_locate(rsrc->name, 0))) {
		return errno;
	}

	/* Find the root for this resource */
	if(!(root = rsrc_root_locate(rsrc->name, 0))) {
		return errno;
	}

	/*
	 Remove/Split/Modify the create block to erase this range.  
	 Then remove the active resources from the range.  
	 Makes it multipass but this is a low runner function.
	*/
	prev = &root->created;	
	while((target = *prev) && target->start <= rsrc->end) {
		if(target->node != node || target->creator != pid || 
		   !OVERLAPE(target->start, target->end, rsrc->start, rsrc->end)) {
			prev = &(*prev)->next;
			continue;
		}

		//Remove entire block, split the block or shrink block
		if(rsrc->start <= target->start && target->end <= rsrc->end) {
			*prev = target->next;
			rsrc_create_free(target);
		} else if(target->start < rsrc->start && rsrc->end < target->end) {
			if(!(target = rsrc_create_allocate())) {
				return ENOMEM;
			}
			//Duplicate this block and insert w/ adjusted end point
			memcpy(target, *prev, sizeof(*target));
			target->start = rsrc->end + 1;
			(*prev)->end = rsrc->start - 1;

			target->next = NULL;
			rsrc_insert_create(&root->created, target);
			prev = &(*prev)->next;
		} else if(rsrc->start <= target->start) {
			target->start = rsrc->end + 1;
			*prev = target->next;

			target->next = NULL;
			rsrc_insert_create(&root->created, target);
			//Don't adjust prev, look at it again
		} else { /* rsrc->end => target->end */
			target->end = rsrc->start -1;
			prev = &target->next;
		}
	}

	/* Remove the data blocks after we removed the create block */
	_rsrc_block_limit_op(root, rsrc->start, rsrc->end, 
						 NULL, NULL,				/* Overlap ranges */	
						 NULL, _remove_callout);	/* Non-overlap ranges */

	return EOK;
}

/*
 Find, Remove and Attach all work to find a resource based on the
 set of criteria which the user has specified.
*/
int _rsrcdbmgr_find(rsrc_request_t *rsrc, pid_t pid, int *priority) {
	return _rsrc_block_search_op(rsrc, pid, 0, priority);
}
int _rsrcdbmgr_remove(rsrc_request_t *rsrc, pid_t pid, int *priority) {
	if(pid < 0) {
		return _rsrc_block_search_op(rsrc, -pid, FLAG_ATTACH_RMFROMPID | FLAG_ATTACH_DONTLOOK, priority);
	} else {
		return _rsrc_block_search_op(rsrc, pid, FLAG_ATTACH_RMTOPID | FLAG_ATTACH_DONTLOOK, priority);
	}
}

int _rsrcdbmgr_attach(rsrc_request_t *rsrc, pid_t pid) {
	return _rsrc_block_search_op(rsrc, pid, FLAG_ATTACH_RMTOPID, 0);
}

/*
 Return a value to the system.  This must be a value that is valid
 for the process, otherwise this will error.  This function can't
 be used to return all values to the system. 
*/ 
int _rsrcdbmgr_detach(rsrc_request_t *rsrc, pid_t pid) {
	return _rsrc_block_search_op(rsrc, pid, FLAG_ATTACH_RMFROMPID, 0);
}

/*
 Remove all resources associated with this pid.  First we remove all
of the resources that have been created and not allocated, then we
remove all of the resources that this particular process has had allocated.
*/
int _rsrcdbmgr_pid_clear(pid_t pid) {
	PROCESS				*prp;
	rsrc_root_node_t	*root;
	rsrc_create_t		**create, *ctarget;
	rsrc_block_t		**blockp, *btarget;

	if(!(prp = proc_lock_pid(pid))) {
		return 0;
	}
	if(!prp->rsrc_list || !prp->rsrc_list->haversrc) {
		proc_unlock(prp);
		return 0;
	}
	proc_unlock(prp);

	for(root = g_rsrc_root; root; root = root->next_root) {

		create = &root->created;
		while((ctarget = *create)) {
			if(ctarget->creator == pid) {
				*create = ctarget->next;

				_rsrc_block_limit_op(root, ctarget->start, ctarget->end, 
									 NULL, NULL,				/* Overlap ranges */	
									 NULL, _remove_callout);	/* Non-overlap ranges */
				
				rsrc_create_free(ctarget);
			} else {
				create = &ctarget->next;
			}
		}

		blockp = &root->active;
		while((btarget = *blockp)) {
			if(btarget->owner == pid) {
				if((*blockp = btarget->next)) {
					(*blockp)->prev = btarget->prev;
				}

				_rsrc_block_limit_op(root, btarget->start, btarget->end, 
								 NULL, _add_callout,	/* Overlap ranges */	
								 NULL, NULL);			/* Non-overlap ranges */
				rsrc_block_free(btarget);
			} else {
				blockp = &btarget->next;
			}
		}
	}

	return 0;
}

/*
 Mark the process as having accessed a resource
*/
int _rsrcdbmgr_pid_mark(pid_t pid) {
	PROCESS *prp;
	rsrc_list_array_t *pidrsrc;

	if(!(prp = proc_lock_pid(pid))) {
		return -1;
	}
	
	if(!(pidrsrc = (rsrc_list_array_t *)prp->rsrc_list)) {
		pidrsrc =
		prp->rsrc_list = _scalloc(sizeof(*pidrsrc));
	}

	pidrsrc->haversrc = 1;
	proc_unlock(prp);
	return 0;
}

/*
 Things we want to query for: 
	Created Resources (start,end,owner,priority) (any level)
	Free Resources (start,end,flags) (top level)
	Allcated Resources (start,end,owner,flags) (top level)

 Return the start,end,flags & pid of resources which are:
	pid == 0  --> All free resources
	pid == -1 --> All used resources
	pid == -2 --> All resources
	pid == pid--> All resources used by pid

 Starting at the 0 based index defined as start

 If newname is non NULL, then space is carved away from the
 input buffer and the name is returned in the pointer.

 TODO:
	Put this support into the proc filesystem.

 TODO:
	Additionally return information on created blocks rather
 than the allocated blocks if the FLAG_QUERY_CREAT is set.
*/
	
#define FLAG_QUERY_NEXT 	0x1		/* Get the next root element */
#define FLAG_QUERY_COUNT	0x2		/* Return the count of the objects */
#define FLAG_QUERY_CREATE	0x4		/* Look at the create rather than the active blocks */
int _rsrcdbmgr_query(const char *name, pid_t pid, unsigned flags, int start, 
					 rsrc_alloc_t *array, uint32_t nbytes, char **newname) {
	rsrc_root_node_t *root;
	rsrc_block_t	 *blockp;
	int				 i, total;

	root = rsrc_root_locate((name) ? name : NULL, 0);
	if(root && name && flags & FLAG_QUERY_NEXT) {
		root = root->next_root;
	}

	if(!root) {		//This could be viewed as an error and not a 0 count
		return 0;
	}

	if(flags & FLAG_QUERY_CREATE) {
		errno = ENOTSUP;
		return -1;
	}

	if(newname) {
		*newname = NULL;
	}

	for(total=0, blockp=root->active; blockp; blockp=blockp->next) { 
		if(pid == -2 || blockp->owner == pid || (pid == -1 && blockp->owner)) {
			total++;
		}
	}

	if(flags & FLAG_QUERY_COUNT || total == 0) {
		if(newname && array && nbytes > 0) {
			*newname = (char *)array;
			STRLCPY(*newname, root->head_node.name, nbytes);
		}
		return total;
	}

    if(total <= start || !array || !nbytes) {
		errno = EINVAL;
        return -1;
    }

	//Stuff the name at the end of the data array if possible/wanted
	if(newname) {
		if(nbytes < (strlen(root->head_node.name) + 1)) {
			errno = EINVAL;
			return -1;
		}
		nbytes -= strlen(root->head_node.name) + 1;
	}

    //Determine the number of elements that we should copy
    total = __min(total - start, nbytes / sizeof(rsrc_alloc_t));

    for (i=0, blockp=root->active; blockp && i < (start + total); blockp=blockp->next) {
		if(pid == -2 || blockp->owner == pid || (pid == -1 && blockp->owner)) {
			if(i++ < start) {  
				continue;
			}
			array[(i-start)-1].start = blockp->start;
			array[(i-start)-1].end   = blockp->end;
			array[(i-start)-1].flags = blockp->flags;
			*((pid_t *)(&array[(i-start)-1].name)) = blockp->owner;
		}
    }

	if(newname) {
		*newname = (char *)&array[i-start];
		STRLCPY(*newname, root->head_node.name, nbytes - ((size_t)&array[i-start] - (size_t)array));
	}

    return total;
}

/************************/

#if 0
void dump_list(char *name) {
	rsrc_root_node_t *root;
	rsrc_create_t	 *create;
	rsrc_block_t	 *block;

	if(!(root = rsrc_root_locate(name, 0))) {
		return;
	}

	kprintf("Created blocks (%s):\n", name);	
	for(create=root->created; create; create = create->next) {
		kprintf(" %lld-%lld (by %d node %p (pri%d) flags 0x%x)\n",
				create->start, create->end, create->creator, 
				create->node, create->node->priority, create->flags);
	}
	kprintf("Allocated blocks (%s):\n", name);	
	for(block=root->active; block; block = block->next) {
		kprintf(" %lld-%lld (owner %d flags 0x%x)\n",
				block->start, block->end, block->owner, block->flags);
	}
}
#endif


__SRCVERSION("rsrcdbmgr_list.c $Rev: 153052 $");
