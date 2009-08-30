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
** definitions for cross platform dependencies
*/

#ifndef __COMPAT_H__
#define __COMPAT_H__

#if defined(__MINGW32__)
    /*
    ** stdio.h
    */
    #define snprintf _snprintf

    /*
    ** limits.h
    */
    
    #define _POSIX_PATH_MAX 255
    #define NGROUPS_MAX		8
    #define SORT_LINE_MAX	20480
    #define LINE_MAX		2048
    #define NAME_MAX		48

	#include <stdint.h>

   /*
    ** sys/types.h
    */

    #include <sys/types.h>
	
    #ifndef uid_t
    typedef int uid_t;
    #endif
    #ifndef gid_t
    typedef int gid_t;
    #endif

    typedef int nid_t;

    /*
    ** errno.h
    */
    
    #define EOK 0

    /*
    ** err.h
    */
    #define errx(err, ...) do { fprintf(stderr, "split: "); fprintf(stderr, __VA_ARGS__); exit(err); } while (0)
    #define err(err, ...) do { fprintf(stderr, "split: "); fprintf(stderr, __VA_ARGS__); fprintf(stderr, "%s", strerror(errno)) ; exit(err); } while (0)
    
   /*
    ** unix.h
    */
	#ifndef MAXPATHLEN
	#define MAXPATHLEN      _POSIX_PATH_MAX
	#endif

   /*
    ** wait.h
    */
    
    #define WIFEXITED(stat_val)   (((stat_val) & 0x7f) == 0)
    #define WIFSIGNALED(stat_val) (((stat_val) & 0x7f) != 0)
    #define WIFSTOPPED(stat_val)  (0)
    #define WEXITSTATUS(stat_val) (((stat_val) >> 8) & 0xff)
    #define WTERMSIG(stat_val)    (((stat_val) & 0x7f))
    #define WSTOPSIG(stat_val)    (0)
    
   /*
    ** stdlib.h
    */
	#if !defined(max)
	#define max(a,b)  (((a) > (b)) ? (a) : (b))
	#endif
	#if !defined(min) && !defined(__cplusplus)
	#define min(a,b)  (((a) < (b)) ? (a) : (b))
	#endif

	#define sleep _sleep;

	#define strlcpy strncpy    
    /*
    ** fnmatch.h
    */

    #define FNM_PATHNAME 001
    #define FNM_PERIOD   002
    #define FNM_NOESCAPE 004

    #define FNM_NOMATCH  1

    extern int fnmatch(const char *__expr, const char *__str, int __flags);

    /*
    ** unistd.h
    */
    
    extern char     *basename( const char * fname );
    extern char     *dirname(const char *path);
    extern int setenv(const char *var, const char *val, int overwrite);

    int 		fchmod(int fd, mode_t mode);

	#define ftruncate(fd, size)		chsize(fd, size)
    #define getgid() 0
    #define getnid() 0
    #define getuid() 0
    #define geteuid() 0
    #define getpwuid( uid ) (NULL)
    #define getgrgid( uid ) (NULL)
	#define getpwnam( name ) (NULL)
	#define getgrnam( name ) (NULL)
    #define major( d ) 0
    #define minor( d ) 0
    #define chown( fn, uid, gid ) (0)
    #define lchown( fn, uid, gid ) (0)
	#define readlink(a, b, c) (-1)
	#define makedev(node,major,minor)       ((dev_t)(((node) << 16) | ((major) << 10) | (minor)))

	#define link(a, b) (-1)

	extern int symlink(char *src, char* dest);

	#define mknod(path, mode, dev) (-1)
	#define mkfifo(path, mode) (-1)


	/*
	 **  pwd.h       Password operations
	 */
	
	struct passwd {
	    char  *pw_name;     /*  User's login name           */
	    char  *pw_passwd;
	    uid_t  pw_uid;      /*  User ID number              */
	    gid_t  pw_gid;      /*  Group ID number             */
	    char  *pw_age;
	    char  *pw_comment;
	    char  *pw_gecos;
	    char  *pw_dir;      /*  Initial Working Directory   */
	    char  *pw_shell;    /*  Initial User Program        */
	};

	/*
	 **  grp.h
	 */

	struct group {
	    char  *gr_name;
	    char  *gr_passwd;
	    gid_t  gr_gid;
	    char **gr_mem;
	};

	/*
	** misc
	*/
	
    #define FN_CMP( n1, n2 )	stricmp( n1, n2 )

    #define PATHSEP_STR	";"
    #define PATHSEP_CHR	';'
	#define DIRSEP_STR 			"\\"
	#define DIRSEP_CHAR			'\\'

    #define IS_DIRSEP( c )	( (c) == '/' || (c) == '\\' || (c) == ':' )
    #define IS_ABSPATH( n )	((n)[0] == '/' 		\
							|| (n)[0] == '\\'	\
							|| (isalpha((n)[0]) && (n)[1] == ':'))

    int		__has_path( const char *name );
    char* __first_dirsep( const char* path );
    char* __last_dirsep( const char* path );
    
	#define HAS_PATH( n )				__has_path( n )
	#define FIRST_DIRSEP( n )		__first_dirsep( n )
	#define LAST_DIRSEP( n )			__last_dirsep( n )

    /*
    ** sys/stat.h
    */

    #define lstat( path, buff )		stat( path, buff )
    
     /* typedefs from stat.h, to make it compliant with posix types: */
     typedef short nlink_t;
     typedef unsigned long ulong_t;

    
    #define S_ISLNK(m)  0	/*(((m)&_S_IFMT)==_S_IFLNK) Test for symbolic link.    */
    #define S_ISNAM(m)  0	/*(((m)&_S_IFMT)==_S_IFNAM) Test for special named file*/
    #define S_ISSOCK(m) 0	/*(((m)&_S_IFMT)==_S_IFSOCK) Test for socket.           */

    /*
    ** Win32 does not have links. Use these value for compatibility.
    */

    #define S_IFLNK     0xA000
    #define S_IFNAM     0x5000
    #define S_IFSOCK    0xC000

	#define S_ISUID     004000              /* set user id on execution         */
	#define S_ISGID     002000              /* set group id on execution        */
	#define S_ISVTX     001000              /* sticky bit (does nothing yet)    */
	
	/*
	 *  Group permissions
	 */
	#define S_IRWXG     000070              /*  Read, write, execute/search     */
	#define S_IRGRP     000040              /*  Read permission                 */
	#define S_IWGRP     000020              /*  Write permission                */
	#define S_IXGRP     000010              /*  Execute/search permission       */
	
	/*
	 *  Other permissions
	 */
	#define S_IRWXO     000007              /*  Read, write, execute/search     */
	#define S_IROTH     000004              /*  Read permission                 */
	#define S_IWOTH     000002              /*  Write permission                */
	#define S_IXOTH     000001              /*  Execute/search permission       */

	/*
	** signal.h
	*/

	#define SIGHUP 1
	#define SIGQUIT 3
	#define SIGPIPE 13
   
    /*
    ** fnmatch.h
    */

	#define FNM_PATHNAME 001
	#define FNM_PERIOD   002
	#define FNM_NOESCAPE 004
	#if defined(__EXT_QNX)
	#define FNM_QUOTE    FNM_NOESCAPE
	#endif
	#define FNM_NOMATCH  1

	extern int fnmatch(const char *__expr, const char *__str, int __flags);

    /*
    ** wait.h
    */
    
    #define WIFEXITED(stat_val)   (((stat_val) & 0x7f) == 0)
    #define WIFSIGNALED(stat_val) (((stat_val) & 0x7f) != 0)
    #define WIFSTOPPED(stat_val)  (0)
    #define WEXITSTATUS(stat_val) (((stat_val) >> 8) & 0xff)
    #define WTERMSIG(stat_val)    (((stat_val) & 0x7f))
    #define WSTOPSIG(stat_val)    (0)
    

    /*
    ** sys/utsname.h
    */
	#define _SYSNAME_SIZE		(256 + 1)
	
	struct utsname {
	    char    sysname[_SYSNAME_SIZE],		/* SI_SYSNAME */
	            nodename[_SYSNAME_SIZE],	/* SI_HOSTNAME */
	            release[_SYSNAME_SIZE],		/* SI_RELEASE */
	            version[_SYSNAME_SIZE],		/* SI_VERSION */
	            machine[_SYSNAME_SIZE];		/* SI_MACHINE */
	};

	#ifndef __LITTLEENDIAN__
		#define __LITTLEENDIAN__
	#endif

	#ifndef __X86__
		#define __X86__
	#endif

	#define SKIP_DRIVE(n)	((isalpha((n)[0]) && (n)[1] == ':') ? (n)+2 : (n))
	#define MAKE_BINARY_FD( fd )	setmode( fd, O_BINARY )
	#define MAKE_BINARY_FP( fp )	MAKE_BINARY_FD( fileno( fp ) )
	#define IS_EXE_NAME( n1, n2 )	__is_executable_name( n1, n2 )
	#define QSSL_ROOT_VAR		"QSSL"

	/*
	** disk.h 
	*/
	#define _FLOPPY     1
	#define _HARD       2
	#define _RAMDISK    3	
	#define _DRIVER_NAME_LEN   12


