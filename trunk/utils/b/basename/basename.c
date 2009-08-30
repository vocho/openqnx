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



#ifdef __USAGE		/* basename.c */
%C	- return nondirectory portion of pathname (POSIX)

%C	string [suffix]
Note:
 This will print to stdout the basename portion of string, optionally with
 the suffix removed from the end of it (if the suffix was present).
#endif


/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.11  2005/06/03 01:37:41  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.10  2003/08/21 16:46:36  martin
	
	Update QSSL Copyright Notice.
	
	Revision 1.9  1998/09/15 20:14:50  eric
	cvs
	
	Revision 1.8  1997/02/07 21:10:20  eric
	changed reference to stdutil.h to specify <util/stdutil.h> to
	eliminate the need for the include directory assumption in
	cmd.mk

	Revision 1.7  1996/11/12 19:56:04  eric
	removed reference to stdutil.h

	Revision 1.6  1996/10/23 17:29:12  eric
	removed need_usage() (for nto)

	Revision 1.5  1995/09/26 23:46:58  steve
	new include directives.

 * Revision 1.4  1992/07/30  19:10:27  eric
 * added one-liner to usage message
 *
 * Revision 1.3  1991/07/23  18:05:43  eric
 * no change
 *
 * Revision 1.2  1991/07/23  18:03:34  eric
 * rewritten to comply with 1003.2 draft 11
 *
 * Revision 1.1  1991/07/22  15:53:32  eric
 * Initial revision
 *

	$Author: coreos $
	
---------------------------------------------------------------------*/
/*
 *
 * Description:
 *			The basename utility deletes the longest prefix ending in /, and
 *			if specified, the suffix speficied in suffix from the string
 *			argument.  It then writes the result to the standard output.
 *			If the string argument matehes eighet the prefix or the suffix
 *			argument, the null string shall be written to the standard output.
 *
 * Note:	See POSIX 1003.2 Draft 8 Section 4.3
 *
 * Args:
 *			string						A string
 *			suffix						A string
 *
 * Include declarations:
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <util/defns.h>
#include <util/stdutil.h>

/*
 * Define declarations:
 */
#define BASENAME_ERROR	01

/*
 *- ---------------------------------------------------------------------------
 */

int main( int argc, char *argv[] )
{
	int i;
	int inval_usage = FALSE;
	char *src_str = NULL;       /* will always be set from cmd line args, setting
                                   to NULL here satisfies GNU compiler that src_str
                                   will not be uninitialized */
    char *dst_str;
	char *sfx_str = NULL;		/* initialization req'd, NULL = not specified */
	
	while(( i= getopt( argc, argv, "")) != -1)
		if ( i== '?')
			inval_usage = TRUE;
	/*
	 * Set pointers to our source string and if we have one the suffix string
	 */
	switch(argc-optind) {
		case 2:		sfx_str = argv[optind+1];	/* fall thru */
		case 1:		src_str = argv[optind];	
					break;
		default:	fprintf(stderr,"basename: invalid number of operands\n");
					inval_usage++;
					break;
	}

	if ( inval_usage ) exit(EXIT_FAILURE);

	/* step 1 - if string is //, it is implementation defined whether steps
     *          2 through 5 are skipped or processed. 
     *
     * We shall process.
	 */
	#ifdef DEBUG
	printf("step 1 : src_str = %s, dst_str = %s\n",src_str,dst_str);
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

	fprintf( stdout, "%s\n", dst_str);
	exit( EXIT_SUCCESS );
}

