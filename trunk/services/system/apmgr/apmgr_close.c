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

/*==============================================================================
 * 
 * apmgr_close
 * 
 * Provide resource manager close() processing for the partitioning module
 * 
*/

#include "apmgr.h"

/*
extern void *apmgr_ocb_watch_list[10];
static int _watched_val(void **ol, int n, void *o)
{
	unsigned i;
	for (i=0; i<n; i++)
		if (ol[i] == o)
			return !(ol[i] = NULL);
	return 0;
}
#define WATCHED_OCB(o)	_watched_val(apmgr_ocb_watch_list, NUM_ELTS(apmgr_ocb_watch_list), (o))
*/

/*
 * apmgr_close
 * 
 * Called to get rid of the OCB
*/
int apmgr_close(resmgr_context_t *ctp, void *reserved, void *_ocb)
{
	apmgr_ocb_t  *ocb = (apmgr_ocb_t *)_ocb;

	(void)iofunc_ocb_detach(ctp, &ocb->ocb);
	free(ocb);
	return EOK;
}


__SRCVERSION("$IQ: apmgr_close.c,v 1.23 $");

