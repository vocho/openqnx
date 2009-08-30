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
#ifdef __USAGE
%C	- copy files (POSIX)

%C	[options]... source_file target_file
%C	[options]... source_file... target_dir
Options:
 -a/-b date/time   Copy only files dated after/before date/time.
 -A                Preserve source file access times. Cannot be used with -O.
 -B                Use a very small (2k) copy buffer.
 -c                Create any directories necessary to open dest path.
 -d                Distribute to multiple dests. e.g. cp -R /hd to /hd1 /hd2
 -D                Descend past device boundaries.
 -f                Force unlink of destination.
 -g grp[,grp]...   Copy only files belonging to these groups.
 -i                Interactive
 -l n              When -R or -r is specified, recurse only n levels.
                   -l0 will cause cp to not recurse through directories
                   named on the command line.
 -L                Attempt to preserve hard links. To attempt to keep
                   symbolic links, the -R option must be used.
 -M {unix|qnx}     Recursive copies to be done in UNIX or old-style QNX-4
                   mode. (Default: qnx). QNX has, in the past, copied the
                   _contents_ of the directories named on the command line
                   into the target directory. UNIX copies the directory
                   itself into the target directory (like 'mv'). In either
                   case if there is only one directory being copied and
                   the destination names a directory that doesn't exist,
                   cp will create the destination directory and then copy
                   the _contents_ of the source directory into the 
                   destination directory. At some point in the future a
                   version of QNX will be released which makes 'unix' the
                   default for this option.
 -m mode[,mode]... Copy only files with these modes (perms).
 -n                Copy only if src is newer than dst or dst doesn't exist.
 -N                Do not descend past device boundaries.
 -O                Optimize - run cp on the cpu of destination node, do
                   whatever makes sense to speed up the operation.
 -p                Duplicate ownership (root only), permissions & file dates.
 -P pat[,pat]...   Copy only files matching these patterns.
 -R                Recursive copy of directories, attempting to
                   preserve the type of special files. On QNX, the attempt
                   to create block special files and character special files
                   will fail.
 -r                Recursive copy of directories, treating special files
                   the same as regular files (i.e. copy their contents to
                   a regular destination file).
 -s                Safe - skip over any destination files that don't have
                   write permission w/o prompting.
 -t                Inhibit duplication of file time and mode.  Mode will be
                   copied to newly created files (except that directories will
                   always have rwx user permissions).
 -u ownr[,ownr]... Copy only files belonging to these owners.
 -v | -V           Verbose. -V is extra verbose.
 -W                Wildly verbose (debug)
 -x/-X             Copy only if destination file does/doesn't already exist.
Where:
 Date/time format is "[UTC] day/month/yr hr:min:sec", 24 hr clock.
 Mode format is [!]<octal num> or <[!][augo]=[rwxs]>[,...]
 (Similar to the form used in the chmod utility.)
Note: 
 Permissions and file dates are preserved by default. Either exporting the
 POSIX_STRICT environment variable or using the -t option will override this.
#endif
*/

/* neutrino version
#ifdef __USAGENTO 
%C	- copy files (POSIX)

%C	[options]... source_file target_file
%C	[options]... source_file... target_dir
Options:
 -A                Preserve source file access times.
 -B                Use a very small (2k) copy buffer.
 -c                Create any directories necessary to open dest path.
 -d                Distribute to multiple dests. e.g. cp -R /hd to /hd1 /hd2
 -D                Descend past device boundaries. (default)
 -f                Force unlink of destination.
 -i                Interactive
 -l n              When -R or -r is specified, recurse only n levels.
                   -l0 will cause cp to not recurse through directories
                   named on the command line.
 -L                Attempt to preserve hard links. To attempt to keep
                   symbolic links, the -R option must be used.
 -M {unix|qnx}     Recursive copies to be done in UNIX or old-style QNX-4
                   mode. (Default: UNIX). QNX has, in the past, copied the
                   _contents_ of the directories named on the command line
                   into the target directory. UNIX copies the directory
                   itself into the target directory (like 'mv'). In either
                   case if there is only one directory being copied and
                   the destination names a directory that doesn't exist,
                   cp will create the destination directory and then copy
                   the _contents_ of the source directory into the 
                   destination directory.
 -n                Copy only if src is newer than dst or dst doesn't exist.
 -N                Do not descend past device boundaries.
 -p                Duplicate ownership (root only), permissions & file dates.
 -R                Recursive copy of directories, attempting to
                   preserve the type of special files. On QNX, the attempt
                   to create block special files and character special files
                   will fail.
 -r                Recursive copy of directories, treating special files
                   the same as regular files (i.e. copy their contents to
                   a regular destination file).
 -s                Safe - skip over any destination files that don't have
                   write permission w/o prompting.
 -t                Inhibit duplication of file time and mode.  Mode will be
                   copied to newly created files (except that directories will
                   have rwx user permissions).
 -v | -V           Verbose. -V is extra verbose.
 -W                Wildly verbose (debug)
 -x/-X             Copy only if destination file does/doesn't already exist.
Note: 
 Permissions and file dates are preserved by default. Either exporting the
 POSIX_STRICT environment variable or using the -t option will override this.
#endif
*/

/* #define ENVAR to turn on CP_OPT envar support */
/* #define CONFIG_FILE to turn on /etc/config/cp.defaults support */
/* #define NCP to turn on 'new' -R behaviour as default (-Munix) */
/* #define OCP to turn on 'old' -R behaviour as default (-Mqnx) */
/* #define SUPPORT_OLD_OPTIONS for QNX4.24/earlier type options */
/* #define SUPPORT_NET_OPTIMIZATION for -O option support */
/* #define SUPPORT_REMOVABLE_MEDIA for copying to multiple floppy disks */
/* for now, if neither NCP or OCP is specified, */

#if !defined(__QNXNTO__) && !defined(__MINGW32__)
#define SUPPORT_OLD_OPTIONS
#define SUPPORT_NET_OPTIMIZATION
#define SUPPORT_REMOVABLE_MEDIA
#else
#define NCP
#define __far
#endif


/* #define DIAG for extra diagnostics beyond -W mode (e.g. for -c debugging) */


/*----------------------------------------------------------------------
POSIX UTILITY: CP
1003.2 -- Draft 9 (Shell and Utilities working group)
----------------------------------------------------------------------*/

#include <stdio.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <time.h>
#include <fcntl.h>
#include <utime.h>
#ifndef __MINGW32__
#include <libgen.h>
#include <sys/disk.h>
#endif

#include <util/stdutil.h>
#include <util/stat_optimiz.h>

#ifndef __QNXNTO__

#  ifndef __MINGW32__
#include <sys/proc_msg.h>
#include <sys/osinfo.h>
#include <sys/dev.h>
#include <sys/io_msg.h>
#include <sys/prfx.h>
#  else
#include <lib/compat.h>
#  endif

#else

#include <sys/neutrino.h>

#endif // ndef __QNXNTO__

#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <malloc.h>
#include <process.h>
#include <sys/wait.h>
#include <malloc.h>
#include <util/defns.h>

#if defined(SUPPORT_OLD_OPTIONS) && !defined(__MINGW32__)
#include <util/patmodule.h>
#include "full_open.h"
#endif

#ifdef __MINGW32__
#define mkdir(path, mode) _mkdir(path)
#define __futime(fd, filetime) _futime(fd, filetime)
#endif

/*------------------------------------------------------------ defines ------*/

#define PSTRICT_ENVAR			"POSIX_STRICT"

#define TXT(s)					s

#define T_PATH_IS_NONDIR        "cp: target path '%s' is not a directory. Skipping.\n"
#define T_NEED_SRC_AND_DEST		"cp: A source and destination must be specified.\n"
#define T_INVALID_LEVELS		"cp: Invalid levels (%s)\n"
#define T_UTIME_FAILED			"cp: Can't change file time (data copied) (%s)"
#define T_CHOWN_FAILED			"cp: Can't change file owner (data copied) (%s)"
#define T_CHMOD_FAILED			"cp: Can't change file mode (data copied) (%s,%o)"
#define T_LINK_FAILED	 		"cp: Can't create link (%s,%s)"
#define T_NO_DESTINATION		"cp: Missing destination(s) after 'to'.\n"
#define T_CANT_OPEN_DIRECTORY   "cp: Can't open directory. "
#define T_CANT_MKFIFO			"cp: Can't create FIFO file (%s)\n"
#define T_CLOSEDIR_FAILED		"cp: Can't close directory "
#define T_READDIR_FAILED		"cp: Can't read directory "
#define T_CANTRECURSE           "cp: %s is a directory. Use -R to recurse.\n"
#define T_NO_BUF_MEM			"cp: Not enough memory.\n"
#define T_READ_ERROR			"cp: read  "
#define T_WRITE_ERROR			"cp: write "
#define T_FROM_TO				"cp: %s --> %s\n"
#define T_SKIPPING2				"cp: Skipping file %s/%s\n"
#define T_OVERWRITE 			"cp: Destination exists! Overwrite? (y/N) "
#define T_LINKOVERWRITE			"cp: Destination exists! (Need to create link) Overwrite? (y/N) "
#define T_NOWRITEFORCE			"cp: Open dest for write failed. Unlink first? (y/N) "
#define T_VERBOSE_MSG			"cp: Copying %s to %s\n"
#define T_VERBOSE_MKFIFO		"cp: Creating FIFO %s\n"
#define T_VERBOSE_SYMLINK		"cp -R: Creating symbolic link %s --> %s\n"
#define T_LINK_MSG				"cp: Linking %s to %s\n"
#define T_MADE_DIR				"cp: Made directory %s\n"
#define T_MADE_PATH				"cp: Made dir path to %s\n"
#define T_CANT_COPY_TO_SELF		"cp: Can't copy to self. (%s --> %s)\n"
#define T_MEDIA_PROMPT			"cp: Out of room copying '%s'.\ncp: If you wish to continue to another disk, insert\ncp: the disk and reply 'yes'. Continue? (Y/n)? "
#define T_USEDASHC				"cp: %s must be an existing directory. If you want cp to create it,\nyou must use the -c option.\n"
#define T_CANT_CREATE_PATH		"cp: Can't create directory path for %s\n"
#define T_SMALL_BUFFER			"cp: (FYI) can't alloc %d byte buffer. Using %d byte buffer instead.\n"
#define T_DESTINATION			"cp: Destination: %s\n"
#define T_OUT_OF_SPACE_WRITE	"cp: Ran out of disk space writing %s.\n"
#define T_OUT_OF_SPACE_DIR		"cp: Ran out of disk space writing dir %s.\n"
#define T_OUT_OF_SPACE_OPEN		"cp: Ran out of disk space opening %s.\n"
#define T_CANTDUPCHARSPECIAL    "cp -R: Character special file - cannot duplicate. (%s)\n"
#define T_CANTDUPBLOCKSPECIAL	"cp -R: Block special file - cannot duplicate. (%s)\n"
#define T_CANTDUPNAMEDSPECIAL	"cp -R: Special named file - cannot duplicate. (%s)\n"
#define T_CANTDUPSOCKETSPECIAL	"cp -R: Socket - cannot duplicate. (%s)\n"
#define T_CANT_COPY_NONREG_FILE t_cant_copy_nonreg_file
#define T_CANT_SYMLINK			"cp -R: Can't create symbolic link %s --> %s (%s)\n"
#define T_CANT_READLINK			"cp -R: Can't read symbolic link %s (%s)\n"
#define T_FULLPATH_FAILED		t_fullpath_failed
#define T_TARGET_MUST_BE_DIR	t_target_must_be_dir
#define T_CANT_MAKE_DIRECTORY   t_cant_make_directory
#define T_CANT_MAKE_DIRECTORY2  "cp: Can't create path for "
#define T_STAT_FAILED 			t_stat_failed
#define T_CANT_OPEN_SRC			t_cant_open_src
#define T_CANT_OPEN_DST			t_cant_open_dst
#define T_CLOSE_ERROR			t_close_error
#define T_SKIPPING				t_skipping
#define T_SKIPPING_DIR			t_skipping_dir
#define T_CANT_UNLINK			t_cant_unlink
#define T_CREATING_PATH_TO		t_creating_path_to


/* for parsing of envar and config file command line items */
#ifdef CONFIG_FILE
#define CONFIGFILENAME          "/etc/config/cp.defaults"
#define MAXIMUM_CONFIGFILE_DATA (4096)	/* 4k of options max may be in config file */
#define CONFIG_OR_ENVAR         (1)
#else
#ifdef ENVAR
#define CONFIG_OR_ENVAR         (1)
#endif
#endif

#ifdef CONFIG_OR_ENVAR
#define SIMULATED_ARGV_MAXITEMS (64)
#define OPTION_TOKEN_MAXLEN     (PATH_MAX+16)
#endif

/* buffer size used to shorten long names */
#define SHORT_FILENAME_LEN 	 20

/* default maximum number of levels allowed in recursion */
#define MAX_LEVELS			50L

/* do not do a remote spawn on cp -O if only one file is being copied to/from that node,
   and the size of that file is less than this */
#define REMOTE_MINIMUM		(65536L)

/* largest amount of memory to attempt to acquire for buffering purposes.
   the larger the value, the better the speed */
#define MAX_BUF_SIZE		(16*1024)

/* this much memory will be reserved no matter what - the buffer will be
   dynamically reduced as necessary to keep this much memory free */
#define HEAP_REQUIRED		(4*1024)

/* buffer size to use when -B is specified */
#define FALLBACK_BUF_SIZE   (2*1024)

#define RETRY				(-1)	/* rc from copy_guy meaning 'try again' */
#define STDOUT_BUFLEN		(512)	/* 1/2k output buffer */
#define STDERR_BUFLEN		(256)	/* 1/4k stderr buffer */

