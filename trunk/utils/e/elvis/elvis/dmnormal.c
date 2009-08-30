/* dmnormal.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_dmnormal[] = "$Id: dmnormal.c,v 2.63 2003/10/17 17:41:23 steve Exp $";
#endif
#include <time.h>

#if USE_PROTOTYPES
static DMINFO *init(WINDOW win);
static void term(DMINFO *info);
static MARK move(WINDOW w, MARK from, long linedelta, long column, ELVBOOL cmd);
static MARK wordmove(MARK cursor, long count, ELVBOOL backward, ELVBOOL whitespace);
static long mark2col(WINDOW w, MARK mark, ELVBOOL cmd);
static MARK setup(WINDOW win, MARK top, long cursor, MARK bottom, DMINFO *info);
static MARK image(WINDOW w, MARK line, DMINFO *info, void (*draw)(CHAR *p, long qty, _char_ font, long offset));
static void indent(WINDOW w, MARK line, long linedelta);
static MARK tagnext(MARK cursor);
# ifdef FEATURE_TAGS
  static CHAR *tagatcursor(WINDOW win, MARK cursor);
  static MARK tagload(CHAR *tagname, MARK from);
# endif
# ifdef FEATURE_LISTCHARS
  static void getlcs(char *name, CHAR **valptr, int *lenptr);
# endif
# ifdef FEATURE_LPR
  static void header (WINDOW w, int pagenum, DMINFO *info,
		void (*draw)(CHAR *p, long qty, _char_ font, long offset));
# endif
#endif

#ifdef FEATURE_LPR
/* Font/color code to for page header when printing */
static int font_header;
#endif

#ifdef FEATURE_FOLD
/* Font/color code to for displaying hidden lines */
static int font_fold;
#endif

static int font_specialkey;
#ifdef FEATURE_LISTCHARS
static int font_extends;
#endif


#ifdef FEATURE_LISTCHARS
static void getlcs(name, valptr, lenptr)
	char	*name;	/* name of field */
	CHAR	**valptr;/* where to store a pointer to the value */
	int	*lenptr; /* where to store the length of the value */
{
	CHAR	*scan;

	/* look for the value */
	*valptr = calcelement(o_listchars, toCHAR(name));

	/* if no value, then return with *valptr=NULL and *lenptr=0 */
	if (*valptr == NULL)
	{
		*lenptr = 0;
		return;
	}

	/* move *valptr past the ':', to the actual start of the value */
	(*valptr)++;

	/* count the length, up to the next ',' or '\0' */
	for (scan = *valptr; *scan && *scan != ','; scan++)
	{
	}
	*lenptr = (int)(scan - (*valptr));
}
#endif /* FEATURE_LISTCHARS */

/* Return the width of a control character, and optionally output it too if a
 * non-NULL "draw" function is supplied.  If a negative offset is given, then
 * parse the listchars option's value before doing any of this; and if it finds
 * any errors while parsing it will return -1.
 *
 * This is called...
 *  - when drawing a control character with "list" turned on.
 *  - when drawing trailing spaces with "list", col=number of spaces to draw.
 *  - at the end of the line with "list"
 *  - from draw.c with ch='<' or '>' with "wrap" off, with special draw func
 *  - from optglob.c when the listchars option is initialized, with offset=-1
 *  - from optglob.c when the listchars option is changed, with offset=-1
 *
 * This function doesn't test the "list" option.  It is assumed that the
 * calling function has already tested it.
 */
