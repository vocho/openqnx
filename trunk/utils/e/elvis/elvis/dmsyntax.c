/* dmsyntax.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_dmsyntax[] = "$Id: dmsyntax.c,v 2.104 2003/10/19 23:09:33 steve Exp $";
#endif
#ifdef DISPLAY_SYNTAX

/* These are the descriptions and values of some global options */
static OPTDESC globdesc[] =
{
	{"includepath", "inc",	optsstring,	optisstring }
};
static OPTVAL globval[QTY(globdesc)];
#define o_includepath	globval[0].value.string

/* This data type is used to denote a token type.  Values of this type will
 * be used as indices into the cfont[] array, below, to determine which
 * font each language element should use.  The last symbol in the list must
 * be PUNCT, because the declaration of cfont[] depends on this.
 */
typedef enum
{
	COMMENT, COMMENT2, STRING0, STRING1, CHARACTER, REGEXP, REGSUB,
	KEYWORD, DOC, DOCMARKUP, DOCINDENT, FUNCTION, VARIABLE, NUMBER,
	PREP, PREPWORD, PREPQUOTE, OTHER, PUNCT
} TOKENTYPE;

/* This structure stores information about the current language's syntax.
 * Each window has its own copy of this, so different windows which happen
 * to be in "syntax" mode can each display different languages.
 */
typedef struct
{
	/* info about the current parsing state */
	TOKENTYPE token;	/* used during parsing */

	/* info from the "elvis.syn" file */
	spell_t	*keyword;	/* dictionary of keyword names */
	CHAR	*opkeyword;	/* "operator" keyword, or NULL */
	CHAR	*setargs;	/* arguments to ":set" command */
	CHAR	function;	/* character used for function calls */
	CHAR	backslash;	/* string quote character - usually backslash */
	CHAR	strbegin[2];	/* string start-quote character */
	CHAR	strend[2];      /* string end-quote character */
	CHAR	charbegin;	/* character quote character */
	CHAR	charend;	/* character quote character */
	CHAR	preprocessor;	/* first character of preprocessor directives */
	CHAR	pqbegin;	/* preprocessor start-quote character */
	CHAR	pqend;		/* preprocessor start-quote character */
	CHAR	comment[2];	/* start of one-line comment */
	CHAR	combegin[2];	/* start of multi-line comment */
	CHAR	comend[2];	/* end of multi-line comment */
	CHAR	enddoc[20];	/* end of embedded docs ("" if not used) */
	ELVBOOL	allcaps;	/* uppercase words are "other" */
	ELVBOOL	initialcaps;	/* mixed case words starting with upper are "other" */
	ELVBOOL	mixedcaps;	/* mixed case words starting with lower are "other" */
	ELVBOOL	finalt;		/* words ending with "_t" are "other" */
	ELVBOOL	initialpunct;	/* words starting with punctuation */
	ELVBOOL	ignorecase;	/* keywords may be uppercase */
	ELVBOOL	strnewline;	/* literal newline chars allowed in strings */
	ELVBOOL strnoindent;	/* indentation not allowed in multi-line strings */
	ELVBOOL strnoempty;	/* empty lines not allowed in multi-line strings */
	char	mostly;		/* font code of variables */
	char	wordbits[256];	/* which chars can appear in a word */
} SINFO;
#define STARTWORD	0x01	/* char: can start a word */
#define INWORD		0x02	/* char: can occur within a word */
#define DELIMREGEXP	0x04	/* char: can delimit regular expression */
#define ISPREFIX	0x01	/* word: may be first punct of 2-punct word */
#define ISCOMMENT	0x02	/* word: marks the start of a comment */
#define ISKEYWORD	0x04	/* word: is this a keyword? (else tag) */
#define USEREGEXP	0x10	/* either: occurs before possible regexp */
#define USEREGSUB	0x20	/* either: occurs before possible regsub */
#define OPERATOR	0x40	/* either: prefix to operatorXX name */
#define isstartword(si,c)	((si)->wordbits[(CHAR)(c)] & STARTWORD)
#define isinword(si,c)		((si)->wordbits[(CHAR)(c)] & INWORD)
#define isregexp(si,c)		((si)->wordbits[(CHAR)(c)] & DELIMREGEXP)
#define isbeforeregexp(si,c)	((si)->wordbits[(CHAR)(c)] & USEREGEXP)
#define isbeforeregsub(si,c)	((si)->wordbits[(CHAR)(c)] & USEREGSUB)
#define isoperator(si,c)	((si)->wordbits[(CHAR)(c)] & OPERATOR)
#define wordflags(w)		((w)->flags)
#define wordanchor(w)		(((w)->flags >> 16) & 0xfff)
#define wordsetanchor(w, a)	((w)->flags = ((w)->flags & ~0x0fff0000) | ((a) << 16))
#define wordfont(w)		((char)((w)->flags >> 8))
#define wordsetfont(w, f)	((w)->flags = ((w)->flags & ~0x0000ff00) | ((f) << 8))
#define wordprefix(w)		(wordflags(w) & ISPREFIX)
#define wordcomment(w)		(wordflags(w) & ISCOMMENT)
#define wordkeyword(w)		(wordflags(w) & ISKEYWORD)
#define wordbeforeregexp(w)	(wordflags(w) & USEREGEXP)
#define wordbeforeregsub(w)	(wordflags(w) & USEREGSUB)
#define wordoperator(w)		(wordflags(w) & OPERATOR)

/* Most anchor values are in the range 1 - 0xffe, but these are special */
#define ANCHOR_NONE	0
#define ANCHOR_FRONT	0xfff

#if USE_PROTOTYPES
static spell_t *iskeyword(CHAR *word, long anchor, ELVBOOL indent);
static spell_t *scankeyword(CHAR **refp, long anchor, ELVBOOL indent);
static void addkeyword(CHAR *word, _char_ font, int anchor, _CHAR_ flags);
static DMINFO *init(WINDOW win);
static void term(DMINFO *info);
static MARK setup(WINDOW win, MARK top, long cursor, MARK bottom, DMINFO *info);
static MARK image(WINDOW w, MARK line, DMINFO *info, void (*draw)(CHAR *p, long qty, _char_ font, long offset));
static CHAR *tagatcursor(WINDOW win, MARK cursor);
static MARK tagload(CHAR *tagname, MARK from);
#endif


/* This array stores the fonts to be used with the TOKENTYPES, above.  It is
 * initialized by the setup() function each time the screen is redrawn,
 * to reflect the values of the "font" options above.
 */
static char cfont[PUNCT + 1];
static char font_keyword;
static SINFO *sinfo;

/* This macro computes a hash value for a word, which is used for looking
 * the word up in the SINFO.keyword[] table.  The word is known to be at least
 * one character long and terminated with a '\0', so word[1] is guaranteed to
 * be valid and consistent.
 */
#define KWHASH(word)	(((word)[0] & 0x1f) ^ (((word)[1] & 0x03) << 5))


/* This function scans text from a buffer, to determine whether it is a keyword.
 * If it is a keyword, then its spell_t structure is returned; else NULL.
 * Either way, refp is advanced past the end of the scanned text.
 */
static spell_t *scankeyword(refp, colplusone, indent)
	CHAR	**refp;	/* reference to a scanning variable */
	long	colplusone;/* current column (1-based), or 0 to ignore column */
	ELVBOOL	indent;	/* preceded only by indentation whitespace? */
{
	spell_t *node;
	int	anchor;

	/* look it up, being careful about case sensitivity */
	for (node = sinfo->keyword; node && *refp && **refp; scannext(refp))
	{
		/* if this is a prefix keyword regardless of the next char, or
		 * a normal keyword and the next char isn't an "inword" char,
		 * then we've found a match.
		 */
		if (SPELL_IS_GOOD(node)
		 && (!isinword(sinfo, **refp) || wordprefix(node)))
		{
			/* if anchored, check the column number */
			if (colplusone != 0)
			{
				anchor =  wordanchor(node);
				switch (anchor)
				{
				  case ANCHOR_NONE:
					/* never a problem */
					break;

				  case ANCHOR_FRONT:
					/* any leading whitespace is okay, otherwise reject */
					if (!indent)
					{
						goto Continue;
					}
					break;

				  default:
					/* must be in a specific column */
					if (anchor != colplusone)
					{
						goto Continue;
					}
				}
			}

			/* No problems!  This is it! */
			break;
		}

		/* Still looking - check the next character */
Continue:
		if (sinfo->ignorecase)
			node = spellletter(node, elvtolower(**refp));
		else
			node = spellletter(node, **refp);
	}
	if (!SPELL_IS_GOOD(node))
		return NULL;

	/* return the keyword */
	return node;
}

