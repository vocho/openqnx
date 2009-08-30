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
 *  libelf_int.h
 *

 */
#ifndef _LIBELF_INT_H_
#define _LIBELF_INT_H_

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include <unistd.h>
#include <fcntl.h>
#include _NTO_HDR_(ar.h)
#include _NTO_HDR_(libelf.h)
#include _NTO_HDR_(sys/elf_dyn.h)
#include _NTO_HDR_(sys/elf_notes.h)

/*
 * Flags reserved for library use
 */
#define ELF_F_LOADED			0x80

/* JG changed bit value to not conflict with ELF_F_DIRTY flag */
#define ELF_DATAREF_ALLOCED		0x40
/* add ELF_ARNAME_ALLOCED */

typedef struct {
	Elf_Data		*d_data;
	unsigned long	d_flags;
} Elf_Data_Ref;

struct Elf_Scn {
	unsigned short	s_ndx;
	unsigned short	s_fragments;
	Elf_Data_Ref	*s_data;
	unsigned long	s_flags;
	Elf				*s_elf;		/* back-pointer to the Elf descriptor */
};

struct Elf {
	int				e_fd;
	int				e_refcnt;	/* number of activations (elf_begin) */
	int				xlat;		/* endianess */

	off_t			e_offset;
	off_t			e_curroffset;		/* RK 961206 for bogus seek elimination */

	unsigned long	e_flags;

	off_t			e_arstrings;
	char			*e_arlntp;		/* pointer to hold longname table */
	unsigned		e_arlntsize;		/* number of bytes in longname table */

	char			*e_rawfile;

	Elf_Arsym		*e_arsymp;
	unsigned		e_numsyms;

	Elf_Arhdr		*e_arhdrp;
	Elf				*e_archive;

	Elf32_Ehdr		*e_ehdrp;
	unsigned long	e_ehdr_flags;

	Elf32_Phdr		*e_phdrp;
	unsigned long	e_phdr_flags;

	Elf32_Shdr		*e_shdrp;
	unsigned long	e_shdr_flags;

	unsigned int	e_scns;
	Elf_Scn			**e_scnp;

	Elf_Cmd			e_cmd;
};

int Elf32_swapEhdr( Elf *elf, Elf32_Ehdr *ehdr );
int Elf32_swapShdr( Elf *elf, Elf32_Shdr *shdr, int count );
int Elf32_swapPhdr( Elf *elf );
int Elf32_swapSym( Elf *elf, Elf32_Sym *sym, long size );
int Elf32_swapRela( Elf *elf, char *buf, long size, int use_rela );
int Elf32_swapDyn( Elf *elf, Elf32_Dyn *dyn );
void swap_16( void *ptr );
void swap_32( void *ptr );

#endif
