/* draw.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_draw[] = "$Id: draw.c,v 2.131 2003/10/17 17:41:23 steve Exp $";
#endif

#if defined (GUI_WIN32)
# define SLOPPY_ITALICS
# define isitalic(f)	(colorinfo[(f)&0x7f].da.bits & COLOR_ITALIC)
#else
# undef SLOPPY_ITALICS
#endif

#if defined(GUI_WIN32) || defined(GUI_X11)
# define SLOPPY_BOXED
# define isboxed(f)	((colorinfo[(f)&0x7f].da.bits & COLOR_BOXED) \
			 || (((f) & 0x80) \
				&& (colorinfo[COLOR_FONT_SELECTION].da.bits & COLOR_BOXED)))
#else
# undef SLOPPY_BOXED
#endif

#if USE_PROTOTYPES
static void insimage(CHAR *ch, DRAWATTR *attr, int qty, int extent);
static void delimage(CHAR *ch, char *font, DRAWATTR *attr, int qty, int extent);
static void fillcell(_CHAR_ ch, _char_ font, long offset);
static void drawchar(CHAR *p, long qty, _char_ font, long offset);
static void compareimage(WINDOW win);
static void drawcurrent(DRAWINFO *di, int base, int same, DRAWATTR *da);
static void updateimage(WINDOW win);
static void genlastrow(WINDOW win);
static void opentextline(WINDOW win, CHAR *text, int len);
static void openchar(CHAR *p, long qty, _char_ font, long offset);
static void openmove(WINDOW win, long oldcol, long newcol, CHAR *image, long len);
# ifdef FEATURE_MISC
  static ELVBOOL drawquick(WINDOW win);
# endif
# ifdef FEATURE_LISTCHARS
static void lcsdrawchar(CHAR *p, long qty, _char_ font, long offset);
# endif
#endif

#ifdef FEATURE_LISTCHARS
/* widths of the arrows to indicate side-scrolled text */
static int	lprecedes, lextends;
#endif

#ifdef FEATURE_HLOBJECT

/* This stores the fontcodes for "hlobject1" through "hlobject9" */
static int hlfont[9];


static int hlobjects P_((WINDOW win, int font, long offset));

#ifdef FEATURE_MISC
static ELVBOOL hlprep P_((WINDOW win, BUFFER buf));

/* Decide what needs to be highlighted for "hllayers" and "hlobject".  Return
 * ElvTrue if same as previous highlighting for this window, or ElvFalse
 * otherwise.
 */
static ELVBOOL hlprep(win, buf)
	WINDOW	win;
	BUFFER	buf;
{
	VIINFO	vinf;
	CHAR	*obj, *cp;
	MARKBUF	from, to;
	long	prevcount;
	long	cursoff;
	int	hllimit;
	RESULT	result;
	int	i, j;
	long	hlfrom, hlto;
	ELVBOOL	wholeline;

	/* initialize hlfrom and hlto to impossible values */
	hlfrom = o_bufchars(buf);
	hlto = -1L;

	/* remember the cursor offset, so we can restore it later */
	cursoff = markoffset(win->cursor);

	/* for each layer... */
	wholeline = ElvFalse;
	memset(&vinf, 0, sizeof vinf);
	for (hllimit = 0, obj = o_hlobject(buf);
	     obj && hllimit < o_hllayers(win);
	     hllimit++)
	{
		/* build the vinf textobject command */
		prevcount = vinf.count;
		vinf.count = 0L;
		while (elvspace(*obj))
			obj++;
		if (elvdigit(*obj))
		{
			for (vinf.count = 0; elvdigit(*obj); obj++)
				vinf.count = vinf.count * 10 + *obj - '0';
		}
		if (obj[0] == 'V' && obj[1] && obj[2])
		{
			wholeline = ElvTrue;
			vinf.command = obj[1];
			vinf.key2 = obj[2];
			obj += 3;
		}
		else if (obj[0] && obj[1])
		{
			wholeline = ElvFalse;
			vinf.command = obj[0];
			vinf.key2 = obj[1];
			obj += 2;
		}
		else if (hllimit == 0)
			break;
		else
		{
			if (obj[0])
				obj++;
			vinf.count = prevcount + 1;
		}

		/* compute the endpoints of the textobject */
		marksetoffset(win->cursor, cursoff);
		result = vitextobj(win, &vinf, &from, &to);
		if (result == RESULT_ERROR)
		{
			if (!*obj)
				break;
			hllimit--;
			continue;
		}
		else if (result == RESULT_MORE)
		{
			to.offset = cursoff;
		}

		/* if supposed to use whole line, then expand endpoints */
		if (wholeline)
		{
			/* move "from" backward to char after newline */
			for (scanalloc(&cp, &from); scanprev(&cp) && *cp != '\n'; )
			{
			}
			if (cp)
				from.offset = markoffset(scanmark(&cp)) + 1;
			else
				from.offset = 0L;
			scanfree(&cp);

			/* move "to" forward to newline */
			for (scanalloc(&cp, &to); *cp != '\n' && scannext(&cp); )
			{
			}
			if (cp)
				to.offset = markoffset(scanmark(&cp)) + 1;
			else
				to.offset = o_bufchars(buf);
		}

		/* if innermost range is the same as before, and nothing else
		 * is forcing us to redraw from scratch, then assume all ranges
		 * are unchanged.
		 */
		if (hllimit == 0
		 && win->di->logic == DRAW_NORMAL
		 && o_optimize
		 && win->hlinfo[hllimit].from == markoffset(&from)
		 && win->hlinfo[hllimit].to == markoffset(&to))
		{
			marksetoffset(win->cursor, cursoff);
			return ElvTrue;
		}

		/* store the new range */
		win->hlinfo[hllimit].from = markoffset(&from);
		win->hlinfo[hllimit].to = markoffset(&to);

		/* adjust the affected range */
		if (win->hlinfo[hllimit].from < hlfrom)
			hlfrom = win->hlinfo[hllimit].from;
		if (win->hlinfo[hllimit].to > hlto)
			hlto = win->hlinfo[hllimit].to;
	}
	marksetoffset(win->cursor, cursoff);

	/* store the limits */
	win->hlfrom = hlfrom;
	win->hlto = hlto;

	/* Assign fonts in reverse order, so "hlobject1" is outermost.  If not
	 * enough fonts have been set, then recycle the defined ones.  We can
	 * be sure that the first font has been set because we give it a
	 * default definition in the drawalloc() function.
	 */ 
	for (i = hllimit - 1, j = 0; i >= 0; i--)
	{
		win->hlinfo[i].font = hlfont[j++];
		if (j >= QTY(hlfont) || colorinfo[hlfont[j]].da.bits == 0)
			j = 0;
	}

	/* if nothing was highlighted, then clobber the hlinfo */
	if (hllimit == 0)
		win->hlinfo[0].from = -1L;

	return ElvFalse;
}
#endif /* FEATURE_MISC */

/* compute the highlighted version of a font at a given point */
static int hlobjects(win, font, offset)
	WINDOW	win;
	int	font;
	long	offset;
{
	int	i;

	/* if totally unaffected, then just return the font unchanged */
	if (offset < win->hlfrom || win->hlto <= offset)
		return font;

	/* find the innermost layer which includes this offset */
	for (i = 0; offset < win->hlinfo[i].from || win->hlinfo[i].to <= offset; i++)
	{
	}

	/* combine the fonts */
	return colortmp(font, win->hlinfo[i].font);
}

#endif /* defined(FEATURE_HLOBJECT) */

/* allocate a drawinfo struct, including the related arrays */
DRAWINFO *drawalloc(rows, columns, top)
	int	rows;	/* height of new window image */
	int	columns;/* width of new window image */
	MARK	top;	/* top of screen */
{
	DRAWINFO *newp;
	int	 i;
#ifdef FEATURE_HLOBJECT
	CHAR	name[20];
#endif

	/* allocate the stuff */
	newp = (DRAWINFO *)safealloc(1, sizeof *newp);
	newp->newrow = (DRAWROW *)safealloc(rows, sizeof(DRAWROW));
	newp->newline = (DRAWLINE *)safealloc(rows + 1, sizeof(DRAWLINE));
	newp->curline = (DRAWLINE *)safealloc(rows, sizeof(DRAWLINE));
	newp->newchar = (CHAR *)safealloc(rows * columns, sizeof(CHAR));
	newp->newfont = (char *)safealloc(rows * columns, sizeof(char));
	newp->curchar = (CHAR *)safealloc(rows * columns, sizeof(CHAR));
	newp->curattr = (DRAWATTR *)safealloc(rows * columns, sizeof(DRAWATTR));
	newp->offsets = (long *)safealloc(rows * columns, sizeof(long));
	newp->topmark = markdup(top);
	newp->bottommark = markdup(top);

	/* clear the current image and the "new" image */
	for (i = rows * columns; --i >= 0; )
	{
		newp->newchar[i] = newp->curchar[i] = ' ';
		newp->newfont[i] = 0;
		newp->curattr[i].bits = ~0; /* anything so newfont != curfont */
	}

	/* initialize the other variables */
	newp->rows = rows;
	newp->columns = columns;
	newp->logic = DRAW_SCRATCH;

	/* while we're initializing things, we might as well locate the
	 * font codes, if we haven't already done so.
	 */
#ifdef FEATURE_HLOBJECT
	for (i = 0; i < QTY(hlfont); i++)
	{
		sprintf(tochar8(name), "hlobject%d", i + 1);
		hlfont[i] = colorfind(name);
	}
	colorset(hlfont[0], toCHAR("boxed"), ElvFalse);
#endif
#ifdef FEATURE_HLSEARCH
	if (!searchfont)
	{
		searchfont = colorfind(toCHAR("hlsearch"));
		colorset(searchfont, toCHAR("bold"), ElvFalse);
	}
#endif

	return newp;
}

/* free a drawinfo struct, including the related arrays */
void drawfree(di)
	DRAWINFO	*di;	/* window image to destroy */
{
	/* free the stuff */
	safefree(di->newrow);
	safefree(di->newline);
	safefree(di->curline);
	safefree(di->newchar);
	safefree(di->newfont);
	safefree(di->curchar);
	safefree(di->curattr);
	safefree(di->offsets);
	markfree(di->topmark);
	markfree(di->bottommark);
	if (di->openimage)
	{
		safefree(di->openimage);
	}
	if (di->openline)
	{
		markfree(di->openline);
	}
	safefree(di);
}

