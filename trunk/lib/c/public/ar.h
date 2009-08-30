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
 *  ar.h: *.a archive format.
 *

 */

#ifndef _AR_H_INCLUDED
#define _AR_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include _NTO_HDR_(_pack64.h)

#define ARMAG		"!<arch>\n"
#define SARMAG		8

#define ARFMAG		"`\n"

struct ar_hdr {
	char				ar_name[16];
	char				ar_date[12];
	char				ar_uid[6];
	char				ar_gid[6];
	char				ar_mode[8];
	char				ar_size[10];
	char				ar_fmag[2];
};

#include _NTO_HDR_(_packpop.h)

#endif

/* __SRCVERSION("ar.h $Rev: 153052 $"); */
