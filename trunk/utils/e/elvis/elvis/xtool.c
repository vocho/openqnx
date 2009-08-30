/* xtool.c */

/* Copyright 1997 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_xtool[] = "$Id: xtool.c,v 2.20 2003/10/18 18:20:08 steve Exp $";
#endif
#ifdef GUI_X11
# include "guix11.h"

#define TB_BEVEL	2	/* bevel width for toolbar buttons */
#define TB_ADJACENT	2	/* gap for adjacent buttons */
#define TB_GAP		6	/* minumum gap for "gap" buttons */
#define TB_REDRAW	99	/* impossible bevel level, forces redraw */

/* This stores the definition of toolbar buttons */
typedef struct tool_s
{
	struct tool_s	*next;		/* another toolbar button */
	char		*label;		/* toolbar button legend */
	char		*excmd;		/* command to execute */
	CHAR		*when;		/* condition when button works */
	CHAR		*in;		/* condition when button drawn as "in" */
	CHAR		*comment;	/* one-line description of the button */
	char		*spec;		/* pop-up dialog specificaton */
	int		width;		/* width of button's label */
	int		id;		/* index into xw->toolstate[] */
	unsigned int	textx;		/* label's horiz offset within button */
	ELVBOOL		safer;		/* run with "safer" flag? */
	ELVBOOL		gap;		/* precede this option with a gap */
} TOOL;

#if USE_PROTOTYPES
static TOOL *findtool(char *label, ELVBOOL *isnew);
static void freetool(TOOL *tool);
static ELVBOOL istool(X11WIN *xw, TOOL *tool, int x, int y);
static void draw1tool(X11WIN *xw, TOOL *tool, int bevel);
static void addline(CHAR **retptr, char *label, _char_ op, CHAR *value);
#endif

static TOOL	*tools;		/* items in the toolbar */
static TOOL	*clicked;	/* a tool which has been clicked, not released */
static int	toolheight;	/* height of a button, including bevel */
static int	toolbase;	/* vertical offset of label within button */
static int	bevel;		/* appearance of clicked button */

/* This function searches for a named item in the toolbar, and returns it.
 * If it doesn't exist, this function creates it.
 */
static TOOL *findtool(label, isnew)
	char	*label;		/* toolbar legend */
	ELVBOOL	*isnew;		/* flag to set if new tool */
{
	TOOL	*tool, *lag;

	/* search for the legend */
	for (tool = tools, lag = NULL;
	     tool && strcmp(label, tool->label);
	     lag = tool, tool = tool->next)
	{
	}

	/* if not found, then allocate it */
	if (!tool)
	{
		/* allocate it */
		tool = (TOOL *)safekept(1, sizeof(TOOL));
		tool->label = safekdup(label);
		if (!lag)
			tool->id = 0;		/* first label */
		else if (strcmp(lag->label, label))
			tool->id = lag->id + 1;	/* next new label */
		else
			tool->id = lag->id;	/* subitem of same label */

		/* insert it into the list */
		if (lag)
		{
			tool->next = lag->next;
			lag->next = tool;
		}
		else
			tools = tool;

		/* This is a new tool */
		*isnew = ElvTrue;
	}
	else
	{
		/* This is an old tool */
		*isnew = ElvFalse;
	}

	return tool;
}

/* This function deletes a given item from the tools list, and then frees the
 * item's memory.  It also adjusts the id numbers of all later items from that
 * list.
 */
static void freetool(tool)
	TOOL	*tool;
{
	TOOL	*scan, *lag;

	/* locate it in the list */
	for (scan = tools, lag = NULL;
	     scan && scan != tool;
	     lag = scan, scan = scan->next)
	{
	}

	/* if found, then delete it from the list */
	if (scan)
	{
		/* maybe adjusts IDs of following items */
		if ((!lag || lag->id != scan->id) && scan->next && scan->next->id != scan->id)
		{
			while ((scan = scan->next) != NULL)
			{
				scan->id--;
			}
		}

		/* if this item had a gap, the following item inherits it */
		if (tool->gap && tool->next)
			tool->next->gap = ElvTrue;

		/* delete it from the list */
		if (lag)
			lag->next = tool->next;
		else
			tools = tool->next;
	}

	/* free its memory */
	safefree(tool->label);
	if (tool->excmd) safefree(tool->excmd);
	if (tool->when) safefree(tool->when);
	if (tool->in) safefree(tool->in);
	if (tool->comment) safefree(tool->comment);
	if (tool->spec) safefree(tool->spec);
}