/* re-output a rectangular part of a window's current image.  The top-left
 * corner of the window is (0,0).
 *
 * The redrawn area includes the interior and edges of the window.  This means,
 * for example, that if top==bottom, a single row will be (partially) redrawn.
 */
void drawexpose(win, top, left, bottom, right)
	WINDOW	win;	/* window that was partially exposed */
	int	top;	/* top edge to be redrawn */
	int	left;	/* left edge to be redrawn */
	int	bottom;	/* bottom edge to be redrawn */
	int	right;	/* right edge to be redrawn */
{
#ifndef FEATURE_MISC
	win->di->logic = DRAW_SCRATCH;
#else
	int	row, column, same, base, nonblank;
	long	firstline, lastline;

	assert(win != NULL && top >= 0 && left >= 0 && bottom < o_lines(win)
		&& right < o_columns(win) && top <= bottom && left <= right);

	/* If this GUI has no moveto() function, then do nothing.  We really
	 * should never get here anyway in that case, since elvis can't be
	 * running in full-screen mode.
	 */
	if (!gui->moveto)
	{
		return;
	}

	/* make sure we use the right colors.  Basically, this is responsible
	 * for choosing whether to use the "normal" or "idle" colors for any
	 * unspecified attributes such as background color.
	 */
	if (guicolorsync(win))
	{
		/* colors changed, so must redraw whole window */
		top = 0;
		left = 0;
		bottom = o_lines(win) - 1;
		right = o_columns(win) - 1;
	}
	colorsetup();

	/* for each row in the rectangle... */
	for (row = top; row <= bottom; row++)
	{
		/* find the width of this row, ignoring trailing blanks */
		for (nonblank = o_columns(win), base = o_columns(win) * row;
		     nonblank > left
			&& drawdeffont(win->di, base + nonblank - 1)
			&& win->di->curchar[base + nonblank - 1] == ' ';
		     nonblank--)
		{
		}

		/* move to the first character we'll be redrawing in this row */
		guimoveto(win, left, row);

		/* for each segment of the row which shares the same font... */
		for (column = left, base += column;
		     column <= right && column < nonblank;
		     base += same, column += same)
		{
			/* find the width of the segment */
			for (same = 1;
			     column + same <= right
				&& column + same < nonblank
				&& drawspan(win->di, base + same, base);
			     same++)
			{
			}

			/* output the segment */
			(void)guidraw(win,
				colorexpose(win->di->newfont[base],
					&win->di->curattr[base]),
				&win->di->curchar[base], same, 0);
		}

		/* if necessary, do a clrtoeol */
		if (nonblank < right)
		{
			guiclrtoeol(win);
		}
	}

	/* leave the cursor in the right place */
	guimoveto(win, win->di->curscol, win->di->cursrow);

	/* update the scrollbar, too */
	if (gui->scrollbar)
	{
		if (win->md->move == dmnormal.move)
		{
			/* find line numbers of first and last lines shown */
			firstline = markline(win->di->topmark) - 1;
			lastline = markline(win->di->bottommark) - 1;

			/* Some scrolling commands temporarily set bottommark
			 * to the end of the buffer.  Ordinarily this causes
			 * no problems, but if an expose event occurs before
			 * the screen is redrawn, the the scrollbar's "thumb"
			 * will briefly extend to the bottom of the scrollbar.
			 * If we seem to be in that situation, then make a more
			 * reasonable guess about the screen bottom line number.
			 */
			if (lastline - firstline >= o_lines(win))
			{
				lastline = firstline + o_lines(win) - 1;
			}

			/* update the scrollbar */
			(*gui->scrollbar)(win->gw, firstline, lastline,
				o_buflines(markbuffer(win->cursor)));
		}
		else
		{
			(*gui->scrollbar)(win->gw, markoffset(win->di->topmark),
				markoffset(win->di->bottommark), win->di->curnbytes);
		}
	}
	guiflush();
#endif /* FEATURE_MISC */
}


/*----------------------------------------------------------------------------*/
/* This marks the end of the easy stuff!  The remainder of this file consists
 * of the drawimage() function and its support functions and variables.
 */


static WINDOW	thiswin;	/* The window being drawn */
static long	thisline;	/* the line being drawn */
static char	thislnumstr[16];/* line number, as a string */
static int	linesshown;	/* number of different lines visible */
static int	thiscell;	/* index of next cell to draw */
static int	thiscol;	/* logical column number */
static int	leftcol;	/* leftmost column to actually draw */
static int	rightcol;	/* rightmost column to actully draw, plus 1 */
static int	maxrow;		/* number of rows to be drawn */
static int	wantcurs;	/* largest acceptable cursor row */
static int	thisscroll;	/* scroll distance while drawing this line */
static int	scrollrows;	/* #rows (i.e., area) scrolled by gui->scroll */
static int	maxcell;	/* number of cells to be drawn */
static int	seloffset;	/* offset of character, during selection */
static char	selfont;	/* font of character, during selection */
#ifdef FEATURE_REGION
static region_t *thisregion;
#endif
#ifdef FEATURE_LISTCHARS
static int	lcscell;	/* cell where arrow will be drawn */
#endif



/* insert some blanks into an image */
static void insimage(ch, attr, qty, extent)
	CHAR	*ch;	/* where character insertion should begin */
	DRAWATTR *attr;	/* parallel array of attributes for "ch" characters */
	int	qty;	/* number of blanks to be inserted */
	int	extent;	/* number of characters after the insertion point */
{
	register int i;

	/* shift the characters and font codes */
	for (i = extent; --i >= qty; )
	{
		ch[i] = ch[i - qty];
		attr[i] = attr[i - qty];
	}

	/* initialize the newly inserted cells */
	if (gui->newblank)
	{
		/* the newly inserted characters are all normal spaces */
		for (i = 0; i < qty; i++)
		{
			ch[i] = ' ';
			attr[i] = colorinfo[0].da;
		}
	}
	else
	{
		/* the new cells' contents are undefined */
		for (i = 0; i < qty; i++)
		{
			ch[i] = 0;
			attr[i].bits = ~0;
		}
	}
}

/* delete some characters from an image, and add blanks to the end. */
static void delimage(ch, font, attr, qty, extent)
	CHAR	*ch;	/* where character deletion should begin */
	char	*font;	/* parallel array of font codes for "ch" characters */
	DRAWATTR *attr;	/* parallel array of attributes for "ch" characters */
	int	qty;	/* number of characters to delete */
	int	extent;	/* number of characters after the deletion point */
{
	register int i;

	/* shift the characters */
	for (i = 0; i < extent - qty; i++)
	{
		ch[i] = ch[i + qty];
		if (font)
			font[i] = font[i + qty];
		if (attr)
			attr[i] = attr[i + qty];
	}

	/* we've dragged some normal characters onto the edge of the extent */
	if (gui->newblank)
	{
		/* the new cells are filled with normal blanks */
		for ( ; i < extent; i++)
		{
			ch[i] = ' ';
			if (font)
				font[i] = 0;
			if (attr)
				attr[i] = colorinfo[0].da;
		}
	}
	else
	{
		/* the new cells' contents are undefined */
		for ( ; i < extent; i++)
		{
			ch[i] = 0;
			if (font)
				font[i] = ~0;
			if (attr)
				attr[i].bits = ~0;
		}
	}
}

/* Fill in a character cell.  This function is called by drawchar(), which
 * in turn is called by an edit mode's image() function.
 */
static void fillcell(ch, font, offset)
	_CHAR_	ch;	/* new character to place in the next cell */
	_char_	font;	/* font code of new character */
	long	offset;	/* buffer offset, or -1 if not from buffer */
{
	register int		i;
	register DRAWINFO	*di = thiswin->di;

	assert(thiscell <= maxcell);
	assert(maxcell <= maxrow * di->columns);

	/* if we've reached the end of the screen without finding the cursor,
	 * then we'll need to do a little slop scrolling.
	 */
	if (thiscell == maxcell && (di->cursrow < 0 || di->cursrow > wantcurs))
	{
		/* scroll up 1 row */
		delimage(di->newchar, di->newfont, NULL, (int)o_columns(thiswin), (int)(maxcell + o_columns(thiswin)));
		for (i = 0; i < maxcell - o_columns(thiswin); i++)
		{
			di->offsets[i] = di->offsets[i + o_columns(thiswin)];
		}
		thisscroll++;

		/* scroll the statistics, too */
		di->newrow[1].insrows += di->newrow[0].insrows;
		for (i = 0; i < maxrow - 1; i++)
		{
			di->newrow[i] = di->newrow[i + 1];
		}
		di->newrow[i].insrows =
			di->newrow[i].shiftright =
			di->newrow[i].inschars = 0;
		for (i = 0; i < linesshown; i++)
		{
			di->newline[i].startrow--;
		}
		assert(di->cursrow != 0);
		if (di->cursrow > 0)
			di->cursrow--;

		/* if the top /line/ has scrolled off screen, then scroll
		 * the line statistics, too.
		 */
		if (linesshown > 1 && di->newline[1].startrow == 0)
		{
			linesshown--;
			for (i = 0; i < linesshown; i++)
			{
				di->newline[i] = di->newline[i + 1];
			}
		}

		/* adjust limits, taking o_sidescroll into consideration */
		thiscell -= o_columns(thiswin);
		if (o_wrap(thiswin))
		{
			rightcol += o_columns(thiswin);
		}
	}

	assert(thiscell < maxcell || di->cursrow >= 0);

	/* if this is the cursor character, then the cursor belongs here */
#if 0
	if (offset == markoffset(thiswin->cursor))
#else
	if (offset >= markoffset(thiswin->cursor) && di->cursrow < 0) /*!!!*/
#endif
	{
		di->cursrow = thiscell / o_columns(thiswin);
	}


	/* if this cell really should be drawn, then draw it */
	if (thiscol >= leftcol && thiscol < rightcol && thiscell < maxcell)
	{
		di->newchar[thiscell] = ch;
		di->newfont[thiscell] = font;
		di->offsets[thiscell] = offset;
		thiscell++;
	}

	/* increment the column number */
	thiscol++;
}


