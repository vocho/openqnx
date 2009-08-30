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




#include <stdlib.h>
#include "init.h"

/*
 * This is the format of arguments for main() as passed in
 * from the OS.
 */
/* This is for backwards compability with 2.0 compiled apps. */
int		_wchar_short;

void _CMain(int argc, char *argv[], char *envp[], void *auxp, void *arg_atexit) {
	/* 
	 * Initialize libc
	 */
	_init_libc(argc, argv, envp, auxp, arg_atexit);

	_wchar_short = 1;

	/*
	 * Run main() with those arguments, and pass the return
	 * value back to exit().
	 */
	exit(main(argc, argv, envp));
}

__SRCVERSION("_CMain.c $Rev: 153052 $");