#define MAX_PATTERNS		(8)		/* up to 8 patterns may be spec'd */
#define MAX_GROUPS			(16)	/* up to 16 groups may be spec'd */
#define MAX_OWNERS			(16)	/* up to 16 owners may be spec'd */
#define MAX_MODES			(8)		/* up to 8 modes may be spec'd */

struct link_reg {
	ino_t ino;				/* serial number */
	dev_t dev;				/* device id */
	nlink_t count;			/* when this hits 0, this becomes an unused entry */
	char *pathname;			/* pathname to be linked to */
	struct link_reg *next;	/* right! Its a link list! arh arh arh */
} ;

/* file info - two linked lists built-up from src and destination lists,
   then evaluated to determine what node to choose to run the copy on */
struct finfo {
#ifdef SUPPORT_NET_OPTIMIZATION
	nid_t node;			/* node that this entry is for */
#endif
	int   count;		/* count of non-directories spec'd on cmd line */
	int   dir_count;    /* count of directories - worth 15x what count is */
	long  num_bytes;	/* number of data bytes - we are mainly interested in this
                           to see if there is enough data worth copying when there is
                           just one file on this node to be copied */
	struct finfo *next;	/* ptr to next node's worth */
};

const char *t_cant_copy_nonreg_file ="cp: Can't copy non-regular file (fifo etc)";
const char *t_fullpath_failed		="cp: Can't obtain full path for '%s'";
const char *t_target_must_be_dir	="cp: Dest (%s) must be a dir to copy dirs or multiple files to it.\n";
const char *t_cant_make_directory	="cp: Can't create directory. ";
const char *t_cant_make_directory2	="cp: Can't create path for ";
const char *t_stat_failed			="cp: Can't get file status ";
const char *t_cant_open_src			="cp: Can't open source file. ";
const char *t_cant_open_dst			="cp: Can't open destination file. ";
const char *t_close_error			="cp: close ";
const char *t_skipping				="cp: Skipping file %s\n";
const char *t_skipping_dir			="cp: Skipping directory %s\n";
const char *t_cant_unlink			="cp: Can't unlink file %s\n";
const char *t_creating_path_to		="cp: Creating path to %s\n";

struct link_reg *link_list = NULL;

/* options that are valid to be specified from the cmd line */
#ifndef SUPPORT_OLD_OPTIONS
#ifndef OPTIONS
#define OPTIONS "fiprR**sDtBVvxXnNcdLl:WM:A"
#endif
#else
#ifndef OPTIONS
#define OPTIONS "fiprR**sDtBVvxXnNcdLl:a:b:g:m:P:u:WOM:A"
#endif
#endif 

/* options that are valid to be specified from a config file */
#ifdef CONFIG_FILE
#define CONFIG_OPTIONS "M:BLOtvVW"
#endif

/* options that are valid to be specified from an envar */
#ifndef ENVAR
#define ENVAR_OPTIONS  "M:BLOtvVW"
#endif

char *string_format = "%s\n";
char *twostrpar     = "%s (%s)";
char *twostrparnl   = "%s (%s)\n";
char *twostr		= "%s %s";

int16_t num_files;	/* number of src files supplied on cmd line */
int32_t cur_level;	/* current levels 'down' in a directory tree (recursion) */
int32_t levels;		/* maximum levels of recursion allowed. Norm. MAX_LEVELS */

long  verbose_period;

int16_t first_dest, last_dest, dest_index;

bool	dashp, ignore_device, copy_newest, distribute, copy_only_if_exists,
		force_del, interactive, cp_timemode, cp_ownergroup, recursive, verbose,
		super_verbose, create_paths, dont_copy_if_exists, preserve_links, safe,
		directory_target, dir_to_dir, strict, abort_flag, ranoutofspace,
		freshly_created, wildly_verbose, special_recursive,
		super_verbose_percent, standard_recursive;

#ifdef SUPPORT_NET_OPTIMIZATION
bool    net_optimized;
#endif

bool   preserve_atime = FALSE;

int16_t  target_index;			/* index in argv[] of cmd line target file	 */

uint16_t buf_size = MAX_BUF_SIZE;	/* size of copy buffer */

char *target;		 			/* pointer to string of target file/dir name */
char source[PATH_MAX+1];	/* src pathname								 */
char dest[PATH_MAX+1];   /* dest pathname							 */
char tmp2pathfull[PATH_MAX+1]; /* 2nd buffer for obtaining full paths */

struct stat statbufsrc;			/* used for getting file statuses			 */
struct stat lstatbufsrc;		/*	used for getting file statuses			 */
struct stat statbufdst;			/* used for getting file statuses			 */
struct stat lstatbufdst;		/* used for getting file statuses			 */
bool		lstatbufsrc_valid;
bool		lstatbufdst_valid;
bool		statbufsrc_valid;
bool		statbufdst_valid;
dev_t		dest_dev;			/* memorized to prevent infinite recursive copy */
ino_t		dest_ino;			/*   "   */
int16_t		in, out;			/* input and output file descriptors		 */
int16_t		len;				/* size of read/write buffer for copy		 */
char		*bptr;				/* copy buffer								 */
int16_t		errs = 0;			/* # errors encountered in copy operation	 */
dev_t		source_device;		/* use this to keep track of descending past 
								   device boundaries						 */
char		srstring[16];		/* local copy of super-root string			 */

/* stuff to track directory creation & info to set on these directories 
   after they have been copied into */
int16_t	dirlevel_created = 0;
mode_t  dir_mode_tab[MAX_LEVELS];
uid_t   dir_uid_tab[MAX_LEVELS];
gid_t	dir_gid_tab[MAX_LEVELS];
struct utimbuf dir_time_tab[MAX_LEVELS];

int16_t	Max_levels = MAX_LEVELS;	/* may be knocked to 0 on error cond's */

/*---------------------------------------------------------- PROTOTYPES -----*/

bool	removeable	(char *filename);
bool    insert_new_disk(char *disk_pathname,char *prompt_filename, char *errstring);
bool	try_hard_to_mkdir(char *dest, size_t destsize);
bool	ask_user	( char *src_filename , char *dst_filename , char *format);
bool	dir_exists  ( char * );
int16_t	open_file	( char *filename );
int16_t	open_dest	( char *filename , char *source_filename , bool link_flag, bool *skipped );
int16_t	open_src	( char *filename );
int16_t	copy_guy	( char *src , char *dst , size_t dstsize );
int	qnx_create_path_to (char *fullpath);
void	copy_directory_recursive( char *src , char *dst , size_t dstsize );
char	*purty       ( char *string );
struct	link_reg *locate_link ( struct link_reg *link_list, struct stat *statbufp );
struct	link_reg *register_link ( struct link_reg *link_list, struct stat *statbufp, char * destpath);
void	clear_links (struct link_reg *link_list);
int		create_link ( struct link_reg *link_entry, char *destpath );
#ifdef SUPPORT_NET_OPTIMIZATION
struct finfo *expand_to_fullpaths(char **argv, int num_files, bool writeflag );
#endif
int16_t	inc_errs(char *string);

int16_t inc_errs(char *string)
{
	errs++;
	if (wildly_verbose) fprintf(stderr,"cp: ERRS++ (%ld) %s\n",(long)errs,string);

	return errs;
}

void dirlevel_warning(void) 
{
	inc_errs("dirlevel_warning");
	fprintf(stderr,"cp: Warning - Levels of directories created exceeds maximum (%ld).\n",MAX_LEVELS);
	fprintf(stderr,"cp: No further attempt will be made to duplicate directory\n");
	fprintf(stderr,"cp: permissions or owners.\n");
	dirlevel_created=0;
	Max_levels=0;
}


#if !defined(__QNXNTO__) && !defined(__MINGW32__)
int (full_open)(const char *path, char *fullpath, int oflag, ...)
    {
  	register struct _io_open    *omsg;
    va_list                      ap;
    int                          fd;

    if (
		(omsg=(struct _io_open *) calloc(sizeof(*omsg)+PATH_MAX,1))==0
	   )
        {
        errno = ENOMEM;
        return(-1);
        }

    if((omsg->oflag = oflag) & O_CREAT)
        {
        omsg->oflag &= ~O_CREAT;
        va_start(ap, oflag);
        omsg->mode = va_arg(ap, mode_t);
        if((fd = __resolve_net(_IO_OPEN, 1, omsg, path,
                            sizeof(struct _io_open_reply), fullpath)) == -1)
            {
            omsg->oflag |= O_CREAT;
            fd = __resolve_net(_IO_OPEN, 1, omsg, path,
                                    sizeof(struct _io_open_reply), fullpath);
            }
        else if(oflag & O_EXCL)
            {
            close(fd);
            fd = -1;
            }
        }
    else
        fd = __resolve_net(_IO_OPEN, 1, omsg, path,
                                    sizeof(struct _io_open_reply), fullpath);

    free(omsg);
    return(fd);
    }
#endif


/*---------------------------------------------------------- _fns() --------*/
/* These exist so I can specify -W and see how (in)efficiently cp is doing
   its opens, closes, stats. These are all 'expensive' operations & represent
   a great deal of the time it takes to cp small files						*/
int verbose_mkdir(const char *path, mode_t mode ) {
	int rc, e;

	if (wildly_verbose) fprintf(stderr,"cp: MKDIR(%s,%o)\n",path,mode);
	rc = mkdir(path,mode);
	e = errno;
	if (rc==-1) perror("_mkdir");
	errno=e;
	return(rc);
}

int _chmod ( const char *path, int mode ) {
	if (wildly_verbose) fprintf(stderr,"cp: CHMOD(%s,%o)\n",path,mode);
	return chmod(path,mode);
}

int _chown ( const char *path, int uid, int gid ) {
	if (wildly_verbose) fprintf(stderr,"cp: CHOWN(%s,%d,%d)\n",path,uid,gid);
	return chown(path,uid,gid);
}

int verbose_fchmod ( const int fd, int mode ) {
	if (wildly_verbose) fprintf(stderr,"cp: FCHMOD(%d,%o)\n",fd,mode);
	#ifndef __MINGW32__
	return fchmod(fd,mode);
	#else
	return 0; // pretend you changed the mode 
	//TODO: do real change of attributes here. We can at least controll read-only attribute.
	#endif
}

int verbose_fchown ( const int fd, int uid, int gid ) {
	if (wildly_verbose) fprintf(stderr,"cp: FCHOWN(%d,%d,%d)\n",fd,uid,gid);
	#ifndef __MINGW32__
	return fchown(fd,uid,gid);
	#else 
	return 0; // TODO: we can probably change the owner on a NTFS file system.
	#endif
}

int verbose_open( const char *path, int oflag, int mode ) {
	if (wildly_verbose) fprintf(stderr,"cp: OPEN(%s,%o,%o)\n",path,oflag,mode);
#ifndef __MINGW32__
	if (special_recursive) oflag|=O_NONBLOCK;
#endif
	return(open(path,oflag,mode));
}
int _unlink( const char *path ) {
	if (wildly_verbose) fprintf(stderr,"cp: UNLINK(%s)\n",path);
	return(unlink(path));
}
int _lstat(const char *path, struct stat *buf) {
	if (wildly_verbose) fprintf(stderr,"cp: LSTAT(%s,buf)\n",path);
	return(lstat(path,buf));
}
int verbose_stat(const char *path, struct stat *buf) {
	if (!special_recursive && wildly_verbose) fprintf(stderr,"cp: STAT(%s,buf)\n",path);
	/* if special recursive, only return lstat info */
	return(special_recursive?_lstat(path,buf):stat(path,buf));
}

/* this can be called after a readdir or to validate the stat info on
   a file - if a symlink, it will find the real stat info. Given just
   a filename it will get both lstat and stat info */

int _statsrc(const char *path) {
    errno=0;   /* focus */

	statbufsrc_valid = FALSE;
	if (!lstatbufsrc_valid) {
		lstatbufsrc_valid = FALSE;
		if (_lstat(path,&lstatbufsrc)==-1) return -1;
		lstatbufsrc_valid = TRUE;
	}

	if (S_ISLNK(lstatbufsrc.st_mode) && !special_recursive) {
		/* we need to find stat info on the file */
		if (verbose_stat(path,&statbufsrc)==-1) return -1;
		statbufsrc_valid=TRUE;
	} else {
		statbufsrc=lstatbufsrc;
		statbufsrc_valid=lstatbufsrc_valid;
	}

	return 0;
}

/* this can be called after a readdir or to validate the stat info on
   a file - if a symlink, it will find the real stat info. Given just
   a filename it will get both lstat and stat info */
int _statdst(const char *path) {
	errno=0;         /* focus */

	statbufdst_valid = FALSE;
	if (!lstatbufdst_valid) {
		lstatbufdst_valid = FALSE;
		if (_lstat(path,&lstatbufdst)==-1) return -1;
		lstatbufdst_valid = TRUE;
	}
	if (S_ISLNK(lstatbufdst.st_mode)) {
		/* we need to find stat info on the file */
		if (verbose_stat(path,&statbufdst)==-1) return -1;
	} else statbufdst=lstatbufdst;
	statbufdst_valid=lstatbufdst_valid;
	return 0;
}

char __far *_qnx_fullpath(char *, char *, size_t); 
		
char __far *_qnx_fullpath(char *fullpath, char *path, size_t pathsize) {
	if (wildly_verbose) fprintf(stderr,"cp: QNX_FULLPATH(fullpath,%s)\n",path);
#if !defined(__QNXNTO__) && !defined(__MINGW32__)
	return qnx_fullpath(fullpath, path);
#else
        return _fullpath(fullpath, path, pathsize);
#endif
}