/* Add consecutive characters to the new image.  This function is called by
 * an edit mode's image() function to generate a screen image.  In addition
 * to displayable characters, this function gives special processing to
 * the following:
 *	'\013' (VTAB) is interpretted as two '\n' characters.
 *	'\f' is interpretted as two '\n' characters.
 *	'\n' moves to the start of the next row.
 * Note that tabs and backspaces are not given special treatment here;
 * that's the responsibility of the edit mode's image() function.
 */
static void drawchar(p, qty, font, offset)
	CHAR	*p;	/* the characters to be added */
	long	qty;	/* number of characters to add */
	_char_	font;	/* font code of characters */
	long	offset;	/* buffer offset of character, or -1 if not from buffer */
{
	register 	  CHAR	ch;		/* a character from *p */
	register DRAWINFO *di = thiswin->di;	/* window drawing info */
	long		  delta;		/* value to add to "offset" */
	CHAR		  hifont;		/* possibly highlighted font */
	CHAR		  tmpch;
	int		  i;

#ifdef FEATURE_REGION
	/* if in a region, then merge the region's font into font */
	if (thisregion)
		font = colortmp(font, thisregion->font);
#endif

	/* A negative qty value indicates that all characters have the same
	 * offset.  Otherwise the characters will have consecutive offsets.
	 */
	if (qty < 0)
	{
		qty = -qty;
		delta = 0;
	}
	else
	{
		delta = 1;
	}

	/* If in visual mode and this chunk contains the cursor, then remember
	 * the font now, *before* we add any sort of highlighting
	 */
	if (!thiswin->state->pop
	 && offset >= 0
	 && offset <= markoffset(thiswin->cursor)
	 && (delta == 0 || markoffset(thiswin->cursor) < offset + delta * qty))
	{
		thiswin->di->cursface = font & 0x7f;
	}

	/* for each character... */
	for ( ; qty > 0; qty--, p += delta, offset += delta)
	{
		/* copy the next char into a register variable */
		ch = *p;

		/* Treat '\f' and '\013' (VTAB) as two '\n's in a row
		 * (one of them recursively)
		 */
		if (ch == '\f' || ch == '\013')
		{
			tmpch = ch = '\n';
			drawchar(&tmpch, 1, font, offset);
		}
		/* Handle non-ascii characters */
		else if (ch >= 0x80)
		{
			switch (o_nonascii)
			{
			  case 's':	/* strip */
				ch &= 0x7f;
				if (ch < ' ' || ch == 0x7f)
					ch = '.';
				break;

			  case 'n':	/* none */
				ch = '.';
				break;	/* !!it B4: to make cc happy */

			  case 'm':	/* most */
				if (ch <= 0x9f)
					ch = '.';
				break;	/* !!it B4: to make cc happy */

			  default:	/* all */
				/* no conversion necessary */;
			}
		}

		/* If we're showing line numbers, and this character is supposed
		 * to be displayed in column 0, then output the line number
		 * before this character.
		 */
		if (o_number(thiswin) && thiscol == leftcol)
		{
			for (i = 0; i < 8; i++)
			{
				fillcell((CHAR)thislnumstr[i], COLOR_FONT_LNUM, -1);
			}
		}

		/* If the offset is in the selected region for this window,
		 * then make the font letter uppercase so that it will be
		 * drawn highlighted.  Also, make wide characters be either
		 * totally highlighted or totally unhighlighted.
		 */
		hifont = font;
#ifdef FEATURE_HLOBJECT
		/* maybe use the highlighted version of this font */
		hifont = hlobjects(thiswin, hifont, offset);
#endif
		if (thiswin->seltop)
		{
			if (offset == seloffset && seloffset != -1)
			{
				hifont = selfont;
			}
			else
			{
				i = thiscol;
				if (o_number(thiswin))
					i -= 8;
				if (markoffset(thiswin->seltop) <= offset
					&& offset <= markoffset(thiswin->selbottom)
					&& thiswin->selleft <= i
					&& i <= thiswin->selright
					&& (ch != '\n' || thiswin->seltype != 'r'))
				{
					hifont = font | 0x80;
				}
			}
			selfont = hifont;
			seloffset = offset;
		}
#ifdef FEATURE_TEXTOBJ
		else if (thiswin->match <= offset && offset < thiswin->matchend)
#else
		else if (thiswin->match == offset)
#endif
		{
			hifont = font | 0x80; /* showmatch */
		}

		/* remember where this line started */
		if (linesshown == 0 || di->newline[linesshown - 1].start != thisline)
		{
			assert(linesshown <= maxrow);
			assert((thiscell % o_columns(thiswin)) ==
			    (o_number(thiswin) && leftcol == 0 ? 8 : 0));
			di->newline[linesshown].start = thisline;
			di->newline[linesshown++].startrow = thiscell / o_columns(thiswin);
		}

		/* if this is a newline, then treat it as a bunch of spaces */
		if (ch == '\n')
		{
			/* remember this line's width.  If a single "line"
			 * involves multiple newlines, then the width of the
			 * last non-empty phsyical line is kept.
			 */
			if (thiscol != 0)
			{
				di->newline[linesshown].width = thiscol;
			}

			/* "leftcol" has no effect on newlines */
			if (thiscol < leftcol)
			{
				thiscol = leftcol;
			}

			/* Output a bunch of spaces.  Note that the looping
			 * condition is sensitive to both the column number and
			 * the cell number, so it'll correctly handle lines
			 * whose width==o_columns(win).  It emulates the :xw:
			 * brain-damaged newline, like a VT-100.
			 *
			 * This gets a bit tricky when you're appending to the
			 * end of a line, and you've just extended the line to
			 * reach edge of the screen.  In this case, an extra
			 * blank line *should* appear on the screen; the cursor
			 * will be there.  This test must be made outside the
			 * loop.
			 */
			i = o_columns(thiswin);
			if ((offset == markoffset(thiswin->state->cursor)
				&& thiscol % i == 0) || thiscol == leftcol)
			{
				fillcell(' ', hifont, offset);
			}
			/* Note: thiscell may've changed since previous "if" */
			if (thiscell % i != 0)
			{
				for (i -= thiscell % i; i > 0; i--)
				{
					di->newchar[thiscell] = ' ';
					di->newfont[thiscell] = hifont;
					di->offsets[thiscell] = offset;
					thiscol++, thiscell++;
				}
			}

			/* reset the virtual column number */
			thiscol = 0;
		}
		else /* normal character */
		{
			assert(ch >= ' ');

			/* draw the character */
			fillcell(ch, hifont, offset);
		}
	}
}

#ifdef FEATURE_LISTCHARS
/* This is a simplified version of drawchar(), intended *ONLY* for drawing the
 * little arrows for sidescrolling when ":set nowrap lcs=extends:>,precedes:>"
 */
static void lcsdrawchar(p, qty, font, offset)
	CHAR	*p;	/* the characters to be added */
	long	qty;	/* number of characters to add -- always 1 */
	_char_	font;	/* font code of characters */
	long	offset;	/* offset of chars -- ignored */
{
	register DRAWINFO *di = thiswin->di;	/* window drawing info */
	di->newchar[lcscell] = *p;
	di->newfont[lcscell] = font;
	lcscell++;
}
#endif

/* This function compares old lines to new lines, and determines how much
 * insert/deleting we should do, and approximately where we should do it.
 */
static void compareimage(win)
	WINDOW	win;	/* window to be processed */
{
	typedef enum {NEW, BEFORE, AFTER} type_t;
	int	i, j, k, diff, totdiff;
	struct prevmatch_s
	{
		DRAWLINE *line;	/* ptr to new line's current info */
		type_t	 type;	/* line position, relative to any change */
	}	*prev;

	/* if we're supposed to redraw from scratch, then we won't be doing
	 * any inserting/deleting.  Also, the "guidewidth" option interferes
	 * with shifting, so we disable it if guidewidth is set.
	 */
	if (win->di->logic == DRAW_SCRATCH
	 || o_guidewidth(markbuffer(win->cursor)))
	{
		for (i = 0; i < maxrow; i++)
		{
			win->di->newrow[i].shiftright = 0;
			assert(win->di->newrow[i].insrows == 0 && win->di->newrow[i].inschars == 0);
		}

		/* also, ignore anything in current image */
		if (win->di->logic == DRAW_SCRATCH)
		{
			memset(win->di->curchar, 0, maxcell * sizeof(CHAR));
			memset(win->di->curattr, 0, maxcell * sizeof(DRAWATTR));
		}

		return;
	}

	/* allocate an array for matching new lines to old lines */
	prev = (struct prevmatch_s *)safealloc(linesshown, sizeof *prev);

	/* Try to match each new line against current lines */

	/* try to match top lines as being BEFORE a change */
	i = 0;
	for (j = 0; j < win->di->nlines && win->di->newline[0].start != win->di->curline[j].start; j++)
	{
	}
	if (j < win->di->nlines)
	{
		do
		{
			prev[i].line = &win->di->curline[j];
			prev[i].type = BEFORE;
		} while (++i < linesshown
		      && ++j < win->di->nlines
		      && win->di->newline[i].start == win->di->curline[j].start);
	}

	/* try to match bottom lines as being AFTER a change */
	for (k = linesshown - 1; k >= i; k--)
	{
		for (j = win->di->nlines - 1;
		     j >= 0
			&& o_bufchars(markbuffer(win->cursor)) - win->di->newline[k].start
				!= win->di->curnbytes - win->di->curline[j].start;
		     j--)
		{
		}
		if (j >= 0)
		{
			do
			{
				prev[k].line = &win->di->curline[j];
				prev[k].type = AFTER;
			} while (--k >= i
			      && --j >= i
			      && o_bufchars(markbuffer(win->cursor)) - win->di->newline[k].start
					== win->di->curnbytes - win->di->curline[j].start);
			break;
		}
		prev[k].type = NEW;
	}

	/* any intervening lines are NEW */
	for (; i < k; i++)
	{
		prev[i].type = NEW;
	}

	/* Compute character shifts/inserts/deletes for each row... */
	for (i = 0; i < maxrow && win->di->newrow[i].lineoffset < o_bufchars(markbuffer(win->cursor)); i++)
	{
		/* find the line shown there (actually its index into newline[] and prev[]) */
		for (j = 0; win->di->newrow[i].lineoffset != win->di->newline[j].start; j++)
		{
			assert(j + 1 < linesshown);
		}

		/* if it is a new line, then don't shift it */
		if (prev[j].type == NEW)
		{
			win->di->newrow[i].shiftright = 0;
			assert(win->di->newrow[i].inschars == 0);
		}
		else /* recognized old line, maybe insert/delete chars or rows */
		{
			/* number of chars to insert/delete is computed by change in length */
			win->di->newrow[i].inschars = win->di->newline[j].width - prev[j].line->width;
		}
	}
	for ( ; i < maxrow; i++)
	{
		/* lines after the end of the buffer will show a tilde */
		win->di->newrow[i].shiftright = 0;
		assert(win->di->newrow[i].inschars == 0 && win->di->newrow[i].inschars == 0);
	}

	/* Compute row insertion/deletion for each line... */
	for (i = totdiff = 0; i < linesshown; i++, totdiff += diff)
	{
		/* We never need to insert/delete rows to make a NEW line
		 * appear in the correct position.
		 */
		if (prev[i].type == NEW)
		{
			diff = 0;
			continue;
		}

		/* Compute the positional difference for this line.  Also the
		 * number of lines inserted/delete for preceding lines should
		 * be taken into consideration.
		 */
		diff = win->di->newline[i].startrow - prev[i].line->startrow - totdiff;

		/* If inserting, we'll insert at the line's old starting row.
		 * If deleting, we'll delete on a row above the line's starting
		 * row so the line's current image ends up on the intended line
		 */
		j = prev[i].line->startrow + totdiff;
		if (diff < 0)
		{
			j += diff;
		}

		/* logical lines may start on negative rows, due to slop
		 * scrolling, but physically we can only address lines >= 0.
		 * Any insertions or deletions for a negative row should be
		 * applied to row 0.
		 */
		if (j < 0)
		{
			j = 0;
		}

		/* add the insertions/deletions for the appropriate row. */
		win->di->newrow[j].insrows += diff;
	}

#if 0
for (i = 0; i < maxrow; i++) printf("newrow[%d] = {lineoffset=%ld, insrows=%d, shiftright=%d, inschars=%d}, type=%s\n",
i, win->di->newrow[i].lineoffset, win->di->newrow[i].insrows, win->di->newrow[i].shiftright, win->di->newrow[i].inschars,
prev[i].type==BEFORE?"BEFORE":prev[i].type==NEW?"NEW":prev[i].type==AFTER?"AFTER":"?");
printf("---------------------------------------------------------------------\n");
#endif

	/* we don't need the prev array anymore */
	safefree(prev);
}

