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




/* lzo16bit.h -- configuration for the strict 16-bit memory model

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   If not, write to the Free Software Foundation, Inc.,
   59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

   Markus F.X.J. Oberhumer
   <markus.oberhumer@jk.uni-linz.ac.at>
   http://wildsau.idv.uni-linz.ac.at/mfx/lzo.html
 */


/*
 * NOTE:
 *   the strict 16-bit memory model is *not* officially supported.
 *   This file is only included for the sake of completeness.
 */


#ifndef __LZOCONF_H
#  include <lzoconf.h>
#endif

#ifndef __LZO16BIT_H
#define __LZO16BIT_H

#if defined(__LZO_STRICT_16BIT)
#if (UINT_MAX < LZO_0xffffffffL)

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
//
************************************************************************/

#ifndef LZO_99_UNSUPPORTED
#define LZO_99_UNSUPPORTED
#endif
#ifndef LZO_999_UNSUPPORTED
#define LZO_999_UNSUPPORTED
#endif

typedef unsigned int        lzo_uint;
typedef int                 lzo_int;
#define LZO_UINT_MAX        UINT_MAX
#define LZO_INT_MAX         INT_MAX

#define lzo_sizeof_dict_t   sizeof(lzo_uint)


/***********************************************************************
//
************************************************************************/

#if defined(__LZO_DOS16) || defined(__LZO_WIN16)

#if 0
#define __LZO_MMODEL        __far
#else
#define __LZO_MMODEL
#endif

#endif /* defined(__LZO_DOS16) || defined(__LZO_WIN16) */


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* (UINT_MAX < LZO_0xffffffffL) */
#endif /* defined(__LZO_STRICT_16BIT) */

#endif /* already included */