/* Return ElvTrue if (x,y) is a point in the given button, on a given window */
static ELVBOOL istool(xw, tool, x, y)
	X11WIN	*xw;	/* a top-level app window */
	TOOL	*tool;	/* a tool to check */
	int	x, y;	/* a position to test against tool's position */
{
	if (x >= xw->tb.state[tool->id].x
	 && x < xw->tb.state[tool->id].x + tool->width
	 && y >= xw->tb.state[tool->id].y
	 && y < xw->tb.state[tool->id].y + toolheight
	 && xw->tb.state[tool->id].bevel != 0)
		return ElvTrue;
	return ElvFalse;
}


/* Draw a single toolbar button on a given window */
static void draw1tool(xw, tool, bevel)
	X11WIN	*xw;	/* a top-level app window */
	TOOL	*tool;	/* the tool to draw */
	int	bevel;	/* new bevel level */
{
	/* if same bevel level, then don't bother */
	if (bevel == xw->tb.state[tool->id].bevel)
		return;

	/* draw the button's bevel and face */
	x_drawbevel(xw, xw->tb.win,
		xw->tb.state[tool->id].x, xw->tb.state[tool->id].y,
		tool->width, toolheight, 'b', bevel);

	/* draw the button's label */
	XSetFont(x_display, xw->gc, x_loadedcontrol->fontinfo->fid);
	xw->fg = colorinfo[bevel == 0 ? x_toolbarcolors : x_toolcolors].fg;
	XSetForeground(x_display, xw->gc, xw->fg);
	if (bevel == 0)
	{
		x_drawstring(x_display, xw->tb.win, xw->gc,
			xw->tb.state[tool->id].x + tool->textx,
			xw->tb.state[tool->id].y + toolbase,
			tool->label, strlen(tool->label));
	}
	else
	{
		XDrawString(x_display, xw->tb.win, xw->gc,
			xw->tb.state[tool->id].x + tool->textx,
			xw->tb.state[tool->id].y + toolbase,
			tool->label, strlen(tool->label));
	}

	/* remember the changed the bevel level */
	xw->tb.state[tool->id].bevel = bevel;
}

static void addline(retptr, label, op, value)
	CHAR	**retptr;
	char	*label;
	_char_	op;
	CHAR	*value;
{
	/* if NULL value, then do nothing */
	if (!value || !*value)
		return;

	/* build a command */
	buildstr(retptr, " gui ");
	buildstr(retptr, label);
	buildCHAR(retptr, op);
	for (; *value; value++)
		buildCHAR(retptr, *value);
	/* Note that we don't need an explicit '\n' added here, because all of
	 * the value strings end with '\n' already.
	 */
}

/* This returns a dynamically-allocated string describing the attributes
 * of a tool, or all tools.  The caller must free it via safefree() when
 * the string is no longer needed.  Returns NULL if error or there are no
 * tools.
 */
CHAR *x_tb_dump(label)
	char	*label;	/* label of tool to dump, or NULL for all tools */
{
	CHAR	*ret = NULL;
	TOOL	*tool;

	/* locate it in the list */
	tool = tools;
	if (label && *label)
	{
		for (; tool && strcmp(tool->label, label); tool = tool->next)
		{
		}
	}

	/* build the string */
	for (; tool; tool = (label && *label) ? NULL : tool->next)
	{
		if (tool->gap && !(label && *label))
			buildstr(&ret, " gui gap\n");
		addline(&ret, tool->label, ':', toCHAR(tool->excmd));
		addline(&ret, tool->label, '?', tool->when);
		addline(&ret, tool->label, '=', tool->in);
		addline(&ret, tool->label, '"', tool->comment);
		addline(&ret, tool->label, ';', toCHAR(tool->spec));
	}

	/* return it */
	return ret;
}

