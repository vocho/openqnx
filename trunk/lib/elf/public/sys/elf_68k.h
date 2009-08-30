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
 *  sys/elf_68k.h
 *

 */
#ifndef __ELF_68K_H_INCLUDED
#define __ELF_68K_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __ELF_H_INCLUDED
#include _NTO_HDR_(sys/elf.h)
#endif

__BEGIN_DECLS

enum elf_68k_e_r {
	 R_68K_NONE = 0,
	 R_68K_32,
	 R_68K_16,
	 R_68K_8,
	 R_68K_PC32,
	 R_68K_PC16,
	 R_68K_PC8,
	 R_68K_GOT32,
	 R_68K_GOT16,
	 R_68K_GOT8,
	 R_68K_GOT32O,
	 R_68K_GOT16O,
	 R_68K_GOT8O,
	 R_68K_PLT32,
	 R_68K_PLT16,
	 R_68K_PLT8,
	 R_68K_PLT32O,
	 R_68K_PLT16O,
	 R_68K_PLT8O,
	 R_68K_COPY,
	 R_68K_GLOB_DAT,
	 R_68K_JMP_SLOT,
	 R_68K_RELATIVE
};

__END_DECLS

#endif
