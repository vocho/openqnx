/* lpovrtyp.c */
/* Copyright 1995 by Steve Kirkendall */



/* This file contains a driver for printer types which use overtyping to
 * simulate fonts.  There are two styles: "cr" uses a single carriage return
 * to start overtyping a whole line, and "bs" uses backspace to overtype a
 * single character.  Some line printers refuse to backspace more than /n/
 * times per line; they like the "cr" style.  On the other hand, "bs" is more
 * like the output produced by /nroff/, and the /more/ program understands it.
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_lpovrtyp[] = "$Id: lpovrtyp.c,v 2.18 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef FEATURE_LPR

#if USE_PROTOTYPES
static void outcr(void);
static void outbs(void);
static void before(int minorno, void (*draw)(_CHAR_));
static void fontch(_char_ font, _CHAR_ ch);
static void page(int linesleft);
static void after(int linesleft);
#endif


/* This value is the minor number of the driver being used.  It is set by the
 * before() function, and remains valid until the after() function returns.
 * It should be 0 for "cr", or 1 for "bs".
 */
static int ptype;

/* This is a pointer to the draw() function to use for outputing individual
 * characters.  It is set by the before() function, and remains valid until
 * the after() function returns.
 */
static void (*prtchar) P_((_CHAR_ ch));

/* These are a line buffer.  Each character in the line can have 2 parts,
 * and we also remember the length of the line.  part1 is always filled.
 * part2 may be partially filled, if some characters are normal.  In this
 * case, the length2 indicates the portion of the part2 which has been used.
 */
static CHAR *part1, *part2;
static int column, length2;

/* This function outputs the buffered line using the "cr" technique. */
static void outcr()
{
	int	i;

	/* output part1 */
	for (i = 0; i < column; i++)
	{
		(*prtchar)(part1[i]);
	}

	/* if there is a part2, then output a CR and then part2 */
	if (length2 > 0)
	{
		(*prtchar)('\r');
		for (i = 0; i < length2; i++)
		{
			(*prtchar)(part2[i]);
		}
	}
}

/* This function outputs the buffered line using the "bs" technique */
static void outbs()
{
	int	i;

	/* output each column's pair of characters */
	for (i = 0; i < column; i++)
	{
		(*prtchar)(part1[i]);
		if (part2[i] != ' ')
		{
			(*prtchar)('\b');
			(*prtchar)(part2[i]);
		}
	}
}


/* This is the before() function.  It sets the ptype index, and allocates a
 * line buffer.  Also, if the lpcolumns option is unset, then this sets it to
 * 132 because the cr/bs driver can't handle lines of unknown length.
 */
static void before(minorno, draw)
	int	minorno;		/* which style of control codes to use */
	void	(*draw) P_((_CHAR_));	/* function for sending single char to printer */
{
	assert(minorno == 0 || minorno == 1);

	/* set the ptype and out function */
	ptype = minorno;
	prtchar = draw;

	/* if lpcolumns is unset, then set it to 132 */
	if (o_lpcolumns == 0)
	{
		o_lpcolumns = 132;
	}

	/* allocate storage space for the line buffer */
	part1 = (CHAR *)safealloc((int)o_lpcolumns, sizeof(CHAR));
	part2 = (CHAR *)safealloc((int)o_lpcolumns, sizeof(CHAR));
	column = length2 = 0;
}

/* This function adds a character to the line buffer, or if the character is
 * a control character then it outputs the line and then the control character
 */
static void fontch(font, ch)
	_char_	font;	/* font of the next character from text image */
	_CHAR_	ch;	/* the next character */
{
	int	bits;

	if (font == 0)
		font = 1;

	switch (ch)
	{
	  case '\n':
	  case '\f':
		/* output the line */
		switch (ptype)
		{
		  case 0:	outcr();	break;
		  case 1:	outbs();	break;
		}

		/* output the character.  formfeed implies CR */
		(*prtchar)(ch);
		if (ch == '\f')
		{
			(*prtchar)('\r');
		}

		/* get ready for next line */
		column = length2 = 0;
		break;

	  default:
		assert(column < o_lpcolumns);
		bits = colorinfo[font].da.bits;
	
		if (bits & COLOR_GRAPHIC)
		{
			if (ch >= '1' && ch <= '9')
			{
				ch = '+';
			}
			part1[column] = ch;
			part2[column] = ' ';
			++column;
		}
		else if (bits & (COLOR_ITALIC | COLOR_UNDERLINED))
		{
			part1[column] = '_';
			part2[column] = ch;
			length2 = ++column;
		}
		else if (bits & COLOR_BOLD)
		{
			part1[column] = part2[column] = ch;
			length2 = ++column;
		}
		else
		{
			part1[column] = ch;
			part2[column] = ' ';
			++column;
		}
	}
}

/* This function is called after every page except the last one */
static void page(linesleft)
	int	linesleft;	/* lines remaining on page */
{
	/* output a formfeed character */
	if (linesleft > 0)
	{
		(*prtchar)('\f');
	}
}

/* This function is called at the end of the print job.  It can output a
 * final formfeed, restore fonts, or whatever.  Here, it just frees the
 * line buffer.
 */
static void after(linesleft)
	int	linesleft;	/* lines remaining on final page */
{
	safefree(part1);
	safefree(part2);
	if (o_lpformfeed)
	{
		(*prtchar)((_CHAR_)'\f');
	}
}

/* These describe the printer types supported by these functions */
LPTYPE lpcr =	{"cr", 0, ElvTrue, before, fontch, page, after};
LPTYPE lpbs =	{"bs", 1, ElvTrue, before, fontch, page, after};

#endif /* FEATURE_LPR */
