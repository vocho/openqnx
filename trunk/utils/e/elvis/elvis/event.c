/* event.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_event[] = "$Id: event.c,v 2.85 2003/10/17 17:41:23 steve Exp $";
#endif

#ifndef DEBUG_EVENT
# define WATCH(x)
# define USUAL_SUSPECTS	;
#else
# define WATCH(x)	x
# ifdef NDEBUG
#  define USUAL_SUSPECTS ;
# else
#  define USUAL_SUSPECTS {WINDOW w; for (w = windows; w; w = w->next){ \
	if (!w->state) continue; \
	assert(markbuffer(w->state->top) == markbuffer(w->state->cursor)); \
	assert(markbuffer(w->state->bottom) == markbuffer(w->state->cursor)); \
	assert(markoffset(w->state->top) <= markoffset(w->state->cursor)); \
	assert(markoffset(w->state->bottom) == 0 \
	    || markoffset(w->state->bottom) >= markoffset(w->state->cursor)); \
    } }
# endif
#endif

#if USE_PROTOTYPES
static void setcursor(WINDOW win, long offset, ELVBOOL clean);
#endif

/* This counts the number of events that have been processed.  In particular,
 * if eventcounter==0 then we know we're still performing initialization.
 */
long eventcounter;

/* Regenerate the CUSTOM_BUF if necessary */
void eventupdatecustom(later)
	ELVBOOL	later;	/* update it before waiting for an event? (else now) */
{
#ifdef FEATURE_MKEXRC
	static ELVBOOL mustupdate;
	BUFFER		custom;	/* the CUSTOM_BUF buffer */
	MARKBUF		top, bottom;

	/* if just supposed to update later, then remember that */
	if (later)
	{
		mustupdate = ElvTrue;
		return;
	}

	/* if no update is needed, then do nothing */
	if (!mustupdate)
		return;
	mustupdate = ElvFalse;

	/* find/create the CUSTOM_BUF buffer */
	custom = bufalloc(toCHAR(CUSTOM_BUF), 0, ElvTrue);
	if (o_bufchars(custom) != 0)
	{
		bufreplace(marktmp(top, custom, 0),
			marktmp(bottom, custom, o_bufchars(custom)),
			NULL, 0);
	}

	/* add stuff into it */
	optsave(custom);
	mapsave(custom);
	digsave(custom);
	colorsave(custom);
	/* abbrsave(custom); */
# ifdef FEATURE_ALIAS
	exaliassave(custom);
# endif
# ifdef FEATURE_SPELL
	spellsave(custom);
# endif
# ifdef FEATURE_AUTOCMD
	ausave(custom);
# endif
	if (gui && gui->save && windefault)
	{
		(*gui->save)(custom, windefault->gw);
	}
#endif
}

/* Some events move the cursor. E.g., clicking the mouse or moving the
 * scrollbar's thumb.  If the window is in input mode, then there may be
 * some superfluous text between the cursor and the end of the edit region
 * (usually marked with a '$') which needs to be deleted before the cursor
 * can be moved.
 *
 * This function detects those situations, and performs the necessary steps.
 * The eventXXX functions should always use this function to set the cursor
 * position.
 */
static void setcursor(win, offset, clean)
	WINDOW	win;	/* window whose cursor should be moved */
	long	offset;	/* new offset for the cursor */
	ELVBOOL	clean;	/* definitely delete superfluous text */
{
	/* if in input mode, and there are superfluous characters after the
	 * cursor (probably the end is marked with a '$' character) then we
	 * may need to delete those characters.  Exception: If the new
	 * location is still in the same edit region, then we don't need to
	 * worry about it.
	 */
	if ((win->state->flags & ELVIS_REGION) == ELVIS_REGION
	 && markoffset(win->state->bottom) != markoffset(win->state->cursor)
	 && (clean
	  || offset < markoffset(win->state->top)
	  || markoffset(win->state->bottom) <= offset))
	{
		/* tweak offset to compensate for the following bufreplace() */
		if (offset > markoffset(win->state->bottom))
			offset -= (markoffset(win->state->bottom) - markoffset(win->state->cursor));

		/* delete the superfluous text */
		bufreplace(win->state->cursor, win->state->bottom, NULL, 0L);
	}
	
	/* move the cursor, and maybe adjust the edit region too */
	marksetoffset(win->state->cursor, offset);
	if (offset < markoffset(win->state->top)
		 || markoffset(win->state->bottom) <= offset)
	{
		marksetoffset(win->state->top, offset);
		marksetoffset(win->state->bottom, offset);
	}
}