/* This function returns a pointer to the hashed keyword description if the
 * given word is in fact a keyword.  Otherwise it returns NULL.
 */
static spell_t *iskeyword(word, colplusone, indent)
	CHAR	*word;	/* pointer to word */
	long	colplusone;/* current column (1-based), or 0 to ignore column */
	ELVBOOL	indent;	/* preceded only by indentation whitespace? */
{
	spell_t *node;
	int	anchor;

	/* look it up, being careful about case sensitivity */
	for (node = sinfo->keyword; node && *word; word++)
	{
		if (sinfo->ignorecase)
			node = spellletter(node, elvtolower(*word));
		else
			node = spellletter(node, *word);
	}
	if (!SPELL_IS_GOOD(node))
		return NULL;

	/* if anchored, check the column number */
	if (colplusone != 0)
	{
		anchor =  wordanchor(node);
		switch (anchor)
		{
		  case ANCHOR_NONE:
		  	/* never a problem */
		  	break;

		  case ANCHOR_FRONT:
			/* any leading whitespace is okay, otherwise reject */
			if (!indent)
			{
				return NULL;
			}
			break;

		  default:
		  	/* must be in a specific column */
		  	if (anchor != colplusone)
			{
		  		return NULL;
		  	}
		  }
	}

	/* return the keyword */
	return node;
}


static void addkeyword(word, font, anchor, flags)
	CHAR	*word;		/* a keyword */
	_char_	font;		/* font, or '\0' for default */
	int	anchor;		/* columns, or ANCHOR_FRONT, or ANCHOR_NONE */
	_CHAR_	flags;		/* ISCOMMENT|ISKEYWORD|USEREGEXP|USEREGSUB|OPERATOR */
{
	spell_t	*keyword;	/* spell node of the keyword */
	CHAR	last;

	/* if the last character isn't an "inword" character, then assume it
	 * should be a "prefix" word.
	 */
	last = word[CHARlen(word) - 1];
	if (!isinword(sinfo, last))
		flags |= ISPREFIX;
	if (isbeforeregsub(sinfo, last))
		flags |= USEREGEXP|USEREGSUB;
	else if (isbeforeregexp(sinfo, last))
		flags |= USEREGEXP;

	/* see if the keyword is already in the list */
	keyword = iskeyword(word, 0L, ElvTrue);
	if (!keyword)
	{
		sinfo->keyword = spelladdword(sinfo->keyword, word, 0L);
		keyword = iskeyword(word, 0L, ElvTrue);
	}

	/* set the word's attributes */
	if (font)
		wordsetfont(keyword, font);
	if (anchor != ANCHOR_NONE)
		wordsetanchor(keyword, anchor);
	wordflags(keyword) |= flags;
}



/* This global function returns the prepchar of a window, or '\0' if the
 * window isn't in the syntax display mode, or the syntax doesn't use a
 * preprocessor.
 */
CHAR dmspreprocessor(win)
	WINDOW	win;	/* window to be checked */
{
	if (win->md != &dmsyntax)
		return '\0';
	else
		return ((SINFO *)win->mi)->preprocessor;
}


#ifdef FEATURE_SMARTARGS
/* This global function implements the smartargs option.  It should be called
 * immediately after a character has been inserted.  If the character appears
 * to mark the start of function arguments, then this temporarily inserts
 * the formal args after the cursor, as a hint to the user.
 */
void dmssmartargs(win)
	WINDOW	win;	/* window to be checked */
{
	CHAR	*cp, *build, buf[100];
	CHAR	*calcarg[2];
	long	cursoff;
	int	i;
	spell_t	*kw;

	/* if obviously not a function invocation, then just return NULL */
	if (!o_smartargs(markbuffer(win->cursor)) /* smartargs isn't set */
	 || win->md != &dmsyntax		  /* not in syntax mode */
	 || win->state->cursor != win->cursor)	  /* not in main buffer */
		return;

	/* we'll use this window's syntax */
	sinfo = (SINFO *)win->mi;

	/* verify that we're at the start of a function's args */
	scanalloc(&cp, win->cursor);
	if (!scanprev(&cp) || *cp != sinfo->function)
	{
		scanfree(&cp);
		/* not after "function" character */
		return;
	}

	/* skip whitespace between name & function char */
	do
	{
		if (!scanprev(&cp))
		{
			/* no function name before "function" character */
			scanfree(&cp);
			return;
		}
	} while (elvspace(*cp));

	/* collect the chars of the function name */
	build = &buf[QTY(buf)];
	*--build = '\0';
	while (build > buf && isinword(sinfo, *cp))
	{
		*--build = *cp;
		if (!scanprev(&cp))
			break;
	}
	scanfree(&cp);
	if (!isstartword(sinfo, *build))
		/* doesn't look like a name */
		return;

	/* keywords such as while() look like functions, but should never
	 * have a tag.  Even if they do, this is likely to be more annoying
	 * than useful, so skip it.
	 */
	kw = iskeyword(build, 0, ElvTrue);
	if (kw && wordkeyword(kw))
		return;

	/* build a shell command line that returns the function's definition,
	 * and then run that shell command.
	 */
	calcarg[0] = build;
	calcarg[1] = NULL;
	cp = calculate(toCHAR("shell(\"ref -cd \"$1)"), calcarg, CALC_ALL);
	if (!cp)
		/* calc statement failed */
		return;

	/* Scan for the function name in the returned string.  Note that we're
	 * sloppy about this -- we don't ensure that word boundaries match. */
	i = CHARlen(build);
	while (*cp && CHARncmp(cp, build, i))
		cp++;
	if (!*cp)
		/* function name not found in returned string */
		return;

	/* Skip past the '(' after the name */
	for (cp += i; *cp && *cp != sinfo->function; cp++)
	{
	}
	if (!*cp)
		/* no '(' found after the function name */
		return;
	cp++;

	/* count the length of the remainder of the line */
	for (i = 0; cp[i] && cp[i] != '\n' && cp[i] != '\t'; i++)
	{
	}

	/* Insert that portion of the returned string into the edit buffer
	 * AFTER the cursor.  As the user continues to type more text, this
	 * argument hint will be overwritten.
	 */
	if (markoffset(win->cursor) < markoffset(win->state->bottom))
		bufreplace(win->cursor, win->state->bottom, NULL, 0);
	cursoff = markoffset(win->cursor);
	bufreplace(win->cursor, win->cursor, cp, i);
	marksetoffset(win->state->bottom, cursoff + i);
	marksetoffset(win->cursor, cursoff);
}
#endif /* FEATURE_SMARTARGS */


/* This global function checks whether a given word is a keyword in the current
 * language.  If the window isn't in "syntax" mode then it always returns
 * ElvFalse.
 */
