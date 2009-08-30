/* dmhex.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_dmhex[] = "$Id: dmhex.c,v 2.21 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef DISPLAY_HEX

#ifdef CHAR16
"hex mode doesn't support 16-bit characters"
#endif

#if USE_PROTOTYPES
static DMINFO *init(WINDOW win);
static void term(DMINFO *info);
static long mark2col(WINDOW w, MARK mark, ELVBOOL cmd);
static MARK move(WINDOW w, MARK from, long linedelta, long column, ELVBOOL cmd);
static MARK setup(WINDOW win, MARK top, long cursor, MARK bottom, DMINFO *info);
static MARK image(WINDOW w, MARK line, DMINFO *info, void (*draw)(CHAR *p, long qty, _char_ font, long offset));
#endif

/* Lines look like this:
_offset____0__1__2__3___4__5__6__7___8__9__a__b___c__d__e__f__0123456789abcdef
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
00000000  00 00 00 00  00 00 00 00  00 00 00 00  00 00 00 00  0000000000000000
*/

/* This array is used to convert a column number to an offset within a line */
static char col2offset[] = "00000000000011122233334445556667777888999aaabbbbcccdddeeeffff00123456789abcdef";

/* These are the font codes for the column headings and the hex cursor */
static int font_hexheading;
static int font_hexcursor;
static int font_hexoffset;

/* start the mode, and allocate dminfo */
static DMINFO *init(win)
	WINDOW	win;
{
	/* inherit some functions from dmnormal */
	if (!dmhex.wordmove)
	{
		dmhex.wordmove = dmnormal.wordmove;
		dmhex.tagatcursor = dmnormal.tagatcursor;
		dmhex.tagload = dmnormal.tagload;
		dmhex.tagnext = dmnormal.tagnext;
	}

	/* locate the fonts we'll use */
	font_hexheading = colorfind(toCHAR("hexheading"));
	colorset(font_hexheading, toCHAR("underlined"), ElvFalse);
	font_hexcursor = colorfind(toCHAR("hexcursor"));
	font_hexoffset = colorfind(toCHAR("hexoffset"));

	return NULL;
}

/* end the mode, and free the modeinfo */
static void term(info)
	DMINFO	*info;	/* window-specific info about mode */
{
}

/* Convert a mark to a screen-relative column number */
static long mark2col(w, mark, cmd)
	WINDOW	w;	/* window where buffer is shown */
	MARK	mark;	/* mark to convert */
	ELVBOOL	cmd;	/* if ElvTrue, we're in command mode; else input mode */
{
	return 62 + (markoffset(mark) & 0xf);
}

/* Move vertically, and to a given column (or as close to column as possible) */
static MARK move(w, from, linedelta, column, cmd)
	WINDOW	w;		/* window where buffer is shown */
	MARK	from;		/* old location */
	long	linedelta;	/* line movement */
	long	column;		/* desired column number */
	ELVBOOL	cmd;		/* if ElvTrue, we're in command mode; else input mode */
{
	static MARKBUF	mark;
	long		offset;
	long		coloff;

	/* compute line movement first */
	offset = (markoffset(from) & ~0xf) + 0x10 * linedelta;

	/* convert requested column to nearest buffer offset */
	if (column >= (int)strlen(col2offset))
	{
		coloff = 0xf;
	}
	else if (col2offset[column] >= '0' && col2offset[column] <= '9')
	{
		coloff = col2offset[column] - '0';
	}
	else
	{
		coloff = col2offset[column] - 'a' + 0xa;
	}
	offset += coloff;

	/* never return an offset past the end of the buffer, or before the
	 * beginning.
	 */
	if (offset >= o_bufchars(markbuffer(from)))
	{
		if (o_bufchars(markbuffer(from)) == 0)
		{
			offset = 0;
		}
		else
		{
			offset = o_bufchars(markbuffer(from)) - 1;
		}
	}
	else if (offset < 0)
	{
		offset = coloff;
	}

	return marktmp(mark, markbuffer(from), offset);
}

/* Choose a line to appear at the top of the screen, and return its mark.
 * Also, initialize the info for the next line.
 */
static MARK setup(win, top, cursor, bottom, info)
	WINDOW	win;	/* window to be updated */
	MARK	top;	/* where previous image started */
	long	cursor;	/* offset of cursor */
	MARK	bottom;	/* where previous image ended */
	DMINFO	*info;	/* window-specific info about mode */
{
	static MARKBUF	mark;
	long		topoff, bottomoff;

	/* extract the offsets.  Round down to multiple of 16 */
	topoff = (top ? (markoffset(top) & ~0xf) : -1);
	bottomoff = (bottom ? (markoffset(bottom) & ~0xf) : -1);
	cursor &= ~0xf;

	/* if cursor is on the screen, or very near the bottom, then
	 * keep the current top
	 */
	if (cursor >= topoff && cursor <= bottomoff + 16)
	{
		return marktmp(mark, markbuffer(top), topoff);
	}

	/* if the cursor is on the line before the top, then make the cursor's
	 * line become the new top line.
	 */
	if (cursor == topoff - 16)
	{
		return marktmp(mark, markbuffer(top), cursor);
	}

	/* else it is distantly before or after the the old screen.  Center
	 * the cursor's line in the screen.
 	 */
	topoff = (cursor - (bottomoff - topoff) / 2) & ~0xf;
	if (topoff < 0)
	{
		topoff = 0;
	}
	return marktmp(mark, markbuffer(top), topoff);
}

