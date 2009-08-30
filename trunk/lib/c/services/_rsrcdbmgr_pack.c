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




#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <inttypes.h>
#include <sys/neutrino.h>
#include <sys/rsrcdbmgr.h>
#include <sys/rsrcdbmsg.h>

/* This function is a generic packing function.  It assumes that
you are going to send out data which is fixed width and contains
a character pointer inside of it at noff inside of each dwidth
elements.
   The data is packaged up and sent off as follows:
   [header]
   [data1]
   [data2]
   ...
   [name1]
   [name2]

	The arguments are a pointer to the header, the size of the header
in bytes and the offset within the header to the nbytes field which
is filled in with the size of the message MINUS the header.  The data
pointer follows with the width of the data structure and the offset within
each data structure to the name field.  The number of data structures
sent is nelm.  The final two arguments are for the reply and the number
of reply bytes which can be accepted.
*/
int _rsrcdbmgr_pack(void *header, int hbytes, int nbytes_off,
					void *data, int dwidth, int noff, int nelm,
					void *reply, int rbytes) {
	iov_t	*iovout, iovreply;
	char	**nameptr;
	int		 ret, i, index, nbytes;

	nbytes = index = 0;

	/* We need 2 iov's (header + data) + one for each string */
	if(!(iovout = malloc((2 + nelm) * sizeof(*iovout)))) {
		return -1;
	}

	/* Header */
	SETIOV(&iovout[index], header, hbytes);	
	index++;

	/* Data */
	SETIOV(&iovout[index], data, dwidth * nelm);	
	nbytes += dwidth * nelm;
	index++;

	/* Names */
	for(i=0; i<nelm; i++) {
		nameptr = (char **)((char *)data + (i*dwidth) + noff);
		SETIOV(&iovout[index], nameptr[0], strlen(nameptr[0]) + 1);	
		nbytes += strlen(nameptr[0]) + 1;
		index++;
	}

	/* Reply */
	SETIOV(&iovreply, reply, rbytes);	

	*((int *)((char *)header + nbytes_off)) = nbytes;

	if((ret = MsgSendv(RSRCDBMGR_COID, iovout, index, &iovreply, 1)) != -1) {
		/* We need to restore the name pointers when we come out */
		for(i=0; i<nelm; i++) {
			nameptr = (char **)((char *)data + (i*dwidth) + noff);
			nameptr[0] = GETIOVBASE(&iovout[i+2]);
		}
	}

	free(iovout);

	return ret;
}

/*** Resource Commands ***/
#if 0
int rsrcdbmgr_create(rsrc_alloc_t *list, int32_t count) {
    rsrc_cmd_t  request;
	int			ret;

	if (!list || !count) {
		errno = EINVAL;
		return(-1);
	}

	request.i.type =  RSRCDBMGR_RSRC_CMD;
	request.i.subtype = RSRCDBMGR_REQ_CREATE;
	request.i.pid = 0;
	request.i.count = count;
	//request.i.nbytes set above

	ret = _rsrcdbmgr_pack(&request, sizeof(request), offsetof(struct _rsrc_cmd, nbytes),
						   list, sizeof(*list), offsetof(rsrc_alloc_t, name), count,
						   NULL, 0);
	printf("Create %d ret %d fl 0x%08x 0x%llx-0x%llx \n", 
			count, ret,
			list[0].flags, list[0].start, list[0].end);
	return ret;
}

int rsrcdbmgr_destroy(rsrc_alloc_t *list, int32_t count) {
	rsrc_cmd_t  request;
	int			ret;

	if (!list || !count) {
		errno = EINVAL;
		return(-1);
	}

	request.i.type =  RSRCDBMGR_RSRC_CMD;
	request.i.subtype = RSRCDBMGR_REQ_DESTROY;
	request.i.pid = 0;
	request.i.count = count;
	//request.i.nbytes set above
	
	ret = _rsrcdbmgr_pack(&request, sizeof(request), offsetof(struct _rsrc_cmd, nbytes),
						   list, sizeof(*list), offsetof(rsrc_alloc_t, name), count,
						   NULL, 0);
	printf("Destroy %d ret %d fl 0x%08x 0x%llx-0x%llx \n", 
			count, ret,
			list[0].flags, list[0].start, list[0].end);
	return ret;
}

int rsrcdbmgr_attach(rsrc_request_t *list, int32_t count) {
	rsrc_cmd_t	request;
	int			ret;

	if (!list || !count) {
		errno = EINVAL;
		return -1;
	}

	request.i.type =  RSRCDBMGR_RSRC_CMD;
	request.i.subtype = RSRCDBMGR_REQ_ATTACH;
	request.i.pid = 0;
	request.i.count = count;
	//request.i.nbytes set in the pack code

	ret = _rsrcdbmgr_pack(&request, sizeof(request), offsetof(struct _rsrc_cmd, nbytes),
						   list, sizeof(*list), offsetof(rsrc_request_t, name), count,
						   list, count * sizeof(*list));
	printf("Attach %d ret %d fl 0x%08x 0x%llx-0x%llx \n", 
			count, ret,
			list[0].flags, list[0].start, list[0].end);
	return ret;
}

int rsrcdbmgr_detach(rsrc_request_t *list, int32_t count) {
    rsrc_cmd_t  request;
	int			ret;

	if (!list || count <= 0) {
		errno = EINVAL;
		return -1;
	}

	request.i.type =  RSRCDBMGR_RSRC_CMD;
	request.i.subtype = RSRCDBMGR_REQ_DETACH;
	request.i.pid = 0;
	request.i.count = count;
	//request.i.nbytes set in pack code
		
	ret = _rsrcdbmgr_pack(&request, sizeof(request), offsetof(struct _rsrc_cmd, nbytes),
						   list, sizeof(*list), offsetof(rsrc_request_t, name), count,
						   NULL, 0);
	printf("Detach %d ret %d fl 0x%08x 0x%llx-0x%llx \n", 
			count, ret,
			list[0].flags, list[0].start, list[0].end);
	return ret;
}
#endif


__SRCVERSION("_rsrcdbmgr_pack.c $Rev: 153052 $");