int dmnlistchars(ch, offset, col, tabstop, draw)
	_CHAR_	ch;	/* character */
	long	offset;	/* offset of character */
	long	col;	/* column number, or (when ch=' ') number of spaces */
	short	*tabstop;/* tabstop columns -- value of o_tabstop() */
	void	(*draw) P_((CHAR *p, long qty, _char_ font, long offst));
{
	CHAR	caret[3];
	CHAR	*string = NULL;
	int	length, width, i;
	char	font;
#ifdef FEATURE_LISTCHARS
	/* These static pointers are either NULL, or point to a location in
	 * the value of the "listchars" option.
	 */
	static CHAR	*tab, *ff, *cr, *esc, *bs, *del, *nul;
	static CHAR	*eol, *trail, *precedes, *extends;
	static int	ltab, lff, lcr, lesc, lbs, ldel, lnul;
	static int	leol, ltrail, lprecedes, lextends;
	static CHAR	space[1] = {' '};

	/* if offset is negative, then parse the "listchars" option */
	if (offset < 0)
	{
		getlcs("tab", &tab, &ltab);
		getlcs("ff", &ff, &lff);
		getlcs("cr", &cr, &lcr);
		getlcs("esc", &esc, &lesc);
		getlcs("bs", &bs, &lbs);
		getlcs("del", &del, &ldel);
		getlcs("nul", &nul, &lnul);
		getlcs("eol", &eol, &leol);
		getlcs("trail", &trail, &ltrail);
		getlcs("precedes", &precedes, &lprecedes);
		getlcs("extends", &extends, &lextends);
	}

	/* recognize some special characters */
	switch (ch)
	{
	  case '<':	string = precedes; length = lprecedes;	break;
	  case '>':	string = extends; length = lextends;	break;
	  case '\n':	string = eol; length = leol;		break;
	  case ' ':	string = trail; length = ltrail;	break;
	  case '\t':	string = tab; length = ltab;		break;
	  case '\f':	string = ff; length = lff;		break;
	  case '\r':	string = cr; length = lcr;		break;
	  case '\0':	string = nul; length = lnul;		break;
	  case '\b':	string = bs; length = lbs;		break;
	  case '\033':	string = esc; length = lesc;		break;
	  case '\177':	string = del; length = ldel;		break;
	}
#endif

	/* no special strings -- use default */
	if (!string)
	{
		switch (ch)
		{
#ifdef FEATURE_LISTCHARS
		  case '<':
		  case '>':	string = caret; length = 0;		break;
		  case ' ':	string = space; length = 1;		break;
		  case '\n':	string = caret; length = 0;		break;
#else
		  case '\n':	caret[0] = '$'; string=caret; length=1;	break;
#endif
		  default:
			caret[0] = '^';
			caret[1] = ch ^ '@';
			string = caret;
			length = 2;
		}
	}

	/* if that's all we care about, then return the width now */
	if (!draw)
	{
#ifdef FEATURE_LISTCHARS
		if (ch == ' ')
			return (int)col;
		else if (string == caret || ch != '\t')
			return length;
		else /* ch == '\t' */
			return opt_totab(tabstop, col);
#else
		return length;
#endif
	}

	/* choose a font */
	if (ch == '\n')
		font = COLOR_FONT_NONTEXT;
#ifdef FEATURE_LISTCHARS
	else if (ch == '<' || ch == '>')
		font = font_extends;
#endif
	else
		font = font_specialkey;

	/* draw the character.  This is tricky for tabs and newlines */
	switch (ch)
	{
	  case '\n':
		for (i = 0; i < length; i++)
			(*draw)(&string[i], -1L, font, offset);
		width = length;
		break;

#ifdef FEATURE_LISTCHARS
	  case ' ':
	  case '\t':
		if (string != caret)
		{
			/* empty string acts like spaces */
			if (!*string)
				string = space;

			/* display the tab character as a bunch of chars */
			width = (ch == '\t') ? opt_totab(tabstop, col) : col;
			if (width <= length)
			{
				for (i = 0; i < width; i++)
					(*draw)(&string[i], -1L, font, offset);
			}
			else
			{
				for (i = 0; i < length; i++)
					(*draw)(&string[i], -1L, font, offset);

				/* repeat the last char to fill the tabstop.
				 * Note that (length - width) is negative
				 */
				(*draw)(&string[length - 1], (long)(length - width), font, offset);
			}
			if (i > 0)
			break;
		}
		/* else fall through */
#endif

	  default:
		for (i = 0; i < length; i++)
			(*draw)(&string[i], -1L, font, offset);
		width = length;
	}

	/* return the width */
	return width;
}


/* start the mode, and allocate modeinfo */
static DMINFO *init(win)
	WINDOW	win;
{
#ifdef FEATURE_FOLD
	/* if first time, then find the "fold" font */
	if (!font_fold)
	{
		font_fold = colorfind(toCHAR("fold"));
		colorset(font_fold, toCHAR("bold boxed"), ElvFalse);
	}
#endif

	/* if first time, then find the "specialkey" font */
	if (!font_specialkey)
	{
		font_specialkey = colorfind(toCHAR("specialkey"));
		colorset(font_specialkey, toCHAR("bold boxed"), ElvFalse);
#ifdef FEATURE_LISTCHARS
		font_extends = colorfind(toCHAR("extends"));
		colorset(font_extends, toCHAR("like specialkey boxed"), ElvFalse);
#endif
	}

	return NULL;
}

/* end the mode, and free the modeinfo */
static void term(info)
	DMINFO	*info;	/* window-specific info about mode */
{
}

