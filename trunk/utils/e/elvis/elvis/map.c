/* map.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_map[] = "$Id: map.c,v 2.54 2003/10/17 17:41:23 steve Exp $";
#endif

#ifdef FEATURE_MAPDB
static void trace P_((char *where));
#endif

/* This structure is used to store maps and abbreviations.  The distinction
 * between them is that maps are stored in the list referenced by the "maps"
 * pointer, while abbreviations are referenced by the "abbrs" pointer.
 */
typedef struct _map
{
	struct _map	*next;	/* another mapping */
	CHAR		*label;	/* label of the map/abbr, or NULL */
	CHAR		*rawin;	/* the "rawin" characters */
	CHAR		*cooked;/* the "cooked" characters */
	CHAR		*mode;	/* mapmode setting to use (NULL for any) */
	short		rawlen;	/* length of the "rawin" characters */
	short		cooklen;/* length of the "cooked" characters */
	MAPFLAGS	flags;	/* various flags */
	ELVBOOL		invoked;/* has the map been used lately? */
} MAP;

static MAP	*maps;	/* the map table */
static MAP	*abbrs;	/* the abbreviation table */



/* This function adds a map to the map table, or replaces an existing map.
 * New maps will be added to the end of the list of maps.
 *
 * "rawin" is the first argument to a ":map" command and "cooked" is the
 * second.  "rawlen" and "cooklen" are their lengths; the strings don't need
 * to be NUL terminated.  The "rawin" string may be either the actual characters
 * to be mapped, or a key label which is supported by the GUI; this function
 * will call the GUI's keylabel() function to perform the conversion.  This
 * function will allocate copies of both of these strings, so the calling
 * function can discard its own copies as soon as this function returns.
 * 
 *"label" is either NULL or a key label.  The :map command always passes NULL,
 * and the GUI's init will usually call this function with a label string.  The
 * label should be NUL-terminated, and its storage space cannot be discarded
 * after the map because this function does *NOT* make a copy of the label.
 *
 * "flags" indicates when the map should be effective.
 */
void mapinsert(rawin, rawlen, cooked, cooklen, label, flags, mode)
	CHAR	*rawin;	/* characters sent by key */
	int	rawlen;	/* length of rawin */
	CHAR	*cooked;/* characters that the key should appear to send */
	int	cooklen;/* length of cooked */
	CHAR	*label;	/* label of the key */
	MAPFLAGS flags;	/* when the map should take effect */
	CHAR	*mode;	/* mapmode value, or NULL for any */
{
	MAP	*scan, *lag;
	MAP	**head;	/* points to either "maps" or "abbrs" */
	CHAR	*dummy;
	CHAR	*mustfree = NULL;
	int	i;

	assert(flags & MAP_WHEN);

	/* Determine whether this will be a map or abbreviation */
	head = (flags & MAP_ABBR) ? &abbrs : &maps;

	/* if no label was supplied, maybe we should try to find one? */
	if (head == &maps && !label)
	{
		/* convert <key> to the raw version */
		i = guikeylabel(rawin, rawlen, &label, &rawin);
		if (i > 0)
		{
			rawlen = i;
		}

		/* make a local copy of the raw text */
		mustfree = (CHAR *)safealloc(rawlen, sizeof(CHAR));
		memcpy(mustfree, rawin, rawlen * sizeof(CHAR));
		rawin = mustfree;

		/* convert <key> symbols in the rhs too */
		cooklen = guikeylabel(cooked, cooklen, &dummy, &cooked);
	}

	/* If any previous map applied to this context & raw value, it doesn't
	 * apply anymore.  And if that leaves it not applying to anything,
	 * then delete it.
	 */
	for (scan = *head, lag = NULL; scan; )
	{
		/* If raw text or mode doesn't match, then skip it */
		if (scan->rawlen != rawlen
		 || memcmp(scan->rawin, rawin, rawlen * sizeof(CHAR))
		 || (mode && scan->mode && CHARcmp(mode, scan->mode)))
		{
			lag = scan, scan = scan->next;
			continue;
		}

		/* If cooked text also matches, then include the old map's
		 * contexts with the new map, so the new map can completely
		 * replace the old one.
		 */
		if (scan->cooklen == cooklen
		 && !memcmp(scan->cooked, cooked, cooklen * sizeof(CHAR)))
		{
			flags |= scan->flags & (MAP_WHEN|MAP_ASCMD);
		}

		/* If existing map's context overlaps the new map's context,
		 * then tweak the existing flag's context to exclude the new
		 * map's context, so the new map is the only one that applies.
		 */
		if ((scan->flags & flags & MAP_WHEN) != 0)
		{
			/* change the existing map's context to exclude the
			 * contexts of the new map.  Note that "visual" goes
			 * away if neither "input" nor "history" is set.
			 */
			scan->flags &= ~(flags & MAP_WHEN);
			if ((scan->flags & (MAP_INPUT|MAP_HISTORY)) == 0)
				scan->flags &= ~MAP_ASCMD;

			/* if existing map doesn't have any context left,
			 * delete it
			 */
			if ((scan->flags & MAP_WHEN) == 0)
			{
				/* remove the map from the list of maps */
				if (lag)
					lag->next = scan->next;
				else
					*head = scan->next;
			
				/* free the map */
				safefree(scan->rawin);
				safefree(scan->cooked);
				safefree(scan);

				/* move to next map */
				scan = (lag ? lag->next : *head);
			}
			else
				lag = scan, scan = scan->next;
		}
		else
			lag = scan, scan = scan->next;
	}

	/* allocate & initialize a MAP struct */
	scan = (MAP *)safekept(1, sizeof(MAP));
	scan->label = label;
	scan->rawin = (CHAR *)safekept(rawlen, sizeof(CHAR));
	memcpy(scan->rawin, rawin, rawlen * sizeof(CHAR));
	scan->rawlen = rawlen;
	scan->cooked = (CHAR *)safekept(cooklen, sizeof(CHAR));
	memcpy(scan->cooked, cooked, cooklen * sizeof(CHAR));
	scan->cooklen = cooklen;
	scan->flags = flags;
	scan->mode = (mode ? CHARkdup(mode) : NULL);

	/* append it to the list of maps */
	if (lag)
	{
		lag->next = scan;
	}
	else
	{
		*head = scan;
	}

	/* if we used a temporary string, then free it now */
	if (mustfree)
		safefree(mustfree);
}

