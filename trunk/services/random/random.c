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






#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/procmgr.h>
#include <sys/sysmgr.h>
#include <sys/dispatch.h>
#include <sys/iofunc.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>
#include <errno.h>
#include <strings.h>
#include <syslog.h>

#include "yarrow.h"
#include "sources.h"

#define MAX_INTR  32

yarrow_t *Yarrow;

int start_resmgr( void );


static void handle_signals( void )
{
    int       ret;
    sigset_t  sset;
    siginfo_t sinfo;

    sigfillset( &sset );
    sigdelset( &sset, SIGTERM );
    sigdelset( &sset, SIGINT );
    sigdelset( &sset, SIGHUP );
    sigprocmask( SIG_BLOCK, &sset, NULL );

    sigemptyset( &sset );
    sigaddset( &sset, SIGTERM );
    sigaddset( &sset, SIGINT );
    sigaddset( &sset, SIGHUP );

    while( 1 )
    {
        ret = SignalWaitinfo( &sset, &sinfo );
        if( ret == -1 )
            continue;

        slogf( _SLOGC_CHAR, _SLOG_ERROR, "random: Exiting on signal %d", 
               sinfo.si_signo );
        break;
    }

    return;
}


int main( int argc, char **argv )
{
    int  ret;
    int  c;
    int  intr_list[MAX_INTR];
    int  intr_count;
    int  system_poll;
    int  hp_timer;
    char path[256];

    intr_count = 0;
    system_poll = 0;
    hp_timer = 0;

    while( ( c = getopt( argc, argv, "hi:tp" ) ) != -1 )
    {
        switch( c )
        {
            case 'h':
                memset( path, 0, sizeof( path ) );
                snprintf( path, 255, "use %s", argv[0] );
                return system( path );

            case 'i':
                if( intr_count < MAX_INTR )
                {
                    intr_list[ intr_count ] = strtol( optarg, 0, 0 );
                    intr_count ++;
                }
                else
                {
                    fprintf( stderr, "random: Only %d interrupt sources "
                             "allowed. Ignoring interrupt 0x%lX.\n", MAX_INTR, 
                             strtol( optarg, 0, 0 ) );
                }
                break;

            case 'p':
                system_poll = 1;
                break;

            case 't':
                hp_timer = 1;
                break;
        }
    }

    if( intr_count == 0 && system_poll == 0 && hp_timer == 0 )
    {
        fprintf( stderr, "random: WARNING - "
                 "no source of random data given.\n" );
    }

    ret = start_resmgr();
    if( ret != 0 )
    {
        fprintf( stderr, "random: Unable to start resmgr: %s\n", 
                 strerror( errno ) );
        return EXIT_FAILURE;
    }

    /* Go into daemon mode! */
    ret = procmgr_daemon( EXIT_SUCCESS, PROCMGR_DAEMON_NOCLOSE );
    if( ret == -1 )
    {
        slogf( _SLOGC_CHAR, _SLOG_ERROR, 
               "random: Unable to detach from controling terminal." );
    }

    /* Initilize the random data */
    Yarrow = yarrow_create();
    if( Yarrow == NULL )
    {
        slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
               "random: Unable to create PRNG: %s", strerror( errno ) );
        return EXIT_FAILURE;
    }

    if( hp_timer )
    {
        ret = start_timer_source();
        if( ret != 0 )
        {
            slogf( _SLOGC_CHAR, _SLOG_ERROR, 
                   "random: Unable to start timer: %s", strerror( errno ) );
        }
    }

    for( c=0; c<intr_count; c++ )
    {
        ret = start_interrupt_source( intr_list[c] );
        if( ret != 0 )
        {
            slogf( _SLOGC_CHAR, _SLOG_ERROR, 
                   "random: Unable to use interrupt %d: %s", intr_list[c], 
                   strerror( errno ) );
        }
    }

    if( system_poll )
    {
        ret = start_syspoll_source();
        if( ret != 0 )
        {
            slogf( _SLOGC_CHAR, _SLOG_ERROR, 
                   "random: Unable to start syspoll: %s", strerror( errno ) );
        }
    }

    handle_signals();

    return EXIT_SUCCESS;
}
