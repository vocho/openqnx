/*
 * This source code may contain confidential information of QNX Software
 * Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
 * modification, disclosure, distribution or transfer of this software,
 * or any software which includes or is based upon any of this code, is
 * prohibited unless expressly authorized by QSSL by written agreement. For
 * more information (including whether this source code file has been
 * published) please email licensing@qnx.com.
 */

/* n2b_d.c -- implementation of the NRV2B decompression algorithm

   This file is part of the UCL real-time data compression library.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   <markus.oberhumer@jk.uni-linz.ac.at>
   http://wildsau.idv.uni-linz.ac.at/mfx/ucl.html
 */


/***********************************************************************
// actual implementation used by a recursive #include
************************************************************************/

#ifdef getbit

#ifdef SAFE
#define fail(x,r)   if (x) { *dst_len = olen; return r; }
#else
#define fail(x,r)
#endif

{
    ucl_uint32 bb = 0;
#ifdef TEST_OVERLAP
    ucl_uint ilen = src_off, olen = 0, last_m_off = 1;
#else
    ucl_uint ilen = 0, olen = 0, last_m_off = 1;
#endif
#ifdef SAFE
    const ucl_uint oend = *dst_len;
#endif
    UCL_UNUSED(wrkmem);

#ifdef TEST_OVERLAP
    src_len += src_off;
    fail(oend >= src_len, UCL_E_OVERLAP_OVERRUN);
#endif

    for (;;)
    {
        ucl_uint m_off, m_len;

        if (getbit(bb))
        {
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            fail(olen >= oend, UCL_E_OUTPUT_OVERRUN);
#ifdef TEST_OVERLAP
            fail(olen > ilen, UCL_E_OVERLAP_OVERRUN);
            olen++; ilen++;
#else
            dst[olen++] = src[ilen++];
#endif
            continue;
        }
        m_off = 1;
        do {
            m_off = m_off*2 + getbit(bb);
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            fail(m_off > 0xffffff + 3, UCL_E_LOOKBEHIND_OVERRUN);
        } while (!getbit(bb));
        if (m_off == 2)
        {
            m_off = last_m_off;
        }
        else
        {
            fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
            m_off = (m_off-3)*256 + src[ilen++];
            if (m_off == 0xffffffff)
                break;
            last_m_off = ++m_off;
        }
        m_len = getbit(bb);
        m_len = m_len*2 + getbit(bb);
        if (m_len == 0)
        {
            m_len++;
            do {
                m_len = m_len*2 + getbit(bb);
                fail(ilen >= src_len, UCL_E_INPUT_OVERRUN);
                fail(m_len >= oend, UCL_E_OUTPUT_OVERRUN);
            } while (!getbit(bb));
            m_len += 2;
        }
        m_len += (m_off > 0xd00);
        fail(olen + m_len > oend, UCL_E_OUTPUT_OVERRUN);
        fail(m_off > olen, UCL_E_LOOKBEHIND_OVERRUN);
#ifdef TEST_OVERLAP
        olen += m_len + 1;
        fail(olen > ilen, UCL_E_OVERLAP_OVERRUN);
#else
        {
            const ucl_byte *m_pos;
            m_pos = dst + olen - m_off;
            dst[olen++] = *m_pos++;
            do dst[olen++] = *m_pos++; while (--m_len > 0);
        }
#endif
    }
    *dst_len = olen;
    return ilen == src_len ? UCL_E_OK : (ilen < src_len ? UCL_E_INPUT_NOT_CONSUMED : UCL_E_INPUT_OVERRUN);
}

#undef fail

#endif /* getbit */


/***********************************************************************
// decompressor entries for the different bit-buffer sizes
************************************************************************/

#ifndef getbit

#include <ucl/ucl.h>
#include "ucl_conf.h"
#include "getbit.h"


UCL_PUBLIC(int)
ucl_nrv2b_decompress_8          ( const ucl_byte *src, ucl_uint  src_len,
                                        ucl_byte *dst, ucl_uint *dst_len,
                                        ucl_voidp wrkmem )
{
#define getbit(bb)      getbit_8(bb,src,ilen)
#include "n2b_d.c"
#undef getbit
}


UCL_PUBLIC(int)
ucl_nrv2b_decompress_le16       ( const ucl_byte *src, ucl_uint  src_len,
                                        ucl_byte *dst, ucl_uint *dst_len,
                                        ucl_voidp wrkmem )
{
#define getbit(bb)      getbit_le16(bb,src,ilen)
#include "n2b_d.c"
#undef getbit
}


UCL_PUBLIC(int)
ucl_nrv2b_decompress_le32       ( const ucl_byte *src, ucl_uint  src_len,
                                        ucl_byte *dst, ucl_uint *dst_len,
                                        ucl_voidp wrkmem )
{
    unsigned bc = 0;
#define getbit(bb)      getbit_le32(bb,bc,src,ilen)
#include "n2b_d.c"
#undef getbit
}


#endif /* !getbit */


/*
vi:ts=4:et
*/

