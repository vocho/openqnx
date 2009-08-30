/* ctags.c */


/* This is a reimplementation of the ctags(1) program.  It supports ANSI C,
 * and has heaps o' flags.  It is meant to be distributed with elvis.
 *
 * 30.04.1995	Michael Beck & Dirk Verworner (beck@informatik.hu-berlin.de)
 *		added support for inline definitions and
 *		function pointers in parameter declaration 
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_ctags[] = "$Id: ctags.c,v 2.44 2003/10/17 17:41:23 steve Exp $";
#endif
#if OSEXPANDARGS
# define JUST_DIRFIRST
# include "osdir.c"
#endif

#ifndef FALSE
# define FALSE	0
# define TRUE	1
#endif
#ifndef TAGS
# define TAGS	"tags"
#endif
#ifndef REFS
# define REFS	"refs"
#endif
#ifndef BLKSIZE
# define BLKSIZE 512
#endif

#define TAGKIND_INDEX 3
#define TAGKIND	attr[TAGKIND_INDEX]

/* #define EOF -1 */
#define DELETED	  0
#define BODY	  1
#define ARGS	  2
#define COMMA	  3
#define SEMICOLON 4
#define CLASSSEP  5
#define TYPEDEF   6
#define CLASS	  7
#define STRUCT	  8
#define UNION	  9
#define ENUM	  10
#define KSTATIC	  11
#define EXTERN	  12
#define NAME	  13
#define INLINE	  14

extern void	file_open P_((char *));
extern int	file_getc P_((void));
extern void	file_ungetc P_((int));
extern int	file_copyline P_((long, FILE *, char *));
extern void	cpp_open P_((char *));
extern void	cpp_echo P_((int));
extern int	cpp_getc P_((void));
extern void	cpp_ungetc P_((int));
extern int	lex_gettoken P_((void));
extern void	maketag P_((int, char*, long, long, int, char*, char *));
extern void	mkbodytags P_((int, char*, char*, long));
extern void	ctags P_((char *));
extern void	usage P_((void));
extern int	main P_((int, char **));


#if defined (GUI_WIN32)
extern void set_current_file (char *file_name);
extern void set_current_tags (int num_tags);
extern void set_total_tags (int num_tags);

static int  num_tags = 0;
static int  total_tags = 0;
#endif

/* support for elvis' ctype macros */
#include "ctypetbl.h"

/* -------------------------------------------------------------------------- */
/* Some global variables */

/* The following boolean variables are set according to command line flags */
char	*del_word;	/* -Dword  ignore word -- good for parameter macro */
int	backward;	/* -B  regexp patterns search backwards */
int	use_numbers;	/* -N  use line numbers instead of regexp patterns */
int	as_global;	/* -g  store static tags as though they were global */
int	incl_static;	/* -s  include static tags */
int	incl_extern;	/* -e  include extern tags */
int	incl_types;	/* -t  include typedefs and structs */
int	incl_vars;	/* -v  include variables */
int	incl_inline;	/* -i  include inline's */
int	make_parse;	/* -p  write parsed tokens to stdout */
int	warn_duplicate;	/* -d  write warnings about duplicates to stdout */
int	make_xtbl;	/* -x  write cross-reference table to stdout */
int	make_tags = 1;	/*     generate a "tags" file (implied by lack of -x) */
int	make_refs;	/* -r  generate a "refs" file */
int	append_files;	/* -a  append to "tags" [and "refs"] files */
int	add_hints;	/* -h  include extra fields that give elvis hints */
int	add_ln;		/* -l  include line number in hints */

/* The following are used for outputting to the "tags" and "refs" files */
FILE	*tags;		/* used for writing to the "tags" file */
FILE	*refs;		/* used for writing to the "refs" file */

/* The following are either empty or store hints */
char	hint_class[200];/* class name, for tags preceeded by a name and :: */

/* -------------------------------------------------------------------------- */
/* display a fatal error message from safe.c */
#if USE_PROTOTYPES
void msg(MSGIMP type, char *msg, ...)
#else
void msg(type, msg)
	MSGIMP	type;
	char	*msg;
#endif
{
	fprintf(stderr, "%s\n", msg);
	abort();
}

/* -------------------------------------------------------------------------- */
/* Most tag sorting functions try to sort by timestamp if nothing else.  ctags
 * is different -- it doesn't care about timestamps.  To get this behavior,
 * we define a custom version of dirtime here.
 */
char *dirtime(filename)
	char *filename;
{
	return "";
}

/* -------------------------------------------------------------------------- */
/* These are used for reading a source file.  It keeps track of line numbers */
char	*file_name;	/* name of the current file */
FILE	*file_fp;	/* stream used for reading the file */
long	file_lnum;	/* line number in the current file */
long	file_seek;	/* fseek() offset to the start of current line */
int	file_afternl;	/* boolean: was previous character a newline? */
int	file_prevch;	/* a single character that was ungotten */
int	file_header;	/* boolean: is the current file a header file? */

/* This function opens a file, and resets the line counter.  If it fails, it
 * it will display an error message and leave the file_fp set to NULL.
 */
