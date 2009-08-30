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
%C - change file access and modification times (POSIX)

%C [-amc] [-r ref_file] [-t time] file...
Options:
 -a             Touch access time
 -c             Don't create files if they do not exist
 -m             Touch modify time
 -r ref_file    Use time of 'ref_file' for touching
 -t time        Use 'time' for touching
Where:
 Time format is [[CC]YY]MMDDhhmm[.SS]
   CC           Century
   YY           Year
   MM           Month (01-12)
   DD           Day (01-31)
   hh           Hour (00-23)
   mm           Minute (00-59)
   SS           Seconds (00-61)
#endif
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <utime.h>
#include <ctype.h>
#include <lib/compat.h>

#define TXT(s)	s
#define T_INV_TIME	"touch: Invalid time format - use [[CC]YY]MMDDhhmm[.SS]\n"
#define T_NO_FILES	"touch: No file(s) to act on\n"
#define T_OUT_RANGE "touch: Invalid time format or time out of range\n"
#define T_BAD_DAY	"touch: Invalid day for month specified\n"
#define T_BAD_MON	"touch: Invalid month specified\n"
#define T_BAD_HOUR	"touch: Invalid hour specified\n"
#define T_BAD_MIN	"touch: Invalid minute specified\n"
#define T_BAD_SEC	"touch: Invalid seconds specified\n"
#define T_BAD_YEAR	"touch: Invalid year specified\n"
#define T_BAD_CENT	"touch: Invalid century specified\n"
#define T_OVERFLOW	"touch: time_t variable overflow\n"
#define T_ONE_TIME	"touch: Specify only a single time option\n"
#define T_SUMMARY	"touch: [-amc] [-r ref_file] [-t time] file...\n"


long		parse_date( char *date_string );


int
main( argc, argv )
	int		argc;
	char	*argv[];
	{
	int 	i;
	int		atime, mtime, no_create, explicit;
	time_t	ttime;
	struct	utimbuf	timestamp;
	struct	stat	statbuf;

	if ( argc == 1 ) {
		fprintf( stderr, T_SUMMARY );
		exit( EXIT_FAILURE );
		}

	ttime = time(NULL), explicit = 0;

	mtime = atime = no_create = 0;
	
	while( ( i = getopt( argc, argv, "acmr:t:" ) ) != -1 ) {
		switch( i ) {
			case 'a':	atime = 1;
						break;

			case 'm':	mtime = 1;
						break;

			case 'c':	no_create = 1;
						break;

			case 'r':	if ( explicit ) {
							fprintf( stderr, TXT( T_ONE_TIME ) );
							exit( EXIT_FAILURE );
							}
						if ( stat( optarg, &statbuf ) == -1 ) {
							perror( optarg );
							exit( EXIT_FAILURE );
							}
						ttime = statbuf.st_mtime, explicit = !0;
						break;

			case 't':	if ( explicit ) {
							fprintf( stderr, TXT( T_ONE_TIME ) );
							exit( EXIT_FAILURE );
							}
						ttime = parse_date( optarg ), explicit = !0;
						break;

			default:
						exit( EXIT_FAILURE );
			}
		}


	/*	If nothing spec'd, change both atime & mtime to be true	*/
	if( !atime  &&  !mtime ) atime = mtime = 1;

#ifndef __QNXNTO__
	if( optind < argc ) i = strlen( argv[optind] );
	else i = 0;
	if ( ( i == 8  ||  i == 10 )  &&  !explicit  &&  argc - optind ) {
		char	*p, crumdate[10];

		/*	no time already set, 8 or 10 char 'filename' and more than two
			operands -- maybe crummy MMDDhhmmYY (OBSOLESCENT) usage	*/
		for ( p = argv[optind]; *p != NULL  &&  isdigit( *p ); p++ );
		if ( *p == NULL ) {	/* if all filename chars were digits */
			if ( i == 10 ) {
				strncpy( crumdate + 2, argv[optind], 8 );
				strncpy( crumdate, argv[optind] + 8, 2 );
				crumdate[10] = '\0';
				}
			else
				strcpy( crumdate, argv[optind] );
			optind++;
			ttime = parse_date( crumdate ), explicit = !0;
			}
		}
#endif

	if ( mtime ) timestamp.modtime = ttime;
	if ( atime ) timestamp.actime = ttime;

	if ( optind >= argc ) {
		fprintf( stderr, TXT( T_NO_FILES ) );
		exit( EXIT_FAILURE );
		}

	for( ; optind < argc; optind++ ) {
		if( stat( argv[ optind ], &statbuf ) == -1 ) {
			int		fd;

			if( errno == ENOENT  &&  !no_create ) {
				/* filename doesn't exist  &&  I'm allowed to create files */
				if( ( fd = creat( argv[ optind ], S_IRUSR|S_IWUSR
							|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH ) ) == -1 ) {
					perror( argv[optind] );
					exit( EXIT_FAILURE );
					}
				else {
					if( fstat( fd, &statbuf ) == -1 ) {
						perror( "fstat" );
						exit( EXIT_FAILURE );
						}
					close( fd );
					}
				}   		
			else if( errno == ENOENT  &&  no_create )
				continue;	/* file doesn't exist && I can't create it */
			else {
				perror( argv[optind] );
				exit( EXIT_FAILURE );
				}
			}/* end of failed stat() */

		if( !atime ) timestamp.actime = statbuf.st_atime;
		if( !mtime ) timestamp.modtime = statbuf.st_mtime;
		
		if ( utime( argv[ optind ], &timestamp ) == -1 ) {
			perror( argv[optind] );
			exit( EXIT_FAILURE );
			}
		}/* end of files */
	return EXIT_SUCCESS;
	}

