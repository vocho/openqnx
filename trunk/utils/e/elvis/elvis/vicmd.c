/* vicmd.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_vicmd[] = "$Id: vicmd.c,v 2.83 2003/10/19 23:13:33 steve Exp $";
#endif



/* This implements the visual @x command, which uses the contents of a
 * cut buffer as simulated keystrokes.
 */
RESULT v_at(win, vinf)
	WINDOW	win;
	VIINFO	*vinf;
{
	CHAR	*keys;

	/* if this buffer is in learn mode, then stop it */
	if (elvalpha(vinf->key2))
		maplearn(vinf->key2, ElvFalse);

	/* copy the cut buffer contents into memory */
	keys = cutmemory(vinf->key2);
	if (!keys)
		return RESULT_ERROR;

	/* Treat the cut buffer contents like keystrokes. */
	mapunget(keys, (int)CHARlen(keys), ElvTrue);
	safefree(keys);

	return RESULT_COMPLETE;
}


/* This function implements the <Z><Z> and <Z><Q> commands */
RESULT v_quit(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	RESULT result = RESULT_COMPLETE;

	switch (vinf->key2)
	{
	  case 'Z':
		result = exstring(win, toCHAR("x"), NULL);
		break;

	  case 'Q':
		result = exstring(win, toCHAR("q!"), NULL);
		break;
	}

	return result;
}

/* This function implements the <Control-R> function.  It does this simply by
 * setting the window's "logic" flag to DRAW_SCRATCH.
 */
RESULT v_expose(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	/* reset the GUI, to bypass any optimizations */
	guireset();

	/* Force the screen to be redrawn from scratch next time */
	win->di->logic = DRAW_SCRATCH;

	return RESULT_COMPLETE;
}


