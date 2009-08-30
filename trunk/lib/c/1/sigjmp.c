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




#include <setjmp.h>
#include <signal.h>
#include <sys/neutrino.h>

/* This is used for backward compatibility with nto beta6 and before */
#define SIGJMP_OLD

#ifdef SIGJMP_OLD
struct jmpbuf {
	int				__flg;
	long			__msk[2];
	jmp_buf			__buf;
};

static char __sigjmp_old;	/* This is initialized to zero */

void __sigjmpsave(sigjmp_buf nenv, int msk) {
	struct jmpbuf	*env = (struct jmpbuf *)nenv;

	/* An old library must have called us, so switch */
	__sigjmp_old = 1;
	if((env->__flg = msk)) {
		SignalProcmask_r(0, 0, SIG_BLOCK, 0, (sigset_t *)(void *)env->__msk);
	}
}
#endif

/*
    Casts through void * are to keep Metaware happy
*/
void __sigjmp_prolog(sigjmp_buf env, int msk) {
	if((env->__flg = msk)) {
		SignalProcmask_r(0, 0, SIG_BLOCK, 0, (sigset_t *)(void *)env->__msk);
	}
}

void siglongjmp(sigjmp_buf env, int val) {
#ifdef SIGJMP_OLD
	if(__sigjmp_old) {
		struct jmpbuf	*oenv = (struct jmpbuf *)env;

		if(oenv->__flg) {
			SignalProcmask_r(0, 0, SIG_SETMASK, (sigset_t *)(void *)oenv->__msk, 0);
		}
		_longjmp(oenv->__buf, val);
	}
#endif
	if(env->__flg) {
		SignalProcmask_r(0, 0, SIG_SETMASK, (sigset_t *)(void *)env->__msk, 0);
	}
	_longjmp(env, val);
}

__SRCVERSION("sigjmp.c $Rev: 153052 $");
