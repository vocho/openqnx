/* input.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_input[] = "$Id: input.c 167211 2008-04-24 17:26:56Z sboucher $";
#endif

/* These data types are used to store the parsing state for input mode
 * commands.  This is very simple, since most commands are only one keystroke
 * long. (The only exceptions are that INP_QUOTE is two keystrokes long, and
 * INP_HEX1/INP_HEX2 is three keystrokes long.)
 */
typedef enum {
	INP_NORMAL,	/* a normal character to insert/replace */
	INP_NEWLINE,	/* insert a newline */
	INP_QUOTE,	/* we're in the middle of a ^V sequence */
	INP_HEX1,	/* we're expecting the first of two hex digits */
	INP_HEX2,	/* we're expecting the second of two hex digits */
	INP_DIG1,	/* we're expecting the first of two digraph chars */
	INP_DIG2,	/* we're expecting the second of two digraph chars */
	INP_TAB,	/* ^I - insert a bunch of spaces to look like a tab */
	INP_ONEVI,	/* ^O - execute one vi command */
	INP_MULTIVI,	/* ESC - execute many vi commands */
	INP_BACKSPACE,	/* ^H - backspace one character */
	INP_BACKWORD,	/* ^W - backspace one word */
	INP_BACKLINE,	/* ^U - backspace one line */
	INP_SHIFTL,	/* ^D - reduce indentation */
	INP_SHIFTR,	/* ^T - increase indentation */
	INP_EXPOSE,	/* ^R/^L - redraw the screen */
	INP_PREVIOUS,	/* ^@ - insert a copy of previous text, then exit */
	INP_AGAIN,	/* ^A - insert a copy of previous text, continue */
	INP_PUT,	/* ^P - insert a copy of anonymous cut buffer */
	INP_KEEP	/* ^Z - move rightward keeping chars -- undo a BS */
} INPCMD;
typedef struct
{
	ELVBOOL	setbottom;	/* Set bottom = cursor before drawing cursor? */
	ELVBOOL	replacing;	/* ElvTrue if we're in "replace" mode */
	ELVBOOL	willdo;		/* ElvTrue if we're starting another change */
	ELVBOOL hexlock;	/* ElvTrue if we're in hex lock mode */
	INPCMD	cmd;		/* the command to perform */
	CHAR	arg;		/* argument -- usually a key to insert */
	CHAR	backsp;		/* char backspaced over, or '\0' */
	CHAR	prev;		/* previously input char, or '\0' */
} INPUTINFO;


#if USE_PROTOTYPES
static void cleanup(WINDOW win, ELVBOOL del, ELVBOOL backsp, ELVBOOL yank);
static void addchar(MARK cursor, MARK top, MARK bottom, INPUTINFO *info);
static ELVBOOL tryabbr(WINDOW win, _CHAR_ nextc);
static RESULT perform(WINDOW win);
static RESULT parse(_CHAR_ key, void *info);
static ELVCURSOR shape(WINDOW win);
#endif


/* This function deletes everything between "cursor" and "bottom" of the
 * current state.  This is used, for example, when the user hits <Esc>
 * after using "cw" to change a long word into a short one.  It should be
 * called for the INP_ONEVI command with backsp=ElvFalse, and before INP_MULTIVI
 * backup=ElvTrue.
 */
static void cleanup(win, del, backsp, yank)
	WINDOW	win;	/* window where input took place */
	ELVBOOL	del;	/* if ElvTrue, delete text after the cursor */
	ELVBOOL	backsp;	/* if ElvTrue, move to the left */
	ELVBOOL	yank;	/* if ElvTrue, yank input text into ". buffer */
{
	/* delete the excess in the edited region */
	if (del && markoffset(win->state->cursor) < markoffset(win->state->bottom)
	        && markoffset(win->state->top) < markoffset(win->state->bottom))
	{
		bufreplace(win->state->cursor, win->state->bottom, NULL, 0);
	}
	else
	{
		marksetoffset(win->state->bottom, markoffset(win->state->cursor));
		if (markoffset(win->state->top) > markoffset(win->state->cursor))
			marksetoffset(win->state->top, markoffset(win->state->cursor));
	}
	assert(markoffset(win->state->cursor) == markoffset(win->state->bottom));

	/* save the newly input text in the "previous input" buffer */
	if (yank)
	{
		cutyank('.', win->state->top, win->state->bottom, 'c', 'y');
	}

	/* move the cursor back one character, unless it is already at the
	 * start of a line.
	 */
	if (backsp && markoffset(win->state->cursor) > 0)
	{
		markaddoffset(win->state->cursor, -1);
		if (scanchar(win->state->cursor) == '\n')
		{
			markaddoffset(win->state->cursor, 1);
		}
		marksetoffset(win->state->top, markoffset(win->state->cursor));
		marksetoffset(win->state->bottom, markoffset(win->state->cursor));
	}

	/* Force the screen to be regenerated */
	if (win->di->logic == DRAW_NORMAL)
		win->di->logic = DRAW_CHANGED;
}

/* This function inserts/replaces a single character in a buffer, and
 * advances the cursor and (if necessary) bottom mark.
 */
