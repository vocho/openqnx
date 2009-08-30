/*
 * $QNXLicenseC:
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




#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <util/util_limits.h>

/*
This routine takes a pointer to a string containing a shell-style
pattern (bracket expressions, *, ?) and converts it to a Basic 
Regular Expression which can be subsequently regcomp()ed.


NOTE that this will *not* work to replace fnmatch() if it is
to be called with the FNM_PATHNAME flag.
*/

char *fnmatch2regex(const char *pattern)
{
	/* pattern will be 

       [ bracket_expr ]

         This will be literally retained; however, any occurrance of
         '!' will be replaced by '^^'

       ?

         This will be replaced by a single period (.) 

       *
  
         This will be replaced by a single period (.) followed by a splat (*)

	*/

	const char *r;
	char *w, *buffer;

	/* the absolute worst case would be for a result which was double
       the original string + 2 chars for preceding ^ and trailing $ */
	if (NULL==(buffer=malloc(3+2*strlen(pattern)))) return NULL;
	
	r=pattern;
	w=buffer;

	*w++='^';

	for (;*r;r++) {		
		switch(*r) {

			/* FIRST SECTION - FNMATCH SPECIAL CHARACTERS */

			case '[':	/* bracket expression */
				*w++=*r++;
				while(*r && *r!=']') {
					if (*r=='!') {
						*w++='^';
						*w++='^';
						r++;
					} else {
						*w++=*r++;
					}
				}

				if (*r) *w++=*r;
				else r--;             /* point r back before NUL so that        
		                               the for() loop will exit after r++ */
				                      
					
				break;

			case '?':   /* match single character */
				*w++='.';
				break;

			case '*':   /* match zero or more characters */
				*w++='.';
				*w++='*';
				break;

			/* NEXT SECTION - REGEX SPECIAL CHARS WHICH NEED TO BE ESCAPED */

			case '.':
			case '$':
			case '^':
			case ']':
			case ':':
			case '{':
			case '}':
			case '\\':
				*w++='\\';	/* escape; fall thru now to default */

			/* DEFAULT - NON-SPECIAL CHARS TO BE LITERALLY MATCHED */
			default:
				*w++=*r;
				break;

		}	/* switch */
	}	/* for */

	w[0]='$';
	w[1]=0;

#ifdef DIAG
	fprintf(stdout,"fnmatch2regex: returning '%s' (",buffer);
	{
		int i;
		for (i=0;buffer[i];i++) {
			fprintf(stdout,"'%c' (0x%02x)",buffer[i],buffer[i]);
		}
	}
	fprintf(stdout,"\n");
#endif
	return buffer;
}


