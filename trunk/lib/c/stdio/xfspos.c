/* _Fspos function */
#include <errno.h>
#include <yfuns.h>
#include "xstdio.h"
_STD_BEGIN

#ifdef __QNX__
#define _Nnl(str, p1, p2)       (off64_t)((p1) < (p2) ? (p2) - (p1) : 0)

int _Fspos(FILE *str, const fpos_t *ptr, off64_t loff, int way)
#else
int _Nnl(FILE *, unsigned char *, unsigned char *);

int _Fspos(FILE *str, const fpos_t *ptr, long loff, int way)
#endif
	{	/* position a file */
#ifdef __QNX__
	if (!(str->_Mode & (_MOPENR | _MOPENW)) || fflush(str)) {
		errno = EBADF;	/* not open or write error */
		return EOF;
	}
	if (way != SEEK_CUR && way != SEEK_END && way != SEEK_SET) {
		errno = EINVAL;	/* invalid whence argument */
		return EOF;
	}
#else
	if (!(str->_Mode & (_MOPENR | _MOPENW)) || fflush(str))
		return (EOF);
#endif

	if (ptr)
		loff += ptr->_Off;	/* fsetpos */
	if (way == SEEK_CUR && str->_Mode & _MREAD)
		loff -=
			_Nnl(str, str->_Rback,
				str ->_Back + sizeof (str->_Back))
			+ _Nnl(str, str->_Next,
				str->_Rsave != 0 ? str->_Rsave : str->_Rend)
			+ _Nnl(str, str->_Next, str->_WRend);
	if (way == SEEK_CUR && loff != 0 || way == SEEK_END
		|| way == SEEK_SET && loff != -1)
#ifdef __QNX__
		loff = lseek64(_FD_NO(str), loff, way);
#else
		loff = _Lseek(_FD_NO(str), loff, way);
#endif

	if (loff == -1)
		return (EOF);
	else
		{	/* success */
		if (str->_Mode & (_MREAD | _MWRITE))
			{	/* empty buffer */
			str->_Next = str->_Buf;
			str->_Rend = str->_Buf, str->_WRend = str->_Buf;
			str->_Wend = str->_Buf, str->_WWend = str->_Buf;
			str->_Rback = str->_Back + sizeof (str->_Back);
			str->_WRback = str->_WBack
				+ sizeof (str->_WBack) / sizeof (wchar_t);
			str->_Rsave = 0;
			}
		if (ptr)
			str->_Wstate = ptr->_Wstate;
		str->_Mode &= ~(_MEOF | _MREAD | _MWRITE);
		return (0);
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xfspos.c $Rev: 153052 $");
