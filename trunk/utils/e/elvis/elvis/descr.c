/* descr.c */

/* Copyright 2000 by Steve Kirkendall.  Redistributable under the terms of
 * the Perl "Clarified Artistic" license.
 */

#include "elvis.h"

#ifdef DISPLAY_ANYDESCR

typedef struct descr_s
{
	struct descr_s	*next;	/* some other description, or NULL */
	char		*file;	/* basename of description file */
	CHAR		*lang;	/* name of language */
	CHAR		*ext;	/* filename extensions, if known */
	unsigned	len;	/* lengths of "ext" field strings */
	void		*desc;	/* description data */
} descr_t;

static CHAR	lang[50];/* language of current description */
static CHAR	ext[50]; /* filename extension of current description, or "" */
static char	*file;	 /* either MARKUP_FILE or SYNTAX_FILE */

#ifdef FEATURE_CACHEDESC
static descr_t	*descriptions;
#endif

static ELVBOOL wrongext P_((CHAR *filename, int len, CHAR *extension));
static CHAR **fetchline P_((void));
static ELVBOOL findbylanguage P_((CHAR *language));
static ELVBOOL findbyextension P_((CHAR *filename));

#ifdef DISPLAY_SYNTAX
static void callext P_((CHAR *ext));

static CHAR	callsbuf[500];	/* buffer, holds text of extensions */
static CHAR	*calls[50];	/* extensions that are callable by this lang */
static int	callslen[50];	/* lengths of extensions */
static int	ncalls;		/* number of callable extensions */
static int	ncallbuf;	/* used for allocating space in callsbuf */

/* If called with NULL, it clobbers the "calls" variables.  Otherwise, the
 * value should be an extension to add to the list.
 */
static void callext(ext)
	CHAR *ext;
{
	int	len;

	if (ext)
	{
		/* is there room to add it? */
		len = CHARlen(ext);
		if (ncalls >= QTY(calls) - 1
		 || ncallbuf + len + 1 >= QTY(callsbuf))
		{
			/* no, so ignore it */
			return;
		}

		/* else add it */
		CHARcpy(callsbuf + ncallbuf, ext);
		callslen[ncalls] = len;
		calls[ncalls] = callsbuf + ncallbuf;
		ncallbuf += len + 1;
		ncalls++;
	}
	else
	{
		/* clobber it */
		ncalls = ncallbuf = 0;
	}
}
#endif /* DISPLAY_SYNTAX */

/* Compare a filename to an extension.  If they match then return FALSE;
 * else return TRUE.  The comparison is first done in a case-sensitive way,
 * and if that fails then it will try again, converting the filename to
 * lowercase.  Thus, "foo.C" matches a ".C" or ".c" extension, but "bar.c"
 * could only match a ".c" extension.
 */
static ELVBOOL wrongext(filename, flen, ext)
	CHAR	*filename;	/* file name (case insensitive) */
	int	flen;		/* length of filename */
	CHAR	*ext;		/* extension (case sensitive if uppercase) */
{
	int	xlen = CHARlen(ext);

	/* if extension is obviously too long to match filename, then quit */
	if (xlen > flen)
		return ElvTrue;

	/* compare characters, converting filename to lowercase if necessary */
	for (filename += flen - xlen; *filename; filename++, ext++)
	{
		if (*filename != *ext && elvtolower(*filename) != *ext)
			return ElvTrue;
	}

	/* no mismatched chars found, so I guess it matches */
	return ElvFalse;
}

/* Read a line from a file via ioread(), and parse it into words.  Skip any
 * blank lines or comments.
 */
static CHAR **fetchline()
{
	static CHAR	line[500];
	static CHAR	*word[50];
	CHAR		ch;
	int		w, l;
	ELVBOOL		inword;
	int		nread;

	do
	{
		for (w = l = 0, inword = ElvFalse;
		     (nread = ioread(&ch, 1)) == 1 && ch != '\n';
		     )
		{
			if (elvspace(ch))
			{
				if (inword)
				{
					line[l++] = '\0';
					inword = ElvFalse;
				}
			}
			else
			{
				if (!inword)
				{
					word[w++] = &line[l];
					inword = ElvTrue;
				}
				line[l++] = ch;
			}
		}
		if (inword)
			line[l] = '\0';
		word[w] = NULL;
	} while (nread == 1 && (!word[0] || *word[0] == '#'));
	return nread==1 ? word : NULL;
}

/* Read lines via ioread() until the first "language" line with the given
 * language.  Return ElvTrue if found, or ElvFalse otherwise.
 */
