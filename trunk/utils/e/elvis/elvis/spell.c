/* spell.c */

/* This file contains functions which maintain lists of words, with attributes.
 * The list is organized as a tree, which allows it to be searched very quickly.
 * Multiple lists can be maintained; the root node of the tree is passed as one
 * of the arguments.  This type of word list is used for storing the keywords
 * in the "syntax" display mode, as well as a real spell-checker.
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_spell[] = "$Id: spell.c,v 1.38 2003/10/17 17:41:23 steve Exp $";
#endif

#define MAXWLEN	100	/* maximum word length */


#if defined(FEATURE_SPELL) || defined(DISPLAY_ANYDESCR)
/* Note that even if the spell-checking feature is disabled, we still need
 * the following dictionary management functions, because the syntax display
 * mode uses dictionaries for storing keywords.
 */

/* Perform spell-checking on a single letter within a word.  Initially (at the
 * start of a word), the node should be the top node of a dictionary.  For
 * each successive letter in the word, the node should be the value returned
 * by the previous invocation of spellletter().
 *
 * Returns a new spell_t node if successful, or NULL if the word is definitely
 * not in the list.  At the end of the word, you should use the SPELL_IS_GOOD()
 * macro to determine whether the word is in the dictionary.
 */
spell_t *spellletter(node, letter)
	spell_t	*node;	/* top of dictionary, or value returned by previous call */
	_CHAR_	letter;	/* the first/next letter to check */
{
	
	if (!node || letter < node->min || letter > node->max)
		return NULL;
	return node->link[letter - node->min];
}

/* Perform spell checking on a whole word.  If len is 0 then the word is
 * assumed to be NUL-terminated; else the word's length is the lesser of len
 * or CHARlen(word).
 *
 * Returns NULL if unknown word, or the spell_t for a known or partial word.
 * You should use the SPELL_IS_GOOD() macro to determine whether the return
 * value indicates that the word is in the dictionary.
 */
spell_t *spellfindword(node, word, len)
	spell_t	*node;	/* top of the dictionary */
	CHAR	*word;	/* the word to check */
	int	len;	/* maximum length to check, or 0 for any length */
{
	/* the default length is determined by looking for a NUL terminator */
	if (len == 0)
		len = CHARlen(word);

	/* check each letter of the word */
	for (; len > 0 && *word; len--, word++)
	{
		/* check the next letter, ignoring case differences */
		node = spellletter(node, *word);
		if (!node)
			break;
	}
	
	/* return the final result */
	return node;
}

/* Add a word to a dictionary, and return the dictionary's root node.
 * Initially (before the first word has been added) the dictionary's root
 * node should be NULL.  Thereafter, the root node should be whatever this
 * function returns.
 *
 * The 28 least-significant bits of "flags" are unspecified; the calling
 * function can use them to store extra information about the word.  The 4
 * most-significant bits are used for the SPELL_FLAG_COMPLETE, SPELL_FLAG_BAD,
 * and SPELL_FLAG_PERSONAL flags.  (The last bit is currently unused.)
 * spelladdword() always turns on the SPELL_FLAG_COMPLETE bit, so often
 * you'll simply leave those 4 bits off.
 *
 * Later, when you look up a word via spellletter() or spellfindword(), if
 * SPELL_IS_GOOD() indicates that the word is in the dictionary, you can use
 * (node->flags) to access the word's flags.
 */
spell_t *spelladdword(node, word, flags)
	spell_t	*node;	/* root node of a dictionary */
	CHAR	*word;	/* NULL-terminated word to add */
	long	flags;	/* bitmap of flags */
{
	int	c, i;
	spell_t *newnode;

#ifndef NDEBUG
	for (i = 0; word[i]; i++)
		assert(!elvspace(word[i]));
	assert(i < MAXWLEN);
#endif
#if 0
	if (~flags & SPELL_FLAG_COMPLETE)
		printf("spelladdword(%s, \"%s\", 0x%lx)\n",
			!node?"NULL":node==spellwords?"spellwords":"spelltags",
			word, flags);
#endif

	/* always set the SPELL_FLAG_COMPLETE flag */
	flags |= SPELL_FLAG_COMPLETE;

	/* if there is no such node, then create one */
	if (!node)
	{
		node = (spell_t *)safekept(1, sizeof(spell_t));
	}

	/* if at end of word, then just store the flags and return */
	if (!*word)
	{
		if (node)
			node->flags = flags;
		return node;
	}

	/* if this char won't fit in this node, then reallocate node */
	c = *word;
	if (node->max == 0) /* empty? will this be the first link? */
	{
		/* link[] already has 1 element, so this will fit */
		node->min = node->max = c;
	}
	else if (c < node->min) /* before current min? */
	{
		newnode = (spell_t *)safekept(1, sizeof(spell_t) + (node->max - c + 1) * sizeof(spell_t *));
		newnode->flags = node->flags;
		newnode->min = c;
		newnode->max = node->max;
		for (i = 0; i <= node->max - node->min; i++)
			newnode->link[node->min - c + i] = node->link[i];
		safefree(node);
		node = newnode;
	}
	else if (c > node->max) /* after current max? */
	{
		newnode = (spell_t *)safekept(1, sizeof(spell_t) + (c - node->min + 1) * sizeof(spell_t *));
		newnode->flags = node->flags;
		newnode->min = node->min;
		newnode->max = c;
		for (i = 0; i <= node->max - node->min; i++)
			newnode->link[i] = node->link[i];
		safefree(node);
		node = newnode;
	}

	/* compute the index of this char's node->link[] pointer */
	i = c - node->min;

	/* recursively add the remainder of the word */
	node->link[i] = spelladdword(node->link[i], word + 1, flags);

	/* return this node */
	return node;
}

