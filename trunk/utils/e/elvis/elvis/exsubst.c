/* exsubst.c */
/* Copyright 1995 by Steve Kirkendall */


/* This file contains code which implements the :s command.  The most
 * challenging variation of that command is the interactive form -- specified
 * via the 'c' flag.  Because elvis is event driven, the only way to implement
 * this is via a whole new edit mode which interprets <y> as a command to
 * replace-and-search, and most other keystrokes as a search command (without
 * replacing).  The :s command itself will simply set this up, push the
 * input state onto the stack, and return.
 *
 * On the other hand, non-interactive versions need to return a "failed"
 * exit status if there is no match, so we can't simply run the interactive
 * code with simulated <y> keystrokes.  We need to have a separate version of
 * the search-and-replace loop.
 *
 * In this file, we have a "findnext" function and a "dosubst" function.
 * These are used by the non-interactive :s command, and the interactive
 * version's key processor -- both of which are also in this file.
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_exsubst[] = "$Id: exsubst.c,v 2.22 2003/10/17 17:41:23 steve Exp $";
#endif

typedef struct
{
    /* THE FOLLOWING ARE RETAINED BETWEEN SEARCHES */
	/* old text and new text */
	regexp	*re;	/* text to search for */
	CHAR	*rplc;	/* replacement text */

	/* options */
 	PFLAG	pflag;	 /* printing flag */
	ELVBOOL	global;	 /* boolean: substitute globally in line? */
	ELVBOOL	execute; /* boolean: execute instead of substitute? */
	ELVBOOL	confirm; /* boolean: ask for confirmation first? */
	ELVBOOL errors;	 /* boolean: is a failed search an error? */
	long	instance;/* numeric: which instance in each line to sub */

    /* THE FOLLOWING ARE RESET FOR EACH SEARCH */
	/* range of lines to check */
	WINDOW	win;	/* window */
	MARK	from;	/* current line */
	MARK	to;	/* end of last line to check */

	/* intermediate results */
	long	thisinst;/* instance of current match within current line */
	CHAR	key;	/* keystroke */
	long	printoff;/* offset of line to be printed, or -1 */

	/* results */
	long	chline;	/* # of lines changed */
	long	chsub;	/* # of substitutions made */
	long	cursoff;/* offset of last change, so cursor can be left there */
	ELVBOOL	ex;	/* revert to ex mode afterward? */
} subst_t;

static void print P_((subst_t *subst));
static ELVBOOL findnext P_((subst_t *subst));
static ELVBOOL dosubst P_((WINDOW win, subst_t *subst));
static RESULT parse P_((_CHAR_ key, void *info));
static RESULT perform P_((WINDOW win));
static ELVCURSOR shape P_((WINDOW win));


/* Print the line containing the most recent change.  This is used when the
 * :s command is invoked with print flags.
 */
static void print(subst)
	subst_t	*subst;
{
	MARK	pline;
	long	fromoff;

	/* If supposed to print, then do so */
	if (subst->pflag == PF_NONE)
		return;

	/* Find the start of the line to be printed.  If 'printoff' is >= 0
	 * then use it as the the starting point; else use the from field.
	 */
	if (subst->printoff >= 0)
	{
		fromoff = markoffset(subst->from);
		marksetoffset(subst->from, subst->printoff);
		pline = (*subst->win->md->move)(subst->win,
					subst->from, 0L, 0L, ElvFalse);
		marksetoffset(subst->from, fromoff);
		subst->printoff = -1L;
	}
	else
	{
		pline = (*subst->win->md->move)(subst->win,
					subst->from, 0L, 0L, ElvFalse);
	}

	/* print the line */
	exprintlines(subst->win, pline, 1L, subst->pflag);
}

/* Find the next matching instance for a given substitution.  If one is found,
 * return ElvTrue; else return ElvFalse.  When a match is found, the contents of
 * the subst argument are modified to reflect its location.  In particular,
 * subst->re->startp[0] gives its start offset, and subst->re->end[0] gives
 * its end offset.
 */
