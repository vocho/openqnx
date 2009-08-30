/*
 * This source code may contain confidential information of QNX Software
 * Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
 * modification, disclosure, distribution or transfer of this software,
 * or any software which includes or is based upon any of this code, is
 * prohibited unless expressly authorized by QSSL by written agreement. For
 * more information (including whether this source code file has been
 * published) please email licensing@qnx.com.
 */

/* n2d_to.c -- implementation of the NRV2D test overlap algorithm

   This file is part of the UCL real-time data compression library.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   <markus.oberhumer@jk.uni-linz.ac.at>
   http://wildsau.idv.uni-linz.ac.at/mfx/ucl.html
 */



/***********************************************************************
// entries for the different bit-buffer sizes
************************************************************************/

#include <ucl/ucl.h>
#include "ucl_conf.h"
#include "getbit.h"

#define SAFE
#define TEST_OVERLAP


UCL_PUBLIC(int)
ucl_nrv2d_test_overlap_8        ( const ucl_byte *src, ucl_uint src_off,
                                        ucl_uint  src_len, ucl_uint *dst_len,
                                        ucl_voidp wrkmem )
{
#define getbit(bb)      getbit_8(bb,src,ilen)
#include "n2d_d.c"
#undef getbit
}


UCL_PUBLIC(int)
ucl_nrv2d_test_overlap_le16     ( const ucl_byte *src, ucl_uint src_off,
                                        ucl_uint  src_len, ucl_uint *dst_len,
                                        ucl_voidp wrkmem )
{
#define getbit(bb)      getbit_le16(bb,src,ilen)
#include "n2d_d.c"
#undef getbit
}


UCL_PUBLIC(int)
ucl_nrv2d_test_overlap_le32     ( const ucl_byte *src, ucl_uint src_off,
                                        ucl_uint  src_len, ucl_uint *dst_len,
                                        ucl_voidp wrkmem )
{
    unsigned bc = 0;
#define getbit(bb)      getbit_le32(bb,bc,src,ilen)
#include "n2d_d.c"
#undef getbit
}


/*
vi:ts=4:et
*/