/* This function creates a WINDOW -- an internal data structure
 * used to describe the characteristics of a window.  The GUI
 * calls this function when an elvis window suddenly pops into
 * existence.  (Note: Each GUI has a creategw() function to request
 * that a new window be created.)
 *
 * The GUIWIN already exists, and the BUFFER named "name" must also
 * exist.  If anything goes wrong, this function issues an error message
 * via msg(), and returns ElvFalse; the GUI should then destroy its window and
 * forget about it.  Returns ElvTrue for successful creations.
 */
ELVBOOL eventcreate(gw, guivals, name, rows, columns)
	GUIWIN	*gw;		/* GUI's handle for new window */
	OPTVAL	*guivals;	/* values of GUI's window-dependent options */
	char	*name;		/* name of the new window */
	int	rows;		/* height of the new window */
	int	columns;	/* width of the new window */
{
	BUFFER	buf;
	WINDOW	win;

WATCH(fprintf(stderr, "eventcreate(..., name=\"%s\", rows=%d, columns=%d)\n", name, rows, columns));
	USUAL_SUSPECTS
	eventcounter++;
	if (windefault)
		o_eventcounter(windefault) = eventcounter;

	/* find/create a buffer with the requested name */
	buf = buffind(toCHAR(name));
	if (!buf)
	{
		buf = bufload((CHAR *)0, name, ElvFalse);
	}

	/* create a WINDOW */
	win = winalloc(gw, guivals, buf, rows, columns);

#ifndef NDEBUG
	if (gui->moveto) USUAL_SUSPECTS
#endif

	return (ELVBOOL)(win != NULL);
}

/* This function frees the WINDOW associated with gw, and frees it. */
void eventdestroy(gw)
	GUIWIN	*gw;	/* GUI's handle for a window which has been deleted */
{
	WINDOW	win;
	BUFFER	msgq;

WATCH(fprintf(stderr, "eventdestroy(...)\n"));
#ifndef NDEBUG
	if (gui->moveto) USUAL_SUSPECTS
#endif
	eventcounter++;

	/* find the window */
	win = winofgw(gw);
	assert(win);

	/* free it */
	winfree(win, ElvFalse);

	/* if there are any messages pending, and we aren't about to exit,
	 * then flush the messages out to some other window.
	 */
	msgq = buffind(toCHAR(MSGQUEUE_BUF));
	if (msgq && o_bufchars(msgq) > 0L && winofbuf(NULL, NULL) != NULL)
	{
		win = winofbuf(NULL, NULL);
		winoptions(win);
		msgflush();
	}
	USUAL_SUSPECTS
}

/* Change the size of an existing window.  This doesn't automatically redraw
 * the window; later expose events or eventdraw() calls will take care of that.
 */
void eventresize(gw, rows, columns)
	GUIWIN	*gw;	/* GUI's handle for the window that was resized */
	int	rows;	/* new height of the window */
	int	columns;/* new width of the window */
{
	WINDOW	win;

	USUAL_SUSPECTS
WATCH(fprintf(stderr, "eventresize(..., rows=%d, columns=%d\n", rows, columns));
	eventcounter++;

	/* if too small, then ignore the change */
	if (rows < 2 || columns < 30)
	{
		return; /* ElvFalse? */
	}

	/* find the window */
	win = winofgw(gw);
	assert(win);

	/* resize it */
	winresize(win, rows, columns);

	USUAL_SUSPECTS
}