void file_open(name)
	char	*name;	/* name of file to be opened */
{
	/* if another file was already open, then close it */
	if (file_fp)
	{
		fclose(file_fp);
	}

	/* try to open the file for reading.  The file must be opened in
	 * "binary" mode because otherwise fseek() would misbehave under DOS.
	 */
	file_fp = fopen(name, "rb");
	if (!file_fp)
	{
		perror(name);
	}

	/* reset the name & line number */
	file_name = name;
	file_lnum = 0L;
	file_seek = 0L;
	file_afternl = TRUE;

	/* determine whether this is a header file */
	file_header = FALSE;
	name += strlen(name) - 2;
	if (name >= file_name && name[0] == '.' && (name[1] == 'h' || name[1] == 'H'))
	{
		file_header = TRUE;
	}
}

/* This function reads a single character from the stream.  If the *previous*
 * character was a newline, then it also increments file_lnum and sets
 * file_offset.
 */
int file_getc()
{
	int	ch;

	/* if there is an ungotten character, then return it.  Don't do any
	 * other processing on it, though, because we already did that the
	 * first time it was read.
	 */
	if (file_prevch)
	{
		ch = file_prevch;
		file_prevch = 0;
		return ch;
	}

	/* if previous character was a newline, then we're starting a line */
	if (file_afternl && file_fp)
	{
		file_afternl = FALSE;
		file_seek = ftell(file_fp);
		file_lnum++;
	}

	/* Get a character.  If no file is open, then return EOF */
	ch = (file_fp ? getc(file_fp) : EOF);

	/* if it is a newline, then remember that fact */
	if (ch == '\n')
	{
		file_afternl = TRUE;
	}

	/* return the character */
	return ch;
}

/* This function ungets a character from the current source file */
void file_ungetc(ch)
	int	ch;	/* character to be ungotten */
{
	file_prevch = ch;
}

/* This function copies the current line out some other fp.  It has no effect
 * on the file_getc() function.  During copying, any '\' characters are doubled
 * and a leading '^' or trailing '$' is also quoted.  The '\n' character is not
 * copied.  If the '\n' is preceded by a '\r', then the '\r' isn't copied.
 *
 * This is meant to be used when generating a tag line.
 */
int file_copyline(seek, fp, buf)
	long	seek;	/* where the lines starts in the source file */
	FILE	*fp;	/* the output stream to copy it to if buf==NULL */
	char	*buf;	/* line buffer, or NULL to write to fp */
{
	long	oldseek;/* where the file's pointer was before we messed it up */
	int	ch;	/* a single character from the file */
	char	next;	/* the next character from this file */
	char	*build;	/* where to store the next character */

	/* go to the start of the line */
	oldseek = ftell(file_fp);
	fseek(file_fp, seek, 0);

	/* write everything up to, but not including, the newline */
	for (ch = getc(file_fp), build = buf; ch != '\n' && ch != EOF; ch = next)
	{
		/* if too many chars have been written already, then stop */
		if (build && build >= &buf[160])
		{
			break;
		}

		/* preread the next character from this file */
		next = getc(file_fp);

		/* if character is '\', or a terminal '$', then quote it */
		if (build && (ch == '\\'
			   || ch == (backward ? '?' : '/')
			   || (ch == '$' && next == '\n')))
		{
			*build++ = '\\';
		}

		/* copy the character, unless it is a terminal '\r' */
		if (ch != '\r' || next != '\n')
		{
			if (build)
				*build++ = ch;
			else
				putc(ch, fp);
		}
	}

	/* mark the end of the line */
	if (build)
		*build = '\0';

	/* seek back to the old position */
	fseek(file_fp, oldseek, 0);

	/* return 1 if it fit, 0 if it was truncated */
	if (build && build >= &buf[160])
		return 0;
	else
		return 1;
}

/* -------------------------------------------------------------------------- */
/* This section handles preprocessor directives.  It strips out all of the
 * directives, and may emit a tag for #define directives.
 */

int	cpp_afternl;	/* boolean: look for '#' character? */
int	cpp_prevch;	/* an ungotten character, if any */
int	cpp_refsok;	/* boolean: can we echo characters out to "refs"? */
int	cpp_refsnl;	/* boolean: dup next \n in "refs"? */

/* This function opens the file & resets variables */
void cpp_open(name)
	char	*name;	/* name of source file to be opened */
{
	/* use the lower-level file_open function to open the file */
	file_open(name);

	/* reset variables */
	cpp_afternl = TRUE;
	cpp_refsok = TRUE;
}

/* This function copies a character from the source file to the "refs" file */
void cpp_echo(ch)
	int	ch; /* the character to copy */
{
	static int	wasnl;

	/* echo non-EOF chars, unless not making "ref", or echo turned off */
	if (ch != EOF && make_refs && cpp_refsok && !file_header)
	{
		/* try to avoid blank lines */
		if (ch == '\n')
		{
			/* hack: double \n at end of declarations, to
			   help `ref' find the right starting point...
			*/
			if (cpp_refsnl)
			{
				putc('\n', refs);
				cpp_refsnl = FALSE;
			}
			if (wasnl)
			{
				return;
			}
			wasnl = TRUE;
		}
		else
		{
			wasnl = FALSE;
		}

		/* add the character */
		putc(ch, refs);
	}
}

