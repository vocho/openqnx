/* xtext.c */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_xtext[] = "$Id: xtext.c,v 2.33 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef GUI_X11
# include "guix11.h"

void x_ta_predict(xw, columns, rows)
	X11WIN		*xw;	/* top-level window to receive new text area */
	unsigned int	columns;/* width of the new text area */
	unsigned int	rows;	/* height of the new text area */
{
	/* remember font metrics */
#ifdef FEATURE_XFT
	if (o_antialias && x_defaultnormal->xftfont)
	{
		xw->ta.cellbase = x_defaultnormal->xftfont->ascent - o_aasqueeze/2;
		xw->ta.cellh = x_defaultnormal->xftfont->height - o_aasqueeze;
		xw->ta.cellw = x_defaultnormal->xftfont->max_advance_width;
	}
	else
#endif
	{
		xw->ta.cellbase = x_defaultnormal->fontinfo->ascent;
		xw->ta.cellh = xw->ta.cellbase + x_defaultnormal->fontinfo->descent;
		xw->ta.cellw = x_defaultnormal->fontinfo->max_bounds.width;
	}

	/* default window geometry */
	xw->ta.rows = rows;
	xw->ta.columns = columns;
	xw->ta.cursx = xw->ta.cursy = 0;
	xw->ta.w = xw->ta.columns * xw->ta.cellw + 2 * o_borderwidth;
	xw->ta.h = xw->ta.rows * xw->ta.cellh + 2 * o_borderwidth;
}

void x_ta_create(xw, x, y)
	X11WIN	*xw;	/* top-level window to receive new text area */
	int	x, y;	/* position of the text area within that window */
{
	/* create the widget window */
	xw->x = x;
	xw->y = y;
	xw->ta.bg = colorinfo[COLOR_FONT_NORMAL].bg;
#ifdef FEATURE_IMAGE
	xw->ta.scrollpixels = 0;
	xw->ta.pixmap = None;
	if (ispixmap(xw->ta.bg))
	{
		xw->ta.win = XCreateSimpleWindow(x_display, xw->win,
			x + o_borderwidth, y + o_borderwidth,
			(unsigned)(xw->ta.w - 2 * o_borderwidth),
			(unsigned)(xw->ta.h - 2 * o_borderwidth),
			0, colorinfo[COLOR_FONT_NORMAL].fg,
			pixmapof(colorinfo[COLOR_FONT_NORMAL].bg).pixel);
		XSetWindowBackgroundPixmap(x_display, xw->win,
			pixmapof(xw->ta.bg).pixmap);
		XSetWindowBackgroundPixmap(x_display, xw->ta.win,
			ParentRelative);
	}
	else
#endif
		xw->ta.win = XCreateSimpleWindow(x_display, xw->win,
			x + o_borderwidth, y + o_borderwidth,
			(unsigned)(xw->ta.w - 2 * o_borderwidth),
			(unsigned)(xw->ta.h - 2 * o_borderwidth),
			0, colorinfo[COLOR_FONT_NORMAL].fg, xw->ta.bg);
	XSelectInput(x_display, xw->ta.win,
	    ButtonPressMask|ButtonMotionMask|ButtonReleaseMask|ExposureMask);

#ifdef FEATURE_XFT
	xw->ta.xftdraw = XftDrawCreate(x_display, xw->ta.win, x_visual, x_colormap);
#endif

	/* pixmap creation, for storing image of character under cursor */
	xw->ta.undercurs = XCreatePixmap(x_display, xw->ta.win, xw->ta.cellw, xw->ta.cellh, (unsigned)x_depth);
	xw->ta.cursor = CURSOR_NONE;
	xw->ta.nextcursor = CURSOR_QUOTE;
}

void x_ta_destroy(xw)
	X11WIN	*xw;	/* top-level window whose text area should be destroyed */
{
#ifdef FEATURE_IMAGE
	/* free the scrolled background pixmap, if any */
	if (xw->ta.pixmap != None)
		XFreePixmap(x_display, xw->ta.pixmap);
#endif

	/* free the cursor pixmap */
	XFreePixmap(x_display, xw->ta.undercurs);

#ifdef FEATURE_XFT
	XftDrawDestroy(xw->ta.xftdraw);
#endif

	/* free the widget window */
	XDestroyWindow(x_display, xw->ta.win);
}


