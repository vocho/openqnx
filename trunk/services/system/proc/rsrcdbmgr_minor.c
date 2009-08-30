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
 Minor operations
 
 The major/minors are packed into an int in the
 following manner:

#define minor(device)                   ((int)((device) & 0x3ff))
#define major(device)                   ((int)(((device) >> 10) & 0x3f))
#define makedev(node,major,minor)       ((dev_t)(((node) << 16) | ((major) << 10) | (minor)))

  Which means there is a maximum of 2^10 minors and 2^6 majors
*/
#include <sys/types.h>
#include "rsrcdbmgr.h"

#define GROW_SIZE 10

rsrc_devno_t g_devno_table[MAX_NUM_MAJORS];

int _rsrc_minor(const char *name, int *major, int *minor_request, int flags) {
	rsrc_devno_t *head = NULL;
	uint16_t	index;	//At most MAX_NUM_MINORS (1024) / 8
	uint8_t		bit;	//At most 8 right now
	uint8_t		mask;	//A mask for 8 bits
	uint8_t		eindex;	//Never more than MAX_NUM_MAJORS (64)

	/*** Previously encapsulated in a different function ***/

	/* major, name are assumed to be valid if they are being searched for */
	for(index=0, eindex=MAX_NUM_MAJORS; index < MAX_NUM_MAJORS; index++) {
		if(g_devno_table[index].name == NULL) {
			eindex = __min(eindex, index);
		} else if((!(flags & RSRCDBMGR_REQ_ATTACH) && index == *major) ||
				  ((flags & RSRCDBMGR_REQ_ATTACH) && strcmp(g_devno_table[index].name, name) == 0)) {
			eindex = index;
			head = &g_devno_table[index];
		}
	}

	if(!head) {
		/* No major device being created, or no space left */
		if(!(flags & RSRCDBMGR_REQ_ATTACH) || eindex == MAX_NUM_MAJORS) {
			return EINVAL;
		}

		/* Pre-allocate with 8 entries if nothing there already */
		g_devno_table[eindex].name = strdup(name); 
		if(g_devno_table[eindex].minors == NULL) {
			g_devno_table[eindex].minors = _scalloc(sizeof(struct _devno_minors));
			g_devno_table[eindex].minors->count = 1;
		}

		head = &g_devno_table[eindex];
	}

	/****/

	if (flags & RSRCDBMGR_REQ_ATTACH) {
		if(*minor_request < 0) {		//Locate a minor entry
			for (bit = index = 0; index < head->minors->count; index++) {
				if((mask = head->minors->bits[index]) != 0xFF) {
					for(bit = 0; mask & (0x1 << bit); bit++) { ; }
					break;
				}
			}
			*minor_request = (index * 8) + bit;
		}
	}

	if(*minor_request >= MAX_NUM_MINORS) {
		return EINVAL;
	}

	index = *minor_request / 8; 
	bit = *minor_request % 8;
	mask = 0x1 << bit;

	if(flags & RSRCDBMGR_REQ_ATTACH) {
		if (index >= head->minors->count) {
			struct _devno_minors *tmp = head->minors;

			if(!(head->minors = _srealloc(head->minors, 
								          (head->minors->count - 1) + sizeof(*head->minors), 
								          index + sizeof(*head->minors)))) {
				head->minors = tmp;
				return ENOMEM;
			}

			memset(&head->minors->bits[head->minors->count], 0, index + 1 - head->minors->count);
			head->minors->count = index + 1;
		} else if (head->minors->bits[index] & mask) {
			return EINVAL;
		}

		head->minors->bits[index] |= mask;
		*major = eindex;
	} else {
		head->minors->bits[index] &= ~mask;

		for(index=0; index < head->minors->count; index++) { 
			if(head->minors->bits[index]) {
				return EOK;
			}
		}

		/* Only free the name, leave the bits allocated */
		free(head->name);
		head->name = NULL;
	}

	return EOK;
}

int rsrcmgr_handle_minor(resmgr_context_t *ctp, rsrc_cmd_t *msg, rsrc_minor_request_t *data) {
	rsrc_list_array_t	*list_array;
	PROCESS				*prp;
	int					i, ret, type;

	//The data name is supposed to live after the structure ...
	data->name = (char*)((char*)data + sizeof(*data));

	switch ((type = msg->i.subtype & RSRCDBMGR_REQ_MASK)) {
	case RSRCDBMGR_REQ_DETACH: 				//Give back a minor number resource
		if(data->major >= MAX_NUM_MAJORS || data->minor >= MAX_NUM_MINORS) {
			return EINVAL;
		}
		ret = makedev(0, data->major, data->minor);
		break;

	case RSRCDBMGR_REQ_ATTACH: 				//Get a minor number for the device
		ret = -1;
		break;

	default:
		return EINVAL;
	}

	prp = proc_lock_pid((msg->i.pid > 0) ? msg->i.pid : ctp->info.pid);

	/* Get ourselves a resource listing even if we don't need it */
	if(!(list_array = prp->rsrc_list)) {
		if(!(prp->rsrc_list = _scalloc(sizeof(*list_array)))) {
			return proc_error(ENOMEM, prp);
		}
		list_array = prp->rsrc_list;
	}

	/* Look for an appropriate index either to delete or insert to */
	for (i = 0; i < list_array->dev_count; i++) {
		if (list_array->dev_list[i] == ret) {
			list_array->dev_list[i] = -(dev_t)1;
			break;
		}
	}

	/* Grow the the array or throw out an error if need be */
	if (i >= list_array->dev_count) {
		dev_t *tmp = list_array->dev_list;

		if(type == RSRCDBMGR_REQ_DETACH) {
			return proc_error(EINVAL, prp);
		}

		list_array->dev_list = _srealloc(list_array->dev_list, 
										 list_array->dev_count * sizeof (dev_t),
										(list_array->dev_count+GROW_SIZE) * sizeof (dev_t));
		if(!list_array->dev_list) {
			list_array->dev_list = tmp;
			return proc_error(ENOMEM, prp);
		}

		memset(&list_array->dev_list[i], -1, GROW_SIZE * sizeof(dev_t));
		list_array->dev_count += GROW_SIZE;
	}

	pthread_mutex_lock(&g_rsrc_mutex);

	/* Actually go and perform the operation now */
	ret = _rsrc_minor(data->name, &data->major, &data->minor, type);

	pthread_mutex_unlock(&g_rsrc_mutex);

	if(type == RSRCDBMGR_REQ_ATTACH && ret == EOK) {
		list_array->dev_list[i] = makedev(0, data->major, data->minor);

		SETIOV(ctp->iov, data, sizeof(*data));
		MsgReplyv(ctp->rcvid, ret, ctp->iov, 1);
		ret = _RESMGR_NOREPLY;
	}
	
	proc_unlock(prp);

	return(ret);
}


__SRCVERSION("rsrcdbmgr_minor.c $Rev: 153052 $");
