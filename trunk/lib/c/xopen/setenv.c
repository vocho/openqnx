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
#include <errno.h>
#include <string.h>
#include "environ.h"

/*
 * Based on the BSD4.4 call setenv().
 */
int setenv(const char *name, const char *value, int rewrite) {
	char *p, *temp;
	int	offset, rc;

	if (value == NULL) {
		_unsetenv(name);
		return 0;
	}

	_environ_lock();

	if (!rewrite && _findenv(name, &offset, 0)) {
		/* see comments in putenv() about why we cannot overwrite */
		_environ_unlock();
		return 0;
	}

	if ((name == NULL) || (environ == NULL)) {
		_environ_unlock();
		errno = EINVAL;
		return -1;
	}

	if(*value == '=') {								/* no `=' in value */
		++value;
	}
	for(p = (char *)name; *p && *p != '='; ++p) {	/* no `=' in name */
		// nothing to do
	}
	if(!(temp = malloc((size_t)((int)(p - name) + strlen(value) + 2)))) {		/* name + `=' + value */
		_environ_unlock();
		errno = ENOMEM;
		return -1;
	}
	for(p = temp; (*p = *name++) && *p != '='; ++p) {
		//nothing to do
	}
	for(*p++ = '='; (*p++ = *value++);) {
		//nothing to do
	}

	if (_findenv(temp, &offset, 1)) {
		environ[offset] = temp;
		rc = 0;
	} else {
		free(temp);
		rc = -1;
	}

	_environ_unlock();
	return rc;
}


__SRCVERSION("setenv.c $Rev: 153052 $");