/* Move vertically, and to a given column (or as close to column as possible) */
static MARK move(w, from, linedelta, column, cmd)
	WINDOW	w;		/* window where buffer is shown */
	MARK	from;		/* old position */
	long	linedelta;	/* change in line number */
	long	column;		/* desired column */
	ELVBOOL	cmd;		/* if ElvTrue, we're in command mode; else input mode */
{
	static MARKBUF	tmp;
	long	col, lnum;
	long	offset;
	CHAR	*cp;

	assert(w != NULL || column == 0);

#ifdef FEATURE_FOLD
	/* if on a folded region, then move relative to first/last line in
	 * that region.
	 */
	if (w && o_folding(w))
	{
		FOLD	fold;

		/* if multi-line move, and any folds are involved, then
		 * recursively move by single lines.  This allows both the
		 * folded and non-folded code to be simpler.
		 */
		if (labs(linedelta) > 1)
		{
			lnum = markline(from) + linedelta;
			if (lnum < 1)
				lnum = 1;
			else if (lnum > o_bufchars(markbuffer(from)))
				lnum = o_bufchars(markbuffer(from));
			tmp = *from;
			marksetline(&tmp, lnum);
			if (RESULT_COMPLETE == foldbyrange(
						linedelta > 0 ? from : &tmp,
						linedelta > 0 ? &tmp : from,
						ElvTrue, FOLD_INSIDE|FOLD_TEST))
			{
				for (lnum = labs(linedelta), linedelta /= lnum;
				     from && lnum > 1;
				     lnum--)
					from = move(w, from, linedelta, 0, cmd);
				return move(w, from, linedelta, column, cmd);
			}
		}

		/* starting within a fold? */
		if ((fold = foldmark(from, ElvTrue)) != NULL)
		{
			/* if moving within the FOLD, handle it here */
			if (linedelta == 0)
			{
				tmp = *fold->from;
				if (column == INFINITY)
					tmp = *fold->to;
				return &tmp;
			}

			/* else moving off the FOLD, do it relative to one end
			 * or the other, depending on direction of travel.
			 */
			if (linedelta < 0)
				marksetoffset(from, markoffset(fold->from));
			else
				marksetoffset(from, markoffset(fold->to));
		}
	}
#endif

	/* move forward/back to the start of the line + linedelta */
	lnum = markline(from) + linedelta;
	if (lnum < 1)
		lnum = 1;
	else if (lnum > o_buflines(markbuffer(from)))
		lnum = o_buflines(markbuffer(from));
	offset = lowline(bufbufinfo(markbuffer(from)), lnum);

	/* now move to the left far enough to find the desired column */
	(void)scanalloc(&cp, marktmp(tmp, markbuffer(from), offset));
	for (col = 0; w && cp && *cp != '\n' && col <= column; offset++, scannext(&cp))
	{
		/* add the width of this character */
		if (*cp == '\t' && (!o_list(w) || w->state->acton))
		{
			col = col + opt_totab(o_tabstop(markbuffer(w->cursor)), col);
		}
		else if (*cp < ' ' || *cp == 127)
		{
			col += (o_list(w) && !w->state->acton)
				? dmnlistchars(*cp, offset, col, o_tabstop(markbuffer(w->cursor)), NULL)
				: 2;
		}
		else
		{
			col++;
		}
	}

	/* The above loop normally exits when we've PASSED the desired column,
	 * so we normally want to back up one character.  Two important
	 * exceptions: In input mode, if we break out of that loop because we
	 * hit '\n' then we want to leave the cursor on the '\n' character.
	 * If we hit the end of the buffer, then we want to leave the cursor
	 * on the last character of the buffer but we also need to be careful
	 * about empty buffers.
	 */
	if (col > 0 && !(!cmd && (!cp || *cp == '\n') && col <= column))
	{
		offset--;
	}

	/* return the mark */
	scanfree(&cp);
	return marktmp(tmp, markbuffer(from), offset);
}


