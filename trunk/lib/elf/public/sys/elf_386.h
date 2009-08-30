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
 *  sys/elf_386.h
 *

 */
#ifndef __ELF_386_H_INCLUDED
#define __ELF_386_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __ELF_H_INCLUDED
#include _NTO_HDR_(sys/elf.h)
#endif

__BEGIN_DECLS

enum Elf_e_flags_i386 {
	EF_I386_NONE	= 0x00000000,
	EF_I386_FP		= 0x00000001,
	EF_I386_WEITEK	= 0x00000002
};

enum elf_386_e_r {
	R_386_NONE = 0,
	R_386_32,
	R_386_PC32,
	R_386_GOT32,
	R_386_PLT32,
	R_386_COPY,
	R_386_GLOB_DAT,
	R_386_JMP_SLOT,
	R_386_RELATIVE,
	R_386_GOTOFF,
	R_386_GOTPC,
	R_386_NUM,
	R_386_16 = 0x80,
	R_386_PC16
};

__END_DECLS

#endif
