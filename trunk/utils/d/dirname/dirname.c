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





#ifdef __USAGE		/* dirname.c */
%C	string
 where:
  string  is a string representing a valid pathname.
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.12  2005/06/03 01:37:44  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.11  2003/12/08 16:56:18  kewarken
	added win32 variant
	
	Revision 1.10  2003/08/21 21:25:19  martin
	Update QSSL Copyright.
	
	Revision 1.9  1998/09/15 18:21:47  eric
	cvs
	
	Revision 1.8  1997/02/12 16:32:34  eric
	eliminated nto ifdef

	Revision 1.7  1996/11/12 19:57:13  eric
	removed nto reference to stdutil.h

	Revision 1.6  1996/11/12 19:00:19  steve
	changed to include stdutil.h?

 * Revision 1.5  1992/07/06  16:18:32  garry
 * Edit Usage.
 *
 * Revision 1.4  1991/07/23  18:13:58  eric
 * fixed bug where dirname "" resulted in "" instead of "."
 *
 * Revision 1.3  1991/07/23  18:05:43  eric
 * no change
 *
 * Revision 1.2  1991/07/23  18:03:54  eric
 * rewritten to comply with 1003.2 draft 11
 *

	$Author: coreos $
	
---------------------------------------------------------------------*/
/*
 * Filename:	dirname.c
 *
 * Description:
 *			The dirname utility returns writes a portion of the string operand
 *			to standard output.  The string operand represents a valid pathname
 *			whose format is <directory_pathname>/<base_filename>, the the
 *			syntax of pathname and filename as given in General Terms 2.3
 *			(Posix 1003.1).  If string does not contain a slash character, the
 *			equivalent pathname ./string shall be used.  The dirname utility
 *			writes the <directory_pathname> component to standard output.  The
 *			behaviour of dirname is unspecified if string is not a valid
 *			pathname.
 *
 * Note:	See POSIX 1003.2 Draft 9 Section 4.19
 *
 * Operands:
 *			string						A string representing a valid pathname
 *
 * Exit Status:
 *			0		Successful Completion
 *			>0		An Error Occurred
 *
 * Include declarations:
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <cdefs.h>
#ifndef __CYGWIN__
#include <util/defns.h>
#include <util/stdutil.h>
#else
#endif

/*
 * Define declarations:
 */
#define DIRNAME_OK		0
#define DIRNAME_ERROR	1

/*
 *- ---------------------------------------------------------------------------
 */

int main(int argc, char *argv[] )
{
int 	i;
int 	inval_usage=0;
char	*src_str, *dst_str=NULL;
	/* dirname will actually leave src_str pointing to 'the answer'. dst_str
     * in a pointer used for temporary fiddling. This is the reverse of what
     * basename does (which is to use dst_str for the result). There is much
     * common code in the initial steps taken by both dirname and basename
     */

	while(( i=getopt( argc, argv, "")) != -1)
		if ( i== '?')
			inval_usage = 1;

	/*
	 * Set pointers to our source string and if we have one the suffix string
	 */
	src_str = argv[ optind ];

	if ((argc-optind)<1) fprintf(stderr,"dirname: must supply a string\n"); 
	else if ((argc-optind)>1) fprintf(stderr,"dirname: too many operands\n");

	if ( inval_usage || ((argc-optind)!=1)) exit(EXIT_FAILURE);

	if (!*src_str) src_str = "X";	/* this will result in . after processing */
		
	/* step 1 - if string is //, skip steps (2) through (5)
     */

	/* I don't really need this goto since in this implementation steps 7
     * and 8 are skipped for a file of //. However, in the interests of
     * having things not break if this is changed, the goto is here anyway.
     */

	if (!strcmp(src_str,"//")) goto step6;
	else {
		/* step 2 - if string consists entirely of slash characters, string
         *          shall be set to a single slash character. In this case,
         *          skip steps 3 through 8.
         */
		for (i=0,dst_str=src_str;*dst_str;dst_str++)
			if (*dst_str!='/') i++; /* i non-zero if any chars other than / encountered */

		if (!i) src_str=--dst_str;	/* all /s, back off 1 so dst_str = "/" */
		else {
			/* step 3 - if there are any trailing slash characters in string,
             *          they shall be removed.
             */
			for (;*--dst_str=='/';*dst_str=0);

			/* step 4 - if there are no slash characters remaining in string,
             *          string shall be set to a single period character. In
             *          this case, skip steps 5 through 8. 
             */
			dst_str = strrchr(src_str,'/');
			if (dst_str==NULL) src_str = ".";
			else {
				/* step 5 - if there are any trailing nonslash characters in
                 *          string, they shall be removed.
                 */
				if (*++dst_str!='/') *dst_str=0;
				
				/* step 6 - if the remaining string is //, it is implementation
                 *          defined whether steps 7 and 8 are skipped or
                 *          processed.
                 *
                 * We will skip 7 and 8 in this case.
                 */
step6:			/* entry point for special case of //. In this implementation
                 * these next two steps are skipped anyway (if conditional below)
                 */
                if (strcmp(src_str,"//")) {
					/* step 7 - if there are any trailing slash characters in
                     *          string, they shall be removed. 
                     */
					for (;dst_str!=src_str && *--dst_str=='/';*dst_str=0);

					/* step 8 - if the remaining string is empty, string shall
                     * be set to a single slash character 					
                     */    
					if (!*src_str) src_str = "/";
				}
			}
		}
	}

	fprintf( stdout, "%s\n", src_str);
	return DIRNAME_OK;
}

