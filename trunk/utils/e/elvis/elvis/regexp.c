/* regexp.c */

/* This file contains the code that compiles regular expressions and executes
 * them.  It supports the same syntax and features as vi's regular expression
 * code.  Specifically, the meta characters are:
 *	^	matches the beginning of a line
 *	$	matches the end of a line
 *	\<	matches the beginning of a word
 *	\>	matches the end of a word
 *	.	matches any single character
 *	[]	matches any character in a character class
 *	\@	matches the word at the cursor, if any
 *	\=	if searching, leaves the cursor here
 *	\(	delimits the start of a subexpression
 *	\)	delimits the end of a subexpression
 *	*	repeats the preceding 0 or more times 
 *	\+	repeats the preceding 1 or more times
 *	\?	repeats the preceding 0 or 1 times
 *	\{m\}	repeats the preceding m times
 *	\{m,\}	repeats the preceding m or more times
 *	\{m,n\}	repeats the preceding between m and n times
 *
 * The following match a single character.  They can be used as part of a
 * [] character list, or as separate subexpressions.
 *	\a	BEL, the bell character
 *	\b	BS, the backspace character
 *	\e	ESC, the escape character
 *	\f	FF, the formfeed character
 *	\n	(reserved for newline)
 *	\r	CR, the carriage return
 *	\t	TAB, the tab character
 *	\d	Any digit
 *	\D	Any non-digit
 *	\p	Any printable character
 *	\P	Any non-printable character
 *	\s	Any whitespace character other than newline
 *	\S	Any non-whitespace character
 *	\w	Any character that can be in a 'w'ord
 *	\W	Any character that can't be in a 'w'ord
 *	\i	Any character that can be in an identifier
 *	\I	Any character that can START an identifier
 *
 * The following affect the parsing rules
 *	\V	Use the traditional vi syntax, as described above
 *	\Q	Require backslashes before each metacharacter
 *	\E	Normal rules -- check 'magicchar' and 'magicperl' options
 *
 * The physical structure of a compiled regexp is as follows:
 *	- First, there is a one-byte value that says how many character classes
 *	  are used in this regular expression
 *	- Next, each character class is stored as a bitmap that is 256 bits
 *	  (32 bytes) long.
 *	- A mixture of literal characters and compiled meta characters follows.
 *	  This begins with M_BEGIN(0) and ends with M_END(0).  All meta chars
 *	  are stored as a \0 followed by a one-byte code, so they take up two
 *	  bytes apiece.  Literal characters take up one byte apiece.  \0 can't
 *	  be used as a literal character, though the M_NUL metacharacter serves
 *	  the same purpose.  Closure operators are stored in prefix notation:
 *	  first the closure operator itself, and then the expression that it
 *	  acts on (which may be either a single character or an M_BEGIN(n)
 *	  metacharacter).
 */

#include <setjmp.h>
#include "elvis.h"


#if USE_PROTOTYPES
static ELVBOOL handlenamedclass(CHAR **textp, REG CHAR *bmap);
static CHAR *makeclass(REG CHAR *text, REG CHAR *bmap);
static ELVBOOL isnongreedy(CHAR **sptr, ELVBOOL magicq);
static int gettoken(CHAR **sptr, regexp *re);
static unsigned calcsize(CHAR *text, MARK cursor);
static int calcminlen(CHAR **scan);
static int match1(regexp *re, REG _CHAR_ ch, REG int token);
static int match(regexp *re, MARK str, REG CHAR *prog, CHAR **here, ELVBOOL bol, int endtoken);

# ifdef DEBUG_REGEXP
static CHAR *decompile(CHAR *prog, int endtoken);
# endif
#endif


static CHAR	*previous;	/* the previous regexp, used when null regexp is given */


/* These are used to classify or recognize meta-characters */
#define META		'\0'
#define BASE_META(m)	((m) - 256)
#define INT_META(c)	((c) + 256)
#define IS_META(m)	((m) >= 256)
#define IS_CLASS(m)	((m) >= M_CLASS(0) && (m) <= M_CLASS(9))
#define IS_START(m)	((m) >= M_START(0) && (m) <= M_START(9))
#define IS_END(m)	((m) >= M_END(0) && (m) <= M_END(9))
#define IS_ALT(m)	((m) >= M_ALT(0) && (m) <= M_ALT(9))
#define IS_CLOSURE(m)	((m) >= M_SPLAT && (m) <= M_NGRANGE)
#define IS_GREEDY(m)	((m) >= M_SPLAT && (m) <= M_RANGE)
#define INDEX_OF(m)	(((m) - M_START(0)) % 10)
#define START_OF(m)	M_START(INDEX_OF(m))
#define END_OF(m)	M_END(INDEX_OF(m))
#define ADD_META(s,m)	(*(s)++ = META, *(s)++ = BASE_META(m))
#define GET_META(s)	(*(s) == META ? INT_META(*++(s)) : *s)

/* These are the internal codes used for each type of meta-character */
#define M_BEGLINE	256		/* internal code for ^ */
#define M_ENDLINE	257		/* internal code for $ */
#define M_BEGWORD	258		/* internal code for \< */
#define M_ENDWORD	259		/* internal code for \> */
#define M_EDGEWORD	260		/* internal code for \h (perl's \b) */
#define M_NOTEDGEWORD	261		/* internap code for \H (perl's \B) */
#define M_NUL		262		/* internal code for a NUL character */
#define M_ANY		263		/* internal code for . */
#define M_LEAVECURSOR	264		/* internal code for \= */
#define M_ATCURSOR	265		/* internal code for \@ */
#define M_SPLAT		266		/* internal code for * */
#define M_PLUS		267		/* internal code for \+ */
#define M_QMARK		268		/* internal code for \? */
#define M_RANGE		269		/* internal code for \{} */
#define M_NGSPLAT	270		/* internal code for *\? */
#define M_NGPLUS	271		/* internal code for \+\? */
#define M_NGQMARK	272		/* internal code for \?\? */
#define M_NGRANGE	273		/* internal code for \{}\? */
#define M_CLASS(n)	(274+(n))	/* internal code for [] */
#define M_START(n)	(284+(n))	/* internal code for \( */
#define M_END(n)	(294+(n))	/* internal code for \) */
#define M_ALT(n)	(304+(n))	/* internal code for \| in \(\) */

#define CLASS_DIGIT	1
#define CLASS_UPPER	2
#define CLASS_LOWER	4
#define CLASS_ALPHA	(CLASS_UPPER|CLASS_LOWER)
#define CLASS_ALNUM	(CLASS_UPPER|CLASS_LOWER|CLASS_DIGIT)

/* These are used during compilation */
static int	class_cnt;	/* used to assign class IDs */
static int	start_cnt;	/* used to assign start IDs */
static int	end_stk[NSUBEXP];/* used to assign end IDs */
static int	end_sp;
static CHAR	*retext;	/* points to the text being compiled */
static char	parserule;	/* one of 'E', 'Q', or 'V' */
static CHAR	*magicchar;	/* metachars that don't need backslash */

/* error-handling stuff */
jmp_buf	errorhandler;
#define FAIL(why)	regerror(why); longjmp(errorhandler, 1)


#ifdef DEBUG_REGEXP
# ifdef USE_PROTOTYPES
extern void regdump(regexp *re);
# endif

