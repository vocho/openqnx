/* search.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_search[] = "$Id: search.c,v 2.56 2003/10/17 17:41:23 steve Exp $";
#endif

static RESULT searchenter P_((WINDOW win));
static RESULT forward P_((WINDOW win));
static RESULT backward P_((WINDOW win));

#ifdef FEATURE_INCSEARCH
static RESULT incsearch P_((WINDOW win, VIINFO *vinf));
static RESULT incparse P_((_CHAR_ key, void *info));
static RESULT incperform P_((WINDOW win));
static ELVCURSOR incshape P_((WINDOW win));
#endif

/* This is the font to use for the hlsearch option.  It is initialize in
 * draw.c.  This is important because the font must be allocated while no
 * temporary fonts are in use -- i.e, before screen updating begins.
 */
char	searchfont;

/* This is the previous regular expression.  We need to remember it after
 * it has been used, so we can implement the <n> and <shift-N> commands.
 */
regexp	*searchre;	/* the regular expression itself */
ELVBOOL searchforward;	/* is the searching being done in a forward direction?*/
ELVBOOL	searchhasdelta;	/* is the regular expression followed by a line delta?*/
long	searchdelta;	/* if searchhasdelta, this is the line offset value */
static ELVBOOL autoselect;/* select the matching text? */

#ifdef FEATURE_HLSEARCH
static ELVBOOL	searchhlflag;	/* is hlsearch currently enabled? */
#endif

/* This function is called when the user hits <Enter> after typing in a
 * regular expression for the visual </> and <?> commands.  It compiles
 * the regular expression, and then matches it in either a forward or
 * backward direction.
 */
static RESULT	searchenter(win)
	WINDOW	win;	/* window where a regexp search line has been entered */
{
	CHAR	*regex;	/* holds regular expression, as string */
	CHAR	delim;	/* delimiter, either '?' or '/' */
	CHAR	*cp;	/* used when copying */
	STATE	*state = win->state;
	ELVBOOL	whole;
	CHAR	sign;
	CHAR	*tmp, *tmp2;

	assert(markoffset(state->top) <= markoffset(state->cursor));
	assert(markoffset(state->cursor) <= markoffset(state->bottom));

	/* copy the regexp into a buffer */
	scanalloc(&cp, state->top);
	if (!cp)
		goto Error;
	delim = *cp;
	if ((delim != '/' && delim != '?') || !scannext(&cp))
		goto Error;
	regex = regbuild(delim, &cp, ElvTrue);

	/* Check for flags and/or line delta */
	searchhasdelta = whole = ElvFalse;
	autoselect = o_autoselect;
	for (; cp && *cp && *cp != '\n'; scannext(&cp))
	{
		switch (*cp)
		{
		  case 'v':
			autoselect = ElvTrue;
			break;

		  case 'n':
			autoselect = ElvFalse;
			break;

		  case 'i':
			o_ignorecase = ElvTrue;
			o_smartcase = ElvFalse;
			break;

		  case 'c':
			o_ignorecase = ElvFalse;
			break;

		  case 's':
			o_ignorecase = ElvTrue;
			o_smartcase = ElvTrue;
			break;

		  case 'w':
		  case 'x':
			if (whole)
			{
				msg(MSG_ERROR, "excessive 'w' or 'x' flags");
				safefree(regex);
				return RESULT_ERROR;
			}
			if (!*regex)
			{
				msg(MSG_ERROR, "can't use 'w' or 'x' flags with an empty regexp");
				safefree(regex);
				return RESULT_ERROR;
			}
			whole = ElvTrue;
			tmp = NULL;
			if (*cp == 'x')
			{
				buildCHAR(&tmp, '^');
				for (tmp2 = regex; *tmp2; tmp2++)
					buildCHAR(&tmp, *tmp2);
				buildCHAR(&tmp, '$');
			}
			else
			{
				buildCHAR(&tmp, '\\');
				buildCHAR(&tmp, '<');
				for (tmp2 = regex; *tmp2; tmp2++)
					buildCHAR(&tmp, *tmp2);
				buildCHAR(&tmp, '\\');
				buildCHAR(&tmp, '>');
			}
			safefree(regex);
			regex = tmp;
			break;

		  case '+':
		  case '-':
			sign = *cp;
			searchhasdelta = ElvTrue;
			for (searchdelta = 0; scannext(&cp) && elvdigit(*cp); )
				searchdelta = searchdelta * 10 + *cp - '0';
			if (sign == '-')
				searchdelta = -searchdelta;
			break;

		  default:
			if (!elvspace(*cp))
			{
				msg(MSG_ERROR, "[C]unknown flag $1", *cp);
				safefree(regex);
				return RESULT_ERROR;
			}
		}
	}
	scanfree(&cp);

	/* Compile the regular expression.  An empty regular expression is
	 * identical to the current regular expression, so we don't need to
	 * compile it in that case.  If there are any errors, then fail.
	 * The regcomp function will have already output an error message.
	 */
	if (*regex)
	{
		/* free the previous regular expression, if any */
		if (searchre)
			safefree((void *)searchre);

		/* compile the new one */
		searchre = regcomp(regex, win->cursor);
	}
	else if (!searchre)
	{
		msg(MSG_ERROR, "no previous RE");
	}

	/* we don't need the text version of the regex any more */
	safefree(regex);

	/* detect errors in compilation */
	if (!searchre)
	{
		return RESULT_ERROR;
	}

	/* '/' implies forward search, '?' implies backward search */
	searchforward = (ELVBOOL)(delim == '/');

	/* Now we can do this search exactly like an <n> search command, except
	 * that it'll be applied to the previous stratum instead of this one.
	 */
	return RESULT_COMPLETE;

Error:
	scanfree(&cp);
	msg(MSG_ERROR, "bad search");
	return RESULT_ERROR;
}

