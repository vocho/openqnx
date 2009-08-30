/* lpescape.c */
/* Copyright 1995 by Steve Kirkendall */



/* This file contains a driver for printer types which use escape sequences
 * to select fonts.  This includes the "epson", "hp", and "dumb" printer types.
 */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_lpescape[] = "$Id: lpescape.c,v 2.29 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef FEATURE_LPR

/* These values are used to index into the following table of escape codes */
typedef enum {BOLD, NOBOLD, UNDLN, NOUNDLN, ITALIC, NOITALIC, FIXED, NOFIXED, BOXED, BEFORE, AFTER, GCMAP} CODE;

#if USE_PROTOTYPES
static void before(int minorno, void (*draw)(_CHAR_));
static void fontch(_char_ font, _CHAR_ ch);
static void page(int linesleft);
static void after(int linesleft);
static void putesc(CODE code);
#endif


/* This is a special "hp2" escape sequence.  It saves the cursor position,
 * draws a half-tone rectangle the size of a single character, and then
 * restores the cursor position.  This is used instead of the SHADE_CHAR
 * for "hp2", because HP likes to replace the normal '\260' with a chunky
 * character which is hard to read through.  Note that the initial Esc is
 * omitted here, since putesc() will add one for us.
 *	       Save RowAdj    Width    Height    Shade    Draw    Restore */
#define HP2BOX "&f0S\033&a-90V\033*c72H\033*c120V\033*c17G\033*c2P\033&f1S"



/* This table lists the escape codes used by each printer type */
static char *codes[][12] =
{	/* BOLD	NOBOLD	UNDLN	NOUNDLN	ITALIC	NOITALIC FIXED	NOFIXED	BOXED		BEFORE 	AFTER	GCMAP */
/*epson*/{"E",	"F",	"-1",	"-0",	"4",	"5",    "p0",	"p0",	"\260\b",	NULL,	NULL,	"+++++++++-|*"},
/*pana*/ {"E",	"F",	"-1",	"-0",	"4",	"5",    "p0",	"p0",	"\260\b",	"t1",	"t0",	"\300\301\331\303\305\264\332\302\277\304\263\371"},
/*ibm*/	 {"E",	"F",	"-1",	"-0",	"4",	"5",    "P0",	"P1",	"\260\b",	NULL,	NULL,	"\300\301\331\303\305\264\332\302\277\304\263\371"},
/*hp*/	 {"(s3B","(s0B","&d1D",	"&d@",	"(s1S",	"(s0S", "(s0P",	"(s1P",	HP2BOX,		"(10U",	NULL,	"\300\301\331\303\305\264\332\302\277\304\263\371"},
/*dumb*/ {NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	NULL,	"",		NULL,	NULL,	"+++++++++-|*"},
/*ansi*/ {"[1m","[22m",	"[4m",	"[24m",	NULL,	NULL,	NULL,	NULL,	"",		NULL,	NULL,	"+++++++++-|*"},
/*html*/ {"<b>","</b>",	"<u>",	"</u>",	"<i>",	"</i>",	NULL,	NULL,	"",		"<html><body><pre>", "</pre></body></html>\n", "+++++++++-|*"}
};


/* This table is used for converting Latin-1 characters to PC-8 characters.
 * This is necessary because the printer is placed in PC-8 mode so that it can
 * output the graphic characters used by the <pre graphic> tag, and most
 * computers these days use Latin-1 internally.  This table only maps the
 * Latin-1 characters within the range 0xa0-0xff; others don't need conversion.
 * Some conversions are not exact.
 */
static unsigned char topc8[] =
{
	' ',  0xad, 0x9b, 0x9c, '*',  0x9d, '|',  0xf5, '"',  0xe9, 0xa6, 0xae, 0xaa, '-',  'R',  '~',
	0xf8, 0xf1, 0xfd, '3',  '\'', 0xe6, 0xf4, 0xf9, ',',  '1',  0xa7, 0xaf, 0xac, 0xab, '3',  0xa8,
	'A',  'A',  'A',  'A',  0x8e, 0x8f, 0x92, 0x80, 'E',  0x90, 'E',  'E',  'I',  'I',  'I',  'I',
	'D',  0xa5, 'O',  'O',  'O',  'O',  0x99, 'x',  'O',  'U',  'U',  'U',  0x9a, 'Y',  0xe8, 0xe1,
	0x85, 0xa0, 0x83, 'a',  0x84, 0x86, 0x91, 0x87, 0x8a, 0x82, 0x88, 0x89, 0x8d, 0xa1, 0x8c, 0x8b,
	0xeb, 0xa4, 0x95, 0xa2, 0x93, 'o',  0x94, 0xf6, 'o',  0x97, 0xa3, 0x97, 0x81, 'y',  0xe7, 'y'
};
static unsigned char *convert;

/* This value is the minor number of the driver being used.  It is set by the
 * before() function, and remains valid until the after() function returns.
 */
static int ptype;

