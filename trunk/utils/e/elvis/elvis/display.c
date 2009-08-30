/* display.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_display[] = "$Id: display.c,v 2.23 2003/10/17 17:41:23 steve Exp $";
#endif

/* This is a list of all known display modes.  The last mode is the default. */
DISPMODE *allmodes[] =
{
#ifdef DISPLAY_HEX
	&dmhex,
#endif
#ifdef DISPLAY_HTML
	&dmhtml,
#endif
#ifdef DISPLAY_MAN
	&dmman,
#endif
#ifdef DISPLAY_TEX
	&dmtex,
#endif
#ifdef DISPLAY_SYNTAX
	&dmsyntax,
#endif
	&dmnormal
};


/* List the available modes */
void displist(win)
	WINDOW	win;
{
	int	i;
	char	text[100];

	for (i = 0; i < QTY(allmodes); i++)
	{
		sprintf(text, "  %c%-6s %s\n",
			win->md == allmodes[i] ? '*' : ' ',
			allmodes[i]->name,
			allmodes[i]->desc);
		drawextext(win, toCHAR(text), (int)strlen(text));
	}
}


/* Set the edit mode of a given window.  If the name of the new mode is NULL,
 * then the default display mode is used.  If the name doesn't match any known
 * mode, then an error message is issued and dispset() returns ElvFalse;
 * normally it returns ElvTrue.
 */
ELVBOOL dispset(win, newmode)
	WINDOW	win;		/* window whose edit mode is to be changed */
	char	*newmode;	/* name of the new display mode */
{
	int	i, len;

	/* if no name given, use the default mode */
	if (!newmode || !*newmode)
	{
		i = QTY(allmodes) - 1;
		newmode = allmodes[i]->name;
		len = strlen(newmode);
	}
	else /* find the requested mode */
	{
		for (len = 0; newmode[len] && newmode[len] != ' '; len++)
		{
		}
		for (i = 0; i < QTY(allmodes) && strncmp(allmodes[i]->name, newmode, len); i++)
		{
		}
		if (i >= QTY(allmodes))
		{
			msg(MSG_ERROR, "[s]bad display mode $1", newmode);
			return ElvFalse;
		}
	}

	/* if previous mode, then terminate it */
	if (win->md)
	{
#ifdef FEATURE_AUTOCMD
		(void)auperform(win, ElvFalse, NULL, AU_DISPLAYLEAVE, o_display(win));
#endif
		(*win->md->term)(win->mi);
		if (optflags(o_display(win)) & OPT_FREE)
		{
			safefree(o_display(win));
			optflags(o_display(win)) &= ~OPT_FREE;
		}
	}

	/* store the display mode */
	win->md = allmodes[i];
	o_display(win) = CHARdup(toCHAR(newmode));
	optflags(o_display(win)) |= OPT_FREE;
	win->mi = allmodes[i]->init(win);
#ifdef FEATURE_AUTOCMD
	(void)auperform(win, ElvFalse, NULL, AU_DISPLAYENTER, o_display(win));
#endif

	return ElvTrue;
}


/* This function is called twice: once before "elvis.ini" is interpreted, and
 * again afterward.  This function makes EVERY display mode's global options
 * available via ":set" so they can be initialized to user-defined values.
 */
void dispinit(before)
	ELVBOOL	before;	/* ElvTrue before "elvis.ini", and ElvFalse after */
{
	int	i;
	/* for each display mode... */
	for (i = 0; i < QTY(allmodes); i++)
	{
		/* if the mode has global options... */
		if (allmodes[i]->nglobopts > 0)
		{
			/* either add its options, or delete them */
			if (before)
			{
				/* allow the mode to initialize its options */
				(void)(*allmodes[i]->init)(NULL);

				/* make them accessible to :set */
				optinsert(allmodes[i]->name, 
					allmodes[i]->nglobopts,
					allmodes[i]->globoptd,
					allmodes[i]->globoptv);
			}
			else
			{
				optdelete(allmodes[i]->globoptv);
			}
		}
	}
}


/* This function makes a given mode be the default mode... meaning that its
 * options are available to the ":set" command.
 */
