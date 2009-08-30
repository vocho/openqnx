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
 *  fcntl.h     File control options used by open
 *

 */

#ifndef _FCNTL_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _FCNTL_H_INCLUDED
#endif

#ifndef _FCNTL_H_DECLARED
#define _FCNTL_H_DECLARED

#ifndef _UNISTD_H_INCLUDED
#include <unistd.h>
#endif

#ifndef __STAT_H_INCLUDED
#include <sys/stat.h>
#endif

_C_STD_BEGIN

#if defined(__MODE_T)
typedef __MODE_T	mode_t;
#undef __MODE_T
#endif

_C_STD_END

#if defined(__OFF_T)
typedef __OFF_T		off_t;
#undef __OFF_T
#endif

#if defined(__OFF64_T)
typedef __OFF64_T	off64_t;
#undef __OFF64_T
#endif

#if defined(__PID_T)
typedef __PID_T		pid_t;
#undef __PID_T
#endif

/*
 *  Flag values accessible to both open() and fcntl()
 *  (The first three can only be set by open)
 */

/*
 *  File access modes.
 */

#define O_RDONLY    000000  /*  Read-only mode  */
#define O_WRONLY    000001  /*  Write-only mode */
#define O_RDWR      000002  /*  Read-Write mode */

/*
 *  Mask for file access modes.
 */

#define O_ACCMODE   000003

/*
 *  File status flags used for open() and fcntl().
 */

#define O_NONBLOCK  000200  /*  Non-blocking I/O                */
#define O_APPEND    000010  /*  Append (writes guaranteed at the end)   */

#ifndef O_NDELAY
#define O_NDELAY O_NONBLOCK
#endif

#if defined(_QNX_SOURCE) || (_POSIX_C_SOURCE+0 >= 199309) || (!defined(NO_EXT_KEYS) && !defined(_POSIX_C_SOURCE))
#define O_DSYNC     000020  /*  Data integrity synch    */
#define O_RSYNC     000100  /*  Data integrity synch    */
#define O_SYNC      000040  /*  File integrity synch    */
#endif

/*
 *  oflag values for open()
 */

#define O_CREAT     000400  /*  Opens with file create      */
#define O_TRUNC     001000  /*  Open with truncation        */
#define O_EXCL      002000  /*  Exclusive open          */
#define O_NOCTTY    004000  /*  Don't assign a controlling terminal */

#if defined(__EXT_PCDOS)
#define O_TEXT      000000  /*  Text file   (DOS thing)     */
#define O_BINARY    000000  /*  Binary file (DOS thing)     */
#endif
#if defined(__EXT_QNX)
#define O_CLOEXEC   020000  /*  File descriptor will be close-on-exec */
#define O_REALIDS   040000  /*  Use real uid/gid for permision checking */
#define O_LARGEFILE 0100000 /*  Largest off_t can be 64 bits */
#define O_ASYNC     0200000 /*  Set async i/o               */

/*
 * Mask of flags that change be set with fcntl()
 */
#define O_SETFLAG   (O_NONBLOCK|O_APPEND|O_DSYNC|O_RSYNC|O_SYNC|O_LARGEFILE)
#endif

/*
 *  fcntl() requests
 */

#define F_DUPFD     0       /*  Duplicate file descriptor   */
#define F_GETFD     1       /*  Get file descriptor flags   */
#define F_SETFD     2       /*  Set file descriptor flags   */
#define F_GETFL     3       /*  Get file status flags   */
#define F_SETFL     4       /*  Set file status flags   */

#if _FILE_OFFSET_BITS - 0 == 64
#define F_SETLK     106     /*  Set record locking info */
#define F_SETLKW    107     /*  Set record locking info;    */
#define F_ALLOCSP   110     /*              */
#define F_FREESP    111     /*              */
#define F_GETLK     114     /*  Get record locking info */
#elif !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
#define F_SETLK     6       /*  Set record locking info */
#define F_SETLKW    7       /*  Set record locking info;    */
#define F_ALLOCSP   10      /*              */
#define F_FREESP    11      /*              */
#define F_GETLK     14      /*  Get record locking info */
#else
 #error _FILE_OFFSET_BITS value is unsupported
