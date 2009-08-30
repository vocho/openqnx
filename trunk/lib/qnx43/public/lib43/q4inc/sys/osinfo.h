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
 *  osinfo.h
 *

 */
#ifndef __OSINFO_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#if __WATCOMC__ > 1000
#pragma pack(push,1);
#else
#pragma pack(1);
#endif

struct _osinfo {
    short unsigned  cpu_speed,          /*  A PC is around 960 */
                    num_procs,
                    tick_size,
                    version,
                    timesel,
                    totmemk,
                    freememk;
    char            primary_monitor,
                    secondary_monitor;
    short unsigned  machinesel,
                    zero1[3];
    unsigned long   ssinfo_offset;
    short unsigned  ssinfo_sel,
                    microkernel_size;
    char            release,
                    zero2;
    long            sflags;
    nid_t           nodename;
    long            cpu,
                    fpu;
    char            machine[16],
                    bootsrc,
                    zero3[9];
    short unsigned  num_names,
                    num_timers,
                    num_sessions,
                    num_handlers,
                    reserve64k,
                    num_semaphores,
                    zero4[5],
                    max_nodes,
                    proc_freemem,
                    cpu_loadmask,
                    fd_freemem,
                    ldt_freemem,
                                        num_fds[3],
                    pidmask,
                    zero5[26];
    } ;


#ifndef __cplusplus
volatile
#endif
struct _timesel {
    long            nsec;
    long            seconds;
    long            nsec_inc;
    } ;


struct _machinesel {
    char    primary_monitor;
    char    secondary_monitor;
    struct _cg_entry {
        short unsigned  offset,
                        segment;
        }   cg_table[10];
    } ;

struct _ssinfo {
    unsigned long   type;
    struct _ssentry_info {
        unsigned long   code_base;
        unsigned long   code_limit;
        unsigned long   entry_point;
        unsigned long   data_base;
        unsigned long   data_limit;
        unsigned long   data_offset;
        }           entry;
    } ;

/*
 *  System flag definitions
 */

#define _PSF_PROTECTED          0x0001
#define _PSF_NDP_INSTALLED      0x0002
#define _PSF_EMULATOR_INSTALLED 0x000c
#define _PSF_EMU16_INSTALLED    0x0004
#define _PSF_EMU32_INSTALLED    0x0008
#define _PSF_APM_INSTALLED      0x0010
#define _PSF_PCI_BIOS           0x2000
#define _PSF_32BIT              0x4000
#define _PSF_RESERVE_DOS        0x8000


/*
 *  Console monitor type definition.
 */
#define _MONITOR_NONE           0x00
#define _MONITOR_MONO           0x01
#define _MONITOR_CGA            0x02
#define _MONITOR_EGA_COLOR      0x04
#define _MONITOR_EGA_MONO       0x05
#define _MONITOR_PGS            0x06
#define _MONITOR_VGA_MONO       0x07
#define _MONITOR_VGA_COLOR      0x08
#define _MONITOR_PS30_MONO      0x0b
#define _MONITOR_PS30_COLOR     0x0c


#ifdef __cplusplus
extern "C" {
#endif
extern int      qnx_sflags( nid_t, long, long, long *, long * );
extern int      qnx_osinfo( nid_t, struct _osinfo * );
#ifdef __cplusplus
};
#endif

#if __WATCOMC__ > 1000
#pragma pack(pop);
#else
#pragma pack();
#endif

#define __OSINFO_H_INCLUDED
#endif