/* This function deletes a map, or changes its break flag.  It is used by the
 * :unmap, :break, and :unbreak commands.  The "rawin" string can be either
 * the label or rawin string.  Returns ElvTrue if successful, or ElvFalse if the map
 * couldn't be found.
 */
ELVBOOL mapdelete(rawin, rawlen, flags, mode, del, brk)
	CHAR	*rawin;	/* the key to be unmapped */
	int	rawlen;	/* length of rawin */
	MAPFLAGS flags;	/* when the key is mapped now */
	CHAR	*mode;	/* mapmode value, or NULL for any */
	ELVBOOL	del;	/* delete the map? (else adjust break flag) */
	ELVBOOL	brk;	/* what to set the break flag to */
{
	MAP	*scan, *lag;
	MAP	**head;
	CHAR	*label;
	int	i;
	ELVBOOL	retval = ElvFalse;

	/* Determine whether this will be a map or abbreviation */
	head = (flags & MAP_ABBR) ? &abbrs : &maps;

	/* When unmapping, we only care about the keystroke parser bits */
	flags &= MAP_WHEN;

	/* if no label was supplied, maybe we should try to find one? */
	if (head == &maps)
	{
		i = guikeylabel(rawin, rawlen, &label, &rawin);
		if (i > 0)
		{
			rawlen = i;
		}
	}

	/* Search for any existing maps which match the given raw text and any
	 * of the contexts.
	 */
	for (scan = *head, lag = NULL; scan; )
	{
		/* If not for this context, or raw text doesn't match,
		 * then skip it.
		 */
		if ((scan->flags & flags) == 0
		 || (mode && (!scan->mode || CHARcmp(mode, scan->mode)))
		 || scan->rawlen != rawlen
		 || memcmp(scan->rawin, rawin, rawlen * sizeof(CHAR)))
		{
			lag = scan, scan = scan->next;
			continue;
		}
		retval = ElvTrue;

		/* What are we supposed to do with it? */
		if (del)
		{
			/* Remove the given context from this map.  If that
			 * leaves the map without any context, then delete it.
			 */
			scan->flags &= ~flags;
			if ((scan->flags & MAP_WHEN) == 0)
			{
				if ((scan->flags & MAP_WHEN) == 0)
				{
					/* remove *scan from the list of maps */
					if (lag)
						lag->next = scan->next;
					else
						*head = scan->next;
				
					/* free the map */
					safefree(scan->rawin);
					safefree(scan->cooked);
					if (scan->mode)
						safefree(scan->mode);
					safefree(scan);

					/* move to next map */
					scan = (lag ? lag->next : *head);
				}
				else
					lag = scan, scan = scan->next;
			}
			else
			{
				/* The map is still used in some context.
				 * If it no longer applies to either "input"
				 * or "history", then it should loose "visual".
				 */
				if ((scan->flags & (MAP_INPUT|MAP_HISTORY)) == 0)
					scan->flags &= ~MAP_ASCMD;

				lag = scan, scan = scan->next;
			}
		}
#ifdef FEATURE_MAPDB
		else if (brk)
		{
			/* set a breakpoint on this map */
			scan->flags |= MAP_BREAK;

			lag = scan, scan = scan->next;
		}
		else
		{
			/* clear a breakpoint on this map */
			scan->flags &= ~MAP_BREAK;
			
			lag = scan, scan = scan->next;
		}
#endif /* FEATURE_MAPDB */
	}

	return retval;
}