#endif

#if _LARGEFILE64_SOURCE - 0 > 0
#define F_SETLK64   106     /*              */
#define F_SETLKW64  107     /*              */
#define F_ALLOCSP64 110     /*              */
#define F_FREESP64  111     /*              */
#define F_GETLK64   114     /*  Get record locking info */
#endif

#define F_CHKFL     8       /*              */
#define F_ISSTREAM  13      /*              */
#define F_PRIV      15      /*              */
#define F_NPRIV     16      /*              */
#define F_QUOTACTL  17      /*              */
#define F_BLOCKS    18      /*              */
#define F_BLKSIZE   19      /*              */

#define F_RSETLK    20      /*              */
#define F_RGETLK    21      /*              */
#define F_RSETLKW   22      /*              */

#define F_GETOWN    35  /* get SIGIO/SIGURG proc/pgrp */
#define F_SETOWN    36  /* set SIGIO/SIGURG proc/pgrp */

/*
 *  File descriptor flags used for fcntl()
 */

#define FD_CLOEXEC  0x01    /*  Close on exec       */

/*
 *  l_type values for record locking with fcntl()
 */

#define F_RDLCK     1       /*  Shared or read lock     */
#define F_WRLCK     2       /*  Exclusive or write lock */
#define F_UNLCK     3       /*  Unlock          */

#if defined(__EXT_UNIX_MISC)
/*
 * operation values to use with flock()
 */
#define   LOCK_SH   0x1    /* Shared lock */
#define   LOCK_EX   0x2    /* Exclusive lock */
#define   LOCK_NB   0x4    /* Don't block when locking */
#define   LOCK_UN   0x8    /* Unlock */
#endif

#if defined(__EXT_POSIX1_200112)	/* Approved 1003.1d D14 */
#define POSIX_FADV_NORMAL		0	/* No advice to give */
#define POSIX_FADV_SEQUENTIAL	1	/* Sequentially from lower to higher offsets */
#define POSIX_FADV_RANDOM		2	/* Random order */
#define POSIX_FADV_WILLNEED		3	/* Expects to access specified data */
#define POSIX_FADV_DONTNEED		4	/* Will not access specified data */
#define POSIX_FADV_NOREUSE		5	/* Will access specified data once */
#endif

#include <_pack64.h>

/*
 *  flock structure.
 */

typedef struct flock {
    _Int16t     l_type;
    _Int16t     l_whence;
    _Int32t     l_zero1;
#if _FILE_OFFSET_BITS - 0 == 64
	off_t		l_start;
	off_t		l_len;
#elif !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
#if defined(__LITTLEENDIAN__)
	off_t		l_start;
	off_t		l_start_hi;
	off_t		l_len;
	off_t		l_len_hi;
#elif defined(__BIGENDIAN__)
	off_t		l_start_hi;
	off_t		l_start;
	off_t		l_len_hi;
	off_t		l_len;
#else
 #error endian not configured for system
#endif
#else
 #error _FILE_OFFSET_BITS value is unsupported
#endif
    pid_t       l_pid;
    _Uint32t	l_sysid;	/* node descriptor */
} flock_t;

#if _LARGEFILE64_SOURCE - 0 > 0
typedef struct flock64 {
    _Int16t     l_type;
    _Int16t     l_whence;
    _Int32t     l_zero1;
    off64_t     l_start;
    off64_t     l_len;
    pid_t       l_pid;
    _Uint32t	l_sysid;	/* node descriptor */
} flock64_t;
#endif

#include <_packpop.h>

__BEGIN_DECLS

/*
 *  POSIX 1003.1 Prototypes.
 */

#if _LARGEFILE64_SOURCE - 0 > 0
extern int open64(const char *__path, int __oflag, ...);
extern int creat64(const char *__path, _CSTD mode_t __mode);
#endif

#if defined(__EXT_POSIX1_198808)
extern int open(const char *__path, int __oflag, ...) __ALIAS64("open64");
extern int creat(const char *__path, _CSTD mode_t __mode) __ALIAS64("creat64");
extern int fcntl(int __fildes, int __cmd, ...);
#endif

