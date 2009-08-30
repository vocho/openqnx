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
 *  sys/elf_arm.h
 *

 */
#ifndef __ELF_ARM_H_INCLUDED
#define __ELF_ARM_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __ELF_H_INCLUDED
#include _NTO_HDR_(sys/elf.h)
#endif

__BEGIN_DECLS

enum Elf_arm_e_flags {
	EF_ARM_RELEXEC		= 0x00000001,
	EF_ARM_HASENTRY		= 0x00000002,
	EF_ARM_INTERWORK	= 0x00000004,
	EF_ARM_APCS_26		= 0x00000008,
	EF_ARM_APCS_FLOAT	= 0x00000010,
	EF_ARM_PIC			= 0x00000020
};

enum Elf_arm_sym_type {
	STT_ARM_TFUNC		= STT_LOPROC
};

enum Elf_arm_p_flags {
	PF_ARM_SB			= 0x10000000
};

enum Elf_arm_s_flags {
	SHF_ARM_ENTRYSECT	= 0x10000000,
	SHF_ARM_COMDEF		= 0x80000000
};

enum Elf_arm_e_r {
	R_ARM_NONE			= 0,
	R_ARM_PC24,
	R_ARM_ABS32,
	R_ARM_REL32,
	R_ARM_ABS8,
	R_ARM_ABS16,
	R_ARM_ABS12,
	R_ARM_THM_ABS5,
	R_ARM_THM_PC22,
	R_ARM_SBREL32,
	R_ARM_AMP_VCALL9,
	R_ARM_THM_PC11,
	R_ARM_THM_PC9,
	R_ARM_COPY			= 20,
	R_ARM_GLOB_DAT,
	R_ARM_JMP_SLOT,
	R_ARM_RELATIVE,
	R_ARM_GOTOFF,
	R_ARM_GOTPC,
	R_ARM_GOT32,
	R_ARM_PLT32
};

__END_DECLS

#endif
