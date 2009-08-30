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
extern char	*__pathbuff;

char *__qssl_rooted_fname( const char *name )
{
	char	*env;

	switch( name[0] ) {
	case '\0':
		return( (char *)name );
	case '/':
	case '\\':
		break;
	default:
	    /* relative pathname */
		return( (char *)name );
	}
	env = getenv( QSSL_ROOT_VAR );
	if( env == (char *)0 ) return( (char *)name );
	strcpy( __pathbuff, env );
	strcpy( &__pathbuff[strlen(__pathbuff)], name );
	return( __pathbuff );
}
