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
	If we have an absolute pathname, stick the contents of the QSSL 
	environment var on the front of it.
*/

char *getenv(const char *);
extern char	__pathbuff[];

static char	*append_one( char *buff, const char *name, unsigned len )
{
	char	*env;

	if( len == 0 ) return( buff );
	switch( name[0] ) {
	case '\0':
		memcpy( buff, name, len );
		return( &buff[len] );
	case '/':
	case '\\':
		break;
	default:
	    /* relative pathname */
		memcpy( buff, name, len );
		return( &buff[len] );
	}
	env = getenv( QSSL_ROOT_VAR );
	if( env == (char *)0 ) return( (char *)name );
	strcpy( buff, env );
	buff += strlen( buff );
	memcpy( buff, name, len );
    return( &buff[len] );
}

char *__qssl_rooted_pathlist( const char *paths )
{
	char	*buff;
	const char	*end;

	buff = __pathbuff;
	for( ;; ) {
		end = strchr( paths, PATHSEP_CHR );
		if( end == (char *)0 ) end = &paths[ strlen( paths ) ];
		buff = append_one( buff, paths, end - paths );
		*buff++ = *end;
		if( *end == '\0' ) break;
		paths = end + 1;
	}
	return( __pathbuff );
}
