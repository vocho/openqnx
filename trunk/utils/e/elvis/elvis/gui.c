/* gui.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_gui[] = "$Id: gui.c,v 2.32 2003/10/19 19:19:53 steve Exp $";
#endif

/* This is a pointer to the chosen GUI. */
GUI *gui;

/* This function chooses a font to combine with "normal", to use for coloring
 * normal text.  This offers a way to change the background color, or some
 * other character attribute, to reflect conditions such as input focus or
 * readonly files.  This should be called before drawing or exposing areas.
 */
ELVBOOL guicolorsync(win)
	WINDOW	win;
{
	unsigned short oldbits;
	ELVBOOL retval = ElvFalse;

	/* choose a font to combine with "normal" for this update */
	oldbits = colorinfo[win->defaultfont].da.bits;
	win->defaultfont = o_hasfocus(win) ? COLOR_FONT_NORMAL
					   : COLOR_FONT_IDLE;

	/* if colors have changed, then update the GUI */
	if (win->fgcolor != colorinfo[win->defaultfont].fg)
	{
		win->fgcolor = colorinfo[win->defaultfont].fg;
		retval = ElvTrue;
	}
	if (win->bgcolor != colorinfo[win->defaultfont].bg)
	{
		win->bgcolor = colorinfo[win->defaultfont].bg;
		if (gui->setbg)
			(*gui->setbg)(win->gw, win->bgcolor);
		retval = ElvTrue;
	}

	/* even if colors haven't changed, if the attributes are different
	 * then we still want to force a redraw.
	 */
	if (((oldbits ^ colorinfo[win->defaultfont].da.bits)
	 & (COLOR_BOLD|COLOR_ITALIC|COLOR_UNDERLINED|COLOR_BOXED)) != 0)
	{
		retval = ElvTrue;
	}

	return retval;
}

/* This function calls the GUI's moveto() function.  This function performs a
 * tiny amount of optimization, however: if the cursor is already in the
 * correct position, it does nothing.
 */
void guimoveto(win, column, row)
	WINDOW	win;	/* window whose cursor is to be moved */
	int	column;	/* column where cursor should be placed */
	int	row;	/* row where cursor should be placed */
{
	/* perform the moveto */
	if (gui->moveto)
	{
		(*gui->moveto)(win->gw, column, row);
		win->cursx = column;
		win->cursy = row;
	}
}

/* This function calls the GUI's draw() function, and then updates elvis'
 * own idea of where the cursor is.  The guimove() function depends on your
 * calling guidraw() instead of (*gui->draw)().
 */
DRAWATTR *guidraw(win, font, text, len, forcebits)
	WINDOW	win;	/* window where text is to be drawn */
	_char_	font;	/* font to use for drawing */
	CHAR	*text;	/* text to be drawn */
	int	len;	/* number of characters in text */
	int	forcebits; /* attributes to add to the font's defaults */
{
	COLORINFO *combo;
 static	DRAWATTR  da;

	/* Combine font attributes, so that any attributes that are unset in
	 * this font will be inherited from the normal font.  Also factor in
	 * the selection font, if appropriate.
	 */
	if ((unsigned)font <= 1)
		combo = &colorinfo[win->defaultfont];
	else
	{

		if (font & 0x80)
			combo = colorcombine(font & 0x7f,
					&colorinfo[COLOR_FONT_SELECTION]);
		else
			combo = &colorinfo[font];
		combo = colorcombine(win->defaultfont, combo);
	}

	/* Draw it */
	(*gui->draw)(win->gw, combo->fg, combo->bg, combo->da.bits | forcebits,
							text, len);
	win->cursx += len;

	/* return the attributes of the non-selected font, including the
	 * forcebits.  If the font is selected, then also set COLOR_SEL bit.
	 */
	da = colorinfo[font & 0x7f].da;
	da.bits = drawfontbits(font);
	da.bits |= forcebits;
	return &da;
}