static CHAR *decompile(prog, endtoken)
	CHAR	*prog;	/* pointer into the "prog" code */
	int	endtoken;/* where to stop decompiling, usually M_END(0) */
{
	CHAR	*tmp = NULL;
	int	token, from, to;
	char	buf[30];

	do
	{
		/* add a space between tokens (but not before first token) */
		if (tmp)
			buildCHAR(&tmp, ' ');

		/* decompile the next token */
		token = GET_META(prog);
		prog++;
		switch (token)
		{
		  case M_BEGLINE:	buildstr(&tmp, "\\^");	break;
		  case M_ENDLINE:	buildstr(&tmp, "\\$");	break;
		  case M_BEGWORD:	buildstr(&tmp, "\\<");	break;
		  case M_ENDWORD:	buildstr(&tmp, "\\>");	break;
		  case M_EDGEWORD:	buildstr(&tmp, "\\h");	break;
		  case M_NOTEDGEWORD:	buildstr(&tmp, "\\H");	break;
		  case M_NUL:		buildstr(&tmp, "\\0");	break;
		  case M_ANY:		buildstr(&tmp, "\\.");	break;
		  case M_LEAVECURSOR:	buildstr(&tmp, "\\=");	break;
		  case M_ATCURSOR:	buildstr(&tmp, "\\@");	break;
		  case M_SPLAT:		buildstr(&tmp, "\\*");	break;
		  case M_PLUS:		buildstr(&tmp, "\\+");	break;
		  case M_QMARK:		buildstr(&tmp, "\\?");	break;
		  case M_RANGE:
			from = *prog++;
			to = *prog++;
			sprintf(buf, "\\{%d,%d}", from, to);
			buildstr(&tmp, buf);
			break;
		  case M_NGSPLAT:	buildstr(&tmp, "\\*\\?"); break;
		  case M_NGPLUS:	buildstr(&tmp, "\\+\\?"); break;
		  case M_NGQMARK:	buildstr(&tmp, "\\?\\?"); break;
		  case M_NGRANGE:
			from = *prog++;
			to = *prog++;
			sprintf(buf, "\\{%d,%d}\\?", from, to);
			buildstr(&tmp, buf);
			break;
		  case M_CLASS(0):	buildstr(&tmp, "\\[0");	break;
		  case M_CLASS(1):	buildstr(&tmp, "\\[1");	break;
		  case M_CLASS(2):	buildstr(&tmp, "\\[2");	break;
		  case M_CLASS(3):	buildstr(&tmp, "\\[3");	break;
		  case M_CLASS(4):	buildstr(&tmp, "\\[4");	break;
		  case M_CLASS(5):	buildstr(&tmp, "\\[5");	break;
		  case M_CLASS(6):	buildstr(&tmp, "\\[6");	break;
		  case M_CLASS(7):	buildstr(&tmp, "\\[7");	break;
		  case M_CLASS(8):	buildstr(&tmp, "\\[8");	break;
		  case M_CLASS(9):	buildstr(&tmp, "\\[9");	break;
		  case M_START(0):	buildstr(&tmp, "\\(0");	break;
		  case M_START(1):	buildstr(&tmp, "\\(1");	break;
		  case M_START(2):	buildstr(&tmp, "\\(2");	break;
		  case M_START(3):	buildstr(&tmp, "\\(3");	break;
		  case M_START(4):	buildstr(&tmp, "\\(4");	break;
		  case M_START(5):	buildstr(&tmp, "\\(5");	break;
		  case M_START(6):	buildstr(&tmp, "\\(6");	break;
		  case M_START(7):	buildstr(&tmp, "\\(7");	break;
		  case M_START(8):	buildstr(&tmp, "\\(8");	break;
		  case M_START(9):	buildstr(&tmp, "\\(9");	break;
		  case M_END(0):	buildstr(&tmp, "\\)0");	break;
		  case M_END(1):	buildstr(&tmp, "\\)1");	break;
		  case M_END(2):	buildstr(&tmp, "\\)2");	break;
		  case M_END(3):	buildstr(&tmp, "\\)3");	break;
		  case M_END(4):	buildstr(&tmp, "\\)4");	break;
		  case M_END(5):	buildstr(&tmp, "\\)5");	break;
		  case M_END(6):	buildstr(&tmp, "\\)6");	break;
		  case M_END(7):	buildstr(&tmp, "\\)7");	break;
		  case M_END(8):	buildstr(&tmp, "\\)8");	break;
		  case M_END(9):	buildstr(&tmp, "\\)9");	break;
		  case M_ALT(0):	buildstr(&tmp, "\\|0");	break;
		  case M_ALT(1):	buildstr(&tmp, "\\|1");	break;
		  case M_ALT(2):	buildstr(&tmp, "\\|2");	break;
		  case M_ALT(3):	buildstr(&tmp, "\\|3");	break;
		  case M_ALT(4):	buildstr(&tmp, "\\|4");	break;
		  case M_ALT(5):	buildstr(&tmp, "\\|5");	break;
		  case M_ALT(6):	buildstr(&tmp, "\\|6");	break;
		  case M_ALT(7):	buildstr(&tmp, "\\|7");	break;
		  case M_ALT(8):	buildstr(&tmp, "\\|8");	break;
		  case M_ALT(9):	buildstr(&tmp, "\\|9");	break;
		  default:		buildCHAR(&tmp, token);
		}
	} while (token != endtoken);

	/* return the string */
	return tmp;
}

/* Display the internal structure of a compiled regular expression */
void regdump(re)
	regexp	*re;	/* the regular expression to dump */
{
	CHAR	*p;	/* used for scanning the regex */
	CHAR	*tmp;	/* temporary string */
	int	i, j, k;

	/* first output the flags & stuff */
	if (re->first)
		fprintf(stderr, "minlen=%d, first='%c'\n",
			re->minlen, re->first);
	else
		fprintf(stderr, "minlen=%d, first is not any single character\n",
			re->minlen);
	fprintf(stderr, "bol=%s, literal=%s, upper=%s, nclasses=%d\n",
		re->bol ? "True" : "False", re->literal ? "True" : "False",
		re->upper ? "True" : "False", re->program[0]);

	/* Output the classes */
	for (p = &re->program[1], i = 0; i < re->program[0]; p += 32, i++)
	{
		tmp = NULL;
		for (j = k = 0; j < 256; j++)
		{
			if (p[j >> 3] & (1 << (j & 7)))
				buildCHAR(&tmp, j);
		}
		buildCHAR(&tmp, '\0');
		fprintf(stderr, "class %d = [%s]\n", k, tmp);
		safefree(tmp);
	}

	/* Decompile the regexp */
	tmp = decompile(p, M_END(0));
	fprintf(stderr, "program = %s\n\n", tmp);
	safefree(tmp);
}
#endif /* DEBUG_REGEXP */

/* This function copies a regular expression into a dynamically allocated
 * string.  The (CHAR*) that refp refers to will be incremented to point to
 * the character after the closing delimiter, or to the newline or NUL if
 * there is no closing delimiter.  It may also be set to NULL if it hits the
 * end of the string/buffer being scanned.
 *
 * The calling function is responsible for calling safefree() on the string
 * when it is no longer needed.
 */
CHAR *regbuild(delim, refp, reg)
	_CHAR_	delim;	/* the delimiter */
	CHAR	**refp;	/* reference to a CHAR* already used with scanalloc() */
	ELVBOOL	reg;	/* really regular expression? (else replacement text) */
{
	CHAR	*retext;
	ELVBOOL	inclass, curly;
	CHAR	name[30], *value;
	int	i;
	int	phase;	/* context within a bracketed character class */
			/* 0: initially & after '-' of range: '-',']' literal */
			/* 1: after end of range: '-' literal, ']' ends */
			/* 2: normal: '-' denotes range, ']' ends */
			/* 3: in named class w/ extra brackets: ']' ends name */

	parserule = 'E';
	for (retext = NULL, inclass = ElvFalse, phase = 0;
	     *refp && **refp && **refp != '\n' && (inclass || **refp != delim);
	     )
	{
		/* $name or ${name} copies character from the named variable */
		if (**refp == '$' && o_magic && o_magicname && parserule == 'E')
		{
			/* verify that a name follows '$' */
			if (!scannext(refp) || (!elvalpha(**refp) && **refp != '{'))
			{
				/* oops, no name so just copy the $ */
				buildCHAR(&retext, '$');
				continue;
			}

			/* copy the name into a temporary buffer */
			if (*refp && **refp == '{')
			{
				for (i = 0; scannext(refp) && **refp != '}'; )
					if (i <  QTY(name) - 1)
						name[i++] = **refp;
				name[i] = '\0';
				if (*refp)
					scannext(refp);
				else
				{
					/* hit end without finding curly --
					 * copy what we did find, literally.
					 */
					buildCHAR(&retext, '$');
					buildCHAR(&retext, '{');
					for (i = 0; name[i]; i++)
						buildCHAR(&retext, name[i]);
					continue;
				}
				curly = ElvTrue;
			}
			else
			{
				for (i = 0; *refp && elvalnum(**refp); scannext(refp))
					if (i < QTY(name) - 1)
						name[i++] = **refp;
				name[i] = '\0';
				curly = ElvFalse;
			}

			/* Add the named option's value into the regexp */
			value = optgetstr(name, NULL);
			if (value)
			{
				while (*value)
					buildCHAR(&retext, *value++);
			}
			else
			{
				/* no such variable - copy $name literally */
				buildCHAR(&retext, '$');
				if (curly)
					buildCHAR(&retext, '{');
				for (i = 0; name[i]; i++)
					buildCHAR(&retext, name[i]);
				if (curly)
					buildCHAR(&retext, '}');
			}
			continue;
		}

		/* if not in a class, then '[' starts a class */
		if (reg && !inclass && **refp == '[' && o_magic)
		{
			inclass = ElvTrue;
			buildCHAR(&retext, '[');
			if (scannext(refp) && **refp == '^')
			{
				buildCHAR(&retext, '^');
				scannext(refp);
			}
			phase = 0;
			continue;
		}

		/* if not in a class, then backslashes need special treatment */
		if (!inclass && **refp == '\\')
		{
			/* move to next character, if any */
			if (!scannext(refp) || !**refp || **refp == '\n')
				continue;

			/* if E V or Q then update parserule */
			if (CHARchr(toCHAR("EVQ"), **refp))
				parserule = **refp;

			/* if the backslashed character is a delimiter, then
			 * add the delimiter without the backslash; else add
			 * both the backslash and the character.
			 */
			if (**refp != delim)
				buildCHAR(&retext, '\\');
			buildCHAR(&retext, **refp);
			scannext(refp);
			continue;
		}

		/* otherwise add the character to the retext */
		buildCHAR(&retext, **refp);

		/* if in class, we need to update phase */
		if (inclass)
		{
			if (**refp == '[')
			{
				if (!scannext(refp))
					continue;
				if (**refp == ':')
					phase = 3; /* next will be class name */
				else if (phase == 0)
					phase = 1; /* next will be after range*/
				else
					phase = 2; /* next will be normal */
				continue;
			}
			else if (**refp == ']' && phase == 3)
				phase = 1; /* next will be char after range */
			else if (**refp == ']' && phase != 0)
				inclass = ElvFalse; /* END OF CHARACTER CLASS */
			else if (**refp == '-' && phase == 2)
				phase = 0; /* next will be end of range */
			else if (phase == 0)
				phase = 1; /* next will be char after range */
			else if (phase != 3)
				phase = 2; /* next will be normal character */
		}

		/* advance to next character */
		scannext(refp);
	}

	/* if no characters, then return a dynamically allocated "" string */
	if (!retext)
		retext = (CHAR *)safealloc(1, sizeof(CHAR));

	/* never return a string containing only parserule metachars */
	for (value = retext; value[0] == '\\' && CHARchr(toCHAR("EQV"), value[1]); value += 2)
	{
	}
	if (!*value && value != retext)
	{
		safefree(retext);
		retext = (CHAR *)safealloc(1, sizeof(CHAR));
	}

	/* if we hit a closing delimiter, then move past it */
	if (*refp && **refp == delim)
		scannext(refp);

	/* return the dynamic string */
	return retext;
}


