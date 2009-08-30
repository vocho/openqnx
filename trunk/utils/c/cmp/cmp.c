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




#ifdef __USAGE
%C	- compare two files (POSIX)

%C	[-l | -s] file1 file2
Options:
 -l     Long format output - show all byte differences, not just the 1st one.
 -s     Silent operation. Return exit status only.
#endif

/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.12  2005/06/03 01:37:43  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.11  2003/09/29 22:24:25  jgarvey
	Make "cmp" utility large-file-aware.  Change counts from int/long to
	off_t (so will resize according to _FILE_OFFSET_BITS compilation) and
	also allow for this in output formatting (printf).
	
	Revision 1.10  2003/08/21 20:20:42  martin
	Add QSSL Copyright.
	
	Revision 1.9  1998/09/15 18:14:46  eric
	cvs
	
	Revision 1.8  1996/08/22 18:44:49  eric
	removed inclusion of <stdutil.h> which was superfluous, and
	removed the need_usage() code.

	Revision 1.7  1995/09/26 23:50:43  steve
	fixed to include files

 * Revision 1.6  1993/11/25  20:10:39  brianc
 * If one file is subset of other and no other differences exit with 1
 *
 * Revision 1.5  1993/05/03  15:06:49  brianc
 * Non-nerrors messages go to stdout
 *
 * Revision 1.4  1993/04/30  21:11:24  brianc
 * When told to be silent don't print anything!
 *
 * Revision 1.3  1992/08/04  20:25:53  eric
 * added usage msg one-liner
 *
 * Revision 1.2  1991/10/23  19:46:17  eric
 * now uses Steve's <fstio.h>
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *
	
---------------------------------------------------------------------*/

/*
-----------------------------------------------------------------------
POSIX UTILITY: CMP

1003.2 -- Draft 9 (Shell and Utilities working group)

Compare two files.

Possible bug	- POSIX didn't say which file should be used to maintain
				  the line count, so if either file comes across a '\n'
				  line count is incremented
-----------------------------------------------------------------------
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <util/defns.h>

/*	Exit status codes	*/
#define FILES_ARE_IDENTICAL_RC	0
#define FILES_DIFFER_RC			1
#define IO_ERROR_RC				2
#define INVALID_USAGE_RC		3

/*	POSIX literal string definitions	*/
#if _FILE_OFFSET_BITS - 0 == 64
#define S_NORMAL_FORMAT				"%s %s differ: char %lld, line %lld\n"
#define S_DASH_L_FORMAT				"%lld %o %o\n"
#else
#define S_NORMAL_FORMAT				"%s %s differ: char %d, line %d\n"
#define S_DASH_L_FORMAT				"%d %o %o\n"
#endif
#define S_EOF_FORMAT				"cmp: EOF on %s\n"

/* Language independent messages	*/
#define TXT(s)						s
#define T_CANT_OPEN_FILE			"cmp: Can't open file '%s' for read\n"
#define T_MISSING_FILENAME_PARMS	"cmp: Missing filename parameter(s)\n"
#define T_TOO_MANY_FILENAMES		"cmp: Too many filename parameters\n"

#define STATIC_BUFSIZE		255
/*	Global variables	*/
int		nerrors;				/* nerrors count during cmd line parsing	*/
int		silent, show_long;	/* flags for cmd line options			*/

off_t	char_count;			/* Number of characters compared so far	*/
off_t	line_count;			/* Number of lines compared so far		*/

extern int optind;			/* needed for DOS; also should be in unistd.h */

