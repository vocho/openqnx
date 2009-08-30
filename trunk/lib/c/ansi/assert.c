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
#include <string.h>
#include <unistd.h>
#undef NDEBUG
#include <assert.h>

_STD_BEGIN

void __assert(const char *expr, const char *file, unsigned line, const char *func) {
	int		fd = fileno(stderr);
	char	lbuf[10];

	if ( utoa( line, lbuf, 10 ) == NULL ) {
		lbuf[0] = '?';
		lbuf[1] = '\0';
	}

	if(func) {	
		write( fd, "In function ", 12);
		write( fd, func, strlen(func));
		write( fd, " -- ", 4 );
	}

	write( fd, file, strlen(file));
	write( fd, ":", 1 );
	write( fd, lbuf, strlen(lbuf));
	write( fd, " ", 1 );
	write( fd, expr, strlen(expr));
	write( fd, " -- assertion failed\n", 21 );
	abort();
}

_STD_END

__SRCVERSION("assert.c $Rev: 167935 $");
