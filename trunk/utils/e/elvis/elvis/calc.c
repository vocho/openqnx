/* calc.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_calc[] = "$Id: calc.c,v 2.143 2003/10/18 04:47:18 steve Exp $";
#endif

#ifdef TRY
# undef FEATURE_CALC
# define FEATURE_CALC 1
#endif

#include <setjmp.h>
#ifdef TRY
# include <getopt.h>
# undef o_true
# define o_true "True"
# undef o_false
# define o_false "False"
# define elvdigit isdigit
# define elvupper isupper
# define elvlower islower
# define elvalnum isalnum
# define elvcntrl iscntrl
# define elvspace isspace
# define elvtoupper toupper
# define elvtolower tolower
# if USE_PROTOTYPES
    extern int isdigit(int c);
    extern int isupper(int c);
    extern int islower(int c);
    extern int isalnum(int c);
    extern int iscntrl(int c);
    extern int isspace(int c);
    extern int toupper(int c);
    extern int tolower(int c);
# endif
# define safedup(s)	strdup(s)
# define safefree(p)	free(p)
#endif /* TRY */


#ifdef FEATURE_CALC
# if USE_PROTOTYPES
static int copyname(CHAR *dest, CHAR *src, ELVBOOL num);
static ELVBOOL func(CHAR *name, CHAR *arg);
static ELVBOOL apply(void);
static CHAR *applyall(int prec);
static CHAR *maybeconcat(CHAR *build, int base, ELVBOOL asmsg);

#  ifdef FEATURE_ARRAY
static CHAR *nextelement(CHAR *element, CHAR **setref);
static CHAR *subnum(CHAR *cp, long nelem, long *from, long *to);
#  endif
# endif /* USE_PROTOTYPES */
#endif /* FEATURE_CALC */

#ifndef TRY
#ifdef FEATURE_CALC
static CHAR *feature[] =
{
# ifdef PROTOCOL_HTTP
	toCHAR("http"),
# endif
# ifdef PROTOCOL_FTP
	toCHAR("ftp"),
# endif
# ifdef FEATURE_ALIAS
	toCHAR("alias"),
# endif
# ifdef FEATURE_ARRAY
	toCHAR("array"),
# endif
# ifdef FEATURE_AUTOCMD
	toCHAR("autocmd"),
# endif
# ifdef FEATURE_BACKTICK
	toCHAR("backtick"),
# endif
# ifdef FEATURE_BROWSE
	toCHAR("browse"),
# endif
# ifdef FEATURE_CACHEDESC
	toCHAR("cachedesc"),
# endif
# ifdef FEATURE_CALC
	toCHAR("calc"),
# endif
# ifdef FEATURE_COMPLETE
	toCHAR("complete"),
# endif
# ifdef FEATURE_EQUALTILDE
	toCHAR("equaltilde"),
# endif
# ifdef FEATURE_FOLD
	toCHAR("fold"),
# endif
# ifdef FEATURE_G
	toCHAR("g"),
# endif
# ifdef FEATURE_HLOBJECT
	toCHAR("hlobject"),
# endif
# ifdef FEATURE_HLSEARCH
	toCHAR("hlsearch"),
# endif
# ifdef FEATURE_IMAGE
	toCHAR("image"),
# endif
# ifdef FEATURE_INCSEARCH
	toCHAR("incsearch"),
# endif
# ifdef FEATURE_LISTCHARS
	toCHAR("listchars"),
# endif
# ifdef FEATURE_LITRE
	toCHAR("litre"),
# endif
# ifdef FEATURE_LPR
	toCHAR("lpr"),
# endif
# ifdef FEATURE_MAKE
	toCHAR("make"),
# endif
# ifdef FEATURE_MAPDB
	toCHAR("mapdb"),
# endif
# ifdef FEATURE_MISC
	toCHAR("misc"),
# endif
# ifdef FEATURE_MKEXRC
	toCHAR("mkexrc"),
# endif
# ifdef FEATURE_NORMAL
	toCHAR("normal"),
# endif
# ifdef FEATURE_PROTO
	toCHAR("proto"),
# endif
# ifdef FEATURE_RAM
	toCHAR("ram"),
# endif
# ifdef FEATURE_RCSID
	toCHAR("rcsid"),
# endif
# ifdef FEATURE_REGION
	toCHAR("region"),
# endif
# ifdef FEATURE_SHOWTAG
	toCHAR("showtag"),
# endif
# ifdef FEATURE_SMARTARGS
	toCHAR("smartargs"),
# endif
# ifdef FEATURE_SPELL
	toCHAR("spell"),
# endif
# ifdef FEATURE_SPLIT
	toCHAR("split"),
# endif
# ifdef FEATURE_STDIN
	toCHAR("stdin"),
# endif
# ifdef FEATURE_TAGS
	toCHAR("tags"),
# endif
# ifdef FEATURE_TEXTOBJ
	toCHAR("textobj"),
# endif
# ifdef FEATURE_V
	toCHAR("v"),
# endif
# ifdef FEATURE_XFT
	toCHAR("xft"),
# endif
	NULL
};
#endif /* FEATURE_CALC */
#endif /* !TRY */

#ifdef FEATURE_CALC
/* This array describes the operators */
static struct
{
	char	*name;	/* name of the operator */
	short	prec;	/* natural precedence of the operator */
	char	code;	/* function code for applying operator */
	char	subcode;/* details, depend on function */
} opinfo[] =
{
	{"Func",1,	'f',	'('},
	{"Cat",	17,	'c',	' '},
	{"Sub",	18,	'a',	'['},
	{";",	2,	'c',	';'},
	{",",	2,	'c',	','},
	{"..",	3,	'c',	'.'},
	{"||",	6,	'b',	'|'},
	{"&&",	7,	'b',	'&'},
	{"!=",	11,	's',	'!'},
	{"==",	11,	's',	'='},
	{"<=",	12,	's',	'l'},
	{">=",	12,	's',	'g'},
	{"<<",	13,	'i',	'<'},
	{">>",	13,	'i',	'>'},
	{":",	5,	't',	':'},
	{"?",	4,	't',	'?'},
	{"|",	8,	'i',	'|'},
	{"^",	9,	'i',	'^'},
	{"&",	10,	'i',	'&'},
	{"=",	11,	's',	'='},
	{"<",	12,	's',	'<'},
	{">",	12,	's',	'>'},
	{"+",	14,	'i',	'+'},
	{"-",	14,	'i',	'-'},
	{"%",	15,	'i',	'%'},
	{"*",	15,	'i',	'*'},
	{"/",	15,	'i',	'/'},
	{"!",	16,	'u',	'!'},
	{"~",	16,	'u',	'~'},
};


/* The following variables are used during the evaluation of each expression */
static struct
{
	CHAR	*first;	/* first argument to operator */
	int	idx;	/* index into opinfo of the operator */
	int	prec;	/* precedence of operator */
} opstack[10];
static int	ops;	/* stack pointer for opstack[] */

/* The following variables are used for storing the parenthesis level */
static CHAR	*parstack[20];

/* ultimately, the result returned by calculate() */
static	CHAR	result[1024];
#define RESULT_END			(&result[QTY(result) - 3])
#define RESULT_AVAIL(from)		((int)(RESULT_END - (CHAR *)(from)))
#define RESULT_OVERFLOW(from, need)	(RESULT_AVAIL(from) < (int)(need))
#define UNSAFE				if (o_security == 'r') goto Unsafe

#endif /* FEATURE_CALC */


/* This function returns ElvTrue iff a string looks like an integer */
ELVBOOL calcnumber(str)
	CHAR	*str;	/* a nul-terminated string to check */
{
	ELVBOOL	dp = ElvFalse;	/* has decimal point been seen yet? */

	/* allow a leading "-" */
	if (*str == '-')
		str++;

	/* The only decimal number that can start with '0' is zero */
	if (str[0] == '0' && str[1])
		return ElvFalse;

	/* Require at least one digit, and don't allow any non-digits */
	do
	{
		if (*str == '.' && dp == ElvFalse && str[1])
			dp = ElvTrue;
		else if (!elvdigit(*str))
			return ElvFalse;
	} while (*++str);
	return ElvTrue;
}


#ifdef FEATURE_CALC
/* This function returns ElvTrue if passed a string which looks like a number,
 * and ElvFalse if it doesn't.  This function differs from calcnumber() in that
 * this one also converts octal numbers, hex numbers, and literal characters
 * to base ten.  Note that the length of the string may change.
 */
ELVBOOL calcbase10(str)
	CHAR	*str;
{
	long	num;
	int	i;

	/* do the easy tests first */
	if (calcnumber(str)) return ElvTrue;
	if (*str != '0' && *str != '\'') return ElvFalse;

	if (str[0] == '\'')
	{
		if (str[1] == '\\')
		{
			if (!str[2] || str[3] != '\'')
				return ElvFalse;
			switch (str[2])
			{
			  case '0':	num = 0;	break;
			  case 'a':	num = '\007';	break;
			  case 'b':	num = '\b';	break;
			  case 'E':
			  case 'e':	num = '\033';	break;
			  case 'f':	num = '\f';	break;
			  case 'n':	num = '\n';	break;
			  case 'r':	num = '\r';	break;
			  case 't':	num = '\t';	break;
			  default:	num = str[2];	break;
			}
			i = 4;
		}
		else
		{
			if (str[1] == '\\' || !str[1] || str[2] != '\'')
				return ElvFalse;
			num = str[1];
			i = 3;
		}
	}
	else if (str[1] == 'x')
	{
		/* is it hex? */
		for (i = 2, num = 0; ; i++)
		{
			if (str[i] >= '0' && str[i] <= '9')
				num = num * 16 + str[i] - '0';
			else if (str[i] >= 'a' && str[i] <= 'f')
				num = num * 16 + str[i] - 'a' + 10;
			else
				break;
		}
	}
	else
	{
		/* is it octal? */
		for (i = 1, num = 0; ; i++)
		{
			if (str[i] >= '0' && str[i] <= '7')
				num = num * 8 + str[i] - '0';
			else
				break;
		}
	}

	/* If we hit a problem before the end of the string, it isn't a number */
	if (str[i]) return ElvFalse;

	/* We have a number!  Convert to decimal */
	long2CHAR(str, num);
	return ElvTrue;
}
#endif /* FEATURE_CALC */