void x_ta_erasecursor(xw)
	X11WIN	*xw;	/* window whose cursor should be hidden */
{
	/* hide the cursor (if shown) */
	if (xw->ta.win != None && xw->ta.cursor != CURSOR_NONE)
	{
		if (xw->ismapped)
		{
			if (xw->grexpose)
			{
				XSetGraphicsExposures(x_display, xw->gc, ElvFalse);
				xw->grexpose = ElvFalse;
			}
			XCopyArea(x_display, xw->ta.undercurs, xw->ta.win, xw->gc,
				0, 0, xw->ta.cellw, xw->ta.cellh,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh));
		}
		xw->ta.cursor = CURSOR_NONE;
	}
}

void x_ta_drawcursor(xw)
	X11WIN	*xw;	/* window whose cursor should be drawn */
{
	unsigned long	color;

	/* This works around an apparent bug Xfree86 4.0.3 on a 3dfx Voodoo3
	 * (and maybe other X setups).  The symptom of the bug is that when
	 * the cursor moves fast over text which is changing, such as when
	 * you're selecting text via the mouse or the v command, parts of the
	 * characters are missing.  Apparently the text rendering hardware
	 * isn't synchronized with the block copying hardware.
	 */
	if (o_synccursor)
		XSync(x_display, ElvFalse);

	/* a NULL "xw" forces all cursors to be redrawn */
	if (!xw)
	{
		for (xw = x_winlist; xw; xw = xw->next)
		{
			xw->ta.nextcursor = xw->ta.cursor;
			x_ta_erasecursor(xw);
			x_ta_drawcursor(xw);
		}
		return;
	}

	/* if not mapped, then no cursor should be drawn */
	if (!xw->ismapped || xw->ta.win == None)
	{
		xw->ta.cursor = CURSOR_NONE;
		return;
	}

	/* if same as before, do nothing */
	if (xw->ta.nextcursor == xw->ta.cursor)
	{
		return;
	}

	/* choose a color */
	if (x_ownselection && colorinfo[x_cursorcolors].da.bits & COLOR_BGSET)
		color = colorinfo[x_cursorcolors].bg;
	else if (colorinfo[x_cursorcolors].da.bits & COLOR_FGSET)
		color = colorinfo[x_cursorcolors].fg;
	else
		color = x_black;

	/* if some other cursor shape is already drawn there, then erase it */
	if (xw->ta.cursor != CURSOR_NONE)
	{
		x_ta_erasecursor(xw);
	}
	else /* save the image of the character where we'll draw the cursor */
	{
		if (xw->grexpose)
		{
			XSetGraphicsExposures(x_display, xw->gc, ElvFalse);
			xw->grexpose = ElvFalse;
		}
		XCopyArea(x_display, xw->ta.win, xw->ta.undercurs, xw->gc,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
			xw->ta.cellw, xw->ta.cellh, 0, 0);
	}
	xw->ta.cursor = xw->ta.nextcursor;

	/* draw the cursor, using the cursor color */
	if (xw->grexpose)
	{
		XSetGraphicsExposures(x_display, xw->gc, ElvFalse);
		xw->grexpose = ElvFalse;
	}
	if (xw->fg != color)
	{
		XSetForeground(x_display, xw->gc, color);
		xw->fg = color;
	}
	switch (xw->ta.cursor)
	{
	  case CURSOR_INSERT:
		XFillRectangle(x_display, xw->ta.win, xw->gc,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
			2, xw->ta.cellh);
		break;

	  case CURSOR_REPLACE:
	  case CURSOR_QUOTE:
		XFillRectangle(x_display, xw->ta.win, xw->gc,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)((xw->ta.cursy + 1) * xw->ta.cellh - 2),
			xw->ta.cellw, 2);
		break;

	  case CURSOR_COMMAND:
		switch ((xw == x_hasfocus) ? o_textcursor : 'h')
		{
		  case 'h': /* hollow */
			XDrawRectangle(x_display, xw->ta.win, xw->gc,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
				xw->ta.cellw - 1, xw->ta.cellh - 1);
			break;

		  case 'o': /* opaque */
			XFillRectangle(x_display, xw->ta.win, xw->gc,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
				xw->ta.cellw, xw->ta.cellh);
			break;

		  case 'x': /* xor */
#ifdef FEATURE_IMAGE
			if (ispixmap(colorinfo[COLOR_FONT_NORMAL].bg))
				xw->fg ^= pixmapof(colorinfo[COLOR_FONT_NORMAL].bg).pixel;
			else
#endif
				xw->fg ^= colorinfo[COLOR_FONT_NORMAL].bg;
			XSetForeground(x_display, xw->gc, xw->fg);
			XSetFunction(x_display, xw->gc, GXxor);
			XFillRectangle(x_display, xw->ta.win, xw->gc,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
				xw->ta.cellw, xw->ta.cellh);
			XSetFunction(x_display, xw->gc, GXcopy);
			break;
		}
		break;

	  case CURSOR_NONE:
		break;
	}
}


