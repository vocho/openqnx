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

//
// Be careful modifying this function. libgcc.a defines it as well and
// this version is compatible with it so it doesn't matter which one
// gets brought in.
//
void exit(int status) {
	extern void _cleanup(void);

	_cleanup();	
	_exit(status);
	#ifdef __GNUC__
		for( ;; ) ; /* Get gcc to shut up about a noreturn function returning */
	#endif
}

__SRCVERSION("exit.c $Rev: 153052 $");
