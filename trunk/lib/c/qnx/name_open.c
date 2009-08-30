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




#include <fcntl.h>
#include <share.h>
#include <errno.h>
#include <string.h>
#include <alloca.h>
#include <sys/stat.h>
#include <sys/ftype.h>
#include <sys/iomsg.h>
#include <sys/neutrino.h>
//#include "attach.h"

#include <sys/dispatch.h>

//Declared in dispatch/name.c
extern const char *__name_prefix_global;
extern const char *__name_prefix_local;

/*
 This will open up and return an fd connection
 to the client for this connection
*/
int name_open(const char *name, int flags) {
	int  mode = 0;
	char *newname;
	const char	*pref;
	unsigned	pref_len;
	unsigned	total_len;

	//Reserve starting w/ a slash for future use
	if (!name || !*name || *name == '/') {
		errno = EINVAL;
		return -1;
	}

	if (flags & NAME_FLAG_ATTACH_GLOBAL) {
		pref = __name_prefix_global;
	} else {
		pref = __name_prefix_local;
	}

	pref_len = strlen(pref);
	total_len = pref_len + strlen(name) + 1;
	newname = alloca(total_len);
	if(newname == NULL) {
		errno = ENOMEM;
		return -1;
	}
	strcpy(newname, pref);
	strcpy(&newname[pref_len], name);

	return _connect(_NTO_SIDE_CHANNEL, newname, mode, O_RDWR, 
					SH_DENYNO, _IO_CONNECT_OPEN, 1, 
					_IO_FLAG_RD | _IO_FLAG_WR, _FTYPE_NAME, 
					0, 0, 0, 0, 0, 0);
}

int name_close(int fd) {
	return ConnectDetach(fd);
}

__SRCVERSION("name_open.c $Rev: 153052 $");
