/* _CWcsxfrm function */
 #include <limits.h>
 #include "xmtloc.h"
 #include "xwcsxfrm.h"
_STD_BEGIN

/* GATHER ARGUMENTS INTO A STRUCTURE */
size_t _CWcsxfrm(wchar_t *sout, const wchar_t **psin,
	size_t size, mbstate_t *ps, _Statab *pwcostate)
	{	/* translate wchar_t string to collatable form */
	const wchar_t *sin = *psin;
	size_t nout = 0;

	if (pwcostate->_Tab[0] == 0)
		{	/* no table, convert 1-to-1 */
		for (; nout < size; ++sin, ++sout)
			{	/* count and deliver a char */
			++nout;
			if ((*sout = *sin) == L'\0')
				break;
			}
		*psin = sin;	/* continue where we left off */
		return (nout);
		}
	else
		{	/* run finite state machine */
		unsigned char state = (unsigned char)ps->_State;
		int leave = 0;
		int limit = 0;
		wchar_t wc = (unsigned short)(ps->_Wchar ? ps->_Wchar : *sin);

		for (; ; )
			{	/* perform a state transformation */
			unsigned short code;
			const unsigned short *stab;

			if (_NSTATE <= state
				|| (stab = pwcostate->_Tab[state]) == 0
				|| size <= nout
				|| (_NSTATE*UCHAR_MAX) <= ++limit
				|| (code = stab[wc & UCHAR_MAX]) == 0)
				break;
			state = (unsigned char)((code & _ST_STATE) >> _ST_STOFF);
			if (code & _ST_FOLD)
				wc = (wchar_t)(wc & ~WCHAR_MAX | code & _ST_CH);
			if (code & _ST_ROTATE)
				wc = (wchar_t)((wc << CHAR_BIT) | (UCHAR_MAX
					& (wc >> (CHAR_BIT * (sizeof (wchar_t) - 1)))));
			if (code & _ST_OUTPUT
				&& ((sout[nout++] = wc) == L'\0'
				|| size <= nout))
				leave = 1;
			if (code & _ST_INPUT)
				if (*sin != L'\0')
					wc = *++sin, limit = 0;
				else
					wc = L'\0', leave = 1;
			if (leave)
				{	/* return for now */
				*psin = sin;
				ps->_State = state;
				ps->_Wchar = wc;
				return (nout);
				}
			}
		sout[nout++] = L'\0';	/* error return */
		*psin = sin;
		ps->_State = _NSTATE;
		return (nout);
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xwcsxfrm.c $Rev: 153052 $");