/* This is a pointer to the draw() function to use for outputing individual
 * characters.  It is set by the before() function, and remains valid until
 * the after() function returns.
 */
static void (*prtchar) P_((_CHAR_ ch));

/* This stores the font currently set for output */
static int curfont;

/* Output an escape sequence */
static void putesc(code)
	CODE	code;	/* which string to output */
{
	char	*scan;

	/* if this printer has no such code, then do nothing */
	scan = codes[ptype][code];
	if (!scan)
		return;

	/* The "Esc" is implied, except for "html" where there is no "Esc".
	 * Also, some special non-ASCII sequences, or control sequences, don't
	 * need an "Esc".
	 */
	if (*scan != '<' && *scan >= ' ' && *scan <= '~')
		(*prtchar)('\033');

	/* Output the string */
	while (*scan)
	{
		(*prtchar)((_CHAR_)*scan);
		scan++;
	}
}

/* This is the before() function.  It sets the ptype index, outputs the
 * BEFORE string if it isn't NULL, and sets the convert variable.
 */
static void before(minorno, draw)
	int	minorno;		/* which control codes to use */
	void	(*draw) P_((_CHAR_));	/* function for printing a single character */
{
	assert(minorno < QTY(codes));

	/* set the ptype and out function */
	ptype = minorno;
	prtchar = draw;
	curfont = 1;

	/* if there is a BEFORE string, output it now */
	putesc(BEFORE);

	/* If the file appears to use Latin-1, and the lpconvert option is set,
	 * and the printer type is a real printer (not "html") then set the
	 * convert pointer to point to the topc8[] array; else set the convert
	 * pointer to NULL.
	 */
	if (o_lpconvert
	 && digraph('A', 'E') == 0xc6
	 && *codes[ptype]
	 && **codes[ptype] != '<')
	{
		convert = topc8;
	}
	else
	{
		convert = NULL;
	}
}


/* This function outputs font-change strings, if necessary, and then outputs
 * a character.
 */
