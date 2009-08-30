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

#include <sh/inout.h>
#include <stdio.h>

/*
 * This is the format of arguments for main() as passed in
 * from the OS.  It's layed on the stack, and extracted using
 * this overlay in _CMain
 */
struct stack_args {
	int argc;
	char *args[0];
};

/* This is for backwards compability with 2.0 compiled apps. */
int		_wchar_short;

/*
 * This is the format of arguments for main() as passed in
 * from the OS.
 */
void _CMain(void *arg_atexit, void *args) {
	register char			**argv, **envp, **auxp;
	struct stack_args		*s = args;

	/* 
	 * Extract the components and initialize libc
	 */
	argv = &s->args[0];
	envp = auxp = &s->args[s->argc + 1];
	while(*auxp++){/*nothing to do*/}

	_init_libc(s->argc, argv, envp, (auxv_t *)auxp, arg_atexit);

	_wchar_short = 1;

	/*
	 * Run main() with those arguments, and pass the return
	 * value back to exit().
	 */
	exit(main(s->argc, argv, envp));
}


__SRCVERSION("_CMain.c $Rev: 153052 $");
