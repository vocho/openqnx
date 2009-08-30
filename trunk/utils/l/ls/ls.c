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





/*---------------------------------------------------------------------
 *
 * $Id: ls.c 212041 2009-01-28 16:22:56Z aristovski@qnx.com $
 *
 * $Log$
 * Revision 1.50  2007/04/30 15:04:59  aristovski
 * PR: 46523
 * CI: rmansfield
 *
 * win32 dir. structure
 * Added WIN32_ENVIRON
 * Added #include <lib/compat.h>
 * added __MINGW32__ ifdef cases.
 *
 * Revision 1.49  2005/06/03 01:37:49  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.48  2003/10/13 20:43:27  jgarvey
 * Undo previous changes involving QNX4/QNX6 disagreement on st_size.
 * Since there are many more with this same assumption, and the disk
 * utilities already explicity use the new stat fields, I am going to
 * hit iofunc_stat() in the libc to bring it to match old QNX4 behaviour.
 *
 * Revision 1.47  2003/10/13 06:59:11  jgarvey
 * "ls" utility seems to assume that for S_ISBLK() devices that
 * "st_size" is in 512-byte units.  This is true for QNX4, but
 * in QNX6 the unit is "st_blocksize" (device/filesystem block
 * size, often but not always 512, eg CD-ROM, devf-*); the new
 * "st_blocks" field is always in 512-byte units, so use that.
 *
 * Revision 1.46  2003/08/26 16:52:40  martin
 * Add QSSL Copyright.
 *
 * Revision 1.45  2001/07/19 17:38:01  jgarvey
 * Optimise "ls" stat handling for Neutrino by using the new "dircntl()"
 * function to hint to the directory IO manager that full stat details
 * will be required and should be embedded into the readdir stream where
 * possible.  Tests on a typical development installation with "ls -laR"
 * show that the number of open/stat calls drops by 85% and the number
 * of readdir calls increases by 10%.  A similar optimisation was placed
 * into "find" as PR/3751.
 *
 * Revision 1.44  1999/08/16 20:45:17  bstecher
 * Clean up some warnings
 *
 * Revision 1.43  1999/04/20 18:49:20  jun
 * Fixed a typo (S_ISLNK -> S_ISSOCK).
 *
 * Revision 1.42  1999/04/12 13:37:12  thomasf
 * Added support for the union directories and files
 *
 * Revision 1.41  1999/04/02 20:35:17  bstecher
 * Did some clean up of the source.
 *
 * Revision 1.40  1999/02/09 17:31:33  peterv
 * Make neutrino ls work like unix. i.e. using specifying a symlink that
 * points to a directory shows the contents of the directory.
 * Display xpg4 characters for mode.
 *
 * Revision 1.39  1999/01/23 18:56:51  bstecher
 * added include file
 *
 * Revision 1.38  1998/11/26 20:58:06  eric
 * changed to have 128k stack for QNX 4. Note: this breaks the 16 bit
 * version(s), which can no longer be linked.
 *
 * Revision 1.37  1998/10/02 20:59:11  peterv
 * Added size checking
 *
 * Revision 1.36  1998/09/29 16:35:38  bstecher
 * handle 64-bit file sizes a bit cleaner
 *
 * Revision 1.35  1998/09/25 21:07:59  eric
 * got rid of ifdef'd-out optind resetting code
 *
 * Revision 1.34  1998/09/09 16:49:49  peterv
 * Fixed for neutrino file_offset_bits being 64-bits.
 * Cleaned up prototypes for compareing to make more ansi.
 *
 * Revision 1.33  1998/09/08 21:28:39  eric
 * *** empty log message ***
 *
 * Revision 1.32  1997/03/31 16:35:15  eric
 * modified to work with very large directories
 *
 * Revision 1.31  1997/03/14 14:58:28  eric
 * reworked to eliminate code differences between nto and qnx4 versions.
 * Needs libc43.a and libutil.a libraries.
 * (for nto, that is) Qnx4 version needs util lib.
 *
 * Revision 1.30  1997/02/26 16:09:08  eric
 * changed buffer (previous change) from 10 to 11 for null byte.
 *
 * Revision 1.29  1997/02/26 16:04:42  eric
 * changed the size of the string buffer used to store the ascii
 * version of unknown user and group ids from 6 bytes to 10 to
 * be able to handle larger 32-bit values. (struid() and strgid())
 *
 * Revision 1.28  1997/02/25 17:07:38  eric
 * now recognizes _FILE_GROWN flag which it will display in the
 * same spot that _FILE_BUSY would be displayed in an ls -l.
 *
 * Revision 1.27  1997/02/17 16:22:03  glen
 * [eric] changed name mapping fn to allow _POSIX_PATH_MAX pathnames
 * instead of NAME_MAX
 *
 * Revision 1.26  1996/12/18 22:52:11  glen
 * we would sometimes use a stat entry without it being valid,
 * and we'd base some allocs on st_size.. could be nasty.
 *
 * Revision 1.25  1996/12/02 21:51:54  peterv
 * Added back the sticky bit support
 *
 * Revision 1.24  1996/09/30 18:35:26  eric
 * minor nto tweak
 *
 * Revision 1.23  1996/09/16 18:45:46  eric
 * neutrino port, not cp'd from but borrowed changes that were
 * done in /src/nto/test/ls.c
 *
 * Revision 1.22  1996/06/20 15:32:04  eric
 * changed struid() and strgid() to take int instead of
 * unsigned. Changed sprintf in these functions to be a
 * %d instead of %u.
 *
 * Revision 1.21  1996/05/21 14:57:03  steve
 * Changed to only call setpwent or setgrent the first time it
 * needs an entry; often never.
 *
 * Revision 1.20  1996/03/15 15:22:38  mphunter
 * fixed useage message for -F (didn't mention symlinks)
 *
 * Revision 1.19  1994/10/03 14:09:55  steve
 * Changed size output (ls -l, ls -s) to be unsigned, primarily for
 * /dev/shmem.
 *
 * Revision 1.18  1994/01/21  22:19:29  brianc
 * Don't output trailing spaces at end of line
 *
 * Revision 1.17  1994/01/20  21:16:48  brianc
 * Use dev_size() instead of funny qnx_term_load()
 *
 * Revision 1.16  1994/01/20  20:45:06  brianc
 * Changed usage message
 *
 * Revision 1.15  1993/11/15  19:48:07  brianc
 * *** empty log message ***
 *
 * Revision 1.14  1993/11/04  20:40:49  brianc
 * Widened size field to accomodate /dev/shmem
 *
 * Revision 1.13  1993/06/08  20:00:46  dtdodge
 * Changed usage message.
 *
 * Revision 1.12  1993/05/06  21:19:18  brianc
 * Increased space for size of block devices
 *
 * Revision 1.11  1992/10/29  20:02:30  peterv
 * Less messages to get width.
 *
 * Revision 1.10  1992/10/29  19:15:06  peterv
 * Enhances width sensing. Now works better with windows.
 *
 * Revision 1.9  1992/10/27  17:10:34  eric
 * added usage one-liner
 *
 * converted xstat stuff to fsys_stat stuff
 *
 * Revision 1.8  1992/08/12  21:08:49  brianc
 * Added -L option which always resolves symlinks
 *
 * Revision 1.7  1992/07/09  10:18:28  garry
 * Editted usage
 *
 * Revision 1.6  1992/02/17  18:30:40  brianc
 * NUL terminate readlink() path
 *
 * Revision 1.5  1992/02/17  16:25:58  eric
 * char special files with ls -l now display --- -- --:-- if
 * the file time is zero. This indicates 'never used'.
 *
 * Revision 1.4  1992/02/11  18:19:05  brianc
 * Added support for symlinks, sockets and special-named files
 *
 * Revision 1.3  1991/09/04  01:30:21  brianc
 * *** empty log message ***
 *
 * Revision 1.2  1991/07/29  13:34:23  brianc
 * Superuser has -a option on by default
 *
 * Revision 1.1  1991/07/11  13:40:54  brianc
 * Initial revision
 *
 *
 * $Author: aristovski@qnx.com $
 *
---------------------------------------------------------------------*/