/* Move the cursor to a given character cell.  The upper left
 * character cell is designated column 0, row 0.
 */
void x_ta_moveto(xw, column, row)
	X11WIN	*xw;	/* the window whose cursor is to be moved */
	int	column;	/* the column to move to */
	int	row;	/* the row to move to */
{
	if (xw->ta.cursx != column || xw->ta.cursy != row)
	{
		x_ta_erasecursor(xw);
		xw->ta.cursx = column;
		xw->ta.cursy = row;
	}
}


/* Displays text on the screen, starting at the cursor's
 * current position, in the given font.  The text string is
 * guaranteed to contain only printable characters.
 *
 * This function should move the text cursor to the end of
 * the output text.
 */
void x_ta_draw(xw, fg, bg, bits, text, len)
	X11WIN	*xw;	/* the window where the text should be drawn */
	long	fg, bg;	/* colors */
	int	bits;	/* bitmap of other attributes */
	CHAR	*text;	/* the text to draw */
	int	len;	/* number of characters in text */
{
	X_LOADEDFONT	*loaded;
	XGCValues	gcvalues;
	int		i, x, y;
	GC		clipgc;
#ifdef FEATURE_XFT
	Region		region;
	XRectangle	rect;
#endif

	xw->ta.cursor = CURSOR_NONE;

	/* if we have a special font for bold or italic, then use it */
	if ((bits & COLOR_GRAPHIC) == COLOR_GRAPHIC)
		loaded = x_defaultnormal;
	else if ((bits & (COLOR_BOLD|COLOR_ITALIC)) == COLOR_BOLD && x_defaultbold)
		loaded = x_defaultbold, bits &= ~COLOR_BOLD;
	else if ((bits & COLOR_ITALIC) != 0 && x_defaultitalic)
		loaded = x_defaultitalic, bits &= ~COLOR_ITALIC;
	else
		loaded = x_defaultnormal;

	/* set the GC values */
	gcvalues.font = loaded->fontinfo->fid;
	gcvalues.graphics_exposures = xw->grexpose = ElvFalse;
	gcvalues.foreground = fg;
#ifdef FEATURE_IMAGE
	if (ispixmap(bg))
	{
		XChangeGC(x_display, xw->gc, 
			GCForeground|GCFont|GCGraphicsExposures, &gcvalues);
	}
	else
#endif
	{
		gcvalues.background = bg;
		XChangeGC(x_display, xw->gc, 
			GCForeground|GCBackground|GCFont|GCGraphicsExposures, &gcvalues);
		xw->bg = gcvalues.background;
	}
	xw->fg = gcvalues.foreground;

	if ((bits & COLOR_GRAPHIC) == COLOR_GRAPHIC)
	{
		/* draw graphic characters instead of text */
		int	 centerx, centery;
		int	 left, right, top, bottom;
		int	 radius;
		XSegment seg[4];
		int	 nsegs;

		/* erase any old info */
#ifdef FEATURE_IMAGE
		if (ispixmap(bg))
		{
			XClearArea(x_display, xw->ta.win,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)((xw->ta.cursy) * xw->ta.cellh),
				len * xw->ta.cellw, xw->ta.cellh, ElvFalse);
			if (o_synccursor)
				XSync(x_display, ElvFalse);
		}
		else
#endif
		{
			XSetForeground(x_display, xw->gc, bg);
			XFillRectangle(x_display, xw->ta.win, xw->gc,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)((xw->ta.cursy) * xw->ta.cellh),
				len * xw->ta.cellw, xw->ta.cellh);
		}

		/* draw the graphic chars individually */
		left = xw->ta.cellw / 2;
		right = xw->ta.cellw - left;
		top = xw->ta.cellh / 2;
		bottom = xw->ta.cellh - top;
		radius = xw->ta.cellw / 3;
		centerx = xw->ta.cursx * xw->ta.cellw + left;
		centery = xw->ta.cursy * xw->ta.cellh + top;
		for (i = 0; i < len; i++, centerx += xw->ta.cellw)
		{
			/* initialize the coords to the center point */
			seg[0].x1 = seg[0].x2 = centerx;
			seg[0].y1 = seg[0].y2 = centery;
			seg[3] = seg[2] = seg[1] = seg[0];
			nsegs = 0;

			/* decide which segments to draw */
			if (CHARchr(toCHAR("456789|"), text[i]))
				seg[nsegs++].y2 += bottom;
			if (CHARchr(toCHAR("123456|"), text[i]))
				seg[nsegs++].y2 -= top;
			if (CHARchr(toCHAR("235689-"), text[i]))
				seg[nsegs++].x2 -= left;
			if (CHARchr(toCHAR("124578-"), text[i]))
				seg[nsegs++].x2 += right;
				
			/* draw the segments */
			XSetForeground(x_display, xw->gc, fg);
			if (nsegs > 0)
				XDrawSegments(x_display, xw->ta.win, xw->gc, seg, nsegs);
			else if (text[i] == 'o')
				XDrawArc(x_display, xw->ta.win, xw->gc,
					centerx - radius, centery - radius,
					radius * 2, radius * 2, 0, 360*64);
			else if (text[i] == '*')
				XFillArc(x_display, xw->ta.win, xw->gc,
					centerx - radius, centery - radius,
					radius * 2 + 1, radius * 2 + 1, 0, 360*64);
		}
	}
	else
	{
		/* bold fonts sometimes extend to the left of the image, which
		 * can cause scrolling to leave pixels dribbling off.  To avoid
		 * this, clip the text.
		 */
		if (loaded == x_defaultbold)
		{
			XRectangle	rect;
			clipgc = XCreateGC(x_display, xw->ta.win, 0, NULL);
			XCopyGC(x_display, xw->gc, ~0, clipgc);
			rect.x = (int)(xw->ta.cursx * xw->ta.cellw);
			rect.y = (int)(xw->ta.cursy * xw->ta.cellh);
			rect.width = xw->ta.w;
			rect.height = xw->ta.cellh;
			XSetClipRectangles(x_display, clipgc, 0, 0, &rect, 1, Unsorted);
		}
		else
			clipgc = xw->gc;
#ifdef FEATURE_XFT
		region = XCreateRegion();
		rect.x = (int)(xw->ta.cursx * xw->ta.cellw);
		rect.y = (int)(xw->ta.cursy * xw->ta.cellh);
		rect.width = xw->ta.w;
		rect.height = xw->ta.cellh;
		XUnionRectWithRegion(&rect, region, region);
		XftDrawSetClip(xw->ta.xftdraw, region);
		XDestroyRegion(region);
#endif


		/* draw the text */
#ifdef FEATURE_XFT
		if (o_antialias && loaded->xftfont)
		{
#ifdef FEATURE_IMAGE
			if (ispixmap(bg))
			{
				XClearArea(x_display, xw->ta.win,
					(int)(xw->ta.cursx * xw->ta.cellw),
					(int)(xw->ta.cursy * xw->ta.cellh),
					(int)(xw->ta.cellw * len),
					(int)xw->ta.cellh, ElvFalse);
			}
			else
#endif
			{
				XSetForeground(x_display, xw->gc, bg);
				XFillRectangle(x_display, xw->ta.win, xw->gc,
					(int)(xw->ta.cursx * xw->ta.cellw),
					(int)(xw->ta.cursy * xw->ta.cellh),
					(int)(xw->ta.cellw * len),
					(int)xw->ta.cellh);
				XSetForeground(x_display, xw->gc, fg);
			}
			XftDrawString8(xw->ta.xftdraw, x_xftpixel(fg),
				loaded->xftfont, 
				(int)(xw->ta.cursx * xw->ta.cellw),
				(int)(xw->ta.cursy * xw->ta.cellh + xw->ta.cellbase),
				(XftChar8 *)text, len);
		}
		else
#endif
#ifdef FEATURE_IMAGE
		if (ispixmap(bg))
		{
			XClearArea(x_display, xw->ta.win,
				(int)(xw->ta.cursx * xw->ta.cellw),
				(int)(xw->ta.cursy * xw->ta.cellh),
				(int)(xw->ta.cellw * len),
				(int)xw->ta.cellh, ElvFalse);
			if (o_synccursor)
				XSync(x_display, ElvFalse);
			XDrawString(x_display, xw->ta.win, clipgc,
				(int)(xw->ta.cursx * xw->ta.cellw),
				(int)(xw->ta.cursy * xw->ta.cellh + xw->ta.cellbase),
				tochar8(text), len);
		}
		else
#endif
			XDrawImageString(x_display, xw->ta.win, clipgc,
				(int)(xw->ta.cursx * xw->ta.cellw),
				(int)(xw->ta.cursy * xw->ta.cellh + xw->ta.cellbase),
				tochar8(text), len);
		if (clipgc != xw->gc)
			XFreeGC(x_display, clipgc);
		if ((bits & COLOR_BOLD) != 0)
		{
#ifdef FEATURE_XFT
			if (o_antialias && loaded->xftfont)
				XftDrawString8(xw->ta.xftdraw, x_xftpixel(fg),
					loaded->xftfont, 
					(int)(xw->ta.cursx * xw->ta.cellw + 1),
					(int)(xw->ta.cursy * xw->ta.cellh + xw->ta.cellbase),
					(XftChar8 *)text, len);
			else
#endif
			XDrawString(x_display, xw->ta.win, xw->gc,
				(int)(xw->ta.cursx * xw->ta.cellw + 1), (int)(xw->ta.cursy * xw->ta.cellh + xw->ta.cellbase),
				tochar8(text), len);
		}
		if ((bits & COLOR_ITALIC) != 0)
		{
			XCopyArea(x_display, xw->ta.win, xw->ta.win, xw->gc,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
				len * xw->ta.cellw - 1, (xw->ta.cellh + 1) / 2,
				(int)(xw->ta.cursx * xw->ta.cellw + 1), (int)(xw->ta.cursy * xw->ta.cellh));
		}
		if ((bits & COLOR_UNDERLINED) != 0)
		{
			XFillRectangle(x_display, xw->ta.win, xw->gc,
				(int)(xw->ta.cursx * xw->ta.cellw), (int)((xw->ta.cursy + 1) * xw->ta.cellh - 1),
				len * xw->ta.cellw, 1);
		}
		x = (int)(xw->ta.cursx * xw->ta.cellw);
		y = (int)(xw->ta.cursy * xw->ta.cellh);
		if ((bits & COLOR_BOXED) != 0)
		{
			XDrawLine(x_display, xw->ta.win, xw->gc,
				x, y,
				x + len * xw->ta.cellw - 1, y);
			XDrawLine(x_display, xw->ta.win, xw->gc,
				x, y + xw->ta.cellh - 1,
				x + len * xw->ta.cellw - 1, y + xw->ta.cellh - 1);
		}
		if ((bits & COLOR_RIGHTBOX) != 0)
		{
			XDrawLine(x_display, xw->ta.win, xw->gc,
				x + len * xw->ta.cellw - 1, y,
				x + len * xw->ta.cellw - 1, y + xw->ta.cellh - 1);
		}
		if ((bits & COLOR_LEFTBOX) != 0)
		{
			if ((bits & COLOR_BOXED) == 0
			 && (colorinfo[x_guidecolors].da.bits & (COLOR_SET|COLOR_FGSET)) == (COLOR_SET|COLOR_FGSET))
			{
				xw->fg = colorinfo[x_guidecolors].fg;
				XSetForeground(x_display, xw->gc, xw->fg);
			}
			XDrawLine(x_display, xw->ta.win, xw->gc,
				x, y,
				x, y + xw->ta.cellh - 1);
		}
	}

	/* leave the cursor after the text */
	xw->ta.cursx += len;
}

