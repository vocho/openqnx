/* xdialog.c */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_xdialog[] = "$Id: xdialog.c,v 2.32 2003/10/18 18:20:08 steve Exp $";
#endif
#ifdef GUI_X11
#include "guix11.h"

#define NUMBER_LENGTH	10
#define STRING_LENGTH	40


dialog_t	*x_dialogs;

static X_BUTTON *addbutton P_((dialog_t *dia, char *label, _char_ shape, int key));
static void drawbutton P_((dialog_t *dia, X_BUTTON *btn));
static X_BUTTON *findbutton P_((dialog_t *dia, int x, int y));
static void drawtext P_((dialog_t *dia, int row));
static void parsespec P_((dialog_t *dia));
static void makecurrent P_((dialog_t *dia, int row));
static CHAR *keyoneof P_((dialog_t *dia, int key));
static CHAR *keystring P_((dialog_t *dia, int key));
static void keystroke P_((dialog_t *dia, int key));
static void exposerow P_((dialog_t *dia, int row, ELVBOOL fromscratch));
static void expose P_((dialog_t *dia));


/* allocate a new button, setting all attributes except x & y position */
static X_BUTTON *addbutton(dia, label, shape, key)
	dialog_t *dia;
	char	*label;	/* label to show on key face */
	_char_	shape;	/* shape of button -- usually 'b' */
	int	key;	/* keystroke to simulate if mouse released on button */
{
	X_BUTTON	*btn;
	XCharStruct	size;
	int		dummy;
	int		controlheight;

	/* allocate storage for the button */
	btn = (X_BUTTON *)safealloc(1, sizeof(X_BUTTON));
	btn->next = dia->button;
	dia->button = btn;

	/* height of the control font */
	controlheight = x_loadedcontrol->fontinfo->ascent + x_loadedcontrol->fontinfo->descent;

	/* size depends on the shape */
	btn->shape = shape;
	btn->state = 2;
	btn->key = key;
	btn->label = label;
	for (btn->lablen = 0; btn->label[btn->lablen] > ' '; btn->lablen++)
	{
	}
	if (shape == 'b')
	{
		/* rectangular - size depends on label */
		btn->h = controlheight + 4;
		XTextExtents(x_loadedcontrol->fontinfo, label, btn->lablen,
			&dummy, &dummy, &dummy, &size);
		btn->w = size.rbearing - size.lbearing + 6;
		btn->textx = 3 - size.lbearing;
		btn->texty = x_loadedcontrol->fontinfo->ascent + 2;
	}
	else
	{
		/* arrowhead - size is square, same size input field's height */
		btn->h = btn->w = x_defaultnormal->fontinfo->ascent + x_defaultnormal->fontinfo->descent + 2;
	}

	/* that's all */
	return btn;
}


/* draw a button */
static void drawbutton(dia, btn)
	dialog_t *dia;
	X_BUTTON *btn;
{
	X11WIN	dummy;

	/* Make a dummy X11WIN structure.  I wish now that I hadn't made it
	 * an argument to the x_drawbevel() function.  Sigh.
	 */
	memset(&dummy, 0, sizeof dummy);
	dummy.gc = dia->gc;

	/* If this is the default button, draw a recessed box around it */
	if (btn->key == '\r')
	{
		x_drawbevel(&dummy, dia->win,
			btn->x - 4, btn->y - 4,
			btn->w + 8, btn->h + 8, 'b', -1);
	}

	/* draw the bevel of the button */
	x_drawbevel(&dummy, dia->win,
		btn->x, btn->y, btn->w, btn->h, btn->shape, btn->state);

	/* draw the label (except on arrow buttons) */
	if (btn->shape == 'b' && btn->lablen > 0)
	{
		XSetForeground(x_display, dia->gc, colorinfo[x_toolcolors].fg);
		XSetFont(x_display, dia->gc, x_loadedcontrol->fontinfo->fid);
		XDrawString(x_display, dia->win, dia->gc,
			btn->x + btn->textx, btn->y + btn->texty,
			btn->label, btn->lablen);
	}
}


/* search through the list for a button containing a given (x,y) position */
static X_BUTTON *findbutton(dia, x, y)
	dialog_t *dia;
	int	 x, y;
{
	X_BUTTON *scan;

	for (scan = dia->button; scan; scan = scan->next)
	{
		if (scan->x <= x && x < scan->x + scan->w
		 && scan->y <= y && y < scan->y + scan->h)
		{
			return scan;
		}
	}
	return NULL;
}



/* Draw a text field */
static void drawtext(dia, row)
	dialog_t *dia;
	int	 row;
{
	X11WIN	dummy;
	int	cursor;
	int	shift;
	int	length;
	int	x, y;

	/* choose shifting amounts */
	if (row == dia->current)
		cursor = dia->cursor, shift = dia->shift;
	else
		cursor = CHARlen(dia->field[row].value), shift = 0;
	if (dia->field[row].ft == FT_NUMBER)
		length = NUMBER_LENGTH;
	else
		length = STRING_LENGTH;
	if (shift > cursor)
		shift = cursor;
	else if (shift < cursor - length)
		shift = cursor - length;
	x = CHARlen(dia->field[row].value) - shift;
	if (length > x)
		length = x;

	/* if this is the current row, then remember that shifting amount */
	if (row == dia->current)
		dia->shift = shift;

	/* all fields except locked ones are drawn in a recessed bevel */
	x = dia->x0 + dia->rowh + 2;
	y = dia->y0 + row * dia->rowh;
	if (dia->field[row].ft != FT_LOCKED)
	{
		/* Make a dummy X11WIN structure.  I wish now that I hadn't
		 * made it an argument to the x_drawbevel() function.  Sigh.
		 */
		memset(&dummy, 0, sizeof dummy);
		dummy.gc = dia->gc;

		/* draw the bevel of the text area */
		x_drawbevel(&dummy, dia->win,
			x, y, dia->field[row].twidth, dia->rowh - 3, 'b', -1);
	}

	/* draw the text itself */
	if (length > 0)
	{
#ifdef FEATURE_IMAGE
		if (ispixmap(colorinfo[x_toolcolors].bg))
			XSetBackground(x_display, dia->gc,
				pixmapof(colorinfo[x_toolcolors].bg).pixel);
		else
#endif
			XSetBackground(x_display, dia->gc,
				colorinfo[x_toolcolors].bg);
		XSetFont(x_display, dia->gc, x_defaultnormal->fontinfo->fid);
		if (dia->field[row].ft == FT_LOCKED)
		{
			XSetForeground(x_display, dia->gc, colorinfo[x_toolbarcolors].fg);
			x_drawstring(x_display, dia->win, dia->gc,
				x + 2, y + x_defaultnormal->fontinfo->ascent + 1,
				tochar8(dia->field[row].value + shift), length);
		}
		else
		{
			XSetForeground(x_display, dia->gc, colorinfo[x_toolcolors].fg);
			XDrawImageString(x_display, dia->win, dia->gc,
				x + 2, y + x_defaultnormal->fontinfo->ascent + 1,
				tochar8(dia->field[row].value + shift), length);
		}
	}

	/* if this is the current row, then draw the cursor too. */
	if (row == dia->current && dia->field[row].ft != FT_LOCKED)
	{
		XSetForeground(x_display, dia->gc, colorinfo[x_cursorcolors].fg);
		XFillRectangle(x_display, dia->win, dia->gc,
			x + 2 + (cursor - shift) * dia->cellw, y + 1,
			2, x_defaultnormal->fontinfo->ascent + x_defaultnormal->fontinfo->descent - 1);
	}
}


