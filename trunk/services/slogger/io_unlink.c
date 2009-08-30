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



 
#include "externs.h"


int io_unlink(resmgr_context_t *ctp, io_unlink_t *msg, RESMGR_HANDLE_T *handle, void *extra) {

	// We use this to clear all event buffers. We don't unlink the device.
	// It was convenient to use this function to clear the event buffers
	// so the user could issue a simple command of
	// # rm /dev/slog           - from the shell
	// unlink("/dev/slog")      - from a C program

	slogger_init(&SlogDev);
	check_overrun(&SlogDev, NULL, -1);

	return(EOK);
}

__SRCVERSION("io_unlink.c $Rev: 153052 $");