/* update a segment of the current image to match the new image */
static void drawcurrent(di, base, same, da)
	DRAWINFO *di;	/* image info of window to be updated */
	int	base;	/* offset of first char to change */
	int	same;	/* number of chars to change */
	DRAWATTR *da;	/* attributes of new characters */
{
	int	i;

	/* write the cells to the "current" image */
	for (i = 0; i < same; i++)
	{
		di->curchar[base + i] = di->newchar[base + i];
		di->curattr[base + i] = *da;
#ifdef SLOPPY_BOXED
		if (i != 0)
			di->curattr[base + i].bits &= ~COLOR_LEFTBOX;
		if (i != same - 1)
			di->curattr[base + i].bits &= ~COLOR_RIGHTBOX;
#endif
	}
}

/* after we've collected the image and some update hints, copy the new image to
 * both the "current" image and also the GUI.
 */
static void updateimage(win)
	WINDOW	win;	/* window to be updated */
{
	int	row, column, same, used;
	int	ncols = o_columns(win);
	register int	  i, base;
	register DRAWINFO *di = win->di;
	register int	  rowXncols;
	DRAWATTR *da;
	int	forcebits, j;
	short	*guides = o_guidewidth(markbuffer(win->cursor));
	int	logicaltab;
#ifdef SLOPPY_BOXED
	int	firstboxed;
#endif

	/* for each row... */
	for (row = rowXncols = 0; row < o_lines(win); row++, rowXncols += ncols)
	{
		if (di->logic == DRAW_SCRATCH)
			di->newrow[row].insrows = di->newrow[row].shiftright
						= di->newrow[row].inschars = 0;

		/* compute the logical tab of the start of this row */
		if (row == o_lines(win) - 1)
			logicaltab = 0;
		else
		{
			for (i = 0; di->newline[i].startrow < row && di->newline[i + 1].startrow - 1 < row; i++)
			{
			}
			logicaltab = (row - di->newline[i].startrow) * ncols;
			if (o_number(win))
				logicaltab -= 8;
			if (!o_wrap(win))
				logicaltab += di->skipped;
		}

		/* if we've hit a ~ row after the last line from the buffer,
		 * then disable guides.
		 */
		if (guides
		 && di->offsets[row * ncols] == -1)
			guides = NULL;

		/* do we want to insert/delete rows? */
		if (di->newrow[row].insrows != 0)
		{
			/* try to perform the insert/delete for the GUI */
			guimoveto(win, 0, row);
			if (guiscroll(win, di->newrow[row].insrows, (ELVBOOL)(maxrow < o_lines(win))))
			{
				/* perform the insert/delete on the "current" image */
				if (di->newrow[row].insrows > 0)
				{
					/* insert some rows */
					insimage(&di->curchar[rowXncols],
						&di->curattr[rowXncols],
						di->newrow[row].insrows * ncols,
						(scrollrows - row) * ncols);

					/* shift amount of new lines is always 0 */
					for (i = row; i < row + di->newrow[row].insrows; i++)
					{
						di->newrow[i].shiftright =
							di->newrow[i].inschars = 0;
					}
				}
				else
				{
					/* delete some rows */
					delimage(&di->curchar[rowXncols],
						NULL,
						&di->curattr[rowXncols],
						-di->newrow[row].insrows * ncols,
						(scrollrows - row) * ncols);

					/* shift amount of new lines is always 0 */
					for (i = maxrow + di->newrow[row].insrows; i < maxrow; i++)
					{
						di->newrow[i].shiftright =
							di->newrow[i].inschars = 0;
					}
				}
			}
			else /* either we can't scroll, or have a huge insert/delete amount */
			{
				/* don't do any more scrolling after this */
				for (i = row; i < o_lines(win); i++)
				{
					di->newrow[i].insrows = 0;
				}
			}
		}

		/* perform shifting, if any */
		if (di->newrow[row].shiftright != 0)
		{
			/* see how many following rows have the same shift */
			for (same = 1;
			     gui->shiftrows && 
			        row + same < di->rows - 1 &&
				di->newrow[row].shiftright == di->newrow[row + same].shiftright;
			     same++)
			{
			}

			/* make the GUI do its shift */
			guimoveto(win, 0, row);
			if (guishift(win, di->newrow[row].shiftright, same))
			{
				/* shift the current image, too */
				for (i = row; i < row + same; i++)
				{
					if (di->newrow[i].shiftright > 0)
					{
						/* shift right by inserting */
						insimage(&di->curchar[i * ncols],
							&di->curattr[i * ncols],
							di->newrow[i].shiftright,
							ncols);
					}
					else 
					{
						/* shift left by deleting */
						delimage(&di->curchar[i * ncols],
							NULL,
							&di->curattr[i * ncols],
							-di->newrow[i].shiftright,
							ncols);
					}
					di->newrow[i].shiftright = 0;
				}
			}
		}

		/* figure out how much of the line we'll be using */
		base = ncols * row;
		used = ncols;
		while (used > 0
			&& (!guides || !opt_istab(guides, used+logicaltab-1))
			&& di->newchar[base + used - 1] == ' '
			&& di->newfont[base + used - 1] == 0)
		{
			used--;
		}

#ifdef SLOPPY_ITALICS
		/* If the line ends with an italic character, then pretend
		 * we're using one additional character.  This is so the last
		 * italic character's right edge isn't clipped off.
		 */
		if (used < ncols && used >= 1 && isitalic(di->newfont[base + used - 1]))
		{
			used++;
		}
#endif

		/* look for mismatched segments */
#ifdef SLOPPY_BOXED
		firstboxed = (di->cursrow == row);
#endif
		for (column = 0; column < used; base += same, column += same)
		{
			/* if this cell matches, then skip to next */
			if (drawnochange(di, base)
#ifdef SLOPPY_BOXED
			 && (firstboxed
				? !isboxed(di->newfont[base])
				: isboxed(di->newfont[base + 1]))
#endif
			 && (!guides || !opt_istab(guides, logicaltab)))
			{
				same = 1;
				logicaltab++;
				continue;
			}
#ifdef SLOPPY_BOXED
			firstboxed = !isboxed(di->newfont[base]);
#endif

			/* move the GUI cursor to the point of interest */
			guimoveto(win, column, row);

			/* perform this row's character insert/delete now,
			 * unless we just shifted this line to the right, and
			 * we're still on the inserted blanks at the start
			 * of the row.  Also, the "guidewidth" option interferes
			 * with insertion/deletion.
			 */
			if (column >= di->newrow[row].shiftright
			  && di->newrow[row].inschars != 0
			  && !guides)
			{
				if (guishift(win, di->newrow[row].inschars, 1))
				{
					/* shift the current image, too */
					if (di->newrow[row].inschars > 0)
					{
						/* shift right by inserting */
						insimage(&di->curchar[rowXncols + column],
							&di->curattr[rowXncols + column],
							di->newrow[row].inschars,
							ncols - column);
					}
					else 
					{
						/* shift left by deleting */
						delimage(&di->curchar[rowXncols + column],
							NULL,
							&di->curattr[rowXncols + column],
							-di->newrow[row].inschars,
							ncols - column);
					}

					/* for other rows of the same line,
					 * insertions will now appear to be
					 * shifts.
					 */
					for (i = row + 1; i < maxrow && di->newrow[i].lineoffset == di->newrow[row].lineoffset; i++)
					{
						di->newrow[i].shiftright += di->newrow[i].inschars;
						di->newrow[i].inschars = 0;
					}
				}

				/* we have now done all the inserting/deleting
				 * that we're ever going to do for this row.
				 */
				di->newrow[row].inschars = 0;
			}

			/* See how many mismatched cells we can find with same
			 * font.  Allow small spans of matched characters, if
			 * it costs less to redraw unchanged characters than
			 * moving the cursor would cost.
			 */
			for (same = 1, i = 0;
			     column + same < used
				&& (!drawnochange(di, base + same)
					|| i < gui->movecost)
				&& (di->newfont[base] == di->newfont[base + same]
#ifdef SLOPPY_ITALICS
					|| (isitalic(di->newfont[base + same - 1]) && di->newfont[base + same] == 0 && di->newchar[base + same] == ' ')
#endif
										 );
			     same++)
			{
				if (!drawnochange(di, base + same))
				{
					i = 0;
				}
				else
				{
					i++;
				}
			}
			same -= i;

			/* we may want to force some highlighting attributes,
			 * but for now we'll assume we don't.
			 */
			forcebits = 0;

#ifdef SLOPPY_ITALICS
			/* if the change is in italics, and the preceding
			 * characters were also italic, then include those
			 * preceding characters in the redrawn span; otherwise 
			 * they would be distorted by the new text.
			 */
			i = di->newfont[base];
			if (isitalic(i) && !guides)
			{
				while (column > 0
				    && di->newfont[base - 1] == i
				    && di->newchar[base - 1] != ' ')
				{
					base--;
					column--;
					same++;
				}
				guimoveto(win, column, row);
			}
#endif

#ifdef SLOPPY_BOXED
			/* if the change is boxed add left & right edges if
			 * necessary the surrounding text is not boxed.
			 */
			i = di->newfont[base];
			if (isboxed(i))
			{
				/* if the characters before/after this boxed
				 * text aren't boxed, or are off the edge of
				 * the screen, then we'll need left/right edges.
				 */
				if (column == 0
				    || !isboxed(di->newfont[base - 1]))
					forcebits |= COLOR_LEFTBOX;
				if (column + same == di->columns
				    || !isboxed(di->newfont[base + same]))
					forcebits |= COLOR_RIGHTBOX;
			}
#endif /* SLOPPY_BOXED */

			/* Write the cells to the GUI.
			 *
			 * NOTE: When this function is called from drawimage(),
			 * the colorexpose() calls always return the font code
			 * that's passed into them, because the temporary
			 * colors are still valid.  However, when called from
			 * drawopenedit() the colorexpose() calls are necessary
			 * to work around the fact that the temporary colors
			 * have been recycled.
			 */
			if (guides
			 && di->newfont[base] != COLOR_FONT_BOTTOM
			 && row < o_lines(win) - 1
			 && logicaltab >= 0)
			{
				if (logicaltab != 0 && opt_istab(guides, logicaltab))
					forcebits |= COLOR_LEFTBOX;
				for (i = same;
				     i > (j = opt_totab(guides, logicaltab));
				     i -= j)
				{
					da = guidraw(win,
					   colorexpose(di->newfont[base+same-i],
						     &di->curattr[base+same-i]),
					   &di->newchar[base+same-i], j,
					   forcebits);
					drawcurrent(di, base + same - i, j, da);
					forcebits |= COLOR_LEFTBOX;
					logicaltab += j;
				}
				da = guidraw(win,
					colorexpose(di->newfont[base+same-i],
						&di->curattr[base+same-i]),
					&di->newchar[base + same - i], i,
					forcebits);
				drawcurrent(di, base + same - i, i, da);
				logicaltab += i;
			}
			else
			{
				da = guidraw(win, colorexpose(di->newfont[base],
							&di->curattr[base]),
					&di->newchar[base], same, forcebits);
				drawcurrent(di, base, same, da);
				logicaltab += same;
			}
		}

		/* Do we need to clear to the end of the row? */
		base = ncols * row;
		for (i = used;
		     i < ncols
			&& di->curchar[base + i] == ' '
			&& drawdeffont(di, base + i);
		     i++)
		{
		}
		if (i < ncols)
		{
			/* Yes, we need to do a clrtoeol */

			/* We must clear at least from column "i", but the
			 * cursor is probably already at column "used".
			 * If the GUI doesn't say we should move to "i",
			 * then move to "used" instead.
			 */
			if (!gui->minimizeclr)
			{
				i = used;
			}
			guimoveto(win, i, row);

			/* perform the clrtoeol */
			guiclrtoeol(win);
			while (i < ncols)
			{
				di->curchar[base + i] = ' ';
				di->curattr[base + i] = colorinfo[0].da;
				i++;
			}
		}
	}
}


