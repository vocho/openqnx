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

#include "externs.h"
#include <gulliver.h>
#include <sys/conf.h>
#include <sys/sysmgr.h>

#define DANH \
"Dan Hildebrand (1961-1998)\n\
Senior Software Architect\n\
Through his courage and enthusiasm we learned there are \
no limits to what we can accomplish.\n"

#if defined(VARIANT_be) || defined(VARIANT_le)
	#define	ENDIAN	ENDIAN_STRINGNAME
#else
	#define	ENDIAN	""
#endif

int sysmgr_confstr_set(int cmd, int name, const char *str) {
	int				len = strlen(str);

	if(strchr(str, ' ')) {
		char			*p1, *s = 0;
		const char		*p2;

		p1 = alloca(len + 1);
		if (p1 == NULL) {
			return ENOMEM;
		}

		for(s = p1, p2 = str; (*p1 = *p2); p1++, p2++) {
			if(*p1 == ' ') {
				*p1 = '_';
			}
		}
		str = (const char *)s;
	}
	return sysmgr_conf_set(SYSMGR_PID, _CONF_STR | cmd, name & ~_CS_SET, strlen(str), str);
}

// This is here to make internal calls to sysconf() not send a
// message to the process manager.
static const long	sysmgr_internal_table[] = {
	_CONF_VALUE_NUM|_CONF_INDIRECT |	_SC_PAGESIZE, 		(intptr_t)&memmgr.pagesize,
	_CONF_END
};


void sysmgr_init(void) {
	_sysconf_list = (long *)&sysmgr_internal_table;

	// "os_version_string" defined in automatically created timestamp.c
	// file, value determined by the _NTO_VERSION macro in <sys/neutrino.h>
	sysmgr_confstr_set(0, _CS_RELEASE, os_version_string);

	// "timestamp" defined in automatically created timestamp.c file,
	// value determined by time of procnto link.
	sysmgr_confstr_set(0, _CS_VERSION, timestamp);

	sysmgr_confstr_set(0, _CS_ARCHITECTURE, CPU_STRINGNAME ENDIAN);
	sysmgr_confstr_set(0, _CS_DANH, DANH);
}

__SRCVERSION("sysmgr_init.c $Rev: 160761 $");