/* Parse the "spec" field for a dialog, and build the "field" array. */
static void parsespec(dia)
	dialog_t *dia;
{
 static char	truefalse[20] = "true false";
	char	*scan;	/* used for scanning through the spec string */
	char	*label;	/* label of next field */
	char	*name;	/* name of next field's option */
	X_FIELDTYPE ft;	/* type of next field */
	char	*limit;	/* list for FT_ONEOF, or min:max for FT_NUMBER */
	char	*expr;	/* default value -- an expression */
	char	*value;	/* the actual value, as a string */
	char	*end;	/* end of the spec string */
	OPTDESC *desc;	/* description of the option for a given row */
	X_FIELD	*nf;	/* new field array */
	char	skipto;	/* end marker for current segment, or '\0' */

	/* Make the associated text window be the default window, so that if
	 * we fetch a window-dependent option's value, we'll be fetching it
	 * from the correct window.
	 */
	eventfocus((GUIWIN *)dia->xw, ElvFalse);

	/* Use the locale-sensitive names for the Boolean values */
	sprintf(truefalse, "%s %s", o_true, o_false);

	/* Parse the string.  Since dia->spec is just a local copy of the
	 * original string, we can insert '\0' characters where convenient.
	 */
	label = name = limit = expr = NULL;
	ft = FT_DEFAULT;
	skipto = '\0';
	for (scan = dia->spec, end = &scan[strlen(scan) + 1]; scan < end; scan++)
	{
		/* skipping to the end of a segment? */
		if (*scan && *scan != ';' && skipto)
		{
			if (*scan != skipto)
				continue;
			*scan = '\0';
			if (skipto != ';')
			{
				skipto = '\0';
				continue;
			}
		}
		skipto = '\0';

		/* between fields, or at the start of a field */
		switch (*scan)
		{
		  case ';':
		  case '\0':
			/* mark the end of the last segment */
			*scan = '\0';

			/* end of a row's info -- is there a valid name? */
			if (name && (value = tochar8(optgetstr(toCHAR(name), &desc))) != NULL)
			{
				/* if no explicit type, then guess */
				if (ft == FT_DEFAULT)
				{
					limit = desc->limit;
					if (optval(name)->flags & OPT_LOCK)
						ft = FT_LOCKED;
					else if (desc->asstring == optnstring)
						ft = FT_NUMBER;
					else if (desc->asstring == optsstring)
						ft = FT_STRING;
					else if (desc->asstring == opt1string)
						ft = FT_ONEOF;
					else if (desc->asstring == opttstring)
					{
						/* tab list: default is to
						 * treat as number if single
						 * value, else treat as string
						 */
						if (CHARchr(value, ',')
						 || atoi(tochar8(value)) == 0)
							ft = FT_STRING;
						else
							ft = FT_NUMBER;
						limit = "1:400";
					}
					else
					{
						ft = FT_ONEOF;
						limit = truefalse;
					}
				}

				/* evaluate the expr, if any */
				if (expr)
					expr = tochar8(calculate(toCHAR(expr), NULL, CALC_ALL));
				if (expr)
					value = expr;

				/* for boolean, force value to "true"/"false" */
				if (ft == FT_ONEOF && limit == truefalse)
				{
					value = tochar8(calctrue(toCHAR(value)) ? o_true : o_false);
				}

				/* if no explicit label, then use the option's
				 * long name.
				 */
				if (!label)
					label = desc->longname;

				/* reallocate the field array */
				nf = (X_FIELD *)safealloc(dia->nfields + 1, sizeof(X_FIELD));
				if (dia->field)
				{
					memcpy(nf, dia->field, dia->nfields * sizeof(X_FIELD));
					safefree(dia->field);
				}
				dia->field = nf;

				/* store the new field */
				dia->field[dia->nfields].label = toCHAR(label);
				dia->field[dia->nfields].name = desc->longname;
				dia->field[dia->nfields].value = CHARdup(toCHAR(value));
				dia->field[dia->nfields].limit = limit;
				dia->field[dia->nfields].ft = ft;
				dia->nfields++;
			}
			else if (expr) /* explicit value, but no option name */
			{
				if (expr)
					value = tochar8(calculate(toCHAR(expr), NULL, CALC_ALL));
				if (value)
				{
					if (!label)
						label = " ";

					/* reallocate the field array */
					nf = (X_FIELD *)safealloc(dia->nfields + 1, sizeof(X_FIELD));
					if (dia->field)
					{
						memcpy(nf, dia->field, dia->nfields * sizeof(X_FIELD));
						safefree(dia->field);
					}
					dia->field = nf;

					/* store the field */
					dia->field[dia->nfields].label = toCHAR(label);
					dia->field[dia->nfields].name = " ";
					dia->field[dia->nfields].value = CHARdup(toCHAR(value));
					dia->field[dia->nfields].limit = limit;
					dia->field[dia->nfields].ft = FT_LOCKED;
					dia->nfields++;
				}
			}

			/* prepare for next field */
			label = name = limit = expr = NULL;
			ft = FT_DEFAULT;
			break;

		  case '(':
			/* the following character indicates the type */
			switch (scan[1])
			{
			  case 'b': ft = FT_ONEOF, limit = truefalse;	break;
			  case 'o': ft = FT_ONEOF, limit = scan;	break;
			  case 'n': ft = FT_NUMBER, limit = scan;	break;
			  case 's': ft = FT_STRING;			break;
			  case 'f': ft = FT_FILE;			break;
			  case 'l': ft = FT_LOCKED;			break;
			}

			/* trim leading spaces from the limit string */
			if (limit && limit != truefalse)
			{
				while (*limit && !elvspace(*limit) && *limit != ')')
					limit++;
				while (elvspace(*limit))
					limit++;
			}
			if (limit && *limit == ')')
				limit = NULL;

			/* skip chars up through the ')' */
			skipto = ')';
			break;

		  case '"':
			/* the label starts after the quote */
			label = &scan[1];

			/* it ends at the next quote */
			skipto = '"';
			break;

		  case '=':
		  	/* just in case the option name ends here, stuff a
		  	 * '\0' where the '=' was.
		  	 */
		  	*scan = '\0';

			/* the expression starts after the '=' */
			expr = &scan[1];

			/* it ends at the next semicolon */
			skipto = ';';
			break;

		  default:
			if (elvalpha(*scan) && !name)
			{
				/* if followed by whitespace, then skip to
				 * the whitespace.
				 */
				for (name = &scan[1]; elvalnum(*name); name++)
				{
				}
				if (elvspace(*name))
					skipto = *name;

				/* the option name starts here */
				name = scan;

			}
		} /* end of switch(*scan) */
	} /* next "scan" character */
}


