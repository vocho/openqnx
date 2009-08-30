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
 * from the OS.  It's layed on the stack, and extracted using
 * this overlay in _CMain
 */

/* This is for backwards compability with 2.0 compiled apps. */
int		_wchar_short;

#ifdef __WATCOMC__
#pragma aux _CMain aborts;
#endif
void _CMain(int argc, char *args, ...) {
	register char			**argv, **envp, **auxp;

	/* 
	 * Extract the components and initialize libc
	 */
	argv = &args;
	envp = auxp = argv + argc + 1;
	while(*auxp++){/*nothing to do*/}
	_init_libc(argc, argv, envp, (auxv_t *)auxp, 0);

	_wchar_short = 1;

	/*
	 * Run main() with those arguments, and pass the return
	 * value back to exit().
	 */
	exit(main(argc, argv, envp));
}

__SRCVERSION("_CMain.c $Rev: 153052 $");