/* Replace the current buffer with some other buffer, for the WINDOW
 * associated with "gw", and then maybe free the old version.  If
 * anything goes wrong, then issue an error message instead.
 */
void eventreplace(gw, freeold, name)
	GUIWIN	*gw;	/* GUI's handle for window to be switched */
	ELVBOOL	freeold;/* if ElvTrue, destroy old buffer; else retain it */
	char	*name;	/* name of new buffer to use */
{
	WINDOW	win;
	BUFFER	buf;

WATCH(fprintf(stderr, "eventreplace(..., freeold=%s, name=\"%s\")\n", freeold?"True":"False", name));
	USUAL_SUSPECTS
	eventcounter++;
	if (windefault)
		o_eventcounter(windefault) = eventcounter;

	/* find/create a buffer with the requested name */
	buf = buffind(toCHAR(name));
	if (!buf)
	{
		buf = bufload((CHAR *)0, name, ElvFalse);
	}

	/* find the window */
	win = winofgw(gw);
	assert(win);

	/* replace its buffer */
	winchgbuf(win, buf, ElvTrue);
	USUAL_SUSPECTS
}

/* Redraw a portion of a window. */
void eventexpose(gw, top, left, bottom, right)
	GUIWIN	*gw;	/* GUI's handle for window which has been exposed */
	int	top;	/* top edge of exposed rectangle */
	int	left;	/* left edge of exposed rectangle */
	int	bottom;	/* bottom edge of exposed rectangle */
	int	right;	/* right edge of exposed rectangle */
{
	WINDOW	win = winofgw(gw);

WATCH(fprintf(stderr, "eventexpose(..., top=%d, left=%d, bottom=%d, right=%d)\n", top, left, bottom, right));
	USUAL_SUSPECTS
	eventcounter++;

	/* permit the bell to ring */
	guibeep(NULL);

	if (top < 0) top = 0;
	if (left < 0) left = 0;
	if (bottom >= o_lines(win)) bottom = o_lines(win) - 1;
	if (right >= o_columns(win)) right = o_columns(win) - 1;
	drawexpose(win, top, left, bottom, right);
	USUAL_SUSPECTS
}

/* Makes sure that a window shows a current image of its buffer's
 * text.  The GUI should only call this function when will have to
 * *wait* for the next event.  It returns the cursor shape.
 */
ELVCURSOR eventdraw(gw)
	GUIWIN	*gw;	/* GUI's handle for window to be updated */
{
	WINDOW	win = winofgw(gw);

WATCH(fprintf(stderr, "eventdraw(...)\n"));
	USUAL_SUSPECTS
	assert(win && win->state);
	eventcounter++;

	/* update the CUSTOM_BUF now, if necessary */
	eventupdatecustom(ElvFalse);

	/* flush any messages */
#ifdef FEATURE_MAKE
	if (!makeflag)
#endif
		msgflush();

	/* permit the bell to ring */
	guibeep(NULL);

	/* call drawopenedit for open-mode windows, or drawimage for visual-mode */
	if (win->state->flags & ELVIS_BOTTOM)
	{
		/* line-at-a-time */
		drawopenedit(win);
	}
	else if (win->di->drawstate == DRAW_OPENOUTPUT)
	{
		/* push a "more" key state */
		morepush(win, win->state->morekey);

		/* display the message */
		drawopenedit(win);
	}
	else
	{
		/* draw the screen */
#ifdef FEATURE_SPELL
		spellbegin();
#endif
		drawimage(win);
#ifdef FEATURE_SPELL
		spellend();
#endif
	}

#ifdef FEATURE_MAKE
	/* After user hits <Enter>, reset the makeflag */
	if (makeflag && morehit)
	{
		makeflag = morehit = ElvFalse;
		msgflush();
	}
#else
	msgflush();
#endif

	USUAL_SUSPECTS
	return (win->state ? (*win->state->shape)(win) : CURSOR_NONE);
}

