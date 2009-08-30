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




#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/iofunc.h>
#include "iofunc.h"

/* 
 Internal Generic Lock Functions.
 For a good reference on file locking see:
 Stevens, Advanced Programming in the Unix Environment, 12.3 - Record Locking
 Posix standard 6.5.2
*/
#define CONTAINS(s1, e1, x)     (((s1) <= (x)) && ((x) <= (e1)))
#define OVERLAP(s1, e1, s2, e2) (CONTAINS(s1, e1, s2) || CONTAINS(s2, e2, s1))
#include <stdio.h>

static int _iofunc_lock_scoid(iofunc_lock_list_t **plist, int scoid, int type, off64_t start, off64_t end);

/*
 *  _iofunc_lock_alloc -- allocates free lock list item
 */
static iofunc_lock_list_t *_iofunc_lock_alloc() {
	iofunc_lock_list_t *lockp;

	if ((lockp = (iofunc_lock_list_t *)malloc(sizeof(*lockp)))) {
		memset(lockp, 0, sizeof(*lockp));
	}
	return lockp;
}

/*
 *  _iofunc_blocked_free - returns a blocked item to the free list 
 *  _iofunc_lock_free - returns a lock item to the free list 
 *                    - If there are any pending items, they are
 *                      send an ENOENT (?EINTR?) to tell them that
 *                      they have been stranded!
 */
static void _iofunc_blocked_free(struct _iofunc_lock_blocked *block) {
	if (block) {
		if (block->pflock) {
			free(block->pflock);
		}
		free(block);
	}
}

void _iofunc_lock_free(iofunc_lock_list_t *lockp) {
	if (lockp) {
		while (lockp->blocked) {
			struct _iofunc_lock_blocked *tmp;
			MsgError(lockp->blocked->rcvid, ENOENT);
			tmp = lockp->blocked;
			lockp->blocked = lockp->blocked->next;
			_iofunc_blocked_free(tmp);
		}
		free(lockp);
	}
}

/*
 *  _iofunc_lock_add - routine to add item to the lock list.  Always add the 
 *  lock after the pointer, unless the pointer points to NULL?
 */
static int _iofunc_lock_add (iofunc_lock_list_t **cl, int scoid, int type, off64_t start, off64_t end) {
    iofunc_lock_list_t  *nl;

    /*
     *  get a lock, return if none available
     */
    if((nl = _iofunc_lock_alloc()) == NULL) {
        errno = ENOMEM;
        return errno;
	}

    /*
     *  link the new entry into list at current spot
     *  fill in the data from the args
     */
	if (*cl) {
	    nl->next = (*cl)->next;
		(*cl)->next = nl;
	}
	else {
		*cl = nl;
		nl->next = NULL;
	}

	//Having the rcvid in here is totally useless so we don't
	//nl->rcvid = rcvid;
    nl->scoid  = scoid;
    nl->start = start;
    nl->end   = end;
    nl->type  = type;
    return EOK;
}

/*
 * Actually do the reply to the client described by the blocked pointer, and 
 * then afterward we have replied free the blocked strucuture.
 */
static void _iofunc_lock_unblock(iofunc_lock_list_t *list, struct _iofunc_lock_blocked *blocked) {
	struct _msg_info			info;
	struct _io_lock_reply		reply;
	struct iovec				iov[2];

	//Some initial sanity checking ...
	if (MsgInfo(blocked->rcvid, &info) == EOK && blocked->pflock && 
		blocked->pflock->l_sysid == info.scoid) {
		
		SETIOV(&iov[0], &reply, sizeof(reply));
		SETIOV(&iov[1], blocked->pflock, sizeof(*blocked->pflock));

		(void)MsgReplyv(blocked->rcvid, EOK, iov, 2);
	}

	_iofunc_blocked_free(blocked);
}

/*
 * Join two list of blocked entries
 */
