/* exmake.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_make[] = "$Id: exmake.c,v 2.43 2003/10/18 04:47:18 steve Exp $";
#endif

#ifdef FEATURE_MAKE

#if USE_PROTOTYPES
static ELVBOOL parse_errmsg(void);
static RESULT gotoerr(EXINFO *xinf);
static void errprep(void);
#endif

ELVBOOL makeflag;

static char	*maybedir;	/* directory name extracted from errlist */
static CHAR	*errfile;	/* name of file where error was detected */
static long	errline;	/* the line number for an error */
static CHAR	*errdesc;	/* description of the error */
static MARK	errnext;	/* used for stepping through the error list */


/* This function tries to parse an error message from the (Elvis error list)
 * buffer; the errnext variable points to the next line in the buffer.
 *
 * Each error message is assumed to fit on a single line.  Within the line,
 * this parser attempts to locate three fields: the source file name, the line
 * number, and the description of the error.
 *
 * The line is scanned for words.  "Words", for this purpose, are considered
 * to be contiguous strings of non-whitespace characters other than ':'
 * (unless followed by a backslash) or any of ( ) , " '
 *
 * If a word happens to be the name of an existing file which is writable
 * by the user, then it is taken to be the filename.  Otherwise, if it contains
 * only digits then it is taken as the line number.
 *
 * After both the filename and line number have been found, the remainder of
 * the line is taken to be the description... except that any garbage between
 * the filename/line# and the interesting part of the description will be
 * discarded.  Usually, this discarded text consists simply of a colon and some
 * whitespace.
 *
 * Returns ElvTrue normally, or ElvFalse at the end of the list.  If it returns ElvTrue,
 * then the errfile, errline, and errdesc variables will have been set to
 * reflect anything that was found in the line; if all three are non-NULL
 * then the line contained an error message.
 */
