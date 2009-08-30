/*
Copyright 2002, QNX Software Systems Ltd. Unpublished Work All Rights
Reserved.

 
This source code contains confidential information of QNX Software Systems
Ltd. (QSSL). Any use, reproduction, modification, disclosure, distribution
or transfer of this software, or any software which includes or is based
upon any of this code, is only permitted under the terms of the QNX
Confidential Source License version 1.0 (see licensing.qnx.com for details)
or as otherwise expressly authorized by a written license agreement from
QSSL. For more information, please email licensing@qnx.com.
*/
/* lzo1b_tm.ch -- implementation of the LZO1B compression algorithm

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */


/* WARNING: this file should *not* be used by applications. It is
   part of the implementation of the library and is subject
   to change.
 */



/***********************************************************************
// test for a potential match
************************************************************************/


#if (DD_BITS == 0)

try_match:
#if !defined(NDEBUG) && defined(LZO_DICT_USE_PTR)
#if defined(LZO_DETERMINISTIC)
		assert(m_pos == NULL || m_pos >= in);
		assert(m_pos == NULL || m_pos < ip);
#endif
		m_pos_sav = m_pos;
#endif
		if (m_pos[0] == ip[0] && m_pos[1] == ip[1] && m_pos[2] == ip[2])
		{
			m_pos += 3;
			goto match;
		}


#else /* (DD_BITS == 0) */


	/* test potential match */

		if (m_len > M2_MIN_LEN)
			goto match;
		if (m_len == M2_MIN_LEN)
		{
#if (_MAX_OFFSET == _M2_MAX_OFFSET)
			goto match;
#else
			if (m_off <= M2_MAX_OFFSET)
				goto match;
#if 0 && (M3_MIN_LEN == M2_MIN_LEN)
			if (ip == ii)
				goto match;
#endif
#endif
		}
		goto literal;


#endif /* (DD_BITS == 0) */



/*
vi:ts=4
*/
