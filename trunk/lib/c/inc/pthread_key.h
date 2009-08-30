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




#ifndef _PTHREAD_KEY_INCLUDED
#define _PTHREAD_KEY_INCLUDED

#include <pthread.h>

#define _KEY_NONE			((_key_destructor_t)-1)

typedef void				(* _key_destructor_t)(void *);
typedef int					_key_count_t;

extern _key_count_t			_key_count;
extern _key_destructor_t	*_key_destructor;
extern pthread_mutex_t		_key_mutex;

extern void					_key_delete(pthread_key_t);

#endif

/* __SRCVERSION("pthread_key.h $Rev: 153052 $"); */
