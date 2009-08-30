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
#include <sys/stat.h>
#include <sys/ipc.h>

key_t ftok(const char *path, int id)
{
	struct stat sbuf;

	if (stat(path, &sbuf) == -1) {
		return (key_t) -1;
	}

	return (key_t) ((id & 0xFF) << 24 |
		(sbuf.st_dev & 0xFFF) << 12 |
		(sbuf.st_ino & 0xFFF));
}

__SRCVERSION("ftok.c $Rev: 153052 $");
