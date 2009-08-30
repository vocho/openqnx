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



/*-


 login.h: library of routines for login family.
*/


#ifndef _LIBLOGIN_H_INCLUDED
#define _LIBLOGIN_H_INCLUDED

#ifndef __QNXNTO__
#ifndef __SIDINFO_H_INCLUDED
#include <sys/sidinfo.h>
#endif
#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif
#endif
#include <util/util_limits.h>

#define	TXT(s)				(s)
#define N_CNTRL_TERM 		"login: warning- no controlling terminal"
#define T_LOGIN_FAIL 		"login: login incorrect"
#define T_SESSION_LEADER	"login: not login shell"
#define T_MUST_TTY			"login: must be on a tty!"
#define	T_NO_MEMORY			"login: no memory!"
#define	T_NO_SHELL 			"login: no shell!"
#define	T_NO_PASSFILE		"login: no passwd file!"
#define	T_OWN_ME			"login: passwd file must be owned by %s\n"
#define	T_PASSWD_FTYPE		"login: passwd file must be regular file\n"

__BEGIN_DECLS

extern int      getsid(int pid);
extern int      lchk_passwd(void);

__END_DECLS

#endif
