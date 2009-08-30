/* fwide function */
#include <stdio.h>

 #if _HAS_DINKUM_CLIB
 #include "xwstdio.h"
_STD_BEGIN

int (fwide)(FILE *str, int mode)
	{	/* set/test wide/narrow file mode */
	int ans;

	_Lockfileatomic(str);
	if (mode < 0 && !(str->_Mode & _MWIDE))
		str->_Mode |= _MBYTE;
	else if (0 < mode && !(str->_Mode & _MBYTE))
		str->_Mode |= _MWIDE;
	ans = str->_Mode & _MBYTE ? -1
		: str->_Mode & _MWIDE ? 1 : 0;
	_Unlockfileatomic(str);
	return (ans);
	}
_STD_END

 #else /* _HAS_DINKUM_CLIB */

  #if defined(__GNUC__) && __GNUC__ < 3	/* compiler test */ \
	 || defined(__BORLANDC__)
   #include <stdio.h>
   #include <yvals.h>

   #if defined(__BORLANDC__)
    #pragma warn -par
   #endif /* defined(__BORLANDC__) */

_STD_BEGIN
int (fwide)(FILE *str, int mode)
	{	/* set/test wide/narrow file mode */
	return (0);
	}
_STD_END
  #endif /* defined(__GNUC__) etc. */

 #endif /* _HAS_DINKUM_CLIB */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fwide.c $Rev: 153052 $");
