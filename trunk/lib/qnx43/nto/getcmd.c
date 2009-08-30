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



#include <string.h>

extern int _argc; /* cmain stuffs these */
extern char * * _argv;

char * getcmd( char * buffer )
{
	int i;

	* buffer = 0;
	for( i = 1; i < _argc; i++ )
	{
		strcat( buffer, _argv[i] );
		if( i < _argc - 1 )
			strcat( buffer, " " );
	}
	return( buffer );
}