/* This function implements the insert/replace commands. */
RESULT v_input(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	tmp;
	long	offset;
	long	topoff;
	CHAR	newline[1];
	CHAR	cmd;
	BUFFER	dotbuf;
	RESULT	result = RESULT_COMPLETE;

	DEFAULT(1);

	/* if given as a ^O command, this can only toggle input/replace mode */
	if (win->state->flags & ELVIS_ONCE)
	{
		/* we aren't the original vi state -- modify the
		 * input/replace flag of the input state that we
		 * came from.
		 */
		inputtoggle(win, (char)(vinf->command == 'R' ? 'R' : 't'));
		return RESULT_COMPLETE;
	}

	/* If this is a "more" invocation (used to come back to this function
	 * after the user inputs text, if count>1) then pretend that this is
	 * a <.> command with a count of one less than the actual count.
	 */
	cmd = vinf->command;
	if (win->state->flags & ELVIS_MORE)
	{
		vinf->tweak |= TWEAK_DOTTING;
		vinf->count--;
		if (cmd == 'i')
		{
			cmd = 'a';
		}
	}

	/* if the buffer is empty, then the only legal command is <Shift-O> */
	if (o_bufchars(markbuffer(win->state->cursor)) == 0)
	{
		cmd = 'O';
	}

	/* some variations of input require preparation */
	topoff = -1;
	switch (cmd)
	{
	  case 'a':
		/* if not zero-length line, then move left 1 char */
		tmp = dispmove(win, 0, 0);
		if (scanchar(tmp) != '\n')
		{
			markaddoffset(win->state->cursor, 1);
		}
		break;

	  case 'A':
		/* go to end of line, and start inserting there */
		tmp = dispmove(win, 0, INFINITY);
		marksetoffset(win->state->cursor, markoffset(tmp));
		if (scanchar(tmp) != '\n')
		{
			markaddoffset(win->state->cursor, 1);
		}
		break;

	  case ELVG('I'):
		/* go to the start of the line, before indentation */
		tmp = dispmove(win, 0L, 0L);
		marksetoffset(win->state->cursor, markoffset(tmp));
		break;

	  case 'I':
		/* go to the front of the line */
		m_front(win, vinf);
		break;

	  case 'O':
		/* insert a new line before this one */
		tmp = dispmove(win, 0, 0);
		newline[0] = '\n';
		bufreplace(tmp, tmp, newline, 1);
		topoff = markoffset(tmp);
		marksetoffset(win->state->cursor, topoff);
		if ((vinf->tweak & TWEAK_DOTTING) == 0)
		{
			dispindent(win, win->state->cursor, 1);
		}
		break;

	  case 'o':
		/* insert a new line after this one */
		tmp = dispmove(win, 1, 0);
		if (markoffset(tmp) <= markoffset(win->state->cursor))
			marksetoffset(tmp, o_bufchars(markbuffer(tmp)));
		newline[0] = '\n';
		bufreplace(tmp, tmp, newline, 1);
		topoff = markoffset(tmp);
		marksetoffset(win->state->cursor, topoff);
		if ((vinf->tweak & TWEAK_DOTTING) == 0)
		{
			dispindent(win, win->state->cursor, -1);
		}
		break;

	  /* 'i' and 'R' need no special preparation */
	}

	/* shrink the current segment around the cursor */
	offset = markoffset(win->state->cursor);
	marksetoffset(win->state->top, topoff >= 0 ? topoff : offset);
	marksetoffset(win->state->bottom, offset);

	/* if we're doing a <.> command, then don't do interactive stuff.
	 * Instead, just paste a copy of the previous input.
	 */
	if (vinf->tweak & TWEAK_DOTTING)
	{
		/* If R command, then delete old characters */
		if (cmd == 'R')
		{
			dotbuf = cutbuffer('.', ElvFalse);
			if (dotbuf && o_bufchars(dotbuf) > 0L)
			{
				offset = o_bufchars(dotbuf);
				if (offset > 0 && o_partiallastline(dotbuf))
					offset--;
				offset = vinf->count * offset
					+ markoffset(win->state->cursor);
				tmp = dispmove(win, 0L, INFINITY);
				if (offset > markoffset(tmp))
				{
					offset = markoffset(tmp);
					if (scanchar(tmp) != '\n')
					{
						offset++;
					}
				}
				marksetoffset(tmp, offset);
				bufreplace(win->state->cursor, tmp, NULL, 0);
			}
		}

		/* insert copies of the previous text */
		tmp = win->state->cursor;
		do
		{
			tmp = cutput('.', win, tmp, ElvFalse, ElvTrue, ElvTrue);
			if (!tmp) return RESULT_ERROR;
			markaddoffset(tmp, 1);
			vinf->count--;

			/* o and O commands need linebreaks between copies */
			if ((cmd == 'o' || cmd == 'O') && vinf->count > 0)
			{
				newline[0] = '\n';
				bufreplace(tmp, tmp, newline, 1);
				markaddoffset(tmp, 1);
				if ((vinf->tweak & TWEAK_DOTTING) == 0)
				{
					dispindent(win, win->state->cursor, -1);
				}
			}
		} while (vinf->count > 0);
		markaddoffset(tmp, -1);
		marksetoffset(win->state->cursor, markoffset(tmp));
	}
	else /* not doing <.> */
	{
		/* really go into input mode */
		inputpush(win, 0, cmd);

		/* if we have more copies to input, remember that */
		result = (vinf->count > 1) ? RESULT_MORE : RESULT_COMPLETE;
	}

	return result;
}


/* set a named mark */
RESULT v_setmark(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	/* check mark name */
	if (vinf->key2 < 'a' || vinf->key2 > 'z')
	{
		return RESULT_ERROR;
	}

	/* if mark already set, then free its old value. */
	if (namedmark[vinf->key2 - 'a'])
	{
		markfree(namedmark[vinf->key2 - 'a']);
	}

	/* set the mark */
	namedmark[vinf->key2 - 'a'] = markdup(win->state->cursor);
	return RESULT_COMPLETE;
}

/* undo a change */
RESULT v_undo(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	long	offset;

	/* choose an appropriate default */
	DEFAULT(vinf->command == 'U' ? 0 : 1 );

	/* if redo, then negate the undo level */
	if (vinf->command == ELVCTRL('R'))
		vinf->count = -vinf->count;

	/* try to switch to the undo version */
	offset = bufundo(win->state->cursor, vinf->count);

	/* did we succeed? */
	if (offset >= 0)
	{
		/* yes! move the cursor to the position of the undone change */
		assert(offset <= o_bufchars(markbuffer(win->state->cursor)));
		marksetoffset(win->state->cursor, offset);
		return RESULT_COMPLETE;
	}
	else
	{
		/* no, failed */
		return RESULT_ERROR;
	}
}


/* Delete/replace characters from the current line.  This implements the <x>,
 * <Shift-X>, <r>c and <~> commands.
 */
