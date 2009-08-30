/*
 * $QNXLicenseC:
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




/*
 * this header provides basic bit-set operations.
 * 
 */


#ifndef	BIT_H
#define	BIT_H
#ifndef _STDLIB_H_INCLUDED
#include <stdlib.h>
#endif
#ifndef _STRING_H_INCLUDED
#include <string.h>
#endif
/*
 * Note that these are the machine dependant bits :-> By changing these
 * manifests and the typedef, you can adjust to word-oriented sets, which may
 * be more efficient on some machines.
 */

#define	BIT_WIDTH	8
#define	BITS_SHIFT	3
#define	BITS_MASK	7
typedef unsigned char BitEl, *BitVect;

#define	BIT_LEN(_n)	(((_n)>>BITS_SHIFT)+(((_n)&BITS_MASK) != 0))
#define	BitSET(_X,_n)	BitEl _X[BIT_LEN(_n)]

#define	_THIS_BYTE(_i)	((_i) >> BITS_SHIFT)
#define	_THIS_BIT(_i)	(1 << ((_i)&BITS_MASK))

#define	INSET(_X,_i)	((_X)[_THIS_BYTE(_i)] &   _THIS_BIT(_i))
#define	ADDSET(_X,_i)	((_X)[_THIS_BYTE(_i)] |=  _THIS_BIT(_i))
#define	DELSET(_X,_i)	((_X)[_THIS_BYTE(_i)] &= ~_THIS_BIT(_i))


#define	clearset(_X,_n)		memset((_X),0,BIT_LEN((_n)))
#define	clearbits(_X,_n)	memset((_X),0,(_n))

#define	BIT_ALLOC(_x)	(calloc(sizeof(char),BIT_LEN(_x)))



#endif