/* search forward from the cursor position for the previously compiled
 * regular expression.
 */
static RESULT forward(win)
	WINDOW	win;	/* the window to search in */
{
	MARKBUF	sbuf;	/* buffer, holds search mark */
	long	lnum;	/* line number -- used for scanning through buffer */
	BLKNO	bi;	/* bufinfo block of the buffer being searched */
	ELVBOOL	bol;	/* are we at the beginning of a line? */
	ELVBOOL	wrapped;/* ElvTrue if we've wrapped at the end of the buffer */
	long	start;	/* line where we started (detects failure after wrap) */
	CHAR	*cp;

#ifdef FEATURE_HLSEARCH
	searchhlflag = ElvTrue;
#endif
	/* setup: determine the line number, whether the cursor is at the
	 * beginning of the line, and where the buffer ends.
	 */
	sbuf = *win->state->cursor;
	if (markoffset(&sbuf) + 1 >= o_bufchars(markbuffer(&sbuf)))
		marksetoffset(&sbuf, 0L);
	else
		markaddoffset(&sbuf, 1);
	bi = bufbufinfo(markbuffer(&sbuf));
	lnum = markline(&sbuf);
	bol = (ELVBOOL)(lowline(bi, lnum) == markoffset(&sbuf));
	wrapped = ElvFalse;
	start = lnum;

	/* search */
	scanalloc(&cp, &sbuf);
	for (;
	     !regexec(searchre, &sbuf, bol);
	     marksetoffset(&sbuf, searchre->nextlinep), bol = ElvTrue)
	{
		/* If user gives up, then fail */
		if (guipoll(ElvFalse))
		{
			scanfree(&cp);
			return RESULT_ERROR;
		}

		/* Advance to next line.  Maybe wrap at the bottom of the
		 * buffer.  This is also where failed searches are detected.
		 */
		lnum++;
		if (wrapped && lnum > start)
		{
			scanfree(&cp);
			msg(MSG_ERROR, "no match");
			return RESULT_ERROR;
		}
		else if (lnum > o_buflines(markbuffer(&sbuf)))
		{
			if (o_wrapscan)
			{
				lnum = 1;
				searchre->nextlinep = 0L;
				wrapped = ElvTrue;
			}
			else
			{
				scanfree(&cp);
				msg(MSG_ERROR, "no match below");
				return RESULT_ERROR;
			}
		}
		scanseek(&cp, &sbuf);
	}
	scanfree(&cp);

	/* if we get here, then the search succeded */
	marksetoffset(win->state->cursor, searchre->leavep);

	/* don't leave the cursor on a '\n' character, except on empty lines */
	if (searchre->leavep > 0 && scanchar(win->state->cursor) == '\n')
	{
		markaddoffset(win->state->cursor, -1L);
		if (scanchar(win->state->cursor) == '\n')
			markaddoffset(win->state->cursor, 1L);
	}

	if (wrapped)
		msg(MSG_INFO, "wrapped");
	return RESULT_COMPLETE;
}

