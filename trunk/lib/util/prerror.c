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
#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <util/stdutil.h>
/*
--------------------------------------------------------- prerror(str1,args)
*/


void prerror(const char *format, ...)
    {
	int len;
        va_list arglist;
        char *buffer;
	int save_errno=errno;
	
        va_start(arglist, format);
	len = vsnprintf(NULL, 0, format, arglist);
	va_end(arglist);

	buffer = malloc(++len);
	if (!buffer)
		return;

        vsnprintf(buffer, len, format, arglist);

	errno=save_errno;
        perror(buffer);
	free(buffer);
    }

void aberror(const char *format, ...)
    {
        va_list arglist;
        char buffer[512];
		int save_errno=errno;
   
        va_start(arglist, format);
        vsprintf(buffer,format,arglist);
		errno=save_errno;

		if (errno) perror(buffer);
		else fprintf(stderr,"%s\n",buffer);
		exit(EXIT_FAILURE);
    }

void abprt(const char *format, ...)
    {
        va_list arglist;
        char buffer[512];
   
        va_start(arglist, format);
        vsprintf(buffer,format,arglist);
	
		fprintf(stderr,"%s\n",buffer);
		exit(EXIT_FAILURE);
    }
