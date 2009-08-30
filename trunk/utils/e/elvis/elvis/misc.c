/* misc.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_misc[] = "$Id: misc.c 167211 2008-04-24 17:26:56Z sboucher $";
#endif



/* This is used as a zero-length string */
CHAR	empty[1];



/* This is used when we need a bunch of blanks */
CHAR	blanks[80] = {
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
	' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '
};



/* These store the args list.  The "arglist" variable points to a dynamically
 * allocated array of (char *) pointers.  Each element of the array ppoints to
 * a dynamically allocated string, except that the last one is NULL.  The
 * "argnext" variable stores the index of the next (not current!) element
 * to be edited.
 */
char	**arglist;	/* array of strings (dynamically allocated) */
int	argnext;	/* index into arglist[] of next arg */



/* This function appends a single character to a dynamically-allocated
 * string.  A NUL character is always appended after the last character,
 * but this function also supports NUL characters in the middle of the
 * string.
 *
 * Only one string can be under construction at a time.  To start a string,
 * Call this function with a pointer to a (CHAR *) variable which is NULL.
 * To append to that string, call this function with a pointer to the same
 * (CHAR *) variable.
 *
 * This function updates the value of the (CHAR *) variable whenever it
 * reallocates memory.  It returns the number of characters added so far,
 * excluding the terminal NUL.
 */
#ifdef DEBUG_ALLOC
int _buildCHAR(file, line, refstr, ch)
	char	*file;
	int	line;
#else
int buildCHAR(refstr, ch)
#endif
	CHAR	**refstr;	/* pointer to variable which points to string */
	_CHAR_	ch;		/* character to append to that string */
{
	static int	len;	/* length of the string so far */
	CHAR		*newp;	/* new memory for the same string */
#define GRANULARITY	32	/* minimum number of chars to allocate */

	/* if the string pointer is currently NULL, then start a new string */
	if (!*refstr)
	{
		len = 0;
#ifdef DEBUG_ALLOC
		*refstr = (CHAR *)_safealloc(file, line, ElvFalse, GRANULARITY, sizeof(CHAR));
#else
		*refstr = (CHAR *)safealloc(GRANULARITY, sizeof(CHAR));
#endif
	}

	/* if the string is expanding beyond the current allocated memory,
	 * then allocate some new memory and copy the string into it.
	 */
	if ((len + 1) % GRANULARITY == 0)
	{
#ifdef DEBUG_ALLOC
		newp = (CHAR *)_safealloc(file, line, ElvFalse, len + 1 + GRANULARITY, sizeof(CHAR));
#else
		newp = (CHAR *)safealloc(len + 1 + GRANULARITY, sizeof(CHAR));
#endif
		memcpy(newp, *refstr, len * sizeof(CHAR));
		safefree(*refstr);
		*refstr = newp;
	}

	/* append the new character, and a NUL character */
	(*refstr)[len++] = ch;
	(*refstr)[len] = '\0';
	return len;
}


/* This function calls buildCHAR() for each character of an argument string.
 * Note that the string is a plain old "char" string, not a "CHAR" string.
 */
#ifdef DEBUG_ALLOC
int _buildstr(file, line, refstr, add)
	char	*file;
	int	line;
#else
int buildstr(refstr, add)
#endif
	CHAR	**refstr;	/* pointer to variable which points to string */
	char	*add;		/* a string to be added */
{
	int	len;

	for (len = 0; *add; add++)
#ifdef DEBUG_ALLOC
		len = _buildCHAR(file, line, refstr, *add);
#else
		len = buildCHAR(refstr, *add);
#endif
	return len;
}


/* This function finds the endpoints of the word at a given point.  Upon
 * return, the offset of the argument MARK will have been changed to the
 * character after the end of the word, and this function will return a
 * static temporary MARK which points to the start of the word.  Exception:
 * If the argument MARK isn't on a word, this function leaves it unchanged
 * and returns NULL.
 */