/* These two variables are used to store characters which have been read but
 * not yet parsed, and maybe not even mapped.
 */
static CHAR	queue[500];	/* the mapping queue */
static int	qty = 0;	/* number of keys in the queue */
static int	resolved = 0;	/* number of resolved keys (no mapping needed) */
static long	learning;	/* bitmap of "learn" buffers */

#ifdef FEATURE_MAPDB
static CHAR	traceimg[60];	/* image of queue, for maptrace option */
static ELVBOOL	tracereal;	/* any real keys since last trace? */
static ELVBOOL	tracestep;	/* don't queue next keystroke */

/* build the trace image, and show it */
static void trace(where)
	char	*where;
{
	int	i, j, end;
	BUFFER	log;
	MARKBUF	logend;
	MARKBUF	logstart;
	CHAR	ch[1];

	/* if not tracing, then do nothing */
	if (o_maptrace == 'o')
		return;

	/* reset tracereal */
	tracereal = ElvFalse;

	/* Decide how much of the queue to show. */
	for (end = qty; end >= 25; end -= 10)
	{
	}

	/* generate the image */
	for (i = j = 0; i < end; i++, j += 2)
	{
		if (elvcntrl(queue[i]))
		{
			traceimg[j] = '^';
			traceimg[j + 1] = ELVCTRL(queue[i]);
		}
		else
		{
			traceimg[j] = ' ';
			traceimg[j + 1] = queue[i];
		}
	}
	traceimg[j] = '\0';

	/* show the image */
	msg(MSG_STATUS, "[sSdd]$1:($2>>50)", where, traceimg, resolved, qty);
	(void)eventdraw(windefault->gw);
	guiflush();

	/* maybe log it */
	if (o_maplog != 'o')
	{
		log = bufalloc(toCHAR(TRACE_BUF), 0, ElvTrue);
		if (o_maplog == 'r')
		{
			bufreplace(marktmp(logstart, log, 0L), marktmp(logend, log, o_bufchars(log)), NULL, 0L);
			o_maplog = 'a';
		}
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, toCHAR(where), (long)strlen(where));
		ch[0] = ':';
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, ch, 1L);
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, traceimg, (long)CHARlen(traceimg));
		ch[0] = '\n';
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, ch, 1L);
	}

	/* maybe arrange for single-stepping to occur on next keystroke */
	if (o_maptrace == 's'
	 && !(windefault && (windefault->state->mapflags & MAP_DISABLE)))
	{
		tracestep = ElvTrue;
	}
}
#endif /* FEATURE_MAPDB */

/* This function implements mapping.  It is called with either 1 or more new
 * characters from keypress events, or with 0 to indicate that a timeout
 * occurred.  It calls the current window's keystroke parser; we assume that
 * the windefault variable is set correctly.
 */
