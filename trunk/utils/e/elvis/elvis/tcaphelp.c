/* tcaphelp.c */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_tcaphelp[] = "$Id: tcaphelp.c,v 2.20 2003/10/17 17:41:23 steve Exp $";
#endif
#if defined(GUI_TERMCAP) || defined(GUI_OPEN)
# include <unistd.h>
# include <fcntl.h>
# ifdef NEED_WINSIZE
#  include <sys/types.h>
#  include <sys/stream.h>
#  include <sys/ptem.h>
# endif

#if defined(linux) || defined(NEED_IOCTL_H)
# include <sys/ioctl.h>
#endif
#if defined(ultrix) || defined(__osf__)
extern int ioctl P_((int d, int request, void *argp));
#endif
#ifdef USE_PROTOTYPES
static void ttyinit2(void);
#endif

/* This file includes one of three three versions of low-level tty control
 * functions: one for POSIX, one for BSD, and one for SysV.  If _POSIX_SOURCE
 * is defined, then the POSIX versions are used; else if bsd is defined, then
 * the BSD versions are used; else SYSV is used.
 *
 * The version-specific functions in the included file are:
 *	ttyinit2()		- remember the initial serial line configuration
 *	ttyraw()		- switch to the mode that elvis runs it
 *	ttynormal()		- switch back to the mode saved by ttyinit()
 *	ttyread(buf,len,timeout)- read characters, possibly with timeout
 *
 * The generic UNIXish functions in this file:
 *	ttywrite(buf,len)	- write characters
 *	ttytermtype()		- return the name of the terminal type
 *	ttysize()		- determine the terminal size
 */

/* This variable is used by all versions to indicate which signals have
 * been caught.
 */
static long caught;

/* This variable is used by all versions to store the fd of the keyboard */
static int ttykbd = 0;
static int ttyscr = 1;

/* include the version-specific functions */
#ifdef USE_SGTTY
# include "tcapbsd.h"
#else
# ifdef USE_TERMIO
#  include "tcapsysv.h"
# else
#  include "tcaposix.h" /* this is the preferred version */
# endif
#endif

/* initialize the screen & keyboard */
void ttyinit()
{
#ifdef FEATURE_STDIN
	/* choose a keyboard fd */
	if (stdin_not_kbd)
	{
		if (isatty(2))
			ttykbd = 2;
		else
		{
			ttykbd = open("/dev/tty", O_RDONLY);
			if (ttykbd < 0)
			{
				perror("/dev/tty");
				exit(2);
			}
		}
	}
#endif

	/* choose a screen fd */
	if (!isatty(1))
	{
		if (isatty(2))
			ttyscr = 2;
		else
		{
			ttyscr = open("/dev/tty", O_WRONLY);
			if (ttykbd < 0)
			{
				perror("/dev/tty");
				exit(2);
			}
		}
	}

	ttyinit2();
}

/* write characters out to the screen */
void ttywrite(buf, len)
	char	*buf;	/* buffer, holds characters to be written */
	int	len;	/* number of characters in buf */
{
	write(ttyscr, buf, (size_t)len);
}

/* determine the terminal type */
char *ttytermtype()
{
	char	*type;

	type = getenv("TERM");
	if (!type)
	{
		type = TTY_DEFAULT;
	}
	return type;
}


/* This function gets the window size. */
ELVBOOL ttysize(linesptr, colsptr)
	int	*linesptr;	/* where to store the number of rows */
	int	*colsptr;	/* where to store the number of columns */
{
#ifdef TIOCGWINSZ
	struct winsize size;

	/* try using the TIOCGWINSZ call, if defined */
	if (ioctl(ttyscr, TIOCGWINSZ, &size) >= 0)
	{
		*linesptr = size.ws_row;
		*colsptr = size.ws_col;
		return ElvTrue;
	}
#endif

	/* no special way to detect screen size */
	return ElvFalse;
}

/* Check for signs of boredom from user, so we can abort a time-consuming
 * operation.  Here we check to see if SIGINT has been caught recently.
 */
ELVBOOL ttypoll(reset)
	ELVBOOL reset;
{
	return (caught & (1L << SIGINT)) ? ElvTrue : ElvFalse;
}

#ifdef SIGSTOP
/* Send SIGSTOP if the shell supports it, and return */
RESULT ttystop()
{
	/* the Bourne shell can't handle ^Z, but BASH can */
	if (!CHARcmp(o_shell, toCHAR("/bin/sh")) &&
		!getenv("BASH_VERSION") && !getenv("BASH"))
	{
		return RESULT_MORE;
	}
	else
	{
		/* save user buffers, if necessary */
		eventsuspend();

		/* switch the tty out of raw mode */
		ttysuspend();

		/* stop and wait for a SIGCONT */
		kill(0, SIGSTOP);

		/* switch back to raw mode */
		ttyresume(ElvTrue);

		return RESULT_COMPLETE;
	}
}
#endif

#endif /* GUI_TERMCAP */