/* This function creates a toolbar window of a given width.  */
void x_tb_predict(xw, w, h)
	X11WIN	 *xw;
	unsigned w, h;	/* width of the toolbar (height is ignored) */
{
	TOOL	 *first;/* first tool on current line */
	TOOL	 *scan;	/* for stepping through tools on current line */
	int	 fit;	/* number of tools that fit on current line */
	int	 wt;	/* total widths of tool labels on currnet line */
	int	 gfit;	/* number of tools IN WHOLE GROUPS that fit */
	int	 gwt;	/* total widths of tools IN WHOLE GROUPS that fit */
	int	 gaps;	/* number of gaps on current line, not counting first */
	int	 gsz;	/* expanded size of gaps in this line */
	int	 dummy;
	XCharStruct size;

	/* if no tools, or "set notoolbar", then toolbar height is always 0 */
	if (!tools || !o_toolbar)
	{
		xw->tb.h = 0;
		xw->tb.w = w;
		xw->tb.win = None;
		return;
	}

	/* compute some info about the font */
	dummy = x_loadedcontrol->fontinfo->ascent
		+ x_loadedcontrol->fontinfo->descent + 2 * TB_BEVEL;
	if (toolheight != dummy)
	{
		toolheight = dummy;

		/* widths are probably wrong, too */
		for (scan = tools; scan; scan = scan->next)
		{
			scan->width = 0;
		}
	}
	toolbase = x_loadedcontrol->fontinfo->max_bounds.ascent + TB_BEVEL;

	/* if any tool lacks a "width" field, then compute it now */
	for (scan = tools; scan; scan = scan->next)
	{
		if (!scan->width)
		{
			XTextExtents(x_loadedcontrol->fontinfo,
				scan->label, strlen(scan->label),
				&dummy, &dummy, &dummy, &size);
			scan->width = size.rbearing - size.lbearing + 2 * TB_BEVEL + 2;
			scan->textx = TB_BEVEL + 1 - size.lbearing;
				/* Note: size.lbearing is non-positive number,
				 * so it actually increases to the scan->textx
				 * and scan->width values.
				 */
		}
	}

	/* the first line of toolbar buttons will begin 1 raster line below
	 * the top of the toolbar window.
	 */
	h = 1;

	/* for each line... */
	for (first = tools; first; first = scan)
	{
		/* see how many fit on the current line */
		for (gwt = wt = 2, scan = first, gfit = fit = 0, gaps = 0;
		     scan && (unsigned)(wt + scan->width + (scan->gap ? TB_GAP : TB_ADJACENT)) < w;
		     scan = scan->next)
		{
			fit++;
			wt += scan->width + (scan->gap ? TB_GAP : TB_ADJACENT);
			if (scan != first && scan->gap)
				gaps++;
			if (!scan->next || scan->next->gap)
				gfit = fit, gwt = wt;
		}

		/* adjust gap size, if there are any gaps */
		if (gaps == 0)
		{
			gfit = fit, gwt = wt;
			gsz = TB_ADJACENT;
		}
		else
		{
			gwt -= (TB_GAP - TB_ADJACENT) * gaps;
			gsz = (w - gwt) / gaps;
		}

		/* select a position for each button that fits on this line */
		for (scan = first, wt = 1; scan && --gfit >= 0; scan = scan->next)
		{
			if (scan->gap && wt > 1)
				wt += gsz;
			xw->tb.state[scan->id].x = wt;
			xw->tb.state[scan->id].y = h;
			wt += scan->width + TB_ADJACENT;
		}

		/* increment the vertical offset for next line */
		h += toolheight + 1;
	}

	/* At this point, "h" is the height needed for the toolbar window */
	xw->tb.w = w;
	xw->tb.h = h;
}


void x_tb_create(xw, x, y)
	X11WIN	 *xw;	/* top-level application window */
	int	 x, y;	/* position of the toolbar within the top-level window */
{
	/* if disabled or empty, then don't create it */
	if (xw->tb.h == 0)
		return;

	/* create the window */
#ifdef FEATURE_IMAGE
	if (ispixmap(colorinfo[x_toolbarcolors].bg))
	{
		xw->tb.win = XCreateSimpleWindow(x_display, xw->win,
			x, y, xw->tb.w, xw->tb.h, 0,
			colorinfo[x_toolbarcolors].fg,
			pixmapof(colorinfo[x_toolbarcolors].bg).pixel);
		XSetWindowBackgroundPixmap(x_display, xw->tb.win,
			pixmapof(colorinfo[x_toolbarcolors].bg).pixmap);
	}
	else
#endif
	{
		xw->tb.win = XCreateSimpleWindow(x_display, xw->win,
			x, y, xw->tb.w, xw->tb.h, 0,
			colorinfo[x_toolbarcolors].fg,
			colorinfo[x_toolbarcolors].bg);
		if (x_mono || colorinfo[x_toolbarcolors].fg == colorinfo[x_toolbarcolors].bg)
			XSetWindowBackgroundPixmap(x_display, xw->tb.win, x_gray);
	}
	XSelectInput(x_display, xw->tb.win,
		ButtonPressMask|ButtonMotionMask|ButtonReleaseMask|ExposureMask);

	/* remember its size and position */
	xw->tb.x = 0;
	xw->tb.y = y;
}

