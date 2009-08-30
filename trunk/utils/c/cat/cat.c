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



#ifdef __USAGE		/* cat.c */
%C	- concatenate and print files (POSIX)

%C	[-cu] [-n|r] [file...]
Options:
 -c      Compress, do not display empty lines.
 -u      Unbuffered, for interactive use.
 -n	 Print line numbers without restarting.
 -r	 Print line numbers restarting count for each file.
 -w      End of line wait of 100ms for each -w given.
#endif


/*---------------------------------------------------------------------

	$Header$

	$Source$

	$Log$
	Revision 1.13  2005/06/03 01:37:42  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.12  2003/09/24 00:17:48  jgarvey
	Make "cat" utility large-file-aware.  Change line count from int to
	off_t (so will resize according to _FILE_OFFSET_BITS compilation) and
	also allow for this in output formatting (printf).
	
	Revision 1.11  2003/08/21 19:50:02  martin
	Update QSSL Copyright.
	
	Revision 1.10  2001/12/17 20:37:56  dwhelan
	PR4075
	
	Revision 1.9  2001/07/31 14:25:03  jbaker
	now builds under cygwin
	
	Revision 1.8  1999/08/09 22:00:30  adrianj
	Clarified usage.
	
	Revision 1.7  1999/08/09 14:26:49  spuri
	- I added the '-n' and '-r' options so that cat prints out line
	  numbers. The '-r' option resets the line counter for each new
	  file. This code works w/ both compressed and unbuffered input.
	
	Revision 1.6  1998/09/26 16:15:12  bstecher
	all uses of <util/cpdeps.h> now should use <lib/compat.h>
	
	Revision 1.5  1998/05/01 16:03:49  eric
	reorg for gcc

	Revision 1.4  1997/07/29 19:32:32  bstecher
	ported to Win32

	Revision 1.3  1996/01/26 19:35:00  dtdodge
	Removed stupid need_usage()

	Revision 1.2  1992/07/30 19:17:41  eric
	added usage message one-liner

 * Revision 1.1  1991/08/27  17:11:38  brianc
 * Initial revision
 *

	$Author: rmansfield $
	
---------------------------------------------------------------------*/
/*
 * Description:
 *			The cat utility reads files sequentially, writing them to the
 *			standard output.  The filename operands are processed in   
 *			command line order and are read from beginning to end.  If no
 *			filenames are given, the cat utility reads from the standard input.
 *
 * Note:	See POSIX 1003.2 Draft 9 Section 4.4
 *
 * Args:
 *			-u		Output is not buffered.
 *			-c		Compress, ignore empty lines
 *			-n	 	Print line numbers
 *			-r	 	Restart line numbers for each new file
 *			filename...	Name of input file(s)
 *
 * Include declarations:
 */
#ifndef __CYGWIN__
	#include <lib/compat.h>
#else
	extern int optind;
	#define MAKE_BINARY_FD(fd)  setmode(fd, O_BINARY)
#endif

#if defined (__MINGW32__)
#include <windows.h>
#endif

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>

static	char	iobuf[4096];
off_t lineCount = 0;	// current line number, only incremented if
				// the user wants to print out line numbers
int resetLineNums = 0;		// true iff the user wants to reset line numbers
				// for each file
int compressLines;
int eol_wait = 0; // pause at the end of each line

int
foldfile(int ifd, int ofd,int iosize, int foldchr)
{
int	nbytes;
int	head, tail;
int	state = 0;

	while ((nbytes = read(ifd, iobuf, iosize)) > 0) {
		head = tail = 0;
		while (head < nbytes) {
			switch (state) {
			case	0:		/*	find first foldchr */
				for (tail=head; tail < nbytes && iobuf[tail] != foldchr; tail++)
					;
				if (tail != nbytes) {
					tail++;   			/* include the foldchr */
					state = 1;
				}
				{ int t=0,m;
					m = tail - head;
					while (m!=0 && (t=write(ofd, iobuf + head, m)) > 0) {
						head += t;
						m -= t;
					}
					if (t <= 0 && m) 
						return 2;
				}
				head = tail;			/* update head ptr */
				break;
			case	1:			/* find the char after the last foldchr */
				while (head < nbytes && iobuf[head] == foldchr) 
					head++;
				state = head == nbytes;	/* if the buffer ran out, stay in state 1, otherwise state 0 */
				break;
			}
		}
	}
	return nbytes < 0 ;
}