/* This function returns ElvFalse if the string looks like any kind of "false"
 * value, and ElvTrue otherwise.  The false values are "", "0", "false", "no",
 * and the value of the `false' option.
 */
ELVBOOL calctrue(str)
	CHAR	*str;	/* the sting to be checked */
{
	if (!str || !*str || !CHARcmp(str, toCHAR("0"))
		|| !CHARcmp(str, toCHAR("false")) || !CHARcmp(str, toCHAR("no"))
		|| (o_false && !CHARcmp(str, o_false)))
	{
		return ElvFalse;
	}
	return ElvTrue;
}


#ifdef FEATURE_CALC
# ifdef FEATURE_ARRAY
/* parse the next number in a subscript, and return a pointer to the point after
 * it.  If there is no next number, then return NULL.  Sets *from and *to to
 * the values, or to 1/nelem if a '.' was involved.
 */
static CHAR *subnum(cp, nelem, from, to)
	CHAR	*cp;	/* number field */
	long	nelem;	/* number of elements */
	long	*from;	/* where to store "from" */
	long	*to;	/* where to store "to" */
{
	ELVBOOL	neg;
	ELVBOOL	from1, tonelem;
	long	val;

	/* skip whitespace or commas */
	while (*cp && (elvspace(*cp) || *cp == ','))
		cp++;
	if (!*cp)
		return NULL;

	/* notice a "." indicating that all earlier items should be included */
	if ((from1 = (ELVBOOL)(*cp == '.')) == ElvTrue)
		cp++;

	/* notice a "-" sign */
	if ((neg = (ELVBOOL)(*cp == '-')) == ElvTrue)
		cp++;

	/* expect a digit or '?'.  If not a digit or '?' then fail */
	if (!elvdigit(*cp) && *cp != '?')
		return NULL;

	/* convert the number */
	if (*cp == '?')
	{
		val = rand() % nelem + 1;
		cp++;
	}
	else
	{
		for (val = 0; elvdigit(*cp); cp++)
			val = val * 10 + (*cp - '0');
	}

	/* notice a "." indicating that later items should be included */
	if ((tonelem = (ELVBOOL)(*cp == '.')) == ElvTrue)
		cp++;

	/* return the appropriate stuff */
	if (val > nelem)
		val = nelem + 1;
	else if (neg && val != 0)
		val = nelem + 1 - val;
	*from = from1 ? 1L : val;
	*to = tonelem ? nelem : val;
	return cp;
}

/* For a given array, calculate the start & end points of a subscript.
 * Stores the information in "chunks", and returns the delimiter.
 */
_CHAR_ calcsubscript(array, sub, max, chunks)
	CHAR	*array;	/* array, as a string with delimiters */
	CHAR	*sub;	/* subscript, as a string of numbers & other symbols */
	int	max;	/* sizeof of "chunks" output array */
	CHUNK	*chunks;/* output array */
{
 static CHAR	nstr[20];/* number of elements, as a string */
	long	nelem;	/* number of elements */
	int	chunk;	/* next element of chunks[] to use */
	CHAR	delim;	/* delimiter char, if not whitespace */
	long	start, end;/* index numbers of start & end of chunk */
	long	lt1, lt2 = 0;/* some other index number */
	CHAR	*cp;
	int	len;

	/* check for field name instead of subscripts */
	if (elvalpha(*sub))
	{
		/* check length -- if field name is longer than array, then
		 * there is no such field.
		 */
		chunks[0].ptr = NULL;
		len = CHARlen(sub);
		if ((int)CHARlen(array) <= len)
			return ' ';

		/* search for the name at the start of a comma-delimited item,
		 * followed by ':'.
		 */
		for (cp = array;
		     ((cp != array && cp[-1] != ',')
			|| cp[len] != ':'
			|| CHARncmp(cp, sub, len));
		     cp++)
		{
			/* if hit end without finding field, then fail */
			if (cp[len] == '\0')
				return ' ';
		}

		/* find the length of the field's value */
		for (chunks[0].ptr = cp += len + 1; *cp && *cp != ','; cp++)
		{
		}
		chunks[0].len = (int)(cp - chunks[0].ptr);
		chunks[1].ptr = NULL;
		return ' ';
	}

	/* check for delimiter at start of sub */
	if (!sub[0])
		delim = '\0';
	else if (sub[1] == ',')
	{
		if (!elvdigit(*sub))
			delim = sub[0], sub += 2;
		else
			delim = ' ';
	}
	else if (sub[0] == ',')
		delim = '\0';
	else if (!elvalnum(*sub) && !elvspace(*sub) && !CHARchr(toCHAR("?-."), *sub))
		delim = *sub++;
	else
		delim = ' ';

	/* count the elements */
	if (delim == '\0')
		nelem = CHARlen(array);
	else if (delim == ' ')
	{
		for (cp = array, nelem = 0; *cp; cp++)
			if ((cp == array || elvspace(cp[-1])) && !elvspace(*cp))
				nelem++;
	}
	else /* some other delimiter */
	{
		for (cp = array, nelem = 1; *cp; cp++)
			if (*cp == delim)
				nelem++;
	}
	long2CHAR(nstr, nelem);

	/* for each chunk... */
	for (chunk = 0; chunk < max - 1; chunk++)
	{
		/* get a number from subscripts string */
		sub = subnum(sub, nelem, &start, &end);
		if (!sub)
			break;

		/* combine following numbers as part of this chunk if possible*/
		if (start != 0)
		{
			while ((cp = subnum(sub, nelem, &lt1, &lt2)) != NULL
			    && end + 1 == lt1)
			{
				sub = cp;
				end = lt2;
			}
		}

		/* now we know this chunk extends from the start of the
		 * "from" item to the end of the "to" item.
		 */
		if (end < start || start > nelem)
		{
			/* backwards elements are ignored */
			chunk--;
		}
		else if (start == 0L)
		{
			/* number of elements, regardless of what "element" means */
			chunks[chunk].ptr = nstr;
			chunks[chunk].len = CHARlen(nstr);
		}
		else if (delim == '\0')
		{
			/* indexed by character */
			chunks[chunk].ptr = &array[start - 1];
			chunks[chunk].len = end - start + 1;
		}
		else if (delim == ' ')
		{
			/* whitespace-delimited */
			for (cp = array, lt1 = 0; *cp; cp++, lt2++)
			{
				if ((cp == array || elvspace(cp[-1]))
				  && !elvspace(*cp))
				{
					lt1++;
					if (lt1 == start)
					{
						chunks[chunk].ptr = cp;
						lt2 = 0L;
					}
				}
				else if (cp != array
					&& !elvspace(cp[-1])
					&& elvspace(*cp)
					&& lt1 == end)
				{
					
					break;
				}
			}
			chunks[chunk].len = lt2;
		}
		else
		{
			/* some other delimiter */
			if (start == 1)
				chunks[chunk].ptr = array;
			for (cp = array, lt1 = 1; *cp; cp++)
			{
				if (*cp == delim)
				{
					if (end == lt1)
					{
						chunks[chunk].len = (int)(cp - chunks[chunk].ptr);
						break;
					}
					lt1++;
					if (start == lt1)
						chunks[chunk].ptr = cp + 1;
				}
			}
			if (!*cp)
				chunks[chunk].len = cp - chunks[chunk].ptr;
		}
	}

	/* mark the end of the "chunks" array */
	chunks[chunk].ptr = NULL;

	/* return the delimiter */
	return delim;
}
# endif /* FEATURE_ARRAY */
	
/* This function copies characters up to the next non-alphanumeric character,
 * nul-terminates the copy, and returns the number of characters copied.
 */
static int copyname(dest, src, num)
	CHAR	*dest;	/* where to copy into */
	CHAR	*src;	/* start of alphanumeric string to copy from */
	ELVBOOL	num;	/* treat numbers specially? */
{
	int	i;

	/* copy until non-alphanumeric character is encountered */
	if (num && elvdigit(*src))
	{
		for (i = 0; elvdigit(*src); i++)
		{
			if (RESULT_OVERFLOW(dest, 2)) return 0;
			*dest++ = *src++;
		}
	}
	else
	{
		for (i = 0; elvalnum(*src) || *src == '_'; i++)
		{
			if (RESULT_OVERFLOW(dest, 2)) return 0;
			*dest++ = *src++;
		}
	}
	*dest = '\0';
	return i;
}

/* This function implements the built-in "functions".  The name indicates
 * which function should be performed, and arg is its only argument.  The
 * result should be a string, and it should be copied over the name; func
 * should then return ElvTrue to indicate success.  If the function fails for
 * any reason, func() should emit an error message and return ElvFalse.
 *
 * The functions supported are:
 *   strlen(string)	return the number of characters in the string.
 *   tolower(string)	returns a lowercase version of string.
 *   toupper(string)	returns an uppercase version of string.
 *   isnumber(string)	return "true" iff string is a decimal number
 *   htmlsafe(string)	return an HTML-encoded version of string
 *   quote(chars,str)	return a copy of str with backslashes before chars
 *   unquote(chars,str)	return a copy of str with backslashes before chars
 *   hex(number)	return a string representing the hex value of number
 *   octal(number)	return a string representing the octal value of number
 *   char(number)	return a single-character string
 *   exists(file)	return "true" iff file exists
 *   dirperm(file)	return a string indicating file attributes.
 *   dirfile(file)	return the filename part of a pathname (including ext)
 *   dirname(file)	return the directory part of a pathname.
 *   dirdir(file)	return the directory, like dirname(file).
 *   dirext(file)	return the filename extension.
 *   basename(file)	return the filename without extension.
 *   elvispath(file)	return the pathname of a file in elvis' path, or ""
 *   buffer(buf)	return "true" iff buffer exist
 *   window(buf)	return windowid if buffer is shown in a window
 *   newbuffer(buf)     create/truncate a buffer, and return its name
 *   feature(name)	return "true" iff the named feature is supported
 *   knownsyntax(file)	return language if the file's extension is in elvis.syn
 *   current(what)	return line, column, word, mode, next, prev, or tag
 *   line(buf,line)	return text of a line
 *   fold(buf,line)     return name of current fold, or "" if none
 *   color(face,attr)   return attributes for a given text face
 *   time(file)		return timestamp in "YYYY-MM-DDThh:mm:ss" format
 *   rand(number)	return a random number between 1 and number
 *   spell(word)        return alternate spellings, or "" if good
 *   spelltag(word)     return alternate spellings, or "" if good
 */
