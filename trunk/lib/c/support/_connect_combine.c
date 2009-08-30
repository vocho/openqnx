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
#include <sys/iomsg.h>

int _connect_combine(const char *path, mode_t mode, unsigned oflag, unsigned sflag, int testcancel,
		unsigned file_type, unsigned extra_len, void *extra, unsigned response_len, void *response) {
	int							fd;
	int							status;
	
	if((fd = _connect(_NTO_SIDE_CHANNEL, path, mode, oflag, sflag, _IO_CONNECT_COMBINE_CLOSE, testcancel, 0, file_type,
				0, extra_len, extra, response_len, response, &status)) == -1) {
		return -1;
	}
	ConnectDetach(fd);
	return status;
}

__SRCVERSION("_connect_combine.c $Rev: 153052 $");
