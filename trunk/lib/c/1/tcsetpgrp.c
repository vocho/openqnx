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




#include <termios.h>
#include <devctl.h>
#include <sys/dcmd_chr.h>
#include <errno.h>
#include <sys/types.h>

int tcsetpgrp(int fd, pid_t pgrp) {
	return _devctl(fd, DCMD_CHR_TCSETPGRP, &pgrp, sizeof pgrp, _DEVCTL_FLAG_NORETVAL | _DEVCTL_FLAG_NOTTY);
}

__SRCVERSION("tcsetpgrp.c $Rev: 153052 $");
