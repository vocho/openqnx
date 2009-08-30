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
 *  v86.h virtual 8086 mode
 *

 */

#ifndef __V86_H_INCLUDED
#define __V86_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include _NTO_HDR_(_pack64.h)

#define _V86_DATA_SIZE				2048
#define _V86_OPTION_CALLDIRECT_SHIFT 8

/*
 options the can be specified within the __swi argument 
 of the _intr_v86() call
*/
#define _V86_OPTION_CALLDIRECT		(1 << _V86_OPTION_CALLDIRECT_SHIFT)

struct _v86reg {
	long 	edi, esi, ebp, exx, ebx, edx, ecx, eax;
	long	eip, cs, efl;
	long	esp, ss, es, ds, fs, gs;
	} ;

/*
 The following structure is located at offset 0 in physical
 memory.
*/
struct _v86_memory {
	long	rvecs[256];					/* Offset     0h */
	char	biosdata[256];				/* Offset   400h */
	char	reserved[256];				/* Offset   500h */
	char	stack[512-128-68];			/* Offset   600h */
	char	stubcode[8];				/* Offset   73ch */
	char	execcode[120];				/* Offset   744h */
	struct _v86reg reg;					/* Offset   7bch */
	char	userdata[_V86_DATA_SIZE];	/* Offset   800h */
	char	memory[0x9f000];			/* Offset  1000h  (4K) */
	char	adaptor[0x40000];			/* Offset a0000h  (640k) */
	char	bioscode[0x20000];			/* Offset e0000h */
	} ;

__BEGIN_DECLS

extern int _intr_v86(int __swi, struct _v86reg *__regs, void *__data, int __datasize);

__END_DECLS

#include _NTO_HDR_(_packpop.h)

#endif

/* __SRCVERSION("v86.h $Rev: 153052 $"); */
