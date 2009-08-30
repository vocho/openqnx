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




#include <sys/time.h>
#include <utime.h>

int utimes(const char *path, const struct timeval tv[2]) {
    struct utimbuf ub;

    if (tv == 0) return utime(path, 0);
    ub.actime = tv[0].tv_sec, ub.modtime = tv[1].tv_sec;
    return utime(path, &ub);
}

__SRCVERSION("utimes.c $Rev: 153052 $");
