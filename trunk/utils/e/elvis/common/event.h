/* event.h */
/* Copyright 1995 by Steve Kirkendall */


typedef enum
{
	CLICK_NONE,	/* return the clicked cell's offset, but don't move */
	CLICK_MOVE,	/* move the cursor; if selecting, adjust selection endpoint */
	CLICK_SSCHAR,	/* don't move, start selecting chars if no sel yet */
	CLICK_SSLINE,	/* don't move, start selecting lines if no sel yet */
	CLICK_SELCHAR,	/* move & start selecting characters */
	CLICK_SELLINE,	/* move & start selecting whole lines */
	CLICK_SELRECT,	/* move & start selecting a rectangle */
	CLICK_CANCEL,	/* cancel the selection; don't move cursor */
	CLICK_YANK,	/* copy selected text to GUI's clipboard; don't move */
	CLICK_PASTE,	/* copy text from GUI's clipboard; don't move */ 
	CLICK_TAG,	/* simulate a <Control-]> keystroke */
	CLICK_UNTAG	/* simulate a <Control-T> keystroke */
} CLICK;

typedef enum
{
	SCROLL_FWDSCR,	/* scroll forward one screen */
	SCROLL_BACKSCR,	/* scroll backward one screen */
	SCROLL_FWDLN,	/* scroll forward one line */
	SCROLL_BACKLN,	/* scroll backward one line */
	SCROLL_COLUMN,	/* scroll sideways to reveal a column */
	SCROLL_PERCENT,	/* move cursor to a given percent of the file */
	SCROLL_LINE	/* move cursor to a given line */
} SCROLL;

extern long	eventcounter;

BEGIN_EXTERNC
extern void	eventupdatecustom P_((ELVBOOL later));
extern ELVBOOL	eventcreate P_((GUIWIN *gw, OPTVAL *guivals, char *name, int rows, int columns));
extern void	eventdestroy P_((GUIWIN *gw));
extern void	eventresize P_((GUIWIN *gw, int rows, int columns));
extern void	eventreplace P_((GUIWIN *gw, ELVBOOL freeold, char *name));
extern void	eventexpose P_((GUIWIN *gw, int top, int left, int bottom, int right));
extern ELVCURSOR eventdraw P_((GUIWIN *gw));
extern ELVCURSOR eventfocus P_((GUIWIN *gw, ELVBOOL change));
extern long	eventclick P_((GUIWIN *gw, int row, int column, CLICK what));
extern MAPSTATE	eventkeys P_((GUIWIN *gw, CHAR *key, int nkeys));
extern ELVBOOL	eventscroll P_((GUIWIN *gw, SCROLL scroll, long count, long denom));
extern void	eventsuspend P_((void));
extern void	eventex P_((GUIWIN *gw, char *excmd, ELVBOOL safer));
END_EXTERNC
