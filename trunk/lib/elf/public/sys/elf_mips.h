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
 *  sys/elf_mips.h
 *

 */
#ifndef __ELF_MIPS_H_INCLUDED
#define __ELF_MIPS_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __ELF_H_INCLUDED
#include _NTO_HDR_(sys/elf.h)
#endif

__BEGIN_DECLS

enum Elf_mips_e_flags {
	EF_MIPS_NONE		= 0x00000000,
	EF_MIPS_NOREORDER	= 0x00000001,
	EF_MIPS_PIC			= 0x00000002,
	EF_MIPS_CPIC		= 0x00000004,
	E_MIPS_ARCH_1		= 0x00000000,
	E_MIPS_ARCH_2		= 0x10000000,
	E_MIPS_ARCH_3		= 0x20000000,
	EF_MIPS_ARCH		= 0xf0000000
};

enum Elf_mips_p_pt {
	PT_MIPS_REGINFO		= 0x70000000,
	PT_MIPS_OPTIONS		= 0x70000001	/* 64-bit */
};

enum Elf_mips_s_nums {
	SHN_MIPS_ACOMMON	= SHN_LOPROC + 0,
	SHN_MIPS_TEXT		= SHN_LOPROC + 1,
	SHN_MIPS_DATA		= SHN_LOPROC + 2,
	SHN_MIPS_SCOMMON	= SHN_LOPROC + 3,
	SHN_MIPS_SUNDEFINED	= SHN_LOPROC + 4
};

enum Elf_mips_s_type {
	SHT_MIPS_LIBLIST	= SHT_LOPROC + 0,
	SHT_MIPS_RTPROC		= SHT_LOPROC + 1,	/* Ada exceptions */
	SHT_MIPS_CONFLICT	= SHT_LOPROC + 2,
	SHT_MIPS_GPTAB		= SHT_LOPROC + 3,
	SHT_MIPS_UCODE		= SHT_LOPROC + 4,
	SHT_MIPS_DEBUG		= SHT_LOPROC + 5,	/* ECOFF debugging info */
	SHT_MIPS_REGINFO	= SHT_LOPROC + 6,
	SHT_MIPS_OPTIONS	= SHT_LOPROC + 13,
	SHT_MIPS_DWARF		= SHT_LOPROC + 30,	/* Dwarf debugging info */
	SHT_MIPS_EVENTS		= SHT_LOPROC + 33
};

enum Elf_mips_s_flags {				/* sh_flags */
	SHF_MIPS_GPREL		= 0x10000000,
	SHF_MIPS_NOSTRIB	= 0x08000000,	/* 64-bit only */
	SHF_MIPS_MERGE		= 0x20000000	/* 64-bit only */
};

enum elf_mips_e_r {
	R_MIPS_NONE = 0,
	R_MIPS_16,
	R_MIPS_32,
	R_MIPS_REL32,
	R_MIPS_REL = R_MIPS_REL32,
	R_MIPS_26,
	R_MIPS_HI16,
	R_MIPS_LO16,
	R_MIPS_GPREL16,
	R_MIPS_LITERAL,
	R_MIPS_GOT16,
	R_MIPS_PC16,
	R_MIPS_CALL16,
	R_MIPS_GPREL32,
	R_MIPS_GOTHI16,
	R_MIPS_GOTLO16,
	R_MIPS_CALLHI16,
	R_MIPS_CALLLO16,
	R_MIPS_64 = 18,
	R_MIPS_GOT_DISP,
	R_MIPS_GOT_PAGE,
	R_MIPS_GOT_OFST,
	R_MIPS_GOT_HI16,
	R_MIPS_GOT_LO16,
	R_MIPS_SUB,
	R_MIPS_HIGHER = 28,
	R_MIPS_HIGHEST,
	R_MIPS_CALL_HI16,
	R_MIPS_CALL_LO16,
	R_MIPS_SCN_DISP,
	R_MIPS_SCN_PJUMP = 35,
	R_MIPS_SCN_JALR = 37,
	R_MIPS_LOVENDOR = 100,
	R_MIPS_QNX_COPY = 126,		/* Extension. Don't conflict with MIPS16 relocs */
	R_MIPS_HIVENDOR = 127
};


#define DT_MIPS_RLD_VERSION  0x70000001	/* Runtime linker interface version */
#define DT_MIPS_TIME_STAMP   0x70000002	/* Timestamp */
#define DT_MIPS_ICHECKSUM    0x70000003	/* Checksum */
#define DT_MIPS_IVERSION     0x70000004	/* Version string (string tbl index) */
#define DT_MIPS_FLAGS	     0x70000005	/* Flags */
#define DT_MIPS_BASE_ADDRESS 0x70000006	/* Base address */
#define DT_MIPS_CONFLICT     0x70000008	/* Address of CONFLICT section */
#define DT_MIPS_LIBLIST	     0x70000009	/* Address of LIBLIST section */
#define DT_MIPS_LOCAL_GOTNO  0x7000000a	/* Number of local GOT entries */
#define DT_MIPS_CONFLICTNO   0x7000000b	/* Number of CONFLICT entries */
#define DT_MIPS_LIBLISTNO    0x70000010	/* Number of LIBLIST entries */
#define DT_MIPS_SYMTABNO     0x70000011	/* Number of DYNSYM entries */
#define DT_MIPS_UNREFEXTNO   0x70000012	/* First external DYNSYM */
#define DT_MIPS_GOTSYM	     0x70000013	/* First GOT entry in DYNSYM */
#define DT_MIPS_HIPAGENO     0x70000014	/* Number of GOT page table entries */
#define DT_MIPS_RLD_MAP	     0x70000016	/* Address of run time loader map.  */
#define DT_MIPS_LOCAL_GOTIDX 0x70000026 /* 64-bit index of first local */
#define DT_MIPS_HIDDEN_GOTIDX 0x70000027 /* 64-bit index of first hidden symbol */
#define DT_MIPS_PROTECTED_GOTIDX 0x70000028 /* 64-bit index of first protected symbol */
#define DT_MIPS_OPTIONS      0x70000029 /* 64-bit address of .MIPS.options section */
#define DT_MIPS_EXTND_GOTSYM 0x70000100
#define DT_MIPS_REL32_VERSION 0x7fffff00 /* Identify new R_MIPS_REL32 relocs */
#define DT_MIPS_NUM	    	 0x101

typedef struct
{
	Elf32_Word		ri_gprmask;
	Elf32_Word		ri_cprmask[ 4 ];
	Elf32_Sword		ri_gp_value;
} Elf32_mips_RegInfo;


typedef union
{
	struct
	{
		Elf32_Word	gt_current_g_value;
		Elf32_Word	gt_unused;
	} gt_header;

	struct
	{
		Elf32_Word	gt_g_value;
		Elf32_Word	gt_bytes;
	} gt_entry;
} Elf32_mips_gptable;

__END_DECLS

#endif