/* This function calls the GUI's scroll() function, but only if the number of
 * lines to be deleted/inserted is smaller than the number of lines remaining.
 * And only if the GUI has a scroll() function, of course.
 *
 * Returns ElvTrue if the scrolling happened as requested, else ElvFalse.
 */
ELVBOOL guiscroll(win, qty, notlast)
	WINDOW	win;	/* window to be scrolled */
	int	qty;	/* rows to insert (may be negative to delete) */
	ELVBOOL	notlast;/* if ElvTrue, scrolling shouldn't affect last row */
{
	/* if there is no gui->scroll() function, or if we're trying to
	 * insert/delete too many rows, then fail.
	 */
	if (gui->scroll == NULL || abs(qty) >= o_lines(win) - win->cursy)
	{
		return ElvFalse;
	}

	/* else give the GUI a try */
	return (*gui->scroll)(win->gw, qty, notlast);
}

/* This function calls the GUI's shift() function, but only if the number of
 * characters to the right of the cursor is larger than the requested shift
 * amount.  And only if the GUI has a shift() function, of course.
 *
 * Returns ElvTrue if the shifting happened as requested, else ElvFalse.
 */
ELVBOOL guishift(win, qty, rows)
	WINDOW	win;	/* window to be shifted */
	int	qty;	/* columns to insert (may be negative to delete) */
	int	rows;	/* number of rows affected */
{
	/* if there is no gui->shift() function, or if we're trying to
	 * insert/delete too many characters, then fail.
	 */
	if (!gui->shift || abs(qty) >= o_columns(win) - win->cursx)
	{
		return ElvFalse;
	}

	/* else give the GUI a try */
	return (*gui->shift)(win->gw, qty, rows);
}

/* This function calls the GUI's cltroeol() function.  If it doesn't exist,
 * or returns ElvFalse, then this function writes enough space characters to
 * simulate a clrtoeol()
 */
void guiclrtoeol(win)
	WINDOW	win;	/* window whose row is to be cleared */
{
	int		width;
	COLORINFO	*n;

	/* if already at EOL, we're done */
	width = o_columns(win) - win->cursx;
	if (width <= 0)
	{
		return;
	}

	/* try to make the GUI do it */
	if (gui->clrtoeol == NULL || !(*gui->clrtoeol)(win->gw))
	{
		/* No, we need to do it the hard way */
		n = &colorinfo[o_hasfocus(win) ? COLOR_FONT_NORMAL : COLOR_FONT_IDLE];
		guimoveto(win, win->cursx, win->cursy);
		while (width > QTY(blanks))
		{
			(*gui->draw)(win->gw, n->fg, n->bg, n->da.bits, blanks, QTY(blanks));
			width -= QTY(blanks);
		}
		(*gui->draw)(win->gw, n->fg, n->bg, n->da.bits, blanks, width);
		guimoveto(win, win->cursx, win->cursy);
	}
}


/* This function calls the GUI's reset function (if it has one) and also
 * resets the portable GUI wrapper functions' variables.
 *
 * Why do this?  Because the wrapper functions, and some GUI drawing functions,
 * perform some internal optimizations by assuming that nothing else affects
 * the screen when we aren't looking; but when line noise corrupts the screen
 * we want ^L to force the whole screen to be redrawn, that assumption is
 * unsafe.
 */
void guireset()
{
	WINDOW	w;

	/* if the GUI has a reset function call it */
	if (gui->reset)
	{
		(*gui->reset)();
	}

	/* reset the wrapper functions' variables */
	for (w = windows; w; w = w->next)
	{
		w->cursx = w->cursy = -1;
	}
}


/* This function calls the GUI's poll() function.  If it has no poll() function
 * then this function always returns ElvFalse to indicate that the current work
 * should continue.  This function is also sensitive to the pollfrequency
 * option, to reduce the number of calls to poll() since poll() may be slow.
 *
 * Returns ElvFalse if the current work should continue, or ElvTrue if the user
 * has requested that it be cut short.
 */