/* Make the WINDOW and BUFFER associated with "gw" be the defaults.  Returns
 * the cursor shape.  Since the the eventkeys() function does this itself,
 * the only forseeable use of this function is to return the current cursor
 * shape.
 */
ELVCURSOR eventfocus(gw, change)
	GUIWIN	*gw;	/* GUI's handle for window to become new default */
	ELVBOOL	change;	/* really change focus? else just use options */
{
	WINDOW	win = winofgw(gw);
	WINDOW	other;

WATCH(fprintf(stderr, "eventfocus(...)\n"));
	USUAL_SUSPECTS
	assert(!win || win->state);
	eventcounter++;

	if (change && (!win || !o_hasfocus(win)))
	{
#if 1
		for (other = windows; other; other = other->next)
		{
			if (!o_hasfocus(other))
				continue;
			o_hasfocus(other) = ElvFalse;
			if (guicolorsync(other) && gui && gui->draw)
			{
				if (other->state->pop)
					drawexpose(other, 0, 0, (int)o_lines(other)-1, (int)o_columns(other)-1);
				else
				{
					other->di->logic = DRAW_SCRATCH;
					drawimage(other);
				}
			}
		}
#endif
		if (!win)
			return CURSOR_NONE;
		winoptions(win);
		o_hasfocus(win) = ElvTrue;
#if 1
		if (guicolorsync(win) && gui && gui->draw)
		{
			if (win->state->pop)
				drawexpose(win, 0, 0, (int)o_lines(win)-1, (int)o_columns(win)-1);
			else
			{
				win->di->logic = DRAW_SCRATCH;
				drawimage(win);
			}
		}
#endif
	}
	else
		winoptions(win);
	USUAL_SUSPECTS

	return (*win->state->shape)(win);
}

/* Convert a screen point (where (0,0) is the upper-left character cell)
 * to a buffer offset, and move the cursor there.  Optionally start visible
 * marking, depending on the value of "what".
 *
 * This function can also cancel visible marking, when "what" is CLICK_CANCEL.
 * In this case, the "row" and "column" parameters are ignored and the cursor
 * does not move.
 */