MAPSTATE mapdo(keys, nkeys)
	CHAR	*keys;	/* characters from the keyboard */
	int	nkeys;	/* number of keys */
{
	MAP		*scan;		/* used for scanning through maps */
	int		ambkey, ambuser;/* ambiguous key maps and user maps */
	MAP		*match;		/* longest fully matching map */
	ELVBOOL		ascmd = ElvFalse;	/* did we just resolve an ASCMD map? */
	ELVBOOL		didtimeout;	/* did we timeout? */
	MAPFLAGS	now;		/* current keystroke parsing state */
	BUFFER		buf;		/* a cut buffer that is in "learn" mode */
	CHAR		cbname;		/* name of cut buffer */
	MARKBUF		mark;		/* points to the end of buf */
	int		i, j;

	assert(0 <= resolved && resolved <= qty);
	assert(windefault);

	/* if nkeys==0 then we timed out */
	didtimeout = (ELVBOOL)(nkeys == 0);

#ifdef FEATURE_MAPDB
	/* tracing */
	if (!tracereal && guipoll(ElvFalse))
	{
		tracereal = ElvTrue;
		mapalert();
	}
	if (tracestep)
	{
		if (keys[0] == ELVCTRL('C'))
		{
			tracereal = ElvTrue;
			mapalert();
		}
		else if (keys[0] == 'r')
		{
			o_maptrace = 'r';
		}
		nkeys = 0;
		tracestep = ElvFalse;
	}
	if (nkeys > 0)
	{
		tracereal = ElvTrue;
	}
#endif /* FEATURE_MAPDB */

#ifdef FEATURE_AUTOCMD
	/* detect when the display mode changes, so we can trigger DispMapLeave
	 * and DispMapEnter autocmds.  These are often used for implementing
	 * display-specific maps.
	 */
	audispmap();
#endif

	/* append the keys to any cutbuffer that is in "learn" mode */
	if (learning)
	{
		for (cbname = 'a'; cbname <= 'z'; cbname++)
		{
			if (learning & (1 << (cbname & 0x1f)))
			{
				buf = cutbuffer(cbname, ElvFalse);
				if (buf)
				{
					bufreplace(marktmp(mark, buf,
							    o_bufchars(buf)),
						    &mark, keys, (long)nkeys);
				}
			}
		}
	}

	/* Add the new keys to the end of the queue, being careful
	 * to avoid overflow.
	 */
	while (qty < QTY(queue) && nkeys > 0)
	{
		queue[qty++] = *keys++;
		nkeys--;
	}

	/* repeatedly apply maps and then parse resolved keys */
	for (;;)
	{
		/* send any resolved keys to the current window's parser */
		if (resolved > 0)
		{
			while (resolved > 0)
			{
				/* If there aren't any more windows, stop! */
				if (!windows)
					return MAP_CLEAR;

				assert(windefault);

#ifdef FEATURE_MAPDB
				/* maybe show trace */
				if (!tracereal) trace("cmd");
#endif

				/* Delete the next keystroke from the queue */
				resolved--;
				qty--;
				j = queue[0];
				for (i = 0; i < qty; i++)
				{
					queue[i] = queue[i + 1];
				}

				/* If the key is supposed to be treated as a
				 * command, then send a ^O before the keystroke.
				 * This is a kludgy way to implement the
				 * "visual" maps.
				 */
				if (ascmd)
				{
					(void)statekey(ELVCTRL('O'));
				}

				/* Make sure the MAP_DISABLE flag is turned off.
				 * Any character of the cooked string can force
				 * it on, but we only care if the *last* one
				 * forces it on.  By forcing it off before
				 * handling each cooked keystroke, we can
				 * ignore all but the last.
				 */
				windefault->state->mapflags &= ~MAP_DISABLE;

				/* Send the keystroke to the parser. */
				(void)statekey((_CHAR_)j);

#ifdef FEATURE_MAPDB
				/* if single-stepping, then we're done for now*/
				if (tracestep)
				{
					return MAP_CLEAR;
				}
#endif
			}
			ascmd = ElvFalse;
		}

		/* if all keys have been processed, then return MAP_CLEAR */
		if (qty == 0)
		{
			assert(resolved == 0);
			for (scan = maps; scan; scan = scan->next)
			{
				scan->invoked = ElvFalse;
			}
			return MAP_CLEAR;
		}

		/* figure out what the current map context is */
		if (windefault->state->mapflags & MAP_DISABLE)
		{
			now = 0;
			windefault->state->mapflags &= ~MAP_DISABLE;
		}
		else if (windefault->seltop
		      && (windefault->state->mapflags & (MAP_COMMAND|MAP_SELECT|MAP_MOTION)) != 0)
		{
			now = MAP_SELECT;
		}
		else
		{
			now = (windefault->state->mapflags & MAP_WHEN);
		}

		/* try to match the remaining keys to each map */
		ambkey = ambuser = 0;
		match = NULL;
		if (now & (MAP_WHEN)) /* if mapping is allowed... */
		{
			for (scan = maps; scan; scan = scan->next)
			{
				/* ignore maps for a different context */
				if ((scan->flags & now) != now
				 || (scan->mode
				     && (!o_mapmode(bufdefault)
				 	 || CHARcmp(scan->mode, o_mapmode(bufdefault)))))
				{
					continue;
				}

				/* is it an ambiguous (incomplete) match? */
				if (!didtimeout
				 && scan->rawlen > qty
				 && !memcmp(scan->rawin, queue, qty * sizeof(CHAR)))
				{
					/* increment ambiguous match counter */
					if (scan->label)
						ambkey++;
					else
						ambuser++;
				}

				/* is it a complete match, and either the first
				 * such or longer than any previous complete
				 * matches?
				 */
				if (scan->rawlen <= qty
					&& !memcmp(scan->rawin, queue, scan->rawlen * sizeof(CHAR))
					&& (!match || match->rawlen < scan->rawlen))
				{
					/* remember this match */
					match = scan;
				}
			}
		}

		/* if ambiguous, then return MAP_USER or MAP_KEY */
		if (!o_timeout && (ambuser > 0 || ambkey > 0))
			return MAP_CLEAR; /* so no timeouts occur */
		else if (ambuser > 0)
			return MAP_USER;  /* so usertime timeout is used */
		else if (ambkey > 0)
			return MAP_KEY;   /* so keytime timeout is used */

		/* if any complete map, then apply it */
		if (match)
		{
			/* detect animation macros, so we can update the screen
			 * while they run.  Note that we bypass this for key
			 * maps, since an autorepeated arrow key shouldn't make
			 * us update the screen for each line scrolled.
			 */
			if (!match->label && o_optimize)
			{
				if (!match->invoked)
				{
					match->invoked = ElvTrue;
				}
				else
				{
					static long frame;

					if (frame++ >= o_animation)
					{
						frame = 0;
						drawimage(windefault);
						guiflush();
					}
					for (scan=maps; scan; scan = scan->next)
					{
						if (scan != match)
							scan->invoked = ElvFalse;
					}
				}
			}

#ifdef FEATURE_MAPDB
			/* maybe show the the map queue before */
			if ((match->flags & MAP_BREAK) && o_maptrace == 'r')
				o_maptrace = 's';
			trace("map");
#endif

			/* shift the contents of the queue to allow for cooked
			 * strings that are of a different length than rawin.
			 */
			if (match->rawlen < match->cooklen)
			{
				/* insert some room */
				for (i=qty+match->cooklen-match->rawlen, j=qty;
				     j > match->rawlen;
				     )
				{
					queue[--i] = queue[--j];
				}
			}
			else if (match->rawlen > match->cooklen)
			{
				/* delete some keys */
				for (i=match->cooklen, j=match->rawlen; i<qty; )
				{
					queue[i++] = queue[j++];
				}
			}
			qty += match->cooklen - match->rawlen;
			ascmd = (ELVBOOL)((match->flags & MAP_ASCMD) != 0);

			/* copy the cooked string into the queue */
			memcpy(queue, match->cooked, match->cooklen * sizeof(CHAR));

			/* if the "remap" option is off, then the cooked chars
			 * have now been resolved.  Otherwise, if the rhs starts
			 * out the same as the lhs, then the matching portion
			 * is resolved, but not the remainder.
			 */
			if (!o_remap || (match->flags & MAP_NOREMAP) != 0)
			{
				resolved = match->cooklen;
			}
			else if (match->rawlen <= match->cooklen
			    && !memcmp(match->rawin, match->cooked, match->rawlen))
			{
				resolved = match->rawlen;
			}
			/* else resolved remains 0 */

#ifdef FEATURE_MAPDB
			/* if single-stepping, then we're done for now */
			if (tracestep)
			{
				return MAP_CLEAR;
			}
#endif
		}
		else /* no matches of any kind */
		{
			/* first char is resolved: not mapped */
			resolved = 1;
		}
	}
	/*NOTREACHED*/
}