#if _FILE_OFFSET_BITS - 0 == 64
#if defined(__GNUC__)
/* Use __ALIAS64 define */
#elif defined(__WATCOMC__)
#pragma aux open "open64";
#pragma aux creat "creat64";
#elif defined(_PRAGMA_REDEFINE_EXTNAME)
#pragma redefine_extname open open64
#pragma redefine_extname creat creat64
#else
#define open open64
#define creat creat64
#endif
#endif

#if defined(__EXT_UNIX_MISC)
extern int flock(int __fd, int __operation);
#endif
  

#if defined(__EXT_POSIX1_200112)	/* Approved 1003.1d D14 */
#if _LARGEFILE64_SOURCE - 0 > 0 || _FILE_OFFSET_BITS - 0 == 64
/* since posix changed the prototype, we need to change the external linkage name */
#define posix_fallocate64 posix_fallocate64_large
extern int posix_fadvise64(int __fd, off64_t __offset, off64_t __len, int __advice);
extern int posix_fallocate64(int __fd, off64_t __offset, off64_t __len);
#endif

#if _FILE_OFFSET_BITS-0 == 64
#if defined(__WATCOMC__)
extern int posix_fadvise(int __fd, off_t __offset, off_t __len, int __advice);
#pragma aux posix_fadvise "posix_fadvise64";
extern int posix_fallocate(int __fd, off_t __offset, off_t __len);
#pragma aux posix_fallocate "posix_fallocate64";
#else
static __inline int __attribute__((__unused__)) posix_fadvise(int __fd, off_t __offset, off_t __len, int __advice) {
	return posix_fadvise64(__fd, __offset, __len, __advice);
}
static __inline int __attribute__((__unused__)) posix_fallocate(int __fd, off_t __offset, off_t __len) {
	return posix_fallocate64(__fd, __offset, __len);
}
#endif
#elif !defined(_FILE_OFFSET_BITS) || _FILE_OFFSET_BITS == 32
extern int posix_fadvise(int __fd, off_t __offset, off_t __len, int __advice);
extern int posix_fallocate(int __fd, off_t __offset, off_t __len);
#else
 #error _FILE_OFFSET_BITS value is unsupported
#endif

#endif

#if defined(__EXT_PCDOS)
extern int _vopen(const char *__path, int __oflag, int __sflag, __NTO_va_list __ap);
extern int sopen( const char *__path, int __oflag, int __share, ... );
#endif

#if defined(__EXT_QNX)
struct _io_connect_entry;

extern int _connect_entry(int __base, const char *__path, mode_t __mode, unsigned __oflag, unsigned __sflag, unsigned __subtype, int __testcancel, unsigned __access, unsigned __file_type, unsigned __extra_type, unsigned __extra_len, const void *__extra, unsigned __response_len, void *__response, int *__status, struct _io_connect_entry *__entry, int __enoretry);

extern int _connect_fd(int __base, const char *__path, mode_t __mode, unsigned __oflag, unsigned __sflag, unsigned __subtype, int __testcancel, unsigned __access, unsigned __file_type, unsigned __extra_type, unsigned __extra_len, const void *__extra, unsigned __response_len, void *__response, int *__status, int *fd_len, void *fd_array);

extern int _connect(int __base, const char *__path, mode_t __mode, unsigned __oflag, unsigned __sflag, unsigned __subtype, int __testcancel, unsigned __access, unsigned __file_type, unsigned __extra_type, unsigned __extra_len, const void *__extra, unsigned __response_len, void *__response, int *__status);

extern int _connect_combine(const char *__path, mode_t __mode, unsigned __oflag, unsigned __sflag, int __testcancel, unsigned __file_type, unsigned __extra_len, void *__extra, unsigned __response_len, void *__response);

extern int _connect_object(const char *__name, const char *__prefix, mode_t __mode, int __oflag, unsigned __file_type, unsigned __extra_type, unsigned __extra_len, const void *__extra);

extern int _unlink_object(const char *__name, const char *__prefix, unsigned __file_type);
#endif

__END_DECLS

#endif

#ifdef _STD_USING
using std::mode_t;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("fcntl.h $Rev: 173967 $"); */