/* Usage message must be in comments so GCC etc don't try to parse it.
#ifdef __USAGE
%C - list directory contents (POSIX)

%C	[-1CFRacdilqrstu] [-DSbfghnopvx] [file ...]
Options:
 -1    Force output to be one entry per line.
 -C    Multi column output.
 -D    List directories only.
 -F    Display '/' after directories, '*' after executables, '|' after FIFOs, @ after symlinks.
 -L    Resolve symbolic links rather than showing them.
 -R    Recursively list subdirectories encountered.
 -S    Don't sort the output.
 -a    List directory entries whose names begin with a period.
 -b    Use length of file for sorting or printing.
 -c    Use time of last change for sorting or printing.
 -d    Do not treat directories differently than other file types.
 -f    Do not sort the output.
 -g    Like -l but don't show owner.
 -h    Display a header for 'l', 'n' and 'x' options.
 -i    Print serial number of file.
 -l    List in long format.
 -n    Same as '-l' except displays GID/UID numbers rather than names.
 -o    Like -l but don't show group.
 -p    Display relative pathnames (files only).
 -q    Display a '?' in place of non-printable chars (see LOCALE).
 -r    Sort in reverse order.
 -s    Display the number of file system bytes in units of 512.
 -t    Sort by time.
 -u    Use time of last access for sorting or printing.
 -v    List directories first.
 -x    Display file extent information.
#endif
*/

/* Usage message must be in comments so GCC etc don't try to parse it.
#ifdef __USAGENTO
%C - list directory contents (POSIX)

%C	[-1CFRacdilqrstu] [-DSbfghnopvx] [file ...]
Options:
 -1    Force output to be one entry per line.
 -C    Multi column output.
 -D    List directories only.
 -F    Display '/' after directories, '*' after executables, '|' after FIFOs, @ after symlinks.
 -L    Resolve symbolic links rather than showing them.
 -R    Recursively list subdirectories encountered.
 -S    Don't sort the output.
 -a    List directory entries whose names begin with a period.
 -b    Use length of file for sorting or printing.
 -c    Use time of last change for sorting or printing.
 -d    Do not treat directories differently than other file types.
 -f    Do not sort the output.
 -g    Like -l but don't show owner.
 -h    Display a header for 'l', 'n' and 'x' options.
 -i    Print serial number of file.
 -l    List in long format.
 -n    Same as '-l' except displays GID/UID numbers rather than names.
 -o    Like -l but don't show group.
 -p    Display relative pathnames (files only).
 -q    Display a '?' in place of non-printable chars (see LOCALE).
 -r    Sort in reverse order.
 -s    Display the number of file system bytes in units of 512.
 -t    Sort by time.
 -u    Use time of last access for sorting or printing.
 -v    List directories first.
#endif
*/

#define NDEBUG	1		/* Manifest to turn off DEBUG messages */

#define STATFN(p,b) (qx_opts & OPT_x ? fsys_stat(p, (void *) b) :\
					   qx_opts & OPT_L ?  stat(p, (void *) b) :\
					   lstat(p, (void *) b))

// structure to load in stat() type calls
#if defined(__QNXNTO__) || defined(__MINGW32__)
#define STAT stat
#else
#define STAT _fsys_stat
#endif

#ifndef __MINGW32__
#define SETGRENT() setgrent()
#define SETPWENT() setpwent()
#else   // __MINGW32__
#define SETGRENT() 
#define SETPWENT()
#endif

#include <util/stdutil.h>
#include <util/defns.h>
#include <util/util_limits.h>

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#ifndef __MINGW32__
#include <grp.h>
#endif
#include <malloc.h>
#ifndef __MINGW32__
#include <pwd.h>
#endif
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#ifndef __MINGW32__
#include <libgen.h>

#include <sys/disk.h>
#endif


#if defined(__QNXNTO__) || defined(__MINGW32__)

#if defined(__MINGW32__)
    #include <util/fs_qnx4_util.h>
#else
	#include <termios.h>
	#include <sys/fs_qnx4.h>
#endif
	
	#define _BLOCK_SIZE 		QNX4FS_BLOCK_SIZE
	#define _FILE_BUSY 			QNX4FS_FILE_BUSY
	#define _FILE_INODE 		QNX4FS_FILE_INODE
	#define _FSYS_CLEAN			QNX4FS_FSYS_CLEAN
	typedef qnx4fs_xtnt_t 		EXTENT_T;
	typedef qnx4fs_dir_entry_t	DIRENTRY_T;
	typedef unsigned char		NEXTENT_T;
#else
	#include <sys/fsys.h>
	#include <sys/dev.h>

	typedef _xtnt_t 			EXTENT_T;
	typedef _nxtnt_t			NEXTENT_T;
	typedef struct _dir_entry	DIRENTRY_T;
#endif

#include <sys/stat.h>

#include <lib/compat.h>
#include <util/stat_optimiz.h>

#include <inttypes.h>

#ifndef _POSIX_PATH_MAX
#define _POSIX_PATH_MAX UTIL_PATH_MAX
#endif

#ifndef _DIRBUF
#define _DIRBUF     (8)
#endif

#if _FILE_OFFSET_BITS-0 == 64
#define OFFT_FMT	"ll"
#else
#define OFFT_FMT	"l"
#endif

#if !defined(PATH_MAX) && defined(__MINGW32__)
#  define PATH_MAX _MAX_PATH
#endif

