#ifdef __USAGE		/* comm.c */
%C	- select or reject lines common to two files (POSIX)

%C	[-123] file1 file2
Options:
 -1      Suppress writing of lines found only in file1
 -2      Suppress writing of lines found only in file2
 -3      Suppress writing of lines found in both file1 & file2

Description:
file 1 and file 2 must be ordered in collating sequence (see
sort utility). With no options, 3 columns are produced:
 column1 lines found only in file1.
 column2 lines found only in file2.
 column3 lines found both files.

#endif


/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.2  2007/04/30 15:22:19  seanb
	- POSIX says comm should honour LC_COLLATE
	  so use strcoll().
	- Don't compare buffers unless they were
	  were both actually read into.
	PR:19760
	CI:seanb

	Revision 1.1  2007/04/30 13:41:52  seanb
	- Bring over comm from QNX4 repository
	  with minor tweaks to build on QNX6.
	PR: 19760.
	CI: seanb
	
	Revision 1.1  2003/10/27 14:35:53  hsbrown
	Moving utils back to QNX4 CVS.
	
	Revision 1.8  1997/02/10 19:41:09  eric
	removed conditional compilation around historical qnx2-style code

	Revision 1.7  1997/01/29 16:46:00  eric
	fixed problem where comm relied on fclose not clobbering the
	value in errno

	Revision 1.6  1997/01/29 16:24:02  eric
	changed funky -1 stuff to simply check for NULL.

	Revision 1.5  1996/01/28 20:43:04  dtdodge
	*** empty log message ***

	Revision 1.4  1992/08/04 20:27:26  eric
	added usage msg one-liner

 * Revision 1.3  1991/12/09  17:09:27  ajedgar
 * ÿ©ÿ©ÿ©ÿ¡ÿ¡Updated usage and remad
 *
 * Revision 1.2  1991/09/03  18:37:32  ajedgar
 * Buffers now use LINE_MAX. New usage message. Tested.
 *
 * Revision 1.1  1991/09/03  16:25:40  ajedgar
 * Initial revision
 *
	
---------------------------------------------------------------------*/


/*
-----------------------------------------------------------------------
POSIX UTILITY: COMM

1003.2 -- Draft 9 (Shell and Utilities working group)

comm - Select or reject lines common to two files
-----------------------------------------------------------------------
*/

#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

/*
 *  TXT(s) provides support for language independence, once we have it...
 */

#define TXT(s)			  s

#define T_ERR_FILE			"%s: error %d %s file '%s'\n"
#define T_OPEN				"opening"
#define T_READ				"reading"
#define T_WRITE				"writing"

#define E_INVALID_USAGE		1
#define E_ERR				-1
#define E_OK				0

#define C_NULL			'\0'
#define C_QUOTE			'\\'	/*	Command line escape character */
#define C_SLASH			'\\'
#define C_NEWLINE		'\n'
#define C_TAB			'\t'

#ifndef TRUE
#	define TRUE		(1==1)
#	define FALSE	(0==1)
#endif

#define BUFSIZE			LINE_MAX

char *Me;
char Buf1[BUFSIZE];
char Buf2[BUFSIZE];
char Suppress[] = {FALSE,FALSE,FALSE,FALSE};
char *Indent[] = {
	"",
	"\t",
	"\t\t"
/*	{C_TAB, C_TAB, NULL} */
};

/*
 *	Function declarations
 */
FILE *open_file();

/*
 *	comm - Select or reject lines common to two files
 *
 *	Exit status:
 *	   0  - all files processed successfully
 *	   n  - number of files that encountered errors
 */
int
main (int argc, char **argv)
{
	register FILE *fp1, *fp2;
	register int i;
	int opt, t, error = 0;
	int i1 = 1, i2 = 2, res;
	char *fname1, *fname2;
	char read1, read2;
	
	res = 0; /* Silence gcc */
	/*	Parse command line arguments */
	error = 0;
	Me = basename(argv[0]);
	if (*Me == '?') {
		++Me;
		++error;
	}

	while ((opt = getopt(argc,argv,"123")) != -1)
	{
		switch(opt)
		{
			/*	Suppress one of the output columns */
			case '1':
			case '2':
			case '3':
				Suppress[t = (opt-'1')] = TRUE;
				if (t == 0) {	/*	Reduce indents for 'file2' and 'both' */
					--i1;
					--i2;
				}
				if (t == 1) {	/*	Reduce indent for 'both' */
					--i2;
				}
				break;

			/*	User wants a usage message or bad option */
			case '?':
				error++;
				break;
		}
	}

	if ((argc-optind)<1) {
		fprintf(stderr,"%s: filenames must be specified\n",argv[0]);
		exit(1);
	} else if ((argc-optind)<2) {
		fprintf(stderr,"%s: a second filename must be specified\n",argv[0]);
		exit(1);
	}

	/*	Open the files */
	fp1 = open_file(fname1 = argv[optind++]);
	fp2 = open_file(fname2 = argv[optind++]);
	if (! fp1 || ! fp2)
		exit(1);

	/*	Find common lines */
	read1 = read2 = TRUE;
	while(1) {
		int serr=0;

		i = 0;
		/*	Read the first file */
		if (read1 && fp1 != NULL) {
            errno=0;
			if (! fgets(Buf1, BUFSIZE-1, fp1)) {
				serr=errno;

				fclose(fp1);
				fp1 = NULL;
				if (serr) {
					fprintf(stderr, TXT(T_ERR_FILE), Me, serr, TXT(T_READ), fname1);
					++error;
					break;
				}
			}
			read1 = FALSE;
		}
		/*	Read the second file */
		if (read2 && fp2 != NULL) {
            errno=0;
			if (! fgets(Buf2, BUFSIZE-1, fp2)) {
				serr=errno;
				fclose(fp2);
				fp2 = NULL;
				if (serr) {
					fprintf(stderr, TXT(T_ERR_FILE), Me, serr, TXT(T_READ), fname2);
					++error;
					break;
				}
			}
			read2 = FALSE;
		}
		/*	All done ? */
		if (fp1 == NULL && fp2 == NULL)
			break;

		if (fp1 != NULL && fp2 != NULL)
			res = strcoll(Buf1, Buf2);

		if (fp2 == NULL || (fp1 != NULL && res < 0)) {
			/*	String is only in file 1... */
			if (! Suppress[0])
				fputs(Buf1, stdout);
			read1 = TRUE;
			continue;
		}
		if (fp1 == NULL || (fp2 != NULL && res > 0)) {
			/*	String is only in file 2... */
			if (! Suppress[1]) {
				fputs(Indent[i1], stdout);
				fputs(Buf2, stdout);
			}
			read2 = TRUE;
			continue;
		}
		/*	String is in both file 1 and file 2 */
		if (! Suppress[2]) {
			fputs(Indent[i2], stdout);
			fputs(Buf1, stdout);
		}
		read1 = read2 = TRUE;
	}

	exit(error);
}


/*
 *	Open a file
 */
FILE *
open_file (char *fname)
{
	FILE *fp;

	/*	If fname is "-" then use stdin otherwise open the file */
	if (strcmp(fname, "-")) {
		/*	Open the file */
		if (!(fp = fopen(fname, "r"))) {
			/*	If there's an error, display it... */
			fprintf(stderr, TXT(T_ERR_FILE), Me, errno, TXT(T_OPEN), fname);
			return(NULL);
		}
	}
	else {
		fp = stdin;
	}
	return(fp);
}
	