/* This is a utility function for makeclass. It is factored out since makeclass
 * was becoming unwieldy. Returns ElvTrue if a named class; ElvFalse otherwise.
 * Should possibly detect errors in such constructs as [...[:fred:]...]
 */
static ELVBOOL handlenamedclass(textp, bmap)
	  CHAR        **textp;        /* character list (updates *textp) */
      REG CHAR        *bmap;          /* bitmap of selected characters */
{
	static struct clastran {
		char	cname[sizeof "xdigit:]"];
		size_t	nsize;
		int	include;
		enum {NONE=0, INVERT=1, ASCII=2, PRINT=4, BLANK=8} flags;
	} tran[] = {
	    { "alnum:]",  sizeof "alnum:]"  - 1, ELVCT_ALNUM, NONE        },
	    { "alpha:]",  sizeof "alpha:]"  - 1, ELVCT_ALPHA, NONE        },
	    { "ascii:]",  sizeof "ascii:]"  - 1, 0          , ASCII       },
	    { "blank:]",  sizeof "blank:]"  - 1, 0          , BLANK       },
	    { "cntrl:]",  sizeof "cntrl:]"  - 1, ELVCT_CNTRL, NONE        },
	    { "digit:]",  sizeof "digit:]"  - 1, ELVCT_DIGIT, NONE        },
	    { "graph:]",  sizeof "graph:]"  - 1, ELVCT_GRAPH, PRINT       },
	    { "lower:]",  sizeof "lower:]"  - 1, ELVCT_LOWER, NONE        },
	    { "print:]",  sizeof "print:]"  - 1, ELVCT_CNTRL, INVERT|PRINT},
	    { "punct:]",  sizeof "punct:]"  - 1, ELVCT_PUNCT, NONE        },
	    { "space:]",  sizeof "space:]"  - 1, ELVCT_SPACE, NONE        },
	    { "upper:]",  sizeof "upper:]"  - 1, ELVCT_UPPER, NONE        },
	    { "xdigit:]", sizeof "xdigit:]" - 1, ELVCT_XDIGIT,NONE        },
	};

	struct clastran *sp;

	REG CHAR        *text = *textp;
	REG int         i, incl, invert;

	/* if obviously not a named class, then return ElvFalse */
	if (text[0] != '[' || text[1] != ':')
		return ElvFalse;
	text += 2;

	/* strip off the "invert" carat, if present */
	invert = (*text == '^');
	if (invert)
		text++;

	/* search for the class in tran[] */
	for (sp = tran; sp < tran + QTY(tran); sp++)
	{
		if (CHARncmp(text, toCHAR(sp->cname), sp->nsize) == 0)
		{
			/* add the named class to the bitmap */
			for (i = 0; bmap && i < 256; i++)
			{
				/* begin by checking the ctype macros */
				incl = sp->include ? (sp->include & elvct_class[i]) : 1;
				/* invert if necessary */
				if (sp->flags & INVERT)
					incl = !incl;
				if (invert)
					incl = !incl;

				/* the BLANK, ASCII, and PRINT flags eliminate
				 * some characters.
				 */
				if (((sp->flags & BLANK) && i != '\t' && i != ' ')
				 || ((sp->flags & ASCII) && i >= 128))
					incl = 0;
				if (sp->flags & PRINT)
				{
					switch (o_nonascii)
					{
					  case 'a':	break;
					  case 'n':
					  	if (i >= 128)
					  		incl = 0;
					  	break;

					  case 'm':
					  case 's':
						if (i >= 128 && i < 160)
							incl = 0;
						break;
					}
				}
				if (incl)
				{
					bmap[i >> 3] |= 1 << (i & 7);
				}
			}

			/* move past this named class */
			*textp += sp->nsize + 2;
			if (invert)
				(*textp) += 1;
			return ElvTrue;
		}
	}

	/* if we get here, then it was an unknown named class */
	FAIL("unknown named character class");
	/*NOTREACHED*/
}

/* This function builds a bitmap for a particular class.  "text" points
 * to the start of the class string, and "bmap" is a pointer to memory
 * which can be used to store the bitmap of the class.  If "bmap" is NULL,
 * then the class will be parsed but bitmap will not be generated.
 */
static CHAR *makeclass(text, bmap)
	REG CHAR	*text;	/* character list */
	REG CHAR	*bmap;	/* bitmap of selected characters */
{
	REG int		i;
	int		complement = 0;
	ELVBOOL		first;
	CHAR		*namedclass;


	/* zero the bitmap */
	for (i = 0; bmap && i < 32; i++)
	{
		bmap[i] = 0;
	}

	/* see if we're going to complement this class */
	if (*text == '^')
	{
		text++;
		complement = 1;
	}

	/* add in the characters */
	for (first = ElvTrue; *text && (first || *text != ']'); first = ElvFalse)
	{
		/* is this a span of characters? */
		if (text[1] == '-' && text[2])
		{
			/* spans can't be backwards */
			if (text[0] > text[2])
			{
				FAIL("backwards span in \\[]");
			}

			/* add each character in the span to the bitmap */
			for (i = text[0]; bmap && (unsigned)i <= text[2]; i++)
			{
				bmap[i >> 3] |= (1 << (i & 7));
			}

			/* move past this span */
			text += 3;
		}
		else if (!handlenamedclass(&text, bmap))
		{
			namedclass = NULL;
			i = -1;
			if (text[0] == '\\' && text[1])
			{
				switch (*++text)
				{
				  case '0':
					i = '\0';	/* NUL */
					break;

				  case 'a':
					i = '\007';	/* BEL */
					break;

				  case 'b':
					i = '\b';	/* BS */
					break;

				  case 'e':
					i = '\033';	/* ESC */
					break;

				  case 'f':
					i = '\f';	/* FF */
					break;

				  case 'n':
					FAIL("\\n doesn't work in regexp");

				  case 'r':
					i = '\r';	/* CR */
					break;

				  case 't':
					i = '\t';	/* TAB */
					break;

				  case 'd':
					namedclass = toCHAR("[:digit:]");
					break;

				  case 'D':
					namedclass = toCHAR("[:^digit:]");
					break;

				  case 'p':
					namedclass = toCHAR("[:print:]");
					break;

				  case 'P':
					namedclass = toCHAR("[:^print:]");
					break;

				  case 's':
					namedclass = toCHAR("[:space:]");
					break;

				  case 'S':
					namedclass = toCHAR("[:^space:]");
					break;

				  case 'w':
				  case 'i':
					namedclass = toCHAR("[:alnum:]");
					i = '_';
					break;

				  case 'W':
					namedclass = toCHAR("[:^alnum:]");
					i = '_';
					break;

				  case 'I':
					namedclass = toCHAR("[:alpha:]");
					i = '_';
					break;

				  default:
					i = *text;
				}
			}
			else
			{
				/* simple character in list */
				i = *text;
			}
			text++;

			/* At this point, either namedclass is points to a
			 * string describing a character class, or i contains
			 * a single character to add.  Do one or the other.
			 */
			if (namedclass)
			{
				/* add the characters from the list */
				(void)handlenamedclass(&namedclass, bmap);
			}
			if (i != -1 && bmap)
			{
				/* add this single character to the list */
				bmap[i >> 3] |= (1 << (i & 7));
			}
		}
	}

	/* make sure the closing ] isn't missing */
	if (*text++ != ']')
	{
		FAIL("] missing");
	}

	/* if we're supposed to complement this class, then do so */
	if (complement && bmap)
	{
		for (i = 0; i < 32; i++)
		{
			bmap[i] = ~bmap[i];
		}
	}

	return text;
}