struct small_stat {				/* An abbreviated directory entry			*/
	ino_t			st_ino;	
	dev_t			st_dev;	
	off_t			st_size;
	dev_t			st_rdev;
	time_t			st_mtime;			/*	Time of last data modification	*/
	time_t			st_atime;			/*	Time last accessed				*/
	time_t			st_ctime;			/*	Time of last status change		*/
	mode_t			st_mode;			/*	see <sys/stat.h>				*/
	gid_t			st_uid;
	uid_t			st_gid;
	nlink_t			st_nlink;
	unsigned char	st_status;
    unsigned char   spare[3];
	long			fnum_blks;
	EXTENT_T		st_first_xtnt;	
    unsigned char   spare2[3];
	NEXTENT_T		st_num_xtnts;
	long			st_xblk;		
	char		   *symlink;			/* contents of symbolic link		*/
	char			fname[1];		
};

void	list			( char *fnames[], uint32_t ctr );
void	dump_file_info	( struct small_stat *sstat, uint16_t columns, char *dirname, int lastcolumn );
void 	free_list 		( struct small_stat **statlist, uint32_t	count );
void	dump_files 		( struct small_stat **statlist, uint16_t columns, uint32_t count, char *dirname );
int		get_directory	( char *dirname, bool disp_dirname );
void	sortit			( struct small_stat **head, unsigned size );
int	    get_args        ( int margc, char *margv[] );
char	*get_entry_perms( mode_t fmode );
char	get_entry_type	( struct small_stat *sstat );
char	*get_date_fmt	( struct small_stat *sstat );
char	*map_filename	( char *fname, mode_t mode );
void	copy_stat		( struct small_stat *dst, struct STAT *src, char *fname );
char	*my_calloc		( unsigned int nbytes );
void    *my_malloc (size_t);
void    *my_realloc(void*, size_t);
int		get_max_cols	( unsigned  max_name_len );
void    make_header     ( void );
int     fsys_stat       (char *, struct STAT *);
void dump_statlist(struct small_stat **statlist,unsigned int count);

void check_list(struct small_stat **statlist, unsigned int count, char *note);

/*
 *	POSIX defined command line options
 */
#define	OPT_C	0x0001	/* multi column output 								*/
#define	OPT_F	0x0002	/* append / dir, * executable, | fifo, @ symlink	*/
#define	OPT_R	0x0004	/* recursively list subdirectories					*/
#define	OPT_a	0x0008	/* list *all* dir entries ( including '.' and '..' 	*/
#define	OPT_c	0x0010	/* use create_time in file status 					*/
#define	OPT_d	0x0020	/* don't treat directory operands differently		*/
#define	OPT_i	0x0040	/* print serial number								*/
#define	OPT_l	0x0080	/* list long format									*/
#define	OPT_q	0x0100	/* map chars not in LOCALE to '?'					*/
#define	OPT_r	0x0200	/* reverse the sort order							*/
#define	OPT_s	0x0400	/* list size of file rounding up to 512 byte bound	*/
#define	OPT_t	0x0800	/* sort by time_modified							*/
#define	OPT_u	0x1000	/* sort by last_access_time							*/
#define	OPT_1	0x2000	/* force output to be 1 entry per line				*/

/*
 *	QNX extended command line options
 */
#define	OPT_S	0x0001	/* do not sort the output   						*/
#define	OPT_f	0x0001	/* do not sort the output							*/
#define	OPT_b	0x0002	/* sort by size (hard to chose mnemonic options)	*/
#define	OPT_h	0x0004	/* dump a header where appropriate					*/
#define	OPT_n	0x0008	/* same as -l except displays UID and GID numbers	*/
#define	OPT_p	0x0010	/* display relative filename paths					*/
#define	OPT_x	0x0020	/* display number of extents						*/
#define	OPT_D	0x0040	/* list directories only							*/
#define	OPT_g	0x0080	/* same as -l except don't display user name		*/
#define	OPT_o	0x0100	/* same as -l except don't display group name		*/
#define	OPT_v	0x0200	/* list directories first							*/
#define	OPT_A	0x0400	/* list *all* dir entries (except '.' and '..') 	*/
#define	OPT_L	0x0800	/* sames as -l except resolve symlinks				*/

#define	OPT_U	0x1000	/* show union directories (ie doubled up dirs)		*/


#define CMD_OPTS	"1CFRacdilqrstu**ADLSbfghnopvxU"	/* command line options	*/
/*
 *	These options do not require LS to do a STAT()
 */
#define NOSTAT_POPTS	( px_opts & (OPT_1|OPT_C|OPT_a|OPT_q) )
#define NOSTAT_QOPTS	( qx_opts & (OPT_S|OPT_f|OPT_h) )

/*----------- Error messages -----------*/
#define T_OUTMEM				"ls: Out of memory.\n"

#define FILL_WIDTH		4		/* fill spaces between filenames when -C	*/

/* mingw specific:
 */
#ifdef __MINGW32__
int     fsys_stat       (char *p, struct STAT *s)
{
   errno=ENOSYS;
   return -1;
}
#endif

/*
 *	Lengths for dump_file_info()
 */
#define LEN_INODE	7
#define LEN_NBLKS	8

#define ERR		-1
#define NOERR	0

#define SLASH	'/'
#define DOT 	'.'

char	*cwd[1] = { "." };

#define IS_DOT( c ) ( c==DOT )	

uint16_t	px_opts = 0;			/* holds POSIX cmd line option flags		*/
uint16_t	qx_opts = 0;			/* holds QNX EXT cmd line option flags		*/

struct	STAT	statbuf;		/* general purpose STAT entry buffer		*/

char	fullpath[UTIL_PATH_MAX+1];	/* full pathname of file					*/

uint16_t	xtra_spc = 0;			/* used in dump_file_info()					*/

struct name_cache {
	struct name_cache *next;	/* forward link */
    unsigned id;				/* user/group id */
	char *name;					/* user/group name */
} *users, *groups;

bool	simple_ls;				/* Simple LS w/o options ? */
char	*prog;					/* program name */

int 	exit_status;			/* set to >0 if an (any) error occurs */
int		screen_width;			/* from COLUMNS or default to 80 */

long total=0;	/* total bytes dynamic memory allocated */

int main ( int argc, char **argv )
{
	int 	nofiles;

	prog = basename(argv[0]);
	exit_status = EXIT_SUCCESS;

	if (getenv("COLUMNS"))
		screen_width = atoi(getenv("COLUMNS"));
	else
#if defined(__QNXNTO__) 
		tcgetsize(fileno(stdout), (int *) 0, &screen_width);
#elif !defined(__MINGW32__)
		dev_size(fileno(stdout), -1, -1, (int *) 0, &screen_width);
#endif
//TODO: NOTE: for mingw, there is a way to get console width, but for now we
// default to 80 (see bellow)

	if (screen_width <= 0)
		screen_width = 80;
	screen_width--;

	/*
	 * parse cmd line arguments
	 */
	if ( get_args( argc, argv ) == ERR ) 
		exit(EXIT_FAILURE);

	/*
	 * for each file/directory
	 */
	nofiles = argc-optind;
	if ( nofiles == 0 )
		list( cwd, 1 );
	else
		list( &argv[optind], nofiles );

	return exit_status;
}