/* create a new dialog */
void x_dl_add(xw, name, desc, excmd, spec)
	X11WIN	*xw;	/* window where the command should be performed */
	char	*name;	/* name of the toolbar button */
	char	*desc;	/* one-line description of the "submit" action */
	char	*excmd;	/* the command to execute if "submit" is pressed */
	char	*spec;	/* list of options to use as fields */
{
	dialog_t	*dia;
	unsigned	h, w, fieldw;
	XCharStruct	size;
	XGCValues	gcvalues;
	int		dummy;
	XSizeHints	hints;
	int		i, x;
	X_BUTTON	*button;
	char		*s;
	int		bdelta;
	X_LOADEDFONT	*titlefont;
	int		controlheight;

	/* compute the height of the control font */
	controlheight = x_loadedcontrol->fontinfo->ascent + x_loadedcontrol->fontinfo->descent;

	/* allocate a struct for the new dialog */
	dia = (dialog_t *)safealloc(1, sizeof(dialog_t));
	dia->xw = xw;
	dia->name = safedup(name);
	dia->desc = safedup(desc ? desc : name);
	dia->excmd = excmd ? safedup(excmd) : NULL;
	dia->spec = safedup(spec);
	dia->current = -2;

	/* the "desc" string may contain a newline.  If so, delete it */
	i = strlen(dia->desc) - 1;
	if (i >= 0 && dia->desc[i] == '\n')
		dia->desc[i] = '\0';

	/* compute the heading size */
	titlefont = (x_defaultbold ? x_defaultbold : x_defaultnormal);
	XTextExtents(titlefont->fontinfo, name, strlen(name),
		&dummy, &dummy, &dummy, &size);
	w = size.rbearing - size.lbearing;
	XTextExtents(x_loadedcontrol->fontinfo, dia->desc, strlen(dia->desc),
		&dummy, &dummy, &dummy, &size);
	if ((unsigned)(size.rbearing - size.lbearing) > w)
		w = size.rbearing - size.lbearing;
	w += x_elvis_icon_width + 6;
	h = titlefont->fontinfo->ascent + titlefont->fontinfo->descent + controlheight;
	if (h < x_elvis_icon_height)
		h = x_elvis_icon_height;
	dia->w = w;
	dia->h = h + 4;

	/* DO THE FIELD STUFF */

	/* compute the row height, label base, and button delta */
	dia->h += 8;
	dia->x0 = 0;
	dia->y0 = dia->h;
	dia->rowh = x_defaultnormal->fontinfo->ascent + x_defaultnormal->fontinfo->descent + 2;
	bdelta = 0;
	if (controlheight + 4 > dia->rowh)
		dia->rowh = controlheight + 4;
	else
		bdelta = (dia->rowh - (controlheight + 4)) / 2;
	dia->base = x_loadedcontrol->fontinfo->ascent + bdelta;
	dia->rowh += 3;

	/* compute the cell width */
	dia->cellw = x_defaultnormal->fontinfo->max_bounds.width;

	/* parse the row info */
	parsespec(dia);

	/* compute label widths */
	for (i = 0, w = 0; i < dia->nfields; i++)
	{
		XTextExtents(x_loadedcontrol->fontinfo,
			tochar8(dia->field[i].label), CHARlen(dia->field[i].label),
			&dummy, &dummy, &dummy, &size);
		dia->field[i].lwidth = size.rbearing - size.lbearing;
		if (dia->field[i].lwidth > w)
			w = dia->field[i].lwidth;
	}
	dia->x0 = w + 3;

	/* compute the button positions, and the input widths */
	for (i = 0, w = 0; i < dia->nfields; i++)
	{
		x = dia->x0 + dia->rowh + 2;
		switch (dia->field[i].ft)
		{
		  case FT_ONEOF:
			for (s = dia->field[i].limit; *s; s++)
			{
				if (s == dia->field[i].limit
				 || (elvspace(s[-1]) && !elvspace(s[0])))
				{
					button = addbutton(dia, s, 'b', *s);
					button->y = dia->y0 + dia->rowh * i + bdelta;
					button->x = x;
					x += button->w + 2;
				}
			}
			break;

		  case FT_STRING:
		  case FT_FILE:
			button = addbutton(dia, "<", 'l', ELVCTRL('L'));
			button->y = dia->y0 + dia->rowh * i;
			button->x = dia->x0 + 3;

			dia->field[i].twidth = STRING_LENGTH * dia->cellw + 4;
			x += dia->field[i].twidth + 2;

			button = addbutton(dia, ">", 'r', ELVCTRL('R'));
			button->y = dia->y0 + dia->rowh * i;
			button->x = x;
			x += dia->rowh;
			break;

		  case FT_LOCKED:
			dia->field[i].twidth = CHARlen(dia->field[i].value) * dia->cellw + 4;
			x += dia->field[i].twidth + 2 + dia->rowh;
			break;

		  case FT_NUMBER:
			dia->field[i].twidth = NUMBER_LENGTH * dia->cellw + 4;
			x += dia->field[i].twidth + 2;

			button = addbutton(dia, "+", 'b', '+');
			button->y = dia->y0 + dia->rowh * i + bdelta;
			button->x = x;
			x += button->w + 2;

			button = addbutton(dia, "-", 'b', '-');
			button->y = dia->y0 + dia->rowh * i + bdelta;
			button->x = x;
			x += button->w + 2;
			break;

		  default:
			;
		}

		/* if widest seen yet, then remember that */
		if (x + 3 > (int)w)
			w = x + 3;
	}
	fieldw = w;

	/* incorporate the row size into the overall dialog size */
	h = dia->nfields * dia->rowh + 1;
	dia->h += h;
	if (w > dia->w)
		dia->w = w;

	/* compute the submit/cancel size and position */
	dia->submit = addbutton(dia, tochar8(o_submit), 'b', '\r');
	dia->cancel = addbutton(dia, tochar8(o_cancel), 'b', '\033');
	dia->submit->y = dia->cancel->y = dia->h + (dia->nfields > 0 ? 9 : 4);
	h = dia->submit->h + (dia->nfields > 0 ? 13 : 8);
	w = dia->submit->w + dia->cancel->w + 14;
	dia->h += h + 2;
	if (dia->w < w)
		dia->w = w;
	dia->cancel->x = dia->w - (dia->cancel->w + 2);
	dia->submit->x = dia->cancel->x - (dia->submit->w + 10);

	/* if the width of the fields is narrower than the window width,
	 * then center the fields within the window
	 */
	if (fieldw < dia->w)
	{
		fieldw = (dia->w - fieldw) / 2;
		dia->x0 += fieldw;
		for (button = dia->button; button; button = button->next)
		{
			if (button == dia->submit || button == dia->cancel)
				continue;
			button->x += fieldw;
		}
	}

	/* store the chosen size in the "hints" structure */
	hints.x = xw->x + (int)(xw->w - dia->w) / 2;
	if (hints.x < 0)
		hints.x = 0;
	hints.y = xw->y + (int)(xw->h - dia->h) / 2;
	if (hints.y < 0)
		hints.y = 0;
	hints.width = hints.min_width = hints.max_width = dia->w;
	hints.height = hints.min_height = hints.max_height = dia->h;
	hints.flags = (PPosition|PSize|PMinSize|PMaxSize);

	/* create the window */
#ifdef FEATURE_IMAGE
	if (ispixmap(colorinfo[x_toolbarcolors].bg))
	{
		dia->win = XCreateSimpleWindow(x_display,
			RootWindow(x_display, x_screen),
			hints.x, hints.y, hints.width, hints.height, 5,
			colorinfo[x_toolbarcolors].fg,
			pixmapof(colorinfo[x_toolbarcolors].bg).pixel);
		XSetWindowBackgroundPixmap(x_display, dia->win,
			pixmapof(colorinfo[x_toolbarcolors].bg).pixmap);
	}
	else
#endif
	{
		dia->win = XCreateSimpleWindow(x_display,
			RootWindow(x_display, x_screen),
			hints.x, hints.y, hints.width, hints.height, 5,
			colorinfo[x_toolbarcolors].fg,
			colorinfo[x_toolbarcolors].bg);

		/* use a gray pixmap for the background, if necessary */
		if (x_mono || colorinfo[x_toolbarcolors].fg == colorinfo[x_toolbarcolors].bg)
		{
			XSetWindowBackgroundPixmap(x_display, dia->win, x_gray);
		}
	}

	/* create the graphic context for the window */
	gcvalues.foreground = colorinfo[x_toolbarcolors].fg;
	gcvalues.background = xw->bg;
	gcvalues.font = x_loadedcontrol->fontinfo->fid;
	dia->gc = XCreateGC(x_display, xw->win,
		GCForeground|GCBackground|GCFont, &gcvalues);

	/* set the standard properties for the window */
	XSetStandardProperties(x_display, dia->win, dia->name, dia->name,
		None, NULL, 0, &hints);

	/* allow window manager's "Delete" menu item to work */
	XSetWMProtocols(x_display, dia->win, &x_wm_delete_window, 1);

	/* input event selection */
	XSelectInput(x_display, dia->win,
	    KeyPressMask | KeyReleaseMask |
	    ButtonPressMask | ButtonMotionMask | ButtonReleaseMask | 
	    FocusChangeMask | StructureNotifyMask | ExposureMask);

	/* map the window */
	XMapRaised(x_display, dia->win);

	/* insert it into the dialogs list */
	dia->next = x_dialogs;
	x_dialogs = dia;
}


