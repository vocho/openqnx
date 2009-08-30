/*
 * This source code may contain confidential information of QNX Software
 * Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
 * modification, disclosure, distribution or transfer of this software,
 * or any software which includes or is based upon any of this code, is
 * prohibited unless expressly authorized by QSSL by written agreement. For
 * more information (including whether this source code file has been
 * published) please email licensing@qnx.com.
 */

/* n2b_ds.c -- implementation of the NRV2B decompression algorithm

   This file is part of the UCL real-time data compression library.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   <markus.oberhumer@jk.uni-linz.ac.at>
   http://wildsau.idv.uni-linz.ac.at/mfx/ucl.html
 */


#define SAFE
#define ucl_nrv2b_decompress_8      ucl_nrv2b_decompress_safe_8
#define ucl_nrv2b_decompress_le16   ucl_nrv2b_decompress_safe_le16
#define ucl_nrv2b_decompress_le32   ucl_nrv2b_decompress_safe_le32
#include "n2b_d.c"
#undef SAFE


/*
vi:ts=4:et
*/