char *struid(uid_t id)
{
	struct name_cache *tail = users;
	struct passwd *pwent;
	char *name, buff[11];

	if (tail == 0)
		SETPWENT();
	while (tail && tail->id != id)
		tail = tail->next;
	if (tail == 0 || tail->id != id) {
		if ((pwent = getpwuid(id)) == 0)
			sprintf(name = buff, "%d", id);
		else
			name = pwent->pw_name;
		if ((tail = my_malloc(sizeof *tail)) == 0)
			return name;
		tail->id = id;
		tail->name = strdup(name);
		tail->next = users;
		users = tail;
	}
	return tail->name;
}

char *strgid(gid_t id)
{
	struct name_cache *tail = groups;
	struct group *grent;
	char *name, buff[11];

	if (tail == 0)
		SETGRENT();

	while (tail && tail->id != id)
		tail = tail->next;
	if (tail == 0 || tail->id != id) {
		if ((grent = getgrgid(id)) == 0)
			sprintf(name = buff, "%d", id);
		else
			name = grent->gr_name;
		if ((tail = my_malloc(sizeof *tail)) == 0)
			return name;
		tail->id = id;
		tail->name = strdup(name);
		tail->next = groups;
		groups = tail;
	}
	return tail->name;
}

#if defined(__QNXNTO__) || defined(__MINGW32__)
void
check_lnk ( n, s )
char        *n;
struct STAT *s;
{
	struct STAT		buf;

	if(stat(n, &buf) != -1 && S_ISDIR(buf.st_mode))
		*s = buf;
}
#endif

void
list ( fnames, ctr )
char	*fnames[];
uint32_t	ctr;
{
	register struct small_stat *d;
	struct   small_stat	**statlist, **statlistd=NULL, **statlistf=NULL;
	uint32_t	 x, y, maxlen, count, len, columns, dctr, fctr;

	DBG( fprintf( stderr, "list()- nofiles=%d\n", ctr ) );

	maxlen = count = dctr = fctr = 0;

	statlist = (struct small_stat **) my_calloc(sizeof *statlist * ctr);

	for ( x=0; x < ctr; ++x )	{
		DBG( fprintf( stderr, "list()- got <%s>\n", fnames[x] ) );
#ifdef STATS
		printf("stat(%s)\n", fnames[x]);
#endif
		if ( STATFN( fnames[x], &statbuf ) == -1 ) {
			++exit_status;
			fprintf(stderr, "%s: %s (%s)\n", prog, strerror(errno), fnames[x]);
			continue;
		}

#if defined(__QNXNTO__) || defined(__MINGW32__)
		if (!(px_opts & OPT_l) && S_ISLNK(statbuf.st_mode))
			check_lnk(fnames[x], &statbuf);
#endif

		/*
		 *	If list directories only
		 */
		if ( (qx_opts & OPT_D) && (!S_ISDIR( statbuf.st_mode ) || ( px_opts & OPT_d ) ) )
			continue;

		/*
		 *	Mix of files and directories ?
		 */
		if ( S_ISDIR( statbuf.st_mode ) && !(px_opts & OPT_d ) )
			++dctr;
		else 
			++fctr;

		len = strlen(fnames[x]);
		d = (struct small_stat *) my_calloc(sizeof *d + len);

		copy_stat( d, &statbuf, fnames[x] );

		/* Read symbolic link if long format */
		if (px_opts & OPT_l && S_ISLNK(statbuf.st_mode)) {
			int n;
			if ((n = readlink(fnames[x], fullpath, sizeof fullpath)) != -1) {
				strncpy(d->symlink = my_malloc(n + 1), fullpath, n);
				d->symlink[n] = 0;
			}
		}

		statlist[ count++ ] = d;

		if (len > maxlen) maxlen = len;
	}

	if (count == 0)
		return;

	/*	
	 *	Sort the list of files/directories
	 */
	if ( count > 1 && !( qx_opts & OPT_S ) )
		sortit( statlist, count );
	
	if ( dctr )
		statlistd = ( struct small_stat ** ) my_calloc( dctr * sizeof(struct small_stat *) );
	if ( fctr )
		statlistf = ( struct small_stat ** ) my_calloc( fctr * sizeof(struct small_stat *) );
	/*	 * build files and directory lists
	 */
	for ( x=0, y=0, len=0; len < count; len++ )	{
		d = statlist[ len ];
		if ( statlistd!=NULL && S_ISDIR( d->st_mode ) && !( px_opts & OPT_d ) )
			statlistd[ x++ ] = d;
		else if (statlistf!=NULL)
			statlistf[ y++ ] = d;
	}

	free( statlist );
			
	columns = get_max_cols( maxlen );
/*
	setvbuf( stdout, NULL, _IOFBF, 2048 );
*/
	if ( qx_opts & OPT_h )
		make_header();

	/*
	 *	list all the files first
	 */
	if ( !(qx_opts & OPT_v) )	{
		if ( fctr )	{
			dump_files( statlistf, columns, fctr, "" );
			free_list( statlistf, fctr );
		}
	}
	/*
	 *	then list all the directories
	 */
	if ( dctr )	{
		for (x=0; x < dctr; x++ )
			get_directory( statlistd[ x ]->fname, ( (dctr>1) || (dctr&&fctr) ) );
		free_list( statlistd, dctr );
	}

	if ( qx_opts & OPT_v )	{
		if ( fctr )	{
			dump_files( statlistf, columns, fctr, "" );
			free_list( statlistf, fctr );
		}
	}
}

void make_header ()
{
	/*
	 *	Only valid for '-x' and '-l' options
	 */
	if ( !( (px_opts & OPT_l) || (qx_opts & OPT_x) ) )
		return;

	if (px_opts & OPT_i)
		printf( "%*s ", LEN_INODE, "Inode");

	if (px_opts & OPT_s)
		printf( "%*s ", LEN_NBLKS, "Blks");
	
	if (qx_opts & OPT_x)
        printf( "%9s %5s %6s %6s %6s %6s ", "Size", "xtnts", "xblk_1", "xsize", "xblk", "status");
	
	if ( px_opts & OPT_l ) {
		printf( "t-u--g--o- %2s ", "ln" );
		if ( (qx_opts & OPT_o) == 0 ) printf( "%-9s ", "Owner" );
		if ( (qx_opts & OPT_g) == 0 ) printf( "%-9s ", "Group" );
		printf( "%9s %-12s ", "Size", "Date" );
	}

	printf( "Filename\n" );
}

