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
 *  sys/elf.h
 *

 */
#ifndef __ELF_H_INCLUDED
#define __ELF_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __ELFTYPES_H_INCLUDED
#include _NTO_HDR_(sys/elftypes.h)
#endif

__BEGIN_DECLS

#define	ELF32_FSZ_ADDR		4
#define	ELF32_FSZ_OFF		4
#define	ELF32_FSZ_SWORD		4
#define	ELF32_FSZ_WORD		4
#define	ELF32_FSZ_HALF		2

#define	ELF64_FSZ_ADDR		8
#define	ELF64_FSZ_OFF		8
#define	ELF64_FSZ_SWORD		4
#define	ELF64_FSZ_SXWORD	8
#define	ELF64_FSZ_WORD		4
#define	ELF64_FSZ_XWORD		8
#define	ELF64_FSZ_HALF		2

#define ELF_IDSIZE	16
#define ELFMAG0		0177
#define ELFMAG1		'E'
#define ELFMAG2		'L'
#define ELFMAG3		'F'
#define ELFMAG		"\177ELF"
#define SELFMAG		(sizeof(ELFMAG)-1)

enum Elf_e_magic {
	EI_MAG0 = 0,
	EI_MAG1,
	EI_MAG2,
	EI_MAG3,
	EI_CLASS,
	EI_DATA,
	EI_VERSION,
	EI_OSABI,
	EI_ABIVERSION,
	EI_PAD,
	EI_NIDENT = ELF_IDSIZE
};

enum Elf_e_class {
	ELFCLASSNONE = 0,
	ELFCLASS32,
	ELFCLASS64,
	ELFCLASSNUM
};

enum Elf_e_data {
	ELFDATANONE = 0,
	ELFDATA2LSB,
	ELFDATA2MSB,
	ELFDATANUM
};

enum Elf_e_type {
	ET_NONE = 0,
	ET_REL,
	ET_EXEC,
	ET_DYN,
	ET_CORE,
	ET_NUM,
	ET_LOPROC = 0xff00,
	ET_HIPROC = 0xffff
};

enum Elf_e_machine {
	EM_NONE = 0,
	EM_M32,			/* AT&T WE 32100 */
	EM_SPARC,		/* Sun SPARC */
	EM_386,			/* Intel 80386 */
	EM_68k,			/* Motorola 68000 */
	EM_88k,			/* Motorola 88000 */
	EM_486,			/* Intel 80486 */
	EM_860,			/* Intel i860 */
	EM_MIPS,		/* MIPS RS3000 Big-Endian */
	EM_S370,		/* IBM system 370 */
	EM_MIPS_RS3_LE,	/* MIPS RS3000 Little-Endian */
	EM_RS6000,		/* RS6000 */
	EM_UNKNOWN12,
	EM_UNKNOWN13,
	EM_UNKNOWN14,
	EM_PA_RISC,		/* PA_RISC */
	EM_nCUBE,		/* nCUBE */
	EM_VPP500,		/* Fujitsu VPP500 */
	EM_SPARC32PLUS,	/* Sun Sparc 32+ */
	EM_UNKNOWN19,
	EM_PPC,			/* Power PC */
	EM_ARM = 40,	/* ARM */
	EM_SH = 42,		/* Hitachi SH */
	EM_NUM
};

enum Elf_e_osabi {
	ELFOSABI_NONE = 0,
	ELFOSABI_HPUX,		/* Hewlett-Packard HP-UX */
	ELFOSABI_NETBSD,	/* NetBSD */
	ELFOSABI_SOLARIS,	/* Sun Solaris */
	ELFOSABI_AIX,		/* AIX */
	ELFOSABI_IRIX,		/* IRIX */
	ELFOSABI_FREEBSD,	/* FreeBSD */
	ELFOSABI_TRU64, 	/* Compaq TRU64 UNIX */
	ELFOSABI_MODESTO, 	/* Novell Modesto */
	ELFOSABI_OPENBSD,	/* Open BSD */
	ELFOSABI_OPENVMS,	/* Open VMS */
	ELFOSABI_NSK,		/* Hewlett-Packard Non-Stop Kernel */
	ELFOSABI_AROS,		/* Amiga Research OS */ 	
	ELFOSABI_ARM = 97	/* ARM */
};