/* if the next character is a ? or \? metacharacter then advance past it and
 * return ElvTrue; else return ElvFalse.
 */
static ELVBOOL isnongreedy(sptr, magicq)
	CHAR	**sptr;	/* reference to pointer into regexp text */
	ELVBOOL	magicq;	/* is the metacharacter a plain "?" (else "\?") */
{
	if (**sptr != (magicq ? '?' : '\\'))
		return ElvFalse;
	(*sptr)++;
	if (!magicq)
	{
		if (**sptr != '?')
		{
			(*sptr)--;
			return ElvFalse;
		}
		else
			(*sptr)++;
	}
	return ElvTrue;
}


/* This function gets the next character or meta character from a string.
 * The pointer is incremented by 1, or by 2 for \-quoted characters.  For [],
 * a bitmap is generated via makeclass() (if re is given), and the
 * character-class text is skipped.  "sptr" is a pointer to the pointer
 * which is used for scanning the text source of the regular expression,
 * and "re" is a pointer to a buffer which will be used to store the
 * compiled regular expression, or NULL if it hasn't been allocated yet.
 */
static int gettoken(sptr, re)
	CHAR	**sptr;	/* pointer to the text scanning pointer */
	regexp	*re;	/* pointer to the regexp being built, or NULL */
{
	int	c;
	CHAR	*subexpr;
	ELVBOOL	magicq;	/* is ? magic without a preceding backslash? */

	/* determine whether \? or ? is magic */
	magicq = (ELVBOOL)(o_magic && magicchar && CHARchr(magicchar, '?'));

	c = **sptr;
	if (!c)
	{
		return c;
	}
	++*sptr;
	if (c == '\\')
	{
		c = **sptr;
		++*sptr;
		if (o_magic && magicchar && CHARchr(magicchar, c))
			return c;
		switch (c)
		{
		  case '<':
			return M_BEGWORD;

		  case '>':
			return M_ENDWORD;

		  case '(':
			if (start_cnt >= NSUBEXP)
			{
				FAIL("too many \\(s");
			}
			end_stk[end_sp++] = start_cnt;
			return M_START(start_cnt++);

		  case ')':
			if (end_sp <= 0)
			{
				FAIL("mismatched \\)");
			}
			return M_END(end_stk[--end_sp]);

		  case '|':
			return M_ALT(end_sp ? end_stk[end_sp - 1] : 0);

		  case '*':
			return isnongreedy(sptr, magicq) ? M_NGSPLAT : M_SPLAT;

		  case '.':
			return M_ANY;

		  case '+':
			return isnongreedy(sptr, magicq) ? M_NGPLUS : M_PLUS;

		  case '?':
			return isnongreedy(sptr, magicq) ? M_NGQMARK : M_QMARK;

		  case '=':
			return M_LEAVECURSOR;

		  case '@':
			return M_ATCURSOR;

		  case '{': /*}*/
			/* Non-greedy version will be detected elsewhere, after
			 * the range numbers have been parsed.
			 */
			return M_RANGE;

		  case '0':
			return M_NUL;

		  case 'a':
			return '\007';	/* BEL */

		  case 'b':
			return (parserule == 'E' && o_magicperl) ? M_EDGEWORD
								 : '\b';

		  case 'e':
			return '\033';	/* ESC */

		  case 'f':
			return '\f';	/* FF */

		  case 'n':
			FAIL("\\n doesn't work in regexp");

		  case 'r':
			return '\r';	/* CR */

		  case 't':
			return '\t';	/* TAB */

		  case 'd':
			subexpr = toCHAR("[[:digit:]]");
			return gettoken(&subexpr, re);

		  case 'D':
			subexpr = toCHAR("[^[:digit:]]");
			return gettoken(&subexpr, re);

		  case 'p':
			subexpr = toCHAR("[[:print:]]");
			return gettoken(&subexpr, re);

		  case 'P':
			subexpr = toCHAR("[^[:print:]]");
			return gettoken(&subexpr, re);

		  case 's':
			subexpr = toCHAR("[[:space:]]");
			return gettoken(&subexpr, re);

		  case 'S':
			subexpr = toCHAR("[^[:space:]]");
			return gettoken(&subexpr, re);

		  case 'w':
		  case 'i':
			subexpr = toCHAR("[[:alnum:]_]");
			return gettoken(&subexpr, re);

		  case 'W':
			subexpr = toCHAR("[^[:alnum:]_]");
			return gettoken(&subexpr, re);

		  case 'I':
			subexpr = toCHAR("[[:alpha:]_]");
			return gettoken(&subexpr, re);

		  case 'h':
			return M_EDGEWORD;

		  case 'H':
		  case 'B':
			return M_NOTEDGEWORD;

		  case '\0':
			FAIL("extra \\ at end of regexp");

		  case 'E':
			parserule = 'E';
			magicchar = o_magicchar;
			return gettoken(sptr, re);

		  case 'V':
			parserule = 'V';
			magicchar = toCHAR("^$.*[");
			return gettoken(sptr, re);

		  case 'Q':
			parserule = 'Q';
			magicchar = NULL;
			return gettoken(sptr, re);

		  default:
			return c;
		}
	}
	else if (o_magic) /* and char is not a backslash */
	{
		if (!magicchar || !CHARchr(magicchar, c))
			return c;
		switch (c)
		{
		  case '<':
			return M_BEGWORD;

		  case '>':
			return M_ENDWORD;

		  case '(':
			if (start_cnt >= NSUBEXP)
			{
				FAIL("too many \\(s");
			}
			end_stk[end_sp++] = start_cnt;
			return M_START(start_cnt++);

		  case ')':
			if (end_sp <= 0)
			{
				FAIL("mismatched \\)");
			}
			return M_END(end_stk[--end_sp]);

		  case '|':
			return M_ALT(end_sp ? end_stk[end_sp - 1] : 0);

		  case '*':
			return isnongreedy(sptr, magicq) ? M_NGSPLAT : M_SPLAT;

		  case '.':
			return M_ANY;

		  case '+':
			return isnongreedy(sptr, magicq) ? M_NGPLUS : M_PLUS;

		  case '?':
			return isnongreedy(sptr, magicq) ? M_NGQMARK : M_QMARK;

		  case '=':
			return M_LEAVECURSOR;

		  case '@':
			return M_ATCURSOR;

		  case '{': /*}*/
			/* Non-greedy version will be detected elsewhere, after
			 * the range numbers have been parsed.
			 */
			return M_RANGE;

		  case '^':
			/* At this point, (*sptr) has been incremented past the
			 * '^' character, so subtract 1 from it in the following
			 * comparisons.
			 */
			if ((*sptr) - 1 == retext
			 || ((*sptr) - 1 == retext + 2
			 	&& retext[0] == '\\'
			 	&& CHARchr(toCHAR("QVE"), retext[1])))
			{
				return M_BEGLINE;
			}
			return c;

		  case '$':
			if (!**sptr)
			{
				return M_ENDLINE;
			}
			return c;

		  case '[':
			/* make sure we don't have too many classes */
			if (class_cnt >= 10)
			{
				FAIL("too many \\[]s");
			}

			/* process the character list for this class */
			if (re)
			{
				/* generate the bitmap for this class */
				*sptr = makeclass(*sptr, re->program + 1 + 32 * class_cnt);
			}
			else
			{
				/* skip to end of the class */
				*sptr = makeclass(*sptr, (CHAR *)0);
			}
#ifdef DEBUG_REGEXP
			fprintf(stderr, "gettoken() returning M_CLASS(%d)\n", class_cnt);
#endif
			return M_CLASS(class_cnt++);

		  default:
			return c;
		}
	}
	else	/* unquoted nomagic */
	{
		switch (c)
		{
		  case '^':
			if (*sptr == retext + 1)
			{
				return M_BEGLINE;
			}
			return c;

		  case '$':
			if (!**sptr)
			{
				return M_ENDLINE;
			}
			return c;

		  default:
			return c;
		}
	}
	/*NOTREACHED*/
}