void
dump_file_info ( st, columns, dirname, lastcolumn )
struct  small_stat	*st;
uint16_t  columns;				/* No of columns to display 	*/
char	*dirname;
int		lastcolumn;
{
	register char	*fn;
	int freeflag;
	freeflag = FALSE;

	DBG( printf( "dump_file_info()- file is <%s>, col=%d\n", st->fname, columns ) );

	/*
	 *	-p means list relative file pathnames only ... no directories
	 */
	if ( (qx_opts & OPT_p) && S_ISDIR(st->st_mode) )
		return;

#ifdef PARTIAL
	if ( !(qx_opts & OPT_p) )	{
		fn = strrchr( st->fname+1, SLASH );	/* get fname only from entire path */
		if ( !fn )
			fn = st->fname;
		else
			++fn;
	}
	else	
#endif
		fn = st->fname;

    if ( px_opts & OPT_i )
        printf( "%*" OFFT_FMT "d ", LEN_INODE, st->st_ino );

    if ( px_opts & OPT_s )	
        printf( "%*" OFFT_FMT "u ", LEN_NBLKS, S_ISBLK(st->st_mode) ?
			   st->st_size : ((st->st_size + _BLOCK_SIZE - 1) / _BLOCK_SIZE));

    if (qx_opts & OPT_x) {
        printf( "%9" OFFT_FMT "u %5u %06lx %6ld %06lx  %c%c%c%c  ",
				st->st_size,
				st->st_num_xtnts,
				(long)st->st_first_xtnt.xtnt_blk,
				(unsigned long)st->st_first_xtnt.xtnt_size,
				st->st_xblk,
                (st->st_status&_FILE_BUSY)?'B':'-',
                (st->st_status&_FILE_INODE)?'I':'-',
                (st->st_status&_FSYS_CLEAN)?'C':'-',
#ifdef _FILE_GROWN
                (st->st_status&_FILE_GROWN)?'G':'-'
#else
				'-'
#endif
                );
	}

    if ( px_opts & OPT_l )    { 
		/*
		 *	Print <file mode> and <number of links>
		 */
		printf( "%c%s %2u ", get_entry_type( st ),
							get_entry_perms( st->st_mode ),
							st->st_nlink );
		/*
		 *	Print owner/group names
		 */
		if ( qx_opts & OPT_n ) {
			if ( (qx_opts & OPT_o) == 0 ) printf("%9u ", st->st_uid);
			if ( (qx_opts & OPT_g) == 0 ) printf("%9u ", st->st_gid);
		} else {
			if ( (qx_opts & OPT_o) == 0 ) printf("%-9s ", struid(st->st_uid));
			if ( (qx_opts & OPT_g) == 0 ) printf("%-9s ", strgid(st->st_gid));
		}

		/*
		 *	If character/block special display major/minor device numbers
		 */
		if (S_ISCHR(st->st_mode) || S_ISBLK(st->st_mode))
			printf("%4d, %3d ", major(st->st_rdev), minor(st->st_rdev));
		else
			printf("%9" OFFT_FMT "u ", st->st_size);		/* file size */
		
		printf("%12s ", get_date_fmt(st));		/* disp mod date */
	}

	if ( px_opts & (OPT_F | OPT_q) ){
		freeflag = TRUE;
		fn = map_filename( fn, st->st_mode );
	}

	if ( columns == 1 )	{
		printf( "%s%s", qx_opts & OPT_p ? dirname : "", fn );
		if ( px_opts & OPT_l && st->symlink )
			printf( " -> %s", st->symlink );
		putchar('\n');
	} else if ( lastcolumn )
		printf( "%s", fn );
	else
		printf( "%-*s", (screen_width / columns) - xtra_spc, fn );
	
	//Ensure we free our buffer if we used one for mapping.
	if (freeflag)
		free(fn);
}

