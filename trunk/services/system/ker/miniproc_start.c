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
#include <fcntl.h>
#include <inttypes.h>
#include <sys/sysmsg.h>
#include <sys/procmsg.h>
#include <sys/pathmsg.h>
#include <sys/memmsg.h>
#include <sys/dcmd_chr.h>
#include "apm.h"

static union _miniproc_msgs {
	uint16_t						type;
	mem_map_t						memmap;
	mem_ctrl_t						memctrl;
	sys_cmd_t						syscmd;
	io_write_t						write;
	io_devctl_t						devctl;
	io_open_t						open;
	char							buf[sizeof(struct _io_connect) + 100];
} __msg;

static struct _path_link {
	struct _path_link	*next;
	uint32_t			 nd;
	int					 pid;
	int					 chid;
	unsigned			 handle;
	char 				 path[1];
} *path_link_list;


static int
path_link(char *path, struct _io_resmgr_link_extra *lep) {
	struct _path_link			*prev, *cur, *new;
	char						*ecp;			// existing
	const char					*ncp;			// new
	size_t  slen;

	// Step through each existing path.
	for(prev = (void *) &path_link_list ; (cur = prev->next) ; prev = cur) {
		// Find characters in common.
		for(ecp = cur->path, ncp = path; *ecp  &&  *ncp == *ecp; ++ecp, ++ncp) {
			/* nothing to do */
		}

		// Is this existing path a subset of the new path?
		if(*ecp == '\0'  &&  (*ncp == '/' || ecp == cur->path + 1)) {
			break;
		}
	}

	// Create a new link.
	slen = strlen(path);		// '\0' char is included in sizeof(*new->path)
	if((new = (void *)_smalloc(sizeof(*new) + slen)) == NULL) {
		return ENOMEM;
	}

	new->nd     = lep->nd;
	new->pid    = lep->pid;
	new->chid   = lep->chid;
	new->handle = lep->handle;
	memcpy(new->path, path, slen + 1);	// '\0' guaranteed to be copied
	new->next = cur;
	prev->next = new;

	return EOK;
}


static int
path_resolve(char *path, struct _io_connect_entry *enp) {
	struct _path_link			*plp;

	// Step through each existing path.
	for(plp = path_link_list ; plp ; plp = plp->next) {
		if(strncmp(path, plp->path, strlen(plp->path)) == 0) {
			enp->pid    = plp->pid;
			enp->nd     = plp->nd;
			enp->chid   = plp->chid;
			enp->handle = plp->handle;
			return EMORE;
		}
	}
	return ENOENT;
}