static void addchar(cursor, top, bottom, info)
	MARK		cursor;	/* where to add a character */
	MARK		top;	/* start of edit bounds */
	MARK		bottom;	/* end of edit bounds */
	INPUTINFO	*info;	/* other info, including char to be inserted */
{
	MARKBUF	replaced;

	/* decide whether to insert or replace */
	replaced = *cursor;
	if (markoffset(cursor) < markoffset(bottom))
	{
		replaced.offset++;
	}
	else if (info->replacing && markoffset(cursor) < o_bufchars(markbuffer(cursor)))
	{
		if (scanchar(cursor) != '\n')
		{
			replaced.offset++;
		}
	}

	/* do it */
	bufreplace(cursor, &replaced, &info->arg, 1);

	/* we need to advance the cursor, and maybe bottom */
	markaddoffset(cursor, 1);
	if (markoffset(cursor) > markoffset(bottom))
	{
		marksetoffset(bottom, markoffset(cursor));
	}
}


/* This function attempts to expand an abbreviation at the cursor location,
 * if there is one.  If so, it deletes the short form, and pushes the long
 * form and following character back onto the type-ahead buffer.  Else it
 * returns ElvFalse.
 */
static ELVBOOL tryabbr(win, nextc)
	WINDOW	win;	/* window where abbreviation may need expanding */
	_CHAR_	nextc;	/* character after the word */
{
	MARKBUF	from, to;
	CHAR	*cp, *build;
	long	slen, llen, tlen;
	CHAR	cbuf[1];


#if 0 /* this would prevent abbrs from being expanded on command lines */
	if((win->state->flags & (ELVIS_POP|ELVIS_ONCE|ELVIS_1LINE)) ==
		 (ELVIS_POP|ELVIS_ONCE|ELVIS_1LINE)) { /* nishi */
	    return ElvFalse;
	}
#endif

	/* Try to do an abbreviation.  To do this, we collect
	 * characters backward to the preceding whitespace.  We
	 * go to the preceding whitespace because abbreviations
	 * can't contain whitespace; we know we'll never need
	 * more characters to recognize an abbreviation.  We
	 * collect the characters backwards just because it is
	 * easier.
	 */
	for (scanalloc(&cp, win->state->cursor), build = NULL;
	     cp && scanprev(&cp) && !elvspace(*cp);
	     )
	{
		buildCHAR(&build, *cp);
	}
	scanfree(&cp);
	if (build)
	{
		cp = mapabbr(build, &slen, &llen, (ELVBOOL)(win->state->acton != NULL));
		if (cp)
		{
			/* yes, we have an abbreviation! */

			/* count the characters in the long form which appear
			 * before the first control character.
			 */
			for (tlen = 0; tlen < llen && cp[tlen] >= ' '; tlen++)
			{
			}

			/* replace the short form with the plain text part of
			 * the long form.
			 */
			cleanup(win, ElvTrue, ElvFalse, ElvFalse);
			(void)marktmp(from, markbuffer(win->state->cursor), markoffset(win->state->cursor) - slen);
			(void)marktmp(to, markbuffer(win->state->cursor), markoffset(win->state->cursor));
			bufreplace(&from, &to, cp, tlen);
			cp += tlen;
			llen -= tlen;
			markaddoffset(&from, tlen);

			/* handle the final character (space or tab typed by
			 * the user) and any remaining characters from the
			 * long form of the abbreviation.
			 */
			if (nextc)
			{
				cbuf[0] = nextc;
				if (llen > 0)
				{
					/* stuff the long form, and the user's
					 * non-alnum character, into the queue
					 */
					mapunget(cbuf, 1, ElvFalse);
					mapunget(cp, (int)llen, ElvFalse);
				}
				else
				{
					/* insert the final character into the
					 * text
					 */
					bufreplace(&from, &from, cbuf, 1L);
					markaddoffset(&from, 1L);
				}
			}

			/* move cursor to where word goes */
			marksetoffset(win->state->bottom, markoffset(&from));
			marksetoffset(win->state->cursor, markoffset(&from));
			safefree(build);
			return ElvTrue;
		}
		safefree(build);
	}
	return ElvFalse;
}


/* This function performs an input-mode command.  Usually, this will be a
 * character to insert/replace in the text.
 */
