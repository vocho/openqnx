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
#include <stdio.h>
#include <share.h>

_STD_BEGIN

FILE *fopen64(const char *filename, const char *mode) {
extern FILE *__fsopen(const char *filename, const char *mode, int sflag, int large);

	return(__fsopen(filename, mode, SH_DENYNO, O_LARGEFILE));
}

_STD_END

__SRCVERSION("fopen64.c $Rev: 153052 $");
