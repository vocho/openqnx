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





/* $Source$
 *
 * $Revision: 153052 $
 *
 * ttyio.c - Terminal/Console I/O functions for all archive interfaces
 *
 * DESCRIPTION
 *
 *	These routines provide a consistent, general purpose interface to
 *	the user via the users terminal, if it is available to the
 *	process.
 *
 * AUTHOR
 *
 *     Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice is duplicated in all such 
 * forms and that any documentation, advertising materials, and other 
 * materials related to such distribution and use acknowledge that the 
 * software was developed * by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Log$
 * Revision 1.5  2005/06/03 01:37:53  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.4  2003/09/24 19:51:53  thomasf
 * Updates to make the build work with the mingw platform.
 *
 * Revision 1.3  2003/08/27 18:16:57  martin
 * Add QSSL Copyright to cover QNX contributions.
 *
 * Revision 1.2  1998/12/03 19:00:48  garry
 * ?
 *
 * Revision 1.1  1996/07/30 18:09:51  garry
 * Initial revision
 *
 * Revision 1.2  89/02/12  10:06:11  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:39  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: ttyio.c 153052 2008-08-13 01:17:50Z coreos $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"


/* open_tty - open the terminal for interactive queries
 *
 * DESCRIPTION
 *
 * 	Assumes that background processes ignore interrupts and that the
 *	open() or the isatty() will fail for processes which are not
 *	attached to terminals. Returns a file descriptor or -1 if
 *	unsuccessful. 
 *
 * RETURNS
 *
 *	Returns a file descriptor which can be used to read and write
 *	directly to the user's terminal, or -1 on failure.  
 *
 * ERRORS
 *
 *	If SIGINT cannot be ignored, or the open fails, or the newly opened
 *	terminal device is not a tty, then open_tty will return a -1 to the
 *	caller.
 */

#if defined(__NT__) || defined(__MINGW32__)
   static int prompterfd;

#else

   #define prompterfd ttyf

#endif

#ifdef __STDC__

int open_tty(void)

#else

int open_tty()

#endif
{
    int             fd;		/* file descriptor for terminal */
    SIG_T         (*intr)();	/* used to restore interupts if signal fails */

    if ((intr = signal(SIGINT, SIG_IGN)) == SIG_IGN) {
	return (-1);
    }
    signal(SIGINT, intr);
#if defined(__NT__) || defined(__MINGW32__)
    /*
       Stupid NT won't let us open "con" for both read and write
       at the same time.
    */
    if ((fd = open(TTY, O_RDONLY)) < 0) {
	return (-1);
    }
    if (!isatty(fd)) {
	close(fd);
	return (-1);
    }
    if ((prompterfd = open(TTY, O_WRONLY)) < 0) {
	close(fd);
	return (-1);
    }
    return (fd);
#else
    if ((fd = open(TTY, O_RDWR)) < 0) {
	return (-1);
    }
    if (isatty(fd)) {
	return (fd);
    }
    close(fd);
    return (-1);
#endif
}


/* nextask - ask a question and get a response
 *
 * DESCRIPTION
 *
 *	Give the user a prompt and wait for their response.  The prompt,
 *	located in "msg" is printed, then the user is allowed to type
 *	a response to the message.  The first "limit" characters of the
 *	user response is stored in "answer".
 *
 *	Nextask ignores spaces and tabs. 
 *
 * PARAMETERS
 *
 *	char *msg	- Message to display for user 
 *	char *answer	- Pointer to user's response to question 
 *	int limit	- Limit of length for user's response
 *
 * RETURNS
 *
 *	Returns the number of characters in the user response to the 
 *	calling function.  If an EOF was encountered, a -1 is returned to
 *	the calling function.  If an error occured which causes the read
 *	to return with a value of -1, then the function will return a
 *	non-zero return status to the calling process, and abort
 *	execution.
 */

#ifdef __STDC__

int nextask(char *msg, char *answer, int limit)

#else

int nextask(msg, answer, limit)
char           *msg;		/* message to display for user */
char           *answer;		/* pointer to user's response to question */
int             limit;		/* limit of length for user's response */

#endif
{
    int             idx;	/* index into answer for character input */
    int             got;	/* number of characters read */
    char            c;		/* character read */

    if (ttyf < 0) {
	fatal( TTY " Unavailable");
    }
    write(prompterfd, msg, (uint) strlen(msg));
    idx = 0;
    while ((got = read(ttyf, &c, 1)) == 1) {
	if (c == '\n') {
	    break;
	} else if (c == ' ' || c == '\t') {
	    continue;
	} else if (idx < limit - 1) {
	    answer[idx++] = c;
	}
    }
    if (got == 0) {		/* got an EOF */
        return(-1);
    }
    if (got < 0) {
	fatal(strerror());
    }
    answer[idx] = '\0';
    return(0);
}


/* lineget - get a line from a given stream
 *
 * DESCRIPTION
 * 
 *	Get a line of input for the stream named by "stream".  The data on
 *	the stream is put into the buffer "buf".
 *
 * PARAMETERS
 *
 *	FILE *stream		- Stream to get input from 
 *	char *buf		- Buffer to put input into
 *
 * RETURNS
 *
 * 	Returns 0 if successful, -1 at EOF or input exceeds PATH_MAX. 
 */

#ifdef __STDC__

int lineget(FILE *stream, char *buf)

#else

int lineget(stream, buf)
FILE           *stream;		/* stream to get input from */
char           *buf;		/* buffer to put input into */

#endif
{
    int             c, i=0;

    for (;;) {
	if (++i > PATH_MAX) {
	    fatal("File name too long");
	    return (-1);
	}
	if ((c = getc(stream)) == EOF) {
	    return (-1);
	}
	if (c == '\n') {
	    break;
	}
	*buf++ = c;
    }
    *buf = '\0';
    return (0);
}


/* next - Advance to the next archive volume. 
 *
 * DESCRIPTION
 *
 *	Prompts the user to replace the backup medium with a new volume
 *	when the old one is full.  There are some cases, such as when
 *	archiving to a file on a hard disk, that the message can be a
 *	little surprising.  Assumes that background processes ignore
 *	interrupts and that the open() or the isatty() will fail for
 *	processes which are not attached to terminals. Returns a file
 *	descriptor or -1 if unsuccessful. 
 *
 * PARAMETERS
 *
 *	int mode	- mode of archive (READ, WRITE, PASS) 
 */

#ifdef __STDC__

void next(int mode)

#else

void next(mode)
int             mode;		/* mode of archive (READ, WRITE, PASS) */

#endif
{
    char            msg[200];	/* buffer for message display */ 
    char            answer[20];	/* buffer for user's answer */
    int             ret;

    close_archive();

    sprintf(msg, "%s: Ready for volume %u\n%s: Type \"go\" when ready to proceed (or \"quit\" to abort): \07",
		   myname, arvolume + 1, myname);
    for (;;) {
	ret = nextask(msg, answer, sizeof(answer));
	if (ret == -1 || strcmp(answer, "quit") == 0) {
	    fatal("Aborted");
	}
	if (strcmp(answer, "go") == 0 && open_archive(mode) == 0) {
	    break;
	}
    }
    warnarch("Continuing", (OFFSET) 0);
}
