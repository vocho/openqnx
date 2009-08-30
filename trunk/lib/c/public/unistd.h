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
 *  unistd.h
 *

 */
#ifndef _UNISTD_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _UNISTD_H_INCLUDED
#endif

#ifndef _UNISTD_H_DECLARED
#define _UNISTD_H_DECLARED

#ifndef _CONFNAME_H_INCLUDED
#include <confname.h>
#endif

_C_STD_BEGIN

#ifndef NULL
  #define NULL   0
#endif

#if defined(__SSIZE_T)
typedef __SSIZE_T	ssize_t;
#undef __SSIZE_T
#endif

#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
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

#if defined(__UID_T)
typedef __UID_T		uid_t;
#undef __UID_T
#endif

#if defined(__GID_T)
typedef __GID_T		gid_t;
#undef __GID_T
#endif

#if defined(__EXT_XOPEN_EX)

#if defined(__USECONDS_T)
typedef __USECONDS_T	useconds_t;
#undef __USECONDS_T
#endif

#endif

#ifndef _PROCESS_H_INCLUDED
#include <process.h>
#endif

/* Symbolic constants for the access() function */

#define R_OK    4       /*  Test for read permission    */
#define W_OK    2       /*  Test for write permission   */
#define X_OK    1       /*  Test for execute permission */
#define F_OK    0       /*  Test for existence of file  */

/* Symbolic constants for the lseek() function */

#ifndef _SEEKPOS_DEFINED_       /* If not already defined, define them  */
#define SEEK_SET    0           /* Seek relative to the start of file   */
#define SEEK_CUR    1           /* Seek relative to current position    */
#define SEEK_END    2           /* Seek relative to the end of the file */
#define _SEEKPOS_DEFINED_
#endif

/* Symbolic constants for stream I/O */

#define STDIN_FILENO    0
#define STDOUT_FILENO   1
#define STDERR_FILENO   2

/* Symbolic constants for lockf() function */

#define F_ULOCK 0      /* Unlock locked sections */
#define F_LOCK  1      /* Lock a section for exclusive use */
#define F_TLOCK 2      /* Test and lock a section for exclusive use */
#define F_TEST  3      /* Test a lock for locks by other processes */

/* Compile-time Symbolic Constants for Portability Specifications */

#undef _POSIX_VERSION
#undef _POSIX2_VERSION
#if   defined(__EXT_POSIX1_200112)
#define _POSIX_VERSION		200112L
#define _POSIX2_VERSION		200112L
#elif defined(__EXT_POSIX1_199506)
#define _POSIX_VERSION		199506L
#define _POSIX2_VERSION		199209L
#elif defined(__EXT_POSIX1_199309)
#define _POSIX_VERSION		199309L
#define _POSIX2_VERSION		199209L
#elif defined(__EXT_POSIX1_199009)
#define _POSIX_VERSION		199009L
#define _POSIX2_VERSION		1
#elif defined(__EXT_POSIX1_198808)
#define _POSIX_VERSION		198808L
#define _POSIX2_VERSION		1
#endif

#if defined(__EXT_POSIX1_199009)
#undef	_POSIX_JOB_CONTROL
#define	_POSIX_SAVED_IDS					1
#endif

#if defined(__EXT_POSIX1_199309)
#define	_POSIX_ASYNCHRONOUS_IO				_POSIX_VERSION
#define	_POSIX_FSYNC						_POSIX_VERSION
#define	_POSIX_MAPPED_FILES					_POSIX_VERSION
#define	_POSIX_MEMLOCK						_POSIX_VERSION
#define	_POSIX_MEMLOCK_RANGE				_POSIX_VERSION
#define	_POSIX_MEMORY_PROTECTION			_POSIX_VERSION
#define	_POSIX_MESSAGE_PASSING				_POSIX_VERSION
#define	_POSIX_PRIORITIZED_IO				_POSIX_VERSION
#define	_POSIX_PRIORITY_SCHEDULING			_POSIX_VERSION
#define	_POSIX_REALTIME_SIGNALS				_POSIX_VERSION
#define	_POSIX_SEMAPHORES					_POSIX_VERSION
#define	_POSIX_SHARED_MEMORY_OBJECTS		_POSIX_VERSION
#define	_POSIX_SYNCHRONIZED_IO				_POSIX_VERSION
#define	_POSIX_TIMERS						_POSIX_VERSION
#endif