static RESULT perform(win)
	WINDOW	win;	/* window where inputting should be done */
{
	STATE	  *state = win->state;
	INPUTINFO *ii = (INPUTINFO *)state->info;
	MARK	  cursor = state->cursor;
	MARK	  tmp;
	MARKBUF	  from, to;
	CHAR	  *cp;
	EXINFO	  xinfb;
	VIINFO	  vinfb;
	long	  col, len;
	ELVBOOL	  oldcb;
#ifdef FEATURE_COMPLETE
#ifdef FEATURE_TAGS
	CHAR	  *end;
#endif
	ELVBOOL	  usetab;
	union
	{
		char	  partial[256];
		CHAR	  full[256];
	}	  name;
	char	  *littlecp, ch, peek;
#endif

	safeinspect();

	/* initially assume there is no matching parenthesis */
#ifdef FEATURE_TEXTOBJ
	win->matchend =
#endif
	win->match = -4;

	/* if cursor has been moved outside the top & bottom, then reset
	 * the top & bottom to match cursor
	 */
	if (markbuffer(state->top) != markbuffer(state->cursor))
	{
		marksetbuffer(state->top, markbuffer(state->cursor));
		marksetoffset(state->top, markoffset(state->cursor));
		marksetbuffer(state->bottom, markbuffer(state->cursor));
		marksetoffset(state->bottom, markoffset(state->cursor));
	}
	else if (markoffset(state->top) > markoffset(state->cursor)
	    || markoffset(state->cursor) > markoffset(state->bottom))
	{
		marksetoffset(state->top, markoffset(state->cursor));
		marksetoffset(state->bottom, markoffset(state->cursor));
	}

	/* if the "smarttab" option is set, and the cursor is in the line's
	 * indentation area, then INP_TAB is treated like INP_SHIFTR.
	 */
	if (o_smarttab && ii->cmd == INP_TAB)
	{
		scanalloc(&cp, state->cursor);
		while (scanprev(&cp) && (*cp == '\t' || *cp == ' '))
		{
		}
		if (!cp || *cp == '\n')
			ii->cmd = INP_SHIFTR;
		scanfree(&cp);
	}

	/* process the keystroke */
	switch (ii->cmd)
	{
	  case INP_NORMAL:
		/* maybe try to do a digraph */
		if (ii->backsp && o_digraph)
		{
			ii->arg = digraph(ii->backsp, ii->arg);
		}

		/* If next character is non-alphanumeric, check for abbrev.
		 * (Note: Since we never expand abbreviations except in the
		 * main buffer or the ex history buffer, we can skip it if
		 * we're editing some other buffer such as regexp history.
		 * Assume only the ex history buffer has inputtab=ex.)
		 */
		if (!elvalnum(ii->arg)
		 && ii->arg != '_'
		 && (state->acton == NULL || o_inputtab(markbuffer(cursor)) == 'e'))
		{
			if (tryabbr(win, ii->arg))
				break;
		}
		/* fall through... */

	  case INP_HEX1:	/* can't happen */
	  case INP_HEX2:
	  case INP_DIG1:	/* can't happen */
	  case INP_DIG2:
	  case INP_QUOTE:
		/* add the character */
		addchar(cursor, state->top, state->bottom, ii);

		/* if it wasn't whitespace, then maybe do wordwrap */
		if (!elvspace(ii->arg)
		 && cursor == win->cursor
		 && win->md->wordwrap
		 && o_textwidth(markbuffer(cursor)) > 0
		 && dispmark2col(win) >= o_textwidth(markbuffer(cursor)))
		{
			/* Figure out where the current word started */
			for (scanalloc(&cp, cursor);
			     cp && scanprev(&cp) && !elvspace(*cp);
			     )
			{
			}
			if (cp)
			{
				to = *scanmark(&cp);
				markaddoffset(&to, 1L);
			}
			else
			{
				to = *dispmove(win, 0L, 0L);
			}

			/* Locate the front of this line.  We won't look past
			 * back before this character.
			 */
			tmp = dispmove(win, 0L, 0L);

			/* scan backward over any whitespace */
			for (len = markoffset(&to) - markoffset(tmp);
			     len > 0 && scanprev(&cp) && elvspace(*cp);
			     len--)
			{
			}
			if (cp)
			{
					from = *scanmark(&cp);
					markaddoffset(&from, 1L);
			}
			assert(cp || len <= 0);
			scanfree(&cp);

			/* We can only do the word wrap stuff if the current
			 * word isn't the line's first word.
			 */
			if (len > 0)
			{
				/* replace the whitespace with a newline */
				bufreplace(&from, &to, toCHAR("\n"), 1L);
				marksetoffset(&to, markoffset(&from) + 1);

				/* handle autoindent */
				dispindent(win, &to, -1L);
			}
		}

		/* remember digraph hints */
		ii->backsp = '\0';
		ii->prev = ii->arg;

		/* maybe delete old text */
		if (calcelement(o_cleantext, toCHAR("input")))
			cleanup(win, ElvTrue, ElvFalse, ElvFalse);

#ifdef DISPLAY_SYNTAX
# ifdef FEATURE_SMARTARGS
		/* Give smartargs a chance */
		dmssmartargs(win);
# endif
#endif

		/* are we supposed to highlight matching chars? */
		if (o_showmatch(win) && !win->state->acton)
		{
#ifdef FEATURE_TEXTOBJ
			/* before doing the usual match, maybe we should make
			 * a special effort for XML tag pairs.
			 */
			if (ii->arg == '>' && o_matchchar
			 && (CHARchr(o_matchchar, 'x') || CHARchr(o_matchchar, 'X')))
			{
				MARKBUF	start, end;

				/* look for a tag pair ending at the cursor */
				from = *cursor;
				assert(markoffset(cursor) > 0);
				markaddoffset(cursor, -1);
				memset((char *)&vinfb, 0, sizeof vinfb);
				vinfb.command = 'a';
				vinfb.key2 = CHARchr(o_matchchar, 'x') ? 'x' : 'X';
				if (vitextobj(win, &vinfb, &start, &end) == RESULT_COMPLETE
				 && markoffset(&end) == markoffset(&from))
				{
					/* the match starts at <tag> */
					win->match = markoffset(&start);
					win->matchend = win->match;

					/* scan for the end of the <tag> */
					scanalloc(&cp, &start);
					while (cp && *cp != '>')
						scannext(&cp);
					if (cp)
					{
						win->matchend = markoffset(scanmark(&cp));

						/* if followed by \n, include that */
						scannext(&cp);
						while (cp && (*cp == ' ' || *cp == '\t'))
							scannext(&cp);
						if (cp && *cp == '\n')
							win->matchend = markoffset(scanmark(&cp));
					}
					scanfree(&cp);

					/* match really ends after that char */
					win->matchend++;
				}
				assert(markbuffer(cursor) == markbuffer(&from));
				*cursor = from;
			}
			else
#endif /* FEATURE_TEXTOBJ */

			/* If the character was a parenthesis then try to
			 * locate its match. [{(
			 */
			if (CHARchr(toCHAR(")}]"), ii->arg))
			{
				from = *cursor;
				assert(markoffset(cursor) > 0);
				markaddoffset(cursor, -1);
				memset((char *)&vinfb, 0, sizeof vinfb);
				vinfb.command = '%';
				if (m_absolute(win, &vinfb) == RESULT_COMPLETE)
				{
					win->match = markoffset(cursor);
#ifdef FEATURE_TEXTOBJ
					win->matchend = win->match + 1;
#endif
				}
				assert(markbuffer(cursor) == markbuffer(&from));
				*cursor = from;
			}
		}
		break;

	  case INP_NEWLINE:
		cleanup(win, ElvTrue, ElvFalse, ElvFalse);
		if (!tryabbr(win, '\n'))
		{
			ii->arg = '\n';
			ii->cmd = INP_NORMAL;
			perform(win);
			dispindent(win, cursor, -1);
		}
		ii->backsp = ii->prev = '\0';
		break;

	  case INP_TAB:
		oldcb = o_completebinary;
		if (!tryabbr(win, '\t'))
		{
			switch (o_inputtab(markbuffer(cursor)))
			{
#ifndef FEATURE_COMPLETE
			  case 'e':
			  case 'f':
			  case 'i':
#endif /* not FEATURE_COMPLETE */
			  case 't':
				/* insert a tab */
				addchar(cursor, state->top, state->bottom, ii);
				break;

			  case 's':
				/* insert enough spaces to look like a tab */
				col = dispmark2col(win);
				ii->arg = ' ';
				do
				{
					addchar(cursor, state->top, state->bottom, ii);
					col++;
				} while (!opt_istab(o_tabstop(markbuffer(cursor)), col));
				break;
#ifdef FEATURE_COMPLETE
			  case 'i':
#ifdef FEATURE_TAGS
				/* try to find a matching tag name */
				cp = tagcomplete(win, cursor);

				/* If no matching tag, or already at end of
				 * complete tag, then insert a literal tab
				 * character, not a space.
				 */
				if (*cp == ' ')
					*cp = '\t';

				/* For other values, delete the trailing
				 * whitespace which would normally be appended.
				 */
				end = CHARchr(cp, ' ');
				if (end)
					*end = '\0';
#else
				cp = toCHAR("\t");
#endif

				/* Now... if we have anything to add, add it */
				if (*cp)
				{
					len = CHARlen(cp);
					col = markoffset(cursor) + len;
					bufreplace(cursor, win->state->bottom, cp, len);
					marksetoffset(cursor, col);
					marksetoffset(win->state->bottom, col);
				}
				break;

			  case 'e':
				/* Assume this is an ex command line, and try
				 * to do something useful.
				 */
				tmp = markdup(cursor);
				(void)marksetline(tmp, markline(cursor));
				cp = excomplete(win, tmp, cursor);
				markfree(tmp);
				if (cp)
				{
					/* excomplete() found something useful,
					 * or returned "\t" to make the <Tab>
					 * be a literal tab character.  Copy
					 * it into the buffer.
					 */
					len = CHARlen(cp);
					col = markoffset(cursor) + len;
					bufreplace(cursor, win->state->bottom, cp, len);
					marksetoffset(cursor, col);
					marksetoffset(win->state->bottom, col);
					break;
				}
				/* else fall through to filename completion */

			  case 'f':
				/* filename completion */

				/* if at start of input, then fail */
				if (markoffset(cursor) == markoffset(state->top))
				{
					guibeep(win);
					break;
				}

				/* locate the previous character */
				tmp = markdup(cursor);
				markaddoffset(tmp, -1);

				/* if previous char can't be in name, fail */
				if (CHARchr(toCHAR("\t\n()<>"), scanchar(tmp)))
				{
					markfree(tmp);
					guibeep(win);
					break;
				}

				/* collect the partial name into char array */
				usetab = (ELVBOOL)!calcelement(o_filenamerules, toCHAR("space"));
				littlecp = &name.partial[QTY(name.partial)];
				*--littlecp = '\0';
				ch = scanchar(tmp);
				markaddoffset(tmp, -1L);
				peek = scanchar(tmp);
				while (markoffset(tmp) > markoffset(state->top)
					&& ch != '\n' && ch && ch != '\t' &&
					(ch != ' ' || usetab || peek == '\\') &&
					(!(ch == '<' || ch == '>' || ch == '(' || ch == ')')
						|| peek == '\\'))
				{
					*--littlecp = ch;
					ch = peek;
					markaddoffset(tmp, -1);
					if (markoffset(tmp) >= 0)
						peek = scanchar(tmp);
				}
				markaddoffset(tmp, 2);

				/* try to expand the filename */
				littlecp = iofilename(littlecp,
					(ch == '(') ? ')' : '\t');
				if (!littlecp)
				{
					markfree(tmp);
					guibeep(win);
					break;
				}

				/* name found -- replace old word with expanded
				 * name.  Note that we need to convert from
				 * char[] to CHAR[], and add backslashes before
				 * certain characters.
				 */
				for (cp = name.full, col = 0;
				     *littlecp != '\0';
				     littlecp++, col++)
				{
					if (usetab)
					{
						if (strchr("<>#%", *littlecp) != NULL
						 || (littlecp[0] == '\\' && !elvalnum(littlecp[1]) && littlecp[1] != '\0'))
							*cp++ = '\\', col++;
						*cp++ = *littlecp;
					}
					else
					{
						if (strchr(" <>#%", *littlecp) != NULL
						 || (littlecp[0] == '\\' && !elvalnum(littlecp[1]) && littlecp[1] != '\0'))
							*cp++ = '\\', col++;
						if (*littlecp == '\t')
							*cp++ = ' ';
						else
							*cp++ = *littlecp;
					}
				}
				*cp = '\0';
				bufreplace(tmp, win->state->bottom, name.full, col);
				marksetoffset(cursor, markoffset(tmp) + col);
				marksetoffset(win->state->bottom, markoffset(cursor));
				markfree(tmp);
#endif /* FEATURE_COMPLETE */
			}
		}
		o_completebinary = oldcb;
		ii->backsp = ii->prev = '\0';
		break;

	  case INP_ONEVI:
		cleanup(win, ElvTrue, ElvFalse, ElvTrue);
		vipush(win, ELVIS_ONCE, NULL);
		ii->backsp = ii->prev = '\0';
		ii->setbottom = ii->willdo = ElvTrue;
		break;

	  case INP_MULTIVI:
		(void)tryabbr(win, '\0');
		cleanup(win, ElvTrue, ElvTrue, ElvTrue);
		win->state->flags |= ELVIS_POP;
		ii->backsp = ii->prev = '\0';

		/* if blank line in autoindent mode, then delete whitespace */
		if (o_autoindent(markbuffer(cursor)))
		{
			for (from = *dispmove(win,0L,0L), scanalloc(&cp, &from);
			     cp && (*cp == ' ' || *cp == '\t');
			     scannext(&cp))
			{
			}
			if (cp && *cp == '\n')
			{
				to = *scanmark(&cp);
				scanfree(&cp);
				if (markoffset(&to) > markoffset(&from) &&
				    markoffset(&to) - 1 == markoffset(cursor))
				{
					bufreplace(&from, &to, NULL, 0L);
				}
			}
			else
			{
				scanfree(&cp);
			}
		}
		break;

	  case INP_BACKSPACE:
		ii->backsp = '\0';
		if (markoffset(win->state->top) < markoffset(cursor))
		{
			/* backspace within the newly typed text */
			markaddoffset(cursor, -1);
			ii->backsp = ii->prev;

			/* maybe clean up */
			if (calcelement(o_cleantext, win->state->acton ? toCHAR("ex") : toCHAR("bs")))
				cleanup(win, ElvTrue, ElvFalse, ElvFalse);
		}
		else if (win->state->acton != NULL
			&& (win->state->flags & ELVIS_1LINE) != 0)
		{
			/* backspace out of an ex command line or regexp line */
			cleanup(win, ElvTrue, ElvTrue, ElvTrue);
			win->state->flags |= ELVIS_POP;
			win->state->pop->flags &= ~(ELVIS_MORE | ELVIS_ONCE);
			if (win->state->pop->perform == _viperform)
			{
				viinitcmd((VIINFO *)win->state->pop->info);
			}
			ii->backsp = '\0';
		}
		else
		{
			/* bump into left edge of new text */
			guibeep(win);
		}
		ii->prev = '\0';
		break;

	  case INP_BACKWORD:
		if (markoffset(win->state->top) < markoffset(cursor))
		{
			/* expect to back up at least one character */
			markaddoffset(cursor, -1);
			scanalloc(&cp, cursor);

			/* if on whitespace, then back up to non-whitespace */
			while (markoffset(win->state->top) < markoffset(win->state->cursor)
			    && elvspace(*cp))
			{
				markaddoffset(cursor, -1);
				scanprev(&cp);
			}

			/* back up to whitespace */
			while (markoffset(win->state->top) < markoffset(win->state->cursor)
			    && !elvspace(*cp))
			{
				markaddoffset(cursor, -1);
				scanprev(&cp);
			}

			/* if we hit whitespace, then leave cursor after it */
			if (elvspace(*cp))
			{
				markaddoffset(cursor, 1);
			}
			scanfree(&cp);

			/* maybe clean up */
			if (calcelement(o_cleantext, win->state->acton ? toCHAR("ex") : toCHAR("bs")))
				cleanup(win, ElvTrue, ElvFalse, ElvFalse);
		}
		else
		{
			guibeep(win);
		}
		ii->backsp = ii->prev = '\0';
		break;

	  case INP_BACKLINE:
		/* find the start of this line, or if the cursor is already
		 * there, then the start of the preceding line.
		 */
		tmp = (*win->md->move)(win, cursor, 0, 0, ElvFalse);
		if (markoffset(tmp) == markoffset(cursor))
		{
			tmp = (*win->md->move)(win, cursor, -1, 0, ElvFalse);
		}

		/* move to either the start of the line or the top of the
		 * edited region, whichever is closer.
		 */
		if (markoffset(tmp) > markoffset(win->state->top))
		{
			marksetoffset(cursor, markoffset(tmp));

			/* maybe clean up */
			if (calcelement(o_cleantext, win->state->acton ? toCHAR("ex") : toCHAR("bs")))
				cleanup(win, ElvTrue, ElvFalse, ElvFalse);
		}
		else if (markoffset(state->top) < markoffset(cursor))
		{
			marksetoffset(cursor, markoffset(state->top));

			/* maybe clean up */
			if (calcelement(o_cleantext, win->state->acton ? toCHAR("ex") : toCHAR("bs")))
				cleanup(win, ElvTrue, ElvFalse, ElvFalse);
		}
		else
		{
			guibeep(win);
		}
		ii->backsp = ii->prev = '\0';
		break;

	  case INP_SHIFTL:
	  case INP_SHIFTR:
		/* delete any text which has been backspaced -- if it happened
		 * to be whitespace then it could confuse the shift commands.
		 */
		cleanup(win, ElvTrue, ElvFalse, ElvFalse);

		/* remember the cursor position, relative to end of buffer */
		len = o_bufchars(markbuffer(cursor)) - markoffset(cursor);
		assert(len >= 0);

		/* build a :<! or :>! ex command */
		memset((char *)&xinfb, 0, sizeof xinfb);
		xinfb.defaddr = *cursor;
		xinfb.from = xinfb.to = markline(cursor);
		xinfb.fromaddr = marktmp(from, markbuffer(cursor),
			lowline(bufbufinfo(markbuffer(cursor)), xinfb.from));
		xinfb.toaddr = marktmp(to, markbuffer(cursor),
			lowline(bufbufinfo(markbuffer(cursor)), xinfb.to + 1));
		xinfb.command = (ii->cmd == INP_SHIFTL) ? EX_SHIFTL : EX_SHIFTR;
		xinfb.multi = 1;
		xinfb.bang = ElvTrue;

		/* execute the command */
		(void)ex_shift(&xinfb);
		ii->backsp = ii->prev = '\0';

		/* restore the cursor position, relative to end of buffer */
		assert(o_bufchars(markbuffer(cursor)) >= len);
		marksetoffset(cursor, o_bufchars(markbuffer(cursor)) - len);
		break;

	  case INP_EXPOSE:
		drawexpose(win, 0, 0, (int)(o_lines(win) - 1), (int)(o_columns(win) - 1));
		break;

	  case INP_PREVIOUS:
	  case INP_AGAIN:
	  case INP_PUT:
		cleanup(win, ElvTrue, ElvFalse, ElvFalse);

		/* Copy the text.  Be careful not to change the "top" mark. */
		from = *state->top;
		tmp = cutput((ii->cmd == INP_PUT ? '1' : '.'),
					win, cursor, ElvFalse, ElvTrue, ElvTrue);
		marksetoffset(state->top, markoffset(&from));

		/* if successful, tweak the "cursor" and "bottom" marks. */
		if (tmp)
		{
			marksetoffset(cursor, markoffset(tmp) + 1);
			marksetoffset(state->bottom, markoffset(cursor));
			if (ii->cmd == INP_PREVIOUS)
			{
				cleanup(win, ElvTrue, ElvTrue, ElvTrue);
				state->flags |= ELVIS_POP;
			}
		}
		ii->backsp = ii->prev = '\0';
		break;

	  case INP_KEEP:
		if (markoffset(cursor) < markoffset(state->bottom))
			markaddoffset(cursor, 1L);
		else
			guibeep(win);
		break;
	}

	/* set wantcol to the cursor's current column */
	win->wantcol = dispmark2col(win);

	/* prepare for next command */
	ii->cmd = ii->hexlock ? INP_HEX1 : INP_NORMAL;

	return RESULT_COMPLETE;
}

