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
#include <process.h>
#include <errno.h>
#include <pthread.h>
#include "pthread_fork.h"

int pthread_atfork( void ( * prepare )( void ), void ( * parent )( void ), void ( * child )( void ) )
{
	struct _pthread_atfork_func * fprepare, * fparent, * fchild;

	if( prepare )
	{
		if( ( fprepare = ( struct _pthread_atfork_func * ) malloc( sizeof( struct _pthread_atfork_func ) ) ) )
		{
			fprepare->func = prepare;
		}
		else
		{
			return( ENOMEM );
		}
	}
	else
	{
		fprepare = 0;
	}
	if( parent )
	{
		if( ( fparent = ( struct _pthread_atfork_func * ) malloc( sizeof( struct _pthread_atfork_func ) ) ) )
		{
			fparent->func = parent;
		}
		else
		{
			if( fprepare )
			{
				free( fprepare );
			}
			return( ENOMEM );
		}
	}
	else
	{
		fparent = 0;
	}
	if( child )
	{
		if( ( fchild = ( struct _pthread_atfork_func * ) malloc( sizeof( struct _pthread_atfork_func ) ) ) )
		{
			fchild->func = child;
		}
		else
		{
			if( fprepare )
			{
				free( fprepare );
			}
			if( fparent )
			{
				free( fparent );
			}
			return( ENOMEM );
		}
	}
	else
	{
		fchild = 0;
	}
	pthread_mutex_lock( & pthread_atfork_mutex );
	if( fprepare )
	{
		fprepare->next = _pthread_atfork_prepare;
		_pthread_atfork_prepare = fprepare;
	}
	if( fparent )
	{
		struct _pthread_atfork_func * * f;

		for( f = & _pthread_atfork_parent; * f; f = & ( ( * f )->next ) ) {
			/* nothing to do */
		}
		fparent->next = * f;
		* f = fparent;
	}
	if( fchild )
	{
		struct _pthread_atfork_func * * f;

		for( f = & _pthread_atfork_child; * f; f = & ( ( * f )->next ) ) {
			/* nothing to do */
		}
		fchild->next = * f;
		* f = fchild;
	}
	pthread_mutex_unlock( & pthread_atfork_mutex );
	return( EOK );
}

__SRCVERSION("pthread_atfork.c $Rev: 153052 $");