static ELVBOOL findnext(subst)
	subst_t	*subst;	/* info about the substitution */
{
	/* if previous line was supposed to be printed but hasn't been,
	 * then print it now.
	 */
	if (subst->printoff >= 0)
		print(subst);

	/* for each line remaining in the range... */
	while (markoffset(subst->from) < markoffset(subst->to))
	{
		/* if the user is impatient, then stop */
		if (guipoll(ElvFalse))
			return ElvFalse;

		/* for each instance within the line... */
		while (regexec(subst->re, subst->from, (ELVBOOL)(subst->thisinst == 0))
		    && (subst->global || subst->thisinst < subst->instance))
		{
			/* increment the instance counter */
			subst->thisinst++;

			/* if this is an instance we care about... */
			if (subst->global || subst->thisinst == subst->instance)
			{
				/* prepare for next loop */
				if (subst->global)
				{
					marksetoffset(subst->from,
							subst->re->endp[0]);

					/* For global substitutions, we need to
					 * be careful about regexps which can
					 * match zero-length text. Specifically,
					 * we need to require at least one
					 * non-matching character between
					 * matches, otherwise we'd just get in
					 * an infinite loop replacing nothing
					 * with something at the same location.
					 */
					if (subst->re->minlen == 0)
					{
						if (scanchar(subst->from) == '\n')
							subst->thisinst = 0;
						markaddoffset(subst->from, 1L);
					}
				}
				else /* single instance in each line */
				{
					/* remember that we'll need to print
					 * this line after the substitution.
					 */
					subst->printoff = markoffset(subst->from);
					/* Increment the chline counter */
					subst->chline++;

					/* move to start of the next line */
					marksetoffset(subst->from,
							subst->re->nextlinep);
					subst->thisinst = 0;
				}


				/* Return ElvTrue to do substitution */
				return ElvTrue;
			}

			/* Move "posn" to the end of the matched region.  If
			 * the regexp could conceivably match a zero-length
			 * string, then skip one character.
			 */
			marksetoffset(subst->from, subst->re->endp[0]);
			if (subst->re->minlen == 0)
			{
				markaddoffset(subst->from, 1);/*!!!*/
				if (scanchar(subst->from) == '\n')
					break;
			}
		}

		/* if any matches were found, then increment chline */
		if (subst->global ? subst->thisinst > 0 : subst->thisinst == subst->instance)
		{
			subst->chline++;

			/* print the changed line, if we're supposed to */
			print(subst);
		}

		/* move forward to the start of the next line */
		marksetoffset(subst->from, subst->re->nextlinep);
		subst->thisinst = 0L;
	}

	/* if last line was supposed to be printed but hasn't been,
	 * then print it now.
	 */
	if (subst->printoff >= 0)
		print(subst);

	return ElvFalse;
}


/* Perform a substitution, or if subst->execute is ElvTrue then execute the
 * text which would have been substituted.  Returns ElvTrue if successful, or
 * ElvFalse if there is an error in the replacement text.
 */
static ELVBOOL dosubst(win, subst)
	WINDOW	win;
	subst_t	*subst;	/* info about the substitution */
{
	CHAR	*newtext;
	MARK	oldcursor;
	MARK	cursor;
	ELVBOOL	oldsaveregexp;

	/* remember the offset of this change so we can move the cursor later */
	subst->cursoff = subst->re->startp[0];

	/* Either execute the replacement, or perform the substitution */
	newtext = regsub(subst->re, subst->rplc, (ELVBOOL)!subst->execute);
	if (!newtext)
	{
		return ElvFalse;
	}
	if (subst->execute)
	{
		/* move the window's cursor to the matching line */
		if (win)
		{
			assert(subst->re->buffer == markbuffer(subst->from));
			if (win->state->pop)
				cursor = win->state->pop->cursor;
			else
				cursor = win->cursor;
			oldcursor = markdup(cursor);
			marksetoffset(cursor, subst->re->startp[0]);
			marksetbuffer(cursor, subst->re->buffer);
			bufoptions(subst->re->buffer);
		}
		else
			cursor = oldcursor = NULL;

		/* temporarily turn off the saveregexp option */
		oldsaveregexp = o_saveregexp;
#if 0
		o_saveregexp = ElvFalse;
#endif

		/* execute the command */
		exstring(win, newtext, NULL);

		/* restore the saveregexp option */
		o_saveregexp = oldsaveregexp;

		/* move the cursor back where it was before */
		if (oldcursor)
		{
			marksetoffset(cursor, markoffset(oldcursor));
			marksetbuffer(cursor, markbuffer(oldcursor));
			markfree(oldcursor);
		}
	}
	safefree(newtext);

	/* increment the substitution change counter */
	subst->chsub++;

	return ElvTrue;
}