/* Convert a mark to a column number */
static long mark2col(w, mark, cmd)
	WINDOW	w;	/* window where buffer is shown */
	MARK	mark;	/* mark to be converted */
	ELVBOOL	cmd;	/* if ElvTrue, we're in command mode; else input mode */
{
	long	col;
	CHAR	*cp;
	MARK	front;
	long	nchars;

	/* if the buffer is empty, the column must be 0 */
	if (o_bufchars(markbuffer(mark)) == 0)
	{
		return 0;
	}

#ifdef FEATURE_FOLD
	/* if in a folded region, then the column number is always wantcol */
	if (w && o_folding(w) && foldmark(mark, ElvTrue))
		return w->wantcol;
#endif

	/* find the front of the line */
	front = move(w, mark, 0, 0, cmd);
	nchars = markoffset(mark) - markoffset(front);

	/* in command mode, we leave the cursor on the last cell of any
	 * wide characters such as tabs.  To accomplish this, we'll find
	 * the column of the following character, and then subtract 1.
	 */
	if (cmd)
	{
		nchars++;
	}

	/* count character widths until we find the requested mark */
	for (scanalloc(&cp, front), col = 0; cp && nchars > 0; nchars--, scannext(&cp))
	{
		if (*cp == '\t' && (!o_list(w) || w->state->acton))
		{
			col = col + opt_totab(o_tabstop(markbuffer(w->cursor)), col);
		}
		else if (*cp == '\n')
		{
			col++;
		}
		else if (*cp < ' ' || *cp == 127)
		{
			col += (o_list(w) && !w->state->acton)
				? dmnlistchars(*cp, 0L, col, o_tabstop(markbuffer(w->cursor)), NULL)
				: 2;
		}
		else
		{
			col++;
		}
	}
	scanfree(&cp);

	/* the other half of the "cmd" hack */
	if (cmd && col > 0)
	{
		col--;
	}

	return col;
}


/* This function implements most of the logic for the visual <b>, <e>, and
 * <w> commands.  If it succedes, it adjusts the starting mark and returns
 * it; if it fails, it returns NULL and leaves the starting mark unchanged.
 */
static MARK wordmove(cursor, count, backward, whitespace)
	MARK	cursor;		/* starting position */
	long	count;		/* number of words to move by */
	ELVBOOL	backward;	/* if ElvTrue, move backward; else forward */
	ELVBOOL	whitespace;	/* if ElvTrue, trailing whitespace is included */
{
	ELVBOOL inword, inpunct;
	CHAR	*cp;
	long	offset;
	long	end;

	/* start the scan */
	scanalloc(&cp, cursor);
	offset = markoffset(cursor);
	assert(cp != NULL);
	end = o_bufchars(markbuffer(cursor));

	/* figure out if we're in the middle of a word */
	if (backward || !whitespace || elvspace(*cp))
	{
		inword = inpunct = ElvFalse;
	}
	else if (elvalnum(*cp) || *cp == '_')
	{
		inword = ElvTrue;
		inpunct = ElvFalse;
	}
	else
	{
		inword = ElvFalse;
		inpunct = ElvTrue;
	}

	/* continue... */
	if (backward)
	{
		/* move backward until we hit the top of the buffer, or
		 * the start of the desired word.
		 */
		while (count > 0 && offset > 0)
		{
			scanprev(&cp);
			assert(cp != NULL);
			if (elvspace(*cp))
			{
				if (inword || inpunct)
				{
					count--;
				}
				inpunct = inword = ElvFalse;
			}
			else if (elvalnum(*cp) || *cp == '_')
			{
				if (inpunct)
				{
					count--;
				}
				inword = ElvTrue;
				inpunct = ElvFalse;
			}
			else
			{
				if (inword)
				{
					count--;
				}
				inword = ElvFalse;
				inpunct = ElvTrue;
			}
			if (count > 0)
			{
				offset--;
			}
		}

		/* if we hit offset==0 and were in a word, we found the start
		 * of the first word.  Count it here.
		 */
		if (offset == 0 && (inword || inpunct))
		{
			count--;
		}
	}
	else
	{
		/* move forward until we hit the end of the buffer, or
		 * the start of the desired word.
		 */
		while (count > 0 && offset < end - 1)
		{
			scannext(&cp);
			assert(cp != NULL);
			if (elvspace(*cp))
			{
				if ((inword || inpunct) && !whitespace)
				{
					count--;
				}
				inword = inpunct = ElvFalse;
				if (count > 0)
				{
					offset++;
				}
			}
			else if (elvalnum(*cp) || *cp == '_')
			{
				if ((!inword && whitespace) || inpunct)
				{
					count--;
				}
				inword = ElvTrue;
				inpunct = ElvFalse;
				if (count > 0 || whitespace)
				{
					offset++;
				}
			}
			else
			{
				if ((!inpunct && whitespace) || inword)
				{
					count--;
				}
				inword = ElvFalse;
				inpunct = ElvTrue;
				if (count > 0 || whitespace)
				{
					offset++;
				}
			}
		}
	}

	/* cleanup */
	scanfree(&cp);

	/* If we were moving forward and had only one more word to go, then
	 * move the cursor to the last character in the buffer -- usually a
	 * newline character.  Pretend the count was decremented to 0.
	 */
	if (count == 1 && !backward && whitespace && end > 0)
	{
		offset = end - 1;
		count = 0;
	}

	/* if the count didn't reach 0, we failed */
	if (count > 0)
	{
		return NULL;
	}

	/* else set the cursor's offset */
	assert(offset < end && offset >= 0);
	marksetoffset(cursor, offset);
	return cursor;
}