/* This function returns the next character which isn't part of a directive */
int cpp_getc()
{
	static
	int	ch;	/* the next input character */
	char	*scan;
	char	name[50];/* name of a macro */
	int	len;	 /* length of macro name */

	/* if we have an ungotten character, then return it */
	if (cpp_prevch)
	{
		ch = cpp_prevch;
		cpp_prevch = 0;
		return ch;
	}

	/* Get a character from the file.  Return it if not special '#' */
	ch = file_getc();
	if (ch == '\n')
	{
		cpp_afternl = TRUE;
		cpp_echo(ch);
		return ch;
	}
	else if (ch != '#' || !cpp_afternl)
	{
		/* normal character.  Any non-whitespace should turn off afternl */
		if (ch != ' ' && ch != '\t')
		{
			cpp_afternl = FALSE;
		}
		cpp_echo(ch);
		return ch;
	}

	/* Yikes!  We found a directive */

	/* see whether this is a #define line */
	scan = " define ";
	while (*scan)
	{
		if (*scan == ' ')
		{
			/* space character matches any whitespace */
			do
			{
				ch = file_getc();
			} while (ch == ' ' || ch == '\t');
			file_ungetc(ch);
		}
		else
		{
			/* other characters should match exactly */
			ch = file_getc();
			if (ch != *scan)
			{
				file_ungetc(ch);
				break;
			}
		}
		scan++;
	}

	/* is this a #define line?  and should we generate a tag for it? */
	if (!*scan)
	{
		/* collect chars of the tag name */
		for (ch = file_getc(), len = 0;
		     elvalnum(ch) || ch == '_';
		     ch = file_getc())
		{
			if ((unsigned)len < sizeof(name) - 2)
			{
				name[len++] = ch;
			}
		}
		name[len] = '\0';

		/* maybe output parsing info */
		if (make_parse)
		{
			printf("#define %s\n", name);
		}

		/* output a tag line */
		maketag(file_header ? 0 : KSTATIC, name, file_lnum, file_seek, TRUE, "d", NULL);
	}

	/* skip to the end of the directive -- a newline that isn't preceded
	 * by a '\' character.
	 */
	while (ch != EOF && ch != '\n')
	{
		if (ch == '\\')
		{
			ch = file_getc();
		}
		ch = file_getc();
	}

	/* return the newline that we found at the end of the directive */
	cpp_echo(ch);
	return ch;
}

/* This puts a character back into the input queue for the source file */
void cpp_ungetc(ch)
	int	ch;	/* a character to be ungotten */
{
	cpp_prevch = ch;
}


/* -------------------------------------------------------------------------- */
/* This is the lexical analyzer.  It gets characters from the preprocessor,
 * and gives tokens to the parser.  Some special codes are...
 *   (deleted)  / *...* / (comments)
 *   (deleted)	/ /...\n  (comments)
 *   (deleted)	(*	(parens used in complex declaration)
 *   (deleted)	[...]	(array subscript, when ... contains no ])
 *   (deleted)	struct	(intro to structure declaration)
 *   (deleted)	:words{	(usually ":public baseclass {" in a class declaration
 *   BODY	{...}	('{' can occur anywhere, '}' only at BOL if ... has '{')
 *   ARGS	(...{	(args of function, not extern or forward)
 *   ARGS	(...);	(args of an extern/forward function declaration)
 *   COMMA	,	(separate declarations that have same scope)
 *   SEMICOLON	;	(separate declarations that have different scope)
 *   SEMICOLON  =...;	(initializer)
 *   CLASSSEP	::	(class separator)
 *   TYPEDEF	typedef	(the "typedef" keyword)
 *   KSTATIC	static	(the "static" keyword)
 *   KSTATIC	private	(the "static" keyword)
 *   KSTATIC	PRIVATE	(the "static" keyword)
 *   NAME	[a-z]+	(really any valid name that isn't reserved word)
 */

char	lex_name[BLKSIZE];	/* the name of a "NAME" token */
long	lex_seek;		/* start of line that contains lex_name */
long	lex_lnum;		/* line number of lex_name occurrence */
char	lex_body[BLKSIZE];	/* space-delimited names from body */

