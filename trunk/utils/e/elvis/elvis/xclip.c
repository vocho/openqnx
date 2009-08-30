/* xclip.c */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_xclip[] = "$Id: xclip.c,v 2.7 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef GUI_X11
# include "guix11.h"

ELVBOOL		x_ownselection;	/* does elvis own the X11 selection? */
static char	*clipbuf;	/* pointer to malloc'ed buffer of chars to/from X */
static long	clipsize;	/* total number of bytes in clipbuf */
static long	clipused;	/* if reading, total number of bytes read previously */
static ELVBOOL	clipwriting;	/* ElvTrue if cutting to X, ElvFalse if pasting from X */



void x_clipevent(event)
	XEvent	*event;
{
	XEvent	notify;
	Window	owner;
	X11WIN	*xw;
	Atom	targets[2];

	switch (event->type)
	{
	  case SelectionClear:
		/* We may have lost our old selection yet still be
		 * responsible for the new selection.  Check to see
		 * if we still own the current selection.
		 */
		owner = XGetSelectionOwner(x_display, XA_PRIMARY);
		for (xw = x_winlist; xw && xw->win != owner; xw = xw->next)
		{
		}
		if (x_ownselection && !xw)
		{
			/* free the selection */
			if (clipbuf)
			{
				safefree(clipbuf);
				clipbuf = NULL;
			}
			x_ownselection = ElvFalse;
			x_ta_drawcursor(NULL);
		}
		break;

	  case SelectionRequest:
		/* create a SelectionNotify event for requestor */
		notify.type = SelectionNotify;
		notify.xselection.requestor= event->xselectionrequest.requestor;
		notify.xselection.selection= event->xselectionrequest.selection;
		notify.xselection.target = event->xselectionrequest.target;
		notify.xselection.time = event->xselectionrequest.time;

		/* try to convert the selection */
		if (event->xselectionrequest.selection == XA_PRIMARY
		 && (event->xselectionrequest.target == XA_STRING
			|| event->xselectionrequest.target == x_compound_text))
		{
			/* store the selection's value into the property */
			XChangeProperty(x_display,
				event->xselectionrequest.requestor,
				event->xselectionrequest.property,
				event->xselectionrequest.target,
				8, PropModeReplace,
				(unsigned char *)(clipbuf ? clipbuf : ""),
				clipsize);
			notify.xselection.property = event->xselectionrequest.property;
		}
		else if (event->xselectionrequest.selection == XA_PRIMARY
		 && event->xselectionrequest.target == x_targets)
		{
			targets[0] = XA_STRING;
			targets[1] = x_compound_text;
			XChangeProperty(x_display,
				event->xselectionrequest.requestor,
				event->xselectionrequest.property,
				event->xselectionrequest.target,
				32, PropModeReplace,
				(unsigned char *)targets,
				sizeof targets / sizeof *targets);
			notify.xselection.property = event->xselectionrequest.property;
		}
		else
		{
			/* can't convert */
			notify.xselection.property = None;
		}

		/* notify the requestor */
		XSendEvent(x_display, notify.xselection.requestor,
			ElvFalse, 0L, &notify);
	}
}

