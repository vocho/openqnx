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
 *  fd.h        File descriptor data structures
 *

 */
#ifndef __FD_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#if __WATCOMC__ > 1000
#pragma pack(push,1);
#else
#pragma pack(1);
#endif

#ifdef __386__
struct _fd_entry {
    nid_t           nid;
    short unsigned  pid,
                    zero1,
                    vid,
                    zero2;
    long unsigned   handle:24,
                    flags:8;
    } ;

#define _FD_INVALID                 0x80
#define _FD_CLOSE_ON_EXEC           0x40
#else

struct _fd_entry {
    nid_t           nid;
    short unsigned  pid,
                    zero1,
                    vid,
                    zero2,
                    handle,
                    flags;
    } ;

#define _FD_INVALID                 0x8000
#define _FD_CLOSE_ON_EXEC           0x4000
#endif


/*
 *  Flag bit definitions.
 */
/*  Someone mismatched the close-on-exec values.
    Since this *can't* be changed (would break binaries),
    the library function maps these.
    The "correct" value corresponds to <fcntl.h>
*/
#define _FD_CORRECT_CLOSE_ON_EXEC   1

struct _psinfo3;        /* for C++ */

#ifdef __cplusplus
extern "C" {
#endif
extern int qnx_fd_attach( pid_t, int, nid_t, pid_t, pid_t, unsigned, unsigned );
extern int qnx_fd_detach( int );
extern int qnx_fd_query( pid_t, pid_t, int, struct _fd_entry * );
extern int qnx_device_attach( void );
extern int qnx_device_detach( int );
extern void *__init_fd( pid_t );
extern unsigned __get_fd( pid_t, int, void * );
extern int __get_pid_info( pid_t, struct _psinfo3 *, void * );
#ifdef __cplusplus
};
#endif

#if __WATCOMC__ > 1000
#pragma pack(pop);
#else
#pragma pack();
#endif

#define __FD_H_INCLUDED
#endif