/* Destroy the toolbar for a window */
void x_tb_destroy(xw)
	X11WIN	*xw;	/* top-level application window to loose the toolbar */
{
	/* destroy the window */
	if (xw->tb.win != None)
		XDestroyWindow(x_display, xw->tb.win);
	xw->tb.win = None;
}


void x_tb_draw(xw, fromscratch)
	X11WIN	*xw;		/* window whose toolbar should be drawn */
	ELVBOOL	fromscratch;	/* redraw background too? */
{
	TOOL	*tool, *lag;
	int	newstate;
	CHAR	*str;

	/* if no toolbar is visible, then don't bother */
	if (xw->tb.h == 0)
	{
		return;
	}

	/* if recolored, then adjust background and redraw from scratch */
	if (xw->tb.recolored)
	{
#ifdef FEATURE_IMAGE
		if (ispixmap(colorinfo[x_toolbarcolors].bg))
			XSetWindowBackgroundPixmap(x_display, xw->tb.win,
				pixmapof(colorinfo[x_toolbarcolors].bg).pixmap);
		else
#endif
		{
			XSetWindowBackground(x_display, xw->tb.win,
				colorinfo[x_toolbarcolors].bg);
			if (x_mono || colorinfo[x_toolbarcolors].fg == colorinfo[x_toolbarcolors].bg)
				XSetWindowBackgroundPixmap(x_display, xw->tb.win, x_gray);
		}
		fromscratch = ElvTrue;
		xw->tb.recolored = ElvFalse;

		/* This also affects the appearance of any dialogs for this
		 * window.  We need to redraw those windows in the new colors.
		 */
		x_dl_docolor(xw);
	}

	/* supposed to draw from scratch? */
	if (fromscratch)
	{
		/* draw the background */
		XClearWindow(x_display, xw->tb.win);

		/* reset the states for all buttons */
		for (tool = tools; tool; tool = tool->next)
			xw->tb.state[tool->id].bevel = TB_REDRAW;
	}

	/* make sure this window is the "current" window.  Otherwise the
	 * calculate() calls below will be looking at the wrong window's
	 * options.
	 */
	(void)eventfocus((GUIWIN *)xw, ElvFalse);

	/* for each button... */
	for (lag = NULL, tool = tools; tool; lag = tool, tool = tool->next)
	{
		/* compute the button's new state */
		newstate = TB_REDRAW;
		if (tool->excmd == NULL && tool->spec == NULL)
		{
			newstate = 0; /* flat, inactive */
		}
		else if (tool->when)
		{
			str = calculate(tool->when, NULL, CALC_ALL);
			if (!str)
			{
				/* error - disable the button semi-permanently */
				safefree(tool->when);
				str = toCHAR("0");
				tool->when = CHARdup(str);
			}
			if (!calctrue(str))
				newstate = 0; /* flat, inactive */
		}
		if (newstate != 0 && !tool->in)
		{
			newstate = TB_BEVEL; /* out, active */
		}
		else if (newstate != 0)
		{
			str = calculate(tool->in, NULL, CALC_ALL);
			if (!str)
			{
				/* error - forget condition, always display out */
				safefree(tool->in);
				tool->in = NULL;
				newstate = TB_BEVEL; /* out, active */
			}
			else if (calctrue(str))
				newstate = -TB_BEVEL; /* in, inactive */
			else
				newstate = TB_BEVEL; /* out, active */
		}

		/* update the button's appearance */
		draw1tool(xw, tool, newstate);

		/* if the button is preceeded by a gap, draw a dimple in gap */
		if (tool->gap && xw->tb.state[tool->id].x > 5)
		{
			x_drawbevel(xw, xw->tb.win, 
				(xw->tb.state[lag->id].x + lag->width + xw->tb.state[tool->id].x) / 2 - 1,
				xw->tb.state[tool->id].y + toolheight / 2,
				3, 2, 'b', -1);
		}
	}
}


