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




#include <sys/neutrino.h>
#include <sys/syspage.h>

#if 0
/*
-- This doesn't work. We need kernel mode to access the interrupt
-- controller.
*/
int
InterruptPending(void)
{
	struct intrinfo_entry	*iip;
	unsigned				num;

	iip = SYSPAGE_ENTRY(intrinfo);
	for(num = _syspage_ptr->intrinfo.entry_size / sizeof(*iip); num > 0; --num) {
		if(iip->pending && iip->pending(_syspage_ptr, iip)) return 1;
		++iip;
	}
	return 0;
}
#endif

__SRCVERSION("interruptpending.c $Rev: 153052 $");