/* search backward from the cursor position for the previously compiled
 * regular expression.
 */
static RESULT backward(win)
	WINDOW	win;	/* the window to search in */
{
	MARKBUF	sbuf;	/* buffer, holds search mark */
	BLKNO	bi;	/* bufinfo block of the buffer being searched */
	long	lnum;	/* line number -- used for scanning through buffer */
	ELVBOOL	wrapped;/* ElvTrue if we've wrapped at the end of the buffer */
	long	start;	/* line where we started (detects failure after wrap) */
	long	endln;	/* offset of the end of a line (really start of next line) */
	long	last;	/* offset of last match found in a line */
	long	laststartp;/* offset of the start of the last match in a line */
	long	lastendp;/* offset of the end of the last match in a line */
	long	lastleavep;/* offset of the leave position of the last match */
	CHAR	*cp;	/* used for scanning for newline characters */

#ifdef FEATURE_HLSEARCH
	searchhlflag = ElvTrue;
#endif
	/* setup: determine the line number, whether the cursor is at the
	 * beginning of the line, and where the buffer ends.
	 */
	sbuf = *win->state->cursor;
	bi = bufbufinfo(markbuffer(&sbuf));
	lnum = markline(&sbuf);
	marksetoffset(&sbuf, lowline(bi, lnum));
	endln = (lnum == o_buflines(markbuffer(&sbuf))
		? o_bufchars(markbuffer(&sbuf)) : lowline(bi, lnum + 1));
	wrapped = ElvFalse;
	start = lnum;

	/* check for match in same line, to left of cursor */
	if (regexec(searchre, &sbuf, ElvTrue)
	 && searchre->leavep < markoffset(win->state->cursor))
	{
		/* find the last match in the line that occurs before the
		 * current position.
		 */
		do
		{
			last = searchre->leavep;
			laststartp = searchre->startp[0];
			lastendp = searchre->endp[0];
			lastleavep = searchre->leavep;
			marksetoffset(&sbuf, last + 1);
		} while (last + 1 < endln
			&& regexec(searchre, &sbuf, ElvFalse)
			&& searchre->leavep < markoffset(win->state->cursor));
		marksetoffset(win->state->cursor, last);
		searchre->startp[0] = laststartp;
		searchre->endp[0] = lastendp;
		searchre->leavep = lastleavep;
		return RESULT_COMPLETE;
	}

	/* search for some other line with a match */
	scanalloc(&cp, &sbuf);
	do
	{
		/* If user gives up, then fail */
		if (guipoll(ElvFalse))
		{
			scanfree(&cp);
			return RESULT_ERROR;
		}

		/* Advance to preceding line.  Maybe wrap at the top of the
		 * buffer.  This is also where failed searches are detected.
		 */
		lnum--;
		if (wrapped && lnum < start)
		{
			scanfree(&cp);
			msg(MSG_ERROR, "no match");
			return RESULT_ERROR;
		}
		else if (lnum < 1)
		{
			if (o_wrapscan)
			{
				lnum = o_buflines(markbuffer(&sbuf));
				endln = o_bufchars(markbuffer(&sbuf));
				wrapped = ElvTrue;
			}
			else
			{
				scanfree(&cp);
				msg(MSG_ERROR, "no match above");
				return RESULT_ERROR;
			}

			/* find the start of the last line */
			marksetoffset(&sbuf, lowline(bi, lnum));
			scanseek(&cp, &sbuf);
		}
		else
		{
			endln = markoffset(&sbuf);
			scanprev(&cp);
			assert(cp && *cp == '\n');
			do
			{
				scanprev(&cp);
			} while (cp && *cp != '\n');
			if (cp)
			{
				scannext(&cp);
				sbuf = *scanmark(&cp);
			}
			else
			{
				marksetoffset(&sbuf, 0L);
				scanseek(&cp, &sbuf);
			}
		}

	} while (!regexec(searchre, &sbuf, ElvTrue));

	/* If we get here, then the search succeded -- meaning we have found
	 * a line which contains at least one match.  Now we need to locate
	 * the LAST match in that line.
	 */
	do
	{
		last = searchre->leavep;
		laststartp = searchre->startp[0];
		lastendp = searchre->endp[0];
		lastleavep = searchre->leavep;
		marksetoffset(&sbuf, last + 1);
	} while (last + 1 < endln
		&& regexec(searchre, &sbuf, ElvFalse));
	scanfree(&cp);

	/* move to the last match */
	marksetoffset(win->state->cursor, last);
	searchre->startp[0] = laststartp;
	searchre->endp[0] = lastendp;
	searchre->leavep = lastleavep;

	/* don't leave the cursor on a '\n' character, except on empty lines */
	if (searchre->leavep > 0 && scanchar(win->state->cursor) == '\n')
	{
		markaddoffset(win->state->cursor, -1L);
		if (scanchar(win->state->cursor) == '\n')
			markaddoffset(win->state->cursor, 1L);
	}

	if (wrapped)
		msg(MSG_INFO, "wrapped");
	return RESULT_COMPLETE;
}