/*
 *  Info about a QNX drive.
 *
 *  NOTE:  It is possible that cylinders, heads and track_sectors may be 0,
 *          in which case only disk_sectors describes the physical disk size.
 */
#pragma pack(1)
struct _disk_entry {
    unsigned long   blk_offset,
                    num_sectors, /* Num sectors in logical drive (partition) */
                    disk_sectors;/* Num sectors on physical disk    */
    unsigned short  cylinders,
                    heads,
                    track_sectors;
    unsigned char   disk_type,
                    disk_drv;           /*  Drive number as known to driver */
    long            reserved[3];
    unsigned char   driver_name[_DRIVER_NAME_LEN];
};
#pragma pack()

	
#elif defined(__NT__) || defined(__DOS__)
	/*
	** util/diskman.h
	*/
	#ifndef PAUSEphrase
	#define PAUSEphrase           "Please type the <Enter> key to continue..."
	#endif

    /*
    ** sys/types.h
    */
    
    typedef int pid_t;
    typedef int nid_t;
    typedef int gid_t;
    typedef int uid_t;
    typedef   signed short 	mpid_t;
    typedef short muid_t;    
    typedef short mgid_t;   
    typedef unsigned short	msg_t;
    typedef short unsigned  nlink_t;/* Used for link counts         */
    
    typedef signed ssize_t;
    typedef short unsigned  mode_t;
    typedef long        off_t; 
    typedef char *          caddr_t;
    typedef unsigned long	paddr_t;
    typedef unsigned char   uchar_t;
    typedef unsigned short  ushort_t;
    typedef unsigned long   ulong_t;
    typedef unsigned char   u_char;
    typedef unsigned short  u_short;
    typedef unsigned int    u_int;
    typedef unsigned long   u_long;
    typedef int				clockid_t;
    typedef int				timer_t;

    #include <inttypes.h>
    
    /*
    ** errno.h
    */
    
    #define EOK 0
    
    /*
    ** limits.h
    */
    
    #define _POSIX_PATH_MAX 255
    #define NGROUPS_MAX		8
    #define SORT_LINE_MAX	20480
    #define LINE_MAX		2048
    #define NAME_MAX		48

    /*
    ** limits.h
    */

    #define strcasecmp stricmp
    #define strncasecmp strnicmp 

    /*
    ** stdlib.h
    */
    
    extern unsigned atoh(const char *);
    extern void      searchenv( const char *__name, const char *__env_var,
                        char *__buf );
    #define _MAX_NODE				_MAX_DRIVE
    
    /*
    ** unistd.h
    */
    
    extern int   optind;        /*  index of current option being scanned */
    extern char *optarg;        /*  points to optional argument */
    extern int   opterr;        /*  print|don't print error message */
    extern int   optopt;        /*  offending letter when error detected */
    
    extern nid_t    getnid( void );
    extern gid_t    getgid( void );
    extern uid_t    getuid( void );
    extern int      getopt( int __argc, char * const* __argv, const char * __optstring );
    extern char     *basename( const char * fname );
    extern char     *dirname(const char *path);
    extern int 	eaccess(const char *path, int mode);
    int 		fchown(int fd, uid_t owner, gid_t group);
    int 		fchmod(int fd, mode_t mode);
    off_t		ltrunc(int fd, off_t offset, int whence);
    char		*getcwd( char *__buf, unsigned __size );
    int			chdir( const char *__path );

    
    #define getgid() 0
    #define getnid() 0
    #define getuid() 0
    #define major( d ) 0
    #define minor( d ) 0
    #define chown( fn, uid, gid )
    #define readlink(a, b, c) (-1)
    #define makedev(node,major,minor)       ((dev_t)(((node) << 16) | ((major) << 10) | (minor)))
    
    /*
    ** signal.h
    */
    
    #define SIGHUP 1
    #define SIGQUIT 3
   
    /*
    ** stdio.h
    */

    #define L_cuserid 14   /* Max length of login names */
 
    /*
    ** vc.h
    */
    
    nid_t qnx_strtonid(const char *nodename, char **str);
    
    /*
    ** stat.h
    */
    
    #define lstat( path, buff )		stat( path, buff )
    
    #define S_ISLNK(m)  0	/*(((m)&_S_IFMT)==_S_IFLNK) Test for symbolic link.    */
    #define S_ISNAM(m)  0	/*(((m)&_S_IFMT)==_S_IFNAM) Test for special named file*/
    #define S_ISSOCK(m) 0	/*(((m)&_S_IFMT)==_S_IFSOCK) Test for socket.           */

    /*
    ** Win32 does not have links. Use these value for compatibility.
    */

    #define S_IFLNK     0xA000
    #define S_IFIFO     0x1000
    #define S_IFBLK     0x6000
    #define S_IFNAM     0x5000

    /*
    ** wait.h
    */
    
    #define WIFEXITED(stat_val)   (((stat_val) & 0x7f) == 0)
    #define WIFSIGNALED(stat_val) (((stat_val) & 0x7f) != 0)
    #define WIFSTOPPED(stat_val)  (0)
    #define WEXITSTATUS(stat_val) (((stat_val) >> 8) & 0xff)
    #define WTERMSIG(stat_val)    (((stat_val) & 0x7f))
    #define WSTOPSIG(stat_val)    (0)
    
    /*
    ** misc
    */
    
	#define NULL_DEVICE_NAME	"NUL"
    #define QSSL_ROOT_VAR		"QSSL"
    
    #define NORMAL_OPEN_PERMS 	(S_IWRITE | S_IREAD)
    #define EXECUTE_OPEN_PERMS 	(S_IWRITE | S_IREAD)

    char	*__qssl_rooted_fname( const char *name );
    char	*__qssl_rooted_pathlist( const char *paths );
    int		__is_executable_name( const char *name1, const char *name2 );
    
    #define QSSL_ROOT( n )			__qssl_rooted_fname( n )
    #define QSSL_ROOT_PATHLIST( n )	__qssl_rooted_pathlist( n )

    #define FN_CMP( n1, n2 )	stricmp( n1, n2 )

    #define IS_EXE_NAME( n1, n2 )	__is_executable_name( n1, n2 )

    #define PATHSEP_STR	";"
    #define PATHSEP_CHR	';'
	#define DIRSEP_STR "\\"
	#define DIRSEP_CHAR '\\'

    #define IS_DIRSEP( c )	( (c) == '/' || (c) == '\\' || (c) == ':' )
    #define IS_ABSPATH( n )	((n)[0] == '/' 		\
							|| (n)[0] == '\\'	\
							|| (isalpha((n)[0]) && (n)[1] == ':'))
	#define SKIP_DRIVE(n)	((isalpha((n)[0]) && (n)[1] == ':') ? (n)+2 : (n))

    int		__has_path( const char *name );
    char* __first_dirsep( const char* path );
    char* __last_dirsep( const char* path );
    
	#define HAS_PATH( n )				__has_path( n )
	#define FIRST_DIRSEP( n )		__first_dirsep( n )
	#define LAST_DIRSEP( n )			__last_dirsep( n )

	#define MAKE_BINARY_FD( fd )	setmode( fd, O_BINARY )
	#define MAKE_BINARY_FP( fp )	MAKE_BINARY_FD( fileno( fp ) )
	
