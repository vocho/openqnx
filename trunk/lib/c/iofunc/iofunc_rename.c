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




#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <share.h>
#include <sys/iofunc.h>

int iofunc_rename(resmgr_context_t *ctp, io_rename_t *msg, iofunc_attr_t *oldattr, iofunc_attr_t *olddattr,
			iofunc_attr_t *newattr, iofunc_attr_t *newdattr, struct _client_info *info) {
	struct _client_info			buff;
	int							status;
	iofunc_attr_t				save_oldattr, save_newattr, save_newdattr;

	if(!oldattr || !olddattr || !newdattr) {
		return EBADFSYS;
	}

	// Make sure we are not trying to rename to a component of the source path. (caller must check as well)
	if(oldattr == newdattr) {
		return EINVAL;
	}

	// Get client info if no info passed in
	if(!info && (status = iofunc_client_info(ctp, msg->connect.ioflag, info = &buff)) != EOK) {
		return status;
	}

	// save incase we have an error
	save_newdattr = *newdattr;

	// Is there an existing entry?
	if(newattr) {
		// If the same, return EOK. We are done (as per POSIX 5.5.3.2)
		if(newattr == oldattr) {
			return EOK;
		}

		if(S_ISDIR(newattr->mode)) {
			// If old link is a dir, and new is not, return an error
			if(!S_ISDIR(oldattr->mode)) {
				return EISDIR;
			}

			// "." is not allowed to be removed
			if(msg->connect.eflag & _IO_CONNECT_EFLAG_DOT) {
				return EINVAL;
			}

			// Directory must be empty (2 is itself and ".")
			if(oldattr->nlink > 2) {
				return ENOTEMPTY;
			}
		} else {
			// If new link is a dir, and old is not, return an error
			if(S_ISDIR(oldattr->mode)) {
				return ENOTDIR;
			}
		}

		// save incase we have an error
		save_newattr = *newattr;

		// try to unlink existing destination
		if((status = iofunc_unlink(ctp, 0, newattr, newdattr, info)) != EOK) {
			return status;
		}
	}

	// save incase we have an error
	save_oldattr = *oldattr;

	// make link to new entry
	if((status = iofunc_link(ctp, 0, oldattr, newdattr, info)) != EOK) {
		if(newattr) {
			*newattr = save_newattr;
		}
		*newdattr = save_newdattr;
		return status;
	}

	// unlink old entry
	if((status = iofunc_unlink(ctp, 0, oldattr, olddattr, info)) != EOK) {
		*oldattr = save_oldattr;
		if(newattr) {
			*newattr = save_newattr;
		}
		*newdattr = save_newdattr;
		return status;
	}

	return EOK;
}

__SRCVERSION("iofunc_rename.c $Rev: 153052 $");
