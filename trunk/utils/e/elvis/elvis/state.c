/* state.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_state[] = "$Id: state.c,v 2.43 2003/10/17 17:41:23 steve Exp $";
#endif

#if USE_PROTOTYPES
static void fixbounds(WINDOW win);
#endif


/* This is the window that we're sending the character to.  At the start of
 * the processing for each keystroke, this is set to focus.  The code
 * here always uses focus instead of focus from that point on, because
 * the code here assumes that the window won't switch during the processing
 * of the keystroke, but focus may in fact change.
 */
WINDOW focus;

/* Push a single state, in the current stratum.
 *
 * After this function returns, several fields in the struct will still
 * need to be initialized.  This function is meant to be called only from
 * vipush() and inputpush(); those functions initialize the other fields.
 */
void statepush(win, flags)
	WINDOW		win;	/* window to receive new key state */
	ELVISSTATE	flags;	/* flags of the new state */
{
	STATE	*newp;

	/* allocate a STATE struct */
	newp = (STATE *)safealloc(1, sizeof(STATE));

	/* initialize the struct's values */
	newp->flags = flags;
	if (win->state != NULL)
	{
		newp->flags |= (win->state->flags & ELVIS_BOTTOM);
		newp->cursor = win->state->cursor;
		newp->top = win->state->top;
		newp->bottom = win->state->bottom;
		newp->acton = win->state->acton;
	}
	else if (!gui->moveto)
	{
		/* if GUI has no move() function, we can't support full-screen */
		newp->flags |= ELVIS_BOTTOM;
	}

	/* link this into the stack */
	newp->pop = win->state;
	win->state = newp;
}

/* Pop a single state */
void statepop(win)
	WINDOW	win;	/* window from which the state will be popped */
{
	STATE	*doomed;

	assert(win->state != NULL);

	/* if this is a stratum, then free the marks */
	if (win->state->enter != NULL)
	{
		/* free the marks */
		markfree(win->state->cursor);
		markfree(win->state->top);
		markfree(win->state->bottom);

		/* also restore wantcol */
		if (win->state->acton)
			win->wantcol = win->state->acton->wantcol;
	}

	/* if we were editing a line before, and popping makes us switch
	 * buffers, then we aren't editing that line anymore.
	 */
	if (!win->state->pop || win->state->cursor != win->state->pop->cursor)
	{
		if (win->di->openline)
		{
			markfree(win->di->openline);
			win->di->openline = NULL;
		}
	}

	/* if this has an info struct, then free it now, too */
	if (win->state->info != NULL)
	{
		safefree(win->state->info);
	}

	/* remove the state from the state stack, and free it */
	doomed = win->state;
	win->state = doomed->pop;
	safefree(doomed);
}

/* Push a new stratum.  This involves appending a new blank line to a buffer,
 * possibly adding a the prompt character to that line, and then pushing an
 * open input state onto the state stack.
 */