RESULT v_delchar(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	long	front, end;
	MARKBUF	tmp;
	long	curs;
	CHAR	*repstr;
	long	replen;
	CHAR	*cp;
	long	i;
	long	travel;

	DEFAULT(1);

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
		if (scanchar(win->state->cursor) != '\n')
			end++;
	}

	/* if zero-length line, then fail */
	if (front == end)
		return RESULT_ERROR;

	/* construct a replacement string */
	switch (vinf->command)
	{
	  case 'r':
		/* would this extend past the end of the line? */
		curs = markoffset(win->state->cursor);
		if (curs + vinf->count > end)
			return RESULT_ERROR;

		/* the <Enter> key is handled specially... */
		if (vinf->key2 == '\r')
		{
			vinf->key2 = '\n';
			travel = replen = 1;
		}
		else
		{
			replen = vinf->count;
			travel = replen - 1;
		}

		/* constuct the replacement string */
		repstr = (CHAR *)safealloc((int)replen, sizeof(CHAR));
		for (i = 0; i < replen; i++)
		{
			repstr[i] = vinf->key2;
		}
		break;

	  case '~':
		/* would this extend past the end of the line? */
		curs = markoffset(win->state->cursor);
		if (curs + vinf->count > end)
			vinf->count = end - curs;

		/* construct the replacement string */
		replen = vinf->count;
		repstr = (CHAR *)safealloc((int)replen, sizeof(CHAR));
		travel = replen;
		for (i = 0, scanalloc(&cp, marktmp(tmp, markbuffer(win->state->cursor), curs));
		     i < replen; i++, scannext(&cp))
		{
			if (elvupper(*cp))
				repstr[i] = elvtolower(*cp);
			else if (elvlower(*cp))
				repstr[i] = elvtoupper(*cp);
			else
				repstr[i] = *cp;
		}
		scanfree(&cp);
		break;

	  case 'X':
		curs = markoffset(win->state->cursor) - vinf->count;
		repstr = NULL;
		replen = 0;
		travel = 0;
		break;

	  case 'x':
	  default: /* "default:" label is to avoid a bogus compiler warning */
		curs = markoffset(win->state->cursor);
		if (curs + vinf->count > end)
			return RESULT_ERROR;
		else if (curs + vinf->count == end)
			curs = end - vinf->count; /* move bkwd */
		repstr = NULL;
		replen = 0;
		travel = 0;
		break;
	}

	/* if the starting offset is on a different line, fail */
	if (curs < front)
	{
		if (repstr)
			safefree(repstr);
		return RESULT_ERROR;
	}

	/* if the "travel" amount would leave the cursor on the newline,
	 * then adjust it.  EXCEPTION: If invoked via ^O in input mode, then
	 * it is okay to leave the cursor on a newline.
	 */
	if (curs + travel >= end - vinf->count + replen
	 && curs + travel != front
	 && (win->state->flags & ELVIS_ONCE) == 0)
		travel--;

	/* move the cursor & replace/delete the characters */
	marksetoffset(win->state->cursor, curs);
	cutyank(vinf->cutbuf, win->state->cursor,
		marktmp(tmp, markbuffer(win->state->cursor), curs + vinf->count),
		'c', 'y');
	bufreplace(win->state->cursor, &tmp, repstr, replen);

	/* if a replacement string was allocated, free it now */
	if (repstr)
	{
		safefree(repstr);
	}

	/* move to the right (maybe) */
	markaddoffset(win->state->cursor, travel);

	/* if <r><Enter>, then worry about autoindent */
	if (vinf->command == 'r' && vinf->key2 == '\n')
	{
		dispindent(win, win->state->cursor, -replen);
	}

	return RESULT_COMPLETE;
}


