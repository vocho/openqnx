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
 *  sys/elf_dyn.h
 *

 */
#ifndef __ELF_DYN_H_INCLUDED
#define __ELF_DYN_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __ELFTYPES_H_INCLUDED
#include _NTO_HDR_(sys/elftypes.h)
#endif

__BEGIN_DECLS

typedef struct {
	Elf32_Sword		d_tag;
	union {
		Elf32_Word		d_val;
		Elf32_Addr		d_ptr;
	} d_un;
} Elf32_Dyn;

typedef struct {
	Elf64_Xword		d_tag;
	union {
		Elf64_Xword		d_val;
		Elf64_Addr		d_ptr;
	} d_un;
} Elf64_Dyn;

/* This is the info that is needed to parse the dynamic section of the file */
#define DT_NULL			0
#define DT_NEEDED		1
#define DT_PLTRELSZ		2
#define DT_PLTGOT		3
#define DT_HASH			4
#define DT_STRTAB		5
#define DT_SYMTAB		6
#define DT_RELA			7
#define DT_RELASZ		8
#define DT_RELAENT		9
#define DT_STRSZ		10
#define DT_SYMENT		11
#define DT_INIT			12
#define DT_FINI			13
#define DT_SONAME		14
#define DT_RPATH 		15
#define DT_SYMBOLIC		16
#define DT_REL	        17
#define DT_RELSZ		18
#define DT_RELENT		19
#define DT_PLTREL		20
#define DT_DEBUG		21
#define DT_TEXTREL		22
#define DT_JMPREL		23
#define DT_BIND_NOW		24
#define DT_INIT_ARRAY		25
#define DT_FINI_ARRAY		26
#define DT_INIT_ARRAYSZ		27
#define DT_FINI_ARRAYSZ		28
#define DT_RUNPATH		29
#define DT_FLAGS		30
#define DT_ENCODING		32
#define DT_PREINIT_ARRAY	32
#define DT_PREINIT_ARRAYSZ	33
#define DT_LOOS			0x6000000D
#define DT_HIOS			0x6ffff000
#define DT_LOPROC		0x70000000
#define DT_HIPROC		0x7fffffff

__END_DECLS

#endif