/* This function calculates the number of bytes that will be needed for a
 * compiled regexp.  Its argument is the uncompiled version.  It is not clever
 * about catching syntax errors; that is done in a later pass.  "text" is
 * a pointer to the source text of the regular expression.
 */
static unsigned calcsize(text, cursor)
	CHAR	*text;	/* source code of the regexp */
	MARK	cursor;	/* cursor position, to support "\@" */
{
	unsigned	size;
	int		token;
	MARKBUF		tmpb;
	MARK		tmp;

	size = 5;
	while ((token = gettoken(&text, (regexp *)0)) != 0)
	{
		if (IS_CLASS(token))
		{
			size += 34;
		}

		else if (token == M_RANGE || token == M_NGRANGE)
		{
			size += 4;
			while ((token = gettoken(&text, (regexp *)0)) != 0
			    && token != '}')
			{
			}
			if (!token)
			{
				return size;
			}
			(void)isnongreedy(&text, (ELVBOOL)(o_magic && magicchar && CHARchr(magicchar, '?')));
		}
		else if (token == M_ATCURSOR)
		{
			tmpb = *cursor;
			tmp = wordatcursor(&tmpb, ElvFalse);
			if (tmp)
			{
				assert(markoffset(&tmpb) > markoffset(tmp));
				size += markoffset(&tmpb) - markoffset(tmp);
			}
		}
		else if (IS_META(token))
		{
			size += 2;
		}
		else
		{
			size++;
		}
	}

#ifdef DEBUG_REGEXP
	fprintf(stderr, "at end of calcsize(), class_cnt=%d, start_cnt=%d, size=%d\n", class_cnt, start_cnt, size);
#endif
	return size;
}


/* calculate the minlen for a part of the compiled regexp */
static int calcminlen(scan)
	CHAR	**scan;	/* ref to pointer to a M_START(n) metacharacter */
{
	int	altlen;	/* length of an alternative */
	int	minlen;	/* minimum length of this subexpression */
	int	sublen;	/* length of an embedded subexpression */
	int	end;	/* the corresponding M_END(n) metacharacter */
	int	token;	/* a token from the compiled regexp */
	int	lb;	/* lowerbound of repeats */

	/* identify the end marker */
	token = GET_META(*scan);
	(*scan)++;
	assert(IS_START(token));
	end = END_OF(token);

	/* count characters, taking metacharacters into account */
	for (minlen = INFINITY, altlen = 0;
	     token = GET_META(*scan), (*scan)++, token != end;
	     )
	{
		/* count this token */
		if (!IS_META(token) || IS_CLASS(token) || token == M_ANY || token == M_NUL)
		{
			/* it is a single character */
			altlen++;
		}
		else if (IS_ALT(token))
		{
			/* it is the end of an alternative */
			if (altlen < minlen)
				minlen = altlen;
			altlen = 0;
		}
		else if (IS_START(token))
		{
			/* it is the start of an embedded subexpression */
			(*scan) -= 2;
			altlen += calcminlen(scan);
		}
		else if (IS_CLOSURE(token))
		{
			/* calc the lowerbound for this type of closure */
			switch (token)
			{
			  case M_SPLAT:
			  case M_NGSPLAT: lb = 0;			break;
			  case M_PLUS:
			  case M_NGPLUS:  lb = 1;			break;
			  case M_QMARK:
			  case M_NGQMARK: lb = 0;			break;
			  case M_RANGE:
			  case M_NGRANGE: lb = **scan; (*scan) += 2;	break;
			}

			/* fetch the token that it applies to */
			token = GET_META(*scan);
			(*scan)++;

			/* is it a single char, or subexpression?*/
			if (!IS_META(token) || IS_CLASS(token) || token == M_ANY || token == M_NUL)
			{
				/* single char */
				sublen = 1;
			}
			else
			{
				/* subexpression */
				(*scan) -= 2; /* move back to \( */
				sublen = calcminlen(scan);
			}

			/* multiply the length by the minimum rep */
			altlen += lb * sublen;
		}
	}

	/* If last alternative was shortest, remember that */
	if (altlen < minlen)
		minlen = altlen;

	/* Return the minlen.  Note that we have also advanced (*scan) to the
	 * ending \), but haven't past it yet.  Recursive calls skip over the
	 * \) perfectly well, and the call from regcomp() doesn't care.
	 */
	return minlen;
}


/* This function compiles a regexp.  "exp" is the source text of the regular
 * expression.
 */