DIR *_opendir(const char *dirname) {
	if (wildly_verbose) fprintf(stderr,"cp: OPENDIR(%s)\n",dirname);
	return(opendir(dirname));
}
int _closedir(DIR *dirp) {
	if (wildly_verbose) fprintf(stderr,"cp: CLOSEDIR(dirp)\n");
	return(closedir(dirp));
}
int _close(int fildes) {
	if (fildes != fileno(stdin)) {
		if (wildly_verbose) fprintf(stderr,"cp: CLOSE(fildes)\n");
		return(close(fildes));
	} else return(0);
}

int parse_options(int argc, char**argv, char *options, char *source) 
{
	int error=0;
	int opt;
	char  *endptr;

	/* reset getopt */
#ifndef __QNXNTO__
	optind=0;
#else
	optind=1;
#endif

	while ((opt = getopt(argc,argv,options)) != -1) {
		switch(opt) {
			/* 1003.2 options */
			case 'f': force_del				= TRUE;				 break;
			case 'i': interactive			= TRUE;				 break;
			case 'p': dashp					= TRUE;
					  cp_timemode			= TRUE;
					  if (!geteuid()) cp_ownergroup = TRUE;
					  break;
			case 'R': special_recursive     = TRUE;
			case 'r': recursive				= TRUE;				 break;

            /* extended options */
#ifdef SUPPORT_NET_OPTIMIZATION
			case 'O': 
					/* we don't want to pay any attention to this if
                       we _are_ the agent copy of cp! */
					if (argv[0][0]!='(')
						net_optimized       = TRUE;				 break;
#endif
			
			case 'A': preserve_atime        = TRUE;              break;
			case 'B': buf_size				= FALLBACK_BUF_SIZE; break;
			case 'W': wildly_verbose		= TRUE;
			case 'V': super_verbose			= TRUE;
			 		  if (isatty(fileno(stdout))) super_verbose_percent = TRUE;
			case 'v': verbose				= TRUE;				 break;
			case 'D': ignore_device			= TRUE;				 break;
			case 'n': copy_newest			= TRUE;				 break;
			case 'N': ignore_device			= FALSE;
			case 'c': create_paths			= TRUE;				 break;
			case 'd': distribute			= TRUE;				 break;
			case 's': safe 					= TRUE;				 break;
			case 'x': copy_only_if_exists	= TRUE;				 break;
			case 'X': dont_copy_if_exists	= TRUE;				 break;
			case 'l': levels				= strtol(optarg,&endptr,10);
					  if (((levels==LONG_MAX || levels==LONG_MIN)&&errno) || (*endptr && !isspace(*endptr)) || (levels<0)) {
						fprintf(stderr,TXT(T_INVALID_LEVELS),optarg);
						error++;
					  }
					  break;
			case 'L': preserve_links		= TRUE;				 break;
			case 't': cp_timemode			= FALSE;			 break;
#if SUPPORT_OLD_OPTIONS
			case 'a': if (patmodule_enter(P_AFTERDATE,optarg)==-1) exit(1); break;
			case 'b': if (patmodule_enter(P_BEFOREDATE,optarg)==-1) exit(1); break;
			case 'g': if (patmodule_enter(P_GROUP,optarg)==-1) exit(1); break;
			case 'm': if (patmodule_enter(P_MODE,optarg)==-1) exit(1); break;
			case 'P': if (patmodule_enter(P_PATTERN,optarg)==-1) exit(1); break;
			case 'u': if (patmodule_enter(P_OWNER,optarg)==-1) exit(1); break;
#endif
			case 'M': if (!strcmp(optarg,"unix")) standard_recursive=TRUE;
			          else if (!strcmp(optarg,"qnx")) standard_recursive=FALSE;
				      else {
                         fprintf(stderr,"%s: '-M %s' not valid. Choices are '-M unix' or '-M qnx' only.\n",argv[0],optarg);
						 error++;
					  }
					  break;

			default:  error++; break;
		}
	}

#ifdef SUPPORT_NET_OPTIMIZATION
	if (preserve_atime && net_optimized) {
		fprintf(stderr,"%s: -A and -O may not be used together\n",argv[0]);
		error++;
	}
#endif

	if (error && NULL!=source) fprintf(stderr,"[in %s]\n",source);
	return error;
}

#ifdef CONFIG_OR_ENVAR
int create_arglist(char *line, char **simulated_argv, char *source)
{
	int n,simulated_argc=1;
	int inside_quote = 0, inside_dblquote = 0;
	char tokenbuf[OPTION_TOKEN_MAXLEN];
	char *p=line;


	if (wildly_verbose)
		fprintf(stderr,"cp: Creating simulated argv[] list from '%s' in %s\n",line,source);

	/* build the command line into an argv-like array */

	for (n=1;n<SIMULATED_ARGV_MAXITEMS;n++) simulated_argv[n] = NULL;
	simulated_argv[0] = "cp";

	n = strlen( p );
    if(n<1) return 0;

	if (line[n-1]=='\n')
		line[--n] = '\0';
	if (n<1) return 0;

	inside_quote = inside_dblquote = 0;
	for (;*p;p++) {
		char *w;

		while(*p && isspace(*p)) p++;

		tokenbuf[0] = 0;
		for (w=tokenbuf;*p;p++) {
			if (!inside_quote && !inside_dblquote && isspace(*p))
				break;

			if (!inside_quote && !inside_dblquote && *p=='\\') {
				/* backslash something */
				p++;
				if (!*p) break;
			} else if (*p=='\'') {
				if (!inside_dblquote) inside_quote=!inside_quote;
				continue;
			} else if (*p=='\"') {
				if (!inside_quote) inside_dblquote=!inside_dblquote;
				continue;
			}
	
			*w = *p;
			w++;
		}

		*w=0;

		if (tokenbuf[0]) {
			if (wildly_verbose) fprintf(stderr,"cp: simulated_argv[%d]='%s'\n",simulated_argc,tokenbuf);
			simulated_argv[simulated_argc] = malloc(strlen(tokenbuf)+1);
			snprintf(simulated_argv[simulated_argc], strlen(tokenbuf)+1, "%s", tokenbuf);
			(simulated_argv[simulated_argc])[strlen(tokenbuf)] = 0;
			simulated_argc++;
		}

		if (!*p) break;
	} /* parse tokens in this line */

	if (inside_quote || inside_dblquote) {
		 if (inside_quote) {
			fprintf(stderr,"cp: no ending \' in '%s'\n",source);
		 }
		 if (inside_dblquote) {
			fprintf(stderr,"cp: no ending \" in %s\n",source);
		 }
		
		 return -1;
	}

    return simulated_argc;
}
#endif


#ifdef ENVAR
int parse_envar()
{
	static char *vlist[SIMULATED_ARGV_MAXITEMS];
	int num_items;
	char *options;

	if (NULL==(options=getenv("CP_OPT"))) return 0;
	num_items=create_arglist(options,&vlist,"envar 'CP_OPT'");
	if (num_items==-1) return -1;	/* error */
	if (num_items) return parse_options(num_items, vlist, ENVAR_OPTIONS,"envar 'CP_OPT'");
	return 0;	
}	
#endif

#ifdef CONFIG_FILE
int parse_config_file()
{
	static char *vlist[SIMULATED_ARGV_MAXITEMS];
	char options[MAXIMUM_CONFIGFILE_DATA];
	char buf[PATH_MAX+16];
	int num_items;
    FILE *fp;

	if (NULL==(fp=fopen(CONFIGFILENAME,"r"))) return 0;

	options[0]=0;
	while (fgets(buf, sizeof(buf), fp)!=NULL) {
		char *newline;

		if (buf[0]=='#') continue;
		if (newline=strchr(buf,'\n'))
			*newline=0;

		strncat(options," ",sizeof(options));
		strncat(options,buf,sizeof(options));
	}
	
	fclose(fp);

	num_items=create_arglist(options,&vlist,"file "CONFIGFILENAME);
	if (num_items==-1) return -1;	/* error */

	if (num_items) return parse_options(num_items, vlist, CONFIG_OPTIONS,"file "CONFIGFILENAME);
	return 0;	
}	
#endif


char * shorten_filename(char * filename, char *buf) {
	memcpy(buf, filename, SHORT_FILENAME_LEN/2);
	buf[SHORT_FILENAME_LEN/2] = '\0';
	strcat(buf, "...");
	memcpy(&buf[SHORT_FILENAME_LEN/2] + 3, &filename[strlen(filename)
		- SHORT_FILENAME_LEN/2], SHORT_FILENAME_LEN/2);
	buf[strlen(buf)] = '\0';
	return buf;
}


/*------------------------------------------------------------ main ---------
	See POSIX 1003.2 draft 9 for functional description of utility.
---------------------------------------------------------------------------*/

