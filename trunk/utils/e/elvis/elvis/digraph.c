/* digraph.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_digraph[] = "$Id: digraph.c,v 2.12 2003/10/17 17:41:23 steve Exp $";
#endif
#ifndef NO_DIGRAPH

static void adjustctype P_((_CHAR_ ch));

/* This structure is used to store digraphs.  Note that "in1" is always less
 * than or equal to "in2".
 */
typedef struct dig_s
{
	struct dig_s	*next;		/* another digraph */
	CHAR		in1, in2;	/* the input characters of this digraph */
	CHAR		out;		/* the character they form */
	ELVBOOL		save;		/* user-defined? */
} DIGRAPH;


/* This is a list of all defined digraphs */
DIGRAPH *digs;


#ifdef NEED_CTYPE
CHAR elvct_upper[256] =
{  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
  48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
  64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
  80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95,
  96, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
  80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90,123,124,125,126,127,
 128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
 144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
 160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
 176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
 192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
 208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
 224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
 240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};
CHAR elvct_lower[256] =
{  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
  32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
  48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
  64, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
 112,113,114,115,116,117,118,119,120,121,122, 91, 92, 93, 94, 95,
  96, 97, 98, 99,100,101,102,103,104,105,106,107,108,109,110,111,
 112,113,114,115,116,117,118,119,120,121,122,123,124,125,126,127,
 128,129,130,131,132,133,134,135,136,137,138,139,140,141,142,143,
 144,145,146,147,148,149,150,151,152,153,154,155,156,157,158,159,
 160,161,162,163,164,165,166,167,168,169,170,171,172,173,174,175,
 176,177,178,179,180,181,182,183,184,185,186,187,188,189,190,191,
 192,193,194,195,196,197,198,199,200,201,202,203,204,205,206,207,
 208,209,210,211,212,213,214,215,216,217,218,219,220,221,222,223,
 224,225,226,227,228,229,230,231,232,233,234,235,236,237,238,239,
 240,241,242,243,244,245,246,247,248,249,250,251,252,253,254,255
};
#define U  ELVCT_UPPER
#define L  ELVCT_LOWER
#define XU ELVCT_UPPER|ELVCT_XDIGIT
#define XL ELVCT_LOWER|ELVCT_XDIGIT
#define XD ELVCT_DIGIT|ELVCT_XDIGIT
#define P  ELVCT_PUNCT
#define S  ELVCT_SPACE
#define C  ELVCT_CNTRL
#define SC ELVCT_SPACE|ELVCT_CNTRL
CHAR elvct_class[256] =
{
   C,  C,  C,  C,  C,  C,  C,  C,  C, SC, SC,  C, SC, SC,  C,  C,
   C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,  C,
   S,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,
  XD, XD, XD, XD, XD, XD, XD, XD, XD, XD,  P,  P,  P,  P,  P,  P,
   P, XU, XU, XU, XU, XU, XU,  U,  U,  U,  U,  U,  U,  U,  U,  U,
   U,  U,  U,  U,  U,  U,  U,  U,  U,  U,  U,  P,  P,  P,  P,  P,
   P, XL, XL, XL, XL, XL, XL,  L,  L,  L,  L,  L,  L,  L,  L,  L,
   L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  L,  P,  P,  P,  P,  C,
   P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,
   P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,
   P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,
   P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,
   P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,
   P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,
   P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,
   P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P,  P
};
#undef U
#undef L
#undef XU
#undef XL
#undef XD
#undef P
#undef S
#undef C
#undef SC

static void adjustctype(ch)
	_CHAR_	ch;	/* a digraph character that changed */
{
	DIGRAPH	*dp;	/* the changed digraph, or NULL if deleted */
	CHAR	tmp;

	/* if ASCII, leave it alone */
	if (ch < 0x80)
		return;

	/* try to find a digraph that results in this character */
	for (dp = digs; dp && dp->out != ch; dp = dp->next)
	{
	}

	/* what kind of change? */
	if (dp && ((elvupper(dp->in1) && !elvlower(dp->in2)) || elvupper(dp->in2)))
	{
		/* making it uppercase */
		setupper(ch);
		clrlower(ch);
		clrpunct(ch);
		tmp = digraph(elvtolower(dp->in1), elvtolower(dp->in2));
		if (tmp >= 0x80)
		{
			/* we can make an uppercase/lowercase pair */
			elvtoupper(tmp) = ch;
			elvtolower(ch) = tmp;
		}
	}
	else if (dp && ((!elvupper(dp->in1) && elvlower(dp->in2)) || elvlower(dp->in1)))
	{
		/* making it lowercase */
		setlower(ch);
		clrupper(ch);
		clrpunct(ch);
		tmp = digraph(elvtoupper(dp->in1), elvtoupper(dp->in2));
		if (tmp >= 0x80)
		{
			/* we can make an uppercase/lowercase pair */
			elvtolower(tmp) = ch;
			elvtoupper(ch) = tmp;
		}
	}
	else
	{
		/* deleting it, or making it punctuation */
		if (elvupper(ch))
		{
			clrupper(ch);
			tmp = elvtolower(ch);
			elvtolower(ch) = ch;
			elvtoupper(tmp) = tmp;
			clrupper(ch);
		}
		else if (elvtolower(ch))
		{
			clrlower(ch);
			tmp = elvtoupper(ch);
			elvtoupper(ch) = ch;
			elvtolower(tmp) = tmp;
			clrlower(ch);
		}
		setpunct(ch);
	}
}
#endif



