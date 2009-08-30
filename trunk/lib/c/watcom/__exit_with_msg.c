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
#include <unistd.h>
#include <string.h>

void __exit_with_msg(char *msg, int status) {
	write(STDERR_FILENO, msg, strlen(msg));
	_exit(status);
}

void __fatal_runtime_error( char *msg, int status ) {
	/* Should check __EnterWVIDEO here (but need to define that first) */
	__exit_with_msg( msg, status );
}

__SRCVERSION("__exit_with_msg.c $Rev: 153052 $");