static ELVBOOL func(name, arg)
	CHAR	*name;	/* name of function to apply */
	CHAR	*arg;	/* the argument to the function */
{
	CHAR	*tmp;
	int	i;
#ifndef TRY
	int	j;
	char	*c;
	MARK	begin;
	MARKBUF	end;
	BUFFER	buf;
	WINDOW	win;
	ELVBOOL	oldsaveregexp, oldmagic, bol;
	regexp	*re;
	CHAR	*fg, *bg, *like;
	unsigned short bits;
#endif

	if (!CHARcmp(name, toCHAR("strlen")))
	{
		long2CHAR(name, (long)CHARlen(arg));
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("tolower")))
	{
		for (; *arg; arg++)
			*name++ = (elvupper(*arg) ? elvtolower(*arg) : *arg);
		*name = '\0';
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("toupper")))
	{
		for (; *arg; arg++)
			*name++ = (elvlower(*arg) ? elvtoupper(*arg) : *arg);
		*name = '\0';
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("isnumber")))
	{
		if (RESULT_OVERFLOW(name, 6)) goto Overflow;
		CHARcpy(name, calcnumber(arg) ? o_true : o_false);
		return ElvTrue;
	}
#ifndef TRY
	else if (!CHARcmp(name, toCHAR("list")))
	{
		/* count the displayed with of this string */
		for (tmp = arg, i = 0; *tmp; tmp++)
			if (elvcntrl(*tmp))
				i += 2;
			else
				i++;
		if (RESULT_OVERFLOW(name, i)) goto Overflow;

		/* Copy the arg, converting control chars to printable.
		 * The display string will overlap "name" on the left, and it
		 * may overlap the end of "arg" on the right.  To prevent
		 * overflow from damaging the arg during conversion, we must
		 * create a temporary copy of it.
		 */
		tmp = CHARdup(arg);
		for (i = j = 0; tmp[i]; i++)
			if (elvcntrl(tmp[i]))
			{
				name[j++] = '^';
				name[j++] = tmp[i] ^ '@';
			}
			else
				name[j++] = tmp[i];
		name[j] = '\0';
		safefree(tmp);
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("htmlsafe")))
	{
		for (i = 0, tmp = NULL; arg[i]; i++)
		{
			switch (arg[i])
			{
			  case '&':	buildstr(&tmp, "&amp;");	break;
			  case '<':	buildstr(&tmp, "&lt;");		break;
			  case '>':	buildstr(&tmp, "&gt;");		break;
			  case '"':	buildstr(&tmp, "&quot;");	break;
			  case '\t':	buildCHAR(&tmp, ' ');		break;
			  default:	buildCHAR(&tmp, arg[i]);
			}
		}
		if (tmp)
		{
			if (RESULT_OVERFLOW(name, CHARlen(tmp))) goto Overflow;
			CHARcpy(name, tmp);
			safefree(tmp);
		}
		else
		{
			*name = '\0';
		}
		return ElvTrue;
	}
#ifdef FEATURE_MISC
	else if (!CHARcmp(name, toCHAR("color")))
	{
		/* separate face name from attribute name */
		tmp = CHARchr(arg, ',');
		if (tmp)
			*tmp++ = '\0';

		/* decide what we're supposed to return */
		if (!tmp)
			j = 0;
		else if (*tmp == 'f')
			j = 1;
		else if (*tmp == 'b')
			j = 2;
		else if (*tmp == 'l')
			j = 3;
		else if (*tmp == 's')
			j = 4;
		else
			goto BadArgs;

		/* search for the named color.  Note that we don't use
		 * colorfind() because we don't want to add it if it hasn't
		 * been set.
		 */
		for (i = 1;
		     i < colornpermanent && CHARcmp(arg, colorinfo[i].name);
		     i++)
		{
		}
		if (i < colornpermanent)
		{
			/* found! */

			/* parse the color description, if necessary */
			tmp = colorinfo[i].descr;
			fg = bg = like = NULL;
			if (j != 0 && j != 4)
				colorparse(tmp, &fg, &bg, &like, &bits);

			/* choose which attribute to return */
			switch (j)
			{
			  case 1: tmp = fg;	break;
			  case 2: tmp = bg;	break;
			  case 3: tmp = like;	break;
			  case 4: tmp = (colorinfo[i].da.bits & COLOR_SET)
			  		? o_true
			  		: o_false;
						break;
			}

			/* return it */
			if (tmp)
			{
				if (RESULT_OVERFLOW(name, CHARlen(tmp)))
				{
					if (fg) safefree(fg);
					if (bg) safefree(bg);
					if (like) safefree(like);
					goto Overflow;
				}
				CHARcpy(name, tmp);
			}
			else
			{
				*name = '\0';
			}
			if (fg) safefree(fg);
			if (bg) safefree(bg);
			if (like) safefree(like);
		}
		else
		{
			/* not found -- return "" */
			*name = '\0';
		}
		return ElvTrue;
	}
#endif /* FEATURE_MISC */
	else if (!CHARcmp(name, toCHAR("time")))
	{
		if (RESULT_OVERFLOW(name, 20))
			goto Overflow;
		if (arg[0])
			UNSAFE;
		CHARcpy(name, toCHAR(dirtime(tochar8(arg))));
		return ElvTrue;
	}
#endif /* !TRY */
	else if (!CHARcmp(name, toCHAR("hex")))
	{
		if (!calcnumber(arg)) goto NeedNumber;
		if (RESULT_OVERFLOW(name, 10)) goto Overflow;
		sprintf((char *)name, "0x%lx", CHAR2long(arg));
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("octal")))
	{
		if (!calcnumber(arg)) goto NeedNumber;
		if (RESULT_OVERFLOW(name, 12)) goto Overflow;
		sprintf((char *)name, "0%lo", CHAR2long(arg));
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("char")))
	{
		if (!calcnumber(arg)) goto NeedNumber;
		*name++ = (CHAR)CHAR2long(arg);
		*name = '\0';
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("ascii")))
	{
		sprintf((char *)name, "%d", *arg);
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("dirext")))
	{
		/* if this is a URL, and it contains a '#' or '?' character,
		 * then truncate it there.
		 */
		if ((arg[3] == ':' || arg[4] == ':')
		 && ((tmp = CHARchr(arg, '#')) != NULL
		   || (tmp = CHARchr(arg, '?')) != NULL))
			*tmp = '\0';

		/* find the last '.' in the name */
		for (tmp = arg + CHARlen(arg); --tmp >= arg && elvalnum(*tmp); )
		{
		}
		if (*tmp != '.')
		{
			tmp = toCHAR("");
		}
		CHARcpy(name, tmp);
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("rand")))
	{
		if (calcnumber(arg))
		{
			long2CHAR(name, 1 + rand() % CHAR2long(arg));
		}
		else
		{
			/* count the comma-delimited values */
			for (i = 0, tmp = arg; tmp; tmp = CHARchr(tmp+1, ','))
				i++;

			/* choose one randomly */
			i = rand() % i;

			/* locate it, and return it */
			tmp = arg;
			if (i > 0)
				for ( ; i > 0; tmp = CHARchr(tmp, ',') + 1)
					i--;
			for (; *tmp && *tmp != ','; )
				*name++ = *tmp++;
			*name = '\0';
		}
		return ElvTrue;
	}
