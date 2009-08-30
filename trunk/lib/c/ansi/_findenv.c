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
#include <string.h>
#include <malloc.h>
#include "environ.h"

/*
 * A utility function used be getenv(), putenv() and setenv().
 */
char *_findenv(const char *name, int *offset, int grow) {
	int len, cnt, status;
	const char *p, **pp;

	status = EINVAL;
	cnt = 0;

	/* search */
	if (name && environ) {
		status = ESRCH;
		for(p = name, len = 0; *p && *p != '='; ++p, ++len) {
			/* nothing to do */
		}
		for(pp = (const char **)environ; *pp; ++pp, ++cnt) {
			if(!strncmp(*pp, name, len)) {
				if(*(p = *pp + len) == '=') {
					*offset = cnt;
					return (char *)++p;
				}
			}
		}
	}

	/* grow vector, if required */
	if (name && grow && environ) {
		char** pp2;
		static char** envp;

		status = ENOMEM;
		if(environ == envp) {					/* just increase size */
			pp2 = realloc(environ, (size_t)(sizeof(char *) * (cnt + 2)));
		} else {								/* get new space */
			pp2 = (char **)malloc((size_t)(sizeof(char *) * (cnt + 2)));
			if (pp2) {
				memcpy(pp2, environ, cnt * sizeof(char *));
			}
		}
		if (pp2) {
			environ = envp = pp2;
			*offset = cnt;
			environ[cnt + 1] = NULL;
			return environ[cnt] = "";
		}
	}

	errno = status;
	return NULL;
}


__SRCVERSION("_findenv.c $Rev: 153052 $");