/* Discard a spelling dictionary */
void spellfree(node)
	spell_t	*node;	/* root node of a dictionary tree to free */
{
	int	i;

	/* if NULL, then do nothing */
	if (!node)
		return;

	/* recursively free any subtrees */
	for (i = 0; i <= node->max - node->min; i++)
		spellfree(node->link[i]);

	/* free this node */
	safefree(node);
}
#endif /* FEATURE_SPELL or DISPLAY_ANYDESCR */


#ifdef FEATURE_SPELL

static void wildspell P_((spell_t *node, CHAR *word, CHAR *partial));
static void savehelp P_((spell_t *node, CHAR *partial));
static void spelldraw P_((CHAR *p, long qty, _char_ font, long offset));

/* font to use to highlight spelling errors */
char spellfont;

/* dictionaries */
spell_t	*spelltags;	/* spelling words from "tags" files */
spell_t	*spellwords;	/* spelling words from "words" file, or :words cmd */

/* font-dependent spellcheck limits */
static spellcheck_t spelllimit[128];
static ELVBOOL spellset[128];

/* info about the last value displayed via the "spellshow" option, or returned
 * by the last spellfix() function call.
 */
static MARKBUF	showbegin, showend;	/* extents of the word */
static CHAR	showsuggest[100];	/* suggested alternatives */
static long	showchange;		/* used to detect changes */


/* prepare for doing some spell-checking.  This updates the "tags" dictionary
 * if there are any user buffers which have "spell" set.
 */
void spellbegin()
{
	char	*newstamp;
 static	char	oldstamp[20];	/* YYYY-MM-DDThh:mm:ss */
	char	*path;
	char	*fullname;
	CHAR	tagname[MAXWLEN];
	BUFFER	buf;
	int	c, i;
	FILE	*fp;

	/* locate font (only required the first time) */
	if (!spellfont)
	{
		spellfont = colorfind(toCHAR("spell"));
		colorset(spellfont, toCHAR("underlined"), ElvFalse);
	}

	/* if no user buffers have "spell" set, then do nothing */
	for (buf = buflist((BUFFER)NULL);
	     buf && (o_internal(buf) || !o_spell(buf));
	     buf = buflist(buf))
	{
	}
	if (!buf)
		return;

	/* if timestamp of "tags" hasn't changed, then do nothing */
	newstamp = dirtime("tags");
	if (!newstamp || !strcmp(newstamp, oldstamp))
		return;
	strcpy(oldstamp, newstamp);

	/* free the old spelltags dictionary, if any */
	if (spelltags)
	{
		spellfree(spelltags);
		spelltags = NULL;
	}

	/* for each file in the elvispath or tagpath... */
	for (path = tochar8(o_elvispath ? o_elvispath : o_tags);
	     path;
	     path = (path == tochar8(o_tags)) ? NULL : tochar8(o_tags))
	{
		for (fullname = iopath(path, "tags", ElvTrue);
		     fullname;
		     fullname = iopath(NULL, "tags", ElvTrue))
		{
			/* for each tag in the file... */
			fp = fopen(fullname, "r");
			if (!fp)
				continue;
			for (i = 0; (c = getc(fp)) != EOF; )
			{
				if (c == '\n')
					i = 0;
				else if (i < 0)
					continue;
				else if (elvspace(c))
				{
					tagname[i] = '\0';
					if (tagname[0] > '!' && i < MAXWLEN - 1)
						spelltags = spelladdword(spelltags, tagname, 0);
					i = -1;
				}
				else if (i < MAXWLEN - 2)
					tagname[i++] = c;
			}
			fclose(fp);
		}
	}
}

/* This file pointer refers to a list of natural-language words */
static FILE *wordfp;
static long wordsize;

/* Search through the word file for a given word, using a binary search.
 * If found, then add the word to the given dictionary.  If not found, then
 * still add it but with the SPELL_FLAG_BAD bit set.   Return TRUE if good,
 * FALSE if bad.
 */
ELVBOOL spellsearch(word)
	CHAR	*word;	/* word to look for */
{
	long	pos, step;
	int	c;
	CHAR	*letterptr;

	/* if the file hasn't been opened yet, then open it now */
	if (!wordfp)
	{
		/* if no file is specified, then don't bother */
		if (!o_spelldict)
			return ElvFalse;

		/* try to open the file.  If error, then clobber the option */
		wordfp = fopen(tochar8(o_spelldict), "rb");
		if (!wordfp)
		{
			optputstr(toCHAR("spelldict"), toCHAR(""), ElvFalse);
			return ElvFalse;
		}

		/* get the size of the file */
		fseek(wordfp, 0L, SEEK_END);
		wordsize = ftell(wordfp);
	}

	/* perform binary search until within a few bytes of word */
	pos = 0;
	step = wordsize / 2;
	while (step > 100)
	{
		/* find start of line after seek position */
		fseek(wordfp, pos + step, SEEK_SET);
		while ((c = getc(wordfp)) != '\n' && c != EOF)
		{
		}

		/* compare to word */
		for (letterptr = word;
		     (c = elvtolower(getc(wordfp))) == *letterptr;
		     letterptr++)
		{
		}

		/* exact match, by any chance? */
		if (*letterptr == '\0' && elvspace(c))
		{
			spellwords = spelladdword(spellwords, word, 0);
			return ElvTrue;
		}

		/* if EOF or greater then we want to step back; else forward */
		if (c != EOF && c < *letterptr)
			pos += step;
		step /= 2;
	}

	/* we're close.  skip to start of next line, and then read linearly */
	fseek(wordfp, pos, SEEK_SET);
	if (pos != 0)
	{
		while ((c = getc(wordfp)) != '\n' && c != EOF)
		{
		}
	}

	/* now read each word linearly, comparing to desired word */
	letterptr = word;
	while ((c = elvtolower(getc(wordfp))) != EOF && c <= *letterptr)
	{
		if (c == '\r')
			continue;
		else if (c == '\n')
		{
			if (*letterptr == '\0')
				break;
			else
				letterptr = word;
		}
		else if (c == *letterptr)
			letterptr++;
		else
		{
			/* mismatch -- skip to next word */
			while ((c = getc(wordfp)) != EOF && c != '\n')
			{
			}
			letterptr = word;
		}
	}

	/* if the above search was successful, then *letterptr should point
	 * to the NUL at the end of the word and c should be non-whitespace.
	 * Any other result means the word wasn't found.
	 */
	if (*letterptr || !elvspace(c))
	{
		spellwords = spelladdword(spellwords, word, SPELL_FLAG_BAD);
		return ElvFalse;
	}
	else
	{
		spellwords = spelladdword(spellwords, word, 0);
		return ElvTrue;
	}
}

