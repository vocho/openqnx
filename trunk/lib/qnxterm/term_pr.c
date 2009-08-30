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




#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <sys/qnxterm.h>

#define t	term_state

#ifdef __STDC__
int term_printf( int row, int col, unsigned attr, const char *fmt, ... )
	{
	va_list arglist;
	unsigned len;
	char buffer[ TERM_PRINTF_MAX];

	va_start( arglist, fmt );
	if ( (len = vsprintf( &buffer[0], (char *) fmt, arglist ) ) > 0 )
			return( term_type( row, col, buffer, len, attr ) );
	else	return( 0 );
	}
#else
int term_printf( row, col, attr, fmt, arg1 )
	int row, col, attr, arg1;
	char *fmt;
	{
	unsigned len;
	char buffer[TERM_PRINTF_MAX];

	if ( (len = sprintf( buffer, "%r", &fmt ) ) > 0 )
			return( term_type( row, col, buffer, len, attr ) );
	else	return( 0 );
	}
#endif
