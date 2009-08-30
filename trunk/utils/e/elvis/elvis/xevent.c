/* xevent.c */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_xevent[] = "$Id: xevent.c,v 2.30 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef GUI_X11

# include "guix11.h"

ELVBOOL		x_didcmd;
ELVBOOL		x_repeating;

static long	keytime;
static X11WIN	*keyxw;
static long	rpttime;
static XEvent	rptevent;



/* Arrange for following x_getevent() calls to return a given event if no
 * other events are received before the given timeout value.  To stop the
 * repeating, call with a NULL value for the event pointer.
 */
void x_ev_repeat(event, timeout)
	XEvent	*event;
	long	timeout;
{
	if (event)
	{
		rptevent = *event;
		rpttime = timeout;
		x_repeating = ElvTrue;
	}
	else
	{
		rpttime = 0L;
		x_repeating = ElvFalse;
	}
}


/* Read the next event.  This is complicated by the need to autorepeat some
 * scrollbar events, and the need to make the cursor blink.
 */
XEvent *x_ev_wait()
{
	static XEvent	event;
	XEvent		notify;
	fd_set		rfds, wfds, efds;
	struct timeval	timeout;
	int		i;

	/* If there's a key timeout pending, then wait for it to expire or for
	 * an event.  If it expires, then alert elvis of that fact.  If an event
	 * arrives, then process it normally in the remainder of this function.
	 */
	if (keytime > 0 && XPending(x_display) == 0)
	{
		/* wait for timeout to elapse, or for an event to arrive */
		FD_ZERO(&rfds);
		FD_ZERO(&wfds);
		FD_ZERO(&efds);
		FD_SET(ConnectionNumber(x_display), &rfds);
		timeout.tv_sec = keytime / 10;
		timeout.tv_usec = (keytime % 10) * 100000;
		i = select(ConnectionNumber(x_display) + 1,
				&rfds, &wfds, &efds, &timeout);

		/* if timeout, then simulate a special KeyPress event. */
		if (i == 0)
		{
			memset(&event, 0, sizeof event);
			event.type = KeyPress;
			event.xkey.window = keyxw->win;
			event.xkey.time = x_now;
			keytime = 0;
			goto End;
		}
	}

	if (x_repeating && XPending(x_display) <= 0)
	{
		/* wait for either rpttime to expire, or for an event
		 * other than MotionNotify to be received.  If rpttime
		 * expires first, then pretend the rptevent was received
		 * again.
		 */
		XFlush(x_display);
		do
		{
			FD_ZERO(&rfds);
			FD_ZERO(&wfds);
			FD_ZERO(&efds);
			FD_SET(ConnectionNumber(x_display), &rfds);
			timeout.tv_sec = rpttime / 10;
			timeout.tv_usec = (rpttime % 10) * 100000 + 33333;
			i = select(ConnectionNumber(x_display) + 1,
					&rfds, &wfds, &efds, &timeout);
			if (i > 0)
			{
				do
				{
					XNextEvent(x_display, &event);
				} while (event.type == MotionNotify && XPending(x_display));
			}
		} while (i > 0 && event.type == MotionNotify);
		if (i == 0)
		{
			event = rptevent;
		}
	}
	else
	{
		/* blink the cursor until an event is available */
		if (x_hasfocus && XPending(x_display) == 0 && o_blinktime > 0)
		{
			do
			{
				FD_ZERO(&rfds);
				FD_ZERO(&wfds);
				FD_ZERO(&efds);
				FD_SET(ConnectionNumber(x_display), &rfds);
				timeout.tv_sec = o_blinktime / 10;
				timeout.tv_usec = (o_blinktime % 10) * 100000;
				i = select(ConnectionNumber(x_display) + 1,
						&rfds, &wfds, &efds, &timeout);
				if (i == 0)
				{
					if (x_hasfocus->ta.cursor == CURSOR_NONE)
						x_ta_drawcursor(x_hasfocus);
					else
						x_ta_erasecursor(x_hasfocus);
					XFlush(x_display);
				}
			} while (i == 0);

			/* make cursor visible while processing event */
			if (x_hasfocus->ta.cursor == CURSOR_NONE)
				x_ta_drawcursor(x_hasfocus);
		}

		/* read the next event */
		XNextEvent(x_display, &event);

		/* compress MotionNotify events */
		if (event.type == MotionNotify)
		{
			while (XEventsQueued(x_display, QueuedAfterReading) > 0)
			{
				XPeekEvent(x_display, &notify);
				if (notify.type != MotionNotify)
					break;
				XNextEvent(x_display, &event);
			}
		}
	}


	/* any user event other than KeyRelease turns off timeout */
End:
	switch (event.type)
	{
	  case DestroyNotify:
	  case MotionNotify:
	  case ButtonPress:
	  case ButtonRelease:
	  case KeyPress:
		keytime = 0;
	}

	return &event;
}


