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
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/procmsg.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>

/*
 * waitid over network.
 */
int __waitid_net(int nd, idtype_t idtype, id_t id, siginfo_t *infop, int options) {
	proc_wait_t					msg;
	iov_t						iov[2];
	int							coid, ret;

	if(ND_NODE_CMP(nd, ND_LOCAL_NODE) == 0) {
		return waitid(idtype, id, infop, options);
	}

	if((coid = ConnectAttach(nd, SYSMGR_PID, SYSMGR_CHID, _NTO_SIDE_CHANNEL, 0)) == -1) {
		return -1;
	}
	msg.i.type = _PROC_WAIT;
	msg.i.idtype = idtype;
	msg.i.id = id;
	msg.i.options = options;
	SETIOV(iov + 0, &msg.i, sizeof msg.i);
	SETIOV(iov + 1, infop, sizeof *infop);

	ret = MsgSendv(coid, iov + 0, 1, iov + 1, infop ? 1 : 0);

	ConnectDetach(coid);

	return ret;
}

__SRCVERSION("__waitid_net.c $Rev: 153052 $");