enum Elf_e_version {
	EV_NONE = 0,
	EV_CURRENT,
	EV_NUM
};

typedef struct {
	unsigned char	e_ident[EI_NIDENT];	/* Id bytes */
	Elf32_Half		e_type;			/* file type */
	Elf32_Half		e_machine;		/* machine type */
	Elf32_Word		e_version;		/* version number */
	Elf32_Addr		e_entry;		/* entry point */
	Elf32_Off		e_phoff;		/* Program hdr offset */
	Elf32_Off		e_shoff;		/* Section hdr offset */
	Elf32_Word		e_flags;		/* Processor flags */
	Elf32_Half      e_ehsize;		/* sizeof ehdr */
	Elf32_Half      e_phentsize;	/* Program header entry size */
	Elf32_Half      e_phnum;		/* Number of program headers */
	Elf32_Half      e_shentsize;	/* Section header entry size */
	Elf32_Half      e_shnum;		/* Number of section headers */
	Elf32_Half      e_shstrndx;		/* String table index */
} Elf32_Ehdr;

typedef struct {
	unsigned char	e_ident[EI_NIDENT];	/* Id bytes */
	Elf64_Half		e_type;			/* file type */
	Elf64_Half		e_machine;		/* machine type */
	Elf64_Word		e_version;		/* version number */
	Elf64_Addr		e_entry;		/* entry point */
	Elf64_Off		e_phoff;		/* Program hdr offset */
	Elf64_Off		e_shoff;		/* Section hdr offset */
	Elf64_Word		e_flags;		/* Processor flags */
	Elf64_Half      e_ehsize;		/* sizeof ehdr */
	Elf64_Half      e_phentsize;	/* Program header entry size */
	Elf64_Half      e_phnum;		/* Number of program headers */
	Elf64_Half      e_shentsize;	/* Section header entry size */
	Elf64_Half      e_shnum;		/* Number of section headers */
	Elf64_Half      e_shstrndx;		/* String table index */
} Elf64_Ehdr;

enum Elf_p_pf {
	PF_X = 0x00000001,
	PF_W = 0x00000002,
	PF_R = 0x00000004
};

enum Elf_p_pt {
	PT_NULL = 0,					/* Program header table entry unused */
	PT_LOAD,						/* Loadable program segment */
	PT_DYNAMIC,						/* Dynamic linking information */
	PT_INTERP,						/* Program interpreter */
	PT_NOTE,						/* Auxiliary information */
	PT_SHLIB,						/* Reserved, unspecified semantics */
	PT_PHDR,						/* Entry for header table itself */
	PT_NUM,							/* Number of p types */
	PT_COMPRESS = 0x4000,			/* QNX extension, compressed loadable segment */
	PT_SEGREL
#if __INT_BITS__ >= 32
	,
	PT_LOPROC = 0x70000000,			/* Processor-specific */
	PT_HIPROC = 0x7FFFFFFF			/* Processor-specific */
#endif
};

typedef struct {
	Elf32_Word		p_type;			/* entry type */
	Elf32_Off		p_offset;		/* offset */
	Elf32_Addr		p_vaddr;		/* virtual address */
	Elf32_Addr		p_paddr;		/* physical address */
	Elf32_Word		p_filesz;		/* file size */
	Elf32_Word		p_memsz;		/* memory size */
	Elf32_Word		p_flags;		/* flags */
	Elf32_Word		p_align;		/* memory & file alignment */
} Elf32_Phdr;

typedef struct {
	Elf64_Word		p_type;			/* entry type */
	Elf64_Word		p_flags;		/* flags */
	Elf64_Off		p_offset;		/* offset */
	Elf64_Addr		p_vaddr;		/* virtual address */
	Elf64_Addr		p_paddr;		/* physical address */
	Elf64_Xword		p_filesz;		/* file size */
	Elf64_Xword		p_memsz;		/* memory size */
	Elf64_Xword		p_align;		/* memory & file alignment */
} Elf64_Phdr;

