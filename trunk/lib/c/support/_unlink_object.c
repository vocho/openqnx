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
#include <alloca.h>
#include <unistd.h>
#include <fcntl.h>
#include <share.h>
#include <sys/iofunc.h>

int _unlink_object(const char *name, const char *prefix, unsigned file_type) {
	char						*path;
	int							fd;
	int							status;

	if(*name != '/' || strchr(name + 1, '/')) {
		path = (char *)name;
	} else {
		if(!(path = (char *)alloca(strlen(prefix) + strlen(name) + 1))) {
			errno = ENOMEM;
			return -1;
		}
		strcat(strcpy(path, prefix), name);
	}

    if((fd = _connect(_NTO_SIDE_CHANNEL, path, S_IFLNK, O_NOCTTY, SH_DENYNO, _IO_CONNECT_UNLINK, 0, 0, file_type, 0, 0, 0, 0, 0, &status)) == -1) {
        return -1;
    }
    ConnectDetach(fd);
    return status;
}

__SRCVERSION("_unlink_object.c $Rev: 153052 $");
