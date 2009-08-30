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
 *  sh/sh4acpu.h
 *

 */

#ifndef __SH_SH4ACPU_H_INCLUDED
#define __SH_SH4ACPU_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#define SH4A_STRINGNAME	"sh4a"

/*
 * Memory Mapped Register address
 */
#define SH4A_MMR_PMB_ADDR			0xf6100000
#define SH4A_MMR_PMB_DATA			0xf7100000
#define SH4A_MMR_PASCR				0xff000070

/* SH4A PASCR */
#define SH4A_PASCR_SE				_ONEBIT32L(31)

/* SH4A PMB */
#define	SH4A_PMB_ADDR_V				_ONEBIT32L(8)
#define	SH4A_PMB_DATA_UB			_ONEBIT32L(9)
#define	SH4A_PMB_DATA_V				_ONEBIT32L(8)
#define	SH4A_PMB_DATA_SZ_M			_ONEBIT32L(4) | _ONEBIT32L(7)
#define	SH4A_PMB_DATA_SZ16M			0
#define	SH4A_PMB_DATA_SZ64M			_ONEBIT32L(4)
#define	SH4A_PMB_DATA_SZ128M		_ONEBIT32L(7)
#define	SH4A_PMB_DATA_SZ512M		_ONEBIT32L(4) | _ONEBIT32L(7)
#define	SH4A_PMB_DATA_C				_ONEBIT32L(3)
#define	SH4A_PMB_DATA_WT			_ONEBIT32L(0)

/* SH4A SMP */
#define	SH4A_MMR_CPIDR				0xff000048

#define SH4A_MMR_CPIDR_CPU(v) ((v)&0xff)

#endif

/* __SRCVERSION("sh4acpu.h $Rev: 204740 $"); */
