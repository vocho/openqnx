/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice 
 * is preserved.
 * ====================================================
 */
/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */


#ifndef _OLD_MATH_H_INCLUDED
#define _OLD_MATH_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef _MATH_H_INCLUDED
#include <math.h>
#endif

#include <_pack64.h>

/*
 We are always going to use the IEEE version of the old
 math library in conjunction with the new Dinkum math
 library.
*/
#define _IEEE_LIBM 1

__BEGIN_DECLS

#if 0
#if defined(__EXT_MATH_199001)
extern double j0 __P((double)) __attribute__((__const__));
extern double j1 __P((double)) __attribute__((__const__));
extern double jn __P((int, double)) __attribute__((__const__));
extern double y0 __P((double)) __attribute__((__const__));
extern double y1 __P((double)) __attribute__((__const__));
extern double yn __P((int, double)) __attribute__((__const__));

#if !defined(_XOPEN_SOURCE)
extern float j0f __P((float)) __attribute__((__const__));
extern float j1f __P((float)) __attribute__((__const__));
extern float jnf __P((int, float)) __attribute__((__const__));
extern float y0f __P((float)) __attribute__((__const__));
extern float y1f __P((float)) __attribute__((__const__));
extern float ynf __P((int, float)) __attribute__((__const__));

#endif
#endif
#endif

__END_DECLS

#endif /* _MATH_H_ */
