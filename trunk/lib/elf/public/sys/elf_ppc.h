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
 *  sys/elf_ppc.h
 *

 */
#ifndef __ELF_PPC_H_INCLUDED
#define __ELF_PPC_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __ELF_H_INCLUDED
#include _NTO_HDR_(sys/elf.h)
#endif

__BEGIN_DECLS

enum Elf_ppc_e_flags {
	EF_PPC_EMB			= 0x80000000
};

enum Elf_ppc_s_type {
	SHT_ORDERED			= SHT_HIPROC
};

enum Elf_ppc_s_flags {				/* sh_flags */
	SHF_EXCLUDE			= 0x80000000
};

enum elf_ppc_e_r {
	R_PPC_NONE = 0,
	R_PPC_ADDR32,
	R_PPC_ADDR24,
	R_PPC_ADDR16,
	R_PPC_ADDR16_LO,
	R_PPC_ADDR16_HI,
	R_PPC_ADDR16_HA,
	R_PPC_ADDR14,
	R_PPC_ADDR14_BRTAKEN,
	R_PPC_ADDR14_BRNTAKEN,
	R_PPC_REL24,
	R_PPC_REL14,
	R_PPC_REL14_BRTAKEN,
	R_PPC_REL14_BRNTAKEN,
	R_PPC_GOT16,
	R_PPC_GOT16_LO,
	R_PPC_GOT16_HI,
	R_PPC_GOT16_HA,
	R_PPC_PLTREL24,
	R_PPC_COPY,
	R_PPC_GLOB_DAT,
	R_PPC_JMP_SLOT,
	R_PPC_RELATIVE,
	R_PPC_LOCAL24PC,
	R_PPC_UADDR32,
	R_PPC_UADDR16,
	R_PPC_REL32,
	R_PPC_PLT32,
	R_PPC_PLTREL32,
	R_PPC_PLT16_LO,
	R_PPC_PLT16_HI,
	R_PPC_PLT16_HA,
	R_PPC_SDAREL16,
	R_PPC_SECTOFF,
	R_PPC_SECTOFF_LO,
	R_PPC_SECTOFF_HI,
	R_PPC_SECTOFF_HA,
	R_PPC_ADDR30,		/* 37 */
	R_PPC_EMB_NADDR32 = 101,
	R_PPC_EMB_NADDR16,
	R_PPC_EMB_NADDR16_LO,
	R_PPC_EMB_NADDR16_HI,
	R_PPC_EMB_NADDR16_HA,
	R_PPC_EMB_SDAI16,
	R_PPC_EMB_SDA2I16,
	R_PPC_EMB_SDA2REL,
	R_PPC_EMB_SDA21,
	R_PPC_EMB_MRKREF,
	R_PPC_EMB_RELSEC16,
	R_PPC_EMB_RELST_LO,
	R_PPC_EMB_RELST_HI,
	R_PPC_EMB_RELST_HA,
	R_PPC_EMB_BIT_FLD,
	R_PPC_EMB_RELSDA	/* 116 */
	
};

typedef struct {
	Elf32_Half		sg_indx;
	Elf32_Half		sg_flags;
	Elf32_Word		sg_name;
	Elf32_Word		sg_info;
} Elf32_PPC_EMB_seginfo;

enum Elf_ppc_emg_sg_flags {
	PPC_EMB_SG_ROMCOPY =			0x0001
};

__END_DECLS

#endif