#if defined(__EXT_POSIX1_199506)
#define	_POSIX_THREADS						_POSIX_VERSION
#define	_POSIX_THREAD_PRIO_INHERIT			_POSIX_VERSION
#define	_POSIX_THREAD_PRIO_PROTECT			_POSIX_VERSION
#define	_POSIX_THREAD_PRIORITY_SCHEDULING	_POSIX_VERSION
#define	_POSIX_THREAD_ATTR_STACKADDR		_POSIX_VERSION
#define	_POSIX_THREAD_SAFE_FUNCTIONS		_POSIX_VERSION
#define	_POSIX_THREAD_PROCESS_SHARED		_POSIX_VERSION
#define	_POSIX_THREAD_ATTR_STACKSIZE		_POSIX_VERSION
#endif

#if defined(__EXT_POSIX1_200112)
/* POSIX 1003.1d D14 */
#undef	_POSIX_ADVISORY_INFO
#undef	_POSIX_CPUTIME
#define	_POSIX_SPAWN						_POSIX_VERSION
#define	_POSIX_SPORADIC_SERVER				_POSIX_VERSION
#undef	_POSIX_THREAD_CPUTIME
#define	_POSIX_TIMEOUTS						_POSIX_VERSION
#define	_POSIX_THREAD_SPORADIC_SERVER		_POSIX_VERSION

/* POSIX 1003.1j D10 */
#define	_POSIX_BARRIERS						_POSIX_VERSION
#define	_POSIX_CLOCK_SELECTION				_POSIX_VERSION
#define	_POSIX_MONOTONIC_CLOCK				_POSIX_VERSION
#define	_POSIX_READER_WRITER_LOCKS			_POSIX_VERSION
#define	_POSIX_SPIN_LOCKS					_POSIX_VERSION
#define	_POSIX_TYPED_MEMORY_OBJECTS			_POSIX_VERSION

/* POSIX 1003.1q-2000 */
#undef _POSIX_TRACE_EVENT_FILTER
#undef _POSIX_TRACE
#undef _POSIX_TRACE_INHERIT
#undef _POSIX_TRACE_LOG

#undef _POSIX_IPV6
#undef _POSIX_RAW_SOCKETS

/* POSIX 1003.1-2001 states job control is mandatory */
#define _POSIX_JOB_CONTROL	1

/* POSIX 1003.1-2001 states regular expression handling is mandatory */
#define _POSIX_REGEXP		1

/* POSIX 1003.1-2001 states POSIX shell is mandatory */
#define _POSIX_SHELL		1

#define _POSIX_V6_ILP32_OFF32	1
#define _POSIX_V6_ILP32_OFFBIG	1
#undef	_POSIX_V6_LP64_OFF64
#undef	_POSIX_V6_LPBIG_OFFBIG
#endif

#if defined(__EXT_POSIX2)
#define _POSIX2_C_VERSION	_POSIX2_VERSION
#define _POSIX2_C_BIND		_POSIX2_VERSION
#define _POSIX2_CHAR_TERM	_POSIX2_VERSION
#define _POSIX2_LOCALEDEF	_POSIX2_VERSION
#define _POSIX2_UPE			_POSIX2_VERSION
#define _POSIX2_FORT_DEV	_POSIX2_VERSION
#define _POSIX2_FORT_RUN	_POSIX2_VERSION
#define _POSIX2_LOCALEDEF	_POSIX2_VERSION
#define _POSIX2_SW_DEV		_POSIX2_VERSION
#endif

#if defined(__EXT_XOPEN_EX)
#define _XOPEN_VERSION		500	
#define _XOPEN_XCU_VERSION	1
#define _XOPEN_XPG2             1
#define _XOPEN_XPG3             1
#define _XOPEN_XPG4             1
#define _XOPEN_UNIX             1
#define _XOPEN_CRYPT		1
#define _XOPEN_ENH_I18N		1
#define _XOPEN_LEGACY		1
#define _XOPEN_REALTIME		1
#define _XOPEN_REALTIME_THREADS	1
#define _XOPEN_SHM		1
#define _XBS5_ILP32_OFF32	1
#define _XBS5_ILP32_OFFBIG	1
#define _XBS5_LP64_OFF64	1
#define _XBS5_LPBIG_OFFBIG	1
#endif

#if defined(__EXT_QNX)
/* POSIX 1003.1d Draft 8 */
#define _POSIX_DEVICE_CONTROL       1
#define _POSIX_DEVCTL_DIRECTION     1
#define _POSIX_INTERRUPT_CONTROL    1
/* POSIX 1003.1j Draft 5 */
#undef _POSIX_PROCESS_SPIN_LOCKS
#undef _POSIX_THREAD_SPIN_LOCKS
#undef _POSIX_TYPED_MEMORY_ACCESS_MGMT
#undef _POSIX_SYNCHRONIZED_CLOCK
#define _POSIX_THREAD_ASYNC_ABORT	1
#endif