void statestratum(win, bufname, prompt, enter)
	WINDOW	win;		/* window to receive new stratum */
	CHAR	*bufname;	/* name of buffer to use in new stratum */
	_CHAR_	prompt;		/* prompt character, or '\0' for none */
	RESULT	(*enter)P_((WINDOW window));/* function which executes line */
{
	BUFFER	buf;
	CHAR	newtext[2];
	MARK	mark;
	MARKBUF	tmp;

	/* find the buffer.  If it doesn't exist, then create it */
	buf = bufalloc(bufname, 0, ElvTrue); /*!!! sometimes ElvFalse? */

	/* create a blank line at the end of the buffer, and insert the prompt
	 * character there, if given.
	 */
	mark = markalloc(buf, o_bufchars(buf));
	if (prompt && (prompt != ':' || o_prompt))
	{
		/* if no other window is using this buffer, and the buffer
		 * already ends with a line that looks like our prompt line,
		 * then reuse that line.
		 */
		if (wincount(buf) == 0
		 && o_bufchars(buf) > 2
		 && scanchar(marktmp(tmp, buf, o_bufchars(buf) - 1)) == '\n'
		 && scanchar(marktmp(tmp, buf, o_bufchars(buf) - 2)) == prompt 
		 && scanchar(marktmp(tmp, buf, o_bufchars(buf) - 3)) == '\n')
		{
			markaddoffset(mark, -1);
		}
		else /* add a new prompt line */
		{
			newtext[0] = prompt;
			newtext[1] = '\n';
			bufreplace(mark, mark, newtext, 2);
			marksetoffset(mark, o_bufchars(buf) - 1);
		}
	}
	else
	{
		newtext[0] = '\n';
		bufreplace(mark, mark, newtext, 1);
		marksetoffset(mark, o_bufchars(buf) - 1);
	}

	/* use the prompt as a special key in case we hit a [More] prompt
	 * when switching back to the old key state.
	 */
	win->state->morekey = prompt;

	/* push a new input state */
	inputpush(win, ELVIS_BOTTOM|ELVIS_1LINE, 'i');

	/* initialize the state to look like a new stratum */
	win->state->cursor = mark;
	win->state->top = markdup(mark);
	win->state->bottom = markdup(mark);
	win->state->acton = win->state->pop;
	win->state->enter = enter;
	win->state->prompt = prompt;

	/* For history buffers, we want to use the MAP_HISTORY map context
	 * instead of the normal MAP_INPUT context.  We assume this is a
	 * history buffer if it normally has a prompt.
	 */
	if (prompt)
		win->state->mapflags = MAP_HISTORY;

	/* save the old stratum's wantcol (if there was an old stratum) */
	if (win->state->acton)
		win->state->acton->wantcol = win->wantcol;
}


static void fixbounds(win)
	WINDOW	win;	/* window whose edit bounds need tweaking */
{
	STATE	*state;

	/* Fix the edit bounds.
	 *
	 * Note that we do this for all strata on the stack, not just
	 * the current one.  This is mostly for the benefit of the
	 * visual / and ? commands -- After the search, the current
	 * strata is still the regexp line entry one, but we need to
	 * worry about the edit limits of the main strata.
	 */
	for (state = win->state; state; state = state->acton)
	{
		if (markbuffer(state->top) != markbuffer(state->cursor)
		 || markbuffer(state->top) != markbuffer(state->bottom)
		 || markoffset(state->top) > markoffset(state->cursor)
		 || markoffset(state->cursor) > markoffset(state->bottom))
		{
			marksetbuffer(state->top, markbuffer(state->cursor));
			marksetbuffer(state->bottom, markbuffer(state->cursor));
			if (state->acton == NULL)
			{
				/* in the main edit buffer, the edit bounds are
				 * changed to equal the cursor.
				 */
				marksetoffset(state->top, markoffset(state->cursor));
				marksetoffset(state->bottom, markoffset(state->cursor));
			}
			else
			{
				/* in a history buffer, the edit bounds are set
				 * to the whole line that the cursor is on, and
				 * if the cursor has moved to a different line
				 * then it is moved to the end of that line.
				 */
				marksetoffset(state->top, markoffset((*dmnormal.move)(focus, state->top, 0L, 0L, ElvFalse)));
				marksetoffset(state->bottom, markoffset((*dmnormal.move)(focus, state->top, 0L, INFINITY, ElvFalse)));
				if (markoffset(state->cursor) < markoffset(state->top)
				 || markoffset(state->cursor) > markoffset(state->bottom))
				{
					marksetoffset(state->top, markoffset(dispmove(focus, 0L, 0L)));
					marksetoffset(state->bottom, markoffset(dispmove(focus, 0L, INFINITY)));
					marksetoffset(state->cursor, markoffset(state->bottom));
				}
			}
		}
	}
}


/* This function processes a single keystroke in the context of the default
 * window.
 */