#ifndef TRY
	else if (!CHARcmp(name, toCHAR("quote")))
	{
		/* divide the arg into "chars" and "str" fields */
		tmp = CHARchr(arg, (CHAR)',');
		if (!tmp)
		{
			goto Need2Args;
		}
		*tmp++ = '\0';

		/* build a copy with backslashes */
		tmp = addquotes(arg, tmp);

		/* if the resulting string fits in buffer, then store it */
		if (RESULT_OVERFLOW(name, CHARlen(tmp)))
			goto Overflow;
		CHARcpy(name, tmp);
		safefree(tmp);
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("unquote")))
	{
		/* divide the arg into "chars" and "str" fields */
		tmp = CHARchr(arg, (CHAR)',');
		if (!tmp)
		{
			goto Need2Args;
		}
		*tmp++ = '\0';

		/* build a copy with backslashes */
		tmp = removequotes(arg, tmp);

		/* store the resulting string (it *will* fit) */
		CHARcpy(name, tmp);
		safefree(tmp);
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("exists")))
	{
		UNSAFE;

#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
		if (urlremote(tochar8(arg)))
		{
			CHARcpy(name, o_false);
		}
		else
#endif
		{
			switch (urlperm(tochar8(arg)))
			{
			  case DIR_INVALID:
			  case DIR_BADPATH:
			  case DIR_NOTFILE:
			  case DIR_NEW:
				CHARcpy(name, o_false);
				break;

			  default:
				CHARcpy(name, o_true);
				break;
			}
		}
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("dirperm")))
	{
		UNSAFE;

		switch (urlperm(tochar8(arg)))
		{
		  case DIR_INVALID:
			CHARcpy(name, toCHAR("invalid"));
			break;

		  case DIR_BADPATH:
			CHARcpy(name, toCHAR("badpath"));
			break;

		  case DIR_NOTFILE:
			CHARcpy(name, toCHAR("notfile"));
			break;

		  case DIR_DIRECTORY:
			CHARcpy(name, toCHAR("directory"));
			break;

		  case DIR_NEW:
			CHARcpy(name, toCHAR("new"));
			break;

		  case DIR_UNREADABLE:
			CHARcpy(name, toCHAR("unreadable"));
			break;

		  case DIR_READONLY:
			CHARcpy(name, toCHAR("readonly"));
			break;

		  case DIR_READWRITE:
			CHARcpy(name, toCHAR("readwrite"));
			break;
		}
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("dirfile")))
	{
		CHARcpy(name, toCHAR(dirfile(tochar8(arg))));
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("dirname")) || !CHARcmp(name, toCHAR("dirdir")))
	{
		CHARcpy(name, toCHAR(dirdir(tochar8(arg))));
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("basename")))
	{
		CHARcpy(name, toCHAR(dirfile(tochar8(arg))));
		/* find the last '.' in the name */
		for (tmp = name + CHARlen(name); --tmp >= name && elvalnum(*tmp); )
		{
		}
		if (*tmp == '.')
		{
			*tmp = '\0';
		}
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("fileeol")))
	{
		UNSAFE;
		CHARcpy(name, toCHAR(ioeol(tochar8(arg))));
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("elvispath")))
	{
		tmp = toCHAR(iopath(tochar8(o_elvispath), tochar8(arg), ElvFalse));
		if (!tmp)
			*name = '\0';
		else
		{
			if (RESULT_OVERFLOW(name, CHARlen(tmp))) goto Overflow;
			CHARcpy(name, tmp);
		}
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("getcwd")))
	{
		c = dircwd();
		if (RESULT_OVERFLOW(name, strlen(c))) goto Overflow;
		CHARcpy(name, toCHAR(c));
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("absolute")))
	{
		c = ioabsolute(tochar8(arg));
		if (RESULT_OVERFLOW(name, strlen(c))) goto Overflow;
		CHARcpy(name, toCHAR(c));
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("buffer")))
	{
		CHARcpy(name, buffind(arg) ? o_true : o_false);
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("window")))
	{
		buf = buffind(arg);
		win = (buf ? winofbuf(NULL, buf) : NULL);
		if (win)
			long2CHAR(name, o_windowid(win));
		else
			*name = '\0';
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("newbuffer")))
	{
		if (*arg && (buf = buffind(arg)) != NULL)
		{
			/* fail -- tried to create a buffer that exists */
			*name = '\0';
			return ElvTrue;
		}
		buf = bufalloc(*arg ? arg : NULL, 0, ElvFalse);
		assert(buf);
		CHARcpy(name, o_bufname(buf));
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("alias")))
	{
# ifdef FEATURE_ALIAS
		c = exisalias(tochar8(arg), ElvTrue);
		tmp = (c ? o_true : o_false);
# else
		tmp = o_false;
# endif
		if (RESULT_OVERFLOW(name, CHARlen(tmp))) goto Overflow;
		CHARcpy(name, tmp);
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("feature")))
	{
		/* for now */
		CHARcpy(name, o_false);

		/* is it the name of a supported display mode? */
		for (i = 0; allmodes[i] != &dmnormal; i++)
		{
			if (!CHARcmp(toCHAR(allmodes[i]->name), arg))
			{
				CHARcpy(name, o_true);
				return ElvTrue;
			}
		}

		/* is it the name of a supported protocol */
		for (i = 0; feature[i] && CHARcmp(feature[i], arg); i++)
		{
		}
		if (feature[i])
			CHARcpy(name, o_true);

		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("knownsyntax")))
	{
#ifdef DISPLAY_SYNTAX
		tmp = descr_known(tochar8(arg), SYNTAX_FILE);
		if (!tmp)
			*name = '\0';
		else if (RESULT_OVERFLOW(name, CHARlen(tmp)))
			goto Overflow;
		else
			CHARcpy(name, tmp);
#else
		*name = '\0'; /* no syntax modes are supported */
#endif
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("knownmarkup")))
	{
#ifdef DISPLAY_MARKUP
		tmp = descr_known(tochar8(arg), MARKUP_FILE);
		if (!tmp)
			*name = '\0';
		else if (RESULT_OVERFLOW(name, CHARlen(tmp)))
			goto Overflow;
		else
			CHARcpy(name, tmp);
#else
		*name = '\0'; /* no syntax modes are supported */
#endif
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("current")))
	{
		/* The default return value is an empty string */
		*name = '\0';

		/* Other possible values depend on the arg */
		switch (*arg)
		{
		  case '/':	/* regular expression */
			/* if no default window, then do nothing */
			if (!windefault)
				break;

			/* An optimization: If regexp is /./ then get a single
			 * character from the cursor position (unless newline)
			 */
			if (arg[1] == '.' && !arg[2])
			{
				*name = scanchar(windefault->cursor);
				if (*name == '\n')
					*name = '\0';
				else
					name[1] = '\0';
				break;
			}

			/* parse the regular expression */
			oldsaveregexp = o_saveregexp;
			oldmagic = o_magic;
			o_saveregexp = ElvFalse;
			o_magic = ElvTrue;
			++arg;
			re = regcomp(arg, windefault->cursor);
			o_saveregexp = oldsaveregexp;
			if (!re)
			{
				/* error message already given */
				o_magic = oldmagic;
				return ElvFalse;
			}

			/* Search for matches, starting at the beginning of
			 * the line.  Stop when we find one that ends after
			 * the cursor position.
			 */
			begin = markdup((*dmnormal.move)(
				windefault, windefault->cursor, 0L, 0, ElvTrue));
			for (bol = ElvTrue;
			     markoffset(begin) <= markoffset(windefault->cursor)
				&& regexec(re, begin, bol);
			     bol = ElvFalse)
			{
				/* Does this match include the cursor? */
				if (re->startp[0] <= markoffset(windefault->cursor)
				 && markoffset(windefault->cursor) < re->endp[0])
				{
					/* Yes!  Copy the matching text */
					tmp = regsub(re, toCHAR("&"), ElvFalse);
					CHARncpy(name, tmp, RESULT_AVAIL(name));
					safefree(tmp);
					break;
				}

				/* This match doesn't include the cursor, so
				 * repeat the search after the match.
				 */
				marksetoffset(begin, re->endp[0]);
				if (re->startp[0] == re->endp[0])
					markaddoffset(begin, 1L);
			}

			/* don't need the regexp or mark anymore */
			safefree(re);
			markfree(begin);
			o_magic = oldmagic;
			break;

		  case 'l':	/* line number */
			if (windefault)
				sprintf(tochar8(name), "%ld", markline(windefault->cursor));
			break;

		  case 'c':	/* column number */
			if (windefault)
				sprintf(tochar8(name), "%ld", (*windefault->md->mark2col)(windefault, windefault->cursor, viiscmd(windefault)) + 1);
			break;

		  case 'f':	/* text face */
			if (windefault)
			{
				i = windefault->di->cursface;
				if (i < colornpermanent && colorinfo[i].name)
					CHARcpy(name, colorinfo[i].name);
			}
			break;

		  case 'w':	/* word at cursor */
		  	if (windefault)
		  	{
				end = *windefault->cursor;
				begin = wordatcursor(&end, (ELVBOOL)(arg[1] == 's'));
				if (begin && RESULT_OVERFLOW(name, markoffset(&end) - markoffset(begin)))
					goto Overflow;
				if (begin)
				{
					scanalloc(&tmp, begin);
					for (i = (int)(markoffset(&end) - markoffset(begin)); i > 0; i--)
					{
						*name++ = *tmp;
						scannext(&tmp);
					}
					scanfree(&tmp);
					*name = '\0';
				}
			}
			break;

		  case 'm':	/* mode */
		  	if (windefault && !windefault->state->acton)
		  	{
				for (c = windefault->state->modename; *c; c++)
				{
					if (*c != ' ')
						*name++ = elvtolower(*c);
				}
			}
			break;

		  case 's':	/* visible selection */
		  	if (windefault && !windefault->state->acton && windefault->seltop)
		  	{
		  		switch (windefault->seltype)
		  		{
		  		  case 'c':
		  			CHARcpy(name, toCHAR("character"));
		  			break;

		  		  case 'r':
		  			CHARcpy(name, toCHAR("rectangle"));
		  			break;

		  		  default:
		  		  	CHARcpy(name, toCHAR("line"));
		  		}
		  	}
		  	break;

		  case 'n':	/* next arg */
			if (arglist[argnext])
			{
				if (RESULT_OVERFLOW(name, strlen(arglist[argnext])))
					goto Overflow;
				for (c = arglist[argnext]; *c; )
				{
					*name++ = *c++;
				}
				*name = '\0';
			}
			break;

		  case 'p':	/* previous arg */
			if (argnext >= 2)
			{
				if (RESULT_OVERFLOW(name, strlen(arglist[argnext - 2])))
					goto Overflow;
				for (c = arglist[argnext - 2]; *c; )
				{
					*name++ = *c++;
				}
				*name = '\0';
			}
			break;

		  case 't':	/* tag or tagstack */
			tmp = NULL;
#ifdef FEATURE_SHOWTAG
			if (!CHARcmp(arg, "tag"))
				tmp = telabel(windefault->cursor);
			else
#endif
			if (windefault && windefault->tagstack->origin)
				tmp = o_bufname(markbuffer(windefault->tagstack->origin));
			if (tmp)
			{
				if (RESULT_OVERFLOW(name, CHARlen(tmp)))
					goto Overflow;
				CHARcpy(name, tmp);
			}
			break;

		  case 'b':
			if (~colorinfo[COLOR_FONT_NORMAL].da.bits & COLOR_BG)
				tmp = toCHAR("");
			else if (colorinfo[COLOR_FONT_NORMAL].da.bg_rgb[0] +
				 colorinfo[COLOR_FONT_NORMAL].da.bg_rgb[1] +
				 colorinfo[COLOR_FONT_NORMAL].da.bg_rgb[2]>=384)
				tmp = toCHAR("light");
			else
				tmp = toCHAR("dark");
			if (RESULT_OVERFLOW(name, CHARlen(tmp)))
				goto Overflow;
			CHARcpy(name, tmp);
			break;

#ifdef FEATURE_REGION
		  case 'r':	/* region or rcomment*/
			tmp = NULL;
			if (windefault)
			{
				region_t *r = regionfind(windefault->cursor);
				if (r && arg[1] == 'c')
					tmp = r->comment;
				else if (r)
					tmp = colorinfo[(int)r->font].name;
			}
			if (tmp)
			{
				if (RESULT_OVERFLOW(name, CHARlen(tmp)))
					goto Overflow;
				CHARcpy(name, tmp);
			}
			break;
#endif
		}
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("line"))
#ifdef FEATURE_FOLD
	      || !CHARcmp(name, toCHAR("folded"))
	      || !CHARcmp(name, toCHAR("unfolded"))
