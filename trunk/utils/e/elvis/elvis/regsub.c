/* regsub.c */

/* This file contains the regsub() function, which performs substitutions
 * after a regexp match has been found.
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_regsub[] = "$Id: regsub.c 167211 2008-04-24 17:26:56Z sboucher $";
#endif

/* Allocate a new copy of the replacement string, with all ~'s replaced by
 * the previous replacement string.
 *
 * NOTE: The value returned by this function should never be freed from outside
 * this function, because this function maintains an internal pointer to the
 * same memory so it knows what value to substitute for ~ on next invocation.
 */
CHAR *regtilde(newp)
	CHAR	*newp;	/* new text as supplied by user */
{
	static CHAR *prev;	/* previous replacement text */
	static CHAR *willfree;	/* previous replacement text if nosaveregex */
	CHAR	*ret;		/* returned string */
	CHAR	*scan;		/* used for stepping through chars of "prev" */

	/* If "willfree" isn't NULL, then free it now.  This is used when
	 * the saveregexp option is turned off, so we can leave prev unchanged.
	 */
	if (willfree)
	{
		safefree(willfree);
		willfree = NULL;
	}

	/* copy new into ret, replacing the ~s by the previous text */
	for (ret = NULL; *newp; )
	{
		if (o_magic && *newp == '~')
		{
			if (!prev) goto Fail;
			for (scan = prev; *scan; scan++)
				buildCHAR(&ret, *scan);
			newp++;
		}
		else if (!o_magic && *newp == '\\' && *(newp + 1) == '~')
		{
			if (!prev) goto Fail;
			for (scan = prev; *scan; scan++)
				buildCHAR(&ret, *scan);
			newp += 2;
		}
		else
		{
			if (*newp == '\\' && *(newp + 1))
			{
				buildCHAR(&ret, *newp++);
			}
			buildCHAR(&ret, *newp++);
		}
	}

	/* if empty string, then allocate a single '\0' character */
	if (!ret)
		ret = (CHAR *)safealloc(1, sizeof(CHAR));

	/* remember this as the "previous" for next time */
	if (o_saveregexp)
	{
		if (prev)
			safefree(prev);
		prev = ret;
	}
	else
	{
		/* leave "prev" unchanged, but remember it somewhere else so
		 * we can free the text when regtilde() is called next time.
		 */
		willfree = ret;
	}

	return ret;

Fail:
	msg(MSG_ERROR, "no previous text to substitute for ~");
	if (ret)
		safefree(ret);
	return NULL;
}

/* Perform substitutions after a regexp match.  "re" is the compiled regular
 * expression which has been matched to a text string.  "new" is a pointer to
 * the replacement text string.  Return the actual replacement text (after all
 * metacharacters have been processed) if successful, or NULL if error.  The
 * calling function is responsible for calling safefree() on the returned
 * string.
 */
