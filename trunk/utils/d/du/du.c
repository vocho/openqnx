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




#ifdef __USAGE		/* du.c */
%C	- estimate disk space usage (POSIX)

%C	[-a|-s] [-ikpqx] [file]...
Options:
 -a        Generate a report for each file.
 -i        Count extra blocks used for >1 extent/file (slower)
 -k        Display output in Kbytes (default is 512 byte blocks).
 -p        Report space figure in bytes.
 -q        Supress nerrors messages.
 -r        Ignored as it is a default.  Warn about unaccesable files.
 -s        Display total sum only for each file named on cmd line.
 -x        Do not span device boundaries.

Note:

du figures disk usage by counting space occupied by files and directories.
It does not count overhead associated with these files such as separate extent
blocks, or extents which have more than 511 bytes unused.  To calculate space
remaining on the disk, the df utility should be used instead.

Block special and named special files will count as 0 towards the
total sum. However, if -a is specified their size will be reported
in their individual reports even though the number is not counted
in the total sum.
#endif

#ifdef __USAGENTO
%C	- estimate disk space usage (POSIX)

%C	[-a|-s] [-kpqx] [file]...
Options:
 -a        Generate a report for each file.
 -k        Display output in Kbytes (default is 512 byte blocks).
 -p        Report space figure in bytes.
 -q        Supress nerrors messages.
 -r        Ignored as it is a default.  Warn about unaccesable files.
 -s        Display total sum only for each file named on cmd line.
 -x        Do not span device boundaries.

Note:

du figures disk usage by counting space occupied by files and directories.
It does not count overhead associated with these files such as separate extent
blocks, or extents which have more than 511 bytes unused.  To calculate space
remaining on the disk, the df utility should be used instead.

The -i option available under QNX4 which accounts for extra extent blocks
by obtaining file extent information is not currently supported under
the Neutrino version.

