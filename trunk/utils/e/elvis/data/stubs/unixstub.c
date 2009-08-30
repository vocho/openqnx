/* <errno.h> ==================================== */

/* <errno.h> */
typedef int error_t;

/* <errno.h> */
/* To be thread-safe, this is often defined as a macro which calls a function
 * to return the current thread's error status.
 */
int errno;


/* <errno.h> */
#define	EPERM		 1	/* Operation not permitted */

/* <errno.h> */
#define	ENOENT		 2	/* No such file or directory */

/* <errno.h> */
#define	ESRCH		 3	/* No such process */

/* <errno.h> */
#define	EINTR		 4	/* Interrupted system call */

/* <errno.h> */
#define	EIO		 5	/* I/O error */

/* <errno.h> */
#define	ENXIO		 6	/* No such device or address */

/* <errno.h> */
#define	E2BIG		 7	/* Argument list too long */

/* <errno.h> */
#define	ENOEXEC		 8	/* Exec format error */

/* <errno.h> */
#define	EBADF		 9	/* Bad file number */

/* <errno.h> */
#define	ECHILD		10	/* No child processes */

/* <errno.h> */
#define	EAGAIN		11	/* Try again */

/* <errno.h> */
#define	EWOULDBLOCK	EAGAIN	/* Operation would block */

/* <errno.h> */
#define	ENOMEM		12	/* Out of memory */

/* <errno.h> */
#define	EACCES		13	/* Permission denied */

/* <errno.h> */
#define	EFAULT		14	/* Bad address */

/* <errno.h> */
#define	ENOTBLK		15	/* Block device required */

/* <errno.h> */
#define	EBUSY		16	/* Device or resource busy */

/* <errno.h> */
#define	EEXIST		17	/* File exists */

/* <errno.h> */
#define	EXDEV		18	/* Cross-device link */

/* <errno.h> */
#define	ENODEV		19	/* No such device */

/* <errno.h> */
#define	ENOTDIR		20	/* Not a directory */

/* <errno.h> */
#define	EISDIR		21	/* Is a directory */

/* <errno.h> */
#define	EINVAL		22	/* Invalid argument */

/* <errno.h> */
#define	ENFILE		23	/* File table overflow */

/* <errno.h> */
#define	EMFILE		24	/* Too many open files */

/* <errno.h> */
#define	ENOTTY		25	/* Not a typewriter */

/* <errno.h> */
#define	ETXTBSY		26	/* Text file busy */

/* <errno.h> */
#define	EFBIG		27	/* File too large */

/* <errno.h> */
#define	ENOSPC		28	/* No space left on device */

/* <errno.h> */
#define	ESPIPE		29	/* Illegal seek */

/* <errno.h> */
#define	EROFS		30	/* Read-only file system */

/* <errno.h> */
#define	EMLINK		31	/* Too many links */

/* <errno.h> */
#define	EPIPE		32	/* Broken pipe */

/* <errno.h> */
#define	EDOM		33	/* Math argument out of domain of func */

/* <errno.h> */
#define	ERANGE		34	/* Math result not representable */

/* <errno.h> */
#define	EDEADLK		35	/* Resource deadlock would occur */

/* <errno.h> */
#define	EDEADLOCK	35	/* Resource deadlock would occur */

/* <errno.h> */
#define	ENAMETOOLONG	36	/* File name too long */

/* <errno.h> */
#define	ENOLCK		37	/* No record locks available */

/* <errno.h> */
#define	ENOSYS		38	/* Function not implemented */

/* <errno.h> */
#define	ENOTEMPTY	39	/* Directory not empty */

/* <errno.h> */
#define	ELOOP		40	/* Too many symbolic links encountered */

/* <errno.h> */
#define	ENOMSG		42	/* No message of desired type */

/* <errno.h> */
#define	EIDRM		43	/* Identifier removed */

/* <errno.h> */
#define	ECHRNG		44	/* Channel number out of range */

/* <errno.h> */
#define	EL2NSYNC	45	/* Level 2 not synchronized */

/* <errno.h> */
#define	EL3HLT		46	/* Level 3 halted */

/* <errno.h> */
#define	EL3RST		47	/* Level 3 reset */

/* <errno.h> */
#define	ELNRNG		48	/* Link number out of range */

/* <errno.h> */
#define	EUNATCH		49	/* Protocol driver not attached */

/* <errno.h> */
#define	ENOCSI		50	/* No CSI structure available */

/* <errno.h> */
#define	EL2HLT		51	/* Level 2 halted */

/* <errno.h> */
#define	EBADE		52	/* Invalid exchange */

/* <errno.h> */
#define	EBADR		53	/* Invalid request descriptor */

/* <errno.h> */
#define	EXFULL		54	/* Exchange full */

/* <errno.h> */
#define	ENOANO		55	/* No anode */

/* <errno.h> */
#define	EBADRQC		56	/* Invalid request code */

/* <errno.h> */
#define	EBADSLT		57	/* Invalid slot */

/* <errno.h> */
#define	EBFONT		59	/* Bad font file format */

/* <errno.h> */
#define	ENOSTR		60	/* Device not a stream */

/* <errno.h> */
#define	ENODATA		61	/* No data available */

/* <errno.h> */
#define	ETIME		62	/* Timer expired */

/* <errno.h> */
#define	ENOSR		63	/* Out of streams resources */

/* <errno.h> */
#define	ENONET		64	/* Machine is not on the network */

/* <errno.h> */
#define	ENOPKG		65	/* Package not installed */

/* <errno.h> */
#define	EREMOTE		66	/* Object is remote */

/* <errno.h> */
#define	ENOLINK		67	/* Link has been severed */

/* <errno.h> */
#define	EADV		68	/* Advertise error */

/* <errno.h> */
#define	ESRMNT		69	/* Srmount error */

/* <errno.h> */
#define	ECOMM		70	/* Communication error on send */

/* <errno.h> */
#define	EPROTO		71	/* Protocol error */

/* <errno.h> */
#define	EMULTIHOP	72	/* Multihop attempted */

/* <errno.h> */
#define	EDOTDOT		73	/* RFS specific error */

/* <errno.h> */
#define	EBADMSG		74	/* Not a data message */

/* <errno.h> */
#define	EOVERFLOW	75	/* Value too large for defined data type */

/* <errno.h> */
#define	ENOTUNIQ	76	/* Name not unique on network */

/* <errno.h> */
#define	EBADFD		77	/* File descriptor in bad state */

/* <errno.h> */
#define	EREMCHG		78	/* Remote address changed */

/* <errno.h> */
#define	ELIBACC		79	/* Can not access a needed shared library */

/* <errno.h> */
#define	ELIBBAD		80	/* Accessing a corrupted shared library */

/* <errno.h> */
#define	ELIBSCN		81	/* .lib section in a.out corrupted */

/* <errno.h> */
#define	ELIBMAX		82	/* Attempting to link in too many shared libraries */

/* <errno.h> */
#define	ELIBEXEC	83	/* Cannot exec a shared library directly */

/* <errno.h> */
#define	EILSEQ		84	/* Illegal byte sequence */

/* <errno.h> */
#define	ERESTART	85	/* Interrupted system call should be restarted */

/* <errno.h> */
#define	ESTRPIPE	86	/* Streams pipe error */

/* <errno.h> */
#define	EUSERS		87	/* Too many users */

/* <errno.h> */
#define	ENOTSOCK	88	/* Socket operation on non-socket */

/* <errno.h> */
#define	EDESTADDRREQ	89	/* Destination address required */

/* <errno.h> */
#define	EMSGSIZE	90	/* Message too long */

/* <errno.h> */
#define	EPROTOTYPE	91	/* Protocol wrong type for socket */

/* <errno.h> */
#define	ENOPROTOOPT	92	/* Protocol not available */

/* <errno.h> */
#define	EPROTONOSUPPORT	93	/* Protocol not supported */

/* <errno.h> */
#define	ESOCKTNOSUPPORT	94	/* Socket type not supported */

/* <errno.h> */
#define	EOPNOTSUPP	95	/* Operation not supported on transport endpoint */

/* <errno.h> */
#define	EPFNOSUPPORT	96	/* Protocol family not supported */

/* <errno.h> */
#define	EAFNOSUPPORT	97	/* Address family not supported by protocol */

/* <errno.h> */
#define	EADDRINUSE	98	/* Address already in use */

/* <errno.h> */
#define	EADDRNOTAVAIL	99	/* Cannot assign requested address */

/* <errno.h> */
#define	ENETDOWN	100	/* Network is down */

/* <errno.h> */
#define	ENETUNREACH	101	/* Network is unreachable */

/* <errno.h> */
#define	ENETRESET	102	/* Network dropped connection because of reset */

/* <errno.h> */
#define	ECONNABORTED	103	/* Software caused connection abort */

/* <errno.h> */
#define	ECONNRESET	104	/* Connection reset by peer */

/* <errno.h> */
#define	ENOBUFS		105	/* No buffer space available */

/* <errno.h> */
#define	EISCONN		106	/* Transport endpoint is already connected */

/* <errno.h> */
#define	ENOTCONN	107	/* Transport endpoint is not connected */

/* <errno.h> */
#define	ESHUTDOWN	108	/* Cannot send after transport endpoint shutdown */

/* <errno.h> */
#define	ETOOMANYREFS	109	/* Too many references: cannot splice */

/* <errno.h> */
#define	ETIMEDOUT	110	/* Connection timed out */

/* <errno.h> */
#define	ECONNREFUSED	111	/* Connection refused */

/* <errno.h> */
#define	EHOSTDOWN	112	/* Host is down */

/* <errno.h> */
#define	EHOSTUNREACH	113	/* No route to host */

/* <errno.h> */
#define	EALREADY	114	/* Operation already in progress */

/* <errno.h> */
#define	EINPROGRESS	115	/* Operation now in progress */

/* <errno.h> */
#define	ESTALE		116	/* Stale NFS file handle */

/* <errno.h> */
#define	EUCLEAN		117	/* Structure needs cleaning */

/* <errno.h> */
#define	ENOTNAM		118	/* Not a XENIX named type file */

/* <errno.h> */
#define	ENAVAIL		119	/* No XENIX semaphores available */

/* <errno.h> */
#define	EISNAM		120	/* Is a named type file */

/* <errno.h> */
#define	EREMOTEIO	121	/* Remote I/O error */

/* <errno.h> */
#define	EDQUOT		122	/* Quota exceeded */

/* <errno.h> */
#define	ENOMEDIUM	123	/* No medium found */

/* <errno.h> */
#define	EMEDIUMTYPE	124	/* Wrong medium type */

/* <fcntl.h> ==================================== */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_ACCMODE	   0003 /* mask for the read/write bitfield */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_RDONLY	     00

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_WRONLY	     01

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_RDWR		     02

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_CREAT		   0100	/* not fcntl */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_EXCL		   0200	/* not fcntl */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_NOCTTY	   0400	/* not fcntl */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_TRUNC		  01000	/* not fcntl */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_APPEND	  02000

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_NONBLOCK	  04000

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_NDELAY	O_NONBLOCK

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_SYNC		 010000

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_FSYNC		 O_SYNC

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_ASYNC		 020000

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_DIRECT	 040000	/* Direct disk access. (GNU only) */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_LARGEFILE	0100000 /* 64-bit offsets (not portable) */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_DIRECTORY	0200000	/* Must be a directory.	(GNU only)*/

/* <fcntl.h> (really <bits/fcntl.h>) */
#define O_NOFOLLOW	0400000	/* Do not follow links.	(GNU only)*/

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_DUPFD		0	/* Duplicate file descriptor.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_GETFD		1	/* Get file descriptor flags.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_SETFD		2	/* Set file descriptor flags.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_GETFL		3	/* Get file status flags.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_SETFL		4	/* Set file status flags.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_GETLK		5	/* Get record locking info.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_SETLK		6	/* Set record locking info (non-blocking).  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_SETLKW	7	/* Set record locking info (blocking).	*/

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_GETLK64	12	/* Get record locking info.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_SETLK64	13	/* Set record locking info (non-blocking).  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_SETLKW64	14	/* Set record locking info (blocking).	*/

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_SETOWN	8	/* Get owner of socket (receiver of SIGIO).  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_GETOWN	9	/* Set owner of socket (receiver of SIGIO).  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_SETSIG	10	/* Set number of signal to be sent. (GNU only) */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_GETSIG	11	/* Get number of signal to be sent. (GNU only) */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_SETLEASE	1024	/* Set a lease. (GNU only) */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_GETLEASE	1025	/* Enquire what lease is active. (GNU only) */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_NOTIFY	1026	/* Request notfications on a directory. (GNU only) */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define FD_CLOEXEC	1	/* For F_[GET|SET]FL.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_RDLCK		0	/* Read lock.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_WRLCK		1	/* Write lock.	*/

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_UNLCK		2	/* Remove lock.	 */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_EXLCK		4	/* or 3... either way, this is obsolete */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define F_SHLCK		8	/* or 4... either way, this is obsolete */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define DN_ACCESS	0x00000001	/* Notify when File accessed.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define DN_MODIFY	0x00000002	/* Notify when File modified.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define DN_CREATE	0x00000004	/* Notify when File created.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define DN_DELETE	0x00000008	/* Notify when File removed.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define DN_RENAME	0x00000010	/* Notify when File renamed.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define DN_ATTRIB	0x00000020	/* Notify when File changed attibutes.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
#define DN_MULTISHOT	0x80000000	/* Don't remove notifier.  */


/* <fcntl.h> (really <bits/fcntl.h>) arg to posix_fadvise() */
#define POSIX_FADV_NORMAL	0 /* No further special treatment.  */

/* <fcntl.h> (really <bits/fcntl.h>) arg to posix_fadvise() */
#define POSIX_FADV_RANDOM	1 /* Expect random page references.  */

/* <fcntl.h> (really <bits/fcntl.h>) arg to posix_fadvise() */
#define POSIX_FADV_SEQUENTIAL	2 /* Expect sequential page references.	 */

/* <fcntl.h> (really <bits/fcntl.h>) arg to posix_fadvise() */
#define POSIX_FADV_WILLNEED	3 /* Will need these pages.  */

/* <fcntl.h> (really <bits/fcntl.h>) arg to posix_fadvise() */
#define POSIX_FADV_DONTNEED	4 /* Don't need these pages.  */

/* <fcntl.h> (really <bits/fcntl.h>) arg to posix_fadvise() */
#define POSIX_FADV_NOREUSE	5 /* Data will be accessed once.  */

/* <fcntl.h> (really <bits/fcntl.h>) */
/* if __USE_FILE_OFFSET64 is defined, then off_t becomes off64_t */
struct flock
{
    short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.	*/
    short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
    off_t l_start;	/* Offset where the lock begins.  */
    off_t l_len;	/* Size of the locked area; zero means until EOF.  */
    pid_t l_pid;	/* Process holding the lock.  */
};

/* <fcntl.h> (really <bits/fcntl.h>) */
struct flock64
{
    short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.	*/
    short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
    __off64_t l_start;	/* Offset where the lock begins.  */
    __off64_t l_len;	/* Size of the locked area; zero means until EOF.  */
    __pid_t l_pid;	/* Process holding the lock.  */
};


/* <fcntl.h> */
int fcntl(int fd, int cmd, ...)
{
}

/* <fcntl.h> */
/* When oflag includes O_CREAT, a third arg gives the file's permissions */
int open(const char *filename, int oflag, ...)
{
}

/* <fcntl.h> */
int open64(const char *filename, int oflag, ...)
{
}

/* <fcntl.h> */
int creat(const char *filename, int mode)
{
}

/* <fcntl.h> */
int creat64(const char *filename, int mode)
{
}

/* <fcntl.h> */
int lockf(int fd, int cmd, off_t len);
{
}

/* <fcntl.h> */
int lockf64(int fd, int cmd, off64_t len);
{
}

/* <fcntl.h> */
int posix_fadvise(int fd, off_t offset, size_t len, int advise)
{
}

/* <fcntl.h> */
int posix_fadvise64(int fd, off_t offset, size_t len, int advise)
{
}

/* <fcntl.h> */
int posix_fallocate(int fd, off_t offset, size_t len)
{
}

/* <fcntl.h> */
int posix_fallocate64(int fd, off64_t offset, size_t len)
{
}


/* <sys/ioctl.h> ==================================== */

/* <unistd.h> ==================================== */

/* <unistd.h> */
# define F_LOCK  1	/* Lock a region for exclusive use.  */

/* <unistd.h> */
#define	F_OK	0		/* access() - Test for existence.  */

/* <unistd.h> */
# define F_TEST  3	/* Test a region for other processes locks.  */

/* <unistd.h> */
# define F_TLOCK 2	/* Test and lock a region for exclusive use.  */

/* <unistd.h> */
# define F_ULOCK 0	/* Unlock a previously locked region.  */

/* <unistd.h> */
# define L_INCR		SEEK_CUR

/* <unistd.h> */
/* Old BSD names for the same constants; just for compatibility.  */
# define L_SET		SEEK_SET

/* <unistd.h> */
# define L_XTND		SEEK_END

/* <unistd.h> */
#define	R_OK	4		/* access() - Test for read permission.  */

/* <unistd.h> */
# define SEEK_CUR	1	/* Seek from current position.  */

/* <unistd.h> */
# define SEEK_END	2	/* Seek from end of file.  */

/* <unistd.h> */
# define SEEK_SET	0	/* Seek from beginning of file.  */

/* <unistd.h> */
#define	STDERR_FILENO	2	/* Standard error output.  */

/* <unistd.h> */
/* Standard file descriptors.  */
#define	STDIN_FILENO	0	/* Standard input.  */

/* <unistd.h> */
#define	STDOUT_FILENO	1	/* Standard output.  */

/* <unistd.h> */
# define TEMP_FAILURE_RETRY(expression) ...

/* <unistd.h> */
#define	W_OK	2		/* access() - Test for write permission.  */

/* <unistd.h> */
#define	X_OK	1		/* access() - Test for execute permission.  */

/* <unistd.h> */
/* If defined, the implementation supports the
   C Language Bindings Option.  */
#define	_POSIX2_C_BIND	1

/* <unistd.h> */
/* If defined, the implementation supports the
   C Language Development Utilities Option.  */
#define	_POSIX2_C_DEV	1

/* <unistd.h> */
/* POSIX Standard approved as ISO/IEC 9945-2 as of December, 1993.  */
#define	_POSIX2_C_VERSION	199209L

/* <unistd.h> */
/* If defined, the implementation supports the
   creation of locales with the localedef utility.  */
#define _POSIX2_LOCALEDEF       1

/* <unistd.h> */
/* If defined, the implementation supports the
   Software Development Utilities Option.  */
#define	_POSIX2_SW_DEV	1

/* <unistd.h> */
/* The utilities on GNU systems also correspond to this version.  */
#define _POSIX2_VERSION	199209L

/* <unistd.h> */
/* POSIX Standard approved as ISO/IEC 9945-1 as of August, 1988 and
   extended by POSIX-1b (aka POSIX-4) and POSIX-1c (aka POSIX threads).  */
#define	_POSIX_VERSION	199506L

/* <unistd.h> */
#define	_UNISTD_H	1

/* <unistd.h> */
/* Encryption is present.  */
#define	_XOPEN_CRYPT	1

/* <unistd.h> */
/* The enhanced internationalization capabilities according to XPG4.2
   are present.  */
#define	_XOPEN_ENH_I18N	1

/* <unistd.h> */
/* The legacy interfaces are also available.  */
#define _XOPEN_LEGACY	1

/* <unistd.h> */
/* The X/Open Unix extensions are available.  */
#define _XOPEN_UNIX	1

/* <unistd.h> */
# define _XOPEN_VERSION	500

/* <unistd.h> */
# define _XOPEN_VERSION	4

/* <unistd.h> */
/* Commands and utilities from XPG4 are available.  */
#define _XOPEN_XCU_VERSION	4

/* <unistd.h> */
/* We are compatible with the old published standards as well.  */
#define _XOPEN_XPG2	1

/* <unistd.h> */
#define _XOPEN_XPG3	1

/* <unistd.h> */
#define _XOPEN_XPG4	1

/* <unistd.h> */
typedef __gid_t gid_t;

/* <unistd.h> */
typedef __intptr_t intptr_t;

/* <unistd.h> */
typedef __off64_t off64_t;

/* <unistd.h> */
typedef __off_t off_t;

/* <unistd.h> */
typedef __off64_t off_t;

/* <unistd.h> */
typedef __pid_t pid_t;

/* <unistd.h> */
typedef __socklen_t socklen_t;

/* <unistd.h> */
typedef __ssize_t ssize_t;

/* <unistd.h> */
typedef __uid_t uid_t;

/* <unistd.h> */
typedef __useconds_t useconds_t;

/* <unistd.h> */
/* whence is one of SEEK_SET, SEEK_CUR, or SEEK_END */
off_t lseek(int fd, off_t offset, int whence)
{
}

/* <unistd.h> */
/* whence is one of SEEK_SET, SEEK_CUR, or SEEK_END */
off64_t lseek64(int fd, off64_t offset, int whence)
{
}

/* <unistd.h> */
/* type is a bitwise-OR of R_OK, W_OK, X_OK, or simply F_OK to test existence */
int access(const char *name, int type)
{
}

/* <unistd.h> */
/* type is a bitwise-OR of R_OK, W_OK, X_OK, or simply F_OK to test existence */
/* This uses effective user id, not real user id like access().  (GNU only) */
int euidaccess(const char *name, int type)
{
}

/* <unistd.h> */
int close(int fd)
{
}

/* <unistd.h> */
ssize_t read(int fd, void *buf, size_t nbytes)
{
}

/* <unistd.h> */
ssize_t write(int fd, void *buf, size_t nbytes)
{
}

/* <unistd.h> */
ssize_t pread(int fd, void *buf, size_t nbytes, off_t offset)
{
}

/* <unistd.h> */
ssize_t pwrite(int fd, void *buf, size_t nbytes, off_t offset)
{
}

/* <unistd.h> */
ssize_t pread64(int fd, void *buf, size_t nbytes, off64_t offset)
{
}

/* <unistd.h> */
ssize_t pwrite64(int fd, void *buf, size_t nbytes, off64_t offset)
{
}

/* <unistd.h> */
/* read from filedes[0], write to filedes[1] */
int pipe(int filedes[2])
{
}

/* <unistd.h> */
unsigned int alarm(unsigned int seconds)
{
}

/* <unistd.h> */
unsigned int sleep(unsigned int seconds)
{
}

/* <unistd.h> microsecond alarm -- not portable */
__useconds_t ualarm(__useconds_t value, __useconds_t interval)
{
}

/* <unistd.h> microsecond sleep -- not portable */
int usleep(__useconds_t value)
{
}

/* <unistd.h> */
/* sleep until next signal */
int pause(void)
{
}

/* <unistd.h> */
int chown(const char *file, uid_t owner, gid_t group)
{
}

/* <unistd.h> */
int fchown(int fd, uid_t owner, gid_t group)
{
}

/* <unistd.h> */
/* change ownership of a symbolic link */
int lchown(const char *file, uid_t owner, gid_t group)
{
}

/* <unistd.h> */
int chdir(const char *path)
{
}

/* <unistd.h> */
int fchdir(int fd)
{
}

/* <unistd.h> */
/* GNU allows getcwd(NULL,0) to allocate a buffer via malloc() */
char *getcwd(char *buf, size_t size)
{
}

/* <unistd.h> */
/* OBSOLETE!  Use getcwd() instead */
char *getwd(char *buf)
{
}

/* <unistd.h> */
int dup(int fd)
{
}

/* <unistd.h> */
/* duplicate fd into newfd.  If newfd was already used, close it first */
int dup2(int fd, int newfd)
{
}

/* <unistd.h> */
int execve(const char *path, char const *argv[], char const *envp[])
{
}

/* <unistd.h> */
int execv(const char *path, char const *argv[])
{
}

/* <unistd.h> */
/* search for "file" in $PATH */
int execvp(const char *file, char const *argv[])
{
}

/* <unistd.h> */
/* arg0 is followed by other args, then NULL, and then envp[] */
int execle(const char *path, const char *arg0, ...)
{
}

/* <unistd.h> */
/* arg0 is followed by other args, then NULL */
int execl(const char *path, char const *arg0, ...)
{
}