#endif
					       )
	{
		if (!*arg)
		{
			/* no arguments, use current line of current file */
			if (!windefault)
				goto NeedWindefault;
			begin = markdup(windefault->cursor);
			(void)marksetline(begin, markline(begin));
		}
		else if (calcnumber(arg))
		{
			/* just a number, use given line of current file */
			if (!windefault)
				goto NeedWindefault;
			begin = markdup(windefault->cursor);
			if (marksetline(begin, atol(tochar8(arg))) == NULL)
				goto BadArgs;
		}
		else
		{
			/* given name & number, I hope! */
			tmp = CHARrchr(arg, ',');
			if (!tmp)
				goto BadArgs;
			*tmp++ = '\0';
			end.buffer = buffind(arg);
			if (!end.buffer
			 || !calcnumber(tmp)
			 || marksetline(&end, atol(tochar8(tmp))) == NULL)
			
				goto BadArgs;
			begin = markdup(&end);
		}

		/* one way or another, "begin" is now set to the start of the
		 * line.  Use it as appropriate for this function.
		 */
#ifdef FEATURE_FOLD
		if (*name == 'f' || *name == 'u')
		{
			FOLD	fold = foldmark(begin, (ELVBOOL)(*name == 'f'));

			/* copy FOLD name into result buffer */
			if (fold)
			{
				if (RESULT_OVERFLOW(name, CHARlen(fold->name) + 10))
					goto Overflow;
				CHARcpy(name, fold->name);
			}
			else
				*name = '\0';
		}
		else
#endif
		{
			/* copy text into result buffer */
			for (scanalloc(&tmp, begin);
			     tmp && *tmp != '\n' && RESULT_AVAIL(name) > 10;
			     scannext(&tmp))
			{
				*name++ = *tmp;
			}
			scanfree(&tmp);
			*name = '\0';
		}
		return ElvTrue;
	}
	else if (!CHARcmp(name, toCHAR("shell")))
	{
		/* This is unsafe.  We can't use the UNSAFE macro, though,
		 * because this particular function is unsafe even if
		 * security=safer; it doesn't depend on security=restricted.
		 */
		if (o_security != 'n') goto Unsafe;

		/* Insert a '!' at the front of arg.  (This is safe
		 * since we know that "shell\0" appears before it.)
		 */
		*--arg = '!';

		/* Run the command and read its output */
		if (!ioopen(tochar8(arg), 'r', ElvFalse, ElvFalse, 't'))
			return ElvFalse;
		arg = name;
		while ((i = ioread(arg,RESULT_AVAIL(name))) > 0)
		{
			if (RESULT_OVERFLOW(arg, i + 1))
			{
				ioclose();
				goto Overflow;
			}
			arg += i;
		}
		ioclose();

		/* Remove the last newline */
		if (arg != name && arg[-1] == '\n')
			arg--;
		*arg = '\0';
		return ElvTrue;
	}
# ifdef FEATURE_SPELL
	else if (!CHARcmp(name, toCHAR("spell"))
	      || !CHARcmp(name, toCHAR("spelltag")))
	{
		spellfix(arg, name, RESULT_AVAIL(name) - 100, (ELVBOOL)(name[5] != '\0'));
		return ElvTrue;
	}
# endif /* FEATURE_SPELL */

#endif /* not TRY */

#ifdef TRY
	msg(MSG_ERROR, "unknown function %s", name);
#else
	msg(MSG_ERROR, "[S]unknown function $1", name);
#endif
	return ElvFalse;

NeedNumber:
#ifdef TRY
	msg(MSG_ERROR, "%s requires a numeric argument", name);
#else
	msg(MSG_ERROR, "[S]$1 requires a numeric argument", name);
#endif
	return ElvFalse;

#ifndef TRY
Need2Args:
	msg(MSG_ERROR, "[S]$1 requires two arguments", name);
	return ElvFalse;
#endif

Overflow:
	msg(MSG_ERROR, "result too long");
	return ElvFalse;

#ifndef TRY
NeedWindefault:
	msg(MSG_ERROR, "[S]$1 requires both a buffer name and a line number when there is no window", name);
	return ElvFalse;

BadArgs:
	msg(MSG_ERROR, "[S]$1 called with bad arguments", name);
	return ElvFalse;

Unsafe:
	msg(MSG_ERROR, "[S]$1 is unsafe", name);
	return ElvFalse;
#endif

}


/* Search for an element within a set.  If not found, then return NULL.
 * If found, then return a pointer into the set, pointing at the character
 * after the element's name.  That should be ':' if it has a value.
 */
CHAR *calcelement(set, element)
	CHAR	*set;	/* the set to scan */
	CHAR	*element;/* element, terminated with NUL, comma, or colon */
{
	int	len;

	/* if set is empty then fail */
	if (!set || !*set)
		return NULL;

	/* find the length of the element name */
	for (len = 0;
	     element[len] && element[len] != ',' && element[len] != ':';
	     len++)
	{
	}

	/* never find "" */
	if (len == 0)
		return NULL;

	/* scan the set */
	do
	{
		/* is this it? */
		if (!CHARncmp(element, set, len)
		 && (!set[len] || set[len] == ',' || set[len] == ':'))
			return set + len;

		/* look for the next element in set */
		set = CHARchr(set, ',');
	} while (set++);

	/* didn't find it */
	return NULL;
}

#ifdef FEATURE_ARRAY
/* Return a pointer to the next element in a list, or "" if no more elements.
 * Also, if setref is non-NULL, then insert this element into the set that
 * (*setref) points to.
 */
static CHAR *nextelement(element, setref)
	CHAR	*element; /* pointer to an element within a set */
	CHAR	**setref; /* NULL, or reference to set which will have element added */
{
	int	len;
	int	setlen = 0;
	CHAR	*scan;

	/* Find the length of the element name and optional value.  If we have
	 * a set to add this to, then make sure the set has enough memory to
	 * hold the new value.
	 */
	setlen = 0;
	if (setref && *setref)
	{
		setlen = buildCHAR(setref, ',') - 1;
	}
	for (len = 0; element[len] && element[len] != ','; len++)
	{
		if (setref)
			(void)buildCHAR(setref, element[len]);
	}
	if (len == 0)
		return element + 1;

	/* Decide where to insert the new element */
	if (setlen > 0 && setref)
	{
		/* look for an element in *setref > this element */
		for (scan = *setref; scan && CHARncmp(scan, element, len) <= 0;)
		{
			scan = CHARchr(scan, ',');
			if (scan)
				scan++;
		}

		/* Insert it, if we found a spot.  (Otherwise it should be
		 * appended, but we already did that when we counted the
		 * element's length.)
		 */
		if (scan)
		{
			/* compute the length of the tail */
			setlen -= (int)(scan - *setref);

			/* move the tail to make room for the element */
			memmove(scan + len + 1, scan, setlen * sizeof(CHAR));

			/* copy the element into the set */
			memcpy(scan, element, len);

			/* append a comma */
			scan[len] = ',';
		}
	}

	/* Move past the element */
	element += len;
	if (*element)
		element++;
	return element;
}

/* Perform an operation on two sets, and return the result in a dynamically-
 * allocated string.  If the result is the empty set, then return NULL.
 * The sets are comma-delimited lists of names, or name:value pairs.  Only
 * the names matter, though this function is clever enough to consistently
 * keep the values from the right operand.
 */
CHAR *calcset(left, op, right)
	CHAR	*left;	/* left set */
	_CHAR_	op;	/* operator: & intersect, | union, ^ difference */
	CHAR	*right;	/* right set */
{
	CHAR *set = NULL;
	CHAR *scan;

	switch (op)
	{
	  case '&': /* INTERSECTION */
		for (scan = right; *scan; )
			scan = nextelement(scan, calcelement(left, scan) ? &set : NULL);
		break;

	  case '|': /* UNION */
		for (scan = left; *scan; )
			scan = nextelement(scan, calcelement(right, scan) ? NULL : &set);
		for (scan = right; *scan; )
			scan = nextelement(scan, &set);
		break;

	  case '^': /* DIFFERENCE */
		for (scan = left; *scan; )
			scan = nextelement(scan, calcelement(right, scan) ? NULL : &set);
		break;
	}

	return set;
}
#endif /* FEATURE_ARRAY */

/* This function applies a single operator.  Returns ElvFalse on error.  Its
 * side effects are that it decrements the "ops" counter, and alters the
 * contents of the result buffer.
 */
