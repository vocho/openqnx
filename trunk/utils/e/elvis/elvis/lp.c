/* lp.c */
/* Copyright 1995 by Steve Kirkendall */


/* This file contains generic printing code. */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_lp[] = "$Id: lp.c,v 2.39 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef FEATURE_LPR

#if USE_PROTOTYPES
static LPTYPE *findtype(char *name);
# if defined (GUI_WIN32)
static void dummyprt(_CHAR_ ch);
# endif
static void prtchar(_CHAR_ ch);
static void draw(CHAR *p, long qty, _char_ font, long offset);
#endif


/* This is a list of all known printer types.  The first one is the default. */
static LPTYPE	*alltypes[] = { &lpdumb, &lpepson, &lppana, &lpibm, &lphp,
				&lpcr, &lpbs, &lpansi, &lphtml, &lpps, &lpps2,
#if defined (GUI_WIN32)
				&lpwindows,
#endif
};
static LPTYPE	*type;		/* type of printer being used at the moment */

/* Page formatting variables */
static int	pagenum;	/* page number withing this printing */
static int	linesleft;	/* number of usable lines left on this page */
static int	column;		/* column number, used to implement wrapping */
static WINDOW	prwin;		/* window doing the printing */
static ELVBOOL	anytext;	/* ElvFalse if blank page, ElvTrue if any text */
static long	lnum;		/* line number of current line */

/* Output buffering variables */
static CHAR	*iobuf;		/* pointer to the I/O buffer */
static int	iomax;		/* number of CHARS that the iobuf can hold */
static int	ionext;		/* number of CHARS currently in iobuf */

/* Return the value of a field from lpoptions, as a string */
char *lpoptfield(field, dflt)
	char	*field; /* name of the field to retrieve */
	char	*dflt;	/* default value, if not in lpoptions */
{
	int	len;
	static char buf[50];
	char	*scan;

	/* if no options, then return "" */
	if (!o_lpoptions || !*o_lpoptions)
		return dflt;

	/* look for the requested field */
	len = strlen(field);
	for (scan = tochar8(o_lpoptions); scan && *scan;)
	{
		/* is this it? */
		if (!strncmp(scan, field, len) && scan[len] == ':')
		{
			/* found! copy the value into buf */
			for (len = 0;
			     len < QTY(buf)-1 && scan[len] && scan[len] != '\0';
			     len++)
			{
				buf[len] = scan[len];
			}
			buf[len] = '\0';

			/* if default is Boolean then force Boolean values */
			if (!strcmp(dflt, "false") || !strcmp(dflt, "true"))
			{
				if (strchr("nNfF0", buf[0]))
					strcpy(buf, "false");
				else
					strcpy(buf, "true");
			}

			/* return it */
			return buf;
		}

		/* scan for the next field */
		scan = strchr(scan, ',');
		if (scan)
		{
			scan++;
			while (elvspace(*scan))
				scan++;
		}
	}

	/*not found -- return the default */
	return dflt;
}


/* This function converts a type name to an LPTYPE pointer.  Returns NULL if
 * unsuccessful.  If the name is NULL, it returns the default LPTYPE.
 */
static LPTYPE *findtype(name)
	char	*name;	/* name of printer type */
{
	int	i;

	/* if name is NULL, return the default type */
	if (name == NULL)
	{
		return alltypes[0];
	}

	/* search for the name in the list */
	for (i = 0; i < QTY(alltypes); i++)
	{
		if (!strcmp(name, alltypes[i]->name))
		{
			return alltypes[i];
		}
	}

	/* failed */
	return NULL;
}

/* return the RGB value for the foreground of a given font.  This takes into
 * consideration the "lpcolor" and "lpcontrast" options.
 */
unsigned char *lpfg(fontcode)
	_char_	fontcode;
{
	static unsigned char rgb[3];
	int	i, j;

	if (o_lpcolor)
	{
		/* fetch the RGB value from colorinfo[] */
		memcpy(rgb, colorinfo[fontcode].lpfg_rgb, 3);

		/* if the color is too pale, then darken it */
		i = rgb[0] + rgb[1] + rgb[2];
		j = 3 * (255 - (o_lpcontrast * 255 / 100));
		if (i > j)
		{
			rgb[0] = (unsigned char)(rgb[0] * j / i);
			rgb[1] = (unsigned char)(rgb[1] * j / i);
			rgb[2] = (unsigned char)(rgb[2] * j / i);
		}
	}
	else
	{
		/* nolpcolor, always use black */
		memset(rgb, 0, 3);
	}

	return rgb;
}

