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




#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

void perror(const char *ustr) {
	char				const *estr = strerror(errno);

	if(ustr && *ustr) {
		write(STDERR_FILENO, ustr, strlen(ustr));
		write(STDERR_FILENO, ": ", sizeof ": " - 1);
	}
	write(STDERR_FILENO, estr, strlen(estr));
	write(STDERR_FILENO, "\n", sizeof "\n" - 1);
}

__SRCVERSION("perror.c $Rev: 153052 $");