static ELVBOOL apply()
{
	long	i, j;
	CHAR	*first, *second, *third;
	char	subcode;

#ifdef FEATURE_ARRAY
	CHAR	delim;
	CHUNK	chunks[20];
#endif

	assert(ops >= 1);

	second = opstack[ops--].first;
	first = opstack[ops].first;
	subcode = opinfo[opstack[ops].idx].subcode;
	switch (opinfo[opstack[ops].idx].code)
	{
	  case 'u':	/* unary operators */
		/* Unary operators depend only on their second argument.
		 * The result is concatenated to the first argument, which
		 * is normally an empty string.
		 */
		if (subcode == '!')
		{
			(void)CHARcat(first, calctrue(second) ? o_false : o_true);
		}
		else /* '~' */
		{
			if (calcnumber(second))
			{
				/* bitwise negation */
				long2CHAR(first + CHARlen(first), ~CHAR2long(second));
			}
			else
			{
				/* stuff a ~ between the strings */
				second[-1] = '~';
			}
		}
		break;

	  case 'i': /* integer operators */
		/* If either argument is a non-number, then concatenate them
		 * with the operator between them.  This is tricky because
		 * the << and >> operators are too large to simply replace
		 * the '\0' between the strings.
		 */
		if (!calcnumber(first) || !calcnumber(second))
		{
			if (subcode == '<' || subcode == '>')
			{
				/* As a special case, "string << number" and
				 * "string >> number" truncate the string to
				 * the length given by the number, keeping
				 * characters from the left or right.
				 */
				if (calcnumber(second))
				{
					/* convert arguments */
					i = CHARlen(first);
					j = CHAR2long(second);

					/* make sure this width wouldn't
					 * overflow the result buffer.
					 */
					if (RESULT_OVERFLOW(first, j))
					{
						msg(MSG_ERROR, "result too long");
						return ElvFalse;
					}

					/* Pad or truncate */
					if (subcode == '<')
					{
						/* Pad or truncate, keeping
						 * chars on left.  The first
						 * arg's characters are already
						 * in the right place, so we
						 * don't need to copy them.
						 * Just pad if first is short.
						 */
						while (i < j)
							first[i++] = ' ';
						first[j] = '\0';
					}
					else
					{
						if (i < j)
						{
							/* String needs to be
							 * padded.  Shift it
							 * to right, and then
							 * pad on the left.
							 */
							first[j] = '\0';
							while (i > 0)
								first[--j] = first[--i];
							while (j > 0)
								first[--j] = ' ';
						}
						else if (i > j)
						{
							/* String needs to be
							 * truncated.  Shift it
							 * to the left.
							 */
							CHARcpy(first, first + i - j);
						}
					}
					break;
				}
				msg(MSG_WARNING, "<< and >> only partially implemented");
			}
			else if (subcode == '*' && calcnumber(second))
			{
				/* Expressions of the form "string * number"
				 * return multiple copies of the string.
				 */

				/* convert arguments */
				j = CHAR2long(second);
				if (j <= 0)
				{
					/* trivial case */
					*first = '\0';
					break;
				}
				i = CHARlen(first);

				/* make sure this width wouldn't
				 * overflow the result buffer.
				 */
				if (RESULT_OVERFLOW(first, j * i))
				{
					msg(MSG_ERROR, "result too long");
					return ElvFalse;
				}

				/* The first copy is already there; we need
				 * to make j-1 more copies.
				 */
				while (--j > 0)
				{
					second = first + i;
					CHARncpy(second, first, (int)i);
					first = second;
				}
				first[i] = '\0';
				break;
			}
#ifdef FEATURE_ARRAY
			else if (subcode == '&' || subcode == '|' || subcode == '^')
			{
				third = calcset(first, subcode, second);
				if (third)
				{
					CHARcpy(first, third);
					safefree(third);
				}
				else
					*first = '\0';
				break;
			}
#endif /* FEATURE_ARRAY */
#ifndef TRY
			else if (subcode == '/')
			{
				/* When the / operator is passed strings as
				 * arguments, it contatenates them as a
				 * directory name and a file name.
				 */
				CHARcpy(first, toCHAR(dirpath(tochar8(first), tochar8(second))));
				break;
			}
#endif
			second[-1] = subcode;
		}
		else
		{
			i = CHAR2long(first);
			j = CHAR2long(second);
			switch (subcode)
			{
			  case '*':	i *= j;		break;
			  case '+':	i += j;		break;
			  case '-':	i -= j;		break;
			  case '<':	i <<= j;	break;
			  case '>':	i >>= j;	break;
			  case '&':	i &= j;		break;
			  case '^':	i ^= j;		break;
			  case '|':	i |= j;		break;
			  case '/': 	if (j == 0) goto DivZero;
					i /= j;		break;
			  case '%':	if (j == 0) goto DivZero;
					i %= j;		break;
			}
			long2CHAR(first, i);
		}
		break;

#ifdef FEATURE_ARRAY
	  case 'a': /* array subscript */
		/* first check for an valueless named element */
		if (elvalpha(*second))
		{
			third = calcelement(first, second);
			if (third && *third != ':')
			{
				CHARcpy(first, o_true);
				break;
			}
		}

		/* do it the usual way */
		third = CHARdup(first);
		delim = calcsubscript(third, second, QTY(chunks), chunks);
		for (i = 0; chunks[i].ptr; i++)
		{
			/* if not enough room, then fail */
			if (RESULT_OVERFLOW(first, chunks[i].len + 10))
			{
				msg(MSG_ERROR, "result too long");
				safefree(third);
				return ElvFalse;
			}

			/* add a delimiter between chunks */
			if (delim && i > 0)
				*first++ = delim;

			/*  add this chunk */
			CHARncpy(first, chunks[i].ptr, chunks[i].len);
			first += chunks[i].len;
		}
		*first = '\0';
		safefree(third);
		break;
#endif

	  case 's': /* string or integer comparison operators */
		/* if both arguments look like numbers, then compare
		 * numerically; else compare as strings.
		 */
		if (calcnumber(first) && calcnumber(second))
		{
			i = CHAR2long(first) - CHAR2long(second);
		}
		else
		{
			i = CHARcmp(first, second);
		}
		switch (subcode)
		{
		  case '<':	i = (i < 0);	break;
		  case 'l':	i = (i <= 0);	break;
		  case '>':	i = (i > 0);	break;
		  case 'g':	i = (i >= 0);	break;
		  case '=':	i = (i == 0);	break;
		  case '!':	i = (i != 0);	break;
		}
		(void)CHARcpy(first, toCHAR(i ? o_true : o_false));
		break;

	  case 'c': /* concatenation operators */
		switch (subcode)
		{
		  case ' ':
			if (*first && *second && !elvspace(*second) && !elvspace(second[-2]))
			{
				second[-1] = ' ';
				break;
			}
			/* else fall through for spaceless concatenation */

		  case ';':
			memmove(second - 1, second, sizeof(CHAR) * (CHARlen(second) + 1));
			break;

		  case '.':
			if (calcnumber(first) && calcnumber(second))
			{
				/* if second < first, then return "" */
				i = CHAR2long(first);
				j = CHAR2long(second);
				if (j < i)
				{
					*first = '\0';
					break;
				}

				/* otherwise, count from first to second */
				for (; i <= j; i++)
				{
					if (RESULT_OVERFLOW(first, 20))
						goto Overflow;
					long2CHAR(first, i);
					first += CHARlen(first);
					if (i < j)
						*first++ = ' ';
				}
				break;
			}
			/* else fall through for non-numbers */

		  default:
			second[-1] = subcode;
		}
		break;

	  case 'b': /* boolean operators */
		if (subcode == '&')
			i = (long)calctrue(first);
		else /* subcode == '|' */
			i = !(long)calctrue(first);
		if (i)
			while ((*first++ = *second++) != '\0')
			{
			}
		break;

	  case 't': /* ternary operator */
		/* This should be either (bool ? string : string) if we're
		 * evaluating a ':', or just (bool ? string) if we're
		 * evaluating a '?'.  The '?' and ':' operators are parsed as
		 * if they were separate binary operators.  The '?' has a
		 * slightly lower precedence than ':', so if we're evaluating
		 * a ':' we can expect the '?' to be the preceding operator on
		 * the stack.  That's very important!
		 */
		if (subcode == ':')
		{
			/* complain if not after a '?' */
			if (ops < 1 || opstack[ops - 1].prec != opstack[ops].prec - 1)
			{
				/* on second thought, it might be handy to make
				 * a binary ":" operator concatenate its args
				 * with an OSPATHDELIM between them.
				 */
				second[-1] = OSPATHDELIM;
				return ElvTrue;
			}

			/* shift the arguments */
			third = second;
			second = first;
			first = opstack[--ops].first;
		}
		else
		{
			/* (bool ? string) is legal -- assume third arg is "" */
			third = toCHAR("");
		}

		/* replace the first boolean with either second or third arg */
		CHARcpy(first, calctrue(first) ? second : third);
		break;

	  case 'f': /* functions */
		/* use func() to apply the function. */
		return func(first, second);
	}
	return ElvTrue;

DivZero:
	msg(MSG_ERROR, "division by 0");
	return ElvFalse;

Overflow:
	msg(MSG_ERROR, "result too long");
	return ElvFalse;
}

/* This function iteratively applies all preceding operators with a precedence
 * no lower than some given level.  Leaves "result" and "ops" altered.  Returns
 * a pointer to the end of the result, or NULL if error.
 */
static CHAR *applyall(prec)
	int	prec;	/* lowest precedence to apply */
{
	while (ops > 0 && opstack[ops - 1].prec >= prec)
	{
		if (!apply())
		{
			return (CHAR *)0;
		}
	}
	return opstack[ops].first + CHARlen(opstack[ops].first);
}


/* Push a concatenation operator onto the opstack, if there is a non-empty
 * first argument.  This should be called from calculate() before parsing
 * anything except an operator or closing parenthesis.  It returns the new
 * value of build.
 */