RESULT m_search(win, vinf)
	WINDOW	win;	/* window where operation should take place */
	VIINFO	*vinf;	/* the command to execute */
{
	ELVBOOL	fwd;	/* is this search going to go forward? */
	ELVBOOL	hint;	/* show a "/" or "?" during the search? */
	RESULT	rc;	/* return code */
	VIINFO	vinfbuf;/* used for constructing a vi command for selections */

	assert(vinf->command == 'n' || vinf->command == 'N'
		|| vinf->command == '/' || vinf->command == '?'
		|| vinf->command == ELVCTRL('A')
		|| vinf->command == ELVG('d')
		|| vinf->command == ELVG('D'));

	/* If repeating an operator with / or ?, then use n instead.  (Since
	 * search commands don't change the buffer themselves, the only way <.>
	 * would repeat one is in conjunction with an operator.  So we don't
	 * need do explicitly check for an operator; we KNOW there is one.)
	 */
	if ((vinf->tweak & TWEAK_DOTTING) != 0
			&& (vinf->command == '/' || vinf->command == '?'))
	{
		vinf->command = 'n';
	}

	hint = ElvFalse;
	switch (vinf->command)
	{
	  case 'n':
		fwd = searchforward;
		hint = ElvTrue;
		break;

	  case 'N':
		/* reverse the direction of the search */
		fwd = (ELVBOOL)!searchforward;
		hint = ElvTrue;
		break;

	  case ELVCTRL('A'):
	  case ELVG('d'):
	  case ELVG('D'):
		/* Free the previous regular expression, if any */
		if (searchre)
		{
			safefree((void *)searchre);
		}

		/* Compile the regexp /\<\@\>/ */
		searchre = regcomp(toCHAR("\\<\\@\\>"), win->cursor);
		if (!searchre)
			return RESULT_ERROR;

		/* always search in the forward direction */
		fwd = searchforward = ElvTrue;
		searchhasdelta = ElvFalse;
		searchdelta = 0L;
		hint = ElvTrue;

		/* for gd and gD, start at top of function or file */
		if (vinf->command == ELVG('d'))
		{
			vinfbuf.command = vinfbuf.key2 = '[' ;
			vinfbuf.count = 1;
			(void)m_bsection(win, &vinfbuf);
		}
		else if (vinf->command == ELVG('D'))
		{
			marksetoffset(win->state->cursor, 0L);
		}
		break;

	  default:
		/* There are two modes of operation here.  If the "more"
		 * flag is not set, then we need to push a new stratum for
		 * reading a regular expression to search for.  If "more"
		 * is set, then we act just like 'n'.
		 */
		if (win->state->flags & ELVIS_MORE)
		{
			win->state->flags &= ~ELVIS_MORE;
			fwd = searchforward;
			break;
		}

#ifdef FEATURE_INCSEARCH
		/* if supposed to use incremental search, then stay in full-
		 * screen mode but push a new input state which reads parts
		 * of a regexp, and immediately searches for it.
		 */
		if (o_incsearch && !vinf->oper)
			return incsearch(win, vinf);
#endif

		/* I guess we just want to read a regexp line */
		statestratum(win, toCHAR(REGEXP_BUF), vinf->command, searchenter);
		return RESULT_MORE;
	}

	/* If we get here, we need to do an 'n' search */

	/* This'll only work if we have a regexp */
	if (!searchre)
	{
		msg(MSG_ERROR, "no previous search");
		return RESULT_ERROR;
	}

	/* give a hint that we're searching, if necessary */
	if (hint && win->di->drawstate == DRAW_VISUAL)
	{
		drawmsg(win, MSG_STATUS, toCHAR(fwd ? "/" : "?"), 1);
	}

	/* do the search */
	if (fwd)
	{
		rc = forward(win);
	}
	else
	{
		rc = backward(win);
	}

	/* remove the hint that we're searching, if necessary */
	if (hint && win->di->drawstate == DRAW_VISUAL)
	{
		drawmsg(win, MSG_STATUS, toCHAR(""), 0);
		win->di->newmsg = ElvFalse;
	}

	/* if the search was successful, take care of autoselect & line delta */
	if (rc == RESULT_COMPLETE)
	{
#ifdef FEATURE_V
		/* autoselect, maybe with line delta */
		if (autoselect)
		{
			/* Cancel any previous selection */
			vinfbuf.command = ELVCTRL('[');
			(void)v_visible(win, &vinfbuf);

			/* Move to the end of the matched text, and start
			 * marking characters from there.  Note that since
			 * visible selections include the last character, and
			 * endp[0] contains the offset of the first character
			 * AFTER the matched text, we usually want to subtract
			 * 1 from endp[0].  However, if the matching text is
			 * is zero-length, we select nothing and give a warning.
			 */
			if (searchre->startp[0] < searchre->endp[0])
			{
				marksetoffset(win->state->cursor, searchre->endp[0] - 1);
				vinfbuf.command = (searchhasdelta ? 'V' : 'v');
				(void)v_visible(win, &vinfbuf);
			}
			else if (searchhasdelta)
			{
				marksetoffset(win->state->cursor, searchre->endp[0] - 1);
				vinfbuf.command = (searchhasdelta ? 'V' : 'v');
				(void)v_visible(win, &vinfbuf);
			}
			else
			{
				msg(MSG_INFO, "match is zero-length");
			}

			/* Move back to the beginning of the matched text,
			 * stretching the selected region to follow the cursor.
			 * This is also a good time to factor in the line-delta
			 * if any.
			 */
			if (searchhasdelta)
			{
				/* non-zero deltas move the cursor to the front
				 * of a relative line.
				 */
				if (searchdelta != 0)
				{
					/* move the cursor some number of whole lines */
					marksetoffset(win->state->cursor,
						markoffset(dispmove(win, searchdelta, 0)));

					/* leave the cursor at the front of that line */
					vinf->tweak |= TWEAK_FRONT;
				}
				(void)v_visible(win, NULL);
			}
			else if (searchre->startp[0] < searchre->endp[0])
			{
				marksetoffset(win->state->cursor, searchre->startp[0]);
				(void)v_visible(win, NULL);
			}
			/* else we already gave a warning instead of selecting text */
		}
		else
#endif /* FEATURE_V */
		/* no autoselect, but maybe still a line delta */
		if (searchhasdelta)
		{
			/* All line deltas have the side effect of making the
			 * regexp search command be a line-oriented command
			 * instead of a character-oriented command.
			 */
			vinf->tweak |= TWEAK_LINE | TWEAK_INCL;

			/* non-zero deltas move the cursor to the front
			 * of a relative line.
			 */
			if (searchdelta != 0)
			{
				/* move the cursor some number of whole lines */
				marksetoffset(win->state->cursor,
					markoffset(dispmove(win, searchdelta, 0)));

				/* leave the cursor at the front of that line */
				vinf->tweak |= TWEAK_FRONT;
			}
		}
	}

	return rc;
}

