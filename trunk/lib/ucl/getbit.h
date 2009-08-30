/*
 * This source code may contain confidential information of QNX Software
 * Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
 * modification, disclosure, distribution or transfer of this software,
 * or any software which includes or is based upon any of this code, is
 * prohibited unless expressly authorized by QSSL by written agreement. For
 * more information (including whether this source code file has been
 * published) please email licensing@qnx.com.
 */

/* getbit.h -- bit-buffer access

   This file is part of the UCL real-time data compression library.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   <markus.oberhumer@jk.uni-linz.ac.at>
   http://wildsau.idv.uni-linz.ac.at/mfx/ucl.html
 */


/***********************************************************************
//
************************************************************************/

#if 1
#define getbit_8(bb, src, ilen) \
    (bb*=2,bb&0xff ? (bb>>8)&1 : ((bb=src[ilen++]*2+1)>>8)&1)
#else
#define getbit_8(bb, src, ilen) \
    (((bb*=2, bb&0xff ? bb : bb = src[ilen++]*2+1) >> 8) & 1)
#endif


#define getbit_le16(bb, src, ilen) \
    (bb*=2,bb&0xffff ? (bb>>16)&1 : (ilen+=2,((bb=(src[ilen-2]+src[ilen-1]*256U)*2+1)>>16)&1))


#if 1 && defined(UCL_UNALIGNED_OK_4) && (UCL_BYTE_ORDER == UCL_LITTLE_ENDIAN)
#define getbit_le32(bb, bc, src, ilen) \
    (bc > 0 ? ((bb>>--bc)&1) : (bc=31,\
    bb=*(const ucl_uint32p)((src)+ilen),ilen+=4,(bb>>31)&1))
#else
#define getbit_le32(bb, bc, src, ilen) \
    (bc > 0 ? ((bb>>--bc)&1) : (bc=31,\
    bb=src[ilen]+src[ilen+1]*0x100+src[ilen+2]*0x10000+src[ilen+3]*0x1000000,\
    ilen+=4,(bb>>31)&1))
#endif


/*
vi:ts=4:et
*/