int lex_gettoken()
{
	char	name[BLKSIZE];	/* stores a temporary word */
	char	newname[BLKSIZE];/* stores a temporary word */
	int	ch;		/* a character from the preprocessor */
	int	next;		/* the next character */
	int	token;		/* the token that we'll return */
	int	par;		/* '()' counter */
	int	hasname;	/* have we seen an arg name lately? */
	int	i;

	/* loop until we get a token that isn't "DELETED" */
	do
	{
		/* get the next character */
		ch = cpp_getc();

		/* process the character */
		switch (ch)
		{
		  case ',':
			token = COMMA;
			break;

		  case ';':
			token = SEMICOLON;
			break;

		  case '/':
			/* get the next character */
			ch = cpp_getc();
			switch (ch)
			{
			  case '*':	/* start of C comment */
				ch = cpp_getc();
				next = cpp_getc();
				while (next != EOF && (ch != '*' || next != '/'))
				{
					ch = next;
					next = cpp_getc();
				}
				break;

			  case '/':	/* start of a C++ comment */
				do
				{
					ch = cpp_getc();
				} while (ch != '\n' && ch != EOF);
				break;

			  default:	/* some other slash */
				cpp_ungetc(ch);
			}
			token = DELETED;
			break;

		  case '(':
			ch = cpp_getc();
			if (ch == '*')
			{
				/* In a declaration such as (*name)(), we
				 * want to delete the (* but show the name.
				 */
				token = DELETED;
			}
			else
			{
				/* After the name of a function declaration or
				 * definition, we want to parse the args as a
				 * single token.  New-style args are easy to
				 * parse, but old-style is not.  Begin by
				 * parsing matching parentheses.
				 */
				for (par = 1; par > 0; ch = cpp_getc())
				{
					switch (ch)
					{
					  case '(':  par++;	break;
					  case ')':  par--;	break;
					  case EOF:  par = 0;	break;
					}
				}

				/* now we may have old-style arguments, or
				 * a semicolon or a '{'.  We want to stop
				 * before the first semicolon which isn't
				 * preceded by a word, or before any '{'
				 */
				for (hasname = FALSE;
				     ch!=EOF && ch!='{' && (ch!=';' || hasname);
				     ch = cpp_getc())
				{
					if (elvalpha(ch) || ch == '_')
						hasname = TRUE;
				}
				if (ch != EOF)
				{
					cpp_ungetc(ch);
				}
				token = ARGS;
			}
			break;

		  case '{':/*}*/
			/* don't send the next characters to "refs" */
			cpp_refsok = FALSE;

			/* skip ahead to closing '}', or to embedded '{' */
			i = hasname = 0;
			*lex_body = '\0';
			do
			{
				ch = cpp_getc();
				if (elvalnum(ch) || ch == '_')
				{
					newname[i++] = ch;
				}
				else
				{
					/* is this the end of a name? */
					if (i > 0)
					{
						/* mark the end of the name */
						newname[i] = '\0';
						i = 0;

						/* store it, unless it is
						 * a name that is specifically
						 * supposed to be ignored, or
						 * appears to be a number.
						 */
						if (strcmp(newname, "P_")
						 && strcmp(newname, "__P")
						 && (!del_word || strcmp(newname, del_word))
						 && !elvdigit(*newname))
						{
							hasname = 1;
							strcpy(name, newname);
						}
					}

					/* Skip comments.  The best way to do
					 * this would be to recurse, but we do
					 * it the cheap way: skip everything
					 * between / and EOL.
					 */
					if (ch == '/')
					{
						while ((ch = cpp_getc()) != '\n')
						{
						}
					}

					/* the last name before a comma, '=', or
					 * semicolon should be saved so we can
					 * generate tags for the struct/enum
					 * body later.
					 */
					if (hasname && (ch == ',' || ch == '=' || ch == ';' || ch == '}'))
					{
						if (strlen(name) + strlen(lex_body) + 2 < QTY(lex_body))
						{
							if (*lex_body)
								strcat(lex_body, " ");
							strcat(lex_body, name);
						}
						hasname = 0;
					}

					/* the contents of parentheses should
					 * be skipped, so we see function names
					 * instead of parameter names as being
					 * the last name before a semicolon.
					 * Note that this doesn't work well for
					 * inline functions -- the {} trips it
					 * up.
					 */
					if (hasname && ch == '(')
					{
						i = 1;
						do
						{
							ch = cpp_getc();
							switch (ch)
							{
							  case '(':
								i++;
								break;
							  case ')':
								i--;
								break;
							  case '{':
							  case '}':
							  case EOF:
								i = 0;
								break;
							}
						} while (i != 0);
					}
				}
			} while (ch != '{' && ch != '}' && ch != EOF);

			/* if has embedded '{', then skip to '}' in column 1 */
			if (ch == '{') /*}*/
			{
				ch = cpp_getc();
				next = cpp_getc();
				while (ch != EOF && (ch != '\n' || next != '}'))/*{*/
				{
					ch = next;
					next = cpp_getc();
				}
			}

			/* resume "refs" processing */
			cpp_refsok = TRUE;
			cpp_echo('}');

			token = BODY;
			cpp_refsnl = TRUE;
			break;

		  case '[':
			/* skip to matching ']' */
			do
			{
				ch = cpp_getc();
			} while (ch != ']' && ch != EOF);
			token = DELETED;
			break;

		  case '=':
		  	/* skip to next ';' */
			do
			{
				ch = cpp_getc();

				/* leave array initializers out of "refs" */
				if (ch == '{')
				{
					cpp_refsok = FALSE;
				}
			} while (ch != ';' && ch != EOF);

			/* resume echoing to "refs" */
			if (!cpp_refsok)
			{
				cpp_refsok = TRUE;
				cpp_echo('}');
				cpp_echo(';');
			}
			token = SEMICOLON;
			break;

		  case ':':
		  	/* check for a second ':' */
		  	ch = cpp_getc();
		  	if (ch == ':')
		  	{
		  		token = CLASSSEP;
		  	}
		  	else
		  	{
		  		/* probably ": public baseclass {" in a class
		  		 * declaration.
		  		 */
		  		while (elvspace(ch) || elvalnum(ch) || ch == '_')
		  		{
		  			ch = cpp_getc();
		  		}
				cpp_ungetc(ch);
				token = DELETED;
		  	}
		  	break;

		  case EOF:
			token = EOF;
			break;

		  default:
			/* is this the start of a name/keyword? */
			if (elvalpha(ch) || ch == '_')
			{
				/* collect the whole word */
				name[0] = ch;
				for (i = 1, ch = cpp_getc();
				     i < BLKSIZE - 1 && (elvalnum(ch) || ch == '_');
				     i++, ch = cpp_getc())
				{
					name[i] = ch;
				}
				name[i] = '\0';
				cpp_ungetc(ch);

				/* is it a reserved word? */
				if (!strcmp(name, "typedef"))
				{
					token = TYPEDEF;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "class"))
				{
					token = CLASS;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "struct"))
				{
					token = STRUCT;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "union"))
				{
					token = UNION;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "enum"))
				{
					token = ENUM;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "static")
				      || !strcmp(name, "private")
				      || !strcmp(name, "protected")
				      || !strcmp(name, "PRIVATE"))
				{
					token = KSTATIC;
					/* ch already contains ungotten next
					 * char.  If ':' then delete it.
					 */
					if (ch == ':')
						(void)cpp_getc();
					lex_seek = -1L;
				}
				else if (!strcmp(name, "extern")
				      || !strcmp(name, "EXTERN")
				      || !strcmp(name, "FORWARD")
				      || !strcmp(name, "virtual"))
				{
					/* if `extern "C"' then skip chars
					 * up to following non-space char.
					 * If that char is '{', skip it too.
					 */
					do
					{
						ch = cpp_getc();
					} while (elvspace(ch));
					if (ch == '"')
					{
						do
						{
							ch = cpp_getc();
						} while (ch != '"');
						do
						{
							ch = cpp_getc();
						} while (elvspace(ch));
						if (ch != '{')
							cpp_ungetc(ch);
						token = DELETED;
					}
					else
					{
						cpp_ungetc(ch);
						token = EXTERN;
						lex_seek = -1L;
					}
				}
				else if (!strcmp(name, "inline")
				      || !strcmp(name, "__inline")
				      || !strcmp(name, "__inline__"))
				{
					token = INLINE;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "public")
				      || !strcmp(name, "P_")
				      || !strcmp(name, "__P")
				      || (del_word && !strcmp(name, del_word)))
				{
					token = DELETED;
					lex_seek = -1L;
				}
				else if (!strcmp(name, "operator"))
				{
					token = NAME;
					ch = cpp_getc();
					while (strchr("[]<>=!%^&*+-?/:,|~", ch))
					{
						name[i++] = ch;
						ch = cpp_getc();
					}
					cpp_ungetc(ch);

					name[i] = '\0';
					strcpy(lex_name, name);
					lex_seek = file_seek;
					lex_lnum = file_lnum;
				}
				else
				{
					token = NAME;
					strcpy(lex_name, name);
					lex_seek = file_seek;
					lex_lnum = file_lnum;
				}
			}
			else /* not part of a name/keyword */
			{
				token = DELETED;
			}

		} /* end switch(ch) */

	} while (token == DELETED);

	return token;
}