ELVBOOL dmskeyword(win, word)
	WINDOW	win;		/* window to be checked */
	CHAR	*word;		/* the word to look up */
{
	spell_t *kw;		/* info about a possible keyword */
#if 0
	int	nupper, nlower, i;
#endif

	/* if not in syntax display mode, then just return ElvFalse. */
	if (win->md != &dmsyntax)
		return ElvFalse;

	/* first check real keywords */
	sinfo = (SINFO *)win->mi;
	kw = iskeyword(word, 0L, ElvTrue);
	if (kw != NULL && wordkeyword(kw))
		return ElvTrue;

#if 0
	/* Check for "other" words */
	if (sinfo->initialpunct && !elvalnum(word[0]))
		return ElvTrue;
	for (i = nupper = nlower = 0; word[i]; i++)
		if (elvupper(word[i]))
			nupper++;
		else if (elvlower(word[i]))
			nlower++;
	if (sinfo->finalt && i > 2 && !CHARcmp(word + i - 2, toCHAR("_t")))
		return ElvTrue;
	if (sinfo->allcaps && nlower == 0 && nupper > 0)
		return ElvTrue;
	if (sinfo->initialcaps && nlower > 0 && elvupper(word[0]))
		return ElvTrue;
	if (sinfo->mixedcaps && nlower > 0 && nupper > 0)
		return ElvTrue;
#endif

	/* I guess not */
	return ElvFalse;
}