/* Handle an event on the toolbar */
void x_tb_event(xw, event)
	X11WIN	*xw;	/* top-level application window */
	XEvent	*event;	/* event received by xw's toolbar */
{
	TOOL	*scan;

	switch (event->type)
	{
	  case ButtonPress:
		/* find the clicked button, if any */
		for (scan = tools, clicked = NULL;
		     scan && !clicked;
		     scan = scan->next)
		{
			if(istool(xw, scan, event->xbutton.x, event->xbutton.y))
			{
				clicked = scan;
				bevel = xw->tb.state[scan->id].bevel;
			}
		}

		if (clicked)
		{
			/* redraw it with reduced bevel */
			draw1tool(xw, clicked, (bevel > 0 ? 1 : -1));
			if (clicked->comment)
				x_st_info(xw, clicked->comment);
			XFlush(x_display);
		}
		break;

	  case MotionNotify:
		/* if nothing clicked, then ignore it */
		if (!clicked)
			break;

		/* redraw the button with either its full bevel or a reduced
		 * bevel, depending on whether the pointer is still on it.
		 */
		if (istool(xw, clicked, event->xmotion.x, event->xmotion.y))
		{
			draw1tool(xw, clicked, (bevel > 0 ? 1 : -1));
			if (clicked->comment)
				x_st_info(xw, clicked->comment);
		}
		else
		{
			draw1tool(xw, clicked, bevel);
			x_st_info(xw, NULL);
		}
		XFlush(x_display);
		break;

	  case ButtonRelease:
		/* if nothing clicked, then ignore it */
		if (!clicked)
			break;

		/* Hide the one-line comment (if any) */
		x_st_info(xw, NULL);

		/* if not the same as the clicked button, then ignore it */
		if (!istool(xw, clicked, event->xbutton.x, event->xbutton.y))
		{
			draw1tool(xw, clicked, bevel);
			clicked = NULL;
			XFlush(x_display);
			break;
		}

		/* draw the tool in its reversed state, briefly */
		draw1tool(xw, clicked, -xw->tb.state[clicked->id].bevel);
		XFlush(x_display);

		/* Is this button supposed to use a dialog window? */
		if (clicked->spec)
		{
			/* Create the dialog window.  If/when the user hits
			 * the [Submit] button, the excmd will be executed.
			 */
			x_dl_add(xw, clicked->label, clicked->comment ? tochar8(clicked->comment) : NULL,
				clicked->excmd, clicked->spec);
		}
		else if (clicked->excmd)
		{
			/* execute its ex command */
			eventex((GUIWIN *)xw, clicked->excmd, clicked->safer);
		}

		x_didcmd = ElvTrue;
		break;

	  case Expose:
		if (event->xexpose.count == 0)
			x_tb_draw(xw, ElvTrue);
		break;
	}
}

/* Configure the toolbar.
 *
 * NOTE: After calling this function, you will need to create the widgets for
 * each window, sort of like you need to do after a resize.  This is because
 * the height of the toolbar may change, so the positions of the other widgets
 * may change.
 *
 * Returns ElvTrue if the screens all need to be reconfigured, ElvFalse otherwise.
 */
ELVBOOL x_tb_config(gap, label, op, value)
	ELVBOOL	gap;	/* insert a gap before this item? */
	char	*label;	/* name of a toolbar button, or NULL to delete all */
	_char_	op;	/* :excmd, ?enable, =pushedin, or '~' to delete */
	char	*value;	/* an ex command or an expression, depending on op */
{
	TOOL	*tool;
	ELVBOOL	changed;/* do we need to reconfigure after this? */

	/* if no label was given, then clobber all tools */
	if (!label)
	{
		changed = (ELVBOOL)(tools && o_toolbar);
		while (tools)
		{
			freetool(tools);
		}
		return changed;
	}

	/* find the tool in question */
	tool = findtool(label, &changed);

	/* If the "gap" argument is ElvTrue, then set the tool's "gap" flag */
	if (!changed && tool->gap != gap)
		changed = ElvTrue;
	tool->gap |= gap;

	/* If the value is an empty string, treat it as no value */
	if (value && !*value)
		value = NULL;

	/* apply the operator to the tool */
	switch (op)
	{
	  case ':':
		if (tool->excmd) safefree(tool->excmd);
		tool->excmd = (value ? safedup(value) : NULL);
		break;

	  case '?':
		if (tool->when) safefree(tool->when);
		tool->when = (value ? CHARdup(toCHAR(value)) : NULL);
		break;

	  case '=':
		if (tool->in) safefree(tool->in);
		tool->in = (value ? CHARdup(toCHAR(value)) : NULL);
		break;

	  case '"':
	  	if (tool->comment) safefree(tool->comment);
		tool->comment = (value ? CHARdup(toCHAR(value)) : NULL);
		break;

	  case ';':
		if (tool->spec) safefree(tool->spec);
		tool->spec = (value ? safedup(value) : NULL);
		break;

	  case '~':
	  	freetool(tool);
	  	changed = ElvTrue;
	  	break;
	}

	return changed;
}

void x_tb_recolor(xw)
	X11WIN	*xw;	/* window to be recolored */
{
	/* Arrange for a complete redraw later */
	xw->tb.recolored = ElvTrue;
}
#endif