/* -------------------------------------------------------------------------- */
/* This is the parser.  It locates tag candidates, and then decides whether to
 * generate a tag for them.
 */

/* This function generates a tag for the object in lex_name, whose tag line is
 * located at a given seek offset.
 */
void maketag(scope, name, lnum, seek, number, kind, inside)
	int	scope;	/* 0 if global, or KSTATIC if static */
	char	*name;	/* name of the tag */
	long	lnum;	/* line number of the tag */
	long	seek;	/* the seek offset of the line */
	int	number;	/* 1 to give line number, or 0 to give regexp */
	char	*kind;	/* token type of the tag: "f" for function, etc. */
	char	*inside;/* name of struct/enum that name is defined in */
{
	TAG	tag;	/* structure for storing tag info */
	char	buf[300];
	char	lnbuf[20];
	TAG	*scan, *ref, *next;


	/* if tag has a scope that we don't care about, ignore it */
	if ((scope == EXTERN && !incl_extern)
	 || (scope == KSTATIC && !incl_static))
	{
		return;
	}

	if (make_tags)
	{
		/* search for a matching name */
		for (scan = taglist;
		     scan && strcmp(scan->TAGNAME, name) < 0;
		     scan = next)
		{
			if (scan->bighop && strcmp(scan->TAGNAME, name) < 0)
				next = scan->bighop;
			else
				next = scan->next;
		}
		if (scan && strcmp(scan->TAGNAME, name) != 0)
			scan = NULL;

		/* Ensure that tags with kind="t" are unique.  This is helpful
		 * because the parser can be stupid about typedef tags in some
		 * contexts.
		 */
		if (scan)
		{
			if (kind && !strcmp(kind, "t"))
			{
				/* this is a kind="t" tag */

				/* delete it, and any other matches after it */
				for (; scan && !strcmp(scan->TAGNAME,name); scan = next)
				{
					if (warn_duplicate)
						printf("duplicate tag \"%s\" is typedef: keeping one from %s, not %ss\n", name, file_name, scan->TAGFILE);

					/* make any references to it point to
					 * the next tag instead.
					 */
					for (ref=taglist; ref != scan; ref = next)
					{
						assert(ref != NULL);
						next = ref->next;
						if (ref->next == scan)
							ref->next = scan->next;
						if (ref->bighop == scan)
							ref->bighop = scan->next;
					}
					if (taglist == scan)
						taglist = scan->next;

					/* delete this one */
					(void)tagfree(scan);

				}
			}
			else if (scan->TAGKIND && !strcmp(scan->TAGKIND, "t"))
			{
				/* This is not a kind="t" tag, but an existing
				 * duplicate tag is.  Skip this new tag.
				 */
				if (warn_duplicate)
					printf("duplicate tag \"%s\" skipped: existing tag is typedef\n", name);
				return;
			}
			else
			{
				if (warn_duplicate)
					printf("duplicate tag \"%s\" added: other tag is from %s\n", name, scan->TAGFILE);
			}
		}

#if defined (GUI_WIN32)
		set_current_tags (++num_tags);
		set_total_tags (++total_tags);
#endif

		/* store the basic attributes */
		memset(&tag, 0, sizeof tag);
		tag.TAGNAME = name;
		tag.TAGFILE = file_name;
		if (number)
		{
			sprintf(buf, "%ld", lnum);
		}
		else
		{
			strcpy(buf, backward ? "?^" : "/^");
			if (file_copyline(seek, NULL, buf + 2))
				strcat(buf, "$");
			strcat(buf, backward ? "?" : "/");
		}
		tag.TAGADDR = buf;

		/* add any relevant hints */
		if (add_hints)
		{
			if (scope == KSTATIC && !as_global)
				tagattr(&tag, "file", "");
			if (hint_class[0])
				tagattr(&tag, "class", hint_class);
			if (kind)
				tagattr(&tag, "kind", kind);
			if (add_ln)
			{
				sprintf(lnbuf, "%ld", lnum);
				tagattr(&tag, "ln", lnbuf);
			}
			if (inside)
			{
				switch (scope)
				{
				  case STRUCT:
					tagattr(&tag, "struct", inside);
					break;

				  case UNION:
					tagattr(&tag, "union", inside);
					break;

				  case ENUM:
					tagattr(&tag, "enum", inside);
					break;

				  case CLASS:
					tagattr(&tag, "class", inside);
					break;
				}
			}
		}

		/* store the tag */
		tagadd(tagdup(&tag));
	}

	if (make_xtbl)
	{
		printf("%-15.15s%6ld %-16.16s ", name, lnum, file_name);
		file_copyline(seek, stdout, NULL);
		putchar('\n');
	}
}


