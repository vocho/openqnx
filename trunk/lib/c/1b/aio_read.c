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




#undef  _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE 1
#undef  _FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS 32
#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <malloc.h>
#include <aio_priv.h>


int aio_read64(struct aiocb64 *aiocbp)
{
	if (!_aio_cb && _aio_init(NULL)) {
		return -1;
	}

	aiocbp->_aio_iotype = _AIO_OPCODE_READ;

	return _aio_insert((struct aiocb *)aiocbp);
}

int aio_read(struct aiocb *aiocbp)
{
	aiocbp->aio_offset_hi = (aiocbp->aio_offset < 0) ? -1 : 0;
	return aio_read64((struct aiocb64 *)aiocbp);
}

__SRCVERSION("aio_read.c $Rev: 153052 $");
