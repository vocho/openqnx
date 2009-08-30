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
 *  Spawn flags passed to qnx_spawn()
 *  These are unique to QNX
 */

#include "../process.h"

#define _SPAWN_DEBUG        0x0001
#define _SPAWN_HOLD         0x0002
#define _SPAWN_BGROUND      0x0004
#define _SPAWN_NEWPGRP      0x0008
#define _SPAWN_TCSETPGRP    0x0010
#define _SPAWN_NOZOMBIE     0x0020
#define _SPAWN_XCACHE       0x0040
#define _SPAWN_SIGCLR       0x0080
#define _SPAWN_SETSID       0x0100
#define _SPAWN_NOHUP        0x0200

struct _proc_spawn;     /* for C++ */

extern pid_t qnx_spawn( int __mode, struct _proc_spawn *__msgbuf, nid_t __node,
                 int __prio, int __sched_algo, int __flags,
                 const char *__path, char **__argv, char **__envp,
                 char *__iov, int __ctfd );

extern pid_t tfork(void *stk_addr, unsigned stk_size, int (*func)(void *arg),
                   void *arg, int flags);
