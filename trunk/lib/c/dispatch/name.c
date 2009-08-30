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
#include <malloc.h>
#include <stddef.h>
#include <stdlib.h>
#include <share.h>
#include <fcntl.h>
#include <process.h>
#include <sys/iomsg.h>
#include <sys/resmgr.h>
#include <sys/pathmsg.h>
#include <sys/dispatch.h>
#include <sys/netmgr.h>
#include <string.h>
#include <stdio.h>
#include "dispatch.h"
#include "resmgr.h"

const char *__name_prefix_global = "/dev/name/global/";
const char *__name_prefix_local = "/dev/name/local/";

name_attach_t *name_attach(dispatch_t *dpp, const char *path, unsigned flags) {
	name_attach_t *attach;
	int auto_create, mntid, chid;
	char *newname;

	//Reserve leading /'s for future use
	if (!path || !*path || *path == '/') {
		errno = EINVAL;
		return NULL;
	}

	//Don't allow ../ or .. attaches to escape
	if ((newname = (char *)strstr(path, "..")) && 
	    (newname[2] == '/' || newname[2] == '\0')) {
		errno = EINVAL;
		return NULL;
	}
	newname = NULL;

	mntid = chid = -1;
	auto_create = (dpp) ? 0 : 1;

	if (!dpp && !(dpp = dispatch_create())) {
		errno = EINVAL;
		return NULL;
	}

	if (flags & NAME_FLAG_ATTACH_GLOBAL) {
		if (!(newname = alloca(strlen(__name_prefix_global) + strlen(path) + 1))) {
			errno = ENOMEM;
			goto failure;
		}
		strcpy(newname, __name_prefix_global);
	}
	else {
		if (!(newname = alloca(strlen(__name_prefix_local) + strlen(path) + 1))) {
			errno = ENOMEM;
			goto failure;
		}
		strcpy(newname, __name_prefix_local);
	}
	strcat(newname, path);

	//TODO: Make the last thing really an attribute matching us?
	//TODO: Allow the user to specify a function they want to be called
    
    /* We have to regist with global service manager, this is done by:
	 *  1) do an empty resmgr_attach() to setup things (in case dpp->chid isn't there)
	 *  2) send an _IO_CONNECT_OPEN, _IO_CONNECT_EXTRA_RESMGR_LINK, to the
	 *     manager. (What we really want is _IO_CONENCT_LINK plus
	 *    _IO_CONNECT_EXTRA_RESMGR_LINK, but PathMgr won't pass it down to GNS
	 *  3) if 2) faild, there is no local GNS exist, then ONLY IF the attach
	 *     if for _NAME_FLAG_ATTACH_LOCAL, we will send a _IO_CONNECT_LINK, 
	 *     _IO_CONNECT_EXTRA_RESMGR_LINK, as if pathmgr_link() did, this will 
	 *     regist it with pathmgr. And everything still works in local nodes.
	 */
	{
		unsigned nflag;
		struct link *linkl;
		struct _io_resmgr_link_extra extra;
			
		/* make sure no real attach will happen */
		nflag = flags & ~_RESMGR_FLAG_FTYPEONLY;
		if (resmgr_attach(dpp, NULL, NULL, 0, nflag, NULL, NULL, NULL) == -1) {
			goto failure;
		}
			
		if (!(linkl = _resmgr_link_alloc())) {			
			goto failure;
		}
		linkl->connect_funcs = 0;
		linkl->io_funcs = 0;
		linkl->handle = 0;
		
		extra.nd = ND_LOCAL_NODE;
		extra.pid = getpid();
		extra.chid = dpp->chid;
		extra.handle = mntid = linkl->id;
		extra.file_type = _FTYPE_NAME;
		extra.flags = flags & _RESMGR_FLAG_MASK;
		linkl->link_id = _connect(PATHMGR_COID, newname, 0, 0,
								 SH_DENYNO, _IO_CONNECT_OPEN, 0, 0, _FTYPE_NAME,
								 _IO_CONNECT_EXTRA_RESMGR_LINK, sizeof extra, &extra,
								 0, 0, 0);
		
		if (linkl->link_id == -1 && !(flags & NAME_FLAG_ATTACH_GLOBAL) &&
			(errno == ENOTSUP || errno == ENOENT || errno == ENOSYS))
		{
			linkl->link_id = _connect(PATHMGR_COID, newname, 0, 0,
									 SH_DENYNO, _IO_CONNECT_LINK, 0, 0, _FTYPE_NAME,
									 _IO_CONNECT_EXTRA_RESMGR_LINK, sizeof extra, &extra,
									 0, 0, 0);
		}

		if (linkl->link_id == -1) {
			_resmgr_link_free(linkl->id, _RESMGR_DETACH_ALL);
			mntid = -1;
			goto failure;
		}
		linkl->flags &= ~_RESMGR_LINK_HALFOPEN;
	}
	
	chid = _DPP(dpp)->chid;
	
	if (!(attach = malloc(sizeof(*attach)))) {
		errno = ENOMEM;
		goto failure;
	}	
	attach->dpp = dpp;
	attach->chid = chid;
	attach->mntid = mntid;
	
	return attach;
	
failure:
	if (mntid > 0) {
		(void)resmgr_detach(dpp, mntid, 0);
	}
	if (auto_create && dpp) {
		(void)dispatch_destroy(dpp);
	}
	return NULL;
}

/*
 Most of the time w/ these function the user wants the
 dpp to be autocreated, so the default is to destroy 
 it when we detach.
*/
int name_detach(name_attach_t *attach, unsigned flags) {
	int ret = EOK;
	if (attach) {
		ret = resmgr_detach(attach->dpp, attach->mntid, 0);
		if (!(flags & NAME_FLAG_DETACH_SAVEDPP)) {
			(void)dispatch_destroy(attach->dpp);
		}
		free(attach);
	}
	return ret;
}

#if 0
/* TF NOTE: Can we do away with these all together? */
resmgr_context_t *name_context_alloc(dispatch_t *dpp) {
	return resmgr_context_alloc(dpp);
}

void name_context_free(resmgr_context_t *ctp) {
	resmgr_context_free(ctp);
}

resmgr_context_t *name_block(resmgr_context_t *ctp) {
	return resmgr_block(ctp);
}

int name_handler(resmgr_context_t *ctp) {
	_resmgr_handler(ctp);
	return 0;
}
#endif

__SRCVERSION("name.c $Rev: 153052 $");
