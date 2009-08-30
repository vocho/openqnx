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
/* lzo1_d.ch -- common decompression stuff

   This file is part of the LZO real-time data compression library.

   Copyright (C) 1996-1999 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   markus.oberhumer@jk.uni-linz.ac.at
 */



#if defined(LZO_TEST_DECOMPRESS_OVERRUN)
#  if !defined(LZO_TEST_DECOMPRESS_OVERRUN_INPUT)
#    define LZO_TEST_DECOMPRESS_OVERRUN_INPUT		2
#  endif
#  if !defined(LZO_TEST_DECOMPRESS_OVERRUN_OUTPUT)
#    define LZO_TEST_DECOMPRESS_OVERRUN_OUTPUT		2
#  endif
#  if !defined(LZO_TEST_DECOMPRESS_OVERRUN_LOOKBEHIND)
#    define LZO_TEST_DECOMPRESS_OVERRUN_LOOKBEHIND
#  endif
#endif


/***********************************************************************
// Overrun detection is internally handled by these macros:
//
//   TEST_IP    test input overrun at loop begin
//   NEED_IP    test input overrun at every input byte
//
//   TEST_OP    test output overrun at loop begin
//   NEED_OP    test output overrun at every output byte
//
//   TEST_LOOKBEHIND    test match postion
//
// The fastest decompressor results when testing for no overruns
// and using LZO_EOF_CODE.
************************************************************************/

#undef TEST_IP
#undef TEST_OP
#undef TEST_LOOKBEHIND
#undef NEED_IP
#undef NEED_OP
#undef HAVE_TEST_IP
#undef HAVE_TEST_OP
#undef HAVE_NEED_IP
#undef HAVE_NEED_OP
#undef HAVE_ANY_IP
#undef HAVE_ANY_OP


#if defined(LZO_TEST_DECOMPRESS_OVERRUN_INPUT)
#  if (LZO_TEST_DECOMPRESS_OVERRUN_INPUT >= 1)
#    define TEST_IP				(ip < ip_end)
#  endif
#  if (LZO_TEST_DECOMPRESS_OVERRUN_INPUT >= 2)
#    define NEED_IP(x) \
			if ((lzo_uint)(ip_end - ip) < (lzo_uint)(x))  goto input_overrun
#  endif
#endif

#if defined(LZO_TEST_DECOMPRESS_OVERRUN_OUTPUT)
#  if (LZO_TEST_DECOMPRESS_OVERRUN_OUTPUT >= 1)
#    define TEST_OP				(op <= op_end)
#  endif
#  if (LZO_TEST_DECOMPRESS_OVERRUN_OUTPUT >= 2)
#    undef TEST_OP				/* don't need both of the tests here */
#    define NEED_OP(x) \
			if ((lzo_uint)(op_end - op) < (lzo_uint)(x))  goto output_overrun
#  endif
#endif

#if defined(LZO_TEST_DECOMPRESS_OVERRUN_LOOKBEHIND)
#  define TEST_LOOKBEHIND(m_pos,out)	if (m_pos < out) goto lookbehind_overrun
#else
#  define TEST_LOOKBEHIND(m_pos,op)		((void) 0)
#endif


#if !defined(LZO_EOF_CODE) && !defined(TEST_IP)
   /* if we have no EOF code, we have to test for the end of the input */
#  define TEST_IP				(ip < ip_end)
#endif


#if defined(TEST_IP)
#  define HAVE_TEST_IP
#else
#  define TEST_IP				1
#endif
#if defined(TEST_OP)
#  define HAVE_TEST_OP
#else
#  define TEST_OP				1
#endif

#if defined(NEED_IP)
#  define HAVE_NEED_IP
#else
#  define NEED_IP(x)			((void) 0)
#endif
#if defined(NEED_OP)
#  define HAVE_NEED_OP
#else
#  define NEED_OP(x)			((void) 0)
#endif


#if defined(HAVE_TEST_IP) || defined(HAVE_NEED_IP)
#  define HAVE_ANY_IP
#endif
#if defined(HAVE_TEST_OP) || defined(HAVE_NEED_OP)
#  define HAVE_ANY_OP
#endif



/*
vi:ts=4
*/