/* Execution-time Symbolic Constants for Portability Specifications */

#if defined(__EXT_POSIX1_199009)
#define _POSIX_CHOWN_RESTRICTED 1       /* restricted use of chown() */
#define _POSIX_NO_TRUNC         1       /* pathname components > NAME_MAX */
#define _POSIX_VDISABLE         0       /* terminal special chars can be disabled */
#endif

#if defined(__EXT_POSIX1_199309)
#define _POSIX_ASYNC_IO         1
#define _POSIX_PRIO_IO          1
#define _POSIX_SYNC_IO          1
#endif

__BEGIN_DECLS

#if defined(__EXT_POSIX2) && !defined(__EXT_UNIX_HIST)
#if defined(__SLIB_DATA_INDIRECT) && !defined(optind) && !defined(__SLIB)
   int *__get_optind_ptr(void);
   #define optind (*__get_optind_ptr())
#else
   extern int   optind;        /*  index of current option being scanned */
#endif

#if defined(__SLIB_DATA_INDIRECT) && !defined(optarg) && !defined(__SLIB)
   char **__get_optarg_ptr(void);
   #define optarg (*__get_optarg_ptr())
#else
   extern char *optarg;        /*  points to optional argument */
#endif

#if defined(__SLIB_DATA_INDIRECT) && !defined(opterr) && !defined(__SLIB)
   int *__get_opterr_ptr(void);
   #define opterr (*__get_opterr_ptr())
#else
   extern int   opterr;        /*  print|don't print error message */
#endif

#if defined(__SLIB_DATA_INDIRECT) && !defined(optopt) && !defined(__SLIB)
   int *__get_optopt_ptr(void);
   #define optopt (*__get_optopt_ptr())
#else
   extern int   optopt;        /*  offending letter when error detected */
#endif
#endif

#if defined(__EXT_QNX)
#ifndef SETIOV
#define SETIOV(_iov, _addr, _len)	((_iov)->iov_base = (void *)(_addr), (_iov)->iov_len = (_len))
#endif
#ifndef GETIOVBASE
#define GETIOVBASE(_iov)            ((_iov)->iov_base)
#endif
#ifndef GETIOVLEN
#define GETIOVLEN(_iov)              ((_iov)->iov_len)
#endif
#endif

/*
 *  POSIX 1003.1 Prototypes
 */

#if _LARGEFILE64_SOURCE - 0 > 0
extern off64_t  lseek64(int __fildes, off64_t __offset, int __whence);
extern int      ftruncate64(int __fd, off64_t __length);
extern _CSTD ssize_t	pread64(int __filedes, void *__buff, _CSTD size_t __nbytes, off64_t __offset);
extern _CSTD ssize_t	pwrite64(int __filedes, const void *__buff, _CSTD size_t __nbytes, off64_t __offset);
#if defined(__EXT_QNX)
extern off64_t  tell64(int __fildes);
#endif
#if defined(__EXT_XOPEN_EX)
extern int      truncate64(const char *__path, off64_t __length);
extern int	    lockf64(int __fd, int __function, off64_t __size); 
#endif
#endif

#if defined(__EXT_POSIX1_198808) && !defined(__EXT_UNIX_HIST)
extern char     *ctermid( char * );
#endif

