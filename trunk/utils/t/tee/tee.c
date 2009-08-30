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





#ifdef __USAGE		/* tee.c */
%C - duplicate standard input (POSIX)

%C	[-a] [-i] [file...]
Options:
 -a     Append to files
 -i     Ignore SIGINT
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.4  2006/04/11 16:16:31  rmansfield
	PR: 23548
	CI: kewarken

	Stage 1 of the warnings/errors cleanup.

	Revision 1.3  2005/06/03 01:38:01  adanko
	
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.
	
	Note: only comments were changed.
	
	PR25328
	
	Revision 1.2  2003/09/02 14:33:44  martin
	Update QSSL Copyright.
	
	Revision 1.1  1998/10/22 02:39:01  dtdodge
	CVS initial checkin
	
	Revision 1.2  1992/10/27 19:08:25  eric
	added usage one-liner

 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *
	
	$Author: sboucher $
	
---------------------------------------------------------------------*/

#include <stdio.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifndef TRUE
#	define TRUE		(1==1)
#	define FALSE	(1==0)
#endif

#define BUFSIZE	2048

int		*fd_array;			/*	File descriptor table	*/
int		fd_num;				/*	# of files on cmdline	*/
char	*buf;				/*	Data buffer				*/
int		append = FALSE;		/*	Command line option		*/
int		error = 0;			/*	Exit status (errors?)	*/
int		i, n;				/*	General purpose			*/
int     bufsize=BUFSIZE;    /*  amount to read at a time */

#define TXT(s)				s
#define T_UNABLE_TO_OPEN	"tee:  Unable to open '%s'\n"
#define T_NO_FD_TABLE		"tee:  Unable to allocate file descriptor table\n"
#define T_NO_BUF			"tee:  Unable to allocate data buffer\n"

ssize_t
safe_write (int fd, char *buf, size_t len)
{
    ssize_t written;
    ssize_t total_len = len;
    while ((written=write(fd, buf, len)) != len) {
        if (written == -1) {
            if (errno != EAGAIN && errno != EINTR)
                return -1;
        } else {
            buf+=written;
            len-=written;
        }
    }
    return total_len;
}

int
main( argc, argv )
	int		argc;
	char	*argv[];
	{

	while( ( n = getopt( argc, argv, "aiu" ) ) != -1 ) {
		switch( n ) {
			case 'a':	append = TRUE; 						break;
			case 'i':	signal( SIGINT, SIG_IGN );			break;
			case 'u':   bufsize=1;							break;
			default:	error++;							break;
			}
		}

	if ( error )
		exit( EXIT_FAILURE );

	fd_num = argc - optind;
	if ( fd_num )
		fd_array = calloc( fd_num, sizeof( int ) );
	buf = calloc( 1, BUFSIZE + 1 );

	if ( !fd_array  &&  fd_num ) {
		fprintf( stderr, TXT( T_NO_FD_TABLE ) );
		exit( EXIT_FAILURE );
		}

	if ( !buf ) {
		fprintf( stderr, TXT( T_NO_BUF ) );
		exit( EXIT_FAILURE );
		}

	/* Open all the files	*/
	for( i = 0; optind < argc; optind++, i++ ) {
		if ( append == TRUE )
			fd_array[i] = open( argv[optind], O_CREAT|O_APPEND|O_WRONLY, 0666 );
		else
			fd_array[i] = creat( argv[optind], 0666 );
		if ( fd_array[i] == -1 ) {
			error++;
			fprintf( stderr, T_UNABLE_TO_OPEN, argv[optind] );
			}
		}

	/* Get bufsize input from stdin		*/
	while ( (n = read( STDIN_FILENO, buf, bufsize )) ) {
		if ( n == 0 )	/* EOF */
			break;
		/* Output to stdout	*/
		if ( safe_write( STDOUT_FILENO, buf, n ) != n )
			exit( EXIT_FAILURE );	/* Signify error, die */
		for( i = 0; i < fd_num; i++ ) {
			if ( fd_array[i] != -1 ) {
				/* Output to each individual file	*/
				if ( safe_write( fd_array[i], buf, n ) != n ) {
					close( fd_array[i] );
					error++;
					fd_array[i] = -1;
					}
				}
			}
		memset( buf, 0, bufsize );
		}

	/* Close all the files	*/
	for( i = 0; i < fd_num; i++ )
		if ( fd_array[i] != -1 )
			close( fd_array[i] );

	return( error );
	}