void spellend()
{
	/* if the words file is open, close it */
	if (wordfp)
	{
		fclose(wordfp);
		wordfp = NULL;
	}
}

/* Given a MARK for the start of a word, perform spell checking on the
 * word, and return the result.  Move the mark past the end of the word.
 */
spellresult_t spellcheck(mark, tagonly, cursoff)
	MARK	mark;		/* start of word */
	ELVBOOL	tagonly;	/* skip the natural-language dictionary? */
	long	cursoff;	/* offset of cursor */
{
	spell_t	*tag = spelltags;
	spell_t	*word = tagonly ? NULL : spellwords;
	spell_t	*icword = word;
	CHAR	*cp, letter;
	CHAR	wordbuf[MAXWLEN], swordbuf[MAXWLEN];
	int	wlen, slen, i;
	spellresult_t result;
	long	wordoff;

#if 0
	disp_t	*dm;

	/* choose the display mode, via the bufdisplay option */
	dm = ...
#endif

	/* get the first letter.  The character at the mark is assumed to
	 * be the first letter of the word, even if it isn't a letter.
	 */
	scanalloc(&cp, mark);
	wordoff = markoffset(mark);
#if 0
	if (dm->wordletter)
		letter = (*dm->wordletter)(cp, ElvTrue);
	else
#endif
	{
		letter = *cp;
		scannext(&cp);
	}

	/* check each character of the word */
	wlen = 0;
	do
	{
		/* add this letter to the word buffer */
		if (wlen < MAXWLEN)
			wordbuf[wlen++] = elvtolower(letter);

		/* look up this letter in the relevant dictionaries */
		tag = spellletter(tag, letter);
		word = spellletter(word, letter);
		icword = spellletter(icword, elvtolower(letter));

		/* get the next letter */
		if (!cp)
			letter = '\0';
		else if (elvalnum(*cp) || *cp == '_')
		{
			letter = *cp;
			scannext(&cp);
		}
		else if (*cp == '\'')
		{
			letter = *cp;
			scannext(&cp);
			if (!cp || !elvalpha(*cp))
			{
				if (cp)
					scanprev(&cp);
				letter = '\0';
			}
		}
#if 0
		else if (dm->wordletter)
			letter = (*dm->wordletter)(cp, ElvFalse);
#endif
		else
			letter = '\0';

	} while (letter);

	/* Move the mark past the end of the word.  We do this regardless of
	 * whether the word is correct or not.  This is slightly complicated
	 * by the fact that we may have hit the end of the buffer, in which
	 * case cp would be NULL.
	 */
	if (cp)
		marksetoffset(mark, markoffset(scanmark(&cp)));
	else
		marksetoffset(mark, o_bufchars(markbuffer(mark)));
	scanfree(&cp);

	/* if too long to be a valid word, then it must be bad */
	if (wlen >= MAXWLEN)
		return SPELL_BAD;
	wordbuf[wlen] = '\0';

	/* Decide whether to use "tag" or "word"/"icword".  We want to use
	 * "tag" if "word"/"icword" is NULL or incomplete; otherwise use
	 * "word"/"icword".
	 */
	if (!word || (~word->flags & SPELL_FLAG_COMPLETE))
		word = icword;
	if (!word || (tag && ((~word->flags & SPELL_FLAG_COMPLETE) || (word->flags & SPELL_FLAG_BAD))))
		word = tag;

	/* If the cursor is inside or at the end of an unknown or incomplete
	 * word, then pretend it is known to be "good".
	 */
	if (wordoff <= cursoff
	 && cursoff <= markoffset(mark)
	 && (!word || (~word->flags & SPELL_FLAG_COMPLETE)))
		return SPELL_GOOD;

	/* If unknown, and the natural-language dictionary is allowed, then
	 * search for the word.  Exception: If the cursor is located at the
	 * end of an incomplete word, then don't try this until the user
	 * completes the word.
	 */
	if (!tagonly
	 && o_spelldict
	 && (!word || (~word->flags & SPELL_FLAG_COMPLETE)))
	{
		if (!spellsearch(wordbuf) && o_spellsuffix)
		{

			/* not found as given -- try suffix substitution */
			for (cp = o_spellsuffix; *cp; cp++)
			{
				/* skip whitespace between suffixes */
				if (elvspace(*cp))
					continue;

				/* count the length of this suffix */
				for (slen = 0; cp[slen] && cp[slen] != '=' && !elvspace(cp[slen]); slen++)
				{
				}

				/* does the word end with this suffix? */
				if (slen < wlen && !CHARncmp(wordbuf + wlen - slen, cp, slen))
				{
					/* Yes! Use this suffix */
					CHARcpy(swordbuf, wordbuf);
					if (cp[slen] == '=')
					{
						for (i = 0; cp[slen + 1 + i] && !elvspace(cp[slen + 1 + i]); i++)
							swordbuf[wlen - slen + i] = cp[slen + 1 + i];
						swordbuf[wlen - slen + i] = '\0';
					}
					else
						swordbuf[wlen - slen] = '\0';

					/* Search for the alternate-suffix word.
					 * If that word is found and is good,
					 * then also add the word as it appears
					 * in the buffer.
					 */
					word = spellfindword(spellwords,swordbuf,0);
					if (SPELL_IS_GOOD(word)
					 || spellsearch(swordbuf))
					{
						spellwords = spelladdword(spellwords, wordbuf, 0);
					}
					else
					{
						spellwords = spelladdword(spellwords, wordbuf, SPELL_FLAG_BAD);
					}

					/* Stop after trying one suffix */
					break;
				}

				/* skip to end of this suffix */
				while (*cp && !elvspace(*cp))
					cp++;
			}
		}
		word = spellfindword(spellwords, wordbuf, wlen);
	}

	/* Classify the results */
	if (!word)
		result = SPELL_UNKNOWN;
	else if (~word->flags & SPELL_FLAG_COMPLETE)
		result = SPELL_INCOMPLETE;
	else if (word->flags & SPELL_FLAG_BAD)
		result = SPELL_BAD;
	else
		result = SPELL_GOOD;

	return result;
}