/* This function calls the GUI's "tabcmd" function, if it has one. */
RESULT v_window(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	WINDOW	next, w2;

	switch (vinf->key2)
	{
	  case 'w':
	  case ELVCTRL('W'):
		/* were we given a window number? */
		if (vinf->count == 0)
		{
			if (vinf->key2 == 'w')
			{
				/* go to window after this one */
				next = winofbuf(win, NULL);
				if (!next)
				{
				       next = winofbuf(NULL, NULL);
				}
			}
			else
			{
				/* go to window with the highest eventcounter,
				 * other than this one.
				 */
				next = w2 = NULL;
				while ((w2 = winofbuf(w2, NULL)) != NULL)
				{
					if (w2 == win)
						continue;
					if (!next || o_eventcounter(w2) > o_eventcounter(next))
						next = w2;
				}
			}
		}
		else
		{
			/* go to window number 'n' */
			for (next = winofbuf(NULL, NULL);
			     next && o_windowid(next) != vinf->count; /* nishi */
			     next = winofbuf(next, NULL))
			{
			}
		}
		break;

	  case 'k':
		/* move up 1 window */
		for (next = NULL; winofbuf(next, NULL) != win; )
		{
			next = winofbuf(next, NULL);
			if (!next)
				break;
		}
		break;

	  case 'j':
		/* move down 1 window */
		next = winofbuf(win, NULL);
		break;

	  case 's':
		return exstring(win, toCHAR("split"), NULL);

	  case 'n':
		return exstring(win, toCHAR("snew"), NULL);

	  case 'q':
		return exstring(win, toCHAR("xit"), NULL);

	  case 'c':
		return exstring(win, toCHAR("close"), NULL);

	  case 'o':
	  	return exstring(win, toCHAR("only"), NULL);

	  case ']':
	  case ELVCTRL(']'):
		/* Perform a tag lookup.  The v_tag function is clever enough
		 * to realize that this is the splitting style of tag lookup.
		 */
		return v_tag(win, vinf);

	  case 'd':
		if (!CHARcmp(o_display(win), o_bufdisplay(markbuffer(win->cursor))))
		{
			if (!CHARcmp(o_display(win), toCHAR("normal")))
				dispset(win, "hex");
#ifdef DISPLAY_SYNTAX
			else if (CHARncmp(o_bufdisplay(markbuffer(win->cursor)), toCHAR("syntax"), 6)
			      && o_filename(markbuffer(win->cursor))
			      && descr_known(tochar8(o_filename(markbuffer(win->cursor))), SYNTAX_FILE)
			      && CHARcmp(o_display(win), toCHAR("syntax")) )
				dispset(win, "syntax");
#endif
			else
				dispset(win, "normal");
		}
		else
		{
			dispset(win, tochar8(o_bufdisplay(markbuffer(win->cursor))));
		}
		win->di->logic = DRAW_CHANGED;
		return RESULT_COMPLETE;

	  case 'S':
		o_wrap(win) = (ELVBOOL)!o_wrap(win);
		win->di->logic = DRAW_CHANGED;
		return RESULT_COMPLETE;

	  default:
		/* run the GUI's tabcmd function.  If it doesn't have one, or
		 * if it has one but it returns ElvFalse, then fail.
		 */
		if (!gui->tabcmd || !(*gui->tabcmd)(win->gw, vinf->key2, vinf->count))
		{
			return RESULT_ERROR;
		}
		return RESULT_COMPLETE;
	}

	/* did we find a window? */
	if (!next)
	{
		msg(MSG_ERROR, "no such window");
		return RESULT_ERROR;
	}

	/* go to the requested window */
	if (gui->focusgw)
	{
		(*gui->focusgw)(next->gw);
	}
	else
	{
		eventfocus(next->gw, ElvTrue);
	}
	return RESULT_COMPLETE;
}


/* This function implements the visual commands which deal with tags */
RESULT v_tag(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	CHAR	*cmd;
	CHAR	*tagname;
	CHAR	*scan;
	RESULT	result;

	assert(vinf->command == ELVCTRL('T') || vinf->command == ELVCTRL(']')
		|| vinf->command == ELVCTRL('W'));

	/* These commands only work on the first stratum */
	if (win->state->acton)
	{
		msg(MSG_ERROR, "only works on window's default buffer");
		return RESULT_ERROR;
	}

	/* the ^T command is easy... */
	if (vinf->command == ELVCTRL('T'))
		return exstring(win, toCHAR("pop"), NULL);

	/* get the tag name */
	tagname = (*win->md->tagatcursor)(win, win->cursor);
	if (!tagname) return RESULT_ERROR;

	/* build the command.  Use "stag" for ^W[ and "tag" for ^[ */
	cmd = NULL;
	if (vinf->command == ELVCTRL('W'))
		buildCHAR(&cmd, 's');
	for (scan = toCHAR("tag "); *scan; scan++)
		buildCHAR(&cmd, *scan);
	for (scan = tagname; *scan; scan++)
	{
		if (*scan == '|')
			buildCHAR(&cmd, '\\');
		buildCHAR(&cmd, *scan);
	}

	/* run the command */
	result = exstring(win, cmd, NULL);

	/* clean up & exit */
	safefree(tagname);
	safefree(cmd);
	return result;
}


/* This function does three things.  When called with vinf=NULL, it updates the
 * selection to reflect the new cursor position.  When called with a command
 * while a visible selection is pending, it reconfigures the selection or
 * cancels it.  When called with a command when no selection is pending, it
 * starts a selection.
 */