/* <unistd.h> */
/* search for "file" in $PATH */
/* arg0 is followed by other args, then NULL */
int execlp(const char *file, char const *arg0, ...)
{
}

/* <unistd.h> */
int nice(int inc)
{
}

/* <unistd.h> */
void _exit(int status)
{
}

enum
{
    _PC_LINK_MAX,		/* maximum links to a file */
    _PC_MAX_CANON,		/* maximum length of formatted input line */
    _PC_MAX_INPUT,		/* maximum length of input line */
    _PC_NAME_MAX,		/* maximum file name length */
    _PC_PATH_MAX,		/* maximum length of relative path name */
    _PC_PIPE_BUF,		/* size of the pipe buffer */
    _PC_CHOWN_RESTRICTED,	/* predicts permissions failure for chown() */
    _PC_NO_TRUNC,		/* non-zero if long names aren't an error */
    _PC_VDISABLE,		/* non-zero if raw-mode tty is possible */
    _PC_SYNC_IO,
    _PC_ASYNC_IO,
    _PC_PRIO_IO,
    _PC_SOCK_MAXBUF,
    _PC_FILESIZEBITS,
    _PC_REC_INCR_XFER_SIZE,
    _PC_REC_MAX_XFER_SIZE,
    _PC_REC_MIN_XFER_SIZE,
    _PC_REC_XFER_ALIGN,
    _PC_ALLOC_SIZE_MIN,
    _PC_SYMLINK_MAX
};

/* <unistd.h> */
/* pcname is one of _PC_LINK_MAX, _PC_MAX_CANNON, etc. */
long int pathconf(char *path, int pcname)
{
}

/* <unistd.h> */
/* pcname is one of _PC_LINK_MAX, _PC_MAX_CANNON, etc. */
long int fpathconf(int fd, int pcname)
{
}

enum
{
    _SC_ARG_MAX,
    _SC_CHILD_MAX,
    _SC_CLK_TCK,
    _SC_NGROUPS_MAX,
    _SC_OPEN_MAX,
    _SC_STREAM_MAX,
    _SC_TZNAME_MAX,
    _SC_JOB_CONTROL,
    _SC_SAVED_IDS,
    _SC_REALTIME_SIGNALS,
    _SC_PRIORITY_SCHEDULING,
    _SC_TIMERS,
    _SC_ASYNCHRONOUS_IO,
    _SC_PRIORITIZED_IO,
    _SC_SYNCHRONIZED_IO,
    _SC_FSYNC,
    _SC_MAPPED_FILES,
    _SC_MEMLOCK,
    _SC_MEMLOCK_RANGE,
    _SC_MEMORY_PROTECTION,
    _SC_MESSAGE_PASSING,
    _SC_SEMAPHORES,
    _SC_SHARED_MEMORY_OBJECTS,
    _SC_AIO_LISTIO_MAX,
    _SC_AIO_MAX,
    _SC_AIO_PRIO_DELTA_MAX,
    _SC_DELAYTIMER_MAX,
    _SC_MQ_OPEN_MAX,
    _SC_MQ_PRIO_MAX,
    _SC_VERSION,
    _SC_PAGESIZE,
    _SC_RTSIG_MAX,
    _SC_SEM_NSEMS_MAX,
    _SC_SEM_VALUE_MAX,
    _SC_SIGQUEUE_MAX,
    _SC_TIMER_MAX,

    /* Values for the argument to `sysconf'
       corresponding to _POSIX2_* symbols.  */
    _SC_BC_BASE_MAX,
    _SC_BC_DIM_MAX,
    _SC_BC_SCALE_MAX,
    _SC_BC_STRING_MAX,
    _SC_COLL_WEIGHTS_MAX,
    _SC_EQUIV_CLASS_MAX,
    _SC_EXPR_NEST_MAX,
    _SC_LINE_MAX,
    _SC_RE_DUP_MAX,
    _SC_CHARCLASS_NAME_MAX,

    _SC_2_VERSION,
    _SC_2_C_BIND,
    _SC_2_C_DEV,
    _SC_2_FORT_DEV,
    _SC_2_FORT_RUN,
    _SC_2_SW_DEV,
    _SC_2_LOCALEDEF,

    _SC_PII,
    _SC_PII_XTI,
    _SC_PII_SOCKET,
    _SC_PII_INTERNET,
    _SC_PII_OSI,
    _SC_POLL,
    _SC_SELECT,
    _SC_UIO_MAXIOV,
    _SC_IOV_MAX = _SC_UIO_MAXIOV,
    _SC_PII_INTERNET_STREAM,
    _SC_PII_INTERNET_DGRAM,
    _SC_PII_OSI_COTS,
    _SC_PII_OSI_CLTS,
    _SC_PII_OSI_M,
    _SC_T_IOV_MAX,

    /* Values according to POSIX 1003.1c (POSIX threads).  */
    _SC_THREADS,
    _SC_THREAD_SAFE_FUNCTIONS,
    _SC_GETGR_R_SIZE_MAX,
    _SC_GETPW_R_SIZE_MAX,
    _SC_LOGIN_NAME_MAX,
    _SC_TTY_NAME_MAX,
    _SC_THREAD_DESTRUCTOR_ITERATIONS,
    _SC_THREAD_KEYS_MAX,
    _SC_THREAD_STACK_MIN,
    _SC_THREAD_THREADS_MAX,
    _SC_THREAD_ATTR_STACKADDR,
    _SC_THREAD_ATTR_STACKSIZE,
    _SC_THREAD_PRIORITY_SCHEDULING,
    _SC_THREAD_PRIO_INHERIT,
    _SC_THREAD_PRIO_PROTECT,
    _SC_THREAD_PROCESS_SHARED,

    _SC_NPROCESSORS_CONF,
    _SC_NPROCESSORS_ONLN,
    _SC_PHYS_PAGES,
    _SC_AVPHYS_PAGES,
    _SC_ATEXIT_MAX,
    _SC_PASS_MAX,

    _SC_XOPEN_VERSION,
    _SC_XOPEN_XCU_VERSION,
    _SC_XOPEN_UNIX,
    _SC_XOPEN_CRYPT,
    _SC_XOPEN_ENH_I18N,
    _SC_XOPEN_SHM,

    _SC_2_CHAR_TERM,
    _SC_2_C_VERSION,
    _SC_2_UPE,

    _SC_XOPEN_XPG2,
    _SC_XOPEN_XPG3,
    _SC_XOPEN_XPG4,

    _SC_CHAR_BIT,
    _SC_CHAR_MAX,
    _SC_CHAR_MIN,
    _SC_INT_MAX,
    _SC_INT_MIN,
    _SC_LONG_BIT,
    _SC_WORD_BIT,
    _SC_MB_LEN_MAX,
    _SC_NZERO,
    _SC_SSIZE_MAX,
    _SC_SCHAR_MAX,
    _SC_SCHAR_MIN,
    _SC_SHRT_MAX,
    _SC_SHRT_MIN,
    _SC_UCHAR_MAX,
    _SC_UINT_MAX,
    _SC_ULONG_MAX,
    _SC_USHRT_MAX,

    _SC_NL_ARGMAX,
    _SC_NL_LANGMAX,
    _SC_NL_MSGMAX,
    _SC_NL_NMAX,
    _SC_NL_SETMAX,
    _SC_NL_TEXTMAX,

    _SC_XBS5_ILP32_OFF32,
    _SC_XBS5_ILP32_OFFBIG,
    _SC_XBS5_LP64_OFF64,
    _SC_XBS5_LPBIG_OFFBIG,

    _SC_XOPEN_LEGACY,
    _SC_XOPEN_REALTIME,
    _SC_XOPEN_REALTIME_THREADS,

    _SC_ADVISORY_INFO,
    _SC_BARRIERS,
    _SC_BASE,
    _SC_C_LANG_SUPPORT,
    _SC_C_LANG_SUPPORT_R,
    _SC_CLOCK_SELECTION,
    _SC_CPUTIME,
    _SC_THREAD_CPUTIME,
    _SC_DEVICE_IO,
    _SC_DEVICE_SPECIFIC,
    _SC_DEVICE_SPECIFIC_R,
    _SC_FD_MGMT,
    _SC_FIFO,
    _SC_PIPE,
    _SC_FILE_ATTRIBUTES,
    _SC_FILE_LOCKING,
    _SC_FILE_SYSTEM,
    _SC_MONOTONIC_CLOCK,
    _SC_MULTI_PROCESS,
    _SC_SINGLE_PROCESS,
    _SC_NETWORKING,
    _SC_READER_WRITER_LOCKS,
    _SC_SPIN_LOCKS,
    _SC_REGEXP,
    _SC_REGEX_VERSION,
    _SC_SHELL,
    _SC_SIGNALS,
    _SC_SPAWN,
    _SC_SPORADIC_SERVER,
    _SC_THREAD_SPORADIC_SERVER,
    _SC_SYSTEM_DATABASE,
    _SC_SYSTEM_DATABASE_R,
    _SC_TIMEOUTS,
    _SC_TYPED_MEMORY_OBJECTS,
    _SC_USER_GROUPS,
    _SC_USER_GROUPS_R,
    _SC_2_PBS,
    _SC_2_PBS_ACCOUNTING,
    _SC_2_PBS_LOCATE,
    _SC_2_PBS_MESSAGE,
    _SC_2_PBS_TRACK,
    _SC_SYMLOOP_MAX,
    _SC_STREAMS,
    _SC_2_PBS_CHECKPOINT,

    _SC_V6_ILP32_OFF32,
    _SC_V6_ILP32_OFFBIG,
    _SC_V6_LP64_OFF64,
    _SC_V6_LPBIG_OFFBIG,

    _SC_HOST_NAME_MAX,
    _SC_TRACE,
    _SC_TRACE_EVENT_FILTER,
    _SC_TRACE_INHERIT,
    _SC_TRACE_LOG
};

/* <unistd.h> */
/* name is one of _SC_ARG_MAX, SC_TRACE_LOG, etc. */
long int sysconf(int name);

/* <unistd.h> */
/* Values for the NAME argument to `confstr'.  */
enum
{
    _CS_PATH,			/* The default search path.  */
    _CS_V6_WIDTH_RESTRICTED_ENVS,
    _CS_GNU_LIBC_VERSION,
    _CS_GNU_LIBPTHREAD_VERSION,
    _CS_LFS_CFLAGS = 1000,
    _CS_LFS_LDFLAGS,
    _CS_LFS_LIBS,
    _CS_LFS_LINTFLAGS,
    _CS_LFS64_CFLAGS,
    _CS_LFS64_LDFLAGS,
    _CS_LFS64_LIBS,
    _CS_LFS64_LINTFLAGS,
    _CS_XBS5_ILP32_OFF32_CFLAGS = 1100,
    _CS_XBS5_ILP32_OFF32_LDFLAGS,
    _CS_XBS5_ILP32_OFF32_LIBS,
    _CS_XBS5_ILP32_OFF32_LINTFLAGS,
    _CS_XBS5_ILP32_OFFBIG_CFLAGS,
    _CS_XBS5_ILP32_OFFBIG_LDFLAGS,
    _CS_XBS5_ILP32_OFFBIG_LIBS,
    _CS_XBS5_ILP32_OFFBIG_LINTFLAGS,
    _CS_XBS5_LP64_OFF64_CFLAGS,
    _CS_XBS5_LP64_OFF64_LDFLAGS,
    _CS_XBS5_LP64_OFF64_LIBS,
    _CS_XBS5_LP64_OFF64_LINTFLAGS,
    _CS_XBS5_LPBIG_OFFBIG_CFLAGS,
    _CS_XBS5_LPBIG_OFFBIG_LDFLAGS,
    _CS_XBS5_LPBIG_OFFBIG_LIBS,
    _CS_XBS5_LPBIG_OFFBIG_LINTFLAGS,
    _CS_POSIX_V6_ILP32_OFF32_CFLAGS,
    _CS_POSIX_V6_ILP32_OFF32_LDFLAGS,
    _CS_POSIX_V6_ILP32_OFF32_LIBS,
    _CS_POSIX_V6_ILP32_OFF32_LINTFLAGS,
    _CS_POSIX_V6_ILP32_OFFBIG_CFLAGS,
    _CS_POSIX_V6_ILP32_OFFBIG_LDFLAGS,
    _CS_POSIX_V6_ILP32_OFFBIG_LIBS,
    _CS_POSIX_V6_ILP32_OFFBIG_LINTFLAGS,
    _CS_POSIX_V6_LP64_OFF64_CFLAGS,
    _CS_POSIX_V6_LP64_OFF64_LDFLAGS,
    _CS_POSIX_V6_LP64_OFF64_LIBS,
    _CS_POSIX_V6_LP64_OFF64_LINTFLAGS,
    _CS_POSIX_V6_LPBIG_OFFBIG_CFLAGS,
    _CS_POSIX_V6_LPBIG_OFFBIG_LDFLAGS,
    _CS_POSIX_V6_LPBIG_OFFBIG_LIBS,
    _CS_POSIX_V6_LPBIG_OFFBIG_LINTFLAGS
};

/* <unistd.h> */
/* name is one of _CS_PATH, etc. */
size_t confstr(int name, char *buf, size_t len)
{
}

/* <unistd.h> */
pid_t getpid(void)
{
}

/* <unistd.h> */
pid_t getppid(void)
{
}

/* <unistd.h> */
/* Old BSD version takes a pid_t argument */
pid_t getpgrp(void)
{
}

/* <unistd.h> */
pid_t getpgid(void)
{
}

/* <unistd.h> */
int setpgid(pid_t pid, pid_t pgif)
{
}

/* <unistd.h> */
int setpgrp(void)
{
}

/* <unistd.h> */
pid_t setsid(void)
{
}

/* <unistd.h> */
pid_t getsid(pid_t pid)
{
}

/* <unistd.h> */
uid_t getuid(void)
{
}

/* <unistd.h> */
uid_t geteuid(void)
{
}

/* <unistd.h> */
gid_t getgid(void)
{
}

/* <unistd.h> */
gid_t getegid(void)
{
}

/* <unistd.h> */
/* if size=0 then return the number of groups; else fill list and return number
 * filled.
 */
int getgroups(int size, gid_t list[])
{
}

/* <unistd.h> */
int setuid(uid_t uid)
{
}

/* <unistd.h> */
int seteuid(uid_t uid)
{
}

/* <unistd.h> */
int setreuid(uid_t ruid, uid_t euid)
{
}

/* <unistd.h> */
int setgid(gid_t gid)
{
}

/* <unistd.h> */
int setegid(gid_t gid)
{
}

/* <unistd.h> */
int setregid(gid_t rgid, gid_t egid)
{
}

/* <unistd.h> */
/* returns -1 if error, 0 to child, or child's pid to the parent */
pid_t fork(void)
{
}

/* <unistd.h> */
char *ttyname(int fd)
{
}

/* <unistd.h> */
/* thread-safe, reentrant version of ttyname() */
char *ttyname_r(int fd, char *buf, size_t buflen)
{
}

/* <unistd.h> */
int isatty(int fd)
{
}

/* <unistd.h> */
int link(const char *from, const chr *to)
{
}

/* <unistd.h> */
int symlink(const char *from, const chr *to)
{
}

/* <unistd.h> */
int readlink(const char *path, char *buf, size_t len)
{
}

/* <unistd.h> */
int unlink(const char *name)
{
}

/* <unistd.h> */
int rmdir(const char *dirname)
{
}

/* <unistd.h> */
pid_t tcgetpgrp(int fd)
{
}

/* <unistd.h> */
pid_t tcsetpgrp(int fd, pid_t pgrpid)
{
}

/* <unistd.h> */
char *getlogin(void)
{
}

/* <unistd.h> */
/* thread-safe, reentrant version of getlogin() */
int getlogin_r(char *name, size_t namelen)
{
}

/* <unistd.h> */
int gethostname(char *name, size_t namelen)
{
}

/* <unistd.h> */
int sethostname(char *name, size_t namelen)
{
}

/* <unistd.h> */
int sethostid(long id)
{
}

/* <unistd.h> */
int getdomainname(char *name, size_t namelen)
{
}

/* <unistd.h> */
int setdomainname(const char *name, size_t namelen)
{
}

/* <unistd.h> */
int vhangup(void)
{
}

/* <unistd.h> */
int revoke(const char *filename)
{
}

/* <unistd.h> */
int profil(unsigned short *sample_buffer, size_t size, size_t offset, unsigned scale)
{
}

/* <unistd.h> */
int acct(const char *name)
{
}

/* <unistd.h> */
char *getusershell(void)
{
}

/* <unistd.h> */
void endusershell(void)
{
}

/* <unistd.h> */
void setusershell(void)
{
}

/* <unistd.h> */
int daemon(int nochdir, int noclose)
{
}

/* <unistd.h> */
int chroot(const char *path)
{
}

/* <unistd.h> */
char *getpass(const char *prompt)
{
}

/* <unistd.h> */
void sync(void)
{
}

/* <unistd.h> */
int getpagesize(void)
{
}

/* <unistd.h> */
int truncate(const char *file, off_t length)
{
}

/* <unistd.h> */
int truncate64(const char *file, off64_t length)
{
}

/* <unistd.h> */
int ftruncate(int fd, off_t length)
{
}

/* <unistd.h> */
int ftruncate64(int fd, off64_t length)
{
}

/* <unistd.h> */
int getdtablesize(void)
{
}

/* <unistd.h> */
int brk(void *addr)
{
}

/* <unistd.h> */
int sbrk(int *delta)
{
}

/* <unistd.h> */
#define TEMP_FAILURE_RETRY(expr)	/* retry expr until not EINTR */

/* <unistd.h> */
int fdatasync(int fd)
{
}

/* <unistd.h> */
char *crypt(const char *key, const char *salt)
{
}

/* <unistd.h> */
void encrypt(char *block, int edflag)
{
}

/* <unistd.h> */
void swab(const void *from, void *to, ssize_t nbytes)
{
}

/* <unistd.h> */
char *ctermid(char *buf)
{
}

/* <unistd.h> */
int pthread_atfork(void (*prepare)(void), void (*parent)(void), void (*child)(void))
{
}


/* <unistd.h> or <getopt.h> ==================================== */

/* <getopt.h> or <unistd.h> */
char *optarg;

/* <getopt.h> or <unistd.h> */
int optind;

/* <getopt.h> or <unistd.h> */
int opterr;

/* <getopt.h> or <unistd.h> */
int optopt;

/* <getopt.h> or <unistd.h> */
struct option
{
	const char *name;	/* name of long option */
	int	   has_arg;	/* 0=no arg, 1=mandatory, 2=optional */
	int	   *flag;	/* where to store "val" if --name is given */
	int	   val;		/* value to store at *flag */
};

/* <getopt.h> or <unistd.h> */
int getopt(int argc, char * const argv[], const char *letters)
{
}

/* <getopt.h> */
int getopt_long(int argc, char * const argv[], const char *letters, const struct option *longopts, int *longind)
{
}

/* <getopt.h> */
/* like getopt_long(), but allow -name in addition to --name */
int getopt_long_only(int argc, char * const argv[], const char *letters, const struct option *longopts, int *longind)
{
}

/* <sys/acct.h> ==================================== */

/* <sys/acct.h> */
#define ACCT_COMM 16

/* <sys/acct.h> */
#define AHZ     100

/* <sys/acct.h> */
#define _SYS_ACCT_H	1

/* <sys/acct.h> */
#define	__need_time_t

/* <sys/acct.h> */
struct acct
{
    char ac_flag;			/* Accounting flags.  */
    u_int16_t ac_uid;			/* Accounting user ID.  */
    u_int16_t ac_gid;			/* Accounting group ID.  */
    u_int16_t ac_tty;			/* Controlling tty.  */
    u_int32_t ac_btime;			/* Beginning time.  */
    comp_t ac_utime;			/* Accounting user time.  */
    comp_t ac_stime;			/* Accounting system time.  */
    comp_t ac_etime;			/* Accounting elapsed time.  */
    comp_t ac_mem;			/* Accounting average memory usage.  */
    comp_t ac_io;			/* Accounting chars transferred.  */
    comp_t ac_rw;			/* Accounting blocks read or written. */
    comp_t ac_minflt;			/* Accounting minor pagefaults.  */
    comp_t ac_majflt;			/* Accounting major pagefaults.  */
    comp_t ac_swaps;			/* Accounting number of swaps.  */
    u_int32_t ac_exitcode;		/* Accounting process exitcode.  */
    char ac_comm[ACCT_COMM+1];		/* Accounting command name.  */
    char ac_pad[10];			/* Accounting padding bytes.  */
};

/* <sys/acct.h> */
typedef u_int16_t comp_t;

/* <sys/asoundlib.h> ==================================== */

/* <sys/asoundlib.h> */
#define SND_LIB_MAJOR		0

/* <sys/asoundlib.h> */
#define SND_LIB_MINOR		5

/* <sys/asoundlib.h> */
#define SND_LIB_SUBMINOR	10

/* <sys/asoundlib.h> */
#define SND_LIB_VERSION		((SND_LIB_MAJOR<<16)|(SND_LIB_MINOR<<8)|SND_LIB_SUBMINOR)

/* <sys/asoundlib.h> */
#define SND_LIB_VERSION_STR	"0.5.10b"

/* <sys/asoundlib.h> */
#define SND_ERROR_BEGIN				500000

/* <sys/asoundlib.h> */
#define SND_ERROR_INCOMPATIBLE_VERSION		(SND_ERROR_BEGIN+0)

/* <sys/asoundlib.h> */
const char *snd_strerror( int errnum )
{
}

/* <sys/asoundlib.h> */
typedef struct snd_ctl_callbacks {
	void *private_data;	/* should be used by an application */
	void (*rebuild) (void *private_data)
	void (*xswitch) (void *private_data, int cmd, int iface, int device, int channel, snd_switch_list_item_t *item)
	void *reserved[29];	/* reserved for the future use - must be NULL!!! */
} snd_ctl_callbacks_t;

/* <sys/asoundlib.h> */
typedef struct snd_ctl snd_ctl_t;

/* <sys/asoundlib.h> */
int snd_card_load(int card)
{
}

/* <sys/asoundlib.h> */
int snd_cards(void)
{
}

/* <sys/asoundlib.h> */
unsigned int snd_cards_mask(void)
{
}

/* <sys/asoundlib.h> */
int snd_card_name(const char *name)
{
}

/* <sys/asoundlib.h> */
int snd_card_get_name(int card, char **name)
{
}

/* <sys/asoundlib.h> */
int snd_card_get_longname(int card, char **name)
{
}

/* <sys/asoundlib.h> */
int snd_defaults_card(void)
{
}

/* <sys/asoundlib.h> */
int snd_defaults_mixer_card(void)
{
}

/* <sys/asoundlib.h> */
int snd_defaults_mixer_device(void)
{
}

/* <sys/asoundlib.h> */
int snd_defaults_pcm_card(void)
{
}

/* <sys/asoundlib.h> */
int snd_defaults_pcm_device(void)
{
}

/* <sys/asoundlib.h> */
int snd_defaults_rawmidi_card(void)
{
}