#elif defined(__QNXNTO__)

	#include <stdlib.h>
    #include <sys/types.h>
    #include <stdint.h>
	
   /*
    ** unix.h
    */
	#ifndef MAXPATHLEN
	#define MAXPATHLEN      _POSIX_PATH_MAX
	#endif

	#if !defined(max)
	#define max(a,b)  (((a) > (b)) ? (a) : (b))
	#endif
	#if !defined(min) && !defined(__cplusplus)
	#define min(a,b)  (((a) < (b)) ? (a) : (b))
	#endif
	
	#define NORMAL_OPEN_PERMS   0666
	#define EXECTUTE_OPEN_PERMS 0777
	#define FIXUP_FILE_MODE
	
	#define NULL_DEVICE_NAME    "/dev/null"
	
	#define NORMAL_OPEN_PERMS   0666
	#define EXECTUTE_OPEN_PERMS 0777
	#define FIXUP_FILE_MODE
	
	#define NULL_DEVICE_NAME    "/dev/null"
	
	#define QSSL_ROOT( n )          (n)
	#define QSSL_ROOT_PATHLIST( n )     (n)
	#define FN_CMP( n1, n2 )        strcmp( n1, n2 )
	#define IS_EXE_NAME( n1, n2 )   (strcmp( n1, n2 )==0)
	
	#define PATHSEP_STR         ":"
	#define PATHSEP_CHR         ':'
	#define DIRSEP_STR 			"/"
	#define DIRSEP_CHAR			'/'
	
	#define IS_DIRSEP( c )          ( (c) == '/' )
	#define IS_ABSPATH( n )         ( (n)[0] == '/' )
	#define HAS_PATH( n )           (strchr( n, '/' ) != 0)
	#define FIRST_DIRSEP( n )		strchr( n, '/' )
	#define LAST_DIRSEP( n )			strrchr( n, '/' )

	#define SKIP_DRIVE(n)           (n)
	
	#define MAKE_BINARY_FD( fd )
	#define MAKE_BINARY_FP( fp )

	extern char    *openbsd_dirname(char *path);
	extern size_t  strlcpy( char *dst, const char *src, size_t siz );
	extern void    strmode( mode_t mode, char *p );
	
