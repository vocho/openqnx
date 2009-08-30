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
#include "asyncmsg_priv.h"

int asyncmsg_channel_destroy(int chid)
{
	struct _asyncmsg_channel_context *acc;
	
	if (ChannelDestroy(chid) == -1) {
		return -1;
	}
	
	if ((acc = _asyncmsg_handle(chid, _ASYNCMSG_HANDLE_DELETE | _ASYNCMSG_HANDLE_CHANNEL, 0)) == NULL)
	{
		return -1;
	}

	/* how do we destroy mutex safely ? */
	pthread_mutex_destroy(&acc->mutex);
	
	if (acc->free) {
		if (!acc->recvbuf_cb) {
			struct _asyncmsg_get_header *ahp;
			
			while (ahp = acc->free) {
				acc->free = ahp->next;
				asyncmsg_free(ahp);
			}
		} else {
			int i, n;
			void **buffs, *single_buf;
			
			if ((buffs = alloca(acc->num_free * sizeof(void *))) == NULL) {
				buffs = &single_buf;
				n = 1;
			} else {
				n = acc->num_free;
			}
			
			do {
				for (i = 0; i < n; i++) {
					buffs[i] = acc->free;
					acc->free = acc->free->next;
				}
				acc->recvbuf_cb(acc->buffer_size + sizeof(iov_t) + sizeof(struct _asyncmsg_get_header),
								n, buffs, ASYNCMSG_RECVBUF_FREE);
				acc->num_free -= n;
			} while (acc->num_free);
		}
	}
	free(acc->iovs);
	free(acc);
	return 0;
}

__SRCVERSION("asyncmsg_channel_destroy.c $Rev: 153052 $");
