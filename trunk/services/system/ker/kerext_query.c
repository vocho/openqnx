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

#include "externs.h"


struct kerargs_query_object {
	int			type;
	unsigned	index1;
	int			subtype;
	unsigned	index2;
	unsigned	*next;
	void		*objbuf;
	int			objsize;
	unsigned		restarts;
};

#define MAX_RESTARTS	10

static void
kerext_query_object(void *data) {
	struct kerargs_query_object 	*kap = data;
	THREAD	*act = actives[KERNCPU];
	void	*obj;

	if(++kap->restarts > MAX_RESTARTS) {
		// The interrupt rate is high enough that we're not able to
		// complete the nano_query without restarting. Lock the kernel
		// so we can get the job done. Increases latency, but at least
		// we'll complete the operation
		lock_kernel();
	}
	obj = nano_query(kap->type, kap->index1, kap->subtype,
				kap->index2, (unsigned *)kap->next, kap->objbuf, kap->objsize);
	lock_kernel();
	if((obj != NULL) 
	  && (kap->type == _QUERY_PROCESS) 
	  && (kap->subtype == _QUERY_PROCESS_VECTOR)) {
		// We got a PROCESS pointer back. We have to mark it as having a
		// query in-process so that the ProcessDestroy() code doesn't
		// free the memory for it while some other thread is looking at 
		// the fields.
		((PROCESS *)obj)->querying = 1;
	}

	SETKSTATUS(act,obj);
}

void *QueryObject( int type, unsigned index1, int subtype,
			unsigned index2, unsigned *next, void *objbuf, int objsize ) {

	struct kerargs_query_object	data;

	data.type = type;
	data.index1 = index1;
	data.subtype = subtype;
	data.index2 = index2;
	data.next = next;
	data.objbuf = objbuf;
	data.objsize = objsize;
	data.restarts = 0;
	return( (void *)__Ring0( kerext_query_object, &data ) );
}


void 
QueryObjectDone(void *obj) {
	// Who ever called QueryObject on a process is done looking at
	// the fields and it's OK for a ProcessDestroy() to happen
	((PROCESS *)obj)->querying = 0;
}

__SRCVERSION("kerext_query.c $Rev: 204178 $");
