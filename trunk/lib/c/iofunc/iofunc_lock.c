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




#define _FILE_OFFSET_BITS		32
#define _IOFUNC_OFFSET_BITS		64
#include <stdlib.h>
#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/iofunc.h>
#include <sys/netmgr.h>
#include "iofunc.h"

#if defined(__WATCOMC__) && __WATCOMC__ <= 1100
#define FIXCONST	__based(__segname("CONST2"))
#else
#define FIXCONST
#endif

int iofunc_lock(resmgr_context_t *ctp, io_lock_t *msg, iofunc_ocb_t *ocb, iofunc_attr_t *attr) {
									/* "void *" is to keep Metware happy */
	flock_t				*lockp = (flock_t *)(void *)(msg + 1);
	int					(*attr_lock)(iofunc_attr_t *), (*attr_unlock)(iofunc_attr_t *);
    iofunc_funcs_t		*funcs;
	iofunc_lock_list_t	*head;
	off64_t				start, end;
	int					status;
	int					cmd, toeof;

	if (lockp->l_type != F_RDLCK && lockp->l_type != F_WRLCK && lockp->l_type != F_UNLCK)
		return(EINVAL);

	switch(cmd = msg->i.subtype) {
	case F_SETLK:
		cmd = F_SETLK64;
		goto setlk;
	case F_SETLKW:
		cmd = F_SETLKW64;
setlk:
		lockp->l_start_hi = (lockp->l_start < 0) ? -1 : 0;
		lockp->l_len_hi = (lockp->l_len < 0) ? -1 : 0;
		/* Fall Through */
	case F_SETLK64:
	case F_SETLKW64:
		// If mandatory locks enabled, and locks exist, return an error
		if(PTR_VALUE(attr->lock_list, iofunc_lock_list_t) && (attr->mode & (S_IXGRP | S_ISGID)) == S_ISGID) {
			return EAGAIN;
		}
		switch(lockp->l_type) {
		case F_RDLCK:
			if((ocb->ioflag & _IO_FLAG_RD) == 0) {
				return EBADF;
			}
			break;
		case F_WRLCK:
			if((ocb->ioflag & _IO_FLAG_WR) == 0) {
				return EBADF;
			}
			break;
		case F_UNLCK:
			break;		//Nothing to check here ...
		default:
			break;
		}
		break;

	case F_GETLK:
		cmd = F_GETLK64;
		lockp->l_start_hi = (lockp->l_start < 0) ? -1 : 0;
		lockp->l_len_hi = (lockp->l_len < 0) ? -1 : 0;
		/* Fall Through */
	case F_GETLK64:
		break;

	default:
		return EINVAL;
	}

	start = (unsigned)lockp->l_start | ((off64_t)lockp->l_start_hi << 32);
	switch(lockp->l_whence) {
	case SEEK_SET:
		break;

	case SEEK_CUR:
		start += ocb->offset;
		break;

	case SEEK_END:
		start += attr->nbytes;
		break;

	default:
		return EINVAL;
	}

	if ((toeof = (lockp->l_len == 0 && lockp->l_len_hi == 0))) {
		end = LONGLONG_MAX;
	}
	else if (lockp->l_len_hi < 0) {
		end = start - 1;
		start += ((unsigned)lockp->l_len | ((off64_t)lockp->l_len_hi << 32));
	}
	else {
		end = ((unsigned)lockp->l_len | ((off64_t)lockp->l_len_hi << 32)) + start - 1;
	}

	if (start < 0 || end < 0)
		return(EINVAL);
	if (IS32BIT(attr, ocb->ioflag) && (toeof ? start : end) > LONG_MAX)
		return(EOVERFLOW);

	status = EOK;
	if (attr->mount == NULL || (funcs = attr->mount->funcs) == NULL || funcs->nfuncs < (offsetof(iofunc_funcs_t, attr_unlock) / sizeof(void *)) || (attr_lock = funcs->attr_lock) == NULL || (attr_unlock = funcs->attr_unlock) == NULL) {
		attr_lock = iofunc_attr_lock, attr_unlock = iofunc_attr_unlock;
	}
	(void)(*attr_lock)(attr);
	(void)_iofunc_llist_lock(attr);
	head = attr->lock_list, PTR_UNLOCK(head);

	if(cmd == F_GETLK64) {
		iofunc_lock_list_t			*l;

		if(lockp->l_type == F_UNLCK) {
			status = EINVAL;
		} else if((l = _iofunc_lock_find(head, ctp->info.scoid, lockp->l_type, start, end))) {
			struct _client_info				info;

			lockp->l_type = l->type;
			lockp->l_whence = SEEK_SET;
			lockp->l_start = l->start & 0xffffffff;
			lockp->l_start_hi = l->start >> 32;
			if(l->end == LONGLONG_MAX) {
				lockp->l_len = lockp->l_len_hi = 0;
			} else {
				off64_t			len = (l->end - l->start) + 1;

				lockp->l_len = len & 0xffffffff;
				lockp->l_len_hi = len >> 32;
			}
			memset(&info, 0, sizeof(info));
			ConnectClientInfo(l->scoid, &info, 0);
			lockp->l_pid = info.pid;
			lockp->l_sysid = netmgr_remote_nd(ctp->info.nd, info.nd);
		} else {
			lockp->l_type = F_UNLCK;
		}
	} else {
		iofunc_lock_list_t			*l;

		if((l = _iofunc_lock_find(head, ctp->info.scoid, lockp->l_type, start, end))) {
			if(cmd == F_SETLKW64) {
				struct _iofunc_lock_blocked		*b;

				//TODO: Do more than this, actually look at the entire locked list
				if (ctp->info.scoid == l->scoid) {
					status = EDEADLK;
				}
				else if(!(b = malloc(sizeof *b)) ||
				   !(b->pflock = malloc(sizeof *lockp))) {
					if (b) {
						free(b);
					}
					status = ENOMEM;
				} else {
					memcpy(b->pflock, lockp, sizeof(*lockp));
					b->pflock->l_sysid = ctp->info.scoid;
					b->rcvid = ctp->rcvid;
					b->start = start;
					b->end = end;

					//Insert at the end of the list
					if (!(b->next = l->blocked)) {
						l->blocked = b;
					}
					else {
						while (b->next->next) { b->next = b->next->next; }
						b->next->next = b;
					}
					b->next = NULL;

					status = _RESMGR_NOREPLY;
				}
			} else {
				status = EAGAIN;
			}
		} else {
			status = _iofunc_lock(ctp, &head, lockp->l_type, start, end);
		}
	}

	PTR_LOCK(head), attr->lock_list = head;
	_iofunc_llist_unlock(attr);
	(void)(*attr_unlock)(attr);
	if(status != EOK) {
		return status;
	}

	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o + sizeof *lockp);
}

__SRCVERSION("iofunc_lock.c $Rev: 153052 $");
