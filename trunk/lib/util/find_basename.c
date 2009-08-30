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
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <util/defns.h>
#include <limits.h>
#ifdef __MINGW32__
#include <lib/compat.h>
#endif

/*
 * Define declarations:
 */
#define BASENAME_ERROR	01

/*
 *- ---------------------------------------------------------------------------
 */

char * find_basename (char *s)
{
	int i;
	static char src_str[_POSIX_PATH_MAX];
	char *dst_str;
	char *sfx_str = NULL;		/* initialization req'd, NULL = not specified */

	strncpy(src_str,s,sizeof(src_str));
	
	/* step 1 - if string is //, it is implementation defined whether steps
     *          2 through 5 are skipped or processed. 
     *
     * We shall process.
	 */
	#ifdef DEBUG
	printf("step 1 : src_str = %s\n",src_str);
	#endif

	/* step 2 - if string consists entirely of slash characters, string shall
     *          be set to a single / character. In this case, skip steps
     *          3 through 5
     */

	for(i=0,dst_str=src_str;*dst_str;dst_str++)
		if (*dst_str!='/') i++;		/* i non-zero if any chars other than / encountered */
	if (!i) dst_str--;              /* all /s - back off 1 so dst_str = "/" */
	else {
		#ifdef DEBUG
		printf("step 2 : src_str = %s, dst_str = %s\n",src_str,dst_str);
		#endif
		/* step 3 - if there are any trailing slash characters in string,
         *          they shall be removed.
         */
		for (;*--dst_str=='/';*dst_str = 0);
		#ifdef DEBUG
		printf("step 3 : src_str = %s, dst_str = %s\n",src_str,dst_str);
		#endif

		/* step 4 - if there are any slash characters remaining in string,
         *          the prefix of string up to and including the last slash
         *          character in string shall be removed.
		 */
	    dst_str = strrchr( src_str, '/');
		if (dst_str==NULL) dst_str = src_str;
		else dst_str++;

		#ifdef DEBUG
		printf("step 4 : src_str = %s, dst_str = %s\n",src_str,dst_str);
		#endif
		/* step 5 - if the suffix operand is present, is not identical to 
         *          the characters remaining in string, and is identical to a
         *          suffic of the characters remaining in the string, the suffix
         *          shall be removed from string. Otherwise, string shall
         *          not be modified by this step. It shall not be considered
         *          an error if suffix is not found in string.
		 */

		if (sfx_str!=NULL) {
			char *end_str;

			end_str = dst_str+strlen(dst_str)-strlen(sfx_str);
			if ((end_str>dst_str)&&!strcmp(end_str,sfx_str)) *end_str = 0;
		}
		#ifdef DEBUG
		printf("step 5 : src_str = %s, dst_str = %s\n",src_str,dst_str);
		#endif
	}

	return dst_str;
}