/* This function parses a command.  This involves remembering whether we're
 * in the middle of a ^V quoted character, and also recognizing some special
 * characters.
 */
static RESULT parse(key, info)
	_CHAR_	key;	/* next keystroke */
	void	*info;	/* current parsing state */
{
	INPUTINFO *ii = (INPUTINFO *)info;
	CHAR	sym;	/* visible symbol, e.g. '^' after a ^V */

	/* maybe adjust the edit bounds */
	if (ii->setbottom)
	{
		marksetoffset(windefault->state->bottom,
			markoffset(windefault->state->cursor));
		ii->setbottom = ElvFalse;
	}

	/* maybe start a new "undo" level for the next input key */
	if (ii->willdo)
		bufwilldo(windefault->state->cursor, ElvTrue);

	/* parse the input command */
	if (ii->cmd == INP_HEX1 || ii->cmd == INP_HEX2)
	{
		/* convert hex digit from ASCII to binary */
		if (key >= '0' && key <= '9')
		{
			key -= '0';
		}
		else if (key >= 'a' && key <= 'f')
		{
			key -= 'a' - 10;
		}
		else if (key >= 'A' && key <= 'F')
		{
			key -= 'A' - 10;
		}
		else if (key == ELVCTRL('X'))
		{
			/* toggle hexlock */
			if (ii->hexlock)
			{
				ii->hexlock = ElvFalse;
				ii->cmd = INP_NORMAL;
			}
			else
			{
				ii->hexlock = ElvTrue;
				ii->cmd = INP_HEX1;
			}
			return RESULT_MORE;
		}
		else if (key == '\b')
		{
			/* backspace, but don't alter hexlock */
		  	ii->arg = key;
		  	ii->cmd = INP_BACKSPACE;
			return RESULT_COMPLETE;
		}
		else if (ii->hexlock)
		{
			/* any other non-hex character terminates hexlock,
			 * and then gets processed normally.
			 */
			ii->hexlock = ElvFalse;
			ii->cmd = INP_NORMAL;
			return parse(key, info);
		}
		else
		{
			/* this command is invalid; prepare for next command */
			ii->cmd = INP_NORMAL;
			return RESULT_ERROR;
		}

		/* merge into arg */
		if (ii->cmd == INP_HEX1)
		{
			ii->arg = (key << 4);
			ii->cmd = INP_HEX2;
			windefault->state->mapflags |= MAP_DISABLE;
			return RESULT_MORE;
		}
		else
		{
			ii->arg |= key;
			return RESULT_COMPLETE;
		}
	}
	else if (ii->cmd == INP_DIG1 || ii->cmd == INP_DIG2)
	{
		if (ii->cmd == INP_DIG1)
		{
			ii->arg = key;
			ii->cmd = INP_DIG2;
			windefault->state->mapflags |= MAP_DISABLE;
			return RESULT_MORE;
		}
		else
		{
			ii->arg = digraph(ii->arg, key);
			return RESULT_COMPLETE;
		}
	}
	else if (ii->cmd == INP_QUOTE)
	{
		ii->arg = key;
	}
	else
	{
		ii->arg = key;
		sym = '\0';
		switch (key)
		{
		  case ELVCTRL('@'):	ii->cmd = INP_PREVIOUS;		break;
		  case ELVCTRL('A'):	ii->cmd = INP_AGAIN;		break;
		  case ELVCTRL('D'):	ii->cmd = INP_SHIFTL;		break;
		  case '\177': /* usually mapped to "visual x", else... */
		  case ELVCTRL('H'):	ii->cmd = INP_BACKSPACE;	break;
		  case ELVCTRL('I'):	ii->cmd = INP_TAB;		break;
		  case ELVCTRL('J'):	ii->cmd = INP_NEWLINE;		break;
		  case ELVCTRL('K'):	ii->cmd = INP_DIG1, sym = '+';	break;
		  case ELVCTRL('M'):	ii->cmd = INP_NEWLINE;		break;
		  case ELVCTRL('O'):	ii->cmd = INP_ONEVI;		break;
		  case ELVCTRL('P'):	ii->cmd = INP_PUT;		break;
		  case ELVCTRL('R'):	ii->cmd = INP_EXPOSE;		break;
		  case ELVCTRL('T'):	ii->cmd = INP_SHIFTR;		break;
		  case ELVCTRL('U'):	ii->cmd = INP_BACKLINE;		break;
		  case ELVCTRL('V'):	ii->cmd = INP_QUOTE, sym = '^';	break;
		  case ELVCTRL('W'):	ii->cmd = INP_BACKWORD;		break;
		  case ELVCTRL('X'):	ii->cmd = INP_HEX1, sym = '$';	break;
		  case ELVCTRL('Z'):	ii->cmd = INP_KEEP;		break;
		  case ELVCTRL('['):	ii->cmd = INP_MULTIVI;		break;
		  default:		ii->cmd = INP_NORMAL;
		}

		/* ^V, ^X, and ^K require more keystrokes... */
		if (sym)
		{
			/* display a symbol on the screen */
			ii->arg = sym;
			addchar(windefault->state->cursor,
				windefault->state->top,
				windefault->state->bottom, ii);
			markaddoffset(windefault->state->cursor, -1);

			/* disable maps during extra character[s] */
			windefault->state->mapflags |= MAP_DISABLE;
			return RESULT_MORE;
		}
	}

	/* the command is complete */
	return RESULT_COMPLETE;
}

