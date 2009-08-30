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
 * apsmgr_close
 * 
 * Provide resource manager close() processing for the scheduler partitioning
 * module
 * 
*/

#include "apsmgr.h"

/*
 * apsgr_close
 * 
 * Called to get rid of the OCB
*/
int apsmgr_close(resmgr_context_t *ctp, void *reserved, RESMGR_OCB_T *_ocb)
{
	iofunc_ocb_t  *ocb = (iofunc_ocb_t *)_ocb;
	apxmgr_attr_t  *mp = GET_PART_ATTR(ocb);

	if (mp->type == part_type_SCHEDPART_REAL) {
		(void)unregister_events(mp, ocb, NUM_SCHEDPART_EVTTYPES);
	}

	(void)iofunc_ocb_detach(ctp, ocb);
	free(ocb);
	return EOK;
}


__SRCVERSION("$IQ: apsmgr_close.c,v 1.23 $");