static ELVBOOL findbylanguage(language)
	CHAR	*language;
{
	CHAR	**values;
	int	i;

	while ((values = fetchline()) != NULL)
	{
		if (CHARcmp(values[0], toCHAR("language")))
			continue;
		for (i = 1; values[i] && CHARcmp(values[i], language); i++)
		{
		}
		if (values[i])
		{
			CHARcpy(lang, language);
			*ext = '\0';
			return ElvTrue;
		}
	}
	return ElvFalse;
}

/* Read lines via ioread() until the first "extension" line which matches the
 * given filename's extension.  Return ElvTrue if found, or ElvFalse otherwise.
 */
static ELVBOOL findbyextension(filename)
	CHAR	*filename;
{
	unsigned len;
	int	i;
	CHAR	**values;

	/* locate an "extension" line that ends like filename */
	len = CHARlen(filename);
	while ((values = fetchline()) != NULL)
	{
		/* remember language, just in case this is the one we want */
		if (!CHARcmp(values[0], toCHAR("language")) && values[1])
			CHARcpy(lang, values[1]);

		/* if not an "extension" line then skip */
		if (CHARcmp(values[0], toCHAR("extension")))
			continue;

		/* Does any extension here match the filename? */
		for (i = 1; values[i] && wrongext(filename, len, values[i]); i++)
		{
		}
		if (values[i])
		{
			/* remember the extension for this description */
			CHARcpy(ext, values[i]);

#ifdef DISPLAY_SYNTAX
			/* remember every extension for descr_cancall(). */
			for (i = 1; values[i]; i++)
				callext(values[i]);
#endif
			return ElvTrue;
		}
	}
	return ElvFalse;
}

#ifdef FEATURE_CACHEDESC
/* Attempt to locate a previously-recalled description, based on either the
 * display option of a window, or the filename extension of its buffer.
 * If found, return a pointer to its description (cast to (void *) for
 * the sake of code reuse); otherwise return NULL.
 */
void *descr_recall(win, dfile)
	WINDOW	win;		/* window in which to load the markup */
	char	*dfile;		/* either SYNTAX_FILE or MARKUP_FILE */
{
	CHAR	*cp;
	descr_t	*descr;
	unsigned len;

	/* Determine whether we want to search by language or extension */
	cp = CHARchr(o_display(win), ' ');
	if (cp)
	{
		cp++;
		for (descr = descriptions; descr; descr = descr->next)
			if (!CHARcmp(descr->lang, cp)
			 && !strcmp(descr->file, dfile))
				return descr->desc;
	}
	else if (o_filename(markbuffer(win->cursor)))
	{
		cp = o_filename(markbuffer(win->cursor));
		len = CHARlen(cp);
		for (descr = descriptions; descr; descr = descr->next)
		{
			/* Note that we do CASE SENSITIVE comparisons here.
			 * We only use case insensitive comparisons when
			 * loading a description.
			 */
			if (descr->ext
			 && descr->len <= len
			 && !CHARcmp(cp + len - descr->len, descr->ext)
			 && !strcmp(descr->file, dfile))
				return descr->desc;
		}
	}
	return NULL;
}
#endif /* defined FEATURE_CACHEDESC */

/* Open a description file, and skip ahead to the start of a description
 * that is appropriate for a given window, using either the window's display
 * option or the filename extension of its buffer.  Return ElvTrue if
 * successful, or ElvFalse if no description was found.
 *
 * After reading the file, you must call descr_close() to close the file and
 * store the description.
 */
ELVBOOL descr_open(win, dfile)
	WINDOW	win;		/* window in which to load the markup */
	char	*dfile;		/* either SYNTAX_FILE or MARKUP_FILE */
{
	CHAR	*cp;
	ELVBOOL	result;
	char	*pathname;

	/* Clobber the calls[] table */
	callext(NULL);

	/* Attempt to locate and open the description file */
	pathname = iopath(tochar8(o_elvispath), dfile, ElvFalse);
	if (!pathname || !ioopen(pathname, 'r', ElvFalse, ElvFalse, 't'))
		return ElvFalse;

	/* Determine whether we want to search by language or extension */
	cp = CHARchr(o_display(win), ' ');
	if (cp)
		result = findbylanguage(cp + 1);
	else if (o_filename(markbuffer(win->cursor)))
		result = findbyextension(o_filename(markbuffer(win->cursor)));
	else
		result = ElvFalse;

	/* If failed, then close the file */
	if (!result)
		ioclose();
	else
		file = dfile;

	return result;
}

/* Read the next line, parse it into words, and return an array of the
 * words.  At the end of the current description, return NULL instead.
 */