/* Choose a line to appear at the top of the screen, and return its mark.
 * Also, initialize the info for the next line.
 */
static MARK setup(win, top, cursor, bottom, info)
	WINDOW	win;	/* window to be updated */
	MARK	top;	/* where previous image started */
	long	cursor;	/* offset of cursor position */
	MARK	bottom;	/* where previous image ended */
	DMINFO	*info;	/* window-specific info about mode */
{
	static MARKBUF tmp;
	long	topoff;
	long	bottomoff;
	long	other;
	long	i;

	/* if the cursor is still on the screen (or very near the bottom)
	 * then use the same top.
	 */
	topoff = markoffset(move((WINDOW)0, top, 0, 0, ElvTrue));
	bottomoff = markoffset(move((WINDOW)0, bottom, o_nearscroll, 0, ElvTrue));
	if (cursor >= topoff && (cursor < bottomoff || bottomoff < markoffset(bottom) + 1))
	{
		return marktmp(tmp, markbuffer(top), topoff);
	}

	/* if the cursor is on the line before the top, then scroll back */
	if (topoff > 0)
	{
		for (i = 1; i < o_nearscroll; i++)
		{
			other = markoffset(move((WINDOW)0, top, -i, 0, ElvTrue));
			if (cursor >= other && cursor < topoff)
			{
				return marktmp(tmp, markbuffer(top), other);
			}
		}
	}

	/* else try to center the line in the window */
	other = cursor - (bottomoff - topoff) / 2;
	if (other < 0)
	{
		other = 0;
	}
	other = markoffset(move((WINDOW)0, marktmp(tmp, markbuffer(top), other), 0, 0, ElvTrue));
	win->di->logic=DRAW_CENTER;
	return marktmp(tmp, markbuffer(top), other);
}

static MARK image(w, line, info, draw)
	WINDOW	w;		/* window where drawing will go */
	MARK	line;		/* start of line to draw next */
	DMINFO	*info;		/* window-specific info about mode */
	void	(*draw)P_((CHAR *p, long qty, _char_ font, long offset));
				/* function for drawing a single character */
{
	int	col;
	CHAR	*cp;
	CHAR	tmpchar;
	long	offset;
	static MARKBUF tmp;
	long	startoffset;	/* offset of first contiguous normal char */
	int	qty;		/* number of contiguous normal chars */
	CHAR	buf[100];	/* buffer, holds the contiguous normal chars */
	int	i;
#ifdef FEATURE_LISTCHARS
	ELVBOOL hastrail;	/* can highlight trailing spaces */
#endif

#ifdef FEATURE_FOLD
	FOLD	fold;

	/* if in a fold, then display the fold's name */
	if (o_folding(w) && (fold = foldmark(line, ElvTrue)) != NULL)
	{
		/* output whitespace from the front of the first line */
		for (scanalloc(&cp, fold->from), col = 0, startoffset = markoffset(fold->from);
		     cp && (*cp == ' ' || *cp == '\t');
		     scannext(&cp), startoffset++)
		{
			if (*cp == ' ')
			{
				(*draw)(blanks, 1, 0, startoffset);
				col++;
			}
			else
			{
				i = opt_totab(o_tabstop(markbuffer(w->cursor)), col);
				(*draw)(blanks, -i, 0, startoffset);
				col += i;
			}
		}
		scanfree(&cp);

		/* output the name of the fold */
		(*draw)(fold->name, CHARlen(fold->name), font_fold, startoffset);

		/* output a newline */
		(*draw)(toCHAR("\n"), 1, font_fold, markoffset(fold->to));

		/* next line starts after the fold */
		(void) marktmp(tmp, markbuffer(fold->to), markoffset(fold->to) + 1);
		return &tmp;
	}
#endif

#ifdef FEATURE_LISTCHARS
	hastrail = (ELVBOOL)(dmnlistchars(' ', -1L, 1L, NULL, NULL) > 0);
#endif

	/* initialize startoffset just to silence a compiler warning */
	startoffset = 0;

	/* for each character in the line... */
	qty = 0;
	for (col = 0, offset = markoffset(line), scanalloc(&cp, line);
	     cp && *cp != '\n';
	     offset++, scannext(&cp))
	{
		/* some characters are handled specially */
		if (*cp == '\f' && markoffset(w->cursor) == o_bufchars(markbuffer(w->cursor)))
		{
			/* when printing, a formfeed ends the line (and page) */
			break;
		}
		else if (*cp == '\t' && (!o_list(w) || w->state->acton))
		{
			/* output any preceding normal characters */
			if (qty > 0)
			{
				(*draw)(buf, qty, 0, startoffset);
				qty = 0;
			}

			/* display the tab character as a bunch of spaces */
			i = opt_totab(o_tabstop(markbuffer(w->cursor)), col);
			tmpchar = ' ';
			(*draw)(&tmpchar, -i, 0, offset);
			col += i;
		}
		else if (*cp < ' ' || *cp == 127)
		{
			/* output any preceding normal characters */
			if (qty > 0)
			{
				(*draw)(buf, qty, 0, startoffset);
				qty = 0;
			}

			/* control characters */
			if (o_list(w) && !w->state->acton)
				col += dmnlistchars(*cp, offset, col, o_tabstop(markbuffer(w->cursor)), draw);
			else
			{
				tmpchar = '^';
				(*draw)(&tmpchar, 1, font_specialkey, offset);
				tmpchar = *cp ^ 0x40;
				(*draw)(&tmpchar, 1, font_specialkey, offset);
				col += 2;
			}
		}
		else
		{
			/* starting a new string of contiguous normal chars? */
			if (qty == 0)
			{
				startoffset = offset;
			}

			/* add this char to the string */
			buf[qty++] = *cp;
			col++;

			/* if buf[] is full, flush it now */
			if (qty == QTY(buf))
			{
				(*draw)(buf, qty, 0, startoffset);
				qty = 0;
			}
		}
	}

	/* output any normal chars from the end of the line */
	if (qty > 0)
	{
#ifdef FEATURE_LISTCHARS
		if (o_list(w) && hastrail && w->state->acton == NULL && markoffset(w->state->cursor) != startoffset + qty)
		{
			/* check for trailing spaces */
			for (i = qty; --i >= 0 && buf[i] == ' '; )
			{
			}
			i++;

			/* output the non-space trailing chars */
			if (i > 0)
			{
				(*draw)(buf, (long)i, 0, startoffset);
				qty -= i;
				startoffset += i;
			}

			/* output any trailing spaces */
			if (qty > 0)
				(void)dmnlistchars(' ', startoffset, qty, NULL, draw);
		}
		else
#endif
			(*draw)(buf, qty, 0, startoffset);
		qty = 0;
	}

	/* end the line */
	if (o_list(w) && !w->state->acton && *cp == '\n')
	{
		col += dmnlistchars(*cp, offset, col, o_tabstop(markbuffer(w->cursor)), draw);
	}
	(*draw)(cp ? cp : toCHAR("\n"), 1, 0, offset);
	if (cp)
	{
		offset++;
	}
	else
	{
		offset = o_bufchars(markbuffer(w->cursor));
	}
	scanfree(&cp);
	return marktmp(tmp, markbuffer(w->cursor), offset);
}