long eventclick(gw, row, column, what)
	GUIWIN	*gw;	/* GUI's handle for window that was clicked */
	int	row;	/* row where clicked */
	int	column;	/* column where clicked */
	CLICK	what;	/* action that the click should perform */
{
#ifndef FEATURE_MISC
	return 0;
#else
	WINDOW	win = winofgw(gw);
	VIINFO	vinfbuf;
	MARKBUF	tmp;
	MARK	newcurs;
	long	offset;

WATCH(fprintf(stderr, "eventclick(..., row=%d, column=%d, what=%d)\n", row, column, (int)what));
	USUAL_SUSPECTS
	eventcounter++;

	/* reset the poll frequency counter */
	guipoll(ElvTrue);
	bufmsgtype = MSG_STATUS;

	/* use the options for this window */
	eventfocus(gw, ElvTrue);
	if (windefault)
		o_eventcounter(windefault) = eventcounter;

	/* if the window is showing "Hit <Enter> to continue" then any click
	 * is treated as an <Enter> keystoke.
	 */
	if (!strcmp(win->state->modename, "More"))
	{
		(void)eventkeys(gw, toCHAR("\n"), 1);
		return markoffset(win->cursor);
	}

	/* if not in visual mode, then most clicks mean nothing */
	if (win->state->acton && what != CLICK_PASTE)
	{
		return -1L;
	}

	/* for some operations, the position doesn't matter */
	switch (what)
	{
#ifndef FEATURE_V
	  case CLICK_CANCEL:
	  case CLICK_SSLINE:
	  case CLICK_SSCHAR:
	  case CLICK_YANK:
		return 0;
#else
	  case CLICK_CANCEL:
		vinfbuf.command = ELVCTRL('[');
		(void)v_visible(win, &vinfbuf);
		USUAL_SUSPECTS
		return 0;

	  case CLICK_SSLINE:
		if (win->seltop == NULL)
		{
			vinfbuf.command = 'V';
			(void)v_visible(win, &vinfbuf);
			USUAL_SUSPECTS
		}
		return 0;

	  case CLICK_SSCHAR:
		if (win->seltop == NULL)
		{
			vinfbuf.command = 'v';
			(void)v_visible(win, &vinfbuf);
			USUAL_SUSPECTS
		}
		return 0;

	  case CLICK_YANK:
		/* if no text is marked, then fail */
		if (!win->seltop)
		{
			USUAL_SUSPECTS
			return -1;
		}

		/* Do the yank */
		switch (win->seltype)
		{
		  case 'c':
			/* for character yanks, we need to tweak the "to" value
			 * so the last character is included.
			 */
			tmp = *win->selbottom;
			markaddoffset(&tmp, 1);
			cutyank((_CHAR_)'>', win->seltop, &tmp, win->seltype, 'y');
			break;

		  case 'l':
			/* for line yanks, we want to avoid readjusting the
			 * endpoints, so we'll pass 'L' as the type.
			 */
			cutyank((_CHAR_)'>', win->seltop, win->selbottom, 'L', 'y');
			break;

		  case 'r':
			tmp = *win->selbottom;
			markaddoffset(&tmp, 1);
			cutyank((_CHAR_)'>', win->seltop, &tmp, win->seltype, 'y');
			break;
		}
		USUAL_SUSPECTS
		return 0;
#endif /* FEATURE_V */

	  case CLICK_PASTE:
		/* end any pending selection */
		vinfbuf.command = ELVCTRL('[');
		(void)v_visible(win, &vinfbuf);

		/* delete any superfluous text after the cursor */
		setcursor(win, markoffset(win->state->cursor), ElvTrue);

		/* set the buffer's "willdo" flag so this paste is undoable */
		if (!win->state->pop)
			bufwilldo(win->state->cursor, ElvTrue);

		/* paste the text */
		newcurs = cutput((_CHAR_)'^', win, win->state->cursor, ElvFalse, ElvTrue, ElvTrue);
		if (newcurs)
		{
			/* newcurs is normally at the last character, but we
			 * want to leave it *AFTER* the last character.
			 */
			markaddoffset(newcurs, 1L);

			/* if the edit buffer was empty before, then we may
			 * need to add a newline after this.
			 */
			if (markoffset(newcurs) >= o_bufchars(markbuffer(win->state->cursor)))
			{
				marksetoffset(newcurs, o_bufchars(markbuffer(win->state->cursor)));
				bufreplace(newcurs, newcurs, toCHAR("\n"), 1L);
				markaddoffset(newcurs, 1L);
				marksetoffset(newcurs, o_bufchars(markbuffer(win->state->cursor)) - 2L);
			}

			/* Now we can move the cursor there.  Note that we
			 * don't call setcursor() to do this, because we don't
			 * want to clobber the edit region -- the user should
			 * be able to backspace over the pasted text.
			 */
			marksetoffset(win->state->cursor, markoffset(newcurs));
			if (win->state->bottom)
				marksetoffset(win->state->bottom, markoffset(newcurs));
		}
		USUAL_SUSPECTS
		return 0;

	  case CLICK_TAG:
		/* simulate a <Control-]> keystroke */
		vinfbuf.command = ELVCTRL(']');
		tmp = *win->state->cursor;
		(void)v_tag(win, &vinfbuf);
		USUAL_SUSPECTS
		return 0;
		
	  case CLICK_UNTAG:
	  	/* simulate a <Control-T> keystroke */
	  	vinfbuf.command = ELVCTRL('T');
		tmp = *win->state->cursor;
		(void)v_tag(win, &vinfbuf);
		USUAL_SUSPECTS
	  	return 0;

	  default:
		/* handled below... */
		;
	}

	/* Bad positions are always -1.  If the screen is in ex mode, all
	 * positions are bad.
	 */
	if (win->di->drawstate == DRAW_OPENEDIT
	 || win->di->drawstate == DRAW_OPENOUTPUT
	 || row < 0 || row >= o_lines(win) - 1
	 || column < 0 || column >= o_columns(win))
	{
		USUAL_SUSPECTS
		return -1;
	}

#ifdef FEATURE_LISTCHARS
	/* if sidescrolled, and click is on left margin, then scroll to reveal
	 * chars to left of screen.
	 */
	if (!o_wrap(win) && win->di->skipped > 0 && column < dmnlistchars('<', 9L, 0L, NULL, NULL))
	{
		if (win->di->skipped > o_sidescroll(win))
			win->di->skipped -= o_sidescroll(win);
		else
			win->di->skipped = 0;
		win->di->logic = DRAW_CHANGED;
	}
#endif
	/* Else look it up.  If the clicked-on character is bad (not part of
	 * the buffer) then look leftward in that line for a good character
	 * and use it instead.
	 */
	do
	{
		offset =  win->di->offsets[row * o_columns(win) + column];
	} while (offset < 0 && ++column < o_columns(win));
	if (offset < 0 || offset >= o_bufchars(markbuffer(win->state->cursor)))
	{
		USUAL_SUSPECTS
		return -1;
	}

	/* move the cursor to the click-on cell */
	win->wantcol = column;
	if (o_number(win))
		win->wantcol -= 8;
	setcursor(win, offset, ElvFalse);

#ifdef FEATURE_V
	/* perform the requested operation */
	switch (what)
	{
	  case CLICK_MOVE:
		/* If a selection is in progress, then adjust one end of the
		 * selection to match the cursor position.
		 */
		if (win->seltop)
		{
			(void)v_visible(win, NULL);
		}
		break;

	  case CLICK_SELCHAR:
		/* start character selection */
		vinfbuf.command = ELVCTRL('[');
		(void)v_visible(win, &vinfbuf);
		vinfbuf.command = 'v';
		(void)v_visible(win, &vinfbuf);
		break;

	  case CLICK_SELLINE:
		/* start line selection */
		vinfbuf.command = ELVCTRL('[');
		(void)v_visible(win, &vinfbuf);
		vinfbuf.command = 'V';
		(void)v_visible(win, &vinfbuf);
		break;

	  case CLICK_SELRECT:
		/* start rectangle selection */
		vinfbuf.command = ELVCTRL('[');
		(void)v_visible(win, &vinfbuf);
		vinfbuf.command = ELVCTRL('V');
		(void)v_visible(win, &vinfbuf);
		break;

	  case CLICK_SSCHAR:
	  case CLICK_SSLINE:
	  case CLICK_CANCEL:
	  case CLICK_NONE:
	  case CLICK_YANK:
	  case CLICK_PASTE:
	  case CLICK_TAG:
	  case CLICK_UNTAG:
		/*NOTREACHED*/
		;
	}
#endif
	USUAL_SUSPECTS
	return offset;
#endif /* FEATURE_MISC */
}