int
main (int argc,char **argv, char **arge)
{
	int16_t error=0;
	int16_t file;
	int16_t first_index, last_index; 	/* argv[index]->src files on cmd line */
#ifndef __MINGW32__
	_amblksiz=256;
#endif

	levels = MAX_LEVELS;
	strict = (getenv(PSTRICT_ENVAR)!=NULL);
	cp_timemode		= !strict;
#ifdef __QNXNTO__
/* make this the default....too many device boundaries in NTO */
	ignore_device	= TRUE; 
#else
	ignore_device	= strict; 
#endif
	#ifdef OCP
	standard_recursive = FALSE;
	#else
	#ifdef NCP
	standard_recursive = TRUE;
	#else
	standard_recursive = FALSE;
	#endif
	#endif

	setvbuf(stdout,NULL,_IOLBF,STDOUT_BUFLEN);
	setvbuf(stderr,NULL,_IOLBF,STDERR_BUFLEN);

#ifdef SUPPORT_OLD_OPTIONS
	/* initialize pattern module for 16 conditions of each category */
	if (patmodule_init(MAX_PATTERNS,MAX_GROUPS,MAX_OWNERS,MAX_MODES)==-1) {
		perror("pmi");
		exit(EXIT_FAILURE);
	}
#endif
	#ifdef CONFIG_OR_ENVAR
	if (!strict) {
		#ifdef CONFIG_FILE
		/* parse config file options */
		error+=parse_config_file(); 
		#endif
	
		#ifdef ENVAR
		/* parse ENVAR options */
		error+=parse_envar();
		#endif
	}
	#endif
	
	/* parse command line options */
    error+=parse_options(argc,argv,OPTIONS,NULL);

	if (super_verbose_percent==TRUE) {
		#if defined(__QNXNTO__) || defined(__MINGW32__)
		verbose_period=65536L;		/* 1 update/second for unknown types */
		#else
		struct _dev_info_entry info;

		verbose_period=65536L;		/* 1 update/second for unknown types */

		/* find out the tty type */
		if (-1!=dev_info(fileno(stdout),&info)) {
				if (!strcmp(info.driver_type,"console")) verbose_period=4096L; /* 16 updates/s */
				else if (!strcmp(info.driver_type,"serial")) verbose_period=131072L; /* 1 update/2s */
				else if (!strcmp(info.driver_type,"netline")) verbose_period=32768L; /* 2 updates/s */
  				else if (!strcmp(info.driver_type,"pseudo")) verbose_period=16384L;   /* 4 updates/s */
		}
       #endif
	}

#ifdef SUPPORT_OLD_OPTIONS
	/* this will check for all negative patterns having been supplied,
	   and will add the positive pattern '*' if this is the case */
	patmodule_commit();
#endif

	first_index = optind;
	if (!distribute) last_index  = argc-2;   
	else {
		/* scan for 'to' keyword seperating destinations from source files */
		int i;
		last_index = 0;
		for (i=first_index+1;i<argc;i++) {
			if (argv[i][0]=='t' && argv[i][1]=='o' && argv[i][2]=='\000' )
				last_index = i-1;
		}

		/* if no 'to' found, act like a normal cp */
		if (last_index==0) {
			last_index = argc-2;
			distribute=FALSE;	
		} else if (last_index==(argc-2)) {
			fprintf(stderr,TXT(T_NO_DESTINATION));
			exit(EXIT_FAILURE);
		}
	}

	num_files = last_index - first_index + 1;

	if (distribute) {
		first_dest = last_index+2;	/* skipping past 'to' */
		last_dest = argc-1;		
	} else {
		first_dest = argc - 1;
		last_dest = argc - 1;
	}

	if (num_files<1) fprintf(stderr,TXT(T_NEED_SRC_AND_DEST));
	if ((num_files<1) || error) exit(EXIT_FAILURE);

#ifdef SUPPORT_NET_OPTIMIZATION
	if (net_optimized) {
		nid_t			src_node, dst_node;
		struct finfo	*srcinfo, *dstinfo, *p;
		int				src_count=0, dst_count=0, num_src_nodes, num_dst_nodes;
		pid_t			pid;
		char			*cmdname=malloc(128);
		long			num_bytes;

		/* find out our super-root, to be prepended to any filenames
           starting with a single /, when invoking a remote cp for speed */

		snprintf(srstring, sizeof(srstring), "%s", qnx_prefix_getroot());
		if (!srstring[0]) snprintf(srstring, sizeof(srstring), "//%d",getnid());
		srstring[sizeof(srstring)-1] = 0;

		/* make sources full paths (kinda, at least, prepended with srstring) */
		srcinfo = expand_to_fullpaths(argv+first_index,num_files,FALSE);

		/* make destination(s) full paths */
		dstinfo = expand_to_fullpaths(argv+first_dest,last_dest-first_dest+1,TRUE);

		num_bytes = 0L;

		if ((cmdname!=NULL)&&(srcinfo!=NULL)&&(dstinfo!=NULL)) {

			for (num_src_nodes=0,p=srcinfo;p!=NULL;p=p->next) {
				if ((p->count+p->dir_count*15)>src_count) {
					src_count = p->count+p->dir_count*15;
					src_node  = p->node;
					num_bytes = p->num_bytes;
				}
				num_src_nodes++;
			}

			for (num_dst_nodes=0,p=dstinfo;p!=NULL;p=p->next) {
				if ((p->count+p->dir_count*15)>dst_count) {
					dst_count = p->count+p->dir_count*15;
					dst_node  = p->node;
				}
				num_dst_nodes++;
			}

			if (num_dst_nodes>1) {
				/* only if more than one destination node consider running
	               cp on the src node */
	            if ((num_src_nodes>1)&&(dst_count>15))  {
					/* if two or more destinations are on the same node */
					/* then definitely run at the destination */
					src_node = dst_node;
				}
			} else {
				if (src_node != getnid())
					src_node = dst_node;
			}

			if ((src_node!=getnid())&&((src_count>1)||(num_bytes>REMOTE_MINIMUM)))  {

				if (qnx_fullpath(cmdname,argv[0])==NULL || access(cmdname,X_OK)) strcpy(cmdname,"/bin/cp");
				argv[0]="(remote cp)";

				/* spawn cp on <src_node> (may be source or destination - best guess) */
				pid = qnx_spawn(0,NULL,src_node,-1,-1,0,cmdname,argv,arge,NULL,-1);
				if (pid!=-1) {
					int rc;

					if (verbose) {
						printf("cp: spawning %s on node %ld\n",cmdname,src_node);
						fflush(stdout);
					}

					wait(&rc);
					exit(WEXITSTATUS(rc));
				} else {
					fprintf(stderr,"cp: warning - cannot start %s on node %ld :%s\n",cmdname,src_node,strerror(errno));
				}
			}
		}
		/* fall through to normal copy */
	}
#endif /* QNX4 only */

	/* choose a copy buffer size. Use largest possible. */
	for (len=buf_size+HEAP_REQUIRED;(!bptr) && (len>HEAP_REQUIRED);) {
		bptr = malloc(len);
		if (!bptr) len-=1024;
	}

	if (bptr) {
		free(bptr);
		len-=HEAP_REQUIRED;
		bptr = malloc(len);
	} else {
		fprintf(stderr,string_format,TXT(T_NO_BUF_MEM));
		exit(EXIT_FAILURE);
	}
    /* ok - copy buffer now set up. size of copy buffer is len. */

	if ((len<buf_size) && super_verbose) fprintf(stdout,TXT(T_SMALL_BUFFER),buf_size,len);

	for (dest_index = first_dest ; dest_index <= last_dest ; dest_index++) {
		statbufsrc_valid = statbufdst_valid = FALSE;
		lstatbufsrc_valid = lstatbufdst_valid = FALSE;

		target = argv[dest_index];		
		if (target[0]=='/' && target[1]==0) target="/.";
             
		if (verbose&&distribute) fprintf(stdout,TXT(T_DESTINATION),target);

		if (_statdst(target)==-1) {
			if (errno!=ENOENT) {
				if (strlen(target) < PATH_MAX) 
					prerror(twostrpar,TXT(T_STAT_FAILED),target);
				else { 
					char buf[SHORT_FILENAME_LEN+4]; 
					shorten_filename(target, buf);
					prerror(twostrpar,TXT(T_STAT_FAILED),buf);
				}
				exit(EXIT_FAILURE);
			} else {	/* target does not exist. */
				if ((num_files > 1) && (!create_paths)) {
					fprintf(stderr,TXT(T_TARGET_MUST_BE_DIR),purty(target));
#ifdef DEBUG
                    fprintf(stderr,"zzx: num_files=%d\n",num_files);
#endif
					exit(EXIT_FAILURE);
				}

				if (num_files > 1) directory_target = TRUE;
				else {
				    /* only one file is being copied. is it a directory? */
					snprintf(source, sizeof(source), "%s", argv[first_index]);
					source[sizeof(source)-1] = 0;

					if ((source[0]!='-' || source[1]) && (_statsrc(source)!=-1)) {

						if (((S_ISLNK(lstatbufsrc.st_mode) && S_ISDIR(statbufsrc.st_mode)) || 
							S_ISDIR(lstatbufsrc.st_mode)) && recursive) {
							/* POSIX.2 cp algorithim 2.e:  If the directory dest_file
							 *   does not exist, it will be created with file permission
							 *   bits set to the same value as those of source_file, modified
							 *   by the file creation mask of the user if the -p option was
							 *   not specified, and then bitwise inclusively ORed with
							 *   S_IRWXU.  (Continues ...)
							 * Note that the below modes are only preserved with POSIX_STRICT,
							 *   and without -p.  Otherwise they are re-set later.
							 */
							mode_t old_mask, new_mode;
							umask((old_mask = umask(0)) & ~S_IRWXU); /* effects mkdir */
							new_mode = S_IRWXU | (0777 & statbufsrc.st_mode);

							directory_target = TRUE;
							if (!create_paths) {
								if (verbose_mkdir(target,new_mode)== -1) {
									prerror(twostrpar,TXT(T_CANT_MAKE_DIRECTORY),target);
									exit(EXIT_FAILURE);
								}
							} else {
								/*	here, we are copying a dir to a dir which
								 *	doesn't exist, we don't create the dest dir
								 * 	because create_paths is turned on.
                            	 *
								 *	The dir_to_dir flag is *not* (!) set? */
								snprintf(dest, sizeof(dest), "%s/z", target);
								dest[sizeof(dest)-1] = 0;
								
								if (qnx_create_path_to(dest)==-1) {
									prerror(twostrpar,TXT(T_CANT_MAKE_DIRECTORY2),target);
									exit(EXIT_FAILURE);
								} else if (verbose) fprintf(stdout,TXT(T_MADE_PATH),target);
								chmod(target, new_mode);
								
							}
							umask(old_mask);

							dir_to_dir = TRUE;

							if (dirlevel_created<=Max_levels) {
								dir_mode_tab[dirlevel_created] = (dashp?07777:0777) & statbufsrc.st_mode;
								dir_uid_tab[dirlevel_created] = statbufsrc.st_uid;
								dir_gid_tab[dirlevel_created] = statbufsrc.st_gid;
								dir_time_tab[dirlevel_created].actime= statbufsrc.st_atime;
								dir_time_tab[dirlevel_created].modtime= statbufsrc.st_mtime;
								dirlevel_created++;
							} else dirlevel_warning();
 						} else {
							/* not a directory */
							char *p;
							directory_target = FALSE;

							if ((p=strrchr(target,'/'))) {
								if (p[1]==0 || strcmp(p,"/.")==0 || strcmp(p,"/..")==0) {
		                           directory_target = TRUE;
								}
							}
						}
	                } else {
						char *p;
						directory_target = FALSE;
						if ((p=strrchr(target,'/'))) {
							if (p[1]==0 || strcmp(p,"/.")==0 || strcmp(p,"/..")==0) {
	                           directory_target = TRUE;
							}
						}
					}
				} /* end else number of files copied to directory that doesn't exist is 1 */


				if ((directory_target) && (!dir_to_dir)) {
					if (create_paths) {
						snprintf(dest, sizeof(dest), "%s/z", target);
						dest[sizeof(dest)-1] = 0;						

						if (qnx_create_path_to(dest)==-1) {
							prerror(twostrpar,TXT(T_CANT_MAKE_DIRECTORY2),target);
							exit(EXIT_FAILURE);
						} else if (verbose) fprintf(stdout,TXT(T_MADE_PATH),target);
					} else {
						fprintf(stderr,TXT(T_USEDASHC),purty(target));
						exit(EXIT_FAILURE);
					}
				}
			}
		} else {
			/* destination exists already - stat/lstat succeeded */
			if (S_ISDIR(statbufdst.st_mode)) directory_target = TRUE;
			else if (num_files > 1) {
				fprintf(stderr,TXT(T_TARGET_MUST_BE_DIR),purty(target));
				exit(EXIT_FAILURE);
			} else { /* destination exists, is not a directory */
				directory_target = FALSE;
			}
		}

	    /*  If directory_target, we have already created it by this point.
		 *	Save away device & inode of the directory so we don't enter
         *  into an infinite recursion i.e. if we hit this dir as a SOURCE,
         *  we won't descend into it. */
		if (directory_target && recursive) {
			if (!statbufdst_valid) {
				if (_statdst(target)==-1) {
					prerror(twostrpar,TXT(T_STAT_FAILED),target);
                    inc_errs("stat()");
					continue;
				}
			}
			/* STAT, not LSTAT info - this should catch subtle infinite
				recursion possibilities involving symbolic links */
			dest_dev   = statbufdst.st_dev;
			dest_ino   = statbufdst.st_ino;
			/* fprintf(stderr,"dest_dev = %ld, dest_ino = %ld\n",dest_dev,dest_ino); */
		}
	
		for (file=first_index,source_device=-1;(file<=last_index)&&(!abort_flag);file++) {
			snprintf(source, sizeof(source), "%s", argv[file]);
			source[sizeof(source)-1] = 0;
			snprintf(dest, sizeof(dest), "%s", target);
			dest[sizeof(dest)-1] = 0;

			/* stdin special case */
			if (source[0]=='-' && !source[1]) {
				if (directory_target) {
					fprintf(stderr,"CP: stdin may only be copied to a file (not a directory)\n");
					exit(EXIT_FAILURE);
				}
					
				while (copy_guy(source,dest, sizeof(dest))==RETRY);
                break;	/* could be continue, makes no difference */
			}

			if (!statbufsrc_valid) {
				if (_statsrc(source)==-1) {
					if (errno==ENOENT)
						fprintf(stderr,twostrparnl,TXT(T_CANT_OPEN_SRC),purty(source));
					else {
						if (strlen(source) < PATH_MAX)
	        						prerror(twostrpar,TXT(T_STAT_FAILED),source);
					  	else { 
							char buf[SHORT_FILENAME_LEN+4]; 
							shorten_filename(source, buf);
							prerror(twostrpar,TXT(T_STAT_FAILED),buf);
						}
					}
					inc_errs("stat()");
					goto srcloopend;	/* was continue */
				}
			}

			if (S_ISDIR(lstatbufsrc.st_mode) || (S_ISLNK(lstatbufsrc.st_mode) && S_ISDIR(statbufsrc.st_mode))) {
				if (!directory_target) {	/* if src is a dir, dest must also be */
					fprintf(stderr,TXT(T_TARGET_MUST_BE_DIR),purty(dest));
					goto srcloopend;
				}
					
				if (!ignore_device)	source_device = statbufsrc.st_dev;
				if (recursive) {
					if (levels>0) {
						if ((statbufsrc.st_dev !=dest_dev) ||
							(statbufsrc.st_ino !=dest_ino)) {
							/* POSIX cp! When copying a directory to
							   'x', the directory is copied to x/'dirname',
							   which, if it doesn't exist must be created */

							if (!dir_to_dir && standard_recursive) {
								if (strlen(basename(dest))) strncat(dest,"/",sizeof(dest));
								strncat(dest,basename(source),sizeof(dest));
								dest[sizeof(dest)-1] = 0;

								/* invalidate our stat bufs since we just changed the dest filename */
								lstatbufdst_valid=statbufdst_valid=FALSE;
                    		}
	
							if (_statdst(dest)==-1) {
								if (errno==ENOENT) {
									if (copy_only_if_exists) {
										if (super_verbose) fprintf(stdout,TXT(T_SKIPPING_DIR),purty(dest));

										goto srcloopend;
									} else if (!try_hard_to_mkdir(dest, sizeof(dest))) {
										inc_errs("try_hard_to_mkdir(dest)");
										goto srcloopend;
									} else if (dirlevel_created<=Max_levels) {
										dir_mode_tab[dirlevel_created] = (dashp?07777:0777) & statbufsrc.st_mode;
										dir_uid_tab[dirlevel_created] = statbufsrc.st_uid;
										dir_gid_tab[dirlevel_created] = statbufsrc.st_gid;
										dir_time_tab[dirlevel_created].actime= statbufsrc.st_atime;
										dir_time_tab[dirlevel_created].modtime= statbufsrc.st_mtime;
										dirlevel_created++;
									} else dirlevel_warning();
		                        } else {
									prerror("cp: %s",dest);
									goto srcloopend;
								}
							}

							cur_level++;
							copy_directory_recursive(source,dest, sizeof(dest));
							cur_level--;
						} else {
							if (verbose) fprintf(stderr,TXT(T_SKIPPING_DIR),purty(source));
							/* errs++; */
						}
					}
				} else fprintf(stderr,TXT(T_CANTRECURSE),purty(source));
	
				goto srcloopend;
			} 

#ifdef SUPPORT_OLD_OPTIONS
			if (!patmodule_check(basename(source),&statbufsrc))	goto srcloopend;
#endif

			/* If we are copying to a directory, build full destination filename*/ 
			if (directory_target) {
				char *ptr;
	
				strncat(dest,"//",sizeof(dest));
				if ((ptr=strrchr(source,'/'))) strncat(dest,ptr+1,sizeof(dest));
				else strncat(dest,source,sizeof(dest));
				statbufdst_valid = lstatbufdst_valid = FALSE;	/* new name! */
				dest[sizeof(dest)-1] = 0;

				if ((!recursive) || (num_files==1) ||
					S_ISREG(statbufsrc.st_mode) ||
					S_ISBLK(statbufsrc.st_mode))
				{
					/* actually copy the file */
					while(copy_guy(source,dest, sizeof(dest))==RETRY);

				} else {
					if (special_recursive || S_ISBLK(statbufsrc.st_mode) || S_ISREG(statbufsrc.st_mode))
						while(copy_guy(source,dest, sizeof(dest))==RETRY);
					else {
						fprintf(stderr,twostrparnl,TXT(T_CANT_COPY_NONREG_FILE),source);
						inc_errs("can't_copy_nonreg_file");
					}
				}

				ptr = strrchr(dest,'/');
				*(--ptr) = (char) 0x00;
			} else {
				if ((!recursive) || (num_files==1) ||
					S_ISREG(statbufsrc.st_mode) ||
					S_ISBLK(statbufsrc.st_mode))
				{
					/* actually copy the file */
					while(copy_guy(source,dest, sizeof(dest))==RETRY);
				} else {
					if (special_recursive || S_ISBLK(statbufsrc.st_mode) || S_ISREG(statbufsrc.st_mode))
						while(copy_guy(source,dest, sizeof(dest))==RETRY);
					else {
						fprintf(stderr,twostrparnl,TXT(T_CANT_COPY_NONREG_FILE),source); 
						inc_errs("can't_copy_nonreg_file 2");
					}

				}
			}		
srcloopend:
			statbufsrc_valid  = statbufdst_valid = FALSE;
			lstatbufsrc_valid = lstatbufdst_valid = FALSE;
		}			
		if (preserve_links) clear_links(link_list);
	}						
	return (errs?EXIT_FAILURE:EXIT_SUCCESS);
}