/* start the mode, and allocate dminfo */
static DMINFO *init(win)
	WINDOW	win;
{
	char	*str, *path;
	CHAR	*cp, **values, dummy[1];
	spell_t	*kw;
	int	i, j, flags;

	/* if this is the first-ever time a window has been initialized to
	 * this mode, then we have some extra work to do...
	 */
	if (!dmsyntax.mark2col)
	{
		/* Inherit some functions from normal mode. */
		(*dmnormal.init)(win);
		dmsyntax.mark2col = dmnormal.mark2col;
		dmsyntax.move = dmnormal.move;
		dmsyntax.wordmove = dmnormal.wordmove;
		dmsyntax.header = dmnormal.header;
		dmsyntax.indent = dmnormal.indent; /* !!! really a good idea? */
		dmsyntax.tagnext = dmnormal.tagnext;

		/* initialize the mode's global options */
		str = getenv("INCLUDE");
#ifdef OSINCLUDEPATH
		if (!str)
			str = OSINCLUDEPATH;
#endif
		optpreset(o_includepath, toCHAR(str), OPT_HIDE);

		/* locate the default fonts */
		cfont[COMMENT] =
		cfont[COMMENT2] = colorfind(toCHAR("comment"));
		cfont[STRING0] = cfont[STRING1] = colorfind(toCHAR("string"));
		cfont[CHARACTER] = colorfind(toCHAR("char"));
		cfont[REGEXP] = colorfind(toCHAR("regexp"));
		cfont[REGSUB] = colorfind(toCHAR("regsub"));
		font_keyword =
		cfont[KEYWORD] = colorfind(toCHAR("keyword"));
		cfont[DOC] = colorfind(toCHAR("doc"));
		cfont[DOCMARKUP] = colorfind(toCHAR("docmarkup"));
		cfont[DOCINDENT] = colorfind(toCHAR("docindent"));
		cfont[FUNCTION] = colorfind(toCHAR("function"));
		cfont[VARIABLE] = 0; /* set to sinfo->mostly in setup() */
		cfont[NUMBER] = colorfind(toCHAR("number"));
		cfont[PREP] =
		cfont[PREPWORD] = colorfind(toCHAR("prep"));
		cfont[PREPQUOTE] = colorfind(toCHAR("prepquote"));
		cfont[OTHER] = colorfind(toCHAR("other"));
		cfont[PUNCT] = 0; /* generic font for generic chars */

		/* set some defaults */
		colorset(cfont[COMMENT], toCHAR("italic"), ElvFalse);
		colorset(cfont[CHARACTER], toCHAR("like string"), ElvFalse);
		colorset(cfont[REGEXP], toCHAR("like string"), ElvFalse);
		colorset(cfont[REGSUB], toCHAR("like regexp"), ElvFalse);
		colorset(cfont[KEYWORD], toCHAR("bold"), ElvFalse);
		colorset(cfont[PREP], toCHAR("like keyword"), ElvFalse);
		colorset(cfont[PREPQUOTE], toCHAR("like string"), ElvFalse);
		colorset(cfont[OTHER], toCHAR("like keyword"), ElvFalse);
		colorset(cfont[DOC], toCHAR("proportional"), ElvFalse);
		colorset(cfont[DOCMARKUP], toCHAR("like doc bold"), ElvFalse);
		colorset(cfont[DOCINDENT], toCHAR("like doc fixed"), ElvFalse);

		/* if no real window, then we're done! */
		if (!win)
			return NULL;
	}

	/* if possible, reuse an existing SINFO structure */
#ifdef FEATURE_CACHEDESC
	sinfo = (SINFO *)descr_recall(win, SYNTAX_FILE);
	if (!sinfo)
#endif
	{
		/* allocate a SINFO structure for this window */
		sinfo = (SINFO *)safealloc(1, sizeof(SINFO));

		/* initialize the wordbits[] array to allow letters and digits */
		for (i = 0; i < QTY(sinfo->wordbits); i++)
		{
			sinfo->wordbits[i] = (elvalnum(i) ? (STARTWORD|INWORD) : 0);
		}

		/* set the "mostly" font code to "variable", for now */
		sinfo->mostly = colorfind(toCHAR("variable"));

		/* the default string quote is a backslash */
		sinfo->backslash = '\\';

		/* locate the description in the "elvis.syn" file */
		if (descr_open(win, SYNTAX_FILE))
		{
			while ((values = descr_line()) != NULL)
			{
				str = tochar8(values[0]);
				if (!strcmp(str, "keyword"))
				{
					for (i = 1; values[i]; i++)
					{
						addkeyword(values[i], '\0', ANCHOR_NONE, ISKEYWORD);
					}
				}
				else if (!strcmp(str, "font") && values[1])
				{
					for (i = 2; values[i]; i++)
					{
						addkeyword(values[i], colorfind(values[1]), ANCHOR_NONE, ISKEYWORD);
					}
				}
				else if (!strcmp(str, "prefix") && values[1])
				{
					for (i = 1; values[i]; i++)
					{
						addkeyword(values[i], '\0', ANCHOR_NONE, ISKEYWORD|ISPREFIX);
					}
				}
				else if (!strcmp(str, "anchor") && values[1])
				{
					if (values[1][0] == '^')
						j = ANCHOR_FRONT;
					else
						j = atoi(tochar8(values[1]));
					if (j > 0 && j <= ANCHOR_FRONT)
					{
						for (i = 2; values[i]; i++)
						{
							addkeyword(values[i], '\0', j, ISKEYWORD);
						}
					}
				}
				else if (!strcmp(str, "comment"))
				{
					for (i = 1; values[i]; i++)
					{
						if (values[i + 1] && !elvalnum(*values[i]))
						{
							CHARncpy(sinfo->combegin, values[i], 2);
							CHARncpy(sinfo->comend, values[++i], 2);
						}
						else
						{
							CHARncpy(sinfo->comment, values[1], 2);
							addkeyword(values[i], '\0', 0, ISCOMMENT|ISKEYWORD);
						}
					}
				}
				else if (!strcmp(str, "useregexp")
				      || !strcmp(str, "useregsub")
				      || !strcmp(str, "operator"))
				{
					if (!strcmp(str, "useregexp"))
						flags = USEREGEXP;
					else if (!strcmp(str, "useregsub"))
						flags = USEREGEXP|USEREGSUB;
					else
						flags = OPERATOR;

					for (i = 1; values[i]; i++)
					{
						kw = iskeyword(values[i], 0L, ElvTrue);
						if (kw)
						{
							wordflags(kw) |= flags;
						}
						else if (elvalpha(values[i][0]))
						{
							addkeyword(values[i], '\0', ANCHOR_NONE, ISKEYWORD|flags);
						}
						else /* character list */
						{
							for (j = 0; values[i][j]; j++)
							{
								sinfo->wordbits[values[i][j]] |= flags;
							}
						}

						/* if first "operator" keyword
						 * then remember it for use in
						 * tag searches.
						 */
						if (!sinfo->opkeyword
						  && flags == OPERATOR
						  && elvalpha(values[i][0]))
						{
							sinfo->opkeyword = CHARdup(values[i]);
						}
					}
				}
				else if (!strcmp(str, "string"))
				{
				        long li = 0;
				        /* find item */
				        if( sinfo->strbegin[li] ) 
				          /* already used */
				          li = 1;

					if (values[1] && values[2])
					{
						sinfo->strbegin[li] = *values[1];
						sinfo->strend[li] = *values[2];
					}
					else if (values[1] && values[1][1])
					{
						sinfo->strbegin[li] = values[1][0];
						sinfo->strend[li] = values[1][1];
					}
					else if (values[1])
					{
						sinfo->strbegin[li] =
						sinfo->strend[li] = *values[1];
					}
				}
				else if (!strcmp(str, "backslash"))
				{
					if (!values[1] || elvalpha(values[1][0]))
						sinfo->backslash = '\0';
					else
						sinfo->backslash = values[1][0];
				}
				else if (!strcmp(str, "strnewline"))
				{
					str = tochar8(values[1]);
					if (!values[1] || !strcmp(str, "backslash"))
						sinfo->strnewline = ElvFalse;
					else if (!strcmp(str, "allowed"))
						sinfo->strnewline = ElvTrue;
					else if (!strcmp(str, "indent"))
						sinfo->strnewline = sinfo->strnoindent = ElvTrue;
					else if (!strcmp(str, "empty"))
						sinfo->strnewline = sinfo->strnoempty = ElvTrue;
					/* else unknown newline type */
				}
				else if (!strcmp(str, "character"))
				{
					if (values[1] && values[2])
					{
						sinfo->charbegin = *values[1];
						sinfo->charend = *values[2];
					}
					else if (values[1] && values[1][1])
					{
						sinfo->charbegin = values[1][0];
						sinfo->charend = values[1][1];
					}
					else if (values[1])
					{
						sinfo->charbegin =
						sinfo->charend = *values[1];
					}
				}
				else if (!strcmp(str, "regexp"))
				{
					for (i = 1; values[i]; i++)
					{
						for (j = 0; values[i][j]; j++)
						{
							sinfo->wordbits[values[i][j]] |= DELIMREGEXP;
						}
					}
				}
				else if (!strcmp(str, "preprocessor"))
				{
					if (values[1])
						sinfo->preprocessor = *values[1];
				}
				else if (!strcmp(str, "prepquote"))
				{
					if (values[1] && values[2])
					{
						sinfo->pqbegin = *values[1];
						sinfo->pqend = *values[2];
					}
					else if (values[1] && values[1][1])
					{
						sinfo->pqbegin = values[1][0];
						sinfo->pqend = values[1][1];
					}
					else if (values[1])
					{
						sinfo->pqbegin =
						sinfo->pqend = *values[1];
					}
				}
				else if (!strcmp(str, "function"))
				{
					if (values[1])
						sinfo->function = *values[1];
				}
				else if (!strcmp(str, "other"))
				{
					for (i = 1; values[i]; i++)
					{
						str = tochar8(values[i]);
						if (!strcmp(str, "allcaps"))
							sinfo->allcaps = (ELVBOOL)!sinfo->allcaps;
						else if (!strcmp(str, "initialcaps"))
							sinfo->initialcaps = (ELVBOOL)!sinfo->initialcaps;
						else if (!strcmp(str, "mixedcaps"))
							sinfo->mixedcaps = (ELVBOOL)!sinfo->mixedcaps;
						else if (!strcmp(str, "final_t"))
							sinfo->finalt = (ELVBOOL)!sinfo->finalt;
						else if (!strcmp(str, "initialpunct"))
							sinfo->initialpunct = (ELVBOOL)!sinfo->initialpunct;
						/* else unknown type */
					}
				}
				else if (!strcmp(str, "startword"))
				{
					for (i = 1; values[i]; i++)
					{
						for (j = 0; values[i][j]; j++)
						{
							sinfo->wordbits[values[i][j]] |= STARTWORD;
						}
					}
				}
				else if (!strcmp(str, "inword"))
				{
					for (i = 1; values[i]; i++)
					{
						for (j = 0; values[i][j]; j++)
						{
							sinfo->wordbits[values[i][j]] |= INWORD;
						}
					}
				}
				else if (!strcmp(str, "ignorecase"))
				{
					if (values[1])
						sinfo->ignorecase = calctrue(values[1]);
					else
						sinfo->ignorecase = ElvTrue;
				}
				else if (!strcmp(str, "color") && values[1] && values[2])
				{
					i = colorfind(values[1]);
					colorset(i, values[2], ElvFalse);
				}
				else if (!strcmp(str, "set") && values[1])
				{
					/* merge this "set" with any previous
					 * "set" arguments.
					 */
					if (sinfo->setargs)
					{
						cp = (CHAR *)safealloc(CHARlen(sinfo->setargs) + CHARlen(values[1]) + 2, sizeof(CHAR));
						CHARcpy(cp, sinfo->setargs);
						CHARcat(cp, toCHAR(" "));
						CHARcat(cp, values[1]);
						safefree(sinfo->setargs);
						sinfo->setargs = cp;
					}
					else
					{
						sinfo->setargs = CHARdup(values[1]);
					}
				}
				else if (!strcmp(str, "mostly") && values[1])
				{
					/* change the "mostly" font */
					sinfo->mostly = colorfind(values[1]);
				}
				else if (!strcmp(str, "documentation") && values[1])
				{
					CHARncpy(sinfo->enddoc, values[1],
							QTY(sinfo->enddoc) - 1);
				}
				/* else unknown attribute, or missing args */
			}

			/* If the string allows literal newlines, and the
			 * same character is used for both the opening
			 * quote and the closing quote, then we'll have a
			 * hard time detecting whether a quote character in
			 * the buffer marks the start of a string, or the
			 * end of one.  We'll assume that no string
			 * contains a newline followed by whitespace.
			 */
			if (sinfo->strnewline
			 && ( sinfo->strbegin[0] == sinfo->strend[0] ||
			      sinfo->strbegin[1] == sinfo->strend[1] 
			    )
			 && !sinfo->strnoindent)
			{
				sinfo->strnoempty = ElvTrue;
			}

			/* close the description file */
			descr_close((void *)sinfo);

			/* if taglibrary is set, then search for library tags */
			if (o_tagkind)
			{
				/* for each "tags" file in tagpath... */
				for (path = tochar8(o_tags);
				     (str = iopath(path, "tags", ElvTrue)) != NULL;
				     path = NULL)
				{
					/* add its tags to the keywords */
					sinfo->keyword = telibrary(str, sinfo->keyword, sinfo->ignorecase, toCHAR("kind"));
				}
			}
			if (o_taglibrary)
			{
				/* for each "tags" file in elvispath... */
				for (path = tochar8(o_elvispath);
				     (str = iopath(path, "tags", ElvTrue)) != NULL;
				     path = NULL)
				{
					/* add its tags to the keywords */
					sinfo->keyword = telibrary(str, sinfo->keyword, sinfo->ignorecase, toCHAR("lib"));
				}
			}
		}
	}

	/* set options for this language, if any */
	if (sinfo->setargs)
	{
		/* must use a copy of the string, since optset() alters it */
		cp = CHARdup(sinfo->setargs);
		(void)optset(ElvTrue, cp, dummy, 0);
		safefree(cp);
	}

	/* return the window's SINFO structure */
	return (DMINFO *)sinfo;
}


/* end the mode, and free the modeinfo */
static void term(info)
	DMINFO	*info;	/* window-specific information about mode */
{
#ifdef FEATURE_CACHEDESC
	/* Don't delete it -- it is still used in the descr cache */
#else
	/* Okay, delete it */
	SINFO	*si = (SINFO *)info;

	/* free the dictionary */
	spellfree(si->keyword);

	/* free the "operator" keyword, if any */
	if (si->opkeyword)
		safefree(si->opkeyword);

	/* free the sinfo struct itself */
	safefree(si);
#endif
}


/* Choose a line to appear at the top of the screen, and return its mark.
 * Also, initialize the info for the next line.
 */
