/* fgetwc function */
#include <string.h>
#include "xwstdio.h"
_STD_BEGIN

wint_t (fgetwc)(FILE *str)
	{	/* get a wchar_t from wide stream */
	_Lockfileatomic(str);
	if (str->_WRback < str->_WBack
		 + sizeof (str->_WBack) / sizeof (wchar_t))
		{	/* deliver putback character */
		wint_t ch = *str->_WRback++;

		_Unlockfileatomic(str);
		return (ch);
		}

	for (; ; )
		{	/* loop until wide char built */
		int nc;
		size_t nbuf;
		size_t nback = str->_Back + sizeof (str->_Back) - str->_Rback;
		unsigned char *pbuf;
		wchar_t wc;

		if (0 < nback && (str->_Mode & _MWIDE) != 0)
			pbuf = str->_Rback, nbuf = nback;
		else if (str->_Next < str->_WRend || 0 < _WFrprep(str))
			pbuf = str->_Next, nbuf = str->_WRend - str->_Next;
		else
			{	/* nothing to read */
			_Unlockfileatomic(str);
			return (WEOF);
			}

		switch (nc = _Mbtowc(&wc, (const char *)pbuf, nbuf, &str->_Wstate))
			{	/* check completion code */
		case -2:	/* not done yet */
			if (sizeof (str->_Back) <= nbuf)
				nback = 0;	/* more chars won't help, signal failure */
			else if (nback == 0)
				{	/* set up buffer in str->_Back area */
				str->_Rback = str->_Back + sizeof (str->_Back) - nbuf;
				memcpy(str->_Rback, str->_Next, nbuf);
				str->_Next += nbuf;
				nback = nbuf;
				}

			if (nback == 0)
				;	/* report failure */
			else if (0 < _WFrprep(str))
				{	/* add chars to _Back buffer and retry */
				nbuf = str->_WRend - str->_Next;
				if (sizeof (str->_Back) - nback < nbuf)
					nbuf = sizeof (str->_Back) - nback;
				pbuf = ((str->_Back + sizeof (str->_Back)) - nbuf) - nback;
				memmove(pbuf, str->_Rback, nback);
				memcpy(pbuf + nback, str->_Next, nbuf);
				str->_Rback = pbuf;
				str->_Next += nbuf;
				break;
				}
			/* fall through */

		case -1:	/* bad multibyte character */
			str->_Mode |= _MERR;
			_Unlockfileatomic(str);
			return (WEOF);

		case 0:	/* may be null character */
			if (wc == L'\0')
				nc = strlen((const char *)pbuf) + 1;
			/* fall through */

		default:	/* got a wide character */
			if (nc == -3)
				nc = 0;	/* generated a wide character from state info */
			if (0 < nback)
				str->_Rback += nc;
			else
				str->_Next +=  nc;
			_Unlockfileatomic(str);
			return (wc);
			}
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("fgetwc.c $Rev: 153052 $");
