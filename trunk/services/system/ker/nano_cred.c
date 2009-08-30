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

void rdecl
cred_dirty(PROCESS *prp) {
	if(++prp->seq == 0) {
		VECTOR					*vec;
		int						i;

		// Don't allow the prp sequence number to be zero..
		++prp->seq;
		for(vec = &prp->fdcons;;) {
			for(i = 0; i < vec->nentries; ++i) {
				CONNECT					*cop;

				if((cop = VECP2(cop, vec, i))) {
					if(cop->type == TYPE_CONNECTION  &&
					   cop->process == prp &&
					   (cop->flags & COF_NETCON) == 0) {
						// Seq connection to zero indicate DIRTY credentials
						cop->un.lcl.seq = 0;
					}
				}
			}
			if(vec == &prp->chancons) {
				break;
			}
			vec = &prp->chancons;
		}
	}
}

int rdecl
cred_set(CREDENTIAL **pcrp, struct _cred_info *cip) {
	CREDENTIAL			*crp = *pcrp;
	unsigned			 size;
	struct _cred_info	 info;

	size = 0;
	if(cip) {
		// Check to see if the number of groups passed is more
		// than we were compiled for.
		if(cip->ngroups > NUM_ELTS(cip->grouplist)) {
			// Only remember the number of groups set a compile time.
			info = *cip;
			info.ngroups = NUM_ELTS(cip->grouplist);
			cip = &info;
		}
		size = offsetof(struct _cred_info, grouplist) + (cip->ngroups * sizeof(cip->grouplist[0]));
	}

	if(crp) {
#ifndef NDEBUG
		if(crp->links == 0) crash();
#endif

		// If there was no change ignore the request.
		if(cip != NULL  &&  memcmp(&crp->info, cip, size) == 0) {
			return(EOK);
		}
		
		// Decrement the link count and free if not used any more.
		if(--crp->links == 0) {
			LINK1_REM(credential_list, crp, CREDENTIAL);
			object_free(0, &credential_souls, crp);
		}
	}

	// A NULL pointer is a request to unlink the entry from the process.
	if(cip == NULL) {
		*pcrp = NULL;
		return(EOK);
	}

	// We need a new credential entry.
	// Try and find an existing match and link to it first.
	for(crp = credential_list ; crp ; crp = crp->next) {
		if(crp->info.ruid == cip->ruid) {
			if(memcmp(&crp->info, cip, size) == 0) {
				++crp->links;
				*pcrp = crp;

				return(EOK);
			}
		}
	}

	// No match so we need to create a new entry.
	if((crp = object_alloc(0, &credential_souls)) == NULL) {
		return(EAGAIN);
	}

	crp->links = 1;
	memcpy(&crp->info, cip, size);
	crp->next = credential_list;
	credential_list = crp;
	*pcrp = crp;
	
	return(EOK);
}

__SRCVERSION("nano_cred.c $Rev: 153052 $");