static MARK setup(win, top, cursor, bottom, info)
	WINDOW	win;	/* window to be updated */
	MARK	top;	/* where the image drawing began last time */
	long	cursor;	/* cursor's offset into buffer */
	MARK	bottom;	/* where the image drawing ended last time */
	DMINFO	*info;	/* window-specific information about mode */
{
	MARK	newtop;
	ELVBOOL	oddquotes;
	CHAR	*cp, *enddoc;
	CHAR	following;
	ELVBOOL	knowstr, knowcom;


	/* Use window's info.  Variables use this language's "mostly" font */
	sinfo = (SINFO *)info;
	cfont[VARIABLE] = sinfo->mostly;

	/* use the normal mode's setup function to choose the screen top */
	newtop = (*dmnormal.setup)(win, top, cursor, bottom, info);
	if (!newtop || markoffset(newtop) >= o_bufchars(markbuffer(newtop)))
		return newtop;

	/* Does this language support some form of embedded documentation? */
	if (*sinfo->enddoc)
	{
		/* The top line could be a continuation of DOC token.  To
		 * find out, scan backward for a line that starts with the
		 * DOC character.  Exception: If the top line itself starts
		 * with the DOC character then it is definitely a
		 * continuation (even if it is just "=cut")
		 */
		if (scanchar(newtop) == *sinfo->enddoc)
		{
			sinfo->token = STRING0;
			return newtop;
		}

		/* Scan backward for a DOC markup, or strong evidence that
		 * we're in code.  Strong evidence means any punctuation
		 * character other than ", ', or (.
		 */
		scanalloc(&cp, newtop);
		do
		{
			/* skip if "following" isn't at start of line */
			following = *cp;
			scanprev(&cp);
			if (cp && *cp != '\n')
				continue;

			/* if DOC directive, then we have our answer */
			if (following == *sinfo->enddoc)
			{
				/* The answer is PUNCT if enddoc, or DOC
				 * otherwise.  The only tricky part is that
				 * if we hit the top of the buffer, then cp
				 * became NULL.  In this case, assume DOC.
				 */
				if (!cp)
				{
					sinfo->token = DOC;
					scanfree(&cp);
					return newtop;
				}
				scannext(&cp);
				for (enddoc = sinfo->enddoc;
				     *cp == *enddoc;
				     scannext(&cp), enddoc++)
				{
				}
				if (*enddoc || (cp && elvalnum(*cp)))
					sinfo->token = DOC;
				else
					sinfo->token = PUNCT;
				scanfree(&cp);
				return newtop;
			}
		} while (cp && (*cp != '\n' || following != sinfo->comment[0]));
		/* Either we hit the top without finding a DOC marker, or
		 * we found strong evidence that we're in normal code.
		 * Either way, fall through to normal COMMENT/STRINGn test.
		 */
		 scanfree(&cp);
	}

	/* The top line could be a continuation of a COMMENT or STRINGn.
	 * (Other tokens can't span a newline, so we can ignore them.)
	 * Scan backward for clues about comments or strings.
	 *
	 * This isn't perfect.  To do the job perfectly, we'd need to start
	 * at the top of the buffer, and scan *forward* to the top of the
	 * screen, but that could take far too long.
	 */
	following = *scanalloc(&cp, newtop);
	if ( following == sinfo->strend[0] ||
	     following == sinfo->strend[1]
   	   )
		following = '\0';
	oddquotes = ElvFalse;
	knowstr = (ELVBOOL)(
            sinfo->strbegin[0] == '\0' || (cp && *cp == sinfo->strbegin[0]) ||
            sinfo->strbegin[1] == '\0' || (cp && *cp == sinfo->strbegin[1]));
	knowcom = (ELVBOOL)!sinfo->combegin[0];
	sinfo->token = PUNCT;
	for (; scanprev(&cp) && (!knowstr || !knowcom); following = *cp)
	{
		/* detect string clues */
		if ( ( sinfo->strend[0] || sinfo->strend[1] )
                     && *cp
                     && *cp != sinfo->backslash
                     && ( following == sinfo->strend[0] ||
                          following == sinfo->strend[1] )
                     && !knowstr
                   )
		{
			/* a " which isn't preceded by a \ toggles the quote
			 * state
			 */
			oddquotes = (ELVBOOL)!oddquotes;
		}
		else if ((sinfo->strend[0] || sinfo->strend[1])
		      && !sinfo->strnewline
                      && *cp
		      && *cp != sinfo->backslash
		      && following == '\n')
		{
			/* some strings can't span a newline unless preceded
			 * by a backslash
			 */
			knowstr = ElvTrue;
		}
		else if ( ( sinfo->strend[0] || sinfo->strend[1] )
			  && ((sinfo->strnoindent
			 	&& *cp == '\n'
			 	&& (following == ' ' || following == '\t'))
			     || (sinfo->strnewline
			        && ( *cp == sinfo->strbegin[0] ||
			             *cp == sinfo->strbegin[1] )
			        && following == ';')
			     || (sinfo->strnoempty
			 	&& *cp == '\n'
			 	&& following == '\n')))
		{
			/* some strings don't allow whitespace after a newline,
			 * or two consecutive newlines, or an opening quote
			 * which is immediately followed by a newline.
			 */
			knowstr = ElvTrue;
		}

		/* detect comment clues */
		if (sinfo->combegin[0]
		      && *cp == sinfo->combegin[0]
		      && (!sinfo->combegin[1] || following == sinfo->combegin[1]))
		{
			/* We'll assume that slash-asterisk always starts a
			 * comment (i.e., that it never occurs inside a string).
			 * However, some C++ programmers like to begin comments
			 * with slash-slash and a bunch of asterisks; we need
			 * to watch out for that.
			 */
			knowstr = knowcom = ElvTrue;
			if (*cp != sinfo->comment[1] || !scanprev(&cp) || *cp != sinfo->comment[0])
			{
				sinfo->token = COMMENT;
				break;
			}
		}
		else if (sinfo->comend[0]
		      && *cp == sinfo->comend[0]
		      && (!sinfo->comend[1] || following == sinfo->comend[1]))
		{
			/* We'll assume that asterisk-slash always ends a
			 * comment.  (I.e., that it never occurs inside a
			 * string.)
			 */
			knowstr = knowcom = ElvTrue;
		}
		else if (sinfo->comment[0]
		      && *cp == sinfo->comment[0]
		      && (!sinfo->comment[1] || following == sinfo->comment[1]))
		{
			/* We'll assume that slash-slash always indicates a
			 * single-line comment.  (I.e., that it never occurs
			 * in a string or slash-asterisk type comment.
			 */
			knowstr = knowcom = ElvTrue;
		}
	}
	scanfree(&cp);

	/* If it isn't a comment, then it might be a string... check oddquotes */
	if (sinfo->token == PUNCT && oddquotes)
	{
		sinfo->token = STRING0;
	}

	return newtop;
}

