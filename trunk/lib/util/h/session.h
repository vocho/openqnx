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
 *  The posix comittee didn't embelish many session id
 *  with a functionally complete set of routines, so here's
 *  our own.....
 */


#ifndef _session_h_included
#define _session_h_included


#include <sys/sidinfo.h>


#ifdef __cplusplus
extern "C" {
#endif

#ifdef __STDC__

/* get pid of session leader. */
pid_t getspid(int sid);

/* get session id of this process */
int	getsid(pid_t pid);	
#define __oldgetsid()    getsid(getpid())
/* set name of this session... */
int	sid_name(int  sid, char *name);

#else

pid_t getspid();
int	getsid();
int	sid_name();

#endif

#ifdef __cplusplus
};
#endif
#endif
