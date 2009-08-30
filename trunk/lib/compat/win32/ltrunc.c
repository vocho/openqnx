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



#if 0
#include <lib/compat.h>

off_t ltrunc( int fd, off_t offset, int whence )
{
	off_t where;
	off_t total;
	off_t current;

	total = filelength( fd );
	if( total == -1 )
	{
		errno = EBADF;
		return( -1 );
	}
	current = tell( fd );
	if( current == -1 )
	{
		errno = EBADF;
		return( -1 );
	}
	switch( whence )
	{
		case SEEK_SET:
			where = offset;
			break;
		case SEEK_CUR:
			where = current + offset;
			break;
		case SEEK_END:
			where = total + offset;
			break;
	}
	if( where >= total )
	{
		errno = EINVAL;
		return( -1 );
	}
	return( chsize( fd, where ) );
}
#endif