/* generate the image of a line, and return the mark of the next line */
static MARK image(w, line, info, draw)
	WINDOW	w;		/* window where drawing will take place */
	MARK	line;		/* line to be drawn */
	DMINFO	*info;		/* window-specific information about mode */
	void	(*draw)P_((CHAR *p, long qty, _char_ font, long offset));
				/* function for drawing a single character */
{
	int	col;
	CHAR	*cp;
	CHAR	tmpchar;
	CHAR	regexpdelim;	/* delimiter for current regexp */
	long	offset;
	static MARKBUF tmp;
	CHAR	undec[40];	/* characters of undecided font */
	CHAR	*up;		/* pointer used for scanning chars */
	CHAR	prev, prev2;	/* the preceding two characters */
	ELVBOOL	quote;		/* ElvTrue after a backslash in STRINGn or CHARACTER */
	int	upper, lower;	/* counts letters of each case */
	ELVBOOL	indent;
	spell_t	*kp;		/* pointer to a keyword */
	ELVBOOL	expectregexp;	/* allow a regular expression to start next */
	ELVBOOL	expectregsub;	/* allow substitution text to follow regexp */
	ELVBOOL	expectprepq;	/* allow preprocessor quotes */
	int	i;

#ifdef FEATURE_FOLD
	/* if this line is folded, then draw using the "normal" image() func */
	if (o_folding(w) && foldmark(line, ElvTrue))
	{
		return (*dmnormal.image)(w, line, NULL, draw);
	}
#endif

	/* initially, we'll assume we continue the font of the previous line */
	sinfo = (SINFO *)info;
	quote = ElvFalse;
	indent = (ELVBOOL)(sinfo->token != DOC);
	expectregexp = (ELVBOOL)(sinfo->token == PUNCT);
	expectregsub = expectprepq = ElvFalse;

	/* this is just to silence a compiler warning */
	regexpdelim = '/';

	/* Detect DOC control lines.  The documentation itself (between control
	 * lines) can be handled by the normal formatter, below.
	 */
	scanalloc(&cp, line);
	if (*sinfo->enddoc && *cp == *sinfo->enddoc)
	{
		/* Output this line in DOC font.  Also compare to enddoc */
		offset = markoffset(line);
		up = sinfo->enddoc; 
		do
		{
			/* If char doesn't match enddoc, then set "up" to NULL
			 * just as a way to denote this.
			 */
			if (up)
			{
				if (*up != *cp)
					up = NULL;
				else
					up++;
			}

			/* Output this character */
			(*draw)(cp, 1, cfont[DOCMARKUP], offset);

			/* advance to next character */
			scannext(&cp);
			offset++;
		} while (cp && *cp != '\n');

		/* End the line */
		tmpchar = '\n';
		(*draw)(&tmpchar, 1, cfont[DOCMARKUP], offset);
		if (cp)
		{
			scannext(&cp);
			offset++;
		}

		/* next line will be in PUNCT or DOC mode, depending on whether
		 * this line matched enddoc or not.
		 */
		sinfo->token = (up && !*up) ? PUNCT : DOC;
		tmp = *line;
		tmp.offset = offset;
		scanfree(&cp);
		return &tmp;
	}

	/* if indented DOC, then use DOCINDENT instead */
	if (sinfo->token == DOC && *cp != '\n' && elvspace(*cp))
		sinfo->token = DOCINDENT;

	/* for each character in the line... */
	for (prev = ' ', col = 0, offset = markoffset(line);
	     cp && *cp != '\n' && (*cp != '\f' || markoffset(w->cursor) < o_bufchars(markbuffer(w->cursor)));
	     prev = *cp, offset++, scannext(&cp))
	{
		/* some characters are handled specially */
		if (*cp == '\t' && !o_list(w))
		{
			/* prepword can still be followed by prepquote */
			if (sinfo->token == PREPWORD)
				expectprepq = ElvTrue;

			/* tab ends any symbol */
			if (sinfo->token == KEYWORD || sinfo->token == FUNCTION
			   || sinfo->token == VARIABLE || sinfo->token == OTHER
			   || sinfo->token == NUMBER || sinfo->token == PREPWORD)
			{
				sinfo->token = PUNCT;
			}

			/* display the tab character as a bunch of spaces */
			tmpchar = ' ';
			i = opt_totab(o_tabstop(markbuffer(w->cursor)), col);
			(*draw)(&tmpchar, -i, indent ? 0 : cfont[sinfo->token], offset);
			col += i;
		}
		else if (*cp < ' ' || *cp == 127)
		{
			/* any control character ends any symbol */
			if (sinfo->token == KEYWORD || sinfo->token == FUNCTION
			   || sinfo->token == VARIABLE || sinfo->token == OTHER
			   || sinfo->token == NUMBER || sinfo->token == PREPWORD)
			{
				sinfo->token = PUNCT;
			}

			/* also ends indentation */
			indent = ElvFalse;

			/* control characters */
			col += dmnlistchars(*cp, offset, col, o_tabstop(markbuffer(line)), draw);
		}
#ifdef FEATURE_LISTCHARS
		else if (*cp == ' ')
		{
			/* prepword can still be followed by prepquote */
			if (sinfo->token == PREPWORD)
				expectprepq = ElvTrue;

			/* ending a keyword/function/variable? */
			if (sinfo->token == KEYWORD || sinfo->token == FUNCTION
			   || sinfo->token == VARIABLE || sinfo->token == OTHER
			   || sinfo->token == NUMBER || sinfo->token == PREPWORD)
			{
				sinfo->token = PUNCT;
			}

			/* scan ahead to see if this is trailing space */
			scandup(&up, &cp);
			for (i = 1; scannext(&up) && *up == ' '; i++)
			{
			}
			if (up && *up == '\n')
				up = NULL;
			scanfree(&up);               

			/* if trailing, and the cursor isn't at the end of it,
			 * and "list" is set, then show trail
			 */
			if (!up && o_list(w) && offset + i != markoffset(w->state->cursor))
			{
				col += dmnlistchars(' ', offset, i, NULL, draw);
				offset += i - 1; /* for-loop does "offset++" */
				scanseek(&cp, marktmp(tmp, markbuffer(line), offset));
			}
			else
			{
				int chunk;
				while (i > 0)
				{
					chunk = scanright(&cp);
					if (chunk > i)
						chunk = i;
					(*draw)(cp, chunk, cfont[sinfo->token], offset);
					col += chunk;
					offset += chunk;
					i -= chunk;
					scanseek(&cp, marktmp(tmp, markbuffer(line), offset));
				}
				scanprev(&cp);
				offset--;
			}
		}
#endif
		else if (sinfo->preprocessor && indent
			&& *cp == sinfo->preprocessor && sinfo->token == PUNCT)
		{
			/* output the '#' in prepfont */
			indent = ElvFalse;
			sinfo->token = PREP;
			(*draw)(cp, 1, cfont[PREP], offset);
			col++;
		}
		else /* normal printable character */
		{
			/* ending a keyword/function/variable? */
			if (!isinword(sinfo, *cp)
				&& (sinfo->token == FUNCTION
					|| sinfo->token == VARIABLE
					|| sinfo->token == NUMBER
					|| sinfo->token == OTHER))
			{
				sinfo->token = PUNCT;
			}

			/* premature end to a preprocessor directive? */
			if (elvpunct(*cp) && sinfo->token == PREP)
			{
				sinfo->token = PUNCT;
			}

			/* is it a keyword? */
			scandup(&up, &cp);
			if (sinfo->token == PUNCT
			 && (kp = scankeyword(&up, col + 1, indent)) != NULL)
			{
				/* It's a keyword.  Is it a comment keyword? */
				if (wordcomment(kp))
				{
					/* enter COMMENT2 mode */
					sinfo->token = COMMENT2;
					cfont[COMMENT2] = wordfont(kp);
					if (!cfont[COMMENT2])
						cfont[COMMENT2]=cfont[COMMENT];
				}
				else /* normal keyword */
				{
					/* choose a font */
					cfont[KEYWORD] = wordfont(kp);
					if (!cfont[KEYWORD])
						cfont[KEYWORD] = font_keyword;

					/* output the whole keyword now.  This
					 * is easier than setting a state, since
					 * we'd have a hard time knowing when
					 * the state ended, after "up" changes.
					 */
					scanfree(&up);
					for (; cp != up; scannext(&cp), offset++)
					{
						(*draw)(cp, 1, cfont[KEYWORD], offset);
						col++;
					}
					scanprev(&cp);
					offset--;

					/* remember if regex's are allowed */
					expectregexp = (ELVBOOL)wordbeforeregexp(kp);
					expectregsub = (ELVBOOL)wordbeforeregsub(kp);
					/* we don't dare complete the rest of
					 * the loop body, because a prefix-type
					 * keyword could immediately be followed
					 * by anything, including some things
					 * that are detected earlier in this
					 * loop.  So we continue from here.
					 */
					continue;
				}
			}
			scanfree(&up);

			/* starting a keyword/function/variable? */
			if (sinfo->token == PUNCT && isstartword(sinfo, *cp))
			{
				/* this isn't regexp */
				expectregexp = expectregsub = ElvFalse;

				/* store first letter of possible keyword */
				lower = upper = 0;
				undec[0] = *cp;
				if (elvlower(*cp))
					lower++;
				else if (elvupper(*cp))
					upper++;

				/* collect more letters of possible keyword */
				for (i = 1, prev2 = prev = '\0',
					scanalloc(&up, marktmp(tmp, markbuffer(line), offset + 1));
				     i < QTY(undec) - 2 && up && isinword(sinfo, *up);
				     prev2 = prev, prev = *up, i++, scannext(&up))
				{
					undec[i] = *up;
					if (elvlower(*up))
						lower++;
					else if (elvupper(*up))
						upper++;
				}
				undec[i] = '\0';

				/* continue on to the end of the word */
				for (;
				     up && isinword(sinfo, *up);
				     prev2 = prev, prev = *up, scannext(&up))
				{
					if (elvlower(*up))
						lower++;
					else if (elvupper(*up))
						upper++;
				}

#if 0
				/* skip any following whitespace */
				for (;
				     up && (*up == ' ' || *up == '\t');
				     scannext(&up))
				{
				}
#else
				/* allow following spaces, but not tabs.  This
				 * way, macro definitions won't be displayed
				 * like functions, in directives like this:
				 * #define HEIGHT	(TOP - BOTTOM)
				 */
				for (;
				     up && *up == ' ';
				     scannext(&up))
				{
				}
#endif

				/* is the word followed by a '(' ? */
				if (up && sinfo->function && *up == sinfo->function)
				{
					sinfo->token = FUNCTION;
				}
				else if (sinfo->finalt && prev2 == '_' && prev == 't')
				{
					sinfo->token = OTHER;
				}
				else if (sinfo->initialpunct && !elvalnum(undec[0]))
				{
					sinfo->token = OTHER;
				}
				else if (sinfo->allcaps && upper >= 2 && lower == 0)
				{
					sinfo->token = OTHER;
				}
				else if (sinfo->initialcaps && lower > 0 && elvupper(undec[0]))
				{
					sinfo->token = OTHER;
				}
				else if (sinfo->mixedcaps && lower > 0 && upper > 0)
				{
					sinfo->token = OTHER;
				}
				else if (i > 1 || elvalnum(*cp))
				{
					sinfo->token = elvdigit(*cp) ?
						NUMBER : VARIABLE;
				}
				/* else leave it set to PUNCT so we can
				 * recognize two-punctuation keywords.
				 */
				scanfree(&up);
			}
			else if (sinfo->token == PREP && !elvspace(*cp))
			{
				sinfo->token = PREPWORD;
			}
			else if (sinfo->token == PREPWORD && !elvalnum(*cp))
			{
				sinfo->token = PUNCT;
				expectprepq = ElvTrue;
			}

			/* start of preprocessor quote? */
			if (sinfo->token == PUNCT && expectprepq && *cp == sinfo->pqbegin)
			{
				sinfo->token = PREPQUOTE;
				expectprepq = ElvFalse;
			}

			/* start of a string? */
			if (sinfo->strbegin[0] && sinfo->token == PUNCT && *cp == sinfo->strbegin[0])
			{
				sinfo->token = STRING0;
				expectregexp = expectregsub = expectprepq = ElvFalse;

				/* make sure the initial quote character
				 * isn't going to be mistaken for the
				 * terminating quote character.
				 */
				quote = ElvTrue;
			}
			if (sinfo->strbegin[1] && sinfo->token == PUNCT && *cp == sinfo->strbegin[1])
			{
				sinfo->token = STRING1;
				expectregexp = expectregsub = expectprepq = ElvFalse;

				/* make sure the initial quote character
				 * isn't going to be mistaken for the
				 * terminating quote character.
				 */
				quote = ElvTrue;
			}

			/* start of a character literal? */
			if (sinfo->charbegin && sinfo->token == PUNCT && *cp == sinfo->charbegin)
			{
				sinfo->token = CHARACTER;
				expectregexp = expectregsub = ElvFalse;

				/* make sure the initial quote character
				 * isn't going to be mistaken for the
				 * terminating quote character.
				 */
				quote = ElvTrue;
			}

			/* start of a comment? */
			if (sinfo->token == PUNCT
				&& sinfo->combegin[0]
				&& *cp == sinfo->combegin[0]
				&& (!sinfo->combegin[1]
					|| scanchar(marktmp(tmp, markbuffer(line), offset + 1)) == sinfo->combegin[1]))
			{
				sinfo->token = COMMENT;
				expectregexp = expectregsub = ElvFalse;
			}

			/* start of a regular expression? */
			if (sinfo->token == PUNCT && expectregexp && isregexp(sinfo, *cp))
			{
				sinfo->token = REGEXP;
				expectregexp = ElvFalse;
				regexpdelim = *cp;

				/* make sure the initial quote character
				 * isn't going to be mistaken for the
				 * terminating quote character.
				 */
				quote = ElvTrue;
			}

			/* would a regexp be allowed after this char? */
			if (sinfo->token == PUNCT && !elvspace(*cp))
			{
				expectregexp = (ELVBOOL)isbeforeregexp(sinfo, *cp);
				expectregsub = (ELVBOOL)isbeforeregsub(sinfo, *cp);
			}

			/* any non-whitespace ends indent */
			if (!elvspace(*cp))
			{
				indent = ElvFalse;
			}

			/* draw the character */
			(*draw)(cp, 1, indent ? 0 : cfont[sinfo->token], offset);
			col++;

			/* end of a string? */
			if (((sinfo->token==STRING0 && *cp == sinfo->strend[0])
			  || (sinfo->token==STRING1 && *cp == sinfo->strend[1]))
			 && !quote)
			{
				sinfo->token = PUNCT;
			}

			/* end of a character? */
			if (sinfo->token == CHARACTER && *cp == sinfo->charend && !quote)
			{
				sinfo->token = PUNCT;
			}

			/* end of a comment? */
			if (sinfo->token == COMMENT
			 && (sinfo->comend[1]
				? (prev == sinfo->comend[0] && *cp == sinfo->comend[1])
				: *cp == sinfo->comend[0]))
			{
				sinfo->token = PUNCT;
			}

			/* end of a regexp or substitution text? */
			if ((sinfo->token == REGEXP || sinfo->token == REGSUB)
				&& *cp == regexpdelim && !quote)
			{
				if (expectregsub)
				{
					sinfo->token = REGSUB;
					quote = ElvTrue;
					expectregsub = ElvFalse;
				}
				else
				{
					sinfo->token = PUNCT;
				}
			}

			/* end of a prepquote? */
			if (sinfo->token == PREPQUOTE && *cp == sinfo->pqend)
			{
				sinfo->token = PUNCT;
			}

			/* in a STRINGn, CHARACTER, REGEXP, or REGSUB constant,
			 * backslash is used to quote the following character.
			 */
			if ((sinfo->token == STRING0 || sinfo->token == STRING1
			  || sinfo->token == CHARACTER
			  || sinfo->token == REGEXP || sinfo->token == REGSUB)
				&& *cp && *cp == sinfo->backslash && !quote)
			{
				quote = ElvTrue;
			}
			else
			{
				quote = ElvFalse;
			}
		}
	}

	/* end the line */
	if (o_list(w) && (!cp || *cp == '\n'))
	{
		col += dmnlistchars('\n', offset, col, o_tabstop(markbuffer(line)), draw);
	}
	tmpchar = (cp ? *cp : '\n');
	if (sinfo->token == DOC || sinfo->token == DOCINDENT)
		(*draw)(&tmpchar, 1, cfont[sinfo->token], offset);
	else
		(*draw)(&tmpchar, 1, 0, offset);
	if (cp)
	{
		offset++;
	}
	else
	{
		offset = o_bufchars(markbuffer(w->cursor));
	}

	/* Strings can span a newline if the newline is preceded by a
	 * backslash.  Old-style C comments can span a newline.  Embedded
	 * documentation can span long distances. Everything else ends here.
	 */
	if (sinfo->token == DOCINDENT)
		sinfo->token = DOC;
	else if (((sinfo->token != STRING0 && sinfo->token != STRING1)
		|| !(sinfo->strnewline || (prev && prev==sinfo->backslash)))
	  && sinfo->token != COMMENT && sinfo->token != DOC)
		sinfo->token = PUNCT;

	/* clean up & return the MARK of the next line */
	scanfree(&cp);
	return marktmp(tmp, markbuffer(w->cursor), offset);
}