/* Generate tags for words used inside the body of some other tag.  This mostly
 * means fields of a struct, or members of a class.
 */
void mkbodytags(scope, inside, body, lnum)
	int	scope;	/* type of container: UNION, STRUCT, or ENUM */
	char	*inside;/* name of the container tag */
	char	*body;	/* body of the tag (space-delimited list of word) */
	long	lnum;	/* line number of the tag */
{

	char	*word, *scan;

	/* for each word...*/
	for (word = scan = body; *scan++; )
	{
		if (*scan == ' ' || *scan == '\0')
		{
			if (*scan == ' ')
				*scan++ = '\0';
			maketag(scope, word, lnum, 0L, 1, NULL, inside);
			word = scan;
		}
	}

	/* clobber the body */
	*body = '\0';
}


/* This function parses a source file, adding any tags that it finds */
void ctags(name)
	char	*name;	/* the name of a source file to be checked */
{
	int	prev;	/* the previous token from the source file */
	int	token;	/* the current token from the source file */
	int	scope;	/* normally 0, but could be a TYPEDEF or KSTATIC token */
	int	gotname;/* boolean: does lex_name contain a tag candidate? */
	long	tagseek;/* start of line that contains lex_name */

#if defined (GUI_WIN32)
    set_current_file (name);
    set_current_tags (0);
    num_tags = 0;
#endif

	/* open the file */
	cpp_open(name);

	/* reset */
	scope = 0;
	gotname = FALSE;
	token = SEMICOLON;
	tagseek = 0L;
	*hint_class = '\0';

	/* parse until the end of the file */
	while (prev = token, (token = lex_gettoken()) != EOF)
	{
		if (make_parse)
		{
			switch (token)
			{
			  case  BODY:	   printf("{%s}\n", lex_body);	break;
			  case  ARGS:	   printf("() ");		break;
			  case  COMMA:	   printf(",\n");		break;
			  case  SEMICOLON: printf(";\n");		break;
			  case	CLASSSEP:  printf("::");		break;
			  case  TYPEDEF:   printf("typedef ");		break;
			  case	CLASS:	   printf("class ");		break;
			  case	STRUCT:	   printf("struct ");		break;
			  case	UNION:	   printf("union ");		break;
			  case	ENUM:	   printf("enum ");		break;
			  case  KSTATIC:   printf("static ");		break;
			  case  EXTERN:	   printf("extern ");		break;
			  case  NAME:	   printf("\"%s\" ", lex_name);	break;
			  case  INLINE:	   printf("inline ");		break;
			}
		}

		/* the class name is clobbered after most tokens */
		if (prev != NAME && prev != CLASSSEP && prev != ARGS)
		{
			*hint_class = '\0';
		}

		/* scope keyword? */
		if (token == TYPEDEF || token == KSTATIC
		 || token == EXTERN || token == CLASS
		 || token == STRUCT || token == UNION || token == ENUM)
		{
			/* If we don't have a scope already, then this is it */
			if (!scope || scope == TYPEDEF)
				scope = token;
			gotname = FALSE;
			continue;
		}

		/* inline ? */
		if (incl_inline && token == INLINE)
		{
			if (!scope || (scope == EXTERN)) scope = token;
			gotname = FALSE;
			continue;
		}

		/* name of a possible tag candidate? */
		if (token == NAME)
		{
			tagseek = file_seek;
			gotname = TRUE;
			continue;
		}

		/* if NAME CLASSSEP, then NAME is a possible class name */
		if (gotname && token == CLASSSEP)
		{
			strcpy(hint_class, lex_name);
			gotname = FALSE;
			continue;
		}

		/* if NAME BODY, without ARGS, then NAME is a struct tag or a
		 * class name
		 */
		if (gotname && token == BODY && prev != ARGS)
		{
			gotname = FALSE;
			
			/* ignore if in typedef -- better name is coming soon */
			if (scope == TYPEDEF)
			{
				continue;
			}

			/* generate a tag, if -t and maybe -s */
			if (incl_types)
			{
				maketag(file_header ? 0 : KSTATIC,
					lex_name, lex_lnum, tagseek,
					use_numbers,
					scope==CLASS ? "c" :
					scope==STRUCT ? "s" : "u",
					NULL);
			}
		}

		/* If NAME ARGS BODY, then NAME is a function */
		if (gotname && prev == ARGS && token == BODY)
		{
			gotname = FALSE;
			
			/* generate a tag, maybe checking -s */
			maketag(scope, lex_name, lex_lnum, tagseek, use_numbers, "f", NULL);
		}

		/* If NAME SEMICOLON or NAME COMMA, then NAME is var/typedef.
		 * Note that NAME ARGS SEMICOLON is an extern function
		 * declaration, even if the word "extern" was left off, so we
		 * need to guard against that possibility.
		 */
		if (gotname && (token == SEMICOLON || token == COMMA))
		{
			gotname = FALSE;

			if (prev == ARGS)
			{
				maketag(EXTERN, lex_name, lex_lnum, tagseek, use_numbers, "x", NULL);
			}
			/* generate a tag, if -v/-t and maybe -s */
			else if (scope==TYPEDEF || scope==STRUCT || scope==UNION ? incl_types : incl_vars)
			{
				/* a TYPEDEF outside of a header is KSTATIC */
				if (scope == TYPEDEF && !file_header)
				{
					maketag(KSTATIC, lex_name, lex_lnum, tagseek, use_numbers, "t", NULL);
				}
				else /* use whatever scope was declared */
				{
					maketag(scope, lex_name, lex_lnum, tagseek, use_numbers, scope==TYPEDEF || scope==STRUCT || scope==UNION ? "t" : "v", NULL);
				}

				/* if declaration included a body, then generate
				 * tags for the names in the body.
				 */
				mkbodytags(scope, lex_name, lex_body, lex_lnum);
			}
		}

		/* reset after a semicolon or ARGS BODY pair */
		if (token == SEMICOLON || (prev == ARGS && token == BODY))
		{
			scope = 0;
			gotname = FALSE;
			*lex_body = '\0';
		}
	}

	/* The source file will be automatically closed */
}