void dispoptions(mode, info)
	DISPMODE	*mode;	/* the display mode */
	DMINFO		*info;	/* window-dependent options of this mode */
{
	static DMINFO	*curinfo;
	static OPTVAL	*curglob;

	/* If the global mode options are changing... */
	if (!mode || mode->globoptv != curglob)
	{
		/* delete the old global options, if any */
		if (curglob)
		{
			optdelete(curglob);
		}

		/* insert the new global options, if any */
		if (mode != NULL && mode->nglobopts > 0)
		{
			optinsert(mode->name, mode->nglobopts, mode->globoptd, mode->globoptv);
			curglob = mode->globoptv;
		}
		else
		{
			curglob = NULL;
		}
	}

	/* if the window-dependent mode options are changing... */
	if (info != curinfo)
	{
		/* delete old window-dependent mode options, if any */
		if (curinfo)
		{
			optdelete((OPTVAL *)curinfo);
		}

		/* insert new window-dependent mode options, if any */
		if (mode != NULL && info != NULL && mode->nwinopts > 0)
		{
			optinsert("windisp", mode->nwinopts, mode->winoptd, (OPTVAL *)info);
			curinfo = info;
		}
		else
		{
			curinfo = NULL;
		}
	}
}


/* This function calls the "move" function for a mode.  If the state stack
 * of the given window indicates that we're editing the window's main buffer,
 * then the window's mode is used; otherwise, dmnormal is always used.
 * If in the original visual command state, then the move function will be
 * called for "cmd" behavior.
 */
MARK dispmove(win, linedelta, wantcol)
	WINDOW	win;		/* the window whose mode to use */
	long	linedelta;	/* line movement */
	long	wantcol;	/* desired column number */
{
	ELVBOOL	cmd;

	/* use "cmd" behavior if in vi command state */
	cmd = viiscmd(win);

	/* call the right function */
	if (win->state->acton)
	{
		/* editing a history buffer - use dmnormal */
		return (*dmnormal.move)(win, win->state->cursor, linedelta, wantcol, cmd);
	}
	else
	{
		/* editing window's main buffer - use window's mode */
		return (*win->md->move)(win, win->state->cursor, linedelta, wantcol, cmd);
	}
}


/* This function calls the "mark2col" function for a mode.  If the state stack
 * of the given window indicates that we're editing the window's main buffer,
 * then the window's mode is used; otherwise, dmnormal is always used.
 * If in the original visual command state, then the move function will be
 * called for "cmd" behavior.
 */
long dispmark2col(win)
	WINDOW	win;	/* window whose mode to use */
{
	ELVBOOL	cmd = ElvFalse ;

	/* use "cmd" behavior if in vi command state */
	if (!win->state->pop)
	{
		cmd = ElvTrue;
	}

	/* call the right function */
	if (win->state->acton)
	{
		/* editing a history buffer - use dmnormal */
		return (*dmnormal.mark2col)(win, win->state->cursor, cmd);
	}
	else
	{
		/* editing window's main buffer - use window's mode */
		return (*win->md->mark2col)(win, win->state->cursor, cmd);
	}
}


/* This function implements autoindent.  Given the MARK of a newly created
 * line, insert a copy of the indentation from another line.  The line whose
 * indentation is to be copied is specified as a line delta.  Usually, this
 * will be -1 so the new line has the same indentation as a previous line.
 * The <Shift-O> command uses a linedelta of 1 so the new line will have the
 * same indentation as the following line.
 */
void dispindent(w, line, linedelta)
	WINDOW	w;		/* windows whose options are used */
	MARK	line;		/* new line to adjust */
	long	linedelta;	/* -1 to copy from previous line, etc. */
{
	/* autoindent only works if supported and turned on, and even then
	 * only if we're editing the window's main edit buffer.
	 */
	if (!w->md->indent
	 || !o_autoindent(markbuffer(w->cursor))
	 || markbuffer(w->cursor) != markbuffer(line))
	{
		return;
	}

	/* call the mode's indent function */
	(*w->md->indent)(w, line, linedelta);

	/* if necessary, extend the edit region to include the cursor's
	 * new position.
	 */
	if (markoffset(w->state->cursor) > markoffset(w->state->bottom))
		marksetoffset(w->state->bottom, markoffset(w->state->cursor));
}