static void add_blocked_to_end(struct _iofunc_lock_blocked **list, struct _iofunc_lock_blocked *toadd) {
	struct _iofunc_lock_blocked *tmp;

	if (!(tmp = *list)) {
		*list = toadd;
	}
	else  {
		while (tmp->next) { tmp = tmp->next; }
		tmp->next = toadd;
	}
}

static void reblock_pending_locks(struct _iofunc_lock_blocked **blocked_list, iofunc_lock_list_t **plist_head) {
	struct _iofunc_lock_blocked *bitem, *tbitem;
	iofunc_lock_list_t         *litem;
	int							type, scoid;

	bitem = *blocked_list;

	while (bitem) {
		tbitem = bitem;
		bitem = bitem->next;

		if (!tbitem->pflock) {
			MsgError(tbitem->rcvid, EINVAL);
			_iofunc_blocked_free(tbitem);
			continue;
		}

		type = tbitem->pflock->l_type;
		scoid = tbitem->pflock->l_sysid;

		//TODO: A faster lookup?
		litem = _iofunc_lock_find(*plist_head, scoid, type, tbitem->start, tbitem->end);

		if (litem) {
			if (!(tbitem->next = litem->blocked)) {
				litem->blocked = tbitem;
			}
			else {
				while (tbitem->next->next) { tbitem->next = tbitem->next->next; }
				tbitem->next->next = tbitem;
			}
			tbitem->next = NULL;
		}
		else {
			type = _iofunc_lock_scoid(plist_head, scoid, type, tbitem->start, tbitem->end);
			if (type != EOK) {
				MsgError(tbitem->rcvid, type);
				_iofunc_blocked_free(tbitem);
			}
			else {
				_iofunc_lock_unblock(*plist_head, tbitem);
			}
		}
	}
}


/*
 *  locksplit - routine to split a lock into 2 or 3 pieces
 *              and return a pointer to the new piece
 *
 *  Split the lock referenced by cl into 2 pieces
 *  if the request overlaps the beginning or end of the lock.
 *  Split the lock referenced by cl into 3 pieces
 *  if the request overlaps the middle of the lock.
 *
 *  This routine would only be called if the request type
 *  differs from the lock type, so don't bother checking for this.
 *  Otherwise the request could just be merged with the lock.
 */
static iofunc_lock_list_t *locksplit(iofunc_lock_list_t **head, iofunc_lock_list_t *cl, 
										int scoid, int type, off64_t start, off64_t end) {
	struct _iofunc_lock_blocked *pending;

	//There must be an overlap and a type difference for us to get going on this
	if (!OVERLAP(cl->start, cl->end, start, end) || type == cl->type) {
		return cl;
	}

	/*
	 Same strategy as before, if we are re-arranging
	 then we will re-examine the blocked entries
	 after we are done with our fiddling.
	*/
	pending = NULL;
	add_blocked_to_end(&pending, cl->blocked);
	cl->blocked = NULL;


	/* New block is bigger than existing block */
	if (start <= cl->start && end >= cl->end) {
		cl->start = start;
		cl->end = end;
		cl->type = type;
	}
	/* New block is smaller than existing block */
	else if (cl->start < start && cl->end > end) {

        if(_iofunc_lock_add(&cl, cl->scoid, type, start, end) != EOK) {
            return(NULL);
		}

        if(_iofunc_lock_add(&cl->next, cl->scoid, cl->type, end+1, cl->end) != EOK) {
            return(NULL);
		}

		cl->end   = start -1;
	}
	/* New block overlaps on one side of the existing block
	   grow to one side or the other (FRONT, then BACK) */
	else if (cl->start >= start) {
		if (_iofunc_lock_add(&cl, cl->scoid, cl->type, end + 1, cl->end) != EOK) {
			return NULL;
		}

		cl->type = type;
		cl->start = start;
		cl->end = end;
	}
	else {
		if (_iofunc_lock_add(&cl, cl->scoid, type, start, end) != EOK) {
			return NULL;
		}	
		cl->end = start -1;
	}

	if (pending) {
		reblock_pending_locks(&pending, head);
	}

	return cl;
}


