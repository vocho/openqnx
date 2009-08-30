/* window.h */
/* Copyright 1995 by Steve Kirkendall */


typedef struct window_s
{
	struct window_s *next;	/* some other window */
	GUIWIN	*gw;		/* guiwin associated with this window */
	MARK	cursor;		/* main buffer & cursor's offset in it */
	MARK	prevcursor;	/* previous position where <'><'> goes */
	DISPMODE *md;		/* display mode */
	DMINFO	*mi;		/* mode-dependent info */
	DRAWINFO *di;		/* drawing info */
	STATE	*state;		/* stack of keystroke processing states */
	MARK	seltop;		/* start of selected text, or NULL */
	MARK	selbottom;	/* end of selected text, or NULL */
	long	selleft;	/* left column limit for marking (0 for any) */
	long	selright;	/* right column limit (INFINITY for any) */
	long	selorigcol;	/* column where cursor was when marking began */
	ELVBOOL	selattop;	/* boolean: does seltop follow cursor? (else selbottom does) */
	CHAR	seltype;	/* 'c'=character, 'l'=line, 'r'=rectangle */
	CHAR	cmdchars[7];	/* up to six characters of a command */
	int	defaultfont;	/* font to combine with "normal" */
	long	fgcolor;	/* foreground color of previous defaultfont */
	long	bgcolor;	/* background color of previous defaultfont */
	long	match;		/* offset of matching parenthesis, for showmatch */
#ifdef FEATURE_TEXTOBJ
	long	matchend;	/* offset of ending, for ax/ix showmatch */
#endif
	long	wantcol;	/* column where cursor wants to be */
	int	cursx, cursy;	/* cursor position on window */
	char	*modename;	/* current mode, for "showmode" */
	struct
	{
	   MARK	origin;		/* where the cursor was when tag was followed */
	   char	*display;	/* name of display mode of origin buffer */
	   CHAR *prevtag;	/* dynamically alloc'ed copy of prev tag name */
	}	tagstack[TAGSTK];/* array used for storing tag stack */
#ifdef FEATURE_HLOBJECT
	long	hlfrom, hlto;	/* limits of highlighted region */
	struct {		/* details of highlighting */
		long	from;
		long	to;
		int	font;
	} hlinfo[30];		/* "30" corresponds to maximum hllayers value */
#endif
	OPTVAL	*guivals;	/* GUI option values */
	OPTVAL	windowid;	/* unique number to identify this window */
	OPTVAL	columns;	/* number of columns */
	OPTVAL	lines;		/* number of rows */
	OPTVAL	list;		/* show tabs and EOL? */
	OPTVAL	display;	/* display mode */
	OPTVAL	number;		/* show line numbers? */
	OPTVAL	ruler;		/* show ruler? */
	OPTVAL	scroll;		/* scroll amount for ^D/^U */
	OPTVAL	showmatch;	/* show word wrap? */
	OPTVAL	showmode;	/* show command mode? */
	OPTVAL	wrap;		/* wrap long lines? (else scroll sideways) */
	OPTVAL	sidescroll;	/* scroll size when scrolling sideways */
	OPTVAL	wrapmargin;	/* word wrap margin */
	OPTVAL	hasfocus;	/* this window currently has input focus? */
	OPTVAL	folding;	/* is folding visible in this window? */
	OPTVAL	hllayers;	/* number of textobject layers to highlight */
	OPTVAL	eventcounter;	/* most recent input event for this window */
	OPTVAL	ww;		/* generic window option */
} WINDOWBUF, *WINDOW;

#define o_windowid(win)		((win)->windowid.value.number)
#define o_columns(win)		((win)->columns.value.number)
#define o_lines(win)		((win)->lines.value.number)
#define o_list(win)		((win)->list.value.boolean)
#define o_display(win)		((win)->display.value.string)
#define o_number(win)		((win)->number.value.boolean)
#define o_ruler(win)		((win)->ruler.value.boolean)
#define o_scroll(win)		((win)->scroll.value.number)
#define o_showmatch(win)	((win)->showmatch.value.boolean)
#define o_showmode(win)		((win)->showmode.value.boolean)
#define o_wrap(win)		((win)->wrap.value.boolean)
#define o_sidescroll(win)	((win)->sidescroll.value.number)
#define o_wrapmargin(win)	use_o_textwidth_instead
#define o_hasfocus(win)		((win)->hasfocus.value.boolean)
#define o_folding(win)		((win)->folding.value.boolean)
#define o_hllayers(win)		((win)->hllayers.value.number)
#define o_eventcounter(win)	((win)->eventcounter.value.number)
#define o_ww(win)		((win)->ww.value.string)

BEGIN_EXTERNC
extern void wininit P_((void));
extern WINDOW winalloc P_((GUIWIN *gw, OPTVAL *gvals, BUFFER buf, long rows, long columns));
extern void winfree P_((WINDOW win, ELVBOOL force));
extern void winresize P_((WINDOW win, long rows, long columns));
extern void winoptions P_((WINDOW win));
extern int wincount P_((BUFFER buf));
extern void winchgbuf P_((WINDOW win, BUFFER newbuf, ELVBOOL force));
extern WINDOW winofbuf P_((WINDOW win, BUFFER buf));
extern WINDOW winofgw P_((GUIWIN *gw));
END_EXTERNC

extern WINDOW windows;
extern WINDOW windefault;
