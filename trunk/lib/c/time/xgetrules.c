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



/*

 


Also copyright P.J. Plauger - see bottom of file for details.
*/

/* _Isdst function */
#include <stdlib.h>
#include "xtime.h"
_STD_BEGIN

Dstrule *_Getrules(int *dst_state)
	{	/* load Daylight Savings Time rules
			if they exist. Set *dst_state (if not NULL) as follows
			0  - DST not in effect (DST field does not exist)
			-1 - DST field exists but could not be decoded
			1	- DST is in effect
		*/
	Dstrule *pr;
#ifdef __QNX__
	static const char *olddst;
	static Dstrule *rules;
#else
	static const char *olddst = 0;
	static Dstrule *rules = NULL;
#endif
    unsigned  DST_field_present = 0;	// assume no DST info

	if (olddst == _Times._Isdst)
		{
			if (dst_state)
				*dst_state = (rules == NULL) ? 0 : 1;
			return rules;
		}
	else
		{	/* find current dst_rules */
		if (_Times._Isdst[0] != '\0')
			DST_field_present = 1;
		else
			{	/* look beyond time_zone info */
			int n;

			if (_Times._Tzone[0] == '\0')
				_Times._Tzone = _Getzone();
#ifdef __QNX__
			_Times._Isdst = _Gettime(_Times._Tzone, 4, &n);
#else
			_Times._Isdst = _Gettime(_Times._Tzone, 3, &n);
#endif
			if (_Times._Isdst[0] != '\0')
				{
				DST_field_present = 1;	// DST field present
				--_Times._Isdst;	/* point to delimiter */
				}
			}
			
		if ((pr = _Getdst(_Times._Isdst)) == 0)
			{
			/*
			 * PR27178
			 * as per POSIX, if DST field is not present, assume no DST (ie. not in effect). This
			 * will result in *dst_state == 0, however if the field is present but not parsable,
			 * *dst_state = -1 to indicate that the information was not available (even though
			 * it was supposed to be)
			*/
			if (dst_state)
				*dst_state = DST_field_present ? -1 : 0;
			return (NULL);
			}
		else
			{
			/* DST is in effect, free old rules and set new */
			free(rules);
			rules = pr;
			olddst = _Times._Isdst;
			if (dst_state)
				*dst_state = 1;
			
			return rules;
			}
		}
	}
_STD_END

/*
 * Copyright (c) 1992-2003 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V4.02:1296 */

__SRCVERSION("xgetrules.c $Rev: 153052 $");
