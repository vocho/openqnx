/* xscroll.c */

/* Copyright 1997 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_xscroll[] = "$Id: xscroll.c,v 2.16 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef GUI_X11
# include "guix11.h"

/* scroll bar appearance */
#define SB_GAP			1	/* blank pixels between buttons */
#define SB_BEVEL		2	/* width of 3-D shading for buttons */
#define SB_BORDER		1	/* width of border around scrollbar */
#define SB_MINTHUMB		4	/* minimum thumb size */

static	ELVBOOL	paging;		/* clicking on button of current window? */
static	ELVBOOL	thumbing;	/* dragging thumb of current window?*/
static	int	dragfrom;	/* clicked height, relative to sb.top */

/* Predict the size of a scrollbar */
void x_sb_predict(xw, w, h)
	X11WIN	*xw;	/* top-level window where scrollbar will reside */
	unsigned w, h;	/* size of the scrollbar */
{
	/* if scrollbar is disabled, then width is 0 */
	if (!o_scrollbar)
	{
		xw->sb.w = 0;
		xw->sb.h = h;
		return;
	}

	/* if too narrow, then widen it */
	if (w < 5)
		w = 5;

	/* remember the size */
	xw->sb.w = w;
	xw->sb.h = h;
}

/* Create a scrollbar of a given height, and attach it to top-level app window.
 * Note that the position is relative to the top-level app window.  Returns
 * the width of the scrollbar; 0 if none.
 */
void x_sb_create(xw, x, y)
	X11WIN	 *xw;	/* the window to attach the scrollbar to */
	int	 x, y;	/* position of the scrollbar */
{
	/* if we aren't supposed to have a scrollbar, then don't make one */
	if (!o_scrollbar)
		return;

	/* create the window for the scrollbar widget */
#ifdef FEATURE_IMAGE
	if (ispixmap(colorinfo[x_scrollbarcolors].bg))
	{
		xw->sb.win = XCreateSimpleWindow(x_display, xw->win,
			x, y, xw->sb.w, xw->sb.h, 0,
			colorinfo[x_scrollcolors].bg,
			pixmapof(colorinfo[x_scrollbarcolors].bg).pixel);
		XSetWindowBackgroundPixmap(x_display, xw->sb.win,
			pixmapof(colorinfo[x_scrollbarcolors].bg).pixmap);
	}
	else
#endif
	{
		xw->sb.win = XCreateSimpleWindow(x_display, xw->win,
				x, y, xw->sb.w, xw->sb.h, 0,
				colorinfo[x_scrollcolors].bg,
				colorinfo[x_scrollbarcolors].bg);
		if (x_mono)
			XSetWindowBackgroundPixmap(x_display, xw->sb.win, x_gray);
	}
	XSelectInput(x_display, xw->sb.win,
	    ButtonPressMask|ButtonMotionMask|ButtonReleaseMask|ExposureMask);

	/* remember the position of the scrollbar */
	xw->sb.x = x;
	xw->sb.y = y;

	/* compute the height of the page/thumb area, reserving room for the
	 * scrollbar's edges and the line up/down buttons.
	 */
	xw->sb.offset = SB_BORDER + xw->sb.w + SB_GAP;
	xw->sb.height = xw->sb.h - 2 * xw->sb.offset - SB_MINTHUMB;
	xw->sb.width = xw->sb.w - 2 * SB_BORDER;

	/* some safe dummy values to hold us until x_sb_thumb() is called */
	xw->sb.top = 0;
	xw->sb.bottom = xw->sb.h - 2 * xw->sb.offset;
	xw->sb.state = X_SB_NORMAL;
}