/* This function implements autoindent.  Given the MARK of a newly created
 * line, insert a copy of the indentation from another line.  The line whose
 * indentation is to be copied is specified as a line delta.  Usually, this
 * will be -1 so the new line has the same indentation as a previous line.
 * The <Shift-O> command uses a linedelta of 1 so the new line will have the
 * same indentation as the following line.
 */
static void indent(w, line, linedelta)
	WINDOW	w;		/* windows whose options are used */
	MARK	line;		/* new line to adjust */
	long	linedelta;	/* -1 to copy from previous line, etc. */
{
	MARKBUF	from, to;	/* bounds of whitespace in source line */
	MARKBUF	bline;		/* copy of the "line" argument */
	CHAR	*cp;		/* used for scanning whitespace */
	ELVBOOL	srcblank;	/* is source indentation from a blank line? */

	assert(o_autoindent(markbuffer(line)));
	assert(markbuffer(w->cursor) == markbuffer(line));
	/*assert(scanchar(w->cursor) == '\n');*/

	/* find the start of the source line */
	bline = *line;
	from = *dispmove(w, linedelta, 0);
	if (markoffset(&from) == markoffset(&bline))
	{
		/* can't find source line -- at edge of buffer, maybe? */
		return;
	}

	/* find the end of the source line's whitespace */
	for (scanalloc(&cp, &from); cp && (*cp == ' ' || *cp == '\t'); scannext(&cp))
	{
	}
	if (!cp)
	{
		/* hit end of buffer without finding end of line -- do nothing */
		scanfree(&cp);
		return;
	}
	to = *scanmark(&cp);
	srcblank = (ELVBOOL)(*cp == '\n');
	scanfree(&cp);

	if (markoffset(&from) != markoffset(&to))
	{
		/* copy the source whitespace into the new line */
		bufpaste(&bline, &from, &to);
	
		/* if the source line was blank, then delete its whitespace */
		if (srcblank)
		{
			if (linedelta > 0L)
			{
				/* tweak from & to, due to bufpaste() */
				markaddoffset(&to, markoffset(&to) - markoffset(&from));
				marksetoffset(&to, markoffset(&from));
			}
			else
			{
				/* tweak bline for the following bufreplace() */
				markaddoffset(&bline, markoffset(&from) - markoffset(&to));
			}
			bufreplace(&from, &to, NULL, 0L);
		}

		/* tweak the argument mark, to leave cursor after whitespace */
		marksetoffset(line, markoffset(&bline) + markoffset(&to) - markoffset(&from));
		bline = *line;
	}

	/* if the line had some other indentation before, then delete that */
	for (scanalloc(&cp, line); cp && (*cp == ' ' || *cp == '\t'); scannext(&cp))
	{
	}
	if (cp)
	{
		to = *scanmark(&cp);
		scanfree(&cp);
		bufreplace(&bline, &to, NULL, 0);
	}
	else
		scanfree(&cp);
}


