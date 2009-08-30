
/*
 * $QNXLicenseC:
 * Copyright 2009, QNX Software Systems. All Rights Reserved.
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
#include <fcntl.h>
#include "pinode.h"

static int std_open(resmgr_context_t *, io_open_t *, RESMGR_HANDLE_T *, void *);
static int std_stat(resmgr_context_t *, io_stat_t *, void *);

static resmgr_connect_funcs_t std_connect_funcs = {
	_RESMGR_CONNECT_NFUNCS,
	std_open,
};

static resmgr_io_funcs_t std_io_funcs = {
	_RESMGR_IO_NFUNCS,
	NULL,			/* read */
	NULL,			/* write */
	NULL,			/* close_ocb */
	std_stat,
};

static struct pinfo {
	const char	*path;
	int		idx;
	int		path_id;
	dev_t		devno;
} pinfo[3] = {
	{
		"/dev/stdin",
		STDIN_FILENO
	},
	{
		"/dev/stdout",
		STDOUT_FILENO
	},
	{
		"/dev/stderr",
		STDERR_FILENO
	},
};


static int
std_stat(resmgr_context_t *ctp, io_stat_t *msg, void *ocb)
{
	struct pinfo	*pp;

	pp = ocb;

	memset(&msg->o, 0x00, sizeof msg->o);
	msg->o.st_ino = PINO_STDIN + pp->idx;
	msg->o.st_size = 0;
	msg->o.st_ctime =
	msg->o.st_mtime =
	msg->o.st_atime = time(0);
	msg->o.st_mode = 0666 | S_IFCHR;
	msg->o.st_nlink = 1;
	msg->o.st_dev = (ctp->info.srcnd << ND_NODE_BITS) | path_devno;
	msg->o.st_rdev = (ctp->info.srcnd << ND_NODE_BITS) | pp->devno;
	return _RESMGR_PTR(ctp, &msg->o, sizeof msg->o);
}

static int
std_open(resmgr_context_t *ctp, io_open_t *msg, void *hdl, void *extra)
{
	struct _io_connect_link_reply	*linkp;
	struct _server_info		*info;
	io_dup_t			*new;
	unsigned			eflag;
	struct pinfo			*pp;

	linkp = (void *)msg;
	info = (void *)(linkp + 1);
	new = (void *)(info + 1);

	pp = hdl;

	if ((msg->connect.ioflag & (_IO_FLAG_RD | _IO_FLAG_WR)) == 0) {
		if (resmgr_open_bind(ctp, pp, 0) == -1) {
			return errno;
		}
		return EOK;
	}

	if (ctp->info.nd != ND_LOCAL_NODE) {
		return ENOREMOTE;
	}

	eflag = msg->connect.eflag;

	if (ConnectServerInfo(ctp->info.pid, pp->idx, info) != pp->idx) {
		return EBADF;
	}

	memset(new, 0x00, sizeof(*new));
	new->i.type = _IO_DUP;
	new->i.combine_len = sizeof(new->i);
	new->i.info.nd = netmgr_remote_nd(info->nd, ND_LOCAL_NODE);
	new->i.info.pid = ctp->info.pid;
	new->i.info.chid = info->chid;
	new->i.info.scoid = info->scoid;
	new->i.info.coid = pp->idx;

	_IO_SET_CONNECT_RET(ctp, _IO_CONNECT_RET_MSG);
	memset(linkp, 0x00, sizeof *linkp);
	linkp->eflag = eflag;
	linkp->path_len = sizeof *new;
	return _RESMGR_PTR(ctp, msg, sizeof *linkp + sizeof *info + sizeof *new);
}

void
devstd_init(void)
{
	int		i;
	struct pinfo	*pp;

	for (i = 0; i < sizeof(pinfo) / sizeof(pinfo[0]); i++) {
		pp = &pinfo[i];
		pp->path_id = resmgr_attach(dpp, NULL, pp->path,
		    _FTYPE_ANY, 0, &std_connect_funcs, &std_io_funcs,
		    pp);
		rsrcdbmgr_proc_devno("dev", &pp->devno, -1, 0);
	}
}
