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
 *  sys_msg.h      Standard system message definition.  Type 0 is reserved
 *         and should not be redefined by any server.
 *

 */
#ifndef __SYS_MSG_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#if __WATCOMC__ > 1000
#pragma pack(push,1);
#else
#pragma pack(1);
#endif

/*
 * Definition of type and subtype fields.
 */
#define _SYSMSG            0
#define _SYSMSG_SUBTYPE_DEATH      0
#define _SYSMSG_SUBTYPE_SIGNAL     1
#define _SYSMSG_SUBTYPE_TRACE      2
#define _SYSMSG_SUBTYPE_VERSION    3
#define _SYSMSG_SUBTYPE_SLIB       4

struct _sysmsg_hdr {
    short unsigned  type,
            subtype;
    /* One of the following (based on subtype) structures starts here */
    } ;

struct _sysmsg_death {
    short int       signum;
    long int        xstatus;
    } ;

struct _sysmsg_signal {
    long int        bits;
    } ;

struct _sysmsg_trace {
    short unsigned  op,
            point;
    long        value;
    short unsigned  datalen;
    char        data[1];    /* variable (datalen) length. */
    } ;

struct _sysmsg_version {
    short unsigned  unit;   /* Make this a zero for now. */
    } ;



/*
 * Reples to the system message. Note that not all messages have a
 * reply.
 */
struct _sysmsg_hdr_reply {
    short unsigned  status,
            zero;
    /* One of the following (based on subtype) structures starts here */
    } ;

struct _sysmsg_trace_reply {
    short unsigned  zero,
            point;
    long        value;
    short unsigned  datalen;
    char        data[1];    /* variable (datalen) length. */
    } ;

/* To compare use (((long)version) << 8) | letter */
struct _sysmsg_version_reply {
    char        name[12],
            date[12];
    short unsigned  version;
    char        letter,
            more;
    } ;

struct _sysmsg_slib_reply {
    short unsigned  naddrs;
    struct {
       unsigned long  map_offset;
       unsigned short map_segment;
       unsigned long  mem_offset;
       unsigned short mem_segment;
       } addrs[8];
    } ;

#if __WATCOMC__ > 1000
#pragma pack(pop);
#else
#pragma pack();
#endif

#define __SYS_MSG_H_INCLUDED
#endif
