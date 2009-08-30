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




#include <lib/compat.h>
#if defined(_NTO_HDR_DIR_)
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#ifndef __SOLARIS__
#include <stdint.h>
#endif
#include <errno.h>
#include <sys/types.h>

#if defined(__LINUX__) 
typedef unsigned long long paddr64_t;
typedef unsigned long long paddr_t;
#elif defined(__MINGW32__)
typedef unsigned long long paddr64_t;
typedef unsigned long long paddr_t;
#elif defined(__SOLARIS__)
typedef unsigned long long paddr64_t;
#endif

#define ELF_TARGET_ALL
#include _NTO_HDR_(sys/elf.h)
#include _NTO_HDR_(sys/kdump.h)
#include _NTO_HDR_(sys/kdebug.h)

typedef	uint32_t	target_ptr;
#define TARGET_NULL	((target_ptr)0)

struct cpuinfo {
	int			(*init)(int);
	int			(*v2p)(target_ptr, paddr64_t *, unsigned *);
	void		(*cvt_regset)(const void *, void *);
	unsigned	pgtbl_size;
	unsigned	gdb_regset_size;
	unsigned	ptr_size;
};

extern int			core_init(const char *);
extern unsigned		core_read_paddr(paddr64_t, void *, unsigned);
extern unsigned		core_read_vaddr(target_ptr, void *, unsigned);
extern void			core_fini(void);
extern void			set_default_dump_state(void);

extern uint16_t		endian_native16(uint16_t);
extern uint32_t		endian_native32(uint32_t);
extern uint64_t		endian_native64(uint64_t);
extern target_ptr	endian_native_ptr(target_ptr);
extern target_ptr	get_ptr_paddr(paddr64_t);
extern target_ptr	get_ptr_vaddr(target_ptr);
extern paddr64_t	find_pid(unsigned);
extern paddr64_t	find_tid(paddr64_t, unsigned *);
extern int			set_pgtbl(paddr64_t);
extern int			set_regset(paddr64_t);

extern void 		server(void);

#ifndef EXTERN
#define EXTERN	extern
#endif

EXTERN int						debug_flag;
EXTERN int						protocol;
EXTERN struct kdump_note		*note;
EXTERN struct kdump_note_cpu	*note_cpu;
EXTERN void						*pgtbl;
EXTERN struct cpuinfo			*cpu;
EXTERN void						*regset;
EXTERN paddr_t					kdump_paddr;
EXTERN paddr_t					syspage_paddr;
EXTERN int						cross_endian;

extern struct cpuinfo		*available_cpus[];
