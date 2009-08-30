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
 *  ustat.h
 *

 *
 */
#ifndef _USTAT_H_INCLUDED
#define _USTAT_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __TYPES_H_INCLUDED
#include <sys/types.h>
#endif

#include <_pack64.h>

struct ustat {
    daddr_t f_tfree;        /* total free blocks */
    ino_t   f_tinode;       /* number of free inodes */
    char    f_fname[6];     /* file system name */
    char    f_fpack[6];     /* file system pack name */
};

#include <_packpop.h>

__BEGIN_DECLS

extern int ustat(dev_t __dev, struct ustat *__ub);

__END_DECLS

#endif

/* __SRCVERSION("ustat.h $Rev: 153052 $"); */