/* This function generates the image of the last row.  This is where "showmode"
 * and "ruler" come into play.  Also, part of the previous line may be retained
 */
static void genlastrow(win)
	WINDOW	win;	/* window whose last row is to be generated */
{
	int	i, j, base;
	char	*scan;
	CHAR	*cp;
	char	buf[25];
	CHAR	left[100];
	CHAR	*build, *arg;
#ifdef DISPLAY_HTML
	CHAR	*linkname;
#endif
#ifdef FEATURE_REGION
	region_t *region;
#endif

	/* generate the last row's "show" data */
	for (cp = o_show, build = left; (!win->di->newmsg || gui->status) && cp && *cp; cp++)
	{
		/* detect special words */
		arg = NULL;
#ifdef DISPLAY_HTML
		linkname = NULL;
#endif
		if (!CHARncmp(cp, "file", 4))
		{
			cp += 3;
			arg = o_filename(markbuffer(win->cursor));
			if (!arg)
				arg = o_bufname(markbuffer(win->cursor));
		}
#ifdef FEATURE_SHOWTAG
		else if (!CHARncmp(cp, "tag", 3))
		{
			arg = telabel(win->state->cursor);
		}
#endif
		else if (!CHARncmp(cp, "cmd", 3))
		{
			arg = win->cmdchars;
		}
		else if (!CHARncmp(cp, "face", 4))
		{
			if (win->di->cursface < colornpermanent)
				arg = colorinfo[(int)win->di->cursface].name;
		}
#ifdef DISPLAY_HTML
		else if (!CHARncmp(cp, "link", 4))
		{
			if (win->md->tagatcursor != NULL
			 && win->md->tagnext == dmhtml.tagnext)
				arg = (*win->md->tagatcursor)(win, win->cursor);
			linkname = arg;
		}
#endif
#ifdef FEATURE_SPELL
		else if (o_spell(markbuffer(win->cursor))
		      && !CHARncmp(cp, "spell", 5))
		{
			i = win->di->cursrow * win->di->columns + win->di->curscol;
			arg = spellshow(win->cursor, win->di->newfont[i]);
		}
#endif
#ifdef FEATURE_REGION
		else if (!CHARncmp(cp, "region", 6))
		{
			region = regionfind(win->state->cursor);
			if (region)
				arg = region->comment;
		}
#endif
		else if (*cp == '/')
		{
			/* If '/' and we've found anything to show, then
			 * skip the rest
			 */
			if (build != left)
				break;
			else
				continue;
		}
		else
			continue;

		/* if this item is empty or too large, skip it */
		if (!arg || !*arg || build + CHARlen(arg) >= left + QTY(left) - 1)
			continue;

		/* add a space between items (but not before first item) */
		if (build != left)
			*build++ = ' ';

		/* add this item */
		CHARcpy(build, arg);
		build += CHARlen(build);

#ifdef DISPLAY_HTML
		/* if supposed to free it, then do so */
		if (linkname)
			safefree(linkname);
#endif
	}
	*build = '\0';

	/* were there any rows inserted or deleted? */
	for (i = 0; !win->di->newmsg && i < o_lines(win) - 1 && win->di->newrow[i].insrows == 0; i++)
	{
	}
	base = (o_lines(win) - 1) * o_columns(win);
	if (!win->di->newmsg && (win->di->tmpmsg || i < o_lines(win) - 1))
	{
		/* Some rows were inserted/deleted, and there are no unread
		 * messages on the last row; Assume the last row should be
		 * wiped out.
		 */
		for (i = o_columns(win); --i >= 0; )
		{
			win->di->newchar[base + i] = ' ';
			win->di->newfont[base + i] = 0;
		}
	}
	else
	{
		/* No unread messages, and no rows inserted/deleted;
		 * Assume most of the last row is unchanged.
		 */
		for (i = o_columns(win); --i >= 0; )
		{
			win->di->newchar[base + i] = win->di->curchar[base + i];
#if 0 /*  !!! how to convert this? */
			win->di->newfont[base + i] = win->di->curfont[base + i];
#else
			win->di->newfont[base + i] = 0;
#endif
		}
	}

	/* if colors changed, then we need to redraw last line from scratch */
	if (win->di->logic != DRAW_NORMAL)
	{
		guimoveto(win, 0, win->di->rows - 1);
		guiclrtoeol(win);
		for (i = o_columns(win); --i >= 0; )
		{
			win->di->curchar[base + i] = ' ';
			win->di->curattr[base + i].bits = ~0;
		}
	}

	/* does the GUI have a status function? */
	if (!gui->status || !(*gui->status)(win->gw, left, 
		markline(win->state->cursor),
		(*win->md->mark2col)(win, win->state->cursor, viiscmd(win)) + 1,
		maplrnchar(o_modified(markbuffer(win->cursor)) ? '*' : ','),
		win->state->modename))
	{
		/* no, but maybe show status on the window's bottom row */

		/* show as much of the left message as possible, unless we're
		 * in the middle of an incremental search.  During incsearch,
		 * we don't want to cover the search pattern there.
		 */
		if (!win->di->newmsg
#ifdef FEATURE_INCSEARCH
			&& CHARcmp(win->state->modename, toCHAR("IncSrch"))
#endif
		)
		{
			i = base;
			for (j = 0; j < o_columns(win) - 1 && left[j]; j++, i++)
			{
				win->di->newchar[i] = left[j];
				win->di->newfont[i] = 0;
			}
			for (; j < o_columns(win); j++, i++)
			{
				win->di->newchar[i] = ' ';
				win->di->newfont[i] = 0;
			}
		}

		/* if "showmode", then show it */
		if (o_showmode(win))
		{
			scan = win->state->modename;
			for (i = base + o_columns(win) - 10; i < base + o_columns(win) - 3; i++)
			{
				win->di->newchar[i] = *scan++;
				win->di->newfont[i] = COLOR_FONT_SHOWMODE;
				if (!*scan)
				{
					scan = " ";
				}
			}
		}

		/* if "ruler", then show it */
		if (o_ruler(win) && o_bufchars(markbuffer(win->cursor)) > 0)
		{
			sprintf(buf, " %6ld%c%-4ld",
				markline(win->state->cursor),
				maplrnchar(o_modified(markbuffer(win->cursor)) ? '*' : ','),
				(*win->md->mark2col)(win, win->state->cursor, viiscmd(win)) + 1);
	
			scan = buf;
			for (i = base + o_columns(win) - 10 - strlen(buf); *scan; i++)
			{
				win->di->newchar[i] = *scan++;
				win->di->newfont[i] = COLOR_FONT_RULER;
			}
		}
		else
		{
			/* show the maplearn character anyway */
			i = base + o_columns(win) - 15;
			win->di->newchar[i] = maplrnchar(o_modified(markbuffer(win->cursor)) ? '*' : ',');
			if (win->di->newchar[i] == ',')
				win->di->newchar[i] = ' ';
			win->di->newfont[i] = COLOR_FONT_RULER;
		}

	}

	/* the last row is never inserted/deleted/shifted */
	win->di->newrow[o_lines(win) - 1].insrows = 0;
	win->di->newrow[o_lines(win) - 1].inschars = 0;
	win->di->newrow[o_lines(win) - 1].shiftright = 0;

	/* reset flags, in preparation for next update */
	win->di->newmsg = ElvFalse;
	win->di->tmpmsg = ElvFalse;
}


