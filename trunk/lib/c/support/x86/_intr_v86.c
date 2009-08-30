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




#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/neutrino.h>
#include <x86/v86.h>
#include <x86/cpumsg.h>

int _intr_v86(int swi, struct _v86reg *regs, void *data, int datasize) {
	x86_cpu_v86_t				msg;
	iov_t						iov[3];

	msg.i.type = _X86_CPU_V86;
	msg.i.cputype = SYSPAGE_X86;
	msg.i.swi = swi;

	SETIOV(iov + 0, &msg, offsetof(struct _x86_cpu_v86, regs));
	SETIOV(iov + 1, regs, sizeof *regs);
	SETIOV(iov + 2, data, min(datasize, _V86_DATA_SIZE));
	return MsgSendvnc(SYSMGR_COID, iov, 3, iov, 3);
}

__SRCVERSION("_intr_v86.c $Rev: 153052 $");