enum { CC = 0, YY, MM, DD, hh, mm, SS };
struct ranges {
	int	lo, hi;
	} valid[7] = {
		{19,20}, {0,99}, {1,12}, {1,31}, {0,23}, {0,59}, {0,61} };
#define BOUND_CHK(what,myval) (valid[what].hi < j || valid[what].lo > j)

/* 29 days for feburary in case of a leap year */
int	days_per_month[12] = { 31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

long parse_date( char *date_string )
	{
	struct tm	*tptr;
	char		*p;
	int			i, j, cent = 19, bad_date = 0;
	time_t      tme;

	tme = time( (time_t *)NULL );
	tptr = localtime( &tme );

	p = date_string + (i = strlen( date_string ));
	switch( i ) {
		case 11:	/*     MMDDhhmm.SS				*/
		case 13:	/*   YYMMDDhhmm.SS				*/
		case 15:	/* CCYYMMDDhhmm.SS				*/
				/* deal with seconds */
				p -= 3;
				tptr->tm_sec = j = atoi( p + 1 );
				if ( BOUND_CHK(SS,j) ) {
					fprintf( stderr, T_BAD_SEC );
					bad_date++;
					}
				*p = '\0';
		case 8:		/*     MMDDhhmm					*/
		case 10:	/*   YYMMDDhhmm 				*/
		case 12:	/* CCYYMMDDhhmm					*/
				p -= 2;/* minutes */
				tptr->tm_min = j = atoi( p );
				if ( BOUND_CHK(mm,j) ) {
					fprintf( stderr, T_BAD_MIN );
					bad_date++;
					}
				*p = '\0';
				p -= 2;/* hours */
				tptr->tm_hour = j = atoi( p );
				if ( BOUND_CHK(hh,j) ) {
					fprintf( stderr, T_BAD_HOUR );
					bad_date++;
					}
				*p = '\0';
				p -= 2;/* day */
				tptr->tm_mday = j = atoi( p );
				if ( BOUND_CHK(DD,j) ) {
					fprintf( stderr, T_BAD_DAY );
					bad_date++;
					}
				*p = '\0';
				p -= 2;/* month */
				tptr->tm_mon = j = atoi( p );
				if ( BOUND_CHK(MM,j) ) {
					fprintf( stderr, T_BAD_MON );
					bad_date++;
					}
				tptr->tm_mon--;/* mktime() 0-11 instead of 1-12 */
				if ( tptr->tm_mday > days_per_month[tptr->tm_mon] ) {
					fprintf( stderr, TXT( T_BAD_DAY ) );
					bad_date++;
					}
				*p = '\0';
				if ( i >= 10  &&  i != 11 ) {
					p -= 2;/* year */
					tptr->tm_year = j = atoi( p );
					if ( BOUND_CHK(YY,j) ) {
						fprintf( stderr, T_BAD_YEAR );
						bad_date++;
						}
					if ( j >= 0  &&  j <= 68 ) {
						tptr->tm_year += 100;
						cent = 20;
						}
					*p = '\0';
					if ( i == 12  ||  i == 15 ) {
						p -= 2;/* century */
						j = atoi( p );
						tptr->tm_year += 100*( j - cent );
						if ( BOUND_CHK(CC,j) ) {
							fprintf( stderr, T_BAD_CENT );
							bad_date++;
							}
						}
					}
				tptr->tm_isdst = -1;/* force mktime to make best
									   guess as to whether this new
									   time is dst or not */
				tme = mktime( tptr );
				if (tme == (time_t)-1) {
					fprintf( stderr, TXT( T_OVERFLOW ) );
					bad_date++;
				}
				break;
		default:
				fprintf( stderr, TXT( T_INV_TIME ) );
				exit( EXIT_FAILURE );
		}
	if ( bad_date )	exit( EXIT_FAILURE );
	return( tme );
	}
