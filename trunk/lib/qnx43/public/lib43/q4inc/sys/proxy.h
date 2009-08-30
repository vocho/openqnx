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
 *  proxy.h     Proxy process prototypes
 *

 */
#ifndef __PROXY_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern pid_t   qnx_proxy_attach( pid_t, const void *, int, int );
extern int     qnx_proxy_detach( pid_t );
extern pid_t   qnx_proxy_rem_attach( nid_t, pid_t );
extern int     qnx_proxy_rem_detach( nid_t, pid_t );
#ifdef __cplusplus
};
#endif

#define __PROXY_H_INCLUDED
#endif
