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
 *  prfx.h  File prefix prototypes
 *

 */
#ifndef __PRFX_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#define _PREFIX_NAME_ONLY   0x0001

#ifdef __cplusplus
extern "C" {
#endif
extern int  qnx_prefix_attach( const char *, const char *, short unsigned );
extern int  qnx_prefix_detach( const char * );
extern int  qnx_prefix_query( nid_t, const char *, char *, int );
extern int  qnx_prefix_setroot( const char * );
extern char *qnx_prefix_getroot( void );
#ifdef __cplusplus
};
#endif

#define __PRFX_H_INCLUDED
#endif