static void fontch(font, ch)
	_char_	font;	/* font of the next character from text image */
	_CHAR_	ch;	/* the next character to draw */
{
	char	*scan;
	int	setting, resetting, tmp;
	char	color[24];
	int	newfg, oldfg;

	if (font == 0)
		font = 1;

	/* initialize newfg just to avoid a bogus compiler warning */
	newfg = 0;

	if (font != curfont)
	{
		/* Determine which attributes must be set or reset */
		resetting = colorinfo[curfont].da.bits & ~(COLOR_FG|COLOR_BG);
		setting = colorinfo[font].da.bits & ~(COLOR_FG|COLOR_BG);
		if (*codes[ptype] && **codes[ptype] == '[') /* ansi */
		{
			/* Ansi can set both the foreground & background, but
			 * for now we'll just do foreground.  Convert RGB
			 * colors to ansi.  This conversion may affect the
			 * "bold" flag.
			 *
			 * Note that we use the "video" colors instead of the
			 * "lp" colors.  The "lp" colors were chosen to contrast
			 * with white, while the "video" colors were chosen to
			 * contrast with elvis' best guess of the terminal
			 * background color.
			 */
			newfg = colorrgb2ansi(ElvTrue, colorinfo[font].da.fg_rgb);
			if (newfg > 10)
			{
				if (setting & COLOR_BOLD)
					newfg = 7;
				else
					setting |= COLOR_BOLD, newfg -= 10;
			}
			oldfg = colorrgb2ansi(ElvTrue, colorinfo[curfont].da.fg_rgb);
			if (oldfg > 10)
			{
				if (resetting & COLOR_BOLD)
					oldfg = 7;
				else
					resetting |= COLOR_BOLD, oldfg -= 10;
			}

			/* if color is different, then we must set it */
			if (newfg != oldfg)
				setting |= COLOR_FG;
		}
		else if (*codes[ptype] && **codes[ptype] == '<') /* html */
		{
			/* HTML is tricky because attributes might not mix,
			 * and because start and end tags must nest correctly.
			 * Consequently, when changing any bold/italic/underline
			 * attribute, we must change all of them.  The
			 * "resetting" and "setting" variables already reflect
			 * this.  HOWEVER, if there is no change among the
			 * bold/italic/underline bits and the color doesn't
			 * change, then we don't need to make any change.
			 */
			if (o_lpcolor && colorinfo[curfont].fg != colorinfo[font].fg)
			{
				if (colorinfo[curfont].da.fg_rgb[0] != 0
				 || colorinfo[curfont].da.fg_rgb[1] != 0
				 || colorinfo[curfont].da.fg_rgb[2] != 0)
					resetting |= COLOR_FG;
				if (colorinfo[font].da.fg_rgb[0] != 0
				 || colorinfo[font].da.fg_rgb[1] != 0
				 || colorinfo[font].da.fg_rgb[2] != 0)
					setting |= COLOR_FG;
			}
			else if (((resetting^setting) & (COLOR_BOLD|COLOR_ITALIC|COLOR_UNDERLINED)) == 0)
				resetting = setting = 0;
		}
		else
		{
			/* Don't set/reset any attributes which are unchanged */
			tmp = resetting;
			resetting &= ~setting;
			setting &= ~tmp;
		}

		/* Reset some modes */
		if (resetting & COLOR_BOLD)
			putesc(NOBOLD);
		if (resetting & COLOR_ITALIC)
			putesc(NOITALIC);
		if (resetting & COLOR_UNDERLINED)
			putesc(NOUNDLN);
		if (resetting & COLOR_PROP)
			putesc(FIXED);
		if (resetting & COLOR_FG)
		{
			if (*codes[ptype][0] == '<') /* html */
				for (scan = "</font>"; *scan; scan++)
					(*prtchar)((_CHAR_)*scan);
		}

		/* Set some modes */
		if (setting & COLOR_FG)
		{
			if (*codes[ptype][0] == '<') /* html */
				sprintf(color, "<font color=\"#%02x%02x%02x\">", colorinfo[font].da.fg_rgb[0],
					colorinfo[font].da.fg_rgb[1], colorinfo[font].da.fg_rgb[2]);
			else /* ansi */
				sprintf(color, "\033[3%dm", newfg);
			for (scan = color; *scan; scan++)
				(*prtchar)((_CHAR_)*scan);
		}
		if (setting & COLOR_PROP)
			putesc(NOFIXED);
		if (setting & COLOR_UNDERLINED)
			putesc(UNDLN);
		if (setting & COLOR_ITALIC)
			putesc(ITALIC);
		if (setting & COLOR_BOLD)
			putesc(BOLD);

		/* Okay!  We have now switched fonts! */
		curfont = font;
	}

	/* If BOXED then print a grayscale rectangle the size of a character. */
	if (colorinfo[font].da.bits & COLOR_BOXED)
		putesc(BOXED);

	/* if in graphic mode, convert graphic characters */
	if ((colorinfo[font].da.bits & COLOR_GRAPHIC) && codes[ptype][GCMAP])
	{
		switch (ch)
		{
		  case '1':	ch = codes[ptype][GCMAP][0];	break;
		  case '2':	ch = codes[ptype][GCMAP][1];	break;
		  case '3':	ch = codes[ptype][GCMAP][2];	break;
		  case '4':	ch = codes[ptype][GCMAP][3];	break;
		  case '5':	ch = codes[ptype][GCMAP][4];	break;
		  case '6':	ch = codes[ptype][GCMAP][5];	break;
		  case '7':	ch = codes[ptype][GCMAP][6];	break;
		  case '8':	ch = codes[ptype][GCMAP][7];	break;
		  case '9':	ch = codes[ptype][GCMAP][8];	break;
		  case '-':	ch = codes[ptype][GCMAP][9];	break;
		  case '|':	ch = codes[ptype][GCMAP][10];	break;
		  case '*':	ch = codes[ptype][GCMAP][11];	break;
		}
	}
	else if (convert && ch >= 0xa0)
	{
		ch = convert[ch - 0xa0];
	}

	/* Output the character.  For HTML this may be tricky. */
	if (*codes[ptype] && **codes[ptype] == '<')
	{
		if (ch == '<')
			scan = "&lt;";
		else if (ch == '>')
			scan = "&gt;";
		else if (ch == '&')
			scan = "&amp;";
		else
			scan = NULL;
		if (scan)
		{
			while (*scan)
				(*prtchar)(*scan++);
			return;
		}
	}
	(*prtchar)(ch);
}

/* This function is called after every page except the last one */
static void page(linesleft)
	int	linesleft;	/* number of lines remaining on page */
{
	/* output a formfeed character, except for "html" */
	if (*codes[ptype] && **codes[ptype] != '<')
		(*prtchar)('\f');
}

/* This function is called at the end of the print job.  It can output a
 * final formfeed, restore fonts, or whatever.  Here, it just outputs the
 * AFTER string, if there is one.
 */
static void after(linesleft)
	int	linesleft;	/* number of lines remaining on final page */
{
	/* if there is an AFTER string, output it now */
	putesc(AFTER);

	/* and maybe output a formfeed, too */
	if (o_lpformfeed && *codes[ptype] && **codes[ptype] != '<')
	{
		(*prtchar)((_CHAR_)'\f');
	}
}

/* These describe the printer types supported by these functions */
LPTYPE lpepson ={"epson", 0, ElvTrue, before, fontch, page, after};
LPTYPE lppana =	{"pana", 1, ElvTrue, before, fontch, page, after};
LPTYPE lpibm =	{"ibm", 2, ElvTrue, before, fontch, page, after};
LPTYPE lphp =	{"hp", 3, ElvTrue, before, fontch, page, after};
LPTYPE lpdumb =	{"dumb", 4, ElvTrue, before, fontch, page, after};
LPTYPE lpansi = {"ansi", 5, ElvTrue, before, fontch, page, after};
LPTYPE lphtml =	{"html", 6, ElvTrue, before, fontch, page, after};

#endif /* FEATURE_LPR */
