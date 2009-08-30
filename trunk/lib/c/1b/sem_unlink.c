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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/ftype.h>

int sem_unlink(const char *name) {
	int	rc;

	rc = _unlink_object(name, "/dev/sem", _FTYPE_SEM);
	// Semaphore's don't have directories.
	if((rc != 0) && (errno == ENOTEMPTY)) errno = ENOENT;
	return rc;
}

__SRCVERSION("sem_unlink.c $Rev: 153052 $");
