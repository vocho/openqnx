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




#include <pthread.h>

/*
   Trampoline code - this function is actually 'returned' to when the
   main thread function falls off the end of it's code. The 'value_ptr'
   parameter will be in the CPU's return register (EAX for X86).
*/

#ifdef __MY_THREAD_EXIT_ATTR_REGPARM
#error __MY_THREAD_EXIT_ATTR_REGPARM already defined!
#else
#define __MY_THREAD_EXIT_ATTR_REGPARM
#endif

#if defined(__WATCOMC__)
  #pragma aux __my_thread_exit parm [eax] aborts;
#elif defined(__GNUC__) || defined(__INTEL_COMPILER)
  #undef  __MY_THREAD_EXIT_ATTR_REGPARM
  #define __MY_THREAD_EXIT_ATTR_REGPARM __attribute__((regparm(1)))
  void __MY_THREAD_EXIT_ATTR_REGPARM __my_thread_exit( void * value_ptr );
#else
  #error not configured for compiler
#endif

void __MY_THREAD_EXIT_ATTR_REGPARM
__my_thread_exit( void * value_ptr ) {
	/* for now do whatever pthread_exit does */
	pthread_exit( value_ptr );
}

#undef  __MY_THREAD_EXIT_ATTR_REGPARM


__SRCVERSION("__my_thread_exit.c $Rev: 153052 $");
