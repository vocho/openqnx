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
#include <errno.h>
#include <strings.h>
#include <sys/slog.h>
#include <sys/slogcodes.h>

#include "yarrow.h"

extern yarrow_t *Yarrow;

static int dev_random_read( resmgr_context_t *ctp, io_read_t *msg, 
                            RESMGR_OCB_T *ocb )
{
    int status;
    int bytes_used;
    uint8_t local_data[1024];

    status = iofunc_read_verify( ctp, msg, ocb, NULL );
    if( status != EOK ) {
        return status;
    }

    if( ( msg->i.xtype & _IO_XTYPE_MASK ) != _IO_XTYPE_NONE ) {
        return EINVAL;
    }

    if( !Yarrow ) {
        return EAGAIN;
    }

    bytes_used = min( sizeof( local_data ), msg->i.nbytes );

    memset( local_data, 0, sizeof( local_data ) );

    if( Yarrow && bytes_used > 0 ) {
        yarrow_output( Yarrow, local_data, bytes_used );
    }

    MsgReply( ctp->rcvid, bytes_used, local_data, bytes_used );

    return  _RESMGR_NOREPLY;
}


static void * resmgr_thread( void *p )
{
    resmgr_context_t         *ctp;
    resmgr_connect_funcs_t   connect_funcs;
    resmgr_io_funcs_t        io_funcs;
    iofunc_attr_t            io_attr;
    resmgr_attr_t            res_attr;
    int                      id1;
    int                      id2;
    dispatch_t               *dispatch;

    dispatch = dispatch_create();
    if( dispatch == NULL )
    {
        slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
               "random: Unable to create dispatch context: %s.", 
               strerror( errno ) );
        return NULL;
    }

    memset( &res_attr, 0, sizeof( res_attr ) );
    res_attr.nparts_max = 10;
    res_attr.msg_max_size = 0;

    iofunc_func_init( _RESMGR_CONNECT_NFUNCS, &connect_funcs, 
                      _RESMGR_IO_NFUNCS, &io_funcs );

    io_funcs.read = dev_random_read;

    iofunc_attr_init( &io_attr, S_IFNAM | 0666, 0, 0 );
    
    id1 = resmgr_attach( dispatch, &res_attr, "/dev/random", _FTYPE_ANY, 
                         0, &connect_funcs, &io_funcs, &io_attr );
    id2 = resmgr_attach( dispatch, &res_attr, "/dev/urandom", _FTYPE_ANY, 
                         0, &connect_funcs, &io_funcs, &io_attr );
    if( id1 == -1 || id2 == -1  )
    {
        slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
               "random: Unable to attach resmgr: %s.", strerror( errno ) );
        return NULL;
    }

    
    ctp = resmgr_context_alloc( dispatch );
    if( ctp == NULL )
    {
        slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
               "random: Unable to allocate resmgr context: %s.", 
               strerror( errno ) );
        return NULL;
    }

    while( 1 )
    {
        ctp = resmgr_block( ctp );
        if( ctp == NULL )
        {
            slogf( _SLOGC_CHAR, _SLOG_CRITICAL, 
                   "random: resmgr_block() failed: %s.", strerror( errno ) );
            return NULL;
        }

        resmgr_handler( ctp );
    }

    return NULL;
}


int start_resmgr( void )
{
    return pthread_create( NULL, NULL, resmgr_thread, NULL );
}