regexp *regcomp(exp, cursor)
	CHAR	*exp;	/* source code of the regular expression */
	MARK	cursor;	/* cursor position, to support "\@" */
{
	int		needfirst;
	unsigned	size;
	int		token;
	int		peek;
	CHAR		*scan, *build;
	regexp		*re;
	int		from, to;
	int		digit;
#ifdef DEBUG_REGEXP
	int		calced;
#endif
	MARK		tmp;


#ifdef DEBUG_REGEXP
	if (cursor)
		fprintf(stderr, "regcomp(\"%s\", {0x%lx, %ld})...\n",
			exp, (long)markbuffer(cursor), markoffset(cursor));
	else
		fprintf(stderr, "regcomp(\"%s\", NULL)...\n", exp);
#endif

	/* prepare for error handling */
	re = (regexp *)0;
	from = to = 0;
	if (setjmp(errorhandler))
	{
		if (re)
		{
			safefree(re);
		}
		return (regexp *)0;
	}

	/* if an empty regexp string was given, use the previous one */
	if (*exp == 0)
	{
		if (!previous)
		{
			FAIL("no previous regexp");
		}
		exp = previous;
	}
	else if (o_saveregexp) /* non-empty regexp given, so remember it */
	{
		if (previous)
			safefree(previous);
		previous = (CHAR *)safekept((int)(CHARlen(exp) + 1), sizeof(CHAR));
		if (previous)
			CHARcpy(previous, exp);
	}

	/* allocate memory */
	class_cnt = 0;
	start_cnt = 1;
	end_sp = 0;
	retext = exp;
	parserule = 'E';
	magicchar = o_magicchar;
#ifdef DEBUG_REGEXP
	calced = calcsize(exp, cursor);
	size = calced + sizeof(regexp);
#else
	size = calcsize(exp, cursor) + sizeof(regexp) + 10; /* !!! 10 bytes for slop */
#endif
#ifdef lint
	re = (regexp *)0;
#else
	re = (regexp *)safekept((int)size, sizeof(CHAR));
#endif
	if (!re)
	{
		FAIL("not enough memory for this regexp");
	}

	/* compile it */
	build = &re->program[1 + 32 * class_cnt];
	re->program[0] = class_cnt;
	parserule = 'E';
	magicchar = o_magicchar;
	for (token = 0; token < NSUBEXP; token++)
	{
		re->startp[token] = re->endp[token] = -1;
	}
	re->leavep = -1;
	re->first = 0;
	re->bol = ElvFalse;
	re->upper = ElvFalse;
	needfirst = 1;
	class_cnt = 0;
	start_cnt = 1;
	end_sp = 0;
	retext = exp;
	for (token = M_START(0), peek = gettoken(&exp, re);
	     token;
	     token = peek, peek = gettoken(&exp, re))
	{
		/* special processing for the closure operator */
		if (IS_CLOSURE(peek))
		{
			/* detect misuse of closure operator */
			if (IS_START(token))
			{
				FAIL("closure operator follows nothing");
			}
			else if (IS_META(token) && token != M_ANY && token != M_NUL && !IS_CLASS(token) && !IS_END(token))
			{
				FAIL("closure operators can only follow a normal character or \\. or \\[] or \\)");
			}

			/* if \{ \} then read the range */
			if (peek == M_RANGE)
			{
				from = 0;
				for (digit = gettoken(&exp, re);
				     !IS_META(digit) && elvdigit(digit);
				     digit = gettoken(&exp, re))
				{
					from = from * 10 + digit - '0';
				}
				/*{*/
				if (digit == '}')
				{
					to = from;
				}
				else if (digit == ',')
				{
					to = 0;
					for (digit = gettoken(&exp, re);
					     !IS_META(digit) && elvdigit(digit);
					     digit = gettoken(&exp, re))
					{
						to = to * 10 + digit - '0';
					}
					if (to == 0)
					{
						to = 255;
					}
				}
				/*{*/
				if (digit != '}')
				{
					FAIL("bad characters after \\{"); /*}*/
				}
				else if (to < from || to == 0 || from >= 255)
				{
					FAIL("invalid range for \\{ \\}");
				}

				/* detect whether this is the nongreedy version */
				if (isnongreedy(&exp, (ELVBOOL)(o_magic && magicchar && CHARchr(magicchar, '?'))))
				{
					peek = M_NGRANGE;
				}
			}

			/* it is okay -- make it prefix instead of postfix */
			if (IS_END(token))
			{
				/* applied to complex subexpression - find the
				 * start of that subexpression.
				 */
				for (scan = &re->program[1 + 32 * class_cnt];
				     GET_META(scan) != START_OF(token);
				     scan++)
				{
				}
				scan--;/*since GET_META moved past first byte*/

				/* shift the whole subexpression to make room
				 * for the closure operator *before* the subexpr
				 */
				if (peek == M_RANGE || peek == M_NGRANGE)
				{
					memmove(scan+4, scan, (int)(build - scan));
					build += 4;
					ADD_META(scan, peek);
					*build++ = from;
					*build++ = (to < 255 ? to : 255);
				}
				else
				{
					memmove(scan+2, scan, (int)(build - scan));
					build += 2;
					ADD_META(scan, peek);
				}
			}
			else
			{
				ADD_META(build, peek);
				if (peek == M_RANGE || peek == M_NGRANGE)
				{
					*build++ = from;
					*build++ = (to < 255 ? to : 255);
				}
			}
			

			/* take care of "needfirst" - is this the first char? */
			if (needfirst && peek == M_PLUS && !IS_META(token))
			{
				re->first = token;
			}
			needfirst = 0;

			/* we used "peek" -- need to refill it */
			peek = gettoken(&exp, re);
			if (IS_CLOSURE(peek))
			{
				FAIL("\\* or \\+ or \\? doubled up");
			}
		}
		else if (!IS_META(token))
		{
			/* normal char is NOT argument of closure */
			if (needfirst)
			{
				re->first = token;
				needfirst = 0;
			}
		}
		else if (token == M_ANY || IS_CLASS(token) || token == M_NUL)
		{
			/* . or [] is NOT argument of closure */
			needfirst = 0;
		}
		else if (IS_ALT(token))
		{
			/* \| is tricky.  As implemented here, the presence
			 * of any alternation operator will clobber the "first"
			 * character.  This works, but there are also some
			 * cases where "first" and \| could work together which
			 * this code doesn't support, so some searches will be
			 * slower than they otherwise might.
			 */ 
			needfirst = 0;
			re->first = 0;
		}

		/* the "token" character is not closure -- process it normally */
		if (token == M_BEGLINE)
		{
			/* set the BOL flag instead of storing M_BEGLINE */
			re->bol = ElvTrue;
		}
		else if (token == M_ATCURSOR)
		{
			tmp = wordatcursor(cursor, ElvFalse);
			if (!tmp)
			{
				FAIL("cursor not on word");
			}
			scanalloc(&scan, tmp);
			from = to = (int)(markoffset(cursor) - markoffset(tmp));
			if (needfirst && from > 1)
				re->first = *scan;
			needfirst = 0;
			for (; scan && --from >= 0; scannext(&scan))
			{
				*build++ = *scan;
			}
			scanfree(&scan);
		}
		else if (IS_META(token))
		{
			ADD_META(build, token);
		}
		else
		{
			*build++ = token;
			if (elvupper(token))
				re->upper = ElvTrue;
		}
	}
#ifdef DEBUG_REGEXP
	fprintf(stderr, "re->program[0]=%d, class_cnt=%d (should equal each other)\n", re->program[0], class_cnt);
	fprintf(stderr, "re->program[1]=%d, re->program[%d]=%d,%d (second should be 0,%d)\n",
		re->program[1], 1+32*class_cnt, re->program[1+32*class_cnt], re->program[2+32*class_cnt], BASE_META(M_START(0)));
#endif

	/* end it with a \) which MUST MATCH the opening \( */
	ADD_META(build, M_END(0));
	if (end_sp > 0)
	{
		FAIL("not enough \\)s");
	}

#ifdef FEATURE_LITRE
	/* Detect whether this is a literal regexp.  Literal regexps contain
	 * no metacharacters except M_BEGIN(0) and M_END(0).
	 */
	for (scan = &re->program[2 + 32 * re->program[0]]; *scan != META; scan++)
	{
	}
	if (GET_META(scan) == M_END(0))
		re->literal = ElvTrue;
	else
		re->literal = ElvFalse;
#endif

#ifdef DEBUG_REGEXP
	if ((int)(build - re->program) != calced)
	{
		msg(MSG_WARNING, "[dd]regcomp error: calced=$1, actual=$2", calced, (int)(build - re->program));
	}
#endif

	/* recompute the minimum length, by examining the regexp */
#ifdef DEBUG_REGEXP
	regdump(re);
#endif
	scan = &re->program[1 + 32 * re->program[0]];
	re->minlen = calcminlen(&scan);

	return re;
}


/* allocate a new copy of a regular expression */
regexp *regdup(re)
	regexp	*re;
{
	CHAR	*p;
	int	i;
	regexp	*newp;

	/* count the size of the regular expression's program */
	for (p = &re->program[1 + 32 * re->program[0]];
	     GET_META(p) != M_END(0);
	     p++)
	{
	}
	i = (int)(p - re->program);

	/* allocate memory */
	i = sizeof(regexp) + i * sizeof re->program[0];
	newp = safealloc(1, i);
	memcpy(newp, re, i);
	return newp;
}


/*---------------------------------------------------------------------------*/


/* This function checks for a match between a character and a token which is
 * known to represent a single character.  It returns 0 if they match, or
 * 1 if they don't.  "re" is a pointer to the compiled regular expression,
 * "ch" is the next character or '\n' at the end of the line, and "token"
 * is the particular part of the regular expression which this character
 * is supposed to match.
 */
static int match1(re, ch, token)
	regexp		*re;	/* regular expression being matched */
	REG _CHAR_	ch;	/* character from searched text */
	REG int		token;	/* token from regular expression */
{
	if (ch == '\n')
	{
		/* the end of a line can't match any regexp of width 1 */
		return 1;
	}
	if (token == M_ANY)
	{
		return 0;
	}
	else if (token == M_NUL)
	{
		if (ch == '\0')
			return 0;
	}
	else if (IS_CLASS(token))
	{
		if (re->program[1 + 32 * (token - M_CLASS(0)) + (ch >> 3)] & (1 << (ch & 7)))
			return 0;
	}
	else if ((_char_)ch == token
	      || (o_ignorecase
			&& !(o_smartcase && re->upper)
			&& elvtolower((_char_)ch) == elvtolower(token)))
	{
		return 0;
	}
	return 1;
}



/* This function checks characters up to and including the next closure, at
 * which point it does a recursive call to check the rest of it.  This function
 * returns 0 if everything matches, or 1 if something doesn't match.
 *
 * Also, if it matches then it has the side-effect of advancing the *here
 * scan variable past the matching text.  When matching fails, *here is left
 * unchanged.
 */