static void
do_miniproc(void) {
	union _miniproc_msgs	*msp = &__msg;
	int						rcvid, nbytes;
	int						off, n;
	char					*cp;
	int						err, status;

	for(;;) {
		rcvid = MsgReceive(SYSMGR_CHID, msp, sizeof(*msp), NULL);
		nbytes = 0;

		status = 0;
		err = EOK;
		switch(msp->type) {
		case _IO_WRITE:
			off = sizeof(msp->write);	// Step over the header
			cp = off + (char *) msp;
			status = nbytes = msp->write.i.nbytes;
			while(nbytes) {
				if((n = MsgRead(rcvid, cp, min(nbytes, sizeof(*msp)-sizeof(msp->write)), off)) <= 0)
					break;
				scrn_display(cp, n);
				off += n;
				nbytes -= n;
			}
			status -= nbytes;
			nbytes = 0;
			break;

		case _IO_DEVCTL:
			err = (msp->devctl.i.dcmd == DCMD_CHR_ISATTY) ? EOK : ENOSYS;
			break;

		case _IO_CONNECT: {
			struct {
				struct _io_connect_link_reply	reply;
				struct _io_connect_entry		entry;
			}								rmsg;
			IOV								iov[2];
			
			SETIOV(iov + 0, &rmsg, sizeof(rmsg));
			SETIOV(iov + 1, msp->open.connect.path, msp->open.connect.path_len);
			memset(&rmsg, 0, sizeof(rmsg));
			status |= msp->open.connect.file_type;
			rmsg.reply.path_len = msp->open.connect.path_len;

			if(msp->open.connect.subtype == _IO_CONNECT_LINK) {
				switch(msp->open.connect.extra_type) {
				case _IO_CONNECT_EXTRA_RESMGR_LINK:
					err = path_link((char *)msp->open.connect.path,
							(void *)&msp->open.connect.path[msp->open.connect.path_len]);
					break;
				case _IO_CONNECT_EXTRA_PROC_SYMLINK:
					//NYI: handle creation of a symlink....
					//check out pathmgr_link.c:pathmgr_link() in proc
				default:
					err = ENOSYS;
					break;
				}
			} else {
				rmsg.reply.nentries = 1;
				err = path_resolve((char *)msp->open.connect.path, &rmsg.entry);
			}

			if(err == EOK)
				MsgReplyv(rcvid, status, iov, 2);
			else
				MsgError(rcvid, err);
			nbytes = -1;
			break;
		}

		case _MEM_MAP:
			err = memmgr.mmap(0, msp->memmap.i.addr, msp->memmap.i.len,
						msp->memmap.i.prot, msp->memmap.i.flags,
						0, (unsigned)msp->memmap.i.offset, msp->memmap.i.align, 
						msp->memmap.i.preload, msp->memmap.i.fd,
						(void **)&msp->memmap.o.real_addr, (size_t *)&msp->memmap.o.real_size,
						mempart_getid(NULL, sys_memclass_id)); // FIX ME - this ok for miniproc ?
			msp->memmap.o.addr = msp->memmap.o.real_addr;
			nbytes = sizeof(msp->memmap.o);
			break;

		case _MEM_CTRL:
			switch(msp->memctrl.i.subtype) {
			case _MEM_CTRL_UNMAP:
				err = memmgr.munmap(0, msp->memctrl.i.addr, msp->memctrl.i.len, msp->memctrl.i.flags,
									mempart_getid(NULL, sys_memclass_id));
				break;

			case _MEM_CTRL_SYNC:
				err = memmgr.msync(0, msp->memctrl.i.addr, msp->memctrl.i.len, msp->memctrl.i.flags);
				break;

			default:
				err = ENOSYS;
				break;
			}
			break;

		case _SYS_CMD:
			switch(msp->syscmd.i.cmd) {
			case _SYS_CMD_REBOOT:
				RebootSystem(0);
				break;

			default:
				err = ENOSYS;
				break;
			}
			break;

		default:
			err = ENOSYS;
			break;
		}

		if(nbytes >= 0) {
			if(err != EOK) {
				MsgError(rcvid, err);
			} else {
				MsgReply(rcvid, status, msp, nbytes);
			}
		}
	}
}

static void *
_miniproc(void *dummy) {
	do_miniproc();
	return NULL;
}

void
miniproc_start() {
	struct	_thread_attr	attr;

	// We need to hold open the first spot for SYSMGR_COID so we grab
	// then free an extra channel.
	(void)ChannelCreate(0);			// MUST BE SYSMGR_CHID

	// Allocate the process manager connection (MUST BE SYSMGR_COID)
	ConnectAttach(0, SYSMGR_PID, SYSMGR_CHID, SYSMGR_COID, 0);

	// Allocate stdin, stdout and stderr fd's
	ConnectAttach(0, SYSMGR_PID, SYSMGR_CHID, 0, 0);
	ConnectAttach(0, SYSMGR_PID, SYSMGR_CHID, 1, 0);
	ConnectAttach(0, SYSMGR_PID, SYSMGR_CHID, 2, 0);

	// Start miniproc
	memset(&attr, 0, sizeof(attr));
	attr.__stacksize = 1024;
	(void)ThreadCreate(0, &_miniproc, NULL, &attr);
}

__SRCVERSION("miniproc_start.c $Rev: 156323 $");
