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




#include "cpucfg.h"

//
// Macros for converting vararg lists to argument vectors (for spawn*/exec*)
//

#ifdef L2V_CHEAT
	// These are used when the calling convention puts everything on the
	// stack as an array for us already.

	#define CVT_L2V(arg0, argv)	\
			(argv) = (char **)&(arg0)

	#define CVT_L2V_ENV(arg0, argv, envv)		\
			CVT_L2V(arg0, argv);				\
			do {   char **p;					\
				for(p = argv; *p != 0; ++p) {	\
					/* nothing to do */		\
				}				\
				(envv) = (char **)p[1];			\
			} while(0)

#else
	// These are used when we have to do everything 'by the book'.

	#define DOIT_CVT_L2V(arg0, argv, envv_assignment)		\
			do {	va_list		ap;							\
				unsigned	num;							\
				char		**p;							\
															\
				num = 1;									\
				if(arg0 != 0) {								\
					va_start(ap, arg0);						\
					for(++num; va_arg(ap, char *); num++) {	\
							/* nothing to do */				\
					}										\
					va_end(ap);								\
				}											\
															\
				if(!(argv = alloca(num * sizeof *argv))) {	\
					errno = ENOMEM;							\
					return -1;								\
				}											\
															\
				p = argv;									\
				*p++ = (char *)arg0;						\
				va_start(ap, arg0);							\
				if(arg0 != 0) {								\
					while((*p++ = va_arg(ap, char *))) {	\
							/* nothing to do */				\
					}										\
				}											\
				(envv_assignment);							\
				va_end(ap);									\
			} while(0);

	#define CVT_L2V(arg0, argv)	\
			DOIT_CVT_L2V(arg0, argv, num = 1)

	#define CVT_L2V_ENV(arg0, argv, envv)	\
			DOIT_CVT_L2V(arg0, argv, envv = va_arg(ap, char **))


#endif

/* __SRCVERSION("cvtl2v.h $Rev: 153052 $"); */