/* This function decides on a cursor shape */
static ELVCURSOR shape(win)
	WINDOW	win;	/* window whose shape should be returned */
{
	STATE	*state = win->state;
	INPUTINFO *info = (INPUTINFO *)state->info;
	MARK	cursor = state->cursor;

	/* if in the middle of ^V, then always CURSOR_QUOTE */
	if (info->cmd == INP_QUOTE)
	{
		state->mapflags |= MAP_DISABLE;
		return CURSOR_QUOTE;
	}

	/* decide whether to insert or replace */
	if (markoffset(state->top) <= markoffset(cursor)
	 && markoffset(cursor) < markoffset(state->bottom))
	{
		if (info->setbottom)
		{
			marksetoffset(state->bottom, markoffset(cursor));
			info->setbottom = ElvFalse;
			bufwilldo(cursor, ElvTrue);
			return CURSOR_INSERT;
		}
		return CURSOR_REPLACE;
	}
	else if (info->replacing && markoffset(cursor) < o_bufchars(markbuffer(cursor)))
	{
		if (scanchar(cursor) != '\n')
		{
			info->setbottom = ElvFalse;
			return CURSOR_REPLACE;
		}
	}
	info->setbottom = ElvFalse;
	return CURSOR_INSERT;
}


/* This function pushes a state onto the state stack, and then initializes it
 * to be either an input or replace state, with the cursor at a given location.
 * The "mode" argument can be 'R' for replace, or anything else for input.
 * The input cursor is "cursor", which should generally the result of a
 * markalloc() or markdup() function call.  Ditto for the top and bottom of
 * the edit region.
 */