/* Insert "qty" characters into the current row, starting at
 * the current cursor position.  A negative "qty" value means
 * that characters should be deleted.
 *
 * This function is optional.  If omitted, elvis will rewrite
 * the text that would have been shifted.
 */
void x_ta_shift(xw, qty, rows)
	X11WIN	*xw;	/* window to be shifted */
	int	qty;	/* amount to shift by */
	int	rows;	/* number of rows affected */
{
	/* erase the cursor */
	x_ta_erasecursor(xw);

	/* make sure we have the right background */
	if (!xw->grexpose)
	{
		XSetGraphicsExposures(x_display, xw->gc, ElvTrue);
		xw->grexpose = ElvTrue;
	}
	if (xw->bg != (unsigned long)xw->ta.bg)
	{
		XSetBackground(x_display, xw->gc, xw->ta.bg);
		xw->bg = xw->ta.bg;
	}

	if (qty > 0)
	{
		/* we'll be inserting */

		/* shift the characters */
		XCopyArea(x_display, xw->ta.win, xw->ta.win, xw->gc,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
			xw->ta.cellw * (xw->ta.columns - xw->ta.cursx - qty), xw->ta.cellh * rows,
			(int)((xw->ta.cursx + qty) * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh));
	}
	else
	{
		/* we'll be deleting.  Convert qty to absolute value. */
		qty = -qty;

		/* shift the characters */
		XCopyArea(x_display, xw->ta.win, xw->ta.win, xw->gc,
			(int)((xw->ta.cursx + qty) * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
			xw->ta.cellw * (xw->ta.columns - xw->ta.cursx - qty), xw->ta.cellh * rows,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh));
	}
}