Block special and named special files will count as 0 towards the
total sum. However, if -a is specified their size will be reported
in their individual reports even though the number is not counted
in the total sum.
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.22  2005/06/03 01:37:45  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.21  2003/10/13 20:43:27  jgarvey
	Undo previous changes involving QNX4/QNX6 disagreement on st_size.
	Since there are many more with this same assumption, and the disk
	utilities already explicity use the new stat fields, I am going to
	hit iofunc_stat() in the libc to bring it to match old QNX4 behaviour.
	
	Revision 1.20  2003/10/13 07:31:17  jgarvey
	Like "ls", "du" also made assumptions about S_ISBLK st_size units
	(512-bytes for QNX4, but st_blocksize for QNX6), so use the new
	field st_blocks which is defined to be in 512-byte units.
	
	Revision 1.19  2003/08/21 21:43:42  martin
	Update QSSL Copyright.
	
	Revision 1.18  1999/08/13 21:26:13  adrianj
	Added an ignored -r option for POSIX.  -r is already default behavior.
	
	Revision 1.17  1998/09/15 19:50:53  eric
	cvs
	
	Revision 1.16  1998/04/16 20:11:31  eric
	fixed bug where if last file in a dir was a hard link (links>1),
	the directory itself would be skipped after the last item was
	processed; also improved verbose diagnostic when files
	are skipped due to -x (now displays filename)

	Revision 1.15  1998/04/16 14:34:08  eric
	no change

	Revision 1.14  1997/02/13 21:08:27  eric
	now should detect readdir() errors

	Revision 1.13  1997/02/12 18:36:15  eric
	reduced number of ifdefs required to compile for nto

	Revision 1.12  1997/01/29 21:25:53  eric
	added nto usage message

	Revision 1.11  1996/11/12 19:58:35  eric
	removed nto reference to stdutil.h

	Revision 1.10  1996/11/12 19:01:13  eric
	nto port

	Revision 1.9  1996/08/21 20:38:21  eric
	now supports 4gigablock sums, does not count block special and
	named special files in the sum, and uses floating point to
	print out byte counts (vs the long ints for block or kbyte
	units)

	Revision 1.8  1996/08/21 17:43:02  steve
	dunno (eric)

 * Revision 1.7  1993/01/26  20:29:34  eric
 * modified usage message to fit in 80 columns
 *
 * Revision 1.6  1992/11/24  14:56:36  eric
 * now only prints 'too many links' message once (encounters too many
 * hard links to remember all of them(
 *
 * Revision 1.5  1992/10/28  15:59:16  eric
 * now calls fsys_stat instead of xstat
 *
 * Revision 1.4  1992/09/23  15:42:57  eric
 * fixed NULL pointer assignment problem when regular files are specified on the cmd line.
 *
 * Revision 1.3  1992/09/09  19:08:51  eric
 * added -k, -x; many fixes, now treats sym links ok.
 *
 * Revision 1.2  1992/03/07  17:29:41  eric
 * du was not checking return codes from fullpath - caused sigsegv
 * when you gave it a bad path
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *

	$Author: sboucher $
	
---------------------------------------------------------------------*/

/*--------------------------------------------------------- USAGE MESSAGE ----*/

/*--------------------------------------------------------- END OF USAGE ----*/

#include <util/stdutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
// #include <sys/fsys.h>
#include <errno.h>
// #include <cdefs.h>
#include <sys/types.h>
#include <util/defns.h>
#include <util/util_limits.h>
#include <util/stat_optimiz.h>

unsigned long process( char *, int ); /* returns number of 512byte blocks */

#define TXT(s)		s
#define T_FORMAT    "%13ld %s\n"
#define T_TOTAL 	"%13lu %s\n"
#define T_FLOAT     "%13.0f %s\n"
#define T_NO_STAT		"du:  Unable to stat file '%s' : (%d) %s\n"
#define T_NO_FSYS_STAT	"du:  Unable to fsys_stat file '%s' : %s\n"
#define T_NAME_TOO_LONG	"du:  Filename too long: '%s'\n"

#define MAX_LINKS	5000	/* 5000 per tree to process (file system) */

ino_t	link_ids[ MAX_LINKS ];
int		linkwarning_printed=0;
uint_t	nlinks = 0;
uchar_t	lnk;
int		nerrors=0;
int		use_fsys_stat=FALSE;
int		span_devices=TRUE;

uchar_t	aflag, kflag, pflag, qflag, sflag;
char	dir[UTIL_PATH_MAX+1];
char	fname[UTIL_PATH_MAX];
struct	dirent	*Dirent;


int main( int argc, char *argv[] )
{
	int		i;

#ifndef __QNXNTO__
#define EXTENT_OPTION "i"
#else
#define EXTENT_OPTION 
#endif

	while( ( i = getopt( argc, argv, "rakpqsx" EXTENT_OPTION ) ) != -1 ) {
		switch( i ) {
			case 'a':	aflag = TRUE;		break;
			case 'i':   use_fsys_stat = TRUE;	break;  /* try to count indirect blocks (xtent) */
			case 'k':	kflag = TRUE;		break;	/* output in k */
			case 'p':	pflag = TRUE;		break;	/* output in bytes */
			case 'q':	qflag = TRUE;		break;	/* quiet about nerrorss */
			case 's':	sflag = TRUE;		break;	/* only total sum for each cmd line file/dir*/
			case 'x':	span_devices = FALSE;	break;
			case 'r':	break; /* POSIX.2: specifies default behavior */
			default:	nerrors++;			break;	
			}
		}

	if ( nerrors ) exit(EXIT_FAILURE);

	memset( link_ids, 0, sizeof( link_ids ) );
	nlinks = 0;

	if ( optind >= argc ) {
/*
		if ((p=(char*)qnx_fullpath(&dir,"."))==NULL) {
			fprintf(stderr,"du: %s - %s\n",".",strnerrors(errno));
			nerrors++;
		} else process( p , ".", TRUE );
*/
		strcpy(dir,".");
		process(dir,TRUE);
	} else {
		for( ; optind < argc; optind++ ) {
/*
			if ((p=(char*)qnx_fullpath(&dir,argv[optind]))==NULL) {
				fprintf(stderr,"du: %s - %s\n",argv[optind],strnerrors(errno));
				nerrors++;
				continue;
			}
			process( p , argv[optind], TRUE );
*/

			if (strlen(argv[optind]) > UTIL_PATH_MAX) {
                fprintf(stderr, TXT(T_NAME_TOO_LONG), argv[optind]);
                nerrors++;
            } else {
                strcpy(dir,argv[optind]);
                process(dir,TRUE);
            }
		}
	}

	return (nerrors?EXIT_FAILURE:EXIT_SUCCESS);
}

#define ERRR 0L

unsigned long process(char *name, int root )
{
	static  struct stat statbuf, *Stat;
#ifndef __QNXNTO__
	static	struct _fsys_stat fsys_statbuf, *Fsys_Stat;
	int  fsys_stat_valid;
#endif
	static dev_t current_dev=0;
	int		i;
	unsigned long dirtotal = 0L;
    long thissize = 0L;
	uchar_t	isdir = FALSE, islnk = FALSE;
	DIR		*fd;

	Stat = &statbuf;
#ifndef __QNXNTO__
	Fsys_Stat = &fsys_statbuf;
#endif

	errno=0;
	if ( lstat( name, Stat ) == -1 ) {
		if ( !qflag ) fprintf( stderr, TXT( T_NO_STAT ), name, errno, strerror(errno) );
		return 0L;	
	}

#ifndef __QNXNTO__
	fsys_stat_valid = 0;
	if (use_fsys_stat) {
		if (!S_ISLNK(Stat->st_mode)) {
			if ( fsys_stat( name, Fsys_Stat ) == -1 ) {
				if ( !qflag ) fprintf( stderr, TXT( T_NO_FSYS_STAT ), name, strerror(errno) );
			} else fsys_stat_valid = 1;
		}
	}
#endif

	if (!span_devices) {
		/* skip anything which is on a different device from the original file */

		if (root) current_dev = Stat->st_dev;

		if (Stat->st_dev!=current_dev) {
			if (!qflag) fprintf(stderr,"du: File %s on different device; skipping...\n",name);
			return 0L;
		}
	}

	if ( S_ISDIR( Stat->st_mode ) ) {
		isdir = TRUE;

		if ( ( fd = opendir( name ) ) == NULL ) {
			if ( !qflag ) fprintf(stderr,"du:  Unable to open directory '%s' (%s)\n", name, strerror(errno) );
			return( ERRR );
		}

	/* loop reading dir entries */
		errno=0;
		while (NULL!=(Dirent=readdir(fd))) {
			islnk = FALSE;

			/* avoid backing up to parent directory!! */
			if ( strcmp( Dirent->d_name, ".." ) == 0 ) continue;

			/* '.' is still making it through here. Is that right? 
			   Yes. We need to count the size allocated for the directory itself. (EJ) */

			/* remove any single trailing / from the name of the directory */
			if ( name[strlen(name)-1] == '/' ) name[strlen(name)-1] = '\0';

			/* create file name (dir name being scanned / name in dir entry) */
			sprintf( fname, "%s/%s", name, Dirent->d_name );

			Stat=&statbuf;
			if (-1==lstat_optimize(Dirent, Stat)) {
				if ( ( lstat( fname, Stat ) ) == -1 ) {
					if ( !qflag ) fprintf(stderr, "du:  Unable to lstat '%s' (%s)\n", fname, strerror(errno) );
					continue;
				}
			}

			if (!S_ISNAM(Stat->st_mode) && Stat->st_size<0L) {
#if _FILE_OFFSET_BITS - 0 == 64
				if (!qflag) fprintf(stderr,"du: Bad size for '%s' (%lld); will treat as 0\n", fname, Stat->st_size);
#else
				if (!qflag) fprintf(stderr,"du: Bad size for '%s' (%d); will treat as 0\n", fname, Stat->st_size);
#endif
				Stat->st_size=0;
			}

#ifndef __QNXNTO__
			fsys_stat_valid = 0;

			if (use_fsys_stat) {
				/* get fsys_stat info */
				if (!S_ISLNK(Stat->st_mode)) {
					if ( fsys_stat( fname, Fsys_Stat ) == -1 ) {
						if ( !qflag ) fprintf(stderr, TXT( T_NO_FSYS_STAT ), fname, strerror(errno) );
					} else fsys_stat_valid = 1;
				}
			}
#endif

			if (!span_devices) {
				/* skip anything that is on a different device */
				if (Stat->st_dev != current_dev) {
					if (!qflag) fprintf(stderr,"du: File %s on different device; skipping...\n",fname);
					continue;
				}
			}

			/* only count links for regular files, we don't allow hard links
			   to directories other than the automatic .. link to it of its
               subdirectories */

			if ( (Stat->st_nlink>1) && S_ISREG(Stat->st_mode)) {
				for( i = 0; i < nlinks; i++ ) {
					if ( link_ids[i] == Stat->st_ino ) {
						islnk = TRUE;
#ifdef DIAG
						printf("du: File '%s' is a duplicate link already encountered\n",
								fname);
#endif
						break;
					}
				}

				if ( islnk == FALSE ) {
					if ( nlinks < MAX_LINKS )
						link_ids[nlinks++] = Stat->st_ino;
					else
						if (linkwarning_printed==0) {
							fprintf(stderr,"du: Warning - too many links found (%d), may count some links more than once.\n",nlinks);
							linkwarning_printed++;
						}

				}
			}

			if ( S_ISDIR( Stat->st_mode ) ) {
				/* don't recurse into ourself! */
				if (strcmp(Dirent->d_name, ".")) {
					strcpy( dir, fname );
					dirtotal += process( &dir[0], FALSE );
					do {
						/* In case of '//' notation */
						*(strrchr( dir, (int)'/')) = '\0';
					} while( dir[strlen(dir)-1] == '/' );
				} else {
				/* else we're processing . and we should count the size of this
				   directory. note xtnts+60 not 61 because 1st xtent has no overhead 
                   (is part of dir entry) */
					dirtotal += (
									((Stat->st_size+511)/512)
#ifndef __QNXNTO__
                                    + ((fsys_stat_valid?((Fsys_Stat->st_num_xtnts+60)/62):0))
#endif
								);
				}
		 	} else {
				if ( islnk == FALSE) { /* don't add size in if we've seen link b4 */
					if (S_ISNAM(Stat->st_mode)) {
						thissize=Stat->st_size/512;
					} else if (S_ISBLK(Stat->st_mode)) {
						thissize=Stat->st_size;
					} else {
						dirtotal += (thissize=(
									((Stat->st_size+511)/512)
#ifndef __QNXNTO__
									+((fsys_stat_valid?((Fsys_Stat->st_num_xtnts+60)/62):0))
#endif
                                    )
								);
					}
				}

				/* not a directory, if -a was specified, print the data for this file */
				if ( aflag == TRUE  &&  islnk == FALSE ) {
					if (pflag) {
						/* show thissize * 512. dirtotal may be up to ULONG_MAX before
			               being multiplied. Must use floats. */
						double d;
			
						d=thissize;
						d*=512.0;
						printf( TXT( T_FLOAT ), d, fname );
					} else if (kflag) {
						printf( TXT( T_FORMAT ),thissize/2, fname );
					} else {
						printf( TXT( T_FORMAT ),thissize, fname );
					}
				}
			}
			errno=0;
		}
	
		if (errno) {
			fprintf(stderr,"du: readdir of %s failed (%s)\n",name,strerror(errno));
		}

		closedir( fd );

		/* we used islnk when processing the items within the directory,
           but it is FALSE for the directory itself, so we must reset it here
           in case the last directory item processed was a file with link count
           greater than 1 */
		islnk=FALSE;
	} else {
		/* Argh! Someone passed me a file directly!!! */

		/*	Setup a FULL name for the file	*/
		strcpy( fname, name );
		if (strrchr(fname,(int)'/')!=NULL)
			*(strrchr( fname, (int)'/' )) = '\0';

		if ( Stat->st_nlink>1 && S_ISREG(Stat->st_mode)) {
			for( i = 0; i < nlinks; i++ ) {
				if ( link_ids[i] == Stat->st_ino ) {
					islnk = TRUE;
					break;
				}
			}
			if ( islnk == FALSE ) {
				if (nlinks < MAX_LINKS)	link_ids[nlinks++] = Stat->st_ino;
			}
		}

		if (!S_ISNAM(Stat->st_mode) && Stat->st_size<0L) {
#if _FILE_OFFSET_BITS - 0 == 64
			if (!qflag) fprintf(stderr,"du: Bad size for '%s' (%lld); will treat as 0\n", fname, Stat->st_size);
#else
			if (!qflag) fprintf(stderr,"du: Bad size for '%s' (%d); will treat as 0\n", fname, Stat->st_size);
#endif
			Stat->st_size=0;
		}

		if ( islnk == TRUE )	/* File is a link to something	*/
			dirtotal = 0L;		/* we've seen before			*/
		else {
            if (S_ISNAM(Stat->st_mode)) {
				thissize=Stat->st_size/512;
			} else if (S_ISBLK(Stat->st_mode)) {
				thissize=Stat->st_size;
			} else {
				dirtotal += (thissize=(
						((Stat->st_size+511)/512)
#ifndef __QNXNTO__
						+((fsys_stat_valid?((Fsys_Stat->st_num_xtnts+60)/62):0))
#endif
                        )
					);
			}
		}

		/* this doesn't make sense since process will only be called with a non-directory
           if the file was one specified on the command line */
		/* if (aflag==FALSE)	return( dirtotal ); */
	}

	if ( (root  &&  sflag)  ||  !sflag  &&  !islnk ) {
		if (pflag) {
			/* show dirtotal * 512. dirtotal may be up to ULONG_MAX before
               being multiplied. Must use floats. */
			double d;

			d=dirtotal;
			d*=512.0;
			printf( TXT( T_FLOAT ), d, name );
		} else if (kflag) {
			printf( TXT( T_TOTAL ), dirtotal/2, name );
		} else {
			printf( TXT( T_TOTAL ), dirtotal, name );
		}
	}
	return( dirtotal );
}
	