static ELVBOOL parse_errmsg()
{
	BUFFER		errlist;	/* buffer containing err list */
	CHAR		*word;		/* dynamically allocated string */
	CHAR		*cp;		/* used for scanning line */
	CHAR		prevch;		/* previous char while scanning */
	DIRPERM		perms;		/* permissions of a file */
	long		top;		/* start of line */
	WINDOW		errwin;		/* window which displays errors */

	/* if we aren't already reading the errlist, then start at beginning */
	if (!errnext)
	{
		errlist = buffind(toCHAR(ERRLIST_BUF));
		if (!errlist)
			goto NoMoreErrors;
		errnext = markalloc(errlist, 0);
	}

	/* if we reached the end of the errlist, then say so */
	if (markoffset(errnext) >= o_bufchars(markbuffer(errnext)))
		goto NoMoreErrors;

	/* prepare to start scanning */
	top = markoffset(errnext);
	if (errfile)
		safefree(errfile);
	if (errdesc)
		safefree(errdesc);
	errfile = NULL;
	errline = 0;
	errdesc = NULL;
	word = NULL;

	/* scan the line */
	for (scanalloc(&cp, errnext), prevch = 0;
	     cp && *cp != '\n';
	     prevch = *cp, cp && scannext(&cp))
	{
		/* is the character legal in a word? */
		if (!elvspace(*cp) && CHARchr("():\"'`,", *cp) == NULL)
		{
			/* work around a quirk of Microsoft's compilers:
			 * if the sequence "/\" is seen, convert it to "/".
			 */
			if (*cp == '\\' && prevch == '/')
				continue;

			/* character in word */
			buildCHAR(&word, *cp);
			continue;
		}
		else if (*cp == ':' && word)
		{
			/* followed by a backslash? */
			if (scannext(&cp) && *cp == '\\')
			{
				/* both characters are in the word */
				buildCHAR(&word, (_CHAR_)':');
				buildCHAR(&word, *cp);
				continue;
			}

			/* The ':' isn't in the word, and we shouldn't have
			 * used up the following character yet.  Go back if
			 * possible.
			 */
			if (cp)
				scanprev(&cp);
		}
		else if (errfile && errline && (*cp == '"' && *cp == '\'' && *cp != '`'))
		{
			/* quotes are allowed at the start of a description */
			buildCHAR(&word, *cp);
			continue;
		}

		/* if we get here, then we aren't in a word.  If no word was
		 * in progress before this character, then we can ignore this
		 * character.
		 */
		if (!word)
			continue;

		/* So we must have a word that we need to process.  Is it the
		 * name of an existing, writable text file?
		 */
		if (!errfile
		 && (elvalnum(*word) || *word == '/' || *word == '\\')
		 && ((perms = dirperm(tochar8(word))) == DIR_READWRITE
			|| (o_anyerror && perms == DIR_READONLY))
		 && *ioeol(tochar8(word)) != 'b')
		{
			/* this is the name of the source file */
			errfile = word;
		}
		else if (dirperm(tochar8(word)) == DIR_DIRECTORY)
		{
			/* this may be the name of a directory where
			 * source files reside */
			if (maybedir)
				safefree(maybedir);
			maybedir = tochar8(word);
		}
		else if (!errfile
		 && maybedir
		 && ((perms = dirperm(dirpath(maybedir, tochar8(word))) ) == DIR_READWRITE
			|| (o_anyerror && perms == DIR_READONLY))
		 && *ioeol(dirpath(maybedir, tochar8(word))) != 'b')
		{
			errfile = CHARdup(toCHAR(dirpath(maybedir, tochar8(word))));
			safefree(word);
		}
		else if (!errline && calcnumber(word))
		{
			/* this is the line number */
			errline = atol(tochar8(word));
			safefree(word);
		}
		else if (errfile && errline &&
			(!CHARcmp(word + 1, toCHAR("arning"))
				|| !CHARcmp(word + 1, toCHAR("rror"))))
		{
			/* skip over error number and other garbage */
			safefree(word);
			word = NULL;
			while (cp && *cp != '\n' &&
				(elvspace(*cp) || elvdigit(*cp) || *cp == ':' || *cp == '-'))
			{
				scannext(&cp);
			}

			/* if we hit the end of the line, then we have no
			 * description.
			 */
			if (!cp || *cp == '\n')
			{
				errdesc = CHARdup(toCHAR("unknown error"));
			}
			
			/* back up one character, if possible */
			if (cp)
			{
				scanprev(&cp);
			}
		}
		else if (errfile && errline)
		{
			/* This word marks the start of the description.
			 * Collect the rest of it.
			 */
			while (cp && *cp != '\n')
			{
				buildCHAR(&word, *cp);
				scannext(&cp);
			}
			break;
		}
		else
		{
			/* just some word mixed in with filename & line# */
			safefree(word);
		}

		/* prepare for next word */
		word = NULL;
	}

	/* if we're in a word here, it must be the description */
	if (word)
	{
		assert(!errdesc);
		errdesc = word;
	}

	/* if scanning ended at '\n', then move past the '\n' */
	if (cp && *cp == '\n')
	{
		scannext(&cp);
	}

	/* remember where next line starts */
	if (cp)
	{
		marksetoffset(errnext, markoffset(scanmark(&cp)));
	}
	else
	{
		marksetoffset(errnext, o_bufchars(markbuffer(errnext)));
	}

	/* If a window is showing the error list, then highlight the current
	 * line, and leave the cursor at its end.
	 */
	errwin = winofbuf(NULL, markbuffer(errnext));
	if (errwin && !errwin->state->acton)
	{
		/* move the cursor to the end of the line */
		marksetoffset(errwin->cursor, markoffset(errnext) - 2);
		marksetoffset(errwin->state->top, markoffset(errwin->cursor));
		marksetoffset(errwin->state->bottom, markoffset(errwin->cursor));

		/* select the line, so it appears highlighted */
		if (!errwin->seltop)	errwin->seltop = markdup(errwin->cursor);
		if (!errwin->selbottom)	errwin->selbottom = markdup(errwin->cursor);
		marksetoffset(errwin->seltop, top);
		marksetoffset(errwin->selbottom, markoffset(errnext) - 1);
		errwin->selleft = 0;
		errwin->selright = INFINITY;
		errwin->seltype = 'l';
	}

	/* Set the changepos to end of the message.  This is so that if, after
	 * finding one or more messages, the user creates a new window for
	 * viewing the error list, the new window's cursor will start at the
	 * end of the current message instead of at the top of the buffer.
	 */
	markaddoffset(errnext, -1L);
	bufwilldo(errnext, ElvFalse);
	markaddoffset(errnext, 1L);

	/* return ElvTrue since there was a line for us to parse */
	scanfree(&cp);
	return ElvTrue;

NoMoreErrors:
	/* if a window is showing the error list, then unhighlight the old
	 * line (if any was highlighted)
	 */
	errlist = buffind(toCHAR(ERRLIST_BUF));
	errwin = errlist ? winofbuf(NULL, errlist) : NULL;
	if (errwin && !errwin->state->acton)
	{
		/* leave the cursor after the last error message */
		if (o_bufchars(errlist) > 2)
			marksetoffset(errwin->cursor, o_bufchars(errlist) - 2);
		else
			marksetoffset(errwin->cursor, 0L);
		marksetoffset(errwin->state->top, markoffset(errwin->cursor));
		marksetoffset(errwin->state->bottom, markoffset(errwin->cursor));

		/* unselect the line, so nothing appears highlighted */
		if (errwin->seltop)
		{
			assert(errwin->selbottom);
			markfree(errwin->seltop);
			markfree(errwin->selbottom);
			errwin->seltop = errwin->selbottom = NULL;
		}

		/* make sure the window will be redrawn */
		errwin->di->logic = DRAW_CHANGED;
	}

	return ElvFalse;
}


