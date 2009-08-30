/* dmmarkup.c */
/* Copyright 1995 by Steve Kirkendall */


/* This file contains some fairly generic text formatting code -- generic
 * in the sense that it can be easily tweaked to format a variety of types
 * marked-up text.  Currently, it supports useful subsets of NROFF, HTML,
 * and TeX instructions.
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_dmmarkup[] = "$Id: dmmarkup.c,v 2.136 2003/10/19 23:13:33 steve Exp $";
#endif
#ifdef DISPLAY_ANYMARKUP


#define SGML_HACK

#define GRANULARITY 64	/* number of LINEINFOs to allocate at a time */


typedef enum { TWRAP_NO, TWRAP_AFTER, TWRAP_BEFORE } twrap_t;

typedef struct
{
	CHAR	text[256];	/* raw text of token */
	long	offset[256];	/* offsets of characters */
	int	nchars;		/* number of characters in text[] */
	int	width;		/* normal displayed width of text[] */
	struct markup_s	*markup;/* info about markup token */
} TOKEN;

typedef struct markup_s
{
	char	*name;		/* name of the markup */
	char	attr[8];	/* attributes of markup */
	twrap_t	(*fn)P_((TOKEN *));/* ptr to special function */	
} MARKUP;
#define TITLE	attr[0]		/* in title: -, N, Y */
#define BREAKLN	attr[1]		/* line break: -, 0, 1, 2, c, or p */
#define INDENT	attr[2]		/* -, <, >, or a number */
#define LIST	attr[3]		/* in list: -, N, Y, # */
#define FONT	attr[4]		/* font: -,=,n,b,u,i,f,e,N,B,U,I,F,E */
#define FILL	attr[5]		/* Y=fill, N=preformatted, -=no chg. */
#define DEST	attr[6]		/* S=section, P=paragraph, T=<tab> key */

typedef struct
{
	long	 offset;	/* offset of start of line */
	struct
	{
	    unsigned indent : 8;  /* indentation amount, in spaces */
	    unsigned listcnt : 8; /* Counter for nest#1 numbered list; 0=not numbered */
	    unsigned nest : 6;	  /* nesting level of list/menu; 0=not in list*/
	    unsigned prefmt : 1;  /* 1=literal whitespace, 0=fill */
	    unsigned graphic : 1; /* 1=replace |-^. with graphic chars */
	    unsigned midline: 1;  /* 1=after a newline, 0=after other char */
	    unsigned reduce : 1;  /* 1=fewer newlines, 0=normal qty newlines */
	    unsigned deffont : 4; /* index into fonttable[] of default font char */
	    unsigned curfont : 4; /* index into fonttable[] of current font char */
	}	state;
} LINEINFO;

typedef struct
{
	TOKEN	*(*get)P_((CHAR **));	/* mode-dependent get() */
	void	(*escape)P_((TOKEN *));	/* mode-dependent escape() */
	LINEINFO *line;			/* line array */
	long	nlines;			/* number of lines in line array */
	long	endtitle;		/* offset of the end of the title */
	CHAR	*title;			/* title of document, or NULL */
	CHAR	**defs;			/* macros within the text */
#ifdef SGML_HACK
	int	flavor;			/* which macro package to emulate */
#endif
} MUINFO;

static ELVBOOL	first;	/* is this the first token on this line? */
static ELVBOOL	anyspc;	/* has whitespace been encountered? */
static ELVBOOL	title;	/* collecting characters of the title */
static ELVBOOL	list;	/* o_list && o_listchars.markup */
static ELVBOOL	readonly;/* o_readonly -- affects &entity with no ; */
static int	textwidth;/* o_columns */
static int	tabstop;/* repeating part of o_tabstop */
static int	listind;/* o_shiftwidth/2, or 2 if shiftwidth<=4 */
static int	col;	/* logical column number */
static MUINFO	*mui;	/* pointer to muinfo */

static ELVBOOL	prefmt;	/* ElvTrue=literal whitespace, ElvFalse=fill */
static ELVBOOL	graphic;/* ElvTrue=replace |-^. with graphic chars */
static ELVBOOL	midline;/* ElvFalse=after newline, ElvTrue=after other character */
static ELVBOOL	reduce;	/* ElvTrue=fewer newlines, ElvFalse=normal qty newlines */
static char	deffont;/* default font */
static char	curfont;/* current font */
static int	indent;	/* indentation amount */
static int	nest;	/* nesting level of list/menu; 0=not in list */
static int	listcnt;/* Counter for nest#1 numbered list; 0=not numbered */

/* Because the MUINFO font fields are only 4 bits long, we can only store a
 * few specific font codes.  The following table is used to convert the font
 * codes from the letter format to the MUINFO format.
 */
static char	fonttable[] = "nbiufeldg*m\b\r\\\t\f";

/* These variables store the string and font collected by the manarg() function
 * and output by the manput() function.  The "manlen" variable should be
 * initialized to 0 before the first call to manarg().  NOTE: These variables
 * and the manput() function are also used by htmlimg().
 */
static int	manlen;		/* length of "mantext" string */
static CHAR	mantext[80];	/* buffer, holds args from .XX macro */
static long	manoffset[80];	/* holds offsets of mantext[] chars */
static char	manfont[80];	/* holds fonts of mantext[] chars */

#ifdef SGML_HACK
/* This stores the text of an expected terminator tag.  SGML allows the use of
 * "<tt/some text/" as a shorthand for "<tt>some text</tt>", so if this var
 * is set to a non-empty string then the next "/" character is interpreted
 * as this string.
 */
static CHAR	sgmltag[20];
#endif /* SGML_HACK */

/* Forward declarations of some functions which are static to this file */
#ifdef DISPLAY_HTML
static void	initfonts P_((void));
static void	htmlescape P_((TOKEN *tok));
static twrap_t	htmlimg P_((TOKEN *token));
static twrap_t	htmlpre P_((TOKEN *token));
static twrap_t	htmlli P_((TOKEN *token));
static twrap_t	htmlinput P_((TOKEN *token));
static twrap_t	htmla P_((TOKEN *token));
static int	htmlmarkup P_((TOKEN *token));
static TOKEN	*htmlget P_((CHAR **refp));
static DMINFO	*htmlinit P_((WINDOW win));
static CHAR	*htmltagatcursor P_((WINDOW win, MARK cursor));
static MARK	htmltagload P_((CHAR *tagname, MARK from));
static MARK	htmltagnext P_((MARK cursor));
#endif /* defined(DISPLAY_HTML) */

#if defined(DISPLAY_HTML) || defined(DISPLAY_TEX)
static twrap_t	htmlhr P_((TOKEN *token));
#endif

#if defined(DISPLAY_HTML) || defined(DISPLAY_MAN)
static twrap_t	manput P_((void));
#endif

#ifdef DISPLAY_MAN
static void	manescape P_((TOKEN *tok));
static int	manarg P_((TOKEN *token, int start, _char_ font, ELVBOOL spc));
static twrap_t	manTH P_((TOKEN *token));
static twrap_t	manSH P_((TOKEN *token));
static twrap_t	manBI P_((TOKEN *token));
static twrap_t	manIP P_((TOKEN *token));
static void	manmarkup P_((TOKEN *token));
static TOKEN	*manget P_((CHAR **refp));
static DMINFO	*maninit P_((WINDOW win));
#endif /* defined(DISPLAY_MAN) */

#ifdef DISPLAY_TEX
static void	texescape P_((TOKEN *tok));
static twrap_t	texscope P_((TOKEN *token));
static twrap_t	texoutput P_((TOKEN *token));
static twrap_t	texitem P_((TOKEN *token));
static twrap_t	textitle P_((TOKEN *token));
static twrap_t	texdigraph P_((TOKEN *token));
static long	texpair P_((CHAR **refp, TOKEN *token));
static TOKEN	*texget P_((CHAR **refp));
static DMINFO	*texinit P_((WINDOW win));
#endif /* defined(DISPLAY_TEX) */

static void	countchar P_((CHAR *p, long qty, _char_ font, long offset));
static twrap_t	put P_((TOKEN *token));
static void	term P_((DMINFO *info));
static long	mark2col P_((WINDOW w, MARK mark, ELVBOOL cmd));
static MARK	move P_((WINDOW w, MARK from, long linedelta, long column, ELVBOOL cmd));
static MARK	setup P_((WINDOW win, MARK top, long cursor, MARK bottom, DMINFO *info));
static MARK	image P_((WINDOW w, MARK line, DMINFO *info, void (*draw)(CHAR *p, long qty, _char_ font, long offset)));
static int	start P_((WINDOW win, MARK from, void (*draw)(CHAR *p, long qty, _char_ font, long offset)));
static void	storestate P_((long offset, LINEINFO *dest));
static void	findtitle P_((BUFFER buf));
#ifdef FEATURE_LPR
static void	header P_((WINDOW w, int pagenum, DMINFO *info, void (*draw)(CHAR *p, long qty, _char_ font, long offset)));
#endif

/* Only a single TOKEN is ever really needed at one time */
static TOKEN	rettok;

/* Offset of cursor.  This affects the expansion of escapes, and the visibility
 * of markups.
 */
static long	cursoff;

/* Offset & font of a space character, if "anyspc" is ElvTrue. */
static long	spcoffset;
static char	spcfont;	

/* This the drawchar pointer points to a function for outputting a single
 * character.
 */
static void	(*drawchar) P_((CHAR *p, long qty, _char_ font, long offset));

/* Special characters.  These are stored in variables rather than macros so
 * that we can pass their address to (*drawchar)().
 */
static CHAR	hyphen[1] = {'-'};
static CHAR	newline[1] = {'\n'};
static CHAR	formfeed[1] = {'\f'};
static CHAR	vtab[1] = {'\013'};
static CHAR	space[1] = {' '};
static CHAR	bullet1[1] = {'*'};
static CHAR	bullet2[1] = {'o'};

#ifdef DISPLAY_HTML
/*----------------------------------------------------------------------------*/
/* Array for converting single-letter font names into font codes.  The long
 * names of the fonts are converted into font codes in the initialization
 * function.
 */
static char	fontcode[128];

static void initfonts()
{
	static ELVBOOL first = ElvTrue;
	char	form;

	/* if not first time, then do nothing */
	if (!first)
		return;
	first = ElvFalse;

	/* initialize the fontcode[] array */
	fontcode['n'] = fontcode['N'] = colorfind(toCHAR("formatted"));
	fontcode['b'] = fontcode['B'] = colorfind(toCHAR("bold"));
	fontcode['i'] = fontcode['I'] = colorfind(toCHAR("italic"));
	fontcode['u'] = fontcode['U'] = colorfind(toCHAR("underlined"));
	fontcode['f'] = fontcode['F'] = colorfind(toCHAR("fixed"));
	fontcode['e'] = fontcode['E'] = colorfind(toCHAR("emphasized"));
	fontcode['l'] = fontcode['L'] = colorfind(toCHAR("link"));
	fontcode['d'] = fontcode['D'] = colorfind(toCHAR("definition"));
	fontcode['g'] = fontcode['G'] = colorfind(toCHAR("graphic"));
	fontcode['m'] = fontcode['M'] = colorfind(toCHAR("markup"));
	fontcode['*'] = colorfind(toCHAR("bullet"));
	fontcode['\b'] = colorfind(toCHAR("form_button"));
	fontcode['\r'] = colorfind(toCHAR("form_radio"));
	fontcode['\\'] = colorfind(toCHAR("form_check"));
	fontcode['\t'] = colorfind(toCHAR("form_text"));
	fontcode['\f'] = colorfind(toCHAR("form_other"));
	form = colorfind(toCHAR("form"));

	/* some hardcoded defaults for the appearances of those fonts */
	colorset(fontcode['n'], toCHAR("proportional"), ElvFalse);
	colorset(fontcode['b'], toCHAR("bold like formatted"), ElvFalse);
	colorset(fontcode['i'], toCHAR("italic like formatted"), ElvFalse);
	colorset(fontcode['u'], toCHAR("underlined like formatted"), ElvFalse);
	colorset(fontcode['e'], toCHAR("like bold"), ElvFalse);
	colorset(fontcode['f'], toCHAR("fixed like formatted"), ElvFalse);
	colorset(fontcode['l'], toCHAR("underlined blue like formatted"), ElvFalse);
	colorset(fontcode['d'], toCHAR("like bold"), ElvFalse);
	colorset(fontcode['g'], toCHAR("graphic like fixed"), ElvFalse);
	colorset(fontcode['*'], toCHAR("graphic"), ElvFalse);
	colorset(fontcode['m'], toCHAR("bold green"), ElvFalse);
	colorset(form, toCHAR("red"), ElvFalse);
	colorset(fontcode['\b'], toCHAR("boxed like form"), ElvFalse);
	colorset(fontcode['\r'], toCHAR("boxed like form"), ElvFalse);
	colorset(fontcode['\\'], toCHAR("boxed like form"), ElvFalse);
	colorset(fontcode['\t'], toCHAR("underlined like form"), ElvFalse);
	colorset(fontcode['\f'], toCHAR("boxed like form"), ElvFalse);
}


/*----------------------------------------------------------------------------*/
/* HTML-specific functions and variables                                      */

/* Replace entities such as &lt; with their single-character equivelent. */
static void htmlescape(token)
	TOKEN	*token;	/* a token whose text is to be expanded */
{
	char	*src, *dst;
	long	*off;
	size_t	len, truelen;
	int	width;
	int	i;

static struct {
	size_t	len;	/* overall length of the entity */
	char	name[6];/* name of the entity, without & ; or variable char */
	CHAR	c1,c2;	/* digraph chars; c2=0 if variable; c1=0 if c2 ascii */
	} entities[] = {
		{4,	"lt",		  0, '<'},
		{4,	"gt",		  0, '>'},
		{5,	"amp",		  0, '&'},
		{6,	"quot",		  0, '"'},
		{6,	"nbsp",		  0, ' '},
		{7,	"AElig",	'E', 'A'},
		{7,	"aelig",	'e', 'a'},
		{7,	"szlig",	's', 'z'},
		{8,	"grave",	'`',   0},
		{8,	"acute",	'\'',  0},
		{7,	"circ",		'^',   0},
		{8,	"tilde",	'~',   0},
		{6,	"uml",		'"',   0},
		{7,	"ring",		'*',   0},
		{8,	"cedil",	',',   0},
		{8,	"slash",	'/',   0},
		{7,	"ldquo",	  0, '"'},
		{7,	"rdquo",	  0, '"'},
		{7,	"lsquo",	  0, '`'},
		{7,	"rsquo",	  0,'\''},
		{7,	"tilde",	  0, '~'},
		{5,	"shy",		  0, '-'},
		{5,	"ETH",		'-', 'D'},
		{5,	"eth",		'-', 'd'},
		{7,	"THORN",	'T', 'P'},
		{7,	"thorn",	't', 'p'},
		{6,	"copy",		'O', 'c'},
		{5,	"reg",		'O', 'r'},
		{7,	"iexcl",	'~', '!'},
		{7,	"laquo",	'<', '<'},
		{7,	"raquo",	'>', '>'},
		{7,	"pound",	'$', 'L'},
		{6,	"cent",		'$', 'C'},
		{5,	"yen",		'$', 'Y'},
		{6,	"euro",		'$', 'E'},
		{5,	"deg",		'*', '*'},
#if USE_PROTOTYPES /* because K&R C can't handle 6 chars in a char[6] field */
		{8,	"iquest",	'~', '?'},
		{8,	"curren",	'$', 'X'},
		{8,	"percnt",	  0, '%'},
#endif
		{4,	"LT",		  0, '<'},
		{4,	"GT",		  0, '>'},
		{5,	"AMP",		  0, '&'},
		{6,	"QUOT",		  0, '"'}
	};

	/* step through the string */
	for (src = dst = (char *)token->text, off = token->offset, width = 0;
	     src < (char *)&token->text[token->nchars];
	     src++, off++, width++)
	{
		/* if not a &, then this can't be an escape */
		if (*src != '&')
			goto NoEscape;

		/* find the length of this escape's name */
		for (len = truelen = 1; src[len] != ';'; truelen++, len++)
		{
			if (!src[len] || elvspace(src[len]))
			{
				if (readonly)
				{
					truelen--;
					break;
				}
				else
					goto NoEscape;
			}
		}
		len++, truelen++;

		/* if the cursor is on this escape, then don't expand it */
		if ((o_showmarkups && off[0] <= cursoff && cursoff <= off[len - 1]) || list)
		{
			/* Tweak the value of "width" so that after all of
			 * this escape's characters have been counted, "width"
			 * will have been incremented by only 1 since that's
			 * how wide the escape would normally be.
			 */
			width -= (len - 1);

			/* Don't expand the escape */
			goto NoEscape;
		}

		/* recognize it? */
		if (src[1] == '#')
			*dst++ = atoi(&src[2]);
		else
		{
			for (i = 0; i < QTY(entities); i++)
			{
				if (len != entities[i].len)
					continue;
				if (entities[i].c2 == 0)
				{
					if (!strncmp(&src[2], entities[i].name, len - 3))
					{
						*dst++ = digraph(entities[i].c1, (_CHAR_)src[1]);
						break;
					}
				}
				else
				{
					if (!strncmp(&src[1], entities[i].name, len - 2))
					{
						if (entities[i].c1)
							*dst++ = digraph(entities[i].c1, entities[i].c2);
						else
							*dst++ = entities[i].c2;
						break;
					}
				}
			}

			/* did we find it? */
			if (i >= QTY(entities))
			{
				/* NO! Tweak the value of "width" so that after
				 * all of this escape's characters have been
				 * counted, "width" will have been incremented
				 * by only 1 since that's how wide the escape
				 * would normally be if it was recognized.
				 */
				width -= (truelen - 1);

				/* Don't expand the escape */
				goto NoEscape;
			}
		}

		/* Skip past the escape sequence. */
		token->offset[(int)(dst - (char *)token->text) - 1] = *off;
		src += truelen - 1;
		off += truelen - 1;
		/* plus one more at the for-loop */
		continue;

NoEscape:
		/* Not an escape -- copy it literally */
		*dst++ = *src;
		token->offset[(int)(dst - (char *)token->text) - 1] = *off;
	}

	/* compute the new length */
	token->nchars = (int)(dst - (char *)token->text);
	token->text[token->nchars] = '\0';
	token->width = width;
}