/* Locate each word on the screen, and highlight any that appear to be spelled
 * incorrectly.  Note that the attributes affect the spelling rules, and also
 * that in "markup" display modes, the markups themselves are normally hidden
 * and hence won't be spell checked.
 */
void spellhighlight(win)
	WINDOW	win;
{
	MARKBUF	mark;
	int	i, bottom;
	int	offset;
	char	newfont, combofont;
	long	end;
	CHAR	*cp;
	spellresult_t result;
	ELVBOOL	inputmode;	/* are we in input mode? */

	/* if spell-checking is disabled, then do nothing */
	if (!o_spell(markbuffer(win->cursor)))
		return;

	/* spell checking makes no sense in "hex" mode.  In fact, it is worse
	 * than useless since a punctuation character's hex value may look like
	 * a two-letter word on the screen, which confuses the spell checker.
	 */
	if (win->md == &dmhex)
		return;

	/* determine whether we're in input mode */
	inputmode = (ELVBOOL)((win->state->mapflags & MAP_INPUT) != 0);

	/* for each word on the screen... */
	scanalloc(&cp, win->di->topmark);
	offset = markoffset(win->di->topmark);
	end = markoffset(win->di->bottommark);
	i = 0;
	bottom = (win->di->rows - 1) * win->di->columns;
	mark.buffer = markbuffer(win->di->topmark);
	for (; cp && offset < end; scannext(&cp), offset++)
	{
		/* skip if not on screen */
		while (i < bottom && win->di->offsets[i] < offset)
			i++;
		if (i > bottom)
			break;
		if (win->di->offsets[i] > offset)
			continue;

		/* Skip if non-alphanumeric */
		if (!elvalnum(*cp) && *cp != '_')
#if 0
				 && dm->wordletter && !(*dm->wordletter)(cp, ElvTrue))
#endif
			continue;

		/* skip if font has "graphic" attribute or SPELL_CHECK_NONE */
		newfont = (win->di->newfont[i] & 0x7f);
		if (colorinfo[(int)newfont].da.bits & COLOR_GRAPHIC
		 || spelllimit[(int)newfont] == SPELL_CHECK_NONE)
			continue;

		/* okay, we have found a word.  "cp" points to it, "offset" is
		 * its offset, and "i" is its position in the screen.  Now we
		 * need to check the status of that word.  (This also locates
		 * the end of the word, by moving "mark".)
		 */
		mark.offset = offset;
		result = spellcheck(&mark,
			(ELVBOOL)(spelllimit[(int)newfont] == SPELL_CHECK_TAGONLY),
			inputmode ? markoffset(win->cursor) : INFINITY);

		/* if word began with a digit, then ignore errors */
		if (elvdigit(*cp))
			result = SPELL_GOOD;

		/* if sidescrolling is enabled, and this word began at the
		 * left edge of the screen, and that edge isn't the start of
		 * a line, then what we've really done here is spell-check
		 * a partial word, not a whole word.  To avoid possible false
		 * alarms, skip this word.  (We still had to call spellcheck()
		 * to find the end of the word.)
		 */
		if (!o_wrap(win)
		 && (i % win->di->columns) == 0
		 && win->di->skipped > 0)
		{
			/* move to the end of the word. */
			offset = mark.offset;
			scanseek(&cp, &mark);
			continue;
		}

		/* if the word is bad, then highlight it */
		if (result == SPELL_BAD
		 || result == SPELL_UNKNOWN
		 || (result == SPELL_INCOMPLETE && markoffset(win->cursor) != mark.offset))
		{
			while (i < bottom
			    && win->di->offsets[i] >= offset
			    && win->di->offsets[i] < mark.offset)
			{
				newfont = win->di->newfont[i];
				if (!(newfont & 0x7f))
					newfont |= o_hasfocus(win)
						? COLOR_FONT_NORMAL
						: COLOR_FONT_IDLE;
				combofont = win->di->newfont[i] =
						colortmp(newfont, spellfont);
				i++;
			}
		}

		/* move to the end of the word. */
		offset = mark.offset;
		scanseek(&cp, &mark);
	}
	scanfree(&cp);
}


static BUFFER savebuf;		/* buf to append lines to, or NULL to show */
static CHAR saveline[200];	/* buffer, holds a single line of words */
static CHAR savecmd[20];	/* beginning of line: either "" or "try words"*/
static CHAR saveword[MAXWLEN];	/* current word, or partial word */
static CHAR saveflag;		/* flag within saveline: either '+' or '-' */
static long savecount;		/* number of personal words found */

