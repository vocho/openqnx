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




#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/procmgr.h>
#include <sys/conf.h>
#include <sys/sysmsg.h>
#include <sys/sysmgr.h>
#include <inttypes.h>

static int _confstr_get(int name, long *value, char *str);
#ifdef __PIC__	/* PIC bug in GCC where casting to (intptr_t) causes a rdonly section fixup */
static long _confstr_table[] =  {
#else
static const long _confstr_table[] =  {
#endif
	_CONF_LINK_FCN, (intptr_t)_confstr_get
};

static int _confstr_get(int name, long *value, char *str) {
	sys_conf_t				msg;
	iov_t					iov[2];

	msg.i.type = _SYS_CONF;
	msg.i.subtype = _SYS_SUB_GET;
	msg.i.cmd = _CONF_STR;
	msg.i.name = name;
	msg.i.value = *value;
    SETIOV(iov + 0, &msg.o, sizeof msg.o);
	SETIOV(iov + 1, str, *value);
	if(MsgSendvnc_r(SYSMGR_COID, (iov_t *)&msg.i, -sizeof msg.i, iov, 2)) {
		return -1;
	}

	*value = msg.o.value;
    return msg.o.match;
}

static pthread_mutex_t		_confstr_list_mutex = PTHREAD_MUTEX_INITIALIZER;
static unsigned				_confstr_cache_enabled = 0;
static volatile unsigned	_confstr_list_invalid = 1;
long						*_confstr_list;

void
confstr_cache_enable( void )
{
	_confstr_cache_enabled = 1;
}

void
confstr_cache_disable( void )
{
	_confstr_cache_enabled = 0;
}

void
confstr_cache_invalidate( void )
{
	_confstr_list_invalid = 1;
}

static void _confstr_invalidate_thread(union sigval arg)
{
	_confstr_list_invalid = 1;
}

/* the reason we allow for an external interface to handle cache
 * invalidation is because a process can only have ONE set of procmgr
 * notification, so if they want to use the api for other notifications too
 * then they will have to manage it manually.
 */
void
confstr_cache_attach( void )
{
	struct sigevent _confstr_event;
	
	SIGEV_THREAD_INIT( &_confstr_event, _confstr_invalidate_thread, NULL, NULL );

	(void)procmgr_event_notify(PROCMGR_EVENT_CONFSTR, &_confstr_event );

	_confstr_cache_enabled = 1;
}


size_t confstr(int name, char *buff, size_t len) {
	long					size = len;
	int						err;

	if(name & _CS_SET) {
		if(buff && len == 0) {
			/* confstr() returns 0 on error, so bump up this
			   return code by 1 (return derived from a MsgSend())
			   to get an error code which makes sense when setting */
			return sysmgr_confstr_set(_CONF_STICKY, name & ~_CS_SET, buff) +1; 
		}
		errno = EINVAL;
		return 0;
	}
	
	if ( (err = pthread_mutex_lock( &_confstr_list_mutex )) != EOK ) {
		errno = err;
		return 0;
	}

	do {
		if ( _confstr_list != NULL && _confstr_list_invalid ) {
			/* our local cached copy of confstr variables is out of date - discard it */
			free( _confstr_list );
			_confstr_list = NULL;
			_confstr_list_invalid = 0;
		} else if ( _confstr_list != NULL && _conf_get(_confstr_list, name, &size, buff) != -1) {
			pthread_mutex_unlock( &_confstr_list_mutex );
			if(buff && len && size >= len) {
				buff[len-1] = '\0';
			}
			return size;
		}
		if(_conf_get(_confstr_table, name, &size, buff) == -1) {
			//Unix 98 says that confstr returns 0 and does not set errno
			//when the 'name' is not set. If 'name' is incorrect, it retuns
			//zero and sets errno to EINVAL. What, exactly, is an illegal
			//'name' isn't specified. For right now, we assume that all 'name'
			//values are OK, and _conf_get returning -1 indicates that
			//'name' is unset.
//			errno = EINVAL;
			size = 0;
			break;
		} else if ( buff != NULL && _confstr_cache_enabled ) {
			/* set value in local cached table */
			(void)_conf_set(&_confstr_list, _CONF_STR, name, size, buff );
		} else {
			break;
		}
	} while( _confstr_list_invalid );

	pthread_mutex_unlock( &_confstr_list_mutex );

	if(buff && len && size >= len) {
		buff[len-1] = '\0';
	}
	return size;
}

__SRCVERSION("confstr.c $Rev: 159798 $");
