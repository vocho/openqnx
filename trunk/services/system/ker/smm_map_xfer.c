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
#include <unistd.h>

int
smm_map_xfer(PROCESS *actprp, PROCESS *prp,  IOV **piov, int *pparts, int *poff, IOV *niov, int *pnparts, unsigned flags) {
	IOV *iov;
	int parts, bytes, off;

	iov = *piov;
	parts = *pparts;
	off = *poff;

	while(off >= GETIOVLEN(iov)) {						
		off -= GETIOVLEN(iov);							
		if(--parts == 0) { 							
			*pnparts = 0;
			*pparts = 0;
			return 0;									
		}												
		++iov;											
	}												

	*pnparts = parts;
	bytes = 0;
	SETIOV(niov, (uintptr_t)GETIOVBASE(iov) + off, GETIOVLEN(iov) - off);
	bytes += GETIOVLEN(niov);
	parts --; iov++; niov++;
	for(;parts; parts--) {
		bytes += GETIOVLEN(iov);
		*niov++ = *iov++;
	}

	*pparts = 0;

	return bytes;
}

__SRCVERSION("smm_map_xfer.c $Rev: 153052 $");
