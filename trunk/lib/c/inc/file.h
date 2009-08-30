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




#ifndef _PTHREAD_H_INCLUDED
#include <pthread.h>
#endif

#ifndef EXT
#define EXT extern
#define INIT(a)
#else
#define INIT(a) = a
#endif

extern void __fpbufinit(FILE *fp);
extern FILE *__fpinit(FILE *fp, int fd, int oflag);
extern int __parse_oflag(const char *mode);

EXT pthread_mutex_t		_stdio_mutex	INIT(PTHREAD_MUTEX_INITIALIZER);
EXT pthread_cond_t		_stdio_cond		INIT(PTHREAD_COND_INITIALIZER);

/* __SRCVERSION("file.h $Rev: 153052 $"); */