#ifdef SUPPORT_NET_OPTIMIZATION
/*------------------------------------------------- expand_to_fullpaths() ---*/
struct finfo *expand_to_fullpaths(char **argv, int num_files, bool writeflag) 
{
	int i, fd;
	nid_t n;
	struct finfo *root, *curr, *p;
	char *s;
	static char fpath[PATH_MAX];

	if ((root = malloc(sizeof(struct finfo)))==NULL) return NULL;
	curr = root;
	curr->node = 0;
	curr->num_bytes = 0L;
	curr->next = NULL;

	for (i=0;i<num_files;i++) {
		if ((fd=full_open(argv[i],fpath,0))==-1) {
			/* if the dest didn't exist (so we can't get full path), 
               create a file there to get the full path, unlink it,
               and continue */
			if (writeflag && (errno == ENOENT)) {
				if ((fd=full_open(argv[i],fpath,O_WRONLY|O_CREAT|O_EXCL,511))==-1) {
					perror(argv[i]);
					return NULL;
				}
				else {
					if (_unlink(fpath)!=0) {
						fprintf(stderr,"cp: unlink of %s failed\n",fpath);
						return NULL;
					}
				}
			} else return NULL;
		}


		/* deal with super-root stuff for filenames starting with exactly one
           or more than two /s */

		if (argv[i][0]=='/' && (argv[i][1]!='/' || argv[i][2]=='/')) {
			if ((s=malloc(strlen(argv[i])+strlen(srstring)+2))==NULL) return NULL;
			sprintf(s,"%s/%s",srstring,argv[i]);
			if (wildly_verbose) fprintf(stderr,"cp: fullpath('%s')='%s'\n",argv[i],s);
			argv[i] = s;
		}

		/* fstat ok - we just want to know about the node its on
		   and whether it is a directory */
        if (fstat(fd,&statbufsrc)==-1) {
			close(fd);
			break;
		}
		statbufsrc_valid = FALSE;	/* for consistency - flag that we've
									   trashed anything that might have 
									   been in there previously */

		n = (nid_t) (statbufsrc.st_dev>>16);
		for (p=root;p;p=p->next) {		
			if (p->node == n) {
				if (S_ISDIR(statbufsrc.st_mode)) p->dir_count++;
				else {
					p->count++;
					if (!p->num_bytes) {
						p->num_bytes = statbufsrc.st_size;
					}
				}				
				break;
			}
		}

		if (!p) {
			/* this node not encountered before */
			if ((curr->next = malloc(sizeof(struct finfo)))==NULL) break;
			curr=curr->next;
			curr->count = curr->dir_count = 0;
			curr->next = NULL;
			curr->node = n;
			curr->num_bytes = 0L;
			if (S_ISDIR(statbufsrc.st_mode)) curr->dir_count++;
			else {
				curr->count++;
				/* record the size of the 1st file encountered for this node. If at
                   the end of the scan the count for this node is still ONE (and only
                   one), then cp will look at the num_bytes to determine if it is
                   worth the effort to spawn off */
				if (!curr->num_bytes) {
					curr->num_bytes = statbufsrc.st_size;
				}
			}
		}
		close(fd);
	}
	return(root->next);
}
#endif
		
/*--------------------------------------------------------- locate_link() ---*/

/* returns a link registration if there is a match, otherwise returns NULL */
struct link_reg *locate_link (struct link_reg *link_list,struct stat *statbufp)
{
	while (link_list!=NULL) {
		if (link_list->count &&
			(link_list->dev == statbufp->st_dev) &&
			(link_list->ino == statbufp->st_ino)) return(link_list);
		link_list = link_list->next;
	}
	return(NULL);
}

/*--------------------------------------------------------- create_link() ---*/

int create_link (struct link_reg *link_entry, char *destpath)
{
	int r;

	if (verbose) fprintf(stdout,TXT(T_LINK_MSG),link_entry->pathname,purty(destpath));

	if (link(link_entry->pathname,destpath)==-1) {
		prerror(TXT(T_LINK_FAILED),link_entry->pathname,destpath);
		r = -1;
	} else r = 0;

	if (!(--(link_entry->count))) free(link_entry->pathname);
	return r;
}

/*-------------------------------------------------------- register_link() ---*/
/* returns a pointer to the first registration entry in link_list */
struct link_reg *register_link ( struct link_reg *link_list, struct stat *statbufp, char * destpath) 
{
	struct link_reg *tmp;

	/* path MUST be the path at the DESTINATION, not the SOURCE. It is this that
	   a link will be created to, not the source! The dev and ino entries must
	   be those of the SOURCE, however. PAY ATTENTION! */
	
	if (link_list==NULL) {
		if ((link_list = malloc(sizeof(struct link_reg)))==NULL) return(link_list);
		link_list->next = NULL;
		link_list->pathname = NULL;
		link_list->count = 0;
	}

	tmp = link_list;
	while ((tmp->next!=NULL) && tmp->count) tmp = tmp->next;
	if (!tmp->count) {
		/* use this entry */
		tmp->dev = statbufp->st_dev;
		tmp->ino = statbufp->st_ino;
		if ((tmp->pathname = malloc(strlen(destpath)+1))) {
			strcpy(tmp->pathname,destpath);
			tmp->count = statbufp->st_nlink - 1;
		}
	} else {
		/* allocate a new entry */
		if ((tmp->next = malloc(sizeof(struct link_reg)))==NULL) return(link_list);
		tmp->next->count = 0;
		tmp->next->next = NULL;
		tmp->next->pathname = NULL;
		return(register_link( link_list, statbufp, destpath));
	}
	return(link_list);
}		

/*-------------------------------------------------------- clear_links() ---*/
void	clear_links (struct link_reg *link_list) 
{
	while (link_list!=NULL) {
		if (link_list->count) {
			if (link_list->pathname) free(link_list->pathname);
			link_list->count = 0;
		}
		link_list = link_list->next;
	}
}

/*-------------------------------------------------------- dir_exists() ----*/
bool dir_exists(char *dirname)
{
    if (_statdst(dirname)==-1) {
		if (errno!=ENOENT) prerror(twostrpar,TXT(T_STAT_FAILED),dirname);
		return(FALSE);
	} else {
		if (S_ISDIR(statbufdst.st_mode)) return(TRUE);
		else return(FALSE);
	}
}

/*---------------------------------------------------- try_hard_to_mkdir() ---*/

bool try_hard_to_mkdir(char *dest, size_t destsize)
{
	/* really loops when we run out of disk space
	   trying to create the new directory */
	for (;;) {
		mode_t old_mask;

		/* supposed to set to the same value as src file at this point,
           modified by existing umask, ored with S_IRWXU. YEch. I don't
           see how a test suite could verify this.. ;-) */
           
		old_mask = umask(~S_IRWXU);
		if (verbose_mkdir(dest,S_IRWXU)==-1) {
			int e=errno;
			umask(old_mask);
			if (e==EEXIST) return(TRUE);
			if (e==ENOSPC) {
				if (!_qnx_fullpath(tmp2pathfull, dest, destsize)) {
					prerror(TXT(T_FULLPATH_FAILED),purty(dest));
					return(FALSE);
				}

				if (insert_new_disk(tmp2pathfull,dest,TXT(T_OUT_OF_SPACE_DIR)))
					continue;
				else exit(EXIT_FAILURE);
			} else {
				errno=e;
				prerror(twostrpar,TXT(T_CANT_MAKE_DIRECTORY),purty(dest));
				return(FALSE);
			}
		} else {
			umask(old_mask);
			if (verbose) fprintf(stdout,TXT(T_MADE_DIR),purty(dest));
			return(TRUE);
		}
	}
}


/*------------------------------------------------------------- purty -------*/
/* strips multiple '/'s */
char *purty (char *string)
{
	static char purtystr[PATH_MAX+1];
	char *ptr=string, *prty = purtystr;
		
	/* copy over valid leading // only when not '///' */
	if (ptr[0]=='/'&&ptr[1]=='/'&&ptr[2]!='/') {
		*prty++=*ptr++;
		*prty++=*ptr++;
	}

	for (;*ptr;ptr++)	if (*ptr!='/'||*(ptr+1)!='/') *prty++=*ptr;
	*prty=*ptr;	/* null terminate */
	return(purtystr);
}
	
/*-------------------------------------------------------- removeable --------*/

bool removeable(filename)
char *filename;
{
#ifndef SUPPORT_REMOVABLE_MEDIA
	return FALSE;
#else
	int16_t fd;
	struct _disk_entry dbuf;
	char *ptr;

	ptr = strrchr(filename,'/');
	if (ptr!=NULL) 	*ptr = 0;
    if ((fd=verbose_open(filename,0,0))==-1) {
		if (ptr!=NULL) *ptr = '/';
		return(FALSE);
	} 
	if (ptr!=NULL) *ptr = '/';

	disk_get_entry(fd,&dbuf);
	_close(fd);

	if ((dbuf.disk_type == _FLOPPY) || (dbuf.disk_type == _REMOVABLE)) return(TRUE);
	return(FALSE);
#endif
}


/*------------------------------------------------------ interrogation code -*/
bool ask_user(src_filename,dst_filename,format)
char *src_filename;
char *dst_filename;
char *format;
{
	char answer[80];

	fprintf(stderr,TXT(T_FROM_TO),src_filename,purty(dst_filename));
	fprintf(stderr,format,src_filename,purty(dst_filename));
	fflush(stdout);
	fflush(stderr);
	fgets(answer,sizeof(answer),stdin);
	if (strlen(answer)>1) {
		if (!strncmp(answer,"yes",strlen(answer)-1)) return(TRUE);
		if (!strncmp(answer,"YES",strlen(answer)-1)) return(TRUE);
	}
	if (verbose) fprintf(stdout,TXT(T_SKIPPING),purty(src_filename));
	return(FALSE);
}

/*------------------------------------------------------ equal() ------------*/
int16_t equal(fname1,fname2)
char *fname1, *fname2;
{
	for (;*fname2;)	if (*fname1++ != *fname2++) return(0);
	return(*fname1?0:1);
}

/*------------------------------------------------ copy_directory_recursive() 
	copies a directory and all its contents. dst must be path to a directory.
	Called only from main() or recursively from itself.
-----------------------------------------------------------------------------*/

