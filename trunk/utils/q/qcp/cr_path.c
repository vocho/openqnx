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
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SKIP_TO_SLASH(p)	{for(;*p && *p!='/';p++);}

/*
	qnx_create_path_to(filename)

	Will create directories in the path as neccessary to make it possible to
	subsequently create <filename>.

	This call does not guarantee that the file could actually be
	written - just that the path does exist. (i.e. no checking of
	permissions is done 

	returns 0 on success. Returns -1 on failure.
*/
int
qnx_create_path_to( char *fullpath ) {
	char *ptr = fullpath;

	if (fullpath[0] == '/' && fullpath[1] == '/')
	{
		/* starts with '//' */
		ptr += 2;
		SKIP_TO_SLASH(ptr);
		if (!*ptr || !*(ptr+1)) return(0);  /* zzx should check that path exists */
		ptr++;
		SKIP_TO_SLASH(ptr);
		if (!*ptr || !*(ptr+1)) return(0);
		ptr++;
	} else if (fullpath[0] == '/') ptr++;

	SKIP_TO_SLASH(ptr);
	
	for (;*ptr;)
	{
		*ptr = (char)0x00;
		if (mkdir(fullpath,S_IRWXU)==-1)
		{
			if (errno!=EEXIST)
			{
				fprintf(stderr,"Unable to create path %s: %s\n", fullpath, strerror(errno) );
				*ptr = '/';
				return(-1);
			}
		}
		*ptr = '/';
		ptr++;
		SKIP_TO_SLASH(ptr);
	}
	return(0);
}
