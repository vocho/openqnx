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


/*
 * utils.c
 */

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/netmgr.h>
#include <sys/syspage.h>
#include <sys/trace.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/kercalls.h>
#include <time.h>
#include <unistd.h>

#include "utils.h" 

int verbosity = VERBOSITY_NONE;
 
int info( const char *fmt, ... )
{
va_list arglist;
int ret;

	if ( verbosity < VERBOSITY_INFO )
		return 0;
	va_start( arglist, fmt );
	ret = vfprintf( stderr, fmt, arglist );
	va_end( arglist );
	return ret;
}
int debug( const char *fmt, ... )
{
va_list arglist;
int ret;

	if ( verbosity < VERBOSITY_DEBUG )
		return 0;
	va_start( arglist, fmt );
	ret = vfprintf( stderr, fmt, arglist );
	va_end( arglist );
	return ret;
}
int hack( const char *fmt, ... )
{
va_list arglist;
int ret;

	if ( verbosity < VERBOSITY_HACK )
		return 0;
	va_start( arglist, fmt );
	ret = vfprintf( stderr, fmt, arglist );
	va_end( arglist );
	return ret;
}

/* RUSH - this should be done in a more future proof and elegant way */
unsigned discover_num_colours(void)
{
#if defined(__MIPS__) || defined(__SH__)
	unsigned                    num_colours;
	struct cpuinfo_entry        *cpu;
	struct cacheattr_entry      *cache;
	unsigned					dcache_size, icache_size;

	cpu = SYSPAGE_ENTRY(cpuinfo);

	cache = &SYSPAGE_ENTRY(cacheattr)[cpu->data_cache];
	dcache_size = cache->num_lines * cache->line_size;

	cache = &SYSPAGE_ENTRY(cacheattr)[cpu->ins_cache];
	icache_size = cache->num_lines * cache->line_size;

	num_colours = max(icache_size, dcache_size) / __PAGESIZE;
	hack("discovered %d colours of cache\n", num_colours);

	return num_colours;
#else
	return 0;
#endif
}

struct attributes *g_extra_attributes = 0;

/* 
 Extract the user's command line additions to the attributes,
 User attributes are specified as KEY=VALUE pairs and if the
 input string contains characters that are special to the formatting
 (ie delimiters) then the entry is ignored and a warning is 
 logged.
*/
int add_user_attribute(const char *str) {
	//Replace all "bad" text with this string
	struct attributes *newattr;
	char  *c, *buffer = NULL;

	//If we find forbidden strings, then we log an note and ignore it
	if((c = strstr(str, _TRACE_HEADER_POSTFIX)) != NULL) {
		info("Invalid use of %s characters in attribute %s \n", _TRACE_HEADER_POSTFIX, str);
		return -1;
	}
	
	if((c = strstr(str, _TRACE_HEADER_PREFIX)) != NULL) {
		info("Invalid use of %s characters in attribute %s \n", _TRACE_HEADER_PREFIX, str);
		return -1;
	}

	//Entry looks good, make a local copy instead of messing with the arguments
	buffer = strdup(str);	
	if(buffer == NULL) {
		return -1;
	}

	c = strchr(buffer, '=');
	if(c == NULL) {
		free(buffer);
		info("Attribute %s must be formatted as key=value\n",str);
		return -1;
	}
	*c++ = '\0';

	newattr = (struct attributes *)calloc(1, sizeof(*newattr));
	if(newattr == NULL) {
		free(buffer);
		return -1;
	}

	newattr->key = buffer;
	newattr->value = c; 
	newattr->next = g_extra_attributes;
	g_extra_attributes = newattr;

	return 0;
}
   

__SRCVERSION("utils.c $Rev: 153052 $");
