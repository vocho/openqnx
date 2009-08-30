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
 *  aio.h    POSIX Asynchronous Input and Output
 *

 */

#ifndef _AIO_H_INCLUDED
#define _AIO_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif


#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#ifndef _SIGNAL_H_INCLUDED
 #include <signal.h>
#endif

#if !defined(__EXT_POSIX1_199309) && (defined(__EXT_POSIX1_198808) || defined(__EXT_POSIX1_199009)) 
#error POSIX Asynchronous Input and Output needs P1003.1b-1993 or later
#endif

#include <_pack64.h>

#if defined(__TIMESPEC_INTERNAL)
__TIMESPEC_INTERNAL;
#undef __TIMESPEC_INTERNAL
#endif

#if defined(__SCHED_PARAM_INTERNAL)
__SCHED_PARAM_INTERNAL;
#undef __SCHED_PARAM_INTERNAL
#endif

struct aiocb {
	int					aio_fildes;
	int					aio_reqprio;
#if _FILE_OFFSET_BITS - 0 == 64
	off_t				aio_offset;
#elif !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
#if defined(__LITTLEENDIAN__)
	off_t				aio_offset;
	off_t				aio_offset_hi;
#elif defined(__BIGENDIAN__)
	off_t				aio_offset_hi;
	off_t				aio_offset;
#else
 #error endian not configured for system
#endif
#else
 #error _FILE_OFFSET_BITS value is unsupported
#endif
	__volatile void*	aio_buf;
	size_t				aio_nbytes;
	struct sigevent		aio_sigevent;
	int					aio_lio_opcode;
	ssize_t				_aio_reserved;
	int					_aio_pad[3];
	
	/* used by the library, application shouldn't touch them */
	struct aiocb        *_aio_next;
	unsigned            _aio_flag;
	unsigned            _aio_iotype;
	unsigned            _aio_result;
	unsigned            _aio_error;
	void                *_aio_suspend;
	void                *_aio_plist;
	int                 _aio_policy;
	struct __sched_param  _aio_param;
};

#if _LARGEFILE64_SOURCE - 0 > 0
struct aiocb64 {
	int					aio_fildes;
	int					aio_reqprio;
	off64_t				aio_offset;
	__volatile void*	aio_buf;
	size_t				aio_nbytes;
	struct sigevent		aio_sigevent;
	int					aio_lio_opcode;
	ssize_t				_aio_reserved;
	int					_aio_pad[3];
	
	/* used by the library, application shouldn't touch them */
	struct aiocb        *_aio_next;
	unsigned            _aio_flag;
	unsigned            _aio_iotype;
	unsigned            _aio_result;
	unsigned            _aio_error;
	void                *_aio_suspend;
	void                *_aio_plist;
	unsigned            _aio_policy;
	struct __sched_param  _aio_param;
};
#endif /* _LARGEFILE64_SOURCE */

#if defined(__TIMESPEC)
__TIMESPEC;
#undef __TIMESPEC
#endif

#include <_packpop.h>


#define AIO_CANCELED	0
#define AIO_ALLDONE		1
#define AIO_NOTCANCELED	2

#define LIO_NOWAIT		0
#define LIO_WAIT		1

#define LIO_NOP			0
#define	LIO_READ		1
#define LIO_WRITE		2

/*
 * POSIX 1003.1b-1993 Prototypes.
 */
__BEGIN_DECLS

#if _LARGEFILE64_SOURCE - 0 > 0
extern int aio_cancel64(int __fd, struct aiocb64 *__aiocbp);
extern int aio_error64(__const struct aiocb64 *__aiocbp);
extern int aio_fsync64(int __operation, struct aiocb64 *__aiocbp);
extern int aio_read64(struct aiocb64 *__aiocbp);
extern ssize_t aio_return64(struct aiocb64 *__aiocpb);
extern int aio_suspend64(__const struct aiocb64 *__const __list[], int __nent,	__const struct timespec *__timeout);
extern int aio_write64(struct aiocb64 *__aiocpb);
extern int lio_listio64(int __mode, struct aiocb64 *__const __list[], int __nent,	struct sigevent *__sig);

/*
 * These 64 bits functions does not touch the "aio_offset" member,
 * we map them back to their 32 bits counterpart.
 */
#define aio_cancel64(__fd, __aiocbp) aio_cancel(__fd, (struct aiocb *)(__aiocbp))
#define aio_error64(__aiocbp) aio_error((struct aiocb *)(__aiocbp))
#define aio_fsync64(__oper, __aiocbp) aio_fsync(__oper, (struct aiocb *)(__aiocbp))
#define aio_return64(__aiocbp) aio_return((struct aiocb *)(__aiocbp))
#define aio_suspend64(__list, __nent, __timeout) aio_suspend((struct aiocb **)__list, __nent, __timeout)
#endif

#if _FILE_OFFSET_BITS - 0 == 64
#if defined(__GNUC__)
/* Use the __ALIAS64 define */
#elif defined(__WATCOMC__)
#pragma aux aio_read "aio_read64";
#pragma aux aio_write "aio_write64";
#pragma aux lio_listio "lio_listio64";
#elif defined(_PRAGMA_REDEFINE_EXTNAME)
#pragma redefine_extname aio_read aio_read64
#pragma redefine_extname aio_write aio_write64
#pragma redefine_extname lio_listio lio_listio64
#else
#define aio_read aio_read64
#define aio_write aio_write64
#define lio_listio lio_listio64
#endif
#endif

extern int aio_cancel(int __fd, struct aiocb *__aiocbp);
extern int aio_error(__const struct aiocb *__aiocbp);
extern int aio_fsync(int __operation, struct aiocb *__aiocbp);
extern int aio_read(struct aiocb *__aiocbp) __ALIAS64("aio_read64");
extern ssize_t aio_return(struct aiocb *__aiocpb);
extern int aio_suspend(__const struct aiocb *__const __list[], int __nent,	__const struct timespec *__timeout);
extern int aio_write(struct aiocb *__aiocpb) __ALIAS64("aio_write64");
extern int lio_listio(int __mode, struct aiocb *__const __list[], int __nent, struct sigevent *__sig) __ALIAS64("lio_listio64");

__END_DECLS

#endif






/* __SRCVERSION("aio.h $Rev: 154597 $"); */
