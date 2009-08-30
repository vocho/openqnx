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




/* lzo1y_1.c -- LZO1Y-1 compression

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


#define LZO_NEED_DICT_H
#define D_BITS			14
#define D_INDEX1(d,p)		d = DX3(p,5,5,6); d += d >> 5; d = DM(d)
#define D_INDEX2(d,p)		d = (d & (D_MASK & 0x7ff)) ^ (D_HIGH | 0x1f)

#include "config1y.h"

#define DO_COMPRESS		lzo1y_1_compress

#include "lzo1x_c.ch"
