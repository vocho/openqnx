/* xstate.h internal header */
#ifndef _XSTATE
#define _XSTATE
#ifndef _YVALS
 #include <yvals.h>
#endif /* _YVALS */

_C_STD_BEGIN

		/* macros for finite state machines */
#define _ST_CH		0x00ff
#define _ST_STATE	0x0f00
#define _ST_STOFF	8
#define _ST_FOLD	0x8000
#define _ST_INPUT	0x4000
#define _ST_OUTPUT	0x2000
#define _ST_ROTATE	0x1000
#define _NSTATE		16

		/* type definitions */
typedef struct
	{	/* pointer to state-machine table */
	const unsigned short *_Tab[_NSTATE];
	} _Statab;
_C_STD_END
#endif /* _XSTATE */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("xstate.h $Rev: 153052 $"); */