/*
 *  unlock - handle unlock request (called from lock hander)
 */
int _iofunc_unlock_scoid (iofunc_lock_list_t **plist, int scoid, off64_t start, off64_t end) {
	struct _iofunc_lock_blocked *pending;
    iofunc_lock_list_t  *cl,    /*  current list item   */
						**pl;   /*  prev list item  */

    /*
     *  Starting at list head scan for locks in the range by this process.
     */
    if (!(pl = plist) || !*pl) {
		return EOK;
	}

	pending = NULL;
    for(cl = *pl; cl; cl = *pl) {
        /*
         *  If not by this process skip to next lock.
		 *  Check for locks in the proper range.
         */
        if(cl->scoid != scoid || !OVERLAP(start, end, cl->start, cl->end)) {
			pl = &cl->next;
            continue;
        }

		/*
		 The strategy here is that we will strip all 
		 of the pending locks off of anything that
		 is changing size, and then after we have
		 iterated through the list we will go back and
		 attach these locks to the appropriate blocks,
		 or create entirely new blocks and tell the pending
		 people.
		*/
		add_blocked_to_end(&pending, cl->blocked);
		cl->blocked = NULL;

        /*
         *  For locks fully contained within requested range,
         *  just delete the item.
         */
        if(start <= cl->start  &&  cl->end <= end) {
			*pl = cl->next;
            _iofunc_lock_free(cl);
            continue;
        }

		pl = &cl->next;

        /*
         *  Middle section is being removed.
         *  Add new lock for last section,
         *  modify existing lock for first section.
         *  If no locks, return in error.
         */
        if(cl->start < start  &&  end < cl->end) {
            if(_iofunc_lock_add(&cl, cl->scoid, cl->type, end + 1, cl->end) != EOK) {
                return EINVAL;
			}
            cl->end = start - 1;
		}
        /*
         *  First section is being deleted,
         *  just move starting point up.
         */
        else if(start <= cl->start  &&  end < cl->end) {
            cl->start = end + 1;
		}
        /*
         *  Must be deleting last part of this section.
         *  Move ending point down.
         *  Continue looking for locks covered by upper limit of unlock range.
         */
        else { 
			cl->end = start - 1;
		}
	}

	if (pending) {
		reblock_pending_locks(&pending, plist);
	}

    return EOK;
}

int _iofunc_unlock (resmgr_context_t *ctp, iofunc_lock_list_t **plist, off64_t start, off64_t end) {
	return _iofunc_unlock_scoid (plist, ctp->info.scoid, start, end);
} 


/*
 *  lock - handle lock requests return errno to reply to client w/
 */