/* delete a dialog window */
void x_dl_delete(dia)
	dialog_t	*dia;
{
	dialog_t	*scan;
	X_BUTTON	*button;
	int		i;

	/* free the GC */
	XFreeGC(x_display, dia->gc);

	/* free the X11 window (if it wasn't freed already) */
	if (dia->win != None)
		XDestroyWindow(x_display, dia->win);

	/* remove the struct from the list */
	if (x_dialogs == dia)
		x_dialogs = dia->next;
	else
	{
		for (scan = x_dialogs; scan->next != dia; scan = scan->next)
		{
		}
		scan->next = dia->next;
	}

	/* free all allocated memory */
	if (dia->name)
		safefree(dia->name);
	safefree(dia->desc);
	if (dia->excmd)
		safefree(dia->excmd);
	safefree(dia->spec);
	if (dia->field)
	{
		for (i = 0; i < dia->nfields; i++)
			if (dia->field[i].value)
				safefree(dia->field[i].value);
		safefree(dia->field);
	}
	while (dia->button)
	{
		button = dia->button;
		dia->button = dia->button->next;
		safefree(button);
	}
	safefree(dia);
}



/* Destroy all dialogs that referred to the freed window */
void x_dl_destroy(xw)
	X11WIN	*xw;
{
	dialog_t *dia;

	for (dia = x_dialogs; dia && dia->next; )
	{
		if (dia->next->xw == xw)
			x_dl_delete(dia->next);
		else
			dia = dia->next;
	}
	if (x_dialogs && x_dialogs->xw == xw)
	{
		x_dl_delete(x_dialogs);
	}
}



