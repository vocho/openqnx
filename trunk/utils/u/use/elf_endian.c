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

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <stdlib.h>
#include <malloc.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include _NTO_HDR_(sys/elf.h)
#include _NTO_HDR_(gulliver.h)

void endian_adjust_ehdr(Elf32_Ehdr *ehdr,int flip)
{
	if (flip==0) return;

    /*  unsigned char   e_ident[EI_NIDENT] */
    /*  ELF32_E_TYPE    e_type             */                             
    /*  (elf32-half) ELF32_E_MACHINE e_machine */
	ENDIAN_SWAP16(&ehdr->e_machine);

	/*  ELF32_E_VERSION e_version */                       
    ENDIAN_SWAP32(&ehdr->e_version);

	/*  Elf32_Addr      e_entry; */
    ENDIAN_SWAP32(&ehdr->e_entry);

	/*  Elf32_Off       e_phoff; */
	ENDIAN_SWAP32(&ehdr->e_phoff);

	/*  Elf32_Off       e_shoff; */
    ENDIAN_SWAP32(&ehdr->e_shoff);

	/*  Elf32_Word      e_flags; */
    ENDIAN_SWAP32(&ehdr->e_flags);

	/*  Elf32_Half      e_ehsize; */
    ENDIAN_SWAP16(&ehdr->e_ehsize);

	/*  Elf32_Half      e_phentsize; */
    ENDIAN_SWAP16(&ehdr->e_phentsize);

	/*  Elf32_Half      e_phnum; */
	ENDIAN_SWAP16(&ehdr->e_phnum);

	/*  Elf32_Half      e_shentsize */
	ENDIAN_SWAP16(&ehdr->e_shentsize);

	/*  Elf32_Half      e_shnum; */
    ENDIAN_SWAP16(&ehdr->e_shnum);

	/*  Elf32_Half      e_shstrndx; */
	ENDIAN_SWAP16(&ehdr->e_shstrndx);

    return;
}


void endian_adjust_shdrs(Elf32_Shdr *shdr,int nentries,int flip)
{
	int i;

	if (flip==0) return;

	for (i=0;i<nentries;i++) {
		/*  Elf32_Word      sh_name; */
		ENDIAN_SWAP32(&shdr[i].sh_name);
		/*  ELF32_S_TYPE    sh_type; */
		ENDIAN_SWAP32(&shdr[i].sh_type);
		/*  ELF32_S_FLAGS   sh_flags; */
		ENDIAN_SWAP32(&shdr[i].sh_flags);
		/*  Elf32_Addr      sh_addr; */
	    ENDIAN_SWAP32(&shdr[i].sh_addr);
		/*  Elf32_Off       sh_offset; */
		ENDIAN_SWAP32(&shdr[i].sh_offset);
		/*  Elf32_Word      sh_size; */
		ENDIAN_SWAP32(&shdr[i].sh_size);
		/*  Elf32_Word      sh_link; */
		ENDIAN_SWAP32(&shdr[i].sh_link);
		/*  Elf32_Word      sh_info; */
		ENDIAN_SWAP32(&shdr[i].sh_info);
		/*  Elf32_Word      sh_addralign */
		ENDIAN_SWAP32(&shdr[i].sh_addralign);
		/*  Elf32_Word      sh_entsize; */
		ENDIAN_SWAP32(&shdr[i].sh_entsize);
	}

	return;
}

__SRCVERSION("elf_endian.c $Rev: 153052 $");
