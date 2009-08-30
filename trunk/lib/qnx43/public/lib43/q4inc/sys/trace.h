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
 *  trace.h     Trace data structures and definitions
 *

 */

#ifndef __TRACE_MSG_H_INCLUDED

#ifndef __SYS_MSG_H_INCLUDED
 #include <sys/sys_msg.h>
#endif

#ifndef _I86_H_INCLUDED
 #include <i86.h>
#endif

#pragma pack(1);

/*
 * trace subtype values.
 */
#define _TRACE_GET_POINT    0
#define _TRACE_SET_POINT    1

/*
 * Trace points for PROC
 */
#define _TRACE_PROC_SAC     0

/*
 * Trace severity flags
 */
#define _TRACE_CRITICAL     0   /* Major fault, should never happen      */
#define _TRACE_SEVERE       1   /* Hardware failure (major)              */
#define _TRACE_TRANSIENT    2   /* Could possibly happen (eg bad blocks) */
#define _TRACE_EXPECTED     3   /* Expected errors (eg crc error)        */
#define _TRACE_COMMON       6   /* Common errors (not logged by default  */
#define _TRACE_TESTING      7   /* Used only for debugging code          */

/*
 * Trace event format
 */
struct _trace_event {
    short unsigned   length:9;    /* total length of full message min=12    */
    short unsigned   reserved2:2; /* for future expantion                   */
    short unsigned   smallint:1;  /* 1=16bit ints, 0=32bit ints             */
    short unsigned   severity:3;  /* severity level 0-7 0=most critical     */
    short unsigned   reserved1:1; /* for future expantion                   */
    short unsigned   nano_hi;     /* hi word of long nanoseconds stamp      */
    time_t           seconds;     /* seconds stamp                          */
    long unsigned    code;        /* hi word=major, low word=minor          */
#ifdef __cplusplus
    short unsigned   data[1];     /* variable amount of data 0-502 bytes    */
#else
    short unsigned   data[];      /* variable amount of data 0-502 bytes    */
#endif
};

struct _trace_info {
    long            buffsize;
    long            datasize;
    short unsigned  overruns;
    short unsigned  severity;
    mpid_t          reader;
    short unsigned  spare1;
    short unsigned  tracesel;
    long            hi_water;
    mpid_t          proxy;
    short unsigned  spare2;
};

/*
 * Trace open flags. Will cause action on every read.
 */
#define _TRACE_CLEAR_BUFF           0x0001
#define _TRACE_CLEAR_OVERRUNS       0x0002

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Trace query prototypes
 */
extern int qnx_trace_trigger(pid_t proc_pid, long hi_water, pid_t proxy);
extern int qnx_trace_severity(pid_t proc_pid, int severity);
extern int qnx_trace_info(pid_t proc_pid, struct _trace_info *info);
extern int qnx_trace_open(pid_t proc_pid, unsigned flags);
extern int qnx_trace_read(pid_t proc_pid, size_t size, char *buffer, int *overruns);
extern int qnx_trace_close(pid_t proc_pid);

/*
 * Trace function prototypes
 */
extern int Trace0(unsigned long code, int severity);
extern int Trace0b(unsigned long code, int severity, int nbytes, void *data);
extern int Trace1(unsigned long code, int severity, int arg1);
extern int Trace1b(unsigned long code, int severity, int arg1, int nbytes, void *data);
extern int Trace2(unsigned long code, int severity, int arg1, int arg2);
extern int Trace2b(unsigned long code, int severity, int arg1, int arg2, int nbytes, void *data);
extern int Trace3(unsigned long code, int severity, int arg1, int arg2, int arg3);
extern int Trace3b(unsigned long code, int severity, int arg1, int arg2, int arg3, int nbytes, void *data);
extern int Trace4(unsigned long code, int severity, int arg1, int arg2, int arg3, int arg4);
extern int Trace4b(unsigned long code, int severity, int arg1, int arg2, int arg3, int arg4, int nbytes, void *data);
extern int Trace5(unsigned long code, int severity, int arg1, int arg2, int arg3, int arg4, int arg5);
extern int Trace5b(unsigned long code, int severity, int arg1, int arg2, int arg3, int arg4, int arg5, int nbytes, void *data);
extern int Trace6(unsigned long code, int severity, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6);
extern int Trace6b(unsigned long code, int severity, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int nbytes, void *data);


