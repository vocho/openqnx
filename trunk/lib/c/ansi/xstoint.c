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
 * This convert a string to a constant using the POSIX locale
 */

#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <ctype.h>

_STD_BEGIN

_ULonglong _Stoint(const char *nptr, char **endptr, int base, int flags) {
	int					neg, overflow, once;
	const char			*p;
	_ULonglong					ll_cutoff, ll_cutlim;
	unsigned long				l_cutoff, l_cutlim;
	_ULonglong			ll_value;
	unsigned long		l_value;
	int					n;

	/* Part 1, skip white space characters */
	p = nptr;
	while(isspace(*p)) {
		p++;
	}

	/* Part 2, Optional sign */
	neg = 0;
	if(*p == '+') {
		p++;
	} else if(*p == '-') {
		neg = 1;
		p++;
	}

	/* start with 0 for octal, 0x or 0X for hex, rest is decimal */
	if(base == 0) {
		if(*p == '0') {
			if(*(p+1) == 'x' || *(p+1) == 'X') {
				p += 2;
				base = 16;
			} else {
				base = 8;
			}
		} else {
			base = 10;
		}
	} else if(base == 16 && *p == '0' && (*(p+1) == 'x' || *(p+1) == 'X')) {
		/* Base 16 is allowed to start with 0x or 0X if there */
		p += 2;
	}

	/* PR 7424 */
	ll_cutoff = ll_cutlim = 0;
	l_cutoff = l_cutlim = 0;
	if (flags & _STOINT_LLONG) {
		if(flags & _STOINT_SIGNED) {
			ll_cutoff = neg ? -(_ULonglong)LLONG_MIN : LLONG_MAX;
		} else {
			ll_cutoff = (_ULonglong)ULLONG_MAX;
		}
		ll_cutlim = ll_cutoff % (_ULonglong)(_Longlong) base;
		ll_cutoff = ll_cutoff / (_ULonglong)(_Longlong) base;
	} else {
		if(flags & _STOINT_SIGNED) {
			l_cutoff = neg ? -(unsigned long)LONG_MIN : LONG_MAX;
		} else {
			l_cutoff = (_ULonglong)ULONG_MAX;
		}
		l_cutlim = l_cutoff % (unsigned long) base;
		l_cutoff = l_cutoff / (unsigned long) base;
	}

	ll_value = 0;
	l_value = 0;
	once = overflow = 0;
	/* Only valid bases are 2 to 36 */
	if(base >= 2 && base <= 36) {
		while((n = *p)) {
			_ULonglong					ll_save;
			unsigned long				l_save;

			/* Adjust character to from a char to an integer */
			if(n >= '0' && n <= '9') {
				n -= '0';
			} else if(n >= 'a' && n <= 'z') {
				n -= 'a' - 10;
			} else if(n >= 'A' && n <= 'Z') {
				n -= 'A' - 10;
			} else {
				/* Unknown, done */
				break;
			}
			if(n >= base) {
				/* Out of radix range, done */
				break;
			}

			/* Character is valid */
			p++;

			/* Scale, and add digit, check for overflow */
			if (flags & _STOINT_LLONG) {
				if (ll_value > ll_cutoff || (ll_value == ll_cutoff && n > ll_cutlim)) {
					overflow = 1;
				}
				ll_save = ll_value;
				ll_value = ll_value * base + n;
				if(ll_value < ll_save) {
					overflow = 1;
				}
			} else {
				if (l_value > l_cutoff || (l_value == l_cutoff && n > l_cutlim)) {
					overflow = 1;
				}
				l_save = l_value;
				l_value = l_value * base + n;
				if(l_value < l_save) {
					overflow = 1;
				}
			}
			once = 1;
		}

		/* Check for a signed number overflow */
		if(flags & _STOINT_SIGNED) {
		   if(flags & _STOINT_LLONG) {
			  if(((_Longlong)ll_value < 0) && !(neg && (ll_value == -(_ULonglong)LLONG_MIN))) {
				 overflow = 1;
			  }
		   } else {
			  if(((long)l_value < 0) && !(neg && (l_value == -(unsigned long)LONG_MIN))) {
				 overflow = 1;
			  }
		   }
		}
	}

	/* Store the end of part 2, so caller can get to part 3 */
	if(endptr) {
		*endptr = (char *)(once ? p : nptr);
	}

	/* If base not supported or no conversion was done, return 0 setting errno to EINVAL (UNIX98) */
	if(!once) {
		errno = EINVAL;
		return 0;
	}

	/* If no conversion was done, return respective max setting errno to ERANGE (UNIX98) */
	if(overflow) {
		errno = ERANGE;
		if (flags & _STOINT_LLONG) {
			return (flags & _STOINT_SIGNED) ? (neg ? LLONG_MIN : LLONG_MAX) : ULLONG_MAX;
		} else {
			return (flags & _STOINT_SIGNED) ? (neg ? LONG_MIN : LONG_MAX) : ULONG_MAX;
		}
	}

	/* Everything OK, return the number */
	if (flags & _STOINT_LLONG) {
		return neg ? -ll_value : ll_value;
	} else {
		return neg ? -l_value : l_value;
	}
}

_STD_END

__SRCVERSION("xstoint.c $Rev: 153052 $");