CHAR *regsub(re, newp, doit)
	regexp		*re;	/* a regular expression that has been matched */
	REG CHAR	*newp;	/* the replacement text */
	ELVBOOL		doit;	/* perform the substitution? (else just return string) */
{
	MARKBUF		cpy;	/* start of text to copy */
	long		end;	/* length of text to copy */
	REG CHAR	c;	/* a character from "new" text */
	long		cval;	/* numeric value of 'c', if 'c' is digit */
	CHAR		*inst;	/* the new next, after processing escapes */
	int		mod = 0;/* used to track \U, \L, \u, \l, and \E */
	int		len;	/* used to calculate length of subst string */
	MARKBUF		tmp;	/* end of replacement region */
	CHAR		*scan;	/* used for scanning a segment of orig text */
	char		lnum[12];/* line number */

	/* initialize "cval" just to silence a compiler warning */
	cval = 0;

	/* for each character of the new text... */
	for (inst = NULL, len = 0; (c = *newp++) != '\0'; )
	{
		/* recognize any meta characters */
		if (c == '&' && o_magic)
		{
			(void)marktmp(cpy, re->buffer, re->startp[0]);
			end = re->endp[0] - re->startp[0];
		}
		else if (c == '\\')
		{
			c = *newp++;
			switch (c)
			{
			  case '0':
				/* Traditionally \0 has been a synonym for &,
				 * but we need a way to insert NUL so...
				 */
				len = buildCHAR(&inst, '\0');
				continue;

			  case '1':
			  case '2':
			  case '3':
			  case '4':
			  case '5':
			  case '6':
			  case '7':
			  case '8':
			  case '9':
				/* \0 thru \9 mean "copy subexpression" */
				cval = c - '0';
				(void)marktmp(cpy, re->buffer, re->startp[cval]);
				end = re->endp[cval] - re->startp[cval];
				break;

			  case 'U':
			  case 'u':
			  case 'L':
			  case 'l':
				/* \U and \L mean "convert to upper/lowercase" */
				mod = c;
				continue;

			  case 'E':
			  case 'e':
				/* \E ends the \U or \L */
				mod = 0;
				continue;

			  case '&':
				/* "\&" means "original text" */
				if (o_magic)
				{
					len = buildCHAR(&inst, c);
					continue;
				}
				(void)marktmp(cpy, re->buffer, re->startp[0]);
				end = re->endp[0] - re->startp[0];
				break;

			  case '#':	/* "\#" means "line number" */
				sprintf(lnum, "%ld", markline(marktmp(tmp, re->buffer, re->startp[0])));
				len = buildstr(&inst, lnum);
				continue;

			  case 'a':	/* \a => ^G, <BEL>	*/
				len = buildCHAR(&inst, '\007');
				continue;

			  case 'b':	/* \b => ^H, <BS>	*/
				len = buildCHAR(&inst, '\b');
				continue;

 			  case 'f':	/* \f => ^L, <FF>	*/
				len = buildCHAR(&inst, '\f');
				continue;

 			  case 'n':	/* \n => ^J, <NL>	*/
				len = buildCHAR(&inst, '\n');
				continue;

 			  case 'r':	/* \r => ^M, <CR>	*/
				len = buildCHAR(&inst, '\r');
				continue;

			  case 't':	/* \t => ^I, <TAB>	*/
				len = buildCHAR(&inst, '\t');
				continue;

#if 0
				/* \e is already taken for ending of \U and \L.
				 * Though it's a shame both \E and \e are well
				 * documented to ex/vi here we' like \E to
				 * suffice, so we would use \e for <Esc>.
				 */
			  case 'e':	/* \e => ^[, <ESC>	*/
				len = buildCHAR(&inst, '\033');
				continue;
#endif

			  default:
				/* ordinary char preceded by backslash */
				len = buildCHAR(&inst, c);
				continue;
			}
		}
# if OSK
		else if (c == '\l')
# else
		else if (c == '\r')
# endif
		{
			/* transliterate ^M into newline */
			len = buildCHAR(&inst, '\n');
			continue;
		}
		else
		{
			/* ordinary character, so just copy it */
			if (!mod)
				len = buildCHAR(&inst, c);
			else if (elvtolower(mod) == 'l')
				len = buildCHAR(&inst, elvtolower(c));
			else
				len = buildCHAR(&inst, elvtoupper(c));
			if (elvlower(mod))
				mod = 0;
			continue;
		}

		/* Note: to reach this point in the code, we have evaded
		 * all "continue" statements.  To do that, we must have hit
		 * a metacharacter that involves copying.
		 */

		/* if there is nothing to copy, loop */
		if (markoffset(&cpy) < 0)
		{
			msg(MSG_ERROR, "[d]too few \\\\\\(\\\\\\)s to use \\\\$1", cval);
			if (inst)
				safefree(inst);
			return NULL;
		}

		/* copy over a portion of the original */
		for (scanalloc(&scan, &cpy);
		     scan && end > 0;
		     scannext(&scan), end--)
		{
			switch (mod)
			{
			  case 'U':
			  case 'u':
				/* convert to uppercase */
				len = buildCHAR(&inst, (_CHAR_)elvtoupper(*scan));
				break;

			  case 'L':
			  case 'l':
				/* convert to lowercase */
				len = buildCHAR(&inst, (_CHAR_)elvtolower(*scan));
				break;

			  default:
				/* copy without any conversion */
				len = buildCHAR(&inst, *scan);
			}

			/* \u and \l end automatically after the first char */
			if (mod == 'u' || mod == 'l')
			{
				mod = 0;
			}
		}
		scanfree(&scan);
	}

	/* if we're supposed to perform the substitution, then do it */
	if (doit)
	{
		/* replace the old text with the new text in the buffer */
		bufreplace(marktmp(cpy, re->buffer, re->startp[0]),
			marktmp(tmp, re->buffer, re->endp[0]), inst, len);
	
		/* Adjust the offset of the end of the whole expression
		 * to compensate for the change in the length of text.
		 * Also, if this regexp could conceivably match a
		 * zero-length string, then require at least 1 unmatched
		 * character between matches.
		 */
		re->endp[0] = re->startp[0] + len;
		if (re->minlen == 0
			&& re->endp[0] < o_bufchars(re->buffer)
			&& scanchar(marktmp(tmp, re->buffer, re->endp[0])) != '\n')
		{
			re->endp[0]++;
		}
	}

	/* At this point, we know we were successful but the "inst" pointer
	 * will be NULL if the replacement text is 0 characters long.  We don't
	 * want to return NULL for a successful substitution, so allocate
	 * a string which contains only a '\0' character and return that.
	 */
	if (!inst)
	{
		assert(len == 0);
		buildCHAR(&inst, (_CHAR_)'\0');
		assert(inst != NULL);
	}

	return inst;
}