int
get_directory ( dirname, disp_dirname )
char	*dirname;
bool	disp_dirname;	/* display the directory name -R */
{
	register int 	len;
	register struct small_stat *d;
	struct	 STAT		*stat_p;
	struct	 dirent		*dir_st;
	DIR		 *dirp;
	struct   small_stat	**statlist;
	char	 *p;
	uint16_t   columns;
	uint32_t   count,idx;
	ulong_t	 total = 0L;
	bool	 slash = FALSE;
	unsigned long no_entries;
#if defined(__QNXNTO__) || defined(__MINGW32__)
	int		 dirflags;
#endif

	DBG( printf( "get_directory()- dir is <%s>\n", dirname ) );

	dirp = opendir( dirname );			/* Read directory	*/
	if ( dirp == (DIR *)0 ) {
		fprintf(stderr, "%s: %s (%s)\n", prog, strerror(errno), dirname);
		++exit_status;
		return( ERR );
	}

#ifdef __QNXNTO__
	/* Hint to the Neutrino IO manager that we would like stat() details */
	if (!simple_ls && (dirflags = dircntl(dirp, D_GETFLAG)) != -1)
		dircntl(dirp, D_SETFLAG, dirflags | D_FLAG_STAT);
#endif

	count = idx = no_entries = 0;
	statlist = ( struct small_stat ** ) NULL;

	if ( dirname[ strlen( dirname )-1 ] == SLASH )
		++slash;

	while (errno=0, ( dir_st = readdir( dirp ) ) != NULL ) 	{
		//check_list(statlist,count+1,"[after readdir()]");
		sprintf( fullpath, "%s%s%s", dirname, slash ? "" : "/", dir_st->d_name );

        stat_p = NULL;

		/* want fsys_stat if OPT_x, lstat normally, stat if OPT_L */

		if (!simple_ls) {
			/* we need stat info */
  			if ((qx_opts&(OPT_x|OPT_L))==0) {
  				/* need lstat */
  				if (lstat_optimize(dir_st, (void*)&statbuf)!=-1) stat_p=&statbuf;
  			} else if ((qx_opts&OPT_x)==0) {
  				/* must be OPT_L ; need stat */
  				if (stat_optimize(dir_st, (void*)&statbuf)!=-1) stat_p=&statbuf;
  			}

		    if(stat_p == NULL) {
				if ( STATFN( fullpath, &statbuf ) == -1 ) {
					fprintf(stderr, "%s: %s (%s)\n", prog, strerror(errno), fullpath);
					++exit_status;
					continue;
				}
				stat_p = &statbuf;
			}
		} else {
			stat_p = &statbuf;
			statbuf.st_mode=0;

			statbuf.st_size=sizeof(DIRENTRY_T)*_DIRBUF*2;
		}
		//check_list(statlist,count+1,"[after stat()]");
		
		// if first char of filename is '.'   .  ..  .kshrc  .plan etc
		if ( IS_DOT( dir_st->d_name[0] ) )	{
			if (dir_st->d_name[1]==0) {
				// if directly is "." (current dir), and we have not
                // allocated the statlist table yet, allocate one which
                // is big enough to contain the number of directory
                // entries which will fit in the directory

				if ( statlist == (struct small_stat ** ) NULL ) 	{
					no_entries = (stat_p->st_size / sizeof( DIRENTRY_T )) +_DIRBUF;

					statlist = ( struct small_stat ** ) 
								my_calloc( no_entries * sizeof(struct small_stat *) );
				}
			}
		
			if ( (px_opts & OPT_a) == 0 &&
			   ( (px_opts & OPT_R) == 0 || !S_ISDIR(stat_p->st_mode) ) )
				continue;
		}
		//check_list(statlist,count+1,"[after DOT]");

		/*
         *  Check to make sure we haven't run out of room in the statlist
         *  table of file stat info. This is necessary since the original
         *  may not have been correctly sized or may not even be allocated
         *  yet.
		 *	[Not all admins set the "." entry size correctly
		 *	"." is not guaranteed to be the 1st entry]
		 */
		if ( count == no_entries )	{
			no_entries += _DIRBUF;		/* alloc _DIRBUF entries at a time */
			statlist = ( struct small_stat ** ) 
						my_realloc( statlist, no_entries * sizeof(struct small_stat *) );
			if ( statlist == (struct small_stat ** ) NULL )	{
				perror( T_OUTMEM );
				exit( 2 );
			}
		}
		//check_list(statlist,count+1,"[after statlist size check]");
		
		/*
		 *	If list directories only
		 */
		if ( (qx_opts & OPT_D) && (!S_ISDIR( stat_p->st_mode ) || ( px_opts & OPT_d ) ) )
			continue;

		len = strlen( dir_st->d_name );
		//check_list(statlist,count+1,"[before calloc d]");
		d = ( struct small_stat * )	my_calloc( sizeof( struct small_stat ) + len );

		//check_list(statlist,count+1,"[after calloc d]");
		/* Save the STAT info */
		copy_stat( d, stat_p, dir_st->d_name );
		//check_list(statlist,count+1,"[after copy_stat]");

		/* Read symbolic link if long format */
		if (px_opts & OPT_l && S_ISLNK( d->st_mode )) {
			int n;
			if ((n = readlink(fullpath, fullpath, sizeof fullpath)) != -1) {
				strncpy(d->symlink = my_malloc(n + 1), fullpath, n);
				d->symlink[n] = 0;
			}
		}
		//check_list(statlist,count+1,"[after reading symlink]");

		if (px_opts & (OPT_l | OPT_s))
			/* keep track of total blocks 	*/
			total += S_ISBLK(d->st_mode) ? d->st_size
				: ((d->st_size + _BLOCK_SIZE - 1) / _BLOCK_SIZE);

		//check_list(statlist,count+1,"[before statlist[count]=d]");

		statlist[ count ] = d;			/* and number of files */

		//check_list(statlist,count+1,"[after statlist[count]=d]");

		count++;

		if (len > idx) idx = len;
	}
	if (errno) {
		fprintf(stderr,"%s: readdir of '%s' failed (%s)\n",
			prog,  dirname, strerror(errno));
	}
	closedir( dirp );

	/*
	 *	-p means list relative file pathnames only ... nothing else !
	 */
	if ( !(qx_opts & OPT_p) )		{
		if ( disp_dirname || ( px_opts & OPT_R ) )	
			printf( "\n%s:\n", dirname );

		if ( px_opts & (OPT_l|OPT_s) )
			printf( "total %ld\n", total );
	}

/* BUG: ls -R should list all dirs... including empty ones */
	if ( !count )						/* if no files found, return	*/
		return( NOERR );

	if ( count > 1 && !( qx_opts & OPT_S ) )
		sortit( statlist, count );

	columns = get_max_cols( idx );
		
	dump_files( statlist, columns, count, dirname );

	/*
	 *	If recurse through the directory tree
	 */
    if ( px_opts & OPT_R )	{
		idx = 0;
		p = my_calloc( UTIL_PATH_MAX+1 ); /*better once here than many in the loop */
		do {
			d = statlist[ idx++ ];	
			if ( S_ISDIR( d->st_mode ) && !( px_opts & OPT_d ) &&
				!(d->fname[0]=='.' && d->fname[1]==0) &&
				!(d->fname[0]=='.' && d->fname[1]=='.' && d->fname[2]==0)
               ) 
			{
				sprintf( p, "%s%s%s", dirname, slash ? "" : "/", d->fname );
				get_directory ( p, TRUE );
			}
		} while ( idx < count );

		free( p );
	}

	/*
	 *	Free up memory now
	 */
	free_list( statlist, count );
	return( NOERR );
}

void
free_list ( statlist, count )
struct  small_stat	**statlist;
uint32_t	count;
{
	int		x;

	for ( x=0; x < count; ++x )
		free( statlist[x] );

	free( statlist );
}

void dump_statlist(struct small_stat **statlist, unsigned int count) {
	int i;
	int diff;
	fprintf(stdout,"dump_statlist: statlist=%4p\n",statlist);

	for (i=0;i<count;i++) {
		diff = ((unsigned)statlist[i+1])-((unsigned)statlist[i]);
		fprintf(stdout,"dump_statlist: statlist[%d]=%4p (%d bytes) ->name='%s'\n",
                i,statlist[i],diff,statlist[i]->fname);
     	
	}
}

void check_list(struct small_stat **statlist, unsigned int count, char *note)
{
	static char memorized[UTIL_PATH_MAX];

	if (count==1) {
		if (statlist && statlist[0]) {
			/* memorize statlist[count-1]->fname */
			strcpy(memorized,statlist[0]->fname);
			fprintf(stdout,"check_list: memorized '%s'\n",memorized);
		} else return;
	} else {
		if (strcmp(memorized,statlist[0]->fname)) {
			fprintf(stderr,"check_list: count=%d; statlist[0]->fname!='%s' (was '%s') %s\n",
				count,memorized,statlist[0]->fname,note?note:"");
			fprintf(stderr,"check_list: statlist[0]%8p, statlist[count]=%8p, &statlist[count]=%8p\n",
					statlist[0],statlist[count],&statlist[count]);
							
		}
	}
}

void
dump_files ( statlist, columns, count, dirname )
struct   small_stat	**statlist;
uint16_t   columns;
uint32_t   count;
char	 *dirname;
{
	uint32_t	c_count, st, ch, step, i;
	struct small_stat **unique_statlist = NULL;

	
	//Modifications for union file system, only works if sorted
	//We run the list and adjust the count here if duplicates removed
	unique_statlist = statlist;
	if (!(qx_opts & OPT_U)) {
		int oldcount = count;

		//This is brutal ... overallocate then copy pointers
		unique_statlist = (struct small_stat **)malloc(count * sizeof(struct small_stat *));

		//Override statlist and newcount if we can
		if (unique_statlist) {
			unique_statlist[0] = statlist[0];
			for (i=1, count=1; i<oldcount; i++) {
				if (strcmp(statlist[i]->fname, statlist[i-1]->fname) != 0) {
					unique_statlist[count] = statlist[i];
					count++;
				}
			}
		}
		else {
			unique_statlist = statlist;
		}
	}

	c_count = st = ch = 0;
	step = count / columns + 1;

	strcpy(fullpath, dirname);
	if (qx_opts & OPT_p) {
		i = strlen( dirname );
		if ( i && dirname[i-1] != SLASH ) 
			strcat(fullpath, "/");
	}

	
	while (ch < count) {
		dump_file_info( unique_statlist[ ch ], columns, fullpath, 
						   columns > 1 && (c_count + 1 >= columns || ch + step >= count) );

		if (px_opts & OPT_C) {
			ch += step;					/* Point to next file name			*/
			if ( (++c_count >= columns) || (ch >= count) ) {
				c_count = 0;
				if ( ++st < step ) 
					ch = st;
				if ( columns > 1 )
					putchar( '\n' );
			}
		}
		else
			++ch;
	}

	//Free the statlist if we had to allocate it
	if (unique_statlist && unique_statlist != statlist)
		free(unique_statlist);
}

