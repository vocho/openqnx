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
 * apxmgr_resmgrfuncs
 * 
 * Resource manager miscellaneous connect and I/O functions not warranting their
 * own file
 * 
*/

#include "apmgr.h"
#include "apmmgr.h"
#include "apsmgr.h"


/*******************************************************************************
 * apxmgr_chmod
*/
int apxmgr_chmod(resmgr_context_t *ctp, io_chmod_t *msg, RESMGR_OCB_T *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	apxmgr_attr_t  *p = GET_PART_ATTR(ocb);
	int  r;
	struct _client_info ci;

	if ((r = PART_ATTR_LOCK(p)) != EOK)
		return r;

	/* enforce that only the owner can change the mode */
	if (((r = iofunc_client_info(ctp, ocb->ioflag, &ci)) == EOK) &&
		((r = ((ci.cred.euid != p->attr.uid) ? EACCES : EOK)) == EOK))
		r = iofunc_chmod(ctp, msg, ocb, &p->attr);
	
	PART_ATTR_UNLOCK(p);
	return r;
}

/*******************************************************************************
 * apxmgr_chown
*/
int apxmgr_chown(resmgr_context_t *ctp, io_chown_t *msg, RESMGR_OCB_T *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	apxmgr_attr_t  *p = GET_PART_ATTR(ocb);
	int  r;
	struct _client_info ci;

	if ((r = PART_ATTR_LOCK(p)) != EOK)
		return r;

	if (((r = iofunc_client_info(ctp, ocb->ioflag, &ci)) == EOK) &&
		((r = ((ci.cred.euid != p->attr.uid) ? EACCES : EOK)) == EOK))
/*		r = iofunc_chown(ctp, msg, ocb, &p->attr); */
	{
		if ((msg->i.uid != p->attr.uid) || (msg->i.gid != p->attr.gid))
		{
			p->attr.uid = msg->i.uid;
			p->attr.gid = msg->i.gid;
			p->attr.flags |= IOFUNC_ATTR_DIRTY_OWNER;
		}
		// Mark ctime for update
		p->attr.flags |= (IOFUNC_ATTR_CTIME | IOFUNC_ATTR_DIRTY_TIME);
	}

	PART_ATTR_UNLOCK(p);
	return r;
}

/*******************************************************************************
 * apxmgr_lseek
 * 
 * This function provided primarily to support rewinddir()
*/
 #define IS32BIT(_attr, _ioflag) ((!((_ioflag) & O_LARGEFILE)) || ((_attr)->mount != NULL && (_attr)->mount->flags & IOFUNC_MOUNT_32BIT))
int apxmgr_lseek(resmgr_context_t *ctp, io_lseek_t *msg, RESMGR_OCB_T *_ocb)
{
	iofunc_ocb_t *ocb = (iofunc_ocb_t *)_ocb;
	apxmgr_attr_t  *p = (apxmgr_attr_t *)GET_PART_ATTR(ocb);
	off64_t  offset = msg->i.offset;
	unsigned dirent_max;
	int  r;

	if ((r = PART_ATTR_LOCK(p)) != EOK)
		return r;

	dirent_max = LIST_COUNT(p->children);
	if (p->type == part_type_MEMPART_REAL)
	{
		apmmgr_attr_t *mp = (apmmgr_attr_t *)p;
		mempart_t *mpart = MEMPART_ID_TO_T(mp->data.mpid);
		CRASHCHECK(mpart == NULL);
		dirent_max += LIST_COUNT(mpart->prp_list);
	}
	else if (p->type == part_type_MEMPART_REAL)
	{
		apsmgr_attr_t *sp = (apsmgr_attr_t *)p;
		schedpart_t *spart = SCHEDPART_ID_TO_T(sp->data.spid);
		CRASHCHECK(spart == NULL);
		dirent_max += LIST_COUNT(spart->prp_list);
	}

	if ((offset < 0) || (offset >= dirent_max)) {
		PART_ATTR_UNLOCK(p);
		return EINVAL;
	}

 	switch (msg->i.whence)
 	{
		case SEEK_SET:
			break;

		case SEEK_CUR:
			offset += ocb->offset;
			break;

		case SEEK_END:
			offset = dirent_max;
			break;

		default:
			PART_ATTR_UNLOCK(p);
			return EINVAL;
	}


	if (IS32BIT(&p->attr, ocb->ioflag) && (offset > LONG_MAX)) {
		PART_ATTR_UNLOCK(p);
		return(EOVERFLOW);
	}

	PART_ATTR_UNLOCK(p);

	ocb->offset = offset;

	if (msg->i.combine_len & _IO_COMBINE_FLAG) {
		return EOK;
	}

	msg->o = offset;
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}



__SRCVERSION("$IQ: apxmgr_resmgrfuncs.c,v 1.23 $");