/* This function looks up a digraph.  If it finds the digraph, it returns
 * the non-ASCII character; otherwise it returns the second parameter character.
 */
CHAR digraph(in1, in2)
	_CHAR_	in1;	/* the underlying character */
	_CHAR_	in2;	/* the second character */
{
	CHAR	newkey;
	DIGRAPH	*dp;

	/* remember the new key, so we can return it if this isn't a digraph */
	newkey = in2;

	/* sort in1 and in2, so that their original order won't matter */
	if (in1 > in2)
	{
		in2 = in1;
		in1 = newkey;
	}

	/* scan through the digraph chart */
	for (dp = digs;
	     dp && (dp->in1 != in1 || dp->in2 != in2);
	     dp = dp->next)
	{
	}

	/* if this combination isn't in there, just use the new key */
	if (!dp)
	{
		return newkey;
	}

	/* else use the digraph key */
	return dp->out;
}


/* This function lists, defines, or deletes digraphs.  If passed a NULL
 * pointer it will list the user-defined digraphs or all digraphs, depending
 * on the value of "bang."  If passed a 2-character string, it will delete
 * a digraph.  If passed a 3-character string, it will define a digraph.
 */
void digaction(win, bang, extra)
	WINDOW	win;	/* window to write to, if listing */
	ELVBOOL	bang;	/* list all, or define non-ASCII */
	CHAR	*extra;	/* NULL to list, "xx" to delete, "xxy" to add */
{
	int		dig;
	DIGRAPH		*dp;
	DIGRAPH		*prev;
	CHAR		listbuf[8];

	/* if no args, then display the existing digraphs */
	if (!extra)
	{
		listbuf[0] = listbuf[1] = listbuf[2] = listbuf[5] = ' ';
		listbuf[7] = '\0';
		for (dig = 0, dp = digs; dp; dp = dp->next)
		{
			if (dp->save || bang)
			{
				dig += 7;
				if (dig >= o_columns(win))
				{
					drawextext(win, toCHAR("\n"), 1);
					dig = 7;
				}
				listbuf[3] = dp->in1;
				listbuf[4] = dp->in2;
				listbuf[6] = dp->out;
				drawextext(win, listbuf, 7);
			}
		}
		drawextext(win, toCHAR("\n"), 1);
		return;
	}

	/* make sure we have at least two characters */
	if (!extra[1])
	{
		msg(MSG_ERROR, "digraphs must be composed of two characters");
		return;
	}

	/* sort in1 and in2, so that their original order won't matter */
	if (extra[0] > extra[1])
	{
		dig = extra[0];
		extra[0] = extra[1];
		extra[1] = dig;
	}

	/* locate the new digraph character */
	for (dig = 2; extra[dig] == ' ' || extra[dig] == '\t'; dig++)
	{
	}
	dig = extra[dig];
	if (!bang && dig)
	{
		dig |= 0x80;
	}

	/* search for the digraph */
	for (prev = (DIGRAPH *)0, dp = digs;
	     dp && (dp->in1 != extra[0] || dp->in2 != extra[1]);
	     prev = dp, dp = dp->next)
	{
	}

	/* deleting the digraph? */
	if (!dig)
	{
		if (!dp)
		{
			return;
		}
		if (prev)
			prev->next = dp->next;
		else
			digs = dp->next;
#ifdef NEED_CTYPE
		adjustctype(dp->out);
#endif
		safefree(dp);
		return;
	}

	/* if necessary, create a new digraph struct for the new digraph */
	if (dig && !dp)
	{
		dp = (DIGRAPH *)safekept(1, sizeof *dp);
		if (prev)
			prev->next = dp;
		else
			digs = dp;
		dp->next = (DIGRAPH *)0;
	}

	/* assign it the new digraph value */
	dp->in1 = extra[0];
	dp->in2 = extra[1];
	dp->out = dig;
	dp->save = (ELVBOOL)(win != (WINDOW)0);
# ifdef NEED_CTYPE
	adjustctype(dig);
# endif
}

# ifdef FEATURE_MKEXRC
void digsave(buf)
	BUFFER	buf;	/* the buffer to append commands onto */
{
	static CHAR	text[] = "digraph! XX Y\n";
	DIGRAPH		*dp;
	MARK		append;

	append = markalloc(buf, o_bufchars(buf));
	for (dp = digs; dp; dp = dp->next)
	{
		if (dp->save)
		{
			text[9] = dp->in1;
			text[10] = dp->in2;
			text[12] = dp->out;
			bufreplace(append, append, text, QTY(text) - 1);
			markaddoffset(append, QTY(text) - 1);
		}
	}
	markfree(append);
}
# endif /* FEATURE_MKEXRC */
#endif