#ifdef FEATURE_MISC
/* This function checks to see if we can skip regenerating a window image.
 * If not, then it returns ElvFalse without doing anything...
 * 
 * But if we can, it updates the bottom row (status line) and moves the cursor
 * to where it should be, and then returns ElvTrue.
 */
static ELVBOOL drawquick(win)
	WINDOW	win;	/* window to be updated */
{
	DRAWINFO *di = win->di;
	long	cursoff = markoffset(win->cursor);
	int	i, j, col, base;

	/* if long lines wrap onto multiple rows, adjust curscol accordingly */
	col = di->curscol;
	if (o_number(win))
		col += 8;
	if (o_wrap(win))
		col %= di->columns;
	else
		col -= di->skipped;

	/* check for obvious reasons that we might need to redraw */
	if (!o_optimize ||					/* user doesn't want to optimize */
	    !win->md->canopt ||					/* display mode doesn't support optimization */
	    di->logic != DRAW_NORMAL ||				/* last command made a subtle change */
	    di->drawstate != DRAW_VISUAL ||			/* screen doesn't already show image */
	    markbuffer(win->cursor)!=markbuffer(di->topmark) ||	/* cursor is in different buffer */
	    di->curchgs != markbuffer(win->cursor)->changes ||	/* last command made a normal change */
	    cursoff < markoffset(di->topmark) ||		/* cursor off top of screen */
	    cursoff >= markoffset(di->bottommark) ||		/* cursor off bottom of screen */
	    col < 0 ||						/* cursor off left edge of screen */
	    col >= di->columns ||				/* cursor off right edge of screen */
	    win->seltop)					/* visually selecting text */
	{
		return ElvFalse;
	}

	/* scan all rows at that column, looking for the cursor's offset */
	for (i = 0, j = col; di->offsets[j] != cursoff; i++, j += di->columns)
	{
		if (i >= di->rows - 2)
			return ElvFalse;
	}

	/* Hooray!  We can skip generating a new image */
	di->cursrow = i;
	di->curscol = col;

	/* Except for the last line */
	genlastrow(win);
	for (col = 0, i = base = (di->rows - 1) * di->columns, j = -1;
	     col < di->columns - 1;
	     col++, i++)
	{
		if (!drawnochange(di, i))
		{
			if (j < 0)
			{
				j = i;
			}
			else if (di->newfont[j] != di->newfont[i])
			{
				guimoveto(win, j - base, di->rows - 1);
				(void)guidraw(win, di->newfont[j], &di->newchar[j], i - j, 0);
				j = i;
			}
		}
		else if (j > 0)
		{
			guimoveto(win, j - base, di->rows - 1);
			(void)guidraw(win, di->newfont[j], &di->newchar[j], i - j, 0);
			j = -1;
		}
	}
	if (j > 0)
	{
		guimoveto(win, j - base, di->rows - 1);
		(void)guidraw(win, di->newfont[j], &di->newchar[j], i - j, 0);
		j = -1;
	}
	memcpy(&di->curchar[base], &di->newchar[base], di->columns * sizeof(*di->curchar));
	for (i = 0; i < di->columns; i++)
		di->curattr[base + 1] = colorinfo[(int)di->newfont[base + i]].da;

	/* leave the cursor in the right place */
	guimoveto(win, di->curscol, di->cursrow);

	/* That should do it */
	return ElvTrue;
}
#endif /* FEATURE_MISC */


/* update the image of a window to reflect changes to its buffer */
void drawimage(win)
	WINDOW	win;	/* window to be updated */
{
	MARKBUF first, last;
	MARK	next;
	int	i, row;
	CHAR	tmpch;
	DRAWLOGIC nextlogic = DRAW_NORMAL;

	assert(gui->moveto && gui->draw);

#ifdef FEATURE_LISTCHARS
	if (!o_wrap(win))
	{
		lprecedes = dmnlistchars('<', 0L, 0L, NULL, NULL);
		lextends = dmnlistchars('>', 0L, 0L, NULL, NULL);
	}
#endif

	/* unavoidable setup performed here */
	if (o_bufchars(markbuffer(win->cursor)) > 0)
	{
		win->di->curscol = (*win->md->mark2col)(win,
						marktmp(first,
						     markbuffer(win->cursor),
						     markoffset(win->cursor)),
						viiscmd(win));
	}
	else
	{
		win->di->curscol = 0;
	}
	if (win->di->curbuf != markbuffer(win->cursor))
	{
		/* place the cursor's line at the center of the screen */
		markset(win->di->topmark, dispmove(win, -(o_lines(win) / 2), 0));
		marksetbuffer(win->di->bottommark, markbuffer(win->cursor));
		marksetoffset(win->di->bottommark, o_bufchars(markbuffer(win->cursor)));
		win->di->logic = DRAW_CENTER;
		win->di->curbuf = markbuffer(win->cursor);
	}

	/* make sure we use the right colors */
	if (guicolorsync(win))
		win->di->logic = DRAW_SCRATCH;

#ifdef FEATURE_MISC
	/* see if maybe everything else *is* avoidable*/
	if (
#ifdef FEATURE_HLOBJECT
	    hlprep(win, markbuffer(win->cursor)) &&
#endif
	    drawquick(win))
	{
		return;
	}
#endif /* FEATURE_MISC */

	/* setup, and choose a starting point for the drawing */
	colorsetup();
	next = (*win->md->setup)(win, win->di->topmark,
		markoffset(win->cursor), win->di->bottommark, win->mi);
	thiswin = win;
	thiscell = 0;
	thiscol = 0;
	maxrow = o_lines(win) - 1;
	maxcell = maxrow * o_columns(win);
	scrollrows = (gui->scrolllast ? maxrow + 1 : maxrow);
	linesshown = 0;
	if (o_number(win))
	{
		win->di->curscol += 8; /* since line numbers will push text to the right */
	}
	if (o_wrap(win))
	{
		/* we're doing traditional vi line wrapping */
		leftcol = 0;
		for (i = 0; i < maxrow; i++)
		{
			thiswin->di->newrow[i].insrows =
				thiswin->di->newrow[i].shiftright =
				thiswin->di->newrow[i].inschars = 0;
		}
		win->di->curscol %= o_columns(win);
	}
	else
	{
		/* we're doing side-scroll.  Adjust leftcol, if necessary, to
		 * make sure the cursor will be visible after drawing.
		 */
		leftcol = win->di->skipped;
		i = win->di->curscol;
#ifdef FEATURE_LISTCHARS
		while (i >= leftcol + o_columns(win) - lextends)
#else
		while (i >= leftcol + o_columns(win))
#endif
		{
			leftcol += o_sidescroll(win);
		}
		if (o_number(win))
			i -= 8; /* because line number pushed it over 8 cols */
#ifdef FEATURE_LISTCHARS
		while (leftcol > 0 && i < leftcol + lprecedes)
#else
		while (i < leftcol)
#endif
		{
			leftcol -= o_sidescroll(win);
		}
		if (leftcol < 0)
		{
			leftcol = 0;
		}
		for (i = 0; i < maxrow; i++)
		{
			thiswin->di->newrow[i].insrows =
				thiswin->di->newrow[i].inschars = 0;
			thiswin->di->newrow[i].shiftright = win->di->skipped - leftcol;
		}
		win->di->skipped = leftcol;
		win->di->curscol -= leftcol;
	}
	win->di->cursrow = -1;
	seloffset = -1;

	/* are we going to try and center the cursor? */
	if (win->di->logic == DRAW_CENTER)
		wantcurs = win->di->rows / 2;
	else
		wantcurs = win->di->rows;

	/* draw each line until we reach the last row */
	markset(win->di->topmark, next);
	for (row = 0; thiscell < maxcell || win->di->cursrow < 0 || (win->di->cursrow > wantcurs && markoffset(next) < o_bufchars(markbuffer(win->cursor))); )
	{
		/* set column limits, taking o_sidescroll into consideration */
		if (o_wrap(win))
		{
			rightcol = maxcell - thiscell;
		}
		else
		{
			rightcol = leftcol + o_columns(win);
		}

		/* if we're at the end of the buffer, pad with tildes */
		if (markoffset(next) >= o_bufchars(markbuffer(win->cursor)))
		{
			/* never show numbers on ~ lines */
			if (o_number(win))
				strcpy(thislnumstr, "        ");
			thiscol = leftcol;
#ifdef FEATURE_REGION
			thisregion = NULL;
#endif
			if (row != 0)
			{
				tmpch = '~';
				drawchar(&tmpch, 1, COLOR_FONT_NONTEXT, -1);
			}
			tmpch = '\n';
			drawchar(&tmpch, 1, COLOR_FONT_NONTEXT, -1);
			if (win->di->cursrow < 0)
			{
				win->di->cursrow = row;
			}
			win->di->newrow[row++].lineoffset = o_bufchars(markbuffer(win->cursor));
		}
		else
		{
			/* if we show line numbers, then convert this line's
			 * offset into a line number in ASCII format.
			 */
			if (o_number(win))
				sprintf(thislnumstr, "%6ld  ", markline(next));

			/* draw the line & remember its width */
			thisline = markoffset(next);
			thiswin->di->newline[linesshown].width = 0;
			thisscroll = 0;
#ifdef FEATURE_LISTCHARS
			lcscell = thiscell;
#endif
#ifdef FEATURE_REGION
			thisregion = regionfind(next);
#endif
			next = (*win->md->image)(win, next, win->mi, drawchar);

#ifdef FEATURE_LISTCHARS
			/* add arrows, if necessary */
			if (!o_wrap(win))
			{
				int	lcswidth;
				lcswidth = thiswin->di->newline[linesshown].width;
				if (lprecedes > 0 && leftcol > 0 && lcswidth > 0)
				{
					/* add a left arrow at front of line */
					(void)dmnlistchars('<', thisline, 0L, NULL, lcsdrawchar);
				}
				if (lextends > 0 && lcswidth > rightcol)
				{
					/* add a right arrow at end of line */
					lcscell = thiscell - lextends;
					(void)dmnlistchars('>', markoffset(next) - 1L, 0L, NULL, lcsdrawchar);
				}
			}
#endif /* FEATURE_LISTCHARS */

			/* remember where each row started. */
			row -= thisscroll;
			if (row < 0)
			{
				row = 0;
				nextlogic = DRAW_CHANGED;
			}
			for (row = (row > thisscroll) ? row - thisscroll : 0;
			     row < thiscell / o_columns(win);
			     row++)
			{
				win->di->newrow[row].lineoffset = thisline;
			}

			/* If the cursor probably should have been on this line,
			 * then set cursrow to the row we just finished, even if
			 * we didn't see the exact character where it belongs.
			 */
			if (thiswin->di->cursrow < 0 && markoffset(thiswin->cursor) < markoffset(next))
			{
				assert(row > 0);
				thiswin->di->cursrow = row - 1;
			}
		}
	}
	markset(win->di->bottommark, next);
	assert(linesshown > 0 && linesshown <= maxrow);

	/* update the scrollbar */
	if (gui->scrollbar)
	{
		if (win->md->move == dmnormal.move)
		{
			(*gui->scrollbar)(win->gw,
				markline(marktmp(first, markbuffer(win->cursor), win->di->newline[0].start)) - 1,
				markline(marktmp(last, markbuffer(win->cursor), markoffset(next))) - 1,
				o_buflines(markbuffer(win->cursor)));
		}
		else
		{
			(*gui->scrollbar)(win->gw, win->di->newline[0].start,
				markoffset(next), o_bufchars(markbuffer(next)));
		}
		guiflush();
	}

#ifdef FEATURE_SPELL
	spellhighlight(win);
#endif

#ifdef FEATURE_HLSEARCH
	/* highlight any text that matches the most recent search */
	searchhighlight(win, linesshown, markoffset(next));
#endif

	/* compute insert/delete info */
	compareimage(win);

	/* figure out what the last row looks like */
	genlastrow(win);

	/* copy the image to the window */
	updateimage(win);

	/* remember some final info about this image, to help us update it
	 * efficiently next time.
	 */
	win->di->logic = nextlogic;
	marksetoffset(win->di->topmark, win->di->newline[0].start);
	markset(win->di->bottommark, next);
	win->di->nlines = linesshown;
	win->di->curnbytes = o_bufchars(markbuffer(next));
	win->di->curchgs = markbuffer(next)->changes;
	for (i = 0; i < linesshown; i++)
	{
		win->di->curline[i] = win->di->newline[i];
	}

	/* we're left in visual mode */
	win->di->drawstate = DRAW_VISUAL;

	/* leave the cursor in the right place */
	guimoveto(win, win->di->curscol, win->di->cursrow);
}