#if defined(__EXT_POSIX1_198808)
extern int      access( const char *__path, int __mode );
extern unsigned alarm( unsigned int __seconds );
extern int      chdir( const char *__path );
extern int      chown( const char *__path, uid_t __owner, gid_t __group );
extern int      close( int __fildes );
extern _CSTD size_t   confstr( int, char*, _CSTD size_t );
#if defined(__EXT_QNX)
extern void	confstr_cache_enable( void );
extern void	confstr_cache_disable( void );
extern void	confstr_cache_invalidate( void );
extern void	confstr_cache_attach( void );
#endif
#if defined(__NYI)
extern char     *cuserid( char * ); 
#endif
extern int      dup( int __fildes );
extern int      dup2( int __fildes, int __fildes2 );
extern int      fchown( int __fildes, uid_t __owner, gid_t __group );
extern long     fpathconf( int __fildes, int __name );
extern char     *getcwd( char *__buf, _CSTD size_t __size );
extern gid_t    getegid( void );
extern uid_t    geteuid( void );
extern gid_t    getgid( void );
extern int      getgroups( int __gidsetsize, gid_t __grouplist[] );
extern char     *getlogin( void );
extern pid_t    setsid( void );
extern uid_t    getuid( void );
extern int      isatty( int __fildes );
extern int      link( const char *__path1, const char *__path2 );
extern off_t    lseek(int __fildes, off_t __offset, int __whence) __ALIAS64("lseek64");
extern long     pathconf( const char *__path, int __name );
extern int      pause( void );
extern int      pipe( int __fildes[2] );
extern _CSTD ssize_t	pread(int __filedes, void *__buff, _CSTD size_t __nbytes, off_t __offset) __ALIAS64("pread64");
extern _CSTD ssize_t	pwrite(int __filedes, const void *__buff, _CSTD size_t __nbytes, off_t __offset) __ALIAS64("pwrite64");
extern _CSTD ssize_t  read( int __fildes, void *__buffer, _CSTD size_t __len );
extern int      readlink( const char *__path, char *__buf, _CSTD size_t __bufsiz );
extern int      rmdir( const char *__path );
extern int      setgid( gid_t __newgroup );
extern int      setuid( uid_t __newuserid );
extern unsigned int sleep( unsigned int __seconds );
extern int      symlink( const char *__pname, const char *__slink );
extern void     sync( void );
extern long     sysconf( int __name );
extern pid_t    tcgetpgrp( int __fildes );
extern int      tcsetpgrp( int __fildes, pid_t __pgrp_id );
extern char     *ttyname( int __fildes );
extern int      unlink( const char *__path );
extern _CSTD ssize_t  write( int __fildes, const void *__buf, _CSTD size_t __len );
#endif

#if defined(__EXT_POSIX1_199309)
extern int      fdatasync(int __fildes);
extern int      fsync(int __fildes);
extern int      ftruncate(int __fd, off_t __length) __ALIAS64("ftruncate64");
#endif

#if defined(__EXT_POSIX1_199506)
extern int	getlogin_r(char* __name, _CSTD size_t __namesize);
extern int  ttyname_r( int __fildes, char *__buf, _CSTD size_t __bufsize );
#endif

#if _FILE_OFFSET_BITS - 0 == 64
#if defined(__GNUC__)
/* Use __ALIAS64 define */
#elif defined(__WATCOMC__)
#pragma aux lseek "lseek64";
#pragma aux pread "pread64";
#pragma aux pwrite "pwrite64";
#if defined(__EXT_POSIX1_199309)
#pragma aux ftruncate "ftruncate64";
#endif
#if defined(__EXT_QNX)
#pragma aux tell "tell64";
#endif
#if defined(__EXT_XOPEN_EX)
#pragma aux truncate "truncate64";
#pragma aux lockf "lockf64";
#endif
#elif defined(_PRAGMA_REDEFINE_EXTNAME)
#pragma redefine_extname lseek lseek64
#pragma redefine_extname pread pread64
#pragma redefine_extname pwrite pwrite64
#if defined(__EXT_POSIX1_199309)
#pragma redefine_extname ftruncate ftruncate64
#endif
#if defined(__EXT_QNX)
#pragma redefine_extname tell tell64
#endif
#if defined(__EXT_XOPEN_EX)
#pragma redefine_extname truncate truncate64
#pragma redefine_extname lockf lockf64
#endif
#else
#define lseek lseek64
#define pread pread64
#define pwrite pwrite64
#if defined(__EXT_POSIX1_199309)
#define ftruncate ftruncate64
#endif
#if defined(__EXT_QNX)
#define tell tell64
#endif
#if defined(__EXT_XOPEN_EX)
#define truncate truncate64
#define lockf lockf64
#endif
#endif
#endif

#if defined(__EXT_UNIX_MISC)
extern int sethostname(const char *__buffer, _CSTD size_t __buffer_length);
extern int setgroups( int __gidsetsize, const gid_t *__grouplist );
extern int getgrouplist(const char *__uname, gid_t __agroup, gid_t *__groups, int *__grpcnt);
/* extern int mount( const char *__special, const char * __dir, int __rwflag ); */
/* extern int umount( const char *__special ); */
extern int	rcmd(char **__ahost, unsigned short __inport, const char *__locuser, const char *__remuser, const char *__cmd, int *__fd2p);
extern int	rcmd_af(char **, unsigned short, const char *, const char *, const char *, int*, int);
extern int 	rresvport(int *__port);
extern int	rresvport_af(int *, int);
extern int 	ruserok(char *__rhost, int __superuser, char *__ruser, char *__luser);
extern int	iruserok(_Uint32t, int, const char *, const char *);
extern int	iruserok_sa(const void *, int, int, const char *, const char *);
#endif

