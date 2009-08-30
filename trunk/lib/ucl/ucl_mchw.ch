/*
 * This source code may contain confidential information of QNX Software
 * Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
 * modification, disclosure, distribution or transfer of this software,
 * or any software which includes or is based upon any of this code, is
 * prohibited unless expressly authorized by QSSL by written agreement. For
 * more information (including whether this source code file has been
 * published) please email licensing@qnx.com.
 */

/* ucl_mchw.ch -- matching functions using a window

   This file is part of the UCL real-time data compression library.

   Copyright (C) 1996-2000 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */




/***********************************************************************
//
************************************************************************/

typedef struct
{
    int init;

    ucl_uint look;          /* bytes in lookahead buffer */

    ucl_uint m_len;
    ucl_uint m_off;

    ucl_uint last_m_len;
    ucl_uint last_m_off;

    const ucl_byte *bp;
    const ucl_byte *ip;
    const ucl_byte *in;
    const ucl_byte *in_end;
    ucl_byte *out;

    ucl_uint32 bb_b;
    unsigned bb_k;
    unsigned bb_c_endian;
    unsigned bb_c_s;
    unsigned bb_c_s8;
    ucl_byte *bb_p;
    ucl_byte *bb_op;

    struct ucl_compress_config_t conf;
    ucl_uintp result;

    ucl_progress_callback_p cb;

    ucl_uint textsize;      /* text size counter */
    ucl_uint codesize;      /* code size counter */
    ucl_uint printcount;    /* counter for reporting progress every 1K bytes */

    /* some stats */
    unsigned long lit_bytes;
    unsigned long match_bytes;
    unsigned long rep_bytes;
    unsigned long lazy;
}
UCL_COMPRESS_T;



#if defined(__PUREC__) && defined(__UCL_TOS16)
/* the cast is needed to work around a bug in Pure C */
#define getbyte(c)  ((c).ip < (c).in_end ? (unsigned) *((c).ip)++ : (-1))
#else
#define getbyte(c)  ((c).ip < (c).in_end ? *((c).ip)++ : (-1))
#endif

#include "ucl_swd.ch"



/***********************************************************************
//
************************************************************************/

static int
init_match ( UCL_COMPRESS_T *c, ucl_swd_t *s,
             const ucl_byte *dict, ucl_uint dict_len,
             ucl_uint32 flags )
{
    int r;

    assert(!c->init);
    c->init = 1;

    s->c = c;

    c->last_m_len = c->last_m_off = 0;

    c->textsize = c->codesize = c->printcount = 0;
    c->lit_bytes = c->match_bytes = c->rep_bytes = 0;
    c->lazy = 0;

    r = swd_init(s,dict,dict_len);
    if (r != UCL_E_OK)
    {
        swd_exit(s);
        return r;
    }

    s->use_best_off = (flags & 1) ? 1 : 0;
    return UCL_E_OK;
}


/***********************************************************************
//
************************************************************************/

static int
find_match ( UCL_COMPRESS_T *c, ucl_swd_t *s,
             ucl_uint this_len, ucl_uint skip )
{
    assert(c->init);

    if (skip > 0)
    {
        assert(this_len >= skip);
        swd_accept(s, this_len - skip);
        c->textsize += this_len - skip + 1;
    }
    else
    {
        assert(this_len <= 1);
        c->textsize += this_len - skip;
    }

    s->m_len = 1;
    s->m_len = THRESHOLD;
#ifdef SWD_BEST_OFF
    if (s->use_best_off)
        memset(s->best_pos,0,sizeof(s->best_pos));
#endif
    swd_findbest(s);
    c->m_len = s->m_len;
    c->m_off = s->m_off;

    swd_getbyte(s);

    if (s->b_char < 0)
    {
        c->look = 0;
        c->m_len = 0;
        swd_exit(s);
    }
    else
    {
        c->look = s->look + 1;
    }
    c->bp = c->ip - c->look;

#if 0
    /* brute force match search */
    if (c->m_len > THRESHOLD && c->m_len + 1 <= c->look)
    {
        const ucl_byte *ip = c->bp;
        const ucl_byte *m  = c->bp - c->m_off;
        const ucl_byte *in = c->in;

        if (ip - in > N)
            in = ip - N;
        for (;;)
        {
            while (*in != *ip)
                in++;
            if (in == ip)
                break;
            if (in != m)
                if (memcmp(in,ip,c->m_len+1) == 0)
                    printf("%p %p %p %5d\n",in,ip,m,c->m_len);
            in++;
        }
    }
#endif

    if (c->cb && c->textsize > c->printcount)
    {
        (*c->cb->callback)(c->textsize,c->codesize,3,c->cb->user);
        c->printcount += 1024;
    }

    return UCL_E_OK;
}


/***********************************************************************
// bit buffer
************************************************************************/

static int bbConfig(UCL_COMPRESS_T *c, int endian, int bitsize)
{
    if (endian != -1)
    {
        if (endian != 0)
            return UCL_E_ERROR;
        c->bb_c_endian = endian;
    }
    if (bitsize != -1)
    {
        if (bitsize != 8 && bitsize != 16 && bitsize != 32)
            return UCL_E_ERROR;
        c->bb_c_s = bitsize;
        c->bb_c_s8 = bitsize / 8;
    }
    c->bb_b = 0; c->bb_k = 0;
    c->bb_p = NULL;
    c->bb_op = NULL;
    return UCL_E_OK;
}


static void bbWriteBits(UCL_COMPRESS_T *c)
{
    ucl_byte *p = c->bb_p;
    ucl_uint32 b = c->bb_b;

    p[0] = UCL_BYTE(b >>  0);
    if (c->bb_c_s >= 16)
    {
        p[1] = UCL_BYTE(b >>  8);
        if (c->bb_c_s == 32)
        {
            p[2] = UCL_BYTE(b >> 16);
            p[3] = UCL_BYTE(b >> 24);
        }
    }
}


static void bbPutBit(UCL_COMPRESS_T *c, unsigned bit)
{
    assert(bit == 0 || bit == 1);
    assert(c->bb_k <= c->bb_c_s);

    if (c->bb_k < c->bb_c_s)
    {
        if (c->bb_k == 0)
        {
            assert(c->bb_p == NULL);
            c->bb_p = c->bb_op;
            c->bb_op += c->bb_c_s8;
        }
        assert(c->bb_p != NULL);
        assert(c->bb_p + c->bb_c_s8 <= c->bb_op);

        c->bb_b = (c->bb_b << 1) + bit;
        c->bb_k++;
    }
    else
    {
        assert(c->bb_p != NULL);
        assert(c->bb_p + c->bb_c_s8 <= c->bb_op);

        bbWriteBits(c);
        c->bb_p = c->bb_op;
        c->bb_op += c->bb_c_s8;
        c->bb_b = bit;
        c->bb_k = 1;
    }
}


static void bbPutByte(UCL_COMPRESS_T *c, unsigned b)
{
    /**printf("putbyte %p %p %x  (%d)\n", op, bb_p, x, bb_k);*/
    assert(c->bb_p == NULL || c->bb_p + c->bb_c_s8 <= c->bb_op);
    *c->bb_op++ = UCL_BYTE(b);
}


static void bbFlushBits(UCL_COMPRESS_T *c, unsigned filler_bit)
{
    if (c->bb_k > 0)
    {
        assert(c->bb_k <= c->bb_c_s);
        while (c->bb_k != c->bb_c_s)
            bbPutBit(c, filler_bit);
        bbWriteBits(c);
        c->bb_k = 0;
    }
    c->bb_p = NULL;
}



/*
vi:ts=4:et
*/

