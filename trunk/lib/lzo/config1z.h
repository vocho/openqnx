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




/* config1z.h -- configuration for the LZO1Z algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */


#ifndef __LZO_CONFIG1Z_H
#define __LZO_CONFIG1Z_H

#if !defined(LZO1Z)
#  define LZO1Z
#endif

#include <lzo1z.h>

#define M2_MAX_OFFSET	0x0700
#include "config1x.h"

#endif /* already included */

/*
vi:ts=4
*/