RESULT v_visible(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
#ifndef FEATURE_V
	return RESULT_ERROR;
#else
	assert(win->seltop || vinf);
	/*assert(!win->state->acton);*/

	/* Whenever a visible selection is in progress, redrawing is implied.
	 * We only need to force the screen to be regenerated when we're
	 * canceling a visible selection.
	 */
	if (vinf && win->seltop)
		win->di->logic = DRAW_CHANGED;

	/*--------------------------------------------------------------------*/

	/* are we supposed to be adjusting the visible marking? */
	if (!vinf)
	{
		/* if different buffer (e.g., if entering an ex command line
		 * after highlighting some text) then do nothing.
		 */
		if (markbuffer(win->cursor) != markbuffer(win->seltop))
			return RESULT_COMPLETE;

		/* change "selattop" if appropriate */
		if (markoffset(win->cursor) < markoffset(win->seltop))
		{
			if (!win->selattop)
			{
				if (win->seltype != 'c')
				{
					/* set the bottom mark to the end of
					 * the line which contains the former
					 * top mark.
					 */
					marksetoffset(win->selbottom, markoffset(win->md->move(win, win->seltop, 0, INFINITY, ElvFalse)));
				}
				else
				{
					/* former top mark becomes new bottom */
					marksetoffset(win->selbottom, markoffset(win->seltop));
				}
				win->selattop = ElvTrue;
			}
		}
		else if (markoffset(win->selbottom) < markoffset(win->cursor))
		{
			if (win->selattop)
			{
				if (win->seltype != 'c')
				{
					/* set the top mark to the start of
					 * the line which contains the former
					 * bottom mark.
					 */
					marksetoffset(win->seltop,
						markoffset(win->md->move(win,
							win->selbottom, 0, 0,
							ElvFalse)));
				}
				else
				{
					/* former bottom mark becomes new top */
					marksetoffset(win->seltop,
						markoffset(win->selbottom));
				}
				win->selattop = ElvFalse;
			}
		}

		/* adjust the appropriate endpoint */
		if (win->selattop)
		{
			/* set the top limit */
			if (win->seltype != 'c')
			{
				/* set top to start of cursor line */
				marksetoffset(win->seltop,
					markoffset(win->md->move(win,
						win->cursor, 0, 0, ElvFalse)));
			}
			else
			{
				/* set top to cursor position */
				marksetoffset(win->seltop,
					markoffset(win->cursor));
			}
		}
		else /* we're adjusting the bottom of the marked region */
		{
			/* set the bottom limit */
			if (win->seltype != 'c')
			{
				/* set bottom to end of cursor line */
				marksetoffset(win->selbottom,
					markoffset(win->md->move(win,
						win->cursor, 0, INFINITY,
						ElvFalse)));
			}
			else
			{
				/* set bottom to cursor position */
				marksetoffset(win->selbottom,
					markoffset(win->cursor));
			}
		}

		/* if rectangular, then we also need to adjust column limits */
		if (win->seltype == 'r')
		{
			/* Note that we use wantcol instead of computing the
			 * cursor's actual column number.  The wasn't done
			 * merely for efficiency -- it also has the side-effect
			 * of making $ select all characters to the end of each
			 * line.
			 */
			if (win->selorigcol < win->wantcol)
			{
				win->selleft = win->selorigcol;
				win->selright = win->wantcol;
			}
			else
			{
				win->selleft = win->wantcol;
				win->selright = win->selorigcol;
			}
		}

		/* Whew!  That was hard. */
		return RESULT_COMPLETE;
	}

	/*--------------------------------------------------------------------*/

	/* if already visibly marking, then reconfigure or cancel the marking */
	if (win->seltop)
	{
		/* if <Esc> or same visible type, then cancel selection */
		if (vinf->command == ELVCTRL('[')
		 || vinf->command == (win->seltype == 'c' ? 'v' :
		 		      win->seltype == 'l' ? 'V' : ELVCTRL('V')))
		{
			markfree(win->seltop);
			markfree(win->selbottom);
			win->seltop = win->selbottom = NULL;
			return RESULT_COMPLETE;
		}

		/* Otherwise we reconfigure the existing selection.  We can use
		 * selorigcol and wantcol to find the columns, and seltop and
		 * selbottom to find the lines.  To figure out which column
		 * goes with which line, use selattop.
		 */
		switch (vinf->command)
		{
		  case 'v':
			win->seltype = 'c';
			win->selleft = 0;
			win->selright = INFINITY;
			if (win->selattop)
				marksetoffset(win->selbottom,
					markoffset((*win->md->move)(win,
						win->selbottom, 0L,
						win->selorigcol, ElvFalse)));
			else
				marksetoffset(win->seltop,
					markoffset((*win->md->move)(win,
						win->seltop, 0L,
						win->selorigcol, ElvFalse)));
			break;

		  case 'V':
			win->seltype = 'l';
			win->selleft = 0;
			win->selright = INFINITY;
			if (win->selattop)
				marksetoffset(win->selbottom,
					markoffset((*win->md->move)(win,
						win->selbottom, 0L,
						INFINITY, ElvFalse)));
			else
				marksetoffset(win->seltop,
					markoffset((*win->md->move)(win,
						win->seltop, 0L,
						0L, ElvFalse)));
			break;

		  case ELVCTRL('V'):
			win->seltype = 'r';
			if (win->selorigcol < win->wantcol)
			{
				win->selleft = win->selorigcol;
				win->selright = win->wantcol;
			}
			else
			{
				win->selleft = win->wantcol;
				win->selright = win->selorigcol;
			}
			if (win->selattop)
				marksetoffset(win->selbottom,
					markoffset((*win->md->move)(win,
						win->selbottom, 0L,
						INFINITY, ElvFalse)));
			else
				marksetoffset(win->seltop,
					markoffset((*win->md->move)(win,
						win->seltop, 0L,
						0L, ElvFalse)));
			break;

		  case ELVG('%'):
			win->selattop = (ELVBOOL)!win->selattop;
			if (win->selattop)
				marksetoffset(win->cursor, markoffset(win->seltop));
			else
				marksetoffset(win->cursor, markoffset(win->selbottom));
			switch (win->seltype)
			{
			  case 'c':
				win->wantcol = dispmark2col(win);
				return RESULT_COMPLETE;

			  case 'l':
				marksetoffset(win->cursor, markoffset(dispmove(win, 0L, win->wantcol)));
				return RESULT_COMPLETE;
			}
			/* else fall through for 'r' to move to opposite edge */

		  case ELVG(ELVCTRL('V')):
			if (win->seltype != 'r')
				return RESULT_ERROR;
			if (win->wantcol <= win->selleft)
			{
				win->wantcol = win->selright;
				win->selorigcol = win->selleft;
			}
			else
			{
				win->wantcol = win->selleft;
				win->selorigcol = win->selright;
			}
			break;
		}
		return RESULT_COMPLETE;
	}

	/*--------------------------------------------------------------------*/

	/* else we need to start marking characters, lines, or a rectangle,
	 * depending on what the command key was.
	 */
	switch (vinf->command)
	{
	  case 'v':
		win->seltop = markdup(win->cursor);
		win->selbottom = markdup(win->cursor);
		win->selleft = 0;
		win->selright = INFINITY;
		win->selattop = ElvFalse;
		win->selorigcol = 0;
		win->seltype = 'c';
		break;

	  case 'V':
		win->seltop = markdup(win->md->move(win, win->cursor, 0, 0, ElvFalse));
		win->selbottom = markdup(win->md->move(win, win->cursor, 0, INFINITY, ElvFalse));
		win->selleft = 0;
		win->selright = INFINITY;
		win->selorigcol = 0;
		win->selattop = ElvFalse;
		win->seltype = 'l';
		break;

	  case ELVCTRL('V'):
		win->seltop = markdup(win->md->move(win, win->cursor, 0, 0, ElvFalse));
		win->selbottom = markdup(win->md->move(win, win->cursor, 0, INFINITY, ElvFalse));
		win->selleft = win->md->mark2col(win, win->cursor, ElvTrue);
		win->selright = win->selleft;
		win->selorigcol = win->selleft;
		win->selattop = ElvFalse;
		win->seltype = 'r';
		break;
	}
	return RESULT_COMPLETE;
#endif /* FEATURE_V */
}


