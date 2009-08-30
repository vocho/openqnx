/* _Fgpos function */
#include <errno.h>
#include <yfuns.h>
#include "xstdio.h"
_STD_BEGIN

#ifdef __QNX__

#define _Nnl(str, p1, p2)	(off64_t)((p1) < (p2) ? (p2) - (p1) : 0)

#else
int _Nnl(FILE *str, unsigned char *p1, unsigned char *p2)
	{	/* number of text bytes in nonempty buffer */

 #if _WIN32_C_LIB
	int n;

	if (str->_Mode & _MBIN)
		return (p1 < p2 ? p2 - p1 : 0);
	for (n = 0; p1 < p2; ++p1, ++n)
		if (*p1 == '\n')
			++n;
	return (n);

 #else /* _LIB version */
	return (p1 < p2 ? p2 - p1 : 0);
 #endif /* _LIB version */

	}
#endif

#ifdef __QNX__
off64_t _Fgpos(FILE *str, fpos_t *ptr)
	{	/* get file position */
	off64_t loff = _Lseek(_FD_NO(str), 0L, 1);
#else
long _Fgpos(FILE *str, fpos_t *ptr)
	{	/* get file position */
	long loff = _Lseek(_FD_NO(str), 0L, 1);
#endif

	if (!(str->_Mode & (_MOPENR|_MOPENW)) || loff == -1)
		return (EOF);

	if (str->_Mode & _MWRITE)
		loff += _Nnl(str, str->_Buf, str->_Next);
	else if (str->_Mode & _MREAD)
		loff -=
			_Nnl(str, str->_Rback,
				str ->_Back + sizeof (str->_Back))
			+ _Nnl(str, str->_Next, str->_Rsave != 0
				? str->_Rsave : str->_Rend)
			+ _Nnl(str, str->_Next, str->_WRend);

	if (ptr == 0)
		return (loff);	/* ftell */
	else
		{	/* fgetpos */
		ptr->_Off = loff;
		ptr->_Wstate = str->_Wstate;
		return (0);
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xfgpos.c $Rev: 153052 $");