/*----------------------------------------------------------------------------*/
/* The following are line-oriented output functions */

/* This function either calls gui->textline, or simulates it via the usual
 * screen update functions.
 */
static void opentextline(win, text, len)
	WINDOW	win;	/* window where text line is to be output */
	CHAR	*text;	/* text to be output */
	int	len;	/* length of text */
{
	int	i, j;

	/* if the GUI has a textline function, great! */
	if (gui->textline)
	{
		(*gui->textline)(win->gw, text, len);
		return;
	}

	/* else we have to simulate textline */

	/* for each character... */
	for (i = 0; i < len; i++)
	{
		/* process this character */
		switch (text[i])
		{
		  case '\b':
			if (win->di->opencell > 0)
				win->di->opencell--;
			thiscol = win->di->opencell % win->di->columns;
			break;

		  case '\t':
			do
			{
				win->di->newchar[win->di->opencell] = ' ';
				win->di->newfont[win->di->opencell] = COLOR_FONT_BOTTOM;
				win->di->opencell++;
				thiscol++;
			} while (thiscol < win->di->columns && thiscol % 8 != 0);
			break;
				
		  case '\r':
			win->di->opencell -= thiscol;
			thiscol = 0;
			break;

		  case '\n':
			win->di->opencell += win->di->columns;
			break;

		  case '\f':
		  case '\013':	/* VTAB character */
			/* ignore */
			break;

		  default:
			win->di->newchar[win->di->opencell] = text[i];
			win->di->newfont[win->di->opencell] = COLOR_FONT_BOTTOM;
			win->di->opencell++;
			thiscol++;
		}

		/* protect thiscol against wrap-around */
		if (thiscol >= win->di->columns)
		{
			thiscol %= win->di->columns;
		}

		/* if reached end of window, then scroll */
		while (win->di->opencell >= win->di->rows * win->di->columns)
		{
			delimage(win->di->newchar,
				win->di->newfont,
				NULL,
				win->di->columns,
				win->di->rows * win->di->columns);
			if (win->di->logic != DRAW_SCRATCH)
				win->di->newrow[0].insrows--;
			win->di->opencell -= win->di->columns;
#if 0
			/* This assertion seems to fail (meaninglessly) if you
			 * backspace around the end of the line.
			 */
			assert((win->di->opencell - thiscol) % win->di->columns == 0);
#endif
			for (j = win->di->opencell - thiscol;
			     j < win->di->rows * win->di->columns;
			     j++)
			{
				win->di->newchar[j] = ' ';
				win->di->newfont[j] = COLOR_FONT_BOTTOM;
			}
		}
	}

	/* set the cursor coordinates */
	win->di->curscol = thiscol;
	win->di->cursrow = win->di->opencell / win->di->columns;
}

/* This function is called before outputting a different line.  If the screen
 * is in open-edit state, then the current line is output along with a newline.
 * If the screen is in visual state, then the cursor is moved to the bottom of
 * the screen and its line is cleared.  Otherwise it has no effect.
 */
void drawopencomplete(win)
	WINDOW	win;	/* window where output will take place */
{
	int	len, i;

	switch (win->di->drawstate)
	{
	  case DRAW_VISUAL:
		/* SET UP FOR SCROLLING */

		/* row hints initially say rows won't change */
		for (i = 0; i < win->di->rows; i++)
		{
			/* win->di->newrow[i].lineoffset remains unchanged */
			win->di->newrow[i].insrows =
				win->di->newrow[i].shiftright =
				win->di->newrow[i].inschars = 0;
		}

		/* move to the bottom row */
		win->di->opencell = win->di->rows * win->di->columns - win->di->columns;
		thiscol = 0;

		/* clear the bottom row */
		for (i = win->di->opencell; i < win->di->rows * win->di->columns; i++)
		{
			win->di->newchar[i] = ' ';
			win->di->newfont[i] = COLOR_FONT_BOTTOM;
		}
		break;

	  case DRAW_OPENEDIT:
		thiscol = win->di->opencell % win->di->columns;
		len = win->di->opencnt - win->di->opencursor;
		if (len > 0)
		{
			opentextline(win, win->di->openimage + win->di->opencursor, len);
		}
		opentextline(win, toCHAR("\r\n"), 2);
		break;

	  case DRAW_OPENOUTPUT:
		thiscol = win->di->opencell % win->di->columns;
		break;

	  case DRAW_VMSG:
		/* scroll upward, and leave cursor on bottom */
		win->di->opencell = (win->di->rows - 1) * win->di->columns;
		thiscol = 0;
		win->di->newrow[0].insrows = 0;
		opentextline(win, toCHAR("\r\n"), 2);
		break;
	}

	/* either way, the current image is gone */
	if (win->di->openimage)
	{
		safefree(win->di->openimage);
		win->di->openimage = NULL;
	}
	if (win->di->openline)
	{
		markfree(win->di->openline);
		win->di->openline = NULL;
	}
	win->di->drawstate = DRAW_OPENOUTPUT;
}

/* Draw a message on the bottom row of a window.  If the previous message's
 * importance is MSG_STATUS, then overwrite; else append.  Handle "[more]"
 * prompts.
 */
void drawmsg(win, imp, verbose, len)
	WINDOW	win;		/* window that the message pertains to */
	MSGIMP	imp;		/* type of message */
	CHAR	*verbose;	/* text of message */
	int	len;		/* length of text */
{
	int	i, base;

	/* if the window is in open mode, then just write the message with
	 * a newline.
	 */
	if (win->di->drawstate != DRAW_VISUAL)
	{
		drawextext(win, verbose, len);
		drawextext(win, toCHAR("\n"), 1);
	}
	else /* full-screen */
	{
		/* draw the message on the bottom row */
		/* for now, any message overwrites any other */
		base = (o_lines(win) - 1) * o_columns(win);
		for (i = 0; i < o_columns(win); i++)
		{
			if (i < len)
			{
				win->di->newchar[base + i] = win->di->curchar[base + i] = verbose[i];
			}
			else
			{
				win->di->newchar[base + i] = win->di->curchar[base + i] = ' ';
			}
			win->di->newfont[base + i] = COLOR_FONT_BOTTOM;
			win->di->curattr[base + i] = colorinfo[COLOR_FONT_BOTTOM].da;
		}
		drawexpose(win, (int)(o_lines(win)-1), 0, (int)(o_lines(win)-1), (int)(o_columns(win)-1));
		win->di->newmsg = ElvTrue;
	}
	
	/* non-status messages force us out of DRAW_VISUAL state */
	if (imp != MSG_STATUS && win->di->logic != DRAW_SCRATCH && win->di->drawstate == DRAW_VISUAL)
	{
		win->di->drawstate = DRAW_VMSG;
	}
}