/* -------------------------------------------------------------------------- */

void usage()
{
	fprintf(stderr, "usage: ctags [flags] filenames...\n");
	fprintf(stderr, "\t-Dword  Ignore \"word\" -- handy for parameter macro name\n");
	fprintf(stderr, "\t-F      Use /regexp/ (default)\n");
	fprintf(stderr, "\t-B      Use ?regexp? instead of /regexp/\n");
	fprintf(stderr, "\t-N      Use line numbers instead of /regexp/\n");
	fprintf(stderr, "\t-g      Store static tags as though they were global (implies -h -s)\n");
	fprintf(stderr, "\t-s      Include static tags\n");
	fprintf(stderr, "\t-e      Include extern tags\n");
	fprintf(stderr, "\t-i      Include inline definitions\n");
	fprintf(stderr, "\t-t      Include typedefs\n");
	fprintf(stderr, "\t-v      Include variable declarations\n");
	fprintf(stderr, "\t-h      Add hints to help elvis distinguish between overloaded tags\n");
	fprintf(stderr, "\t-l      Add a \"ln\" line number hint (implies -h)\n");
	fprintf(stderr, "\t-p      Write parse info to stdout (for debugging ctags)\n");
	fprintf(stderr, "\t-d      Warn about duplicates, on stdout\n");
	fprintf(stderr, "\t-x      Write cross-reference table to stdout; skip \"tags\"\n");
	fprintf(stderr, "\t-r      Write a \"refs\" file, in addition to \"tags\"\n");
	fprintf(stderr, "\t-a      Append to \"tags\", instead of overwriting\n");
	fprintf(stderr, "If no flags are given, ctags assumes it should use -l -i -s -t -v\n");
	fprintf(stderr, "Report bugs to kirkenda@cs.pdx.edu\n");
	exit(2);
}



