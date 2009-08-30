/* fflush function */
#include "xstdio.h"
#include "yfuns.h"
_STD_BEGIN

#ifdef __QNX__
int (flushall)(void)
	{	/* recurse on all streams */
	return (fflush(0));
	}
#endif

int (fflush)(FILE *str)
	{	/* flush an output stream */
	int n;
	unsigned char *s;

	if (str == 0)
		{	/* recurse on all streams */
#ifdef __QNX__
		FILE *fp;
		int stat;

		_Locksyslock(_LOCK_STREAM);
		for (stat = 0, fp = _Files[0]; fp; fp = fp->_NextFile)
			if (_CSTD fflush(fp) < 0)
				stat = EOF;
		_Unlocksyslock(_LOCK_STREAM);
#else
		int nf, stat;

		_Locksyslock(_LOCK_STREAM);
		for (stat = 0, nf = 0; nf < FOPEN_MAX; ++nf)
			if (_Files[nf] != 0 && _CSTD fflush(_Files[nf]) < 0)
				stat = EOF;
		_Unlocksyslock(_LOCK_STREAM);
#endif
		return (stat);
		}

	_Lockfileatomic(str);
	if (!(str->_Mode & _MWRITE))
		{	/* stream not writable */
		_Unlockfileatomic(str);
		return (0);
		}
	for (s = str->_Buf; s < str->_Next; s += n)
		{	/* try to write buffer */
		n = _Fwrite(str, s, str->_Next - s);
		if (n <= 0)
			{	/* report error and fail */
			str->_Next = str->_Buf;
			str->_Wend = str->_Buf, str->_WWend = str->_Buf;
			str->_Mode |= _MERR;
			_Unlockfileatomic(str);
			return (EOF);
			}
		}
	str->_Next = str->_Buf;
	str->_Wend = str->_Buf, str->_WWend = str->_Buf;
	str->_Mode &= ~_MWRITE;
	_Unlockfileatomic(str);
	return (0);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fflush.c $Rev: 153052 $");
