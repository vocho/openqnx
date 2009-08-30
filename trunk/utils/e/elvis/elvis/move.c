/* move.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_move[] = "$Id: move.c,v 2.66 2003/10/17 17:41:23 steve Exp $";
#endif

#ifdef FEATURE_G
static void gdraw P_((CHAR *p, long qty, _char_ font, long offset));
static long gstartrow P_((WINDOW win, long col));
#endif

/* This function implements the "h" command */
RESULT m_left(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	tmp;
	DEFAULT(1);

	/* find the start of this line */
	tmp = dispmove(win, 0L, 0L);

	/* if already at the start of the line, then fail */
	if (markoffset(tmp) == markoffset(win->state->cursor))
	{
		return RESULT_ERROR;
	}

	/* move either the requested number of characters left, or to the
	 * start of the line, whichever is closer
	 */
	if (markoffset(win->state->cursor) - vinf->count > markoffset(tmp))
	{
		markaddoffset(win->state->cursor, -vinf->count);
	}
	else
	{
		marksetoffset(win->state->cursor, markoffset(tmp));
	}

	return RESULT_COMPLETE;
}


/* This function implements the "l" command */
RESULT m_right(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	tmp;
	DEFAULT(1);

	/* find the end of this line.  This is complicated by the fact that
	 * when used as the target of an operator, the l command can move
	 * past the end of the line.
	 */
	if (vinf->oper && !win->state->acton)
		tmp = (*win->md->move)(win, win->cursor, 0L, INFINITY, ElvFalse);
	else
		tmp = dispmove(win, 0L, INFINITY);

	/* if already at the end of the line, then fail */
	if (markoffset(tmp) == markoffset(win->state->cursor))
	{
		return RESULT_ERROR;
	}

	/* move either the requested number of characters right, or to the
	 * end of the line, whichever is closer
	 */
	if (markoffset(win->state->cursor) + vinf->count < markoffset(tmp))
	{
		markaddoffset(win->state->cursor, vinf->count);
	}
	else
	{
		marksetoffset(win->state->cursor, markoffset(tmp));
	}

	return RESULT_COMPLETE;
}


/* This function implements the "j" and "k" functions */
RESULT m_updown(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	tmp = NULL;
	DEFAULT(1);

	/* do the move */
	switch (vinf->command)
	{
	  case '_':
		/* decremement count & then treat like <Enter>... */
		vinf->count--;
		/* fall through... */

	  case ELVCTRL('J'):
	  case ELVCTRL('M'):
	  case ELVCTRL('N'):
	  case '+':
	  case 'j':
		tmp = dispmove(win, vinf->count, win->wantcol);
		break;

	  case ELVCTRL('P'):
	  case '-':
	  case 'k':
		tmp = dispmove(win, -vinf->count, win->wantcol);
		break;

#ifndef NDEBUG
	  default:
		abort();
#endif
	}

	/* check for goofy return values */
	if (markoffset(tmp) < 0
	 || markoffset(tmp) >= o_bufchars(markbuffer(win->state->cursor))
	 || (markoffset(tmp) == markoffset(win->state->cursor) && vinf->count != 0))
	{
		return RESULT_ERROR;
	}

	/* It's good! */
	marksetoffset(win->state->cursor, markoffset(tmp));
	return RESULT_COMPLETE;
}


/* This function implements the "^" function */
RESULT m_front(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	CHAR	*cp;

	/* scan from the start of the line, to the first non-space */
	scanalloc(&cp, dispmove(win, 0, 0));
	if (cp && (*cp == '\t' || *cp == ' '))
	{
		do
		{
			scannext(&cp);
		} while (cp && (*cp == ' ' || *cp == '\t'));
		if (*cp == '\n')
		{
			scanprev(&cp);
		}
	}
	if (!cp)
	{
		return RESULT_ERROR;
	}
	marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
	scanfree(&cp);
	return RESULT_COMPLETE;
}

/* This function implements the <Shift-G>, <Control-G>, and <%> commands, which
 * move the cursor to a specific line, character, or percentage of the buffer.
 * The number is given in the "count" field; if no number is given, then either
 * move to the last line, show buffer statistics, or move to matching bracket,
 * respectively.
 * 
 * Support for #if/#else/#endif derived from Antony Bowesman's contributed code.
 *  When a # is found the next two characters are matched.  If an 'if' is 
 *  found then the search is set forwards.  If an 'en' is found the search
 *  goes backwards.  if an 'el' (For else or elif) the search direction
 *  proceeds in the opposite direction of the last direction used in else
 *  match.
 */