/* some standard event processing */
void x_ev_process(event)
	XEvent	*event;
{
	X11WIN	*xw;
	Window	w;	/* widget window which received requested */
	CHAR	modifier;/* behavior of the <Alt> key */
	char	prefix;	/* char to use as prefix for keys -- usually ^K */
	char	text[20];
	KeySym	mykey;
	ELVISSTATE state;
	Atom	gottype;
	char	*excmd;
	unsigned long	exlen, ldummy;
	int	i, j;
	XEvent	event2;
 static XComposeStatus compose;
 static long	delayresize = 0L;

	/* Process window-independent events, or pluck the window value out of
	 * window-dependent events.
	 */
	w = None;
	switch (event->type)
	{
	  case Expose:
		w = event->xexpose.window;

#if 1
		/* compress Expose events for this window */
		while (XPending(x_display) > 0
		    && XCheckTypedWindowEvent(x_display, w, Expose, &event2))
		{
			if (event2.xexpose.x < event->xexpose.x)
			{
				event->xexpose.width += (event->xexpose.x - event2.xexpose.x);
				event->xexpose.x = event2.xexpose.x;
			}
			if (event2.xexpose.x + event2.xexpose.width > event->xexpose.x + event->xexpose.width)
			{
				event->xexpose.width = event2.xexpose.x + event2.xexpose.width - event->xexpose.x;
			}
			if (event2.xexpose.y < event->xexpose.y)
			{
				event->xexpose.height += (event->xexpose.y - event2.xexpose.y);
				event->xexpose.y = event2.xexpose.y;
			}
			if (event2.xexpose.y + event2.xexpose.height > event->xexpose.y + event->xexpose.height)
			{
				event->xexpose.height = event2.xexpose.y + event2.xexpose.height - event->xexpose.y;
			}
		}

		/* After compression, this *MUST* be the last Expose event
		 * for the queue.  Compression does that.
		 */
		event->xexpose.count = 0;
#endif
		break;

	  case GraphicsExpose:
	  case MapNotify:
	  case UnmapNotify:
		w = event->xexpose.window;
		break;

	  case MappingNotify:
		/* process keyboard mapping changes */
		XRefreshKeyboardMapping(&event->xmapping);
		break;

	  case FocusIn:
	  case FocusOut:
		w = event->xfocus.window;
		break;

	  case SelectionClear:
	  case SelectionRequest:
		x_clipevent(event);
		break;

	  case ButtonPress:
	  case ButtonRelease:
		w = event->xbutton.window;
		x_now = event->xbutton.time;
		delayresize = 0L;
		break;

	  case MotionNotify:
		w = event->xmotion.window;
		x_now = event->xmotion.time;
		delayresize = 0L;
		break;

	  case KeyPress:
		w = event->xkey.window;
		delayresize = 0L;
		x_now = event->xkey.time;
		break;

	  case ConfigureNotify:
		w = event->xconfigure.window;
		break;

	  case ClientMessage:
		w = event->xclient.window;
		break;

	  case DestroyNotify:
		w = event->xdestroywindow.window;
		break;

	  case PropertyNotify:
		w = event->xproperty.window;
		break;
	}

	/* If no window-specific processing is required, we're done. */
	if (w == None)
		return;

	/* Figure out which widget, in which window, has received the event,
	 * and send it to that widget.  Note that we also detect few events
	 * which affect the whole window here, and process them after the loop.
	 */
	for (xw = x_winlist; xw; xw = xw->next)
	{
		/* Scrollwheel events are always treated as though they were
		 * in the text window, regardless of the mouse position.
		 * However, the statusbar and top-level window don't receive
		 * any mouse events so we can ignore them.
		 */
		if ((event->type == ButtonPress || event->type == ButtonRelease)
		 && (event->xbutton.button == 4 || event->xbutton.button == 5)
		 && (xw->tb.win == w || xw->sb.win == w))
			w = xw->ta.win;

		/* let each window handle its events */
		if (xw->ta.win == w)
		{
			x_ta_event(xw, event);
			return;
		}
		else if (xw->tb.win == w)
		{
			x_tb_event(xw, event);
			return;
		}
		else if (xw->sb.win == w)
		{
			x_sb_event(xw, event);
			return;
		}
		else if (xw->st.win == w)
		{
			x_st_event(xw, event);
			return;
		}
		else if (xw->win == w)
			/* whole-window events are handled below, after we give
			 * dialogs a chance to handle their own events.
			 */
			break;
	}
	if (!xw)
	{
		/* not a text window, but give dialog windows a chance */
		x_dl_event(w, event);
		return;
	}

	/* process an event which affects the application window as a whole */
	switch (event->type)
	{
	  case MapNotify:
		xw->ismapped = ElvTrue;
		if (o_focusnew)
			(void)(*guix11.focusgw)((GUIWIN *)xw);
		x_sb_setstate(xw, X_SB_REDRAW);
		break;

	  case UnmapNotify:
		xw->ismapped = ElvFalse;
		xw->willraise = ElvFalse;
		break;

	  case FocusIn:
		x_hasfocus = xw;
		x_ta_erasecursor(xw);
		x_ta_drawcursor(xw);
#ifndef NO_XLOCALE
		if (xw->ic)
			XSetICFocus(xw->ic);
#endif
		x_didcmd = ElvTrue;
		break;

	  case FocusOut:
		if (xw == x_hasfocus)
		{
			x_hasfocus = NULL;
			x_ta_erasecursor(xw);
			x_ta_drawcursor(xw);
			x_didcmd = ElvTrue;
		}
#ifndef NO_XLOCALE
		if (xw->ic)
			XUnsetICFocus(xw->ic);
#endif
		break;

	  case KeyPress:
		/* if notification of timeout, then just send 0 keystrokes */
		if (!event->xkey.display)
		{
			eventkeys((GUIWIN *)w, NULL, 0);
			xw->state = eventkeys((GUIWIN *)xw, toCHAR(text), 0);
			break;
		}

		/* get some standard info from the event */
		xw->x = event->xkey.x_root - event->xkey.x;
		xw->y = event->xkey.y_root - event->xkey.y;

		/* check for modifier keys */
		if (event->xkey.state & Mod1Mask)
			modifier = o_altkey;
		else if (event->xkey.state & ShiftMask)
			modifier = 'v';
		else
			modifier = '\0';

		/* modifiers also affect the keys */
		if (event->xkey.state & ControlMask)
			prefix = ELVCTRL('C');
		else if (event->xkey.state & ShiftMask)
			prefix = ELVCTRL('S');
		else
			prefix = ELVCTRL('K');

		/* convert the keypress to a KeySym and string */
#ifndef NO_XLOCALE
		if (XFilterEvent(event, None))
			break;
		if (xw->ic != NULL)
		{
			Status status_return;
			i = XmbLookupString(xw->ic, &event->xkey,
					text, sizeof text, &mykey,
					&status_return);
		}
		else
		{
			i = XLookupString(&event->xkey,
					text, sizeof text, &mykey,
					&compose);
		}
#else	/* NO_XLOCALE */
		i = XLookupString(&event->xkey, text, sizeof text, &mykey, &compose);
#endif	/* NO_XLOCALE */
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
		if (mykey == XK_Delete && event->xkey.keycode == 22)
		{
			mykey = XK_BackSpace;
			text[0] = '\b';
			text[1] = '\0';
			i = 1;
		}
#endif
		if (i == 0)
		{
			if (!IsModifierKey(mykey) && mykey != XK_Mode_switch)
			{
				/* function keys become a control sequence */
				sprintf(text, "%c%04x", prefix,
					mykey ? (unsigned)mykey : event->xkey.keycode);
				i = strlen(text);
			}
		}
		if (i > 0)
		{
			switch (modifier)
			{
			  case 'c':
				(void)eventkeys((GUIWIN *)xw, toCHAR("\017"), 1);
				break;

			  case 's':
				if (i == 1) text[0] |= 0x80;
				break;

			  case 'v':
				if (i > 1)
				{
					switch ((int)mykey)
					{
					  case XK_Up:
					  case XK_Down:
						eventclick((GUIWIN *)xw,
							   0, 0, CLICK_SSLINE);
						text[0] = ELVCTRL('K');
						break;

					  case XK_Left:
					  case XK_Right:
					  case XK_Home:
					  case XK_End:
						eventclick((GUIWIN *)xw,
							   0, 0, CLICK_SSCHAR);
						text[0] = ELVCTRL('K');
						break;

					  case XK_Insert:
					  case XK_KP_Insert:
						eventclick((GUIWIN *)xw,
							  0, 0, CLICK_PASTE);
						i = 0;
						break;

					  default:
						;/* do nothing for other keys */
					}

				}

			  default: ;/* do nothing */
			}
			state = eventkeys((GUIWIN *)xw, toCHAR(text), i);

			/* The "xw" window may have been deleted during
			 * the eventkeys() call above.  If the window
			 * still exists then we have some work to do.
			 */
			for (xw = x_winlist; xw && xw->win != event->xkey.window; xw = xw->next)
			{
			}
			if (xw)
			{
				/* User maps are handled elsewhere; we just set
				 * the "keytime" variable to the timeout value.
				 * Key maps timeout immediately, right here.
				 */
				xw->state = state;
				if (xw->state == MAP_KEY)
					xw->state = eventkeys((GUIWIN *)xw, toCHAR(text), 0);
				else if (xw->state == MAP_USER)
					keytime = o_usertime, keyxw = xw;
				x_didcmd = ElvTrue;
			}
		}
		break;

	  case ConfigureNotify:
	  	/* Read any other ConfigureNotify events, and only process the
	  	 * last one.  This is more than just an optimization; it is
	  	 * intended to solve a problem in which elvis gets stuck in
	  	 * a loop toggling between two different sizes.
	  	 */
	  	if (delayresize > 0L)
		{
			/* brief delay to allow more events to arrive */
			struct timeval	timeout;
			timeout.tv_sec = 0;
			timeout.tv_usec = delayresize;
			(void)select(0, NULL, NULL, NULL, &timeout);
		}

		while (XEventsQueued(x_display, QueuedAfterReading) > 0
		    && XCheckTypedWindowEvent(x_display, xw->win, ConfigureNotify, event))
		{
		}

		if ((unsigned)event->xconfigure.width != xw->w
		 || (unsigned)event->xconfigure.height != xw->h)
		{
			i = (event->xconfigure.width - xw->sb.w - 2 * o_borderwidth) / xw->ta.cellw;
			j = (event->xconfigure.height - xw->tb.h - xw->st.h - 2 * o_borderwidth) / xw->ta.cellh;
			if ((unsigned)i != xw->ta.columns
			 || (unsigned)j != xw->ta.rows)
			{
				/* reconfigure the window */
				x_reconfig(xw, i, j);

				/* If next resize happens too soon,
				 * then delay it slightly
				 */
				if (delayresize < 600000)
					delayresize += 100000;
			}
		}
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
			if (x_winlist->next)
				excmd = "close";
			else
				excmd = "quit";
			eventex((GUIWIN *)xw, excmd, ElvFalse);
		}
		break;

	  case DestroyNotify:
		/* The top-level window & all widget windows are already gone */
		xw->win = None;
		xw->tb.win = None;
		xw->sb.win = None;
		xw->ta.win = None;

		/* other stuff still needs to be taken care of */
		(*guix11.destroygw)((GUIWIN *)xw, ElvTrue);
		break;

	  case PropertyNotify:
		if (event->xproperty.atom == x_elvis_server
			&& event->xproperty.state == PropertyNewValue)
		{
			XGetWindowProperty(x_display,
				xw->win, x_elvis_server, 0L, 8192,
				ElvTrue, XA_STRING, &gottype,
				&i, (unsigned long *)&exlen,
				(unsigned long *)&ldummy,
				(unsigned char **)&excmd);
			if (exlen > 0)
			{
				if (gottype == XA_STRING && i == 8)
				{
					excmd[exlen] = '\0';
					eventex((GUIWIN *)xw, excmd, ElvTrue);
					x_didcmd = ElvTrue;
				}
				XFree(excmd);
			}

			/* never leave old data in the property */
			if (ldummy > 0)
			{
				XDeleteProperty(x_display, xw->win, x_elvis_server);
			}
		}
		break;
	}
}


/* Test for signs of boredom from the user, so we can cancel long operations.
 * Here, we check to see if user has clicked on the window.
 */
ELVBOOL x_ev_poll(reset)
	ELVBOOL reset;
{
	XEvent	event;
	X11WIN	*scan;

	/* ignore if simply trying to reset */
	if (reset)
		return ElvFalse;

	/* Check every window to see if it has a ButtonPress event pending */
	if (XPending(x_display) > 0
	 && XCheckTypedEvent(x_display, ButtonPress, &event))
	{
		return ElvTrue;
	}

	/* Handle any Expose events now */
	while (XPending(x_display) > 0
	 && XCheckTypedEvent(x_display, Expose, &event))
	{
		x_ev_process(&event);
	}

	/* Since we're polling, redraw all scrollbars to show stop signs so
	 * the user knows that a click will abort.
	 */
	for (scan = x_winlist; scan; scan = scan->next)
	{
		x_sb_setstate(scan, X_SB_STOP);
	}

	return ElvFalse;
}
#endif