static void savehelp(node, partial)
	spell_t	*node;	/* node to dump */
	CHAR	*partial;/* partial word */
{
	CHAR	flag[2];
	MARKBUF	end;
	int	i;

	/* if no such node, then do nothing */
	if (!node)
		return;

	/* if this is a personal word, then dump it */
	if (node->flags & SPELL_FLAG_PERSONAL)
	{
		/* mark the end of this word */
		*partial = '\0';

		/* determine whether the flag is + or - */
		if (node->flags & SPELL_FLAG_BAD)
			flag[0] = '-';
		else
			flag[0] = '+';
		flag[1] = '\0';

		/* if it won't fit on this line, then start a new one */
		if (CHARlen(saveline) + (partial - saveword) > 77)
		{
			CHARcat(saveline, toCHAR("\n"));
			if (savebuf)
			{
				end.buffer = savebuf;
				end.offset = o_bufchars(savebuf);
				bufreplace(&end, &end, saveline, CHARlen(saveline));
			}
			else
				drawextext(windefault, saveline, CHARlen(saveline));
			CHARcpy(saveline, savecmd);
			saveflag = '+';
		}

		/* add the word */
		if (*saveline)
			CHARcat(saveline, toCHAR(" "));
		if (*flag != saveflag)
		{
			CHARcat(saveline, flag);
			saveflag = *flag;
		}
		CHARcat(saveline, saveword);
		savecount++;
	}

	/* check longer words too */
	for (i = node->min; i <= node->max; i++)
	{
		*partial = i;
		savehelp(node->link[*partial - node->min], partial+1);
	}
}

/* This function walks through the spellwords tree, and outputs any any
 * personal words that it finds there.  This is used for both :mkexrc,
 * and for listing the words via :word without arguments.
 */
void spellsave(custom)
	BUFFER	custom;	/* buffer to append spell commands to, or NULL */
{
	MARKBUF end;
	int	i, col, len;
	spellcheck_t level;

	/* use savehelp() to recursively search for personal words */
	savebuf = custom;
	if (custom)
		CHARcpy(savecmd, toCHAR("try words"));
	else
		*savecmd = '\0';
	CHARcpy(saveline, savecmd);
	saveflag = '+';
	savecount = 0L;
	savehelp(spellwords, saveword);

	/* finish the last command */
	if (savecount > 0)
	{
		CHARcat(saveline, toCHAR("\n"));
		if (custom)
		{
			end.buffer = custom;
			end.offset = o_bufchars(custom);
			bufreplace(&end, &end, saveline, CHARlen(saveline));
		}
		else
		{
			drawextext(windefault, saveline, CHARlen(saveline));
		}
	}

	/* if just displaying, then we're done. */
	if (!custom)
		return;

	/* also save the :check commands */
	end.buffer = custom;
	end.offset = o_bufchars(custom);
	for (level = SPELL_CHECK_ALL; level <= SPELL_CHECK_NONE; level++)
	{
		for (i = 1, col = 0; i < colornpermanent; i++)
		{
			if (spelllimit[i] != level || !spellset[i])
				continue;

			/* start a new command? */
			len = CHARlen(colorinfo[i].name);
			if (col == 0 || col + len > 77)
			{
				if (col > 0)
				{
					bufreplace(&end, &end, toCHAR("\n"), 1);
					end.offset++;
				}
				bufreplace(&end, &end,
					toCHAR(level == SPELL_CHECK_NONE
						? "try check -"
						: level == SPELL_CHECK_TAGONLY
							? "try check +"
							: "try check *"),
					11);
				end.offset += 11;
				col = 11;
			}
			else
			{
				/* space between words */
				bufreplace(&end, &end, blanks, 1);
				end.offset++;
				col++;
			}

			/* add this font name */
			bufreplace(&end, &end, colorinfo[i].name, len);
			end.offset += len;
		}

		/* end the line */
		if (col > 0)
		{
			bufreplace(&end, &end, toCHAR("\n"), 1);
			end.offset++;
		}
	}
}


/* These indicate where any found matches should go */
static CHAR	*wildbuf;	/* buffer */
static int	wildlen;	/* size of wildbuf */
static int	wildused;	/* used portion of wildbuf */
static ELVBOOL	wildcap;	/* should matching words be capitalized? */

/* Look up a potential corrected spelling.  This differs from spellfindword()
 * in the way it handles case mismatches, and also it allows ' ' as a wildcard
 * meaning "any single character".  If the word is found, then it will be
 * copied from the "partial" buffer onto the end of the "wildbuf".
 */
static void wildspell(node, word, partial)
	spell_t	*node;		/* tree to search */
	CHAR	*word;		/* partial candidate word to search for */
	CHAR	*partial;	/* partial actual word found */
{
	int	len, i;

	/* if no node, then do nothing */
	if (!node)
		return;

	/* if reached end of word, and it is complete, then use it */
	if (!*word)
	{
		len = (int)(partial - saveline);
		if (SPELL_IS_GOOD(node)
		 && (int)(partial - saveline) < wildlen - wildused - 2
		 && (wildused < len || CHARncmp(saveline, wildbuf + wildused - len, len)))
		{
			if (wildused != 0)
				wildbuf[wildused++] = ' ';
			CHARncpy(&wildbuf[wildused], saveline, len);
			if (wildcap)
				wildbuf[wildused] = elvtoupper(*saveline);
			wildused += len;
		}
		return;
	}

	/* if ' ' then try every character */
	if (*word == ' ')
	{
		len = node->max;
		for (i = node->min; i <= node->max; i++)
		{
			*partial = (CHAR)i;
			if (node->link[*partial - node->min])
				wildspell(node->link[*partial - node->min],
					word + 1, partial + 1);
		}
	}
	else
	{
		/* try an exact match for this character */
		*partial = *word;
		if (*partial >= node->min && *partial <= node->max
		  && node->link[*partial - node->min])
			wildspell(node->link[*partial - node->min],
				word + 1, partial + 1);

		/* if lowercase, then try an uppercase version of the same */
		if (elvlower(*word))
		{
			*partial = elvtoupper(*word);
			if (*partial >= node->min && *partial <= node->max
			  && node->link[*partial - node->min])
				wildspell(node->link[*partial - node->min], word + 1, partial + 1);
		}
	}
}