/* If key is <Esc> then cancel the change.  If <n> or <N> then skip this change
 * but continue to the next match.  Any other key performs the substitution and
 * continues to the next match.
 */
static RESULT parse(key, info)
	_CHAR_	key;
	void	*info;
{
	subst_t	*subst = (subst_t *)info;
	WINDOW	win;
#ifdef FEATURE_V
	VIINFO	vinf;

	/* unhighlight the previous match */
	vinf.command = ELVCTRL('[');
	v_visible(subst->win, &vinf);
#endif

	/* be lenient about keystrokes */
	if (key == 'N' || key == 'n')
		key = 'n';
	else if (key == ELVCTRL('[') || key == ELVCTRL('C'))
		key = ELVCTRL('[');
	else
		key = 'y';

	/* do the required steps */
	switch (key)
	{
	  case 'y':
		/* perform this substitution */
		if (!dosubst(subst->win, subst))
		{
			markbuffer(subst->win->cursor)->willdo = ElvFalse;
			return RESULT_ERROR;
		}
		/* fall through... */

	  case 'n':
		/* find the next match */
		if (findnext(subst))
		{
#ifdef FEATURE_V
			/* highlight the next match */
			if (subst->re->endp[0] > subst->re->startp[0])
			{
				marksetoffset(subst->win->cursor, subst->re->endp[0] - 1);
				vinf.command = 'v';
				v_visible(subst->win, &vinf);
				marksetoffset(subst->win->cursor, subst->re->startp[0]);
				v_visible(subst->win, NULL);
			}
			else /* matching text is 0 characters long */
#endif /* FEATURE_V */
			{
				/* at least move the cursor there */
				marksetoffset(subst->win->cursor, subst->re->startp[0]);
			}

			return RESULT_MORE;
		}
	}

	/* Either the user canceled or there are no other matches, so we're
	 * done.  Announce the results.
	 */
	if ((subst->chline >= o_report && o_report != 0) || subst->confirm)
	{
		msg(MSG_INFO, "[dd]$1 substitutions on $2 lines", subst->chsub, subst->chline);
	}

	/* Free the marks. */
	markfree(subst->from);
	markfree(subst->to);
	safefree(subst->rplc);
#ifdef FEATURE_AUTOCMD
	markbuffer(subst->win->cursor)->eachedit = ElvFalse;
#endif

	/* Either pop the confirm state (if we started it from visual mode)
	 * or replace the confirm state with an ex state (if we started from
	 * ex mode).
	 */
	if (subst->ex)
	{
		/* Pop off the "confirm" state right away, and them push an
		 * "ex" stratum.
		 */
		win = subst->win;
		statepop(win);
		statestratum(win, toCHAR(EX_BUF), ':', exenter);
		win->state->flags &= ~(ELVIS_POP|ELVIS_ONCE|ELVIS_1LINE);
	}
	else
	{
		/* Pop off the "confirm" state so we return to the "vi" state */
		subst->win->state->flags |= ELVIS_POP;
	}
	return RESULT_COMPLETE;
}

static RESULT perform(win)
	WINDOW	win;
{
	return RESULT_COMPLETE;
}