void inputpush(win, flags, mode)
	WINDOW		win;	/* window that should be switched to input mode */
	ELVISSTATE	flags;	/* flags describing this state */
	_char_		mode;	/* 'R' for replace, or anything else for insert */
{
	/* push the state */
	flags |= ELVIS_REGION;
	statepush(win, flags);

	/* initialize the state */
	win->state->parse = parse;
	win->state->perform = perform;
	win->state->shape = shape;
	win->state->info = safealloc(1, sizeof (INPUTINFO));
	win->state->mapflags |= MAP_INPUT;
	if (mode == 'R')
	{
		((INPUTINFO *)win->state->info)->replacing = ElvTrue;
		win->state->modename = "Replace";
	}
	else
	{
		((INPUTINFO *)win->state->info)->replacing = ElvFalse;
		win->state->modename = " Input ";
	}
}

/* This function tweaks the most recent "input" or "replace" state.
 * The "mode" can be 't' to toggle "input" to "replace" or vice verse,
 * 'R' to force the mode to be "replace", or anything else to force the
 * mode to be "input".  This function is called by vi mode's perform()
 * function.
 */
void inputtoggle(win, mode)
	WINDOW	win;	/* window to be toggled */
	_char_	mode;	/* 'R' for replace, 't' to toggle, else insert */
{
	STATE	*state;

	/* find the most recent "input" or "replace" state */
	for (state = win->state; state && state->perform != perform; state = state->pop)
	{
	}
	assert(state != NULL);

	/* change the mode */
	switch (mode)
	{
	  case 't':
		((INPUTINFO *)state->info)->replacing = (ELVBOOL)!((INPUTINFO *)state->info)->replacing;
		break;

	  case 'R':
		((INPUTINFO *)state->info)->replacing = ElvTrue;
		break;

	  default:
		((INPUTINFO *)state->info)->replacing = ElvFalse;
	}

	if (((INPUTINFO *)state->info)->replacing)
	{
		state->modename = "Replace";
	}
	else
	{
		state->modename = " Input";
	}
}