/* Output the "alt" text from an <img> tag */
static twrap_t htmlimg(token)
	TOKEN	*token;
{
	int	i, j;
	ELVBOOL	htmlurl;

	/* detect whether this is an SGML <htmlurl> or <ulink> tag */
	htmlurl = (ELVBOOL)(token->text[1] == 'h' || token->text[1] == 'u');

	/* look for an "alt=..." argument */
	for (i = 5; i < token->nchars && CHARncmp(&token->text[i - 5], toCHAR(" alt="), 5); i++)
	{
	}

	/* if no "alt=" then search for "name=" (This is for <frame ...>) */
	if (i >= token->nchars)
	{
		for (i = 6; i < token->nchars && CHARncmp(&token->text[i - 6], toCHAR(" name="), 6); i++)
		{
		}
	}

	/* if we still have no "alt=" then try "src=" or "url=" */
	if (i >= token->nchars)
	{
		for (i = 5;
		     i < token->nchars
			&& CHARncmp(&token->text[i - 5], toCHAR(" src="), 5)
			&& CHARncmp(&token->text[i - 5], toCHAR(" url="), 5);
		     i++)
		{
		}

		/* some images have long names -- can we trim this? */
		for (j = i + 1; j < token->nchars && token->text[j] != ' ' && token->text[j] != '"'; j++)
		{
		}
		while (--j >= i && token->text[j] != '/')
		{
		}
		i = j + 1;
	}

	/* decide how to display this image */
	if (i >= token->nchars)
	{
		/* there is no "alt=..." string, so display the tag name */
		i = 1;
		for (j = 1; j + i < token->nchars && elvalpha(token->text[j + i]); j++)
		{
		}
	}
	else if (token->text[i] == '"')
	{
		/* the "alt=" argument has a quoted argument */
		i++;
		for (j = i; j < token->nchars && token->text[j] != '"'; j++)
		{
		}
	}
	else
	{
		/* the "alt=" argument is unquoted */
		for (j = i;
		     j < token->nchars && token->text[j] != ' '
			&& token->text[j] != '"' && token->text[j] != '>';
		     j++)
		{
		}
	}

	/* limit length to 14 characters -- tables look better that way */
	if (!htmlurl && j > i + 14)
		j = i + 14;

	/* Insert a little arrow in front of the label, pointing to the label.
	 * This serves two purposes:  It ensures that a character will be output
	 * which has the same offset as the first character of the tag (which
	 * is desirable because that's where the <Tab> key leaves the cursor).
	 * And it gives the user an obvious place to double-click, to download
	 * the image.
	 */
	manlen = 0;
	if (!htmlurl)
	{
		mantext[0] = '>';
		manoffset[0] = token->offset[0];
		manfont[0] = fontcode['l'];
		manlen++;
	}

	/* copy it into the manput() arguments, and then put it. */
	for ( ; i < j && manlen < QTY(mantext); i++, manlen++)
	{
		mantext[manlen] = token->text[i];
		manoffset[manlen] = token->offset[i];
		manfont[manlen] = fontcode[htmlurl ? 'u' : 'N'];
	}
	return manput();
}


/* Set the modeinfo's "graphic" flag if <pre graphic> */
static twrap_t htmlpre(token)
	TOKEN	*token;
{
	int	i;

	for (graphic = ElvFalse, i = 0; i < token->nchars; i++)
	{
		if (token->text[i] == 'g')
		{
			graphic = ElvTrue;
			break;
		}
	}
	return TWRAP_NO;
}

/* List items are preceded by a less-indented number or bullet */
static twrap_t htmlli(token)
	TOKEN	*token;
{
	CHAR	buf[10];
	int	len;

	assert(col == 0);

	/* output a bullet or count */
	if (nest == 1 && listcnt > 0)
	{
		/* convert item# to characters */
		long2CHAR(buf, (long)listcnt++);
		CHARcat(buf, toCHAR(")"));
		len = CHARlen(buf);

		/* output whitespace for indentation */
		if (indent - len > 1)
		{
			(*drawchar)(space, 1 + len - indent, 0, -1);
			col += indent - len - 1;
		}

		/* output the item number */
		(*drawchar)(buf, len, 0, -1);
		col += len;
	}
	else
	{
		/* output whitespace for indentation */
		if (indent > 2)
		{
			(*drawchar)(space, 2 - indent, 0, -1);
			col += indent - 2;
		}

		/* output a bullet */
		(*drawchar)((nest & 1) ? bullet1 : bullet2, 1, fontcode['*'], -1);
		col++;
	}

	/* Note: We would like to do an assert(mui->col == mui->indent - 1)
	 * here, but if the number/bullet doesn't fit within the indentation
	 * space then our indentation might be off.  So we won't.
	 */
	return TWRAP_NO;
}

/* Form elements are shown as reverse-video areas */
static twrap_t htmlinput(token)
	TOKEN	*token;
{
	int	height;	/* 1 for input, 2 for textarea */
	int	width;	/* displayed width of item */
	int	vallen;	/* length of the value */
	int	validx;	/* index into token->text[] of initial value */
	ELVBOOL	button;	/* does this form item appear to be a button? */
	ELVBOOL	radio;	/* does this form item appear to be a radio button? */
	char	font;	/* font - 'E' for buttons, or 'N' for any other */
	int	mycol;
	int	i;

	/* parse the arguments */
	height = (token->text[1] == 't') ? 3 : 1;
	width = vallen = validx = 0;
	button = radio = ElvFalse;
	font = fontcode['\t'];
	for (i = 4; i < token->nchars; i++)
	{
		if (!CHARncmp(&token->text[i], toCHAR("value="), 6))
		{
			i += 6;
			if (token->text[i] == '"')
			{
				i++;
				validx = i;
				for (vallen = i; i < token->nchars && token->text[i] != '"'; i++)
				{
				}
			}
			else
			{
				validx = i;
				for (vallen = i; i < token->nchars && elvalnum(token->text[i]); i++)
				{
				}
			}
			vallen = i - vallen;
		}
		else if (!CHARncmp(&token->text[i], toCHAR("size="), 5)
		      || !CHARncmp(&token->text[i], toCHAR("cols="), 5))
		{
			i += 5;
			width = atoi(tochar8(&token->text[i]));
		}
		else if (!CHARncmp(&token->text[i], toCHAR("type="), 5))
		{
			i += 5;
			if (token->text[i] == '"')
				i++;
			if (!CHARncmp(&token->text[i], toCHAR("checkbox"), 8)
			 || !CHARncmp(&token->text[i], toCHAR("CHECKBOX"), 8))
			{
				/* CHECKBOX button */
				button = radio = ElvTrue;
				i += 8;
				font = fontcode['\\'];
			}
			else if (!CHARncmp(&token->text[i], toCHAR("hidden"), 6)
			      || !CHARncmp(&token->text[i], toCHAR("HIDDEN"), 6))
			{
				/* HIDDEN field -- do nothing with it */
				return TWRAP_NO;
			}
			else if (!CHARncmp(&token->text[i], toCHAR("radio"), 5)
			      || !CHARncmp(&token->text[i], toCHAR("RADIO"), 5))
			{
				/* RADIO button */
				button = radio = ElvTrue;
				i += 5;
				font = fontcode['\r'];
			}
			else if (token->text[i] != 't' && token->text[i] != 'T')
			{
				/* not TEXT, probably SUBMIT or RESET button */
				button = ElvTrue;
				font = fontcode['\b'];
			}
		}
		else if (!CHARncmp(&token->text[i], toCHAR("checked"), 7))
		{
			/* font = fontcode['\\'] BUT WITH HIGHLIGHTING */
			i += 7;
		}
	}

	/* Most buttons are always as wide as their value, but radio & checkbox
	 * buttons only need to show a single character.
	 */
	if (radio)
	{
		vallen = 1;
	}
	if (button)
	{
		width = vallen;
	}

	/* remember the column */
	mycol = col;
	if (mycol < indent)
		mycol = indent;
	else if (anyspc)
	{
		mycol++;
		anyspc = ElvFalse;
	}

	/* will it fit on this line? */
	if (!first && mycol + width > textwidth)
	{
		/* no it won't */
		(*drawchar)(newline, 1, 0, -1);
		col = 0;
		return TWRAP_BEFORE;
	}

	/* output the image */
	for (i = 1; i <= height; i++)
	{
		if (col > mycol)
		{
			(*drawchar)(newline, 1, 0, -1);
			col = 0;
		}
		if (col < mycol)
		{
			(*drawchar)(space, col - mycol , 0, -1);
			col = mycol;
		}
		if (vallen > 0)
			(*drawchar)(&token->text[validx], vallen,
				font, token->offset[validx]);
		if (width > vallen)
			(*drawchar)(space, vallen - width, /* <- negative! */
				font, token->offset[token->nchars - 1]);
		col += width;
	}
	anyspc = ElvFalse;

	return TWRAP_NO;
}

/* switch to underline if this is an href anchor (else leave the font unchanged
 * for name anchor).
 */
static twrap_t htmla(token)
	TOKEN	*token;
{
	/* whether we set the font or not, </a> will reset it so we always
	 * need to store the current font.
	 */
	deffont = curfont;

	/* if the token starts with "a href" then force font to "link" */
	if (!CHARncmp(token->text, toCHAR("<a href="), 8))
		curfont = fontcode['l'];

	/* zero width, always fits on line */
	return TWRAP_NO;
}