static void makecurrent(dia, row)
	dialog_t *dia;
	int	 row;
{
	int	previous;

	/* ignore if the new row is goofy, or unchanged */
	if (row < 0 || row >= dia->nfields || row == dia->current)
		return;

	/* change the "current" row value */
	previous = dia->current;
	dia->current = row;

	/* if some other row is already highlighted, then unhighlight it */
	if (previous >= 0)
	{
		if (x_mono
#ifdef FEATURE_IMAGE
		 || ispixmap(colorinfo[x_toolbarcolors].bg)
#endif
		 || colorinfo[x_toolbarcolors].fg == colorinfo[x_toolbarcolors].bg)
		{
			XClearArea(x_display, dia->win,
				1, dia->y0 - 2 + previous * dia->rowh,
				dia->w - 2, dia->rowh + 1, ElvFalse);
		}
		else
		{
			XSetForeground(x_display, dia->gc, colorinfo[x_toolbarcolors].bg);
			XDrawRectangle(x_display, dia->win, dia->gc,
				1, dia->y0 - 2 + previous * dia->rowh,
				dia->w - 3, dia->rowh);
		}
		exposerow(dia, previous, ElvTrue);
	}

	/* move the cursor to the end of its value */
	dia->cursor = CHARlen(dia->field[row].value);

	/* set the shift amount to 0 initially.  if necessary, other code will
	 * change it to make the cursor visible.
	 */
	dia->shift = 0;

	/* highlight the new "current" row */
	XSetForeground(x_display, dia->gc, colorinfo[x_toolbarcolors].fg);
	XDrawRectangle(x_display, dia->win, dia->gc,
		1, dia->y0 - 2 + row * dia->rowh,
		dia->w - 3, dia->rowh);
	exposerow(dia, row, ElvTrue);
}


/* handle a keystroke within a oneof field, return new value or NULL */
static CHAR *keyoneof(dia, key)
	dialog_t *dia;
	int	 key;
{
	int	nlegals, this;
	char	*legal[10];
	char	*s;
	int	i;
	CHAR	*val;

	/* build a list of legal names */
	legal[0] = dia->field[dia->current].limit;
	for (nlegals = 1, s = legal[0] + 1; nlegals < QTY(legal) && *s; s++)
	{
		if (elvspace(s[-1]) && !elvspace(s[0]))
			legal[nlegals++] = s;
	}

	switch (key)
	{
	  case XK_Left:
	  case XK_KP_Left:
	  case XK_Right:
	  case XK_KP_Right:
		/* find the current value */
		val = dia->field[dia->current].value;
		i = CHARlen(val);
		for (this = 0;
		     this < nlegals && CHARncmp(val, toCHAR(legal[this]), i);
		     this++)
		{
		}

		/* move left or right */
		if (key == XK_Left || key == XK_KP_Left)
			this--;
		else
			this++;
		break;

	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
		/* find the nth option */
		this = key - '1';
		break;

	  default:
	  	/* find an option which begins with that letter */
	  	for (this = 0; this < nlegals && *legal[this] != key; this++)
	  	{
	  	}
	}

	/* if illegal, then no change */
	if (this < 0 || this >= nlegals)
		return NULL;

	/* make a dynamic copy of that string */
	if (this == nlegals - 1)
		val = CHARdup(toCHAR(legal[this]));
	else
	{
		i = (int)(legal[this + 1] - legal[this]) - 1;
		val = (CHAR *)safealloc(i + 1, sizeof(CHAR));
		CHARncpy(val, toCHAR(legal[this]), i);
		val[i] = '\0';
	}
	return val;
}


/* handle a keystroke within a text field, return new value or NULL */
static CHAR *keystring(dia, key)
	dialog_t *dia;
	int	 key;
{
	CHAR	*orig = dia->field[dia->current].value;
	int	origlen = CHARlen(orig);
	CHAR	*val = NULL;

	switch (key)
	{
	  case XK_Left:
	  case XK_KP_Left:
		if (dia->cursor > 0)
			dia->cursor--;
		break;

	  case XK_Right:
	  case XK_KP_Right:
		if (dia->cursor < origlen)
			dia->cursor++;
		break;

	  case ELVCTRL('L'):
		if (dia->shift > 0)
			dia->shift--;
		if (dia->cursor > dia->shift + STRING_LENGTH)
			dia->cursor = dia->shift + STRING_LENGTH;
		break;

	  case ELVCTRL('R'):
		if (dia->shift + STRING_LENGTH < origlen)
			dia->shift++;
		if (dia->cursor < dia->shift)
			dia->cursor = dia->shift;
		break;

	  case ELVCTRL('U'):
		dia->field[dia->current].value[0] = '\0';
		dia->cursor = 0;
		break;

	  case XK_Home:
	  case XK_KP_Home:
		dia->cursor = 0;
		break;

	  case XK_End:
	  case XK_KP_End:
		dia->cursor = CHARlen(dia->field[dia->current].value);
		break;

	  case '\b':
	  case XK_BackSpace:
		if (dia->cursor > 0)
		{
			CHARcpy(&orig[dia->cursor - 1], &orig[dia->cursor]);
			dia->cursor--;
		}
		break;

	  case '\177':
	  case XK_Delete:
	  case XK_KP_Delete:
		if (dia->cursor < origlen)
			CHARcpy(&orig[dia->cursor], &orig[dia->cursor + 1]);
		break;

	  default:
		if (key == '\t' || (key >= ' ' && key <= 255))
		{
			val = (CHAR *)safealloc(origlen + 2, sizeof(CHAR));
			if (dia->cursor > 0)
				CHARncpy(val, orig, dia->cursor);
			val[dia->cursor] = key;
			if (dia->cursor < origlen)
				CHARcpy(&val[dia->cursor + 1], &orig[dia->cursor]);
			dia->cursor++;
		}
	}

	return val;
}