/* This function attempts to place one or more characters back into the
 * keyboard's typeahead queue.  If this would cause the typeahead queue
 * to overflow, then this function has no effect.
 */
void mapunget(keys, nkeys, remap)
	CHAR	*keys;	/* keys to stuff into the type-ahead buffer */
	int	nkeys;	/* number of keys */
	ELVBOOL	remap;	/* are the ungotten keys subject to key maps? */
{
	int	i;

	/* if this would cause overflow, then do nothing */
	if (nkeys + qty > QTY(queue))
	{
		return;
	}

#ifdef FEATURE_MAPDB
	/* maybe show trace */
	trace("ung");
#endif

	/* shift old characters to make room for new characters */
	if (qty > 0)
	{
		for (i = qty; --i >= 0; )
		{
			queue[i + nkeys] = queue[i];
		}
	}

	/* copy the new characters into the queue */
	for (i = 0; i < nkeys; i++)
	{
		queue[i] = keys[i];
	}
	qty += nkeys;

	/* Should the new characters be subjected to mapping? */
	if (!remap)
	{
		/* no -- assume they're resolved */
		resolved += nkeys;
	}
	else
	{
		/* yes -- assume they're unresolved, along with any following
		 * characters.
		 */
		resolved = 0;
	}
}


/* This function is used for listing the contents of the map table in a
 * human-readable format.  Each call returns a single line of text in a
 * static CHAR array, or NULL after the last line has been output.  After
 * calling this function once, you *MUST* call it repeatedly until it
 * returns NULL.  No other map functions should be called during this time.
 */