/* Look up an html markup token in a table */
static int htmlmarkup(token)
	TOKEN	*token;	/* the token to lookup */
{
	static MARKUP	tbl[] =
	{

		/* Tag		 Effects	Function	*/
		/*               TBILFFD                        */

#ifdef SGML_HACK
		/* These are SGML tags.  We only consider using these when
		 * the display mode is "html sgml".
		 */
		{ "title",	"N0--b--"			},
		{ "/title",	"N2|-=--"			},
		{ "subtitle",	"N0--i--"			},
		{ "/subtitle",	"N2|-=--"			},
		{ "/",		"N---=--"			},
		{ "author",	"N0--I--"			},
		{ "date",	"N0--N--"			},
		{ "sect",	"Np0-BYS"			}, /* <h1> */
		{ "sect1",	"Nc1-BYS"			}, /* <h2> */
		{ "sect2",	"N12-BYS"			}, /* <h3> */
		{ "para",	"N1|-NYP"			}, /* <p> */
		{ "simpara",	"N1--NYP"			}, /* <p> */
		{ "orderedlist","N->#-YP"			}, /* <ol> */
		{ "/orderedlist","N0<N-Y-"			}, /* </ol> */
		{ "itemizedlist","N->Y-YP"			}, /* <ul> */
		{ "/itemizedlist","N0<N-Y-"			}, /* </ul> */
		{ "listitem",	"N0-----",	htmlli		}, /* <li> */
		{ "enum",	"N->#-YP"			}, /* <ol> */
		{ "/enum",	"N0<N-Y-"			}, /* </ol> */
		{ "itemize",	"N->Y-YP"			}, /* <ul> */
		{ "/itemize",	"N0<N-Y-"			}, /* </ul> */
		{ "item",	"N0-----",	htmlli		}, /* <li> */
		{ "abstract",	"N14-NYP"			}, /* </blockquote> */
		{ "/abstract",	"N12-NY-"			}, /* </blockquote> */
		{ "verb",	"N0---NP"			},
		{ "/verb",	"N0---Y-"			},
		{ "tscreen",	"N0>-F--",	htmlpre		},
		{ "/tscreen",	"N0<-N--"			},
		{ "descrip",	"N-2-NYS"			}, /* <dl> */
		{ "/descrip",	"N02-NY-"			}, /* </dl> */
		{ "tag",	"N12-BYP"			}, /* <dt> */
		{ "/tag",	"N03-NY-"			}, /* <dd> */
		{ "variablelist","N-2-NYS"			}, /* <dl> */
		{ "/variablelist","N02-NY-"			}, /* </dl> */
		{ "term",	"N12-BYP"			}, /* <dt> */
		{ "/term",	"N03-NY-"			}, /* <dd> */
		{ "htmlurl",	"N-----T",	htmlimg		}, /* <img> */
		{ "url",	"N-----T",	htmlimg		}, /* <img> */
		{ "ulink",	"N---u-T"			}, /* <a> */
		{ "/ulink",	"N---=--"			}, /* </a> */
		{ "replaceable","N---i--"			}, /* <var> */
		{ "/replaceable","N---=--"			}, /* </var> */
		{ "bf",		"N---b--"			}, /* <b> */
		{ "/bf",	"N---=--"			}, /* </b> */
		{ "sf",		"N---b--"			}, /* <strong>*/
		{ "/sf",	"N---=--"			}, /*</strong>*/
		{ "it",		"N---i--"			}, /* <i> */
		{ "/it",	"N---=--"			}, /* </i> */
		{ "sl",		"N---i--"			}, /* <i> */
		{ "/sl",	"N---=--"			}, /* </i> */
		{ "informaltable",	"N0|-NY-"		}, /* <table> */
		{ "/informaltable",	"N0@-NY-"		}, /* </table>*/
		{ "row",	"N02--Y-"			}, /* <tr> */
		{ "entry",	"N-=-NY-"			}, /* <td> */
		{ "literallayout","N0---NP"			},
		{ "/literallayout","N0---Y-"			},
#endif /* SGML_HACK */

		/* These are HTML tags */
		/* Tag		 Effects	Function	*/
		/*               TBILFFD                        */
		{ "html",	"Y-2-NY-"			},
		{ "/html",	"N-2-NY-"			},
		{ "head",	"Y-2-NY-"			},
		{ "/head",	"N-2-NY-"			},
		{ "title",	"Y-2-NY-"			},
		{ "/title",	"N-2-NY-"			},
		{ "body",	"N-2-NY-"			},
		{ "/body",	"N-2-NY-"			},
		{ "h1",		"Np0-BYS"			},
		{ "/h1",	"N12-NY-"			},
		{ "h2",		"Nc1-BYS"			},
		{ "/h2",	"N12-NY-"			},
		{ "h3",		"N12-BYS"			},
		{ "/h3",	"N02-NY-"			},
		{ "h4",		"N12-IY-"			},
		{ "/h4",	"N02-NY-"			},
		{ "h5",		"N12-IY-"			},
		{ "/h5",	"N02-NY-"			},
		{ "h6",		"N12-IY-"			},
		{ "/h6",	"N02-NY-"			},
		{ "p",		"N1|-NYP"			},
		{ "hr",		"N0-----",	htmlhr		},
		{ "img",	"N------",	htmlimg		},
		{ "frame",	"N-----T",	htmlimg		},
		{ "embed",	"N-----T",	htmlimg		},
		{ "br",		"N0---Y-"			},
		{ "table",	"N0|-NY-"			},
		{ "/table",	"N0@-NY-"			},
		{ "tr",		"N0@--Y-"			},
		{ "th",		"N-=-BY-"			},
		{ "td",		"N-=-NY-"			},
		{ "blockquote",	"N14-NYP"			},
		{ "/blockquote","N12-NY-"			},
		{ "pre",	"N0--FNP",	htmlpre		},
		{ "/pre",	"N0--NY-"			},
		{ "dir",	"N0>-FNP",	htmlpre		},
		{ "/dir",	"N0<-NY-"			},
		{ "xmp",	"N0>-FN-",	htmlpre		},
		{ "/xmp",	"N0<-NY-"			},
		{ "dl",		"N-2-NYS"			},
		{ "/dl",	"N02-NY-"			},
		{ "dt",		"N12-DYP"			},
		{ "dd",		"N03-NY-"			},
		{ "ol",		"N->#-YP"			},
		{ "/ol",	"N0<N-Y-"			},
		{ "ul",		"N->Y-YP"			},
		{ "/ul",	"N0<N-Y-"			},
		{ "menu",	"N->Y-Y-"			},
		{ "/menu",	"N-<N-Y-"			},
		{ "li",		"N0-----",	htmlli		},
		{ "input",	"N-----T",	htmlinput	},
		{ "textarea",	"N-----T",	htmlinput	},
		{ "a",		"N-----T",	htmla		},
		{ "/a",		"N---=--"			},
		{ "cite",	"N---i--"			},
		{ "/cite",	"N---=--"			},
		{ "dfn",	"N---d--"			},
		{ "/dfn",	"N---=--"			},
		{ "em",		"N---i--"			},
		{ "/em",	"N---=--"			},
		{ "kbd",	"N---b--"			},
		{ "/kbd",	"N---=--"			},
		{ "strong",	"N---b--"			},
		{ "/strong",	"N---=--"			},
		{ "var",	"N---i--"			},
		{ "/var",	"N---=--"			},
		{ "address",	"N---i--"			},
		{ "/address",	"N---=--"			},
		{ "code",	"N---f--"			},
		{ "/code",	"N---=--"			},
		{ "b",		"N---b--"			},
		{ "/b",		"N---=--"			},
		{ "i",		"N---i--"			},
		{ "/i",		"N---=--"			},
		{ "u",		"N---u--"			},
		{ "/u",		"N---=--"			},
		{ "tt",		"N---f--"			},
		{ "/tt",	"N---=--"			},

		{ (char *)0,	"N------"			}
	};
	MARKUP	*scan;	/* used for scanning the tbl[] array */
	int	len;	/* length of the markup */

	/* find the length of the markup's name */
	assert(token->nchars > 1 && token->text[0] == '<');
	for (len = 1;
	     len < token->nchars &&
		((len == 1 && token->text[len] == '/') || elvalnum(token->text[len]));
	     len++)
	{
	}
	len--; /* since we started at 1 */

	/* look it up in the table */
#ifdef SGML_HACK
	for (scan = mui ? &tbl[mui->flavor] : tbl; /* won't always have mui */
#else
	for (scan = tbl;
#endif
	     scan->name &&
		(strlen(scan->name) != (unsigned)len ||
		    strncmp(scan->name, (char *)token->text + 1, (size_t)len));
	     scan++)
	{
	}

	/* remember it */
	token->markup = scan;

	/* return the index of the markup in tbl[] */
	return (int)(scan - tbl);
}

/* Read the next token. */
static TOKEN *htmlget(refp)
	CHAR	**refp;	/* address of a (CHAR *) used for scanning */
{
	long	offset;
	ELVBOOL lower, nameonly;
	int	i;

	/* if the CHAR pointer is NULL, then return NULL */
	if (!*refp)
		return NULL;

	/* Get first character of token */
	offset = markoffset(scanmark(refp));
	rettok.text[0] = **refp;
	rettok.offset[0] = offset++;
	rettok.nchars = 1;
	rettok.markup = NULL;
	scannext(refp);

#ifdef SGML_HACK
	/* If '/' and we were expecting '/' to be interpreted as an SGML-style
	 * terminator, then do that.
	 */
	if (rettok.text[0] == '/' && *sgmltag)
	{
		/* replace the '/' with the SGML terminator tag name */
		CHARcpy(rettok.text, sgmltag);
		for ( ; rettok.text[rettok.nchars]; rettok.nchars++)
			rettok.offset[rettok.nchars] = rettok.offset[0];

		/* lookup the markup */
		(void)htmlmarkup(&rettok);

		/* when computing line breaks, assume this markup is hidden */
		rettok.width = 0;

		/* clear the sgmltag variable... unless this tag forces a
		 * newline before it is processed, in which case we need to
		 * return the same tag twice so we can't clobber it the first
		 * time.
		 */
		if (rettok.markup->BREAKLN == '-' || first)
			*sgmltag = '\0';

		return &rettok;
	}
#endif /* SGML_HACK */

	/* If '<' then token is a markup */
	if (rettok.text[0] == '<')
	{
		/* This is a markup.  Collect characters up to next '>' */
		for (lower = nameonly = ElvTrue;
		     *refp && **refp != '>' && !(**refp == '/' && nameonly && rettok.nchars > 1);
		     offset++, scannext(refp))
		{
			/* If this is a comment token, then incorporate
			 * everything and including the next hyphen before
			 * going back to the top of the loop.  This way,
			 * '>' chars in the comment won't be erroneously
			 * detected as the end of the comment.
			 */
			if (rettok.nchars >= 3
			 && rettok.text[1] == '!'
			 && rettok.text[2] == '-')
			{
				while (*refp && **refp != '-')
				{
					if (rettok.nchars <= QTY(rettok.text) - 3)
					{
						rettok.text[rettok.nchars] = **refp;
						rettok.offset[rettok.nchars] = offset;
						rettok.nchars++;
					}
					offset++;
					scannext(refp);
				}
				if (!*refp)
					break;

				/* else we found a '-' which should be processed
				 * normally, so continue on with the normal body
				 * of the loop...
				 */
			}
			
			/* if token text is full, then skip this char */
			if (rettok.nchars >= QTY(rettok.text) - 2)
				continue;

			/* if not alphanumeric, then clear nameonly */
			if (nameonly && !elvalnum(**refp))
				nameonly = ElvFalse;

			/* Store the character.  This is a little complex
			 * because we want to convert uppercase tags and
			 * parameter names to lowercase, but leave parameter
			 * values unchanged.  Also, any whitespace character
			 * should be displayed as a space.
			 */
			if (**refp == '=')
				lower = ElvFalse;
			else if (elvspace(**refp))
				lower = ElvTrue;

			if (**refp <= ' ')
				rettok.text[rettok.nchars] = ' ';
			else if (elvupper(**refp) && lower)
				rettok.text[rettok.nchars] = elvtolower(**refp);
			else
				rettok.text[rettok.nchars] = **refp;
			rettok.offset[rettok.nchars++] = offset;
		}

#ifdef SGML_HACK
		/* if terminated with / then remember that the next / should be
		 * the ternminator.  So "<tt/some text/" is interpreted as
		 * "<tt>some text</tt>".  (This is an SGML thing.)
		 */
		if (*refp && **refp == '/')
		{
			CHARncpy(sgmltag, toCHAR("</"), sizeof sgmltag);
			CHARncpy(sgmltag + 2, rettok.text + 1, rettok.nchars - 1);
			CHARcat(sgmltag, toCHAR(">"));
		}
#endif /* SGML_HACK */

		/* store the terminating '>' */
		rettok.text[rettok.nchars] = '>';
		rettok.offset[rettok.nchars] = offset;
		rettok.nchars++;
		rettok.text[rettok.nchars] = '\0';
		if (*refp)
			scannext(refp);

		/* lookup the markup */
		(void)htmlmarkup(&rettok);

		/* when computing line breaks, assume this markup is hidden */
		rettok.width = 0;
	}
	else if (rettok.text[0] <= ' ')
	{
		/* This is a whitespace token.  Each whitespace token contains
		 * a SINGLE whitespace character.  Control characters other
		 * than '\t' and '\n' are treated as spaces.
		 */
		if (rettok.text[0] != '\t' && rettok.text[0] != '\n')
		{
			rettok.text[0] = ' ';
		}

		/* when computing line breaks, assume this whitespace shows */
		rettok.width = 1;
	}
	else
	{
		/* This is a word.  Collect characters up to next whitespace */
		for (;
		     *refp 
			&& rettok.nchars < QTY(rettok.text) - 1
			&& **refp > ' ' /* !elvspace(**refp) */
#ifdef SGML_HACK
			&& !(**refp == '/' && *sgmltag)
#endif
			&& **refp != '<';
		     offset++, scannext(refp))
		{
			rettok.text[rettok.nchars] = **refp;
			rettok.offset[rettok.nchars] = offset;
			rettok.nchars++;
		}

		/* For now, when computing line breaks, assume each character
		 * of this word is normally visible.  When character escapes
		 * are processed, this may change.
		 */
		rettok.width = rettok.nchars;

		/* If this is a readonly file, then be forgiving about illegal
		 * characters.  Many Windows HTML editors produce documents
		 * which use characters 0x80-0x9f for kerned punctuation.
		 */
		if (readonly && o_nonascii == 'm')
		{
			for (i = 0; i < rettok.width; i++)
			{
				switch (rettok.text[i])
				{
				  case 0x85:	rettok.text[i] = ':';	break;
				  case 0x91:	rettok.text[i] = '`';	break;
				  case 0x92:	rettok.text[i] = '\'';	break;
				  case 0x93:	rettok.text[i] = '"';	break;
				  case 0x94:	rettok.text[i] = '"';	break;
				  case 0x96:	rettok.text[i] = '-';	break;
				  case 0x97:	rettok.text[i] = '-';	break;
				}
			}
		}
	}

	/* Mark the end of the text, and return the token */
	rettok.text[rettok.nchars] = '\0';
	return &rettok;
}


/* start the mode, and allocate dminfo */
static DMINFO *htmlinit(win)
	WINDOW	win;
{
	long	cursoffset;	/* offset of cursor */
	TOKEN	*token;
	MARKBUF	top;
	CHAR	*p;
	twrap_t	result;

	/* inherit some functions from dmnormal */
	dmhtml.wordmove = dmnormal.wordmove;

	/* initialize the fonts */
	initfonts();

	/* if merely initializing options, we're done */
	if (!win)
		return NULL;

	/* allocate the info struct */
	mui = (MUINFO *)safealloc(1, sizeof(MUINFO));
	mui->get = htmlget;
	mui->escape = htmlescape;
	mui->line = (LINEINFO *)safealloc(GRANULARITY, sizeof(LINEINFO));
	mui->nlines = 1; /* every buffer has at least one line */

	/* decide which tags we should use -- HTML or SGML */
#ifdef SGML_HACK
	mui->flavor = 0;
	if (CHARcmp(o_display(win), toCHAR("html sgml")))
	{
		/* we want plain HTML -- find the first HTML tag */
		CHARcpy(rettok.text, toCHAR("<html>"));
		rettok.nchars = 6;
		mui->flavor = htmlmarkup(&rettok);
	}
	assert(mui->flavor < 100);
#endif /* SGML_HACK */

	/* temporarily move the cursor someplace harmless */
	cursoffset = markoffset(win->cursor);
	marksetoffset(win->cursor, o_bufchars(markbuffer(win->cursor)));

	/* format the buffer */
	win->mi = (DMINFO *)mui;
	(void)start(win, marktmp(top, markbuffer(win->cursor), 0), NULL);
	p = scanalloc(&p, &top);
	result = TWRAP_NO;
	for (token = (*mui->get)(&p);
	     token;
	     token = (*mui->get)(&p))
	{
		/* if previous token forced a newline after it, then this token
		 * is the start of the next line.
		 */
		if (result == TWRAP_AFTER)
		{
			/* assert(first == ElvTrue && col == 0); */
			storestate(token->offset[0], NULL);
		}

		/* if cursor is at position 0, and this is a text token,
		 * then move the cursor here.  This is done because otherwise
		 * the cursor would always start on an ugly formatting code.
		 */
		if (cursoffset == 0L 
		 && !title
		 && !elvspace(token->text[0])
		 && !token->markup)
		{
			cursoffset = token->offset[0];
		}

		/* output the token.  If it forces a new line, remember where
		 * that new line started.
		 */
		result = put(token);
		if (result == TWRAP_BEFORE)
		{
			assert(first == ElvTrue && col == 0);
			storestate(token->offset[0], NULL);
			(void)put(token);
		}
	}
	scanfree(&p);

	/* locate the title */
	findtitle(markbuffer(win->cursor));

	/* set the cursor back to its original position, or its adjusted one */
	marksetoffset(win->cursor, cursoffset);

	/* Done! */
	return (DMINFO *)mui;
}


/* Return a dynamically-allocated string containing the name of the tag at
 * the cursor, or NULL if the cursor isn't on a tag.
 */
static CHAR *htmltagatcursor(win, cursor)
	WINDOW	win;
	MARK	cursor;
{
	CHAR	*ret;	/* the return value */
	CHAR	*p;	/* used for scanning */
	TOKEN	*token;	/* a token from the buffer */
	TOKEN	anchor;	/* copy of last <a...> or </a> token */
	long	endoffset;/* where to stop scanning */
	int	i;	/* index into mui->line */
	ELVBOOL	anyviz;	/* have any visible tokens been encountered? */
	MARKBUF	tmp;
	int	lb;	/* line break counter */

	/* We need to find the last <a ...> or </a> tag which occurs before
	 * before the cursor.  Since we can't read tokens backward, we must
	 * start scanning at the beginning of the line and read forward until
	 * we pass the cursor, remembering the last <a ...> or </a> as we go.
	 * If we haven't found one by the time we pass the cursor, then we
	 * need to restart scanning at the beginning of the preceding line.
	 */

	/* find the start of the cursor's line */
	mui = (MUINFO *)win->mi;
	if (mui->nlines == 0)
		return NULL;
	endoffset = markoffset(cursor) + 1;
	for (i = 1; i < mui->nlines && mui->line[i].offset <= endoffset; i++)
	{
	}
	i--; /* the above loop went at least one line too far -- maybe more */

	/* read forward from the start of the line, watching for <a ...> and
	 * </a> tags.  Stop when we know we've found the last one before the
	 * cursor.
	 */
	anchor.text[0] = 0;
	anchor.text[1] = '/';
	do
	{
		scanalloc(&p, marktmp(tmp, markbuffer(cursor), mui->line[i].offset));
		anyviz = ElvFalse;
		lb = 3;
		while ((token = htmlget(&p)) != NULL
				&& (!anyviz || token->offset[0] < endoffset))
		{
			if (!token->markup)
			{
				if (!elvspace(token->text[0]))
					anyviz = ElvTrue;
				continue;
			}
			if (!CHARncmp(token->text, toCHAR("<a "), 3)
			 || !CHARcmp(token->text, toCHAR("</a>"))
			 || !CHARncmp(token->text, toCHAR("<frame "), 7)
			 || !CHARncmp(token->text, toCHAR("<embed "), 7)
			 || !CHARncmp(token->text, toCHAR("<htmlurl "), 9)
			 || !CHARncmp(token->text, toCHAR("<ulink "), 7)
			 || (anchor.text[1] == '/'
				&& !CHARncmp(token->text, toCHAR("<img "), 5)))
			{
				anchor = *token;
				anyviz = ElvFalse;
			}
			if (!CHARncmp(token->text, toCHAR("<img "), 5))
				anyviz = ElvTrue;

			/* Don't look across too many line breaks, for speed */
			if (token->markup->BREAKLN != '-'
			 || (token->markup->DEST != '-' && token->markup->DEST != 'T'))
				lb--;
			if (lb <= 0)
				break;
		}
		scanfree(&p);
#if 0
		endoffset = mui->line[i].offset;
#endif
	} while (anchor.text[0] == 0 && --i >= 0);

	/* If we found an <a ...> tag, then generate a dynamically-allocated
	 * copy of the URL.
	 */
	ret = NULL;
	if (anchor.text[1] != '/')
	{
		/* search for an HREF=, SRC=, or URL= parameter */
		for (p = &anchor.text[3];
		     CHARncmp(p, toCHAR("href="), 5)
			&& CHARncmp(p, toCHAR("src="), 4)
			&& CHARncmp(p, toCHAR("url="), 4);
		     p++)
		{
			if (*p == '>' || !*p)
				return NULL;
		}
		p += (*p=='h' ? 5 : 4);

		/* Copy the URL.  Beware of quotes! */
		if (*p == '"')
		{
			while (*++p != '"' && *p)
			{
				buildCHAR(&ret, *p);
			}
		}
		else
		{
			do
			{
				buildCHAR(&ret, *p);
			} while (*++p != ' ' && *p && *p != '>');
		}
	}

	/* return the URL or a NULL pointer */
	return ret;
}


static MARK htmltagload(tagname, from)
	CHAR	*tagname;
	MARK	from;		/* where the cursor is while we're loading */
{
 static	MARKBUF	retmark;	/* the return value */
	char	*filename;	/* name of file containing tag */
	char	*inherit;	/* part of filename that reference is from */
	CHAR	*anchorname;	/* name of tag's anchor */
	CHAR	separator;	/* '\0', '#', or '?' from URL */
	ELVBOOL	wasmagic;	/* original value of o_magic */
	ELVBOOL	hasprotocol;	/* is a network protocol specified in tagname? */
	EXINFO	xinfb;		/* ex command, holds result of parsing addr */
	CHAR	*p, *addr;
	TOKEN	*token;
	int	i, j;
	char	*tmp, *fnfree;

	if (o_verbose >= 5)
		msg(MSG_INFO, "[SS]htmltagload\\(tagname=$1, from=$2\\)", tagname, from ? o_bufname(markbuffer(from)) : toCHAR("NULL"));

	/* if no tagname is given, then fail */
	if (!tagname || !*tagname)
		return NULL;

	/* If protocol is "buffer:" then use the remainder of the tagname as
	 * a buffer name, and return the changepos of that buffer
	 */
	if (!CHARncmp(tagname, "buffer:", 7))
	{
		/* find the buffer */
		retmark.buffer = buffind(tagname + 7);
		if (!retmark.buffer)
		{
			msg(MSG_ERROR, "[S]no buffer named $1", tagname + 7);
			return NULL;
		}
		retmark.offset = retmark.buffer->docursor;
		return &retmark;
	}

	/* if protocol is "file:", OR "http:" without a host, then skip that. */
	hasprotocol = ElvFalse;
	if ((!CHARncmp(tagname, "file:", 5))
	  || (!CHARncmp(tagname, "http:", 5) && (tagname[5] != '/' || tagname[6] != '/')))
		tagname += 5;
	else if (elvalnum(tagname[0]) && elvalnum(tagname[1]) && CHARchr(tagname, ':'))
	{
		for (i = 2; elvalnum(tagname[i]); i++)
		{
		}
		if (tagname[i] == ':')
		{
			hasprotocol = ElvTrue;
		}
	}

	/* separate the tagname into filename and anchorname */
	fnfree = filename = (char *)safealloc((int)CHARlen(tagname) + 1, sizeof(char));
	for (anchorname = tagname, i = 0;
	     *anchorname && *anchorname != '#' && *anchorname != '?';
	     anchorname++, i++)
	{
		filename[i] = *anchorname;
	}

	/* For any protocol except "file", if the anchor is delimited with
	 * '?' then it will be passed along with the request.
	 */
	if ((!CHARncmp(tagname, toCHAR("http:"), 5)
		|| (elvalpha(*tagname) && !CHARncmp(tagname+1, toCHAR("http:"), 5)))
	  && *anchorname == '?')
	{
		while (*anchorname)
		{
			filename[i++] = *anchorname++;
		}
	}

	/* mark the end of the filename */
	filename[i] = '\0';

	/* Skip the '#' or '?' at the start of the anchor name, if any */
	separator = *anchorname++;

	/* Load the file into a buffer.  If the filename isn't a full pathname
	 * then assume that the file is in the same directory as the current
	 * file.  In fact, check to see if it is the same buffer first!
	 */
	if (!filename[0]
	    || (from
		&& o_filename(markbuffer(from))
		&& !CHARcmp(o_filename(markbuffer(from)), toCHAR(filename))))
	{
		/* in same buffer */
		marktmp(retmark, markbuffer(from), 0);
	}
	else if (from && o_filename(markbuffer(from)) && !hasprotocol
	 && (elvalpha(*filename) || *filename == '.' || *filename == '/' /*!!!*/
	 	|| urllocal(tochar8(o_filename(markbuffer(from))))
	 			!= tochar8(o_filename(markbuffer(from))) ))
	{
		/* get the directory part of the origin name */
		inherit = dirdir(tochar8(o_filename(markbuffer(from))));

		/* if it contains '?' or '#' then lop that off and try again */
		if ((tmp = strchr(inherit, '?')) != NULL
		 || (tmp = strchr(inherit, '#')) != NULL)
		{
			*tmp = '\0';
			inherit = dirdir(inherit);
		}

		/* if the inherit string contains a pathname, and the new URL
		 * doesn't, then combine the new URL with the inherit pathname.
		 */
		if (strlen(inherit) > 2
		 && inherit[strlen(inherit) - 1] == '/'
		 && !urllocal(tochar8(o_filename(markbuffer(from)))))
			strcpy(inherit, tochar8(o_filename(markbuffer(from))));
		if (*filename == '/')
		{
			/* remove '/' from name -- use '/' from inherit */
			filename++;

			/* make inherit[] use the root dir, not inherited dir */
			tmp = strchr(inherit, '/');
			if (tmp && tmp[1] == '/')
				tmp = strchr(tmp + 2, '/');
			if (tmp)
				tmp[1] = '\0';
		}
		tmp = (char *)safealloc(strlen(inherit) + strlen(filename) + 2, sizeof(char));
		if (*inherit && inherit[strlen(inherit) - 1] == '/')
			sprintf(tmp, "%s%s", inherit, filename);
		else
			sprintf(tmp, "%s/%s", inherit, filename);
		marktmp(retmark, bufload(NULL, tmp, ElvFalse), 0);
		safefree(tmp);
	}
	else
	{
		marktmp(retmark, bufload(NULL, filename, ElvFalse), 0);
	}
	safefree(fnfree);
	if (!markbuffer(&retmark) || o_bufchars(markbuffer(&retmark)) == 0)
	{
		return NULL;
	}

	/* scan forward from top for an anchor token with this name */
	if (separator == '#')
	{
		i = (int)CHARlen(anchorname);
		scanalloc(&p, &retmark);
		mui = NULL;
		for (token = htmlget(&p); token; token = htmlget(&p))
		{
			/* ignore if not markup or not a possible target */
			if (!token->markup || token->markup->DEST == '-')
				continue;

			/* scan for "id=" or "name=" */
			for (j = 2; j < token->nchars; j++)
			{
				if (!CHARncmp(&token->text[j], toCHAR(" name="), 6))
				{
					j += 6;
					break;
				}
				if (!CHARncmp(&token->text[j], toCHAR(" id="), 4))
				{
					j += 4;
					break;
				}
			}
			if (j >= token->nchars)
				continue;

			/* compare to sought name.  Beware of quotes */
			if (token->text[j] == '"'
			    ? !CHARncmp(&token->text[++j], anchorname, (size_t)i)
				&& token->text[j + i] == '"'
			    : !CHARncmp(&token->text[j], anchorname, (size_t)i)
				&& (token->text[j + i] == '>' || token->text[j + i] == ' '))
			{
				break;
			}
		}

		/* skip to the following non-whitespace text token */
		while (token && (token->markup || elvspace(token->text[0])))
		{
			token = htmlget(&p);
		}

		/* did we find the tag? */
		if (token)
		{
			marksetoffset(&retmark, token->offset[0]);
		}
		else
		{
			msg(MSG_WARNING, "[S]anchor $1 not found", anchorname);
		}
		scanfree(&p);
	}
	else if (separator == '?')
	{
		/* convert address from URL-encoded form to plain text */
		addr = NULL;
		while (*anchorname)
		{
			if (*anchorname == '+')
			{
				buildCHAR(&addr, ' ');
				anchorname++;
			}
			else if (*anchorname == '%' && anchorname[1] && anchorname[2])
			{
				anchorname++;
				if (elvdigit(*anchorname))
					separator = *anchorname - '0';
				else
					separator = (*anchorname & 0xf) + 9;
				separator <<= 4;
				anchorname++;
				if (elvdigit(*anchorname))
					separator |= *anchorname - '0';
				else
					separator |= (*anchorname & 0xf) + 9;
				buildCHAR(&addr, separator);
				anchorname++;
			}
			else
			{
				buildCHAR(&addr, *anchorname++);
			}
		}

		/* search for an address */
		scanstring(&p, addr);
		memset((char *)&xinfb, 0, sizeof xinfb);
		(void)marktmp(xinfb.defaddr, markbuffer(&retmark), 0);
		wasmagic = o_magic;
		o_magic = ElvFalse;
		if (!exparseaddress(&p, &xinfb))
		{
			xinfb.from = 1;
		}
		scanfree(&p);
		o_magic = wasmagic;
		if (xinfb.fromoffset)
			marksetoffset(&retmark, xinfb.fromoffset);
		else
			marksetoffset(&retmark, lowline(bufbufinfo(markbuffer(&retmark)), xinfb.to));
		exfree(&xinfb);
		safefree(addr);
	}

	return &retmark;
}


static MARK htmltagnext(cursor)
	MARK	cursor;
{
	CHAR	*p;	/* used for scanning */
	TOKEN	*token;	/* a token from the text */
 static MARKBUF	ret;	/* return value */

	/* read forward to next <a ...> token with an href parameter */
	for (scanalloc(&p, cursor);
	     (token = htmlget(&p)) != NULL
		&& (!token->markup
			|| token->markup->DEST != 'T'
			|| !CHARncmp(token->text, "<a name=", 8)
			|| token->offset[0] == markoffset(cursor));
	     )
	{
	}

	/* if we found an <a ...> token, then skip ahead to the next text token
	 * or the </a> token.
	 */
	if (p && !CHARncmp(token->text, toCHAR("<a "), 3))
	{
		while ((token = htmlget(&p)) != NULL
			&& elvspace(token->text[0]))
		{
		}
	}
	scanfree(&p);

	/* if we couldn't find anyplace, return NULL */
	if (!token)
	{
		return NULL;
	}

	/* else construct a mark for the start of the last token */
	return marktmp(ret, markbuffer(cursor), token->offset[0]);
}
#endif /* defined(DISPLAY_HTML) */

#if defined(DISPLAY_HTML) || defined(DISPLAY_TEX)
/* output a horizontal rule */
static twrap_t htmlhr(token)
	TOKEN	*token;
{
	int	len;

	assert(col == 0 && textwidth > 0);

	len = textwidth - indent * 3 / 2;
	if (len < 3)
		len = 3;

	/* draw the hrule */
	(*drawchar)(space, -indent, 0, (anyspc ? spcoffset : 1));
	(*drawchar)(hyphen, -len, fontcode['g'], token->offset[0]);
	col = indent + len;
	anyspc = ElvFalse;
	return TWRAP_NO;
}
#endif /* defined(DISPLAY_HTML) || defined(DISPLAY_TEX) */

#if defined(DISPLAY_HTML) || defined(DISPLAY_MAN)
/* This function either outputs mantext[] and returns ElvFalse, or (if mantext[]
 * is too wide to fit on this line) it outputs a newline and returns ElvTrue.
 */
static twrap_t manput()
{
	int	i, start;

	/* if it won't fit, then output a newline instead */
	if (!first && col > indent && col + manlen > textwidth - listind)
	{
		/* output a newline */
		(*drawchar)(newline, 1, 0, anyspc ? spcoffset : -1);
		col = 0;
		first = ElvTrue;
		return TWRAP_BEFORE;
	}

	/* It will fit.  If we need to adjust our indent, do it now */
	if (col < indent)
	{
		(*drawchar)(space, col - indent, 0, anyspc ? spcoffset : -1);
		col = indent;
		anyspc = ElvFalse;
	}

	/* Output a space between tokens, usually. */
	if (anyspc)
	{
		(*drawchar)(space, 1, spcfont, spcoffset);
		col++;
		anyspc = ElvFalse;
	}

	/* Output mantext[] in as few chunks as possible */
	for (start = 0; start < manlen; start = i)
	{
		for (i = start + 1;
		     i < manlen
			&& manfont[i] == manfont[start]
			&& manoffset[i] == manoffset[i - 1] + 1;
		     i++)
		{
		}
		(*drawchar)(&mantext[start], (long)(i - start), manfont[start], manoffset[start]);
	}
	col += manlen;

	return TWRAP_NO;
}
#endif /* defined(DISPLAY_HTML) || defined(DISPLAY_MAN) */

#ifdef DISPLAY_MAN
/*----------------------------------------------------------------------------*/
/* "man" mode functions                                                       */

/* This function interprets codes in a text token, converting them to regular
 * text.  Note that none of the codes can change the font.
 */
static void manescape(token)
	TOKEN	*token;
{
	int	i, j, n;
	TOKEN	temp;
	CHAR	*arg;
	static struct {
		CHAR	name1, name2, c1, c2;
	} special[] = {
		{ 'b', 'r',	'|' },		/* should be graphic */
		{ 'b', 'u',	'*' },		/* should be graphic */
		{ 'c', 'o',	'O', 'c' },
		{ 'd', 'g',	'~', '!' },
		{ 'e', 'm',	'-' },		/* should be wide */
		{ 'h', 'y',	'-' },
		{ 'l', 'q',	'"' },
		{ 'r', 'n',	'-' },		/* should be graphic */
		{ 'r', 'q',	'"' },
		{ 'u', 'l',	'_' },
		{0}
	};

	/* if the cursor is in this somewhere, then don't change it but we
	 * still need to compute the width so change a temporary copy.
	 */
	if (o_showmarkups
	 && cursoff >= token->offset[0]
	 && cursoff <= token->offset[token->nchars - 1])
	{
		temp = *token;
		temp.offset[0] = cursoff + 1;
		manescape(&temp);
		token->width = temp.width;
		return;
	}

	/* for each character... */
	for (i = j = 0; i < token->nchars; i++)
	{
		/* most characters are copied as-is */
		if (token->text[i] != '\\')
		{
			token->text[j] = token->text[i];
			token->offset[j++] = token->offset[i];
		}
		else if (i + 1 < token->nchars)
		{
			switch (token->text[++i])
			{
			  case '|':
			  case '&':
			  case '^':
			  case 'c':
				/* delete the \|, \&, \^, or \c */
				break;

			  case '(':
				/* some special characters */
				arg = &token->text[i + 1];
				for (n = 0; special[n].name1 && (arg[0] != special[n].name1 || arg[1] != special[n].name2); n++)
				{
				}
				if (special[n].name1)
				{
					if (special[n].c2)
						token->text[j] = digraph(special[n].c1, special[n].c2);
					else
						token->text[j] = special[n].c1;
					token->offset[j] = token->offset[i - 1];
					j++;
				}
				i += 2;
				break;

			  case 's':
				/* delete the \s and the number that follows */
				if (token->text[i + 1] == '\'')
				{
					/* in single-quotes, skip to next */
					for (i += 2; token->text[i] && token->text[i] != '\''; i++)
					{
					}
				}
				else
				{
					/* unquoted, so skip sign & digits */
					if (token->text[i + 1] == '+' || token->text[i + 1] == '-')
						i++;
					do
					{
						i++;
					} while (elvdigit(token->text[i]));
					i--;
				}
				break;

			  case '*':
				/* man predefines a few special strings */
				if (token->text[i + 1] == 'R')
				{
					token->text[j] = digraph('O', 'r');
					token->offset[j] = token->offset[i - 1];
					j++;
					i++;
					break;
				}
				else if (token->text[i + 1] == 'S')
				{
					/* hide it */
					i++;
					break;
				}
				else if (!CHARncmp(token->text + i, toCHAR("*(Tm"), 4))
				{
					token->text[j] = digraph('M', 'T');
					token->offset[j] = token->offset[i - 1];
					if (token->text[j] == 'T')
					{
						token->text[++j] = 'M';
						token->offset[j] = token->offset[i];
					}
					j++;
					i += 3;
					break;
				}
				else if (!CHARncmp(token->text + i, toCHAR("*(lq"), 4)
				      || !CHARncmp(token->text + i, toCHAR("*(rq"), 4))
				{
					token->text[j] = '"';
					token->offset[j] = token->offset[i - 1];
					j++;
					i += 3;
					break;
				}
				/* else fall through... */

			  case 'n':
				/* keep the \* or \n as-is */
				token->text[j] = '\\';
				token->offset[j++] = token->offset[i - 1];
				token->text[j] = token->text[i];
				token->offset[j++] = token->offset[i];
				break;

			  case 'e':
				/* convert \e to backslash */
				token->text[j] = '\\';
				token->offset[j++] = token->offset[i];
				break;

			  case '0':
				/* convert \0 to space */
				token->text[j] = ' ';
				token->offset[j++] = token->offset[i];
				break;

			  default:
				/* convert \X to just plain X */
				token->text[j] = token->text[i];
				token->offset[j++] = token->offset[i];
			}
		}
	}

	/* mark the end of the string */
	token ->width = token->nchars = j;
	token->text[j] = '\0';
}

/* This function is used to collect the arguments to a .XX macro together
 * as a string with a parallel array storing the font.
 */
static int manarg(token, start, font, spc)
	TOKEN	*token;	/* the token to parse */
	int	start;	/* where to begin scanning */
	_char_	font;	/* initial font of arg */
	ELVBOOL	spc;	/* insert a space before the word? */
{
	ELVBOOL	quote;	/* is this arg enclosed in quotes? */

	/* skip leading whitespace */
	while (elvspace(token->text[start]))
	{
		start++;
	}

	/* is this arg quoted? */
	quote = (ELVBOOL)(token->text[start] == '"');
	if (quote)
		start++;

	/* are we supposed to insert a space? */
	if (spc && start < token->nchars - 2)
	{
		assert(start > 0);
		if (manlen < QTY(mantext))
		{
			mantext[manlen] = ' ';
			manoffset[manlen] = token->offset[start - 1];
			manfont[manlen] = fontcode['b'];
			manlen++;
		}
	}

	/* collect text to end of arg, or until mantext[] is full */
	while (manlen < QTY(mantext) - 1
		&& start < token->nchars - 2
		&& (quote ? token->text[start] != '"' : !elvspace(token->text[start])))
	{
		/* handle \fX */
		if (token->text[start] == '\\' && token->text[start + 1] == 'f')
		{
			switch (token->text[start + 2])
			{
			  case '1':
			  case 'B':	font = fontcode['b'];	break;
			  case '2':
			  case 'I':	font = fontcode['i'];	break;
			  default:	font = 0;		break;
			}
			start += 3;
		}
		else if (token->text[start] == '\\' && start < token->nchars - 2)
		{
			switch (token->text[++start])
			{
			  case '|':
			  case '&':
			  case '^':
			  case 'c':
				break;

			  case 'e':
				if (manlen < QTY(mantext))
				{
					mantext[manlen] = '\\';
					manoffset[manlen] =token->offset[start];
					manfont[manlen] = font;
					manlen++;
				}
				break;

			  case 's':
				/* delete the \s and the number that follows */
				start++;
				if (token->text[start] == '+' || token->text[start] == '-')
					start++;
				do
				{
					start++;
				} while (elvdigit(token->text[start]));
				start--;
				break;

			  case '(':
				/* handle the \(bu special character only! */
				if (token->text[start + 1] == 'b' || token->text[start + 2] == 'u')
				{
					mantext[manlen] = '*';
					manoffset[manlen] =token->offset[start];
					manfont[manlen] = fontcode['g'];
					manlen++;
					start += 2;
					break;
				}
				/* else fall through... */

			  default:
				if (manlen < QTY(mantext))
				{
					mantext[manlen] = token->text[start];
					manoffset[manlen] =token->offset[start];
					manfont[manlen] = font;
					manlen++;
				}
			}
			start++;
		}
		else if (manlen < QTY(mantext))
		{
			if (token->text[start] <= ' ')
				mantext[manlen] = ' ';
			else
				mantext[manlen] = token->text[start];
			manoffset[manlen] = token->offset[start];
			manfont[manlen] = font;
			manlen++;
			start++;
		}
	}

	/* skip the closing quote (if quoted) */
	if (quote && token->text[start] == '"')
		start++;

	/* return the index of the end of the arg */
	return start;
}


/* This function implements the .TH macro, which declares the page's title */
static twrap_t manTH(token)
	TOKEN	*token;
{
	int	i;

	/* combine the first & second args as the document name */
	manlen = 0;
	i = manarg(token, 3, 0, ElvFalse);
	mantext[manlen++] = '(';
	(void)manarg(token, i, 0, ElvFalse);
	mantext[manlen++] = ')';
	mantext[manlen] = '\0';

	/* If the title is different, then store it now */
	if (!mui->title || CHARcmp(mui->title, mantext))
	{
		if (mui->title)
			safefree(mui->title);
		mui->title = CHARdup(mantext);
	}

	return TWRAP_NO;
}

/* This function implements the .SH and .SS macros */
static twrap_t manSH(token)
	TOKEN	*token;
{
	long	nloff;	/* offset of newline */
	int	i;

	/* Get the arguments, as the section title */
	manlen = 0;
	for (i = 4; i < token->nchars; i++)
	{
		i = manarg(token, i, fontcode['b'], (ELVBOOL)(i != 4));
	}

	/* If the cursor is located in this token, or we're doing some sort
	 * of move operation, then pretend that the offsets of all the
	 * generated characters are -1.
	 */
	nloff = token->offset[token->nchars - 1];
	if ((o_showmarkups && token->offset[0] <= cursoff && cursoff <= token->offset[token->nchars - 1])
		|| list || drawchar == countchar)
	{
		for (i = 0; i < manlen; i++)
			manoffset[i] = -1;
		nloff = -1;
	}

	/* output the title, followed by a newline */
	anyspc = ElvFalse;
	manput();
	(*drawchar)(newline, 1, 0, nloff);
	col = 0;
	first = ElvTrue;
	anyspc = ElvFalse;
	reduce = ElvTrue;

	/* force the indentation to be the default */
	indent = listind * 2;

	/* Force the line to end after this title; this affects movement,
	 * not the appearance of the title since we already output a newline
	 * here.
	 */
	return TWRAP_AFTER;
}

/* This function implements the font-changing macros such as .BI, .RB, and
 * so on.  The resulting text is treated as a single word; if it can't fit
 * on the current line, then this function merely outputs a newline.
 */
static twrap_t manBI(token)
	TOKEN	*token;
{
	char	font1, font2;
	int	start, i;

	/* choose the fonts & start, based on macro name */
	switch (token->text[1])
	{
	  case 'B':	font1 = 'b';	break;
	  case 'I':	font1 = 'i';	break;
	  case 'Z':	font1 = 'f';	break;
	  default:	font1 = 'n';
	}
	switch (token->text[2])
	{
	  case 'B':	font2 = 'b';	start = 4;	break;
	  case 'I':	font2 = 'i';	start = 4;	break;
	  case 'S':
	  case 'N':
	  case 'R':	font2 = 'n';	start = 4;	break;
	  default:	font2 = font1;	start = 3;
	}
	font1 = fontcode[(int)font1];
	font2 = fontcode[(int)font2];

	/* collect the args, with their fonts */
	manlen = 0;
	start = manarg(token, start, font1, ElvFalse);
	for (i = 2; i < (readonly ? 12 : 6); i++)
	{
		start = manarg(token, start, (i & 1) ? font1 : font2, (ELVBOOL)(font1 == font2));
	}

	/* If the cursor is on this token, then just tweak its width and
	 * return ElvFalse so the remaining token-putting code will handle
	 * line-wrap.  Else perform the token's output.
	 */
	if (list || (o_showmarkups && token->offset[0] <= cursoff && cursoff <= token->offset[token->nchars - 1]))
	{
		token->width = manlen + 1; /* "+ 1" to allow for whitespace */
		return TWRAP_NO;
	}
	else if (manput() != TWRAP_NO)
	{
		return TWRAP_BEFORE;
	}

	/* assume there should be some whitespace after this */
	anyspc = ElvTrue;
	spcoffset = token->offset[token->nchars - 1];
	spcfont = curfont;
	return TWRAP_NO;
}

/* This function implements the .IP macro.  It outputs its first argument,
 * and then increases the indentation.  If necessary, it will then output
 * a newline character, but this newline character isn't considered to be
 * the end of "line" as far as moving the cursor is concerned; it is just
 * a funny type of wrapping.
 */
static twrap_t manIP(token)
	TOKEN	*token;
{
	int	i, start;
	char	font, font2;
	int	moreindent;
	double	f;

	/* if this token is going to be displayed anyway, then don't bother
	 * performing its output.
	 */
	if (list || (o_showmarkups && token->offset[0] <= cursoff && cursoff <= token->offset[token->nchars - 1]))
		return TWRAP_NO;

	/* Choose the default indentation.  .IP normally increases indentation
	 * by one full tab stop, and .TP normally does it by two full tabstops.
	 */
	if (token->text[1] == 'T')
		moreindent = 4 * listind;
	else
		moreindent = 2 * listind;

	/* get the first arg, as the paragraph tag */
	manlen = 0;
	start = manarg(token, 4, fontcode['n'], ElvFalse);

	/* For .TP (but not .IP) get other args as part of the tag, too */
	if (token->text[1] == 'T')
	{
		/* if first word was .B or .I, then change font */
		if (manlen == 2 && mantext[0] == '.' && (mantext[1] == 'B' || mantext[1] == 'I'))
		{
			font = font2 = fontcode[mantext[1]];
			manlen = 0;
		}
		else if (manlen == 3 && mantext[0] == '.')
		{
			font = fontcode[mantext[1]];
			if (font != fontcode['b'] && font != fontcode['i'])
				font = fontcode['n'];
			font2 = fontcode[mantext[2]];
			if (font2 != fontcode['b'] && font2 != fontcode['i'])
				font2 = fontcode['n'];
			manlen = 0;
		}
		else
		{
			font = font2 = fontcode['n'];
		}

		/* Copy the remaining args, in the desired font[s].  If the
		 * fonts are different, then delete whitespace from between
		 * arguments.
		 */
		for (i = 0; i < 5; i++)
		{
			start = manarg(token, start, (i & 1) ? font2 : font, (ELVBOOL)(font == font2 && manlen > 0));
		}
	}
	else
	{
		/* For .IP, second argument is indentation amount */

		/* skip intervening whitespace */
		while (elvspace(token->text[start]))
			start++;

		/* Convert indentation to integer.  Assume the unit of
		 * measurement is "n".
		 */
		if (elvdigit(token->text[start]))
		{
			f = atof(tochar8(&token->text[start]));
			while (elvdigit(token->text[start]) || token->text[start] == '.')
				start++;
			switch (token->text[start])
			{
			  case 'n':	i = (int)f;		break;
			  case 'i':	i = (int)(f * 10);	break;
			  case 'p':	i = (int)(f / 7.2);	break;
			  default:	i = 0;
			}
			if (i > 0 && indent + i < 40)
				moreindent = i;
		}
	}

	/* output the paragraph tag */
	(void)manput();

	/* add the indentation */
	indent += moreindent;

	/* If necessary, output a newline as part of the text */
	if (col >= indent)
	{
		(*drawchar)(newline, 1, 0, token->offset[token->nchars - 1]);
		col = 0;
		return TWRAP_AFTER;
	}
	return TWRAP_NO;
}

/* Look up a man markup token in a table */
static void manmarkup(token)
	TOKEN	*token;	/* the token to lookup */
{
	static MARKUP	tbl[] =
	{

		/* Tag		 Effects	Function	*/
		/*               TBILFFD                        */
		{ "\\\"",	"Y------"			},
		{ "TH",		"Y----Y-",	manTH		},
		{ "SH",		"Nc0-NYS",	manSH		},
		{ "SS",		"N11-NYS",	manSH		},
		{ "B",		"N------",	manBI		},
		{ "I",		"N------",	manBI		},
		{ "SM",		"N------",	manBI		},
		{ "BI",		"N------",	manBI		},
		{ "IB",		"N------",	manBI		},
		{ "RI",		"N------",	manBI		},
		{ "IR",		"N------",	manBI		},
		{ "BR",		"N------",	manBI		},
		{ "RB",		"N------",	manBI		},
		{ "BS",		"N------",	manBI		},
		{ "SB",		"N------",	manBI		},
		{ "ZN",		"N------",	manBI		},
		{ "IP",		"N12-NYP",	manIP		},
		{ "TP",		"N12-NYP",	manIP		},
		{ "PP",		"N12-NYP"			},
		{ "P",		"N12-NYP"			},
		{ "LP",		"N12-NYP"			},
		{ "HP",		"N12-NYP"			},
		{ "RS",		"N->----"			},
		{ "RE",		"N-<----"			},
		{ "br",		"N0-----"			},
		{ "sp",		"N1-----"			},
		{ "nf",		"N0--FN-"			},
		{ "fi",		"N---NY-"			},
		{ "DS",		"N1---N-"			},
		{ "DE",		"N0---Y-"			},
		{ "TS",		"N0---N-"			},
		{ "TE",		"N0---Y-"			},
		{ (char *)0,	"N------"			}
	};
	MARKUP	*scan;	/* used for scanning the tbl[] array */

	/* look it up in the table */
	for (scan = tbl;
	     scan->name &&
		(strncmp(scan->name, tochar8(token->text+1), strlen(scan->name))
		|| elvalnum(token->text[strlen(scan->name) + 1]));
	     scan++)
	{
	}

	/* remember it */
	token->markup = scan;
}


/* Get the next token from a "man" document, or return NULL at end. */
static TOKEN *manget(refp)
	CHAR	**refp;
{
	MARK	back;	/* address of a backslash */
	long	offset;	/* offset of character that *refp points to */
	long	start;	/* offset of start of token */
 static	MARKUP	fontchg;/* describes font change markups */
 	TOKEN	tmp, *next;
 	ELVBOOL	quote;

	/* Initialize "back" just to silence a compiler warning */
	back = NULL;

	/* if the CHAR pointer is NULL, then return NULL */
	if (!*refp)
	{
#ifdef DEBUG_MARKUP
		fprintf(stderr, "manget() returning NULL\n");
#endif
		return NULL;
	}

	/* Get first character of token */
	start = offset = markoffset(scanmark(refp));
	rettok.text[0] = **refp;
	rettok.offset[0] = offset++;
	rettok.nchars = 1;
	rettok.width = 0;
	rettok.markup = NULL;
	scannext(refp);

	/* If not in "no fill" mode, Two or more consecutive newlines act
	 * like a ".sp" markup.
	 */
	if (!prefmt && rettok.text[0] == '\n' && *refp && **refp == '\n')
	{
		/* skip the extra newlines */
		/*offset++;*/
		while (scannext(refp) && **refp == '\n')
		{
			offset++;
		}

		/* Peek at the next markup.  If it would cause a line break,
		 * then return it instead of the blank lines.
		 */
		if (*refp && **refp == '.')
		{
			midline = ElvFalse;
			tmp = rettok;
			back = scanmark(refp);
			next = manget(refp);
			if (next && next->markup && next->markup->BREAKLN != '-')
			{
				next->offset[0] = start;
#ifdef DEBUG_MARKUP
				fprintf(stderr, "manget() returning newline-tweaked \"%s\", offset=%ld, BREAKLN\n", next->text[0] == '\n' ? "\\n" : (char *)next->text, next->offset[0]);
#endif
				return next;
			}
			scanseek(refp, back);
			rettok = tmp;
		}

		/* build a ".sp" command */
		rettok.text[0] = '.';
		rettok.offset[0] = start;
		rettok.text[1] = 's';
		rettok.offset[1] = offset - 1;
		rettok.text[2] = 'p';
		rettok.offset[2] = offset - 1;
		rettok.text[3] = '^';
		rettok.offset[3] = offset - 1;
		rettok.text[4] = 'J';
		rettok.offset[4] = offset - 1;
		rettok.text[5] = '\0';
		rettok.nchars = 5;
		manmarkup(&rettok);
		midline = ElvFalse;
#ifdef DEBUG_MARKUP
		fprintf(stderr, "manget() returning constructed \"%s\", offset=%ld\n", (char *)rettok.text, rettok.offset[0]);
#endif
		return &rettok;
	}

	/* If '.' at the start of a line, then token is a markup */
	if ((rettok.text[0] == '.' || rettok.text[0] == '\'') && !midline)
	{
		/* This is a markup.  Collect characters up to next '\n' */
		for (;
		     *refp && rettok.nchars < QTY(rettok.text) - 3 && **refp != '\n';
		     offset++, scannext(refp))
		{
			rettok.text[rettok.nchars] = **refp;
			rettok.offset[rettok.nchars++] = offset;
		}

		/* If this is ".TP" or ".TS" then read the following line
		 * as part of this token.
		 */
		if ((!CHARncmp(rettok.text, toCHAR(".TP"), 3)
			|| !CHARncmp(rettok.text, toCHAR(".TS"), 3))
		 && *refp
		 && **refp == '\n'
		 && scannext(refp))
		{
			/* the newline after ".TP" becomes a space */
			rettok.nchars = 3;
			rettok.text[rettok.nchars] = ' ';
			rettok.offset[rettok.nchars] = offset;
			rettok.nchars++;
			offset++;

			for (;
			     *refp && rettok.nchars < QTY(rettok.text) - 3 && **refp != '\n';
			     offset++, scannext(refp))
			{
				rettok.text[rettok.nchars] = **refp;
				rettok.offset[rettok.nchars++] = offset;
			}
		}

		/* store the terminating '\n' as "^J" */
		rettok.text[rettok.nchars] = '^';
		rettok.offset[rettok.nchars] = offset;
		rettok.nchars++;
		rettok.text[rettok.nchars] = 'J';
		rettok.offset[rettok.nchars] = offset;
		rettok.nchars++;
		rettok.text[rettok.nchars] = '\0';
		if (*refp)
			scannext(refp);

		/* lookup the markup */
		manmarkup(&rettok);

		/* remember that we stopped after a newline */
		midline = ElvFalse;
	}
	else if (*refp && rettok.text[0] == '\\' && **refp == 'f')
	{
		/* This is a font-change macro.  Get the final character */
		rettok.text[1] = 'f';
		rettok.offset[1] = offset++;
		scannext(refp);
		rettok.text[2] = (*refp ? **refp : 'R');
		rettok.offset[2] = offset++;
		rettok.nchars = 3;
		if (*refp)
			scannext(refp);

		/* construct a simple MARKUP struct */
		fontchg.name = tochar8(rettok.text);
		strcpy(fontchg.attr, "----n--");
		switch (rettok.text[2])
		{
		  case '1':
		  case 'B':	fontchg.FONT = 'b';	break;
		  case '2':
		  case 'I':	fontchg.FONT = 'i';	break;
		  case 'P':	fontchg.FONT = '=';	break;
		  default:	fontchg.FONT = 'n';	break;
		}
		rettok.markup = &fontchg;

		/* no newline at the end of this token! */
		midline = ElvTrue;
	}
	else if (rettok.text[0] <= ' ')
	{
		/* This is a whitespace token.  Each whitespace token contains
		 * a SINGLE whitespace character.  Control characters other
		 * than '\t' and '\n' are treated as spaces.
		 */
		if (rettok.text[0] != '\t' && rettok.text[0] != '\n')
			rettok.text[0] = ' ';

		/* remember if this is a newline or not */
		midline = (ELVBOOL)(rettok.text[0] != '\n');

		/* assume this whitespace will show, for computing line break */
		rettok.width = 1;
	}
	else
	{
		/* This is a word.  Collect chars up to next whitespace or
		 * "\fX" string.
		 */
		for (quote = (ELVBOOL)(rettok.text[0] == '\\');
		     *refp 
			&& rettok.nchars < QTY(rettok.text) - 1
			&& !elvspace(**refp);
		     offset++, scannext(refp))
		{
			if (quote)
				quote = ElvFalse;
			else if (**refp == '\\')
			{
				if (!scannext(refp))
					break;
				if (**refp == 'f')
				{
					scanprev(refp);
					break;
				}
				rettok.text[rettok.nchars] = '\\';
				rettok.offset[rettok.nchars] = offset;
				rettok.nchars++;
				offset++;
				quote = ElvTrue;
			}
			rettok.text[rettok.nchars] = **refp;
			rettok.offset[rettok.nchars] = offset;
			rettok.nchars++;
		}

		/* this didn't end with a newline */
		midline = ElvTrue;

		/* For now, assume all characters of this work will be visible.
		 * When escapes are processed, this may change.
		 */
		rettok.width = rettok.nchars;
	}

	/* Mark the end of the text, and return the token */
	rettok.text[rettok.nchars] = '\0';
#ifdef DEBUG_MARKUP
		fprintf(stderr, "manget() returning \"%s\", offset=%ld\n", rettok.text[0] == '\n' ? "\\n" : (char *)rettok.text, rettok.offset[0]);
#endif
	return &rettok;
}

static DMINFO *maninit(win)
	WINDOW	win;
{
	TOKEN	*token;
	MARKBUF	top;
	CHAR	*p;
	long	cursoffset;
	twrap_t	result;

	/* inherit some functions from dmnormal */
	dmman.wordmove = dmnormal.wordmove;
	dmman.tagatcursor = dmnormal.tagatcursor;
	dmman.tagload = dmnormal.tagload;
	dmman.tagnext = dmnormal.tagnext;

	/* initialize the fonts */
	initfonts();

	/* if merely initializing the options, we're done */
	if (!win)
		return NULL;

	/* allocate the info struct */
	mui = (MUINFO *)safealloc(1, sizeof(MUINFO));
	mui->get = manget;
	mui->escape = manescape;
	mui->line = (LINEINFO *)safealloc(GRANULARITY, sizeof(LINEINFO));
	mui->nlines = 1; /* every buffer has at least one line */

	/* move the cursor someplace harmless, so it won't affect formatting */
	cursoffset = markoffset(win->cursor);
	marksetoffset(win->cursor, o_bufchars(markbuffer(win->cursor)));

	/* format the buffer */
#ifdef DEBUG_MARKUP
	fprintf(stderr, "maninit() starting\n");
#endif
	win->mi = (DMINFO *)mui;
	(void)start(win, marktmp(top, markbuffer(win->cursor), 0), NULL);
	p = scanalloc(&p, &top);
	result = TWRAP_NO;
	for (token = (*mui->get)(&p);
	     token;
	     token = (*mui->get)(&p))
	{
		/* if previous token forced a newline after it, then this token
		 * is the start of the next line.
		 */
		if (result == TWRAP_AFTER)
		{
#ifdef DEBUG_MARKUP
			fprintf(stderr, "maninit() storing line %ld, offset=%ld, result=TWRAP_AFTER\n", mui->nlines + 1, token->offset[0]);
#endif
			assert(first == ElvTrue && col == 0);
			storestate(token->offset[0], NULL);
		}

		/* if cursor is at position 0, and this is a text token,
		 * then move the cursor here.  This is done because otherwise
		 * the cursor would always start on an ugly formatting code.
		 */
		if (cursoffset == 0L 
		 && !title
		 && !elvspace(token->text[0])
		 && !token->markup)
		{
			cursoffset = token->offset[0];
		}

		/* output the token.  If it forces a new line, remember where
		 * that new line started.
		 */
		result = put(token);
		if (result == TWRAP_BEFORE)
		{
#ifdef DEBUG_MARKUP
			fprintf(stderr, "maninit() storing line %ld, offset=%ld, result=TWRAP_BEFORE\n", mui->nlines + 1, token->offset[0]);
#endif
			assert(first == ElvTrue && col == 0);
			storestate(token->offset[0], NULL);
			result = put(token);
#ifdef DEBUG_MARKUP
			fprintf(stderr, "maninit() after 2nd put(), result=TWRAP_%s\n", result==TWRAP_BEFORE ? "BEFORE" : "AFTER");
#endif
		}
	}
	scanfree(&p);
#ifdef DEBUG_MARKUP
	fprintf(stderr, "maninit() finished\n");
#endif

	/* restore the cursor position */
	marksetoffset(win->cursor, cursoffset);

	/* Done! */
	return (DMINFO *)mui;
}
#endif /* defined(DISPLAY_MAN) */

#ifdef DISPLAY_TEX
/*----------------------------------------------------------------------------*/
/* TeX functions */

static void texescape(tok)
	TOKEN	*tok;
{
	/* TeX doesn't need escapes */
}

/* This implements a '{' -- it starts a scope by saving the current font as
 * the new default font, so '}' can switch back.
 */
static twrap_t texscope(token)
	TOKEN	*token;
{
	deffont = curfont;
	return TWRAP_NO;
}

/* This outputs characters embedded in its name */
static twrap_t texoutput(token)
	TOKEN	*token;
{
	int	i;
	char	font;

	/* if the characters don't fit on the line, then fail */
	if (token->width + col > textwidth - 4 && col > indent)
	{
		(*drawchar)(newline, 1, 0, anyspc ? spcoffset : -1);
		col = 0;
		first = ElvTrue;
		return TWRAP_BEFORE;
	}

	/* find the text */
	if (token->width == 1)
	{
		i = token->nchars - token->width;
		font = curfont;
	}
	else
	{
		i = token->nchars - token->width;
		font = fontcode[(curfont == fontcode['f']) ? 'f' : 'b'];
	}

	/* output the indentation, if necessary */
	if (col < indent)
	{
		(*drawchar)(space, (long)(col - indent), 0, -1);
		col = indent;
	}
	else if (anyspc)
	{
		(*drawchar)(space, -1, spcfont, spcoffset);
		col++;
		anyspc = ElvFalse;
	}


	/* output the text in the current font, or bold font */
	(*drawchar)(&token->text[i], token->width, font, token->offset[i]);
	col += token->width;
	return TWRAP_NO;
}

/* Output the label for a list item */
static twrap_t texitem(token)
	TOKEN	*token;
{
	CHAR	buf[10];
	CHAR	*label;
	int	len;
	long	offset;

	assert(col == 0);

	/* choose a label (and len and offset) */
	if (token->text[5] == '[')
	{
		/* label is given in \item[label] notation */
		label = &token->text[6];
		len = token->nchars - 7;
		offset = token->offset[6];

		/* Watch out for items wrapped in $'s */
		if (label[0] == '$' && label[len - 1] == '$')
		{
			label++;
			len -= 2;
			offset++;
		}
	}
	else if (nest == 1 && listcnt > 0)
	{
		/* enumerating -- convert item# to characters */
		long2CHAR(buf, (long)listcnt++);
		CHARcat(buf, toCHAR(")"));
		label = buf;
		len = CHARlen(buf);
		offset = token->offset[0];
	}
	else
	{
		CHARcpy(buf, toCHAR("*"));
		label = buf;
		len = 1;
		offset = token->offset[0];
	}

	/* if nest==0 then we must be doing \begin{description}, in which case
	 * the label should be left-justified.  Others are right-justified.
	 */
	if (nest == 0 && token->text[1] == 'b')
	{
		/* indent only to normal left margin */
		(*drawchar)(space,  -2 * listind, 0, -1);

		/* draw the label in bold font */
		(*drawchar)(label, len, fontcode['b'], offset);

		/* if label is too wide, then start text on next line */
		col = 2 * listind + len;
		if (col >= indent)
		{
			(*drawchar)(newline, 1, 0, (anyspc ? spcoffset : -1));
			col = 0;
		}
	}
	else
	{
		/* output whitespace for indentation */
		if (indent > len)
		{
			(*drawchar)(space, 1 + len - indent, 0, -1);
		}

		/* output the label in bold font */
		(*drawchar)(label, len, fontcode['b'], offset);
		col = indent - 1;
	}

	return TWRAP_NO;
}


/* This outputs a title, author, section, or subsection string */
static twrap_t textitle(token)
	TOKEN	*token;
{
	char	font;
	long	offset;
	long	indent;
	ELVBOOL	center;
	int	i, j;

	/* initialize "indent" to avoid a bogus compiler warning */
	indent = 0;

	/* the font and indentation depend on the keyword */
	switch (token->text[4]) /* <- Tricky! */
	{
	  case 'l':	/* \titLe{} */
		center = ElvTrue;
		font = 'b';
		break;

	  case 'h':	/* \autHor{} */
		center = ElvTrue;
		font = 'i';
		break;

	  case 'p':	/* \chaPter{} */
		center = ElvTrue;
		font = 'b';
		break;

	  case 't':	/* \secTion{} or \parT{} */
		center = (ELVBOOL)(token->text[1] == 'p'); /* part */
		indent = 0;
		font = 'b';
		break;

	  default:	/* case 's': \subSection{} */
		center = ElvFalse;
		indent = 4;
		font = 'b';
		break;
	}
	font = fontcode[(int)font];

	/* locate the label text (and its len) */
	for (i = 1; token->text[i - 1] != '{' && i < token->nchars; i++)
	{
	}
	if (i >= token->nchars)
		return TWRAP_NO;
	offset = token->offset[i];
	for (j = 0; i < token->nchars; i++)
	{
		if (token->text[i] == '\\' && i + 1 < token->nchars - 1)
			i++;
		if (j > 0 && token->text[i] == '\n')
			mantext[j++] = ' ';
		else if (token->text[i] != '{'
		      && token->text[i] != '}'
		      && token->text[i] != '$'
		      && token->text[i] != '\\'
		      && (j > 0 || !elvspace(token->text[i])))
			mantext[j++] = token->text[i];

		/* if forced line break, then do it now */
		if (j > 72
		 || (j > 65 && token->text[i] == ' ')
		 || token->text[i] == '}'
		 || (token->text[i] == '\\' && token->text[i - 1] == '\\')
		 || i == token->nchars - 1)
		{
			/* output the indentation and label */
			if (center)
				indent = (textwidth - j) / 2;
			(*drawchar)(space, -indent, 0, -1L);
			(*drawchar)(mantext, (long)j, font, offset);

			/* output a newline */
			anyspc = ElvFalse;
			(*drawchar)(newline, 1, 0, -1L);
			col = 0;

			/* prepare for next line of title */
			j = 0; 
			offset = token->offset[i];
		}
	}

	/* If the title is different, then store it now.  (This assumes the
	 * title fits on a single line */
	if (token->text[4] == 'l' && (!mui->title || CHARcmp(mui->title, mantext)))
	{
		if (mui->title)
			safefree(mui->title);
		mui->title = (CHAR *)safealloc(j + 1, sizeof(CHAR));
		CHARncpy(mui->title, mantext, (int)j);
	}

	/* remember that the last the we output was a newline */
	col = 0;
	first = ElvTrue;
	anyspc = ElvFalse;
	reduce = ElvTrue;

	return TWRAP_AFTER;
}


/* output a single digraph character */
static twrap_t texdigraph(token)
	TOKEN	*token;
{
	CHAR	dig[1];

	/* convert 2nd & 4th chars into a digraph char, and output it */
	*dig = digraph(token->text[1], token->text[3]);
	(*drawchar)(dig, 1, curfont, token->offset[0]);
	return TWRAP_NO;
}


/* This is a helper function for texget() */
static long texpair(refp, token)
	CHAR	**refp;	/* pointer to parse from */
	TOKEN	*token;	/* the token we're generating */
{
	long	offset = markoffset(scanmark(refp));
	int	nest;
	CHAR	first, match;

	/* is the next char matchable */
	first = **refp;
	switch (first)
	{
	  case '[':	match = ']';	break;
	  case '{':	match = '}';	break;
	  default:	return offset;
	}

	/* add chars up to matching char */
	nest = 0;
	do
	{
		token->text[token->nchars] = (elvcntrl(**refp) ? ' ' : **refp);
		token->offset[token->nchars] = offset++;
		if (token->nchars < QTY(token->text) - 2)
			token->nchars++;
		if (**refp == first)
			nest++;
		else if (**refp == match)
			nest--;
	} while (scannext(refp) && nest > 0);

	/* done */
	return offset;
}

/* Get the next token from a TeX document, or return NULL at end.  This is
 * very complex, because the function also tries to warp the way TeX is
 * tokenized, to make its syntax resemble HTML or nroff since that's what
 * the dmmarkup.c file was originally designed around.
 */
static TOKEN *texget(refp)
	CHAR	**refp;
{
	long	offset;	/* offset of character that *refp points to */
	int	i;
 static MARKUP	markups[] = {
	/*		      TITLE	 in title: -, N, Y */
	/*		      |BREAKLN	 line break: -, 0, 1, 2, c, or p */
	/*		      ||INDENT	 -, <, >, =, or a number */
	/*		      |||LIST	 in list: -, N, Y, # */
	/*		      ||||FONT	 font: -,=,n,b,u,i,f,e,N,B,U,I,F,E */
	/*		      |||||FILL	 Y=fill, N=preformatted, -=no chg. */
	/*		      ||||||DEST S=section, P=paragraph, T=<tab> key */
	/* NAME		     "TBILFFD"	FUNCTION */
#define TEX_OPENCURLY &markups[0]
    	{ "{",		     "-------",	texscope	},
#define TEX_CLOSECURLY &markups[1]
    	{ "}",		     "----<--"			},
#define TEX_1DOLLAR &markups[2]
	{ "$",		     "----~--"			},
#define TEX_2DOLLARS &markups[3]
	{ "$$",		     "-1--~--"			},
#define TEX_IGNORE &markups[4]
    	{ "ignore",	     "-------"			},
#define TEX_PARAGRAPH &markups[5]
    	{ "p",		     "-12N-YP"			},
#define TEX_OUTPUT &markups[6]
	{ "mathrel",	     "-------",	texoutput	},
#define TEX_ITEM &markups[7]
	{ "item",	     "-0-----",	texitem		},
#define TEX_CR &markups[8]
	{ "cr",		     "-02----"			},
#define TEX_TITLE &markups[9]
    	{ "title",	     "-22-NY-",	textitle	},
#define TEX_AUTHOR &markups[10]
    	{ "author",	     "-12-NY-",	textitle	},
#define TEX_PART &markups[11]
    	{ "part",	     "-c2NNYS",	textitle	},
#define TEX_CHAPTER &markups[12]
    	{ "chapter",	     "-c2NNYS",	textitle	},
#define TEX_SECTION &markups[13]
    	{ "section",	     "-c2NNYS",	textitle	},
#define TEX_SUBSECTION &markups[14]
    	{ "subsection",	     "-12NNYS",	textitle	},
#define TEX_DIGRAPH &markups[15]
	{ "digraph",	     "-------", texdigraph	},
#define TEX_HFIL &markups[16]
	{ "hfil",	     "--=----"			},
	{ "hfill",	     "--=----"			},
	{ "hline",	     "-02--Y-",	htmlhr		}, /* reuse HTML! */
	{ "begin{table}",    "-02-NY-"			},
	{ "end{table}",	     "-02-NY-"			},
	{ "begin{eqnarray}", "-02-fY-"			},
	{ "end{eqnarray}",   "-02-NY-"			},
	{ "begin{quote}",    "-14-NYP"			},
	{ "end{quote}",	     "-12-NY-"			},
	{ "begin{verbatim}", "-0--FNP"			},
	{ "end{verbatim}",   "-0--NY-"			},
	{ "begin{code}",     "-0--FNP"			},
	{ "end{code}",       "-0--NY-"			},
	{"begin{description}","-04-NYS"			},
	{ "end{description}","-12-NY-"			},
	{"begin{enumerate}", "-0>#NYP"			},
	{ "end{enumerate}",  "-1<NNY-"			},
	{ "begin{itemize}",  "-0>YNYP"			},
	{ "end{itemize}",    "-1<NNY-"			},
	{ "rm",		     "----n--"			},
	{ "sf",		     "----n--"			},
	{ "tt",		     "----f--"			},
	{ "sc",		     "----f--"			},
	{ "bf",		     "----b--"			},
	{ "em",		     "----i--"			},
	{ "it",		     "----i--"			},
	{ "sl",		     "----i--"			},
	{ "fo",		     "----i--"			},
	{ "textrm{",	     "----n--"			},
	{ "textsf{",	     "----n--"			},
	{ "texttt{",	     "----f--"			},
	{ "textsc{",	     "----f--"			},
	{ "textbf{",	     "----b--"			},
	{ "textem{",	     "----i--"			},
	{ "textit{",	     "----i--"			},
	{ "textsl{",	     "----i--"			},
	{ "textfo{",	     "----i--"			},
	{ "code{",	     "----f--"			},
	{ "emph{",	     "----i--"			}
    };

	/* if the CHAR pointer is NULL, then return NULL */
	if (!*refp)
		return NULL;

	/* Get first character of token */
	offset = markoffset(scanmark(refp));
	rettok.text[0] = **refp;
	rettok.offset[0] = offset++;
	rettok.nchars = 1;
	rettok.width = 0;
	rettok.markup = NULL;
	scannext(refp);

	if (!prefmt && rettok.text[0] == '\n' && *refp && **refp == '\n')
	{
		/* In fill mode, two or more consecutive newlines mark the
		 * end of a paragraph.
		 */
		do
		{
			scannext(refp);
		} while (*refp && elvspace(**refp));
		rettok.text[0] = '\\';
		rettok.text[1] = 'p';
		rettok.offset[1] = offset++;
		rettok.nchars++;
		rettok.markup = TEX_PARAGRAPH;

		reduce = ElvFalse; /* !!! why? */
		goto End;
	}
	else if (rettok.text[0] <= ' ')
	{
		/* This is a whitespace token.  Each whitespace token contains
		 * a SINGLE whitespace character.  Control characters other
		 * than '\t' and '\n' are treated as spaces.
		 */
		if (rettok.text[0] != '\t' && rettok.text[0] != '\n')
			rettok.text[0] = ' ';

		/* assume this whitespace will show, for computing line break */
		rettok.width = 1;
	}
	else if (rettok.text[0] == '\\' && *refp && elvalpha(**refp))
	{
		/* keyword -- collect the rest of the keyword name */
		do
		{
			if (rettok.nchars < QTY(rettok.text) - 3)
			{
				rettok.text[rettok.nchars] = **refp;
				rettok.offset[rettok.nchars] = offset++;
				rettok.nchars++;
			}
		} while (scannext(refp) && (elvalpha(**refp) || **refp == '_'));

		/* If name is followed by [], parse that */
		if (*refp && **refp == '[')
		{
			offset = texpair(refp, &rettok);
		}

		/* allow one space character before a { } character */
		if (*refp && (**refp == ' ' || **refp == '\t') && scannext(refp) && **refp != '{') /*}*/
		{
			/* Oops!  There was a space, but it wasn't followed by
			 * a '{' character.  We need to undo scannext().    }
			 */
			scanprev(refp);
		}

		/* For some keyword markups, parse the following {} */
		rettok.text[rettok.nchars] = '\0';
		if (*refp && (
			!CHARcmp(rettok.text, "\\begin")
		     || !CHARcmp(rettok.text, "\\end")
		     || !CHARcmp(rettok.text, "\\vspace")
		     || !CHARcmp(rettok.text, "\\label")
		     || !CHARcmp(rettok.text, "\\footnote")
		     ||	!CHARcmp(rettok.text, "\\title")
		     ||	!CHARcmp(rettok.text, "\\author")
		     || !CHARcmp(rettok.text, "\\part")
		     || !CHARcmp(rettok.text, "\\chapter")
		     || !CHARcmp(rettok.text, "\\section")
		     || !CHARcmp(rettok.text, "\\subsection")
		     || !CHARcmp(rettok.text, "\\cite")
		     || !CHARcmp(rettok.text, "\\ref")
		     || !CHARcmp(rettok.text, "\\bibitem")))
		{
			offset = texpair(refp, &rettok);
			rettok.text[rettok.nchars] = '\0';

			/* Some of those keywords need no more processing */
			if (!CHARncmp(rettok.text, "\\title", 6))
			{
				rettok.markup = TEX_TITLE;
				goto End;
			}
			if (!CHARncmp(rettok.text, "\\author", 7))
			{
				rettok.markup = TEX_AUTHOR;
				goto End;
			}
			if (!CHARncmp(rettok.text, "\\part", 5))
			{
				rettok.markup = TEX_PART;
				goto End;
			}
			if (!CHARncmp(rettok.text, "\\chapter", 8))
			{
				rettok.markup = TEX_CHAPTER;
				goto End;
			}
			if (!CHARncmp(rettok.text, "\\section", 8))
			{
				rettok.markup = TEX_SECTION;
				goto End;
			}
			if (!CHARncmp(rettok.text, "\\subsection", 11))
			{
				rettok.markup = TEX_SUBSECTION;
				goto End;
			}
		}

		/* for the \textXX tokens, parse a single { so the new font
		 * doesn't become the default font.
		 */
		if ((!CHARncmp(rettok.text, "\\text", 4)
			|| !CHARcmp(rettok.text, "\\code")
			|| !CHARcmp(rettok.text, "\\emph"))
		 && *refp
		 && **refp == '{')
		{
			rettok.text[rettok.nchars] = '{'; /*}*/
			rettok.offset[rettok.nchars] = offset;
			rettok.nchars++;
			offset++;
			scannext(refp);
		}

		/* Some special cases... */
		if (!CHARcmp(rettok.text, "\\halign")
		 || !CHARcmp(rettok.text, "\\begin{tabular}")
		 || !CHARcmp(rettok.text, "\\multicolumn")
		 || !CHARncmp(rettok.text, "\\set", 4)
		 || !CHARncmp(rettok.text, "\\def", 4)
		 || !CHARncmp(rettok.text, "\\new", 4)
		 || !CHARncmp(rettok.text, "\\catcode", 8)
		 || !CHARncmp(rettok.text, "\\document", 9))
		{
			/* collect chars up to EOL */
			if (*refp)
			{
				do
				{
					if (rettok.nchars < QTY(rettok.text) - 2)
					{
						rettok.text[rettok.nchars] = **refp;
						rettok.offset[rettok.nchars] = offset++;
						rettok.nchars++;
					}
				} while (scannext(refp) && **refp != '\n');
			}

			/* it has no effect */
			rettok.markup = TEX_IGNORE;
			goto End;
		}
		if (*refp && (
			!CHARcmp(rettok.text, "\\char")
		     || !CHARcmp(rettok.text, "\\mathrel")))
		{
			/* include the next char in the name */
			rettok.text[rettok.nchars] = **refp;
			rettok.offset[rettok.nchars] = offset++;
			rettok.nchars++;
			scannext(refp);

			/* ignore \charX, or output \mathrelX */
			rettok.markup = (rettok.text[1] == 'c')
					? TEX_IGNORE
					: TEX_OUTPUT;
			rettok.width = 1;
			goto End;
		}
		if (!CHARncmp(rettok.text, "\\item", 5))
		{
			rettok.markup = TEX_ITEM;
			goto End;
		}
		if (!CHARncmp(rettok.text, "\\cite", 5)
		 || !CHARncmp(rettok.text, "\\ref", 4)
		 || !CHARncmp(rettok.text, "\\bibitem", 8))
		{
			rettok.markup = TEX_OUTPUT;
			for (rettok.width = rettok.nchars;
			     rettok.width > 0 &&
				rettok.text[rettok.nchars-rettok.width] != '{';
			     rettok.width--) /*}*/
			{
			}
			if (rettok.width <= 0)
				rettok.width = rettok.nchars;
			goto End;
		}
		if (!CHARncmp(rettok.text, "\\vspace", 7))
		{
			rettok.markup = TEX_PARAGRAPH;
			goto End;
		}

		/* look up the keyword */
		rettok.text[rettok.nchars] = '\0';
		for (i = 0; i < QTY(markups) && CHARcmp(rettok.text + 1, markups[i].name); i++)
		{
		}
		if (i == QTY(markups))
		{
			rettok.markup = TEX_IGNORE;
			if (*refp && !CHARchr(rettok.text, '{')) /*}*/
			{
				switch (**refp)
				{
				  case '{': /*}*/
				  	offset = texpair(refp, &rettok);
				  	break;

				  case '=':
				  	/* include chars up to next newline */
					if (curfont == fontcode['f'])
					{
						rettok.markup = TEX_OUTPUT;
						rettok.width = rettok.nchars-1;
					}
					else
					{
						do
						{
							if (rettok.nchars < QTY(rettok.text) - 2)
							{
								rettok.text[rettok.nchars] = **refp;
								rettok.offset[rettok.nchars] = offset++;
								rettok.nchars++;
							}
						} while (scannext(refp) && **refp != '\n');
					}
				  	break;

				  case ' ':
				  case '\t':
				  case '\n':
				  case '\r':
					/* Output the word if font is "fixed",
					 * else do nothing special.  This is
					 * intended to make equations readable
					 * without cluttering up normal text.
					 */
					if (curfont == fontcode['f'])
					{
						rettok.markup = TEX_OUTPUT;
						rettok.width = rettok.nchars-1;
					}
					break;

				  case '\\':
				  	/* If next char is whitespace, then
				  	 * this word should be output.
				  	 */
				  	if (scannext(refp))
				  	{
				  		if (elvspace(**refp))
				  		{
							rettok.markup = TEX_OUTPUT;
							rettok.width = rettok.nchars - 1;
						}
						scanprev(refp);
					}
					break;

				  case '$':
					/* output the word */
					rettok.markup = TEX_OUTPUT;
					rettok.width = rettok.nchars - 1;
					break;

				  default: /* any punctuation */
					rettok.text[rettok.nchars] = **refp;
					rettok.offset[rettok.nchars] = offset++;
					rettok.nchars++;
					scannext(refp);
					rettok.markup = TEX_OUTPUT;
					rettok.width = rettok.nchars - (rettok.text[rettok.nchars - 1]=='\\' ? 2 : 1);
				}
			}
		}
		else
		{
			rettok.markup = &markups[i];
		}
	}
	else if (*refp && rettok.text[0] == '\\')
	{
		/* include the following punctuation character */
		rettok.text[rettok.nchars] = **refp;
		rettok.offset[rettok.nchars] = offset++;
		rettok.nchars++;
		scannext(refp);

		/* if followed by {...} then assume it should be a digraph.
		 * A double-backslash is treated like a \cr; others like text.
		 */
		if (*refp && **refp == '{')
		{
			offset = texpair(refp, &rettok);
			rettok.markup = TEX_DIGRAPH;
		}
		else if (rettok.text[1] == '\\')
			rettok.markup = TEX_CR;
		else if (rettok.text[1] == '/')
			rettok.markup = TEX_IGNORE;
		else if (rettok.text[1] != '\'')
		{
			if (rettok.text[1] < ' ')
				rettok.text[1] = ' ';
			rettok.markup = TEX_OUTPUT;
			rettok.width = rettok.nchars - 1;
		}
	}
	else if (rettok.text[0] == '%' && !prefmt)
	{
		/* comment -- collect chars up to end of line */
		while (**refp != '\n')
		{
			if (rettok.nchars < QTY(rettok.text) - 2)
			{
				rettok.text[rettok.nchars] = **refp;
				rettok.offset[rettok.nchars] = offset++;
				rettok.nchars++;
			}
			if (!scannext(refp))
				break;
		}
		rettok.markup = TEX_IGNORE;
	}
	else if (rettok.text[0] == '&' && !prefmt)
	{
		rettok.markup = TEX_HFIL;
	}
	else if (rettok.text[0] == '{' && !prefmt)
	{
		rettok.markup = TEX_OPENCURLY;
	}
	else if (rettok.text[0] == '}' && !prefmt)
	{
		rettok.markup = TEX_CLOSECURLY;
	}
	else if (rettok.text[0] == '$' && !prefmt)
	{
		/* if next char is also '$', then do it too */
		if (*refp && **refp == '$')
		{
			rettok.text[rettok.nchars] = **refp;
			rettok.offset[rettok.nchars] = offset++;
			rettok.nchars++;
			scannext(refp);
			rettok.markup = TEX_2DOLLARS;
		}
		else
		{
			rettok.markup = TEX_1DOLLAR;
		}
	}
	else
	{
		/* This is a word.  We already got the first char, now get the
		 * rest of them.
		 */
		for (;
		     *refp 
			&& rettok.nchars < QTY(rettok.text) - 1
			&& !CHARchr("{}\\$& \t\n\r", **refp);
		     offset++, scannext(refp))
		{
			rettok.text[rettok.nchars] = **refp;
			rettok.offset[rettok.nchars] = offset;
			rettok.nchars++;
		}
		rettok.width = rettok.nchars;
	}

	/* Mark the end of the text, and return the token */
End:
	rettok.text[rettok.nchars] = '\0';
	return &rettok;
}

static DMINFO *texinit(win)
	WINDOW	win;
{
	TOKEN	*token;
	MARKBUF	top;
	CHAR	*p;
	long	cursoffset;
	twrap_t	result;

	/* inherit some functions from dmnormal */
	dmtex.wordmove = dmnormal.wordmove;
	dmtex.tagatcursor = dmnormal.tagatcursor;
	dmtex.tagload = dmnormal.tagload;
	dmtex.tagnext = dmnormal.tagnext;

	/* initialize the fonts */
	initfonts();

	/* if merely initializing options, we're done */
	if (!win)
		return NULL;

	/* allocate the info struct */
	mui = (MUINFO *)safealloc(1, sizeof(MUINFO));
	mui->get = texget;
	mui->escape = texescape;
	mui->line = (LINEINFO *)safealloc(GRANULARITY, sizeof(LINEINFO));
	mui->nlines = 1; /* every buffer has at least one line */

	/* move the cursor someplace harmless, so it won't affect formatting */
	cursoffset = markoffset(win->cursor);
	marksetoffset(win->cursor, o_bufchars(markbuffer(win->cursor)));

	/* format the buffer */
	win->mi = (DMINFO *)mui;
	(void)start(win, marktmp(top, markbuffer(win->cursor), 0), NULL);
	p = scanalloc(&p, &top);
	result = TWRAP_NO;
	for (token = (*mui->get)(&p);
	     token;
	     token = (*mui->get)(&p))
	{
		/* if previous token forced a newline after it, then this token
		 * is the start of the next line.
		 */
		if (result == TWRAP_AFTER)
		{
			assert(first == ElvTrue && col == 0);
			storestate(token->offset[0], NULL);
		}

		/* if cursor is at position 0, and this is a text token,
		 * then move the cursor here.  This is done because otherwise
		 * the cursor would always start on an ugly formatting code.
		 */
		if (cursoffset == 0L 
		 && !title
		 && !elvspace(token->text[0])
		 && !token->markup)
		{
			cursoffset = token->offset[0];
		}

		/* output the token.  If it forces a new line, remember where
		 * that new line started.
		 */
		result = put(token);
		if (result == TWRAP_BEFORE)
		{
			assert(first == ElvTrue && col == 0);
			storestate(token->offset[0], NULL);
			(void)put(token);
		}
	}
	scanfree(&p);

	/* restore the cursor position */
	marksetoffset(win->cursor, cursoffset);

	/* Done! */
	return (DMINFO *)mui;
}
#endif /* DISPLAY_TEX */

/*----------------------------------------------------------------------------*/
/* Generic markup functions                                                   */

/* This function performs a single token.  If the token is markup, then the
 * side effects are performed and the function (if any) is called.  If text,
 * or if the cursor is in the token, then the text of the token is output.
 * It is assumed that drawchar and cursoff are set before this function is
 * called.  Returns an indicator of whether the text fits:
 *	TWRAP_NO	Text fits, continue line with next token
 *	TWRAP_BEFORE	Text doesn't fit, start new line with same token
 *	TWRAP_AFTER	Text fits, start new line with next token
 */
static twrap_t put(token)
	TOKEN	*token;
{
	char	tmpfont;
	CHAR	tmpch, lch, rch;
	int	i, origcol;
	ELVBOOL hascursor;	/* indicates whether this token contains cursor */
	twrap_t	result;

	/* determine whether the cursor is in this token.  If the "list"
	 * option is set, then pretend that all tokens contain the cursor.
	 */
	hascursor = (ELVBOOL)(list || (o_showmarkups && token->offset[0] <= cursoff && cursoff <= token->offset[token->nchars - 1]));

#ifdef DEBUG_MARKUP
	fprintf(stderr, "put(\"%s\") width=%d, hascursor=%s\n", token->text[0] == '\n' ? "\\n" : (char *)token->text, token->width, hascursor?"True":"False");
#endif
	/* is it a markup? */
	if (token->markup)
	{
		/* if this token will cause a line break, and no line break
		 * was already pending anyway, then do the line break and
		 * nothing else.  We'll do the rest of this markup's job
		 * at the start of the next line.
		 */
		if (token->markup->BREAKLN != '-' && !first)
		{
			(*drawchar)(newline, 1, 0, anyspc ? spcoffset : -1);
			reduce = (ELVBOOL)(col == 0);
			col = 0;
			first = ElvTrue;
			return TWRAP_BEFORE;
		}

		/* do all the standard effects */
		switch (token->markup->TITLE)
		{
		  case 'Y': title = ElvTrue;	break;
		  case 'N': title = ElvFalse;	break;
		}
		switch (token->markup->BREAKLN)
		{
		  case '0':
			reduce = ElvTrue;
			break;

		  case '1':
			if (!reduce)
				(*drawchar)(newline, 1, 0, anyspc ? spcoffset : -1);
			reduce = ElvTrue;
			break;

		  case '2':
			(*drawchar)(newline, reduce ? 1 : 2, 0, anyspc ? spcoffset : -1);
			reduce = ElvTrue;
			break;

		  case 'c':
			(*drawchar)(vtab, 1, 0, -1);
			reduce = ElvTrue;
			break;

		  case 'p':
			(*drawchar)(formfeed, 1, 0, -1);
			reduce = ElvTrue;
			break;
		}
		switch (token->markup->INDENT)
		{
		  case '<':
			/* decrease indentation */
			indent = (indent < listind ? 0 : indent - listind);
			break;

		  case '>':
			/* increase indentation */
			indent += listind;
			break;

		  case '=':
			/* set indentation to start of a table column */
			if (col >= tabstop)
			{
				do
				{
					indent += 2 * tabstop;
				} while (indent < col + 1);
			}
			break;

		  case '@':
			/* set indentation to start of leftmost table column */
			while (indent >= 3 * tabstop)
				indent -= 2 * tabstop;
			break;

		  case '|':
			/* if in heading, then end heading; else no change */
			if (indent < tabstop)
			{
				indent = tabstop;
				curfont = deffont = 0;
			}
			break;

		  case '0':
		  case '1':
		  case '2':
		  case '3':
		  case '4':
		  case '5':
		  case '6':
			/* set it to a specific indentation; '2' is normal */
			indent = (token->markup->INDENT - '0') * listind;
			break;
		}
		switch (token->markup->LIST)
		{
		  case 'N':
			if (nest > 0)
				nest--;
			break;

		  case 'Y':
			nest++;
			if (nest == 1)
				listcnt = 0;
			break;

		  case '#':
			nest++;
			if (nest == 1)
				listcnt = 1;
			break;
		}
		switch (token->markup->FONT)
		{
		  case '=': curfont = deffont;				break;
		  case '<': curfont = deffont; deffont = 0;		break;
		  case '~':
			/* toggle curfont between fixed & proportional, using
			 * deffont if possible.
			 */
			if (colorinfo[(int)curfont].da.bits & COLOR_PROP)
				if (colorinfo[(int)deffont].da.bits & COLOR_PROP)
					curfont = deffont = fontcode['f'];
				else
					curfont = deffont;
			else
				if (colorinfo[(int)deffont].da.bits & COLOR_PROP)
					curfont = deffont;
				else
					curfont = deffont = fontcode['n'];
			break;

		  case 'n':
		  case 'b':
		  case 'u':
		  case 'f':
		  case 'e':
		  case 'd':
		  case 'i': curfont = fontcode[(int)token->markup->FONT];break;
		  case 'N':
		  case 'B':
		  case 'U':
		  case 'F':
		  case 'E':
		  case 'D':
		  case 'I': curfont = deffont = fontcode[(int)token->markup->FONT]; break;
		}
		switch (token->markup->FILL)
		{
		  case 'Y': graphic = prefmt = ElvFalse;		break;
		  case 'N': prefmt = ElvTrue;				break;
		  case '~': prefmt = (ELVBOOL)(curfont==fontcode['f']);	break;
		}

		/* If there is a function, call it too.  If the function
		 * returns ElvTrue, then act as though the markup caused a
		 * newline.
		 */
		if (token->markup->fn)
		{
			result = (*token->markup->fn)(token);
			if (result != TWRAP_NO)
				return result;
		}

		/* if the cursor isn't on this token, don't show it */
		if (!hascursor)
		{
			first = ElvFalse;
			return TWRAP_NO;
		}
	}
	else
	{
		reduce = ElvFalse;
	}

	/* no token causes visible output when in "title" mode, unless the
	 * cursor happens to be located on it.
	 */
	if (title && !hascursor)
	{
		first = ElvFalse;
		return TWRAP_NO;
	}

	/* Newlines are handled differently that other whitespace, when in
	 * prefmt mode.  This is because newlines mark the end of one line,
	 * so they become the first token of the next line... but when we
	 * get to the next line we don't want to process the newline again,
	 * or we'd get into an endless loop.  Here we catch the case were
	 * we are calling put() the second time.
	 */
	if (prefmt && token->text[0] == '\n' && first)
	{
		first = ElvFalse;
#if 0
		anyspc = ElvTrue;
		spcoffset = token->offset[0];
		spcfont = curfont;
#else
		anyspc = ElvFalse;
#endif
		return TWRAP_NO;
	}

	/* Expand any escape sequences in the token's text */
	if (mui->escape && !token->markup)
		(*mui->escape)(token);

	/* Is it whitespace?  Are we supposed to adjust the text formatting? */
	if (token->nchars == 1 && elvspace(token->text[0]) && !prefmt)
	{
		/* Just set a flag indicating that a space has been
		 * encountered.  Also remember its offset if it is the first
		 * space encountered, or if the cursor is on this space.
		 */
		if (!anyspc || hascursor)
		{
			spcoffset = token->offset[0];
			spcfont = curfont;
		}
		anyspc = ElvTrue;
		first = ElvFalse;
		return TWRAP_NO;
	}

	/* If not a markup, and it won't fit on the current line, then output
	 * a '\n' and return ElvTrue so it'll appear on the next line.
	 *
	 * NOTE: In order to prevent orphan punctuation, if the current token
	 * wasn't preceded by whitespace and it wouldn't cause a line wrap,
	 * then it is considered to fit; if preceded by whitespace, then we
	 * reduce the line length slightly because it looks better that way.
	 */
	else if (col + token->width + 1 > textwidth - (anyspc ? 4 : 0)
		&& !prefmt && col != 0 && !token->markup)
	{
		(*drawchar)(newline, 1, 0, anyspc ? spcoffset : -1);
		col = 0;
		anyspc = ElvFalse;
		first = ElvTrue;
		return TWRAP_BEFORE;
	}

	/* if we need to adjust our indent, do it now */
	if (col < indent)
	{
		(*drawchar)(space, col - indent, 0, anyspc ? spcoffset : -1);
		col = indent;
		anyspc = ElvFalse;
	}

	/* Output a space between tokens, usually. */
	if (anyspc)
	{
		(*drawchar)(space, 1, spcfont, spcoffset);
		col++;
		anyspc = ElvFalse;
	}

	/* remember our current column, so we can pretend visible markups are
	 * zero characters wide later.
	 */
	origcol = col;

	/* Output the token's text.  Watch for whitespace and maybe graphics. */
	for (i = 0; i < token->nchars; i++)
	{
		switch (token->text[i])
		{
		  case '\t':
			/* convert to spaces */
			token->width = tabstop - (col - indent) % tabstop;
			(*drawchar)(space, -token->width, curfont, token->offset[0]);
			break;

		  case '\n':
			/* newlines are tricky.  If we get here, then we must
			 * be in no-fill mode, so the newline should be output.
			 * However, we'll need to be on the lookout for this
			 * same newline next time.
			 */
			(*drawchar)(newline, 1, 0, -1);
			col = 0;
			anyspc = ElvTrue;
			first = ElvTrue;
			return TWRAP_BEFORE;

		  case '|':
		  case '.':
		  case '^':
		  case '-':
			tmpfont = curfont;
			tmpch = token->text[i];
			if (graphic)
			{
				lch = (i > 0 ? token->text[i - 1] : ' ');
				rch = (i < token->nchars - 1 ? token->text[i + 1] : ' ');
				if (lch == '-' && rch == '-')
				{
					if (tmpch == '|')	tmpch = '5';
					else if (tmpch == '.')	tmpch = '8';
					else if (tmpch == '^')	tmpch = '2';
					tmpfont = fontcode['g'];
				}
				else if (lch == '-')
				{
					if (tmpch == '|')	tmpch = '6';
					else if (tmpch == '.')	tmpch = '9';
					else if (tmpch == '^')	tmpch = '3';
					tmpfont = fontcode['g'];
				}
				else if (rch == '-')
				{
					if (tmpch == '|')	tmpch = '4';
					else if (tmpch == '.')	tmpch = '7';
					else if (tmpch == '^')	tmpch = '1';
					tmpfont = fontcode['g'];
				}
				else if (tmpch == '|')
				{
					tmpfont = fontcode['g'];
				}
			}
			(*drawchar)(&tmpch, 1, tmpfont, token->offset[i]);
			col++;
			break;

		  default:
			if (token->text[i] < ' ')
				token->text[i] = ' ';
			(*drawchar)(&token->text[i], 1, token->markup ? fontcode['m'] : curfont, token->offset[i]);
			col++;
		}
	}

	/* we won't need any more implied spaces between tokens until we hit
	 * the next whitespace token in fill mode.
	 */
	anyspc = ElvFalse;

	/* markup tokens aren't supposed to affect the logical column number */
	col = origcol + token->width;

	/* Done! */
	first = ElvFalse;
	return TWRAP_NO;
}



/* This is a version of drawchar() which does nothing except count the column
 * position.  It is used by mark2col() to figure out which column the cursor
 * is on, and move() to find the offset for a given column.
 */
static long physcol;	/* physical column counter - initialize to 0 */
static long wantoffset;	/* desired offset, whose column we'll be tracking */
static long outcol;	/* physical column of wantoffset - initialize to -1 */
static long wantcol;	/* desired column, whose offsets we'll be tracking */
static long outoffset;	/* offset of char at a given column - init to -1 */
static long prevoffset;	/* previous offset which was >= 0 */
static long lastoffset;	/* offset of last non-blank character before wantcol */

static void countchar(p, qty, font, offset)
	CHAR	*p;	/* the character being output */
	long	qty;	/* quantity of characters */
	_char_	font;	/* the font to show it in (ignored) */
	long	offset;	/* which buffer char this corresponds to */
{
	long	delta;
	register CHAR ch;

	/* a negative qty indicates that the same character repeats */
	if (qty < 0)
	{
		delta = 0;
		qty = -qty;
	}
	else
	{
		delta = 1;
	}

	/* for each character... */
	for ( ; qty > 0; qty--, p += delta, offset += delta)
	{
		/* copy *p into a register variable */
		ch = *p;

		/* if this is the desired offset, remember its column */
		if (offset >= wantoffset && outcol < 0)
			outcol = physcol;

		/* if this offset >= 0, then remember it */
		if (offset >= 0)
			prevoffset = offset;

		/* process the character */
		if (ch == '\n' || ch == '\f' || ch == '\013')
		{
			if (outoffset == -1)
			{
				outoffset = lastoffset;
			}
			lastoffset = -1;
			physcol = 0;
		}
		else
		{
			if (physcol >= wantcol)
			{
				if (outoffset < 0)
					outoffset = prevoffset;
			}
			else if (!elvspace(ch))
				lastoffset = offset;
			physcol++;
		}
	}
}


/*----------------------------------------------------------------------------*/
/* Elvis display mode functions                                               */

/* end the mode, and free the modeinfo */
static void term(info)
	DMINFO	*info;	/* window-specific info about mode */
{
	MUINFO	*mui = (MUINFO *)info;

	if (mui->title)
		safefree(mui->title);
	if (mui->line)
		safefree(mui->line);
	safefree(info);
}

/* Convert a mark to a screen-relative column number */
static long mark2col(w, mark, cmd)
	WINDOW	w;	/* window where buffer is shown */
	MARK	mark;	/* mark to convert */
	ELVBOOL	cmd;	/* if ElvTrue, we're in command mode; else input mode */
{
	CHAR	*p;	/* used for scanning */
	TOKEN	*token;
	MARKBUF	tmp;
	int	i;

	/* Start scanning at the start of the line which contains this mark.
	 * Count characters until we find the mark's offset, or until the
	 * end of the line is reached.
	 */
	i = start(w, mark, NULL);
	p = scanalloc(&p, marktmp(tmp, markbuffer(mark), mui->line[i].offset));
	while ((token = (*mui->get)(&p)) != NULL && put(token) == TWRAP_NO && outcol < 0)
	{
	}
	scanfree(&p);
	return (outcol>=0 ? outcol : physcol);
}


/* Move vertically, and to a given column (or as close to column as possible) */
static MARK move(w, from, linedelta, column, cmd)
	WINDOW	w;		/* window where buffer is shown */
	MARK	from;		/* old location */
	long	linedelta;	/* line movement */
	long	column;		/* desired column number */
	ELVBOOL	cmd;		/* if ElvTrue, we're in command mode; else input mode */
{
	int	i, j;
	CHAR	*p;
	TOKEN	*token;
 static MARKBUF	tmp;

	/* find the start of the line containing the mark */
	i = start(w, from, NULL);

	/* Add linedelta to the index, being careful not to move outside
	 * the array.
	 */
	i += linedelta;
	if (i >= mui->nlines)
		i = mui->nlines - 1;
	if (i < 0)
		i = 0;

	/* Move to the given column, or as close as possible */
	if (column == 0)
	{
		outoffset = mui->line[i].offset;
	}
	else
	{
		j = start(w, marktmp(tmp, markbuffer(from), mui->line[i].offset), NULL);
		assert(j == i);
		wantcol = column;
		scanalloc(&p, marktmp(tmp, markbuffer(from), mui->line[j].offset));
		while ((token = (*mui->get)(&p)) != NULL
		    && put(token) == TWRAP_NO
		    && outoffset < 0)
		{
		}
		scanfree(&p);
		if (outoffset < 0L)
			outoffset = lastoffset;
	}

	/* return the found mark */
	if (outoffset < mui->line[i].offset)
		outoffset = mui->line[i].offset;
	return marktmp(tmp, markbuffer(from), outoffset);
}

/* Choose a line to appear at the top of the screen, and return its mark.
 * Also, initialize the info for the next line.
 */
static MARK setup(win, top, cursor, bottom, info)
	WINDOW	win;	/* window we're probably doing this for */
	MARK	top;	/* where previous image started */
	long	cursor;	/* offset of cursor */
	MARK	bottom;	/* where previous image ended */
	DMINFO	*info;	/* window-specific info about mode */
{
	int	topi, bottomi; /* line indicies of the top & bottom */
 static MARKBUF	tmp;
	int	i;

	/* we can optimize if "nolist noshowmarkup" */
	i = (calcelement(o_listchars, toCHAR("markup")) != NULL);
#ifdef DISPLAY_HTML
	dmhtml.canopt =
#endif
#ifdef DISPLAY_MAN
	dmman.canopt =
#endif
#ifdef DISPLAY_TEX
	dmtex.canopt =
#endif
		(ELVBOOL)(!(o_list(win) && i) && !o_showmarkups);
	
	/* find the line indicies of the top & bottom marks, and the cursor */
	/* NOTE: This could have been implemented more efficiently! */
	topi = start(win, top, NULL);
	bottomi = start(win, bottom, NULL);
	i = start(win, marktmp(tmp, markbuffer(top), cursor), NULL);

	/* if the cursor is between top & bottom, great!  Else we need to
	 * either scroll or jump.
	 */
	if (topi <= i && i < bottomi)
	{
		/* great! */
	}
	else if (topi - o_nearscroll <= i && i < topi)
	{
		topi = i;
	}
	else if (bottomi <= i && i < bottomi + o_nearscroll)
	{
		topi = i - o_lines(win) - 1;
		/* NOTE: if sidescrolling is disabled and the window is narrow,
		 * then the above computation may set topi back farther than
		 * necessary.  This is inefficient, but doesn't result in any
		 * visible problems because the Draw module will compensate by
		 * slop scrolling.
		 */
	}
	else
	{
		topi = i - o_lines(win) / 2;
		win->di->logic = DRAW_CENTER;
	}

	/* topi should never be negative */
	if (topi < 0)
		topi = 0;

	/* return the top of the starting line */
	return marktmp(tmp, markbuffer(top), mui->line[topi].offset);
}

static MARK image(w, line, info, draw)
	WINDOW	w;		/* window where drawing will go */
	MARK	line;		/* start of line to draw */
	DMINFO	*info;		/* window-specific info about mode */
	void	(*draw)P_((CHAR *p, long qty, _char_ font, long offset));
				/* function for drawing a single character */
{
	CHAR	*p;
	TOKEN	*token;
 static MARKBUF	tmp;
	twrap_t	result;

	/* initialize "result" just to keep the compiler happy */
	result = TWRAP_BEFORE;

	/* generate the line image, using the given "draw" function */
	(void)start(w, line, draw);
	scanalloc(&p, line);
	for (token = (*mui->get)(&p);
	     token && (result = put(token)) == TWRAP_NO;
	     token = (*mui->get)(&p))
	{
	}
	if (p)
		tmp = *scanmark(&p);
	scanfree(&p);

	/* if we hit the end of the buffer, and didn't output a newline,
	 * then output a newline now.
	 */
	if (!token)
	{
		(*draw)(newline, 1, 0, anyspc ? spcoffset : -1);
	}

	/* return the offset of the first token of the next line */
	if (!token)
		return marktmp(tmp, markbuffer(line), o_bufchars(markbuffer(line)));
	else if (result == TWRAP_BEFORE)
		return marktmp(tmp, markbuffer(line), token->offset[0]);
	else
		return &tmp; /* already set via scanmark(), above */
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
	CHAR	*sides;	/* text to show on left & right sides of page */
	CHAR	*middle;/* text to show in middle of page */
	int	slen;	/* length of side string */
	int	mlen;	/* length of middle string */
	long	gap1;	/* width of gap between left side & middle */
	long	gap2;	/* width of gap between middle & right side */
 static int	font_header;

	assert(info == (DMINFO *)mui);

	/* if lpheader isn't set, then don't bother */
	if (!o_lpheader)
		return;

	/* if first time, then find the "header" font */
	if (!font_header)
	{
		font_header = colorfind(toCHAR("header"));
		colorset(font_header, toCHAR("underlined"), ElvFalse);
	}

	/* covert page number to text */
	long2CHAR(pg, (long)pagenum);

	/* find the title of the document */
	title = mui->title;
	if (!title)
		title = o_filename(markbuffer(w->cursor));
	if (!title)
		title = o_bufname(markbuffer(w->cursor));

	/* Find the widths of things */

	/* figure out what goes where */
#ifdef DISPLAY_MAN
	if (mui->escape == manescape)
	{
		sides = title;
		middle = pg;
	}
	else
#endif
	{
		sides = pg;
		middle = title;
	}

	/* find the lengths of things */
	slen = CHARlen(sides);
	mlen = CHARlen(middle);
	if (slen + 2 + mlen + 2 + slen > textwidth)
	{
		mlen = textwidth - 4 - 2 * slen;
		if (mlen <= 0)
		{
			mlen = CHARlen(middle);
			slen = (textwidth - 4 - mlen) / 2;
		}
	}
	gap1 = (textwidth - mlen) / 2 - slen;
	gap2 = textwidth - mlen - 2 * slen - gap1;

	/* Output the parts of the headings */
	(*draw)(sides, slen, font_header, -2L);
	(*draw)(space, -gap1, font_header, -2L);
	(*draw)(middle, mlen, font_header, -2L);
	(*draw)(space, -gap2, font_header, -2L);
	(*draw)(sides, slen, font_header, -2L);

	/* End the header line, and then skip one or two more lines */
	(*draw)(newline, pagenum==1 ? -2L : -3L, 0, -2L);
	reduce = ElvTrue;
}
#endif /* FEATURE_LPR */


/*----------------------------------------------------------------------------*/
/* Maintenance functions                                                      */

/* store the offset and parse state in "dest"... or append it to mui->line
 * if "dest" is a NULL pointer.
 */
static void storestate(offset, dest)
	long	offset;
	LINEINFO *dest;
{
	int	i;

	/* if "dest" is NULL, then we'll be appending */
	if (!dest)
	{
		/* we may need to reallocate mui->line */
		if (mui->nlines % GRANULARITY == 0)
		{
			dest = mui->line;
			mui->line = (LINEINFO *)safealloc((int)(mui->nlines + GRANULARITY), sizeof(LINEINFO));
			if (dest)
			{
				memcpy(mui->line, dest, (size_t)mui->nlines * sizeof(LINEINFO));
				safefree(dest);
			}
		}

		/* locate the end of mui->line[], and increment mui->nlines */
		dest = &mui->line[mui->nlines++];
	}

	/* store the info */
	dest->offset = offset;
	dest->state.prefmt = prefmt ? 1 : 0;
	dest->state.graphic = graphic ? 1 : 0;
	dest->state.midline = midline ? 1 : 0;
	dest->state.reduce = reduce ? 1 : 0;
	dest->state.deffont = dest->state.curfont = 0;
	for (i = 0; fonttable[i]; i++)
	{
		 if (deffont == fontcode[(int)(fonttable[i])])
			dest->state.deffont = i;
		 if (curfont == fontcode[(int)(fonttable[i])])
			dest->state.curfont = i;
	}
	dest->state.indent = indent;
	dest->state.nest = nest;
	dest->state.listcnt = listcnt;
}


/* This function sets "mui" and the other parsing variables from the
 * line information of a given window, for a given starting point.  This
 * function also sets "drawchar" function pointer to a given value; if no
 * "drawchar" function is given, then it uses the internal dummy function
 * and sets up that function's initial state.
 *
 * Returns the index into mui->line[] of the starting line
 */
static int start(win, from, draw)
	WINDOW	win;	/* window to draw for */
	MARK	from;	/* starting point */
	void	(*draw) P_((CHAR *p, long qty, _char_ font, long offset));
{
	int	i;

	/* find the start of the line which contains "from" */
	mui = (MUINFO *)win->mi;
	for (i = 0; i < mui->nlines && mui->line[i].offset <= markoffset(from); i++)
	{
	}
	if (i > 0) i--; /* the above loop took us one line too far */

	/* copy that line's parsing state into parsing variables */
	prefmt = (ELVBOOL)mui->line[i].state.prefmt;
	graphic = (ELVBOOL)mui->line[i].state.graphic;
	midline = (ELVBOOL)mui->line[i].state.midline;
	reduce = (ELVBOOL)mui->line[i].state.reduce;
	deffont = fontcode[(int)(fonttable[mui->line[i].state.deffont])];
	curfont = fontcode[(int)(fonttable[mui->line[i].state.curfont])];
	indent = mui->line[i].state.indent;
	nest = mui->line[i].state.nest;
	listcnt = mui->line[i].state.listcnt;

	/* initialize other variables, too */
	first = ElvTrue;
	anyspc = ElvFalse;
	title = ElvFalse; /* nothing that causes a linebreak can appear in title */
	list = (ELVBOOL)(o_list(win) && (calcelement(o_listchars, toCHAR("markup")) != NULL));
	readonly = o_readonly(markbuffer(from));
	textwidth = o_columns(win);
	tabstop = o_tabstop(markbuffer(win->cursor))[1];
	listind = o_shiftwidth(markbuffer(win->cursor))[1] / 2;
	if (listind < 2)
		listind = 2;
	col = 0;
	cursoff = markoffset(win->cursor);

	/* set up the drawchar function pointer */
	if (draw)
	{
		drawchar = draw;
	}
	else
	{
		drawchar = countchar;
		physcol = 0;
		wantoffset = markoffset(from);
		outcol = -1;
		wantcol = 0;
		outoffset = -1;
		prevoffset = -1;
		lastoffset = -1;
	}

	return i;
}


/* Locate the title in a given buffer, and store it in mui->title.  Also
 * sets mui->endtitle to an appropriate value.  It is assumed that the mui
 * pointer has been initialized before this function is called.
 */
static void findtitle(buf)
	BUFFER	buf;	/* the buffer to scan */
{
	CHAR	*p;	/* used for scanning the buffer */
	TOKEN	*token;	/* a token from the buffer */
	MARKBUF	tmp;
	int	i;

	/* free the old title, if any */
	if (mui->title)
	{
		safefree(mui->title);
		mui->title = NULL;
	}

	/* read the first non-whitespace token from the buffer */
	scanalloc(&p, marktmp(tmp, buf, 0));
	do
	{
		token = (*mui->get)(&p);
	} while (token && elvspace(token->text[0]));

	/* if no token, then no title */
	if (!token)
	{
		mui->endtitle = o_bufchars(buf);
		scanfree(&p);
		return;
	}

	/* if token doesn't explicitly allow title to follow, then no title */
	if (!token->markup || token->markup->TITLE != 'Y')
	{
		mui->endtitle = token->offset[0];
		scanfree(&p);
		return;
	}

	/* Collect characters from text tokens, up to next markup which
	 * can't appear in the title.
	 */
	while ((token = (*mui->get)(&p)) != NULL && (!token->markup || token->markup->TITLE == 'Y'))
	{
		/* if text, then append it to title, with a leading blank */
		if (!token->markup && !elvspace(token->text[0]))
		{
			if (mui->title)
				buildCHAR(&mui->title, ' ');
			for (i = 0; i < token->nchars; i++)
				buildCHAR(&mui->title, token->text[i]);
		}
	}
	scanfree(&p);
}


/* Adjust the line tables of any window whose main buffer has been altered.
 * This function is meant to be called only from bufreplace().
 */
void dmmuadjust(from, to, delta)
	MARK	from;	/* old start of text */
	MARK	to;	/* old end of text */
	long	delta;	/* difference between old "to" and new "to" offsets */
{
	WINDOW	win;	/* used for scanning windows */
	CHAR	*p;	/* used for scanning text */
	TOKEN	*token;	/* used for scanning text */
	LINEINFO li;	/* buffer for generating LINEINFO record */
	LINEINFO *old;	/* old value of mui->line (previous line array) */
#if 0
	int	noldlines;/* old value of mui->nlines */
#endif
	MARKBUF	tmp;
	int	i;
	twrap_t	result;

#if 0
	fprintf(stderr, "dmmuadjust({%s,%ld}, {%s,%ld}, %ld)\n",
		o_bufname(markbuffer(from)), markoffset(from),
		o_bufname(markbuffer(to)), markoffset(to),
		delta);
#endif

	/* for each window that uses this buffer... */
	for (win = winofbuf(NULL, markbuffer(from));
	     win;
	     win = winofbuf(win, markbuffer(from)))
	{
		/* ignore if not in a markup display mode */
		if (win->md->setup != setup)
		{
			continue;
		}

		/* If necessary, rescan the title of this buffer */
		mui = (MUINFO *)win->mi;
		if (mui->endtitle <= markoffset(from))
			findtitle(markbuffer(from));

		/* Start on the line BEFORE the one where changes begin.
		 * This is because if we insert a small word at the front of
		 * a line, it might fit at the end of the preceding line.
		 */
		i = start(win, from, (void(*)P_((CHAR *,long,_char_,long)))0);
		if (i > 0) i--;

		/* Pretend the cursor is someplace harmless.  This is done
		 * because the cursor position affects the appearance of the
		 * text, and sometimes this affects where line breaks occur.
		 */
		cursoff = o_bufchars(markbuffer(from));

		/* regenerate that preceding line */
		p = scanalloc(&p, marktmp(tmp, markbuffer(from), mui->line[i].offset));
		do
		{
			token = (*mui->get)(&p);
		} while (token && (result = put(token)) == TWRAP_NO);

		/* regenerate the line where the change took place */
		if (token && token->offset[token->nchars - 1] != o_bufchars(markbuffer(from)) - 1)
		{
			/* but first, store results of preceding line's scan */
			i++;
			storestate(token->offset[0], &mui->line[i]);

			/* okay, now scan the changed line.  We already have
			 * the first token, unless the previous put() returned
			 * TWRAP_AFTER.
			 */
			if (result == TWRAP_AFTER)
			{
				token = (*mui->get)(&p);
			}
			while (token && !put(token))
			{
				token = (*mui->get)(&p);
			}
		}

		/* If we hit the end of the buffer, then no adjustments are
		 * necessary.  This may leave superfluous data at the end of
		 * the array, but who cares? 
		 */
		if (!token || token->offset[token->nchars - 1] == o_bufchars(markbuffer(from)) - 1)
		{
			mui->nlines = i + 1;
			scanfree(&p);
#if 0
			fprintf(stderr, "\tNo adjustment needed; hit end of buffer\n");
#endif
			continue;
		}

		/* If the new line ended with the same (adjusted) offset and
		 * same state, then no adjustments are necessary except to
		 * the offsets.
		 */
		storestate(token->offset[0], &li);
		if (i + 1 < mui->nlines
		 && mui->line[i + 1].offset + delta == token->offset[0]
		 && !memcmp(&li.state, &mui->line[i + 1].state, sizeof li.state)
		 && mui->line[i + 1].offset >= markoffset(to))
		{
			if (delta == 0)
			{
#if 0
				fprintf(stderr, "\tNo adjustment needed; same state and delta=0\n");
#endif
				scanfree(&p);
				continue;
			}

			for (i++; i < mui->nlines; i++)
			{
				mui->line[i].offset += delta;
			}
			scanfree(&p);
#if 0
			fprintf(stderr, "\tAdjusted the offsets only!\n");
#endif
			continue;
		}
		
		/* Adjustments are going to be complex.  Start a new array,
		 * but don't throw the old one out yet because it may save
		 * us some work.
		 */
		old = mui->line;
#if 0
		noldlines = mui->nlines;
#endif
		mui->line = (LINEINFO *)safealloc((i + 1 + GRANULARITY) - ((i + 1) % GRANULARITY), sizeof(LINEINFO));
		for (mui->nlines = 0; mui->nlines <= i; mui->nlines++)
		{
			mui->line[mui->nlines] = old[mui->nlines];
		}
		mui->line[mui->nlines++] = li;

		/* regenerate lines until we have same (adjusted) offset and
		 * state as we had before the change.
		 */ 
		do
		{
			/* Regenerate a line */
			put(token); /* does twrap_t affect this? */
			do
			{
				token = (*mui->get)(&p);
			} while (token && (result = put(token)) == TWRAP_NO);
			if (!token)
			{
				break;
			}

			/* Append its state to the new array.  */
			storestate(token->offset[0], (LINEINFO *)0);

			/* if the line's offset comes after the adjusted "to"
			 * offset, and there is an old state & adjusted offset
			 * which match, then just copy the old info.
			 */
			/*!!!*/
		} while (token);
		scanfree(&p);
		safefree(old);
#if 0
		fprintf(stderr, "\tRegenerated info for every following line\n");
#endif
	}
}


/*----------------------------------------------------------------------------*/
#ifdef DISPLAY_HTML
DISPMODE dmhtml =
{
	"html",
	"WWW hypertext",
	ElvFalse,	/* display generating can't be optimized */
	ElvFalse,	/* shouldn't use standard wordwrap */
	0,	/* no window options */
	NULL,
	0,	/* no global options */
	NULL,
	NULL,
	htmlinit,
	term,
	mark2col,
	move,
	NULL,	/* wordmove will be set to dmnormal.wordmove in init() */
	setup,
	image,
#ifdef FEATURE_LPR
	header,
#else
	NULL,	/* no header function, since printing is disabled */
#endif
	NULL,	/* no autoindent */
	htmltagatcursor,
	htmltagload,
	htmltagnext
};
#endif /* defined(DISPLAY_HTML) */

#ifdef DISPLAY_MAN
DISPMODE dmman =
{
	"man",
	"nroff -man",
	ElvFalse,	/* display generating can't be optimized */
	ElvFalse,	/* shouldn't use standard wordwrap */
	0,	/* no window options */
	NULL,
	0,	/* no global options */
	NULL,
	NULL,
	maninit,
	term,
	mark2col,
	move,
	NULL,	/* wordmove will be set to dmnormal.wordmove in init() */
	setup,
	image,
#ifdef FEATURE_LPR
	header,
#else
	NULL,	/* no header function, since printing is disabled */
#endif
	NULL,	/* no autoindent */
	NULL,	/* tagatcursor will be set to dmnormal.tagatcursor in init() */
	NULL,	/* tagload will be set to dmnormal.tagload in init() */
	NULL,	/* tagnext will be set to dmnormal.tagnext in init() */
};
#endif /* defined(DISPLAY_MAN) */

#ifdef DISPLAY_TEX
DISPMODE dmtex =
{
	"tex",
	"TeX",
	ElvFalse,	/* display generating can't be optimized */
	ElvFalse,	/* shouldn't use standard wordwrap */
	0,	/* no window options */
	NULL,
	0,	/* no global options */
	NULL,
	NULL,
	texinit,
	term,
	mark2col,
	move,
	NULL,	/* wordmove will be set to dmnormal.wordmove in init() */
	setup,
	image,
#ifdef FEATURE_LPR
	header,
#else
	NULL,	/* no header function, since printing is disabled */
#endif
	NULL,	/* no autoindent */
	NULL,	/* tagatcursor will be set to dmnormal.tagatcursor in init() */
	NULL,	/* tagload will be set to dmnormal.tagload in init() */
	NULL,	/* tagnext will be set to dmnormal.tagnext in init() */
};
#endif /* defined(DISPLAY_TEX) */

#endif /* DISPLAY_ANYMARKUP */
