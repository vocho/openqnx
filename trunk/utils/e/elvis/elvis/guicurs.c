/* guicurs.c */
/* Copyright 1995 by Steve Kirkendall */


#define WINDOW elviswin
#include "elvis.h"
#ifdef FEATURE_RCSID
char id_guicurs[] = "$Id: guicurs.c,v 2.25 2003/10/17 17:41:23 steve Exp $";
#endif
#undef WINDOW
#ifdef GUI_CURSES

#include <curses.h>


/* Graphic characters */
#ifndef ACS_ULCORNER
# define ACS_ULCORNER	'.'
# define ACS_LLCORNER	'`'
# define ACS_URCORNER	'.'
# define ACS_LRCORNER	'\''
# define ACS_RTEE	'|'
# define ACS_LTEE	'|'
# define ACS_BTEE	'^'
# define ACS_TTEE	'-'
# define ACS_HLINE	'-'
# define ACS_VLINE	'|'
# define ACS_PLUS	'+'
# define ACS(w, on_off)
#else
# define ACS(w, on_off)	wattrset(w, (on_off) ? A_ALTCHARSET : A_NORMAL);
#endif


typedef struct cwin_s
{
	struct cwin_s	*next;		/* pointer to some other window */
	WINDOW		*frame;		/* the window plus the frame */
	WINDOW		*window;	/* the text portion of the window */
	char		*title;		/* name of the window */
	int		rows, columns;	/* size of the window, in cells */
	int		ypos, xpos;	/* position on the screen */
} CWIN;

#if USE_PROTOTYPES
static ELVBOOL clr2eol(GUIWIN *gw);
static ELVBOOL creategw(char *name, char *firstcmd);
static ELVBOOL focusgw (GUIWIN *gw);
static int init(int argc, char **argv);
static int test(void);
static void bell(GUIWIN *gw);
static void destroygw(GUIWIN *gw, ELVBOOL force);
static void draw(GUIWIN *gw, _char_ font, CHAR *text, int len);
static void flush(void);
static void loop(void);
static void moveto(GUIWIN *gw, int column, int row);
static void retitle(GUIWIN *gw, char *name);
static void term(void);
#endif

CWIN *winlist, *current;

/* Test whether this GUI is available in this environment.
 * Returns 0 if the GUI is unavailable, or 1 if available.
 * This should not have any visible side-effects.  If the
 * GUI can't be tested without side-effects, then this
 * function should return 2 to indicate "maybe available".
 */
static int test()
{
	return getenv("TERMCAP") ? 1 : 0;
}

/* Start the GUI.
 *
 * argc and argv are the command line arguments.  The GUI
 * may scan the arguments for GUI-specific options; if it
 * finds any, then they should be deleted from the argv list.
 * The resulting value of argc should be returned normally.
 * If the GUI couldn't initialize itself, it should emit an
 * error message and return -1.
 *
 * Other than "name" and "test", no other fields of the GUI
 * structure are accessed before this function has been called.
 */
static int init(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	initscr();
	raw();
	noecho();
	nonl();
	return argc;
}

/* In a loop, receive events from the GUI and call elvis
 * functions which will act on the event.  When this function
 * returns, elvis will call the GUI's term() function and then exit.
 * (This function should return only when the number of windows becomes 0.)
 */
static void loop()
{
	int	key;
	CHAR	text[10];
	int	len;
	CWIN	*cwin;

	/* peform the -c command or -t tag */
	if (mainfirstcmd(windefault))
		return;

	/* loop until we don't have any windows left */
	while (winlist)
	{
		/* refresh all windows, expecially the current one */
		for (cwin = winlist; cwin; cwin = cwin->next)
		{
			eventdraw((GUIWIN *)cwin);
			wrefresh(cwin->frame);
		}
		touchwin(current->frame);
		wrefresh(current->frame);
		wrefresh(current->window);

		/* read the next event */
		key = wgetch(current->frame);

		/* process the event */
		text[0] = key;
		len = 1;
		eventfocus((GUIWIN *)current);
		(void)eventkeys((GUIWIN *)current, text, len);
	}
}


/* End the GUI.  For "termcap" this means switching the
 * the terminal back into "cooked" mode.  For "x11", the
 * window should be deleted and any other X resources freed.
 *
 * This function is called after all windows have been deleted
 * by delwin(), when elvis is about to terminate.
 */