/* This function considers the possibility that the cursor may be on a quoted
 * filename.  If so, it returns the name, with the quotes.  Otherwise it gets
 * the word at the cursor, and if that word is "operator" then it gets the
 * following operator characters as well.
 *
 * The return value is a dynamically-allocated string; the calling function
 * is responsible for freeing it when it is no longer required.
 */
static CHAR *tagatcursor(win, cursor)
	WINDOW win;	/* window, used for finding mode-dependent info */
	MARK cursor;	/* where the desired tag name can be found */
{
	CHAR	*ret;	/* return value */
	CHAR	*cp;	/* used for scanning */
	CHAR	endq;	/* end quote character */
	spell_t	*kw;
	MARKBUF	curscopy;	/* a copy of the cursor */
	MARK	word;		/* mark for the front of the word */


	/* initialization */
	sinfo = (SINFO *)win->mi;
	ret = NULL;

	/* search backward for first quote or whitespace */
	for (scanalloc(&cp, cursor);
	     cp && !elvspace(*cp) && *cp != sinfo->pqbegin && 
		*cp != sinfo->strbegin[0] && *cp != sinfo->strbegin[1];
	     scanprev(&cp))
	{
	}

	/* did we find something? */
	if (cp && !elvspace(*cp))
	{
		/* choose an end quote */
		if (*cp == sinfo->strbegin[0])
			endq = sinfo->strend[0];
		else if (*cp == sinfo->strbegin[1])
			endq = sinfo->strend[1];
		else
			endq = sinfo->pqend;

		/* Search forward for closing quote, collecting chars along
		 * the way.  Beware of whitespace.
		 */
		do
		{
			buildCHAR(&ret, *cp);
		} while (scannext(&cp) && !elvspace(*cp) && *cp != endq);
		if (cp && *cp == endq
                       && (!o_previoustag || CHARcmp(o_previoustag, ret + 1)))
		{
			buildCHAR(&ret, *cp);
			scanfree(&cp);
			return ret;
		}
	}

	/* cleanup */
	scanfree(&cp);
	if (ret)
		safefree(ret);

	/*------------------------------------------------------------------*
	 * If we get here, then it wasn't a filename.  Try to get operator. *
	 *------------------------------------------------------------------*/

	if (isoperator(sinfo, scanchar(cursor)))
	{
		/* if there is an "operator" keyword, use it */
		ret = NULL;
		for (cp = sinfo->opkeyword; cp && *cp; cp++)
			buildCHAR(&ret, *cp);

		/* find the first character of the operator itself */
		scanalloc(&cp, cursor);
		while (scanprev(&cp) && isoperator(sinfo, *cp))
		{
		}
		if (cp)
			scannext(&cp);
		else
		{
			/* oops!  we backed up past the top of the buffer! */
			curscopy = *cursor;
			curscopy.offset = 0L;
			scanseek(&cp, &curscopy);
		}

		/* add all operator character to the tag name */
		while (isoperator(sinfo, *cp))
		{
			buildCHAR(&ret, *cp);
			scannext(&cp);
		}
		scanfree(&cp);

		/* return the tag name */
		return ret;
	}

	/*------------------------------------------------------------------*
	 * If we get here, then it wasn't an operator.  Get word at cursor. *
	 *------------------------------------------------------------------*/

	/* find the ends of the word */
	curscopy = *cursor;
	word = wordatcursor(&curscopy, ElvFalse);

	/* if not on a word, then return NULL */
	if (!word)
		return NULL;

	/* copy the word into RAM */
	ret = bufmemory(word, &curscopy);

	/* is this an "operator"-type keyword? */
	if ((kw = iskeyword(ret, 0L, ElvTrue)) != NULL  && wordoperator(kw))
	{
		/* Try to extend the word to include any operator chars */
		for (scanalloc(&cp, &curscopy);
		     *cp && isoperator(sinfo, *cp);
		     scannext(&cp))
		{
		}
		if (*cp)
		{
			safefree(ret);
			ret = bufmemory(word, scanmark(&cp));
		}
		scanfree(&cp);
	}

	return ret;
}