ELVBOOL guipoll(reset)
	ELVBOOL	reset;	/* reset the pollctr variable? */
{
	static long	pollctr = 0;

	/* if just supposed to reset, then do that and then quit */
	if (reset)
	{
		pollctr = 0;
		if (gui && gui->poll)
			return (*gui->poll)(reset);
		else
			return ElvFalse;
	}

	/* if no GUI has been chosen yet, or there is no poll() function, or
	 * we're quitting, or pollfrequency indicates that poll() shouldn't
	 * be called yet, then return ElvFalse so the current operation will
	 * continue.
	 */
	if (!gui || !gui->poll || !windows || ++pollctr < o_pollfrequency)
	{
		return ElvFalse;
	}

	/* reset the pollctr variable */
	pollctr = 0;

	/* call the GUI's poll() function, and return its value */
	reset = (*gui->poll)(reset);
	if (reset)
	{
		msg(MSG_ERROR, "aborted");
	}
	return reset;
}

/* ring the bell, if there is one.  Limit it to one ding per eventdraw() */
void guibeep(win)
	WINDOW	win;	/* window to ding, or NULL to indicate an eventdraw() */
{
	static ELVBOOL	dingable = ElvTrue;

	if (!win)
	{
		dingable = ElvTrue;
	}
	else if (gui && gui->beep && dingable)
	{
		(*gui->beep)(win->gw);
		dingable = ElvFalse;
	}
}

/* Translate a key name or raw key sequence into both an official key name
 * and rawin sequence, and return the length of the rawin sequence.  This is
 * a wrapper around the (*gui->keylabel)() function, and it adds support for
 * some standard keys.
 */