/* open an X cut buffer for reading or writing.  Returns ElvTrue if successful */
ELVBOOL	x_clipopen(forwrite)
	ELVBOOL	forwrite;	/* ElvTrue for writing, ElvFalse for reading */
{
	XEvent	event;
	Atom	gottype;
	long	extra;
	int	i;

	/* free the old clipbuf, if there was one (except when reading from
	 * own own selection).
	 */
	if (clipbuf && (forwrite || !x_ownselection))
	{
		safefree(clipbuf);
		clipbuf = NULL;
	}

	if (forwrite)
	{
		/* prepare to collect bytes as clipwrite() gets called */
		clipwriting = ElvTrue;
		clipsize = 0;
	}
	else
	{
		/* does elvis own the selection? */
		if (x_ownselection)
		{
			/* yes -- clipbuf already contains it */
		}
		else
		{
			/* no -- try to fetch text from X two ways... */

			/* is there a selection owner? */
			if (XGetSelectionOwner(x_display, XA_PRIMARY) == None)
			{
				/* don't bother to try getting selection */
				event.type = ButtonPress;
			}
			else
			{
				/* Try to fetch the selection */
				XConvertSelection(x_display,
					XA_PRIMARY, XA_STRING, x_elvis_cutbuffer,
					((X11WIN *)windefault->gw)->win, x_now);
				
				/* Wait for the selection to arrive, or for a
				 * button press.  Eat any other events.  (The
				 * button press event gives the user a way to
				 * abort the operation.)
				 */
				do
				{
					XNextEvent(x_display, &event);
				} while (event.type != SelectionNotify
					&& event.type != ButtonPress);
			}

			/* did we succeed in fetching the selection? */
			if (event.type == SelectionNotify
			 && event.xselection.property != None)
			{
				/* Yes -- fetch bytes from X property */
				XGetWindowProperty(x_display,
					event.xselection.requestor,
					event.xselection.property,
					0L, 65536L, ElvTrue,
					event.xselection.target, &gottype, &i,
					(unsigned long *)&clipsize,
					(unsigned long *)&extra,
					(unsigned char **)&clipbuf);
				if (extra > 0)
					msg(MSG_WARNING, "[d]$1 bytes skipped", extra);
			}
			else
			{
				/* No -- fetch bytes from X cut buffer */
				clipbuf = XFetchBytes(x_display, &i);
				clipsize = i;
				if (!clipbuf)
				{
					return ElvFalse;
				}
			}
		}

		/* prepare to receive bytes as clipread() gets called */
		clipwriting = ElvFalse;
		clipused = 0;
	}
	return ElvTrue;
}

/* add text to the buffer */
int x_clipwrite(text, len)
	CHAR	*text;	/* pointer to buffer containing some bytes */
	int	len;	/* number of bytes to add */
{
	char	*newp;

	assert(clipwriting);

	/* combine old text (if any) with new text in a malloc'ed buffer */
	newp = safealloc(len + clipsize, sizeof(char));
	if (clipsize > 0)
	{
		memcpy(newp, clipbuf, (size_t)clipsize);
		safefree(clipbuf);
	}
	clipbuf = newp;
	memcpy(clipbuf + clipsize, text, (size_t)len);
	clipsize += len;

	return len;
}

/* extract text from the buffer */
int x_clipread(text, len)
	CHAR	*text;	/* pointer to buffer where bytes should go */
	int	len;	/* number of bytes to read this time */
{
	assert(!clipwriting);

	if (!clipbuf || clipused >= clipsize)
	{
		/* everything already sent; return 0 */
		return 0;
	}
	else if (clipused + len >= clipsize)
	{
		/* if everything fits, return everything */
		len = clipsize - clipused;
	}
	memcpy(text, clipbuf + clipused, (size_t)len);
	clipused += len;
	return len;
}

/* end the cut/paste operation */
void x_clipclose()
{
	if (clipwriting)
	{
		/* send bytes to the X server */
		XStoreBytes(x_display, clipbuf ? clipbuf : "", clipsize);

		/* claim ownership of the selection */
		XSetSelectionOwner(x_display, XA_PRIMARY,
			((X11WIN *)windefault->gw)->win, x_now);

		/* was the claim successful? */
		if (XGetSelectionOwner(x_display, XA_PRIMARY) ==
			((X11WIN *)windefault->gw)->win)
		{
			x_ownselection = ElvTrue;
			x_ta_drawcursor(NULL);
		}
		else
		{
			/* elvis doesn't own the selection */
			x_ownselection = ElvFalse;
			x_ta_drawcursor(NULL);
			safefree(clipbuf);
			clipbuf = NULL;
		}
	}
	else
	{
		if (!x_ownselection)
		{
			/* free Xlib's copy of the cut buffer */
			XFree(clipbuf);
			clipbuf = NULL;
		}
	}
}
#endif
