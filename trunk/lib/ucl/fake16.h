/*
 * This source code may contain confidential information of QNX Software
 * Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
 * modification, disclosure, distribution or transfer of this software,
 * or any software which includes or is based upon any of this code, is
 * prohibited unless expressly authorized by QSSL by written agreement. For
 * more information (including whether this source code file has been
 * published) please email licensing@qnx.com.
 */

/* fake16.h -- fake the strict 16-bit memory model for test purposes

   This file is part of the UCL real-time data compression library.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer

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
 *   See also <ucl/ucl16bit.h>
 *
 *   Usage: #include "src/fake16.h" at the top of <ucl/uclconf.h>
 */


#ifndef __UCLFAKE16BIT_H
#define __UCLFAKE16BIT_H

#ifdef __UCLCONF_H
#  error "include this file before uclconf.h"
#endif

#include <limits.h>

#if (USHRT_MAX == 0xffff)

#ifdef __cplusplus
extern "C" {
#endif

#define __UCL16BIT_H        /* do not use <ucl/ucl16bit.h> */

#define __UCL_STRICT_16BIT
#define __UCL_FAKE_STRICT_16BIT

#define UCL_99_UNSUPPORTED
#define UCL_999_UNSUPPORTED

typedef unsigned short      ucl_uint;
typedef short               ucl_int;
#define UCL_UINT_MAX        USHRT_MAX
#define UCL_INT_MAX         SHRT_MAX

#define ucl_sizeof_dict_t   sizeof(ucl_uint)

#if 1
#define __UCL_NO_UNALIGNED
#define __UCL_NO_ALIGNED
#endif

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

#endif /* already included */