MARK wordatcursor(cursor, apostrophe)
	MARK	cursor;	/* some point in the word */
	ELVBOOL	apostrophe;	/* allow apostrophe between letters? */
{
 static	MARKBUF	retmark;/* the return value */
	CHAR	*p;
	CHAR	prev;

	/* If "cursor" is NULL, fail */
	if (!cursor)
	{
		return NULL;
	}

	/* If "cursor" isn't on a letter, digit, or underscore, then fail */
	scanalloc(&p, cursor);
	if (!p || (!elvalnum(*p) && *p != '_' && !(apostrophe && *p == '\'')))
	{
		scanfree(&p);
		return NULL;
	}

	/* search back to the start of the word */
	retmark = *cursor;
	do
	{
		prev = *p;
		scanprev(&p);
		markaddoffset(&retmark, -1);
	} while (p && (elvalnum(*p) || *p == '_' || (apostrophe && *p == '\'' && prev != '\'')));
	markaddoffset(&retmark, 1);

	/* can't start on an apostrophe */
	if (apostrophe && scanchar(&retmark) == '\'')
		markaddoffset(&retmark, 1);

	/* search forward to the end of the word */
	scanseek(&p, cursor);
	do
	{
		prev = *p;
		scannext(&p);
	} while (p && (elvalnum(*p) || *p == '_' || (apostrophe && *p == '\'' && prev != '\'')));

	/* can't end on an apostrophe */
	if (apostrophe && prev == '\'')
	{
		scanprev(&p);
	}

	/* length must be at least 1 */
	if (markoffset(scanmark(&p)) - markoffset(&retmark) < 1)
	{
		scanfree(&p);
		return NULL;
	}

	/* move the cursor to the end of the word */
	marksetoffset(cursor, markoffset(scanmark(&p)));

	/* clean up & return the front of the word */
	scanfree(&p);
	return &retmark;
}


/* Return a copy of str with backslashes before chars.  The calling function
 * is responsible for freeing the returned string when it is no longer needed.
 *
 * This also adds a backslash before each existing backslash, unless the
 * existing backslash is followed by a letter or digit, or appears at the end
 * of str.
 */
CHAR *addquotes(chars, str)
	CHAR	*chars;	/* list of chars to be quoted, other than backslash */
	CHAR	*str;	/* the string to be quoted */
{
	CHAR	*tmp;

	/* build a quoted copy of the string */
	for (tmp = NULL; *str; str++)
	{
		if ((*str == '\\' && str[1] && !elvalnum(str[1]))
		 || CHARchr(chars, *str))
			buildCHAR(&tmp, '\\');
		buildCHAR(&tmp, *str);
	}

	/* if empty string, then return "" instead of NULL */
	if (tmp == NULL)
		tmp = (CHAR *)safealloc(1, sizeof(CHAR));

	/* return the copy */
	return tmp;
}

/* Return a copy of str, from which the backslash characters have been
 * removed if they're followed by certain other characters.  This is intended
 * to be the exact opposite of the addquotes() function.
 */
CHAR *removequotes(chars, str)
	CHAR	*chars;	/* list of chars to be quoted, other than backslash */
	CHAR	*str;	/* the string to be quoted */
{
	CHAR	*tmp;

	/* build an unquoted copy of the string */
	for (tmp = NULL; *str; str++)
	{
		if (*str != '\\'
		 || (!str[1] || (str[1] != '\\' && !CHARchr(chars, str[1]))))
		buildCHAR(&tmp, *str);
	}

	/* if empty string, then return "" instead of NULL */
	if (tmp == NULL)
		tmp = (CHAR *)safealloc(1, sizeof(CHAR));

	/* return the copy */
	return tmp;
}

/* Compare two strings in a case-insensitive way */
int CHARncasecmp(s1, s2, len)
	CHAR	*s1, *s2;	/* strings to compare */
	int	len;		/* length of the strings to compare */
{
	/* look for a difference */
	while (len > 0 && elvtolower(*s1) == elvtolower(*s2))
	{
		len--;
		s1++;
		s2++;
	}

	/* return the difference */
	if (len > 0)
		len = elvtolower(*s1) - elvtolower(*s2);
	return len;
}