int
catfile(int ifd, int ofd, int iosize)
{
	int	nbytes, lbytes; // n = total bytes, l = bytes per line
	char	*obufp;
	char	strLineCount[20];
	int	newline = 1;
	char	*pnewline = 0;

	#if !defined(__MINGW32__)
	struct timespec sleeptime = {0, eol_wait*100*1000*1000 };
	#endif

	// loop forever
	while (1)
	{
		int	t = 0;		// how many bytes to write out

		// read as much as we can into the buffer
		obufp = iobuf;
		nbytes = read(ifd, obufp, iosize);
		if (nbytes <= 0)
			break;

		// process each line in the input we just read
		while (nbytes)
		{
			if (lineCount || compressLines)
			{
				// look for the end of this line
				pnewline = obufp;
				while (pnewline - obufp < nbytes)
				{
					if (*pnewline == '\n')
						break;
					pnewline++;
				}
				lbytes = pnewline - obufp;
				// if we have an entire line in our buffer, make sure
				// we print out the '\n' as well
				if (lbytes < nbytes)
					lbytes++;			
			}
			else
			{
				// don't bother wasting time processing lines if the
				// user doesn't want line numbers or compressed lines
				lbytes = nbytes;
			}

			nbytes -= lbytes;
			// determine if the current line is empty
			if (compressLines && (*obufp == '\n') && newline)
			{
				obufp++;
				continue;
			}

			// print out the line number at the beginning of a new line
			// check lbytes to make sure we don't print out a line number
			// after we printed out the last line
			if (lineCount && newline && lbytes)
			{
#if _FILE_OFFSET_BITS - 0 == 64
				sprintf (strLineCount, "%6lld: ", lineCount);
#else
				sprintf (strLineCount, "%6d: ", lineCount);
#endif
				write (ofd, strLineCount, strlen (strLineCount));
				lineCount++;
				newline = 0;
			}

			// write out the current line
			while (lbytes)
			{
				t = write(ofd, obufp, lbytes);
				if (t <= 0)
					break;
				lbytes -= t;
				obufp += t;
			}

			// determine if we just wrote out an entire line by checking
			// whether pnewline - obufp < nbytes
			if (lineCount || compressLines)
				newline = (pnewline - obufp < nbytes);
		}

		if (t <= 0 && nbytes) 
		{
			return 2;
		}

    		if (eol_wait) {
			# if !defined(__MINGW32__)
      			nanosleep(&sleeptime, NULL);
			# else
			Sleep(eol_wait*100);
			# endif
    		}
	}
	// reset the line number count for each file if the user wants us to
	if (resetLineNums)
		lineCount = 1;
	return nbytes < 0;  // true only on read error
}


int
main( int argc, char *argv[] )
{
	int 	i;
	int 	iosize = sizeof(iobuf);
	int	nerrs = 0;
	int	ifd;	

	while(( i= getopt( argc, argv, "u**cnrw")) != EOF) {
		switch ( i )
		{
		case 'u':
			iosize = 1;
			break;
		case 'c':
			compressLines = 1;
			break;
		case 'n':
			lineCount = 1;
			resetLineNums = 0;
			break;
		case 'r':
			lineCount = 1;
			resetLineNums = 1;
			break;
    		case 'w':
      			eol_wait++;
      			iosize = 80;
      			break;
		default:
			exit( EXIT_FAILURE );
		}
	}
#define	OUT_FD	1		
	MAKE_BINARY_FD( OUT_FD );
	if (optind == argc) {
		MAKE_BINARY_FD( 0 );
//		switch (compress_line ? foldfile(0, OUT_FD, iosize, '\n')
//								: catfile(0, OUT_FD, iosize)) 
		switch (catfile(0, OUT_FD, iosize))
		{
		case	1:	perror("--stdin--");	nerrs++;	break;
		case	2:	perror("--stdout--");	nerrs++;	break;
		}
	} else {
		for (; optind < argc; optind++) {
			if (strcmp(argv[ optind ], "-")) {
				if ((ifd = open(argv[optind], O_RDONLY|O_BINARY)) == -1) {
					perror(argv[optind]);
					nerrs++;
					continue;
				}
			} else {
				ifd = 0;
				MAKE_BINARY_FD( ifd );
			}
//			switch(compress_line ? foldfile(ifd, OUT_FD, iosize, '\n')
//								: catfile(ifd, OUT_FD, iosize)) 
			switch (catfile(ifd, OUT_FD, iosize))
			{
			case	1:
				perror(ifd == 0 ? "--stdin--" : argv[optind]);
				nerrs++;
				break;
			case	2:
				perror("--stdout--");
				nerrs++;
				break;
			}
			if (ifd)
				close(ifd);
		}                 
    }
	return nerrs ? EXIT_FAILURE : EXIT_SUCCESS;
}


