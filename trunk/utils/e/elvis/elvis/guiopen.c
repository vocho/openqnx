/* guiopen.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_guiopen[] = "$Id: guiopen.c,v 2.30 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef GUI_OPEN
# if ANY_UNIX
#  include <unistd.h>
# else
#  include <io.h>
# endif

/* This file contains a user interface which internally resembles guitcap.c,
 * but which doesn't depend on termcap and can therefore be used with terminals
 * of an unknown type.
 */


#if USE_PROTOTYPES
static ELVBOOL creategw(char *name, char *firstcmd);
static int init(int argc, char **argv);
static int scriptinit(int argc, char **argv);
static int test(void);
static void beep(GUIWIN *gw);
static void destroygw(GUIWIN *gw, ELVBOOL force);
static void endtty(void);
static void loop(void);
static void scriptloop(void);
static void quitloop(void);
static ELVBOOL scriptmsg(GUIWIN *gw, MSGIMP imp, CHAR *text, int len);
static void starttty(void);
static void term(void);
static void scriptterm(void);
static void textline(GUIWIN *gw, CHAR *text, int len);
static ELVBOOL oprgopen(char *command, ELVBOOL willread, ELVBOOL willwrite);
static int oprgclose(void);
#endif


static char ttyerasekey;	/* taken from the ioctl structure */

static void starttty()
{
	/* change the terminal mode to cbreak/noecho */
	ttyinit();
	ttyraw(&ttyerasekey);
}

static void endtty()
{
	/* change the terminal mode back the way it was */
	ttynormal();

	/* leave the cursor on a fresh line */
	ttywrite("\n", 1);
}

/*----------------------------------------------------------------------------*/
/* start of GUI functions */

GUIWIN *current;
ELVBOOL	anycmd;

/* return ElvTrue if available. */
static int test()
{
	return 1;
}


/* initialize the "open" interface. */
static int init(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	int	i;

	/* initialize the tty */
	starttty();

	/* set anycmd if there is a "-c cmd" or "+cmd" argument. */
	for (i = 1; i < argc && !anycmd; i++)
	{
		anycmd = (ELVBOOL)(argv[i][0] == '+' ||
				  (argv[i][0] == '-' && argv[i][1] == 'c'));
	}
	return argc;
}


/* Repeatedly get events (keystrokes), and call elvis' event functions */
static void loop()
{
	char	buf[10];
	int	len;
	MAPSTATE mst = MAP_CLEAR;
 
	/* peform the -c command or -t tag */
	if (mainfirstcmd(windefault))
		return;

	while (current)
	{
		/* redraw the window(s) */
		(void)eventdraw(current);

		/* read events */
		do
		{
			len = ttyread(buf, sizeof buf, (mst==MAP_CLEAR ? 0 : 2));
		} while (len < 1);

		/* process keystroke data */
		mst = eventkeys(current, toCHAR(buf), len);
	}
}


/* shut down the "open" interface */
static void term()
{
	if (gui == &guiopen)
		endtty();
}


/* initialize the "script" or "quit" interface. */
static int scriptinit(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	int	i;

	/* set anycmd if there is a "-c cmd" or "+cmd" argument. */
	for (i = 1; i < argc && !anycmd; i++)
	{
		anycmd = (ELVBOOL)(argv[i][0] == '+' ||
				  (argv[i][0] == '-' && argv[i][1] == 'c'));
	}
	return argc;
}


/* This is the loop function for the "script" user interface.  It reads text
 * from stdin, and then executes it.  Finishes by deleting the only window.
 */
static void scriptloop()
{
	BUFFER	script;
	MARKBUF	start, end;
	RESULT	result;
	char	inbuf[1024];
	long	n;

	/* peform the -c command or -t tag */
	if (mainfirstcmd(windefault))
		return;

	/* create a buffer to hold the script */
	script = bufalloc(NULL, 0, ElvTrue);

	/* read from stdin, into a buffer */
	while ((n = read(0, inbuf, sizeof(inbuf))) > 0)
	{
		(void)marktmp(end, script, o_bufchars(script));
		bufreplace(&end, &end, toCHAR(inbuf), n);
	}

	/* execute the script */
	eventfocus(current, ElvTrue);
	result = experform(windefault, marktmp(start, script, 0L),
				    marktmp(end, script, o_bufchars(script)));

	/* destroy the window (if the script didn't do that already) */
	if (current)
		(void)eventdestroy(current);
}