/* This function checks the tagname.  If it is a quoted filename, then it
 * attempts to load the file.  Otherwise it calls dmnormal.tagload() to try
 * and load a normal tag.
 */
static MARK tagload(tagname, from)
	CHAR *tagname;	/* name of tag to move to */
	MARK from;	/* where we're coming from */
{
	char	*name;
	char	*tmp;
	unsigned len;
	DIRPERM	perms;
	BUFFER	buf;
 static	MARKBUF	retb;

	/* is it a quoted filename? */
	len = tagname ? CHARlen(tagname) : 0;
	if (len >= 3 && (*tagname == '"' || (sinfo && sinfo->pqbegin && *tagname == sinfo->pqbegin)))
	{
		/* make an unquoted, 8-bit version of the name */
		name = safedup(tochar8(tagname + 1));
		if ((*tagname == '"' && tagname[len - 1] == '"')
		 || (sinfo && sinfo->pqbegin && *tagname == sinfo->pqbegin && tagname[len - 1] == sinfo->pqend))
		{
			name[len - 2] = '\0';
		}

		/* if plain old quote character, then look for it first in
		 * the directory where the current file resides, and the
		 * current directory.
		 */
		if (*tagname == '"')
		{
			/* check the directory where the current file resides */
			if (o_filename(markbuffer(from)))
			{
				tmp = dirpath(dirdir(tochar8(o_filename(markbuffer(from)))), name);
				perms = dirperm(tmp);
				if (perms == DIR_READONLY || perms == DIR_READWRITE)
				{
					tmp = safedup(tmp);
					buf = bufload(NULL, tmp, ElvFalse);
					assert(buf != NULL);
					safefree(tmp);
					safefree(name);
					return marktmp(retb, buf, buf->docursor);
				}
			}

			/* check the current directory */
			perms = dirperm(name);
			if (perms == DIR_READONLY || perms == DIR_READWRITE)
			{
				buf = bufload(NULL, name, ElvFalse);
				assert(buf != NULL);
				safefree(name);
				return marktmp(retb, buf, buf->docursor);
			}
		}

		/* search through the include path */
		if (o_includepath)
		{
			tmp = iopath(tochar8(o_includepath), name, ElvFalse);
			if (tmp)
			{
				buf = bufload(NULL, tmp, ElvFalse);
				assert(buf != NULL);
				safefree(name);
				return marktmp(retb, buf, buf->docursor);
			}
		}

		/* Failed!  Complain, clean up, and return NULL */
		msg(MSG_ERROR, "[s]header file $1 not found", name);
		safefree(name);
		return NULL;
	}

	/* use dmnormal's tagload() function */
	return (*dmnormal.tagload)(tagname, from);
}


DISPMODE dmsyntax =
{
	"syntax",
	"generic syntax coloring",
	ElvTrue,/* can optimize */
	ElvTrue,/* can use normal wordwrap */
	0,	/* no window options */
	NULL,
	QTY(globdesc),
	globdesc,
	globval,
	init,
	term,
	NULL,	/* init() sets this to be identical to dmnormal's mark2col() */
	NULL,	/* init() sets this to be identical to dmnormal's move() */
	NULL,	/* init() sets this to be identical to dmnormal's moveword() */
	setup,
	image,
	NULL,	/* init() sets this to be identical to dmnormal's header() */
	NULL,	/* init() sets this to be identical to dmnormal's indent() */
	tagatcursor,
	tagload,
	NULL	/* init() sets this to be identical to dmnormal's tagnext() */
};
#endif /* DISPLAY_SYNTAX */