/* This function makes the window and buffer associated with "gw"
 * be the current window and buffer, and then interprets keystrokes.
 * Keystroke interpretation involves mapping, states, commands, and
 * all that stuff.
 */
MAPSTATE eventkeys(gw, key, nkeys)
	GUIWIN	*gw;	/* GUI's handle for window that received keypress event */
	CHAR	*key;	/* array of ASCII characters from key */
	int	nkeys;	/* number of ASCII characters */
{
	MAPSTATE mapstate;
WATCH(fprintf(stderr, "eventkeys(..., key={%d, ...}, nkeys=%d)\n", key[0], nkeys));
	USUAL_SUSPECTS
	eventcounter++;

	/* reset the poll frequency counter */
	guipoll(ElvTrue);
	bufmsgtype = MSG_STATUS;

	/* Use the options for this window */
	eventfocus(gw, ElvTrue);
	if (windefault)
		o_eventcounter(windefault) = eventcounter;

	/* send the keys through the mapper */
	mapstate = mapdo(key, nkeys);
	USUAL_SUSPECTS
	return mapstate;
}


/* This scrolls the screen, and returns ElvTrue if successful.  The screen's
 * image won't be adjusted to reflect this, though, until the next eventdraw().
 * This function is intended mostly to be used for processing mouse clicks on
 * the scrollbar.  It may also be useful for mouse draw-through operations
 * in which the mouse is dragged off the edge of the screen.
 */