/* This is the loop function for the "quit" user interface.  It immediately
 * deletes the only window, and then exits.
 */
static void quitloop()
{
	/* peform the -c command or -t tag */
	if (mainfirstcmd(windefault))
		return;

	if (!anycmd)
		msg(MSG_INFO, "session=(session)");
	if (current)
		(void)eventdestroy(current);
}


/* shut down the "script" or "quit" interface */
static void scriptterm()
{
	/* nothing actions needed */
}


/* This function creates a window */
static ELVBOOL creategw(name, firstcmd)
	char	*name;		/* name of new window's buffer */
	char	*firstcmd;	/* other parameters, if any */
{
	/* We can only have one window.  If we already made it, fail */
	if (current)
	{
		return ElvFalse;
	}

	/* make elvis do its own initialization */
	current = (GUIWIN *)1;
	if (!eventcreate(current, NULL, name, 24, 80))
	{
		/* elvis can't make it -- fail */
		return ElvFalse;
	}

	/* if there is a firstcmd, then execute it */
	if (firstcmd)
	{
		winoptions(winofgw(current));
		exstring(windefault, toCHAR(firstcmd), "+cmd");
	}

	return ElvTrue;
}


/* This function deletes a window */
static void destroygw(gw, force)
	GUIWIN	*gw;	/* window to be destroyed */
	ELVBOOL	force;	/* if ElvTrue, try harder */
{
	assert(gw == current);

	/* simulate a "destroy" event */
	eventdestroy(gw);
	current = NULL;
}

/* This function rings the bell */
static void beep(gw)
	GUIWIN	*gw;	/* window that generated a beep request */
{
	ttywrite("\007", 1);
}


/* This function outputs a mixture of text and control characters.  The only
 * possible control characters will be '\b', '\n', and '\r'.
 */
static void textline(gw, text, len)
	GUIWIN	*gw;	/* where the text is to be output */
	CHAR	*text;	/* the text to be output */
	int	len;	/* length of text */
{
	ttywrite(tochar8(text), len);
}


/* This function is used by the "quit" gui to hide all messages except errors */
static ELVBOOL scriptmsg(gw, imp, text, len)
	GUIWIN	*gw;	/* window which generated the message */
	MSGIMP	imp;	/* message importance */
	CHAR	*text;	/* the message itself */
	int	len;	/* length of the message */
{
	if (imp == MSG_ERROR || imp == MSG_FATAL)
		return ElvFalse; /* so error is displayed normally */

	/* other message types are ignored */
	return ElvTrue;
}


static ELVBOOL oprgopen(command, willwrite, willread)
	char	*command;
	ELVBOOL	willwrite;
	ELVBOOL	willread;
{
	/* switch the tty to normal mode */
	ttynormal();
	
	/* use the OS prgopen() function */
	return prgopen(command, willwrite, willread);
}


static int oprgclose()
{
	int	ret;
	
	/* use the OS perclose() function */
	ret = prgclose();

	/* switch the tty back to raw mode */
	ttyraw(&ttyerasekey);
	
	return ret;
}