static CHAR *maybeconcat(build, base, asmsg)
	CHAR	*build;	/* current argument */
	int	base;	/* precedence, influenced by parentheses */
	ELVBOOL	asmsg;	/* using "simpler syntax" ? */
{
	int	prec;	/* precedence of this concatenation */

	/* if first arg would be empty, then do nothing */
	if (build == opstack[ops].first)
		return build;

	/* if using "simpler syntax" and outside of parentheses, do nothing */
	if (asmsg && base == 0)
		return build;

	/* Otherwise we have an implied concatenation operator.  Apply any
	 * preceding operators with equal or higher priority.
	 */
	prec = base + opinfo[opstack[ops].idx].prec;
	build = applyall(prec);
	if (!build)
	{
		return (CHAR *)0;
	}

	/* mark the end of the "build" arg and start a new one */
	opstack[ops].idx = 1;
	opstack[ops].prec = prec;
	opstack[++ops].first = ++build;
	*build = '\0';
	return build;
}

/* This function evaluates an expression, as for a :if or :let command.
 * Returns the result of the evaluation, or NULL if error.
 */
CHAR *calculate(expr, arg, rule)
	CHAR	*expr;	/* an expression to evaluate */
	CHAR	**arg;	/* arguments, to replace $1 through $9 */
	CALCRULE rule;	/* bitmap of CALC_DOLLAR, CALC_PAREN, CALC_OUTER */
{
	CHAR	*build;		/* the result so far */
	int	base = 0;	/* precedence base, keeps track or () pairs */
	int	nargs;		/* number of arguments in arg[] */
	CHAR	*tmp;
	int	i, prec;
	ELVBOOL	here_regexp, next_regexp;
	ELVBOOL	asmsg = (ELVBOOL)((rule & CALC_OUTER) == 0);
#ifndef TRY
	CHAR	*scan;
#endif

	/* count the args */
	for (nargs = 0; arg && arg[nargs]; nargs++)
	{
	}

	/* reset stack & result */
	ops = 0;
	opstack[ops].first = build = result;
	*build = '\0';
	

	/* process the expression from left to right... */
	next_regexp = ElvFalse;
	while (*expr)
	{
		here_regexp = next_regexp;
		next_regexp = ElvFalse;

		if (RESULT_OVERFLOW(build, 1)) goto Overflow;
		switch (expr[0] == '.' && expr[1] != '.' ? '\0' : *expr)
		{
		  case ' ':
		  case '\t':
		  case '\n':
			/* whitespace is ignored unless asmsg */
			if (base == 0 && asmsg)
			{
				*build++ = *expr;
				*build = '\0';
			}
			expr++;
			break;

		  case '"':
			if (base == 0 && asmsg)
			{
				/* For messages, " is just another character */
				*build++ = *expr++;
				*build = '\0';
			}
			else
			{
				/* quoted text is copied verbatim */
				build = maybeconcat(build, base, asmsg);
				if (!build)
					return NULL;
				while (*++expr && *expr != '"')
				{
					if (RESULT_OVERFLOW(build, 1))
						goto Overflow;
					if (*expr != '\\')
					{
						*build++ = *expr;
						continue;
					}
					switch (*++expr)
					{
					  case 0:
						*build++ = '\\';
						expr--;
						break;

					  case '\n': break;
					  case 'a': *build++ = '\007'; break;
					  case 'b': *build++ = '\b'; break;
					  case 'E': *build++ = '\033'; break;
					  case 'f': *build++ = '\f'; break;
					  case 'n': *build++ = '\n'; break;
					  case 'r': *build++ = '\r'; break;
					  case 't': *build++ = '\t'; break;
					  default: *build++ = *expr;
					}
				}
				if (*expr == '"')
				{
					expr++;
				}
				*build = '\0';
			}
			break;

		  case '\\':
			/* In most contexts, a backslash is treated as a
			 * literal character.  However, it can also be used to
			 * quote the special characters of a message string:
			 * dollar sign, parentheses, and the backslash itself.
			 */
			expr++;
			if (build == result || !*expr || !strchr("$()\\", *expr))
			{
				/* at front of expression, or if followed by
				 * normal character - literal */
				build = maybeconcat(build, base, asmsg);
				if (!build)
					return NULL;
				*build++ = '\\';
			}
			else if (*expr)
			{
				/* followed by special character - quote */
				build = maybeconcat(build, base, asmsg);
				if (!build)
					return NULL;
				*build++ = *expr++;
			}
			*build = '\0';
			break;

		  case '$':
			if (base == 0 && (rule & CALC_DOLLAR) == 0)
			{
				/* '$' is just another character */
				*build++ = *expr++;
				*build = '\0';
				break;
			}

			/* if it isn't followed by an alphanumeric character,
			 * then treat the '$' as a literal character.
			 */
			build = maybeconcat(build, base, asmsg);
			if (!build)
				return NULL;
			expr++;
			if (!elvalnum(*expr) && *expr != '_')
			{
				*build++ = '$';
				*build = '\0';
				break;
			}

			/* copy the name into the result buffer temporarily,
			 * just so we have a nul-terminated copy of it.
			 */
			i = copyname(build, expr, ElvTrue);
			if (i == 0) goto Overflow;
			expr += i;

			/* if number instead of a name, then use arg[i] */
			if (calcnumber(build))
			{
				i = CHAR2long(build);
				if (i <= 0 || i > nargs)
				{
#ifdef TRY
					msg(MSG_ERROR, "args must be $1 through $%d", nargs);
#else
					msg(MSG_ERROR, "[d]args must be \\$1 through \\$$1", nargs);
#endif
					return (CHAR *)0;
				}
				if (RESULT_OVERFLOW(build, CHARlen(arg[i - 1])))
					goto Overflow;
				(void)CHARcpy(build, arg[i - 1]);
				build += CHARlen(build);
			}
			else
			{
				/* try to fetch its value; stuff the value (or
				 * an empty string) into the result buffer.
				 */
				tmp = toCHAR(getenv(tochar8(build)));
#ifndef TRY
				if (!tmp)
				{
					if (optval(tochar8(build)) || !CHARcmp(build, toCHAR("_")))
					{
						/* skip the $ but reparse the
						 * name as an option or _.
						 */
						expr -= i;
					}
					else
					{
						/* convert to uppercase &
						 * try again
						 */
						for (tmp = build; *tmp; tmp++)
							*tmp = elvtoupper(*tmp);
						tmp = toCHAR(getenv(tochar8(build)));
					}
				}
#endif
				if (tmp)
				{
					if (RESULT_OVERFLOW(build, CHARlen(tmp)))
						goto Overflow;
					(void)CHARcpy(build, tmp);
					build += CHARlen(build);
				}
				else
				{
					/* clobber the name.  Harmess for
					 * options, but for unset environment
					 * variables it causes the expression
					 * to return "".
					 */
					*build = '\0';
				}
			}
			break;

		  case '(':
			if (base == 0 && (rule & CALC_PAREN) == 0)
			{
				/* '(' is just another character */
				*build++ = *expr++;
				*build = '\0';
				break;
			}

			build = maybeconcat(build, base, asmsg);
			if (!build)
				return NULL;

			/* increment the precedence base */
			parstack[base / 20] = opstack[ops].first;
			base += 20;

			/* adjust the start of arguments */
			opstack[ops].first = build;

			expr++;
			break;

#ifdef FEATURE_ARRAY
		  case '\0': /* really the '.' operator */
		  case '[':
			/* in "simpler" syntax, ignore outside of parens */
			if (asmsg && base < 20)
			{
				*build++ = *expr++;
				*build = '\0';
				break;
			}

			/* store this operator, and start a new one
			 * right after it.
			 */
			opstack[ops].idx = i = 2; /* Sub */
			opstack[ops].prec = base + opinfo[i].prec;
			opstack[++ops].first = ++build;
			*build = '\0';
			
			/* increment the precedence base */
			parstack[base / 20] = opstack[ops].first;
			base += 20;

			/* adjust the start of arguments */
			opstack[ops].first = build;

			if (*expr++ == '[')
			{
				break;
			}
			else /* the '.' operator */
			{
				/* collect the chars in the field name */
				i = copyname(build, expr, ElvFalse);
				if (i == 0) goto Overflow;
				expr += i;

				/* We'll fall through to the ')' case below,
				 * which will expect to use `expr++' to move
				 * past the ')' character, but we don't really
				 * have a ')' character to move past, so...
				 */
				expr--;
			}
			/* and fall through (for the '.' operator only) */
			
		  case ']':
			if (asmsg && base < 20)
			{
				*build++ = *expr++;
				*build = '\0';
				break;
			}
			/* else fall through... */
#endif /* FEATURE_ARRAY */
	
		  case ')':
			/* detect mismatched ')' */
			if (base == 0)
			{
				/* if not doing parens, this is okay */
				if ((rule & CALC_PAREN) == 0)
				{
					*build++ = *expr++;
					*build = '\0';
					break;
				}

				/* else it's an error */
				goto Mismatch;
			}

			/* apply any preceding higher-precedence operators */
			build = applyall(base);
			if (!build)
			{
				return (CHAR *)0;
			}

			/* decrement the precedence base */
			base -= 20;
			opstack[ops].first = parstack[base / 20];

			expr++;
			break;

		  case '_':
			if (asmsg && base < 20)
			{
				/* insert a literal '_' character */
				*build++ = '_';
			}
			else
			{
				/* insert a copy of the current line */
				CHARcpy(build, toCHAR("line"));
				(void)func(build, toCHAR(""));
				while (*build)
					build++;
			}
			expr++;
			break;

		  default:
			/* It may be an option name, a number, or an operator.
			 * If it appears to be alphanumeric, then assume it
			 * is either a number or a name.
			 */
			if (elvalnum(*expr))
			{
				build = maybeconcat(build, base, asmsg);
				if (!build)
					return NULL;

				/* Copy the string into the result buffer. */
				i = copyname(build, expr, ElvFalse);
				if (i == 0) goto Overflow;
				expr += i;

				/* if asmsg, then do no further processing of it */
				if (base == 0 && asmsg)
				{
					build += i;
					*build = '\0';
					break;
				}

				/* If the string looks like a number, leave it.
				 * If not a number, then look it up as the name
				 * of an option, and replace it with the value
				 * of that option.
				 */
				if (calcbase10(build))
				{
					/* number -- keep it */
					build += CHARlen(build);
				}
				else if (*expr == '(')
				{
					/* function name -- push a '('
					 * operator with a precedence set
					 * so that next ')' will cause it
					 * to be evaluated.
					 */
					parstack[base / 20] = opstack[ops].first;
					opstack[ops].first = build;

					/* keep the function name */
					build += i;

					/* increment the precedence base */
					base += 20;

					/* compute the precedence of the operator */
					prec = 1 + base;

					/* first argument may be a regexp */
					next_regexp = ElvTrue;

					/* store this operator, and start a new
					 * one right after it.
					 */
					opstack[ops].idx = 0;
					opstack[ops].prec = prec;
					opstack[++ops].first = ++build;
					*build = '\0';
					expr++;
				}
				else
				{
					/* option name -- look it up */
					tmp = optgetstr(build, NULL);
					if (!tmp)
					{
#ifdef TRY
						msg(MSG_ERROR, "bad option name %s", build);
#else
						msg(MSG_ERROR, "[s]bad option name $1", build);
#endif
						return (CHAR *)0;
					}
					if (RESULT_OVERFLOW(build, CHARlen(tmp)))
						goto Overflow;
					(void)CHARcpy(build, tmp);
					build += CHARlen(build);
				}
			}
			else /* not alphanumeric */
			{
				/* if asmsg, then use it as plain text */
				if (base == 0 && asmsg)
				{
					build = maybeconcat(build, base, asmsg);
					if (!build)
						return NULL;
					*build++ = *expr++;
					*build = '\0';
					break;
				}

				/* may be a character constant, as in '\t' */
				if (expr[0] == '\'')
				{
					/* copy it into build[] */
					build = maybeconcat(build, base, asmsg);
					if (!build)
						return NULL;
					build[0] = *expr++;
					for (i = 1; *expr && *expr != '\''; )
					{
						build[i] = *expr++;
						if (build[i++] == '\\' && *expr)
							build[i++] = *expr++;
					}
					if (*expr == '\'')
						build[i++] = *expr++;
					build[i] = '\0';
							
					/* convert to number */
					if (calcbase10(build))
					{
						build += CHARlen(build);
						break;
					}
					else
					{
						msg(MSG_ERROR, "bad character literal");
						return (CHAR *)0;
					}
				}

#ifndef TRY
				/* some contexts allow a regular expression */
				if (expr[0] == '/' && here_regexp)
				{
					build = maybeconcat(build, base, asmsg);
					if (!build)
						return NULL;

					/* parse the regexp */
					scanstring(&scan, expr + 1);
					tmp = regbuild('/', &scan, ElvTrue);

					/* copy the regexp as a string, keeping
					 * the initial / delimiter, but not the
					 * closing delimiter.
					 */
					*build++ = '/';
					if (RESULT_OVERFLOW(build, CHARlen(tmp)))
					{
						scanfree(&scan);
						msg(MSG_ERROR, "result too long");
						return (CHAR *)0;
					}
					CHARcpy(build, tmp);
					build += CHARlen(build);
					safefree(tmp);

					/* move past the end of the regexp */
					expr = (scan ? scan : toCHAR(""));
					scanfree(&scan);
					break;
				}
#endif /* !TRY */

				/* try to identify an operator. This is slightly
				 * trickier than it looks -- the order in which
				 * the comparisons was made had to be fine-tuned
				 * so it wouldn't think a "!=" was a "!".
				 */
				for (i = 0;
				     i < QTY(opinfo) && CHARncmp(opinfo[i].name, expr, strlen(opinfo[i].name));
				     i++)
				{
				}
				if (i >= QTY(opinfo))
				{
#ifdef TRY
					msg(MSG_ERROR, "bad operator %c", *expr);
#else
					msg(MSG_ERROR, "[C]bad operator $1", *expr);
#endif
					return (CHAR *)0;
				}

				/* compute the precedence of the operator */
				prec = opinfo[i].prec + base;

				/* apply any preceding operators with equal or
				 * higher priority.
				 */
				build = applyall(prec);
				if (!build)
				{
					return (CHAR *)0;
				}

				/* store this operator, and start a new one
				 * right after it.
				 */
				opstack[ops].idx = i;
				opstack[ops].prec = prec;
				opstack[++ops].first = ++build;
				*build = '\0';
				expr += strlen(opinfo[i].name);

				/* Allow "..." as a synonym for ".." */
				if (opinfo[i].name[0] == '.' && *expr == '.')
					expr++;
			}
		}
	}

	/* detect situation where more '(' than ')' were given */
	if (base > 0)
	{
Mismatch:
		msg(MSG_ERROR, "(\"()\") mismatch");
		return (CHAR *)0;
	}

	/* evaluate any remaining operators */
	build = applyall(0);
	if (!build)
	{
		return (CHAR *)0;
	}

	/* return the result */
	return result;

Overflow:
	msg(MSG_ERROR, "result too long");
	return (CHAR *)0;
}

