/* more.c */
/* Copyright 1995 by Steve Kirkendall */



/* This file contains code which implements the "Hit <Enter> to continue"
 * message.  For the sake of uniformity, this is implemented as a special
 * key processing state, applied to a buffer which just happens to contain
 * the "Hit.." message.
 * 
 * Any single character causes this edit mode to exit, so that editing can
 * resume on the real edit buffer.
 *
 * When this key state is pushed, one character can be designated for
 * special treatment.  If that character is pressed, it will be re-inserted
 * into the type-ahead queue.  This is intended to support the use of <:>
 * to start one ex command after another.
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_more[] = "$Id: more.c,v 2.19 2003/10/17 17:41:23 steve Exp $";
#endif

#if USE_PROTOTYPES
static RESULT perform(WINDOW win);
static RESULT enter(WINDOW win);
static RESULT parse(_CHAR_ key, void *info);
static ELVCURSOR shape(WINDOW win);
#endif

ELVBOOL morehit;

typedef struct
{
	CHAR	special;	/* character to receive special processing */
} MOREINFO;

/* This function performs a "[More]" command.  It doesn't really do much;
 * we really only wanted to wait for the user to hit a key.
 */
static RESULT perform(win)
	WINDOW	win;	/* window where <Enter> was hit */
{
	MARKBUF	from;

	/* Erase the "Hit..." message.  Why is that so hard? */
	bufreplace(marktmp(from, markbuffer(win->state->cursor), 0),
		   win->state->cursor, NULL, 0);
	drawopenedit(win);
	win->di->drawstate = DRAW_VISUAL;
	msg(MSG_STATUS, "");
	win->di->drawstate = DRAW_OPENEDIT;

	/* reset the morehit flag */
	morehit = ElvTrue;

	return RESULT_COMPLETE;
}

/* If the user really does hit <Enter> at the "Hit <Enter> to continue" prompt,
 * then this function is called after perform().  This function is a stub,
 * because perform() does everything that needs to be done.  In fact, the only
 * reason we even bother to have an enter() function is so that the state.c
 * module will know that the "Hit <Enter>..." prompt is a separate stratum.
 */
static RESULT enter(win)
	WINDOW	win;	/* window where <Enter> was hit */
{
	return RESULT_MORE;
	/* Not RESULT_COMPLETE, because RESULT_COMPLETE interferes with the
	 * perform() function, some how.  I may be working around a bug here.
	 */
}


/* This function parses a command.  For "[More]", any single key completes the
 * command, so this is trivial.  If it is the special character, then the
 * character is pushed back onto the typeahead queue.
 */
static RESULT parse(key, info)
	_CHAR_	key;	/* the key that the user hit */
	void	*info;	/* struct, contains the special character */
{
	MOREINFO *mi = (MOREINFO *)info;
	CHAR	keyarray[1];

	/* if this is the special key, push it back into the type-ahead queue */
	if (mi->special != '\0' && key == mi->special)
	{
		keyarray[0] = key;
		mapunget(keyarray, 1, ElvFalse);
	}

	return RESULT_COMPLETE;
}

/* This function decides on a cursor shape */
static ELVCURSOR shape(win)
	WINDOW	win;	/* window waiting for <Enter> to be hit */
{
	return CURSOR_COMMAND;
}


/* This function causes elvis to display "Hit <Enter> to continue", and
 * then wait for a keystroke.  Any keystroke is acceptable.  If the keystroke
 * happens to match the "special" argument, then it will be placed back in
 * the type-ahead queue for further processing.
 */
void morepush(win, special)
	WINDOW	win;	/* window that should wait for <Enter> */
	_CHAR_	special;/* a special character, or '\0' if none */
{
	BUFFER	buf;
	MARKBUF	from, to;

	/* if we're in the middle of a macro, never wait for <Enter> */
	if (mapbusy())
		return;

	/* Create the "more" buffer, and put the prompt into it */
	buf = bufalloc(toCHAR(MORE_BUF), 0, ElvTrue);
	bufreplace(marktmp(from, buf, 0), marktmp(to, buf, o_bufchars(buf)),
		toCHAR("Hit <Enter> to continue\n"), 24);

	/* Push the state.  Make sure this keystate will be popped after
	 * a single command (keystroke)
	 */
	statepush(win, ELVIS_POP|ELVIS_BOTTOM);

	/* initialize the state */
	win->state->enter = enter;
	win->state->perform = perform;
	win->state->parse = parse;
	win->state->shape = shape;
	win->state->info = safealloc(1, sizeof (MOREINFO));
	((MOREINFO *)win->state->info)->special = special;
	win->state->modename = "More";
	win->state->top = markalloc(buf, 0);
	win->state->bottom = markalloc(buf, o_bufchars(buf));
	win->state->cursor = markalloc(buf, o_bufchars(buf) - 1);
	win->state->acton = win->state->pop;
}
