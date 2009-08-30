/* w_j0f.c -- float version of w_j0.c.
 * Conversion to float by Ian Lance Taylor, Cygnus Support, ian@cygnus.com.
 */

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


#if defined(LIBM_SCCS) && !defined(lint)
static char rcsid[] = "$NetBSD: w_j0f.c,v 1.3 1995/05/10 20:49:13 jtc Exp $";
#endif

/*
 * wrapper j0f(float x), y0f(float x)
 */

#include "old-math.h"
#include "math_private.h"

#ifdef __STDC__
	float j0f(float x)		/* wrapper j0f */
#else
	float j0f(x)			/* wrapper j0f */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_j0f(x);
#else
	float z = __ieee754_j0f(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(x)) return z;
	if(fabsf(x)>(float)X_TLOSS) {
		/* j0f(|x|>X_TLOSS) */
	        return (float)__kernel_standard((double)x,(double)x,134);
	} else
	    return z;
#endif
}

#ifdef __STDC__
	float y0f(float x)		/* wrapper y0f */
#else
	float y0f(x)			/* wrapper y0f */
	float x;
#endif
{
#ifdef _IEEE_LIBM
	return __ieee754_y0f(x);
#else
	float z;
	z = __ieee754_y0f(x);
	if(_LIB_VERSION == _IEEE_ || isnanf(x) ) return z;
        if(x <= (float)0.0){
                if(x==(float)0.0)
                    /* d= -one/(x-x); */
                    return (float)__kernel_standard((double)x,(double)x,108);
                else
                    /* d = zero/(x-x); */
                    return (float)__kernel_standard((double)x,(double)x,109);
        }
	if(x>(float)X_TLOSS) {
		/* y0(x>X_TLOSS) */
	        return (float)__kernel_standard((double)x,(double)x,135);
	} else
	    return z;
#endif
}