#if defined (GUI_WIN32)
static void dummyprt(ch)
	_CHAR_	ch;	/* character to be sent to printer */
{
}
#endif
/* This function copies a single character to the printer (or file).
 * Internally, is uses a large I/O buffer for the sake of efficiency.
 */
static void prtchar(ch)
	_CHAR_	ch;	/* character to be sent to printer */
{
	assert(ionext >= 0 && ionext < iomax);
	iobuf[ionext++] = ch;
	if (ionext == iomax)
	{
		iowrite(iobuf, ionext);
		ionext = 0;
	}
}

/* This function takes a single character from the mode's image() function
 * and passes it to the LP driver.  This function also performs newline
 * conversion, line wrapping/clipping, and page breaks.
 */
static void draw(p, qty, font, offset)
	CHAR	*p;	/* character to be output */
	long	qty;	/* number of characters to be output */
	_char_	font;	/* font of character */
	long	offset;	/* buffer offset of ch, or -1 if not from buffer */
{
	WINDOW	win;
	long	delta;
	CHAR	ch;
	char	lnumstr[20];

	/* A negative qty indicates that the character *p is to be repeated.
	 */
	if (qty < 0)
	{
		delta = 0;
		qty = -qty;
	}
	else
	{
		delta = 1;
	}

	for ( ; qty > 0; qty--, p += delta)
	{
		ch = *p;

		/* VTAB is treated either as a '\f', or as two '\n's */
		if (ch == '\013' || (o_lplines == 0 && ch == '\f'))
		{
			if (o_lplines == 0 || linesleft >= 5)
			{
				ch = '\n';
				draw(&ch, 1L, font, offset);
			}
			else
			{
				ch = '\f';
			}
		}

		/* maybe output a line number */
		if (column == 0		/* at start of line */
		 && o_lpnumber		/* "lpnumber" option is set */
		 && offset >= -1	/* not doing header or line number */
		 && ch != '\f')		/* character isn't formfeed */
		{
			/* output a line number */
			sprintf(lnumstr, "%6ld  ", lnum);
			draw(toCHAR(lnumstr), 8, 'n', -2L);
		}

		/* if line is too long, then wrap it or clip it */
		if (o_lpcolumns > 0 && column >= o_lpcolumns && ch != '\f' && ch != '\n')
		{
			/* if we're supposed to clip, then just ignore this char */
			if (!o_lpwrap)
			{
				continue;
			}

			/* else insert a newline */
			if (o_lpcrlf)
			{
				(*type->fontch)(font, '\r');
			}
			(*type->fontch)(font, '\n');

			/* that newline has the side-effects of incrementing the line
			 * number and resetting the column number.
			 */
			linesleft--;
			column = 0;
		}

		/* FF is treated specially, below */
		if (ch != '\f')
		{
			/* maybe convert newline to CR-LF */
			if (ch == '\n' && o_lpcrlf)
			{
				(*type->fontch)(font, '\r');
			}

			/* pass the character to the LP driver */
			(*type->fontch)(font, ch);

			/* if the character was newline, then decrement the
			 * number of lines left on this page, and reset the
			 * column to 0.
			 */
			if (ch == '\n')
			{
				linesleft--;
				column = 0;
			}
			else /* must have been a normal printable character */
			{
				column++;
				anytext = ElvTrue;
			}
		}

		if ((ch == '\f' && anytext) || (o_lplines > 0 && linesleft <= 1))
		{
			/* End the line, if it has any characters in it */
			if (column > 0)
			{
				(*type->fontch)('n', '\n');
				column = 0;
				linesleft--;
			}

			/* End this page */
			(*type->page)(linesleft);
			linesleft = o_lplines;

			/* Start the next page by calling the mode's header() function.
			 * This can cause this draw() function to be called recursively.
			 * If the header is larger than the page size, we could
			 * potentially get stuck in a loop.  To avoid this, we will
			 * temporarily set prwin to NULL.
			 */
			pagenum++;
			if (prwin && prwin->md->header)
			{
				win = prwin;
				prwin = NULL;
				(*win->md->header)(win, pagenum, win->mi, draw);
				prwin = win;
			}
			anytext = ElvFalse;
		}
	}
}