#elif defined(__CYGWIN__)
	#include <sys/types.h>

	typedef unsigned long   ulong_t;

	#if !defined(max)
	#define max(a,b)  (((a) > (b)) ? (a) : (b))
	#endif
	#if !defined(min) && !defined(__cplusplus)
	#define min(a,b)  (((a) < (b)) ? (a) : (b))
	#endif
	
	#define NORMAL_OPEN_PERMS   0666
	#define EXECTUTE_OPEN_PERMS 0777
	#define FIXUP_FILE_MODE
	
	#define NULL_DEVICE_NAME    "/dev/null"
	
	#define NORMAL_OPEN_PERMS   0666
	#define EXECTUTE_OPEN_PERMS 0777
	#define FIXUP_FILE_MODE
	
	#define NULL_DEVICE_NAME    "/dev/null"
	
	#define QSSL_ROOT_VAR		"QSSL"
	#define QSSL_ROOT( n )          (n)
	#define QSSL_ROOT_PATHLIST( n )     (n)
	#define FN_CMP( n1, n2 )        strcmp( n1, n2 )
	#define IS_EXE_NAME( n1, n2 )   (strcmp( n1, n2 )==0)
	
	#define PATHSEP_STR         ":"
	#define PATHSEP_CHR         ':'
	#define DIRSEP_STR 			"/"
	#define DIRSEP_CHAR			'/'
	
	#define IS_DIRSEP( c )          ( (c) == '/' )
	#define IS_ABSPATH( n )         ( (n)[0] == '/' )
	#define HAS_PATH( n )           (strchr( n, '/' ) != 0)
	#define FIRST_DIRSEP( n )			strchr( n, '/' )
	#define LAST_DIRSEP( n )			strrchr( n, '/' )
	#define SKIP_DRIVE(n)           (n)
	
	#define MAKE_BINARY_FD( fd )
	#define MAKE_BINARY_FP( fp )	

    /*
    ** fnmatch.h
    */

	#define FNM_PATHNAME 001
	#define FNM_PERIOD   002
	#define FNM_NOESCAPE 004
	#if defined(__EXT_QNX)
	#define FNM_QUOTE    FNM_NOESCAPE
	#endif
	#define FNM_NOMATCH  1

	#ifndef __BEGIN_DECLS
		#define __BEGIN_DECLS
	#endif
	#ifndef __END_DECLS
		#define __END_DECLS
	#endif