static void term()
{
	move(LINES-1, 0);
	clrtoeol();
	refresh();
	nl();
	noraw();
	echo();
	endwin();
}

/* This function makes a particular window current -- placing on top of all
 * other windows, and sending any keystrokes to it.
 */
static ELVBOOL focusgw(gw)
	GUIWIN	*gw;	/* window to receive keystrokes */
{
	int	i;
	CWIN	*cw = (CWIN *)gw;

	/* if some other window was already current, it isn't now */
	if (current)
	{
		/* draw the frame around the window */
		ACS(current->frame, ElvTrue);
		box(current->frame, ACS_VLINE, ACS_HLINE);
		ACS(current->frame, ElvFalse);
		wstandout(current->frame);
		mvwprintw(current->frame, 0, 0, "%.*s", current->columns + 2, current->title); /* nishi */
		wstandend(current->frame);
		wrefresh(current->frame);
	}

	/* make the new window current */
	current = cw;

	/* draw a *highlighted* frame around the window */
	wstandout(cw->frame);
	for (i = 0; i < current->rows + 2; i++)
	{
		mvwaddch(cw->frame, i, 0, ' ');
		mvwaddch(cw->frame, i, cw->columns + 1, ' ');
	}
	for (i = 1; i < cw->columns + 1; i++)
	{
		mvwaddch(cw->frame, 0, i, ' ');
		mvwaddch(cw->frame, cw->rows + 1, i, ' ');
	}
	mvwprintw(cw->frame, 0, 0, "%.*s", cw->columns + 2, cw->title);
	wstandend(cw->frame);
	return ElvTrue;
}


/* Create a new window for the buffer named name.  If successful,
 * return TRUE and then simulate a "create" event later.  Return
 * FALSE if the GUIWIN can't be created, e.g., because the GUI doesn't
 * support multiple windows.  The msg() function should be called to
 * describe the reason for the failure.
 */
static ELVBOOL creategw(name, firstcmd)
	char	*name;		/* name of window to create */
	char	*firstcmd;	/* first command to execute in window */
{
	CWIN	*newp;

	/* allocate storate space for the window */
	newp = (CWIN *)safealloc(1, sizeof(CWIN));
	newp->next = winlist;

	/* initialize it. */
	winlist = newp;
	newp->rows = 18;
	newp->columns = 70;
	newp->ypos = rand() & 3;
	newp->xpos = rand() & 7;
	newp->frame = newwin(newp->rows + 2, newp->columns + 2, newp->ypos, newp->xpos);
	newp->window = subwin(newp->frame, newp->rows, newp->columns, newp->ypos + 1, newp->xpos + 1);
	newp->title = safedup(name);
	leaveok(newp->frame, ElvTrue);

	/* make it the current window */
	focusgw((GUIWIN *)newp);

	/* simulate a "window create" event */
	eventcreate((GUIWIN *)newp, NULL, name, newp->rows, newp->columns);

	/* execute the firstcmd, if any */
	if (firstcmd)
	{
		winoptions(winofgw((GUIWIN *)newp));
		exstring(windefault, toCHAR(firstcmd), "+cmd");
	}

	return ElvTrue;
}


/* Simulate a "destroy" event for the window. */
static void destroygw(gw, force)
	GUIWIN	*gw;	/* window to destroy */
	ELVBOOL	force;	/* if ElvTrue, try harder to destroy the window */
{
	CWIN	*cw, *prev;

	/* find the doomed window */
	for (cw = winlist, prev = NULL; cw != (CWIN *)gw; prev = cw, cw = cw->next)
	{
		assert(cw->next != NULL);
	}

	eventdestroy((GUIWIN *)cw);

	/* delete the window from the list of existing windows */
	if (prev)
	{
		prev->next = cw->next;
	}
	else
	{
		winlist = cw->next;
	}

	/* if it was current, it isn't now! */
	if (current == cw && winlist != NULL)
	{
		focusgw((GUIWIN *)winlist);
	}

	/* free the window's resources */
	safefree(cw->title);
	werase(cw->window);	/* nishi */
	werase(cw->frame);	/* nishi */
	wrefresh(cw->window);	/* nishi */
	wrefresh(cw->frame);	/* nishi */
	delwin(cw->window);
	delwin(cw->frame);
	safefree(cw);
}