#ifdef FEATURE_TAGS
/* Return a dynamically-allocated string containing the name of the tag at the
 * cursor, or NULL if the cursor isn't on a tag.
 */
static CHAR *tagatcursor(win, cursor)
	WINDOW	win;
	MARK	cursor;
{
	MARKBUF	curscopy;	/* a copy of the cursor */
	MARK	word;		/* mark for the front of the word */

	/* find the ends of the word */
	curscopy = *cursor;
	word = wordatcursor(&curscopy, ElvFalse);

	/* if not on a word, then return NULL */
	if (!word)
		return NULL;

	/* copy the word into RAM, and return it */
	return bufmemory(word, &curscopy);
}


/* Lookup a tag name, load the file where that tag was defined, and return
 * the MARK of its position within that buffer.  If the tag can't be found
 * or loaded for any reason, then issue an error message and return NULL.
 */
static MARK tagload(tagname, from)
	CHAR	*tagname;	/* name of tag to lookup */
	MARK	from;		/* initial position of the cursor */
{
 static MARKBUF	retmark;	/* the return value */
	MARKBUF	linemark;	/* start of line */
	BUFFER	buf;		/* the buffer containing the tag */
	EXINFO	xinfb;		/* dummy ex command, for parsing tag address */
	ELVBOOL	wasmagic;	/* stores the normal value of o_magic */
	TAG	*tag;		/* the found tag */
	CHAR	*cp, prev;	/* for scanning the line */
	char	*name;		/* for scanning the name */

	/* search for the tag[s] */
	tag = tetag(tagname);
	if (!tag)
		/* tetag() already gave an error message */
		return NULL;

	/* find a buffer containing the file, or load the file into a buffer */
	buf = buffind(toCHAR(tag->TAGFILE));
	if (!buf)
	{
		if (dirperm(tag->TAGFILE) == DIR_NEW)
		{
			msg(MSG_ERROR, "[s]$1 doesn't exist", tag->TAGFILE);
			return NULL;
		}
		buf = bufload(NULL, tag->TAGFILE, ElvFalse);
		if (!buf)
		{
			/* bufload() already gave error message */
			return NULL;
		}
	}
	if (o_bufchars(buf) == 0)
	{
		goto NotFound;
	}

	/* convert the tag address into a line number */
	scanstring(&cp, toCHAR(tag->TAGADDR));
	memset((char *)&xinfb, 0, sizeof xinfb);
	(void)marktmp(xinfb.defaddr, buf, 0);
	wasmagic = o_magic;
	o_magic = ElvFalse;
	if (!exparseaddress(&cp, &xinfb))
	{
		scanfree(&cp);
		o_magic = wasmagic;
		goto NotFound;
	}
	scanfree(&cp);
	o_magic = wasmagic;
	(void)marktmp(retmark, buf, lowline(bufbufinfo(buf), xinfb.to));
	exfree(&xinfb);

	/* search for the tagname within the line */
	linemark = retmark;
	for (prev = '\n', scanalloc(&cp, &linemark), name = NULL;
	     cp && *cp != '\n';
	     prev = *cp, scannext(&cp))
	{
		if (!name
		 && (CHAR)*tag->TAGNAME == *cp
		 && !(elvalnum(prev) || prev == '_'))
		{
			/* starting a new name */
			retmark = *scanmark(&cp);
			name = tag->TAGNAME + 1;
		}
		else if (name && !*name)
		{
			/* ending a name? */
			if (elvalnum(*cp) || *cp == '_')
			{
				/* word in the line is longer than name */
				name = NULL;
			}
			else
			{
				/* found it! */
				break;
			}
		}
		else if (name && (CHAR)*name++ != *cp)
		{
			/* word in line is different than the name */
			name = NULL;
		}
	}
	scanfree(&cp);

	/* if name not found in line, then return the start of line */
	if (!name || *name)
		retmark = linemark;

	return &retmark;

NotFound:
	msg(MSG_WARNING, "tag address out of date");
	return marktmp(retmark, buf, 0L);
}
#endif /* FEATURE_TAGS */