enum Elf_s_type {
	SHT_NULL = 0,
	SHT_PROGBITS,
	SHT_SYMTAB,
	SHT_STRTAB,
	SHT_RELA,
	SHT_HASH,
	SHT_DYNAMIC,
	SHT_NOTE,
	SHT_NOBITS,
	SHT_REL,
	SHT_SHLIB,
	SHT_DYNSYM,
	SHT_COMDAT,
	SHT_NUM
#if __INT_BITS__ >= 32
	,
	SHT_QNXREL = 0x60000000,
	SHT_LOPROC = 0x70000000,
	SHT_HIPROC = 0x7fffffff,
	SHT_LOUSER = 0x80000000,
	SHT_HIUSER = 0xffffffff
#endif
};

typedef struct {
	Elf32_Word		sh_name;		/* section name */
	Elf32_Word		sh_type;		/* section type */
	Elf32_Word		sh_flags;		/* section flags */
	Elf32_Addr		sh_addr;		/* virtual address */
	Elf32_Off		sh_offset;		/* file offset */
	Elf32_Word		sh_size;		/* section size */
	Elf32_Word		sh_link;		/* misc info */
	Elf32_Word		sh_info;		/* misc info */
	Elf32_Word		sh_addralign;	/* memory alignment */
	Elf32_Word		sh_entsize;		/* entry size if table */
} Elf32_Shdr;

typedef struct {
	Elf64_Word		sh_name;		/* section name */
	Elf64_Word		sh_type;		/* section type */
	Elf64_Xword		sh_flags;		/* section flags */
	Elf64_Addr		sh_addr;		/* virtual address */
	Elf64_Off		sh_offset;		/* file offset */
	Elf64_Xword		sh_size;		/* section size */
	Elf64_Word		sh_link;		/* misc info */
	Elf64_Word		sh_info;		/* misc info */
	Elf64_Xword		sh_addralign;	/* memory alignment */
	Elf64_Xword		sh_entsize;		/* entry size if table */
} Elf64_Shdr;

enum Elf_s_flags {				/* sh_flags */
	SHF_WRITE		= 0x00000001,
	SHF_ALLOC		= 0x00000002,
	SHF_EXECINSTR	= 0x00000004
#if __INT_BITS__ >= 32
	,
	SHF_MASKPROC = 0xf0000000		/* processor specific values */
#endif
};


enum Elf_s_nums {
	SHN_UNDEF = 0,			/* special section numbers */
	SHN_LORESERVE = 0xff00,
	SHN_LOPROC = 0xff00,	/* processor specific range */
	SHN_HIPROC = 0xff1f,
	SHN_ABS = 0xfff1,
	SHN_COMMON,
	SHN_HIRESERVE = 0xffff
};

typedef struct {
	Elf32_Word		n_namesz;
	Elf32_Word		n_descsz;
	Elf32_Word		n_type;
} Elf32_Nhdr;

typedef struct {
	union {
		struct {
			Elf32_Half		srh_phndx;
			Elf32_Half		srh_align;
		}				sr_hdr;
		struct {
			Elf32_Half		sra_index;
			Elf32_Half		sra_skip;
		}				sr_addr;
		struct {
			Elf32_Half		sre_phndx;
			Elf32_Half		sre_offset;
		}				sr_ent;
	}				sr_un;
} Elf32_Segrel;

/* This info is needed when parsing the symbol table */

enum Elf_s_bind {
	STB_LOCAL = 0,
	STB_GLOBAL,
	STB_WEAK,
	STB_GLOBALOMIT,
	STB_LAZY,
	STB_NUM,
	STB_LOPROC = 13,
	STB_HIPROC = 15
};

enum Elf_st_type {
	STT_NOTYPE = 0,
	STT_OBJECT,
	STT_FUNC,
	STT_SECTION,
	STT_FILE,
	STT_NUM,
	STT_LOPROC = 13,
	STT_HIPROC = 15
};