/* This function moves the cursor to the next error that was detected, and
 * outputs an informational line describing the error.  This function is
 * called by both ex_errlist() and ex_make().
 */
static RESULT gotoerr(xinf)
	EXINFO	*xinf;
{
	BUFFER	blamebuf;
	long	blameline;
	WINDOW	errwin;
	WINDOW	blamewin;

	/* if there is a window showing the error buffer, then search forward
	 * from its cursor. (Else search forward from end of previous error.)
	 */
	errwin = (errnext ? winofbuf(NULL, markbuffer(errnext)) : NULL);
	if (errwin)
		marksetoffset(errnext, markoffset(errwin->cursor));

	/* parse lines from the errlist buffer until we find an error message */
	do
	{
		if (!parse_errmsg())
		{
			msg(MSG_ERROR, "no more errors");
			return RESULT_ERROR;
		}
	} while (!errfile || !errline || !errdesc);

	/* load (if necessary) the file where error was detected. */
	blamebuf = bufload(NULL, tochar8(errfile), ElvFalse);
	assert(blamebuf != NULL);

	/* figure out which line the cursor should be left on, taking into
	 * account the fact that lines may have been inserted/deleted since
	 * the errlist was created.
	 */
	blameline = errline + o_buflines(blamebuf) - o_errlines(blamebuf);
	if (blameline < 1)
		blameline = 1;
	else if (blameline > o_buflines(blamebuf))
		blameline = o_buflines(blamebuf);

	/* move the cursor to the erroneous line of the new buffer */
	blamewin = winofbuf(NULL, blamebuf);
	if (blamewin && markbuffer(xinf->window->cursor) != blamebuf)
	{
		/* another window is already showing this buffer -- switch
		 * to that window, leaving this window unchanged.
		 */
		if (gui->focusgw)
		{
			(*gui->focusgw)(blamewin->gw);
		}
		else
		{
			eventfocus(blamewin->gw, ElvTrue);
		}
		marksetoffset(blamewin->cursor,
				lowline(bufbufinfo(blamebuf), blameline));
	}
	else
	{
		xinf->newcurs = markalloc(blamebuf,
				lowline(bufbufinfo(blamebuf), blameline));
	}

	/* describe the error */
	if (o_buflines(blamebuf) == o_errlines(blamebuf))
	{
		msg(MSG_INFO, "[dS]line $1: $2", errline, errdesc);
	}
	else if (o_buflines(blamebuf) < o_errlines(blamebuf))
	{
		msg(MSG_INFO, "[ddS]line $1-$2: $3", errline,
			o_errlines(blamebuf) - o_buflines(blamebuf), errdesc);
	}
	else
	{
		msg(MSG_INFO, "[ddS]line $1+$2: $3", errline,
			o_buflines(blamebuf) - o_errlines(blamebuf), errdesc);
	}

	return RESULT_COMPLETE;
}


/* This function does some preparatory steps towards creating a new errlist */
static void errprep()
{
	BUFFER	buf;
	MARKBUF	top;
	MARKBUF	end;

	/* find the "Elvis error list" buffer */
	buf = buffind(toCHAR(ERRLIST_BUF));
	assert(buf != NULL);

	/* if it has old text in it, discard that text */
	if (o_bufchars(buf) > 0)
	{
		bufreplace(marktmp(top, buf, 0),
			   marktmp(end, buf, o_bufchars(buf)),
			   NULL, 0);
	}

	/* reset the "errnext" mark so we start at the top of the buffer */
	if (errnext)
	{
		markfree(errnext);
		errnext = NULL;
	}

	/* clobber the maybedir */
	if (maybedir)
	{
		safefree(maybedir);
		maybedir = NULL;
	}

	/* remember how many lines each buffer has now */
	for (buf = NULL; (buf = buflist(buf)) != NULL; )
	{
		o_errlines(buf) = o_buflines(buf);
	}
}


RESULT	ex_errlist(xinf)
	EXINFO	*xinf;
{
	BUFFER	errbuf;
	WINDOW	errwin;
	MARKBUF	from;
	ELVBOOL	retval;

	assert(xinf->command == EX_ERRLIST);

	/* was a filename given? */
	if (xinf->nfiles > 0 || xinf->rhs)
	{
		/* prepare to create a new error list */
		errprep();

		/* load the buffer from the named file */
		errbuf = bufalloc(toCHAR(ERRLIST_BUF), 0, ElvTrue);
		retval = bufread(marktmp(from, errbuf, 0), xinf->rhs ? tochar8(xinf->rhs) : xinf->file[0]);

		/* turn off the "modified" flag on the (Elvis error list) buf */
		o_modified(errbuf) = ElvFalse;

		/* if there are any windows showing this buffer, move its cursor
		 * to offset 0
		 */
		for (errwin = NULL; (errwin = winofbuf(errwin, errbuf)) != NULL; )
		{
			marksetoffset(errwin->cursor, 0L);
		}

		/* if failed to read errors, then return RESULT_ERROR */
		if (!retval)
			return RESULT_ERROR;
	}

	/* move the cursor to the next error */
	return gotoerr(xinf);
}