RESULT m_absolute(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	BUFFER	buf = markbuffer(win->state->cursor);
	CHAR	match;		/* the parenthesis we want (for <%> command) */
	CHAR	nest = 0;	/* the starting parenthesis */
	CHAR	*cp, *cp2;	/* used for scanning through text */
	long	count;		/* nesting depth */
	EXINFO	xinfb;		/* an ex command */
	ELVBOOL	fwd;		/* forward search? (else backward) */
#ifdef DISPLAY_SYNTAX
 static	ELVBOOL	fwdelse;	/* last "else" move direction */
	CHAR	prepchar;	/* preprocessor character, usually '#' */
	char	prepname[3];	/* first two chars of preprocessor directive */
#endif
#ifdef FEATURE_TEXTOBJ
	VIINFO	vicmd;		/* describes a text object */
	MARKBUF	start, end;	/* endpoints of an object */
	MARKBUF	beststart,bestend;/* endpoints of best object */
	CHAR	*mustfree;	/* temporarily holds non-object pairs */

	mustfree = NULL;
	end.buffer = NULL;
#endif

	assert(vinf->command == 'G' || vinf->command == ELVCTRL('G') || vinf->command == '%');

	switch (vinf->command)
	{
	  case 'G':
		DEFAULT(o_buflines(buf));

		/* Try to go to the requestedc line.  Catch errors, including a
		 * numberless <G> in an empty buffer.
		 */
		if (!marksetline(win->state->cursor, vinf->count))
		{
			msg(MSG_ERROR, "[d]only $1 lines", o_buflines(buf));
			return RESULT_ERROR;
		}
		break;

	  case ELVCTRL('G'):
		if (!vinf->count)
		{
			/* no count, just show buffer status */
			memset((char *)&xinfb, 0, sizeof xinfb);
			xinfb.window = win;
			xinfb.defaddr = *win->state->cursor;
			xinfb.command = EX_FILE;
			ex_file(&xinfb);
		}
		else if (vinf->count < 0 || vinf->count > o_bufchars(buf))
		{
			/* request offset is out of range */
			return RESULT_ERROR;
		}
		else
		{
			/* set the cursor to the requested offset */
			marksetoffset(win->state->cursor, vinf->count - 1);
		}
		break;

	  case '%':
		if (!vinf->count)
		{
#ifdef DISPLAY_SYNTAX
			/* find the preprocessor character, if any */
			prepchar = dmspreprocessor(win);
#endif

#ifdef FEATURE_TEXTOBJ
			/* scan for text objects in matchchar */
			bestend.buffer = NULL;
			for (cp2 = o_matchchar; cp2[0] && cp2[1]; cp2 += 2)
			{
				if (*cp2 == 'a' || *cp2 == 'i')
				{
					memset(&vicmd, 0, sizeof vicmd);
					vicmd.command = *cp2;
					vicmd.key2 = cp2[1];
					if (vitextobj(win, &vicmd, &start, &end) == RESULT_COMPLETE
					 && (bestend.buffer == NULL
					  || end.offset < bestend.offset))
					{
						beststart = start;
						bestend = end;
					}
				}
				else
				{
					buildCHAR(&mustfree, cp2[0]);
					buildCHAR(&mustfree, cp2[1]);
				}
			}
#endif /* FEATURE_TEXTOBJ */

			/* search forward within line for one of "[](){}" */
			for (match = '\0', scanalloc(&cp, win->state->cursor); !match;)
			{
#ifdef FEATURE_TEXTOBJ
				/* if hit end of text object, then we'll be
				 * doing the textobject bouncy thing.
				 */
				if (bestend.buffer
				 && (!cp || markoffset(scanmark(&cp)) >= bestend.offset))
				{
					/* is the cursor already at the front?*/
					if (markoffset(win->state->cursor) <= beststart.offset)
						marksetoffset(win->state->cursor, bestend.offset - 1L);
					else
						marksetoffset(win->state->cursor, beststart.offset);
					scanfree(&cp);
					if (mustfree)
						safefree(mustfree);
					return RESULT_COMPLETE;

				}
#endif /* FEATURE_TEXTOBJ */

				/* if hit end-of-line or end-of-buffer without
				 * finding a matchable character, then fail.
				 */
				if (!cp || *cp == '\n'
#ifdef FEATURE_TEXTOBJ
				 ||  (bestend.buffer
				   && markoffset(scanmark(&cp)) >= bestend.offset)
#endif
				   )
				{
					scanfree(&cp);
#ifdef FEATURE_TEXTOBJ
					if (mustfree)
						safefree(mustfree);
					if (bestend.buffer)
					{
						/* is the cursor already at the front?*/
						if (markoffset(win->state->cursor) <= beststart.offset)
							marksetoffset(win->state->cursor, bestend.offset - 1L);
						else
							marksetoffset(win->state->cursor, beststart.offset);
						return RESULT_COMPLETE;
					}
#endif
					return RESULT_ERROR;
				}

				/* is this a matchable char? */
				nest = *cp;
#ifdef DISPLAY_SYNTAX
				if (prepchar && nest == prepchar)
				{
					/* PREPROCESSOR DIRECTIVE */

					/* skip whitespace, get directive name*/
					scandup(&cp2, &cp);
					do
					{
						scannext(&cp2);
					} while (cp2 && (*cp2 == ' ' || *cp2 == '\t'));
					memset(prepname, 0, sizeof prepname);
					if (cp2)
					{
						prepname[0] = elvtolower(*cp2);
						scannext(&cp2);
						if (cp2)
							prepname[1] = elvtolower(*cp2);
					}
					scanfree(&cp2);

					/* choose direction from name */
					if (!strcmp(prepname, "if"))
					{
						fwd = fwdelse = ElvTrue;
						match = prepchar;
					}
					else if (!strcmp(prepname, "en"))
					{
						fwd = fwdelse = ElvFalse;
						match = prepchar;
					}
					else if (!strcmp(prepname, "el"))
					{
						fwd = fwdelse;
						match = prepchar;
					}
					else
					{
						/* UNRECOGNIZED DIRECTIVE */
						scanfree(&cp);
#ifdef FEATURE_TEXTOBJ
						if (mustfree)
							safefree(mustfree);
#endif
						return RESULT_ERROR;
					}
				}
				else
#endif /* defined(DISPLAY_SYNTAX) */
				{
					/* look in matchchar option... */
#ifdef FEATURE_TEXTOBJ
					for (cp2 = mustfree, fwd = ElvTrue;
#else
					for (cp2 = o_matchchar, fwd = ElvTrue;
#endif
					     cp2 && *cp2 && *cp2 != nest;
					     cp2++, fwd = (ELVBOOL)!fwd)
					{
					}
					if (!cp2 || !*cp2)
					{
						/* NOT MATCHABLE */
						scannext(&cp);
						continue;
					}

					/* figure out what matching char is */
					if (!fwd)
						/* MATCH PREVIOUS IN LIST */
						match = cp2[-1];
					else if (cp2[1])
						/* MATCH FOLLOWING IN LIST */
						match = cp2[1];
					else
						/* MATCH SELF */
						match = nest;

					/* if nest=match, choose a direction */
					if (nest == match)
					{
						scandup(&cp2, &cp);
						for (fwd = ElvTrue;
						     cp2 && *cp2 != '\n';
						     scannext(&cp2))
						{
							if (*cp2 == nest)
								fwd = (ELVBOOL)!fwd;
						}
						scanfree(&cp2);
					}
				}
			}
			assert(cp != NULL);
#ifdef FEATURE_SYNTAX
			if (prepchar != match)
				prepchar = '\0';
#endif

			/* search forward or backward for match */
			if (!fwd)
			{
				/* search backward */
				for (count = 1; count > 0; )
				{
					/* back up 1 char; give up at top of buffer */
					if (!scanprev(&cp))
					{
						break;
					}

					/* check the char */
#ifdef DISPLAY_SYNTAX
					if (prepchar && *cp == prepchar)
					{
						if ((prepname[0] == 'i' && prepname[1] == 'f')
						  || (count == 1 && prepname[0] == 'e' && prepname[1] == 'l'))
							count--;
						else if (prepname[0] == 'e' && prepname[1] == 'n')
							count++;
					}
					else
#endif
					if (*cp == match)
						count--;
					else if (*cp == nest)
						count++;

#ifdef DISPLAY_SYNTAX
					/* for benefit of preprocessor, shift
					 * non-whitespace chars into prepname[]
					 */
					if (!elvspace(*cp))
					{
						prepname[1] = prepname[0];
						prepname[0] = elvtolower(*cp);
					}
#endif
				}
			}
			else
			{
				/* search forward */
				for (count = 1; count > 0; )
				{
					/* advance 1 char; give up at end of buffer */
					if (!scannext(&cp))
					{
						break;
					}

					/* check the char */
#ifdef DISPLAY_SYNTAX
					if (prepchar && *cp == prepchar)
					{
						/* peek ahead for prepname */
						scandup(&cp2, &cp);
						do
						{
							scannext(&cp2);
						} while (cp2 && (*cp2 == ' ' || *cp2 == '\t'));
						memset(prepname, 0, sizeof prepname);
						if (cp2)
						{
							prepname[0] = elvtolower(*cp2);
							scannext(&cp2);
							if (cp2)
								prepname[1] = elvtolower(*cp2);
						}
						scanfree(&cp2);

						/* check for nesting, etc */
						if (prepname[0] == 'e'
						 && (prepname[1] == 'n'
						    || (count == 1 && prepname[1] == 'l')))
							count--;
						else if (prepname[0] == 'i' && prepname[1] == 'f')
							count++;
					}
#endif /* defined(DISPLAY_SYNTAX) */
					else if (*cp == match)
						count--;
					else if (*cp == nest)
						count++;
				}
			}

			/* if we hit the end of the buffer, then fail */
			if (!cp)
			{
				scanfree(&cp);
#ifdef FEATURE_TEXTOBJ
				if (mustfree)
					safefree(mustfree);
#endif
				return RESULT_ERROR;
			}

			/* move the cursor to the match */
			marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
			scanfree(&cp);
		}
		else if (vinf->count < 1 || vinf->count > 100)
		{
			msg(MSG_ERROR, "bad percentage");
#ifdef FEATURE_TEXTOBJ
			if (mustfree)
				safefree(mustfree);
#endif
			return RESULT_ERROR;
		}
		else
		{
			/* Compute the character offset, given the percentage.
			 * I'm slightly careful here to avoid overflowing
			 * the long int which stores the offset.
			 */
			if (o_bufchars(buf) > 1000000L)
			{
				marksetoffset(win->state->cursor,
					(o_bufchars(buf) / 100) * vinf->count);
			}
			else
			{
				marksetoffset(win->state->cursor,
					(o_bufchars(buf) * vinf->count) / 100);
			}
			vinf->tweak |= TWEAK_FRONT;
		}
	}

#ifdef FEATURE_TEXTOBJ
	if (mustfree)
		safefree(mustfree);
#endif
	return RESULT_COMPLETE;
}

/* Move to a mark.  This function implements the <'> and <`> commands. */
RESULT m_mark(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	newmark;

	/* check for <'><'> or <`><`> */
	if (vinf->command == vinf->key2)
	{
		if (win->prevcursor
		 && markbuffer(win->state->cursor) == markbuffer(win->prevcursor))
		{
			marksetoffset(win->state->cursor, markoffset(win->prevcursor));
			return RESULT_COMPLETE;
		}
		return RESULT_ERROR;
	}

	/* else key2 had better be a lowercase ASCII letter */
	if (vinf->key2 < 'a' || vinf->key2 > 'z')
	{
		return RESULT_ERROR;
	}

	/* look up the named mark */
	newmark = namedmark[vinf->key2 - 'a'];
	if (!newmark)
	{
		msg(MSG_ERROR, "[C]'$1 unset", vinf->key2);
		return RESULT_ERROR;
	}

	/* if the named mark is in a different buffer, fail. */
	/* (A later version of elvis may be able to do this!) */
	if (markbuffer(newmark) != markbuffer(win->state->cursor))
	{
		msg(MSG_ERROR, "[C]'$1 in other buffer", vinf->key2);
		return RESULT_ERROR;
	}

	/* move to the named mark */
	marksetoffset(win->state->cursor, markoffset(newmark));
	return RESULT_COMPLETE;
}

/* This function implements the whitespace-delimited word movement commands:
 * <Shift-W>, <Shift-B>, and <Shift-E>.
 */
RESULT m_bigword(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	ELVBOOL	whitespace;	/* do we include following whitespace? */
	ELVBOOL backward;	/* are we searching backwards? */
	long	offset;		/* offset of *cp character */
	long	count;		/* number of words to skip */
	long	end;		/* offset of the end of the buffer */
	ELVBOOL	inword;		/* are we currently in a word? */
	CHAR	*cp;		/* used for scanning chars of buffer */

	DEFAULT(1);

	/* start the scan */
	offset = markoffset(win->state->cursor);
	scanalloc(&cp, win->state->cursor);
	assert(cp != NULL);
	count = vinf->count;
	end = o_bufchars(markbuffer(win->state->cursor));

	/* figure out which type of movement we'll be doing */
	switch (vinf->command)
	{
	  case 'B':
		backward = ElvTrue;
		whitespace = ElvFalse;
		inword = ElvFalse;
		break;

	  case 'E':
		backward = ElvFalse;
		whitespace = ElvFalse;
		inword = ElvFalse;
		break;

	  default:
		backward = ElvFalse;
		inword = (ELVBOOL)!elvspace(*cp);
		if (vinf->oper == 'c')
		{
			/* "cW" acts like "cE", pretty much */
			whitespace = ElvFalse;
			vinf->tweak |= TWEAK_INCL;

			/* starting on whitespace? */
			if (!inword)
			{
				/* When "cW" starts on whitespace, it changes
				 * one less word than normal.  If it would
				 * normally change just one word, then it
				 * should change a single whitespace character.
				 */
				vinf->count--;
				if (vinf->count == 0)
				{
					scanfree(&cp);
					return RESULT_COMPLETE;
				}
			}
			else if (markoffset(win->state->cursor) > 0)
			{
				/* When "cW" starts on the last character of a
				 * word, then it should change just that last
				 * character.  By temporarily moving the
				 * cursor back one char, we can achieve this
				 * effect without affecting the results of any
				 * other movement.
				 */
				scanprev(&cp);
				offset--;
			}
		}
		else
		{
			whitespace = ElvTrue;
		}
		break;
	}

	/* continue... */
	if (backward)
	{
		/* move backward until we hit the top of the buffer, or
		 * the start of the desired word.
		 */
		while (count > 0 && offset > 0)
		{
			scanprev(&cp);
			assert(cp != NULL);
			if (elvspace(*cp))
			{
				if (inword)
				{
					count--;
				}
				inword = ElvFalse;
			}
			else
			{
				inword = ElvTrue;
			}
			if (count > 0)
			{
				offset--;
				if (offset == 0 && count == 1)
				{
					count = 0;
				}
			}
		}
	}
	else
	{
		/* move forward until we hit the end of the buffer, or
		 * the start of the desired word.
		 */
		while (count > 0 && offset < end - 1)
		{
			scannext(&cp);
			assert(cp != NULL);
			if (elvspace(*cp))
			{
				if (vinf->oper && *cp == '\n' && count == 1)
				{
					count = 0;
					if (!(vinf->tweak & TWEAK_INCL))
						offset++;
				}
				else if (inword && !whitespace)
				{
					count--;
				}
				inword = ElvFalse;
				if (count > 0)
				{
					offset++;
				}
			}
			else
			{
				if (!inword && whitespace)
				{
					count--;
				}
				inword = ElvTrue;
				offset++;
			}
		}
	}

	/* cleanup */
	scanfree(&cp);

	/* if the count didn't reach 0, we failed */
	if (count > 0)
	{
		return RESULT_ERROR;
	}

	/* else set the cursor's offset */
	assert(offset < end && offset >= 0);
	marksetoffset(win->state->cursor, offset);
	return RESULT_COMPLETE;
}

/* This function implements the <b>, <e>, and <w> word movement commands
 * by calling the mode-dependent wordmove function.
 */
RESULT m_word(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	ELVBOOL	whitespace;	/* include trailing whitespace? */
	ELVBOOL backward;	/* are we moving backward? */
	long	span;		/* offset of starting position */
	long	newline;	/* offset of newline */
	CHAR	*cp;		/* used for scanning backward for newline */
	CHAR	atcursor;	/* character at the cursor position */

	DEFAULT(1);

	/* figure out which type of movement we'll be doing */
	switch (vinf->command)
	{
	  case 'b':
		backward = ElvTrue;
		whitespace = ElvFalse;
		break;

	  case 'e':
		backward = ElvFalse;
		whitespace = ElvFalse;
		break;

	  default:
		backward = ElvFalse;
		if (vinf->oper == 'c')
		{
			/* "cw" acts like "ce", pretty much */
			whitespace = ElvFalse;
			vinf->tweak |= TWEAK_INCL;

			/* starting on whitespace? */
			atcursor = scanchar(win->state->cursor);
			if (atcursor == ' ' || atcursor == '\t')
			{
				/* When "cw" starts on whitespace, it changes
				 * one less word than normal.  If it would
				 * normally change just one word, then it
				 * should change a single whitespace character.
				 */
				vinf->count--;
				if (vinf->count == 0)
				{
					return RESULT_COMPLETE;
				}
			}
			else if (markoffset(win->state->cursor) > 0)
			{
				/* When "cw" starts on the last character of a
				 * word, then it should change just that last
				 * character.  By temporarily moving the
				 * cursor back one char, we can achieve this
				 * effect without affecting the results of any
				 * other movement.
				 */
				markaddoffset(win->state->cursor, -1);
			}
		}
		else
		{
			whitespace = ElvTrue;
		}
		break;
	}

	/* remember the starting position */
	span = markoffset(win->state->cursor);

	/* Call the mode-dependent function.  If we're editing a history buffer
	 * then always use dmnormal's version; else (for the window's main
	 * buffer) use the window's mode's function.
	 */
	if (!(*(win->state->acton ? dmnormal.wordmove : win->md->wordmove))
		(win->state->cursor, vinf->count, backward, whitespace))
	{
		/* movement failed */
		return RESULT_ERROR;
	}

	/* NOTE: If we get here, then the word movement succeeded and the
	 * cursor has been moved.
	 */

	/* We need to avoid newlines for <w> movements that are used as the
	 * target of an operator (except for <c><w> which doesn't include
	 * whitespace).
	 */
	if (whitespace && vinf->oper && vinf->oper != 'c')
	{
		newline = markoffset(win->state->cursor);
		span = newline - span;
		scanalloc(&cp, win->state->cursor);
		while (scanprev(&cp) && span-- > 0)
		{
			if (*cp == '\n')
				newline = markoffset(scanmark(&cp));
		}
		scanfree(&cp);
		marksetoffset(win->state->cursor, newline);
	}
	else if (whitespace && !vinf->oper && scanchar(win->state->cursor) == '\n')
	{
		/* tried a plain old "w" command at end of file -- 
		 * move cursor back to starting point and fail.
		 */
		marksetoffset(win->state->cursor, span);
		return RESULT_ERROR;
	}

	return RESULT_COMPLETE;
}

/* This function scrolls the screen, implementing the <Control-E>, <Control-Y>,
 * <Control-F>, <Control-B>, <Control-D>, and <Control-U> commands.
 */
RESULT	m_scroll(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	assert(!win->state->acton);
	assert(!(win->state->flags & ELVIS_BOTTOM) || vinf->command == ELVCTRL('D'));

	/* adjust the count */
	if (vinf->command == ELVCTRL('U') || vinf->command == ELVCTRL('D'))
	{
		if (vinf->count == 0)
		{
			vinf->count = o_scroll(win);
		}
		else
		{
			if (vinf->count > o_lines(win) - 1)
			{
				vinf->count = o_lines(win) - 1;
			}
			o_scroll(win) = vinf->count;
		}
	}
	else
	{
		DEFAULT(1);
	}

	/* Do the scroll */
	switch (vinf->command)
	{
	  case ELVCTRL('U'):
		markset(win->di->topmark, (*win->md->move)(win, win->di->topmark, -vinf->count, 0, ElvTrue));
		markset(win->di->bottommark, (*win->md->move)(win, win->di->bottommark, -vinf->count, 0, ElvTrue));
		marksetoffset(win->cursor, markoffset(dispmove(win, -vinf->count, 0)));
		break;

	  case ELVCTRL('D'):
		markset(win->di->topmark, (*win->md->move)(win, win->di->topmark, vinf->count, 0, ElvTrue));
		markset(win->di->bottommark, (*win->md->move)(win, win->di->bottommark, vinf->count, 0, ElvTrue));
		marksetoffset(win->cursor, markoffset(dispmove(win, vinf->count, 0)));
		break;

	  case ELVCTRL('Y'):
		markset(win->di->topmark, (*win->md->move)(win, win->di->topmark, -vinf->count, 0, ElvTrue));
		markset(win->di->bottommark, (*win->md->move)(win, win->di->bottommark, -vinf->count, 0, ElvTrue));
		marksetoffset(win->cursor, markoffset(dispmove(win, 0, win->wantcol)));
		if (markoffset(win->cursor) >= markoffset(win->di->bottommark))
		{
			marksetoffset(win->cursor, markoffset(win->di->bottommark));
			marksetoffset(win->cursor, markoffset(dispmove(win, -1, win->wantcol)));
		}
		break;

	  case ELVCTRL('E'):
		markset(win->di->topmark, (*win->md->move)(win, win->di->topmark, vinf->count, 0, ElvTrue));
		markset(win->di->bottommark, (*win->md->move)(win, win->di->bottommark, vinf->count, 0, ElvTrue));
		if (markoffset(win->cursor) < markoffset(win->di->topmark))
		{
			marksetoffset(win->cursor, markoffset(win->di->topmark));
			marksetoffset(win->cursor, markoffset(dispmove(win, 0, win->wantcol)));
		}
		break;

	  case ELVCTRL('F'):
		if (o_bufchars(markbuffer(win->di->bottommark)) > 0
		 && markoffset(win->di->bottommark) == o_bufchars(markbuffer(win->di->bottommark)))
		{
			marksetoffset(win->cursor, markoffset(win->di->bottommark) - 1L);
		}
		else
		{
			marksetoffset(win->cursor, markoffset(win->di->bottommark));
			markset(win->di->topmark, dispmove(win, -2L, 0));
			marksetoffset(win->cursor, markoffset(win->di->topmark));
			markset(win->di->bottommark, (*win->md->move)(win, win->di->topmark, o_lines(win), 0, ElvTrue));
		}
		break;

	  case ELVCTRL('B'):
		if (markoffset(win->di->topmark) == 0)
		{
			marksetoffset(win->cursor, 0L);
		}
		else
		{
			marksetoffset(win->cursor, markoffset(win->di->topmark));
			markset(win->di->topmark, dispmove(win, 3L - o_lines(win), 0));
			markset(win->di->bottommark, (*win->md->move)(win, win->di->topmark, o_lines(win), 0, ElvTrue));
			markset(win->cursor, dispmove(win, 1L, 0));
		}
		break;
	}

	/* partially disable optimization for the next redraw - it doesn't
	 * automatically realize that scrolling is a type of change.
	 */
	win->di->logic = DRAW_CHANGED;

	return RESULT_COMPLETE;
}

/* This function moves the cursor to a given column.  The window's desired
 * column ("wantcol") is set to the requested column.  This implements the
 * <|>, <0>, <Control-X>, and <$> commands.
 */
RESULT	m_column(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	dest;
	long	row = 0L;

	/* choose a column number */
	switch (vinf->command)
	{
	  case '|':
	  case '0':
		DEFAULT(1);
		break;

	  case ELVCTRL('X'):
		DEFAULT(o_columns(win));
		vinf->count += win->di->skipped;
		break;

	  case '$':
		if (vinf->count > 0)
			row = vinf->count - 1;
		vinf->count = INFINITY;
		break;
	}

	/* move to the requested column (or as close as possible). */
	dest = dispmove(win, row, vinf->count - 1);
	marksetoffset(win->state->cursor, markoffset(dest));

	/* if the window is editing the main buffer, set wantcol...
	 * even if the cursor didn't quite make it to the requested column.
	 */
	win->wantcol = vinf->count - 1;

	return RESULT_COMPLETE;
}


/* This function implements the character-search commands: f, t, F, T, comma,
 * and semicolon.
 */
RESULT	m_csearch(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	static CHAR	prevcmd;	/* previous command, for <,> and <;> */
	static CHAR	prevtarget;	/* previous target character */
	CHAR		*cp;		/* used for scanning text */
	long		front, end;	/* endpoints of the line */
	long		offset;		/* offset of current char in line */

	DEFAULT(1);

	assert(strchr("fFtT,;", vinf->command));

	/* Find the endpoints of this line.  Note that we need to be careful
	 * about the end of the line, because we don't want to allow the
	 * newline character to be deleted.
	 */
	front = markoffset(dispmove(win, 0, 0));
	if (win->state->acton)
	{
		end = markoffset((*dmnormal.move)(win, win->state->cursor, 0, INFINITY, ElvTrue));
	}
	else
	{
		end = markoffset((*win->md->move)(win, win->state->cursor, 0, INFINITY, ElvTrue));
	}

	/* comma and semicolon recall the previous character search */
	if (vinf->command == ';' || vinf->command == ',')
	{
		/* fail if there was no previous command */
		if (!prevcmd)
		{
			msg(MSG_ERROR, "no previous char search");
			return RESULT_ERROR;
		}

		/* use the previous command, or its opposite */
		if (vinf->command == ';')
		{
			vinf->command = prevcmd;
		}
		else if (elvupper(prevcmd))
		{
			vinf->command = elvtolower(prevcmd);
		}
		else
		{
			vinf->command = elvtoupper(prevcmd);
		}

		/* use the previous target character, too */
		vinf->key2 = prevtarget;
	}
	else /* not comma or semicolon */
	{
		/* remember this command so it can be repeated later */
		prevcmd = vinf->command;
		prevtarget = vinf->key2;
	}

	/* Which way should we scan?  Forward or backward? */
	offset = markoffset(win->state->cursor);
	(void)scanalloc(&cp, win->state->cursor);
	if (elvlower(vinf->command))
	{
		/* scan forward */
		while (vinf->count > 0 && scannext(&cp) && ++offset <= end)
		{
			if (*cp == vinf->key2)
			{
				vinf->count--;
			}
		}
	}
	else
	{
		/* scan backward */
		while (vinf->count > 0 && scanprev(&cp) && --offset >= front)
		{
			if (*cp == vinf->key2)
			{
				vinf->count--;
			}
		}
	}

	/* if hit EOF or newline, then fail */
	if (!cp || offset < front || offset > end)
	{
		scanfree(&cp);
		return RESULT_ERROR;
	}

	/* if <t> or <T> then back off one character */
	if (vinf->command == 't')
	{
		scanprev(&cp);
	}
	else if (vinf->command == 'T')
	{
		scannext(&cp);
	}

	/* move the cursor */
	marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
	scanfree(&cp);
	return RESULT_COMPLETE;
}

/* This function moves the cursor to the next tag in the current buffer */
RESULT m_tag(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	next;	/* where the next tag is located */
#ifdef FEATURE_G
	MARKBUF	tmp;
#endif

	/* This only works when editing the main stratum */
	if (win->state->acton)
		return RESULT_ERROR;

	/* If the display mode has no "next tag" function, then fail */
	if (!win->md->tagnext)
		return RESULT_ERROR;

	/* else call the "next tag" function */
#ifdef FEATURE_G
	if (vinf->command == ELVG('\t'))
	{
		next = marktmp(tmp, markbuffer(win->cursor), 0L);
		do
		{
			if (next != &tmp)
				tmp = *next;
			next = (*win->md->tagnext)(next);
		} while (next && markoffset(next) < markoffset(win->cursor));
		next = (tmp.offset == 0) ? NULL : &tmp;
	}
	else
#endif
		next = (*win->md->tagnext)(win->cursor);
	if (!next)
		return RESULT_ERROR;

	/* move the cursor */
	assert(markbuffer(next) == markbuffer(win->state->cursor));
	marksetoffset(win->state->cursor, markoffset(next));

	/* may also require the window to be redrawn  -- we'd better check */
	win->di->logic = DRAW_CHANGED;
	return RESULT_COMPLETE;
}


/* This function implements [[ and { movement commands */
RESULT m_bsection(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	BUFFER	buf;	/* buffer being addressed */
	CHAR	*cp;
	CHAR	nroff[3];	/* characters after current position */
	CHAR	*codes;
	ELVBOOL	sect;		/* are we looking through section? */
	long	curly;

	DEFAULT(1);

	assert(vinf->command == '[' || vinf->command == '{');

	/* if this is the start of a "learn" mode, do that! */
	if (vinf->command == '[' && vinf->key2 != '[')
	{
		return maplearn(vinf->key2, ElvTrue);
	}

	/* search backward for a section or paragraph */
	buf = markbuffer(win->state->cursor);
	scanalloc(&cp, win->state->cursor);
	memset(nroff, 0, sizeof nroff);
	nroff[0] = (*cp == '{' ? ' ' : *cp);
	nroff[1] = '\n';
	curly = -1L;
	do
	{
		/* move back one character */
		if (!scanprev(&cp))
			break;

		/* if curly, then remember it for duration of line */
		if (o_tweaksection && *cp == '{')
			curly = markoffset(scanmark(&cp));

		/* if this is a newline, look for special stuff... */
		if (*cp == '\n')
		{
			if (nroff[0] == '{' || (curly >= 0L && !elvspace(nroff[0])))
				vinf->count--;
			else if (nroff[1] != '\n' && nroff[0] == '\n' && vinf->command == '{')
				vinf->count--;
			else if (nroff[0] == '.')
			{
				for (codes = o_sections(buf), sect = (ELVBOOL)(vinf->command == '{');
				     codes && *codes;
				     )
				{
					if (codes[0] == nroff[1] &&
						(codes[1] == nroff[2] ||
						    (!elvalnum(nroff[2]) &&
							(!codes[1] ||
							    codes[1] == ' '
							)
						     )
						)
					   )
					{
						vinf->count--;
						break;
					}

					/* after section, chain to paragraph */
					if ((!codes[1] || !codes[2]) && sect)
					{
						codes = o_paragraphs(buf);
						sect = ElvFalse;
					}
					else if (!codes[1])
						codes++;
					else
						codes += 2;
				}
			}

			/* moving into new line, forget curly from this line */
			if (vinf->count > 0L)
				curly = -1L;
		}

		/* shift this character into "nroff" string */
		nroff[2] = nroff[1];
		nroff[1] = nroff[0];
		nroff[0] = *cp;

	} while (vinf->count > 0);

	/* At this point, cp either points to the newline before a section,
	 * or it is NULL because we hit the top of the buffer.  If it is NULL
	 * and we were only looking to go back one more section/paragraph, and
	 * the cursor wasn't already at the top of the buffer, then we should
	 * move the cursor to the top of the buffer.  If we hit a curly brace
	 * on the final (top) line, then use that.  Otherwise a NULL cp
	 * indicates an error.  A non-NULL cp should cause the cursor to be
	 * left after the newline that it points to.
	 */

	if (!cp && vinf->count == 1 && markoffset(win->state->cursor) != 0)
	{
		marksetoffset(win->state->cursor, 0);
	}
	else if (curly >= 0L)
	{
		marksetoffset(win->state->cursor, curly);
	}
	else if (!cp)
	{
		scanfree(&cp);
		return RESULT_ERROR;
	}
	else
	{
		marksetoffset(win->state->cursor, markoffset(scanmark(&cp)) + 1);
	}
	scanfree(&cp);
	return RESULT_COMPLETE;
}



/* This function implements ]] and } movement commands */
RESULT m_fsection(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	BUFFER	buf;	/* buffer being addressed */
	CHAR	*cp;
	CHAR	nroff[3];	/* characters before current position */
	CHAR	*codes;
	long	offset;		/* offset of potential destination */
	ELVBOOL	sect;		/* are we looking through section? */
	ELVBOOL	indented;	/* does current line start with whitespace? */

	DEFAULT(1);

	assert(vinf->command == ']' || vinf->command == /*{*/'}');

	/* Initialize "offset" just to silence a compiler warning */
	offset = 0;

	/* if this is the end of a "learn" mode, do that! */
	if (vinf->command == ']' && vinf->key2 != ']')
	{
		return maplearn(vinf->key2, ElvFalse);
	}

	/* search forward for a section or paragraph */
	buf = markbuffer(win->state->cursor);
	scanalloc(&cp, win->state->cursor);
	memset(nroff, 0, sizeof nroff);
	nroff[2] = *cp;
	nroff[1] = (*cp == '.' ? '\0' : '\n');
	indented = ElvTrue;
	do
	{
		/* move ahead one character */
		if (!scannext(&cp))
			break;

		/* if start of line, then test for indentation */
		if (nroff[2] == '\n')
			indented = (ELVBOOL)elvspace(*cp);
		else if (!o_tweaksection)
			indented = ElvTrue;

		/* look for special stuff... */
		if (!indented && *cp == '{')
		{
			offset = markoffset(scanmark(&cp));
			vinf->count--;
		}
		else if (nroff[1] != '\n' && nroff[2] == '\n' && *cp == '\n' && vinf->command == '}')
		{
			offset = markoffset(scanmark(&cp));
			vinf->count--;
		}
		else if (nroff[0] == '\n' && nroff[1] == '.')
		{
			for (codes = o_sections(buf), sect = (ELVBOOL)(vinf->command == '}');
			     codes && *codes;
			     )
			{
				if (codes[0] == nroff[2] &&
					(codes[1] == *cp ||
					    (!elvalnum(*cp) &&
						(!codes[1] || codes[1] == ' ')
					    )
					)
				   )
				{
					offset = markoffset(scanmark(&cp)) - 2;
					vinf->count--;
					break;
				}

				/* after section, chain to paragraph */
				if ((!codes[1] || !codes[2]) && sect)
				{
					codes = o_paragraphs(buf);
					sect = ElvFalse;
				}
				else if (!codes[1])
					codes++;
				else
					codes += 2;
			}
		}

		/* shift this character into "nroff" string */
		nroff[0] = nroff[1];
		nroff[1] = nroff[2];
		nroff[2] = *cp;

	} while (vinf->count > 0);

	/* At this point, cp either points to the last character of a section
	 * header (and "offset" is the start of that header), or cp is NULL
	 * because we hit the end of the buffer before finding a section.
	 * If it is NULL and we were only looking to go forward 1 more section,
	 * then move the cursor to the end of the buffer.  Otherwise a NULL
	 * cp indicates an error.  A non-NULL cp should cause the cursor to be
	 * left at the "offset" value.
	 */ 

	if (!cp && vinf->count == 1
		&& markoffset(win->state->cursor) < o_bufchars(buf) - 2)
	{
		/* leave cursor on the character before the final newline,
		 * unless the final line consists of only a newline character;
		 * then leave it on that newline.
		 */
		marksetoffset(win->state->cursor, o_bufchars(buf) - 2);
		if (scanchar(win->state->cursor) == '\n')
		{
			markaddoffset(win->state->cursor, 1);
		}

		/* doing a line-mode operator? */
		if (vinf->oper)
		{
			/* include the final line */
			vinf->tweak |= TWEAK_INCL;
		}
	}
	else if (!cp)
	{
		scanfree(&cp);
		return RESULT_ERROR;
	}
	else
	{
		marksetoffset(win->state->cursor, offset);
	}
	scanfree(&cp);
	return RESULT_COMPLETE;
}


/* This implements the screen-relative movement commands: H, M, and L */
RESULT m_scrnrel(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	long	delta;	/* number of lines to move forward */
	long	srcoff;	/* offset of source line */
	MARKBUF	srcbuf;	/* mark of line that we're moving relative to */
	MARK	tmp;	/* value returned by display mode's move() function */
	int	rows;	/* number of rows showing something other than "~" */

	assert(vinf->command == 'H' || vinf->command == 'M' || vinf->command == 'L');
	assert(win->di && win->di->rows > 1);

	DEFAULT(1);

	/* see how many rows are visible */
	for (rows = win->di->rows - 2;
	     rows > 1 && win->di->newrow[rows].lineoffset >= o_bufchars(markbuffer(win->state->cursor));
	     rows--)
	{
	}

	/* choose a source offset and line delta, depending on command */
	switch (vinf->command)
	{
	  case 'H':
		delta = vinf->count - 1;
		srcoff = win->di->newrow[0].lineoffset;
		break;

	  case 'M':
		delta = 0;
		srcoff = win->di->newrow[rows / 2].lineoffset;
		break;

	  default: /* 'L' */
		delta = 1 - vinf->count;
		srcoff = win->di->newrow[rows].lineoffset;
		break;
	}

	/* if bad offset, then fail */
	if (srcoff < 0 || srcoff >= o_bufchars(markbuffer(win->state->cursor)))
		return RESULT_ERROR;

	/* maybe move forward or backward from that line */
	if (delta != 0)
	{
		(void)marktmp(srcbuf, markbuffer(win->state->cursor), srcoff);
		tmp = (*win->md->move)(win, &srcbuf, delta, 0, ElvFalse);
		if (!tmp)
			return RESULT_ERROR;
		srcoff = markoffset(tmp);
	}

	/* move the cursor to that line */
	marksetoffset(win->state->cursor, srcoff);
	return RESULT_COMPLETE;
}


RESULT m_z(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	/* This only works on the window's primary buffer */
	if (win->state->cursor != win->cursor)
		return RESULT_ERROR;

	/* If a count is given, then move the cursor to that line */
	if (vinf->count > 0 && vinf->count < o_buflines(markbuffer(win->cursor)))
	{
		marksetoffset(win->cursor,
		    lowline(bufbufinfo(markbuffer(win->cursor)), vinf->count));
	}

	/* tweak the window's top & bottom to force the current line to
	 * appear in a given location on the screen.
	 */
	switch (vinf->key2)
	{
	  case '\n':
	  case '\r':
	  case '+':
	  case 'H':
		/* The current line should appear at the top of the screen.
		 * We'll tweak the top mark so it refers to this line.  The
		 * bottom mark will be set to the end of the file.  When the
		 * window is redrawn, the redrawing logic will cause this line
		 * to be output first, and then it'll just continue showing
		 * lines until the bottom of the screen.
		 */
		markset(win->di->topmark, dispmove(win, 0L, 0));
		marksetoffset(win->di->bottommark, o_bufchars(markbuffer(win->cursor)));
		win->di->logic = DRAW_CHANGED;
		break;

	  case '.':
	  case 'z':
	  case 'M':
		/* The current line should appear in the middle of the screen.
		 * To do this, we'll set the top half a screenful's number of
		 * lines back, and the bottom some point after the cursor.
		 * We'll also set the drawing logic to perform slop-scrolling
		 * until the cursor is in the top half of the screen, just in
		 * case the lines at the top of the screen fill more than one
		 * row.
		 */
		markset(win->di->topmark, dispmove(win, -(o_lines(win) / 2), 0));
		marksetoffset(win->di->bottommark, o_bufchars(markbuffer(win->cursor)));
		win->di->logic = DRAW_CENTER;
		break;

	  case '-':
	  case 'L':
		/* The current line should appear at the bottom of the screen.
		 * To do this, we'll set the top back a whole screenload's
		 * number of lines before the cursor line, and the bottom 
		 * to some point after the current line.  When the window is
		 * redrawn, the drawing logic will start drawing from the
		 * computed top, and then scroll the window if necessary to
		 * bring the current line onto the screen.
		 */
		markset(win->di->topmark, dispmove(win, -o_lines(win), 0));
		marksetoffset(win->di->bottommark, markoffset(win->cursor) + 1);
		win->di->logic = DRAW_CHANGED;
		break;
	}

	return RESULT_COMPLETE;
}


RESULT m_fsentence(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	ELVBOOL	ending;	/* have we seen at least one sentence ender? */
	ELVBOOL	didpara;/* between paragraph and first sentence in paragraph */
	int	spaces;	/* number of spaces seen so far */
	CHAR	*cp;	/* used for scanning through text */
	CHAR	*end;	/* characters that end a sentence */
	CHAR	*quote;	/* quote/parenthesis character that may appear at end */
	CHAR	oper;	/* operator command, or '\0' */
	long	newline;/* offset of first newline in trailing whitespace */
	long	para;
	long	offset;
	long	count;

	DEFAULT(1);

	/* If sentenceend and sentencequote are unset, use default values */
	end = o_sentenceend ? o_sentenceend : toCHAR(".?!");
	quote = o_sentencequote ? o_sentencequote : toCHAR("\")]");

	count = vinf->count;
	oper = vinf->oper;

	/* detect whether we're at the start of a paragraph */
	offset = markoffset(win->state->cursor);
	scanalloc(&cp, win->state->cursor);
	if (*cp != '\n')
		scanprev(&cp);
	else
		while (cp && *cp == '\n')
		{
			scanprev(&cp);
		}
	if (cp)
	{
		marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
		vinf->count = 1;
		vinf->command = '}';
		m_fsection(win, vinf);
		para = markoffset(win->state->cursor);
	}
	else
	{
		para = 0;
	}
	marksetoffset(win->state->cursor, offset);
	if (para == offset)
		count++;
	else if (para < offset)
	{
		/* find the next paragraph */
		marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
		vinf->count = 1;
		vinf->command = '}';
		m_fsection(win, vinf);
		para = markoffset(win->state->cursor);
	}

	/* for each count... */
	newline = -1;
	scanseek(&cp, win->state->cursor);
	for (; cp && count > 0; count--)
	{
		/* for each character in the sentence... */
		for (ending = didpara = ElvFalse, spaces = 0;
		     cp && (!ending || spaces < o_sentencegap || elvspace(*cp));
		     scannext(&cp), offset++)
		{
			/* if paragraph, then... */
			if (offset == para)
			{
				/* if still more sentences to skip... */
				if (count > 1)
				{
					/* count this as a sentence, and
					 * arrange for next line to also be a
					 * sentence.
					 */
					count--;
					newline = -1;
					ending = ElvTrue;
					if (*cp == '\n')
					{
						didpara = ElvFalse;
						spaces = o_sentencegap;
					}
					else
					{
						didpara = ElvTrue;
						spaces = 0;
					}

					/* oh, and we need to find the next
					 * paragraph, too.
					 */
					marksetoffset(win->state->cursor, offset);
					vinf->count = 1;
					vinf->command = '}';/*{*/
					if (m_fsection(win, vinf) == RESULT_COMPLETE)
						para = markoffset(win->state->cursor);
					else
						para = -1;

					continue;
				}
				else
				{
					/* we're here! */
					break;
				}
			}

			/* check the character */
			if (*cp == '\n')
			{
				spaces = o_sentencegap;
				didpara = ElvFalse;
				if (newline < 0)
					newline = markoffset(scanmark(&cp));
			}
			else if (didpara)
				/* skip characters in a ".P" line */;
			else if (elvspace(*cp))
				spaces++;
			else if (CHARchr(end, *cp))
				newline = -1, ending = ElvTrue, spaces = 0;
			else if (!CHARchr(quote, *cp))
				newline = -1, ending = ElvFalse, spaces = 0;
			else /* quote character */
				newline = -1;
		}
	}

	/* did we find it? */
	if (cp)
	{
		/* if target of operator, and a newline was encountered in the
		 * trailing whitespace, then move the cursor to that newline;
		 * else move the cursor to the start of the following sentence.
		 */
		if (oper && newline >= 0)
			marksetoffset(win->state->cursor, newline);
		else
			marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
	}
	scanfree(&cp);
	return cp ? RESULT_COMPLETE : RESULT_ERROR;
}

RESULT m_bsentence(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	ELVBOOL	first;	/* ElvTrue until we pass some mid-sentence stuff */
	ELVBOOL	ending;	/* have we seen at least one sentence ender? */
	ELVBOOL	anynext;/* any text seen on following line */
	ELVBOOL	anythis;/* any text seen on this line */
	int	spaces;	/* number of spaces seen so far */
	CHAR	*cp;	/* used for scanning through text */
	CHAR	*end;	/* characters that end a sentence */
	CHAR	*quote;	/* quote/parenthesis character that may appear at end */
	long	count;	/* sentences to move over */
	long	para;	/* top of current paragraph */
	long	offset;
	RESULT	result;

	DEFAULT(1);

	/* If sentenceend and sentencequote are unset, use default values */
	end = o_sentenceend ? o_sentenceend : toCHAR(".?!");
	quote = o_sentencequote ? o_sentencequote : toCHAR("\")]");

	/* misc initialization */
	anynext = anythis = ElvFalse;
	offset = markoffset(win->state->cursor);

	/* NOTE: The "first" variable is used to handle the situation where
	 * we start at the beginning of one sentence.  From here, we want
	 * to go back one extra sentence-end; otherwise <(> would just move
	 * us to the start of the same sentence.
	 */
	first = ElvTrue;

	/* Start scanning at the cursor location */
	scanalloc(&cp, win->state->cursor);
	count = vinf->count;

	/* Find the start of this paragraph.  That counts as a "sentence" */
	vinf->command = '{';
	vinf->count = 1;
	para = (m_bsection(win, vinf) == RESULT_COMPLETE) ? markoffset(win->state->cursor) : -1;

	/* For each count... */
	for (; cp && count > 0; count--)
	{
		/* for each character in the sentence... */
		for (ending = ElvTrue, anythis = anynext = ElvFalse, spaces = 0,
			scanprev(&cp), offset = markoffset(scanmark(&cp));
		     cp && offset != para &&
			(!ending || spaces<o_sentencegap || !CHARchr(end,*cp));
		     scanprev(&cp), offset--)
		{
			if (*cp == '\n')
			{
				spaces = o_sentencegap;
				ending = ElvTrue;
				anynext = anythis;
				anythis = ElvFalse;
			}
			else if (elvspace(*cp))
			{
				spaces++,
				ending = ElvTrue;
			}
			else if (!CHARchr(quote, *cp))
			{
				first = ending = ElvFalse;
				anythis = ElvTrue;
				spaces = 0;
			}
			else
			{
				anythis = ElvTrue;
			}
		}

		/* If this sentence is actually a paragraph start, and we
		 * still have more sentences to move over, then find the
		 * next higher paragraph.
		 */
		if (offset == para && count > 1)
		{
			vinf->count = 1;
			para = (m_bsection(win, vinf) == RESULT_COMPLETE) ? markoffset(win->state->cursor) : -1;
		}

		/* If this sentence ender was encountered before any
		 * mid-sentence text (i.e., if we started at the front of
		 * a sentence) then we should go back one extra sentence-end.
		 */
		if (first && offset != para)
		{
			count++;
		}
	}

	/* If we hit a paragraph top which occurs on a blank line, then we
	 * need to do a little extra processing because we exited the loop
	 * a little too soon.
	 */
	if (offset == para && cp && *cp == '\n')
	{
		anynext = anythis;
	}

	/* did we find it? */
	if (cp)
	{
		/* move the cursor to the start of the next sentence */
		if (offset > para)
		{
			/* found a sentence end -- move the cursor to the to
			 * start of the following sentence.
			 */
			do
			{
				scannext(&cp);
				assert(cp);
			} while (elvspace(*cp) || CHARchr(quote, *cp));
			marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
		}
		else if (anynext)
		{
			/* bumped into a paragraph start after scanning some
			 * sentence text -- move the cursor to the first text
			 * on the next non-blank line.
			 */
			while (cp && *cp != '\n')
			{
				scannext(&cp);
			}
			while (cp && elvspace(*cp))
			{
				scannext(&cp);
			}
			if (&cp)
				marksetoffset(win->state->cursor, markoffset(scanmark(&cp)));
			else
			{
				marksetoffset(win->state->cursor, o_bufchars(markbuffer(win->state->cursor)) - 2);
				if (markoffset(win->state->cursor) < 0)
					marksetoffset(win->state->cursor, 0);
			}
		}
		else
		{
			/* found a paragraph start -- leave the cursor there */
			offset = para;
			marksetoffset(win->state->cursor, para);
		}
		result = RESULT_COMPLETE;
	}
	else if (count == 0)
	{
		/* move the cursor to the first character of the buffer */
		marksetoffset(win->state->cursor, 0);
		result = RESULT_COMPLETE;
	}
	else
	{
		/* tried to move past beginning of buffer */
		result = RESULT_ERROR;
	}
	scanfree(&cp);
	return result;
}


/* find the next misspelled word */
RESULT m_spell(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
#ifdef FEATURE_SPELL
	MARK	badword;

	/* only works in window's main edit buffer */
	if (win->state->cursor != win->cursor)
		return RESULT_ERROR;

	/* if given a count, then try to find the count'th alternative for
	 * the current word, and replace the bad word with it.  This only
	 * works if the "show" option's value contains "spell".
	 */
	if (vinf->count > 0)
	{
		if (!spellcount(win->cursor, vinf->count))
		{
			return RESULT_ERROR;
		}
	}

	/* find the next bad word */
	badword = spellnext(win, win->cursor);
	if (badword)
	{
		marksetoffset(win->cursor, markoffset(badword));
		return RESULT_COMPLETE;
	}

	/* I guess there weren't any */
	msg(MSG_ERROR, "no misspelled words below");
#endif
	return RESULT_ERROR;
}

/* move to the end of the current spelling word */
RESULT m_endspell(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
#ifdef FEATURE_SPELL

	/* find the extents of the current word */
	if (wordatcursor(win->state->cursor, ElvTrue) != NULL)
	{
		markaddoffset(win->state->cursor, -1L);
		return RESULT_COMPLETE;
	}
#endif
	return RESULT_ERROR;
}


#ifdef FEATURE_G
static long *goffsets;	/* list of offsets */
static long gwidth;	/* width of line (length of goffsets array) */
static CHAR gcommand;	/* this affects which whitespace is allowed */

/* This function is used as the "draw" function by a display mode's image()
 * function.  It builds a list of offsets.  This function is only used by the
 * m_g() motion function, below.
 *
 * Actually, the offset list is a little complicated.  Synthesized characters
 * are distinguished by a negative offset; we can't move the cursor onto them.
 * Control characters do come from the buffer, but should act as though they
 * are synthesized so we don't leave the cursor on a newline.  Some commands
 * also ignore other whitespace.
 */
static void gdraw(p, qty, font, offset)
	CHAR	*p;	/* first letter of text to draw */
	long	qty;	/* quantity to draw (negative to repeat *p) */
	_char_	font;	/* font code of the text */
	long	offset;	/* buffer offset of *p */
{
	long	delta;		/* value to add to "offset" */
	long	*newoff;

	/* A negative qty value indicates that all characters have the same
	 * offset.  Otherwise the characters will have consecutive offsets.
	 */
	if (qty < 0)
	{
		qty = -qty;
		delta = 0;
	}
	else
	{
		delta = 1;
	}

	/* for each character... */
	for ( ; qty > 0; qty--, offset += delta, p += delta)
	{
		/* if necessary, expand the goffsets array */
		if (gwidth % 1024 == 0)
		{
			newoff = safealloc(gwidth + 1024, sizeof(long));
			if (gwidth > 0)
			{
				memcpy(newoff, goffsets, gwidth * sizeof(long));
				safefree(goffsets);
			}
			goffsets = newoff;
		}

		/* store this offset */
		if (*p < ' ' || (*p == ' ' && gcommand == '^'))
			goffsets[gwidth++] = -1L;
		else
			goffsets[gwidth++] = offset;
	}
}

/* This returns the (line-oriented) column position of the start of a row,
 * given a (line-oriented) column position in that row.
 */
static long gstartrow(win, col)
	WINDOW	win;	/* window containing the line */
	long	col;	/* line-oriented column position */
{
	ELVBOOL number;	/* does this line start with a line number? */

	number = (ELVBOOL)(o_number(win) && win->cursor == win->state->cursor);
	if (number)
		col += 8;
	col -= col % o_columns(win);
	if (number)
	{
		col -= 8;
		if (col < 0)
			col = 0;
	}
	return col;
}
#endif /* FEATURE_G */


/* This implements the g0, g^, g$, gh, and gl motion commands */
RESULT m_g (win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
#ifdef FEATURE_G
	MARKBUF	mark;
	MARK	line;
	long	i;
	long	start, end, hardend;	/* where cursor's row starts & ends */
	ELVBOOL	history;

	/* This is just to silence a bogus compiler warning */
	i = 0;

	/* All of these commands format a line, and try to move with it by
	 * its image, skipping over hidden characters and taking wrap into
	 * account.  Build an array of offsets.
	 */
	goffsets = NULL;
	gwidth = 0;
	gcommand = ELVUNG(vinf->command);
	mark = *win->state->cursor;
	marksetoffset(&mark, o_bufchars(markbuffer(&mark)));
	history = (ELVBOOL)(win->state->cursor != win->cursor);
	if (history)
	{
		line = (*dmnormal.setup)(win, win->state->cursor,
						markoffset(win->state->cursor),
						&mark, NULL);
		line = (*dmnormal.image)(win, line, win->mi, gdraw);
	}
	else
	{
		line = (*win->md->setup)(win, win->cursor,
						markoffset(win->cursor),
						&mark, win->mi);
		line = (*win->md->image)(win, line, win->mi, gdraw);
	}
	assert(gwidth > 0);

	/* find the start & end of the cursor's row */
	if (o_wrap(win))
	{
		for (i = 0; i < gwidth && goffsets[i] < markoffset(win->state->cursor); i++)
		{
		}
		start = gstartrow(win, i);
		end = gstartrow(win, start + o_columns(win));
	}
	else
	{
		start = win->di->skipped;
		end = start + o_columns(win);
	}
	hardend = end;
	if (end > gwidth)
		end = gwidth;

	/* find the index of the desired character */
	switch (vinf->command)
	{
	  case ELVG('0'):
	  case ELVG('^'):
		for (i = start; i < end && goffsets[i] < 0; i++)
		{
		}
		if (i >= end)
			goto Fail;
		break;

	  case ELVG('$'):
		for (i = end; --i >= start && goffsets[i] < 0; )
		{
		}
		if (i < start)
			goto Fail;
		win->wantcol = hardend - 1L;
		break;

	  case ELVG('h'):
		DEFAULT(1);

		/* find the cursor */
		for (i = 0; i < gwidth && goffsets[i] < markoffset(win->state->cursor); i++)
		{
		}

		/* move left, skipping non-buffer chars & duplicates */
		while (vinf->count > 0 && --i >= 0)
			if (goffsets[i] >= 0 && (i+1 >= gwidth || goffsets[i] != goffsets[i+1]))
				vinf->count--;
		if (vinf->count > 0)
			goto Fail;
		break;

	  case ELVG('l'):
		DEFAULT(1);

		/* find the cursor */
		for (i = 0; i < gwidth && goffsets[i] < markoffset(win->state->cursor); i++)
		{
		}

		/* move right, skipping non-buffer chars & duplicates */
		while (vinf->count > 0 && ++i < gwidth)
			if (goffsets[i] >= 0 && goffsets[i] != goffsets[i-1])
				vinf->count--;
		if (vinf->count > 0)
			goto Fail;
		break;
	}

	/* adjust the cursor's offset */
	assert(i >= 0 && i < gwidth);
	assert(goffsets[i] >= 0);
	marksetoffset(win->state->cursor, goffsets[i]);

	safefree(goffsets);
	return RESULT_COMPLETE;

Fail:
	safefree(goffsets);
#endif /* FEATURE_G */
	return RESULT_ERROR;
}


/* implements the gj and gk commands */
RESULT m_gupdown(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
#ifdef FEATURE_G
	long	wantcol;/* desired column within a line */
	long	col;	/* some other column */
	long	rowcol;	/* column number of start of row */
	MARK	newcurs;
	long	origoff;

	DEFAULT(1);

	/* This is just to silence a bogus compiler warning */
	col = 0;

	/* If "wrap" is off, then gj and gk are the same as j and k.  For the
	 * sake of simplicity, we will also treat them the same if we're editing
	 * a history buffer -- even though gj and gk could conceivably be useful
	 * when entering long command lines, this doesn't seem useful enough to
	 * justify the effort needed.
	 */
	if (!o_wrap(win) || win->state->cursor != win->cursor)
	{
		vinf->command = ELVUNG(vinf->command);
		return m_updown(win, vinf);
	}

	/* Otherwise, we try to move using the 'wantcol' column.  This is
	 * trickier than you might think, since we need to convert line-oriented
	 * columns to row-oriented columns -- i.e., if win->wantcol == 60, and
	 * lines wrap at 80 columns, you could be moving from 140 to 220.
	 */

	/* some initialization */
	origoff = markoffset(win->cursor);
	wantcol = win->wantcol;
	if (vinf->command == ELVG('j'))
	{
		newcurs = dispmove(win, 0L, INFINITY);
		marksetoffset(win->cursor, markoffset(newcurs));
		col = dispmark2col(win);
	}

	/* for each row that the user wants to move... */
	while (--vinf->count >= 0)
	{
		switch (vinf->command)
		{
		  case ELVG('k'):
			if (gstartrow(win, wantcol) > 0)
			{
				/* adjust wantcol within this line */
				wantcol -= o_columns(win);

				/* When line numbers are shown, it is possible
				 * for wantcol to be negative.  Although the
				 * gj and gk commands could do useful things
				 * with this, wantcol is also used elsewhere
				 * by functions that don't like negative column
				 * numbers, so we force it to 0 here.
				 */
				if (wantcol < 0)
					wantcol = 0;
			}
			else
			{
				/* move up to previous line.  fail at top */
				newcurs = dispmove(win, -1L, INFINITY);
				if (markoffset(newcurs) >= markoffset(win->cursor))
					goto Fail;
				marksetoffset(win->cursor, markoffset(newcurs));

				/* get the column number at the end of the line */
				col = dispmark2col(win);

				/* we want the column on the last row of the
				 * line.  This may be past the physical end of
				 * the line, but that's okay.
				 */
				rowcol = gstartrow(win, col);
				while (gstartrow(win, wantcol) < rowcol)
					wantcol += o_columns(win);
			}
			break;

		  case ELVG('j'):
			rowcol = gstartrow(win, col);
			if (gstartrow(win, wantcol) < rowcol)
			{
				wantcol += o_columns(win);
			}
			else
			{
				/* need to move down a line */
				newcurs = dispmove(win, 1L, INFINITY);
				if (markoffset(newcurs) == markoffset(win->cursor))
					goto Fail;
				marksetoffset(win->cursor, markoffset(newcurs));
				col = dispmark2col(win);
				wantcol %= o_columns(win);
				if (gstartrow(win, wantcol) > 0)
					wantcol = 0;
			}
			break;
		}
	}

	/* Success!  We're already in the desired line, so now we just need to
	 * move to the desired row.
	 */
	newcurs = dispmove(win, 0L, wantcol);
	marksetoffset(win->cursor, markoffset(newcurs));
	win->wantcol = wantcol;
	return RESULT_COMPLETE;

Fail:
	marksetoffset(win->cursor, origoff);
#endif /* FEATURE_G */
	return RESULT_ERROR;
}