/* This function sets the edit boundaries of an "input" state.  If there
 * is no input state on the state stack, then this function will push one.
 * This function is used to implement the <c> operator, among other things.
 */
void inputchange(win, from, to, linemd)
	WINDOW	win;	/* window to be affected */
	MARK	from;	/* new start of edit bounds */
	MARK	to;	/* new end of edit bounds */
	ELVBOOL	linemd;	/* replace old text with a new line? */
{
	MARKBUF	tmp;
	CHAR	ch;
	ELVBOOL	ctrl_o = ElvFalse;	/* JohnW 19/06/96 */

	assert(markbuffer(from) == markbuffer(win->state->cursor));
	assert(markbuffer(from) == markbuffer(to));
	assert(markoffset(from) <= markoffset(to));

	/* Was this command issued via <Control-O> from input mode?
	 * If not, then we'll need to push one.
	 */
	if (!win->state->pop)
	{
		inputpush(win, 0, 'i');
	}
	else
	{
		ctrl_o = ElvTrue;
	}

	/* replace the last char with '$', if there is a last char
	 * and it is on the same line.  If it is on a different line,
	 * then delete the old text.  If from==to, then do nothing.
	 */
	if (markoffset(from) == markoffset(to))
	{
		/* do nothing */
	}
	else if (ctrl_o || /* JohnW 19/06/96 */
		calcelement(o_cleantext, markoffset(to) > markoffset(dispmove(win, 0, INFINITY)) ? toCHAR("long") : toCHAR("short")))
	{
		/* delete the old text */
		if (linemd)
		{
			if (o_autoindent(markbuffer(from)))
			{
				for (ch = scanchar(from);
				     markoffset(from) < markoffset(to) && (ch == ' ' || ch == '\t');
				     markaddoffset(from, 1), ch = scanchar(from))
				{
				}
			}
			bufreplace(from, to, toCHAR("\n"), 1);
		}
		else
		{
			bufreplace(from, to, NULL, 0);
		}
		marksetoffset(to, markoffset(from));
	}
	else
	{
		/* replace the last character with a '$' */
		tmp = *to;
		tmp.offset--;
		bufreplace(&tmp, to, toCHAR("$"), 1);
	}

	/* set the edit boundaries and the cursor */
	marksetbuffer(win->state->top, markbuffer(from));
	marksetbuffer(win->state->bottom, markbuffer(to));
	marksetoffset(win->state->top, markoffset(from));
	marksetoffset(win->state->bottom, markoffset(to));
	marksetoffset(win->state->cursor, markoffset(from));
}


/* This function is called by statekey() when the user hits <Enter>, before
 * calling the stratum's enter() function.  This function deletes extra
 * characters after the cursor, and adjusts the endpoints of the edited
 * region to make them be whole lines.
 */
void inputbeforeenter(win)
	WINDOW	win;	/* window where <Enter> was just pressed */
{
	/* Make sure "win->state->bottom" includes the cursor position */
	if (markoffset(win->state->bottom) < markoffset(win->state->cursor))
	{
		marksetoffset(win->state->bottom, markoffset(win->state->cursor));
	}

	/* Delete stuff from after the cursor */
	cleanup(win, ElvTrue, ElvFalse, ElvFalse);

	/* Attempt to expand an abbreviation at the end of the line */
	(void)tryabbr(win, '\0');

	/* adjust the endpoints of the edited area to be whole lines */
	marksetoffset(win->state->top,
		markoffset((*dmnormal.move)(win, win->state->top, 0, 0, ElvFalse)));
	marksetoffset(win->state->bottom,
		markoffset((*dmnormal.move)(win, win->state->bottom, 0, INFINITY, ElvFalse)));
}