void x_sb_setstate(xw, newstate)
	X11WIN		*xw;	/* window whose scrollbar is to be redrawn */
	X_SCROLLSTATE	newstate;/* new state, or X_SB_REDRAW to expose */
{
	XPoint	vertex[9];	/* corners of a stop sign */
	int	i;

	/* if there is no scrollbar, then ignore this */
	if (xw->sb.w == 0)
		return;
		
	/* if no change, then do nothing */
	if (xw->sb.state == newstate)
		return;
	if (newstate != X_SB_REDRAW)
		xw->sb.state = newstate;

	/* draw the background, including the edges */
	XClearWindow(x_display, xw->sb.win);
	XSetForeground(x_display, xw->gc, x_white);
	XDrawLine(x_display, xw->sb.win, xw->gc,
		xw->sb.w - 1, 0, xw->sb.w - 1, xw->sb.h - 1);
	XDrawLine(x_display, xw->sb.win, xw->gc,
		xw->sb.w - 1, xw->sb.h - 1, 0, xw->sb.h - 1);
	XSetForeground(x_display, xw->gc, x_black);
	XDrawLine(x_display, xw->sb.win, xw->gc,
		0, xw->sb.h - 1, 0, 0);
	XDrawLine(x_display, xw->sb.win, xw->gc,
		0, 0, xw->sb.w - 1, 0);

	/* is that all we're supposed to do? */
	switch (xw->sb.state)
	{
	  case X_SB_BLANK:
		/* do nothing more */
		break;

	  case X_SB_STOP:
		/* draw a stop sign */

		/* compute vertices of stop sign */
		vertex[0].x = -(int)xw->sb.width * 5/24;
		vertex[0].y = xw->sb.width / 2 - 1;
		vertex[1].x = -vertex[0].y;
		vertex[1].y = -vertex[0].x;
		vertex[2].x = -vertex[0].y;
		vertex[2].y = vertex[0].x;
		vertex[3].x = vertex[0].x;
		vertex[3].y = -vertex[0].y;
		for (i = 4; i < 9; i++)
		{
			vertex[i].x = -vertex[i - 4].x;
			vertex[i].y = -vertex[i - 4].y;
		}
		for (i = 0; i < 9; i++)
		{
			vertex[i].x += xw->sb.w / 2;
			vertex[i].y += xw->sb.h / 2;
		}

		/* draw the stop sign */
		XSetForeground(x_display, xw->gc, colorinfo[x_cursorcolors].fg);
		XFillPolygon(x_display, xw->sb.win, xw->gc, vertex, 8,
			Convex, CoordModeOrigin);
		XSetForeground(x_display, xw->gc, x_white);
		XDrawLines(x_display, xw->sb.win, xw->gc, vertex, 5, CoordModeOrigin);
		XSetForeground(x_display, xw->gc, x_black);
		XDrawLines(x_display, xw->sb.win, xw->gc, vertex + 4, 5, CoordModeOrigin);
		xw->fg = x_black;
		break;

	  default: /* X_SB_NORMAL */
		/* draw the arrow heads and thumb */
		x_drawbevel(xw, xw->sb.win,
			SB_BORDER, SB_BORDER,
			xw->sb.width, xw->sb.width + 1, 'u', SB_BEVEL);
		x_drawbevel(xw, xw->sb.win,
			SB_BORDER, xw->sb.h - (SB_BORDER + xw->sb.width + 1),
			xw->sb.width, xw->sb.width + 1, 'd', SB_BEVEL);
		x_drawbevel(xw, xw->sb.win,
			SB_BORDER, (int)(xw->sb.offset + xw->sb.top),
			xw->sb.width, xw->sb.bottom - xw->sb.top, 'b', SB_BEVEL);
	}
}

void x_sb_destroy(xw)
	X11WIN	*xw;	/* window whose scrollbar is to be destroyed */
{
	/* if there is no scrollbar, then do nothing. */
	if (xw->sb.w == 0)
		return;

	/* free the window, if it isn't freed already */
	if (xw->sb.win != None)
		XDestroyWindow(x_display, xw->sb.win);
	xw->sb.win = None;
}

