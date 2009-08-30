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





/* _Mbtowc function */
#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include "xmtloc.h"
#include "xwchar.h"
_STD_BEGIN

int _Mbtowcx(wchar_t *pwc, const char *s, size_t nin, mbstate_t *pst,
	_Statab *pmbstate)
	{	/* translate multibyte to widechar, given locale info */
	char state = (char)pst->_State;
	unsigned char *su = (unsigned char *)s;
	wchar_t wc = (wchar_t)pst->_Wchar;
	static const mbstate_t initial = {0};

	if (pmbstate->_Tab[0] == 0)
		{	/* no table, convert from UTF8 */
		if (s == 0)
			{	/* set initial state */
			*pst = initial;
			return (0);
			}

 #if WCHAR_MAX <= 0xff
		if (nin == 0)
			return (-2);
		else
			{	/* return a single byte */
			if (pwc != 0)
				*pwc = *su;
			return (*su == 0 ? 0 : 1);
			}

 #else /* WCHAR_MAX <= 0xff */
		for (; ; ++su, --nin)
			{	/* consume an input byte */
			if (nin == 0)
				{	/* report incomplete conversion */
				pst->_Wchar = wc;
				pst->_State = state;
				return (-2);
				}
			else if (0 < state)
				{	/* fold in a successor byte */
				if ((*su & 0xc0) != 0x80)
					{	/* report invalid sequence */
					errno = EILSEQ;
					return (-1);
					}
				wc = (wchar_t)((wc << 6) | (*su & 0x3f));
				--state;
				}
			else if ((*su & 0x80) == 0)
				wc = *su;	/* consume a single byte */
			else if ((*su & 0xe0) == 0xc0)
				{	/* consume first of two bytes */
				wc = (wchar_t)(*su & 0x1f);
				state = 1;
				}
			else if ((*su & 0xf0) == 0xe0)
				{	/* consume first of three bytes */
				wc = (wchar_t)(*su & 0x0f);
				state = 2;
				}

  #if 0xffff < WCHAR_MAX
			else if ((*su & 0xf8) == 0xf0)
				{	/* consume first of four bytes */
				wc = (wchar_t)(*su & 0x07);
				state = 3;
				}
			else if ((*su & 0xfc) == 0xf8)
				{	/* consume first of five bytes */
				wc = (wchar_t)(*su & 0x03);
				state = 4;
				}
			else if ((*su & 0xfc) == 0xfc)
				{	/* consume first of six bytes */
				wc = (wchar_t)(*su & 0x03);
				state = 5;
				}
  #endif /* 0xffff < WCHAR_MAX */

			else
				{	/* report invalid sequence */
				errno = EILSEQ;
				return (-1);
				}
			if (state == 0)
				{	/* produce an output wchar */
				if (pwc != 0)
					*pwc = wc;
				pst->_State = 0;
				return (wc == 0 ? 0 : (const char *)++su - s);
				}
			}
 #endif /* WCHAR_MAX <= 0xff */

		}
	else
		{	/* run finite state machine */
		int limit = 0;

		if (s == 0)
			{	/* set initial state */
			*pst = initial;
			return (pmbstate->_Tab[0][0] & _ST_STATE);
			}

		for (; ; )
			{	/* perform a state transformation */
			unsigned short code;
			const unsigned short *stab;

			if (nin == 0)
				{	/* report incomplete conversion */
				pst->_Wchar = wc;
				pst->_State = state;
				return (-2);
				}
			else if (_NSTATE <= state
				|| (stab = pmbstate->_Tab[(int)state]) == 0
				|| (_NSTATE*UCHAR_MAX) <= ++limit
				|| (code = stab[*su]) == 0)
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
			if (code & _ST_INPUT && *su != '\0')
				++su, --nin, limit = 0;
			if (code & _ST_OUTPUT)
				{	/* produce an output wchar */
				int nused = (const char *)su - s;

				if (pwc)
					*pwc = wc;
				pst->_Wchar = wc;
				pst->_State = state;
				return (wc == 0 ? 0 : nused == 0 ? -3 : nused);
				}
			}
		}
	}

int _Mbtowc(wchar_t *pwc, const char *s, size_t nin, mbstate_t *pst)
	{	/* translate multibyte to widechar using global locale */
	return (_Mbtowcx(pwc, s, nin, pst, _TLS_DATA_PTR(_Mbstate)));
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xmbtowc.c $Rev: 153052 $");