CHAR **descr_line()
{
	CHAR	**values, *end;
	int	i;

	/* read a line */
	values  = fetchline();

	/* skip any "extension" or "foreign" lines, but remember extensions */
	while (values && (!CHARcmp(values[0], toCHAR("extension"))
		       || !CHARcmp(values[0], toCHAR("foreign"))))
	{
		/* remember these extensions */
		for (i = 1; values[i]; i++)
			callext(values[i]);

		/* try another line */
		values  = fetchline();
	}

	/* stop reading at EOF, or at next "language" line. */
	if (!values || !CHARcmp(values[0], toCHAR("language")))
		return NULL;

	/* if "color" or "set" line, then we really didn't want to parse it
	 * into a huge number of words.  For "color", everything from values[2]
	 * onward should be one long string; for "set", everything from
	 * values[1] onward should be one long string.
	 */
	if ((!CHARcmp(values[0], toCHAR("color")) && values[1] && values[2])
	 || (!CHARcmp(values[0], toCHAR("set")) && values[1]))
	{
		for (i = (**values == 'c') ? 2 : 1; values[i + 1]; i++)
			for (end = values[i] + CHARlen(values[i]); !*end; )
				*end++ = ' ';
		values[(**values == 'c') ? 3 : 2] = NULL;
	}

	return values;
}


/* Close a file opened via descr_open(), and remember the parsed description
 * info for later descr_recall() calls.
 */
void descr_close(descr)
	void	*descr;	/* value to return in later descr_recall() calls */
{
#ifdef FEATURE_CACHEDESC
	descr_t	*d;
#endif

	/* close the file */
	ioclose();

#ifdef FEATURE_CACHEDESC
	/* if descr is NULL, then we're done */
	if (!descr)
		return;

	/* create a description struct */
	d = (descr_t *)safekept(1, sizeof *d);
	d->file = file; /* not duped, since always MARKUP_FILE or SYNTAX_FILE */
	d->lang = CHARdup(lang);
	d->ext = *ext ? CHARdup(ext) : NULL;
	d->desc = descr;
	d->len = d->ext ? CHARlen(d->ext) : 0;

	/* insert it into the list of known descriptions */
	d->next = descriptions;
	descriptions = d;
#endif /* undef FEATURE_CACHEDESC */
}

/* If a filename can be found by its extension then return the name of the
 * language; else return NULL.
 */
CHAR *descr_known(filename, dfile)
	char	*filename;/* filename whose extension is to be checked */
	char	*dfile;	/* descriptions -- either SYNTAX_FILE or MARKUP_FILE */
{
#ifdef FEATURE_CACHEDESC
	descr_t	*d;	/* used for scanning through the descriptions list */
#endif
	unsigned len;	/* length of filename */
	CHAR	*file2;	/* CHAR version of filename */
	char	*pathname;

	/* convert filename to make it easier to compare */
	file2 = toCHAR(filename);
	len = CHARlen(file2);

#ifdef FEATURE_CACHEDESC
	/* first check the description list.  THIS IS CASE SENSITIVE!
	 * Although we load descriptions in a case-insensitive way, we must
	 * check the cached descriptions with case-sensitivity because there
	 * are some ambiguities in case-insensitive extensions which are
	 * resolved by the order in which the languages appear in the
	 * elvis.syn file.  The cache doesn't have any way to resolve those
	 * ambiguities.
	 */
	for (d = descriptions; d; d = d->next)
	{
		if (d->ext
		 && len >= d->len
		 && !CHARcmp(file2 + len - d->len, d->ext)
		 && !strcmp(d->file, dfile))
			return d->lang;
	}
#endif

	/* attempt to locate and open the description file */
	pathname = iopath(tochar8(o_elvispath), dfile, ElvFalse);
	if (!pathname || !ioopen(pathname, 'r', ElvFalse, ElvFalse, 't'))
		return NULL;

	/* attempt to locate an extension line which matches this file */
	if (findbyextension(file2))
	{
		ioclose();
		return lang;
	}
	ioclose();

	/* couldn't find it anywhere */
	return NULL;
}


#ifdef DISPLAY_SYNTAX
/* Return ElvTrue if the current language can call functions in a given file, or
 * ElvFalse otherwise.  This function is used when scanning for library tags.
 *
 * This function should only be called after descr_open() returns ElvTrue.  In
 * particular, it will *NOT* work after a descr_recall() call.
 */
ELVBOOL descr_cancall(filename)
	CHAR *filename;	/* name of some other source file */
{
	int	i, len;

	/* find the length of this filename */
	len = CHARlen(filename);

	/* for each possible extension... */
	for (i = 0; i < ncalls; i++)
	{
		/* if this file has this extension... */
		if (!wrongext(filename, len, calls[i]))
		{
			/* Yes, this language can call funcs in this file */
			return ElvTrue;
		}
	}
	return ElvFalse;
}

# endif /* DISPLAY_SYNTAX */
#endif /* DISPLAY_ANYDESCR */