/* handle a keystroke */
static void keystroke(dia, key)
	dialog_t *dia;
	int	 key;
{
	int	i;
	CHAR	*newvalue;
	char	tmp[300];
	long	l, min, max;
	GUIWIN	*gw;
	char	*excmd;

	switch (key)
	{
	  case '\r':
	  case '\n':
	  case XK_Linefeed:
	  case XK_Return:
	  case XK_KP_Enter:
		/* store the values of all options */
		eventfocus((GUIWIN *)dia->xw, ElvFalse);
		for (i = 0; i < dia->nfields; i++)
			if (dia->field[i].ft != FT_LOCKED)
				optputstr(toCHAR(dia->field[i].name),
					  dia->field[i].value, ElvFalse);

		/* Destroy the dialog, unless it is pinned.  This must be done
		 * before eventex() is called, because the ex command might
		 * destroy the text window and hence also destroy the dialog.
		 * Right now we *know* that the dialog still exists and must
		 * be destroyed.
		 */
		gw = (GUIWIN *)dia->xw;
		excmd = dia->excmd ? safedup(dia->excmd) : NULL;
		if (!dia->pinned)
			x_dl_delete(dia);

		/* execute the ex command, if any */
		if (excmd)
		{
			eventex(gw, excmd, ElvFalse);
			safefree(excmd);
		}

		/* we'll need to update the screen after this */
		x_didcmd = ElvTrue;
		break;

	  case '\033':
	  case XK_Escape:
		/* destroy the dialog */
		x_dl_delete(dia);
		break;

	  case XK_Up:
	  case XK_KP_Up:
		makecurrent(dia, dia->current - 1);
		break;

	  case XK_Down:
	  case XK_KP_Down:
  		makecurrent(dia, dia->current + 1);
		break;

	  default:
		/* depends on field type */
		if (dia->current < 0)
			return;
		newvalue = dia->field[dia->current].value;
		switch (dia->field[dia->current].ft)
		{
		  case FT_ONEOF:
		  	newvalue = keyoneof(dia, key);
			break;

		  case FT_NUMBER:
			l = atol(tochar8(newvalue));
			switch (key)
			{
			  case '+':
				l++;
				break;

			  case '-':
				l--;
				break;

			  default:
				if ((key >= '0' && key <= '9')
				 || key >= 256
				 || key < ' '
				 || key == '\177')
				{
					newvalue = keystring(dia, key);
					if (!newvalue)
						newvalue = dia->field[dia->current].value;
					l = atol(tochar8(newvalue));

					if (newvalue != dia->field[dia->current].value)
						safefree(newvalue);
				}
			}
			if (dia->field[dia->current].limit)
			{
				sscanf(dia->field[dia->current].limit, "%ld:%ld", &min, &max);
				if (l > max)
					l = max;
				if (l < min)
					l = min;
			}
			sprintf(tochar8(tmp), "%ld", l);
			newvalue = CHARdup(tmp);
			if ((unsigned)dia->cursor > CHARlen(newvalue))
				dia->cursor = CHARlen(newvalue);
			break;

		  case FT_STRING:
			newvalue = keystring(dia, key);
			break;

		  case FT_FILE:
#ifdef FEATURE_COMPLETE
			if (key == '\t')
			{
				/* try filename completion */
				excmd = iofilename(tochar8(newvalue), '\0');
				if (excmd)
					newvalue = CHARdup(toCHAR(excmd));
				dia->cursor = CHARlen(newvalue);

				/* that may have caused some output on the
				 * elvis text window.
				 */
				x_didcmd = ElvTrue;
			}
			else
#endif /* FEATURE_COMPLETE */
			{
				newvalue = keystring(dia, key);
			}
			break;

		  case FT_DEFAULT:
		  case FT_LOCKED:
			break;
		}

		/* if reallocated memory then use the new value */
		if (newvalue && newvalue != dia->field[dia->current].value)
		{
			if (dia->field[dia->current].value)
				safefree(dia->field[dia->current].value);
			dia->field[dia->current].value = newvalue;
		}

		/* redraw the row */
		exposerow(dia, dia->current, ElvFalse);
	}
}