/* Perform spell-fixing on a whole word.
 *
 * The result is stored into the "result" CHAR buffer.  This will be "" for
 * valid words, "?" for invalid words with no suggestions, or a space-delimited
 * list of suggestions for invalid words that can be derived from valid ones.
 */
void spellfix(word, result, resultlen, tagonly)
	CHAR	*word;		/* the word to fix */
	CHAR	*result;	/* where to store the result */
	int	resultlen;	/* maximum length of result */
	ELVBOOL	tagonly;	/* ignore natural-language dictionary */
{
	int	i;
	CHAR	tmp, tmp2;
	int	len;		/* length of word */
	spell_t	*dict, *found;
	CHAR	*wordcopy;

	static CHAR *loadeddict;

	/* if given "" or number, then return "" */
	if (!*word || elvdigit(*word))
	{
		*result = '\0';
		return;
	}

	/* lookup the word unchanged.  If exactly right then return "" */
	found = spellfindword(spelltags, word, 0);
	if (SPELL_IS_GOOD(found))
	{
		*result = '\0';
		return;
	}
	if (!tagonly)
	{
		found = spellfindword(spellwords, word, 0);
		if (SPELL_IS_GOOD(found))
		{
			*result = '\0';
			return;
		}
	}

	/* Detect whether the word is capitalized -- meaning the first
	 * character is uppercase but all other characters are lowercase
	 */
	wildcap = (ELVBOOL)elvupper(word[0]);
	for (len = 1; wildcap && word[len]; len++)
		if (elvupper(word[len]))
			wildcap = ElvFalse;

	/* copy the word into a private buffer, converting to lowercase */
	for (len = 0; word[len]; len++)
		saveword[len] = elvtolower(word[len]);
	saveword[len] = '\0';
	wildbuf = result;
	wildlen = resultlen;
	wildused = 0;

	/* if doing natural-language search, then look for an exact (except
	 * for possible case differences) match one more time.
	 */
	if (!tagonly)
	{
		found = spellfindword(spellwords, saveword, 0);
		if (SPELL_IS_GOOD(found))
		{
			*result = '\0';
			return;
		}
	}

	/* if the "spellautoload" option is set, and the "spelldict" hasn't
	 * been loaded since the last time it was changed, then load it now.
	 */
	if (o_spellautoload
	 && o_spelldict
	 && (!loadeddict || CHARcmp(o_spelldict, loadeddict)))
	{

		/* Save a copy of the word.  This is important because the
		 * spellload() and spellshow() functions use the same buffer,
		 * so after loading a file, the "word" variable always ends up
		 * containing the last word from the file -- not the one we
		 * want to look up now.
		 */
		wordcopy = CHARdup(word);

		/* load the file  */
		spellload(tochar8(o_spelldict), ElvFalse);

		/* remember that we tried this dictionary */
		if (loadeddict)
			safefree(loadeddict);
		loadeddict = CHARkdup(o_spelldict);

		/* Recursively look up the word.  We do it recursively so we
		 * can come back and free the local copy of the word.
		 */
		spellfix(wordcopy, result, resultlen, tagonly);
		safefree(wordcopy);
		return;
	}

	/* check both dictionaries -- natural language words first */
	for (dict = (spellwords && !tagonly) ? spellwords : spelltags;
	     dict;
	     dict = (dict == spelltags) ? NULL : spelltags)
	{
		/* never force capitalization for words from "tags" */
		if (dict == spelltags)
			wildcap = ElvFalse;

		/* check for case differences */
		wildspell(dict, saveword, saveline);

		/* check derivations with letters transposed */
		for (i = 0; saveword[i + 1]; i++)
		{
			tmp = saveword[i];
			saveword[i] = saveword[i + 1];
			saveword[i + 1] = tmp;

			wildspell(dict, saveword, saveline);

			saveword[i + 1] = saveword[i];
			saveword[i] = tmp;
		}
		if (wildlen < wildused + 2 * len)
			break;

		/* check for derivations with letters deleted */
		tmp = '\0';
		for (i = len - 1, tmp2 = '\0'; i >= 0; i--)
		{
			tmp = saveword[i];
			saveword[i] = tmp2;

			wildspell(dict, saveword, saveline);

			tmp2 = tmp;
		}
		for (i = 0; i < len; i++)
		{
			tmp2 = saveword[i];
			saveword[i] = tmp;
			tmp = tmp2;
		}
		if (wildlen < wildused + 2 * len)
			break;

		/* check for derivations with one letter inserted */
		saveword[len + 1] = '\0';
		for (i = len, tmp2 = '\0'; i >= 0; i--)
		{
			saveword[i + 1] = saveword[i];
			saveword[i] = ' ';

			wildspell(dict, saveword, saveline);
		}
		for (i = 0; i <= len; i++)
			saveword[i] = saveword[i + 1];
		if (wildlen < wildused + 2 * len)
			break;

		/* check for derivations with one letter changed */
		for (i = len - 1; i >= 0; i--)
		{
			tmp = saveword[i];
			saveword[i] = ' ';

			wildspell(dict, saveword, saveline);

			saveword[i] = tmp;
		}
		if (wildlen < wildused + 2 * len)
			break;

		/* check for two words that have been run together */
		for (i = 1;
		     i <= len - 1 && (found = spellfindword(dict, saveword, i));
		     i++)
		{
			if (!SPELL_IS_GOOD(found))
				continue;
			CHARncpy(saveline, saveword, i);
			saveline[i] = '/';
			wildspell(dict, saveword + i, saveline + i + 1);
		}
		if (wildlen < wildused + 2 * len)
			break;
	}

	/* We never want to return "" for misspelled words.  If nothing else
	 * was found, then return a "?" instead.
	 */
	if (wildused == 0)
		result[wildused++] = '?';
	result[wildused] = '\0';
}


