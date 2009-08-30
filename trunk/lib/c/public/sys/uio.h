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



/*
 *  uio.h
 *

 */
#ifndef __UIO_H_INCLUDED
#define __UIO_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include <_pack64.h>

_C_STD_BEGIN

#if defined(__SSIZE_T)
typedef __SSIZE_T	ssize_t;
#undef __SSIZE_T
#endif

#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif

_C_STD_END

#if defined(__EXT_UNIX_MISC)
#define UIO_MAXIOV   1024       /* max 1K of iov's */
#define UIO_SMALLIOV    8       /* 8 on stack, else malloc */
#endif

#if defined(__IOVEC_T)
typedef __IOVEC_T	iov_t;
#undef __IOVEC_T
#endif

#include <_packpop.h>

__BEGIN_DECLS

extern _CSTD ssize_t readv(int __fildes, const struct iovec *__iov, int __nparts);
extern _CSTD ssize_t writev(int __fildes, const struct iovec *__iov, int __nparts);

#if defined(__EXT_QNX)
extern _CSTD ssize_t  _readxv(int __fd, struct iovec *__iovec, int __nparts, unsigned __xtype, void *__xdata, _CSTD size_t __xdatalen, _CSTD size_t __nbytes);
extern _CSTD ssize_t  _writexv(int __fd, struct iovec *__iovec, int __nparts, unsigned __xtype, void *__xdata, _CSTD size_t __xdatalen, _CSTD size_t __nbytes);
#endif

__END_DECLS

#endif

/* __SRCVERSION("uio.h $Rev: 153052 $"); */