char	*file1,	*file2;		/* file names being compared			*/
char	c1, c2;				/* character from each file				*/
int		differ;				/* set if files differ and -l option	*/
int		fd1, fd2;
int		fd1_nread;
int		fd2_nread;
char 	file1buf[STATIC_BUFSIZE];
char	file2buf[STATIC_BUFSIZE];
/*----------------------------------------------------------- main ---------

	See POSIX 1003.2 draft 9 for functional description of utility.

	cmp [-l | -s] file1 file2

	-l = long - print byte # (decimal) and differing bytes (octal) for each
		 difference

	-s = 'silent' operation - don't print anything out, just give ret code

--------------------------------------------------------------------------*/
int main( int argc, char *argv[] )
	{
	int		opt;
	int i;

	nerrors = 0;

	while( ( opt = getopt( argc, argv, "sl" ) ) != -1 ) {
		switch( opt ) {
			case 's':	silent		=	TRUE;		break;
			case 'l':	show_long	=	TRUE;		break;
			default:	nerrors++;	 				break;
			}	/* switch */
		}	/* while */


	if ( optind > (argc-2) ) {
		if (!silent)
			fprintf( stderr, TXT( T_MISSING_FILENAME_PARMS ) );
			exit(2);
		}	/* if */

	if ( nerrors ) exit(EXIT_FAILURE);

	if ( silent == TRUE  &&  show_long == TRUE )
		show_long = FALSE;

	file1 = argv[optind++];
	file2 = argv[optind++];

	if ( optind < argc ) {
		if (!silent)
			fprintf( stderr, TXT( T_TOO_MANY_FILENAMES ) );
		exit(EXIT_FAILURE);
	}

	if ( strcmp( file1, "-" ) == 0 )
		fd1 = STDIN_FILENO;
	else {
		if (( fd1 = open( file1, O_RDONLY )) == -1) {
			if (!silent)
				fprintf( stderr, TXT( T_CANT_OPEN_FILE ), file1 );
			exit( IO_ERROR_RC );
			}
		}

	if ( strcmp( file2, "-" ) == 0 )
		fd2 = STDIN_FILENO;
	else {
		if (( fd2 = open( file2, O_RDONLY)) == -1) {
			if (!silent)
				fprintf( stderr, TXT( T_CANT_OPEN_FILE ), file2 );
			exit( IO_ERROR_RC );
			}
		}
	if ( strcmp( file1, file2 ) == 0 ) {
		close(fd1);
		close(fd2);
		exit( FILES_ARE_IDENTICAL_RC );
		}	/* if */


	char_count = 1;
	line_count = 1;

	for( ;; ) {
		fd1_nread = read(fd1, file1buf, sizeof(file1buf));
		fd2_nread = read(fd2, file2buf, sizeof(file2buf));

		if((fd1_nread <= 0) || (fd2_nread <= 0)) {
			if((fd1_nread == 0) != (fd2_nread == 0)) {
				if ( silent == FALSE )
					fprintf( stderr, S_EOF_FORMAT, fd1_nread==0 ? file1 : file2 );
				exit( differ ? IO_ERROR_RC : FILES_DIFFER_RC );
			}	/* if */
		}

		if(memcmp(file1buf,file2buf,min(fd1_nread,fd2_nread))) {
			char *f1,*f2;
			for(f1=file1buf,f2=file2buf ; *f1 == *f2 ; f1++,f2++,char_count++) {
				if(*f1 == '\n' || *f2 == '\n') line_count++;
			}
			
			if ( show_long == TRUE ) {
				if ( silent == FALSE )
					fprintf( stdout, S_DASH_L_FORMAT, char_count, c1, c2 );
				differ = TRUE;
			} else {
				if ( silent == FALSE )
					fprintf( stdout, S_NORMAL_FORMAT, file1, file2, char_count, line_count );
				exit( FILES_DIFFER_RC );
			}
		}

		if((fd1_nread == 0) && (fd2_nread==0)) {
			break;
		}
		
		char_count+=min(fd1_nread, fd2_nread);
		for(i=0 ; i < min(fd1_nread,fd2_nread) ; i++) {
			if((file1buf[i] == '\n') || (file2buf[i] == '\n'))
				line_count++;
		}
	} /* for */
		
	return ( differ ? FILES_DIFFER_RC : FILES_ARE_IDENTICAL_RC );
}	/* main() */