/* structs of this type are used to describe each available GUI */
GUI guiopen =
{
	"open",		/* name */
	"Generic interface with limited capabilities",
	ElvFalse,	 	/* exonly */
	ElvTrue,	 	/* newblank */
	ElvFalse,	 	/* minimizeclr */
	ElvTrue,		/* scrolllast */
	ElvFalse,		/* shiftrows */
	3,		/* movecost */
	0,		/* opts */
	NULL,		/* optdescs */
	test,
	init,
	NULL,		/* usage */
	loop,
	NULL,	 	/* poll */
	term,
	creategw,
	destroygw,
	NULL,		/* focusgw */
	NULL,		/* retitle */
	NULL,		/* reset */
	NULL,		/* flush */
	NULL,		/* moveto */
	NULL,		/* draw */
	NULL,		/* shift */
	NULL,		/* scroll */
	NULL,		/* clrtoeol */
	textline,
	beep,
	NULL,		/* msg */
	NULL,		/* scrollbar */
	NULL,		/* status */
	NULL,		/* keylabel */
	NULL,		/* clipopen */
	NULL,		/* clipwrite */
	NULL,		/* clipread */
	NULL,		/* clipclose */
	coloransi,	/* color */
	NULL,		/* colorfree */
	NULL,		/* setbg */
	NULL,	 	/* guicmd */
	NULL,		/* tabcmd */
	NULL,		/* save */
	NULL,		/* wildcard */
	oprgopen,	/* prgopen */
	oprgclose,	/* prgclose */
	NULL		/* stop */
};



/* structs of this type are used to describe each available GUI */
GUI guiscript =
{
	"script",	/* name */
	"Reads a script from stdin",
	ElvTrue,	 	/* exonly */
	ElvTrue,	 	/* newblank */
	ElvFalse,	 	/* minimizeclr */
	ElvTrue,		/* scrolllast */
	ElvFalse,		/* shiftrows */
	0,		/* movecost */
	0,		/* opts */
	NULL,		/* optdescs */
	test,
	scriptinit,
	NULL,		/* usage */
	scriptloop,
	NULL,		/* poll */
	scriptterm,
	creategw,
	destroygw,
	NULL,		/* focusgw */
	NULL,		/* retitle */
	NULL,		/* reset */
	NULL,		/* flush */
	NULL,		/* moveto */
	NULL,		/* draw */
	NULL,		/* shift */
	NULL,		/* scroll */
	NULL,		/* clrtoeol */
	textline,
	beep,
	scriptmsg,
	NULL,		/* scrollbar */
	NULL,		/* status */
	NULL,		/* keylabel */
	NULL,		/* clipopen */
	NULL,		/* clipwrite */
	NULL,		/* clipread */
	NULL,		/* clipclose */
	coloransi,	/* color */
	NULL,		/* colorfree */
	NULL,		/* setbg */
	NULL,	 	/* guicmd */
	NULL,		/* tabcmd */
	NULL,		/* save */
	NULL,		/* wildcard */
	NULL,		/* prgopen */
	NULL,		/* prgclose */
	NULL		/* stop */
};



/* structs of this type are used to describe each available GUI */
GUI guiquit =
{
	"quit",		/* name */
	"Quits immediately after processing \"-c cmd\"",
	ElvTrue,	 	/* exonly */
	ElvTrue,	 	/* newblank */
	ElvFalse,	 	/* minimizeclr */
	ElvTrue,		/* scrolllast */
	ElvFalse,		/* shiftrows */
	3,		/* movecost */
	0,		/* opts */
	NULL,		/* optdescs */
	test,
	scriptinit,
	NULL,		/* usage */
	quitloop,
	NULL,		/* poll */
	scriptterm,
	creategw,
	destroygw,
	NULL,		/* focusgw */
	NULL,		/* retitle */
	NULL,		/* reset */
	NULL,		/* flush */
	NULL,		/* moveto */
	NULL,		/* draw */
	NULL,		/* shift */
	NULL,		/* scroll */
	NULL,		/* clrtoeol */
	textline,
	beep,
	scriptmsg,
	NULL,		/* scrollbar */
	NULL,		/* status */
	NULL,		/* keylabel */
	NULL,		/* clipopen */
	NULL,		/* clipwrite */
	NULL,		/* clipread */
	NULL,		/* clipclose */
	coloransi,	/* color */
	NULL,		/* colorfree */
	NULL,		/* setbg */
	NULL,	 	/* guicmd */
	NULL,		/* tabcmd */
	NULL,		/* save */
	NULL,		/* wildcard */
	NULL,		/* prgopen */
	NULL,		/* prgclose */
	NULL		/* stop */
};
#endif