static int match(re, str, prog, here, bol, endtoken)
	regexp		*re;	/* the regular expression being matched */
	MARK		str;	/* string to be compared against regexp */
	REG CHAR	*prog;	/* pointer into body of compiled regexp */
	CHAR		**here;	/* pointer into the "str" string */
	ELVBOOL		bol;	/* if ElvTrue, "str" is the start of a line */
	int		endtoken;/* the \) to stop scanning at */
{
	REG int		token;	/* the token pointed to by prog */
	REG long	nmatched;/* counter, used during closure matching */ 
	REG int		closure;/* the token denoting the type of closure */
	int		maxalt;	/* maximum M_ALT(n) to use */
	long		from;	/* minimum number of matches in closure */
	long		to;	/* maximum number of matches in closure */
	CHAR		ch;	/* character from the buffer being searched */
	CHAR		*tail;	/* pointer into compiled regexp, after subexp */
	MARKBUF		oldhere;/* where *here pointed at start of call */
	MARKBUF		mark;	/* temporary mark */
	struct backoff_s {		/* this is used to construct a stack */
	  struct backoff_s *pop;	/* of partial matches, so that if we */
	  MARKBUF	   texttail;	/* have to back off a greedy closure,*/
	} *bent, *backoff = NULL;	/* we can do so efficiently.         */


#ifdef DEBUG_REGEXP
	static int	nest = 0;

	fprintf(stderr, "%*smatch(..., bol=%s, endtoken=\\)%d)\n",
		nest*4, "", bol ? "True" : "False", INDEX_OF(endtoken));
	nest++;
#endif /* DEBUG_REGEXP */

	/* remember the original position of *here */
	oldhere = *scanmark(here);

TryAgain:
#ifdef DEBUG_REGEXP
	tail = decompile(prog, endtoken);
	fprintf(stderr, "%*sprog=%s\n", nest*4-4, "", tail);
	safefree(tail);
	fprintf(stderr, "%*sstr=\"", nest*4-4, "");
	scandup(&tail, here);
	while (tail && *tail != '\n')
	{
		putc(*tail, stderr);
		scannext(&tail);
	}
	scanfree(&tail);
	fprintf(stderr, "\"\n");
#endif /* DEBUG_REGEXP */

	/* compare a single character or metacharacter */
	for (token = GET_META(prog); !IS_CLOSURE(token); prog++, token = GET_META(prog))
	{
		/* if we hit the end of the buffer, fail */
		if (!*here)
			goto Failure;

		switch (token)
		{
		/*case M_BEGLINE: can't happen; re->bol is used instead */
		  case M_ENDLINE:
			if (!*here || **here != '\n')
				goto Failure;
			break;

		  case M_BEGWORD:
			if (!bol || (*here && markoffset(scanmark(&*here))) != markoffset(str))
			{
				scanprev(&*here);
				ch = **here;
				scannext(&*here);
				if (ch == '_' || elvalnum(ch))
					goto Failure;
			}
			break;

		  case M_ENDWORD:
			ch = **here;
			if (ch == '_' || elvalnum(ch))
				goto Failure;
			break;

		  case M_EDGEWORD:
		  case M_NOTEDGEWORD:
			/* Check the preceding char & current one.  Count the
			 * number of letters in those two chars.
			 */
			ch = (**here=='_' || elvalnum(**here)) ? 1 : 0;
			if (bol || !*here || markoffset(scanmark(&*here)) == markoffset(str))
				ch = ' ';
			else
			{
				scanprev(&*here);
				ch += (**here=='_' || elvalnum(**here)) ? 1 : 0;
				scannext(&*here);
			}

			/* Odd means edge.  Fail if not the desired oddness */
			if ((ch & 1) != (token == M_EDGEWORD ? 1 : 0))
				goto Failure;
			break;

		  case M_LEAVECURSOR:
			re->leavep = markoffset(scanmark(&*here));
			break;

		  case M_ALT(0):
		  case M_ALT(1):
		  case M_ALT(2):
		  case M_ALT(3):
		  case M_ALT(4):
		  case M_ALT(5):
		  case M_ALT(6):
		  case M_ALT(7):
		  case M_ALT(8):
		  case M_ALT(9):
			/* locate the \) after this \| */
			for (tail = prog; GET_META(tail) != END_OF(token); tail++)
			{
			}
			tail--;

			/* recursively try matching the remainder of regexp */
			if (!match(re, str, tail, here, ElvFalse, endtoken))
				goto Success;

			/* failed -- try the alternative.  Note that we need
			 * to reset the text position to the start of this
			 * subexpression.  The prog position is already good.
			 */
			scanseek(here, marktmp(mark, re->buffer, re->startp[INDEX_OF(token)]));
			break;

		  case M_START(0):
		  case M_START(1):
		  case M_START(2):
		  case M_START(3):
		  case M_START(4):
		  case M_START(5):
		  case M_START(6):
		  case M_START(7):
		  case M_START(8):
		  case M_START(9):
			re->startp[INDEX_OF(token)] = markoffset(scanmark(here));
#ifdef DEBUG_REGEXP
			fprintf(stderr, "%*sre->startp[%d] = %ld\n",
				nest*4-4, "", INDEX_OF(token), re->startp[INDEX_OF(token)]);
#endif
			break;

		  case M_END(0):
		  case M_END(1):
		  case M_END(2):
		  case M_END(3):
		  case M_END(4):
		  case M_END(5):
		  case M_END(6):
		  case M_END(7):
		  case M_END(8):
		  case M_END(9):
			re->endp[INDEX_OF(token)] = markoffset(scanmark(here));
#ifdef DEBUG_REGEXP
			fprintf(stderr, "%*sre->endp[%d] = %ld\n",
				nest*4-4, "", INDEX_OF(token), re->endp[INDEX_OF(token)]);
#endif
			if (token == endtoken)
				goto Success;
			break;

		  default: /* literal, M_CLASS(n), M_ANY, or M_NUL */
		  	assert(*here != NULL);
			if (match1(re, **here, token) != 0)
				goto Failure;
			scannext(here);
		}
	}

	/* C L O S U R E */

	/* step 1: see what we have to match against, and move "prog" to point
	 * to the remainder of the compiled regexp.
	 */
	closure = token;
	prog++;
	switch (closure)
	{
	  case M_SPLAT:
	  case M_NGSPLAT:
		from = 0;
		to = INFINITY;
		break;

	  case M_PLUS:
	  case M_NGPLUS:
		from = 1;
		to = INFINITY;
		break;

	  case M_QMARK:
	  case M_NGQMARK:
		from = 0;
		to = 1;
		break;

	  case M_RANGE:
	  case M_NGRANGE:
	  default: /* "default:" label is just to keep the compiler happy */
		from = *prog++;
		to = *prog++;
		if (to == 255)
		{
			to = INFINITY;
		}
		break;

	}
	token = GET_META(prog);
	prog++;

	/* optimized the match-loop for single characters? */
	if (!IS_START(token))
	{
		/* step 2: see how many times we can match that token against the string */
		for (nmatched = 0;
		     nmatched < to
			&& *here
			&& match1(re, **here, token) == 0;
		     nmatched++, scannext(here))
		{
			/* if non-greedy and we've matched the minimum number,
			 * then check to see if the tail also matches.
			 * If so, we're done!
			 */
			if (!IS_GREEDY(closure)
			 && nmatched >= from
			 && !match(re, str, prog, here, ElvFalse, endtoken))
				goto Success;
		}

		/* step 3: try to match the remainder, and back off if it doesn't */
		if (!*here || !IS_GREEDY(closure))
			goto Failure;
		while (nmatched >= from && match(re, str, prog, here, ElvFalse, endtoken) != 0)
		{
			/* back off */
			nmatched--;
			scanprev(here);
			if (!*here)
				goto Failure;
		}

		/* so how did it work out? */
		if (nmatched < from)
			goto Failure;
	}
	else /* closure applied to complex subexpression */
	{
		/* Locate the portion of the program after the subexpression.
		 * We'll need it inside the loop, and it's more efficient to
		 * find it now.
		 */
		for (tail = prog; GET_META(tail) != END_OF(token); tail++)
		{
		}
		tail++;

		/* start the back-off stack for greedy searches */
		backoff = safealloc(1, sizeof *backoff);
		backoff->texttail = *scanmark(here);

		/* try to match the minimum number of times */
		for (nmatched = 0;
		     nmatched <= to
			&& *here
			&& !match(re, str, prog, here, ElvFalse, END_OF(token));
		     nmatched++)
		{
			/* for greedy matches, we may need to back off the
			 * longest match if we can't find a tail that matches.
			 * Since we don't want to repeat all-but-one of these
			 * looped matches if we can avoid it, we instead build
			 * a stack of the match positions.
			 */
			if (IS_GREEDY(closure)
			 && nmatched >= from)
			{
				bent = safealloc(1, sizeof *bent);
				bent->texttail = *scanmark(here);
				bent->pop = backoff;
				backoff = bent;
			}

#ifdef DEBUG_REGEXP
			if (!IS_GREEDY(closure) && nmatched >= from)
				fprintf(stderr, "%*sChecking non-greedy tail: ", nest*4-4, "");
#endif
			/* if we've matched the minimum number, then check to
			 * see if the tail also matches.  If so, we're done!
			 */
			if (!IS_GREEDY(closure)
			 && nmatched >= from
			 && !match(re, str, tail, here, ElvFalse, endtoken))
				goto Success;
		}

		/* At this point, we've matched the maximum number of complex
		 * subexpressions.  If it is less than the minimum, then we
		 * should fail.
		 */
		if (nmatched < from)
			goto Failure;

		/* For greedy searches, we still need to match the tail.
		 * If the tail doesn't match then we need to back off the
		 * search.
		 */
		if (IS_GREEDY(closure))
		{
#ifdef DEBUG_REGEXP
			fprintf(stderr, "%*sChecking greedy tail, nmatched=%ld, from=%ld, to=%ld...\n", nest*4-4, "", nmatched, from, to);
#endif
			while (match(re, str, tail, here, ElvFalse, endtoken))
			{
				/* mismatch, need to back off and try again */
				if (--nmatched < from)
					goto Failure;
				bent = backoff->pop;
				safefree(backoff);
				backoff = bent;
				scanseek(here, &backoff->texttail);
			}
			goto Success;
		}

		/* We tried all possible counts, without success */
		goto Failure;
	}

Success:
	/* discard the closure "back off" stack, if there is one */
	while (backoff)
	{
		bent = backoff->pop;
		safefree(backoff);
		backoff = bent;
	}

#ifdef DEBUG_REGEXP
	nest--;
	fprintf(stderr, "%*sYES!\n", nest*4, "");
	if (nest == 0)
		putc('\n', stderr);
#endif

	return 0;

Failure:
	/* discard the closure "back off" stack, if there is one */
	while (backoff)
	{
		bent = backoff->pop;
		safefree(backoff);
		backoff = bent;
	}

	/* Look for an alternative.  Beware of alternatives that are inside
	 * embedded subexpressions -- they depend on matched text that we
	 * didn't find, so we can't use them.
	 */
	maxalt = M_ALT(10); /* bigger than any real ALT */
	while ((!IS_ALT(token) || token >= maxalt) && token != endtoken)
	{
		if (IS_START(token) && maxalt == M_ALT(10))
			maxalt = M_ALT(INDEX_OF(token));
		token = GET_META(prog);
		prog++;
	}
	if (IS_ALT(token))
	{
#ifdef DEBUG_REGEXP
		fprintf(stderr, "%*sNO! Looping to try alternative \\|%d, textpos=%ld, maxalt=\\|%d\n",
			nest*4-4, "", INDEX_OF(token),
			re->startp[INDEX_OF(token)], INDEX_OF(maxalt));
#endif

		/* reset the text position to the start of the subexpression
		 * that contains that alternative operator.
		 */
		scanseek(here, marktmp(mark, re->buffer, re->startp[INDEX_OF(token)]));

		/* Go back to try it again.  (I apologize for the "goto") */
		goto TryAgain;
	}

	/* restore the "*here" scanning variable */
	scanseek(here, &oldhere);
#ifdef DEBUG_REGEXP
	nest--;
	fprintf(stderr, "%*sNO!\n\n", nest*4, "");
#endif
	return 1;
}