# ifndef TRY
/* This function evaluates a section of a buffer, and replaces that section
 * with the result.  Returns ElvTrue if successful, or ElvFalse if there's
 * an error.
 */
ELVBOOL calcsel(from, to)
	MARK	from;
	MARK	to;
{
	CHAR	*expr = bufmemory(from, to);
	CHAR	*result = calculate(expr, (CHAR **)0, CALC_ALL);

	safefree(expr);
	if (!result)
	{
		return ElvFalse;
	}
	bufreplace(from, to, result, (long)CHARlen(result));
	return ElvTrue;
}
# endif /* !TRY */

#else /* ! FEATURE_CALC */

/* Since the real calculate() function is disabled, we must make a fake one
 * just for outputting messages via message.c.  To do that, it must be able
 * to replace $n with arguments, and (option) with the option's value.
 */
CHAR *calculate(expr, arg, rule)
	CHAR	*expr;	/* the message, with $n for parameters */
	CHAR	**arg;	/* the parameters */
	CALCRULE rule;	/* what kinds of substitutions to make */
{
	static CHAR	result[200];
	CHAR		*build;
	int		nargs, nest;
	CHAR		*name, *buildname;

	/* count arguments */
	for (nargs = 0; arg && arg[nargs]; nargs++)
	{
	}

	/* copy msg into result, expanding parameters along the way */
	for (build = result; *expr; expr++)
	{
		if (*expr == '\\' && expr[1])
			*build++ = *++expr;
		else if ((rule & CALC_DOLLAR) && *expr == '$' && expr[1] >= '1' && expr[1] < '1'+nargs)
		{
			expr++;
			CHARcpy(build, arg[*expr - '1']);
			build += CHARlen(build);
		}
		else if ((rule & CALC_PAREN) && *expr == '(')
		{
			/* skip to matching ')', watching for option name */
			name = buildname = build;
			for (nest = 1, expr++; *expr && nest > 0; expr++)
				if (*expr == '(')
					nest++, name = NULL;
				else if (*expr == ')')
					nest--;
				else if (name && elvalnum(*expr))
					*buildname++ = *expr;
				else
					name = NULL;

			/* if we have a name, then fetch its value */
			if (name)
			{
				*buildname = '\0';
				buildname = optgetstr(name, NULL);
				if (buildname)
				{
					CHARcpy(build, buildname);
					build += CHARlen(build);
				}
			}

			/* leave expr pointing at the ')' */
			expr--;
		}
		else
			*build++ = *expr;
	}
	*build = '\0';
	return result;
}
#endif /* ! FEATURE_CALC */

#ifdef TRY
# include <stdarg.h>
BUFFER bufdefault;

CHAR *optgetstr(name, desc)
	CHAR	*name;
	OPTDESC	**desc;
{
	return name;
}

void msg(MSGIMP imp, char *format, ...)
{
	va_list	argptr;

	va_start(argptr, format);
	vprintf(format, argptr);
	putchar('\n');
	va_end(argptr);
}

int main(int argc, char **argv)
{
	CHAR	expr[200];
	CHAR	*result;
	char	flag;
	int	i;
	ELVRULE	rule = CALC_ALL;

	/* Parse options */
	expr[0] = '\0';
	while ((flag = getopt(argc, argv, "e:m")) >= 0)
	{
		switch (flag)
		{
		  case '?':
			fprintf(stderr, "usage: %s [-m] [-e expr] [arg]...\n", argv[0]);
			fprintf(stderr, "This program is meant to be used primarily for testing elvis' built-in\n");
			fprintf(stderr, "calculator.  It may also be useful for systems that don't have \"bc\".\n");
			fprintf(stderr, "The -m flag causes the expression to be evaluated using elvis' simpler\n");
			fprintf(stderr, "syntax, which is used mostly for outputing messages; otherwise it uses\n");
			fprintf(stderr, "the normal syntax.  The -eexpr flag causes it to evaluate expr and quit;\n");
			fprintf(stderr, "otherwise it reads expressions from stdin until EOF.  Any remaining\n");
			fprintf(stderr, "arguments are used as parameters which are accessible as $1 through $9\n");
			fprintf(stderr, "in the expression.  See the elvis manual for more information.\n");
			fprintf(stderr, "\n");
			fprintf(stderr, "This program is unsupported and carries no guarantees.\n");
			exit(0);
			break;

		  case 'm':
			rule = ELV_DOLLAR|ELV_PAREN;
			break;

		  case 'e':
			for (i = 0; (expr[i] = optarg[i]) != '\0'; i++)
			{
			}
			break;
		}
	}

	/* were we given an expression on the command line? */
	if (*expr)
	{
		result = calculate(expr, (CHAR **)&argv[optind], rule);
		if (result)
			puts(tochar8(result));
	}
	else
	{
		while (fgets(tochar8(expr), sizeof expr, stdin))
		{
			result = calculate(expr, (CHAR **)&argv[optind], rule);
			if (result)
				puts(tochar8(result));
		}
	}
	exit(result ? 0 : 1);
	return result ? 0 : 1;
}
#endif /* TRY */
