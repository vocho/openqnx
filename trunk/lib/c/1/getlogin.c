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




#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pwd.h>

int getlogin_r(char *name, size_t namesize)
{
	struct passwd			pwd_buffer, *pwd;
	char					buffer[BUFSIZ+1];
	int						ret;
	char					*logname;

	if(!(logname = getenv("LOGNAME"))) {
		if (namesize <= 1) {
			return ERANGE;
		}
		if((ret = getpwuid_r(getuid(), &pwd_buffer, buffer, sizeof buffer, &pwd)) != EOK) {
			return ret;
		}

		if(!pwd) {
			*name = '\0';
			return 0;
		}
		logname = pwd->pw_name;
	}
	if(strlen(logname) + 1 > namesize) {
		return ERANGE;
	}
	strcpy(name, logname);
	return 0;
}

char *getlogin(void)
{
	static char				*buffer;
	static long				max;
	char					*name;
	int						ret;

	if(!(name = buffer)) {
		ret = errno;
		if((max = sysconf(_SC_LOGIN_NAME_MAX)) <= 0) {
#ifdef LOGIN_NAME_MAX
			max = LOGIN_NAME_MAX;
#else
			max = 32;			// Pick some kind of default???
#endif
		}
		errno = ret;
		if(!(name = buffer = malloc(++max))) {
			max = 0;
			errno = ENOMEM;
			return NULL;
		}
	}

	errno = 0;
	if((ret = getlogin_r(name, max)) != EOK) {
		errno = ret;
		return 0;
	}

	if(*name == '\0') {
		errno = EOK;
		return NULL;
	}

	return name;
}

__SRCVERSION("getlogin.c $Rev: 153052 $");
