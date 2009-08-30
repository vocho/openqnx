/* _Freeloc function */
#include "xlocale.h"
_STD_BEGIN

void _Freeloc(_Linfo *p)
	{	/* free all storage */
	const _Locitem *q;

	for (q = _Loctab; q->_Name; ++q)
		switch (q->_Code)
			{	/* free all pointers */
		case L_STATE:
			 {	/* free all state entries */
			int i;
			unsigned short **pt
				= &ADDR(p, q, unsigned short *);

			for (i = _NSTATE; 0 <= --i; ++pt)
				if (*pt && (*pt)[-1] != 0)
					free(&(*pt)[-1]);
			 }
			break;
		case L_TABLE:
			if (NEWADDR(p, q, short *))
				free(ADDR(p, q, short *) - 1);
			break;
		case L_GSTRING:
		case L_NAME:
		case L_STRING:
			if (NEWADDR(p, q, char *))
				free(ADDR(p, q, char *));
			break;
		case L_WCTYPE:
			if (NEWADDR(p, q, _Wctab *))
				{	/* free wctype names and ranges */
				int i;
				_Wctab *po = ADDR(p, q, _Wctab *);
				_Wctab *pc = ADDR(&_Clocale, q, _Wctab *);

				if (po[0]._Name != pc[0]._Name)
					free((void *)po->_Name);	/* free ranges */
				for (i = 0; pc[i]._Name != 0; ++i)
					;	/* skip static names */
				for (; po[i]._Name != 0; ++i)
					free((void *)po[i]._Name);	/* free new names */
				free(po);	/* free property table */
				}
#ifdef __QNX__
			break;
		case L_NOTE:
		case L_SETVAL:
		case L_VALUE:
		case L_WSTRING:
			break;
#endif
			}
			}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("xfreeloc.c $Rev: 153052 $");