int main(argc, argv)
	int	argc;
	char	**argv;
{
	int	i, j;
#if OSEXPANDARGS
	char	*name;
#endif


#if defined (GUI_WIN32)
    set_current_file ("");
    set_current_tags (0);
    set_total_tags (0);
#endif

	/* detect special GNU flags */
	if (argc >= 2)
	{
		if (!strcmp(argv[1], "-version")
		 || !strcmp(argv[1], "--version"))
		{
			printf("ctags (elvis) %s\n", VERSION);
#ifdef COPY1
			puts(COPY1);
#endif
#ifdef COPY2
			puts(COPY2);
#endif
#ifdef COPY3
			puts(COPY3);
#endif
#ifdef COPY4
			puts(COPY4);
#endif
#ifdef COPY5
			puts(COPY5);
#endif
#ifdef GUI_WIN32
# ifdef PORTEDBY
			puts(PORTEDBY);
# endif
#endif
			exit(0);
		}
		else if (!strcmp(argv[1], "/?"))
		{
			usage();
		}
	}

	/* parse the option flags */
	for (i = 1; i < argc && argv[i][0] == '-'; i++)
	{
		for (j = 1; argv[i][j]; j++)
		{
			switch (argv[i][j])
			{
			  case 'D':
				if (argv[i][j + 1])
					del_word = &argv[i][j + 1];
				else if (i + 1 < argc)
					del_word = argv[++i];
				j = strlen(argv[i]) - 1;/* to exit inner loop */
				break;
			  case 'F':	backward = FALSE;		break;
			  case 'B':	backward = TRUE;		break;
			  case 'N':	use_numbers = TRUE;		break;
			  case 'g':	as_global = TRUE;		break;
			  case 's':	incl_static = TRUE;		break;
			  case 'e':	incl_extern = TRUE;		break;
			  case 'i':	incl_inline = TRUE;		break;
			  case 't':	incl_types = TRUE;		break;
			  case 'v':	incl_vars = TRUE;		break;
			  case 'r':	make_refs = TRUE;		break;
			  case 'p':	make_parse = TRUE;		break;
			  case 'd':	warn_duplicate = TRUE;		break;
			  case 'x':	make_xtbl=TRUE;			break;
			  case 'a':	append_files = TRUE;		break;
			  case 'h':	add_hints = TRUE;		break;
			  case 'l':	add_ln = TRUE;			break;
			  default:	usage();
			}
		}
	}

	/* There should always be at least one source file named in args */
	if (i == argc)
	{
		usage();
	}

	/* Some flags imply other flags */
	if (as_global)
		incl_static = add_hints = TRUE;
	if (add_ln)
		add_hints = TRUE;
	if (make_xtbl)
		make_tags = FALSE;

	/* If no flags given on command line, then use big defaults */
	if (i == 1)
		add_ln = add_hints = incl_static = incl_types =
					incl_vars = incl_inline = TRUE;

	/* open the "tags" and maybe "refs" files */
	if (make_tags)
	{
		tags = fopen(TAGS, append_files ? "a" : "w");
		if (!tags)
		{
			perror(TAGS);
			exit(3);
		}
	}
	if (make_refs)
	{
		refs = fopen(REFS, append_files ? "a" : "w");
		if (!refs)
		{
			perror(REFS);
			exit(4);
		}
	}

	/* Ensure that the "kind" attribute is always in a consistent place.
	 * This makes the source code in ctags.c easier to implement.
	 */
	tagattrname[TAGKIND_INDEX] = strdup("kind");

	/* parse each source file */
	for (; i < argc; i++)
	{
#if OSEXPANDARGS
		for (name = dirfirst(argv[i], ElvFalse); name; name = dirnext())
		{
			ctags(name);
		}
#else
		ctags(argv[i]);
#endif
	}

	/* if generating a tags file (i.e., if not "-x") ... */
	if (make_tags)
	{
		/* add the extra format args */
		fprintf(tags, "!_TAG_FILE_FORMAT\t%d\t/supported features/\n", add_hints ? 2 : 1);
		fprintf(tags, "!_TAG_FILE_SORTED\t1\t/0=unsorted, 1=sorted/\n");
		fprintf(tags, "!_TAG_PROGRAM_AUTHOR\tSteve Kirkendall\t/kirkenda@cs.pdx.edu/\n");
		fprintf(tags, "!_TAG_PROGRAM_NAME\tElvis Ctags\t//\n");
		fprintf(tags, "!_TAG_PROGRAM_URL\tftp://ftp.cs.pdx.edu/pub/elvis/README.html\t/official site/\n");
		fprintf(tags, "!_TAG_PROGRAM_VERSION\t%s\t//\n", VERSION);

		/* write the tags */
		while (taglist)
		{
			fprintf(tags, "%s\t%s\t%s",
				taglist->TAGNAME, taglist->TAGFILE, taglist->TAGADDR);
			for (i = 3, j = -1; i < MAXATTR; i++)
			{
				if (taglist->attr[i])
				{
					if (j == -1)
						fprintf(tags, ";\"");
					if (strcmp(tagattrname[i], "kind"))
						fprintf(tags, "\t%s:", tagattrname[i]);
					else
						putc('\t', tags);
					for (j = 0; taglist->attr[i][j]; j++)
					{
						switch (taglist->attr[i][j])
						{
						  case '\\':
							putc('\\', tags);
							putc('\\', tags);
							break;

						  case '\n':
							putc('\\', tags);
							putc('n', tags);
							break;

						  case '\r':
							putc('\\', tags);
							putc('r', tags);
							break;

						  case '\t':
							putc('\\', tags);
							putc('t', tags);
							break;

						  default:
							putc(taglist->attr[i][j], tags);
						}
					}
				}
			}
			putc('\n', tags);
			tagdelete(ElvFalse);
		}
	}

	/* close "tags" and maybe "refs" */
	if (make_tags)
	{
		fclose(tags);
	}
	if (make_refs)
	{
		fclose(refs);
	}

	exit(0);
	/*NOTREACHED*/
}