void x_ta_scroll(xw, qty, notlast)
	X11WIN	*xw;	/* window to be scrolled */
	int	qty;	/* amount to scroll by (pos=downward, neg=upward) */
	ELVBOOL	notlast;/* if ElvTrue, last row should not be affected */
{
	int	rows;
#ifdef FEATURE_IMAGE
	int	scrollpixels;
	struct x_bgmap_s *bgm;
#endif

	/* erase the cursor */
	x_ta_erasecursor(xw);

	/* decide bottom row to scroll */
	rows = xw->ta.rows;
	if (notlast)
	{
		rows--;
	}

#ifdef FEATURE_IMAGE
	/* scroll the background image, if any */
	if (ispixmap(xw->ta.bg) && xw->ta.cursy == 0)
	{
		/* find the background pixmap info */
		bgm = &pixmapof(xw->ta.bg);

		/* adjust the number of pixels to scroll the bg image */
		scrollpixels = xw->ta.scrollpixels - qty * xw->ta.cellh;
		if (scrollpixels < 0)
			scrollpixels += xw->ta.rows * bgm->height;
		scrollpixels %= bgm->height;

		/* scroll the background image */
		if (scrollpixels == 0)
		{
			if (xw->ta.pixmap != None)
			{
				XFreePixmap(x_display, xw->ta.pixmap);
				xw->ta.pixmap = None;
			}
			XSetWindowBackgroundPixmap(x_display, xw->win, bgm->pixmap);
		}
		else
		{
			if (xw->ta.pixmap == None)
				xw->ta.pixmap = XCreatePixmap(x_display,
							     xw->ta.win,
							     bgm->width,
							     bgm->height,
							     (unsigned)x_depth);
			XCopyArea(x_display, bgm->pixmap,
				xw->ta.pixmap, xw->gc,
				0, scrollpixels,
				bgm->width, bgm->height - scrollpixels,
				0, 0);
			XCopyArea(x_display, bgm->pixmap,
				xw->ta.pixmap, xw->gc,
				0, 0,
				bgm->width, scrollpixels,
				0, bgm->height - scrollpixels);
			XSetWindowBackgroundPixmap(x_display, xw->win,
				xw->ta.pixmap);
		}
		xw->ta.scrollpixels = scrollpixels;
	}
#endif

	/* make sure we have the right background */
	if (!xw->grexpose)
	{
		XSetGraphicsExposures(x_display, xw->gc, ElvTrue);
		xw->grexpose = ElvTrue;
	}
	if (xw->bg != (unsigned long)xw->ta.bg)
	{
		XSetBackground(x_display, xw->gc, xw->ta.bg);
		xw->bg = xw->ta.bg;
	}

	if (qty > 0)
	{
		/* we'll be inserting */

		/* shift the rows */
		XCopyArea(x_display, xw->ta.win, xw->ta.win, xw->gc,
			0, (int)(xw->ta.cursy * xw->ta.cellh),
			xw->ta.cellw * xw->ta.columns, xw->ta.cellh * (rows - xw->ta.cursy - qty),
			0, (int)((xw->ta.cursy + qty) * xw->ta.cellh));
	}
	else
	{
		/* we'll be deleting.  Convert qty to absolute value. */
		qty = -qty;

		/* shift the rows */
		XCopyArea(x_display, xw->ta.win, xw->ta.win, xw->gc,
			0, (int)((xw->ta.cursy + qty) * xw->ta.cellh),
			xw->ta.cellw * xw->ta.columns, xw->ta.cellh * (rows - xw->ta.cursy - qty),
			0, (int)(xw->ta.cursy * xw->ta.cellh));
	}
}

