/* message.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_message[] = "$Id: message.c,v 2.52 2003/10/18 04:47:18 steve Exp $";
#endif
#if USE_PROTOTYPES
# include <stdarg.h>
#else
# include <varargs.h>
#endif

#if USE_PROTOTYPES
static void translate(char *terse);
#endif

static CHAR	verbose[200];
static FILE	*fperr, *fpinfo;
static ELVBOOL	msghiding;


/* redirect messages to a log file.  If "filename" is NULL then revert to
 * the normal reporting (stdout and stderr)
 */
void msglog(filename)
	char	*filename;
{
	/* if previously redirected, then stop redirection now */
	if (fperr != NULL && fperr != stderr)
		fclose(fperr);

	/* open the log file, if any */
	fperr = fpinfo = filename ? fopen(filename, "w") : NULL;
}

/* These are used for generating an error message which includes a file name
 * and line number, whenever possible.
 */
static char	*scriptnamedup;
static long	scriptline;
static ELVBOOL	scriptknown;

/* This is called by other code to inform the message module of the location
 * of the current command.  This allows the message module to report the file
 * name (or some other name, if not from a file) and line number (usually)
 * as part of an error message.  When executing commands from a buffer or
 * scanned string, "mark" is the value returned by scanmark() for the scanning
 * pointer; otherwise, "mark" should be NULL.  For strings, the "name" value
 * should describe the origin of the string: an alias name, or something else
 * for other strings.
 */
void msgscriptline(mark, name)
	MARK mark;
	char *name;
{
	char	newname[300];

	if (!mark || !markbuffer(mark))
	{
		if (name)
		{
			if (!scriptnamedup || strcmp(scriptnamedup, name))
			{
				free(scriptnamedup);
				sprintf(newname, "\"%s\"", name);
				scriptnamedup = strdup(newname);
				if (!mark || mark->offset == 0)
					scriptline = 1;
				else
					scriptline++;
			}
			scriptknown = ElvTrue;
			return;
		}
		else if (!scriptnamedup)
			scriptnamedup = strdup("unknown script");
		scriptline++;
		scriptknown = ElvFalse;
		return;
	}
	if (o_filename(markbuffer(mark)))
		strcpy(newname, tochar8(o_filename(markbuffer(mark))));
	else
		sprintf(newname, "(%s)", tochar8(o_bufname(markbuffer(mark))));
	if (!scriptnamedup || strcmp(newname, scriptnamedup))
	{
		free(scriptnamedup);
		scriptnamedup = strdup(newname);
	}
	scriptline = markline(mark);
	scriptknown = ElvTrue;
}

/* Copy a message into static verbose[] buffer, declared at the top of this
 * file.  If a buffer named "Elvis messages" exists, translate the message via
 * that buffer along the way.
 */
static void translate(terse)
	char	*terse;	/* terse form of error message */
{
	BUFFER	buf;	/* the "Elvis messages" buffer */
	MARKBUF	mark;	/* the start of the buffer */
	CHAR	*scan;	/* used for scanning the buffer */
	CHAR	*build;	/* used for copying chars into the verbose[] buffer */
	ELVBOOL	bol;	/* are we at the start of a line? */
	int	match;	/* used for counting characters that match */

	/* Copy the terse string into the verbose buffer, as a default */
	for (build = verbose, match = 0;
	     build < &verbose[QTY(verbose) - 1] && terse[match];
	     )
	{
		*build++ = terse[match++];
	}
	*build = '\0';

	/* if the "terse" option is on, then we're done */
	if (o_terse)
	{
		return;
	}

	/* Find the "Elvis messages" buffer.  If it doesn't exist, then
	 * no more translation is necessary.
	 */
	buf = buffind(toCHAR(MSG_BUF));
	if (!buf)
	{
		return;
	}

	/* Scan the buffer for a line which starts with the terse message
	 * followed by a colon.  If there is no such line, then we're done.
	 */
	for (scanalloc(&scan, marktmp(mark, buf, 0L)), match = 0, bol = ElvTrue;
	     scan;
	     scannext(&scan))
	{
		/* if this is a newline, then set "bol" and zero "match" */
		if (*scan == '\n')
		{
			bol = ElvTrue;
			match = 0;
			continue;
		}

		/* if we're in the middle of a match, then check to see if
		 * this position in "Elvis messages" matches the terse message.
		 */
		if (match >= 0)
		{
			if (*scan != (terse[match] ? terse[match] : ':'))
			{
				match = -1;
				continue;
			}
			if (!terse[match])
			{
				break;
			}
			match++;
		}
	}

	/* if we get here and "scan" isn't NULL, then we've found the line
	 * that translates this terse message and "scan" is pointing at the
	 * ':' that marks the end of the terse text.  Copy the verbose text
	 * after the ':' into the verbose[] variable.
	 */
	if (scan)
	{
		/* skip the ':' */
		scannext(&scan);

		/* at this point, the previous character was not a newline */
		bol = ElvFalse;

		/* copy the verbose message from the buffer */
		for (build = verbose; build < &verbose[QTY(verbose) - 1]; )
		{
			/* if non-whitespace, then copy the character */
			if (*scan != ' ' && *scan != '\t' && *scan != '\n')
			{
				*build++ = *scan;
				scannext(&scan);
				continue;
			}

			/* skip whitespace */
			while (scan && (*scan == ' ' || *scan == '\t' || *scan == '\n'))
			{
				bol = (*scan == '\n') ? ElvTrue : ElvFalse;
				scannext(&scan);
			}

			/* if this non-whitespace character appeared right after
			 * a newline, then we're done.  This is because an
			 * unindented line in this file always marks the start
			 * of the next terse message.  We're also done if we
			 * hit the end of the buffer.
			 */
			if (!scan || bol)
			{
				break;
			}

			/* whitespace is converted into a single blank
			 * character, except at the very beginning of the
			 * message where it is deleted completely.
			 */
			if (build != verbose)
			{
				*build++ = ' ';
			}
		}
		*build = '\0';
	}

	/* clean up */
	scanfree(&scan);
}