/* draw one field (row) of a dialog */
static void exposerow(dia, row, fromscratch)
	dialog_t *dia;
	int	 row;
	ELVBOOL	 fromscratch;
{
	X_BUTTON *button;
	int	  top, bottom;
	int	  newstate;

	/* compute row position */
	top = dia->y0 + row * dia->rowh;
	bottom = top + dia->rowh;

	/* draw the row's label */
	XSetForeground(x_display, dia->gc, colorinfo[x_toolbarcolors].fg);
	XSetFont(x_display, dia->gc, x_loadedcontrol->fontinfo->fid);
	x_drawstring(x_display, dia->win, dia->gc,
		dia->x0 - dia->field[row].lwidth,
		top + dia->base + 2, /*!!!*/
		tochar8(dia->field[row].label), CHARlen(dia->field[row].label));

	/* draw the text field, if there is one */
	switch (dia->field[row].ft)
	{
	  case FT_STRING:
	  case FT_FILE:
	  case FT_NUMBER:
	  case FT_LOCKED:
		drawtext(dia, row);
		break;

	  default:
		;
	}

	/* draw the buttons */
	for (button = dia->button; button; button = button->next)
	{
		/* skip if not for this row */
		if (button->y < top || button->y >= bottom)
			continue;

		/* choose a button state -- method varies with data type */
		switch (dia->field[row].ft)
		{
		  case FT_ONEOF:
			if (*dia->field[row].value == *button->label)
				newstate = -2;
			else
				newstate = 2;
			break;

		  case FT_STRING:
		  case FT_FILE:
			if (button->shape == 'l')
				if (row == dia->current
				    ? dia->shift > 0
				    : CHARlen(dia->field[row].value) > STRING_LENGTH)
					newstate = 2;
				else
					newstate = 0;
			else
				if (row == dia->current && CHARlen(dia->field[row].value) - dia->shift > STRING_LENGTH) 
					newstate = 2;
				else
					newstate = 0;
			break;

		  case FT_NUMBER:
			newstate = 2;
			break;

		  default:
			newstate = 0;
		}

		/* draw the button, if different than original state */
		if (fromscratch || newstate != button->state)
		{
			button->state = newstate;
			drawbutton(dia, button);
		}
	}

}

/* draw all of the dialog */
static void expose(dia)
	dialog_t *dia;
{
	XGCValues gcv;
	X11WIN	  *hadfocus;
	int	  x, y;
	int	  i;
	X_BUTTON  stripe;
	X_LOADEDFONT *titlefont;

	/* draw the icon */
	XSetForeground(x_display, dia->gc, colorinfo[x_toolcolors].bg);
	XFillRectangle(x_display, dia->win, dia->gc,
		2, 1, x_elvis_icon_width, x_elvis_icon_height + 2);
	gcv.ts_x_origin = 2;
	gcv.ts_y_origin = 2;
	gcv.stipple = dia->pinned ? x_elvis_pin_icon : x_elvis_icon;
	gcv.fill_style = FillStippled; 
	gcv.foreground = colorinfo[x_toolcolors].fg;
	gcv.background = colorinfo[x_toolbarcolors].bg;
	XChangeGC(x_display, dia->gc,
		GCForeground | GCBackground | GCFillStyle | GCStipple |
		GCTileStipXOrigin | GCTileStipYOrigin, &gcv);
	XFillRectangle(x_display, dia->win, dia->gc,
		2, 2, x_elvis_icon_width, x_elvis_icon_height);

	/* draw the name */
	titlefont = (x_defaultbold ? x_defaultbold : x_defaultnormal);
	gcv.fill_style = FillSolid;
	gcv.font = titlefont->fontinfo->fid;
	gcv.foreground = colorinfo[x_toolbarcolors].fg;
	XChangeGC(x_display, dia->gc, GCFillStyle|GCFont|GCForeground, &gcv);
	x = x_elvis_icon_width + 4;
	y = titlefont->fontinfo->ascent + 2;
	x_drawstring(x_display, dia->win, dia->gc,
		x, y, dia->name, strlen(dia->name));

	/* draw the description */
	y += titlefont->fontinfo->descent
		+ x_loadedcontrol->fontinfo->ascent;
	XSetFont(x_display, dia->gc, x_loadedcontrol->fontinfo->fid);
	x_drawstring(x_display, dia->win, dia->gc,
		x, y, dia->desc, strlen(dia->desc));

	/* draw a stripe below the header */
	stripe.x = 3;
	stripe.y = dia->y0 - 7;
	stripe.h = 2;
	stripe.w = dia->w - 6;
	stripe.shape = 'b';
	stripe.state = -1;
	stripe.lablen = 0;
	drawbutton(dia, &stripe);

	/* if pinned, then draw some blood in the cursor color */
	if (dia->pinned)
	{
		XSetForeground(x_display, dia->gc, colorinfo[x_cursorcolors].fg);
		XDrawLine(x_display, dia->win, dia->gc, 26, 27, 26, 34);
		XDrawLine(x_display, dia->win, dia->gc, 27, 28, 27, 29);
		XDrawLine(x_display, dia->win, dia->gc, 28, 27, 28, 32);
	}

	/* if any fields, then draw a stripe above the "Submit" button */
	if (dia->nfields > 0)
	{
		stripe.y = dia->submit->y - 8;
		drawbutton(dia, &stripe);
	}

	/* draw the "submit" and "cancel" buttons */
	drawbutton(dia, dia->submit);
	drawbutton(dia, dia->cancel);

	/* draw other aspects of all fields */
	for (i = 0; i < dia->nfields; i++)
	{
		/* don't do the current row here -- it'll be handled below */
		if (i == dia->current)
			continue;

		/* expose this row */
		exposerow(dia, i, ElvTrue);
	}

	/* draw the highlight around the current field */
	i = dia->current;
	if (i == -2)
	{
		/* first exposure -- maybe switch keyboard focus here? */
		if (o_focusnew)
		{
			/* if an elvis text window had focus before, it doesn't now! */
			if (x_hasfocus)
			{
				hadfocus = x_hasfocus;
				x_hasfocus = NULL;
				x_ta_erasecursor(hadfocus);
				x_ta_drawcursor(hadfocus);
				x_didcmd = ElvTrue;
			}

			XSetInputFocus(x_display, dia->win, RevertToParent, x_now);
		}

		/* set the "current" value to the first field, or -1 if none */
		if (dia->nfields > 0)
			i = 0;
		else
			i = -1;
	}
	dia->current = -1; /* <- to force redraw */
	makecurrent(dia, i);
}