int compare_fsize(const void *d1, const void *d2) {
	const struct small_stat *p1 = d1;
	const struct small_stat *p2 = d2;
	off_t rc = p2->st_size - p1->st_size;

	return rc < 0 ? -1 : rc > 0;
}

int compare_atime(const void *d1, const void *d2) {
	const struct small_stat *p1 = d1;
	const struct small_stat *p2 = d2;
    long rc = p2->st_atime - p1->st_atime;

	return rc < 0 ? -1 : rc > 0;
}

int compare_ctime(const void *d1, const void *d2) {
	const struct small_stat *p1 = d1;
	const struct small_stat *p2 = d2;
    long rc = p2->st_ctime - p1->st_ctime;

	return rc < 0 ? -1 : rc > 0;
}

int compare_mtime(const void *d1, const void *d2) {
	const struct small_stat *p1 = d1;
	const struct small_stat *p2 = d2;
    long rc = p2->st_mtime - p1->st_mtime;

	return rc < 0 ? -1 : rc > 0;
}

int compare_dirfile(const void *d1, const void *d2) {
	const struct small_stat *p1 = d1;
	const struct small_stat *p2 = d2;

	if ( S_ISDIR(p1->st_mode) != S_ISDIR( p2->st_mode ) )
		return( S_ISDIR(p1->st_mode) ? -1 : 1 );
	else
		return( 0 );
}

static int (*compare)(const void *, const void *);

int qcompare(const void *ptr1, const void *ptr2) {
	struct small_stat * const *p1 = ptr1;
	struct small_stat * const *p2 = ptr2;
	int rc = compare ? (*compare)(*p1, *p2) : 0;

	if (rc == 0) rc = strcmp((*p1)->fname, (*p2)->fname);
	return px_opts & OPT_r ? -rc : rc;
}

/*
 *	This will sort the list according to the command line sorting criteria.
 */
void sortit(struct small_stat **v, unsigned n) {
	
	if (px_opts & OPT_t) {
		if (px_opts & OPT_u)		compare = compare_atime;
		else if (px_opts & OPT_c)	compare = compare_ctime;
		else						compare = compare_mtime;
	} else	{
		if (qx_opts & OPT_b)		compare = compare_fsize;
		else if (qx_opts & OPT_v)	compare = compare_dirfile;
		else						compare = 0;
	}

	qsort((void *) v, (size_t) n, sizeof(*v), qcompare);
	//Run a unique filter here
}

int get_args ( int margc, char *margv[] )
{
	register int opt;

	/*
	 *	Force multi column output if not in STRICT POSIX environment
	 */
	if ( getenv( "POSIX_STRICT" ) != NULL || !isatty(1) ) {
		px_opts |= OPT_1;
	} else {
		px_opts |= OPT_C;
	}

	/* Show all files if super-user */
	if ( geteuid() == 0 ) px_opts |= OPT_a;

	while ( ( opt = getopt( margc, margv, CMD_OPTS ) ) != EOF )	{
		switch( opt )	{
			case '1':	px_opts |= OPT_1;   
						px_opts &= ~OPT_C;	break;
			case 'A':	qx_opts |= OPT_A; 	break;
			case 'C':	px_opts &= ~(OPT_1 | OPT_l);
						px_opts |= OPT_C;	break;
			case 'F':	px_opts |= OPT_F; 	break;
			case 'R':	if ( !(px_opts & OPT_d) )
						px_opts |= OPT_R; 	break;
			case 'U':
						qx_opts |= OPT_U;	break;
			case 'a':	px_opts |= OPT_a; 	break;
			case 'c':	px_opts |= OPT_c; 	break;
			case 'd':	px_opts |= OPT_d;
						px_opts &= ~OPT_R;	break;
			case 'i':	px_opts |= OPT_i; 	break;
			case 'l':	px_opts |= (OPT_l | OPT_1);
						qx_opts &= ~(OPT_o | OPT_g);
						px_opts &= ~OPT_C;
						qx_opts &= ~OPT_x;	break;
			case 'n':	px_opts |= (OPT_l | OPT_1);
						qx_opts &= ~(OPT_o | OPT_g);
						qx_opts |= OPT_n;	break;
			case 'q':	px_opts |= OPT_q; 	break;
			case 'r':	px_opts |= OPT_r; 	break;
			case 's':	px_opts |= OPT_s; 	break;
			case 't':	px_opts |= OPT_t; 	break;
			case 'u':	px_opts |= OPT_u; 	break;
			case 'D':	qx_opts |= OPT_D; 	break;		/* QNX XTD option */
			case 'L':	qx_opts |= OPT_L;	break;		/* QNX XTD option */
			case 'f':	/* the option from UNIX that -S emulates */
			case 'S':	qx_opts |= OPT_S;	break;		/* QNX XTD option */
			case 'v':	qx_opts |= OPT_v;	break;		/* QNX XTD option */
			case 'b':	qx_opts |= OPT_b;	break;		/* QNX XTD option */
			case 'g':	px_opts |= (OPT_1 | OPT_l);		/* QNX XTD option */
						qx_opts |= OPT_o;	break;
			case 'h':	qx_opts |= OPT_h;	break;		/* QNX XTD option */
			case 'o':	px_opts |= (OPT_1 | OPT_l);		/* QNX XTD option */
						qx_opts |= OPT_g;	break;
			case 'p':	qx_opts |= OPT_p;				/* QNX XTD option */
						px_opts |= OPT_1; 	break;
			case 'x':	px_opts &= ~(OPT_C | OPT_l);	/* QNX XTD option */
						px_opts |= OPT_1;
						qx_opts |= OPT_x;	break;
			default :	return ERR;
		}
	}

	/*
	 *	Set this for dump_file_info()
	 */
	xtra_spc = ((px_opts & OPT_s) ? LEN_NBLKS + 1 : 0)
			 + ((px_opts & OPT_i) ? LEN_INODE + 1 : 0);

	/*
	 *	Set SIMPLE flag
	 */
	if ( (NOSTAT_POPTS | px_opts) == NOSTAT_POPTS &&
		 (NOSTAT_QOPTS | qx_opts) == NOSTAT_QOPTS )
		simple_ls = TRUE;

	return( NOERR );
}