#ifdef FEATURE_HLSEARCH
/* search for matches between a start mark and some end offset. */
void searchhighlight(win, linesshown, end)
	WINDOW	win;
	int	linesshown;
	long	end;
{
	MARKBUF	mark;
	int	i, j, endrow, plainfont;
	ELVBOOL	bol;
	DRAWLINE *line;
	long	endline;

	/* if no previous regular expression, then fail */
	if (!searchre || !o_hlsearch || !searchhlflag)
		return;

	/* for each line... */
	for (i = 0, line = win->di->newline; i < linesshown; i++, line++)
	{
		/* for each match in the line... */
		if (i + 1 < linesshown)
		{
			endrow = (line + 1)->startrow * win->di->columns;
			endline = (line + 1)->start;
		}
		else
		{
			endrow = (win->di->rows - 1) * win->di->columns;
			endline = end;
		}
		for ((void)marktmp(mark, markbuffer(win->cursor), line->start), bol = ElvTrue;
		     markoffset(&mark)<endline && regexec(searchre, &mark, bol);
		     marksetoffset(&mark, searchre->endp[0]), bol = ElvFalse)
		{
			/* highlight any chars that are in the match */
			for (j = line->startrow * win->di->columns;
			     j < endrow;
			     j++)
			{
				if (win->di->offsets[j] >= searchre->startp[0]
				 && win->di->offsets[j] < searchre->endp[0])
				{
					plainfont = win->di->newfont[j];
					if (!plainfont)
						plainfont = o_hasfocus(win)
							? COLOR_FONT_NORMAL
							: COLOR_FONT_IDLE;
					win->di->newfont[j] =
						colortmp(plainfont, searchfont);
				}
			}

			/* prevent infinite loops, thanks to H. Merijn Brand */
			if (searchre->startp[0] == searchre->endp[0])
				searchre->endp[0]++;
		}
	}
}