void x_dl_event(w, event)
	Window	w;	/* window that received an event */
	XEvent	*event;	/* the event that was received */
{
	dialog_t *dia;
	X_BUTTON *btn;
	KeySym	  key;
	char	  text[10];
	char	  modifier;
	int	  i;

	/* try to find "w" in the list of dialog windows */
	for (dia = x_dialogs; dia && dia->win != w; dia = dia->next)
	{
	}
	if (!dia)
		return;

	/* process an event which affects the application window as a whole */
	switch (event->type)
	{
	  case FocusIn:
		/* if a text window has focus before, it doesn't now */
		if (x_hasfocus)
		{
			x_hasfocus = NULL;
			x_didcmd = ElvTrue;
		}
		break;

	  case MapNotify:
	  case Expose:
		expose(dia);
		break;

	  case KeyPress:
		/* check for modifier keys */
		if (event->xkey.state & Mod1Mask)
			modifier = o_altkey;
		else
			modifier = '\0';

		/* convert the keypress to a KeySym and string */
		i = XLookupString(&event->xkey, text, sizeof text, &key, 0);
#if 1
		/* THIS IS A HACK!  Some versions of XFree86 come with
		 * a default map which causes the backspace keycode to
		 * be translated to XK_Delete instead of XK_BackSpace.
		 * This causes big problems for elvis, since elvis would
		 * like to make the <backspace> and <delete> keys do
		 * different things.  If the current keystroke appears
		 * to be a backspace keycode which has been mapped to
		 * XK_Delete, then we force it to be mapped to
		 * XK_BackSpace instead.
		 *
		 * This problem could also be solved by the user running
		 * `xmodmap -e "keycode 22 = BackSpace"` after starting
		 * the X server.
		 */
		if (key == XK_Delete && event->xkey.keycode == 22)
		{
			key = XK_BackSpace;
			text[0] = '\b';
			text[1] = '\0';
		}
#endif
		if (i == 0)
		{
			if (!IsModifierKey(key) && key != XK_Mode_switch)
				keystroke(dia, (int)key);
		}
		else if (modifier == 's')
			keystroke(dia, *text | 0x80);
		else
			keystroke(dia, (int)*text);

		break;

	  case ButtonPress:
		/* if clicked on a row, then make that row current */
		makecurrent(dia, (event->xbutton.y - dia->y0) / dia->rowh);

		/* Buttons 4 & 5 act like <+> and <-> keystrokes when on a
		 * numeric field, or like side arrows on other fields.
		 */
		if (event->xbutton.button == 4 || event->xbutton.button == 5)
		{
			if (dia->field[dia->current].ft == FT_NUMBER)
				if (event->xbutton.button == 4)
					key = '+';
				else
					key = '-';
			else
				if (event->xbutton.button == 4)
					key = XK_Left;
				else
					key = XK_Right;
			keystroke(dia, key);
			break;
		}
			
		/* clicked on a button? */
		btn = findbutton(dia, event->xbutton.x, event->xbutton.y);
		if (btn)
		{
			/* redraw it as being slightly pushed in */
			dia->click = btn;
			btn->state = btn->state < 0 ? -1 : 1;
			drawbutton(dia, btn);
		}

		/* clicked on the icon? */
		if (event->xbutton.x < (int)(x_elvis_icon_width + 2)
		 && event->xbutton.y < (int)(x_elvis_icon_height + 2))
		{
			/* change the pushpin */
			dia->pinned = (ELVBOOL)!dia->pinned;

			/* redraw the icon (and everything else) */
			expose(dia);
		}
		break;

	  case ButtonRelease:
		/* redraw the clicked-on button as being untouched */
		if (!dia->click) break;
		dia->click->state = dia->click->state < 0 ? -2 : 2;
		drawbutton(dia, dia->click);

		/* released on the clicked-on button? */
		btn = findbutton(dia, event->xbutton.x, event->xbutton.y);
		if (btn == dia->click)
		{
			dia->click = NULL;
			keystroke(dia, btn->key);
		}
		else
		{
			/* better redraw the current row, if any, in case
			 * the clicked-on button isn't supposed to stick up.
			 */
			exposerow(dia, dia->current, ElvFalse);

			/* not clicked on anything now! */
			dia->click = NULL;
		}
		break;

	  case MotionNotify:
		/* ignore if no button is clicked */
		if (!dia->click)
			break;

		/* still on the clicked-on button? */
		btn = findbutton(dia, event->xmotion.x, event->xmotion.y);
		if (btn == dia->click)
			i = dia->click->state < 0 ? -1 : 1;
		else
			i = dia->click->state < 0 ? -2 : 2;

		/* draw the button, if any change */
		if (i != dia->click->state)
		{
			dia->click->state = i;
			drawbutton(dia, dia->click);
		}
		break;

	  case ConfigureNotify:
		break;

	  case ClientMessage:
		/* if WM_DELETE_WINDOW from the window manager, then
		 * destroy the X11 window.  The server will then send
		 * us a DestroyNotify message so we can finish
		 * cleaning up.
		 */
		if (event->xclient.message_type == x_wm_protocols
		 && event->xclient.format == 32
		 && (Atom)event->xclient.data.l[0] == x_wm_delete_window)
		{
			XDestroyWindow(x_display, event->xclient.window);
		}
		break;

	  case DestroyNotify:
		/* The top-level window is already gone */
		dia->win = None;

		/* other stuff still needs to be taken care of */
		x_dl_delete(dia);
		break;
	}
}

/* Redraw the windows associated with a given text window, to use new colors */
void x_dl_docolor(xw)
	X11WIN	*xw;
{
	dialog_t *dia;

	for (dia = x_dialogs; dia; dia = dia->next)
	{
		/* skip if for a different window */
		if (dia->xw != xw)
			continue;

		/* Change the background color */
#ifdef FEATURE_IMAGE
		if (ispixmap(colorinfo[x_toolbarcolors].bg))
		{
			XSetWindowBackgroundPixmap(x_display, dia->win,

				pixmapof(colorinfo[x_toolbarcolors].bg).pixmap);
		}
		else
#endif
		{
			XSetWindowBackground(x_display, dia->win,
				colorinfo[x_toolbarcolors].bg);
			if (x_mono || colorinfo[x_toolbarcolors].fg == colorinfo[x_toolbarcolors].bg)
			{
				XSetWindowBackgroundPixmap(x_display, dia->win, x_gray);
			}
		}

		/* redraw the window from scratch */
		XClearWindow(x_display, dia->win);
		expose(dia);
	}
}
#endif
