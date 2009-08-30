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
 *  sys/ipc.h    UNIX98 Interprocess Communication Access Structure
 *

 */
#ifndef _IPC_H_INCLUDED
#define _IPC_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#ifdef __EXT_XOPEN_EX

#include <_pack64.h>

struct ipc_perm {
	uid_t	uid;
	gid_t	gid;
	uid_t	cuid;
	gid_t	cgid;
	mode_t	mode;
	uint_t	seq;
	key_t	key;
	int		_reserved[4];
};

#include <_packpop.h>

#define IPC_PRIVATE		((key_t)0)

#define IPC_CREAT		001000
#define IPC_EXCL		002000
#define IPC_NOWAIT		004000

#define IPC_RMID		0
#define IPC_SET			1
#define IPC_STAT		2

/*
 * UNIX98 Prototypes.
 */
__BEGIN_DECLS

extern key_t ftok(__const char *__path, int __id);

__END_DECLS

#endif

#endif

/* __SRCVERSION("ipc.h $Rev: 153052 $"); */