/* This function performs printing. */
RESULT lp(win, top, bottom, force)
	WINDOW	win;	/* window where print request was generated */
	MARK	top;	/* start of text to be printed */
	MARK	bottom;	/* end of text to be printed */
	ELVBOOL	force;	/* allow an existing file to be clobbered? */
{
	long	oldcurs;
	MARK	next;
	CHAR	*origdisplay = NULL;
	char	*out;
	char	rwa;
	
	/* convert the value of o_lptype to an LPTYPE pointer */
	type = findtype(tochar8(o_lptype));
	if (!type)
	{
		msg(MSG_ERROR, "bad lptype");
		return RESULT_ERROR;
	}

	/* Switch to the bufdisplay display mode, if not that way already */
	if (CHARcmp(o_display(win), o_bufdisplay(markbuffer(top))))
	{
		origdisplay = CHARdup(o_display(win));
		if (!dispset(win, tochar8(o_bufdisplay(markbuffer(top)))))
		{
			safefree(origdisplay);
			return RESULT_ERROR;
		}
	}

	/* Call the mode's setup function.  Pretend the cursor is at the
	 * top of the print region, so setup() doesn't try to scroll.
	 */
	next = (*win->md->setup)(win, top, markoffset(top), bottom, win->mi);

	/* open file or filter program */
	if (type->spooled)
	{
		rwa = 'w';
		out = tochar8(o_lpout);
		if (!out || !*out)
		{
			msg(MSG_ERROR, "must set lpout");
			goto Error;
		}
		if (out[0] == '>' && out[1] == '>')
			out += 2, rwa = 'a';
		if (!ioopen(out, rwa, ElvTrue, force, 'b'))
		{
			goto Error;
		}
	}

	/* allocate the I/O buffer */
	iomax = 1024;
	iobuf = (CHAR *)safealloc(iomax, sizeof(CHAR));
	ionext = 0;

	/* initialize the LP driver */
#if defined (GUI_WIN32)
	if (strcmp (o_lptype, "windows") == 0)
		(*type->before)(type->minorno, dummyprt);
	else
#endif
		(*type->before)(type->minorno, prtchar);

	/* start the first page */
	pagenum = 1;
	linesleft = o_lplines;
	column = 0;
	prwin = win;
	if (win->md->header)
	{
		(*win->md->header)(win, pagenum, win->mi, draw);
	}
	anytext = ElvFalse;

	/* generate image lines, and send them to the printer.  Temporarily
	 * move the cursor to the end of the buffer so it doesn't affect
	 * the appearance of the text.
	 */
	oldcurs = markoffset(win->cursor);
	marksetoffset(win->cursor, o_bufchars(markbuffer(win->cursor)));
	while (markoffset(next) < markoffset(bottom))
	{
		lnum = markline(next);
		next = (*win->md->image)(win, next, win->mi, draw);
	}
	marksetoffset(win->cursor, oldcurs);

	/* end the printout */
	(*type->after)(linesleft);

	/* end the I/O */
	if (ionext > 0)
	{
		iowrite(iobuf, ionext);
	}
	safefree(iobuf);
	if (type->spooled && !ioclose())
		goto Error;

	/* clean up and return success */
	if (origdisplay)
	{
		(void)dispset(win, tochar8(origdisplay));
		safefree(origdisplay);
	}
	msg(MSG_STATUS, "[d]$1 pages", (long)pagenum);
	return RESULT_COMPLETE;

Error:
	/* clean up and return failure */
	if (origdisplay)
	{
		(void)dispset(win, tochar8(origdisplay));
		safefree(origdisplay);
	}
	return RESULT_ERROR;
}
#endif /* FEATURE_LPR */