__BEGIN_DECLS
extern int fnmatch(const char *__expr, const char *__str, int __flags);
__END_DECLS

	/*
	** sys/types.h
	*/
	typedef int nid_t;

	#ifndef __LITTLEENDIAN__
		#define __LITTLEENDIAN__
	#endif

	#ifndef __X86__
		#define __X86__
	#endif

	#ifndef EOK
	#define EOK 0
	#endif

	#ifndef LINE_MAX
	#define LINE_MAX 2048
	#endif
	
	/*
	** unistd.h
	*/
	extern char     *basename( const char * fname );

#elif defined(__SOLARIS__) 
	#include <sys/int_types.h>

	#if !defined(max)
	#define max(a,b)  (((a) > (b)) ? (a) : (b))
	#endif
	#if !defined(min) && !defined(__cplusplus)
	#define min(a,b)  (((a) < (b)) ? (a) : (b))
	#endif

	#define minor(device)                   ((int)((device) & 0x3ff))
	#define major(device)                   ((int)(((device) >> 10) & 0x3f))
	#define makedev(node,major,minor)       ((dev_t)(((node) << 16) | ((major) << 10) | (minor)))

	#define eaccess     access

	#define NORMAL_OPEN_PERMS 	0666
	#define EXECTUTE_OPEN_PERMS 0777
	#define FIXUP_FILE_MODE

	#define NULL_DEVICE_NAME	"/dev/null"

	#define QSSL_ROOT( n )			(n)
	#define QSSL_ROOT_PATHLIST( n )		(n)
	#define FN_CMP( n1, n2 )		strcmp( n1, n2 )
	#define IS_EXE_NAME( n1, n2 )	(strcmp( n1, n2 )==0)

	#define PATHSEP_STR			":"
	#define PATHSEP_CHR			':'
	#define DIRSEP_STR 			"/"
	#define DIRSEP_CHAR			'/'

	#define IS_DIRSEP( c )			( (c) == '/' )
	#define IS_ABSPATH( n )			( (n)[0] == '/' )
	#define HAS_PATH( n )			(strchr( n, '/' ) != 0)
	#define FIRST_DIRSEP( n )		strchr( n, '/' )
	#define LAST_DIRSEP( n )			strrchr( n, '/' )
	#define SKIP_DRIVE(n)			(n)

	#define MAKE_BINARY_FD( fd )
	#define MAKE_BINARY_FP( fp )

	#define O_BINARY (0)
	#define EOK 0
	#define stricmp 	strcasecmp 
	#define strnicmp 	strncasecmp 


	/* from disk.h */
	#define _FLOPPY     1
	#define _HARD       2
	#define _RAMDISK    3	
	#define _DRIVER_NAME_LEN   12