/* these are used while generating a line of text */
static long	opencnt;	/* column where next image character goes */
static long	openoffsetcurs;	/* offset of cursor */
static CHAR	*openimage;	/* buffer, holds new line image */
static long	opensize;	/* size of openimage */
static ELVBOOL	opencursfound;	/* has the cursor's cell been found yet? */
static ELVBOOL	openskipping;	/* has the line containing the cursor been completed? */
static ELVBOOL	openselect;	/* currently outputting highlighted text? */
static long	openseltop;	/* offset of top of highlighted region */
static long	openselbottom;	/* offset of bottom of highlighted region */

static void openchar(p, qty, font, offset)
	CHAR	*p;	/* characters to be output */
	long	qty;	/* number of characters */
	_char_	font;	/* font code of that character */
	long	offset;	/* buffer offset of the character, or -1 if not from buffer */
{
	register CHAR ch;
	CHAR	*newp;
	long	delta;

	/* negative qty indicates that the same character is repeated */
	if (qty < 0)
	{
		delta = 0;
		qty = -qty;
	}
	else
	{
		delta = 1;
	}

	/* for each character */
	for ( ; qty > 0; qty--, p += delta, offset += delta)
	{
		/* copy *p into a register variable */
		ch = *p;

		/* ignore formfeed and VT, and extra lines */
		if (ch == '\f' || ch == '\013' || openskipping)
		{
			continue;
		}

		/* is the cursor in this line? */
		if (offset >= openoffsetcurs)
		{
			opencursfound = ElvTrue;
		}

		/* expand the size of the buffer, if necessary */
		if (opencnt + 2 >= opensize)
		{
			newp = (CHAR *)safealloc((int)(opensize + 80), sizeof(CHAR));
			memcpy(newp, openimage, (size_t)opensize);
			safefree(openimage);
			openimage = newp;
			opensize += 80;
		}

		/* if starting the highlighted text, output "*[" */
		if (offset >= openseltop && offset <= openselbottom)
		{
			if (!openselect)
			{
				openimage[opencnt++] = '*';
				openimage[opencnt++] = '[';
				openselect = ElvTrue;
			}
		}
		else
		{
			if (openselect)
			{
				openimage[opencnt++] = ']';
				openimage[opencnt++] = '*';
				openselect = ElvFalse;
			}
		}

		/* newline is generally ignored */
		if (ch == '\n')
		{
			if (opencursfound)
			{
				openskipping = ElvTrue;
			}
			else
			{
				opencnt = 0;
			}
			continue;
		}

		/* store the character */
		openimage[opencnt++] = ch;
	}
}

/* output a bunch of backspace characters or characters to move the cursor
 * one way or the other on the current line.
 */
static void openmove(win, oldcol, newcol, image, len)
	WINDOW	win;	/* window whose cursor should be moved */
	long	oldcol;	/* old position of the cursor, relative to image */
	long	newcol;	/* new position of the cursor, relative to image */
	CHAR	*image;	/* image of the line being edited */
	long	len;	/* length of the image */
{
	static CHAR	tenbs[10] = {'\b', '\b', '\b', '\b', '\b', '\b', '\b', '\b', '\b', '\b'};
	static CHAR	tensp[10] = {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '};

	/* left or right? */
	if (oldcol < newcol)
	{
		/* move right by writing characters from image */
		if (len < newcol)
		{
			/* we'll need to pad the image with blanks */
			if (oldcol < len)
			{
				opentextline(win, &image[oldcol], (int)(len - oldcol));
			}
			while (len + 10 < newcol)
			{
				opentextline(win, tensp, 10);
				len += 10;
			}
			if (len < newcol)
			{
				opentextline(win, tensp, (int)(newcol - len));
			}
		}
		else
		{
			/* don't pad with blanks; just write characters */
			opentextline(win, &image[oldcol], (int)(newcol - oldcol));
		}
	}
	else if (newcol < oldcol)
	{
		/* move right by outputting backspaces */
		while (newcol + 10 < oldcol)
		{
			opentextline(win, tenbs, 10);
			oldcol -= 10;
		}
		if (newcol < oldcol)
		{
			opentextline(win, tenbs, (int)(oldcol - newcol));
		}
	}
	/* else no movement was necessary */
}

/* draw the window's current line from scratch, or just update it, in open mode. */
void drawopenedit(win)
	WINDOW	win;	/* window whose current line is to be output */
{
	MARK	curline;
	long	curcol;
	long	first, last;	/* differences */
	long	i, max;

	/* find the start of the cursor's line & its column */
	curline = dispmove(win, 0, 0);
	curcol = dispmark2col(win);

	/* set up some variables that updateimage() needs */
	thiswin = win;
	thiscell = 0;
	thiscol = 0;
	maxrow = o_lines(win) - 1;
	maxcell = maxrow * o_columns(win);
	scrollrows = (gui->scrolllast ? maxrow + 1 : maxrow);
	linesshown = 0;

	/* if we were editing a different line before, then end it now */
	if (win->di->drawstate != DRAW_OPENEDIT
	 || !win->di->openline
	 || markbuffer(win->di->openline) != markbuffer(curline)
	 || (markoffset(win->di->openline) != markoffset(curline)
		&& !win->state->acton))
	{
		drawopencomplete(win);
		curline = win->di->openline = markdup(curline);
	}
	/* else we're editing the same line as last time */

	/* generate the new line image */
	opencursfound = openskipping = openselect = ElvFalse;
	opencnt = 0;
	openoffsetcurs = markoffset(win->state->cursor);
	openimage = (CHAR *)safealloc(80, sizeof(CHAR));
	opensize = 80;
	openseltop = openselbottom = INFINITY;
	if (win->state->acton)
	{
		curline = (*dmnormal.setup)(win, curline, openoffsetcurs, win->state->cursor, win->mi);
		curline = (*dmnormal.image)(win, curline, win->mi, openchar);
	}
	else
	{
		if (win->seltop)
		{
			openseltop = markoffset(win->seltop);
			openselbottom = markoffset(win->selbottom);
			curcol += 2; /* for the *[ marker */
		}
		curline = (*win->md->setup)(win, curline, openoffsetcurs,
						win->state->cursor, win->mi);
		curline = (*win->md->image)(win, curline, win->mi, openchar);
	}

	/* do we have an old image? */
	if (win->di->openimage)
	{
		/* find the differences */
		first = last = -1;
		max = (opencnt > win->di->opencnt) ? opencnt : win->di->opencnt;
		for (i = 0; i < max; i++)
		{
			if (i < opencnt && win->di->openimage[i] == openimage[i])
			{
				continue;
			}
			if (first < 0)
			{
				first = i;
			}
			last = i;
		}
	}
	else
	{
		/* the whole line is different */
		first = 0;
		last = opencnt - 1;
		win->di->opencursor = 0;
	}

	/* output the different parts of the line */
	if (first >= 0)
	{
		openmove(win, win->di->opencursor, first, openimage, opencnt);
		openmove(win, first, last + 1, openimage, opencnt);
		win->di->opencursor = last + 1;
	}

	/* move the cursor back where it wants to be */
	openmove(win, win->di->opencursor, curcol, openimage, opencnt);
	win->di->opencursor = curcol;

	/* remember this line */
	if (win->di->openimage)
	{
		safefree(win->di->openimage);
	}
	win->di->openimage = openimage;
	win->di->opencnt = opencnt;

	/* this leaves us in open+edit state */
	win->di->drawstate = DRAW_OPENEDIT;

	/* if we're simulating the textline function, then we need to flush
	 * the image to the window.
	 */
	if (!gui->textline)
	{
		colorsetup();
		updateimage(win);
		win->di->curscol = win->di->opencell % win->di->columns;
		win->di->cursrow = win->di->opencell / win->di->columns;
		guimoveto(win, win->di->opencell % win->di->columns, win->di->cursrow);
		win->di->newrow[0].insrows = 0;
	}
}


/* ex commands use this function to output information */
void drawextext(win, text, len)
	WINDOW	win;	/* window where ex output should be drawn */
	CHAR	*text;	/* text to be output */
	int	len;	/* length of text */
{
	int	i, width;

	/* if no window specified, then throw away the text */
	if (!win || eventcounter <= 1)
	{
		fwrite(tochar8(text), len, sizeof(char), stdout);
		if (text[len - 1] == '\n')
		{
			fwrite("\r", 1, sizeof(char), stdout);
		}
		return;
	}

	/* if we were editing a line before, finish it now */
	drawopencomplete(win);

	/* output the text one line at a time, inserting CR before each LF */
	for (i = 0; i < len; i += width + 1)
	{
		for (width = 0; i + width < len && text[i + width] != '\n'; width++)
		{
		}
		if (width > 0)
		{
			opentextline(win, text + i, width);
		}
		if (i + width < len)
		{
			opentextline(win, toCHAR("\r\n"), 2);
		}
	}

	/* flush the image to the screen */
	if (o_exrefresh)
	{
		updateimage(win);
		win->di->curscol = win->di->opencell % win->di->columns;
		win->di->cursrow = win->di->opencell / win->di->columns;
		guimoveto(win, win->di->curscol, win->di->cursrow);
		win->di->newrow[0].insrows = 0;
		if (gui->flush)
		{
			(*gui->flush)();
		}
	}
}

/* This is like drawextext, except that here we're careful about making
 * control characters (other than newline) visible.
 */
void drawexlist(win, text, len)
	WINDOW	win;	/* window where ex output should be drawn */
	CHAR	*text;	/* text to be output */
	int	len;	/* length of text */
{
	CHAR	buf[2];
	int	from, to;

	/* divide the text into segments of all-printable characters, or
	 * or individual non-printable characters.
	 */
	for (from = to = 0; to < len; to++)
	{
		if (text[to] != '\n' && elvcntrl(text[to]))
		{
			/* draw the last printable segment, if any */
			if (from < to)
				drawextext(win, &text[from], to - from);
			from = to + 1;

			/* draw this control char in a printable format */
			buf[0] = '^';
			buf[1] = text[to] ^ '@';
			drawextext(win, buf, 2);
		}
	}

	/* if we ended with a printable segment, then draw it now */
	if (from < to)
		drawextext(win, &text[from], to - from);
}
