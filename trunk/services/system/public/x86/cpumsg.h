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
 *  x86/cpumsg.h
 *

 */
#ifndef __X86_CPUMSG_H_INCLUDED
#define __X86_CPUMSG_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
#pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __SYSMSG_H_INCLUDED
#include _NTO_HDR_(sys/sysmsg.h)
#endif

#ifndef __SYSPAGE_H_INCLUDED
#include _NTO_HDR_(sys/syspage.h)
#endif

#ifndef __V86_H_INCLUDED
#include _NTO_HDR_(x86/v86.h)
#endif

enum {
	_X86_CPU_V86 = _CPUMSG_BASE
};

#include _NTO_HDR_(_pack64.h)

/*
 * Message of _X86_CPU_V86
 */
struct _x86_cpu_v86 {
	_Uint16t						type;
	_Uint16t						cputype;	/* SYSPAGE_X86 */
	_Int32t							swi;
	struct _v86reg					regs;
	/* char							userdata[_V86_DATA_SIZE]; */
};

struct _x86_cpu_v86_reply {
	_Uint32t						zero[2];
	struct _v86reg					regs;
	/* char							userdata[_V86_DATA_SIZE]; */
};

typedef union {
	struct _x86_cpu_v86				i;
	struct _x86_cpu_v86_reply		o;
} x86_cpu_v86_t;

#include _NTO_HDR_(_packpop.h)

#endif

/* __SRCVERSION("cpumsg.h $Rev: 153052 $"); */
