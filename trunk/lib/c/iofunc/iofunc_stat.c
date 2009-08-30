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




#define _FILE_OFFSET_BITS		64
#define _IOFUNC_OFFSET_BITS		64
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/iofunc.h>
#include <sys/netmgr.h>

int iofunc_stat(resmgr_context_t *ctp, iofunc_attr_t *attr, struct stat *pstat) {
	iofunc_mount_t			*mountp;

	pstat->st_ctime = attr->ctime;
	pstat->st_mtime = attr->mtime;
	pstat->st_atime = attr->atime;
	pstat->st_gid = attr->gid;
	pstat->st_uid = attr->uid;
	pstat->st_nlink = attr->nlink;

	pstat->st_dev = 0;
	if((mountp = attr->mount)) {
		/* If there is a mount structure, grab the dev from it */
		pstat->st_dev = mountp->dev;
		pstat->st_blocksize = mountp->blocksize ? mountp->blocksize : 1;
	} else {
		pstat->st_blocksize = 1;
	}
		
	/* If there is no dev, get one!! */
	if(pstat->st_dev == 0) {
		if(!(S_ISNAM(attr->mode))) {
			/* Name special has different meanings for rdev */
			pstat->st_dev = attr->rdev;
		}
		if(pstat->st_dev == 0) {
			if(resmgr_devino(ctp->id, &pstat->st_dev, &pstat->st_ino) != -1) {
				if(attr->rdev == 0) {
					if(!(S_ISNAM(attr->mode))) {
						/* Save for next time */
						attr->rdev = pstat->st_dev;
						attr->flags |= IOFUNC_ATTR_DIRTY_RDEV;
					}
					if(attr->inode == 0) {
						attr->inode = pstat->st_ino;
					}
				}
			}
		}
	}
	pstat->st_ino = attr->inode;
	pstat->st_dev = pstat->st_dev ? (ctp->info.srcnd << ND_NODE_BITS) | (pstat->st_dev & ND_NODE_MASK) : 0;
	pstat->st_rdev = (attr->rdev && (S_ISCHR(attr->mode) || S_ISBLK(attr->mode))) ? 
			(ctp->info.srcnd << ND_NODE_BITS) | (attr->rdev & ND_NODE_MASK) : attr->rdev;

	pstat->st_mode = attr->mode;
	pstat->st_nblocks = (attr->nbytes + pstat->st_blocksize - 1) / pstat->st_blocksize;

	pstat->st_blksize = max(pstat->st_blocksize, 512);		// Optimum I/O size
	pstat->st_blocks = (attr->nbytes + 512 - 1) / 512;		// Number of 512byte blocks (unix)

	pstat->st_size = S_ISBLK(attr->mode) ? pstat->st_blocks : attr->nbytes;

	return EOK;
}

__SRCVERSION("iofunc_stat.c $Rev: 153052 $");