int guikeylabel(given, givenlen, label, rawptr)
	CHAR	*given;	/* what the user typed in as a key code */
	int	givenlen;/* length of the given string */
	CHAR	**label; /* where to store a pointer to the symbolic name */
	CHAR	**rawptr;/* where to store a pointer to the raw characters */
{
	int	rawlen, labellen, i;
	CHAR	*scan;
	static CHAR *build;
	CHAR	*symlabel, *symraw;
	CHAR	ch[1];
	CHAR	first;
	CHAR	hashnotation[30];
	static struct {
		CHAR	*label;
		int	len;
		CHAR	ch;
	} keys[] = {
		{toCHAR("<Nul>"),	5,	'\0'},
		{toCHAR("<BS>"),	4,	'\b'},
		{toCHAR("<Tab>"),	5,	'\t'},
		{toCHAR("<FF>"),	4,	'\f'},
		{toCHAR("<NL>"),	4,	'\n'},
		{toCHAR("<LF>"),	4,	'\n'},
		{toCHAR("<EOL>"),	4,	'\n'},
		{toCHAR("<CR>"),	4,	'\r'},
		{toCHAR("<Return>"),	8,	'\r'},
		{toCHAR("<Enter>"),	7,	'\r'},
		{toCHAR("<Esc>"),	5,	(CHAR)0x1b},
		{toCHAR("<CSI>"),	5,	(CHAR)0x9b},
		{toCHAR("<Del>"),	5,	(CHAR)0x7f},
		{toCHAR("<Space>"),	7,	' '},
		{toCHAR("<lt>"),	4,	'<'},
		{toCHAR("<gt>"),	4,	'>'},
		{toCHAR("<Bar>"),	5,	'|'},
		{toCHAR("<Bslash>"),	8,	'\\'}
	};

	/* if looking up the label of a single character, then check the table */
	if (givenlen == 1 && label)
	{
		for (i = 0; i < QTY(keys); i++)
		{
			if (keys[i].ch == *given)
			{
				*label = keys[i].label;
				break;
			}
		}
		if (rawptr)
			*rawptr = given;
		return givenlen;
	}

	/* Allow the GUI to translate "#number" function keys */
	if (gui->keylabel && given[0] == '#')
	{
		rawlen = (*gui->keylabel)(given, givenlen, label, rawptr);
		if (rawlen > 0)
			return rawlen;
	}

	/* if there's an old raw string lying around, free it now */
	if (build)
		safefree(build);

	/* else we'll build a string by looking for <key> names */
	for (build = NULL, scan = given; scan < &given[givenlen]; scan++)
	{
		/* anything other than '<' is added literally */
		if (*scan != '<')
		{
			buildCHAR(&build, *scan);
			continue;
		}

		/* count the length of the symbolic name, up to next '>' */
		labellen = 1;
		do
		{
			/* watch out for problems */
			if (&scan[labellen] == &given[givenlen]
			 || scan[labellen] == '<'
			 || elvspace(scan[labellen]))
				break;
		} while (scan[labellen++] != '>');

		/* does it look like a symbolic name? */
		symraw = NULL;
		if (scan[labellen - 1] == '>')
		{
			/* is it a shifted of controlled function key? */
			first = elvtolower(scan[1]);
			if (labellen >= 6
			 && (first == 'c' || first == 's')
			 && scan[2] == '-'
			 && elvtolower(scan[3]) == 'f'
			 && elvdigit(scan[4])
			 && gui->keylabel)
			{
				hashnotation[0] = '#';
				memcpy(&hashnotation[1], &scan[4], (labellen - 5) * sizeof(CHAR));
				hashnotation[labellen - 4] = first;
				rawlen = (*gui->keylabel)(hashnotation, labellen - 3, &symlabel, &symraw);
			}

			/* is it a normal function key? */
			if (!symraw
			 && labellen >= 4
			 && first == 'f'
			 && elvdigit(scan[2])
			 && gui->keylabel)
			{
				hashnotation[0] = '#';
				memcpy(&hashnotation[1], &scan[2], (labellen - 3) * sizeof(CHAR));
				rawlen = (*gui->keylabel)(hashnotation, labellen - 2, &symlabel, &symraw);
			}

			/* is it a <C-X> notation? */
			if (!symraw && labellen == 5 && first == 'c' && scan[2] == '-')
			{
				if (scan[3] == '?')
					ch[0] = '\177';
				else
					ch[0] = scan[3] & 0x1f;
				symraw = ch;
				rawlen = 1;
			}

			/* is it a <M-X> or <A-X> notation? */
			if (!symraw
			 && labellen == 5
			 && (first == 'm' || first == 'a')
			 && scan[2] == '-')
			{
				ch[0] = scan[3] | 0x80;
				symraw = ch;
				rawlen = 1;
			}

			/* is this a standard symbol for a character? */
			if (!symraw)
			{
				/* scan the list of keys */
				for (i = 0;
				     i < QTY(keys)
					&& (keys[i].len != labellen
					   || CHARncasecmp(keys[i].label, scan, labellen));
				     i++)
				{
				}

				/* was a key found? */
				if (i < QTY(keys))
				{
					symraw = &keys[i].ch;
					rawlen = 1;
				}
			}

			/* if nothing standard worked, then let the GUI try */
			if (!symraw && gui->keylabel)
				rawlen = (*gui->keylabel)(scan, labellen, &symlabel, &symraw);
		}

		/* if no symbol was recognized, then use it literally */
		if (!symraw)
		{
			/* use the label characters literally */
			rawlen = labellen;
			symraw = scan;
		}

		/* add the symbol's raw chars to the buffer */
		while (rawlen-- > 0)
			buildCHAR(&build, *symraw++);

		/* move the scan variable past the symbolic name */
		scan += labellen - 1;
	}

	/* stick a <nul> on the end, and return the length without the null */
	rawlen = buildCHAR(&build, '\0') - 1;
	*rawptr = build;
		/* ...but leave (*label) unchanged */
	return rawlen;
}