/* This function searches through a string for text that matches a regexp.
 * "re" is the compiled regular expression, "str" is the string to compare
 * against "re", and "bol" is a flag indicating whether "str" points to the
 * start of a line or not.  Returns 1 for match, or 0 for mismatch.
 */
int regexec(re, str, bol)
	regexp	*re;	/* a compiled regular expression */
	MARK	str;	/* a string to compare against the regexp */
	ELVBOOL	bol;	/* if ElvTrue, "str" is the beginning of a string */
{
	CHAR	*prog;	/* the entry point of re->program */
	int	len;	/* length of the string */
	CHAR	*here;	/* pointer used for scanning text */
#ifdef FEATURE_LITRE
	int	right;	/* contiguous characters to right of str */
	MARKBUF	m;
#endif

	/* Remember which buffer "str" comes from */
	re->buffer = markbuffer(str);

	/* find the remaining length of this line */
	scanalloc(&here, str);
	scandup(&prog, &here);
#ifdef FEATURE_LITRE
	right = scanright(&prog);
	for (len = 0; len < right && prog[len] != '\n'; len++)
	{
	}
	if (len >= right)
	{
		scanseek(&prog, marktmp(m, markbuffer(str), markoffset(str) + len));
		for (; prog && *prog != '\n'; scannext(&prog), len++)
		{
		}
		re->nextlinep = (prog
			? markoffset(scanmark(&prog)) + 1
			: o_bufchars(markbuffer(str)));
	}
	else
		re->nextlinep = markoffset(str) + len + 1;
#else
	for (len = 0; prog && *prog != '\n'; scannext(&prog), len++)
	{
	}
	re->nextlinep = (prog
		? markoffset(scanmark(&prog)) + 1
		: o_bufchars(markbuffer(str)));
#endif

	scanfree(&prog);

	/* if must start at the beginning of a line, and this isn't, then fail */
	if (re->bol && !bol)
	{
		scanfree(&here);
		return 0;
	}

	/* NOTE: If we ever support alternation (the \| metacharacter) then
	 * we'll need to reset startp[] and endp[] to -1L.
	 */
	re->leavep = -1;

	/* find the first token of the compiled regular expression */
	prog = re->program + 1 + 32 * re->program[0];

	/* search for the regexp in the string */
#ifdef FEATURE_LITRE
	if (re->literal && re->bol && !o_ignorecase && right >= re->minlen)
	{
		/* must match exactly, right here, and we know we have enough
		 * of this line for the entire match to be in contiguous memory.
		 */
		prog += 2;
		if (CHARncmp(prog, here, re->minlen))
		{
			/* didn't match */
			scanfree(&here);
			return 0;
		}

		/* hey, it did match!  Remember the endpoints */
		re->startp[0] = re->leavep = markoffset(str);
		re->endp[0] = re->startp[0] + re->minlen;
		scanfree(&here);
		return 1;
	}
	else if (re->literal && !o_ignorecase && right >= len)
	{
		/* The regexp must match exactly, anywhere before the end
		 * of this line.  We know the entire line is in contiguous
		 * memory, so we can use CHARncmp() to check for the string,
		 * and we can use here++ instead of scannext(&here) to
		 * increment the pointer in the for() statement.
		 */
		prog += 2;
		for (; len >= re->minlen; len--, here++)
		{
			if (*prog == *here && !CHARncmp(prog, here, re->minlen))
			{
				/* Found a match!  Remember the endpoints */
				re->startp[0] = re->leavep = markoffset(scanmark(&here));
				re->endp[0] = re->startp[0] + re->minlen;
				scanfree(&here);
				return 1;
			}
		}

		/* didn't match */
		scanfree(&here);
		return 0;
	}
	else 
#endif /* FEATURE_LITRE */
	if (re->bol)
	{
		/* must occur at BOL */
		if ((re->first
			&& match1(re, scanchar(str), re->first))/* wrong first letter? */
		 || len < re->minlen				/* not long enough? */
		 || match(re, str, prog, &here, bol, M_END(0)))	/* doesn't match? */
		{
			scanfree(&here);
			return 0;				/* THEN FAIL! */
		}
	}
	else if (!o_ignorecase)
	{
		/* can occur anywhere in the line, noignorecase */
		for (;
		     here && ((re->first && re->first != *here)
			|| match(re, str, prog, &here, bol, M_END(0)));
		     len--, bol = ElvFalse)
		{
			scannext(&here);
			if (!here || len <= re->minlen)
			{
				scanfree(&here);
				return 0;
			}
		}
	}
	else
	{
		/* can occur anywhere in the line, ignorecase */
		for (;
		     here && ((re->first && match1(re, *here, (int)re->first))
			|| match(re, str, prog, &here, bol, M_END(0)));
		     len--, bol = ElvFalse)
		{
			scannext(&here);
			if (!here || len <= re->minlen)
			{
				scanfree(&here);
				return 0;
			}
		}
	}
	scanfree(&here);

	/* if we didn't fail, then we must have succeeded */
	if (re->leavep == -1)
	{
		re->leavep = re->startp[0];
	}
	return 1;
}


/* Report a regexp error.  Metacharacters in the parameter should always be
 * given with a backslash -- this function deletes them if the "magicchar"
 * option indicates that no backslash is necessary.
 */
void regerror(str)
	char	*str;	/* an error message */
{
	char	buf[100];
	char	*build;

	/* delete backslashes before metacharacters mentioned in "magicchar" */
	for (build = buf; *str; str++)
	{
		if (*str != '\\'		  /* not a metacharacter     */
		 || str[1] <= ' '		  /* \ not followed by char  */
		 || !o_magicchar		  /* no metas can omit \     */
		 || !CHARchr(o_magicchar, str[1]))/* this meta can't omit \  */
			*build++ = *str;
	}
	*build = '\0';

	/* output the message */
	msg(MSG_ERROR, "[S]$1", buf);
}