static int _iofunc_lock_scoid(iofunc_lock_list_t **plist, int scoid, int type, off64_t start, off64_t end) {
    iofunc_lock_list_t  *cl,    /*  current list item   */
						*pl,	/*  previous list item */
						*ol;
	int					ret;

	ol = cl = pl = NULL;

    /*
     *  Simple case, no existing locks, simply add new lock.
     */
    if((cl = *plist) == NULL) {
        return _iofunc_lock_add(plist, scoid, type, start, end);
	}

    /*
     *  Simple case, lock is before existing locks.
     *  Just insert at head of list (no possible overlap).
     */
    if(end < cl->start) {
		pl = *plist, *plist= NULL;
        if ((ret = _iofunc_lock_add(plist, scoid, type, start, end)) != EOK) {
			*plist = pl;
		}
		else {
			(*plist)->next = pl;
		}
		return ret;
	}

    /*
     *  Scan thru remaining locklist to find location for new request.
     */
    for(pl = NULL;cl != NULL; pl = cl, cl = cl->next) {
        /*
         *  If lock is owned by same process and overlaps, stop scan.
         *  If more locks in range, skip to next.
         *  Otherwise stop scan.
         */
        if(cl->scoid == scoid  &&  
		   OVERLAP(start, end, cl->start, cl->end)) {
			break;
        }
        else if(cl->start > start) {
			cl = pl;
            break;
		}
	}
	/*
	 *  Request is at end of list, so we can add the lock here.
	 */
	if(cl == NULL){
		return _iofunc_lock_add(&pl,scoid,type,start,end);
	}

    /*
     *  If we overlap & id's match
	 *   -And the types match, create a superblock
	 *   -And the types differ, split the block
     *  Otherwise add new entry
     */
    if(cl->scoid == scoid && OVERLAP(start, end, cl->start, cl->end)) {
        if(cl->type == type) {
			cl->end = __max(cl->end, end);
			cl->start = __min(cl->start, start);
			return EOK;
		}
        else if (!(cl = locksplit(plist, cl, scoid, type, start, end))) {
			return EINVAL;
		}
		//Otherwise go on an compact w/ cl
	}
    else {
		if((ret = _iofunc_lock_add(&cl, scoid, type, start, end))) {
			return ret;
		}
        cl = cl->next;
	}

/* 
 This compaction is not required, though it might optimize things 
 slightly if we were to join blocks together. (Check boundary
 conditions as well as overlap)
*/
#if 0
    /*
     *  Remember the new lock region for later
     */
    ol = cl;

    /*
     *  End point set above may overlap later entries.
     *  If so delete or modify them to perform the compaction.
     */
    while(nl = cl->next; cl && nl; cl = nl, nl = cl->next) {
        /*
         *  If the new range overlaps the first part of the next lock,
         *  if its the same type,
         *      take its end point and delete the next lock
         *  else
         *      move the start point (unlocking anyone waiting).
         *  We should be done.
         */
        if(cl->scoid == scoid) {
            if(end <= nl->end) {
                if(nl->type == type) {
                    ol->end = nl->end;
                    cl->next = nl->next;
                    _iofunc_lock_free(nl);
                }
                else {
                    nl->start = start;
                    if(nl->blocked) {
						printf("TODO: Wakup people! \n");
                        //wakeup(nl);
                    }
				}
                return EOK;
            }
            else {
                /*
                 *  the next lock is fully included in the new range
                 *  so it may be deleted
                 */
                cl->next = nl->next;
                _iofunc_lock_free(nl);
            }
		} 
		else {
            cl = nl;
		}
    }
#endif

    return EOK;
}

int _iofunc_lock(resmgr_context_t *ctp, iofunc_lock_list_t **plist, int type, off64_t start, off64_t end) {
	if (type == F_UNLCK) {
		return _iofunc_unlock(ctp, plist, start, end);
	}
	else {
		return _iofunc_lock_scoid(plist, ctp->info.scoid, type, start, end);
	}
}


/*
 *  find - routine to scan locks and check for a locked condition that
 *         would interfere with the lock requested by scoid of type from start-end
 */
iofunc_lock_list_t *_iofunc_lock_find(iofunc_lock_list_t *list, int scoid, int type, off64_t start, off64_t end) {
   	iofunc_lock_list_t *tmp;
	
	if (type == F_UNLCK) {
		return NULL;
	}

	tmp = list;
	while (tmp) {
		if ((scoid != tmp->scoid) && 
		    OVERLAP(start, end, tmp->start, tmp->end)) {
			//!(tmp->type == type && type == F_RDLCK)
			if (tmp->type != type || type != F_RDLCK) {
				return tmp;
			}
		}
		tmp = tmp->next;
	}

    return NULL;
}

#if 0
/*
 *  deadlock - routine to follow chain of locks and tasks
 *              to find deadlocks on file locks.
 */
static int deadlock (iofunc_lock_list_t *llist, int scoid, int type, off64_t start, off64_t end) {
	return EOK;
}
#endif

__SRCVERSION("_iofunc_lock.c $Rev: 200568 $");
