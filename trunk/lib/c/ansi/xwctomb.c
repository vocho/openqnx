/*
 * $QNXtpLicenseC:
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





/* _Wctomb function */
 #include <errno.h>
 #include <limits.h>
 #include <stdlib.h>
 #include "xmtloc.h"
 #include "xwchar.h"
_STD_BEGIN

int _Wctombx(char *s, wchar_t wc, mbstate_t *pst,
	_Statab *pmbstate, _Statab *pwcstate)
	{	/* translate widechar to multibyte */
	static const mbstate_t initial = {0};

	if (pmbstate->_Tab[0] == 0)
		{	/* no table, convert to UTF8 */
		unsigned char *su = (unsigned char *)s;
		int nextra;

		if (s == 0)
			{	/* set initial state */
			*pst = initial;
			return (0);
			}

 #if WCHAR_MAX <= 0xff
		*su++ = (unsigned char)wc;
		return (1);

 #else /* WCHAR_MAX <= 0xff */
		if ((wc & (wchar_t)~0x7f) == 0)
			{	/* generate a single byte */
			*su++ = (unsigned char)wc;
			nextra = 0;
			}
		else if ((wc & (wchar_t)~0x7ff) == 0)
			{	/* generate two bytes */
			*su++ = (unsigned char)(0xc0 | wc >> 6);
			nextra = 1;
			}

  #if WCHAR_MAX <= 0xffff
		else
			{	/* generate three bytes */
			*su++ = (unsigned char)(0xe0 | (wc >> 12) & 0x0f);
			nextra = 2;
			}

  #else /* WCHAR_MAX <= 0xffff */
		else if ((wc & (wchar_t)~0xffff) == 0)
			{	/* generate three bytes */
			*su++ = (unsigned char)(0xe0 | wc >> 12);
			nextra = 2;
			}
		else if ((wc & (wchar_t)~0x1fffff) == 0)
			{	/* generate four bytes */
			*su++ = (unsigned char)(0xf0 | wc >> 18);
			nextra = 3;
			}
		else if ((wc & (wchar_t)~0x3ffffff) == 0)
			{	/* generate five bytes */
			*su++ = (unsigned char)(0xf8 | wc >> 24);
			nextra = 4;
			}
		else
			{	/* generate six bytes */
			*su++ = (unsigned char)(0xfc | (wc >> 30) & 0x03);
			nextra = 5;
			}
  #endif /* 0xffff < WCHAR_MAX */

		for (; 0 < nextra; )
			*su++ = (unsigned char)(0x80 | ((wc >> (6 * --nextra)) & 0x3f));
		return ((char *)su - s);
 #endif /* WCHAR_MAX <= 0xff */

		}
	else
		{	/* run finite state machine */
		char state = (char)pst->_State;
		int leave = 0;
		int limit = 0;
		int nout = 0;

		if (s == 0)
			{	/* set initial state */
			*pst = initial;
			return (pmbstate->_Tab[0][0] & _ST_STATE);
			}

		for (; ; )
			{	/* perform a state transformation */
			unsigned short code;
			const unsigned short *stab;

			if (_NSTATE <= state
				|| (stab = pwcstate->_Tab[(int)state]) == 0
				|| (int)MB_CUR_MAX <= nout
				|| (_NSTATE*UCHAR_MAX) <= ++limit
				|| (code = stab[wc & UCHAR_MAX]) == 0)
				{	/* report invalid sequence */
				errno = EILSEQ;
				return (-1);
				}
			state = (char)((code & _ST_STATE) >> _ST_STOFF);
			if (code & _ST_FOLD)
				wc = (wchar_t)(wc & ~UCHAR_MAX | code & _ST_CH);
			if (code & _ST_ROTATE)
				wc = (wchar_t)((wc << CHAR_BIT) | (UCHAR_MAX
					& (wc >> (CHAR_BIT * (sizeof (wchar_t) - 1)))));
			if (code & _ST_OUTPUT)
				{	/* produce an output char */
				if ((s[nout++] = (char)(code & _ST_CH ? code : wc)) == '\0')
					leave = 1;
				limit = 0;
				}
			if (code & _ST_INPUT || leave)
				{	/* consume input */
				pst->_State = state;
				return (nout);
				}
			}
		}
	}

int _Wctomb(char *s, wchar_t wc, mbstate_t *pst)
	{	/* translate widechar to multibyte */
	_Statab *pmbstate = _TLS_DATA_PTR(_Mbstate);
	_Statab *pwcstate = pmbstate == 0 || s == 0 ? 0
		: _TLS_DATA_PTR(_Wcstate);

	return (_Wctombx(s, wc, pst, pmbstate, pwcstate));
	}

_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwctomb.c $Rev: 153052 $");