/* This function pastes text.  It implements the <p> and <Shift-P> commands. */
RESULT v_paste(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	MARK	dest;

	/* If repeating a paste from a numbered cut buffer, then allow the
	 * cutput function to locate the next numbered cut buffer itself.
	 * Else use the same cut buffer as in original comand.
	 */
	if ((vinf->tweak & TWEAK_DOTTING) != 0 && elvdigit(vinf->cutbuf))
	{
		dest = cutput('\0', win, win->state->cursor, (ELVBOOL)(vinf->command == 'p'), ElvTrue, ElvFalse);
	}
	else
	{
		dest = cutput(vinf->cutbuf, win, win->state->cursor, (ELVBOOL)(vinf->command == 'p'), ElvTrue, ElvFalse);
	}

	/* check for failure or success. */
	if (!dest)
	{
		return RESULT_ERROR;
	}
	marksetoffset(win->state->cursor, markoffset(dest));
	return RESULT_COMPLETE;
}


/* This function implements the <Shift-Q> and <:> commands. */
RESULT v_ex(win, vinf)
	WINDOW	win;	/* window where command was typed */
	VIINFO	*vinf;	/* information about the command */
{
	/* push a stratum that does ex commands */
	statestratum(win, toCHAR(EX_BUF), ':', exenter);

	/* The statetratum() function pushes a state which exits after a
	 * single command.  If the command was <Shift-Q>, then we want to
	 * stay in the state, so we need to tweak the flags.
	 */
	if (vinf->command == 'Q')
	{
		win->state->flags &= ~(ELVIS_POP|ELVIS_ONCE|ELVIS_1LINE);
	}

	return RESULT_COMPLETE;
}


