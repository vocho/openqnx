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
 *  utsname.h   UTSNAME info
 *

 */
#ifndef __UTSNAME_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#include <_pack64.h>

#define _SYSNAME_SIZE		(256 + 1)

struct utsname {
    char    sysname[_SYSNAME_SIZE],		/* SI_SYSNAME */
            nodename[_SYSNAME_SIZE],	/* SI_HOSTNAME */
            release[_SYSNAME_SIZE],		/* SI_RELEASE */
            version[_SYSNAME_SIZE],		/* SI_VERSION */
            machine[_SYSNAME_SIZE];		/* SI_MACHINE */
};

#include <_packpop.h>

__BEGIN_DECLS

extern int  uname( struct utsname *__name );

__END_DECLS

#define __UTSNAME_H_INCLUDED
#endif

/* __SRCVERSION("utsname.h $Rev: 153052 $"); */