/* This returns the value shown by the "show=spell" option.  The return value
 * is a static buffer; you don't need to free it.
 */
CHAR *spellshow(cursor, font)
	MARK	cursor;	/* the window's cursor */
	_char_	font;	/* the font at the cursor (may be temp or selected) */
{
	MARK	left;
	CHAR	*word;
	CHAR	n, *s;

	/* Strip off "selected" bit.  If no font given, then assume "normal" */
	font &= 0x7f;
	if (!font)
		font = 1;

	/* If this font isn't supposed to be checked, then return "" */
	*showsuggest = '\0';
	if (spelllimit[font] == SPELL_CHECK_NONE)
		return showsuggest;

	/* Get the word at the cursor.  If none, then return "" */
	showend = *cursor;
	left = wordatcursor(&showend, ElvTrue);
	if (!left)
		return showsuggest;
	showbegin = *left;
	word = bufmemory(&showbegin, &showend);

	/* Generate the list of suggestions */
	spellfix(word, showsuggest, QTY(showsuggest),
		(ELVBOOL)(spelllimit[font] == SPELL_CHECK_TAGONLY));

	/* Insert numbers before the first 9 words, if there's room */
	if (*showsuggest != '?' && CHARlen(showsuggest) + 19 < QTY(showsuggest))
	{
		for (n = '1', s = showsuggest; n <= '9' && *s; n++)
		{
			/* insert a number here */
			memmove(s + 2, s, QTY(showsuggest) - (s - showsuggest));
			*s++ = n;
			*s++ = '=';

			/* move to start of next word, if any */
			while (*s && !elvspace(*s))
				s++;
			while (elvspace(*s))
				s++;
		}
	}

	/* Remember the buffer version, so we can detect obsolete info */
	showchange = markbuffer(cursor)->changes;

	/* Clean up & return */
	safefree(word);
	return showsuggest;
}


/* replace a misspelled word with a suggested alternative */
ELVBOOL spellcount(cursor, count)
	MARK	cursor;	/* which word the user wants to change */
	long	count;	/* which alternative to use */
{
	CHAR	*word;
	long	len;

	/* If alternatives are obsolete, or cursor has moved to a different
	 * word, or there are no alternatives, then fail
	 */
	if (markbuffer(cursor)->changes != showchange
	 || markbuffer(cursor) != showbegin.buffer
	 || markoffset(cursor) < showbegin.offset
	 || markoffset(cursor) >= showend.offset
	 || !*showsuggest
	 || *showsuggest == '?')
		return ElvFalse;

	/* Find the start of the requested alternative.  If there aren't
	 * that many alternatives, then fail.
	 */
	for (word = showsuggest; *word && count > 1; word++)
	{
		if (*word == ' ')
			count--;
	}
	if (!*word)
		return ElvFalse;

	/* Skip past the number, if there is one */
	while (elvdigit(*word))
		word++;
	if (*word == '=')
		word++;

	/* Find the length of this alternative.  Also, convert an '/' characters
	 * into spaces.
	 */
	for (len = 1; word[len] && word[len] != ' '; len++)
	{
		if (word[len] == '/')
			word[len] = ' ';
	}

	/* Replace the original word with the alternative */
	bufreplace(&showbegin, &showend, word, len);
	return ElvTrue;
}

/* adjust the spell-checking for a given font */
void spellcheckfont(fontname, check, bang)
	CHAR	*fontname;	/* name of font to adjust */
	spellcheck_t check;	/* new setting */
	ELVBOOL	bang;		/* hidden? */
{
	int	font;
	CHAR	*flag = toCHAR("*+-");
	int	col, len;
	spellcheck_t level;

	/* if given a fontname, then adjust it */
	if (fontname)
	{
		/* find the font */
		font = colorfind(fontname);
		if (!font)
			return;

		/* store the setting */
		spelllimit[font] = check;
		spellset[font] = (ELVBOOL)!bang;

		return;
	}

	/* else just list the settings */

	for (level=SPELL_CHECK_ALL; level <= SPELL_CHECK_NONE; level++, flag++)
	{
		if (level < check)
			continue;

		for (font = 1, col = -1; font < colornpermanent; font++)
		{
			/* skip if not one that we care about */
			if (spelllimit[font] != level
			 || (!spellset[font] && !bang))
				continue;

			/* if won't fit on this line, then start a new line */
			len = CHARlen(colorinfo[font].name);
			if (col < 0 || col + 2 + len > o_columns(windefault))
			{
				if (col > 0)
					drawextext(windefault, toCHAR("\n"), 1);
				drawextext(windefault, flag, 1);
				col = 1;
			}
			else /* space between names */
			{
				drawextext(windefault, blanks, 1);
				col++;
			}
				
			/* write the name of this font */
			drawextext(windefault, colorinfo[font].name, len);
			col += len;
		}

		/* if last line didn't end, then end it now */
		if (col > 0)
			drawextext(windefault, toCHAR("\n"), 1);
	}
}

/* This function is called by colortmp() to alert the spell-checker about any
 * new, temporary fonts.
 */