CHAR *maplist(flags, mode, reflen)
	MAPFLAGS flags;	 /* which maps to output */
	CHAR	 *mode;	 /* name of mode to list, or NULL for any */
	int	 *reflen;/* where to store length, or NULL if don't care */
{
	static MAP *m;	/* used for scanning map list */
	static CHAR buf[200];
	CHAR	*scan, *build;
	int	i;

	/* find first/next map item */
	m = (m ? m->next : (flags & MAP_ABBR) ? abbrs : maps);
	flags &= ~MAP_ABBR;
	while (m && ((m->flags & flags) == 0 || (mode && (!m->mode || CHARcmp(mode, m->mode)))))
	{
		m = m->next;
	}

	/* if no more items, return NULL */
	if (!m)
	{
		return (CHAR *)0;
	}

	memset(buf, ' ', sizeof buf);

	/* if the map has a key label, add it */
	if (m->label)
	{
		i = CHARlen(m->label);
		CHARncpy(buf, m->label, (size_t)(i>9 ? 9 : i));
	}

	/* if no specific mode was requested, and this map has a mode, then
	 * show the mode
	 */
	build = &buf[10];
	if (!mode && m->mode)
	{
		CHARcpy(build, "mode=");
		build += 5;
		CHARcpy(build, m->mode);
		build += CHARlen(build);
		*build++ = ' ';
	}

	/* add "when" flags, unless identical to requested flags */
	if ((m->flags & flags & MAP_WHEN) != (flags & MAP_WHEN))
	{
		*--build = '\0';
		if (m->flags & MAP_NOSAVE)
			CHARcat(build, toCHAR(" nosave"));
		if (m->flags & MAP_INPUT)
			CHARcat(build, toCHAR(" input"));
		if (m->flags & MAP_HISTORY)
			CHARcat(build, toCHAR(" history"));
		if (m->flags & MAP_COMMAND)
			CHARcat(build, toCHAR(" command"));
		if (m->flags & MAP_MOTION)
			CHARcat(build, toCHAR(" motion"));
		if (m->flags & MAP_SELECT)
			CHARcat(build, toCHAR(" select"));
		build += CHARlen(build);
		*build++ = ' ';
	}

	/* add the lhs of the map */
	for (scan = m->rawin, i = m->rawlen;
	     i > 0 && build < &buf[QTY(buf)-4];
	     i--, scan++)
	{
		if (*scan < ' ' || *scan == '\177')
		{
			*build++ = '^';
			*build++ = ELVCTRL(*scan);
		}
		else
		{
			*build++ = *scan;
		}
	}
	do
	{
		*build++ = ' ';
	} while (build < &buf[20]);

	/* add "visual noremap" before rhs, if necessary */
	if ((m->flags & MAP_ASCMD) != 0 && (flags & (MAP_INPUT|MAP_HISTORY)) != 0)
	{
		CHARncpy(build, toCHAR("visual "), 7);
		build += 7;
	}
	if (m->flags & MAP_NOREMAP)
	{
		CHARncpy(build, toCHAR("noremap "), 8);
		build += 8;
	}

	/* Add the rhs */
	for (scan = m->cooked, i = m->cooklen;
	     i > 0 && build < &buf[QTY(buf)-3];
	     i--, scan++)
	{
		if (*scan < ' ' || *scan == '\177')
		{
			*build++ = '^';
			*build++ = ELVCTRL(*scan);
		}
		else
		{
			*build++ = *scan;
		}
	}
	*build++ = '\n';
	*build = '\0';

	/* return the line */
	if (reflen)
	{
		*reflen = (long)(build - buf);
	}
	return buf;
}


/* This function causes future keystrokes to be stored in a cut buffer */
RESULT maplearn(cbname, start)
	_CHAR_	cbname;
	ELVBOOL	start;
{
	long	bit;
	MARKBUF	tmp, end;
	BUFFER	buf;
	CHAR	cmd;
	
	/* reject if not a letter */
	if (!((cbname >= 'a' && cbname <= 'z')
		|| (cbname >= 'A' && cbname <= 'Z')))
	{
		return RESULT_ERROR;
	}
	
	/* Set/reset the "learn" bit for the named cut buffer.  Note that we
	 * return RESULT_COMPLETE if you stop recording keystrokes on a buffer
	 * that wasn't recording to begin with.
	 */
	bit = (1 << (cbname & 0x1f));
	if (start)
		learning |= bit;
	else if (!(learning & bit))
		return RESULT_COMPLETE;
	else
		learning ^= bit;
	
	/* If we're starting and the cut buffer name is lowercase,
	 * then we need to reset the cut buffer to 0 characters
	 */
	if (start && elvlower(cbname))
	{
		/* reset the cut buffer to zero characters */
		cutyank(cbname, marktmp(tmp, bufdefault, 0), &tmp, 'c', 'y');
	}

	/* If we're ending and the last two characters were "]a" or "@a" (or
	 * whatever the buffer name was) then delete the last two characters.
	 */
	if (!start)
	{
		buf = cutbuffer(cbname, ElvFalse);
		if (!buf)
			return RESULT_ERROR;
		if (o_bufchars(buf) >= 2L
		 && scanchar(marktmp(tmp, buf, o_bufchars(buf) - 1)) == cbname
		 && ((cmd = scanchar(marktmp(tmp, buf, o_bufchars(buf) - 2))) == ']'
			|| cmd == '@'))
		{
			bufreplace(marktmp(tmp, buf, o_bufchars(buf) - 2),
				   marktmp(end, buf, o_bufchars(buf)), NULL, 0);
		}
	}

	return RESULT_COMPLETE;
}


