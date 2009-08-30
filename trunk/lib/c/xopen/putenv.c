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
 * UNIX98 implementation of putenv().
 */
int putenv(char *str) {
	int	rc, offset;
	char *p;

	/* qssl extension for portability */
	if ((p = (char *) strchr(str, '=')) == NULL) {
		errno = EINVAL;
		return -1;
	}

	/* qssl extension for portability */
	if (*(p+1) == '\0') {
		_unsetenv(str);
		return 0;
	}

	/*
	 * unix98's putenv() adds the given string to the environment -- it
	 * does not make a copy if it.  The user is responsible for memory
	 * management, not the library.
	 *
	 * This is also why setenv() cannot alter an existing variable -- this
	 * memory belongs to the user and could be readonly, etc.
	 */

	rc = -1;
	_environ_lock();
	if (_findenv(str, &offset, 1)) {
		rc = 0;
		environ[offset] = (char*) str;
	}
	_environ_unlock();

	return rc;
}

__SRCVERSION("putenv.c $Rev: 166470 $");
