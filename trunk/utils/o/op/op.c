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





#include <errno.h>
#include <fcntl.h>
#include <grp.h>
#include <process.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#ifdef __QNXNTO__
#include <libgen.h>
#endif

int main( int argc, char *argv[] ) {
    struct passwd *real;
    int i;
    gid_t gid = getegid();
    uid_t uid = geteuid();

    real = getpwuid( getuid() );
    while( (i = getopt( argc, argv, "g:u:" )) != -1 ) {
	switch( i ) {
	    case 'u':	uid = strtol( optarg, &optarg, 0 );
	    		if( *optarg ) {
	    		    struct passwd *pwd = getpwnam( optarg );

	    		    if( pwd ) uid = pwd->pw_uid;
	    		}
	    		break;

	    case 'g':	gid = strtol( optarg, &optarg, 0 );
	    		if( *optarg ) {
	    		    struct group *grp = getgrnam( optarg );

	    		    if( grp ) gid = grp->gr_gid;
	    		}
	    		break;

	    default:	exit( 1 );
	}
    }
    setgid( gid ), setuid( uid );
    if( optind != argc ) {
    	execvp( argv[optind], &argv[optind] );
		if( uid == 0 && errno == ENOENT ) {
			putenv( "PATH=/sbin:/usr/sbin" );
    		execvp( argv[optind], &argv[optind] );
		}
    	fprintf( stderr, "%s: %s (%s)\n", basename( argv[0] ), argv[optind], strerror( errno ) );
    }
    exit( errno );
}