/* This function returns a character indicating the current learn state.
 * This will be ',' if no cut buffers are in learn mode, or the name of
 * the first buffer which is in learn mode.
 */
CHAR maplrnchar(dflt)
	_CHAR_	dflt;
{
	CHAR	cbname;

	if (!learning)
		return dflt;
	for (cbname = 'a'; (learning & (1 << (cbname & 0x1f))) == 0; cbname++)
	{
		assert(cbname < 'z');
	}
	return cbname;
}


/* Return TRUE if we're currently executing a map, or FALSE otherwise */
ELVBOOL mapbusy()
{
	return (ELVBOOL)(qty > 0);
}

/* This function implements a POSIX "terminal alert."  This involves discarding
 * any pending keytrokes, and aborting @ macros and maps.  And then the GUI's
 * bell must be rung.
 */
void mapalert()
{
#ifdef FEATURE_MAPDB
	/* maybe display log info */
	if (!tracereal) trace(":::");
#endif

	/* cancel all pending key states, etc. */
	qty = resolved = learning = 0;
}


CHAR *mapabbr(bkwd, oldptr, newptr, exline)
	CHAR	*bkwd;	/* possible abbreviation, BACKWARDS */
	long	*oldptr;/* where to store the length of short form */
	long	*newptr;/* where to store the length of long form */
	ELVBOOL	exline;	/* inputting an ex command line? (else normal input) */
{
	MAP	*m;	/* used for scanning the abbr list */
	MAP	*match;	/* longest matching abbreviation */
	int	i, j;

	/* compare against all abbreviations */
	match = NULL;
	for (m = abbrs; m; m = m->next)
	{
		/* Skip this abbr if it is for the wrong context */
		if ((m->flags & MAP_INPUT) == (unsigned)(exline ? MAP_INPUT : 0))
		{
			continue;
		}

		/* Compare each character.  This is a little tricky since the
		 * input word is backwards.
		 */
		for (i = 0, j = m->rawlen - 1;
		     j >= 0 && bkwd[i] == m->rawin[j]; i++, j--)
		{
		}
		
		/* If all characters matched, and the preceding character in the
		 * raw text wasn't alphanumeric, then we have a match.  If this
		 * match is longer than any previous match, then remember it.
		 */
		if (j < 0
		 && !elvalnum(bkwd[i])
		 && (!match || match->rawlen < i))
		{
			match = m;
		}
	}

	/* If we found a match, return it. */
	if (match)
	{
		*oldptr = match->rawlen;
		*newptr = match->cooklen;
		return match->cooked;
	}
	return NULL;
}

#ifdef FEATURE_MKEXRC
/* This function is used for saving the current map table as a series of
 * :map commands.  It is used by the :mkexrc command.
 */
