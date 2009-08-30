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
#include <sys/neutrino.h>
#include <sys/procmgr.h>
#include <sys/sysmgr.h>
#include <sys/dispatch.h>
#include <sys/iofunc.h>
#include <sys/debug.h>
#include <sys/procfs.h>
#include <sched.h>
#include <errno.h>
#include <strings.h>
#include <sys/dir.h>
#include <fcntl.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>

#include "yarrow.h"

extern yarrow_t *Yarrow;


static void *timer_thread( void *p )
{
    int       pool_id;
    int       ret;
    uint64_t  clk;
    uint64_t  rdata;
    int       timeout;

    ret = yarrow_add_source( Yarrow, &pool_id );
    if( ret != 0 )
    {
        slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
               "random: Unable to get pool_id for timer thread." );
        return NULL;
    }

    timeout = 100;
    rdata = 0;

    while( 1 )
    {
        if( Yarrow )
        {
            yarrow_output( Yarrow, (uint8_t *)&rdata, sizeof( rdata ) );
            /* Wait for between 10ms and 1033ms */
            timeout = ( rdata & 0x3FF ) + 10;
        }

        delay( timeout );
        clk = ClockCycles();
        clk = clk ^ rdata;

        if( Yarrow )
            yarrow_input( Yarrow, (uint8_t *)&clk, sizeof( clk ), pool_id, 8 );
    }

    return NULL;
}




static void *syspoll_thread( void *p )
{
    int             pool_id;
    int             ret;
    uint64_t        clk;
    uint64_t        rdata;
    int             timeout;
    DIR             *dir;
    struct          dirent* dir_entry;
    sha1_ctx_t      context;
    uint8_t         digest[20];
    char            as_path[256];
    int             fd;
    int             i;
    debug_thread_t  debug_tid;
    debug_process_t debug_pid;


    ret = yarrow_add_source( Yarrow, &pool_id );
    if( ret != 0 )
    {
        slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
               "random: Unable to get pool_id for syspoll thread." );
        return NULL;
    }

    /* Setup a default 1 minute timeout */
    timeout = 1000 * 60;
    while( 1 )
    {
        if( Yarrow )
        {
            yarrow_output( Yarrow, (uint8_t *)&rdata, sizeof( rdata ) );
            /* Wait from ~16s to ~17.5 minutes */
            timeout = ( ( rdata & 0x3F ) + 1 ) << 14;
        }

        delay( timeout );

        SHA1Init( &context );

        dir = opendir( "/proc" );
        if( dir == NULL )
            continue;

        dir_entry = readdir( dir );
        while( dir_entry != NULL )
        {
            SHA1Update( &context, (uint8_t *)dir_entry, 
                        sizeof( struct dirent ) + dir_entry->d_namelen - 1 );

            if( dir_entry->d_name[0] < '0' || dir_entry->d_name[1] > '9' )
            {
                dir_entry = readdir( dir );
                continue;
            }

            sprintf( as_path, "/proc/%s/as", dir_entry->d_name );
            fd = open( as_path, O_RDONLY );
            while( fd != -1 )
            {
                memset( &debug_pid, 0, sizeof( debug_pid ) );
                ret = devctl( fd, DCMD_PROC_INFO, &debug_pid, 
                              sizeof( debug_pid ), NULL );
                if( ret == -1 )
                    break;

                SHA1Update( &context, (uint8_t *)&debug_pid, 
                            sizeof( debug_pid ) );

                for( i=0; i<debug_pid.num_threads; i++ )
                {
                    memset( &debug_tid, 0, sizeof( debug_tid ) );
                    debug_tid.tid = i + 1;
                    ret = devctl( fd, DCMD_PROC_TIDSTATUS, &debug_tid, 
                                  sizeof( debug_tid ), NULL );
                    if( ret == -1 )
                        continue;

                    SHA1Update( &context, (uint8_t *)&debug_tid, 
                                sizeof( debug_tid ) );
                }

                break;
            }

            if( fd != -1 )
                close( fd );

            dir_entry = readdir( dir );
        }
        closedir( dir );
       
        SHA1Update( &context, (uint8_t *)&rdata, sizeof( rdata ) );

        ClockTime( CLOCK_REALTIME, NULL, &clk );

        SHA1Update( &context, (uint8_t *)&clk, sizeof( clk ) );

        SHA1Final( digest, &context );

        if( Yarrow )
        {
            /* Assume 1 bit in every 8 is random */
            yarrow_input( Yarrow, digest, sizeof( digest ), pool_id, 
                          sizeof( digest ) );
        }
    }

    return NULL;
}