void copy_directory_recursive(src,dst,dstsize)
char *src,*dst;
size_t dstsize;
{
	DIR *dirp;
	struct dirent *entry;
	static int16_t slashesadded = 0;
	struct utimbuf dirtimes;

#ifdef DIAG
	fprintf(stderr,"copy_directory_recursive(%s,%s)\n",src,dst);
	fprintf(stderr,"statbufdst_valid = %d\n",statbufdst_valid);
	fprintf(stderr,"lstatbufdst_valid = %d\n",lstatbufdst_valid);
	fprintf(stderr,"dest is a %s file\n",S_ISDIR(lstatbufsrc.st_mode)?"directory":"non-directory");
#endif
    
	if (preserve_atime) {
		dirtimes.actime=lstatbufsrc.st_atime;
		dirtimes.modtime=lstatbufsrc.st_mtime;
	}
	
	if ((dirp=_opendir(src))==NULL) {
   		prerror(twostrpar,TXT(T_CANT_OPEN_DIRECTORY),src);
		inc_errs("opendir(src)");
		return;
	}

	while ((errno=0,entry=readdir(dirp)) && !abort_flag) {
		statbufsrc_valid = statbufdst_valid = FALSE;
		lstatbufsrc_valid = lstatbufdst_valid = FALSE;

		if (entry->d_name[0] == '.') {	/* skip . and .. */
			if (!entry->d_name[1]) continue;
			if ((entry->d_name[1]=='.') && (!entry->d_name[2])) continue;
			/* abc do I need this any more? */
			if (equal(entry->d_name,".bitmap") || equal(entry->d_name,".inodes")) {
				if (super_verbose) fprintf(stdout,TXT(T_SKIPPING2),purty(src),entry->d_name);
				continue;
			}
		}

		if (src[strlen(src)-1]!='/') {
			strcat(src,"/");
			slashesadded++;
		}

		strcat(src,entry->d_name);

		if (-1!=lstat_optimize(entry, &lstatbufsrc)) lstatbufsrc_valid = TRUE;
		else lstatbufsrc_valid = FALSE;

		if (_statsrc(src)==-1) {
			prerror(twostrpar,TXT(T_STAT_FAILED),src);
			if (slashesadded) {
				*(strrchr(src,'/')) = (char) 0x00;
				slashesadded--;
			} else *(strrchr(src,'/')+1) = (char) 0x00;
			inc_errs("_statsrc(src)");
			continue;
		}

		if ((source_device != -1) && (statbufsrc.st_dev!=source_device)) {
//			if (verbose) {
				fprintf(stderr,S_ISDIR(lstatbufsrc.st_mode)?TXT(T_SKIPPING_DIR):TXT(T_SKIPPING),
						purty(src));
				fprintf(stderr,"cp: (File resides on different device)\n");
				inc_errs("src file not on source device");
				/* so that a cp will return non-zero when it
                   doesn't copy due to device bounds. This is so
                   that mv doesn't erase across those same boundaries
                   when called from mv */
//			}
			if (slashesadded) {
				*(strrchr(src,'/')) = (char) 0x00;
				slashesadded--;
			} else *(strrchr(src,'/')+1) = (char) 0x00;
			continue;
		}			


		if (S_ISDIR(lstatbufsrc.st_mode)) {
			#ifdef DIAG
			fprintf(stderr,"GOT A DIRECTORY! (%s)\n",entry->d_name);
			#endif
			/*
			fprintf(stderr,"entry->d_name \ %s\n",entry->d_name);
			fprintf(stderr,"statbufsrc.st_dev = %ld\n",statbufsrc.st_dev);
			fprintf(stderr,"         dest_dev = %ld\n",dest_dev);
			fprintf(stderr,"statbufsrc.st_ino = %ld\n",statbufsrc.st_ino);
			fprintf(stderr,"         dest_ino = %ld\n",dest_ino);
			*/
			if (	(statbufsrc.st_dev==dest_dev) &&
					(statbufsrc.st_ino==dest_ino)		) {
				if (verbose) fprintf(stderr,TXT(T_SKIPPING_DIR),purty(src));
				/* errs++; */
			} else {
				if (cur_level<levels) {
					cur_level++;

					/* make a directory under destination of same name */
					strcat(dst,"/");
					strcat(dst,entry->d_name);
		   			if (!dir_exists(dst)) {	/* corrupts statbufdst */
						/* may have come back false because file existed
                           _but_ was not a directory */
						if (statbufdst_valid) {
							fprintf(stderr,TXT(T_PATH_IS_NONDIR),dst);
							inc_errs("path_is_nondir");
						} else if (!copy_only_if_exists) {
							if (!try_hard_to_mkdir(dst, dstsize)) exit(EXIT_FAILURE);
							else {
								if (dirlevel_created<=Max_levels) {
									dir_mode_tab[dirlevel_created] = (dashp?07777:0777) & statbufsrc.st_mode;
									dir_uid_tab[dirlevel_created] = statbufsrc.st_uid;
									dir_gid_tab[dirlevel_created] = statbufsrc.st_gid;
									dir_time_tab[dirlevel_created].actime= statbufsrc.st_atime;
									dir_time_tab[dirlevel_created].modtime= statbufsrc.st_mtime;
									dirlevel_created++;
								} else dirlevel_warning();
								copy_directory_recursive(src,dst,dstsize);
							}
						} else if (super_verbose) fprintf(stdout,TXT(T_SKIPPING_DIR),purty(dst));
					} else copy_directory_recursive(src,dst,dstsize);
					cur_level--;
					/* get rid of that addition to the path */
					*(strrchr(dst,'/')) = (char) 0x00;
				}
			}
		} else if (special_recursive || S_ISREG(statbufsrc.st_mode) || S_ISBLK(statbufsrc.st_mode)) {
#ifdef SUPPORT_OLD_OPTIONS
			if (patmodule_check(entry->d_name,&statbufsrc))	
#endif
			{
				strcat(dst,"/");
				strcat(dst,entry->d_name);
				while (copy_guy(src,dst,dstsize)==RETRY);
				*(strrchr(dst,'/')) = (char) 0x00;
			}
		} else {
			fprintf(stderr,twostrparnl,TXT(T_CANT_COPY_NONREG_FILE),src);
			inc_errs("can't_copy_nonreg 3");
		}

		if (slashesadded) {
			*(strrchr(src,'/')) = (char) 0x00;
			slashesadded--;
		} else *(strrchr(src,'/')+1) = (char) 0x00;
	}


	if (errno && !abort_flag) {
		prerror(twostrpar,TXT(T_READDIR_FAILED),src);
		inc_errs("readdir()");
	}

	if (_closedir(dirp)==-1) {
		prerror(twostrpar,TXT(T_CLOSEDIR_FAILED),src);
		inc_errs("_closedir()");
	}

	if (dirlevel_created) {
		dirlevel_created--;
		do {
			/* change file permissions on dest directory */
			if (cp_timemode) {
				if (super_verbose)
					fprintf(stdout,"cp: chmod(%s,%o)\n",dst,dir_mode_tab[dirlevel_created]);
				if (_chmod(dst,dir_mode_tab[dirlevel_created])==-1) {
					prerror(TXT(T_CHMOD_FAILED),dst,dir_mode_tab[dirlevel_created]);
					inc_errs("_chmod()");
					break;
				}

				if (super_verbose)
					fprintf(stdout,"cp: utime(%s,{%d,%d})\n",dst,dir_time_tab[dirlevel_created].actime,dir_time_tab[dirlevel_created].modtime);

				if (utime(dst,&dir_time_tab[dirlevel_created])==-1) {
					prerror("cp: utime(%s,{%d,%d})",dst,dir_time_tab[dirlevel_created].actime,dir_time_tab[dirlevel_created].modtime);
					inc_errs("utime()");
					break;
				}
			}
	
			/* change file uid, gid on dest directory */
			if (cp_ownergroup) {
				if (super_verbose)
					fprintf(stdout,"cp: chown(%s,%d,%d)\n",dst,dir_uid_tab[dirlevel_created],dir_gid_tab[dirlevel_created]);

				if (_chown(dst,dir_uid_tab[dirlevel_created],dir_gid_tab[dirlevel_created])==-1) {
					prerror("cp: chown(%s,%d,%d)",dst,dir_uid_tab[dirlevel_created],dir_gid_tab[dirlevel_created]);
					inc_errs("_chown()");
					break;
				}
			}

		} while (FALSE);
	}	

	if (preserve_atime) {
		if (wildly_verbose) {
			fprintf(stderr,"cp: UTIME(%s,&utm) actime=%ld, modtime=%ld\n",src,(long)dirtimes.actime,(long)dirtimes.modtime);
		}
		if (utime(src, &dirtimes)==-1) {
			prerror(TXT(T_UTIME_FAILED),purty(src));
			inc_errs("utime()");
		}
	}
}	

void change_dir_perms(char *path)
{
	int level;	

	if (!(cp_timemode || cp_ownergroup)) return;

	path[strlen(path)] = 0; /* double-null at end of path */
	*strrchr(path,'/') = 0; /* null out last /, i.e. we now have a directory */
	for (level=dirlevel_created;level;) {
		level--;
		do {
			/* change file permissions on dest directory */
			if (cp_timemode) {
				if (super_verbose)
					fprintf(stdout,"cp: chmod(%s,0%o)\n",path,dir_mode_tab[level]);
				if (_chmod(path,dir_mode_tab[level])==-1) {
					prerror("cp: chmod(%s,%o)",path,dir_mode_tab[level]);
					inc_errs("_chmod()");
					break;
				}

				if (super_verbose)
					fprintf(stdout,"cp: utime(%s,{%d,%d})\n",path,dir_time_tab[level].actime,dir_time_tab[level].modtime);

				if (utime(path,&dir_time_tab[level])==-1) {
					prerror("cp: utime(%s,{%d,%d})",path,dir_time_tab[level].actime,dir_time_tab[level].modtime);
					inc_errs("utime()");
					break;
				}
			}
	
			/* change file uid, gid on dest directory */
			if (cp_ownergroup) {
				if (super_verbose)
					fprintf(stdout,"cp: chown(%s,%d,%d)\n",path,dir_uid_tab[level],dir_gid_tab[level]);
				if (_chown(path,dir_uid_tab[level],dir_gid_tab[level])==-1) {
					prerror("cp: chown(%s,%d,%d)",path,dir_uid_tab[level],dir_gid_tab[level]);
					inc_errs("_chown()");
					break;
				}
			}
		} while (FALSE);
	
		*strrchr(path,'/') = 0;
	}

	/* restore path */                             
	for (level=dirlevel_created+1;level;level--) path[strlen(path)] = '/';
}

/*----------------------------------------------------------- open_file() --
	- code to open a file. Also unlinks first if force_del is true.
---------------------------------------------------------------------------*/

int16_t open_file(char *filename)
{
	int16_t rc;

	if (force_del) {
		if (_unlink(filename)==-1) {
			if (errno!=ENOENT) {
				prerror(TXT(T_CANT_UNLINK),purty(filename));
				return(-1);
			}
		}
	}

	if ((rc=verbose_open(filename,O_WRONLY|O_CREAT|O_TRUNC,statbufsrc.st_mode))==-1) {
		int e;

		e=errno;
		ranoutofspace = (errno==ENOSPC);
		if ((create_paths) && (errno==ENOENT)) {
			if (verbose) fprintf(stdout,TXT(T_CREATING_PATH_TO),purty(filename));
			if (qnx_create_path_to(filename)!=-1) {				
				rc=verbose_open(filename,O_WRONLY|O_CREAT|O_TRUNC,statbufsrc.st_mode);
				ranoutofspace = (errno==ENOSPC);
			} 
		}
		if (force_del&&(rc==-1)) prerror(twostrpar,TXT(T_CANT_OPEN_DST),filename);
		errno=e;
	}
	return rc;
}	

/*----------------------------------------------------- open_dest() -------
 	- open destination file named. Will call open_file.
-------------------------------------------------------------------------*/

int16_t open_dest(char *filename,char *source_filename,bool link_flag,bool *skipped)
{
	int16_t fd;

	*skipped=FALSE;

	/* statbufdst_valid only true here when dest file did not exist & has
       just been created (freshly_created == TRUE) */

	if (!statbufdst_valid) {
		fd=_statdst(filename);
	} else fd=0;
	
	/* if dest exists now, statbufdst_valid is TRUE. If statbufdst_valid
       is false, errno will have been set at the verbose_stat() call above.
    */

	if ((dont_copy_if_exists || copy_newest) && statbufdst_valid) {
		if ((statbufsrc.st_mtime<=statbufdst.st_mtime) || dont_copy_if_exists) {
			if (super_verbose) fprintf(stdout,TXT(T_SKIPPING),purty(filename)); 
			if (freshly_created!=-1) _close(out);
			*skipped=TRUE;
			return(out=-1);
		}
	} else if (copy_only_if_exists && !statbufdst_valid && errno==ENOENT) {
		if (super_verbose) fprintf(stdout,TXT(T_SKIPPING),purty(filename));
		if (freshly_created) _close(out);
		*skipped=TRUE;
		return(out=-1);
	} else if (safe && !freshly_created && statbufdst_valid && saccess(&statbufdst,W_OK)) {
		if (super_verbose) fprintf(stdout,TXT(T_SKIPPING),purty(filename));

		if (freshly_created) _close(out);
		*skipped=TRUE;
		return(out=-1);
	}

#ifdef DIAG
	if (copy_only_if_exists) {
		if (statbufdst_valid) {
			printf("DIAG: Statbuf is valid for '%s'\n",filename);
		} else {
			printf("DIAG: Statbuf not valid for '%s', but errno!=ENOENT (%s)\n",strerror(errno));
		}
	}
#endif
	
	if (link_flag) {
		if (freshly_created) {
			_close(out);
			out=-1;
			if (_unlink(filename)==-1) {
				prerror(TXT(T_CANT_UNLINK),purty(filename));
				inc_errs("unlink()");
				return(-1);
			}
			freshly_created = FALSE;
			return 0;	/* now it doesn't exist.. :-) */
		} else if (statbufdst_valid==FALSE && errno==ENOENT) {
			return 0;	/* yea! go ahead, dest doesn't exist */
		}
#ifdef DIAG
else printf("errno = %d (%s) ???\n",errno,strerror(errno));
#endif

		if (!force_del)
			if (!interactive || ask_user(source_filename,filename,TXT(T_LINKOVERWRITE))==FALSE)
				return(/*errs++,*/ -1);

		if (_unlink(filename)==-1) {
			prerror(TXT(T_CANT_UNLINK),purty(filename));
			inc_errs("unlink() 2");
			if (out!=-1) _close(out);
			return(out=-1);
		} else return(0);	
		/* never */
	}

	if (!freshly_created) {
	 	if (interactive) {
			if (statbufdst_valid) {
				if (ask_user(source_filename,filename,TXT(T_OVERWRITE))==TRUE) {
			        if ((fd=open_file(filename))==-1) {
						if (!force_del) {
							force_del++;
							fd=open_file(filename);
							force_del--;
						} 
					}
				} else fd = (*skipped=TRUE, /*errs++,*/ -1);
			} else if (errno==ENOENT) {
				if ((fd=open_file(filename))==-1) {
					prerror(twostrpar,TXT(T_CANT_OPEN_DST),filename);
					inc_errs("open_file()");
				}
			} else {
				prerror(twostrpar,TXT(T_STAT_FAILED),filename);
				inc_errs("stat() 2");
			}
		} else {
			if ((fd=open_file(filename))==-1) {
				if (errno==EACCES) {
					if (!force_del) {
						if (interactive && statbufdst_valid) {
							/* will be valid if it existed */
							if (ask_user(source_filename,filename,TXT(T_NOWRITEFORCE))==TRUE) {
								force_del++;
								fd=open_file(filename);
								force_del--;
							} else fd = (*skipped=TRUE,/*errs++,*/ -1);
						} else {
							prerror(twostrpar,TXT(T_CANT_OPEN_DST),filename);
							fd=(inc_errs("open_file()"),-1);
						}
					}
				} else if (fd==-1) {
					prerror(twostrpar,TXT(T_CANT_OPEN_DST),filename);
					inc_errs("open_file() 2");
				} else fd = -1;
			} 
		}
	} else {
		fd = out;
    }

	return(fd);
}