ELVBOOL eventscroll(gw, scroll, count, denom)
	GUIWIN	*gw;	/* GUI's handle for window to be scrolled */
	SCROLL	scroll;	/* type of scrolling to perform */
	long	count;	/* amount to scroll */
	long	denom;	/* scrollbar height, for moving the "thumb" */
{
#ifndef FEATURE_MISC
	return ElvFalse;
#else
	WINDOW	win = winofgw(gw);
	VIINFO	cmd;
	RESULT	result;
	long	origoffset;
	long	newoffset;

WATCH(fprintf(stderr, "eventscroll(..., %d, %ld, %ld)\n", scroll, count, denom));
	USUAL_SUSPECTS
	assert(win != NULL && (count > 0 || (count == 0 && scroll == SCROLL_PERCENT)));
	eventcounter++;

	/* reset the poll frequency counter */
	guipoll(ElvTrue);

	/* if window is in open mode, this fails */
	if (win->state->flags & ELVIS_BOTTOM)
	{
		msg(MSG_WARNING, "not while in open mode");
		USUAL_SUSPECTS
		return ElvFalse;
	}

	/* Use the options for this window */
	eventfocus(gw, ElvTrue);
	if (windefault)
		o_eventcounter(windefault) = eventcounter;

	/* remember the cursor's original offset */
	origoffset = markoffset(win->state->cursor);

	/* build & execute a vi command */
	cmd.count = count;
	switch (scroll)
	{
	  case SCROLL_BACKSCR:
		cmd.command = ELVCTRL('B');
		result = m_scroll(win, &cmd);
		break;

	  case SCROLL_FWDSCR:
		cmd.command = ELVCTRL('F');
		result = m_scroll(win, &cmd);
		break;

	  case SCROLL_FWDLN:
		cmd.command = ELVCTRL('E');
		result = m_scroll(win, &cmd);
		break;

	  case SCROLL_BACKLN:
		cmd.command = ELVCTRL('Y');
		result = m_scroll(win, &cmd);
		break;

	  case SCROLL_COLUMN:
		cmd.command = ELVCTRL('|');
		result = m_column(win, &cmd);
		break;

	  case SCROLL_PERCENT:
		if (o_buflines(markbuffer(win->cursor)) == 0L)
		{
			/* do nothing */
		}
		else if (count > 0 && o_buflines(markbuffer(win->cursor)) > 0L)
		{
			if (win->md->move == dmnormal.move && denom != 0)
			{
				/* Prescale so we don't overflow 32-bit math */
				while (0x7fffffff / o_bufchars(markbuffer(win->cursor)) < count)
				{
					count >>= 1;
					denom >>= 1;
				}

				cmd.command = 'G';
				cmd.count = 1 + o_buflines(markbuffer(win->cursor)) * count / denom;
				if (cmd.count > o_buflines(markbuffer(win->cursor)))
					cmd.count = o_buflines(markbuffer(win->cursor));
				result = m_absolute(win, &cmd);
			}
			else if (o_bufchars(markbuffer(win->cursor)) == denom)
			{
				/* Move the cursor by setting its offset */
				marksetoffset(win->cursor, count);
				result = RESULT_COMPLETE;
			}
			else
			{
				/* Prescale so we don't overflow 32-bit math */
				while (0x7fffffff / o_bufchars(markbuffer(win->cursor)) < count)
				{
					count >>= 1;
					denom >>= 1;
				}

				/* Move the cursor by setting its offset */
				if (o_bufchars(markbuffer(win->cursor)) != 0)
					marksetoffset(win->cursor, (o_bufchars(markbuffer(win->cursor)) - 1) * count / denom);
				result = RESULT_COMPLETE;
			}
			if (result == RESULT_COMPLETE)
			{
				/* Scroll the cursor's line to top of window */
				cmd.count = cmd.count2 = 0L;
				cmd.command = 'z';
				cmd.key2 = '+';
				result = m_z(win, &cmd);
				if (result == RESULT_COMPLETE)
				{
					cmd.count = win->wantcol + 1;
					cmd.command = ELVCTRL('|');
					result = m_column(win, &cmd);
				}
			}
		}
		else
		{
			cmd.command = ELVCTRL('G');
			cmd.count = 1L;
			result = m_absolute(win, &cmd);
		}
		break;

	  case SCROLL_LINE:
		cmd.command = 'G';
		result = m_absolute(win, &cmd);
		break;
	}

	/* Now we need to get clever about moving the cursor, due to the
	 * possibility of superfluous text after the old cursor position
	 * if we happened to be in input mode.  Force the cursor back to
	 * its old offset, and then use setcursor() to move it to its new
	 * offset in an input-mode-sensitive way.
	 */
	newoffset = markoffset(win->state->cursor);
	marksetoffset(win->state->cursor, origoffset);
	setcursor(win, newoffset, ElvTrue);

	USUAL_SUSPECTS
	return ElvTrue;
#endif /* FEATURE_MISC */
}