/* Set the message hiding flag to a given value & return its previous value */
ELVBOOL msghide(hide)
	ELVBOOL	hide;	/* should we hide messages? (else reveal them) */
{
	ELVBOOL	previous = msghiding;

	msghiding = hide;
	return previous;
}


/* output a message via the GUI.  Before calling the GUI, it subjects
 * the terse message to a series of transformations.  First, the
 * buffer "Elvis messages" is scanned to perform a user-configurable
 * transformation, such as translating it into a native language.
 * Then the message is evaluated via the calculate() function with
 * its "asmsg" parameter set to ElvTrue.
 *
 * The arg[] array passed into calculate() is built from the extra arguments
 * supplied to msg.  The beginning of the terse string can begin with a list
 * of characters enclosed in square brackets to indicate how the arguments
 * are to be converted to text strings.  The conversion letters are:
 *	d	The argument is a (long int), to be shown as a decimal number
 *	s	The argument is a (char *)
 *	S	The argument is a (CHAR *)
 *	c	The argument is a (char), to be shown as a string of length 1
 *	C	The argument is a (CHAR), to be shown as a string of length 1
 * If no bracketted list appears at the start of the string, then it is assumed
 * that the message has no extra arguments.
 *
 * For example, msg(MSG_info, "[s]\$1=$1, list=(list)", "foo") will output
 * "$1=foo, list=false"
 */