void x_sb_thumb(xw, top, bottom, total)
	X11WIN	*xw;	/* window whose scrollbar is to be moved */
	long	top;	/* position of the top of the thumb */
	long	bottom;	/* position of the bottom of the thumb */
	long	total;	/* total scrollable height */
{
	unsigned newtop, newbottom;

	/* if there is no scrollbar, then ignore this */
	if (xw->sb.w == 0)
		return;

	/* compute the positions of the top and bottom */
	newtop = top * xw->sb.height / total;
	newbottom = bottom * xw->sb.height / total + SB_MINTHUMB;

	/* If recoloring, then adjust colors and maybe change the state */
	if (xw->sb.recolored)
	{
		/* Adjust the window's background */
#ifdef FEATURE_IMAGE
		if (ispixmap(colorinfo[x_scrollbarcolors].bg))
			XSetWindowBackgroundPixmap(x_display, xw->sb.win,
				pixmapof(colorinfo[x_scrollbarcolors].bg).pixmap);
		else
#endif
		if (x_mono)
			XSetWindowBackgroundPixmap(x_display, xw->sb.win, x_gray);
		else
			XSetWindowBackground(x_display, xw->sb.win,
				colorinfo[x_scrollbarcolors].bg);
		xw->sb.recolored = ElvFalse;
		x_sb_setstate(xw, X_SB_REDRAW);
		return;
	}

	/* if no change, then do nothing */
	if (newtop == xw->sb.top && newbottom == xw->sb.bottom)
		return;

	/* replace the old thumb with a new one */
	x_drawbevel(xw, xw->sb.win,
		SB_BORDER, (int)(xw->sb.offset + xw->sb.top),
		xw->sb.width, xw->sb.bottom - xw->sb.top, 'b', 0);
	xw->sb.top = newtop;
	xw->sb.bottom = newbottom;
	x_drawbevel(xw, xw->sb.win,
		SB_BORDER, (int)(xw->sb.offset + xw->sb.top),
		xw->sb.width, xw->sb.bottom - xw->sb.top, 'b', SB_BEVEL);
}

/* Handle a scrollbar event. */
void x_sb_event(xw, event)
	X11WIN	*xw;	/* the top-level window which received the message */
	XEvent	*event;	/* the event */
{
	long	thumbto;

	/* if the scrollbar state isn't X_SB_NORMAL then ignore it */
	if (xw->sb.state != X_SB_NORMAL && event->type != Expose)
		return;

	/* process the event */
	switch (event->type)
	{
	  case ButtonPress:
		if (event->xbutton.y < (int)xw->sb.offset)
		{
			/* clicked on the up arrow */
			(void)eventscroll((GUIWIN *)xw, SCROLL_BACKLN, 1L, 0L);
			paging = ElvTrue;
			x_ev_repeat(event, x_repeating ? 0 : o_scrollbartime);
			x_didcmd = ElvTrue;
		}
		else if (event->xbutton.y >= (int)(xw->sb.h - xw->sb.offset))
		{
			/* clicked on the down arrow */
			(void)eventscroll((GUIWIN *)xw, SCROLL_FWDLN, 1L, 0L);
			paging = ElvTrue;
			x_ev_repeat(event, x_repeating ? 0 : o_scrollbartime);
			x_didcmd = ElvTrue;
		}
		else if (event->xbutton.y < (int)(xw->sb.offset + xw->sb.top))
		{
			/* clicked before the thumb */
			(void)eventscroll((GUIWIN *)xw, SCROLL_BACKSCR, 1L, 0L);
			paging = ElvTrue;
			x_ev_repeat(event, x_repeating ? 0 : o_scrollbartime);
			x_didcmd = ElvTrue;
		}
		else if (event->xbutton.y >= (int)(xw->sb.offset + xw->sb.bottom))
		{
			/* clicked after the thumb */
			(void)eventscroll((GUIWIN *)xw, SCROLL_FWDSCR, 1L, 0L);
			paging = ElvTrue;
			x_ev_repeat(event, x_repeating ? 0 : o_scrollbartime);
			x_didcmd = ElvTrue;
		}
		else
		{
			/* clicked on the thumb - prepare for MotionNotify */
			dragfrom = event->xbutton.y - xw->sb.top;
			thumbing = ElvTrue;
		}
		break;

	  case MotionNotify:
		/* moving the scrollbar's thumb? */
		if (thumbing)
		{
			/* compute the percentage */
			thumbto = event->xbutton.y - dragfrom;
			if (thumbto < 0)
				thumbto = 0;
			else if (thumbto > (int)xw->sb.height)
				thumbto = xw->sb.height;
			(void)eventscroll((GUIWIN *)xw, SCROLL_PERCENT, thumbto, (long)xw->sb.height);
		}
		x_didcmd = ElvTrue;
		break;

	  case ButtonRelease:
		x_ev_repeat(NULL, 0L);
		thumbing = paging = ElvFalse;
		break;

	  case Expose:
		x_sb_setstate(xw, X_SB_REDRAW);
		break;
	}
}

void x_sb_recolor(xw)
	X11WIN	*xw;
{
	/* Arrange for colors to change on next update */
	xw->sb.recolored = ElvTrue;
}
#endif /* defined(GUI_X11) */