char *
get_entry_perms ( fmode )
mode_t	fmode;
{
	register char	*p;
	static 	 char	perms[3+3+3+1];

	p = perms;

	/* 	Handle USER permissions 	*/

	*p++ = ( char ) ( ( fmode & S_IRUSR ) ? 'r' : '-' );
	*p++ = ( char ) ( ( fmode & S_IWUSR ) ? 'w' : '-' );
	if ( fmode & S_ISUID )	{
		if ( fmode & S_IXUSR )
			*p++ = ( char ) 's';
		else
			*p++ = ( char ) 'S';
	}
	else
		*p++ = ( char ) ( ( fmode & S_IXUSR ) ? 'x' : '-' );

	/* Handle GROUP permissions 	*/

	*p++ = ( char ) ( ( fmode & S_IRGRP ) ? 'r' : '-' );
	*p++ = ( char ) ( ( fmode & S_IWGRP ) ? 'w' : '-' );
	if ( fmode & S_ISGID )	{
		if ( fmode & S_IXGRP )
			*p++ = ( char ) 's';
		else
			*p++ = ( char ) 'L';
	}
	else
		*p++ = ( char ) ( ( fmode & S_IXGRP ) ? 'x' : '-' );

	/* Handle OTHER permissions 	*/

	*p++ = ( char ) ( ( fmode & S_IROTH ) ? 'r' : '-' );
	*p++ = ( char ) ( ( fmode & S_IWOTH ) ? 'w' : '-' );
	if ( fmode & S_ISVTX )	{
		if ( fmode & S_IXOTH )
			*p++ = ( char ) 't';
		else
			*p++ = ( char ) 'T';
	}
	else
		*p++ = ( char ) ( ( fmode & S_IXOTH ) ? 'x' : '-' );

	*p = ( char ) NULL;
	return perms;
}

char
get_entry_type ( st )
struct small_stat	*st;
{
	char			fchar;

#ifdef DASH_L_SHOW_ST_STATUS
	if ( st->st_status & _FILE_GROWN )
		fchar = 'G';
	else if ( st->st_status & _FILE_BUSY )
		fchar = 'B';
	else
#endif
	if ( S_ISBLK( st->st_mode ) )
		fchar = 'b';
	else if ( S_ISCHR( st->st_mode ) )
		fchar = 'c';
	else if ( S_ISDIR( st->st_mode ) )
		fchar = 'd';
	else if ( S_ISFIFO( st->st_mode ) )
		fchar = 'p';
	else if ( S_ISLNK( st->st_mode ) )
		fchar = 'l';
	else if ( S_ISNAM( st->st_mode ) )
		fchar = 'n';
	else if ( S_ISSOCK( st->st_mode ) )
		fchar = 's';
	else
		fchar = '-';

	return( fchar );
}

char *
get_date_fmt(struct small_stat *st)
{
	static time_t now = 0;			/* only recompute time() once */
	static char date[14];			/* return buffer with date string */
	time_t then;
	long stale;

	if (now == 0) time(&now);
	then = (px_opts & OPT_u) ? st->st_atime :
	       (px_opts & OPT_c) ? st->st_ctime : st->st_mtime;
	if (!then && S_ISCHR(st->st_mode)) {
		/* Gord's special -- -- -- for char special files with times of 0 */
		strcpy(date,"--- -- --:--");
	} else {
		if ((stale = now - then) < 0)	stale = -stale;
#define SIX_MONTHS ((long) 6 * 30 * 24 * 60 * 60)
		strftime(date, sizeof date, stale < SIX_MONTHS ?
			"%h %d %H:%M" :  "%h %d  %Y", localtime(&then));
	}
	return date;
}

char *
map_filename ( fname, mode )
char 	*fname;
mode_t 	mode;
{
	register char	*p;
	static char	*mapped_name;
	mapped_name = malloc(PATH_MAX);

	snprintf( mapped_name, PATH_MAX, "%s", fname);

	if ( px_opts & OPT_q )	{
		p = mapped_name;
		for ( ; *p; p++ )
			if ( !isprint( *p ) )
				*p = '?'; 
	}

	if ( px_opts & OPT_F )	{
		p = &mapped_name[ strlen(mapped_name) ];
		if ( S_ISDIR( mode ) )
			*p++ = '/';
		else if ( S_ISFIFO( mode ) )
			*p++ = '|';
		else if ( S_ISLNK( mode ) )
			*p++ = '@';
		else if ( S_ISNAM( mode ) )
			*p++ = '#';
		else if ( S_ISSOCK( mode ) )
			*p++ = '=';
		else if ( ( S_IXUSR|S_IXGRP|S_IXOTH ) & mode )
			*p++ = '*';
		*p = NULL;
	}

	return( mapped_name );
}

void
copy_stat ( d, s, fname )
struct	small_stat	*d;
struct	STAT		*s;
char				*fname;
{
	strcpy( d->fname, fname );
	d->st_mode   = s->st_mode;

	if ( simple_ls )
		return;

	d->st_dev    = s->st_dev;
	d->st_ino    = s->st_ino;
	d->st_size   = s->st_size;
	d->st_rdev   = s->st_rdev;
	d->st_mtime  = s->st_mtime;
	d->st_atime  = s->st_atime;
	d->st_ctime  = s->st_ctime;
	d->st_uid    = s->st_uid;
	d->st_gid    = s->st_gid;
	d->st_nlink  = s->st_nlink;
#if !defined(__QNXNTO__) && !defined(__MINGW32__)
	d->st_status = s->st_status;

	d->st_first_xtnt.xtnt_blk  = s->st_first_xtnt.xtnt_blk;
	d->st_first_xtnt.xtnt_size = s->st_first_xtnt.xtnt_size;
	d->st_num_xtnts = s->st_num_xtnts;
	d->st_xblk = s->st_xblk;
#endif
}

char *
my_calloc ( nbytes )
unsigned int nbytes;
{
	register char	*p;
	static long total=0;

	total+=nbytes;

	p = (char *) calloc( nbytes, 1 );
	if ( p == NULL )	{
		fprintf( stderr, T_OUTMEM );
		fprintf( stderr,"[%ld bytes used]\n",total);
		exit( EXIT_FAILURE );
	}

	return( p );
}

void *my_malloc (size_t nbytes)
{
	return my_calloc(nbytes);
}

void *my_realloc (void *ptr, size_t size)
{
	register char	*p;

	p = (char *) realloc(ptr,size);

	if ( p == NULL )	{
		fprintf( stderr, T_OUTMEM );
//		fprintf( stderr,"[realloc: size=%d, _msize(ptr)=%d, total=%d]\n",
//                  size, _msize(ptr), total);
		exit( EXIT_FAILURE );
	}

	return( p );
}	

int
get_max_cols(unsigned maxnamlen)
{
	int cols;	

	if (px_opts & OPT_1) return 1;
	cols = screen_width / (maxnamlen + FILL_WIDTH + xtra_spc);
	return cols == 0 ? 1 : cols;
}