#if !defined(__QNXNTO__) && !defined(__MINGW32__)
struct _timesel __far *fish_qnx_time(void)
{
	static struct _timesel __far *tptr = NULL;

	if (tptr == 0L) {
		struct _osinfo *osdata;

		osdata = alloca(sizeof(struct _osinfo));	/* on stack! */
		if (qnx_osinfo(0,osdata)==-1) return(NULL);
		tptr = MK_FP(osdata->timesel,0);
	}

	return(tptr);
}
#endif
#if !defined(__MINGW32__)

// We don't really need this on our win32 host

#if !defined(__QNXNTO__) 
unsigned long fish_secnsec (unsigned long *res)
{
	static struct _timesel __far *ptr=NULL;
	time_t resultsec, resultnsec;
	static time_t startsec, startnsec;
	unsigned long   result;

	if (ptr==NULL) {
		ptr=fish_qnx_time();
	}

	do {
		resultsec = ptr->seconds;
		resultnsec= ptr->nsec;
	} while (resultsec!= ptr->seconds || resultnsec!=ptr->nsec);

	if (res!=NULL) {
		long deltansec;

		result=(resultsec-startsec);
		if (result>65535L) result=65535L;
		deltansec=(resultnsec-startnsec);

		if (deltansec<0) {
			result--;
			deltansec=1000000000L+deltansec;
		}		

		/* have a seconds difference in 'result', and a nanoseconds in
                                        'deltansec'

           nanoseconds will be between 0 and 999999999.

           we must scale this to fit in 16 bits; divide by 15259
		*/
           

		/* deltansec is now an unsigned, so can be safely shifted */
		deltansec/=15259;

		result=(result<<16)|deltansec;
		*res=result;
	} else {
		startsec=resultsec;
		startnsec=resultnsec;
		result=0;
	}

	return(result);
}
#else
#ifndef OLD_NEUTRINO
unsigned long fish_secnsec (unsigned long *res)
{
	struct timespec ptr;
	time_t resultsec, resultnsec;
	static time_t startsec, startnsec;
	unsigned long   result;

	clock_gettime(CLOCK_REALTIME, &ptr); // if this fails.. we won't think about it..

	resultsec = ptr.tv_sec;
	resultnsec= ptr.tv_nsec;

	if (res!=NULL) {
		long deltansec;

		result=(resultsec-startsec);
		if (result>65535L) result=65535L;
		deltansec=(resultnsec-startnsec);

		if (deltansec<0) {
			result--;
			deltansec=1000000000L+deltansec;
		}		

		/* have a seconds difference in 'result', and a nanoseconds in
                                        'deltansec'

           nanoseconds will be between 0 and 999999999.

           we must scale this to fit in 16 bits; divide by 15259
		*/
           

		/* deltansec is now an unsigned, so can be safely shifted */
		deltansec/=15259;

		result=(result<<16)|deltansec;
		*res=result;
	} else {
		startsec=resultsec;
		startnsec=resultnsec;
		result=0;
	}

	return(result);
}

#else
// old Neutrino 1.10 version where syspage was available
#define shl64(a) ((a).hi <<= 1, \
                  (a).hi |= (((a).lo) >> \
                  ((sizeof(unsigned int) * 8) - 1)), \
                  (a).lo <<= 1)

/* neutrino version */
/*
zzx make this standard Clock(); resoltion will be worse,
but with 1ms accuracy should be OK. If a very small number of
ticks result, display something to indicate that numbers are
approximate.
*/

unsigned long fish_secnsec (unsigned long *res)
{
    struct _my_clockcycle_struct {
		unsigned long lo, hi;
	} resultcycles, delta;

    static struct _my_clockcycle_struct startcycles;
    static unsigned long cycpersec;
	unsigned long   result;

	ClockCycles((struct _clockcycles*) &resultcycles);
	
	if (res!=NULL) {
#ifdef DIAG
		fprintf(stderr,"result.hi=%lu result.lo=%lu\n",resultcycles.hi,resultcycles.lo);
#endif

		if (resultcycles.hi>=startcycles.hi) {
			delta.hi=(resultcycles.hi-startcycles.hi);
		} else {
			delta.hi=(ULONG_MAX-startcycles.hi)+resultcycles.hi;
		}

		if (resultcycles.lo>=startcycles.lo) {
			delta.lo=(resultcycles.lo-startcycles.lo);
		} else {
			delta.hi--;
			delta.lo=(ULONG_MAX-startcycles.lo)+resultcycles.lo;
		}

#ifdef DIAG
		fprintf(stderr,"delta.hi=%lu delta.lo=%lu cycpersec=%lu\n",delta.hi,delta.lo,cycpersec);
#endif

		/* do long long divide by int into 16.16 fixed-point result */
		{
			int i;

			result=delta.hi/cycpersec;
	        delta.hi%=cycpersec;
			/* do 48 bits. This will result in a 16.16 fixed-point result */
            /* will be shifting zeros into delta.hi for the last 16 iterations */
			for (i=0;i<48;i++) {
				shl64(delta);
	            result<<=1;
				result+=delta.hi/cycpersec;
	            delta.hi%=cycpersec;
			}
		}
			
		*res=result;
	} else {
		/* will be called with res=NULL to reset */

		startcycles.hi=resultcycles.hi;
		startcycles.lo=resultcycles.lo;
		cycpersec=_syspage_ptr->qtimeptr->cycles_per_sec;
#ifdef DIAG
		fprintf(stderr,"start.hi=%lu start.lo=%lu cycpersec=%lu\n",startcycles.hi,startcycles.lo,cycpersec);
#endif
		result=0;
	}

#ifdef DIAG
	fflush(stderr);
#endif

	return(result);
}
#endif
#endif

#endif // ifdef __MINGW32__

void show_percent(long totalwritten, struct stat *statbufsrc, int lasttime) 
{
	static unsigned long otens, oones, odeci, ocenti, last_delta=0L;

	static char     last_string[80];
	static int		lastn;
	int             n;
	char            string[80];
	int             bitpos, inbytes=0;
	unsigned long   remainder, copied, hundreds, tens, ones, deci, centi,
                    totalsize, cps;
	unsigned long   deltatime, i;



	if (!S_ISCHR(statbufsrc->st_mode)) {

#ifdef __MINGW32__
		deltatime = time(NULL);
#else
		fish_secnsec(&deltatime);
#endif
		/* deltatime is fixed point seconds * 65536L */

		/* limit to 4 updates per second, except last time in */
		if (!lasttime && ((deltatime-last_delta)<verbose_period)) return;

		if (!S_ISBLK(statbufsrc->st_mode)) {
			totalsize=statbufsrc->st_size/512L;
			if (!totalsize && lasttime) {
				totalsize=statbufsrc->st_size;
				totalwritten=totalsize;
				inbytes=1;
			} else if (totalsize<8) {
				/* show files <4k in bytes */
				totalsize=statbufsrc->st_size;
				totalwritten*=512;	/* amount written will be in byte multiples */
				inbytes++;
			}
		} else totalsize=statbufsrc->st_size;

		if (!inbytes) {
			/* convert to kbytes */
			totalsize/=2;
			totalwritten/=2;
		}

		if (lasttime) totalwritten=totalsize;

		remainder = totalwritten;

		if (totalsize>(INT_MAX/10L)) {
			totalsize/=10L;
			remainder/=10L;
			deltatime/=10L;
		}
	
		copied=totalwritten;

		if (totalsize==0) return;
        else {
			hundreds = remainder/totalsize;
			remainder-=hundreds*totalsize;
	
			remainder*=10;
			tens=remainder/totalsize;
			remainder-=tens*totalsize;
	
			remainder*=10;
			ones=remainder/totalsize;
			remainder-=ones*totalsize;
	
			remainder*=10;
			deci=remainder/totalsize;
			remainder-=deci*totalsize;
	
			remainder*=10;
			centi=remainder/totalsize;
		}

#ifdef NEVER
	fprintf(stderr,"copied=%ld, deltatime=%ld.%03ld, last_delta=%ld.%03ld",copied, deltatime>>16, ((deltatime&0xFFFFL)*1000L)/65536L,last_delta>>16,((last_delta&0xFFFL)*1000L)/65536L);

#endif
       last_delta = deltatime;

		if (deltatime==0) deltatime=1;

        {
			for (cps=0,bitpos=16;bitpos>=0;bitpos--) {
				n=copied/deltatime;
				copied-=n*deltatime;
				cps+=n<<bitpos;
				copied<<=1;
			}			
		}

#ifdef NEVER
fprintf(stderr,"cps=%ld\n",cps);
#endif

		n=sprintf(string,"%c%1ld%1ld.%1ld%1ld%% (%ld/%ld %s, %ld %s/s)",hundreds?'1':' ',  tens, ones, deci, centi, totalwritten, totalsize, inbytes?"bytes":"kbytes", cps,inbytes?"bytes":"kb");

		if (lasttime || strncmp(string, last_string, n)) 
                {
			printf("%s",string);
			//for (i=n;i<lastn;i++) 
                         // printf(" ");
			if (!lasttime) 
                          for (i=0;i<n;i++) 
                            printf("\b");
			  else 
                            printf("\n");
			fflush(stdout);
			otens=tens; 
                        oones=ones; 
                        odeci=deci; 
                        ocenti=centi;
			snprintf(last_string, sizeof(last_string), "%s", string);
			last_string[sizeof(last_string)-1] = 0;
			lastn=n;
		}
	}
}

/*------------------------------------------------------- insert_new_disk */
bool insert_new_disk(char *disk_pathname,char *prompt_filename,char *errstring)
{
	/* check to see if media is not removeable. If not, 'oh well' */
	if (removeable(disk_pathname)) {
		/* update directory owners & permissions */
		change_dir_perms(disk_pathname);

		if (preserve_links) clear_links(link_list);

		if (isatty(fileno(stdin))) {
			for (;;) {
				fprintf(stderr,TXT(T_MEDIA_PROMPT),purty(prompt_filename));
				{
					char answer[16];
	
					fflush(stderr);
					fflush(stdout);
					if (fgets(answer,sizeof(answer),stdin)==NULL) return(FALSE);
					if (toupper(answer[0])=='N') {
						return(FALSE);
					} else {
						/* do not change dirlevel_created.. we are just bringing
                	       us back to the current point */
						if (qnx_create_path_to(disk_pathname)==-1) {
							prerror(TXT(T_CANT_CREATE_PATH),disk_pathname);
						} else return(TRUE);		/* OK to retry now */
					}
				}
			}
		} else return(FALSE);
	} else {
		/* update directory owners & permissions */
		change_dir_perms(disk_pathname);
		fprintf(stderr,errstring, disk_pathname);
		return(FALSE);
	}
}

/*------------------------------------------------------------ copy_guy() ---
	-	copies a file from src to dst.

	Called from main() and copy_dir_recursive().

	If something incredibly bad happens, this routine might exit the
	program. Otherwise, when something bad happens that is recoverable,
	the routine returns 0. If a retry is required (call again, same
	parameters), a -1 is returned.
---------------------------------------------------------------------------*/