/* This function implements commands which are shortcuts for operator commands:
 * <s>, <Shift-C>, <Shift-D>, <Shift-S>, and <Shift-Y>.
 */
RESULT v_notop(win, vinf)
	WINDOW	win;
	VIINFO	*vinf;
{
	switch (vinf->command)
	{
	  case 's':
		vinf->oper = 'c';
		vinf->command = 'l';
		break;

	  case 'C':
		vinf->oper = 'c';
		vinf->command = '$';
		break;

	  case 'D':
		vinf->oper = 'd';
		vinf->command = '$';
		break;

	  case 'S':
		vinf->oper = 'c';
		vinf->command = '_';
		break;

	  case 'Y':
		vinf->oper = 'y';
		vinf->command = '_';
		break;
	}
	return viperform(win, vinf);
}


/* This function implements visual commands which are short-cuts for certain
 * common ex commands.
 */
RESULT v_notex(win, vinf)
	WINDOW	win;
	VIINFO	*vinf;
{
	EXINFO	xinfb;
	MARK	tmpmark;
	CHAR	*word[3];
	BUFFER	buf;
	RESULT	result;
	int	i;

	DEFAULT(2);
	assert(vinf->command == ELVCTRL('^') || vinf->command == '&'
		|| vinf->command == '*' || vinf->command == 'J'
		|| vinf->command == 'K' || vinf->command == ELVCTRL('Z')
		|| vinf->command == ELVG('J'));

	/* build & execute an ex command */
	memset((char *)&xinfb, 0, sizeof xinfb);
	xinfb.window = win;
	xinfb.defaddr = *win->state->cursor;
	switch (vinf->command)
	{
	  case ELVCTRL('Z'):
		result = exstring(win, toCHAR("stop"), NULL);
		break;

	  case ELVCTRL('^'):
		result = exstring(win, toCHAR("e #"), NULL);
		break;

	  case '&':
		result = exstring(win, toCHAR("&"), NULL);
		break;

	  case '*':
#ifdef FEATURE_MAKE
		result = exstring(win, toCHAR("errlist"), NULL);
#else
		result = RESULT_ERROR;
#endif
		break;

	  case 'J':
	  case ELVG('J'):
		/* ":join" */
		xinfb.fromaddr = win->state->cursor;
		xinfb.from = markline(xinfb.fromaddr);
		xinfb.to = xinfb.from + vinf->count - 1;
		if (xinfb.to > o_buflines(markbuffer(xinfb.fromaddr)))
		{
			msg(MSG_ERROR, "not that many lines in buffer");
			return RESULT_ERROR;
		}
		if (vinf->command == ELVG('J'))
			xinfb.bang = ElvTrue;
		result = ex_join(&xinfb);
		if (result == RESULT_COMPLETE && xinfb.newcurs)
		{
			marksetoffset(win->cursor, markoffset(xinfb.newcurs));
			markfree(xinfb.newcurs);
		}
		break;

	  default: /* 'K' */
		/* ":!ref word" */
		buf = markbuffer(win->state->cursor);
		if (!o_keywordprg(buf))
		{
			msg(MSG_ERROR, "keywordprg not set");
			return RESULT_ERROR;
		}
		tmpmark = wordatcursor(&xinfb.defaddr, ElvFalse);
		if (!tmpmark)
			return RESULT_ERROR;
		word[0] = bufmemory(tmpmark, &xinfb.defaddr);
#ifdef FEATURE_CALC
		if (CHARchr(o_keywordprg(buf), '$'))
		{
			word[1] = o_filename(buf) ? o_filename(buf) : toCHAR("");
			word[2] = NULL;
			xinfb.rhs = calculate(o_keywordprg(buf), word, CALC_MSG);
			if (!xinfb.rhs)
				return RESULT_ERROR; /* message already given */
			xinfb.rhs = CHARdup(xinfb.rhs);
		}
		else /* no $1 in expression */
#endif /* FEATURE_CALC */
		{
			/* append word to command name, with a blank between */
			i = CHARlen(o_keywordprg(markbuffer(win->state->cursor)));
			xinfb.rhs = (CHAR *)safealloc(sizeof(CHAR), i + CHARlen(word[0]) + 2);
			CHARcpy(xinfb.rhs, o_keywordprg(markbuffer(win->state->cursor)));
			xinfb.rhs[i] = ' ';
			CHARcpy(&xinfb.rhs[i + 1], word[0]);
		}
		xinfb.command = EX_BANG;
		result = ex_bang(&xinfb);
		safefree(xinfb.rhs);
		break;
	}

	return result;
}


