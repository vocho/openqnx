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




#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

int chsize( int fd, long size ){
#if 0
	long off;
	int err = 0;

	off = tell( fd );
	if( off == -1 ) 
		return -1;
	if( lseek( fd, (off_t)size, SEEK_SET ) == -1 ) 
		return -1;
	if( write( fd, &err, 1 ) != 1 ){
		err = errno;
		lseek( fd, (off_t)off,  SEEK_SET );
		errno = err; 
		return -1;
		}
	if( ltrunc( fd, (off_t)size, SEEK_SET ) == -1 ){
		err = errno;
		lseek( fd, (off_t)off,  SEEK_SET );
		errno = err; 
		return -1;
		}
	if( lseek( fd, (off_t)off,  SEEK_SET ) == -1 )
		return -1;
	return 0;
#else
	return ftruncate(fd, size);
#endif
}

__SRCVERSION("chsize.c $Rev: 153052 $");
