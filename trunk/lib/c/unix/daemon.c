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
#include <stdlib.h>
#include <fcntl.h>
#include <paths.h>

int daemon(int nochdir, int noclose) {
	int dev_null_fd;
	pid_t pf;

	if (!nochdir) {
		if (chdir("/") == -1) {
			return -1;
		}
	}

	if (!noclose) {
		if ((dev_null_fd = open(_PATH_DEVNULL, O_RDWR | O_NOCTTY)) == -1)
			return -1;

		if (dup2(dev_null_fd, STDIN_FILENO) == -1 || 
				dup2(dev_null_fd, STDOUT_FILENO) == -1 || 
				dup2(dev_null_fd, STDERR_FILENO) == -1) {
			return -1;
		}

		if (dev_null_fd > 2)
			close(dev_null_fd);
	}

	if (( pf = fork() )) {
		if (pf == -1) {
			return -1;	
		} else {
			exit(0);
		}
	} else {
		if (setsid() == -1) {
			return -1;
		}
	}
	
	return 0;
}

__SRCVERSION("daemon.c $Rev: 153052 $");