static void *interrupt_thread( void *p )
{
    int ret;
    int intr;
    int intr_id;
    struct sigevent event;
    struct _pulse pulse;
    iov_t  iov;
    int    rcvid;
    int chid;
    int coid;
    int count;
    uint64_t rdata;
    uint16_t target;
    uint64_t clk;
    int    pool_id;

    intr = (int)p;
    rdata = 0;
    target = 512;

    ret = ThreadCtl( _NTO_TCTL_IO, 0 );
    if( ret != 0 )
    {
        slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
               "random: Unable to gain IO privs: %s",
               strerror( errno ) );
        return NULL;
    }

    chid = ChannelCreate( 0 );
    if( chid == -1 )
    {
        slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
               "random: ChannelCreate() failed: %s",
               strerror( errno ) );
        return NULL;
    }

    coid = ConnectAttach( 0, 0, chid, _NTO_SIDE_CHANNEL, 0 );
    if( coid == -1 )
    {
        slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
               "random: ConnectAttach() failed: %s",
               strerror( errno ) );
        return NULL;
    }

    ret = yarrow_add_source( Yarrow, &pool_id );
    if( ret != 0 )
    {
        slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
               "random: Unable to get pool_id for interrupt %d thread.", intr );
        return NULL;
    }

    event.sigev_notify   = SIGEV_PULSE;
    event.sigev_coid     = coid;
    event.sigev_code     = 1;
    event.sigev_priority = 15;
    intr_id = InterruptAttachEvent( intr, &event, _NTO_INTR_FLAGS_TRK_MSK );
    if( intr_id == -1 )
    {
        slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
               "random: Unable to attach event to intr %d: %s", 
               intr, strerror( errno ) );
        return NULL;
    }

    /* This is how many interrupts we are gonna get */
    if( Yarrow )
    {
        yarrow_output( Yarrow, (uint8_t *)&rdata, sizeof( rdata ) );
        target = rdata & 0x1FF;
    }

    count = 0;

    SETIOV( &iov, &pulse, sizeof( pulse ) );
    while( 1 )
    {
        rcvid = MsgReceivev( chid, &iov, 1, NULL );
        if( rcvid == -1 )
        {
            if( errno == ESRCH )
                return NULL;
            continue;
        }

        switch( pulse.code )
        {
            case 1:
                InterruptUnmask( intr, intr_id );
                count++;
                if( count >= target )
                {
                    ClockTime( CLOCK_REALTIME, NULL, &clk );
                    clk = clk ^ rdata;

                    if( Yarrow )
                    {
                        yarrow_input( Yarrow, (uint8_t *)&clk, sizeof( clk ), 
                                      pool_id, 8 );

                        yarrow_output( Yarrow, (uint8_t *)&rdata, 
                                       sizeof( rdata ) );
                    }

                    target = rdata & 0x1FF;
                    count = 0;
                }
                break;

            default:
                if( rcvid )
                    MsgError( rcvid, ENOTSUP );
        }
        
    }

    return NULL;
}


int start_syspoll_source( void )
{
    pthread_attr_t      pattr;
    struct sched_param  param;

    pthread_attr_init( &pattr );
    param.sched_priority = 11;
    pthread_attr_setschedparam( &pattr, &param );
    pthread_attr_setinheritsched( &pattr, PTHREAD_EXPLICIT_SCHED );
    pthread_attr_setstacksize( &pattr, 16*1024 );

    return pthread_create( NULL, NULL, syspoll_thread, NULL );
}


int start_interrupt_source( int intr )
{
    pthread_attr_t      pattr;
    struct sched_param  param;

    pthread_attr_init( &pattr );
    param.sched_priority = 15;
    pthread_attr_setschedparam( &pattr, &param );
    pthread_attr_setinheritsched( &pattr, PTHREAD_EXPLICIT_SCHED );

    return pthread_create( NULL, NULL, interrupt_thread, (void *)intr );
}


int start_timer_source( void )
{
    pthread_attr_t      pattr;
    struct sched_param  param;

    pthread_attr_init( &pattr );
    param.sched_priority = 10;
    pthread_attr_setschedparam( &pattr, &param );
    pthread_attr_setinheritsched( &pattr, PTHREAD_EXPLICIT_SCHED );

    return pthread_create( NULL, &pattr, timer_thread, NULL );
}



