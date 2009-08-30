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



#include <lib/compat.h>

/*
	Was the executable run with the given name?
*/

int __is_executable_name( const char *exe, const char *name )
{
	unsigned	len;

	len = strlen( name );
	exe = basename( exe );
	if( memicmp( exe, name, len ) != 0 ) return( 0 );
	switch( exe[len] ) {
	case '\0':
	case '.':
		return( 1 );
	}
	return( 0 );
}