#if defined(__EXT_XOPEN_EX)
extern int      gethostname(char *__buffer, _CSTD size_t __buffer_length);
extern int      brk(void *__endds);
extern int      chroot(const char *__path);
extern char		*crypt(const char *, const char *);
extern void		encrypt(char[64], int); 
extern int		fchdir(int __fd);
/* extern long	gethostid(void); */
extern int	getdtablesize(void);
extern int	getpagesize(void); /* legacy */
extern char		*getpass(const char *); 
extern pid_t	getpgid(pid_t __pid);
extern pid_t    getsid(pid_t __pid);
extern char	*getwd(char *);
extern int	lchown( const char *__path, uid_t __owner, gid_t __group );
extern int	lockf(int __fd, int __function, off_t __size) __ALIAS64("lockf64"); 
extern int	nice(int);
extern void     *sbrk(int __incr);
extern pid_t	setpgrp(void);
extern int      setregid( gid_t __readgroupid, gid_t __effectivegroupid );
extern int      setreuid( uid_t __readuserid, uid_t __effectiveuserid );
extern void      swab( char *__src, char *__dest, int __num );
extern int      truncate(const char *__path, off_t __length) __ALIAS64("truncate64");
extern useconds_t ualarm(useconds_t __usec, useconds_t __interval );
extern		int usleep(useconds_t __useconds);
#endif

#if defined(__EXT_POSIX2)
#if !defined(__EXT_UNIX_HIST)
extern int      getopt( int __argc, char * const __argv[], const char * __optstring );
#endif
#endif

#if defined(__EXT_UNIX_MISC)
extern int      setegid(gid_t __newegroup);
extern int      seteuid(uid_t __newuserid);
#endif

#if defined(__EXT_QNX)
struct sigevent;

extern unsigned delay(unsigned int __milliseconds);
extern _CSTD ssize_t  _readx(int __fildes, void *__buffer, _CSTD size_t __len, unsigned __xtype, void *__xdata, _CSTD size_t __xdatalen);
extern int      readblock(int __fd, _CSTD size_t __blksize, unsigned __block, int __numblks, void *__buff);
extern _CSTD ssize_t  _writex(int __fildes, const void *__buffer, _CSTD size_t __len, unsigned __xtype, void *__xdata, _CSTD size_t __xdatalen);
extern int      writeblock(int __fd, _CSTD size_t __blksize, unsigned __block, int __numblks, const void *__buff);
extern int      readcond(int __fd, void *__buff, int __nbytes, int __min, int __time, int __timeout);
extern int      ionotify(int __fd, int __action, int __flags, const struct sigevent *__event);
extern int      chsize( int __fildes, long __size );
extern off_t    tell(int __fildes) __ALIAS64("tell64");
extern int      eof( int __fildes );
#if _FILE_OFFSET_BITS-0 != 64
extern off_t    ltrunc( int __fildes, off_t __offset, int __origin );
#endif
extern int      _sopenfd(int __fd, int __oflag, int __sflag, int __xtype);
extern int      sopenfd(int __fd, int __oflag, int __sflag);
extern int      openfd(int __fd, int __oflag);
extern char		*qnx_crypt(const char *, const char *);
extern int      flink(int __fd, const char *__path);
extern int 	getdomainname(char *__name, _CSTD size_t __namelen);
extern int	setdomainname(const char *__name, _CSTD size_t __namelen);

#if defined(__SLIB_DATA_INDIRECT) && !defined(environ) && !defined(__SLIB)
    extern char **__get_environ_ptr(void);
    extern void __set_environ_ptr(char **__newptr);
    #define environ (__get_environ_ptr())
#else
    extern char **environ;   /*  pointer to environment table        */
#endif

#if defined(__SLIB_DATA_INDIRECT) && !defined(_connect_malloc) && !defined(__SLIB)
   char *__get_connect_malloc_ptr(void);
   #define _connect_malloc (*__get_connect_malloc_ptr())
#else
   extern char   _connect_malloc;   /*  connect malloc*/
#endif

#endif

__END_DECLS

#endif

#ifdef _STD_USING
using std::size_t; using std::ssize_t;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("unistd.h $Rev: 175131 $"); */