/* Choose a cursor shape to use during confirmation */
static ELVCURSOR shape(win)
	WINDOW	win;
{
	regexp	*re = ((subst_t *)win->state->info)->re;
	return re->startp[0] == re->endp[0] ? CURSOR_INSERT : CURSOR_REPLACE;
}

/* This function implements the :substitute command, and the :& and :~
 * variations of that command.  It is also used to perform the real work
 * of the visual <&> command.
 */
RESULT	ex_substitute(xinf)
	EXINFO	*xinf;
{
 static subst_t	prev;	/* previous substitution info */
	subst_t	subst;	/* current substitution info */
	CHAR	*opt;	/* substitution options */
	long	count;	/* numeric option: which instance in each line to sub */
	BUFFER	buf;
	STATE	*state, *s;
#ifdef FEATURE_V
	VIINFO	vinf;
#endif


	assert(xinf->command == EX_SUBSTITUTE
		|| xinf->command == EX_SUBAGAIN
		|| xinf->command == EX_SUBRECENT);

	/* if invoked via visual <&> command, or not edcompatible, then reset */
	subst = prev;
	memset(&prev, 0, sizeof prev);
	if (!o_edcompatible || xinf->bang)
	{
		subst.pflag = xinf->pflag;
		subst.global = ElvFalse;
		subst.execute = ElvFalse;
		subst.confirm = ElvFalse;
		subst.errors = ElvTrue;
		subst.instance = 0;
	}
	else if (xinf->pflag != PF_NONE)
		subst.pflag = xinf->pflag;

	/* parse arguments */
	count = 0;
	if (xinf->command == EX_SUBAGAIN
	 || (xinf->command == EX_SUBSTITUTE && !xinf->re))
	{
		/* use the same regexp as the previous :s command */
		if (!subst.re)
		{
			msg(MSG_ERROR, "no previous regular expression");
			return RESULT_ERROR;
		}

		/* same replacement text as last time */
		subst.rplc = regtilde(toCHAR(o_magic ? "~" : "\\~"));
	}
	else if (xinf->command == EX_SUBRECENT)
	{
		/* use the most recent regular expression */
		if (subst.re)
			safefree(subst.re);
		prev.re = NULL;
		subst.re = regcomp(toCHAR(""), xinf->window->state->cursor);
		if (!subst.re)
		{
			/* error message already given by regcomp() */
			return RESULT_ERROR;
		}

		/* same replacement text as last time */
		subst.rplc = regtilde(toCHAR(o_magic ? "~" : "\\~"));
	}
	else /* xinf->command == CMD_SUBSTITUTE */
	{
		/* use the new regexp */
		if (subst.re)
			safefree(subst.re);
		prev.re = NULL;
		subst.re = xinf->re;
		xinf->re = NULL; /* so it isn't clobbered after this cmd */

		/* generate the new text */
		subst.rplc = regtilde(xinf->lhs ? xinf->lhs : toCHAR(""));
	}

	/* analyse the option string */
	for (opt = xinf->rhs; opt && *opt; opt++)
	{
		switch (*opt)
		{
		  case 'g':
			subst.global = (ELVBOOL)!subst.global;
			break;

		  case 'x':
			subst.execute = (ELVBOOL)!subst.execute;
			break;

		  case 'c':
			subst.confirm = (ELVBOOL)!subst.confirm;
			break;

		  case 'e':
			subst.errors = (ELVBOOL)!subst.errors;
			break;

		  case 'p':
			subst.pflag = (subst.pflag==PF_PRINT) ? PF_NONE : PF_PRINT;
			break;

		  case 'l':
			subst.pflag = (subst.pflag==PF_LIST) ? PF_NONE : PF_LIST;
			break;

		  case '#':
			subst.pflag = (subst.pflag==PF_NUMBER) ? PF_NONE : PF_NUMBER;
			break;

		  case '.':
			subst.instance = 0;
			opt++;
			do
			{
				subst.instance *= 10;
				subst.instance += *opt++ - '0';
			} while (elvdigit(*opt));
			opt--;
			break;

		  case '0':
		  case '1':
		  case '2':
		  case '3':
		  case '4':
		  case '5':
		  case '6':
		  case '7':
		  case '8':
		  case '9':
			count = 0;
			do
			{
				count = count * 10 + *opt++ - '0';
			} while (elvdigit(*opt));
			opt--;
			break;

		  default:
			if (!elvspace(*opt))
			{
				msg(MSG_ERROR, "[C]unsupported flag '$1'", *opt);
				return RESULT_ERROR;
			}
		}
	}

	/* sanity checks */
	if (subst.instance != 0 && subst.global)
	{
		msg(MSG_ERROR, "[d]can't mix instance number .$1 and g flag", subst.instance);
		return RESULT_ERROR;
	}
	buf = markbuffer(xinf->fromaddr);
	if (subst.confirm && buf != markbuffer(xinf->window->cursor))
	{
		msg(MSG_ERROR, "the c flag only works on the window's main edit buffer");
		return RESULT_ERROR;
	}
	if (subst.confirm && !xinf->window)
	{
		msg(MSG_ERROR, "the c flag is only available from a window");
		return RESULT_ERROR;
	}

	/* can't mix 'c' flag with a print flag - and don't really need to */
	if (subst.confirm)
		subst.pflag = PF_NONE;

	/* If count>0 then adjust the "xinf->to" mark */
	if (count > 0)
	{
		xinf->to = xinf->from + count - 1L;
		if (xinf->to == o_buflines(markbuffer(&xinf->defaddr)))
			marksetoffset(xinf->toaddr,
				o_bufchars(markbuffer(&xinf->defaddr)));
		else
			marksetoffset(xinf->toaddr,
				lowline(bufbufinfo(markbuffer(xinf->toaddr)), xinf->to + 1));
	}

	/* default behavior is to either replace only first instance,
	 * or all instances, depending on the "gdefault" option.
	 */
	if (subst.instance == 0 && !subst.global)
	{
		if (o_gdefault && !o_edcompatible)
			subst.global = ElvTrue;
		else
			subst.instance = 1;
	}

	/* remember the flags, unless "saveregexp" is off */
	if (o_saveregexp)
	{
		prev = subst;
		prev.re = regdup(prev.re);
	}

	/* if no replacement text, fail */
	if (!subst.rplc)
	{
		return RESULT_ERROR;
	}

	/* this command does its own printing; disable auto printing */
	xinf->pflag = PF_NONE;

	/* reset the change counters */
	subst.chline = subst.chsub = 0L;

	/* make a local copy of replacement string */
	subst.rplc = CHARdup(subst.rplc);

	/* make the scanned buffer be the one used by this window */
	bufoptions(buf);

#ifdef FEATURE_AUTOCMD
	/* we want to trigger Edit events for change, even though we only save
	 * an undo version before the first change.
	 */
	buf->eachedit = ElvTrue;
#endif

	/* for each line in the range... */
	subst.from = markdup(xinf->fromaddr);
	subst.to = markdup(xinf->toaddr);
	subst.thisinst = 0;
	subst.printoff = -1;
	subst.win = xinf->window;
	if (subst.confirm)
	{
		if (findnext(&subst))
		{
			/* insert a state between "ex" and "vi" input
			 * states.  The new state should be in the same
			 * stratum as "vi".  Since there is no state
			 * function for inserting states this way, we
			 * do it by temporarily removing the "ex" state,
			 * pushing the new one, and then putting "ex"
			 * back again.
			 */
			state = subst.win->state;
			subst.win->state = subst.win->state->pop;
			statepush(subst.win, subst.win->state->flags & ~ELVIS_ONCE);
			if (state->acton == subst.win->state->pop)
				state->acton = subst.win->state;
			state->pop = subst.win->state;
			subst.win->state = state;

			/* Use the same cursor, top & bottom marks as
			 * higher "vi" state.   Move them to the match.
			 */
			state = subst.win->state->pop;
			marksetoffset(state->cursor, subst.re->startp[0]);
			marksetoffset(state->top, subst.re->startp[0]);
			marksetoffset(state->bottom, subst.re->endp[0]);

			/* If ex mode (not visual <:> command), then we need
			 * to be clever here: We need to exit ex mode while
			 * the substitution is taking place, and then return
			 * to it again afterward.
			 */
			if ((subst.win->state->flags & (ELVIS_1LINE|ELVIS_ONCE|ELVIS_POP)) == 0)
			{
				/* exit "ex" mode */
				for (s = xinf->window->state;
				     s != xinf->window->state->acton;
				     s = s->pop)
					s->flags |= ELVIS_1LINE;

				/* force main edit state use use bottom line */
				for (; s; s = s->pop)
					s->flags |= ELVIS_BOTTOM;

				/* remember to switch back later */
				subst.ex = ElvTrue;
			}

			/* initialize the remaining fields of the state */
			state->enter = NULL;
			state->perform = perform;
			state->parse = parse;
			state->shape = shape;
			state->info = safealloc(1, sizeof(subst_t));
			*((subst_t *)state->info) = subst;
			state->modename = "Yes/No";

#ifdef FEATURE_V
			/* highlight the first match */
			if (subst.re->endp[0] > subst.re->startp[0])
			{
				marksetoffset(subst.win->cursor, subst.re->endp[0] - 1);
				vinf.command = 'v';
				v_visible(subst.win, &vinf);
				marksetoffset(subst.win->cursor, subst.re->startp[0]);
				v_visible(subst.win, NULL);
			}
			else /* matching text is 0 characters long */
#endif
			{
				/* at least move the cursor there */
				marksetoffset(subst.win->cursor, subst.re->startp[0]);
			}
			marksetoffset(subst.win->state->top, markoffset(subst.win->cursor));

			/* Well, we started successfully */
			return RESULT_COMPLETE;
		}

		/* free the marks & stuff -- we failed */
		markfree(subst.from);
		markfree(subst.to);
		safefree(subst.rplc);
#ifdef FEATURE_AUTOCMD
		buf->eachedit = ElvFalse;
#endif
		return RESULT_ERROR;
	}
	else /* non-interactive */
	{
		/* do all the substitutions now */
		while (findnext(&subst))
			if (!dosubst(xinf->window, &subst))
			{
				markfree(subst.from);
				markfree(subst.to);
				safefree(subst.rplc);
#ifdef FEATURE_AUTOCMD
				buf->eachedit = ElvFalse;
#endif
				return RESULT_ERROR;
			}

		/* Free the marks. */
		markfree(subst.from);
		markfree(subst.to);
		safefree(subst.rplc);
	}
#ifdef FEATURE_AUTOCMD
	buf->eachedit = ElvFalse;
#endif

	/* If used with "e", then finish silently and never cause an error */
	if (!subst.errors)
		return RESULT_COMPLETE;

	/* if used with "x" flag", or if ":set report=0" then finish silently */
	if (subst.execute || o_report == 0)
		return subst.chsub > 0 ? RESULT_COMPLETE : RESULT_ERROR;

	/* Reporting */
	if (subst.chsub == 0)
		msg(MSG_WARNING, "substitution failed");
	else if (subst.chline >= o_report)
		msg(MSG_INFO, "[dd]$1 substitutions on $2 lines", subst.chsub, subst.chline);

	/* leave the cursor at the location of the last change */
	if (subst.chsub > 0)
	{
		xinf->newcurs = markalloc(markbuffer(xinf->window->cursor), subst.cursoff);

		/* try to avoid leaving cursor on a newline */
		if (subst.cursoff > 0L && scanchar(xinf->newcurs) == '\n')
		{
			markaddoffset(xinf->newcurs, -1L);
			if (scanchar(xinf->newcurs) == '\n')
				markaddoffset(xinf->newcurs, 1L);
		}
		return RESULT_COMPLETE;
	}
	return RESULT_ERROR;
}