/* This implements the :nohlsearch command, which temporarily unhighlights the
 * hlsearch matches.
 */
RESULT	ex_nohlsearch(xinf)
	EXINFO	*xinf;
{
	searchhlflag = ElvFalse;
	return RESULT_COMPLETE;
}
#endif /* FEATURE_HLSEARCH */

#ifdef FEATURE_INCSEARCH
typedef struct
{
	WINDOW	win;		/* window where the command is executing */
	MARK	from;		/* starting point of the search */
	long	rightward;	/* distance to \= point */
	ELVBOOL	quote;
	ELVBOOL	success;	/* did the search succeed? */
	CHAR	dir;		/* '/' for forward, or '?' for backward */
	CHAR	text[115];	/* the search regexp so far, as text */
} INCINFO;


static RESULT incparse(key, info)
	_CHAR_	key;	/* input keystroke */
	void	*info;	/* status of incsearch so far */
{
	INCINFO	*ii = (INCINFO *)info;
	int	len = CHARlen(ii->text);
	ELVBOOL	wxflags;
	ELVBOOL	oldhide;
	RESULT	rc;
	CHAR	*cp, *retext, *build, *scan;

	/* add this keystroke into the regexp string */
	if (ii->quote)
	{
		/* after ^V, any char is added to searchre like normal text */
		ii->quote = ElvFalse;
		if (key == '\n')
			key = '\r';
		if (len < QTY(ii->text) - 2)
			ii->text[len] = key;
		else
			guibeep(ii->win);
	}
	else
	{
		switch (key)
		{
		  case '\n':
		  case '\r':
			/* If the regexp is empty, then search now, using the
			 * previous regexp.
			 */
			if (!ii->text[0])
			{
				key = '\n';
				break;
			}

			/* Otherwise we've already searched */
			return RESULT_COMPLETE;

		  case '\177':
		  case '\b':
			if (len > 0)
			{
				ii->text[len - 1] = '\0';
				break;
			}
			/* else fall through to act like <Esc> */

		  case '\033': /* ESC */
			/* act like an unsuccessful search */
			ii->success = ElvFalse;
			return RESULT_COMPLETE;

		  case ELVCTRL('V'):
			/* make the next character be quoted */
			ii->quote = ElvTrue;
			msg(MSG_STATUS, "[CS]$1(list($2))^",
				ii->dir, ii->text);
			return RESULT_MORE;

		  default:
			if (len < QTY(ii->text) - 2)
				ii->text[len] = key;
			else
				guibeep(ii->win);
		}
	}

	/* try to compile the regexp.  Be silent about errors, since the
	 * user may be in the middle of entering a more complex regexp.
	 */
	if (key == '\n')
	{
		/* Reuse previous search expression.  Complain if none */
		if (!searchre)
		{
			msg(MSG_ERROR, "no previous search");
			return RESULT_ERROR;
		}
	}
	else if (!ii->text[0])
	{
		/* if empty regexp so far, then don't search yet */
		msg(MSG_STATUS, "[CS]$1(list($2))", ii->dir, ii->text);
		searchre = NULL;
	}
	else /* non-empty regexp, to be compiled */
	{
		/* Build a copy of the regexp.  We do this for two reasons:
		 * regbuild() is responsible for performing Perl-style
		 * variable interpolation, and we want to find the ending
		 * delimiter so we can parse any flags that appear after it.
		 */
		scanstring(&cp, ii->text);
		retext = regbuild(ii->dir, &cp, ElvTrue);

		/* Parse any flags.  Incremental searching doesn't support
		 * quite as many flags (yet) as normal searching, but the
		 * ones that it does support are compatible with normal search.
		 *
		 * Note that we must parse the flags before compiling the
		 * regexp, because some of these flags affect regcomp().
		 */
		autoselect = o_autoselect;
		searchhasdelta = ElvFalse;
		wxflags = ElvFalse;
		for (; cp && *cp; cp && scannext(&cp))
		{
			switch (*cp)
			{
			  case 'v':
				autoselect = ElvTrue;
				break;

			  case 'n':
				autoselect = ElvFalse;
				break;

			  case 'i':
				o_ignorecase = ElvTrue;
				o_smartcase = ElvFalse;
				break;

			  case 'c':
				o_ignorecase = ElvFalse;
				break;

			  case 's':
				o_ignorecase = ElvTrue;
				o_smartcase = ElvTrue;
				break;

			  case 'w':
			  case 'x':
				if (wxflags)
				{
					msg(MSG_STATUS, "[CS]$1$2  excessive 'w' or 'x' flags",
						ii->dir, ii->text);
					scanfree(&cp);
					safefree(retext);
					return RESULT_MORE;
				}
				wxflags = ElvTrue;
				if (!*retext)
				{
					msg(MSG_STATUS, "[C]$1$1  can't use 'w' or 'x' flags with an empty regexp");
					scanfree(&cp);
					safefree(retext);
					return RESULT_MORE;
				}
				build = NULL;
				buildstr(&build, *cp=='w' ? "\\<" : "^");
				for (scan = retext; *scan; scan++)
					buildCHAR(&build, *scan);
				buildstr(&build, *cp=='w' ? "\\>" : "$");
				safefree(retext);
				retext = build;
				break;

			  case '+':
			  case '-':
				msg(MSG_STATUS, "[CS]$1$2  incsearch doesn't support line deltas",
					ii->dir, ii->text);
				scanfree(&cp);
				safefree(retext);
				return RESULT_MORE;

			  default:
				msg(MSG_ERROR, "[CSC]$1$2  unknown flag $3",
					ii->dir, ii->text, *cp);
				scanfree(&cp);
				safefree(retext);
				return RESULT_MORE;
			}
		}
		scanfree(&cp);

		/* compile the regexp */
		oldhide = msghide(ElvTrue);
		if (searchre)
			safefree(searchre);
		searchre = regcomp(retext, ii->from);
		safefree(retext);
		(void)msghide(oldhide);
		if (!searchre)
		{
			/* show the search expression as a status message */
			msg(MSG_STATUS, "[CS]$1(list($2))  incomplete regexp", ii->dir, ii->text);
			return RESULT_MORE;
		}

	}

	/* if more keystrokes are in the type-ahead queue, then don't bother
	 * to search yet unless this keystroke is <Enter>.
	 */
	if (key != '\n' && mapbusy())
	{
		return RESULT_MORE;
	}

	/* remove old highlighting, if any, and move cursor back to origin */
	if (ii->win->seltop)
	{
		markfree(ii->win->seltop);
		markfree(ii->win->selbottom);
		ii->win->seltop = ii->win->selbottom = NULL;
		ii->win->di->logic = DRAW_CHANGED;
	}
	marksetoffset(ii->win->cursor, markoffset(ii->from));

	/* if empty regexp, then don't search */
	if (!searchre)
		return RESULT_MORE;

	/* Search for the regexp.  If not found, then beep but remain in
	 * IncSrch mode, so the user can correct the search expression.
	 */
	oldhide = msghide(ElvTrue);
	searchforward = (ELVBOOL)(ii->dir == '/');
	if (searchforward)
		rc = forward(ii->win);
	else
		rc = backward(ii->win);
	(void)msghide(oldhide);
	if (rc == RESULT_COMPLETE)
	{
		/* highlight the matching text, and move the cursor there */
		if (searchre->startp[0] != searchre->endp[0])
		{
			ii->win->seltop = markalloc(markbuffer(ii->from), searchre->startp[0]);
			ii->win->selbottom = markalloc(markbuffer(ii->from), searchre->endp[0] - 1);
			ii->win->selleft = 0;
			ii->win->selright = INFINITY;
			ii->win->selattop = ElvTrue;
			ii->win->selorigcol = 0;
			ii->win->seltype = 'c';
		}
		marksetoffset(ii->win->cursor, searchre->startp[0]);
		ii->rightward = searchre->leavep - searchre->startp[0];
		assert(ii->rightward >= 0);
		ii->success = ElvTrue;

		/* show the search expression as a status message */
		msg(MSG_STATUS, "[CS]$1(list($2))", ii->dir, ii->text);

	}
	else
	{
		ii->success = ElvFalse;
		rc = RESULT_ERROR;

		/* show the search expression as a status message */
		msg(MSG_STATUS, "[CS]$1(list($2))  no match", ii->dir, ii->text);
	}

	/* If the key was <Enter> then we want to return the result of this
	 * search; otherwise we continue allowing the user to edit the regexp.
	 */
	if (key != '\n')
		rc = RESULT_MORE;
	return rc;
}

