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

#include <stddef.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <malloc.h>
#include <atomic.h>
#include <gulliver.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/syspage.h>
#include <sys/netmgr.h>

#define THREAD_POOL_PARAM_T		dispatch_context_t
#include <sys/dispatch.h>
#include <sys/resmgr.h>
#include <sys/iomsg.h>

#ifndef KERHDR
#include <kernel/nto.h>
#endif
#undef NKDEBUG

#include "struct.h"
#include "proto.h"
#include "procmgr_proto.h"
#include "pathmgr_proto.h"
#include "memmgr_proto.h"
#include "mclass.h"
#include "apm.h"
#include "aps.h"

#undef EXT
#undef INIT1
#undef INIT2
#undef INIT7
#undef INITSOUL

#ifndef PROCHDR
	#define EXT extern
	#define INIT1(a)
	#define INIT2(a,b)
	#define INIT7(a,b,c,d,e,f,g)
	#define INITSOUL(a,b,c,d,e)
#else
	#define EXT
	#define INIT1(a)				= a
	#define INIT2(a,b)				= { a,b }
	#define INIT7(a,b,c,d,e,f,g)	= { a,b,c,d,e,f,g }
	#define INITSOUL(a,b,c,d,e)		= { 0, a, 0, 0, sizeof(b), 0, c, c }
#endif

EXT int						root_id;
EXT int						link_root_id;
EXT paddr_t					mem_free_size;
EXT dispatch_t				*dpp;
EXT const char *			shell_path;

EXT dev_t					path_devno;

EXT unsigned				guardpagesize;
extern int procmgr_scoid;

EXT int						(rdecl *procfs_devctl_check_hook)(resmgr_context_t *ctp, io_devctl_t *msg, struct procfs_ocb *ocb);
EXT	int						(rdecl *procfs_devctl_hook)(resmgr_context_t *ctp, io_devctl_t *msg, struct procfs_ocb *ocb);

struct loader_startup;
EXT	int (*elf_load_hook)(int fd, const char *path, struct loader_startup *lsp, struct stat *statlocal, struct inheritance *parms);
EXT	int (*sys_vendor_handler_hook)(message_context_t *ctp);

//These two variables automatically created every link in the timestamp.c
//file.
extern const char			timestamp[];
extern const char			os_version_string[];

extern unsigned				__cpu_flags;

#undef INITSOUL
#define INITSOUL(a,b,c,d,e)		= { 0, a, 0, 0, sizeof(b), 0, c, c }

#ifndef INT_STRLEN_MAXIMUM
/*
 * 302 / 1000 is log10(2.0) rounded up.
 * Subtract one for the sign bit if the type is signed;
 * add one for integer division truncation;
 * add one more for a minus sign if the type is signed.
 */
#ifdef __WATCOMC__
/* watcom reports a comparison zero warning, so determine size another way */
#define INT_STRLEN_MAXIMUM(type)     (((sizeof(type) * CHAR_BIT) - (((type)-1)>>1 == -1)) * 302 / 1000 + 1 + (((type)-1)>>1 == -1))
#else
#define INT_STRLEN_MAXIMUM(type)     (((sizeof(type) * CHAR_BIT) - (((type)-1) < 0)) * 302 / 1000 + 1 + (((type)-1) < 0))
#endif
#endif

/* __SRCVERSION("externs.h $Rev: 172513 $"); */