/* The <Tab> key command -- in html it move to the next link, but in normal
 * mode it tries to unfold/refold without moving the cursor.
 */
static MARK tagnext(cursor)
	MARK	cursor;
{
#ifdef FEATURE_FOLD
	ELVBOOL	infold;	/* found a folded FOLD? (else an unfolded one) */

	/* Find a fold from either the "fold" or "unfold" list */
	if (foldmark(cursor, ElvTrue))
		infold = ElvTrue;
	else if (foldmark(cursor, ElvFalse))
		infold = ElvFalse;
	else
		/* no folds here! */
		return NULL;

	/* Move it to the other list. (Can't fail -- we KNOW there's a fold.) */
	(void)foldbyrange(cursor, cursor, infold, FOLD_OUTSIDE|FOLD_TOGGLE);

	/* Return the unchanged cursor */
	return cursor;
#else
	return NULL;
#endif
}


#ifdef FEATURE_LPR
static void header(w, pagenum, info, draw)
	WINDOW	w;	/* window from which we're printing */
	int	pagenum;/* page number */
	DMINFO	*info;	/* drawing state */
	void	(*draw)P_((CHAR *p, long qty, _char_ font, long offset));
{
	CHAR	pg[20];	/* page number, as a text string */
	CHAR	*title;	/* title of the document */
	CHAR	*date;	/* string returned by ctime() */
	int	tlen;	/* length of title string */
	int	plen;	/* length of page# string */
	int	dlen;	/* length of date string */
	long	gap1;	/* width of gap between left side & middle */
	long	gap2;	/* width of gap between middle & right side */
	time_t	now;	/* current time */
	CHAR	space[1];
	CHAR	newline[1];

	space[0] = ' ';
	newline[0] = '\n';

	/* if the "lpheader" option isn't set, then don't bother. */
	if (!o_lpheader)
	{
		return;
	}

	/* if first time, then find font_header */
	if (!font_header)
	{
		font_header = colorfind(toCHAR("header"));
		colorset(font_header, toCHAR("underlined"), ElvFalse);
	}

	/* convert page number to text */
	long2CHAR(pg, (long)pagenum);

	/* find the title of the document */
	title = o_filename(markbuffer(w->cursor));
	if (!title)
		title = o_bufname(markbuffer(w->cursor));

	/* find the date */
	time(&now);
	date = toCHAR(ctime(&now));

	/* find the lengths of things */
	tlen = CHARlen(title);
	plen = CHARlen(pg);
	dlen = CHARlen(date) - 1; /* "- 1" because of terminal '\n' in string */
	gap1 = (o_lpcolumns - plen) / 2 - tlen;
	if (gap1 < 1)
	{
		title -= (gap1 - 1);
		tlen += (gap1 - 1);
		gap1 = 1;
	}
	gap2 = o_lpcolumns - (tlen + gap1 + plen + dlen);
	if (gap2 < 1)
	{
		gap2 = 1;
		dlen = o_lpcolumns - (tlen + gap1 + plen + gap2);
	}

	/* Output the parts of the headings */
	(*draw)(title, tlen, font_header, -2L);
	(*draw)(space, -gap1, font_header, -2L);
	(*draw)(pg, plen, font_header, -2L);
	(*draw)(space, -gap2, font_header, -2L);
	(*draw)(date, dlen, font_header, -2L);

	/* End the header line, and then skip one more line */
	(*draw)(newline, -2L, 0, -2L);
}
#endif /* FEATURE_LPR */


DISPMODE dmnormal =
{
	"normal",
	"Standard vi",
	ElvTrue,/* display generating can be optimized */
	ElvTrue,/* should use normal wordwrap */
	0,	/* no window options */
	NULL,	/* no descriptions of window options */
	0,	/* no global options */
	NULL,	/* no descriptions of global options */
	NULL,	/* no values of global options */
	init,
	term,
	mark2col,
	move,
	wordmove,
	setup,
	image,
#ifdef FEATURE_LPR
	header,
#else
	NULL,	/* no header function, since printing is disabled */
#endif
	indent,
#ifdef FEATURE_TAGS
	tagatcursor,
	tagload,
#else
	NULL,	/* return the tag name at the cursor location */
	NULL,	/* load a given tag */
#endif
	tagnext
};