int16_t copy_guy (src,dst, dstsize)
char *src, *dst;
size_t dstsize;
{
	int16_t			errs_in = errs;
	bool			register_link_on_completion = FALSE;
	bool			skipped=FALSE;
	struct link_reg	*link;
	int16_t			r, w, ww;
	struct utimbuf	utm;
	int16_t			rc=0;
	off_t			totalwritten=0, totalwritten_remainder=0;

	if (wildly_verbose)
		fprintf(stderr,"cp: copy_guy(%s,%s)\n",src,dst);

	if (super_verbose_percent) {
	#ifndef __MINGW32__
		fish_secnsec(NULL);	/* remember the start time of the copy */
	#endif
	}

	if (src[0]=='-' && !src[1]) {
		in = fileno(stdin);
		out=-1;
		if (fstat(in,&statbufsrc)!=-1) {
			lstatbufsrc = statbufsrc;	/* OK, this is stdin, can't be a link! */
			lstatbufsrc_valid = statbufsrc_valid = TRUE;
		} else {
			statbufsrc.st_mode = 0777;
			statbufsrc.st_ino = 0;
			statbufsrc.st_dev = 0;
		}
	} else {
		in = out = -1;
   
		if (!(special_recursive && S_ISLNK(statbufsrc.st_mode))) 
           in=verbose_open(src,O_RDONLY,0); /* only try when not a symlink */

		if (!statbufsrc_valid) {
			if (_statsrc(src)==-1) {
				prerror(twostrpar,TXT(T_STAT_FAILED),src);
				inc_errs("_statsrc()");
				goto exit_copy_guy;
			}
		}

		if (in==-1 && !(special_recursive && S_ISLNK(statbufsrc.st_mode))) {
			prerror(twostrpar,TXT(T_CANT_OPEN_SRC),src);
			inc_errs("_open()");
			goto exit_copy_guy;
		}

		#ifdef DIAG
		fprintf(stderr,"opened src. in=%d\n",in);
		#endif

	}		

	statbufdst_valid = FALSE;

	/* open if the dest DOESN'T already exist and -x not specified */
	if (
      (!copy_only_if_exists) &&
      (!(special_recursive && S_ISLNK(statbufsrc.st_mode)))
    ){
		if ((out = verbose_open(dst, O_WRONLY | O_CREAT | O_EXCL , statbufsrc.st_mode))==-1) {
			if (wildly_verbose) fprintf(stderr,"cp: (Failed; %s )\n",strerror(errno));
			freshly_created = FALSE;
			if (_statdst(dst)!=-1) {
				if ((statbufsrc.st_ino==statbufdst.st_ino)&&(statbufsrc.st_dev==statbufdst.st_dev)) {
					/* if (super_verbose) fprintf(stderr,"ino=%lx, dev=%lx\n",statbufsrc.st_ino,statbufdst.st_dev); */
					fprintf(stderr,TXT(T_CANT_COPY_TO_SELF),src,purty(dst));
					/* errs++; */
					goto exit_copy_guy;
				}
			}
		} else {	/* success */
			freshly_created = TRUE;
			if (_statdst(dst)!=-1) statbufdst_valid = TRUE;
		}
	} else {
		freshly_created = FALSE;
#ifdef DIAG
		fprintf(stderr,"DIAG: did not do O_CREAT open since copy_only_if_exists true,\n   or copying a symlink with -R\n");
#endif
	}

	if (special_recursive && !S_ISREG(lstatbufsrc.st_mode)) {
		if (out!=-1) {
			close(out);
			out=-1;
		}

		if (_unlink(dst)==-1) {
			/* don't treat file not existing as an error */
			if (errno!=ENOENT) {
				fprintf(stderr,TXT(T_CANT_UNLINK),purty(dst));
				inc_errs("unlink() 2");
				goto exit_copy_guy;
			}
		}
		
		lstatbufdst_valid = FALSE;

		if (S_ISFIFO(statbufsrc.st_mode)) {
			/* used to do an unlink here */

 			if (verbose) fprintf(stdout,TXT(T_VERBOSE_MKFIFO),purty(dst));
			if (mkfifo(dst,statbufsrc.st_mode)==-1) {
				fprintf(stderr,TXT(T_CANT_MKFIFO),purty(dst));
				inc_errs("mkfifo()");
				goto exit_copy_guy;
			}				

			if ((out = verbose_open(dst, O_RDONLY,0 ))==-1) {
				inc_errs("_open() 3");
				goto exit_copy_guy;
			}
			goto past_data_copy;
		}
		if (S_ISLNK(lstatbufsrc.st_mode)) {
			int n;

			if ((n=readlink(src,tmp2pathfull,sizeof(tmp2pathfull)-1))==-1) {
				fprintf(stderr,TXT(T_CANT_READLINK),purty(src),strerror(errno));
				inc_errs("readlink()");
				goto exit_copy_guy;
			}
			tmp2pathfull[n]=0;	/* null-terminate */

			/* used to do an unlink here */

	 		if (verbose) fprintf(stdout,TXT(T_VERBOSE_SYMLINK),purty(dst),tmp2pathfull);

			if (symlink(tmp2pathfull,dst)==-1) {
				fprintf(stderr,TXT(T_CANT_SYMLINK),purty(dst),tmp2pathfull,strerror(errno));
				inc_errs("symlink()");
				goto exit_copy_guy;
			}				
	
			out=-1; /* we will not be doing anything with the link */

			goto past_data_copy;
		}			

		if (S_ISCHR(lstatbufsrc.st_mode)) {
			fprintf(stderr,TXT(T_CANTDUPCHARSPECIAL),src);
			errs++;
		}
		if (S_ISBLK(lstatbufsrc.st_mode)){
			fprintf(stderr,TXT(T_CANTDUPBLOCKSPECIAL),src);
			errs++;
		}
		if (S_ISNAM(lstatbufsrc.st_mode)){
			fprintf(stderr,TXT(T_CANTDUPNAMEDSPECIAL),src);
			errs++;
		}
		if (S_ISSOCK(lstatbufsrc.st_mode)){
			fprintf(stderr,TXT(T_CANTDUPSOCKETSPECIAL),src);
			errs++;
		}

		/* Count this as an error */
		errs++;
		goto exit_copy_guy;
	} /* special recursive */

	if (preserve_links) {
		if (statbufsrc.st_nlink > 1) {
			/* see if there we already have one of those */
			if ((link = locate_link(link_list, &statbufsrc))!=NULL)	{
				if (freshly_created) {
					if (_unlink(dst)==-1) {
						prerror(TXT(T_CANT_UNLINK),purty(dst));
						inc_errs("unlink() 4");
						goto exit_copy_guy;
					}
				}
				statbufdst_valid = FALSE;
				lstatbufdst_valid = FALSE;
				freshly_created = FALSE;
				if (open_dest(dst,src,TRUE,&skipped)!=-1) {
					if (create_link(link,dst)==-1) inc_errs("create_link()");
					else goto exit_copy_guy;
				} else goto exit_copy_guy;
			} else register_link_on_completion = TRUE;
			/* fall through to normal copy. 'in' is still open */
		}
	}

	if (out==-1) {	/* out already exists or there is some problem, use big rtn */
		if ((out= open_dest(dst,src,FALSE,&skipped))==-1) {
			if (_close(in)==-1) {
				prerror(twostrpar,TXT(T_CLOSE_ERROR),src);
				inc_errs("_close()");
			}
			in = -1;

			if (ranoutofspace) {
				if (!_qnx_fullpath(tmp2pathfull,dst, dstsize)) {
					prerror(TXT(T_FULLPATH_FAILED),dst);
					exit(EXIT_FAILURE);
				}

				if (insert_new_disk(tmp2pathfull,dst,TXT(T_OUT_OF_SPACE_OPEN))) {
					rc = RETRY;
					in=out=-1;	/* to prevent attempt to close files */
					goto exit_copy_guy;
				} else exit(EXIT_FAILURE);
			} else if (!skipped) inc_errs("open_dest()");

			goto exit_copy_guy;
		}
	}


 	if (verbose) fprintf(stdout,TXT(T_VERBOSE_MSG),src,purty(dst));

	errno = 0;
	abort_flag = FALSE;

	#ifdef DIAG
	fprintf(stderr,"main loop: in=%d, len=%d\n",in,len);
	#endif
	if (super_verbose_percent) {
		show_percent(totalwritten, &statbufsrc,0);
	}

	for (r=read(in,bptr,len);r!=-1 && r>0 && !abort_flag;r=read(in,bptr,len)) {
		w=0;
		#ifdef DIAG
		fprintf(stderr,"read %d bytes\n",r);
		#endif

		do {
			#ifdef DIAG
			fprintf(stderr,"trying to write %d bytes\n",r-w);
			#endif
			w+=(ww=write(out,bptr+w,r-w));
			#ifdef DIAG
			fprintf(stderr,"wrote %d bytes (%d now of %d)\n",ww,w,r); 
			#endif
			if (ww==-1) break;
		} while (w<r);
	

		if (ww==-1) {
			if (errno == ENOSPC) {
				errno=0;
				if (!_qnx_fullpath(tmp2pathfull,dst, dstsize)) {
					prerror(TXT(T_FULLPATH_FAILED),dst);
					exit(EXIT_FAILURE);
				}

				/* close and unlink output file */
				if (_close(out)==-1) {
					prerror(twostrpar,TXT(T_CLOSE_ERROR),dst);
					abort_flag=TRUE;
				}

				if (!S_ISCHR(statbufsrc.st_mode) && !S_ISBLK(lstatbufdst.st_mode))
					if (_unlink(dst)==-1) {
						fprintf(stderr,TXT(T_CANT_UNLINK),purty(dst));
						fprintf(stderr,"\n");
					}

				/* close input file */
				if (_close(in)==-1) {
					prerror(twostrpar,TXT(T_CLOSE_ERROR),src);
					exit(EXIT_FAILURE);
				}

				if (insert_new_disk(tmp2pathfull,dst,TXT(T_OUT_OF_SPACE_WRITE))) {
					rc = RETRY;
					in=out=-1; /* prevent attempt to close in and out */
					goto exit_copy_guy;
				} else exit(EXIT_FAILURE);

				break;
			} else {	/* errno something other than enospc */
				prerror(twostrpar,TXT(T_WRITE_ERROR),dst);
				abort_flag=TRUE;
				inc_errs("write()");
				errno = 0;
			}
		}

		if (super_verbose_percent) {
			totalwritten_remainder += r%512L;
			totalwritten += r/512L + totalwritten_remainder/512L;
			totalwritten_remainder %= 512L;
			show_percent(totalwritten, &statbufsrc,0);
		}
	} /* loop */

	if (super_verbose_percent) {
		int errno_save = errno;
		if (abort_flag!=TRUE&&r!=-1) {
		    show_percent(totalwritten, &statbufsrc,1);
			fflush(stdout);
		} else {
			printf("\n");
			fflush(stdout);
		}
		errno=errno_save;
	}


	if (r==-1) {
		prerror(twostrpar,TXT(T_READ_ERROR),src);
		inc_errs("read()");
	}

	#ifdef DIAG
	fprintf(stderr,"finished copy loop\n");
	#endif

past_data_copy:
if (wildly_verbose)
	fprintf(stderr,"cp: past_data_copy errs_in =%d, errs = %d\n",errs_in, errs);

	/* preserve access time of original file if preserve_atime is set */
	if (preserve_atime && (statbufsrc_valid) && !S_ISLNK(lstatbufsrc.st_mode)) {
		utm.actime = statbufsrc.st_atime;
		utm.modtime= statbufsrc.st_mtime;

		if (wildly_verbose) {
			fprintf(stderr,"cp: __FUTIME(in,&utm) utm.atime=%ld, utm.mtime=%ld\n",(long)utm.actime,(long)utm.modtime);
		}
		#if !defined(__QNXNTO__) && !defined(__MINGW32__)
			if (__futime(in, &utm)==-1) {
				prerror(TXT(T_UTIME_FAILED),purty(src));
				inc_errs("__futime()");
			}
		#else
			if (utime(src, &utm)==-1) {
				prerror(TXT(T_UTIME_FAILED),purty(src));
				inc_errs("utime()");
			}
		#endif
	}

	if ((errs_in == errs) && (statbufsrc_valid) && (!special_recursive || !S_ISLNK(lstatbufsrc.st_mode))) {

		if (!statbufdst_valid || !lstatbufdst_valid ) _statdst(dst);

		/* update the times */
		if (cp_timemode) {
			/* if dest is block special, don't try to update the time to match
			   the original file, unless -p was specified */

			if (!S_ISLNK(lstatbufdst.st_mode) && (dashp || !( S_ISBLK(statbufdst.st_mode)||S_ISCHR(statbufdst.st_mode)))) {
								
				utm.actime = statbufsrc.st_atime;
				utm.modtime= statbufsrc.st_mtime;

				if (wildly_verbose) {
					fprintf(stderr,"cp: __FUTIME(out,&utm) utm.atime=%ld, utm.mtime=%ld\n",(long)utm.actime,(long)utm.modtime);
				}
#if !defined(__QNXNTO__) && !defined(__MINGW32__)
				if (__futime(out, &utm)==-1) {
					prerror(TXT(T_UTIME_FAILED),purty(dst));
					inc_errs("__futime()");
				}
#else
				if (utime(dst, &utm)==-1) {
					prerror(TXT(T_UTIME_FAILED),purty(dst));
					inc_errs("utime()");
				}
#endif

				if (freshly_created || dashp) {
					int mode;

					mode = statbufsrc.st_mode;

					if (statbufdst.st_uid!=statbufsrc.st_uid)
						mode &=~S_ISUID;

					if (statbufdst.st_gid!=statbufsrc.st_gid)
						mode &=~S_ISGID;

					/* copy mode except isuid and isgid bits */
					if (verbose_fchmod(out,mode)==-1) {
						/* to avoid error messages for .boot, .altboot when -t not
			               specified */
						if (dashp || strict || (errno!=EPERM)) {
							prerror(TXT(T_CHMOD_FAILED),dst,statbufsrc.st_mode);
							inc_errs("_fchmod()");
						}
					}
				}
			}
		}

		if (cp_ownergroup&&!S_ISLNK(lstatbufdst.st_mode)) {
			if (verbose_fchown(out,statbufsrc.st_uid,statbufsrc.st_gid)==-1) {
				if (dashp || strict || errno!=EPERM) {
					prerror(TXT(T_CHOWN_FAILED),purty(dst));
					inc_errs("_fchown()");
				}
			} else {
				/* if the chown succeeded, and s_isuid or s_isgid bits need
                   setting, try setting them now */
				if (statbufsrc.st_mode&(S_ISUID|S_ISGID)) {
					if (verbose_fchmod(out,statbufsrc.st_mode)==-1) {
						if (dashp || strict || (errno!=EPERM)) {
							prerror(TXT(T_CHMOD_FAILED),dst,statbufsrc.st_mode);
							inc_errs("_fchmod()");
						}
					}
				}
			}
		}

	}
	
	/* ignore dev/ino of symlink, even if copied version is not a symlink */
 	if ((errs_in == errs)&&(register_link_on_completion)&&statbufsrc_valid&&!S_ISLNK(statbufsrc.st_mode))
		link_list = register_link(link_list,&statbufsrc,dst);

exit_copy_guy:
if (wildly_verbose) fprintf(stderr,"cp: exit_copy_guy (rc=%d)\n",rc);

	statbufsrc_valid = statbufdst_valid = FALSE;
	lstatbufsrc_valid = lstatbufdst_valid = FALSE;
	freshly_created = FALSE;
	if (in!=-1) {
		if (_close(in)==-1) {
			prerror(twostrpar,TXT(T_CLOSE_ERROR),src);
			inc_errs("_close()");
		} 
	} 
	if (out!=-1) {
		if (_close(out)==-1) {
			if (errno==ENOSPC) exit(EXIT_FAILURE);
			prerror(twostrpar,TXT(T_CLOSE_ERROR),dst);
			inc_errs("_close()");
		} 
	}

	#ifdef DIAG
	fprintf(stderr,"returning %d\n",rc);
	#endif
	return(rc);	/* normally 0, could be RETRY though */
}