/* Change the title of the window.  This function is called when a
 * buffer's name changes, or different window becomes associated with
 * a window.  The name argument is the new buffer name.
 */
static void retitle(gw, name)
	GUIWIN	*gw;	/* window to be retitled */
	char	*name;	/* new title (nul-terminated) */
{
	CWIN	*cw = (CWIN *)gw;

	safefree(cw->title);
	cw->title = safedup(name);
	focusgw((GUIWIN *)cw);
}


/* Flush all changes out to the screen */
static void flush()
{
	CWIN	*cw;

	for (cw = winlist; cw; cw = cw->next)
	{
		wrefresh(cw->frame);
	}
}


/* Move the cursor to a given character cell.  The upper left
 * character cell is designated column 0, row 0.
 */
static void moveto(gw, column, row)
	GUIWIN	*gw;	/* window whose cursor should move */
	int	column;	/* column to move to */
	int	row;	/* row to move to */
{
	CWIN	*cw = (CWIN *)gw;

	wmove(cw->window, row, column);
}


/* Displays text on the screen, starting at the cursor's
 * current position, in the given font.  The text string is
 * guaranteed to contain only printable characters.
 *
 * The font is indicated by a single letter.  The letter will
 * be lowercase normally, or uppercase to indicate that the
 * text should be visibly marked for the <v> and <V> commands.
 * The letters are:
 *	n/N	normal characters
 *	b/B	bold characters
 *	i/I	italic characters
 *	u/U	underlined characters
 *	g/G	graphic characters
 *	s	standout (used for messages on bottom row)
 *	p	popup menu
 *
 * This function should move the text cursor to the end of
 * the output text.
 */
static void draw(gw, font, text, len)
	GUIWIN	*gw;	/* window where text should be displayed */
	_char_	font;	/* font to use */
	CHAR	*text;	/* text to be drawn */
	int	len;	/* length of text */
{
	CWIN	*cw = (CWIN *)gw;

	if (font == 'p' || elvupper(font))
	{
		wstandout(cw->window);
		wprintw(cw->window, "%.*s", len, text);
		wstandend(cw->window);
	}
	else
	{
#ifdef A_NORMAL
		switch (font)
		{
		  case 'b':
		  case 'e':	wattrset(cw->window, A_BOLD);		break;
		  case 'u':	wattrset(cw->window, A_UNDERLINE);	break;
		  case 'i':	wattrset(cw->window, A_DIM);		break;
		  default:	wattrset(cw->window, A_NORMAL);
		}
#endif
		wprintw(cw->window, "%.*s", len, text);
	}
}


static ELVBOOL clr2eol(gw)
	GUIWIN	*gw;	/* window whose row is to be cleared */
{
	CWIN	*cw = (CWIN *)gw;

	wclrtoeol(cw->window);
	return ElvTrue;
}

static void bell(gw)
	GUIWIN	*gw;	/* window that generated a bell request */
{
	ttywrite("\007", 1);
}

GUI guicurses =
{
	"curses",	/* name */
	"Curses-based text interface",
	ElvFalse,	/* exonly */
	ElvFalse,	/* newblank */
	ElvFalse,	/* minimizeclr */
	ElvTrue,	/* scrolllast */
	ElvFalse,	/* shiftrows */
	0,	/* movecost */
	0,	/* nopts */
	NULL,	/* optdescs */
	test,
	init,
	NULL,	/* usage */
	loop,
	NULL,	/* wpoll */
	term,
	creategw,
	destroygw,
	focusgw,
	retitle,
	NULL,	/* reset */
	flush,
	moveto,
	draw,
	NULL,	/* shift */
	NULL,	/* scroll */
	clr2eol,
	NULL,	/* newline */
	bell,
	NULL,	/* msg */
	NULL,	/* scrollbar */
	NULL,	/* status */
	NULL,	/* keylabel */
	NULL,	/* clipopen */
	NULL,	/* clipwrite */
	NULL,	/* clipread */
	NULL,	/* clipclose */
	NULL,	/* color */
	NULL,	/* guicmd */
	NULL,	/* tabcmd */
	NULL,	/* save */
	NULL,	/* wildcard */
	NULL,	/* prgopen */
	NULL	/* prgclose */
};
#endif