#ifdef __386__
#pragma aux __trace_int parm caller [eax] [] modify nomemory exact [eax];
#define __TRACE_MODE(_nargs, _buff, _pri) \
                (__TRACE_SIZE##_nargs|_buff<<15|(_pri&7)<<12)
#define __TRACE_SIZE0     4
#define __TRACE_SIZE0B    16
#define __TRACE_SIZE1     8
#define __TRACE_SIZE1B    20
#define __TRACE_SIZE2     12
#define __TRACE_SIZE2B    24
#define __TRACE_SIZE3     16
#define __TRACE_SIZE3B    28
#define __TRACE_SIZE4     20
#define __TRACE_SIZE4B    32
#define __TRACE_SIZE5     24
#define __TRACE_SIZE5B    36
#define __TRACE_SIZE6     28
#define __TRACE_SIZE6B    40
#else
#pragma aux __trace_int parm caller [ax] [] modify nomemory exact [ax];
#define __TRACE_MODE(_nargs, _buff, _pri) \
                (__TRACE_SIZE##_nargs|_buff<<15|(_pri&7)<<12|0x800)
#define __TRACE_SIZE0     4
#define __TRACE_SIZE0B    16
#define __TRACE_SIZE1     6
#define __TRACE_SIZE1B    18
#define __TRACE_SIZE2     8
#define __TRACE_SIZE2B    20
#define __TRACE_SIZE3     10
#define __TRACE_SIZE3B    22
#define __TRACE_SIZE4     12
#define __TRACE_SIZE4B    24
#define __TRACE_SIZE5     14
#define __TRACE_SIZE5B    26
#define __TRACE_SIZE6     16
#define __TRACE_SIZE6B    28
#endif

#define Trace0(_a, _b) __trace0(__TRACE_MODE(0,0,(_b)), _a)
#define Trace0b(_a, _b, _c, _d) __trace0b(__TRACE_MODE(0,1,(_b)), _c, FP_OFF(_d), FP_SEG(_d), _a)
#define Trace1(_a, _b, _c) __trace1(__TRACE_MODE(1,0,(_b)), _a, _c)
#define Trace1b(_a, _b, _c, _d, _e) __trace1b(__TRACE_MODE(1,1,(_b)), _d, FP_OFF(_e), FP_SEG(_e), _a, _c)
#define Trace2(_a, _b, _c, _d) __trace2(__TRACE_MODE(2,0,(_b)), _a, _c, _d)
#define Trace2b(_a, _b, _c, _d, _e, _f) __trace2b(__TRACE_MODE(2,1,(_b)), _e, FP_OFF(_f), FP_SEG(_f), _a, _c, _d)
#define Trace3(_a, _b, _c, _d, _e) __trace3(__TRACE_MODE(3,0,(_b)), _a, _c, _d, _e)
#define Trace3b(_a, _b, _c, _d, _e, _f, _g) __trace3b(__TRACE_MODE(3,1,(_b)), _f, FP_OFF(_g), FP_SEG(_g), _a, _c, _d, _e)
#define Trace4(_a, _b, _c, _d, _e, _f) __trace4(__TRACE_MODE(4,0,(_b)), _a, _c, _d, _e, _f)
#define Trace4b(_a, _b, _c, _d, _e, _f, _g, _h) __trace4b(__TRACE_MODE(4,1,(_b)), _g, FP_OFF(_h), FP_SEG(_h), _a, _c, _d, _e, _f)
#define Trace5(_a, _b, _c, _d, _e, _f, _g) __trace5(__TRACE_MODE(5,0,(_b)), _a, _c, _d, _e, _f, _g)
#define Trace5b(_a, _b, _c, _d, _e, _f, _g, _h, _i) __trace5b(__TRACE_MODE(5,1,(_b)), _h, FP_OFF(_i), FP_SEG(_i), _a, _c, _d, _e, _f, _g)
#define Trace6(_a, _b, _c, _d, _e, _f, _g, _h) __trace6(__TRACE_MODE(6,0,(_b)), _a, _c, _d, _e, _f, _g, _h)
#define Trace6b(_a, _b, _c, _d, _e, _f, _g, _h, _i, _j) __trace6b(__TRACE_MODE(6,1,(_b)), _i, FP_OFF(_j), FP_SEG(_j), _a, _c, _d, _e, _f, _g, _h)


extern int __trace0(int, unsigned long);
extern int __trace0b(int, unsigned long, unsigned long, unsigned long, long);
extern int __trace1(int, unsigned long, int);
extern int __trace1b(int, unsigned long, unsigned long, unsigned long, long, int);
extern int __trace2(int, unsigned long, int, int);
extern int __trace2b(int, unsigned long, unsigned long, unsigned long, long, int, int);
extern int __trace3(int, unsigned long, int, int, int);
extern int __trace3b(int, unsigned long, unsigned long, unsigned long, long, int, int, int);
extern int __trace4(int, unsigned long, int, int, int, int);
extern int __trace4b(int, unsigned long, unsigned long, unsigned long, long, int, int, int, int);
extern int __trace5(int, unsigned long, int, int, int, int, int);
extern int __trace5b(int, unsigned long, unsigned long, unsigned long, long, int, int, int, int, int);
extern int __trace6(int, unsigned long, int, int, int, int, int, int);
extern int __trace6b(int, unsigned long, unsigned long, unsigned long, long, int, int, int, int, int, int);

#if __WATCOMC__ >= 950

#pragma aux (__trace_int) __trace0 = "int 0fah";
#pragma aux (__trace_int) __trace0b = "int 0fah";
#pragma aux (__trace_int) __trace1 = "int 0fah";
#pragma aux (__trace_int) __trace1b = "int 0fah";
#pragma aux (__trace_int) __trace2 = "int 0fah";
#pragma aux (__trace_int) __trace2b = "int 0fah";
#pragma aux (__trace_int) __trace3 = "int 0fah";
#pragma aux (__trace_int) __trace3b = "int 0fah";
#pragma aux (__trace_int) __trace4 = "int 0fah";
#pragma aux (__trace_int) __trace4b = "int 0fah";
#pragma aux (__trace_int) __trace5 = "int 0fah";
#pragma aux (__trace_int) __trace5b = "int 0fah";
#pragma aux (__trace_int) __trace6 = "int 0fah";
#pragma aux (__trace_int) __trace6b = "int 0fah";

#else

#pragma aux (__trace_int) __trace0 = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE0      /*  add     sp,SIZE0    */  \
                ;

#pragma aux (__trace_int) __trace0b = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE0B     /*  add     sp,SIZE0B   */  \
                ;

#pragma aux (__trace_int) __trace1 = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE1      /*  add     sp,SIZE1    */  \
                ;

#pragma aux (__trace_int) __trace1b = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE1B     /*  add     sp,SIZE1B   */  \
                ;

#pragma aux (__trace_int) __trace2 = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE2      /*  add     sp,SIZE2    */  \
                ;

#pragma aux (__trace_int) __trace2b = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE2B     /*  add     sp,SIZE2B   */  \

#pragma aux (__trace_int) __trace3 = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE3      /*  add     sp,SIZE3    */  \
                ;

#pragma aux (__trace_int) __trace3b = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE3B     /*  add     sp,SIZE3B   */  \

#pragma aux (__trace_int) __trace4 = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE4      /*  add     sp,SIZE4    */  \
                ;

#pragma aux (__trace_int) __trace4b = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE4B     /*  add     sp,SIZE4B   */  \

#pragma aux (__trace_int) __trace5 = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE5      /*  add     sp,SIZE5    */  \
                ;

#pragma aux (__trace_int) __trace5b = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE5B     /*  add     sp,SIZE5B   */  \

#pragma aux (__trace_int) __trace6 = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE6      /*  add     sp,SIZE6    */  \
                ;

#pragma aux (__trace_int) __trace6b = \
        0xCD 0xFA                    /*  int     0fah        */  \
        0x83 0xC4 __TRACE_SIZE6B     /*  add     sp,SIZE6B   */  \
                ;

#endif

#ifdef __cplusplus
};
#endif
#pragma pack();

#define __TRACE_MSG_H_INCLUDED
#endif
