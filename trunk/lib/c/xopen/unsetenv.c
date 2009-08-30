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
#include <string.h>
#include <errno.h>
#include "environ.h"

/*
 * POSIX unsetenv()
 */
int unsetenv(const char *name) {
	if (name == NULL || *name == '\0' || strchr(name, '=') != NULL) {
		errno = EINVAL;
		return -1;
	}
	_unsetenv(name);
	return 0;
}

/*
 * Based on the BSD4.4 call unsetenv().
 */
void _unsetenv(const char *name) {
	int		offset;
	char	**pp;
	
	_environ_lock();

	// unset environment variable
	while(_findenv(name, &offset, 0)) {		/* if set multiple times */
		for(pp = &environ[offset];; ++pp) {
			if(!(*pp = *(pp + 1))) {
				break;
			}
		}
	}

	_environ_unlock();

	return;
}

__SRCVERSION("unsetenv.c $Rev: 153052 $");