static MARK image(w, line, info, draw)
	WINDOW	w;		/* window where drawing will go */
	MARK	line;		/* start of line to draw */
	DMINFO	*info;		/* window-specific info about mode */
	void	(*draw)P_((CHAR *p, long qty, _char_ font, long offset));
				/* function for drawing a single character */
{
	char	*c8p;
	CHAR	*cp;
	CHAR	tmp[1];
	CHAR	space[1];	/* usually a space character, maybe bracket character */
	char	buf[10];
	int	i, j;

	/* output headings, if necessary */
	if ((markoffset(line) & 0xf0) == 0)
	{
		c8p = " offset    0  1  2  3   4  5  6  7   8  9  a  b   c  d  e  f  0123456789abcdef";
		while ((tmp[0] = *c8p++) != '\0')
			(*draw)(tmp, 1, font_hexheading, -1L);
		tmp[0] = '\n';
		(*draw)(tmp, 1L, 0, -1L);
	}

	/* output the line offset */
	sprintf(buf, "%08lx", markoffset(line));
	for (i = 0; buf[i]; i++)
	{
		tmp[0] = buf[i];
		(*draw)(tmp, 1, font_hexoffset, -1);
	}

	/* output the hex codes of the line */
	j = markoffset(w->cursor) - markoffset(line);
	space[0] = ' ';
	for ((void)scanalloc(&cp, line), i = 0; i < 16 && cp; scannext(&cp), i++)
	{
		/* special case: if the last newline was added by elvis
		 * (not in the file) then hide it unless the cursor is on it.
		 */
		if (o_partiallastline(markbuffer(line))
		 && (o_readeol(markbuffer(line)) == 'b' || o_writeeol == 'b')
		 && *cp == '\n'
		 && markoffset(line) + i == o_bufchars(markbuffer(line)) - 1
		 && j != i)
		{
			break;
		}

		if ((i & 0x03) == 0)
		{
			(*draw)(space, 1, 0, -1);
			space[0] = ' ';
		}
		if (j == i)
		{
			tmp[0] = '<';
			(*draw)(tmp, 1, 0, -1);
			space[0] = '>';
		}
		else
		{
			(*draw)(space, 1, 0, -1);
			space[0] = ' ';
		}

		sprintf(buf, "%02x", *cp);
		tmp[0] = buf[0];
		(*draw)(tmp, 1, (char)(j==i ? font_hexcursor : 0), markoffset(line) + i);
		tmp[0] = buf[1];
		(*draw)(tmp, 1, (char)(j==i ? font_hexcursor : 0), markoffset(line) + i);
	}
	(*draw)(space, 1, 0, -1);

	/* pad with blanks, if necessary */
	space[0] = ' ';
	while (i < 16)
	{
		(*draw)(space, ((i & 0x03) == 0) ? -4 : -3, 0, -1);
		i++;
	}
	(*draw)(space, 1, 0, -1);

	/* output the characters */
	tmp[0] = '.';
	for ((void)scanseek(&cp, line), i = 0; i < 16 && cp; scannext(&cp), i++)
	{
		/* special case: if the last newline was added by elvis
		 * (not in the file) then hide it unless the cursor is on it.
		 */
		if (o_partiallastline(markbuffer(line))
		 && (o_readeol(markbuffer(line)) == 'b' || o_writeeol == 'b')
		 && *cp == '\n'
		 && markoffset(line) + i == o_bufchars(markbuffer(line)) - 1
		 && j != i)
		{
			(*draw)(blanks, 1, 0, markoffset(line) + i);
		}
		else if (*cp < ' ' || *cp == '\177')
		{
			(*draw)(tmp, 1, 0, markoffset(line) + i);
		}
		else
		{
			(*draw)(cp, 1, 0, markoffset(line) + i);
		}
	}
	scanfree(&cp);
	tmp[0] = '\n';
	(*draw)(tmp, 1L, 0, -1L);

	/* output a blank line after every 16th data line */
	if ((markoffset(line) & 0xf0) == 0xf0)
	{
		(*draw)(tmp, 1L, 0, -1L);
	}

	markoffset(line) += i;
	return line;
}

DISPMODE dmhex =
{
	"hex",
	"Binary hex dump",
	ElvFalse,	/* display generating can't be optimized */
	ElvFalse,	/* shouldn't do normal wordwrap */
	0,	/* no window options */
	NULL,
	0,	/* no global options */
	NULL,
	NULL,
	init,
	term,
	mark2col,
	move,
	NULL,	/* wordmove will be set to dmnormal.wordmove in init() */
	setup,
	image,
	NULL,	/* doesn't need a header */
	NULL	/* no autoindent */
};
#endif /* DISPLAY_HEX */
