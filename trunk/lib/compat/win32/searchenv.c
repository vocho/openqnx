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
	Add support for environment variable substitution in the contents
	of the 'env' string. E.g., the environment contains:

			QSSL=C:\QSSL
			LIBPATH=%QSSL%\lib;%QSSL%\usr\lib

	then if 'env' is "LIBPATH", we'll search the C:\QSSL\lib and 
	C:\QSSL\usr\lib directories for the file.
*/

char *getenv(const char *);
extern char __pathbuff[];

#define	FUNKY_ENVNAME	"_ T M P _"
#define FUNKY_ENVNAME_LEN	(sizeof(FUNKY_ENVNAME)-1)

void searchenv( const char * name, const char * env, char * buf )
{
    const char 	*src;
	char		*dst;
	char		*tmp;

	src = getenv( env );
    if( src == (char *)0 ) {
	    _searchenv( name, env, buf );
		return;
	}
	strcpy( __pathbuff, FUNKY_ENVNAME "=" );
	dst = &__pathbuff[ FUNKY_ENVNAME_LEN + 1 ];
	for( ;; ) {
	    if( *src == '\0' ) break;
	    if( *src == '%' && *++src != '%' ) {
			tmp = dst;
			for( ;; ) {
				*dst = *src;
				if( *src == '\0' ) break;
				if( *src++ == '%' ) break;
				++dst;
			}
			*dst = '\0';
			dst = getenv( tmp );
			if( dst == (char *)0 ) {
				dst = tmp;
			} else {
				strcpy( tmp, dst );
				dst = &tmp[strlen(tmp)];
			}
		} else {
		    *dst++ = *src++;
		}
	}
	*dst = '\0';
	putenv( __pathbuff );
	_searchenv( name, FUNKY_ENVNAME, buf );
	__pathbuff[ FUNKY_ENVNAME_LEN + 1 ] = '\0';
	putenv( __pathbuff );
}