/*
 *  Info about a QNX drive.
 *
 *  NOTE:  It is possible that cylinders, heads and track_sectors may be 0,
 *          in which case only disk_sectors describes the physical disk size.
 */
#pragma pack(1)
struct _disk_entry {
    unsigned long   blk_offset,
                    num_sectors, /* Num sectors in logical drive (partition) */
                    disk_sectors;/* Num sectors on physical disk    */
    unsigned short  cylinders,
                    heads,
                    track_sectors;
    unsigned char   disk_type,
                    disk_drv;           /*  Drive number as known to driver */
    long            reserved[3];
    unsigned char   driver_name[_DRIVER_NAME_LEN];
};
#pragma pack()

	/* from diskman.h */
	#ifndef PAUSEphrase
	#define PAUSEphrase           "Please type the <Enter> key to continue..."
	#endif

	#include <stdlib.h>
	extern char *_fullpath( char *buffer, const char *path, size_t size);
	extern int flushall( void );
	extern char *strsep(char **__stringp, const char *__delim);

	/* from sys/trace.h, sys/confname.h, sys/syspage.h and sys/neutrino.h */
	#define __NEUTRINO_H_INCLUDED
	#ifndef __BEGIN_DECLS
		#define __BEGIN_DECLS
	#endif
	#ifndef __END_DECLS
		#define __END_DECLS
	#endif
	#ifndef __P
	#define __P(x) x
	#endif
	#define __EXT_UNIX_MISC
	#define _CS_LIBPATH                     200             /* default path for runtime to find standard shared objects */
	#define _CS_DOMAIN                      201             /* Domain of this node within the communications network */
	#define _CS_RESOLVE                     202             /* In memory /etc/resolve.conf */
	#define _CS_TIMEZONE            203             /* timezone string (TZ style) */
	#define _CS_LOCALE                      204             /* locale string */
	#define _CS_SET_DOMAIN          (_CS_SET+_CS_DOMAIN)    /* Set the domain for this node */
	#define _CS_SET_RESOLVE         (_CS_SET+_CS_RESOLVE)   /* Set the in-memory /etc/resolve.conf */
	#define _CS_SET_TIMEZONE        (_CS_SET+_CS_TIMEZONE)  /* Set the timezone for this node */
	#define _CS_SET_LOCALE          (_CS_SET+_CS_LOCALE)    /* Set the locale for this node */


	typedef struct intrspin {
		volatile unsigned	value;
	} intrspin_t;


