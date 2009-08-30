/* _CStrxfrm function */
#include <limits.h>
#include "xstrxfrm.h"
_STD_BEGIN

	/* Posix flags */
 #define FORWARD	_ST_FOLD	/* code word flags */
 #define INPUT		_ST_INPUT
 #define OUTPUT		_ST_OUTPUT
 #define REPEAT		_ST_ROTATE

 #define INVERT		_ST_INPUT	/* state flags, including FORWARD */
 #define OFFSETS	_ST_OUTPUT

size_t _CStrxfrm(char *sout, size_t size, Xfrm *ps,
	_Statab *pcostate)
	{	/* translate string to collatable form */
	size_t nout = 0;

	if (pcostate->_Tab[0] == 0)
		{	/* no table, convert 1-to-1 */
		for (ps->sin = ps->sbegin; nout < size; ++ps->sin, ++sout)
			{	/* count and deliver a char */
			++nout;
			if ((*sout = *ps->sin) == '\0')
				break;
			}
		ps->sbegin = ps->sin;	/* continue where we left off */
		return (nout);
		}
	else if ((pcostate->_Tab[0][0] & 0xfff) != 0)
		{	/* run Posix conversion */
		unsigned short ignores = 0;

		if (ps->state == 0)
			{	/* initialize */
			ps->state = pcostate->_Tab[0][0] & ~0xfff;
			if ((ps->state & INVERT) == 0)
				ps->sin = ps->sbegin;
			else
				{	/* backwards, memorize end pointer */
				ps->sin += strlen((const char *)ps->sin);
				ps->send = ps->sin;
				}
			}
		for (; nout < size; )
			{	/* eat an input char and/or produce an output char */
			unsigned short code;

			if (ps->weight != 0)
				if (ignores != 0)
					{	/* put and clear ignores */
					sout[nout++] = (char)ignores;
					ignores = 0;
					}
				else if (0x80 <= ps->weight)
					{	/* put first of weight */
/* provide for bigger weights */
					sout[nout++] = (char)(0x80 | (ps->weight >> 6));
					ps->weight = 0x40 | (ps->weight & 0x3f);
					}
				else
					{	/* put last of weight */
					sout[nout++] = (char)ps->weight;
					ps->weight = 0;
					}
			else if (ps->wc == 0)
				{	/* get next input char */
				if ((ps->state & INVERT) == 0)
					ps->wc = *ps->sin == '\0' ? '\0' : *ps->sin++;
				else
					ps->wc = ps->sin <= ps->sbegin ? '\0' : *--ps->sin;
				if (ps->wc != '\0')
					ps->wc |= ps->phase << 8;
				else if ((pcostate->_Tab[ps->phase][0] & REPEAT) == 0)
					{	/* done, return null */
					sout[nout++] = '\0';
					break;
					}
				else
					{	/* go to next phase */
					ignores = 0;
					sout[nout++] = '\1';
					ps->state = pcostate->_Tab[++ps->phase][0] & ~0xfff;
					if ((ps->state & INVERT) == 0)
						ps->sin = ps->sbegin;
					else if (ps->send != 0)
						ps->sin = ps->send;
					else
						{	/* backwards, memorize end pointer */
						ps->sin += strlen((const char *)ps->sin);
						ps->send = ps->sin;
						}
					}
				}
			else if ((code = pcostate->_Tab[ps->wc >> 8][ps->wc & 0xff])
				& FORWARD)
				ps->wc = code & 0xfff;
			else if (code & OUTPUT || code == 0)
				{	/* put a new weight */
				ps->weight = code & 0xfff;
/* provide for bigger weights */
				if (code & REPEAT)
					++ps->wc;
				else
					ps->wc = 0;
				}
			else if (code & INPUT)
				{	/* consume input and skip if no match */
				int match;

				if ((ps->state & INVERT) == 0)
					for (match = 1; code & INPUT; ++ps->wc,
						code = pcostate->_Tab[ps->wc >> 8][ps->wc & 0xff])
						{	/* match a sequence of chars */
						if (match == 0)
							;
						else if (ps->sin[match - 1] == '\0')
							match = 0;
						else if (ps->sin[match - 1] == (code & 0xff))
							++match;
						else
							match = 0;
						}
				else
					for (match = -1; code & INPUT; ++ps->wc,
						code = pcostate->_Tab[ps->wc >> 8][ps->wc & 0xff])
						{	/* match a sequence of chars */
						if (match == 0)
							;
						else if (&ps->sin[match + 1] <= ps->sbegin)
							match = 0;
						else if (ps->sin[match + 1] == (code & 0xff))
							--match;
						else
							match = 0;
/* drop repeats in localedef? */
						}
				if (match == 0)
					++ps->wc;
				else if ((ps->state & INVERT) == 0)
					ps->sin += match - 1;
				else
					ps->sin += match + 1;
				}
			else if ((ps->state & OFFSETS) && ++ignores == 0xff)
				{	/* put ignores and reset count */
				sout[nout++] = (char)ignores;
				ignores = 0;
				}
			}
		return (nout);
		}
	else
		{	/* run finite state machine */
		int limit = 0;

		for (; nout < size; )
			{	/* perform a state transformation */
			unsigned short code;
			const unsigned short *stab;

			if (_NSTATE <= ps->state
				|| (stab = pcostate->_Tab[ps->state]) == 0
				|| (_NSTATE*UCHAR_MAX) <= ++limit
				|| (code = stab[*ps->sin]) == 0)
				{	/* error return */
				sout[nout++] = '\0';
				ps->state = _NSTATE;
				return (nout);
				}
			ps->state = (code & _ST_STATE) >> _ST_STOFF;
			if (code & _ST_FOLD)
				ps->wc = ps->wc & ~UCHAR_MAX | code & _ST_CH;
			if (code & _ST_ROTATE)
				ps->wc = ps->wc >> CHAR_BIT & UCHAR_MAX | ps->wc << CHAR_BIT;
			if (code & _ST_INPUT)
				{	/* reset limit, increment cyclically */
				limit = 0;
				if (*ps->sin != '\0')
					++ps->sin;
				else
					ps->sin = ps->sbegin;
				}
			if (code & _ST_OUTPUT
				&& (sout[nout++] = ((code & _ST_CH) ? code : ps->wc)) == '\0')
				return (nout);
			}
		return (nout);
		}
	}
_STD_END

/*
 * Copyright (c) 1994-2007 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
V5.04:1296 */

__SRCVERSION("xstrxfrm.c $Rev: 159923 $");