#define ELF32_ST_BIND(x)	((x) >> 4)
#define ELF32_ST_TYPE(x)	(((unsigned int)(x)) & 0xf)
#define ELF32_ST_INFO(b, t)	(((b)<<4) | ((t) & 0xf))
#define ELF32_ST_VISIBILITY(o)	((x) & 0x3)

#define ELF64_ST_BIND(x)	((x) >> 4)
#define ELF64_ST_TYPE(x)	(((unsigned int)(x)) & 0xf)
#define ELF64_ST_INFO(b, t)	(((b)<<4) | ((t) & 0xf))
#define ELF64_ST_VISIBILITY(o)	((x) & 0x3)

/* THe following are used with relocations */
#define ELF32_R_SYM(x)		((x) >> 8)
#define ELF32_R_TYPE(x)		((x) & 0xff)
#define ELF32_R_INFO(s,t)	(((s)<<8) | ((t)&0xff))

#define ELF64_R_SYM(x)		((Elf64_Word)((x) >> 32))
#define ELF64_R_TYPE(x)		((Elf64_Word)((x) & 0xffffffff))
#define ELF64_R_INFO(s,t)	((((Elf64_Xword)(s))<<32) | ((Elf64_Xword)((t)&0xffffffff)))

typedef struct {
	Elf32_Word		st_name;
	Elf32_Addr		st_value;
	Elf32_Word		st_size;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf32_Half		st_shndx;
} Elf32_Sym;

typedef struct {
	Elf64_Word		st_name;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf64_Half		st_shndx;
	Elf64_Addr		st_value;
	Elf64_Xword		st_size;
} Elf64_Sym;

typedef struct {
	Elf32_Addr		r_offset;
	Elf32_Word		r_info;
} Elf32_Rel;

typedef struct {
	Elf64_Addr		r_offset;
	Elf64_Xword		r_info;
} Elf64_Rel;

typedef struct {
	Elf32_Addr		r_offset;
	Elf32_Word		r_info;
	Elf32_Sword		r_addend;
} Elf32_Rela;

typedef struct {
	Elf64_Addr		r_offset;
	Elf64_Xword		r_info;
	Elf64_Sxword	r_addend;
} Elf64_Rela;

enum Elf_st_comdat {
	COMDAT_NONE = 0,
	COMDAT_NOMATCH,
	COMDAT_PICKANY,
	COMDAT_SAMESIZE,
	COMDAT_NUM
};

__END_DECLS

#if !defined(ELF_TARGET_ALL) && !defined(ELF_TARGET_X86) && !defined(ELF_TARGET_PPC) && !defined(ELF_TARGET_MIPS) && !defined(ELF_TARGET_68K) && !defined(ELF_TARGET_ARM)
#if defined(__X86__)
#define ELF_TARGET_X86
#elif defined(__PPC__)
#define ELF_TARGET_PPC
#elif defined(__MIPS__)
#define ELF_TARGET_MIPS
#elif defined(__SH__)
#define ELF_TARGET_SH
#elif defined(__ARM__)
#define ELF_TARGET_ARM
#else
#error not configured for system
#endif
#endif

#if defined(ELF_TARGET_ALL) || defined(ELF_TARGET_X86)
#include _NTO_HDR_(sys/elf_386.h)
#endif

#if defined(ELF_TARGET_ALL) || defined(ELF_TARGET_PPC)
#include _NTO_HDR_(sys/elf_ppc.h)
#endif

#if defined(ELF_TARGET_ALL) || defined(ELF_TARGET_MIPS)
#include _NTO_HDR_(sys/elf_mips.h)
#endif

#if defined(ELF_TARGET_ALL) || defined(ELF_TARGET_68K)
#include _NTO_HDR_(sys/elf_68k.h)
#endif

#if defined(ELF_TARGET_ALL) || defined(ELF_TARGET_ARM)
#include _NTO_HDR_(sys/elf_arm.h)
#endif

#if defined(ELF_TARGET_ALL) || defined(ELF_TARGET_SH)
#include _NTO_HDR_(sys/elf_sh.h)
#endif

#endif