#elif defined(__LINUX__)
	#include <stddef.h>
	#include <sys/types.h>
	#include <stdint.h>
	typedef unsigned long   ulong_t;

	#if !defined(max)
	#define max(a,b)  (((a) > (b)) ? (a) : (b))
	#endif
	#if !defined(min) && !defined(__cplusplus)
	#define min(a,b)  (((a) < (b)) ? (a) : (b))
	#endif

	#define NORMAL_OPEN_PERMS 	0666
	#define EXECTUTE_OPEN_PERMS 0777
	#define FIXUP_FILE_MODE

	#define NULL_DEVICE_NAME	"/dev/null"

	#define QSSL_ROOT( n )			(n)
	#define QSSL_ROOT_PATHLIST( n )		(n)
	#define FN_CMP( n1, n2 )		strcmp( n1, n2 )
	#define IS_EXE_NAME( n1, n2 )	(strcmp( n1, n2 )==0)

	#define PATHSEP_STR			":"
	#define PATHSEP_CHR			':'
	#define DIRSEP_STR 			"/"
	#define DIRSEP_CHAR			'/'

	#define IS_DIRSEP( c )			( (c) == '/' )
	#define IS_ABSPATH( n )			( (n)[0] == '/' )
	#define HAS_PATH( n )			(strchr( n, '/' ) != 0)
	#define FIRST_DIRSEP( n )			strchr( n, '/' )
	#define LAST_DIRSEP( n )			strrchr( n, '/' )
	#define SKIP_DRIVE(n)			(n)

	#define MAKE_BINARY_FD( fd )
	#define MAKE_BINARY_FP( fp )

	#define O_BINARY (0)
	#define EOK 0
	#define stricmp 	strcasecmp 
	#define strnicmp 	strncasecmp 

	#define eaccess		access

	extern char *_fullpath( char *buffer, const char *path, size_t size);
	extern char *pathfind(__const char *__path, __const char *__name, __const char *__mode);
	extern int  memicmp( const void *__s1, const void *__s2, size_t __n );
	extern char     *itoa( int __value, char *__buf, int __radix );
	extern char     *ltoa( long int __value, char *__buf, int __radix );
	extern char     *ultoa( unsigned long int __value, char *__buf, int __radix );
	extern char     *utoa( unsigned int __value, char *__buf, int __radix );
	extern char     *lltoa( int64_t __value, char *__buf, int __radix );
	extern char     *ulltoa( u_int64_t __value, char *__buf, int __radix );
	extern void     strmode( mode_t mode, char *p );
	extern size_t   strlcpy( char *dst, const char *src, size_t siz );
	extern off_t    tell( int __fildes );

	/* from /usr/nto/usr/include/sys/stat.h */
	#define _S_IFNAM    0x5000
	#define S_IFNAM     _S_IFNAM

	/* from disk.h */
	#define _FLOPPY     1
	#define _HARD       2
	#define _RAMDISK    3	
	#define _DRIVER_NAME_LEN   12