void spelltmp(oldfont, newfont, combofont)
	int	oldfont,newfont;/* two fonts that were combined */
	int	combofont;	/* the resulting temporary font */
{
	/* make the combo font use the more restrictive checking style from
	 * either of its parent fonts.
	 */
	if (spelllimit[oldfont] > spelllimit[newfont])
		spelllimit[combofont] = spelllimit[oldfont];
	else
		spelllimit[combofont] = spelllimit[newfont];
}

/* Scan a file for words.  Add each word to the spellwords dictionary, marked
 * as "good" words.  Optionally mark them as being "personal" words too.
 */
void spellload(filename, personal)
	char	*filename;	/* name of file to scan */
	ELVBOOL	personal;	/* mark new words as "personal" ? */
{
	FILE	*fp;
	int	i, c;
	spell_t	*node;

	/* Try to open the file.  If failed, return silently */
	fp = fopen(filename, "r");
	if (!fp)
		return;

	/* find each word in the file */
	for (i = 0; (c = getc(fp)) != EOF; )
	{
		/* apostrophes sometimes need special treatment */
		if (c == '\'' && (i == 0 || !elvalpha(saveword[i - 1])))
		{
			c = getc(fp);
			ungetc(c, fp);
			c = elvalpha(c) ? '\'' : ' ';
		}

		if (i == 0 && elvalnum(c))
		{
			/* start of a new word */
			saveword[i++] = elvtolower(c);
		}
		else if (i > 0 && (elvalnum(c) || c == '_' || c == '\'') && i < MAXWLEN - 1)
		{
			/* continuation of a word */
			saveword[i++] = elvtolower(c);
		}
		else if (i > 0 && i < MAXWLEN)
		{
			/* end of a word */
			saveword[i] = '\0';

			/* add the word */
			if (elvalnum(saveword[0]))
			{
				if (personal)
				{
					node = spellfindword(spellwords, saveword, i);
					if (node && !SPELL_IS_GOOD(node))
						node->flags = SPELL_FLAG_COMPLETE | SPELL_FLAG_PERSONAL;
					else
						spellwords = spelladdword(spellwords, saveword, SPELL_FLAG_PERSONAL);
				}
				else
				{
					spellwords = spelladdword(spellwords, saveword, 0);
				}
			}

			/* prepare for next word */
			i = 0;
		}
	}
	fclose(fp);
}


static MARK	cursor;
static long	badword;
static ELVBOOL	inword;

/* This function is used as the "draw" function by a display mode's image()
 * function.  The spellnext() needs this to spell-check words that aren't
 * necessarily on the screen, so it can find the next bad one.
 */
static void spelldraw(p, qty, font, offset)
	CHAR	*p;	/* first letter of text to draw */
	long	qty;	/* quantity to draw (negative to repeat *p) */
	_char_	font;	/* font code of the text */
	long	offset;	/* buffer offset of *p */
{
	MARKBUF	mark;	/* a temporary mark */

	/* if we've already found a bad word, then skip this. */
	if (badword >= 0)
		return;

	/* repeated characters, or graphic chars can't contain words */
	if (qty < 1L
	 || colorinfo[(int)font].da.bits & COLOR_GRAPHIC)
	{
		inword = ElvFalse;
		return;
	}

	/* check for words in the characters */
	for (; --qty >= 0; p++, offset++)
	{
		/* if non-letter, then we aren't in a word anymore */
		if (!inword && !elvalnum(*p) && *p != '_')
			continue;
		if (!elvalnum(*p) && *p != '_' && !(*p == '\'' && elvalpha(p[1])))
		{
			inword = ElvFalse;
			continue;
		}

		/* If we're in a word that we already checked, then ignore it */
		if (inword)
			continue;

		/* Else we're at the start of a word */
		inword = ElvTrue;

		/* Ignore words before the cursor, or in an unchecked font,
		 * or which start with a digit.
		 */
		if (offset <= markoffset(cursor)
		 || spelllimit[font] == SPELL_CHECK_NONE
		 || elvdigit(*p))
			continue;

		/* If this word is bad, then set badword to its offset */
		if (spellcheck(marktmp(mark, markbuffer(cursor), offset),
			    (ELVBOOL)(spelllimit[font] == SPELL_CHECK_TAGONLY),
			    markoffset(cursor)) != SPELL_GOOD)
		{
			badword = offset;
			break;
		}
	}
}

/* Locate the next misspelled word.  This uses the window's current display
 * mode, regardless of the buffer's o_bufdisplay(buf) setting.  If there is
 * no misspelled word between the cursor and the end of the file, then it
 * returns NULL; else it returns the start of the word
 */
MARK spellnext(win, curs)
	WINDOW	win;	/* the window whose display mode is to be used */
	MARK	curs;	/* find the first word after this */
{
	static MARKBUF	mark;	/* the return mark (also internal temp mark) */
	MARK		line;	/* used during scanning/drawing text */
	long		bufchars;

	/* Always fail if no window, or if in "hex" mode */
	if (!win || win->md == &dmhex)
		return NULL;

	/* Note: Since spell checking is font-sensitive, we must format the
	 * text as though we were updating the display... but instead of
	 * drawing the formatted text, we look for words in it.
	 */
	cursor = curs;
	inword = ElvFalse;
	badword = -1;
	bufchars = o_bufchars(markbuffer(cursor));
	line = (*win->md->setup)(win, cursor, markoffset(cursor),
		marktmp(mark, markbuffer(cursor), bufchars), win->mi);
	while (line && markoffset(line) < bufchars && badword < 0L)
		line = (*win->md->image)(win, line, win->mi, spelldraw);

	/* if we found a bad word, then return its MARK; else return NULL */
	if (badword >= 0)
		return marktmp(mark, markbuffer(curs), badword);
	return NULL;
}
#endif /* FEATURE_SPELL */