/* This function is called by the GUI when the user wants to suspend elvis */
void eventsuspend()
{
	BUFFER	buf;
	WINDOW	win;
	eventcounter++;

	USUAL_SUSPECTS

	/* if autowrite is ElvFalse, then do nothing */
	if (!o_autowrite)
		return;

	/* If there is superfluous text after any cursor, delete it */
	for (win = winofbuf(NULL, NULL); win; win = winofbuf(win, NULL))
	{
		setcursor(win, markoffset(win->state->cursor), ElvTrue);
	}

	/* save all user buffers */
	for (buf = elvis_buffers; buf; buf = buf->next)
	{
		if (!o_internal(buf))
			bufsave(buf, ElvFalse, ElvFalse);
	}

	USUAL_SUSPECTS
}

/* This function is called to execute ex command lines.  This may be a result
 * of selecting a menu item, clicking a button, or something else.  If the
 * command comes from an unreliable source (outside the process) then it should
 * be executed with the "safer" flag set, for security.
 */
void eventex(gw, cmd, safer)
	GUIWIN	*gw;	/* window where command should be run */
	char	*cmd;	/* an ex command to execute */
	ELVBOOL	safer;	/* temporarily set security=safer? */
{
	CHAR	origsecurity;

	eventcounter++;
	USUAL_SUSPECTS

	/* reset the poll frequency counter */
	guipoll(ElvTrue);
	bufmsgtype = MSG_STATUS;

	/* temporarily set the "safer" option appropriately */
	origsecurity = o_security;
	if (safer && o_security == 'n'/* normal */)
		o_security = 's' /* safer */;

	/* Use this window's options while executing the command.  Note that
	 * we don't necessarily assume this window has input focus.
	 */
	winoptions(winofgw(gw));

	/* If there is superfluous text after the cursor, delete it */
	setcursor(windefault, markoffset(windefault->state->cursor), ElvTrue);

	/* execute the command */
	exstring(windefault, toCHAR(cmd), "gui");

	/* update the state as though a key had just been pressed */
	(void)statekey((_CHAR_)-1);

	/* restore the "security" option */
	o_security = origsecurity;

	USUAL_SUSPECTS
}
