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
 *  sys/elftypes.h
 *

 */
#ifndef __ELFTYPES_H_INCLUDED
#define __ELFTYPES_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

__BEGIN_DECLS

typedef _Uint32t	Elf32_Addr;
typedef _Uint32t	Elf32_Off;
typedef _Int32t		Elf32_Sword;
typedef _Uint32t	Elf32_Word;
typedef _Uint16t	Elf32_Half;

typedef _Uint64t	Elf64_Addr;
typedef _Uint64t	Elf64_Off;
typedef _Int32t		Elf64_Sword;
typedef _Int64t		Elf64_Sxword;
typedef _Uint32t	Elf64_Word;
typedef _Uint64t	Elf64_Xword;
typedef _Uint16t	Elf64_Half;

__END_DECLS

#endif
