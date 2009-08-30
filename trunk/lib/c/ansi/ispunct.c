/* ispunct function */
#include <ctype.h>
_STD_BEGIN

int (ispunct)(int c)
	{	/* test for punctuation character */
	return (_Getchrtype(c) & _PU);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("ispunct.c $Rev: 153052 $");
