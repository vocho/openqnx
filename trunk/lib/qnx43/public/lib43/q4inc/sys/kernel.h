/*
 * $QNXtpLicenseC:
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
 *  kernel.h    kernel function calls/pragmas
 *
 *  Copyright by WATCOM International Corp. 1988-1993.  All rights reserved.


 */
#ifndef __KERNEL_H_INCLUDED
#define __KERNEL_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#ifndef __SENDMX_H_INCLUDED
 #include <sys/sendmx.h>
#endif

#define PROC_PID    1

/*
 * Task States
 */

#define STATE_DEAD              0
#define STATE_READY             1
#define STATE_SEND_BLOCKED      2
#define STATE_RECEIVE_BLOCKED   3
#define STATE_REPLY_BLOCKED     4
#define STATE_HELD              5
#define STATE_SIGNAL_BLOCKED    6
#define STATE_WAIT_BLOCKED      7
#define STATE_SEM_BLOCKED       8

#ifdef __cplusplus
extern "C" {
#endif

#define Send( a,b,c,d,e ) __send( a,b,c,d,e )

extern int __send(
            pid_t           __pid,
            const void      *__msg1,
            void            *__msg2,
            unsigned        __nbytes1,
            unsigned        __nbytes2 );

#define Sendfd( a,b,c,d,e ) __sendfd( a,b,c,d,e )

extern int __sendfd(
            int             __fd,
            const void     *__msg1,
            void           *__msg2,
            unsigned        __nbytes1,
            unsigned        __nbytes2 );

#define Receive( a,b,c ) __receive( a,b,c )

extern pid_t __receive(
            pid_t           __pid,
            void           *__msg,
            unsigned        __nbytes );

#define Reply( a,b,c ) __reply( a,b,c )

extern int __reply(
            pid_t           __pid,
            const void     *__msg,
            unsigned        __nbytes );

#define Creceive( a,b,c ) __creceive( a,b,c )

extern pid_t __creceive(
            pid_t           __pid,
            void           *__msg,
            unsigned        __nbytes );

#define Readmsg( a,b,c,d ) __readmsg( a,b,c,d )

extern unsigned __readmsg(
            pid_t           __pid,
            unsigned        __offset,
            void           *__msg,
            unsigned        __nbytes );

#define Writemsg( a,b,c,d ) __writemsg( a,b,c,d )

extern unsigned __writemsg(
            pid_t           __pid,
            unsigned        __offset,
            const void     *__msg,
            unsigned        __nbytes );

extern int __kererr(int errcode);

#define Sendmx( a,b,c,d,e ) __sendmx( a,b,c,d,e )

extern int __sendmx(
            int                   __pid,
            unsigned              __smsg_xparts,
            unsigned              __rmsg_xparts,
            const struct _mxfer_entry  *__smsg,
            const struct _mxfer_entry  *__rmsg );

#define Receivemx( a,b,c ) __receivemx( a,b,c )

extern int __receivemx(
            pid_t                 __pid,
            unsigned              __rmsg_xparts,
            struct _mxfer_entry  *__rmsg );

#define Replymx( a,b,c ) __replymx( a,b,c )

extern int __replymx(
            pid_t                 __pid,
            unsigned              __rmsg_xparts,
            struct _mxfer_entry  *__rmsg );

#define Creceivemx( a,b,c ) __creceivemx( a,b,c )

extern int __creceivemx(
            pid_t                 __pid,
            unsigned              __rmsg_xparts,
            struct _mxfer_entry  *__rmsg );

#define Readmsgmx( a,b,c,d ) __readmsgmx( a,b,c,d )

extern unsigned __readmsgmx(
            pid_t                 __pid,
            unsigned              __offset,
            unsigned              __rmsg_xparts,
            struct _mxfer_entry  *__rmsg);

#define Writemsgmx( a,b,c,d ) __writemsgmx( a,b,c,d )

extern unsigned __writemsgmx(
            pid_t                 __pid,
            unsigned              __offset,
            unsigned              __wmsg_xparts,
            struct _mxfer_entry  *__wmsg );

#define Relay( a,b ) __relay( a,b )

extern int __relay(
            pid_t           __pid1,
            pid_t           __pid2 );

#define Kill( a,b ) __kill( a,b )

extern int __kill(
            pid_t           __pid,
            int             signo );

#define Sigsuspend( a ) __sigsuspend( a )

extern int __sigsuspend(
            long            __mask);

#define __Sret() __sret()

extern void __sret( void );

#define Priority( a,b ) __priority( a,b )

extern int __priority(
            pid_t           __pid1,
            int             __pri );

#define Netdata( a,b ) __netdata( a,b )

extern int __netdata(
            pid_t           __pid1,
            unsigned        __type );

#define Yield() __yield()

extern void __yield( void );

#define Sendfdmx( a,b,c,d,e ) __sendfdmx( a,b,c,d,e )

extern int __sendfdmx(
            int                   __fd,
            unsigned              __smsg_xparts,
            unsigned              __rmsg_xparts,
            struct _mxfer_entry  *__smsg,
            struct _mxfer_entry  *__rmsg );

#define Trigger( a ) __trigger( a )

extern int __trigger( pid_t __pid );

#define Semaphore( a,b ) __semaphore( a,b )

extern int __semaphore(
                        int                             id,
            int             semaction );

#ifdef __cplusplus
};
#endif

#endif
