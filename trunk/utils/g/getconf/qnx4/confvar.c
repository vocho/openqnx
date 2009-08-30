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
#include <limits.h>
#include <sys/uio.h>
#include "confvar.h"

#define STR(value)				#value

#define CONFSTR(name)				{ #name, FLAG_NAME | FLAG_CONFSTR, _CS_##name }
#define CONFSTR_XBS5(name) 			{ STR(_XBS5_##name), FLAG_NAME | FLAG_CONFSTR, _CS_XBS5_##name }, { STR(XBS5_##name), FLAG_NAME | FLAG_CONFSTR, _CS_XBS5_##name }
#define CONFSTR_NONE(name)			{ #name, FLAG_CONFSTR }
#define SYSCONF(name)				{ #name, FLAG_NAME | FLAG_SYSCONF, _SC_##name }
#define SYSCONF__POSIX_NOVALUE(name){ STR(_POSIX_##name), FLAG_NAME | FLAG_SYSCONF, _SC_##name }
#define SYSCONF_POSIX_NOVALUE(name)	{ STR(POSIX_##name), FLAG_NAME | FLAG_SYSCONF, _SC_##name }
#define SYSCONF_NONE(name)			{ #name, FLAG_SYSCONF }
#define SYSCONF_VALUE(name)			{ #name, FLAG_NAME | FLAG_SYSCONF | FLAG_VALUE, _SC_##name, name }
#define SYSCONF_POSIX(name)			{ #name, FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE, _SC_##name, 0, _POSIX_##name }
#define SYSCONF__POSIX(name)		{ STR(_POSIX_##name), FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE, _SC_##name, 0, _POSIX_##name }
#define SYSCONF_POSIX_POSIX(name)	{ STR(POSIX_##name), FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE, _SC_##name, 0, _POSIX_##name }
#define SYSCONF_PTHREAD(name)		{ STR(P##name), FLAG_NAME | FLAG_SYSCONF, _SC_##name }
#define SYSCONF_PTHREAD_POSIX(name)	{ STR(P##name), FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE, _SC_##name, 0, _POSIX_##name }
#define SYSCONF_VALUE_POSIX(name)	{ #name, FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE | FLAG_VALUE, _SC_##name, name, _POSIX_##name }
#define SYSCONF_VALUE__POSIX(name)	{ STR(_POSIX_##name), FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE | FLAG_VALUE, _SC_##name, name, _POSIX_##name }
#define SYSCONF_POSIX2(name)		{ STR(POSIX2_##name), FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE, _SC_##name, 0, _POSIX2_##name }
#define SYSCONF_VALUE_POSIX2(name)	{ STR(POSIX2_##name), FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE | FLAG_VALUE, _SC_##name, name, _POSIX2_##name }
#define SYSCONF_POSIX2_2(name)		{ STR(POSIX2_##name), FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE, _SC_2_##name, 0, _POSIX2_##name }
#define SYSCONF_XOPEN(name)			{ STR(_XOPEN_##name), FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE, _SC_XOPEN_##name, 0, _XOPEN_##name }
#define SYSCONF_VALUE_XOPEN(name)	{ STR(_XOPEN_##name), FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE | FLAG_VALUE, _SC_XOPEN_##name, _XOPEN_##name, _XOPEN_##name }
#define SYSCONF_XBS5(name)			{ STR(_XBS5_##name), FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE, _SC_XBS5_##name, 0, _XBS5_##name }
#define SYSCONF_VALUE_XBS5(name)	{ STR(_XBS5_##name), FLAG_NAME | FLAG_SYSCONF | FLAG_POSIX_VALUE | FLAG_VALUE, _SC_XBS5_##name, _XBS5_##name, _XBS5_##name }
#define PATHCONF(name)				{ #name, FLAG_NAME | FLAG_PATHCONF, _PC_##name }
#define PATHCONF_NONE(name)			{ #name, FLAG_PATHCONF }
#define PATHCONF_POSIX(name)		{ #name, FLAG_NAME | FLAG_PATHCONF | FLAG_POSIX_VALUE, _PC_##name, 0, _POSIX_##name }
#define PATHCONF__POSIX(name)		{ STR(_POSIX_##name), FLAG_NAME | FLAG_PATHCONF | FLAG_POSIX_VALUE, _PC_##name, 0, _POSIX_##name }
#define PATHCONF_VALUE_POSIX(name)	{ #name, FLAG_NAME | FLAG_PATHCONF | FLAG_POSIX_VALUE, _PC_##name, name, _POSIX_##name }
#define PATHCONF_VALUE__POSIX(name)	{ STR(_POSIX_##name), FLAG_NAME | FLAG_PATHCONF | FLAG_POSIX_VALUE, _PC_##name, name, _POSIX_##name }

struct variable table[] = {
	SYSCONF_VALUE_POSIX(	ARG_MAX	),
	SYSCONF_VALUE__POSIX(	ARG_MAX	),
#ifdef CHILD_MAX
	SYSCONF_VALUE__POSIX(	CHILD_MAX	),
#else
	SYSCONF__POSIX(			CHILD_MAX	),
#endif
	SYSCONF(				CLK_TCK	),
#ifdef _POSIX_JOB_CONTROL
	SYSCONF__POSIX(			JOB_CONTROL	),
#else
	SYSCONF_POSIX_NOVALUE(	JOB_CONTROL	),
#endif
	SYSCONF_VALUE_POSIX(	NGROUPS_MAX	),
	SYSCONF_VALUE__POSIX(	NGROUPS_MAX	),
#ifdef OPEN_MAX
	SYSCONF_VALUE_POSIX(	OPEN_MAX	),
	SYSCONF_VALUE__POSIX(	OPEN_MAX	),
#else
	SYSCONF_POSIX(			OPEN_MAX	),
	SYSCONF__POSIX(			OPEN_MAX	),
#endif
	SYSCONF__POSIX(			SAVED_IDS	),
	SYSCONF_VALUE_POSIX(	TZNAME_MAX	),
	SYSCONF_VALUE__POSIX(	TZNAME_MAX	),
	SYSCONF__POSIX(			VERSION	),

	PATHCONF__POSIX(		CHOWN_RESTRICTED	),
	PATHCONF(				DOS_SHARE	),
#ifdef MAX_CANNON
	PATHCONF_VALUE_POSIX(	MAX_CANON	),
	PATHCONF_VALUE__POSIX(	MAX_CANON	),
#else
	PATHCONF_POSIX(			MAX_CANON	),
	PATHCONF__POSIX(		MAX_CANON	),
#endif
#ifdef MAX_INPUT
	PATHCONF_VALUE_POSIX(	MAX_INPUT	),
#else
	PATHCONF_POSIX(			MAX_INPUT	),
#endif
	PATHCONF_VALUE_POSIX(	NAME_MAX	),
	PATHCONF_VALUE__POSIX(	NAME_MAX	),
	PATHCONF__POSIX(		NO_TRUNC	),
	PATHCONF_VALUE_POSIX(	PATH_MAX	),
	PATHCONF_VALUE__POSIX(	PATH_MAX	),
	PATHCONF_VALUE_POSIX(	PIPE_BUF	),
	PATHCONF_VALUE__POSIX(	PIPE_BUF	),
	PATHCONF__POSIX(		VDISABLE	),
	{ 0, FLAG_END }
};
