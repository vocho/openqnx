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




/* lzo1x_1o.c -- LZO1X-1(15) compression

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#define LZO_NEED_DICT_H
#define D_BITS			15
#define D_INDEX1(d,p)		d = DM((0x21*DX3(p,5,5,6)) >> 5)
#define D_INDEX2(d,p)		d = (d & (D_MASK & 0x7ff)) ^ (D_HIGH | 0x1f)

#include "config1x.h"

#define DO_COMPRESS		lzo1x_1_15_compress

#include "lzo1x_c.ch"
