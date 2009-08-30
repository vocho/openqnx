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
 *  psinfo.h
 *

 */
#ifndef __PSINFO_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#ifndef __SEGINFO_H_INCLUDED
 #include <sys/seginfo.h>
#endif

#ifndef __TIMES_H_INCLUDED
 #include <sys/times.h>
#endif

#if __WATCOMC__ > 1000
#pragma pack(push,1);
#else
#pragma pack(1);
#endif

struct _psinfo {
    short int                pid,
                             pid_zero,
                             blocked_on,
                             blocked_on_zero,
                             pid_group,
                             pid_group_zero;
    long                     flags;
    short int                rgid,
                             ruid,
                             egid,
                             euid;
    long                     sp_reg;
    short unsigned           ss_reg;
    long                     magic_off;
    short unsigned           magic_sel,
                             ldt,
                             umask;
    long                     signal_ignore,
                             signal_pending,
                             signal_mask,
                             signal_off;
    short unsigned           signal_sel;

    char                     state,
                             zero0,
                             zero0a,
                             priority,
                             max_priority,
                             sched_algorithm;

    short unsigned           sid;
    nid_t                    sid_nid;
    short unsigned           zero1[5];

    union {
        struct {
            short int                father,
                                     father_zero,
                                     son,
                                     son_zero, 
                                     brother,
                                     brother_zero,
                                     debugger,
                                     debugger_zero,
                                     mpass_pid,
                                     mpass_pid_zero;
            short unsigned           mpass_sel,
                                     mpass_flags;

            char                     name[100];
            short unsigned           links;
            time_t                   file_time;

            short unsigned           nselectors;
            time_t                   start_time;
            struct tms               times;
            short unsigned           mxcount;
            short unsigned           zero2[7];
            } proc;
        struct {
            short int                local_pid,
                                     local_pid_zero,
                                     remote_pid,
                                     remote_pid_zero,
                                     remote_vid,
                                     remote_vid_zero;
            nid_t                    remote_nid;
            short unsigned           vidseg,
                                     links;
            char                     substate,
                                     zero_v1;
            short unsigned           zero2[49];
            } vproc;
        struct {
            short unsigned           count,
                                     zero2[50];
            } mproc;
        } un;

        short unsigned zero3[12];
    } ;


struct _psinfo2 {
    short int                pid,
                             pid_zero,
                             ppid,
                             ppid_zero,
                             pid_group,
                             pid_group_zero;
    long                     flags;
    short int                rgid,
                             ruid,
                             egid,
                             euid;
    } ;


struct _psinfo3 {
    short int                egid,
                             euid;
    long                     flags;
    char                     priority,
                             max_priority;
    } ;

/*
 *  Process flags definitions
 */

#define _PPF_IMMORTAL       0x00000001L
#define _PPF_INFORM         0x00000002L
#define _PPF_FIXED          0x00000004L
#define _PPF_PRIORITY_REC   0x00000010L
#define _PPF_PRIORITY_FLOAT 0x00000020L
#define _PPF_NOCLDSTOP      0x00000040L
#define _PPF_SIGCATCH       0x00000080L
#define _PPF_SIGMASKALL     0x00000100L
#define _PPF_SERVER         0x00000200L
#define _PPF_RDOS           0x00000400L
#define _PPF_WAS_32BIT      0x00008000L     /*  RO */

#define _PPF_32BIT          0x00020000L     /*  RO */
#define _PPF_FLAT           0x00040000L     /*  RO */
#define _PPF_XIP            0x00080000L     /*  RO */
#define _PPF_VID            0x00100000L     /*  RO */
#define _PPF_MID            0x00200000L     /*  RO */
#define _PPF_VMID           0x00400000L     /*  RO */
#define _PPF_EXECING        0x01000000L     /*  RO */
#define _PPF_LOADING        0x02000000L     /*  RO */
#define _PPF_TERMING        0x04000000L     /*  RO */
#define _PPF_TO_BE_HELD     0x08000000L     /*  RO */
#define _PPF_NOZOMBIE       0x10000000L     /*  RO */
#define _PPF_SLEADER        0x20000000L     /*  RO */
#define _PPF_FORKED         0x40000000L     /*  RO */

#ifdef __cplusplus
extern "C" {
#endif
extern pid_t    qnx_psinfo( pid_t, pid_t, struct _psinfo *, unsigned, struct _seginfo *__segdata );
extern int      qnx_pflags( long, long, long *, long * );
#ifdef __cplusplus
};
#endif

#if __WATCOMC__ > 1000
#pragma pack(pop);
#else
#pragma pack();
#endif

#define __PSINFO_H_INCLUDED
#endif