/*
 *  Info about a QNX drive.
 *
 *  NOTE:  It is possible that cylinders, heads and track_sectors may be 0,
 *          in which case only disk_sectors describes the physical disk size.
 */
#pragma pack(1)
struct _disk_entry {
    unsigned long   blk_offset,
                    num_sectors, /* Num sectors in logical drive (partition) */
                    disk_sectors;/* Num sectors on physical disk    */
    unsigned short  cylinders,
                    heads,
                    track_sectors;
    unsigned char   disk_type,
                    disk_drv;           /*  Drive number as known to driver */
    long            reserved[3];
    unsigned char   driver_name[_DRIVER_NAME_LEN];
};
#pragma pack()

	/* from diskman.h */
	#ifndef PAUSEphrase
	#define PAUSEphrase           "Please type the <Enter> key to continue..."
	#endif
/* from confname.h */
#define __EXT_UNIX_MISC
#define _CS_HOSTNAME            2               /* Name of this node within the communications network */
#define _CS_RELEASE                     3               /* Current release level of this implementation */
#define _CS_VERSION                     4               /* Current version of this release */
#define _CS_MACHINE                     5               /* Name of the hardware type on which the system is running */
#define _CS_ARCHITECTURE        6               /* Name of the instructions set architechure */
#define _CS_HW_SERIAL           7               /* A serial number assiciated with the hardware */
#define _CS_HW_PROVIDER         8               /* The name of the hardware manufacturers */
#define _CS_SRPC_DOMAIN         9               /* The secure RPC domain */
#define _CS_SYSNAME                     11              /* Name of this implementation of the operating system */

#define __EXT_QNX
#define _CS_LIBPATH                     200             /* default path for runtime to find standard shared objects */
#define _CS_DOMAIN                      201             /* Domain of this node within the communications network */
#define _CS_RESOLVE                     202             /* In memory /etc/resolve.conf */
#define _CS_TIMEZONE            203             /* timezone string (TZ style) */
#define _CS_LOCALE                      204             /* locale string */
 
#elif defined(__QNX__) 
#if !defined(max)
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#if !defined(min) && !defined(__cplusplus)
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#define NORMAL_OPEN_PERMS   0666
#define EXECTUTE_OPEN_PERMS 0777
#define FIXUP_FILE_MODE

#define NULL_DEVICE_NAME    "/dev/null"

#define NORMAL_OPEN_PERMS   0666
#define EXECTUTE_OPEN_PERMS 0777
#define FIXUP_FILE_MODE

#define NULL_DEVICE_NAME    "/dev/null"

#define QSSL_ROOT( n )          (n)
#define QSSL_ROOT_PATHLIST( n )     (n)
#define FN_CMP( n1, n2 )        strcmp( n1, n2 )
#define IS_EXE_NAME( n1, n2 )   (strcmp( n1, n2 )==0)

#define PATHSEP_STR         ":"
#define PATHSEP_CHR         ':'
#define DIRSEP_STR 			"/"
#define DIRSEP_CHAR			'/'

#define IS_DIRSEP( c )          ( (c) == '/' )
#define IS_ABSPATH( n )         ( (n)[0] == '/' )
#define HAS_PATH( n )           (strchr( n, '/' ) != 0)
#define FIRST_DIRSEP( n )		strchr( n, '/' )
#define LAST_DIRSEP( n )			strrchr( n, '/' )
#define SKIP_DRIVE(n)           (n)

#define MAKE_BINARY_FD( fd )
#define MAKE_BINARY_FP( fp )
#else  
    #error lib/compat.h not configured for system
#endif

#endif