void x_ta_clrtoeol(xw)
	X11WIN	*xw;	/* window whose row is to be cleared */
{
	/* make sure we have the right background */
	XSetForeground(x_display, xw->gc, xw->ta.bg);
	xw->fg = xw->ta.bg;
	if (xw->grexpose)
	{
		XSetGraphicsExposures(x_display, xw->gc, ElvFalse);
		xw->grexpose = ElvFalse;
	}

	/* whether or not the cursor was visible before, it'll be invisible
	 * after we erase the line.
	 */
	xw->ta.cursor = CURSOR_NONE;

	/* erase the line, from the cursor to the right edge */
#ifdef FEATURE_IMAGE
	if (ispixmap(xw->ta.bg))
	{
		XClearArea(x_display, xw->ta.win,
			(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
			(xw->ta.columns - xw->ta.cursx) * xw->ta.cellw, xw->ta.cellh,
			ElvFalse);
		if (o_synccursor)
			XSync(x_display, ElvFalse);
	}
	else
#endif
	XFillRectangle(x_display, xw->ta.win, xw->gc,
		(int)(xw->ta.cursx * xw->ta.cellw), (int)(xw->ta.cursy * xw->ta.cellh),
		(xw->ta.columns - xw->ta.cursx) * xw->ta.cellw, xw->ta.cellh);
}


void x_ta_event(xw, event)
	X11WIN	*xw;	/* top-level window which received event */
	XEvent	*event;	/* the event */
{
	int	x, y, x2, y2;
 static int	prevx, prevy;
 static Time	firstclick;
 static ELVBOOL	marking;
 	long	offset;

	switch (event->type)
	{
	  case Expose:
		x = event->xexpose.x / xw->ta.cellw;
		y = event->xexpose.y / xw->ta.cellh;
		x2 = (event->xexpose.x + event->xexpose.width - 1) / xw->ta.cellw;
		y2 = (event->xexpose.y + event->xexpose.height - 1) / xw->ta.cellh;
		eventexpose((GUIWIN *)xw, y, x, y2, x2);
		if (xw->ta.nextcursor != CURSOR_NONE)
			x_ta_drawcursor(xw);
		break;

	  case GraphicsExpose:
		x = event->xgraphicsexpose.x / xw->ta.cellw;
		y = event->xgraphicsexpose.y / xw->ta.cellh;
		x2 = (event->xgraphicsexpose.x + event->xgraphicsexpose.width - 1) / xw->ta.cellw;
		y2 = (event->xgraphicsexpose.y + event->xgraphicsexpose.height - 1) / xw->ta.cellh;
		eventexpose((GUIWIN *)xw, y, x, y2, x2);
		break;

	  case ButtonPress:
		/* Buttons 4 & 5 are always for scrolling */
		if (event->xbutton.button == 4)
		{
			if (o_scrollwheelspeed < 0)
				eventscroll((GUIWIN *)xw, SCROLL_FWDLN, -o_scrollwheelspeed, 0L);
			else
				eventscroll((GUIWIN *)xw, SCROLL_BACKLN, o_scrollwheelspeed, 0L);
			x_didcmd = ElvTrue;
			break;
		}
		if (event->xbutton.button == 5)
		{
			if (o_scrollwheelspeed < 0)
				eventscroll((GUIWIN *)xw, SCROLL_BACKLN, -o_scrollwheelspeed, 0L);
			else
				eventscroll((GUIWIN *)xw, SCROLL_FWDLN, o_scrollwheelspeed, 0L);
			x_didcmd = ElvTrue;
			break;
		}

		/* determine which character cell was clicked on. */
		y = event->xbutton.y / xw->ta.cellh;
		x = event->xbutton.x / xw->ta.cellw;

		/* Distinguish between single-click and double-click */
		if (x_now - firstclick > (Time)(o_dblclicktime * 100)
			|| event->xbutton.button == Button2
			|| prevy != y
			|| prevx != x)
		{
			/* single-click */

			/* Buttons 1 & 2 cancel any pending selection, but
			 * button 3 does not.
			 */
			if (event->xbutton.button != Button3)
			{
				(void)eventclick((GUIWIN *)xw, -1, -1, CLICK_CANCEL);
			}

			/* Buttons 1 & 3 move the cursor to the clicked-on cell.
			 * NOTE that button 2's "paste from clipboard" operation
			 * is implemented in the ButtonRelease event, because we
			 * don't know whether a ButtonPress is a click or the
			 * start of a drag.
			 */
			if (event->xbutton.button != Button2)
			{
				if (firstclick != 1
				 || prevy != y
				 || prevx != x)
				{
					offset = eventclick((GUIWIN *)xw, y, x, CLICK_MOVE);
					x_didcmd |= (ELVBOOL)(offset >= 0);
				}
				firstclick = x_now;
			}

			/* Button 3 also copies the selected text (if any) into
			 * the clipboard.
			 */
			if (event->xbutton.button == Button3)
			{
				(void)eventclick((GUIWIN *)xw, -1, -1, CLICK_YANK);
			}

			/* Remember some info about this event, to help us
			 * detect drags and double-clicks.
			 */
			prevy = y;
			prevx = x;
			x_didcmd = ElvTrue;
			/* paging = ElvFalse; !!! */
		}
		else
		{
			/* double-click */

			/* Perform either a ^] or ^T tag command.  Note that
			 * for ^], the cursor is already located at the
			 * clicked-on character cell, because the first click
			 * moved it there.
			 */
			(void)eventclick((GUIWIN *)xw, -1, -1,
				event->xbutton.button == Button1 ? CLICK_TAG : CLICK_UNTAG);

			/* There is no such thing as a triple-click.  Clobber
			 * the "firstclick" variable so we're sure to think
			 * the next click is a first click.
			 */
			firstclick = 1;
			x_didcmd = ElvTrue;
			/* paging = ElvFalse; !!! */
		}
		break;

	  case MotionNotify:
		/* convert to character cell coordinates */
		y = event->xbutton.y / xw->ta.cellh;
		x = event->xbutton.x / xw->ta.cellw;

		/* if not the same cell as last time... */
		if (y != prevy || x != prevx)
		{
			offset = eventclick((GUIWIN *)xw, y, x, CLICK_MOVE);
			if (offset >= 0)
			{
				/* If moved off original character, start marking */
				if (!marking &&
					0 <= eventclick((GUIWIN *)xw, prevy, prevx,
					  (event->xmotion.state & Button2Mask) ? CLICK_SELRECT :
					  (event->xmotion.state & Button3Mask) ? CLICK_SELLINE :
					  CLICK_SELCHAR))
				{
					(void)eventclick((GUIWIN *)xw, y, x, CLICK_MOVE);
					marking = ElvTrue;
				}
				prevy = y;
				prevx = x;
				x_didcmd = ElvTrue;
			}
		}
		break;

	  case ButtonRelease:
		if (marking)
		{
			y = event->xbutton.y / xw->ta.cellh;
			x = event->xbutton.x / xw->ta.cellw;
			if (y != prevy || x != prevx)
				(void)eventclick((GUIWIN *)xw, y, x, CLICK_MOVE);
			eventclick((GUIWIN *)xw, y, x, CLICK_YANK);
		}
		else /* if (!paging)*/
		{
			/* end of a click - depends on button */
			if (event->xbutton.state & Button2Mask)
			{
				/* paste from "< buffer */
				eventclick((GUIWIN *)xw, -1, -1, CLICK_PASTE);
				x_didcmd = ElvTrue;
			}
		}

		/* thumbing = ElvFalse; paging = ElvTrue; */
		marking = ElvFalse;
		break;
	}
}

/* Change the background color for a window */
void x_ta_setbg(xw, bg)
	X11WIN	*xw;	/* window whose text area needs a new background */
	long	bg;	/* the new background color */
{
	xw->ta.bg = bg;

# ifdef FEATURE_IMAGE
	if (xw->ta.pixmap != None)
	{
		XFreePixmap(x_display, xw->ta.pixmap);
		xw->ta.scrollpixels = 0;
		xw->ta.pixmap = None;
	}

	if (ispixmap(bg))
	{
		XSetWindowBackgroundPixmap(x_display, xw->win, pixmapof(bg).pixmap);
		XSetWindowBackgroundPixmap(x_display,xw->ta.win,ParentRelative);
	}
	else
# endif
	{
		XSetWindowBackground(x_display, xw->win, bg);
		XSetWindowBackground(x_display, xw->ta.win, bg);
	}
	XClearWindow(x_display, xw->win);
}
#endif