static RESULT incperform(win)
	WINDOW	win;
{
	INCINFO	*ii = (INCINFO *)win->state->info;

	/* turn off highlighting, unless search was successful and "autoselect"
	 * is set.
	 */
	if (win->seltop && (!ii->success || !autoselect))
	{
		markfree(ii->win->seltop);
		markfree(ii->win->selbottom);
		ii->win->seltop = ii->win->selbottom = NULL;
		ii->win->di->logic = DRAW_CHANGED;
	}

	/* if search was unsuccessful, then move back to original spot */
	if (!ii->success)
		marksetoffset(win->cursor, markoffset(ii->from));
	else
	{
		/* adjust cursor -- may be needed if \= appeared in regexp */
		markaddoffset(win->cursor, ii->rightward);

		/* don't leave cursor on '\n', unless line is empty */
		if (markoffset(win->cursor) > 0 && scanchar(win->cursor)=='\n')
		{
			markaddoffset(win->cursor, -1L);
			if (scanchar(win->cursor) == '\n')
				markaddoffset(win->cursor, 1L);
		}

		/* Adjust the window's "wantcol" value */
		win->wantcol = dispmark2col(win);
	}

	/* free the "from" mark */
	markfree(ii->from);

	/* hide the prompt on the status line */
	if (ii->success)
	{
		if (win->di->drawstate == DRAW_VISUAL)
		{
			drawmsg(win, MSG_STATUS, toCHAR(""), 0);
			win->di->newmsg = ElvFalse;
		}
	}
	else
		msg(MSG_ERROR, "no match");

	/* leave the search variables set according to this search */
	searchforward = (ELVBOOL)(ii->dir == '/');
	searchhasdelta = ElvFalse;

	return ii->success ? RESULT_COMPLETE : RESULT_ERROR;
}

