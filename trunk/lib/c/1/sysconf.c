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
#include <sys/conf.h>
#include <sys/sysmsg.h>
#include <inttypes.h>

static int _sysconf_get(int name, long *value, char *str);
#ifdef __PIC__	/* PIC bug in GCC where casting to (intptr_t) causes a rdonly section fixup */
static long _sysconf_table[] =  {
#else
static const long _sysconf_table[] =  {
#endif
	_CONF_CALL_PTR,			(intptr_t)&_sysconf_list,
	_CONF_VALUE_MIN |		_SC_NGROUPS_MAX, 		NGROUPS_MAX,
	_CONF_VALUE_NUM |		_SC_THREAD_DESTRUCTOR_ITERATIONS,	PTHREAD_DESTRUCTOR_ITERATIONS,
	_CONF_VALUE_NUM |		_SC_MB_LEN_MAX,			MB_LEN_MAX,
	_CONF_VALUE_NUM |		_SC_CHAR_BIT,			CHAR_BIT,
	_CONF_VALUE_NUM |		_SC_CHAR_MAX,			CHAR_MAX,
	_CONF_VALUE_NUM |		_SC_CHAR_MIN,			CHAR_MIN,
	_CONF_VALUE_NUM |		_SC_INT_MAX,			INT_MAX,
	_CONF_VALUE_NUM |		_SC_INT_MIN,			INT_MIN,
	_CONF_VALUE_NUM |		_SC_LONG_BIT,			__LONG_BITS__,
	_CONF_VALUE_NUM |		_SC_SCHAR_MAX,			SCHAR_MAX,
	_CONF_VALUE_NUM |		_SC_SCHAR_MIN,			SCHAR_MIN,
	_CONF_VALUE_NUM |		_SC_SHRT_MAX,			SHRT_MAX,
	_CONF_VALUE_NUM |		_SC_SHRT_MIN,			SHRT_MIN,
	_CONF_VALUE_NUM |		_SC_SSIZE_MAX,			LONG_MAX,	/* SSIZE is always 32-bits (sys/target_nto.h) */
	_CONF_VALUE_NUM |		_SC_UCHAR_MAX,			UCHAR_MAX,
	_CONF_VALUE_NUM |		_SC_UINT_MAX,			UINT_MAX,
	_CONF_VALUE_NUM |		_SC_ULONG_MAX,			ULONG_MAX,
	_CONF_VALUE_NUM |		_SC_USHRT_MAX,			USHRT_MAX,
	_CONF_VALUE_NUM |		_SC_WORD_BIT,			__INT_BITS__,
/*
 *  Implement sysconf() support for all defined POSIX option groups;
 *  basically this mirrors the presence and value of anything defined
 *  compile-time in <unistd.h> to a corresponding run-time value.
 *  Let kernel/procnto limit run-time _SC_VERSION
 *  Let mqueue/mq handle run-time _SC_MESSAGE_PASSING
 *  Let mqueue/procnto handle run-time _SC_SEMAPHORES
 */
	_CONF_VALUE_MIN |	_SC_VERSION,					_POSIX_VERSION,
#if defined(_POSIX_ADVISORY_INFO) && defined(_SC_ADVISORY_INFO)
	_CONF_VALUE_NUM |	_SC_ADVISORY_INFO,				_POSIX_ADVISORY_INFO,
#endif
#if defined(_POSIX_ASYNCHRONOUS_IO) && defined(_SC_ASYNCHRONOUS_IO)
	_CONF_VALUE_NUM |	_SC_ASYNCHRONOUS_IO,			_POSIX_ASYNCHRONOUS_IO,
#endif
#if defined(_POSIX_BARRIERS) && defined(_SC_BARRIERS)
	_CONF_VALUE_NUM |	_SC_BARRIERS,					_POSIX_BARRIERS,
#endif
#if defined(_POSIX_CPUTIME) && defined(_SC_CPUTIME)
	_CONF_VALUE_NUM |	_SC_CPUTIME,					_POSIX_CPUTIME,
#endif
#if defined(_POSIX_CLOCK_SELECTION) && defined(_SC_CLOCK_SELECTION)
	_CONF_VALUE_NUM |	_SC_CLOCK_SELECTION,			_POSIX_CLOCK_SELECTION,
#endif
#if defined(_POSIX_FSYNC) && defined(_SC_FSYNC)
	_CONF_VALUE_NUM |	_SC_FSYNC,						_POSIX_FSYNC,
#endif
#if defined(_POSIX_MAPPED_FILES) && defined(_SC_MAPPED_FILES)
	_CONF_VALUE_NUM |	_SC_MAPPED_FILES,				_POSIX_MAPPED_FILES,
#endif
#if defined(_POSIX_MEMLOCK) && defined(_SC_MEMLOCK)
	_CONF_VALUE_NUM |	_SC_MEMLOCK,					_POSIX_MEMLOCK,
#endif
#if defined(_POSIX_MEMLOCK_RANGE) && defined(_SC_MEMLOCK_RANGE)
	_CONF_VALUE_NUM |	_SC_MEMLOCK_RANGE,				_POSIX_MEMLOCK_RANGE,
#endif
#if defined(_POSIX_MONOTONIC_CLOCK) && defined(_SC_MONOTONIC_CLOCK)
	_CONF_VALUE_NUM |	_SC_MONOTONIC_CLOCK,			_POSIX_MONOTONIC_CLOCK,
#endif
#if defined(_POSIX_MEMORY_PROTECTION) && defined(_SC_MEMORY_PROTECTION)
	_CONF_VALUE_NUM |	_SC_MEMORY_PROTECTION,			_POSIX_MEMORY_PROTECTION,
#endif
#if defined(_POSIX_PRIORITIZED_IO) && defined(_SC_PRIORITIZED_IO)
	_CONF_VALUE_NUM |	_SC_PRIORITIZED_IO,				_POSIX_PRIORITIZED_IO,
#endif
#if defined(_POSIX_PRIORITY_SCHEDULING) && defined(_SC_PRIORITY_SCHEDULING)
	_CONF_VALUE_NUM |	_SC_PRIORITY_SCHEDULING,		_POSIX_PRIORITY_SCHEDULING,
#endif
#if defined(_POSIX_REALTIME_SIGNALS) && defined(_SC_REALTIME_SIGNALS)
	_CONF_VALUE_NUM |	_SC_REALTIME_SIGNALS,			_POSIX_REALTIME_SIGNALS,
#endif
#if defined(_POSIX_SHARED_MEMORY_OBJECTS) && defined(_SC_SHARED_MEMORY_OBJECTS)
	_CONF_VALUE_NUM |	_SC_SHARED_MEMORY_OBJECTS,		_POSIX_SHARED_MEMORY_OBJECTS,
#endif
#if defined(_POSIX_SYNCHRONIZED_IO) && defined(_SC_SYNCHRONIZED_IO)
	_CONF_VALUE_NUM |	_SC_SYNCHRONIZED_IO,			_POSIX_SYNCHRONIZED_IO,
#endif
#if defined(_POSIX_SPIN_LOCKS) && defined(_SC_SPIN_LOCKS)
	_CONF_VALUE_NUM |	_SC_SPIN_LOCKS,					_POSIX_SPIN_LOCKS,
#endif
#if defined(_POSIX_SPAWN) && defined(_SC_SPAWN)
	_CONF_VALUE_NUM |	_SC_SPAWN,						_POSIX_SPAWN,
#endif
#if defined(_POSIX_SPORADIC_SERVER) && defined(_SC_SPORADIC_SERVER)
	_CONF_VALUE_NUM |	_SC_SPORADIC_SERVER,			_POSIX_SPORADIC_SERVER,
#endif
#if defined(_POSIX_THREAD_CPUTIME) && defined(_SC_THREAD_CPUTIME)
	_CONF_VALUE_NUM |	_SC_THREAD_CPUTIME,				_POSIX_THREAD_CPUTIME,
#endif
#if defined(_POSIX_THREADS) && defined(_SC_THREADS)
	_CONF_VALUE_NUM |	_SC_THREADS,					_POSIX_THREADS,
#endif
#if defined(_POSIX_TIMEOUTS) && defined(_SC_TIMEOUTS)
	_CONF_VALUE_NUM |	_SC_TIMEOUTS,					_POSIX_TIMEOUTS,
#endif
#if defined(_POSIX_TIMERS) && defined(_SC_TIMERS)
	_CONF_VALUE_NUM |	_SC_TIMERS,						_POSIX_TIMERS,
#endif
#if defined(_POSIX_THREAD_PRIO_INHERIT) && defined(_SC_THREAD_PRIO_INHERIT)
	_CONF_VALUE_NUM |	_SC_THREAD_PRIO_INHERIT,		_POSIX_THREAD_PRIO_INHERIT,
#endif
#if defined(_POSIX_THREAD_PRIO_PROTECT) && defined(_SC_THREAD_PRIO_PROTECT)
	_CONF_VALUE_NUM |	_SC_THREAD_PRIO_PROTECT,		_POSIX_THREAD_PRIO_PROTECT,
#endif
#if defined(_POSIX_THREAD_PRIORITY_SCHEDULING) && defined(_SC_THREAD_PRIORITY_SCHEDULING)
	_CONF_VALUE_NUM |	_SC_THREAD_PRIORITY_SCHEDULING,	_POSIX_THREAD_PRIORITY_SCHEDULING,
#endif
#if defined(_POSIX_THREAD_ATTR_STACKADDR) && defined(_SC_THREAD_ATTR_STACKADDR)
	_CONF_VALUE_NUM |	_SC_THREAD_ATTR_STACKADDR,		_POSIX_THREAD_ATTR_STACKADDR,
#endif
#if defined(_POSIX_THREAD_SAFE_FUNCTIONS) && defined(_SC_THREAD_SAFE_FUNCTIONS)
	_CONF_VALUE_NUM |	_SC_THREAD_SAFE_FUNCTIONS,		_POSIX_THREAD_SAFE_FUNCTIONS,
#endif
#if defined(_POSIX_THREAD_PROCESS_SHARED) && defined(_SC_THREAD_PROCESS_SHARED)
	_CONF_VALUE_NUM |	_SC_THREAD_PROCESS_SHARED,		_POSIX_THREAD_PROCESS_SHARED,
#endif
#if defined(_POSIX_THREAD_SPORADIC_SERVER) && defined(_SC_THREAD_SPORADIC_SERVER)
	_CONF_VALUE_NUM |	_SC_THREAD_SPORADIC_SERVER,		_POSIX_THREAD_SPORADIC_SERVER,
#endif
#if defined(_POSIX_THREAD_ATTR_STACKSIZE) && defined(_SC_THREAD_ATTR_STACKSIZE)
	_CONF_VALUE_NUM |	_SC_THREAD_ATTR_STACKSIZE,		_POSIX_THREAD_ATTR_STACKSIZE,
#endif
#if defined(_POSIX_TYPED_MEMORY_OBJECTS) && defined(_SC_TYPED_MEMORY_OBJECTS)
	_CONF_VALUE_NUM |	_SC_TYPED_MEMORY_OBJECTS,		_POSIX_TYPED_MEMORY_OBJECTS,
#endif
#if defined(TZNAME_MAX) && defined(_SC_TZNAME_MAX)
	_CONF_VALUE_NUM |	_SC_TZNAME_MAX,					TZNAME_MAX,
#endif
	_CONF_LINK_FCN,			(intptr_t)_sysconf_get
};

static int _sysconf_get(int name, long *value, char *str) {
	sys_conf_t				msg;

	msg.i.type = _SYS_CONF;
	msg.i.subtype = _SYS_SUB_GET;
	msg.i.cmd = _CONF_NUM;
	msg.i.name = name;
	msg.i.value = *value;

	if(MsgSendnc_r(SYSMGR_COID, &msg.i, sizeof msg.i, &msg.o, sizeof msg.o)) {
		return -1;
	}
	*value = msg.o.value;
    return msg.o.match;
}

long *_sysconf_list;

long _sysconf(int name) {
	long					value = -1;

	if(_conf_get(_sysconf_table, name, &value, 0) == -1 && value == -1) {
		// POSIX says that sysconf returns -1 and does not set errno
		// when the 'name' is valid and has no limit. If 'name' is invalid,
		// it returns -1 and sets errno to EINVAL. What, exactly, is an
		// invalid 'name' isn't specified. For right now, we assume that
		// all 'name' values are OK, and _conf_get returning -1 indicates
		// that 'name' is unset.  Assume, for PCTS, that -1 is never valid.
		if (name == -1) errno = EINVAL;
	}
	return value;
}

long sysconf(int name) {
	return _sysconf(name);
}

__SRCVERSION("sysconf.c $Rev: 153052 $");
