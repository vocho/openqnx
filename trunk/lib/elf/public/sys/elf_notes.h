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
 *  sys/elf_notes.h
 *

 */
#ifndef __ELF_NOTES_H_INCLUDED
#define __ELF_NOTES_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __ELFTYPES_H_INCLUDED
#include _NTO_HDR_(sys/elftypes.h)
#endif

__BEGIN_DECLS

#define QNX_NOTE_NAME	"QNX"
	
enum Elf_qnx_note_types {
	QNT_NULL = 0,
	QNT_DEBUG_FULLPATH,
	QNT_DEBUG_RELOC,
	QNT_STACK,
	QNT_GENERATOR,
	QNT_DEFAULT_LIB,
	QNT_CORE_SYSINFO,
	QNT_CORE_INFO,
	QNT_CORE_STATUS,
	QNT_CORE_GREG,
	QNT_CORE_FPREG,
	QNT_NUM
};

typedef struct {
	_Uint8t			major_version;
	_Uint8t			minor_version;
	_Uint16t		flags;	
} generator_version;

#define QNXELF_GEN_MAJOR	1
#define QNXELF_GEN_MINOR	0

/* Generator note flags */
enum {
	QNXELF_FLAG_DYNAMIC		= 0x0001,
	QNXELF_FLAG_PRIVATES	= 0x0002
};

__END_DECLS

#endif