void mapsave(buf)
	BUFFER	buf;	/* the buffer to append to */
{
	MARKBUF	append;	/* where to put the command */
	static MAP *m;	/* used for scanning map list */
	static CHAR text[200];
	long	len;
	CHAR	*scan;
	int	i;

	(void)marktmp(append, buf, o_bufchars(buf));

	/* for each map... */
	for (m = maps; m; m = m->next)
	{
		/* if for a GUI-specific key, ignore it */
		if (m->label && *m->label != '#')
		{
			continue;
		}

		/* if specifically marked as "nosave", ignore it */
		if (m->flags & MAP_NOSAVE)
		{
			continue;
		}

		/* construct a "map" command */
		CHARcpy(text, toCHAR("map"));
		len = 3;
		switch (m->flags & MAP_WHEN)
		{
		  case MAP_INPUT|MAP_HISTORY:
			text[len++] = '!';
			break;

		  case MAP_COMMAND|MAP_MOTION|MAP_SELECT:
			break;

		  default:
			if (m->flags & MAP_INPUT)
				CHARcat(text, toCHAR(" input"));
			if (m->flags & MAP_HISTORY)
				CHARcat(text, toCHAR(" history"));
			if (m->flags & MAP_COMMAND)
				CHARcat(text, toCHAR(" command"));
			if (m->flags & MAP_MOTION)
				CHARcat(text, toCHAR(" motion"));
			if (m->flags & MAP_SELECT)
				CHARcat(text, toCHAR(" select"));
			len = CHARlen(text);
		}
		text[len++] = ' ';

		/* append a "mode=..." flag if necessary */
		if (m->mode)
		{
			CHARcpy(&text[len], "mode=");
			len += 5;
			CHARcpy(&text[len], m->mode);
			len += CHARlen(m->mode);
			text[len++] = ' ';
		}

		/* Append the raw code.  Use function label if possible */
		if (m->label)
		{
			/* use the label */
			CHARcpy(&text[len], m->label);
			len += CHARlen(m->label);
		}
		else
		{
			/* Use the raw text.  However, protect against the
			 * case where the raw text looks like a flag.
			 */

			/* Does raw text look like a flag? (lowercase letters
			 * followed by whitespace or NUL)
			 */
			for (scan = m->rawin; elvlower(*scan); scan++)
			{
			}
			if ((!*scan || elvspace(*scan)) && (int)(scan - m->rawin) > 2)
			{
				/* Yes it could be a flag name.  (Even if this
				 * version of elvis doesn't use that particular
				 * text as a flag name, some later version
				 * might.)  Add a ^V before the raw text.
				 */
				text[len++] = ELVCTRL('V');
			}

			/* Add the raw text */
			for (scan = m->rawin, i = m->rawlen;
			     i > 0 && len < QTY(text) - 4;
			     i--, scan++)
			{
				CHAR	*label, *raw;
				int	rawlen;

				/* try to convert to <key> notation */
				label = NULL;
				if ((rawlen = guikeylabel(scan, 1, &label, &raw)) == 1
				 && *raw == *scan
				 && label)
				{
					CHARcpy(&text[len], label);
					len += CHARlen(label);
				}
				else if (elvcntrl(*scan))
				{
					text[len++] = '<';
					text[len++] = 'C';
					text[len++] = '-';
					text[len++] = (*scan & 0x1f) + '@';
					text[len++] = '>';
				}
				else
					text[len++] = *scan;
			}
		}

		/* add behavior tweak flags */
		if (m->flags & MAP_ASCMD)
		{
			CHARncpy(&text[len], toCHAR(" visual"), 7);
			len += 7;
		}
		if (m->flags & MAP_NOREMAP)
		{
			CHARncpy(&text[len], toCHAR(" noremap"), 8);
			len += 8;
		}
		text[len++] = ' ';

		/* Does cooked text look like a flag? (lowercase letters
		 * followed by whitespace or NUL)
		 */
		for (scan = m->cooked; elvlower(*scan); scan++)
		{
		}
		if ((!*scan || elvspace(*scan)) && (int)(scan - m->cooked) > 2)
		{
			/* Yes it could be a flag name.  (Even if this
			 * version of elvis doesn't use that particular
			 * text as a flag name, some later version
			 * might.)  Add a ^V before the raw text.
			 */
			text[len++] = ELVCTRL('V');
		}

		/* construct the "cooked" string */
		for (scan = m->cooked, i = m->cooklen;
		     i > 0 && len < QTY(text) - 3;
		     i--, scan++)
		{
			CHAR	*label, *raw;
			int	rawlen;

			/* try to convert to <key> notation */
			label = NULL;
			if ((rawlen = guikeylabel(scan, 1, &label, &raw)) == 1
			 && *raw == *scan
			 && label)
			{
				CHARcpy(&text[len], label);
				len += CHARlen(label);
			}
			else if (elvcntrl(*scan))
			{
				text[len++] = '<';
				text[len++] = 'C';
				text[len++] = '-';
				text[len++] = (*scan & 0x1f) + '@';
				text[len++] = '>';
			}
			else
				text[len++] = *scan;
		}
		text[len++] = '\n';

		/* append the command to the buffer */
		bufreplace(&append, &append, text, len);
		markaddoffset(&append, len);
	}

	/* for each abbreviation... */
	for (m = abbrs; m; m = m->next)
	{
		/* construct an "ab" command */
		CHARcpy(text, toCHAR("ab"));
		len = 2;
		if (m->flags & MAP_HISTORY)
			text[len++] = '!';
		text[len++] = ' ';

		/* append the abbreviated word */
		for (scan = m->rawin, i = m->rawlen;
		     i > 0 && len < QTY(text) - 4;
		     i--, scan++)
		{
			text[len++] = *scan;
		}
		text[len++] = ' ';

		/* Append the expanded string */
		for (scan = m->cooked, i = m->cooklen;
		     i > 0 && len < QTY(text) - 3;
		     i--, scan++)
		{
			if (*scan == '|' || *scan == ELVCTRL('V') || *scan == '\033')
			{
				text[len++] = ELVCTRL('V');
			}
			text[len++] = *scan;
		}
		text[len++] = '\n';

		/* append the command to the buffer */
		bufreplace(&append, &append, text, len);
		markaddoffset(&append, len);
	}
}
#endif /* def FEATURE_MKEXRC */