RESULT	ex_make(xinf)
	EXINFO	*xinf;
{
	CHAR	*args[3];/* arguments to calculate() expression */
	CHAR	*str;	/* shell command string */
	CHAR	*io;	/* buffer for reading chars from program */
	int	nio;	/* number of characters read */
	MARKBUF	start, end;/* ends of errlist buffer, used for appending */
	ELVBOOL	origrefresh;
	long	origpollfreq;
	BUFFER	buf;
	WINDOW	errwin;	/* window which is displaying errors */

	assert(xinf->command == EX_MAKE || xinf->command == EX_CC);

	/* if any user buffer was modified & not saved, then complain unless
	 * '!' given.
	 */
	if (!xinf->bang)
	{
		for (buf = NULL; (buf = buflist(buf)) != NULL; )
		{
			if (!o_internal(buf) && o_modified(buf))
			{
				if (!o_autowrite)
				{
					msg(MSG_ERROR, "[S]$1 modified, not saved", o_filename(buf) ? o_filename(buf) : o_bufname(buf));
					return RESULT_ERROR;
				}

				/* write the buffer */
				if (!bufsave(buf, ElvFalse, ElvFalse))
					/* error message already given */
					return RESULT_ERROR;
			}
		}
	}

	/* create the shell command line that we'll be running */
	buf = markbuffer(&xinf->defaddr);
	args[0] = (xinf->rhs ? xinf->rhs : toCHAR(""));
	args[1] = (o_filename(buf) ? o_filename(buf) : toCHAR(""));
	args[2] = NULL;
	str = calculate(xinf->command==EX_CC ? o_cc(buf) : o_make(buf), args, CALC_MSG);
	if (!str)
	{
		/* error message already given */
		return RESULT_ERROR;
	}

	/* output the command name, so the user knows what's happening */
	origrefresh = o_exrefresh;
	o_exrefresh = ElvTrue;
	origpollfreq = o_pollfrequency;
	o_pollfrequency = 1;
	(void)guipoll(ElvTrue);
	drawextext(xinf->window, str, (int)CHARlen(str));
	drawextext(xinf->window, toCHAR("\n"), 1);

	/* prepare to create the new errlist */
	errprep();

	/* run the program, and read its stdout/stderr.  Write this to the
	 * window as ex output text, and also store it in the errlist buffer.
	 */
	if (!prgopen(tochar8(str), ElvFalse, ElvTrue) || !prggo())
	{
		/* failed -- error message already given */
		o_exrefresh = origrefresh;
		o_pollfrequency = origpollfreq;
		return RESULT_ERROR;
	}
	buf = buffind(toCHAR(ERRLIST_BUF));
	assert(buf != NULL);
	if (o_bufchars(buf) > 0)
	{
		bufreplace(marktmp(start, buf, 0), marktmp(end, buf, o_bufchars(buf)), NULL, 0);
	}
	io = (CHAR *)safealloc(1024, sizeof(CHAR));
	(void)marktmp(end, buf, 0);
	while (!guipoll(ElvFalse) && (nio = prgread(io, 1024)) > 0)
	{
		/* show it on the screen */
		drawextext(xinf->window, io, nio);

		/* append it to the buffer */
		bufreplace(&end, &end, io, nio);
		markaddoffset(&end, nio);
	}
	(void)prgclose();
	safefree(io);
	o_exrefresh = origrefresh;
	o_pollfrequency = origpollfreq;

	/* turn off the "modified" flag on the (Elvis error list) buf */
	o_modified(buf) = ElvFalse;

	/* if there is a window showing this buffer, move its cursor
	 * to offset 0
	 */
	for (errwin = NULL; (errwin = winofbuf(errwin, buf)) != NULL; )
	{
		marksetoffset(errwin->cursor, 0L);
	}

#if 0
	/* delay the first error message until after <Enter> */
	if (eventcounter > 5)
	{
		makeflag = ElvTrue;
		morehit = ElvFalse;
	}
#else
	/* don't wait for the user to hit <Enter> after this */
	xinf->window->di->logic = DRAW_SCRATCH;
	xinf->window->di->drawstate = DRAW_VISUAL;
#endif

	/* move the cursor to the first error */
	return gotoerr(xinf);
}
#endif /* FEATURE_MAKE */
