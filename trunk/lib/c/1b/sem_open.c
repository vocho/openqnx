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



#include <stdarg.h>
#include <fcntl.h>
#include <inttypes.h>
#include <semaphore.h>

extern sem_t *_nsem_open(const char *, int, mode_t, unsigned);

sem_t *sem_open(const char *name, int oflag, ...) {
	va_list			ap;
	mode_t			mode;
	unsigned		value;

	va_start(ap, oflag);
	if (oflag & O_CREAT) {
		mode = va_arg(ap, mode_t);
		value = va_arg(ap, unsigned int);
	}
	else {
		mode = 0;
		value = 0;
	}
	va_end(ap);
	return _nsem_open(name, (oflag & ~O_ACCMODE) | O_RDWR, mode, value);
}

__SRCVERSION("sem_open.c $Rev: 153052 $");