#if USE_PROTOTYPES
void msg(MSGIMP imp, char *terse, ...)
{
#else
void msg(imp, terse, va_alist)
	MSGIMP	imp;	/* message type */
	char	*terse;	/* terse form of message (may contain %s or %d) */
	va_dcl
{
#endif
	va_list	argptr;
	CHAR	*scan;
	CHAR	*arg[10];
	char	text[12], *str;
	int	i;
	BUFFER	buf;
	MARKBUF	mark;
	ELVBOOL	ding;

	/* if fperr and fpinfo are NULL, then use stderr and stdout */
	if (!fperr)
	{
		fperr = stderr;
#ifdef FEATURE_STDIN
		if (stdin_not_kbd)
			fpinfo = stderr;
		else
#endif
		fpinfo = stdout;
	}

	/* can't nest msg() calls.  If another call is in progress, exit now */
	if (*verbose || (msghiding && (imp == MSG_ERROR || imp == MSG_WARNING)))
	{
		if (imp == MSG_FATAL)
		{
			fprintf(fperr, "%s\n", terse);
			o_tempsession = ElvFalse;
			sesclose();
			if (gui)
				(*gui->term)();
			else if (chosengui)
				(*chosengui->term)();
#ifdef NDEBUG
			exit(1);
#else
			abort();
#endif
		}
		return;
	}

	/* Convert any arguments to CHAR strings */
	if (*terse == '[')
	{
#if USE_PROTOTYPES
		va_start(argptr, terse);
#else
		va_start(argptr);
#endif
		for (i = 0, terse++; *terse != ']'; i++, terse++)
		{
			assert(i < QTY(arg));

			/* convert argument to a CHAR string */
			switch (*terse)
			{
			  case 'd':
				sprintf(text, "%ld", va_arg(argptr, long));
				arg[i] = toCHAR(text);
				break;

			  case 'S':
				arg[i] = va_arg(argptr, CHAR *);
				if (!arg[i])
					arg[i] = toCHAR("NULL");
				break;

			  case 's':
				str = va_arg(argptr, char *);
				if (!str)
					str = "NULL";
				arg[i] = toCHAR(str);
				break;

			  case 'c':
				text[0] = va_arg(argptr, _char_);
				text[1] = '\0';
				arg[i] = toCHAR(text);
				break;

			  case 'C':
				text[0] = va_arg(argptr, _CHAR_);
				text[1] = '\0';
				arg[i] = toCHAR(text);
				break;

			  default:
				/* elvis source code should never have a
				 * bad format code.
				 */
				abort();
			}

			/* dynamically allocate a copy of that string.  This
			 * is done because some parameter types use the text[]
			 * buffer, and a later argument may need to reuse that
			 * buffer.
			 */
			arg[i] = CHARdup(arg[i]);
		}
		va_end(argptr);
		arg[i] = NULL;

		/* move the terse pointer past the closing ']' character */
		assert(*terse == ']');
		terse++;
	}
	else /* no bracketted list at start of terse string */
	{
		/* no extra arguments */
		arg[0] = NULL;
	}

	if (imp == MSG_FATAL && !arg[0])
	{
		/* set "scan" to the message text */
		scan = toCHAR(terse);
	}
	else
	{
		/* translate the terse message via "Elvis messages" buffer */
		translate(terse);

		/* expand any arguments or option names */
		scan = calculate(verbose, arg, CALC_MSG);
		if (!scan)
			scan = calculate(toCHAR(terse), arg, CALC_MSG);
		if (!scan)
			scan = toCHAR(terse);
	}

	/* If it starts with a ^G character, then ring the bell.  Also
	 * ring the bell if errorbells or warningbells is set
	 */
	switch (imp)
	{
	  case MSG_ERROR:	ding = o_errorbells;	break;
	  case MSG_WARNING:	ding = o_warningbells;	break;
	  default:		ding = ElvFalse;
	}
	if (*scan == ELVCTRL('G'))
	{
		scan++;
		ding = ElvTrue;
	}

	/* if warning or error from a script, then put a line number at the
	 * start of the message.
	 */
	if (scan != verbose
	 && (o_verbose >= 1 && scriptnamedup)
	 && (o_verbose >= 8
	    || ((imp == MSG_WARNING || imp == MSG_ERROR || imp == MSG_FATAL)
		&& (strlen(scriptnamedup) != strlen(EX_BUF) + 2 || strncmp(scriptnamedup + 1, EX_BUF, strlen(EX_BUF))))))
	{
		sprintf(tochar8(verbose), "%s, line %ld%s: ",
			scriptnamedup, scriptline, scriptknown ? "" : "?");
	}
	else
		*verbose = '\0';

	/* copy the string into verbose[] */
	CHARncat(verbose, scan, QTY(verbose) - 1 - CHARlen(verbose));
	verbose[QTY(verbose) - 1] = '\0';

	/* free the arg[] strings */
	for (i = 0; arg[i]; i++)
	{
		safefree(arg[i]);
	}

	/* Status and fatal messages are shown immediately, without flushing
	 * the message buffer.  During the initialization phase, other messages
	 * may also be output immediately.
	 *
	 * Also, since scan & change operations can't be done simultaneously,
	 * if there is currently a scan operation in progress then output the
	 * message immediately.
	 */
	if (!verbose[0] && imp != MSG_FATAL)
	{
		/* ignore it.  No output */
	}
	else  if ((o_verbose >= 1 && !windefault)
		|| !gui
#ifndef GUI_WIN32
		|| (eventcounter <= 1 && imp == MSG_ERROR)
#endif
		|| imp == MSG_STATUS
		|| imp == MSG_FATAL
		|| scan__top != NULL)
	{
		/* show the message */
		if (gui && windefault)
		{
			/* Either the GUI will show it, or we will */
			if (!gui->msg || !(*gui->msg)(windefault->gw, imp, verbose, (int)(scan - verbose)))
			{
				/* we have to show it... on bottom of window? */
				drawmsg(windefault, imp, verbose, (int)CHARlen(verbose));
			}

			/* For fatal error messages, also write it to fperr */
			if (imp == MSG_FATAL)
			{
				fprintf(fperr, "%s\n", verbose);
			}
		}
		else if (!gui || !gui->msg || !gui->exonly || !(*gui->msg)(0, imp, verbose, (int)(scan - verbose)))
		{
			/* no GUI yet, so just write it to fpinfo/fperr */
			if (imp == MSG_FATAL || imp == MSG_ERROR)
			{
				fprintf(fperr, "%s\n", verbose);
			}
			else
			{
				fprintf(fpinfo, "%s\r\n", verbose);
			}
		}

		/* clean up & exit */
		if (imp == MSG_FATAL)
		{
			o_tempsession = ElvFalse;
			sesclose();
			if (gui)
				(*gui->term)();
			else if (chosengui)
				(*chosengui->term)();
#ifdef NDEBUG
			exit(1);
#else
			abort();
#endif
		}
	}
	else
	{
		/* append the message to the message buffer */
		buf = bufalloc(toCHAR(MSGQUEUE_BUF), 0, ElvTrue);
		(void)marktmp(mark, buf, o_bufchars(buf));
		bufreplace(&mark, &mark, toCHAR("\n"), 1L);
		bufreplace(&mark, &mark, verbose, (long)CHARlen(verbose));
		bufreplace(&mark, &mark, toCHAR(imp>=MSG_ERROR ? "n" : " "), 1L);
	}

	/* if error, then alert the terminal */
	if (imp >= MSG_ERROR)
	{
		mapalert();
		if ((optflags(o_exitcode) & OPT_SET) == 0 && eventcounter <= 1)
		{
			o_exitcode = 1;
		}
	}

	/* if we're supposed to ring the bell, and this GUI has a bell,
	 * then ring it.
	 */
	if (ding && gui && gui->beep && windefault)
	{
		guibeep(windefault);
	}

	/* Zero the first byte of verbose[], so we can tell that we aren't
	 * in the middle of a message anymore.
	 */
	*verbose = '\0';
}


/* This function flushes messages from the message queue to the current
 * window.  This function should be called before outputting ex text,
 * before reading keystrokes, and when exiting elvis, after the GUI has
 * been shut down but before the session file has been closed.
 */
void msgflush()
{
	BUFFER	buf;
	MARK	mark, end;
	CHAR	*cp;
	int	len;
	MSGIMP	imp;

	/* if we have a GUI but no windows yet, then delay output */
	if (gui && !windefault)
	{
		return;
	}

	/* locate the message queue buffer, if any.  If it doesn't exist,
	 * or is empty, then we're done!
	 */
	buf = buffind(toCHAR(MSGQUEUE_BUF));
	if (!buf || o_bufchars(buf) == 0)
	{
		return;
	}

	/* Copy each line into the "verbose" buffer.  For each one, display
	 * the message as either info or an error.
	 */
	mark = markalloc(buf, 0);
	for (scanalloc(&cp, mark), len = 0; cp; scannext(&cp))
	{
		if (*cp == '\n')
		{
			verbose[len] = '\0';
			imp = (verbose[0]=='*' ? MSG_ERROR : MSG_INFO);

			/* show the message */
			if (gui && windefault)
			{
				/* Either the GUI will show it, or we will */
				if (!gui->msg || !(*gui->msg)(windefault->gw, imp, verbose + 1, len - 1))
				{
					/* we have to show it... on bottom of window? */
					drawmsg(windefault, imp, verbose + 1, len - 1);
				}
			}
			else
			{
				/* no GUI yet, so just write it to fpinfo/fperr */
				fprintf(imp >= MSG_ERROR ? fperr : fpinfo,
					"%s\n", verbose + 1);
			}
			len = 0;
		}
		else if (len < QTY(verbose) - 2)
		{
			verbose[len++] = *cp;
		}
	}
	scanfree(&cp);

	/* cleanup */
	end = markalloc(buf, o_bufchars(buf));
	bufreplace(mark, end, NULL, 0);
	markfree(mark);
	markfree(end);
	*verbose = '\0';
}


/* Translate a simple word or phrase, and return a dynamically-allocated
 * copy of the result.  This also has the side-effect of converting a
 * (char *) to a (CHAR *).
 *
 * This is used mostly for setting some options to locale-sensitive defaults
 */
CHAR *msgtranslate(word)
	char	*word;
{
	CHAR	*ret;

	/* Translate it */
	translate(word);

	/* Make a dynamic copy of it */
	ret = CHARkdup(verbose);

	/* Zero the first byte of verbose[] so msg() doesn't think we're
	 * doing a message.  That's important because msg() skips translation
	 * if an error message occurs while evaluating another message.
	 */
	verbose[0] = '\0';

	/* return the dynamically-allocated copy */
	return ret;
}