/* <sys/asoundlib.h> */
int snd_defaults_rawmidi_device(void)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_open(snd_ctl_t **handle, int card)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_close(snd_ctl_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_file_descriptor(snd_ctl_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_hw_info(snd_ctl_t *handle, struct snd_ctl_hw_info *info)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_switch_list(snd_ctl_t *handle, snd_switch_list_t * list)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_switch_read(snd_ctl_t *handle, snd_switch_t * sw)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_switch_write(snd_ctl_t *handle, snd_switch_t * sw)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_hwdep_info(snd_ctl_t *handle, int dev, snd_hwdep_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_pcm_info(snd_ctl_t *handle, int dev, snd_pcm_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_pcm_channel_info(snd_ctl_t *handle, int dev, int channel, int subdev, snd_pcm_channel_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_pcm_channel_prefer_subdevice(snd_ctl_t *handle, int dev, int channel, int subdev)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_mixer_info(snd_ctl_t *handle, int dev, snd_mixer_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_rawmidi_info(snd_ctl_t *handle, int dev, snd_rawmidi_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_ctl_read(snd_ctl_t *handle, snd_ctl_callbacks_t * callbacks)
{
}

/* <sys/asoundlib.h> */
typedef struct snd_mixer_callbacks {
	void *private_data;	/* should be used with an application */
	void (*rebuild) (void *private_data)
	void (*element) (void *private_data, int cmd, snd_mixer_eid_t *eid)
	void (*group) (void *private_data, int cmd, snd_mixer_gid_t *gid)
	void *reserved[28];	/* reserved for the future use - must be NULL!!! */
} snd_mixer_callbacks_t;

/* <sys/asoundlib.h> */
typedef struct {
	char *name;
	int weight;
} snd_mixer_weight_entry_t;

/* <sys/asoundlib.h> */
typedef struct snd_mixer snd_mixer_t;

/* <sys/asoundlib.h> */
int snd_mixer_open(snd_mixer_t **handle, int card, int device)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_close(snd_mixer_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_file_descriptor(snd_mixer_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_info(snd_mixer_t *handle, snd_mixer_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_elements(snd_mixer_t *handle, snd_mixer_elements_t * elements)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_routes(snd_mixer_t *handle, snd_mixer_routes_t * routes)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_groups(snd_mixer_t *handle, snd_mixer_groups_t * groups)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_group_read(snd_mixer_t *handle, snd_mixer_group_t * group)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_group_write(snd_mixer_t *handle, snd_mixer_group_t * group)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_element_info(snd_mixer_t *handle, snd_mixer_element_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_element_read(snd_mixer_t *handle, snd_mixer_element_t * element)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_element_write(snd_mixer_t *handle, snd_mixer_element_t * element)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_get_filter(snd_mixer_t *handle, snd_mixer_filter_t * filter)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_put_filter(snd_mixer_t *handle, snd_mixer_filter_t * filter)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_read(snd_mixer_t *handle, snd_mixer_callbacks_t * callbacks)
{
}

/* <sys/asoundlib.h> */
void snd_mixer_set_bit(unsigned int *bitmap, int bit, int val)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_get_bit(unsigned int *bitmap, int bit)
{
}

/* <sys/asoundlib.h> */
const char *snd_mixer_channel_name(int channel)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_element_has_info(snd_mixer_eid_t *eid)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_element_info_build(snd_mixer_t *handle, snd_mixer_element_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_element_info_free(snd_mixer_element_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_element_has_control(snd_mixer_eid_t *eid)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_element_build(snd_mixer_t *handle, snd_mixer_element_t * element)
{
}

/* <sys/asoundlib.h> */
int snd_mixer_element_free(snd_mixer_element_t * element)
{
}

/* <sys/asoundlib.h> */
void snd_mixer_sort_eid_name_index(snd_mixer_eid_t *list, int count)
{
}

/* <sys/asoundlib.h> */
void snd_mixer_sort_eid_table(snd_mixer_eid_t *list, int count, snd_mixer_weight_entry_t *table)
{
}

/* <sys/asoundlib.h> */
void snd_mixer_sort_gid_name_index(snd_mixer_gid_t *list, int count)
{
}

/* <sys/asoundlib.h> */
void snd_mixer_sort_gid_table(snd_mixer_gid_t *list, int count, snd_mixer_weight_entry_t *table)
{
}

/* <sys/asoundlib.h> */
extern snd_mixer_weight_entry_t *snd_mixer_default_weights;

/* <sys/asoundlib.h> */
#define SND_PCM_OPEN_PLAYBACK		0x0001

/* <sys/asoundlib.h> */
#define SND_PCM_OPEN_CAPTURE		0x0002

/* <sys/asoundlib.h> */
#define SND_PCM_OPEN_DUPLEX		0x0003

/* <sys/asoundlib.h> */
#define SND_PCM_OPEN_NONBLOCK		0x1000

/* <sys/asoundlib.h> */
typedef struct snd_pcm snd_pcm_t;

/* <sys/asoundlib.h> */
typedef struct snd_pcm_loopback snd_pcm_loopback_t;

/* <sys/asoundlib.h> */
int snd_pcm_open(snd_pcm_t **handle, int card, int device, int mode)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_open_subdevice(snd_pcm_t **handle, int card, int device, int subdevice, int mode)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_close(snd_pcm_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_file_descriptor(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_nonblock_mode(snd_pcm_t *handle, int nonblock)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_info(snd_pcm_t *handle, snd_pcm_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_channel_info(snd_pcm_t *handle, snd_pcm_channel_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_channel_params(snd_pcm_t *handle, snd_pcm_channel_params_t * params)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_channel_setup(snd_pcm_t *handle, snd_pcm_channel_setup_t * setup)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_channel_status(snd_pcm_t *handle, snd_pcm_channel_status_t * status)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_playback_prepare(snd_pcm_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_capture_prepare(snd_pcm_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_channel_prepare(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_playback_go(snd_pcm_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_capture_go(snd_pcm_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_channel_go(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_sync_go(snd_pcm_t *handle, snd_pcm_sync_t *sync)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_playback_drain(snd_pcm_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_playback_flush(snd_pcm_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_capture_flush(snd_pcm_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_channel_flush(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_playback_pause(snd_pcm_t *handle, int enable)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_pcm_transfer_size(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_pcm_write(snd_pcm_t *handle, const void *buffer, size_t size)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_pcm_read(snd_pcm_t *handle, void *buffer, size_t size)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_pcm_writev(snd_pcm_t *pcm, const struct iovec *vector, int count)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_pcm_readv(snd_pcm_t *pcm, const struct iovec *vector, int count)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_mmap(snd_pcm_t *handle, int channel, snd_pcm_mmap_control_t **control, void **buffer)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_munmap(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_format_signed(int format)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_format_unsigned(int format)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_format_linear(int format)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_format_little_endian(int format)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_format_big_endian(int format)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_format_width(int format);			/* in bits */

/* <sys/asoundlib.h> */
int snd_pcm_format_physical_width(int format);		/* in bits */

/* <sys/asoundlib.h> */
int snd_pcm_build_linear_format(int width, int unsignd, int big_endian)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_pcm_format_size(int format, size_t samples)
{
}

/* <sys/asoundlib.h> */
unsigned char snd_pcm_format_silence(int format)
{
}

/* <sys/asoundlib.h> */
const char *snd_pcm_get_format_name(int format)
{
}

/* <sys/asoundlib.h> */
typedef struct snd_stru_pcm_plugin snd_pcm_plugin_t;

/* <sys/asoundlib.h> */
typedef enum {
	INIT = 0,
	PREPARE = 1,
	DRAIN = 2,
	FLUSH = 3
} snd_pcm_plugin_action_t;

/* <sys/asoundlib.h> */
#define snd_pcm_plugin_extra_data(plugin) (((char *)plugin) + sizeof(*plugin))

/* <sys/asoundlib.h> */
struct snd_stru_pcm_plugin {
	char *name;				   /* plug-in name */
	int (*transfer_src_ptr)(snd_pcm_plugin_t *plugin, char **src_ptr, size_t *src_size)
	ssize_t (*transfer)(snd_pcm_plugin_t *plugin, char *src_ptr, size_t src_size, char *dst_ptr, size_t dst_size)
	ssize_t (*src_size)(snd_pcm_plugin_t *plugin, size_t dst_size)
	ssize_t (*dst_size)(snd_pcm_plugin_t *plugin, size_t src_size)
	int (*action)(snd_pcm_plugin_t *plugin, snd_pcm_plugin_action_t action)
	snd_pcm_plugin_t *prev;
	snd_pcm_plugin_t *next;
	void *private_data;
	void (*private_free)(snd_pcm_plugin_t *plugin, void *private_data)
};

/* <sys/asoundlib.h> */
snd_pcm_plugin_t *snd_pcm_plugin_build(const char *name, int extra)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_free(snd_pcm_plugin_t *plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_clear(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_insert(snd_pcm_t *handle, int channel, snd_pcm_plugin_t *plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_append(snd_pcm_t *handle, int channel, snd_pcm_plugin_t *plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_remove_to(snd_pcm_t *handle, int channel, snd_pcm_plugin_t *plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_remove_first(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
snd_pcm_plugin_t *snd_pcm_plugin_first(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
snd_pcm_plugin_t *snd_pcm_plugin_last(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_pcm_plugin_transfer_size(snd_pcm_t *handle, int channel, size_t drv_size)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_pcm_plugin_hardware_size(snd_pcm_t *handle, int channel, size_t trf_size)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_info(snd_pcm_t *handle, snd_pcm_channel_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_params(snd_pcm_t *handle, snd_pcm_channel_params_t *params)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_setup(snd_pcm_t *handle, snd_pcm_channel_setup_t *setup)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_status(snd_pcm_t *handle, snd_pcm_channel_status_t *status)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_prepare(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_playback_drain(snd_pcm_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_flush(snd_pcm_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_pointer(snd_pcm_t *pcm, int channel, void **ptr, size_t *size)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_pcm_plugin_write(snd_pcm_t *handle, const void *buffer, size_t size)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_pcm_plugin_read(snd_pcm_t *handle, void *bufer, size_t size)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_build_stream(snd_pcm_t *handle, int channel, snd_pcm_plugin_t **r_plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_build_block(snd_pcm_t *handle, int channel, snd_pcm_plugin_t **r_plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_build_mmap(snd_pcm_t *handle, int channel, snd_pcm_plugin_t **r_plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_build_interleave(snd_pcm_format_t *src_format, snd_pcm_format_t *dst_format, snd_pcm_plugin_t **r_plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_build_linear(snd_pcm_format_t *src_format, snd_pcm_format_t *dst_format, snd_pcm_plugin_t **r_plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_build_mulaw(snd_pcm_format_t *src_format, snd_pcm_format_t *dst_format, snd_pcm_plugin_t **r_plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_build_alaw(snd_pcm_format_t *src_format, snd_pcm_format_t *dst_format, snd_pcm_plugin_t **r_plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_build_adpcm(snd_pcm_format_t *src_format, snd_pcm_format_t *dst_format, snd_pcm_plugin_t **r_plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_build_rate(snd_pcm_format_t *src_format, snd_pcm_format_t *dst_format, snd_pcm_plugin_t **r_plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_build_voices(snd_pcm_format_t *src_format, snd_pcm_format_t *dst_format, snd_pcm_plugin_t **r_plugin)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_plugin_build_volbal(snd_pcm_format_t *src_format, snd_pcm_format_t *dst_format, int *ttable, snd_pcm_plugin_t **r_plugin)
{
}

/* <sys/asoundlib.h> */
#define SND_PCM_LB_OPEN_PLAYBACK	0

/* <sys/asoundlib.h> */
#define SND_PCM_LB_OPEN_CAPTURE		1

/* <sys/asoundlib.h> */
typedef struct snd_pcm_loopback_callbacks {
	void *private_data;		/* should be used with an application */
	size_t max_buffer_size;		/* zero = default (64kB) */
	void (*data) (void *private_data, char *buffer, size_t count)
	void (*position_change) (void *private_data, unsigned int pos)
	void (*format_change) (void *private_data, snd_pcm_format_t *format)
	void (*silence) (void *private_data, size_t count)
	void *reserved[31];		/* reserved for the future use - must be NULL!!! */
} snd_pcm_loopback_callbacks_t;

/* <sys/asoundlib.h> */
int snd_pcm_loopback_open(snd_pcm_loopback_t **handle, int card, int device, int subdev, int mode)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_loopback_close(snd_pcm_loopback_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_loopback_file_descriptor(snd_pcm_loopback_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_loopback_block_mode(snd_pcm_loopback_t *handle, int enable)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_loopback_stream_mode(snd_pcm_loopback_t *handle, int mode)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_loopback_format(snd_pcm_loopback_t *handle, snd_pcm_format_t * format)
{
}

/* <sys/asoundlib.h> */
int snd_pcm_loopback_status(snd_pcm_loopback_t *handle, snd_pcm_loopback_status_t * status)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_pcm_loopback_read(snd_pcm_loopback_t *handle, snd_pcm_loopback_callbacks_t * callbacks)
{
}

/* <sys/asoundlib.h> */
#define SND_RAWMIDI_OPEN_OUTPUT		(O_WRONLY)

/* <sys/asoundlib.h> */
#define SND_RAWMIDI_OPEN_OUTPUT_APPEND	(O_WRONLY|O_APPEND|O_NONBLOCK)

/* <sys/asoundlib.h> */
#define SND_RAWMIDI_OPEN_INPUT		(O_RDONLY)

/* <sys/asoundlib.h> */
#define SND_RAWMIDI_OPEN_DUPLEX		(O_RDWR)

/* <sys/asoundlib.h> */
#define SND_RAWMIDI_OPEN_DUPLEX_APPEND	(O_RDWR|O_APPEND|O_NONBLOCK)

/* <sys/asoundlib.h> */
#define SND_RAWMIDI_OPEN_NONBLOCK	(O_NONBLOCK)

/* <sys/asoundlib.h> */
typedef struct snd_rawmidi snd_rawmidi_t;

/* <sys/asoundlib.h> */
int snd_rawmidi_open(snd_rawmidi_t **handle, int card, int device, int mode)
{
}

/* <sys/asoundlib.h> */
int snd_rawmidi_close(snd_rawmidi_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_rawmidi_file_descriptor(snd_rawmidi_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_rawmidi_block_mode(snd_rawmidi_t *handle, int enable)
{
}

/* <sys/asoundlib.h> */
int snd_rawmidi_info(snd_rawmidi_t *handle, snd_rawmidi_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_rawmidi_channel_params(snd_rawmidi_t *handle, snd_rawmidi_params_t * params)
{
}

/* <sys/asoundlib.h> */
int snd_rawmidi_channel_status(snd_rawmidi_t *handle, snd_rawmidi_status_t * status)
{
}

/* <sys/asoundlib.h> */
int snd_rawmidi_output_drain(snd_rawmidi_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_rawmidi_output_flush(snd_rawmidi_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_rawmidi_input_flush(snd_rawmidi_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_rawmidi_channel_flush(snd_rawmidi_t *handle, int channel)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_rawmidi_write(snd_rawmidi_t *handle, const void *buffer, size_t size)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_rawmidi_read(snd_rawmidi_t *handle, void *buffer, size_t size)
{
}

/* <sys/asoundlib.h> */
typedef struct snd_timer snd_timer_t;

/* <sys/asoundlib.h> */
int snd_timer_open(snd_timer_t **handle)
{
}

/* <sys/asoundlib.h> */
int snd_timer_close(snd_timer_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_timer_file_descriptor(snd_timer_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_timer_general_info(snd_timer_t *handle, snd_timer_general_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_timer_select(snd_timer_t *handle, snd_timer_select_t *tselect)
{
}

/* <sys/asoundlib.h> */
int snd_timer_info(snd_timer_t *handle, snd_timer_info_t *timer)
{
}

/* <sys/asoundlib.h> */
int snd_timer_params(snd_timer_t *handle, snd_timer_params_t *params)
{
}

/* <sys/asoundlib.h> */
int snd_timer_status(snd_timer_t *handle, snd_timer_status_t *status)
{
}

/* <sys/asoundlib.h> */
int snd_timer_start(snd_timer_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_timer_stop(snd_timer_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_timer_continue(snd_timer_t *handle)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_timer_read(snd_timer_t *handle, void *buffer, size_t size)
{
}

/* <sys/asoundlib.h> */
#define SND_HWDEP_OPEN_READ		(O_RDONLY)

/* <sys/asoundlib.h> */
#define SND_HWDEP_OPEN_WRITE		(O_WRONLY)

/* <sys/asoundlib.h> */
#define SND_HWDEP_OPEN_DUPLEX		(O_RDWR)

/* <sys/asoundlib.h> */
#define SND_HWDEP_OPEN_NONBLOCK		(O_NONBLOCK)

/* <sys/asoundlib.h> */
typedef struct snd_hwdep snd_hwdep_t;

/* <sys/asoundlib.h> */
int snd_hwdep_open(snd_hwdep_t **handle, int card, int device, int mode)
{
}

/* <sys/asoundlib.h> */
int snd_hwdep_close(snd_hwdep_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_hwdep_file_descriptor(snd_hwdep_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_hwdep_block_mode(snd_hwdep_t *handle, int enable)
{
}

/* <sys/asoundlib.h> */
int snd_hwdep_info(snd_hwdep_t *handle, snd_hwdep_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_hwdep_ioctl(snd_hwdep_t *handle, int request, void * arg)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_hwdep_write(snd_hwdep_t *handle, const void *buffer, size_t size)
{
}

/* <sys/asoundlib.h> */
ssize_t snd_hwdep_read(snd_hwdep_t *handle, void *buffer, size_t size)
{
}

/* <sys/asoundlib.h> */
#define SND_SEQ_OPEN_OUT	(O_WRONLY)

/* <sys/asoundlib.h> */
#define SND_SEQ_OPEN_IN		(O_RDONLY)

/* <sys/asoundlib.h> */
#define SND_SEQ_OPEN		(O_RDWR)

/* <sys/asoundlib.h> */
typedef struct snd_seq snd_seq_t;

/* <sys/asoundlib.h> */
int snd_seq_open(snd_seq_t **handle, int mode)
{
}

/* <sys/asoundlib.h> */
int snd_seq_close(snd_seq_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_seq_file_descriptor(snd_seq_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_seq_block_mode(snd_seq_t *handle, int enable)
{
}

/* <sys/asoundlib.h> */
int snd_seq_client_id(snd_seq_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_seq_output_buffer_size(snd_seq_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_seq_input_buffer_size(snd_seq_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_seq_resize_output_buffer(snd_seq_t *handle, int size)
{
}

/* <sys/asoundlib.h> */
int snd_seq_resize_input_buffer(snd_seq_t *handle, int size)
{
}

/* <sys/asoundlib.h> */
int snd_seq_system_info(snd_seq_t *handle, snd_seq_system_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_client_info(snd_seq_t *handle, snd_seq_client_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_any_client_info(snd_seq_t *handle, int client, snd_seq_client_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_client_info(snd_seq_t *handle, snd_seq_client_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_create_port(snd_seq_t *handle, snd_seq_port_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_delete_port(snd_seq_t *handle, snd_seq_port_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_port_info(snd_seq_t *handle, int port, snd_seq_port_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_any_port_info(snd_seq_t *handle, int client, int port, snd_seq_port_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_port_info(snd_seq_t *handle, int port, snd_seq_port_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_port_subscription(snd_seq_t *handle, snd_seq_port_subscribe_t *sub)
{
}

/* <sys/asoundlib.h> */
int snd_seq_subscribe_port(snd_seq_t *handle, snd_seq_port_subscribe_t *sub)
{
}

/* <sys/asoundlib.h> */
int snd_seq_unsubscribe_port(snd_seq_t *handle, snd_seq_port_subscribe_t *sub)
{
}

/* <sys/asoundlib.h> */
int snd_seq_query_port_subscribers(snd_seq_t *seq, snd_seq_query_subs_t * subs)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_queue_status(snd_seq_t *handle, int q, snd_seq_queue_status_t *status)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_queue_tempo(snd_seq_t *handle, int q, snd_seq_queue_tempo_t *tempo)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_queue_tempo(snd_seq_t *handle, int q, snd_seq_queue_tempo_t *tempo)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_queue_owner(snd_seq_t *handle, int q, snd_seq_queue_owner_t *owner)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_queue_owner(snd_seq_t *handle, int q, snd_seq_queue_owner_t *owner)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_queue_timer(snd_seq_t *handle, int q, snd_seq_queue_timer_t *timer)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_queue_timer(snd_seq_t *handle, int q, snd_seq_queue_timer_t *timer)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_queue_sync(snd_seq_t *handle, int q, snd_seq_queue_sync_t *sync)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_queue_sync(snd_seq_t *handle, int q, snd_seq_queue_sync_t *sync)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_queue_client(snd_seq_t *handle, int q, snd_seq_queue_client_t *queue)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_queue_client(snd_seq_t *handle, int q, snd_seq_queue_client_t *queue)
{
}

/* <sys/asoundlib.h> */
int snd_seq_alloc_named_queue(snd_seq_t *seq, char *name)
{
}

/* <sys/asoundlib.h> */
int snd_seq_alloc_queue(snd_seq_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_seq_free_queue(snd_seq_t *handle, int q)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_queue_info(snd_seq_t *seq, int q, snd_seq_queue_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_queue_info(snd_seq_t *seq, int q, snd_seq_queue_info_t *info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_named_queue(snd_seq_t *seq, char *name)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_client_pool(snd_seq_t *handle, snd_seq_client_pool_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_client_pool(snd_seq_t *handle, snd_seq_client_pool_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_query_next_client(snd_seq_t *handle, snd_seq_client_info_t * info)
{
}

/* <sys/asoundlib.h> */
int snd_seq_query_next_port(snd_seq_t *handle, snd_seq_port_info_t * info)
{
}

/* <sys/asoundlib.h> */
snd_seq_event_t *snd_seq_create_event(void)
{
}

/* <sys/asoundlib.h> */
int snd_seq_free_event(snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_event_length(snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_event_output(snd_seq_t *handle, snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_event_output(snd_seq_t *handle, snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_event_output_buffer(snd_seq_t *handle, snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_event_output_direct(snd_seq_t *handle, snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_event_input(snd_seq_t *handle, snd_seq_event_t **ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_event_input_pending(snd_seq_t *seq, int fetch_sequencer)
{
}

/* <sys/asoundlib.h> */
int snd_seq_event_input_selective(snd_seq_t *seq, snd_seq_event_t **ev, int type, int blocking)
{
}

/* <sys/asoundlib.h> */
int snd_seq_flush_output(snd_seq_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_seq_event_output_pending(snd_seq_t *seq)
{
}

/* <sys/asoundlib.h> */
int snd_seq_extract_output(snd_seq_t *handle, snd_seq_event_t **ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_drain_output(snd_seq_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_seq_drain_output_buffer(snd_seq_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_seq_drain_input(snd_seq_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_seq_drain_input_buffer(snd_seq_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_seq_remove_events(snd_seq_t *handle, snd_seq_remove_events_t *info)
{
}

/* <sys/asoundlib.h> */
void snd_seq_set_bit(int nr, void *array)
{
}

/* <sys/asoundlib.h> */
int snd_seq_change_bit(int nr, void *array)
{
}

/* <sys/asoundlib.h> */
int snd_seq_get_bit(int nr, void *array)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_clear(snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_set_dest(snd_seq_event_t *ev, int client, int port)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_set_subs(snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_set_broadcast(snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_set_source(snd_seq_event_t *ev, int port)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_set_direct(snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_schedule_tick(snd_seq_event_t *ev, int q, int relative, snd_seq_tick_time_t tick)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_schedule_real(snd_seq_event_t *ev, int q, int relative, snd_seq_real_time_t *time)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_set_priority(snd_seq_event_t *ev, int high_prior)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_set_fixed(snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_set_variable(snd_seq_event_t *ev, int len, void *ptr)
{
}

/* <sys/asoundlib.h> */
void snd_seq_ev_set_varusr(snd_seq_event_t *ev, int len, void *ptr)
{
}

/* <sys/asoundlib.h> */
int snd_seq_ev_set_queue_start(snd_seq_event_t *ev, int q)
{
}

/* <sys/asoundlib.h> */
int snd_seq_ev_set_queue_stop(snd_seq_event_t *ev, int q)
{
}

/* <sys/asoundlib.h> */
int snd_seq_ev_set_queue_continue(snd_seq_event_t *ev, int q)
{
}

/* <sys/asoundlib.h> */
int snd_seq_ev_set_queue_tempo(snd_seq_event_t *ev, int q, int tempo)
{
}

/* <sys/asoundlib.h> */
int snd_seq_ev_set_queue_control(snd_seq_event_t *ev, int type, int q, int value)
{
}

/* <sys/asoundlib.h> */
int snd_seq_ev_set_queue_pos_real(snd_seq_event_t *ev, int q, snd_seq_real_time_t *rtime)
{
}

/* <sys/asoundlib.h> */
int snd_seq_ev_set_queue_pos_tick(snd_seq_event_t *ev, int q, snd_seq_tick_time_t tick)
{
}

/* <sys/asoundlib.h> */
int snd_seq_use_queue(snd_seq_t *seq, int q, int use)
{
}

/* <sys/asoundlib.h> */
int snd_seq_control_queue(snd_seq_t *seq, int q, int type, int value, snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_start_queue(snd_seq_t *seq, int q, snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_stop_queue(snd_seq_t *seq, int q, snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_continue_queue(snd_seq_t *seq, int q, snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_change_queue_tempo(snd_seq_t *seq, int q, int tempo, snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_setpos_queue(snd_seq_t *seq, int q, snd_seq_timestamp_t *rtime, snd_seq_event_t *ev)
{
}

/* <sys/asoundlib.h> */
int snd_seq_create_simple_port(snd_seq_t *seq, char *name, unsigned int caps, unsigned int type)
{
}

/* <sys/asoundlib.h> */
int snd_seq_delete_simple_port(snd_seq_t *seq, int port)
{
}

/* <sys/asoundlib.h> */
int snd_seq_connect_from(snd_seq_t *seq, int my_port, int src_client, int src_port)
{
}

/* <sys/asoundlib.h> */
int snd_seq_connect_to(snd_seq_t *seq, int my_port, int dest_client, int dest_port)
{
}

/* <sys/asoundlib.h> */
int snd_seq_disconnect_from(snd_seq_t *seq, int my_port, int src_client, int src_port)
{
}

/* <sys/asoundlib.h> */
int snd_seq_disconnect_to(snd_seq_t *seq, int my_port, int dest_client, int dest_port)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_client_name(snd_seq_t *seq, char *name)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_client_group(snd_seq_t *seq, char *name)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_client_filter(snd_seq_t *seq, unsigned int filter)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_client_event_filter(snd_seq_t *seq, int event_type)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_client_pool_output(snd_seq_t *seq, int size)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_client_pool_output_room(snd_seq_t *seq, int size)
{
}

/* <sys/asoundlib.h> */
int snd_seq_set_client_pool_input(snd_seq_t *seq, int size)
{
}

/* <sys/asoundlib.h> */
int snd_seq_reset_pool_output(snd_seq_t *seq)
{
}

/* <sys/asoundlib.h> */
int snd_seq_reset_pool_input(snd_seq_t *seq)
{
}

/* <sys/asoundlib.h> */
#define snd_seq_ev_clear(ev)	memset(ev, 0, sizeof(snd_seq_event_t))

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_dest(ev,c,p) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_subs(ev) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_broadcast(ev) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_source(ev,p) ((ev)->source.port = (p))

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_queue_control(ev,t,q,val) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_queue_start(ev,q) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_queue_stop(ev,q) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_queue_continue(ev,q) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_queue_tempo(ev,q,val) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_queue_pos_real(ev,q,rtime) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_queue_pos_tick(ev,q,ttime) ...

/* <sys/asoundlib.h> */
#define snd_seq_start_queue(seq,q,ev) ...

/* <sys/asoundlib.h> */
#define snd_seq_stop_queue(seq,q,ev) ...

/* <sys/asoundlib.h> */
#define snd_seq_continue_queue(seq,q,ev) ...

/* <sys/asoundlib.h> */
#define snd_seq_change_queue_tempo(seq,q,tempo,ev) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_note(ev,ch,key,vel,dur) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_noteon(ev,ch,key,vel) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_noteoff(ev,ch,key,vel) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_keypress(ev,ch,key,vel) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_controller(ev,ch,cc,val) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_pgmchange(ev,ch,val) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_pitchbend(ev,ch,val) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_chanpress(ev,ch,val) ...

/* <sys/asoundlib.h> */
#define snd_seq_ev_set_sysex(ev,datalen,dataptr) ...

/* <sys/asoundlib.h> */
#define snd_host_to_LE_16(val)	...

/* <sys/asoundlib.h> */
#define snd_LE_to_host_16(val)	...

/* <sys/asoundlib.h> */
#define snd_host_to_LE_32(val)	...

/* <sys/asoundlib.h> */
#define snd_LE_to_host_32(val)	...

/* <sys/asoundlib.h> */
#define snd_host_to_BE_16(val)	...

/* <sys/asoundlib.h> */
#define snd_BE_to_host_16(val)	...

/* <sys/asoundlib.h> */
#define snd_host_to_BE_32(val)	...

/* <sys/asoundlib.h> */
#define snd_BE_to_host_32(val)	...

/* <sys/asoundlib.h> */
typedef void snd_instr_simple_t;

/* <sys/asoundlib.h> */
int snd_instr_simple_convert_to_stream(snd_instr_simple_t *simple, const char *name, snd_seq_instr_put_t **put, long *size)
{
}

/* <sys/asoundlib.h> */
int snd_instr_simple_convert_from_stream(snd_seq_instr_get_t *data, long size, snd_instr_simple_t **simple)
{
}

/* <sys/asoundlib.h> */
int snd_instr_simple_free(snd_instr_simple_t *simple)
{
}

/* <sys/asoundlib.h> */
typedef void snd_instr_iwffff_t;

/* <sys/asoundlib.h> */
typedef struct snd_iwffff_handle snd_iwffff_handle_t;

/* <sys/asoundlib.h> */
int snd_instr_iwffff_open(snd_iwffff_handle_t **handle, const char *name_fff, const char *name_dta)
{
}

/* <sys/asoundlib.h> */
int snd_instr_iwffff_open_rom(snd_iwffff_handle_t **handle, int card, int bank, int file)
{
}

/* <sys/asoundlib.h> */
int snd_instr_iwffff_open_rom_file(snd_iwffff_handle_t **handle, const char *name, int bank, int file)
{
}

/* <sys/asoundlib.h> */
int snd_instr_iwffff_close(snd_iwffff_handle_t *handle)
{
}

/* <sys/asoundlib.h> */
int snd_instr_iwffff_load(snd_iwffff_handle_t *handle, int bank, int prg, snd_instr_iwffff_t **iwffff)
{
}

/* <sys/asoundlib.h> */
int snd_instr_iwffff_convert_to_stream(snd_instr_iwffff_t *iwffff, const char *name, snd_seq_instr_put_t **data, long *size)
{
}

/* <sys/asoundlib.h> */
int snd_instr_iwffff_convert_from_stream(snd_seq_instr_get_t *data, long size, snd_instr_iwffff_t **iwffff)
{
}

/* <sys/asoundlib.h> */
int snd_instr_iwffff_free(snd_instr_iwffff_t *iwffff)
{
}

/* <sys/cdefs.h> ==================================== */

/* <sys/cdefs.h> */
# define __BEGIN_DECLS	extern "C" {

/* <sys/cdefs.h> */
#define __CONCAT(x,y)	x ## y

/* <sys/cdefs.h> */
# define __END_DECLS	}

/* <sys/cdefs.h> */
# define __END_DECLS

/* <sys/cdefs.h> */
# define __P(args)	args

/* <sys/cdefs.h> */
# define __PMT(args)	args

/* <sys/cdefs.h> */
#define __STRING(x)	#x

/* <sys/cdefs.h> */
# define __bounded	/* nothing */

/* <sys/cdefs.h> */
# define __const	const

/* <sys/cdefs.h> */
# define __extension__		/* Ignore */

/* <sys/cdefs.h> */
/* GCC 2.97 supports C99 flexible array members.  */
# define __flexarr	[]

/* <sys/cdefs.h> */
#  define __flexarr	[0]

/* <sys/cdefs.h> */
#   define __flexarr	[]

/* <sys/cdefs.h> */
/* Some other non-C99 compiler.  Approximate with [1].  */
#   define __flexarr	[1]

/* <sys/cdefs.h> */
# define __inline		/* No inline functions.  */

/* <sys/cdefs.h> */
#define __long_double_t  long double

/* <sys/cdefs.h> */
/* This is not a typedef so `const __ptr_t' does the right thing.  */
#define __ptr_t void *

/* <sys/cdefs.h> */
# define __ptrvalue	/* nothing */

/* <sys/cdefs.h> */
# define __restrict	/* Ignore */

/* <sys/cdefs.h> */
# define __restrict_arr	__restrict

/* <sys/cdefs.h> */
#  define __restrict_arr	/* Not supported in old GCC.  */

/* <sys/cdefs.h> */
#   define __restrict_arr	restrict

/* <sys/cdefs.h> */
/* Some other non-C99 compiler.  */
#   define __restrict_arr	/* Not supported.  */

/* <sys/cdefs.h> */
# define __signed	signed

/* <sys/cdefs.h> */
# define __unbounded	/* nothing */

/* <sys/cdefs.h> */
# define __volatile	volatile
/* <sys/debugreg.h> ==================================== */

/* <sys/debugreg.h> */
#define DR_CONTROL 7          /* u_debugreg[DR_CONTROL] */

/* <sys/debugreg.h> */
#define DR_CONTROL_RESERVED (0xFC00) /* Reserved by Intel */

/* <sys/debugreg.h> */
#define DR_CONTROL_SHIFT 16   /* Skip this many bits in ctl register */

/* <sys/debugreg.h> */
#define DR_CONTROL_SIZE  4    /* 4 control bits per register */

/* <sys/debugreg.h> */
#define DR_ENABLE_SIZE	       2   /* 2 enable bits per register */

/* <sys/debugreg.h> */
/* Indicate the register numbers for a number of the specific
   debug registers.  Registers 0-3 contain the addresses we wish to trap on */
#define DR_FIRSTADDR 0        /* u_debugreg[DR_FIRSTADDR] */

/* <sys/debugreg.h> */
#define DR_GLOBAL_ENABLE_MASK (0xAA) /* Set global bits for all 4 regs */

/* <sys/debugreg.h> */
#define DR_GLOBAL_ENABLE_SHIFT 1   /* Extra shift to the global enable bit */

/* <sys/debugreg.h> */
#define DR_GLOBAL_SLOWDOWN  (0x200)  /* Global slow the pipeline */

/* <sys/debugreg.h> */
#define DR_LASTADDR 3         /* u_debugreg[DR_LASTADDR]  */

/* <sys/debugreg.h> */
#define DR_LEN_1 (0x0)	      /* Settings for data length to trap on */

/* <sys/debugreg.h> */
#define DR_LEN_2 (0x4)

/* <sys/debugreg.h> */
#define DR_LEN_4 (0xC)

/* <sys/debugreg.h> */
#define DR_LOCAL_ENABLE_MASK  (0x55) /* Set  local bits for all 4 regs */

/* <sys/debugreg.h> */
#define DR_LOCAL_ENABLE_SHIFT  0   /* Extra shift to the local enable bit */

/* <sys/debugreg.h> */
#define DR_LOCAL_SLOWDOWN   (0x100)  /* Local slow the pipeline */

/* <sys/debugreg.h> */
#define DR_RW_EXECUTE	(0x0) /* Settings for the access types to trap on */

/* <sys/debugreg.h> */
#define DR_RW_READ	(0x3)

/* <sys/debugreg.h> */
#define DR_RW_WRITE	(0x1)

/* <sys/debugreg.h> */
#define DR_STATUS 6           /* u_debugreg[DR_STATUS]     */

/* <sys/debugreg.h> */
#define DR_STEP		(0x4000)	/* single-step */

/* <sys/debugreg.h> */
#define DR_SWITCH	(0x8000)	/* task switch */

/* <sys/debugreg.h> */
#define DR_TRAP0	(0x1)		/* db0 */

/* <sys/debugreg.h> */
#define DR_TRAP1	(0x2)		/* db1 */

/* <sys/debugreg.h> */
#define DR_TRAP2	(0x4)		/* db2 */

/* <sys/debugreg.h> */
#define DR_TRAP3	(0x8)		/* db3 */

/* <sys/debugreg.h> */
#define _SYS_DEBUGREG_H	1

/* <sys/dir.h> ==================================== */

/* <sys/dir.h> */
#define	_SYS_DIR_H	1

/* <sys/dir.h> */
#define	direct	dirent

/* <sys/elf.h> ==================================== */

/* <sys/elf.h> */
#define _SYS_ELF_H	1

/* <sys/file.h> ==================================== */

/* <sys/file.h> */
#define	LOCK_EX	2 	/* Exclusive lock.  */

/* <sys/file.h> */
#define	LOCK_NB	4	/* Don't block when locking.  */

/* <sys/file.h> */
#define	LOCK_SH	1	/* Shared lock.  */

/* <sys/file.h> */
#define	LOCK_UN	8	/* Unlock.  */

/* <sys/file.h> */
# define L_INCR	1	/* Seek from current position.  */

/* <sys/file.h> */
# define L_SET	0	/* Seek from beginning of file.  */

/* <sys/file.h> */
# define L_XTND	2	/* Seek from end of file.  */

/* <sys/gmon.h> ==================================== */

/* <sys/gmon.h> */
/*
 * percent of text space to allocate for tostructs with a minimum.
 */
#define ARCDENSITY	2

/* <sys/gmon.h> */
#define	GMON_PROF_BUSY	1

/* <sys/gmon.h> */
#define	GMON_PROF_ERROR	2

/* <sys/gmon.h> */
#define	GMON_PROF_OFF	3

/* <sys/gmon.h> */
/*
 * Possible states of profiling.
 */
#define	GMON_PROF_ON	0

/* <sys/gmon.h> */
#define	GPROF_COUNT	1	/* struct: profile tick count buffer */

/* <sys/gmon.h> */
#define	GPROF_FROMS	2	/* struct: from location hash bucket */

/* <sys/gmon.h> */
#define	GPROF_GMONPARAM	4	/* struct: profiling parameters (see above) */

/* <sys/gmon.h> */
/*
 * Sysctl definitions for extracting profiling information from the kernel.
 */
#define	GPROF_STATE	0	/* int: profiling enabling variable */

/* <sys/gmon.h> */
#define	GPROF_TOS	3	/* struct: destination/count structure */

/* <sys/gmon.h> */
/*
 * histogram counters are unsigned shorts (according to the kernel).
 */
#define	HISTCOUNTER	unsigned short

/* <sys/gmon.h> */
/*
 * fraction of text space to allocate for histogram counters here, 1/2
 */
#define	HISTFRACTION	2

/* <sys/gmon.h> */
#define MAXARCS		((1 << (8 * sizeof(HISTCOUNTER))) - 2)

/* <sys/gmon.h> */
#define MINARCS		50

/* <sys/gmon.h> */
/*
 * general rounding functions.
 */
#define ROUNDDOWN(x,y)	(((x)/(y))*(y))

/* <sys/gmon.h> */
#define ROUNDUP(x,y)	((((x)+(y)-1)/(y))*(y))

/* <sys/gmon.h> */
#define	_SYS_GMON_H	1

/* <sys/gmon.h> */
/* structure emitted by "gcc -a".  This must match struct bb in
   gcc/libgcc2.c.  It is OK for gcc to declare a longer structure as
   long as the members below are present.  */
struct __bb
{
  long			zero_word;
  const char		*filename;
  long			*counts;
  long			ncounts;
  struct __bb		*next;
  const unsigned long	*addresses;
};

/* <sys/gmon.h> */
extern struct __bb *__bb_head;

/* <sys/gmon.h> */
extern struct gmonparam _gmonparam;

/* <sys/gmon.h> */
/*
 * The profiling data structures are housed in this structure.
 */
struct gmonparam {
	long	state;
	u_short		*kcount;
	u_long		kcountsize;
	u_short		*froms;
	u_long		fromssize;
	struct tostruct	*tos;
	u_long		tossize;
	long		tolimit;
	u_long		lowpc;
	u_long		highpc;
	u_long		textsize;
	u_long		hashfraction;
	long		log_hashfraction;
};

/* <sys/gmon.h> */
/*
 * a raw arc, with pointers to the calling site and
 * the called site and a count.
 */
struct rawarc {
	u_long	raw_frompc;
	u_long	raw_selfpc;
	long	raw_count;
};

/* <sys/gmon.h> */
struct tostruct {
	u_long	selfpc;
	long	count;
	u_short	link;
	u_short pad;
};

/* <sys/gmon_out.h> ==================================== */

/* <sys/gmon_out.h> */
#define	GMON_MAGIC	"gmon"	/* magic cookie */

/* <sys/gmon_out.h> */
/* types of records in this file: */
typedef enum
  {
    GMON_TAG_TIME_HIST = 0,
    GMON_TAG_CG_ARC = 1,
    GMON_TAG_BB_COUNT = 2
  } GMON_Record_Tag;

/* <sys/gmon_out.h> */
/* For profiling shared object we need a new format.  */
#define GMON_SHOBJ_VERSION	0x1ffff

/* <sys/gmon_out.h> */
/* types of records in this file: */
typedef enum
  {
    GMON_TAG_TIME_HIST = 0,
    GMON_TAG_CG_ARC = 1,
    GMON_TAG_BB_COUNT = 2
  } GMON_Record_Tag;

/* <sys/gmon_out.h> */
/* types of records in this file: */
typedef enum
  {
    GMON_TAG_TIME_HIST = 0,
    GMON_TAG_CG_ARC = 1,
    GMON_TAG_BB_COUNT = 2
  } GMON_Record_Tag;

/* <sys/gmon_out.h> */
/* types of records in this file: */
typedef enum
  {
    GMON_TAG_TIME_HIST = 0,
    GMON_TAG_CG_ARC = 1,
    GMON_TAG_BB_COUNT = 2
  } GMON_Record_Tag;

/* <sys/gmon_out.h> */
#define GMON_VERSION	1	/* version number */

/* <sys/gmon_out.h> */
#define _SYS_GMON_OUT_H	1

/* <sys/gmon_out.h> */
struct gmon_cg_arc_record
  {
    char from_pc[sizeof (char *)];	/* address within caller's body */
    char self_pc[sizeof (char *)];	/* address within callee's body */
    char count[4];			/* number of arc traversals */
  };

/* <sys/gmon_out.h> */
/*
 * Raw header as it appears on file (without padding).  This header
 * always comes first in gmon.out and is then followed by a series
 * records defined below.
 */
struct gmon_hdr
  {
    char cookie[4];
    char version[4];
    char spare[3 * 4];
  };

/* <sys/gmon_out.h> */
struct gmon_hist_hdr
  {
    char low_pc[sizeof (char *)];	/* base pc address of sample buffer */
    char high_pc[sizeof (char *)];	/* max pc address of sampled buffer */
    char hist_size[4];			/* size of sample buffer */
    char prof_rate[4];			/* profiling clock rate */
    char dimen[15];			/* phys. dim., usually "seconds" */
    char dimen_abbrev;			/* usually 's' for "seconds" */
  };

/* <sys/io.h> ==================================== */

/* <sys/io.h> */
#define	_SYS_IO_H	1

/* <sys/io.h> */
static __inline unsigned char inb_p (unsigned short port)
{
}

/* <sys/io.h> */
static __inline unsigned int inl (unsigned short port)
{
}

/* <sys/io.h> */
static __inline unsigned int inl_p (unsigned short port)
{
}

/* <sys/io.h> */
static __inline void insb (unsigned short port, void *addr, unsigned long count)
{
}

/* <sys/io.h> */
static __inline void insl (unsigned short port, void *addr, unsigned long count)
{
}

/* <sys/io.h> */
static __inline void insw (unsigned short port, void *addr, unsigned long count)
{
}

/* <sys/io.h> */
static __inline unsigned short inw (unsigned short port)
{
}

/* <sys/io.h> */
static __inline unsigned short inw_p (unsigned short port)
{
}

/* <sys/io.h>
 * Portability note: not all Linux platforms support this call.  Most
 * platforms based on the PC I/O architecture probably will, however.
 * E.g., Linux/Alpha for Alpha PCs supports this.  */
int ioperm (unsigned long __from, unsigned long __num, int __turn_on)
{
}

/* <sys/io.h> */
static __inline void outb (unsigned char value, unsigned short port)
{
}

/* <sys/io.h> */
static __inline void outb_p (unsigned char value, unsigned short port)
{
}

/* <sys/io.h> */
static __inline void outl (unsigned int value, unsigned short port)
{
}

/* <sys/io.h> */
static __inline void outl_p (unsigned int value, unsigned short port)
{
}

/* <sys/io.h> */
static __inline void outsb (unsigned short port, const void *addr, unsigned long count)
{
}

/* <sys/io.h> */
static __inline void outsl (unsigned short port, const void *addr, unsigned long count)
{
}

/* <sys/io.h> */
static __inline void outsw (unsigned short port, const void *addr, unsigned long count)
{
}

/* <sys/io.h> */
static __inline void outw (unsigned short value, unsigned short port)
{
}

/* <sys/io.h> */
static __inline void outw_p (unsigned short value, unsigned short port)
{
}

/* <sys/ipc.h> ==================================== */

/* <sys/ipc.h> */
#define _SYS_IPC_H	1

/* <sys/ipc.h> */
# define __gid_t_defined

/* <sys/ipc.h> */
# define __key_t_defined

/* <sys/ipc.h> */
# define __mode_t_defined

/* <sys/ipc.h> */
# define __uid_t_defined

/* <sys/ipc.h> */
typedef __gid_t gid_t;

/* <sys/ipc.h> */
typedef __key_t key_t;

/* <sys/ipc.h> */
typedef __mode_t mode_t;

/* <sys/ipc.h> */
typedef __uid_t uid_t;

/* <sys/mman.h> ==================================== */

/* <sys/mman.h> */
/* Return value of `mmap' in case of an error.  */
#define MAP_FAILED	((void *) -1)

/* <sys/mman.h> */
#define	_SYS_MMAN_H	1

/* <sys/mman.h> */
# define __mode_t_defined

/* <sys/mman.h> */
#define __need_size_t

/* <sys/mman.h> */
# define __off_t_defined

/* <sys/mman.h> */
#  define mmap mmap64

/* <sys/mman.h> */
typedef __mode_t mode_t;

/* <sys/mman.h> */
typedef __off_t off_t;

/* <sys/mman.h> */
typedef __off64_t off_t;

/* <sys/mount.h> ==================================== */

/* <sys/mount.h> */
#define BLKFLSBUF  _IO(0x12, 97) /* Flush buffer cache.  */

/* <sys/mount.h> */
#define BLKGETSIZE _IO(0x12, 96) /* Return device size.  */

/* <sys/mount.h> */
#define BLKRAGET   _IO(0x12, 99) /* Get current read ahead setting.  */

/* <sys/mount.h> */
#define BLKRASET   _IO(0x12, 98) /* Set read ahead for block device.  */

/* <sys/mount.h> */
#define BLKROGET   _IO(0x12, 94) /* Get read-only status (0 = read_write).  */

/* <sys/mount.h> */
#define BLKROSET   _IO(0x12, 93) /* Set device read-only (0 = read-write).  */

/* <sys/mount.h> */
#define BLKRRPART  _IO(0x12, 95) /* Re-read partition table.  */

/* <sys/mount.h> */
#define BLOCK_SIZE	1024

/* <sys/mount.h> */
#define BLOCK_SIZE_BITS	10

/* <sys/mount.h> */
enum

  MS_RDONLY = 1,		/* Mount read-only.  */
  MS_NOSUID = 2,		/* Ignore suid and sgid bits.  */
  MS_NODEV = 4,			/* Disallow access to device special files.  */
  MS_NOEXEC = 8,		/* Disallow program execution.  */
  MS_SYNCHRONOUS = 16,		/* Writes are synced at once.  */
  MS_REMOUNT = 32,		/* Alter flags of a mounted FS.  */
  MS_MANDLOCK = 64,		/* Allow mandatory locks on an FS.  */
  S_WRITE = 128,		/* Write on file/directory/symlink.  */
  S_APPEND = 256,		/* Append-only file.  */
  S_IMMUTABLE = 512,		/* Immutable file.  */
  MS_NOATIME = 1024,		/* Do not update access times.  */
  MS_NODIRATIME = 2048,		/* Do not update directory access times.  */
  MS_BIND = 4096,		/* Bind directory at different place.  */
};

/* Possible value for FLAGS parameter of `umount2'.  */
enum

  MNT_FORCE = 1			/* Force unmounting.  */
};

/* <sys/mount.h> */
#define MS_MGC_MSK 0xffff0000	/* Magic flag number mask */

/* <sys/mount.h> */
#define MS_MGC_VAL 0xc0ed0000	/* Magic flag number to indicate "new" flags */

/* <sys/mount.h> */
/* Flags that can be altered by MS_REMOUNT  */
#define MS_RMT_MASK (MS_RDONLY | MS_MANDLOCK)

/* <sys/msg.h> ==================================== */

/* <sys/msg.h> */
/* Template for struct to be used as argument for `msgsnd' and `msgrcv'.  */
struct msgbuf
  {
    long mtype;		/* type of received/sent message */
    char mtext[1];		/* text of the message */
  };

/* <sys/mtio.h> ==================================== */

/* <sys/mtio.h> */
# define DEFTAPE	"/dev/tape"

/* <sys/mtio.h> */
			   MTEOM positions after the last FM, ready for
			   appending another file.  */
#define MTERASE 13	/* Erase tape -- be careful!  */

/* <sys/mtio.h> */
#define GMT_BOT(x)              ((x) & 0x40000000)

/* <sys/mtio.h> */
/* #define GMT_ ? 		((x) & 0x00100000) */
/* #define GMT_ ? 		((x) & 0x00080000) */
#define GMT_DR_OPEN(x)          ((x) & 0x00040000)  /* Door open (no tape).  */
/* #define GMT_ ? 		((x) & 0x00020000) */

/* <sys/mtio.h> */
#define GMT_D_1600(x)           ((x) & 0x00400000)

/* <sys/mtio.h> */
#define GMT_D_6250(x)           ((x) & 0x00800000)

/* <sys/mtio.h> */
#define GMT_D_800(x)            ((x) & 0x00200000)
/* #define GMT_ ? 		((x) & 0x00100000) */
/* #define GMT_ ? 		((x) & 0x00080000) */

/* <sys/mtio.h> */
#define GMT_EOD(x)              ((x) & 0x08000000)  /* DDS EOD */

/* <sys/mtio.h> */
/* Generic Mag Tape (device independent) status macros for examining
   mt_gstat -- HP-UX compatible.
   There is room for more generic status bits here, but I don't
   know which of them are reserved. At least three or so should
   be added to make this really useful.  */
#define GMT_EOF(x)              ((x) & 0x80000000)

/* <sys/mtio.h> */
#define GMT_EOT(x)              ((x) & 0x20000000)

/* <sys/mtio.h> */
/* #define GMT_ ? 		((x) & 0x00020000) */
#define GMT_IM_REP_EN(x)        ((x) & 0x00010000)  /* Immediate report mode.*/
/* 16 generic status bits unused.  */

/* <sys/mtio.h> */
/* #define GMT_ ? 		((x) & 0x02000000) */
#define GMT_ONLINE(x)           ((x) & 0x01000000)

/* <sys/mtio.h> */
#define GMT_SM(x)               ((x) & 0x10000000)  /* DDS setmark */

/* <sys/mtio.h> */
#define GMT_WR_PROT(x)          ((x) & 0x04000000)
/* #define GMT_ ? 		((x) & 0x02000000) */

/* <sys/mtio.h> */
			 * position at first record of next file.  */
#define MTBSF	2	/* Backward space FileMark (position before FM).  */

/* <sys/mtio.h> */
#define MTBSFM	10	/* +backward space FileMark, position at FM.  */

/* <sys/mtio.h> */
#define MTBSR	4	/* Backward space record.  */

/* <sys/mtio.h> */
#define MTBSS	26	/* Space backward over setmarks.  */

/* <sys/mtio.h> */
#define MTCOMPRESSION 32/* Control compression with SCSI mode page 15.  */

/* <sys/mtio.h> */
#define MTEOM	12	/* Goto end of recorded media (for appending files).
			   MTEOM positions after the last FM, ready for
			   appending another file.  */

/* <sys/mtio.h> */
			   MTEOM positions after the last FM, ready for
			   appending another file.  */
#define MTERASE 13	/* Erase tape -- be careful!  */

/* <sys/mtio.h> */
#define MTFSF	1	/* Forward space over FileMark,

/* <sys/mtio.h> */
#define MTFSFM  11	/* +forward space FileMark, position at FM.  */

/* <sys/mtio.h> */
#define MTFSR	3	/* Forward space record.  */

/* <sys/mtio.h> */
			     Ordinary buffered operation with code 1.  */
#define MTFSS	25	/* Space forward over setmarks.  */

/* <sys/mtio.h> */
#define	MTIOCGET	_IOR('m', 2, struct mtget)	/* Get tape status.  */

/* <sys/mtio.h> */
/* The next two are used by the QIC-02 driver for runtime reconfiguration.
   See tpqic02.h for struct mtconfiginfo.  */
#define	MTIOCGETCONFIG	_IOR('m', 4, struct mtconfiginfo) /* Get tape config.*/

/* <sys/mtio.h> */
#define	MTIOCPOS	_IOR('m', 3, struct mtpos)	/* Get tape position.*/

/* <sys/mtio.h> */
#define	MTIOCSETCONFIG	_IOW('m', 5, struct mtconfiginfo) /* Set tape config.*/

/* <sys/mtio.h> */
/* Magnetic tape I/O control commands.  */
#define	MTIOCTOP	_IOW('m', 1, struct mtop)	/* Do a mag tape op. */

/* <sys/mtio.h> */
#define MTLOAD  30	/* Execute the SCSI load command.  */

/* <sys/mtio.h> */
#define MTLOCK  28	/* Lock the drive door.  */

/* <sys/mtio.h> */
#define MTMKPART  34	/* Format the tape with one or two partitions.  */

/* <sys/mtio.h> */
#define MTNOP	8	/* No op, set status only (read with MTIOCGET).  */

/* <sys/mtio.h> */
#define MTOFFL	7	/* Rewind and put the drive offline (eject?).  */

/* <sys/mtio.h> */
#define MTRAS1  14	/* Run self test 1 (nondestructive).  */

/* <sys/mtio.h> */
#define MTRAS2	15	/* Run self test 2 (destructive).  */

/* <sys/mtio.h> */
#define MTRAS3  16	/* Reserved for self test 3.  */

/* <sys/mtio.h> */
/* Magnetic Tape operations [Not all operations supported by all drivers].  */
#define MTRESET 0	/* +reset drive in case of problems.  */

/* <sys/mtio.h> */
#define MTRETEN 9	/* Retension tape.  */

/* <sys/mtio.h> */
#define MTREW	6	/* Rewind.  */

/* <sys/mtio.h> */
#define MTSEEK	22	/* Seek to block (Tandberg, etc.).  */

/* <sys/mtio.h> */
#define MTSETBLK 20	/* Set block length (SCSI).  */

/* <sys/mtio.h> */
#define MTSETDENSITY 21	/* Set tape density (SCSI).  */

/* <sys/mtio.h> */
#define MTSETDRVBUFFER 24 /* Set the drive buffering according to SCSI-2.

/* <sys/mtio.h> */
#define MTSETPART 33	/* Change the active tape partition.  */

/* <sys/mtio.h> */
#define MTTELL	23	/* Tell block (Tandberg, etc.).  */

/* <sys/mtio.h> */
#define MTUNLOAD 31	/* Execute the SCSI unload command.  */

/* <sys/mtio.h> */
#define MTUNLOCK 29	/* Unlock the drive door.  */

/* <sys/mtio.h> */
#define MTWEOF	5	/* Write an end-of-file record (mark).  */

/* <sys/mtio.h> */
#define MTWSM	27	/* Write setmarks.  */

/* <sys/mtio.h> */
#define MT_ISARCHIVESC499	0x0A	/* Archive SC-499 QIC-36 controller. */

/* <sys/mtio.h> */
#define MT_ISARCHIVE_2060L	0x09	/* Archive Viper 2060L.  */

/* <sys/mtio.h> */
#define MT_ISARCHIVE_2150L	0x08	/* Archive Viper 2150L.  */

/* <sys/mtio.h> */
#define MT_ISARCHIVE_5945L2	0x04	/* Archive 5945L-2, QIC-24, QIC-02?. */

/* <sys/mtio.h> */
#define MT_ISARCHIVE_VP60I	0x07	/* Archive VP60i, QIC-02.  */

/* <sys/mtio.h> */
#define MT_ISCMSJ500		0x05	/* CMS Jumbo 500 (QIC-02?).  */

/* <sys/mtio.h> */
#define MT_ISDDS1		0x51	/* DDS device without partitions.  */

/* <sys/mtio.h> */
#define MT_ISDDS2		0x52	/* DDS device with partitions.  */

/* <sys/mtio.h> */
					   Teac DC-1 card (Wangtek type).  */
#define MT_ISEVEREX_FT40A	0x32	/* Everex FT40A (QIC-40).  */

/* <sys/mtio.h> */
#define MT_ISFTAPE_FLAG		0x800000

/* <sys/mtio.h> */
/* QIC-40/80/3010/3020 ftape supported drives.
   20bit vendor ID + 0x800000 (see vendors.h in ftape distribution).  */
#define MT_ISFTAPE_UNKNOWN	0x800000 /* obsolete */

/* <sys/mtio.h> */
#define MT_ISQIC02		0x02	/* Generic QIC-02 tape streamer.  */

/* <sys/mtio.h> */
#define MT_ISQIC02_ALL_FEATURES	0x0F	/* Generic QIC-02 with all features. */

/* <sys/mtio.h> */
#define MT_ISSCSI1		0x71	/* Generic ANSI SCSI-1 tape unit.  */

/* <sys/mtio.h> */
#define MT_ISSCSI2		0x72	/* Generic ANSI SCSI-2 tape unit.  */

/* <sys/mtio.h> */
#define MT_ISTDC3610		0x06	/* Tandberg 6310, QIC-24.  */

/* <sys/mtio.h> */
#define MT_ISTEAC_MT2ST		0x12	/* Teac MT-2ST 155mb drive,

/* <sys/mtio.h> */
/* Constants for mt_type. Not all of these are supported, and
   these are not all of the ones that are supported.  */
#define MT_ISUNKNOWN		0x01

/* <sys/mtio.h> */
#define MT_ISWT5099EEN24	0x11	/* Wangtek 5099-een24, 60MB, QIC-24. */

/* <sys/mtio.h> */
#define MT_ISWT5150		0x03	/* Wangtek 5150EQ, QIC-150, QIC-02.  */

/* <sys/mtio.h> */
#define MT_ST_ASYNC_WRITES	0x2

/* <sys/mtio.h> */
#define MT_ST_AUTO_LOCK		0x40

/* <sys/mtio.h> */
#define MT_ST_BLKSIZE_MASK	0xffffff

/* <sys/mtio.h> */
/* SCSI-tape specific definitions.  Bitfield shifts in the status  */
#define MT_ST_BLKSIZE_SHIFT	0

/* <sys/mtio.h> */
#define MT_ST_BOOLEANS		0x10000000

/* <sys/mtio.h> */
#define MT_ST_BUFFER_WRITES	0x1

/* <sys/mtio.h> */
#define MT_ST_CAN_BSR		0x100

/* <sys/mtio.h> */
#define MT_ST_CAN_PARTITIONS    0x400

/* <sys/mtio.h> */
#define MT_ST_CLEARBOOLEANS	0x40000000

/* <sys/mtio.h> */
/* The mode parameters to be controlled. Parameter chosen with bits 20-28.  */
#define MT_ST_CLEAR_DEFAULT	0xfffff

/* <sys/mtio.h> */
#define MT_ST_DEBUGGING		0x8

/* <sys/mtio.h> */
#define MT_ST_DEF_BLKSIZE	0x50000000

/* <sys/mtio.h> */
#define MT_ST_DEF_COMPRESSION	(MT_ST_DEF_OPTIONS | 0x200000)

/* <sys/mtio.h> */
#define MT_ST_DEF_DENSITY	(MT_ST_DEF_OPTIONS | 0x100000)

/* <sys/mtio.h> */
#define MT_ST_DEF_DRVBUFFER	(MT_ST_DEF_OPTIONS | 0x300000)

/* <sys/mtio.h> */
#define MT_ST_DEF_OPTIONS	0x60000000

/* <sys/mtio.h> */
#define MT_ST_DEF_WRITES	0x80

/* <sys/mtio.h> */
#define MT_ST_DENSITY_MASK	0xff000000

/* <sys/mtio.h> */
#define MT_ST_DENSITY_SHIFT	24

/* <sys/mtio.h> */
#define MT_ST_FAST_MTEOM	0x20

/* <sys/mtio.h> */
/* The offset for the arguments for the special HP changer load command.  */
#define MT_ST_HPLOADER_OFFSET 10000

/* <sys/mtio.h> */
#define MT_ST_NO_BLKLIMS	0x200

/* <sys/mtio.h> */
/* Bitfields for the MTSETDRVBUFFER ioctl.  */
#define MT_ST_OPTIONS		0xf0000000

/* <sys/mtio.h> */
#define MT_ST_READ_AHEAD	0x4

/* <sys/mtio.h> */
#define MT_ST_SCSI2LOGICAL      0x800

/* <sys/mtio.h> */
#define MT_ST_SETBOOLEANS	0x30000000

/* <sys/mtio.h> */
#define MT_ST_SOFTERR_MASK	0xffff

/* <sys/mtio.h> */
#define MT_ST_SOFTERR_SHIFT	0

/* <sys/mtio.h> */
#define MT_ST_TWO_FM		0x10

/* <sys/mtio.h> */
#define MT_ST_WRITE_THRESHOLD	0x20000000

/* <sys/mtio.h> */
#define MT_TAPE_INFO \
  {									      \
	{MT_ISUNKNOWN,		"Unknown type of tape device"},		      \
	{MT_ISQIC02,		"Generic QIC-02 tape streamer"},	      \
	{MT_ISWT5150,		"Wangtek 5150, QIC-150"},		      \
	{MT_ISARCHIVE_5945L2,	"Archive 5945L-2"},			      \
	{MT_ISCMSJ500,		"CMS Jumbo 500"},			      \
	{MT_ISTDC3610,		"Tandberg TDC 3610, QIC-24"},		      \
	{MT_ISARCHIVE_VP60I,	"Archive VP60i, QIC-02"},		      \
	{MT_ISARCHIVE_2150L,	"Archive Viper 2150L"},			      \
	{MT_ISARCHIVE_2060L,	"Archive Viper 2060L"},			      \
	{MT_ISARCHIVESC499,	"Archive SC-499 QIC-36 controller"},	      \
	{MT_ISQIC02_ALL_FEATURES, "Generic QIC-02 tape, all features"},	      \
	{MT_ISWT5099EEN24,	"Wangtek 5099-een24, 60MB"},		      \
	{MT_ISTEAC_MT2ST,	"Teac MT-2ST 155mb data cassette drive"},     \
	{MT_ISEVEREX_FT40A,	"Everex FT40A, QIC-40"},		      \
	{MT_ISSCSI1,		"Generic SCSI-1 tape"},			      \
	{MT_ISSCSI2,		"Generic SCSI-2 tape"},			      \
	{0, NULL}							      \
  }

/* <sys/mtio.h> */
#define _IOT_mtconfiginfo /* Hurd ioctl type field.  */ \
  _IOT (_IOTS (long), 2, _IOTS (short), 3, _IOTS (long), 1) /* XXX wrong */

/* <sys/mtio.h> */
    /* The next two fields are not always used.  */
    __daddr_t mt_fileno;	/* Number of current file on tape.  */
    __daddr_t mt_blkno;		/* Current block number.  */
  };
#define _IOT_mtget /* Hurd ioctl type field.  */ \
  _IOT (_IOTS (long), 7, 0, 0, 0, 0)

/* <sys/mtio.h> */
/* Structure for MTIOCTOP - magnetic tape operation command.  */
struct mtop
  {
    short mt_op;		/* Operations defined below.  */
    int mt_count;		/* How many of them.  */
  };
#define _IOT_mtop /* Hurd ioctl type field.  */ \
  _IOT (_IOTS (short), 1, _IOTS (int), 1, 0, 0)

/* <sys/mtio.h> */
struct mtpos
  {
    long mt_blkno;	/* Current block number.  */
  };
#define _IOT_mtpos /* Hurd ioctl type field.  */ \
  _IOT_SIMPLE (long)

/* <sys/mtio.h> */
#define _SYS_MTIO_H	1

/* <sys/mtio.h> */
					   Teac DC-1 card (Wangtek type).  */

/* <sys/mtio.h> */
/* Structure for MTIOCGETCONFIG/MTIOCSETCONFIG primarily intended
   as an interim solution for QIC-02 until DDI is fully implemented.  */
struct mtconfiginfo
  {
    long mt_type;		/* Drive type.  */
    long ifc_type;		/* Interface card type.  */
    unsigned short irqnr;	/* IRQ number to use.  */
    unsigned short dmanr;	/* DMA channel to use.  */
    unsigned short port;	/* IO port base address.  */

/* <sys/mtio.h> */
struct mtget
  {
    long mt_type;		/* Type of magtape device.  */
    long mt_resid;		/* Residual count: (not sure)
				   number of bytes ignored, or
				   number of files not skipped, or
				   number of records not skipped.  */

/* <sys/mtio.h> */
/* Structure for MTIOCTOP - magnetic tape operation command.  */
struct mtop
  {
    short mt_op;		/* Operations defined below.  */
    int mt_count;		/* How many of them.  */
  };
#define _IOT_mtop /* Hurd ioctl type field.  */ \
  _IOT (_IOTS (short), 1, _IOTS (int), 1, 0, 0)

/* <sys/mtio.h> */
struct mtpos
  {
    long mt_blkno;	/* Current block number.  */
  };
#define _IOT_mtpos /* Hurd ioctl type field.  */ \
  _IOT_SIMPLE (long)

/* <sys/param.h> ==================================== */

/* <sys/param.h> */
#define	CANBSIZ		MAX_CANON

/* <sys/param.h> */
/* Unit of `st_blocks'.  */
#define DEV_BSIZE       512

/* <sys/param.h> */
#define	MAX(a,b) (((a)>(b))?(a):(b))

/* <sys/param.h> */
#define MAXPATHLEN	PATH_MAX

/* <sys/param.h> */
#define	MAXSYMLINKS	20

/* <sys/param.h> */
/* Macros for min/max.  */
#define	MIN(a,b) (((a)<(b))?(a):(b))

/* <sys/param.h> */
#define	NBBY		CHAR_BIT

/* <sys/param.h> */
#define	NCARGS		ARG_MAX

/* <sys/param.h> */
# define NGROUPS	NGROUPS_MAX

/* <sys/param.h> */
/* The following is not really correct but it is a value we used for a
   long time and which seems to be usable.  People should not use NOFILE
   anyway.  */
#define NOFILE		256

/* <sys/param.h> */
#define _SYS_PARAM_H	1

/* <sys/param.h> */
#define	clrbit(a,i)	((a)[(i)/NBBY] &= ~(1<<((i)%NBBY)))

/* <sys/param.h> */
# define howmany(x, y)	(((x)+((y)-1))/(y))

/* <sys/param.h> */
#define	isclr(a,i)	(((a)[(i)/NBBY] & (1<<((i)%NBBY))) == 0)

/* <sys/param.h> */
#define	isset(a,i)	((a)[(i)/NBBY] & (1<<((i)%NBBY)))

/* <sys/param.h> */
#define powerof2(x)	((((x)-1)&(x))==0)

/* <sys/param.h> */
#define	roundup(x, y)	((((x)+((y)-1))/(y))*(y))

/* <sys/param.h> */
/* Bit map related macros.  */
#define	setbit(a,i)	((a)[(i)/NBBY] |= 1<<((i)%NBBY))
/* <sys/pci.h> ==================================== */

/* <sys/pci.h> */
#define _SYS_PCI_H	1
/* <sys/perm.h> ==================================== */

/* <sys/perm.h> */
#define _SYS_PERM_H	1
/* <sys/poll.h> ==================================== */

/* <sys/poll.h> */
#define	_SYS_POLL_H	1

/* <sys/poll.h> */
/* Type used for the number of file descriptors.  */
typedef unsigned long nfds_t;

/* <sys/poll.h> */
/* Data structure describing a polling request.  */
struct pollfd
  {
    int fd;			/* File descriptor to poll.  */
    short events;		/* Types of events poller cares about.  */
    short revents;		/* Types of events that actually occurred.  */
  };
/* <sys/prctl.h> ==================================== */

/* <sys/prctl.h> */
#define _SYS_PRCTL_H	1
/* <sys/procfs.h> ==================================== */

/* <sys/procfs.h> */
/* And the whole bunch of them.  We could have used `struct
   user_regs_struct' directly in the typedef, but tradition says that
   the register set is an array, which does have some peculiar
   semantics, so leave it that way.  */
#define ELF_NGREG (sizeof (struct user_regs_struct) / sizeof(elf_greg_t))
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

/* <sys/procfs.h> */
#define ELF_PRARGSZ     (80)    /* Number of chars for args.  */

/* <sys/procfs.h> */
#define _SYS_PROCFS_H	1

/* <sys/procfs.h> */
/* Register set for the floating-point registers.  */
typedef struct user_fpregs_struct elf_fpregset_t;

/* <sys/procfs.h> */
/* Register set for the extended floating-point registers.  Includes
   the Pentium III SSE registers in addition to the classic
   floating-point stuff.  */
typedef struct user_fpxregs_struct elf_fpxregset_t;

/* <sys/procfs.h> */
/* Type for a general-purpose register.  */
typedef unsigned long elf_greg_t;

/* <sys/procfs.h> */
typedef elf_greg_t elf_gregset_t[ELF_NGREG];

/* <sys/procfs.h> */
struct elf_prpsinfo
  {
    char pr_state;			/* Numeric process state.  */
    char pr_sname;			/* Char for pr_state.  */
    char pr_zomb;			/* Zombie.  */
    char pr_nice;			/* Nice val.  */
    unsigned long pr_flag;		/* Flags.  */
    unsigned short pr_uid;
    unsigned short pr_gid;
    int pr_pid, pr_ppid, pr_pgrp, pr_sid;

/* <sys/procfs.h> */
struct elf_prstatus
  {
    struct elf_siginfo pr_info;		/* Info associated with signal.  */
    short pr_cursig;		/* Current signal.  */
    unsigned long pr_sigpend;	/* Set of pending signals.  */
    unsigned long pr_sighold;	/* Set of held signals.  */
    __pid_t pr_pid;
    __pid_t pr_ppid;
    __pid_t pr_pgrp;
    __pid_t pr_sid;
    struct timeval pr_utime;		/* User time.  */
    struct timeval pr_stime;		/* System time.  */
    struct timeval pr_cutime;		/* Cumulative user time.  */
    struct timeval pr_cstime;		/* Cumulative system time.  */
    elf_gregset_t pr_reg;		/* GP registers.  */
    int pr_fpvalid;			/* True if math copro being used.  */
  };

/* <sys/procfs.h> */
/* Signal info.  */
struct elf_siginfo
  {
    int si_signo;			/* Signal number.  */
    int si_code;			/* Extra code.  */
    int si_errno;			/* Errno.  */
  };

/* <sys/procfs.h> */
/* We don't have any differences between processes and threads,
   therefore have only one PID type.  */
typedef __pid_t lwpid_t;

/* <sys/procfs.h> */
typedef elf_fpregset_t prfpregset_t;

/* <sys/procfs.h> */
/* Register sets.  Linux has different names.  */
typedef elf_gregset_t prgregset_t;

/* <sys/procfs.h> */
typedef struct elf_prpsinfo prpsinfo_t;

/* <sys/procfs.h> */
/* Process status and info.  In the end we do provide typedefs for them.  */
typedef struct elf_prstatus prstatus_t;

/* <sys/procfs.h> */
/* Addresses.  */
typedef void *psaddr_t;
/* <sys/profil.h> ==================================== */

/* <sys/profil.h> */
#define _PROFIL_H	1

/* <sys/profil.h> */
struct prof
  {
    void *pr_base;		/* buffer base */
    size_t pr_size;		/* buffer size */
    size_t pr_off;		/* pc offset */
    unsigned long pr_scale;	/* pc scaling (fixed-point number) */
  };
/* <sys/ptrace.h> ==================================== */

/* <sys/ptrace.h> */
  /* Attach to a process that is already running. */
  PTRACE_ATTACH = 16,
#define PT_ATTACH PTRACE_ATTACH

/* <sys/ptrace.h> */
  /* Continue the process.  */
  PTRACE_CONT = 7,
#define PT_CONTINUE PTRACE_CONT

/* <sys/ptrace.h> */
  /* Detach from a process attached to with PTRACE_ATTACH.  */
  PTRACE_DETACH = 17,
#define PT_DETACH PTRACE_DETACH

/* <sys/ptrace.h> */
  /* Get all floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_GETFPREGS = 14,
#define PT_GETFPREGS PTRACE_GETFPREGS

/* <sys/ptrace.h> */
  /* Get all extended floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_GETFPXREGS = 18,
#define PT_GETFPXREGS PTRACE_GETFPXREGS

/* <sys/ptrace.h> */
  /* Get all general purpose registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_GETREGS = 12,
#define PT_GETREGS PTRACE_GETREGS

/* <sys/ptrace.h> */
  /* Kill the process.  */
  PTRACE_KILL = 8,
#define PT_KILL PTRACE_KILL

/* <sys/ptrace.h> */
  /* Return the word in the process's data space at address ADDR.  */
  PTRACE_PEEKDATA = 2,
#define PT_READ_D PTRACE_PEEKDATA

/* <sys/ptrace.h> */
  /* Return the word in the process's text space at address ADDR.  */
  PTRACE_PEEKTEXT = 1,
#define PT_READ_I PTRACE_PEEKTEXT

/* <sys/ptrace.h> */
  /* Return the word in the process's user area at offset ADDR.  */
  PTRACE_PEEKUSER = 3,
#define PT_READ_U PTRACE_PEEKUSER

/* <sys/ptrace.h> */
  /* Set all floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_SETFPREGS = 15,
#define PT_SETFPREGS PTRACE_SETFPREGS

/* <sys/ptrace.h> */
  /* Set all extended floating point registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_SETFPXREGS = 19,
#define PT_SETFPXREGS PTRACE_SETFPXREGS

/* <sys/ptrace.h> */
  /* Set all general purpose registers used by a processes.
     This is not supported on all machines.  */
   PTRACE_SETREGS = 13,
#define PT_SETREGS PTRACE_SETREGS

/* <sys/ptrace.h> */
  /* Single step the process.
     This is not supported on all machines.  */
  PTRACE_SINGLESTEP = 9,
#define PT_STEP PTRACE_SINGLESTEP

/* <sys/ptrace.h> */
  /* Continue and stop at the next (return from) syscall.  */
  PTRACE_SYSCALL = 24
#define PT_SYSCALL PTRACE_SYSCALL

/* <sys/ptrace.h> */
/* Type of the REQUEST argument to `ptrace.'  */
  /* Indicate that the process making this request should be traced.
     All signals received by this process can be intercepted by its
     parent, and its parent can use the other `ptrace' requests.  */
  PTRACE_TRACEME = 0,
#define PT_TRACE_ME PTRACE_TRACEME

/* <sys/ptrace.h> */
  /* Write the word DATA into the process's data space at address ADDR.  */
  PTRACE_POKEDATA = 5,
#define PT_WRITE_D PTRACE_POKEDATA

/* <sys/ptrace.h> */
  /* Write the word DATA into the process's text space at address ADDR.  */
  PTRACE_POKETEXT = 4,
#define PT_WRITE_I PTRACE_POKETEXT

/* <sys/ptrace.h> */
  /* Write the word DATA into the process's user area at offset ADDR.  */
  PTRACE_POKEUSER = 6,
#define PT_WRITE_U PTRACE_POKEUSER

/* <sys/ptrace.h> */
#define _SYS_PTRACE_H	1

/* <sys/ptrace.h> */
/* Type of the REQUEST argument to `ptrace.'  */
enum __ptrace_request

/* <sys/quota.h> ==================================== */

/* <sys/quota.h> */
#define GRPQUOTA  1		/* element used for group quotas */

/* <sys/quota.h> */
/*
 * Definitions for the default names of the quotas files.
 */
#define INITQFNAMES { \
   "user",      /* USRQUOTA */ \
   "group",   /* GRPQUOTA */ \
   "undefined", \
};

/* <sys/quota.h> */
#define MAXQUOTAS 2

/* <sys/quota.h> */
#define MAX_DQ_TIME  604800	/* (7*24*60*60) 1 week */

/* <sys/quota.h> */
/*
 * Definitions for disk quotas imposed on the average user
 * (big brother finally hits Linux).
 *
 * The following constants define the amount of time given a user
 * before the soft limits are treated as hard limits (usually resulting
 * in an allocation failure). The timer is started when the user crosses
 * their soft limit, it is reset when they go below their soft limit.
 */
#define MAX_IQ_TIME  604800	/* (7*24*60*60) 1 week */

/* <sys/quota.h> */
#define NR_DQHASH 43          /* Just an arbitrary number any suggestions ? */

/* <sys/quota.h> */
#define NR_DQUOTS 256         /* Number of quotas active at one time */

/* <sys/quota.h> */
#define QCMD(cmd, type)  (((cmd) << SUBCMDSHIFT) | ((type) & SUBCMDMASK))

/* <sys/quota.h> */
#define QUOTAFILENAME "quota"

/* <sys/quota.h> */
#define QUOTAGROUP "staff"

/* <sys/quota.h> */
#define Q_GETQUOTA 0x0300	/* get limits and usage */

/* <sys/quota.h> */
#define Q_GETSTATS 0x0800	/* get collected stats */

/* <sys/quota.h> */
#define Q_QUOTAOFF 0x0200	/* disable quotas */

/* <sys/quota.h> */
#define Q_QUOTAON  0x0100	/* enable quotas */

/* <sys/quota.h> */
#define Q_RSQUASH  0x1000	/* set root_squash option */

/* <sys/quota.h> */
#define Q_SETQLIM  0x0700	/* set limits */

/* <sys/quota.h> */
#define Q_SETQUOTA 0x0400	/* set limits and usage */

/* <sys/quota.h> */
#define Q_SETUSE   0x0500	/* set usage */

/* <sys/quota.h> */
#define Q_SYNC     0x0600	/* sync disk copy of a filesystems quotas */

/* <sys/quota.h> */
/*
 * Command definitions for the 'quotactl' system call.
 * The commands are broken into a main command defined below
 * and a subcommand that is used to convey the type of
 * quota that is being manipulated (see above).
 */
#define SUBCMDMASK  0x00ff

/* <sys/quota.h> */
#define SUBCMDSHIFT 8

/* <sys/quota.h> */
#define USRQUOTA  0		/* element used for user quotas */

/* <sys/quota.h> */
#define _SYS_QUOTA_H 1

/* <sys/quota.h> */
#define btodb(num) ((num) >> 10)

/* <sys/quota.h> */
/*
 * Convert diskblocks to blocks and the other way around.
 * currently only to fool the BSD source. :-)
 */
#define dbtob(num) ((num) << 10)

/* <sys/quota.h> */
/*
 * Shorthand notation.
 */
#define	dq_bhardlimit	dq_dqb.dqb_bhardlimit

/* <sys/quota.h> */
#define	dq_bsoftlimit	dq_dqb.dqb_bsoftlimit

/* <sys/quota.h> */
#define	dq_btime	dq_dqb.dqb_btime

/* <sys/quota.h> */
#define	dq_curblocks	dq_dqb.dqb_curblocks

/* <sys/quota.h> */
#define	dq_curinodes	dq_dqb.dqb_curinodes

/* <sys/quota.h> */
#define	dq_ihardlimit	dq_dqb.dqb_ihardlimit

/* <sys/quota.h> */
#define	dq_isoftlimit	dq_dqb.dqb_isoftlimit

/* <sys/quota.h> */
#define	dq_itime	dq_dqb.dqb_itime

/* <sys/quota.h> */
/*
 * The following structure defines the format of the disk quota file
 * (as it appears on disk) - the file is an array of these structures
 * indexed by user or group number.
 */
struct dqblk
  {
    u_int32_t dqb_bhardlimit;	/* absolute limit on disk blks alloc */
    u_int32_t dqb_bsoftlimit;	/* preferred limit on disk blks */
    u_int32_t dqb_curblocks;	/* current block count */
    u_int32_t dqb_ihardlimit;	/* maximum # allocated inodes */
    u_int32_t dqb_isoftlimit;	/* preferred inode limit */
    u_int32_t dqb_curinodes;	/* current # allocated inodes */
    time_t dqb_btime;		/* time limit for excessive disk use */
    time_t dqb_itime;		/* time limit for excessive files */
  };

/* <sys/quota.h> */
#define dqoff(UID)      ((loff_t)((UID) * sizeof (struct dqblk)))

/* <sys/quota.h> */
struct dqstats
  {
    u_int32_t lookups;
    u_int32_t drops;
    u_int32_t reads;
    u_int32_t writes;
    u_int32_t cache_hits;
    u_int32_t pages_allocated;
    u_int32_t allocated_dquots;
    u_int32_t free_dquots;
    u_int32_t syncs;
  };

/* <sys/quota.h> */
/*
 * Convert count of filesystem blocks to diskquota blocks, meant
 * for filesystems where i_blksize != BLOCK_SIZE
 */
#define fs_to_dq_blocks(num, blksize) (((num) * (blksize)) / BLOCK_SIZE)
/* <sys/raw.h> ==================================== */

/* <sys/raw.h> */
#define RAW_GETBIND     _IO(0xac, 1)

/* <sys/raw.h> */
/* The major device number for raw devices.  */
#define RAW_MAJOR	162

/* <sys/raw.h> */
/* `ioctl' commands for raw devices.  */
#define RAW_SETBIND     _IO(0xac, 0)

/* <sys/raw.h> */
#define _SYS_RAW_H	1

/* <sys/raw.h> */
struct raw_config_request

  int raw_minor;
  uint64_t block_major;
  uint64_t block_minor;
};
/* <sys/reboot.h> ==================================== */

/* <sys/reboot.h> */
/* Perform a hard reset now.  */
#define RB_AUTOBOOT	0x01234567

/* <sys/reboot.h> */
/* Disable reboot using Ctrl-Alt-Delete keystroke.  */
#define RB_DISABLE_CAD	0

/* <sys/reboot.h> */
/* Enable reboot using Ctrl-Alt-Delete keystroke.  */
#define RB_ENABLE_CAD	0x89abcdef

/* <sys/reboot.h> */
/* Halt the system.  */
#define RB_HALT_SYSTEM	0xcdef0123

/* <sys/reboot.h> */
/* Stop system and switch power off if possible.  */
#define RB_POWER_OFF	0x4321fedc

/* <sys/reboot.h> */
#define _SYS_REBOOT_H	1
/* <sys/reg.h> ==================================== */

/* <sys/reg.h> */
#define CS  13

/* <sys/reg.h> */
#define DS 7

/* <sys/reg.h> */
#define EAX 6

/* <sys/reg.h> */
#define EBP 5

/* <sys/reg.h> */
#define EBX 0

/* <sys/reg.h> */
#define ECX 1

/* <sys/reg.h> */
#define EDI 4

/* <sys/reg.h> */
#define EDX 2

/* <sys/reg.h> */
#define EFL 14

/* <sys/reg.h> */
#define EIP 12

/* <sys/reg.h> */
#define ES 8

/* <sys/reg.h> */
#define ESI 3

/* <sys/reg.h> */
#define FS 9

/* <sys/reg.h> */
#define GS 10

/* <sys/reg.h> */
#define ORIG_EAX 11

/* <sys/reg.h> */
#define SS   16

/* <sys/reg.h> */
#define UESP 15

/* <sys/reg.h> */
#define _SYS_REG_H	1
/* <sys/resource.h> ==================================== */

/* <sys/resource.h> */
#define	_SYS_RESOURCE_H	1

/* <sys/resource.h> */
# define __id_t_defined

/* <sys/resource.h> */
typedef enum __priority_which __priority_which_t;

/* <sys/resource.h> */
typedef int __priority_which_t;

/* <sys/resource.h> */
typedef enum __rlimit_resource __rlimit_resource_t;

/* <sys/resource.h> */
typedef int __rlimit_resource_t;

/* <sys/resource.h> */
typedef enum __rusage_who __rusage_who_t;

/* <sys/resource.h> */
typedef int __rusage_who_t;

/* <sys/resource.h> */
#  define getrlimit getrlimit64

/* <sys/resource.h> */
typedef __id_t id_t;

/* <sys/resource.h> */
#  define setrlimit setrlimit64
/* <sys/select.h> ==================================== */

/* <sys/select.h> */
#define	FD_CLR(fd, fdsetp)	__FD_CLR (fd, fdsetp)

/* <sys/select.h> */
#define	FD_ISSET(fd, fdsetp)	__FD_ISSET (fd, fdsetp)

/* <sys/select.h> */
/* Access macros for `fd_set'.  */
#define	FD_SET(fd, fdsetp)	__FD_SET (fd, fdsetp)

/* <sys/select.h> */
/* Maximum number of file descriptors in `fd_set'.  */
#define	FD_SETSIZE		__FD_SETSIZE

/* <sys/select.h> */
#define	FD_ZERO(fdsetp)		__FD_ZERO (fdsetp)

/* <sys/select.h> */
/* Number of bits per word of `fd_set' (some code assumes this is 32).  */
# define NFDBITS		__NFDBITS

/* <sys/select.h> */
#define _SYS_SELECT_H	1

/* <sys/select.h> */
#define	__FDELT(d)	((d) / __NFDBITS)

/* <sys/select.h> */
#define	__FDMASK(d)	((__fd_mask) 1 << ((d) % __NFDBITS))

/* <sys/select.h> */
# define __FDS_BITS(set) ((set)->fds_bits)

/* <sys/select.h> */
# define __FDS_BITS(set) ((set)->__fds_bits)

/* <sys/select.h> */
/* It's easier to assume 8-bit bytes than to get CHAR_BIT.  */
#define __NFDBITS	(8 * sizeof (__fd_mask))

/* <sys/select.h> */
  } fd_set;

/* <sys/select.h> */
  } fd_set;

/* <sys/select.h> */
/* The fd_set member is required to be an array of longs.  */
typedef long __fd_mask;

/* <sys/select.h> */
/* Get definition of timer specification structures.  */
#define __need_timespec

/* <sys/select.h> */
#define __need_timeval

/* <sys/select.h> */
# define __sigset_t_defined

/* <sys/select.h> */
/* Sometimes the fd_set member is assumed to have this type.  */
typedef __fd_mask fd_mask;

/* <sys/select.h> */
  } fd_set;

/* <sys/select.h> */
typedef __sigset_t sigset_t;
/* <sys/sem.h> ==================================== */

/* <sys/sem.h> */
#define _SYS_SEM_H	1

/* <sys/sem.h> */
#define __need_size_t

/* <sys/sem.h> */
/* Structure used for argument to `semop' to describe operations.  */
struct sembuf

  unsigned short sem_num;	/* semaphore number */
  short sem_op;		/* semaphore operation */
  short sem_flg;		/* operation flag */
};
/* <sys/sendfile.h> ==================================== */

/* <sys/sendfile.h> */
#define _SYS_SENDFILE_H	1
/* <sys/shm.h> ==================================== */

/* <sys/shm.h> */
/* Segment low boundary address multiple.  */
#define SHMLBA		(__getpagesize ())

/* <sys/shm.h> */
int __getpagesize (void)
{
}

/* <sys/shm.h> */
#define _SYS_SHM_H	1

/* <sys/shm.h> */
#define __need_size_t

/* <sys/shm.h> */
/* Define types required by the standard.  */
#define __need_time_t

/* <sys/shm.h> */
#  define __pid_t_defined

/* <sys/shm.h> */
typedef __pid_t pid_t;
/* <sys/signal.h> ==================================== */

/* <sys/socket.h> ==================================== */

/* <sys/socket.h> */
/* The following constants should be used for the second parameter of
   `shutdown'.  */
enum

  SHUT_RD = 0,		/* No more receptions.  */
#define SHUT_RD		SHUT_RD

/* <sys/socket.h> */
  SHUT_RDWR		/* No more receptions or transmissions.  */
#define SHUT_RDWR	SHUT_RDWR

/* <sys/socket.h> */
  SHUT_WR,		/* No more transmissions.  */
#define SHUT_WR		SHUT_WR

/* <sys/socket.h> */
#define	_SYS_SOCKET_H	1

/* <sys/socket.h> */
# define __CONST_SOCKADDR_ARG	__const struct sockaddr *

/* <sys/socket.h> */
# define __SOCKADDR_ALLTYPES ...

/* <sys/socket.h> */
# define __SOCKADDR_ARG		struct sockaddr *__restrict

/* <sys/socket.h> */
# define __SOCKADDR_ONETYPE(type) struct type *__restrict __##type##__;

/* <sys/socket.h> */
# define __SOCKADDR_ONETYPE(type) __const struct type *__restrict __##type##__;

/* <sys/socket.h> */
typedef union { __SOCKADDR_ALLTYPES
	      } __SOCKADDR_ARG ;

/* <sys/socket.h> */
typedef union { __SOCKADDR_ALLTYPES
	      } __CONST_SOCKADDR_ARG ;

/* <sys/socket.h> */
#define	__need_size_t

/* <sys/socket.h> */
/* This is the 4.3 BSD `struct sockaddr' format, which is used as wire
   format in the grotty old 4.3 `talk' protocol.  */
struct osockaddr
  {
    unsigned short sa_family;
    unsigned char sa_data[14];
  };
#endif
/* <sys/socketvar.h> ==================================== */

/* <sys/soundcard.h> ==================================== */

/* <sys/statfs.h> ==================================== */

/* <sys/statfs.h> */
#define	_SYS_STATFS_H	1

/* <sys/statfs.h> */
#  define fstatfs fstatfs64

/* <sys/statfs.h> */
#  define statfs statfs64
/* <sys/stat.h> ==================================== */

/* <sys/stat.h> */
/* Macros for common mode bit masks.  */
# define ACCESSPERMS (S_IRWXU|S_IRWXG|S_IRWXO) /* 0777 */

/* <sys/stat.h> */
# define ALLPERMS (S_ISUID|S_ISGID|S_ISVTX|S_IRWXU|S_IRWXG|S_IRWXO)/* 07777 */

/* <sys/stat.h> */
# define DEFFILEMODE (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH)/* 0666*/

/* <sys/stat.h> */
# define S_BLKSIZE	512	/* Block size for `st_blocks'.  */

/* <sys/stat.h> */
# define S_IEXEC	S_IXUSR

/* <sys/stat.h> */
# define S_IFBLK	__S_IFBLK

/* <sys/stat.h> */
# define S_IFCHR	__S_IFCHR

/* <sys/stat.h> */
# define S_IFDIR	__S_IFDIR

/* <sys/stat.h> */
#  define S_IFIFO	__S_IFIFO

/* <sys/stat.h> */
#  define S_IFLNK	__S_IFLNK

/* <sys/stat.h> */
# define S_IFMT		__S_IFMT

/* <sys/stat.h> */
# define S_IFREG	__S_IFREG

/* <sys/stat.h> */
# define S_IFSOCK	__S_IFSOCK

/* <sys/stat.h> */
# define S_IREAD	S_IRUSR

/* <sys/stat.h> */
#define	S_IRGRP	(S_IRUSR >> 3)	/* Read by group.  */

/* <sys/stat.h> */
#define	S_IROTH	(S_IRGRP >> 3)	/* Read by others.  */

/* <sys/stat.h> */
#define	S_IRUSR	__S_IREAD	/* Read by owner.  */

/* <sys/stat.h> */
/* Read, write, and execute by group.  */
#define	S_IRWXG	(S_IRWXU >> 3)

/* <sys/stat.h> */
/* Read, write, and execute by others.  */
#define	S_IRWXO	(S_IRWXG >> 3)

/* <sys/stat.h> */
/* Read, write, and execute by owner.  */
#define	S_IRWXU	(__S_IREAD|__S_IWRITE|__S_IEXEC)

/* <sys/stat.h> */
#define	S_ISBLK(mode)	 __S_ISTYPE((mode), __S_IFBLK)

/* <sys/stat.h> */
#define	S_ISCHR(mode)	 __S_ISTYPE((mode), __S_IFCHR)

/* <sys/stat.h> */
#define	S_ISDIR(mode)	 __S_ISTYPE((mode), __S_IFDIR)

/* <sys/stat.h> */
# define S_ISFIFO(mode)	 __S_ISTYPE((mode), __S_IFIFO)

/* <sys/stat.h> */
#define	S_ISGID	__S_ISGID	/* Set group ID on execution.  */

/* <sys/stat.h> */
# define S_ISLNK(mode)	 __S_ISTYPE((mode), __S_IFLNK)

/* <sys/stat.h> */
#  define S_ISLNK(mode)  0

/* <sys/stat.h> */
#define	S_ISREG(mode)	 __S_ISTYPE((mode), __S_IFREG)

/* <sys/stat.h> */
#  define S_ISSOCK(mode) __S_ISTYPE((mode), __S_IFSOCK)

/* <sys/stat.h> */
#define	S_ISUID __S_ISUID	/* Set user ID on execution.  */

/* <sys/stat.h> */
/* Save swapped text after use (sticky bit).  This is pretty well obsolete.  */
# define S_ISVTX	__S_ISVTX

/* <sys/stat.h> */
#define	S_IWGRP	(S_IWUSR >> 3)	/* Write by group.  */

/* <sys/stat.h> */
#define	S_IWOTH	(S_IWGRP >> 3)	/* Write by others.  */

/* <sys/stat.h> */
# define S_IWRITE	S_IWUSR

/* <sys/stat.h> */
#define	S_IWUSR	__S_IWRITE	/* Write by owner.  */

/* <sys/stat.h> */
#define	S_IXGRP	(S_IXUSR >> 3)	/* Execute by group.  */
/* Read, write, and execute by group.  */

/* <sys/stat.h> */
#define	S_IXOTH	(S_IXGRP >> 3)	/* Execute by others.  */
/* Read, write, and execute by others.  */

/* <sys/stat.h> */
#define	S_IXUSR	__S_IEXEC	/* Execute by owner.  */

/* <sys/stat.h> */
# define S_TYPEISMQ(buf) __S_TYPEISMQ(buf)

/* <sys/stat.h> */
# define S_TYPEISSEM(buf) __S_TYPEISSEM(buf)

/* <sys/stat.h> */
# define S_TYPEISSHM(buf) __S_TYPEISSHM(buf)

/* <sys/stat.h> */
# define _MKNOD_VER	0

/* <sys/stat.h> */
# define _STAT_VER	0

/* <sys/stat.h> */
#define	_SYS_STAT_H	1

/* <sys/stat.h> */
#define	__S_ISTYPE(mode, mask)	(((mode) & __S_IFMT) == (mask))

/* <sys/stat.h> */
#  define __blkcnt_t_defined

/* <sys/stat.h> */
#  define __blksize_t_defined

/* <sys/stat.h> */
#  define __dev_t_defined

/* <sys/stat.h> */
#  define __fxstat __fxstat64

/* <sys/stat.h> */
#  define __gid_t_defined

/* <sys/stat.h> */
#  define __ino_t_defined

/* <sys/stat.h> */
#  define __lxstat __lxstat64

/* <sys/stat.h> */
#  define __mode_t_defined

/* <sys/stat.h> */
# define __need_time_t

/* <sys/stat.h> */
#  define __nlink_t_defined

/* <sys/stat.h> */
#  define __off_t_defined

/* <sys/stat.h> */
#  define __uid_t_defined

/* <sys/stat.h> */
#  define __xstat __xstat64

/* <sys/stat.h> */
typedef __blkcnt_t blkcnt_t;

/* <sys/stat.h> */
typedef __blkcnt64_t blkcnt_t;

/* <sys/stat.h> */
typedef __blksize_t blksize_t;

/* <sys/stat.h> */
typedef __dev_t dev_t;

/* <sys/stat.h> */
__inline__ int fstat (int __fd, struct stat *__statbuf)
{
}

/* <sys/stat.h> */
#  define fstat fstat64

/* <sys/stat.h> */
__inline__ int fstat64 (int __fd, struct stat64 *__statbuf)
{
}

/* <sys/stat.h> */
typedef __gid_t gid_t;

/* <sys/stat.h> */
typedef __ino_t ino_t;

/* <sys/stat.h> */
typedef __ino64_t ino_t;

/* <sys/stat.h> */
__inline__ int lstat (__const char *__path, struct stat *__statbuf)
{
}

/* <sys/stat.h> */
#   define lstat lstat64

/* <sys/stat.h> */
__inline__ int lstat64 (__const char *__path, struct stat64 *__statbuf)
{
}

/* <sys/stat.h> */
__inline__ int mknod (__const char *__path, __mode_t __mode, __dev_t __dev)
{
}

/* <sys/stat.h> */
typedef __mode_t mode_t;

/* <sys/stat.h> */
typedef __nlink_t nlink_t;

/* <sys/stat.h> */
typedef __off_t off_t;

/* <sys/stat.h> */
typedef __off64_t off_t;

/* <sys/stat.h> */
/* Get file attributes for FILE and put them in BUF.  */
int stat (__const char *__restrict __file, struct stat *__restrict __buf)
{
}

/* <sys/stat.h> */
#  define stat stat64

/* <sys/stat.h> */
typedef __uid_t uid_t;
/* <sys/statvfs.h> ==================================== */

/* <sys/statvfs.h> */
#define	_SYS_STATVFS_H	1

/* <sys/statvfs.h> */
#  define __fsblkcnt_t_defined

/* <sys/statvfs.h> */
#  define __fsblkcnt_t_defined

/* <sys/statvfs.h> */
#  define __fsfilcnt_t_defined

/* <sys/statvfs.h> */
#  define __fsfilcnt_t_defined

/* <sys/statvfs.h> */
typedef __fsblkcnt_t fsblkcnt_t; /* Type to count file system blocks.  */

/* <sys/statvfs.h> */
typedef __fsblkcnt64_t fsblkcnt_t; /* Type to count file system blocks.  */

/* <sys/statvfs.h> */
typedef __fsfilcnt_t fsfilcnt_t; /* Type to count file system inodes.  */

/* <sys/statvfs.h> */
typedef __fsfilcnt64_t fsfilcnt_t; /* Type to count file system inodes.  */

/* <sys/statvfs.h> */
#  define fstatvfs fstatvfs64

/* <sys/statvfs.h> */
#  define statvfs statvfs64
/* <sys/stropts.h> ==================================== */

/* <sys/swap.h> ==================================== */

/* <sys/swap.h> */
/* The swap priority is encoded as:
   (prio << SWAP_FLAG_PRIO_SHIFT) & SWAP_FLAG_PRIO_MASK
*/
#define	SWAP_FLAG_PREFER	0x8000	/* Set if swap priority is specified. */

/* <sys/swap.h> */
#define	SWAP_FLAG_PRIO_MASK	0x7fff

/* <sys/swap.h> */
#define	SWAP_FLAG_PRIO_SHIFT	0

/* <sys/swap.h> */
#define _SYS_SWAP_H	1
/* <sys/syscall.h> ==================================== */

/* <sys/syscall.h> */
#define _SYSCALL_H	1
/* <sys/sysctl.h> ==================================== */

/* <sys/sysctl.h> */
#define	_SYS_SYSCTL_H	1

/* <sys/sysctl.h> */
#define __need_size_t
/* <sys/sysinfo.h> ==================================== */

/* <sys/sysinfo.h> */
#define _SYS_SYSINFO_H	1
/* <sys/syslog.h> ==================================== */

/* <sys/syslog.h> */
typedef struct _code {
	char	*c_name;
	int	c_val;
} CODE;

/* <sys/syslog.h> */
				/* mark "facility" */
#define	INTERNAL_MARK	LOG_MAKEPRI(LOG_NFACILITIES, 0)
typedef struct _code {
	char	*c_name;
	int	c_val;
} CODE;

/* <sys/syslog.h> */
#define	INTERNAL_NOPRI	0x10	/* the "no priority" priority */

/* <sys/syslog.h> */
#define	LOG_ALERT	1	/* action must be taken immediately */

/* <sys/syslog.h> */
#define	LOG_AUTH	(4<<3)	/* security/authorization messages */

/* <sys/syslog.h> */
#define	LOG_AUTHPRIV	(10<<3)	/* security/authorization messages (private) */

/* <sys/syslog.h> */
#define	LOG_CONS	0x02	/* log on the console if errors in sending */

/* <sys/syslog.h> */
#define	LOG_CRIT	2	/* critical conditions */

/* <sys/syslog.h> */
#define	LOG_CRON	(9<<3)	/* clock daemon */

/* <sys/syslog.h> */
#define	LOG_DAEMON	(3<<3)	/* system daemons */

/* <sys/syslog.h> */
#define	LOG_DEBUG	7	/* debug-level messages */

/* <sys/syslog.h> */
/*
 * priorities/facilities are encoded into a single 32-bit quantity, where the
 * bottom 3 bits are the priority (0-7) and the top 28 bits are the facility
 * (0-big number).  Both the priorities and the facilities map roughly
 * one-to-one to strings in the syslogd(8) source code.  This mapping is
 * included in this file.
 *
 * priorities (these are ordered)
 */
#define	LOG_EMERG	0	/* system is unusable */

/* <sys/syslog.h> */
#define	LOG_ERR		3	/* error conditions */

/* <sys/syslog.h> */
				/* facility of pri */
#define	LOG_FAC(p)	(((p) & LOG_FACMASK) >> 3)

/* <sys/syslog.h> */
#define	LOG_FACMASK	0x03f8	/* mask to extract facility part */

/* <sys/syslog.h> */
#define	LOG_FTP		(11<<3)	/* ftp daemon */

/* <sys/syslog.h> */
#define	LOG_INFO	6	/* informational */

/* <sys/syslog.h> */
/* facility codes */
#define	LOG_KERN	(0<<3)	/* kernel messages */

/* <sys/syslog.h> */
	/* other codes through 15 reserved for system use */
#define	LOG_LOCAL0	(16<<3)	/* reserved for local use */

/* <sys/syslog.h> */
#define	LOG_LOCAL1	(17<<3)	/* reserved for local use */

/* <sys/syslog.h> */
#define	LOG_LOCAL2	(18<<3)	/* reserved for local use */

/* <sys/syslog.h> */
#define	LOG_LOCAL3	(19<<3)	/* reserved for local use */

/* <sys/syslog.h> */
#define	LOG_LOCAL4	(20<<3)	/* reserved for local use */

/* <sys/syslog.h> */
#define	LOG_LOCAL5	(21<<3)	/* reserved for local use */

/* <sys/syslog.h> */
#define	LOG_LOCAL6	(22<<3)	/* reserved for local use */

/* <sys/syslog.h> */
#define	LOG_LOCAL7	(23<<3)	/* reserved for local use */

/* <sys/syslog.h> */
#define	LOG_LPR		(6<<3)	/* line printer subsystem */

/* <sys/syslog.h> */
#define	LOG_MAIL	(2<<3)	/* mail system */

/* <sys/syslog.h> */
#define	LOG_MAKEPRI(fac, pri)	(((fac) << 3) | (pri))

/* <sys/syslog.h> */
/*
 * arguments to setlogmask.
 */
#define	LOG_MASK(pri)	(1 << (pri))		/* mask for one priority */

/* <sys/syslog.h> */
#define	LOG_NDELAY	0x08	/* don't delay open */

/* <sys/syslog.h> */
#define	LOG_NEWS	(7<<3)	/* network news subsystem */

/* <sys/syslog.h> */
#define	LOG_NFACILITIES	24	/* current number of facilities */

/* <sys/syslog.h> */
#define	LOG_NOTICE	5	/* normal but significant condition */

/* <sys/syslog.h> */
#define	LOG_NOWAIT	0x10	/* don't wait for console forks: DEPRECATED */

/* <sys/syslog.h> */
#define	LOG_ODELAY	0x04	/* delay open until first syslog() (default) */

/* <sys/syslog.h> */
#define	LOG_PERROR	0x20	/* log to stderr as well */

/* <sys/syslog.h> */
/*
 * Option flags for openlog.
 *
 * LOG_ODELAY no longer does anything.
 * LOG_NDELAY is the inverse of what it used to be.
 */
#define	LOG_PID		0x01	/* log the pid with each message */

/* <sys/syslog.h> */
				/* extract priority */
#define	LOG_PRI(p)	((p) & LOG_PRIMASK)

/* <sys/syslog.h> */
#define	LOG_PRIMASK	0x07	/* mask to extract priority part (internal) */
				/* extract priority */

/* <sys/syslog.h> */
#define	LOG_SYSLOG	(5<<3)	/* messages generated internally by syslogd */

/* <sys/syslog.h> */
#define	LOG_UPTO(pri)	((1 << ((pri)+1)) - 1)	/* all priorities through pri */

/* <sys/syslog.h> */
#define	LOG_USER	(1<<3)	/* random user-level messages */

/* <sys/syslog.h> */
#define	LOG_UUCP	(8<<3)	/* UUCP subsystem */

/* <sys/syslog.h> */
#define	LOG_WARNING	4	/* warning conditions */

/* <sys/syslog.h> */
#define	_PATH_LOG	"/dev/log"

/* <sys/syslog.h> */
#define _SYS_SYSLOG_H 1

/* <sys/syslog.h> */
#define __need___va_list

/* <sys/syslog.h> */
typedef struct _code {
	char	*c_name;
	int	c_val;
} CODE;

/* <sys/syslog.h> */
typedef struct _code {
	char	*c_name;
	int	c_val;
} CODE;

/* <sys/syslog.h> */
typedef struct _code {
	char	*c_name;
	int	c_val;
} CODE;

/* <sys/syslog.h> */
CODE facilitynames[] =
  {
    { "auth", LOG_AUTH },
    { "authpriv", LOG_AUTHPRIV },
    { "cron", LOG_CRON },
    { "daemon", LOG_DAEMON },
    { "ftp", LOG_FTP },
    { "kern", LOG_KERN },
    { "lpr", LOG_LPR },
    { "mail", LOG_MAIL },
    { "mark", INTERNAL_MARK },		/* INTERNAL */
    { "news", LOG_NEWS },
    { "security", LOG_AUTH },		/* DEPRECATED */
    { "syslog", LOG_SYSLOG },
    { "user", LOG_USER },
    { "uucp", LOG_UUCP },
    { "local0", LOG_LOCAL0 },
    { "local1", LOG_LOCAL1 },
    { "local2", LOG_LOCAL2 },
    { "local3", LOG_LOCAL3 },
    { "local4", LOG_LOCAL4 },
    { "local5", LOG_LOCAL5 },
    { "local6", LOG_LOCAL6 },
    { "local7", LOG_LOCAL7 },
    { NULL, -1 }
  };
#endif

/* <sys/syslog.h> */
CODE prioritynames[] =
  {
    { "alert", LOG_ALERT },
    { "crit", LOG_CRIT },
    { "debug", LOG_DEBUG },
    { "emerg", LOG_EMERG },
    { "err", LOG_ERR },
    { "error", LOG_ERR },		/* DEPRECATED */
    { "info", LOG_INFO },
    { "none", INTERNAL_NOPRI },		/* INTERNAL */
    { "notice", LOG_NOTICE },
    { "panic", LOG_EMERG },		/* DEPRECATED */
    { "warn", LOG_WARNING },		/* DEPRECATED */
    { "warning", LOG_WARNING },
    { NULL, -1 }
  };
#endif
/* <sys/sysmacros.h> ==================================== */

/* <sys/sysmacros.h> */
#define _SYS_SYSMACROS_H	1

/* <sys/sysmacros.h> */
# define major(dev) ((int)(((dev) >> 8) & 0xff))

/* <sys/sysmacros.h> */
#  define major(dev) (((dev).__val[1] >> 8) & 0xff)

/* <sys/sysmacros.h> */
#  define major(dev) (((dev).__val[0] >> 8) & 0xff)

/* <sys/sysmacros.h> */
# define makedev(major, minor) ((((unsigned int) (major)) << 8) \
				| ((unsigned int) (minor)))

/* <sys/sysmacros.h> */
#  define makedev(major, minor) { 0, ((((unsigned int) (major)) << 8) \
				      | ((unsigned int) (minor))) }

/* <sys/sysmacros.h> */
#  define makedev(major, minor) { ((((unsigned int) (major)) << 8) \
				   | ((unsigned int) (minor))), 0 }

/* <sys/sysmacros.h> */
# define minor(dev) ((int)((dev) & 0xff))
# define makedev(major, minor) ((((unsigned int) (major)) << 8) \
				| ((unsigned int) (minor)))

/* <sys/sysmacros.h> */
#  define minor(dev) ((dev).__val[1] & 0xff)
#  define makedev(major, minor) { 0, ((((unsigned int) (major)) << 8) \
				      | ((unsigned int) (minor))) }

/* <sys/sysmacros.h> */
#  define minor(dev) ((dev).__val[0] & 0xff)
#  define makedev(major, minor) { ((((unsigned int) (major)) << 8) \
				   | ((unsigned int) (minor))), 0 }
/* <sys/termios.h> ==================================== */

/* <sys/termios.h> */
#define _SYS_TERMIOS_H
/* <sys/timeb.h> ==================================== */

/* <sys/timeb.h> */
#define _SYS_TIMEB_H	1

/* <sys/timeb.h> */
#define __need_time_t

/* <sys/timeb.h> */
int ftime (struct timeb *__timebuf)
{
}

/* <sys/timeb.h> */
struct timeb
  {
    time_t time;		/* Seconds since epoch, as from `time'.  */
    unsigned short millitm;	/* Additional milliseconds.  */
    short timezone;		/* Minutes west of GMT.  */
    short dstflag;		/* Nonzero if Daylight Savings Time used.  */
  };
/* <sys/time.h> ==================================== */

/* <sys/time.h> */
    /* Timers run when the process is executing and when
       the system is executing on behalf of the process.  */
    ITIMER_PROF = 2
#define ITIMER_PROF ITIMER_PROF

/* <sys/time.h> */
/* Values for the first argument to `getitimer' and `setitimer'.  */
    /* Timers run in real time.  */
    ITIMER_REAL = 0,
#define ITIMER_REAL ITIMER_REAL

/* <sys/time.h> */
    /* Timers run only when the process is executing.  */
    ITIMER_VIRTUAL = 1,
#define ITIMER_VIRTUAL ITIMER_VIRTUAL

/* <sys/time.h> */
/* Macros for converting between `struct timeval' and `struct timespec'.  */
# define TIMEVAL_TO_TIMESPEC(tv, ts)

/* <sys/time.h> */
/* Macros for converting between `struct timeval' and `struct timespec'.  */
# define TIMESPEC_TO_TIMEVAL(tv, ts)

/* <sys/time.h> */
#define _SYS_TIME_H	1

/* <sys/time.h> */
/* Use the nicer parameter type only in GNU mode and not for C++ since the
   strict C++ rules prevent the automatic promotion.  */
typedef enum __itimer_which __itimer_which_t;

/* <sys/time.h> */
typedef int __itimer_which_t;

/* <sys/time.h> */
#define __need_time_t

/* <sys/time.h> */
#define __need_timeval

/* <sys/time.h> */
# define __suseconds_t_defined

/* <sys/time.h> */
typedef struct timezone *__restrict __timezone_ptr_t;

/* <sys/time.h> */
typedef void *__restrict __timezone_ptr_t;

/* <sys/time.h> */
/* Get the current time of day and timezone information,
   putting it into *TV and *TZ.  If TZ is NULL, *TZ is not filled.
   Returns 0 on success, -1 on errors.
   NOTE: This form of timezone information is obsolete.
   Use the functions and variables declared in <time.h> instead.  */
int gettimeofday (struct timeval *__restrict __tv, __timezone_ptr_t __tz)
{
}

/* <sys/time.h> */
/* Type of the second argument to `getitimer' and
   the second and third arguments `setitimer'.  */
struct itimerval
  {

/* <sys/time.h> */
typedef __suseconds_t suseconds_t;

/* <sys/time.h> */
# define timercmp(a, b, CMP) 						      \
  (((a)->tv_sec == (b)->tv_sec) ? 					      \
   ((a)->tv_usec CMP (b)->tv_usec) : 					      \
   ((a)->tv_sec CMP (b)->tv_sec))

# define timersub(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;			      \
    if ((result)->tv_usec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_usec += 1000000;					      \
    }									      \
  } while (0)

/* <sys/time.h> */
# define timerclear(tvp)	((tvp)->tv_sec = (tvp)->tv_usec = 0)
# define timercmp(a, b, CMP) 						      \
  (((a)->tv_sec == (b)->tv_sec) ? 					      \
   ((a)->tv_usec CMP (b)->tv_usec) : 					      \
   ((a)->tv_sec CMP (b)->tv_sec))

# define timeradd(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;			      \
    if ((result)->tv_usec >= 1000000)					      \
      {									      \
	++(result)->tv_sec;						      \
	(result)->tv_usec -= 1000000;					      \
      }									      \
  } while (0)

# define timersub(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;			      \
    if ((result)->tv_usec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_usec += 1000000;					      \
    }									      \
  } while (0)

/* <sys/time.h> */
# define timercmp(a, b, CMP) 						      \
  (((a)->tv_sec == (b)->tv_sec) ? 					      \
   ((a)->tv_usec CMP (b)->tv_usec) : 					      \
   ((a)->tv_sec CMP (b)->tv_sec))

# define timeradd(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;			      \
    if ((result)->tv_usec >= 1000000)					      \
      {									      \
	++(result)->tv_sec;						      \
	(result)->tv_usec -= 1000000;					      \
      }									      \
  } while (0)

# define timersub(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;			      \
    if ((result)->tv_usec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_usec += 1000000;					      \
    }									      \
  } while (0)

/* <sys/time.h> */
/* Convenience macros for operations on timevals.
   NOTE: `timercmp' does not work for >= or <=.  */
# define timerisset(tvp)	((tvp)->tv_sec || (tvp)->tv_usec)

/* <sys/time.h> */
# define timercmp(a, b, CMP) 						      \
  (((a)->tv_sec == (b)->tv_sec) ? 					      \
   ((a)->tv_usec CMP (b)->tv_usec) : 					      \
   ((a)->tv_sec CMP (b)->tv_sec))

# define timeradd(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec + (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec + (b)->tv_usec;			      \
    if ((result)->tv_usec >= 1000000)					      \
      {									      \
	++(result)->tv_sec;						      \
	(result)->tv_usec -= 1000000;					      \
      }									      \
  } while (0)

# define timersub(a, b, result)						      \
  do {									      \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;			      \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;			      \
    if ((result)->tv_usec < 0) {					      \
      --(result)->tv_sec;						      \
      (result)->tv_usec += 1000000;					      \
    }									      \
  } while (0)

/* <sys/time.h> */
/* Structure crudely representing a timezone.
   This is obsolete and should never be used.  */
struct timezone
  {
    int tz_minuteswest;		/* Minutes west of GMT.  */
    int tz_dsttime;		/* Nonzero if DST is ever in effect.  */
  };
/* <sys/times.h> ==================================== */

/* <sys/times.h> */
#define	_SYS_TIMES_H	1

/* <sys/times.h> */
#define	__need_clock_t

/* <sys/times.h> */
/* Structure describing CPU time used by a process and its children.  */
struct tms
  {
    clock_t tms_utime;		/* User CPU time.  */
    clock_t tms_stime;		/* System CPU time.  */
/* <sys/timex.h> ==================================== */

/* <sys/timex.h> */
#define ADJ_ESTERROR		0x0008	/* estimated time error */

/* <sys/timex.h> */
#define ADJ_FREQUENCY		0x0002	/* frequency offset */

/* <sys/timex.h> */
#define ADJ_MAXERROR		0x0004	/* maximum time error */

/* <sys/timex.h> */
/* Mode codes (timex.mode) */
#define ADJ_OFFSET		0x0001	/* time offset */

/* <sys/timex.h> */
#define ADJ_OFFSET_SINGLESHOT	0x8001	/* old-fashioned adjtime */

/* <sys/timex.h> */
#define ADJ_STATUS		0x0010	/* clock status */

/* <sys/timex.h> */
#define ADJ_TICK		0x4000	/* tick value */

/* <sys/timex.h> */
#define ADJ_TIMECONST		0x0020	/* pll time constant */

/* <sys/timex.h> */
/* Maximum time constant of the PLL.  */
#define MAXTC		6

/* <sys/timex.h> */
#define MOD_CLKA	ADJ_OFFSET_SINGLESHOT /* 0x8000 in original */

/* <sys/timex.h> */
#define MOD_CLKB	ADJ_TICK

/* <sys/timex.h> */
#define MOD_ESTERROR	ADJ_ESTERROR

/* <sys/timex.h> */
#define MOD_FREQUENCY	ADJ_FREQUENCY

/* <sys/timex.h> */
#define MOD_MAXERROR	ADJ_MAXERROR

/* <sys/timex.h> */
/* xntp 3.4 compatibility names */
#define MOD_OFFSET	ADJ_OFFSET

/* <sys/timex.h> */
#define MOD_STATUS	ADJ_STATUS

/* <sys/timex.h> */
#define MOD_TIMECONST	ADJ_TIMECONST

/* <sys/timex.h> */
#define STA_CLOCKERR	0x1000	/* clock hardware fault (ro) */

/* <sys/timex.h> */
#define STA_DEL		0x0020	/* delete leap (rw) */

/* <sys/timex.h> */
#define STA_FLL		0x0008	/* select frequency-lock mode (rw) */

/* <sys/timex.h> */
#define STA_FREQHOLD	0x0080	/* hold frequency (rw) */

/* <sys/timex.h> */
#define STA_INS		0x0010	/* insert leap (rw) */

/* <sys/timex.h> */
/* Status codes (timex.status) */
#define STA_PLL		0x0001	/* enable PLL updates (rw) */

/* <sys/timex.h> */
#define STA_PPSERROR	0x0800	/* PPS signal calibration error (ro) */

/* <sys/timex.h> */
#define STA_PPSFREQ	0x0002	/* enable PPS freq discipline (rw) */

/* <sys/timex.h> */
#define STA_PPSJITTER	0x0200	/* PPS signal jitter exceeded (ro) */

/* <sys/timex.h> */
#define STA_PPSSIGNAL	0x0100	/* PPS signal present (ro) */

/* <sys/timex.h> */
#define STA_PPSTIME	0x0004	/* enable PPS time discipline (rw) */

/* <sys/timex.h> */
#define STA_PPSWANDER	0x0400	/* PPS signal wander exceeded (ro) */

/* <sys/timex.h>  Read-only bits */
#define STA_RONLY (STA_PPSSIGNAL | STA_PPSJITTER | STA_PPSWANDER | STA_PPSERROR | STA_CLOCKERR)

/* <sys/timex.h> */
#define STA_UNSYNC	0x0040	/* clock unsynchronized (rw) */

/* <sys/timex.h> */
#define TIME_BAD	TIME_ERROR /* bw compat */

/* <sys/timex.h> */
#define TIME_DEL	2	/* delete leap second */

/* <sys/timex.h> */
#define TIME_ERROR	5	/* clock not synchronized */

/* <sys/timex.h> */
#define TIME_INS	1	/* insert leap second */

/* <sys/timex.h> */
/* Clock states (time_state) */
#define TIME_OK		0	/* clock synchronized, no leap second */

/* <sys/timex.h> */
#define TIME_OOP	3	/* leap second in progress */

/* <sys/timex.h> */
#define TIME_WAIT	4	/* leap second has occurred */

/* <sys/timex.h> */
#define	_SYS_TIMEX_H	1

/* <sys/timex.h> */
struct ntptimeval

  struct timeval time;	/* current time (ro) */
  long maxerror;	/* maximum error (us) (ro) */
  long esterror;	/* estimated error (us) (ro) */
};

/* <sys/timex.h> */
struct timex

  unsigned int modes;	/* mode selector */
  long offset;	/* time offset (usec) */
  long freq;	/* frequency offset (scaled ppm) */
  long maxerror;	/* maximum error (usec) */
  long esterror;	/* estimated error (usec) */
  int status;		/* clock command/status */
  long constant;	/* pll time constant */
  long precision;	/* clock precision (usec) (read only) */
  long tolerance;	/* clock frequency tolerance (ppm) (read only) */
  struct timeval time;	/* (read only) */
  long tick;	/* (modified) usecs between clock ticks */
/* <sys/ttychars.h> ==================================== */

/* <sys/ttychars.h> */
#define	_SYS_TTYCHARS_H 1

/* <sys/ttychars.h> */
struct ttychars {
	char	tc_erase;	/* erase last character */
	char	tc_kill;	/* erase entire line */
	char	tc_intrc;	/* interrupt */
	char	tc_quitc;	/* quit */
	char	tc_startc;	/* start output */
	char	tc_stopc;	/* stop output */
	char	tc_eofc;	/* end-of-file */
	char	tc_brkc;	/* input delimiter (like nl) */
	char	tc_suspc;	/* stop process signal */
	char	tc_dsuspc;	/* delayed stop process signal */
	char	tc_rprntc;	/* reprint line */
	char	tc_flushc;	/* flush output (toggles) */
	char	tc_werasc;	/* word erase */
	char	tc_lnextc;	/* literal next character */
};
/* <sys/ttydefaults.h> ==================================== */

/* <sys/ttydefaults.h> */
/* compat */
#define	CBRK		CEOL

/* <sys/ttydefaults.h> */
#define	CDISCARD 	CTRL('o')

/* <sys/ttydefaults.h> */
#define	CDSUSP		CTRL('y')

/* <sys/ttydefaults.h> */
#define	CEOF		CTRL('d')

/* <sys/ttydefaults.h> */
# define CEOL		_POSIX_VDISABLE

/* <sys/ttydefaults.h> */
# define CEOL		'\0'		/* XXX avoid _POSIX_VDISABLE */

/* <sys/ttydefaults.h> */
#define	CEOT		CEOF

/* <sys/ttydefaults.h> */
#define	CERASE		0177

/* <sys/ttydefaults.h> */
#define	CFLUSH		CDISCARD

/* <sys/ttydefaults.h> */
#define	CINTR		CTRL('c')

/* <sys/ttydefaults.h> */
#define	CKILL		CTRL('u')

/* <sys/ttydefaults.h> */
#define	CLNEXT		CTRL('v')

/* <sys/ttydefaults.h> */
#define	CMIN		1

/* <sys/ttydefaults.h> */
#define	CQUIT		034		/* FS, ^\ */

/* <sys/ttydefaults.h> */
#define	CREPRINT 	CTRL('r')

/* <sys/ttydefaults.h> */
#define CRPRNT		CREPRINT

/* <sys/ttydefaults.h> */
#define	CSTART		CTRL('q')

/* <sys/ttydefaults.h> */
# define CSTATUS	_POSIX_VDISABLE

/* <sys/ttydefaults.h> */
# define CSTATUS	'\0'		/* XXX avoid _POSIX_VDISABLE */

/* <sys/ttydefaults.h> */
#define	CSTOP		CTRL('s')

/* <sys/ttydefaults.h> */
#define	CSUSP		CTRL('z')

/* <sys/ttydefaults.h> */
#define	CTIME		0

/* <sys/ttydefaults.h> */
/*
 * Control Character Defaults
 */
#define CTRL(x)	(x&037)

/* <sys/ttydefaults.h> */
#define	CWERASE 	CTRL('w')

/* <sys/ttydefaults.h> */
#define TTYDEF_CFLAG	(CREAD | CS7 | PARENB | HUPCL)

/* <sys/ttydefaults.h> */
/*
 * Defaults on "first" open.
 */
#define	TTYDEF_IFLAG	(BRKINT | ISTRIP | ICRNL | IMAXBEL | IXON | IXANY)

/* <sys/ttydefaults.h> */
#define TTYDEF_LFLAG	(ECHO | ICANON | ISIG | IEXTEN | ECHOE|ECHOKE|ECHOCTL)

/* <sys/ttydefaults.h> */
#define TTYDEF_OFLAG	(OPOST | ONLCR | XTABS)

/* <sys/ttydefaults.h> */
#define TTYDEF_SPEED	(B9600)

/* <sys/ttydefaults.h> */
#define	_SYS_TTYDEFAULTS_H_

/* <sys/ttydefaults.h> */
cc_t	ttydefchars[NCCS] = {
	CEOF,	CEOL,	CEOL,	CERASE, CWERASE, CKILL, CREPRINT,
	_POSIX_VDISABLE, CINTR,	CQUIT,	CSUSP,	CDSUSP,	CSTART,	CSTOP,	CLNEXT,
	CDISCARD, CMIN,	CTIME,  CSTATUS, _POSIX_VDISABLE
};
/* <sys/types.h> ==================================== */

/* <sys/types.h> */
#define	_SYS_TYPES_H	1

/* <sys/types.h> */
#define __BIT_TYPES_DEFINED__	1

/* <sys/types.h> */
typedef int register_t ;

/* <sys/types.h> */
#  define __blkcnt_t_defined

/* <sys/types.h> */
#  define __blkcnt_t_defined

/* <sys/types.h> */
# define __blksize_t_defined

/* <sys/types.h> */
#  define __daddr_t_defined

/* <sys/types.h> */
# define __dev_t_defined

/* <sys/types.h> */
#  define __fsblkcnt_t_defined

/* <sys/types.h> */
#  define __fsblkcnt_t_defined

/* <sys/types.h> */
#  define __fsfilcnt_t_defined

/* <sys/types.h> */
#  define __fsfilcnt_t_defined

/* <sys/types.h> */
# define __gid_t_defined

/* <sys/types.h> */
# define __id_t_defined

/* <sys/types.h> */
# define __ino64_t_defined

/* <sys/types.h> */
# define __ino_t_defined

/* <sys/types.h> */
#  define __int8_t_defined

/* <sys/types.h> */
#  define __int8_t_defined

/* <sys/types.h> */
/* For GCC 2.7 and later, we can use specific type-size attributes.  */
# define __intN_t(N, MODE) \
  typedef int int##N##_t 
# define __u_intN_t(N, MODE) \
  typedef unsigned int u_int##N##_t 

/* <sys/types.h> */
__intN_t (8, __QI__)
{
}

/* <sys/types.h> */
__intN_t (16, __HI__)
{
}

/* <sys/types.h> */
__intN_t (32, __SI__)
{
}

/* <sys/types.h> */
__intN_t (64, __DI__)
{
}

/* <sys/types.h> */
# define __key_t_defined

/* <sys/types.h> */
# define __mode_t_defined

/* <sys/types.h> */
# define __need_clock_t

/* <sys/types.h> */
#define __need_clockid_t

/* <sys/types.h> */
#define	__need_size_t

/* <sys/types.h> */
#define	__need_time_t

/* <sys/types.h> */
#define __need_timer_t

/* <sys/types.h> */
# define __nlink_t_defined

/* <sys/types.h> */
# define __off64_t_defined

/* <sys/types.h> */
# define __off_t_defined

/* <sys/types.h> */
# define __pid_t_defined

/* <sys/types.h> */
# define __ssize_t_defined

/* <sys/types.h> */
#  define __suseconds_t_defined

/* <sys/types.h> */
#  define __u_char_defined

/* <sys/types.h> */
/* For GCC 2.7 and later, we can use specific type-size attributes.  */
# define __intN_t(N, MODE) \
  typedef int int##N##_t 
# define __u_intN_t(N, MODE) \
  typedef unsigned int u_int##N##_t 

/* <sys/types.h> */
__u_intN_t (8, __QI__)

/* <sys/types.h> */
__u_intN_t (16, __HI__)

/* <sys/types.h> */
__u_intN_t (32, __SI__)

/* <sys/types.h> */
__u_intN_t (64, __DI__)

/* <sys/types.h> */
# define __uid_t_defined

/* <sys/types.h> */
#  define __useconds_t_defined

/* <sys/types.h> */
typedef __blkcnt64_t blkcnt64_t;     /* Type to count number of disk blocks. */

/* <sys/types.h> */
typedef __blkcnt_t blkcnt_t;	 /* Type to count number of disk blocks.  */

/* <sys/types.h> */
typedef __blkcnt64_t blkcnt_t;	   /* Type to count number of disk blocks.  */

/* <sys/types.h> */
typedef __blksize_t blksize_t;

/* <sys/types.h> */
typedef __caddr_t caddr_t;

/* <sys/types.h> */
typedef __daddr_t daddr_t;

/* <sys/types.h> */
typedef __dev_t dev_t;

/* <sys/types.h> */
typedef __fsblkcnt64_t fsblkcnt64_t; /* Type to count file system blocks.  */

/* <sys/types.h> */
typedef __fsblkcnt_t fsblkcnt_t; /* Type to count file system blocks.  */

/* <sys/types.h> */
typedef __fsblkcnt64_t fsblkcnt_t; /* Type to count file system blocks.  */

/* <sys/types.h> */
typedef __fsfilcnt64_t fsfilcnt64_t; /* Type to count file system inodes.  */

/* <sys/types.h> */
typedef __fsfilcnt_t fsfilcnt_t; /* Type to count file system inodes.  */

/* <sys/types.h> */
typedef __fsfilcnt64_t fsfilcnt_t; /* Type to count file system inodes.  */

/* <sys/types.h> */
typedef __fsid_t fsid_t;

/* <sys/types.h> */
typedef __gid_t gid_t;

/* <sys/types.h> */
typedef __id_t id_t;

/* <sys/types.h> */
typedef __ino64_t ino64_t;

/* <sys/types.h> */
typedef __ino_t ino_t;

/* <sys/types.h> */
typedef __ino64_t ino_t;

/* <sys/types.h> */
typedef	short int16_t;

/* <sys/types.h> */
typedef	int int32_t;

/* <sys/types.h> */
__extension__ typedef long long int64_t;

/* <sys/types.h> */
typedef	char int8_t;

/* <sys/types.h> */
typedef __key_t key_t;

/* <sys/types.h> */
typedef __loff_t loff_t;

/* <sys/types.h> */
typedef __mode_t mode_t;

/* <sys/types.h> */
typedef __nlink_t nlink_t;

/* <sys/types.h> */
typedef __off64_t off64_t;

/* <sys/types.h> */
typedef __off_t off_t;

/* <sys/types.h> */
typedef __off64_t off_t;

/* <sys/types.h> */
typedef __pid_t pid_t;

/* <sys/types.h> */
typedef __quad_t quad_t;

/* <sys/types.h> */
typedef int register_t;

/* <sys/types.h> */
typedef __ssize_t ssize_t;

/* <sys/types.h> */
typedef __suseconds_t suseconds_t;

/* <sys/types.h> */
typedef __u_char u_char;

/* <sys/types.h> */
typedef __u_int u_int;

/* <sys/types.h> */
typedef	unsigned short u_int16_t;

/* <sys/types.h> */
typedef	unsigned int u_int32_t;

/* <sys/types.h> */
__extension__ typedef unsigned long long u_int64_t;

/* <sys/types.h> */
/* But these were defined by ISO C without the first `_'.  */
typedef	unsigned char u_int8_t;

/* <sys/types.h> */
typedef __u_long u_long;

/* <sys/types.h> */
typedef __u_quad_t u_quad_t;

/* <sys/types.h> */
typedef __u_short u_short;

/* <sys/types.h> */
typedef __uid_t uid_t;

/* <sys/types.h> */
typedef unsigned int uint;

/* <sys/types.h> */
/* Old compatibility names for C types.  */
typedef unsigned long ulong;

/* <sys/types.h> */
typedef __useconds_t useconds_t;

/* <sys/types.h> */
typedef unsigned short ushort;
/* <sys/ucontext.h> ==================================== */

/* <sys/ucontext.h> */
/* Number of general registers.  */
#define NGREG	19

/* <sys/ucontext.h> */
  REG_CS,
# define REG_CS		REG_CS

/* <sys/ucontext.h> */
  REG_DS,
# define REG_DS		REG_DS

/* <sys/ucontext.h> */
  REG_EAX,
# define REG_EAX	REG_EAX

/* <sys/ucontext.h> */
  REG_EBP,
# define REG_EBP	REG_EBP

/* <sys/ucontext.h> */
  REG_EBX,
# define REG_EBX	REG_EBX

/* <sys/ucontext.h> */
  REG_ECX,
# define REG_ECX	REG_ECX

/* <sys/ucontext.h> */
  REG_EDI,
# define REG_EDI	REG_EDI

/* <sys/ucontext.h> */
  REG_EDX,
# define REG_EDX	REG_EDX

/* <sys/ucontext.h> */
  REG_EFL,
# define REG_EFL	REG_EFL

/* <sys/ucontext.h> */
  REG_EIP,
# define REG_EIP	REG_EIP

/* <sys/ucontext.h> */
  REG_ERR,
# define REG_ERR	REG_ERR

/* <sys/ucontext.h> */
  REG_ES,
# define REG_ES		REG_ES

/* <sys/ucontext.h> */
  REG_ESI,
# define REG_ESI	REG_ESI

/* <sys/ucontext.h> */
  REG_ESP,
# define REG_ESP	REG_ESP

/* <sys/ucontext.h> */
  REG_FS,
# define REG_FS		REG_FS

/* <sys/ucontext.h> */
/* Number of each register is the `gregset_t' array.  */
enum

  REG_GS = 0,
# define REG_GS		REG_GS

/* <sys/ucontext.h> */
  REG_SS
# define REG_SS	REG_SS

/* <sys/ucontext.h> */
  REG_TRAPNO,
# define REG_TRAPNO	REG_TRAPNO

/* <sys/ucontext.h> */
  REG_UESP,
# define REG_UESP	REG_UESP

/* <sys/ucontext.h> */
#define _SYS_UCONTEXT_H	1

/* <sys/ucontext.h> */
/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  } ucontext_t;

/* <sys/ucontext.h> */
/* Definitions taken from the kernel headers.  */
struct _libc_fpreg

  unsigned short significand[4];
  unsigned short exponent;
};

/* <sys/ucontext.h> */
struct _libc_fpstate

  unsigned long cw;
  unsigned long sw;
  unsigned long tag;
  unsigned long ipoff;
  unsigned long cssel;
  unsigned long dataoff;
  unsigned long datasel;
  struct _libc_fpreg _st[8];
  unsigned long status;
};

/* <sys/ucontext.h> */
/* Context to describe whole processor state.  */
    /* Due to Linux's history we have to use a pointer here.  The SysV/i386
       ABI requires a struct with the values.  */
    fpregset_t fpregs;
    unsigned long oldmask;
    unsigned long cr2;
  } mcontext_t;

/* <sys/ucontext.h> */
/* Context to describe whole processor state.  */
    /* Due to Linux's history we have to use a pointer here.  The SysV/i386
       ABI requires a struct with the values.  */
    fpregset_t fpregs;
    unsigned long oldmask;
    unsigned long cr2;
  } mcontext_t;

/* <sys/ucontext.h> */
/* Structure to describe FPU registers.  */
typedef struct _libc_fpstate *fpregset_t;

/* <sys/ucontext.h> */
/* Type for general register.  */
typedef int greg_t;

/* <sys/ucontext.h> */
/* Context to describe whole processor state.  */
    /* Due to Linux's history we have to use a pointer here.  The SysV/i386
       ABI requires a struct with the values.  */
    fpregset_t fpregs;
    unsigned long oldmask;
    unsigned long cr2;
  } mcontext_t;

/* <sys/ucontext.h> */
/* Container for all general registers.  */
typedef greg_t gregset_t[NGREG];

/* <sys/ucontext.h> */
/* Context to describe whole processor state.  */
    /* Due to Linux's history we have to use a pointer here.  The SysV/i386
       ABI requires a struct with the values.  */
    fpregset_t fpregs;
    unsigned long oldmask;
    unsigned long cr2;
  } mcontext_t;

/* <sys/ucontext.h> */
/* Context to describe whole processor state.  */
    /* Due to Linux's history we have to use a pointer here.  The SysV/i386
       ABI requires a struct with the values.  */
    fpregset_t fpregs;
    unsigned long oldmask;
    unsigned long cr2;
  } mcontext_t;

/* <sys/ucontext.h> */
/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  } ucontext_t;

/* <sys/ucontext.h> */
/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  } ucontext_t;

/* <sys/ucontext.h> */
/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  } ucontext_t;

/* <sys/ucontext.h> */
/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  } ucontext_t;

/* <sys/ucontext.h> */
/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  } ucontext_t;

/* <sys/ucontext.h> */
/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  } ucontext_t;

/* <sys/ucontext.h> */
/* Userlevel context.  */
typedef struct ucontext
  {
    unsigned long uc_flags;
    struct ucontext *uc_link;
    stack_t uc_stack;
    mcontext_t uc_mcontext;
    __sigset_t uc_sigmask;
    struct _libc_fpstate __fpregs_mem;
  } ucontext_t;
/* <sys/uio.h> ==================================== */

/* <sys/uio.h> */
#define _SYS_UIO_H	1
/* <sys/ultrasound.h> ==================================== */

/* <sys/un.h> ==================================== */

/* <sys/un.h> */
/* Evaluate to actual length of the `sockaddr_un' structure.  */
# define SUN_LEN(ptr) ((size_t) (((struct sockaddr_un *) 0)->sun_path)	      \
		      + strlen ((ptr)->sun_path))

/* <sys/un.h> */
#define	_SYS_UN_H	1

/* <sys/un.h> */
/* Structure describing the address of an AF_LOCAL (aka AF_UNIX) socket.  */
struct sockaddr_un
  {
    __SOCKADDR_COMMON (sun_)

    char sun_path[108];		/* Path name.  */
  };
/* <sys/unistd.h> ==================================== */

/* <sys/user.h> ==================================== */

/* <sys/user.h> */
#define HOST_STACK_END_ADDR	(u.start_stack + u.u_ssize * NBPG)

/* <sys/user.h> */
#define HOST_TEXT_START_ADDR	(u.start_code)

/* <sys/user.h> */
#define NBPG			PAGE_SIZE

/* <sys/user.h> */
#define PAGE_MASK		(~(PAGE_SIZE-1))

/* <sys/user.h> */
#define PAGE_SHIFT		12

/* <sys/user.h> */
#define PAGE_SIZE		(1UL << PAGE_SHIFT)

/* <sys/user.h> */
#define UPAGES			1

/* <sys/user.h> */
#define _SYS_USER_H	1

/* <sys/user.h> */
struct user

  struct user_regs_struct	regs;
  int				u_fpvalid;
  struct user_fpregs_struct	i387;
  unsigned long		u_tsize;
  unsigned long		u_dsize;
  unsigned long		u_ssize;
  unsigned long			start_code;
  unsigned long			start_stack;
  long			signal;
  int				reserved;
  struct user_regs_struct*	u_ar0;
  struct user_fpregs_struct*	u_fpstate;
  unsigned long		magic;
  char				u_comm [32];
  int				u_debugreg [8];
};

/* <sys/user.h> */
struct user_fpregs_struct

  long cwd;
  long swd;
  long twd;
  long fip;
  long fcs;
  long foo;
  long fos;
  long st_space [20];
};

/* <sys/user.h> */
struct user_fpxregs_struct

  unsigned short cwd;
  unsigned short swd;
  unsigned short twd;
  unsigned short fop;
  long fip;
  long fcs;
  long foo;
  long fos;
  long mxcsr;
  long reserved;
  long st_space[32];   /* 8*16 bytes for each FP-reg = 128 bytes */
  long xmm_space[32];  /* 8*16 bytes for each XMM-reg = 128 bytes */
  long padding[56];
};

/* <sys/user.h> */
struct user_regs_struct

  long ebx;
  long ecx;
  long edx;
  long esi;
  long edi;
  long ebp;
  long eax;
  long xds;
  long xes;
  long xfs;
  long xgs;
  long orig_eax;
  long eip;
  long xcs;
  long eflags;
  long esp;
  long xss;
};
/* <sys/ustat.h> ==================================== */

/* <sys/ustat.h> */
#define	_SYS_USTAT_H	1
/* <sys/utsname.h> ==================================== */

/* <sys/utsname.h> */
# define SYS_NMLN  _UTSNAME_LENGTH

/* <sys/utsname.h> */
#define	_SYS_UTSNAME_H	1

/* <sys/utsname.h> */
# define _UTSNAME_NODENAME_LENGTH _UTSNAME_LENGTH

/* <sys/utsname.h> */
/* Structure describing the system and machine.  */
struct utsname
  {
/* <sys/vfs.h> ==================================== */

/* <sys/vlimit.h> ==================================== */

/* <sys/vlimit.h> */
/* This means no limit.  */
#define INFINITY 0x7fffffff

/* <sys/vlimit.h> */
#define _SYS_VLIMIT_H	1

/* <sys/vlimit.h> */
/* Kinds of resource limit.  */
enum __vlimit_resource

/* <sys/vm86.h> ==================================== */

/* <sys/vm86.h> */
#define _SYS_VM86_H	1
/* <sys/vt.h> ==================================== */

/* <sys/vtimes.h> ==================================== */

/* <sys/vtimes.h> */
/* Granularity of the `vm_utime' and `vm_stime' fields of a `struct vtimes'.
   (This is the frequency of the machine's power supply, in Hz.)  */
#define	VTIMES_UNITS_PER_SECOND	60

/* <sys/vtimes.h> */
#define _SYS_VTIMES_H	1

/* <sys/vtimes.h> */
struct vtimes

/* <sys/wait.h> ==================================== */

/* <sys/wait.h> */
typedef enum

  P_ALL,		/* Wait for any child.  */
  P_PID,		/* Wait for specified process.  */
  P_PGID		/* Wait for members of process group.  */
} idtype_t;

/* <sys/wait.h> */
typedef enum

  P_ALL,		/* Wait for any child.  */
  P_PID,		/* Wait for specified process.  */
  P_PGID		/* Wait for members of process group.  */
} idtype_t;

/* <sys/wait.h> */
/* Special values for the PID argument to `waitpid' and `wait4'.  */
# define WAIT_ANY	(-1)	/* Any process.  */

/* <sys/wait.h> */
# define WAIT_MYPGRP	0	/* Any process in my process group.  */

/* <sys/wait.h> */
# define WCOREDUMP(status)	__WCOREDUMP(__WAIT_INT(status))

/* <sys/wait.h> */
# define WCOREFLAG		__WCOREFLAG

/* <sys/wait.h> */
# define WEXITSTATUS(status)	__WEXITSTATUS(__WAIT_INT(status))

/* <sys/wait.h> */
# define WIFEXITED(status)	__WIFEXITED(__WAIT_INT(status))

/* <sys/wait.h> */
# define WIFSIGNALED(status)	__WIFSIGNALED(__WAIT_INT(status))

/* <sys/wait.h> */
# define WIFSTOPPED(status)	__WIFSTOPPED(__WAIT_INT(status))

/* <sys/wait.h> */
# define WSTOPSIG(status)	__WSTOPSIG(__WAIT_INT(status))

/* <sys/wait.h> */
# define WTERMSIG(status)	__WTERMSIG(__WAIT_INT(status))

/* <sys/wait.h> */
# define W_EXITCODE(ret, sig)	__W_EXITCODE(ret, sig)

/* <sys/wait.h> */
# define W_STOPCODE(sig)	__W_STOPCODE(sig)

/* <sys/wait.h> */
#define	_SYS_WAIT_H	1

/* <sys/wait.h> */
#   define __WAIT_INT(status)						      \
  (__extension__ ({ union { __typeof(status) __in; int __i; } __u;	      \
		    __u.__in = (status); __u.__i; }))

/* <sys/wait.h> */
#   define __WAIT_INT(status)	(*(int *) &(status))

/* <sys/wait.h> */
#  define __WAIT_INT(status)	(status)

/* <sys/wait.h> */
#   define __WAIT_STATUS	void *

/* <sys/wait.h> */
#  define __WAIT_STATUS		int *

/* <sys/wait.h> */
#   define __WAIT_STATUS_DEFN	void *

/* <sys/wait.h> */
/* This works in GCC 2.6.1 and later.  */
typedef union
  {
    union wait *__uptr;
    int *__iptr;
  } __WAIT_STATUS ;
#   define __WAIT_STATUS_DEFN	int *

/* <sys/wait.h> */
#  define __WAIT_STATUS_DEFN	int *

/* <sys/wait.h> */
/* This works in GCC 2.6.1 and later.  */
typedef union
  {
    union wait *__uptr;
    int *__iptr;
  } __WAIT_STATUS ;

/* <sys/wait.h> */
# define __need_siginfo_t

/* <sys/wait.h> */
typedef enum

  P_ALL,		/* Wait for any child.  */
  P_PID,		/* Wait for specified process.  */
  P_PGID		/* Wait for members of process group.  */
} idtype_t;

/* <sys/wait.h> */
typedef enum

  P_ALL,		/* Wait for any child.  */
  P_PID,		/* Wait for specified process.  */
  P_PGID		/* Wait for members of process group.  */
} idtype_t;
