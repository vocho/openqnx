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
 *  elf_nto.h
 *

 */
#ifndef __ELF_NTO_H_INCLUDED
#define __ELF_NTO_H_INCLUDED

#if defined(__X86__)
	#define EM_NATIVE		EM_386
#elif defined(__PPC__)
	#define EM_NATIVE		EM_PPC
#elif defined(__MIPS__)
	#define EM_NATIVE		EM_MIPS
#elif defined(__SH__)
	#define EM_NATIVE		EM_SH
#elif defined(__ARM__)
	#define EM_NATIVE		EM_ARM
#else
    #error not configured for system
#endif

#if defined(__LITTLEENDIAN__)
	#define ELFDATANATIVE	ELFDATA2LSB
#elif defined(__BIGENDIAN__)
	#define ELFDATANATIVE	ELFDATA2MSB
#else
	#error endian not configured for system
#endif

#endif