/* This function implements the # command, which adds or subtracts a value
 * to the number that the cursor is on (if it is on a cursor)
 */
RESULT v_number(win, vinf)
	WINDOW	win;
	VIINFO	*vinf;
{
	CHAR	*p;		/* used for scanning text */
	MARKBUF	start, end;	/* start of the number */
	long	number;		/* the value of the number */
	char	asc[12];	/* the result value, as a string */

	DEFAULT(1);

	/* if not on a number, then fail */
	if (!elvdigit(scanchar(win->state->cursor)))
	{
		return RESULT_ERROR;
	}

	/* locate the start of the number */
	for (scanalloc(&p, win->state->cursor); p && elvdigit(*p); scanprev(&p))
	{
	}
	if (p)
	{
		scannext(&p);
		start = *scanmark(&p);
	}
	else
	{
		start = *win->state->cursor;
		marksetoffset(&start, 0);
		scanseek(&p, &start);
	}

	/* fetch the value of the number */
	for (number = 0; p && elvdigit(*p); scannext(&p))
	{
		number = number * 10 + *p - '0';
	}

	/* remember where the number ended */
	if (p)
	{
		end = *scanmark(&p);
	}
	else
	{
		end = *win->state->cursor;
		marksetoffset(win->state->cursor, o_bufchars(markbuffer(&end)));
	}
	scanfree(&p);

	/* add the argument to the number */
	switch (vinf->key2)
	{
	  case '-':	number -= vinf->count;	break;
	  case '=':	number = vinf->count;	break;
	  default:	number += vinf->count;
	}

	/* convert the result back to a character string */
	sprintf(asc, "%ld", number);

	/* replace the old value with the new value */
	bufreplace(&start, &end, toCHAR(asc), (int)strlen(asc));

	/* leave the cursor at the start of the number */
	marksetoffset(win->state->cursor, markoffset(&start));

	return RESULT_COMPLETE;
}

/* display the character code of character at the cursor */
RESULT v_ascii(win, vinf)
	WINDOW	win;
	VIINFO	*vinf;
{
#ifdef FEATURE_G
	CHAR	c;		/* the character */
	char	printable[10];	/* printable version of the character */

	/* get the character */
	c = scanchar(win->state->cursor);

	/* generate a printable version of it */
	printable[0] = (char)c;
	printable[1] = '\0';
	if (c < ' ' || c == 127)
	{
		printable[0] = '^';
		printable[1] = c ^ 0x40;
		printable[2] = '\0';
	}
	else if (c >= 128)
	{
		switch (o_nonascii)
		{
		  case 'a': /* all are printable */
			/* do nothing */
			break;

		  case 'm': /* most, but not 0x80-0x9f */
		  	if (c >= 0x80 && c <= 0x9f)
		  		printable[0] = '\0';
		  	break;

		  default: /* none or strip */
		  	printable[0] = '\0';
		}
		if (printable[0] == '\0')
			sprintf(printable, "\\%03o", c);
	}

	/* output the message */
	msg(MSG_INFO, "[sd]'$1' (hex($2) $2 octal($2))", printable, (long)c);
	return RESULT_COMPLETE;
#else
	return RESULT_ERROR;
#endif /* FEATURE_G */
}
