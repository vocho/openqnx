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




#include <aio.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <aio_priv.h>

int aio_fsync(int op, struct aiocb *aiocbp)
{
	int		mode;

	if (!_aio_cb && _aio_init(NULL)) {
		return -1;
	}

	/*
	 * Check aiocbo->aio_fildes is a valid descriptor open for writing
	 */
	mode = fcntl(aiocbp->aio_fildes, F_GETFL);
	if (mode == -1 || (mode & O_ACCMODE) == O_RDONLY) {
		errno = EBADF;
		return -1;
	}

	switch (op) {
	  case O_DSYNC:
		aiocbp->_aio_iotype = _AIO_OPCODE_DSYNC;
		break;
	  case O_SYNC:
		aiocbp->_aio_iotype = _AIO_OPCODE_SYNC;
		break;
	  default:
		errno = EINVAL;
		return -1;
	}

	return _aio_insert(aiocbp);
}

__SRCVERSION("aio_fsync.c $Rev: 153052 $");
