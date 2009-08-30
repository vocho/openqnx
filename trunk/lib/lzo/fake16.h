/*
 * $QNXtpLicenseC:
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




/* fake16.h -- fake the strict 16-bit memory model for test purposes

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   <markus.oberhumer@jk.uni-linz.ac.at>
 */


/*
 * NOTE:
 *   this file is *only* for testing the strict 16-bit memory model
 *   on a 32-bit machine. Because things like integral promotion,
 *   size_t and ptrdiff_t cannot be faked this is no real substitute
 *   for testing under a real 16-bit system.
 *
 *   See also <lzo16bit.h>
 *
 *   Usage: #include "src/fake16.h" at the top of <lzoconf.h>
 */


#ifndef __LZOFAKE16BIT_H
#define __LZOFAKE16BIT_H

#ifdef __LZOCONF_H
#  error "include this file before lzoconf.h"
#endif

#include <limits.h>

#if (USHRT_MAX == 0xffff)

#ifdef __cplusplus
extern "C" {
#endif

#define __LZO16BIT_H        /* do not use <lzo16bit.h> */

#define __LZO_STRICT_16BIT
#define __LZO_FAKE_STRICT_16BIT

#define LZO_99_UNSUPPORTED
#define LZO_999_UNSUPPORTED

typedef unsigned short      lzo_uint;
typedef short               lzo_int;
#define LZO_UINT_MAX        USHRT_MAX
#define LZO_INT_MAX         SHRT_MAX

#define lzo_sizeof_dict_t   sizeof(lzo_uint)

#if 1
#define __LZO_NO_UNALIGNED
#define __LZO_NO_ALIGNED
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

#endif /* already included */