static ELVCURSOR incshape(win)
	WINDOW	win;
{
	return CURSOR_COMMAND;
}

static RESULT incsearch(win, vinf)
	WINDOW	win;	/* window where searching is to take place */
	VIINFO	*vinf;	/* the / or ? command that started the search */
{
	INCINFO	*ii;

	assert(o_incsearch && (vinf->command == '/' || vinf->command == '?'));

	/* push the state */
	statepush(win, ELVIS_MORE|ELVIS_FREE|ELVIS_ALERT|ELVIS_ONCE);

	/* initialize the state */
	win->state->parse = incparse;
	win->state->perform = incperform;
	win->state->shape = incshape;
	win->state->info = ii = safealloc(1, sizeof (INCINFO));
	win->state->mapflags = (MAPFLAGS)0;
	win->state->modename = "IncSrch";

	/* initialize the INCINFO */
	ii->win = win;
	ii->from = markdup(win->cursor);
	ii->quote = ElvFalse;
	ii->rightward = 0L;
	ii->success = ElvFalse;
	ii->dir = vinf->command;
	ii->text[0] = '\0';

	/* show the prompt as a status message */
	msg(MSG_STATUS, "[CS]$1$2", ii->dir, ii->text);

	return RESULT_COMPLETE;
}
#endif /* FEATURE_INCSEARCH */