RESULT statekey(key)
	_CHAR_	key;	/* a single key to be parsed */
{
	RESULT	result;
	STATE	*state;
	CHAR	newtext[2];
	int	i, j;

	assert(windefault);

	/* make an unalterable copy of windefault */
	focus = windefault;
	state = focus->state;

#ifdef FEATURE_AUTOCMD
	/* detect when the display mode changes, so we can trigger DispMapLeave
	 * and DispMapEnter autocmds.  These are often used for implementing
	 * display-specific maps.
	 */
	audispmap();
#endif

	/* If user wants to abort operation, then ignore this key.  This is
	 * important to check for, because elvis may be stuck in a recursive
	 * loop.
	 */
	if (guipoll(ElvFalse))
	{
		mapalert();
		return RESULT_ERROR;
	}

	/* if <Enter>, and not quoted, and this is a stratum, then call the
	 * enter() function.  If it returns RESULT_MORE then follow that by
	 * processing <Enter> in the usual way; otherwise we're done.
	 */
	if ((key == '\r' || key == '\n')
	  && (*state->shape)(focus) != CURSOR_QUOTE
	  && state->enter)
	{
		/* adjust the input line */
		inputbeforeenter(focus);

		/* if this line was entered via a one-time command from
		 * visual mode, then force drawstate to be DRAW_VISUAL so
		 * the user isn't forced to hit <enter> unless there really
		 * is some useful text to be read.  EXCEPTION: If this GUI
		 * doesn't do full-screen, then don't bother.
		 */
		if ((state->flags & ELVIS_1LINE) != 0 && gui->moveto != NULL)
		{
			focus->di->drawstate = DRAW_VISUAL;
		}

		/* call the "enter" function for this state, and see whether
		 * the command is complete.
		 */
		result = (*state->enter)(focus);

		/* if input line wasn't ended by execution of the enter()
		 * function, then end it now.
		 */
		if (windefault
		 && ((focus->state->acton->flags & ELVIS_BOTTOM) != 0
		   || (focus->state->flags & (ELVIS_ONCE|ELVIS_1LINE|ELVIS_POP)) == 0))
			drawopencomplete(focus);

		/* check the results */
		if (result != RESULT_MORE)
		{
			/* If the window went away, then no more processing
			 * is necessary.  Note that we're checking windefault
			 * here, not focus!  If windefault has become NULL,
			 * that means the window was destroyed, and focus now
			 * points to freed memory!
			 */
			if (!windefault)
			{
				return RESULT_COMPLETE;
			}

			/* We did one command line.  Is that all we wanted? */
			state = focus->state;
			if (state->flags & ELVIS_1LINE)
			{
				/* yes, pop the stratum */
				state = state->acton;
				while (state != focus->state->pop)
				{
					statepop(focus);
				}
				state = focus->state;
				state->flags |= ELVIS_POP;
			}
			else
			{
				marksetoffset(state->cursor, o_bufchars(markbuffer(state->cursor)));
				if (state->prompt && (state->prompt != ':' || o_prompt))
				{
					newtext[0] = state->prompt;
					newtext[1] = '\n';
					bufreplace(state->cursor, state->cursor, newtext, 2);
				}
				else
				{
					newtext[0] = '\n';
					bufreplace(state->cursor, state->cursor, newtext, 1);
				}
				marksetoffset(state->cursor, o_bufchars(markbuffer(state->cursor)) - 1);
			}

			/* do the usual after-keystroke processing */
			goto AfterKeystroke;
		}
	}

	/* parse the keystroke for the current window */
	if (key != (_CHAR_)-1)
		result = (*state->parse)(key, state->info);
	else
		result = RESULT_COMPLETE;

	/* If error, alert the window */
	if (result == RESULT_ERROR)
	{
		/* clobber the "cmdchars" list - all chars entered */
		focus->cmdchars[0] = '\0';

		/* alert the window */
		mapalert();
		if (o_errorbells)
			guibeep(focus);
	}
	else if (result == RESULT_COMPLETE && key != (_CHAR_)-1 && state == focus->state)
	{
		/* clobber the "cmdchars" list - all chars entered */
		focus->cmdchars[0] = '\0';

		/* We have parsed a complete command.  Now perform it */
		switch ((*state->perform)(focus))
		{
		  case RESULT_ERROR:
			/* command failed!  alert the window */
			mapalert();
			if (o_errorbells)
				guibeep(focus);
			break;

		  case RESULT_MORE:
			/* set the pushed state's ELVIS_MORE flag */
			state->flags |= ELVIS_MORE;
			break;

		  case RESULT_COMPLETE:
			/* nothing, just fall through... */
			;
		}

		/* The command may have caused the window to disappear.
		 * If so, then no more processing is necessary.  Note that
		 * we check windefault, not focus!  If windefault has become
		 * NULL, then the window was destroyed and focus points to
		 * freed memory.
		 */
		if (!windefault)
		{
			return RESULT_COMPLETE;
		}

		/* If cursor has moved outside state->top and state->bottom,
		 * then make state->top and state->bottom equal the cursor.
		 */
		fixbounds(focus);

		/* if the "optimize" option is false, and the current window
		 * is in vi mode, then update the current window's image.
		 */
		if (!o_optimize /* && focus */
		 && !focus->state->pop
		 && focus->di->curchgs != markbuffer(focus->cursor)->changes
		 && (focus->di->drawstate == DRAW_VISUAL
			|| focus->di->drawstate == DRAW_VMSG))
		{
			drawimage(focus);
			if (gui->flush)
				(*gui->flush)();
		}
	}
	else if (result == RESULT_MORE)
	{
		/* partial command -- add this key to the cmdchars field */

		/* if the array is full, then shift */
		i = CHARlen(focus->cmdchars);
		j = (elvcntrl(key) ? 2 : 1);
		if (i + j >= QTY(focus->cmdchars))
		{
			for (i = 0; focus->cmdchars[i]; i++)
			{
				focus->cmdchars[i] = focus->cmdchars[i + j];
			}
			i -= j;
		}

		/* stuff the new char into it */
		switch (o_nonascii)
		{
		  case 's': key &= 0x7f;		break;
		  case 'n': key = '.';			break;
		  case 'm': if (key>0x7f && key<=0x9f)
				key = '.';		break;
		}
		if (elvcntrl(key))
		{
			focus->cmdchars[i++] = '^';
			key ^= 0x40;
		}
		focus->cmdchars[i++] = key;
		focus->cmdchars[i++] = '\0';
	}

#ifdef FEATURE_V
	/* if visibly marking, then adjust the marks */
	if (focus->seltop)
	{
		(void)v_visible(focus, NULL);
	}
#endif

	/* if we aren't still parsing, then perform other checks */
	if (result != RESULT_MORE)
	{
AfterKeystroke:
		/* pop states, if we're supposed to */
		while (focus->state && (focus->state->flags & ELVIS_POP))
		{
			/* pop the state */
			statepop(focus);

			/* if the next state has its ELVIS_MORE flag set, then
			 * call the next state's perform() function again.
			 */
			if (focus->state
				&& focus->state->flags & ELVIS_MORE)
			{
				/* call the next state's perform() function again */
				if (result != RESULT_ERROR)
					result = (*focus->state->perform)(focus);
				switch (result)
				{
				  case RESULT_ERROR:
					/* command failed!  alert the window */
					mapalert();
					if (o_errorbells)
						guibeep(focus);
					focus->state->flags &= ~ELVIS_MORE;
					break;

				  case RESULT_COMPLETE:
					focus->state->flags &= ~ELVIS_MORE;
					break;

				  case RESULT_MORE:
					focus->state->flags |= ELVIS_MORE;
					break;
				}
			}
		}

		/* fix the edit bounds */
		fixbounds(focus);

		/* convert ELVIS_ONCE to ELVIS_POP */
		if (focus->state && (focus->state->flags & ELVIS_ONCE))
		{
			focus->state->flags |= ELVIS_POP;
		}

		/* if no states are left, then destroy the window */
		if (!focus->state)
		{
			(*gui->destroygw)(focus->gw, ElvTrue);
		}
	}

	return result;
}
