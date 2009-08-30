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
 *  sys/procmgr.h
 *

 */
#ifndef __PROCMGR_H_INCLUDED
#define __PROCMGR_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
#pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if defined(__PID_T)
typedef __PID_T		pid_t;
#undef __PID_T
#endif

#define PROCMGR_SESSION_SETSID			0
#define PROCMGR_SESSION_TCSETSID		1
#define PROCMGR_SESSION_SETPGRP			2
#define PROCMGR_SESSION_SIGNAL_LEADER	3
#define PROCMGR_SESSION_SIGNAL_PGRP		4
#define PROCMGR_SESSION_SIGNAL_PID		5

#define PROCMGR_DAEMON_NOCHDIR			0x00000001
#define PROCMGR_DAEMON_NOCLOSE			0x00000002
#define PROCMGR_DAEMON_NODEVNULL		0x00000004
#define PROCMGR_DAEMON_KEEPUMASK		0x00000008

#define PROCMGR_EVENT_SYNC				0x00000001	/* sync() was called */
#define PROCMGR_EVENT_PATHSPACE			0x00000002  /* pathname space changed */
#define PROCMGR_EVENT_SYSCONF			0x00010000	/* a sysconf() was changed */
#define PROCMGR_EVENT_CONFSTR			0x00020000	/* a confstr() was changed */
#define PROCMGR_EVENT_DAEMON_DEATH		0x00040000	/* a process in session 1 terminated */
#define PROCMGR_EVENT_PRIVILEGED		0xffff0000

__BEGIN_DECLS

extern pid_t procmgr_guardian(pid_t __pid);
extern int procmgr_session(_Uint32t __nd, pid_t __sid, int __id, unsigned __event);
extern int procmgr_daemon(int __status, unsigned __flags);
struct sigevent;
extern int procmgr_event_notify(unsigned __flags, const struct sigevent *__event);
extern int procmgr_event_notify_add(unsigned flags, const struct sigevent *event);
extern int procmgr_event_notify_delete(int id);
extern int procmgr_event_trigger(unsigned __flags);

__END_DECLS

#endif

/* __SRCVERSION("procmgr.h $Rev: 159359 $"); */
