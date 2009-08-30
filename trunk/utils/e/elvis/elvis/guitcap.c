/* guitcap.c */

#include "elvis.h"
#ifdef GUI_TERMCAP
#include <signal.h>


/* This file contains a termcap-based user interface.  It is derived from the
 * "curses.c" file of elvis 1.8.
 */

#define	MINHEIGHT	4

/* Some termcap packages require the application code to supply a "BC"
 * variable.  Others (particularly ncurses) forbid it.  The nice ones
 * supply one if you don't, so they'll work either way.
 */
#ifdef NEED_BC
       char	*BC;	/* :bc=: move cursor left */
#else
extern char	*BC;	/* :bc=: move cursor left */
#endif

/* HP-UX, and maybe some others, require the application code to supply
 * an "ospeed" variable.
 */
#ifdef NEED_OSPEED
# ifdef NEED_SPEED_T
	speed_t ospeed;
# else
	short	ospeed;
# endif
#endif

/* This macro is used to reset attributes and colors to normal values. */
#define revert(tw)	if ((tw) == current)\
				change(colorinfo[COLOR_FONT_NORMAL].fg,\
					colorinfo[COLOR_FONT_NORMAL].bg,\
					colorinfo[COLOR_FONT_NORMAL].da.bits);\
			else\
				change(colorinfo[COLOR_FONT_IDLE].fg,\
					colorinfo[COLOR_FONT_IDLE].bg,\
					colorinfo[COLOR_FONT_IDLE].da.bits)

/* Structs of this type are used to remember the location and size of each 
 * window.  In the termcap interface, all windows must be as wide as the
 * screen, and the sum of all windows' heights must equal the screen size.
 */
typedef struct twin_s
{
	struct twin_s	*next;		/* some other window on this screen */
	int		height;		/* size of the window */
	int		pos;		/* position of the window */
	int		newheight;	/* height after screen is rearranged */
	int		newpos;		/* position after screen is rearranged */
	int		cursx, cursy;	/* logical cursor position */
	ELVCURSOR	shape;		/* logical cursor shape */
} TWIN;

static TWIN	*twins;		/* list of windows */
static TWIN	*current;	/* window with keyboard focus */

#if USE_PROTOTYPES
static ELVBOOL clrtoeol(GUIWIN *gw);
static ELVBOOL color(int fontcode, CHAR *colornam, ELVBOOL isfg, long *colorptr, unsigned char rgb[3]);
static ELVBOOL creategw(char *name,char * attributes);
static ELVBOOL focusgw(GUIWIN *gw);
static ELVBOOL scroll(GUIWIN *gw, int qty, ELVBOOL notlast);
static ELVBOOL shift(GUIWIN *gw, int qty, int rows);
static ELVBOOL tabcmd(GUIWIN *gw, _CHAR_ key2, long count);
static char *manynames(char *names);
static int init(int argc, char **argv);
static int keylabel(CHAR *given, int givenlen, CHAR **label, CHAR **rawptr);
static int test(void);
static int ttych(int ch);
static void beep(GUIWIN *gw);
static void destroygw(GUIWIN *gw, ELVBOOL force);
static void drawgraphic(GUIWIN *gw, long fg, long bg, int bits, CHAR *text, int len);
static void draw(GUIWIN *gw, long fg, long bg, int bits, CHAR *text, int len);
#ifdef FEATURE_SPLIT
static void drawborder(TWIN *tw);
#endif
static void endtcap(void);
static void flush(void);
static void loop(void);
static void mayhave(char **T, char *s);
static void movecurs(TWIN *tw);
static void moveto(GUIWIN *gw, int column, int row);
static void musthave(char **T, char *s);
static void pair(char **T, char **U, char *sT, char *sU);
static void change(long fg, long bg, int bits);
static void starttcap(void);
static void term(void);
static void ttyflush(void);
static void ttygetsize(void);
static ELVBOOL ttyprgopen(char *command, ELVBOOL willwrite, ELVBOOL willread);
static int ttyprgclose(void);
static RESULT stop(ELVBOOL alwaysfork);
#endif

static void reset P_((void));
static void chgsize P_((TWIN *tw, int newheight, ELVBOOL winch));
static void cursorshape P_((ELVCURSOR shape));

/* termcap values */
static ELVBOOL	AM;		/* :am:  boolean: auto margins? */
static ELVBOOL	PT;		/* :pt:  boolean: physical tabs? */
       char	PC;		/* :pc=: pad character (not a string var!) */
static char	*VB;		/* :vb=: visible bell */
       char	*UP;		/* :up=: move cursor up */
static char	*AF;		/* :AF=: change the foreground color */
static char	*SO;		/* :so=: standout start */
static char	*SE;		/* :se=: standout end */
static char	*US;		/* :us=: underline start */
static char	*UE;		/* :ue=: underline end */
static char	*MD;		/* :md=: bold start */
static char	*ME;		/* :me=: bold end */
static char	*MH;		/* :mh=: half-bright start (end with :me=:) */
static char	*CM;		/* :cm=: cursor movement */
static char	*DO;		/* :do=: move down one line */
static char	*DOmany;	/* :DO=: move down many lines */
static char	*CE;		/* :ce=: clear to end of line */
static char	*AL;		/* :al=: add a line */
static char	*ALmany;	/* :AL=: add many lines */
static char	*DL;		/* :dl=: delete a line */
static char	*DLmany;	/* :DL=: delete many lines */
static char	*SRev;		/* :sr=: scroll reverse */
static char	*KS;		/* :ks=: init string for cursor */
static char	*KE;		/* :ke=: restore string for cursor */
static char	*IC;		/* :ic=: insert following char */
static char	*ICmany;	/* :IC=: insert many characters */
static char	*DC;		/* :dc=: delete a character */
static char	*DCmany;	/* :DC=: delete many characters */
static char	*TI;		/* :ti=: terminal init */
static char	*TE;		/* :te=: terminal exit */
static char	*SC;		/* :sc=: save cursor position & attribute */
static char	*RC;		/* :rc=: restore cursor position & attribute */
static char	*CQ;		/* :cQ=: normal cursor */
static char	*CX;		/* :cX=: cursor used for EX command/entry */
static char	*CV;		/* :cV=: cursor used for VI command mode */
static char	*CI;		/* :cI=: cursor used for VI input mode */
static char	*CR;		/* :cR=: cursor used for VI replace mode */
#ifdef FEATURE_MISC
static char	*GS;		/* :GS=:as=: start graphic character mode */
static char	*GE;		/* :GE=:ae=: end graphic character mode */
static char	GC_V;		/* vertical bar character */
static char	GC_H;		/* horizontal bar character */
static char	GC_1;		/* lower left corner character */
static char	GC_2;		/* horizontal line with up-tick character */
static char	GC_3;		/* lower right corner character */
static char	GC_4;		/* vertical line with right-tick character */
static char	GC_5;		/* center cross character */
static char	GC_6;		/* vertical line with left-tick character */
static char	GC_7;		/* upper left corner character */
static char	GC_8;		/* horizontal line with down-tick character */
static char	GC_9;		/* upper right corner character */
#endif

/* This is a table of keys which should be mapped, if present */
static struct
{
	char	*label;		/* keytop legend of the key */
	char	*capnames;	/* name(s) of the key's capability */
	char	*cooked;	/* what the key should map to (if anything) */
	MAPFLAGS flags;		/* when the map should be effective */
	char	*rawin;		/* raw characters sent by key */
}
	keys[] =
{
	{"<Up>",	"ku",		"k",	MAP_ALL},
	{"<Down>",	"kd",		"j",	MAP_ALL},
	{"<Left>",	"kl",		"h",	MAP_ALL},
	{"<Right>",	"kr",		"l",	MAP_ALL},
	{"<PgUp>",	"PUkPk2",	"\002", MAP_ALL},
	{"<PgDn>",	"PDkNk5",	"\006", MAP_ALL},
	{"<Home>",	"HMkhK1",	"^",	MAP_ALL},
	{"<End>",	"ENkHK5@7",	"$",	MAP_ALL},
	{"<Insert>",	"kI",		"i",	MAP_ALL},
#ifdef FEATURE_MISC
	{"<Delete>",	"kD",		"x",	MAP_ALL},
	{"<Compose>",	"k+",		"\013",	MAP_INPUT},
	{"<C-Left>",	"#4KL",		"B",	MAP_ALL},
	{"<C-Right>",	"%iKR",		"W",	MAP_ALL},
	{"<S-Tab>",	"kB",		"g\t",	MAP_COMMAND},
#endif
	{"#1",		"k1"},
	{"#2",		"k2"},
	{"#3",		"k3"},
	{"#4",		"k4"},
	{"#5",		"k5"},
	{"#6",		"k6"},
	{"#7",		"k7"},
	{"#8",		"k8"},
	{"#9",		"k9"},
	{"#10",		"k0kak;"},
#ifdef FEATURE_MISC
	{"#1s",		"s1"},
	{"#2s",		"s2"},
	{"#3s",		"s3"},
	{"#4s",		"s4"},
	{"#5s",		"s5"},
	{"#6s",		"s6"},
	{"#7s",		"s7"},
	{"#8s",		"s8"},
	{"#9s",		"s9"},
	{"#10s",	"s0"},
	{"#1c",		"c1"},
	{"#2c",		"c2"},
	{"#3c",		"c3"},
	{"#4c",		"c4"},
	{"#5c",		"c5"},
	{"#6c",		"c6"},
	{"#7c",		"c7"},
	{"#8c",		"c8"},
	{"#9c",		"c9"},
	{"#10c",	"c0"},
	{"#1a",		"a1"},
	{"#2a",		"a2"},
	{"#3a",		"a3"},
	{"#4a",		"a4"},
	{"#5a",		"a5"},
	{"#6a",		"a6"},
	{"#7a",		"a7"},
	{"#8a",		"a8"},
	{"#9a",		"a9"},
	{"#10a",	"a0"}
#endif
};

/* These are GUI-dependent global options */
static OPTDESC goptdesc[] =
{
	{"term", "ttytype",		optsstring,	optisstring},
	{"ttyrows", "ttylines",		optnstring,	optisnumber},
	{"ttycolumns", "ttycolumns",	optnstring,	optisnumber},
	{"ttyunderline", "ttyu",	NULL,		NULL	   },
	{"ttyitalic", "ttyi",		NULL,		NULL	   },
	{"ttywrap", "ttyw",		NULL,		NULL	   }
};
struct ttygoptvals_s ttygoptvals;


/*----------------------------------------------------------------------------*/
/* These are mid-level terminal I/O functions.  They buffer the output, but
 * don't do much more than that.
 */
static char ttybuf[1500];	/* the output buffer */
static int  ttycount;		/* number of characters in ttybuf */
static char ttyerasekey;	/* taken from the ioctl structure */
long        ttycaught;		/* bitmap of recently-received signals */

/* This indicates whether we've found the termcap entry yet */
static int gotentry = 0;

/* This function writes the contents of ttybuf() to the screen */
static void ttyflush()
{
	if (ttycount > 0)
	{
		ttywrite(ttybuf, ttycount);
		ttycount = 0;
	}
}

/* This function is used internally.  It is passed to the tputs() function
 * which uses it to output individual characters.  This function saves the
 * characters in a buffer and outputs them in a bunch.
 */
static int ttych(ch)
	int	ch;	/* character to be output */
{
	ttybuf[ttycount++] = ch;
	if (ttycount >= QTY(ttybuf))
		ttyflush();
#ifndef NDEBUG
	ttybuf[ttycount] = '\0';
#endif
	return ch;
}


/* These store masks for resetting COLOR_XXX attribute bits.  Although each
 * of these is intended to reset only its own attribute bits, it is possible
 * that some terminals will use a single string for resetting multiple
 * attributes, so the masks must be computed at run time, after the termcap
 * entry has been read.
 */
static int UEmask; /* attributes reset by :ue=: -- normally UNDERLINE */
static int SEmask; /* attributes reset by :se=: -- normally SET (standout) */
static int MEmask; /* attributes reset by :me=: -- normally BOLD and ITALIC */

static ELVBOOL	fgcolored;	/* have foreground colors been set? */
static ELVBOOL	bgcolored;	/* have background colors been set? */

static long	currentfg;	/* foreground color code */
static long	currentbg;	/* background color code */
static int	currentbits;	/* bitmap of other attributes */

static int	viscreen;	/* is terminal switched to edit screen yet? */ 

/* Change attributes.  The color support assumes your terminal is ANSI-like
 * (which is safe since color() only allows you to set colors for ANSI-like
 * terminals) but the rest of the code is purely termcap-driven.
 */
static void change(fg, bg, bits)
	long	fg;	/* new foreground color, if fgcolored */
	long	bg;	/* new background color, if bgcolored */
	int	bits;	/* attribute bits */
{
	char	ansicolor[50];
	int	resetting, setting;

	/* Don't change colors while the other screen is being displayed */
	if (!viscreen)
		return;

	/* If foreground is set to a bright color, then we want to convert it
	 * to either a dim color + bold (if bold wasn't set already), or to
	 * white + bold (if bold was already set).
	 */
	if (fgcolored && fg >= 10)
	{
		if (bits & COLOR_BOLD)
			fg = 7;
		else
			fg -= 10, bits |= COLOR_BOLD;
	}

	/* Italics are shown via underlining, if there is no :mh=: string */
	if (!MH && (bits & COLOR_ITALIC))
		bits = (bits & ~COLOR_ITALIC) | COLOR_UNDERLINED;

	/* Underlining is disabled if "nottyunderline" and we have bg color */
	/* If "nottyitalic", then never use underline ever */
	if (!o_ttyunderline && (!o_ttyitalic || bgcolored))
		bits &= ~COLOR_UNDERLINED;

	/* Italic is disabled if "nottyitalic" */
	if (!o_ttyitalic)
		bits &= ~COLOR_ITALIC;

	/* Termcap doesn't allow bold & italics to mix.  If attempting to mix,
	 * then use plain bold.
	 */
	if ((bits & (COLOR_BOLD|COLOR_ITALIC)) == (COLOR_BOLD|COLOR_ITALIC))
		bits &= ~COLOR_ITALIC;

	/* Okay, we are finally ready to begin outputting escape sequences.
	 * This proceeds in three phases: First we turn off any attributes
	 * that must be off, then we change colors, and finally we turn on
	 * any attributes which should be on.  We do it this way because
	 * sometimes turning off an attribute will affect colors or other
	 * attributes.
	 */

	/* Decide which attributes to turn off. */
	resetting = currentbits & ~bits;
	if ((resetting & (COLOR_BOLD|COLOR_ITALIC)) && ME)
	{
		tputs(ME, 1, ttych);
		currentbits &= MEmask;
		currentfg = currentbg = -1;
	}
	if ((resetting & COLOR_UNDERLINED) && UE)
	{
		tputs(UE, 1, ttych);
		currentbits &= UEmask;
		currentfg = currentbg = -1;
	}
	if ((resetting & (COLOR_BOXED|COLOR_LEFTBOX|COLOR_RIGHTBOX)) && SE)
	{
		tputs(SE, 1, ttych);
		currentbits &= SEmask;
		currentfg = currentbg = -1;
	}

	/* Change foreground, if set & different */
	if (fgcolored && fg != currentfg)
	{
		/* Try to fold bold & bg into the change */
		if (bgcolored && bg != currentbg)
		{
			if (bg < 8)
				sprintf(ansicolor, "\033[%ld;%ld%sm", bg + 40,
					fg + 30, bits & COLOR_BOLD ? ";1" : "");
			else
			{
				sprintf(ansicolor, "\033[m\033[%ld%sm",
					fg + 30, bits & COLOR_BOLD ? ";1" : "");
				currentbits = 0;
			}
			tputs(ansicolor, 1, ttych);
			currentbits |= (bits & COLOR_BOLD);
			currentbg = bg;
		}
		else
		{
			sprintf(ansicolor, "\033[%ld%sm", 
					fg + 30, bits & COLOR_BOLD ? ";1" : "");
			tputs(ansicolor, 1, ttych);
			currentbits |= (bits & COLOR_BOLD);
		}
		currentfg = fg;
	}

	/* Change background, if set & different */
	if (bgcolored && bg != currentbg)
	{
		/* Just change the background */
		if (bg < 8)
			sprintf(ansicolor, "\033[%ldm", bg + 40);
		else
		{
			strcpy(ansicolor, "\033[m");
			currentbits = 0;
		}
		tputs(ansicolor, 1, ttych);
		currentbg = bg;
	}

	/* Set attributes */
	setting = bits & ~currentbits;
	if ((setting & COLOR_BOLD) && MD)
		tputs(MD, 1, ttych);
	if ((setting & COLOR_ITALIC) && MH)
		tputs(MH, 1, ttych);
	if ((setting & COLOR_UNDERLINED) && US)
		tputs(US, 1, ttych);
	if ((setting & (COLOR_BOXED|COLOR_LEFTBOX|COLOR_RIGHTBOX)) && SO)
		tputs(SO, 1, ttych);
	currentbits = bits;
}


/* Send any required termination strings.  Turn off "raw" mode. */
void ttysuspend()
{
	/* revert to the normal font */
	revert(NULL);

	/* restore some things */
	if (CQ) tputs(CQ, 1, ttych);	/* restore cursor shape */
	if (TE) tputs(TE, 1, ttych);	/* restore terminal mode/page */
	viscreen = 0;
	if (KE) tputs(KE, 1, ttych);	/* restore keypad mode */
	if (RC) tputs(RC, 1, ttych);	/* restore cursor & attributes */

	/* Move the cursor to the bottom of the screen */
	tputs(tgoto(CM, 0, (int)o_ttyrows - 1), 0, ttych);
	ttych('\n');
	ttyflush();

	/* change the terminal mode back the way it was */
	ttynormal();
}

/* Put the terminal in RAW mode.  Send any required strings */
void ttyresume(sendstr)
	ELVBOOL	sendstr;	/* send strings? */
{
	/* change the terminal mode to cbreak/noecho */
	ttyerasekey = ELVCTRL('H');/* the default */
	ttyraw(&ttyerasekey);

	/* send the initialization strings */
	if (sendstr)
	{
		ttych('\r');
		tputs(CE, (int)o_ttycolumns, ttych);
		if (TI) tputs(TI, 1, ttych);
		viscreen = 1;
		if (KS) tputs(KS, 1, ttych);
	}

	/* reset, so we don't try any suspicious optimizations */
	reset();
}

/* This function determines the screen size.  It does this by calling the
 * OS-dependent ttysize() function if possible, or from the termcap entry
 * otherwise.
 */
static void ttygetsize()
{
	int	lines;
	int	cols;
	char	*tmp;

	/* get the window size, one way or another. */
	lines = cols = 0;
	if (!ttysize(&lines, &cols) || lines < 2 || cols < 30)
	{
		/* get lines from $LINES or :li#: */
		tmp = getenv("LINES");
		if (tmp)
			lines = atoi(tmp);
		else
			lines = tgetnum("li");
		if (lines <= 0)
			lines = 24;

		/* get columns from $COLUMNS or :co#: */
		tmp = getenv("COLUMNS");
		if (tmp)
			cols = atoi(tmp);
		else
			cols = tgetnum("co");
		if (cols <= 0)
			cols = 80;
	}

	/* did we get a realistic value? */
	if (lines >= 2 && cols >= 30)
	{
		o_ttyrows = lines;
		o_ttycolumns = cols;
	}
	optflags(o_ttyrows) |= OPT_HIDE;
	optflags(o_ttycolumns) |= OPT_HIDE;
}


/* end of low-level terminal control */
/*----------------------------------------------------------------------------*/
/* start of termcap operations */

static char	*capbuf;/* used for allocation space for termcap strings */

/* This function fetches an optional string from termcap */
static void mayhave(T, s)
	char	**T;	/* where to store the returned pointer */
	char	*s;	/* name of the capability */
{
	char	*val;

	val = tgetstr(s, &capbuf);
	if (val)
	{
		*T = val;
	}
}


/* This function fetches a required string from termcap */
static void musthave(T, s)
	char	**T;	/* where to store the returned pointer */
	char	*s;	/* name of the capability */
{
	mayhave(T, s);
	if (!*T)
	{
		msg(MSG_FATAL, "[s]termcap needs $1", s);
	}
}


/* This function fetches a pair of strings from termcap.  If one of them is
 * missing, then the other one is ignored.
 */
static void pair(T, U, sT, sU)
	char	**T;	/* where to store the first pointer */
	char	**U;	/* where to store the second pointer */
	char	*sT;	/* name of the first capability */
	char	*sU;	/* name of the second capability */
{
	mayhave(T, sT);
	mayhave(U, sU);
	if (!*T || !*U)
	{
		*T = *U = "";
	}
}

/* This function gets a single termcap string in a special static buffer.
 * Returns the string if successful, or NULL if unsuccessful.
 */
static char *manynames(names)
	char	*names; /* possible names (each pair of chars is one name) */
{
	char	name[3];
	int	i;
	char	*value;

	/* for each possible name... */
	for (i = 0; names[i]; i += 2)
	{
		/* see if the termcap string can be found */
		name[0] = names[i];
		name[1] = names[i + 1];
		name[2] = '\0';
		value = tgetstr(name, &capbuf);
		if (value)
		{
			/* found! */
			return value;
		}
	}
	return NULL;
}


/* get termcap values */
static void starttcap()
{
	static char	cbmem[800];
	char		*str;
	int		i;

	/* make sure TERM variable is set */
	o_term = toCHAR(ttytermtype());
	if (!o_term)
	{
		o_term = toCHAR("unknown");
	}
	optflags(o_term) |= OPT_SET|OPT_LOCK|OPT_NODFLT;

	/* If the background option wasn't set via the $ELVISBG environment
	 * variable, then we should try to guess whether this terminal has
	 * a light or dark background.  Our guess: The background is probably
	 * dark unless this is an xterm-like terminal emulator, or KDE's "kvt"
	 * which breaks a lot of xterm conventions, or has a name that ends
	 * with "-r" or "-rv" -- those two suffixes commonly denote reverse
	 * video versions of termcap/terminfo entries.
	 */
	if (getenv("ELVISBG") == NULL)
	{
		o_background = 'd';	/* "dark" */
		if (getenv("WINDOWID") != NULL
		 || !CHARcmp(o_term, toCHAR("kvt"))
		 || !CHARcmp(o_term, toCHAR("xterm"))
		 || ((str = strrchr(tochar8(o_term), '-')) != NULL
			&& str[1] == 'r'))
		{
			o_background = 'l';	/* "light" */
		}
	}

	/* allocate memory for capbuf */
	capbuf = cbmem;

	/* get the termcap entry */
	if (!gotentry)
	{
		switch (tgetent(ttybuf, tochar8(o_term)))
		{
		  case -1:	msg(MSG_FATAL, "termcap database unreadable");
		  case 0:	msg(MSG_FATAL, "[S]TERM=$1 unknown", o_term);
		}
	}

	/* get strings */
	musthave(&UP, "up");
	BC = "\b";
	mayhave(&BC, "bc");
	mayhave(&VB, "vb");
	musthave(&CM, "cm");
	DO = "\n";
	mayhave(&DO, "do");
	mayhave(&DOmany, "DO");
	mayhave(&TI, "ti");
	mayhave(&TE, "te");
	pair(&SC, &RC, "sc", "rc");	/* cursor save/restore */
	pair(&KS, &KE, "ks", "ke");	/* keypad enable/disable */
	mayhave(&AF, "AF");
	if (tgetnum("sg") <= 0)
	{
		pair(&SO, &SE, "so", "se");
	}
	if (tgetnum("ug") <= 0)
	{
		pair(&US, &UE, "us", "ue");
		/* bor: some terminals don't have :md:, but _do_ have :me:
		   Check, that we can switch attribute off before using it
		 */
		mayhave(&ME, "me");
		if (ME && *ME)
		{
			mayhave(&MD, "md");
			mayhave(&MH, "mh");
			mayhave(&str, "mr");
			if (str && (!SO || !strcmp(str, SO)))
			{
				SO = str;
				SE = ME;
			}
		}
	}
	mayhave(&ICmany, "IC");
	mayhave(&IC, "ic");
	mayhave(&DCmany, "DC");
	mayhave(&DC, "dc");
	mayhave(&ALmany, "AL");
	mayhave(&AL, "al");
	mayhave(&DLmany, "DL");
	mayhave(&DL, "dl");
	musthave(&CE, "ce");
	mayhave(&SRev, "sr");

	/* cursor shapes */
	CQ = tgetstr("cQ", &capbuf);
	if (CQ)
	{
		CX = tgetstr("cX", &capbuf);
		if (!CX) CX = CQ;
		CV = tgetstr("cV", &capbuf);
		if (!CV) CV = CQ;
		CI = tgetstr("cI", &capbuf);
		if (!CI) CI = CQ;
		CR = tgetstr("cR", &capbuf);
		if (!CR) CR = CQ;
	}
	else
	{
		CQ = CV = "";
		pair(&CQ, &CV, "ve", "vs");
		CX = CI = CQ;
		CR = CV;
	}

#ifdef FEATURE_MISC
	/* graphic characters */
	str = tgetstr("ac", &capbuf);
	if (str)
	{
		/* apparently we're using the :as=:ae=:ac=: style */
		pair(&GS, &GE, "as", "ae");
		for (i = 0; str[i] && str[i + 1]; i += 2)
		{
			switch (str[i])
			{
			  case 'q':	GC_H = str[i + 1];	break;
			  case 'x':	GC_V = str[i + 1];	break;
			  case 'm':	GC_1 = str[i + 1];	break;
			  case 'v':	GC_2 = str[i + 1];	break;
			  case 'j':	GC_3 = str[i + 1];	break;
			  case 't':	GC_4 = str[i + 1];	break;
			  case 'n':	GC_5 = str[i + 1];	break;
			  case 'u':	GC_6 = str[i + 1];	break;
			  case 'l':	GC_7 = str[i + 1];	break;
			  case 'w':	GC_8 = str[i + 1];	break;
			  case 'k':	GC_9 = str[i + 1];	break;
			}
		}
	}
	else
	{
		/* maybe we have :GH=:GV=:... strings? */
		if ((str = tgetstr("GH", &capbuf)) != NULL)	GC_H = *str;
		if ((str = tgetstr("GV", &capbuf)) != NULL)	GC_V = *str;
		if ((str = tgetstr("G3", &capbuf)) != NULL)	GC_1 = *str;
		if ((str = tgetstr("GU", &capbuf)) != NULL)	GC_2 = *str;
		if ((str = tgetstr("G4", &capbuf)) != NULL)	GC_3 = *str;
		if ((str = tgetstr("GR", &capbuf)) != NULL)	GC_4 = *str;
		if ((str = tgetstr("GC", &capbuf)) != NULL)	GC_5 = *str;
		if ((str = tgetstr("GL", &capbuf)) != NULL)	GC_6 = *str;
		if ((str = tgetstr("G2", &capbuf)) != NULL)	GC_7 = *str;
		if ((str = tgetstr("GD", &capbuf)) != NULL)	GC_8 = *str;
		if ((str = tgetstr("G1", &capbuf)) != NULL)	GC_9 = *str;
		pair(&GS, &GE, "GS", "GE");

		/* if we have no :GS=:GE=: strings, then set MSB of chars */
		if (!GS || !*GS)
		{
			if (GC_H) GC_H |= 0x80;
			if (GC_V) GC_V |= 0x80;
			if (GC_1) GC_1 |= 0x80;
			if (GC_2) GC_2 |= 0x80;
			if (GC_3) GC_3 |= 0x80;
			if (GC_4) GC_4 |= 0x80;
			if (GC_5) GC_5 |= 0x80;
			if (GC_6) GC_6 |= 0x80;
			if (GC_7) GC_7 |= 0x80;
			if (GC_8) GC_8 |= 0x80;
			if (GC_9) GC_9 |= 0x80;
		}
	}
#endif /* FEATURE_MISC */

	/* key strings */
	for (i = 0; i < QTY(keys); i++)
	{
		keys[i].rawin = manynames(keys[i].capnames);
	}

	/* other termcap stuff */
	AM = (ELVBOOL)(tgetflag("am") && !tgetflag("xn"));
	PT = (ELVBOOL)tgetflag("pt");

	/* Compute the attribute resetting masks.  Sometimes the same string
	 * will reset more than one attribute, so we need to check for identical
	 * strings and combine their effect on attributes.
	 */
	SEmask = ~(COLOR_BOXED|COLOR_LEFTBOX|COLOR_RIGHTBOX);
	UEmask = ~COLOR_UNDERLINED;
	MEmask = ~(COLOR_BOLD|COLOR_ITALIC);
	if (SE && UE && !strcmp(SE, UE))
		SEmask = UEmask &= SEmask;
	if (SE && ME && !strcmp(SE, ME))
		SEmask = MEmask &= SEmask;
	if (UE && ME && !strcmp(UE, ME))
		UEmask = MEmask &= UEmask;

	/* change the terminal mode to cbreak/noecho */
	ttyinit();
	if (SC) tputs(SC, 1, ttych);
	ttyresume(ElvTrue);

	/* try to get true screen size, from the operating system */
	ttygetsize();
}

static void endtcap()
{
	/* change the terminal mode back the way it was */
	ttysuspend();
}


/* end of termcap operations */
/*----------------------------------------------------------------------------*/
/* start of GUI functions */


static int	afterprg;	/* expose windows (after running prg) */
static int	afterscrl;	/* number of status lines (after running prg) */
static int	physx, physy;	/* physical cursor position */
static int	nwindows;	/* number of windows allocated */

/*----------------------------------------------------------------------------*/

/* This is an internal function which moves the physical cursor to the logical
 * position of the cursor in a given window, if it isn't there already.
 */
static void movecurs(tw)
	TWIN	*tw;	/* window whose cursor is to be moved */
{
	int	y = tw->pos + tw->cursy;
	int	i;

	/* maybe we don't need to move at all? */
	/* JohnW 13/08/96 > 1 was > 0 */
	if ((afterprg > 1 && y <= o_ttyrows - afterscrl)
		|| (y == physy && tw->cursx == physx))
	{
		/* already there */
		return;
	}

        if (afterprg == 1)  /* JohnW 13/08/96 - only use the bottom line */
        {
		/* We get the 'bottom' line by adding the difference between
		 * the window's bottom line and the screen's bottom line. This
		 * then allows scrolling of an input line and moving back onto
		 * the penultimate line */
		tputs(tgoto(CM, tw->cursx, y + ((int)o_ttyrows - (tw->pos + tw->height))), 1, ttych);
                physx = tw->cursx;
                return;
	}
	else
	/* Try some simple alternatives to the CM string */
	if (y >= physy
	 && (y - physy < gui->movecost || DOmany)
	 && (tw->cursx == 0 || tw->cursx == physx))
	{
		/* output a bunch of newlines, and maybe a carriage return */
		if (y - physy > 1 && DOmany)
			tputs(tgoto(DOmany, y - physy, y - physy), 1, ttych);
		else
			for (i = y - physy; i > 0; i--)
				tputs(DO, 1, ttych);
		if (tw->cursx != physx)
			ttych('\r');
	}
	else if (y == physy && tw->cursx < physx && physx - tw->cursx < gui->movecost)
	{
		/* output a bunch of backspaces */
		for (i = physx - tw->cursx; i > 0; i--)
			tputs(BC, 0, ttych);
	}
	/* many other special cases could be handled here */
	else
	{
		/* revert to the normal font */
		revert(tw);

		/* move it the hard way */
		tputs(tgoto(CM, tw->cursx, y), 1, ttych);
	}

	/* done! */
	physx = tw->cursx;
	physy = tw->cursy + tw->pos;
}

/* clear to end of line */
static ELVBOOL clrtoeol(gw)
	GUIWIN	*gw;	/* window whose row is to be cleared */
{
	TWIN	*tw = (TWIN *)gw;

	/* after running a program, disable the :ce: string for a while. */
	/* JohnW 13/08/96 Not disabled for the bottom line of window */
	if (afterprg && tw->cursy != (int)tw->height - 1)
		return ElvTrue;

	/* if we're on the bottom row of a window which doesn't end at the
	 * bottom of the screen, then fail.  This will cause elvis to output
	 * a bunch of spaces instead.  The draw() function will convert those
	 * spaces to underscore characters so the window has a border.
	 */
	if ( !afterprg && /* JohnW 13/08/96 Not after a prog... */
	    tw->cursy == tw->height - 1 && tw->pos + tw->height != o_ttyrows)
	{
		return ElvFalse;
	}

	/* revert to the normal font */
	revert(tw);

	/* move the physical cursor to where the window thinks it should be */
	movecurs(tw);

	/* output the clear-to-eol string */
	tputs(CE, (int)(o_ttycolumns - tw->cursx), ttych);

	return ElvTrue;
}

/* insert or delete columns */
static ELVBOOL shift(gw, qty, rows)
	GUIWIN	*gw;	/* window to be shifted */
	int	qty;	/* columns to insert (may be negative to delete) */
	int	rows;	/* number of rows affected (always 1 for this GUI) */
{
	/* revert to the normal font */
	revert((TWIN *)gw);

	/* move the physical cursor to where this window thinks it is */
	movecurs((TWIN *)gw);

	if (qty > 0)
	{
		/* can we do many at once? */
		if (qty > 1 && ICmany)
		{
			tputs(tgoto(ICmany, qty, qty), 1, ttych);
		}
		else if (IC)
		{
			for (; qty > 0; qty--)
			{
				tputs(IC, 1, ttych);
			}
		}
		else
		{
			/* don't know how to insert */
			return ElvFalse;
		}
	}
	else
	{
		/* take the absolute value of qty */
		qty = -qty;

		/* can we do many deletions at once? */
		if (qty > 1 && DCmany)
		{
			tputs(tgoto(DCmany, qty, qty), 1, ttych);
		}
		else if (DC)
		{
			for (; qty > 0; qty--)
			{
				tputs(DC, 1, ttych);
			}
		}
		else
		{
			/* don't know how to delete */
			return ElvFalse;
		}
	}
	return ElvTrue;
}

/* insert or delete rows.  qty is positive to insert, negative to delete */
static ELVBOOL scroll(gw, qty, notlast)
	GUIWIN	*gw;	/* window to be scrolled */
	int	qty;	/* rows to insert (may be nagative to delete) */
	ELVBOOL notlast;/* if ElvTrue, then leave last row unchanged */
{
	TWIN	*tw = (TWIN *)gw;
	char	*op;

	/* Mentally adjust the number of rows used for messages.  This is only
	 * significant immediately after running an external program, and is
	 * used for hiding any premature attempts to redraw the window's text
	 * but still show the window's messages.
	 */
	afterscrl -= qty;

	/* If this window isn't the only window, then fail.
	 * Later, this function may be smart enough to use scrolling regions,
	 * or do the idlok() kind of thing, but not yet.
	 */
	if (twins->next)
	{
		return ElvFalse;
	}

	/* revert to the normal font */
	revert(tw);

	/* move the physical cursor to where the window thinks it should be */
	movecurs(tw);

	if (qty > 0)
	{
		/* we'll be inserting.  Can we do it all at once? */
		if (ALmany && !(AL && qty == 1))
		{
			/* all at once */
			tputs(tgoto(ALmany, qty, qty), tw->height - tw->cursy, ttych);
		}
		else
		{
			/* try to use SRev */
			op = (tw->cursy == 0 && tw->pos == 0 && SRev) ? SRev : AL;

			/* if we don't know how to do this, we're screwed */
			if (!op || qty > o_ttyrows / 2)
			{
				return ElvFalse;
			}

			/* a bunch of little insertions */
			while (qty > 0)
			{
				tputs(op, tw->height - tw->cursy, ttych);
				qty--;
			}
		}
	}
	else
	{
		/* take the absolute value of qty */
		qty = -qty;

		/* we'll be deleting.  Can we do it all at once? */
		if (DLmany && !(DL && qty == 1))
		{
			/* all at once */
			tputs(tgoto(DLmany, qty, qty), tw->height - tw->cursy, ttych);
		}
		else
		{
			/* try to use newline */
			op = DL; /* but don't try very hard, for now! */

			/* if we don't know how to do this, we're screwed */
			if (!op || qty > o_ttyrows / 2)
			{
				return ElvFalse;
			}

			/* a bunch of little deletions */
			while (qty > 0)
			{
				tputs(op, tw->height - tw->cursy, ttych);
				qty--;
			}
		}
	}
	return ElvTrue;
}

/* Forget where the cursor is, and which mode we're in */
static void reset()
{
	physx = physy = 9999;
	currentfg = currentbg = -1L;
	currentbits = -1;
}


/* Flush any changes out to the display */
static void flush()
{
	if (current)
	{
		movecurs(current);
	}
	ttyflush();
}

/* Set a window's cursor position. */
static void moveto(gw, column, row)
	GUIWIN	*gw;	/* window whose cursor is to be moved */
	int	column;	/* new column of cursor */
	int	row;	/* new row of cursor */
{
	assert(twins);
	((TWIN *)gw)->cursx = column;
	((TWIN *)gw)->cursy = row;
}


/* put graphic characters.  This function is called only from draw() */
static void drawgraphic(gw, fg, bg, bits, text, len)
	GUIWIN	*gw;	/* window where text should be drawn */
	long	fg;	/* foreground color */
	long	bg;	/* background color */
	int	bits;	/* attribute bits */
	CHAR	*text;	/* plain chars to be mapped to graphic chars */
	int	len;	/* length of text */
{
	TWIN	*tw = (TWIN *)gw;
	int	i;
#ifdef FEATURE_MISC
	ELVBOOL	graf = ElvFalse;
	char	gc;
#endif

	/* Change attributes if necessary.  Note that we mask out COLOR_GRAPHIC
	 * since COLOR_GRAPHIC is handled within this function.
	 */
	change(fg, bg, bits & ~COLOR_GRAPHIC);

	/* draw each character */
	for (i = 0; i < len; i++)
	{
#ifndef FEATURE_MISC
		if (elvdigit(text[i]))
			ttych('+');
		else
			ttych(text[i]);
#else /* FEATURE_MISC */
		/* try to convert plain character to graphic character */
		switch (text[i])
		{
		  case '-':	gc = GC_H;	break;
		  case '|':	gc = GC_V;	break;
		  case '1':	gc = GC_1;	break;
		  case '2':	gc = GC_2;	break;
		  case '3':	gc = GC_3;	break;
		  case '4':	gc = GC_4;	break;
		  case '5':	gc = GC_5;	break;
		  case '6':	gc = GC_6;	break;
		  case '7':	gc = GC_7;	break;
		  case '8':	gc = GC_8;	break;
		  case '9':	gc = GC_9;	break;
		  default:	gc = 0;
		}
		
		/* did we get a graphic character? */
		if (gc)
		{
			/* output the graphic character in graphic mode */
			if (!graf && *GS)
			{
				tputs(GS, 1, ttych);
				graf = ElvTrue;
			}
			ttych(gc);
		}
		else
		{
			/* output elvis' plain character in text mode */
			if (graf)
			{
				tputs(GE, 1, ttych);
				graf = ElvFalse;
			}
			if (elvdigit(text[i]))
				ttych('+');
			else
				ttych(text[i]);
		}
#endif /* FEATURE_MISC */
	}

#ifdef FEATURE_MISC
	/* if still in graphic mode, then revert to text mode now */
	if (graf && GE)
	{
		tputs(GE, 1, ttych);
	}
#endif

	/* drawing the characters has the side-effect of moving the cursor */
	tw->cursx += len;
	physx += len;
	if (physx == o_ttycolumns)
	{
		if (o_ttywrap && AM)
			physx = 0, physy++;
		else
			physx = physy = 9999;
	}
}


/* put characters: first move, then set attribute, then execute char.
 */
static void draw(gw, fg, bg, bits, text, len)
	GUIWIN	*gw;	/* window where text should be drawn */
	long	fg;	/* foreground color */
	long	bg;	/* background color */
	int	bits;	/* other attributes */
	CHAR	*text;	/* text to draw */
	int	len;	/* length of text */
{
	TWIN	*tw = (TWIN *)gw;
	int	i;

	/* if COLOR_LEFTBOX is set, but not COLOR_BOXED, then draw in two
	 * phases: First draw a single character with COLOR_LEFTBOX, and then
	 * draw the remainder without COLOR_LEFTBOX.
	 */
	if ((bits & (COLOR_LEFTBOX|COLOR_BOXED)) == COLOR_LEFTBOX && len > 1)
	{
		draw(gw, fg, bg, bits, text, 1);
		bits &= ~COLOR_LEFTBOX;
		text++;
		len--;
	}

	/* After a program, don't output any text except messages for a while.
	 * This is mostly an optimization; the window is about to be redrawn
	 * from scratch anyway.  But it also prevents the screen from doing
	 * strange, unexpected things.
	 */
	if (afterprg > 0)
	{
		if (tw->cursy < tw->height - afterscrl - 1)
			return;
		else if (tw->cursx > 0 && afterprg == 2) /* JohnW not for 1 */
		{
			ttych('\r');
			ttych('\n');
			ttyflush();
			bits = 0;
		}
	}

	/* if this terminal has :am: automargins (without :xm:), then we
	 * must never draw a character in the last column of the last row.
	 */
	if ((o_ttywrap || AM)
		&& tw->pos + tw->cursy == o_ttyrows - 1
		&& tw->cursx + len >= o_ttycolumns)
	{
		len = o_ttycolumns - 1 - tw->cursx;
		if (len <= 0)
		{
			return;
		}
	}

	/* move the cursor to where this window thinks it is */
	movecurs(tw);

	/* if graphic characters, then handle specially */
	if (bits & COLOR_GRAPHIC)
	{
		drawgraphic(gw, fg, bg, bits, text, len);
		return;
	}

	/* If we're on the bottom row of a window (except the last window)
	 * then any normal characters should be underlined.  This will give
	 * us the effect of a window border.
	 */
	if (tw->cursy == tw->height - 1 && physy < o_ttyrows - 1)
	{
		bits |= COLOR_UNDERLINED;
	}

	/* adjust the colors & attributes */
	change(fg, bg, bits);

	/* Draw each character.  If this is the bottom row of any window except
	 * the bottom window, and underlining is not available, then replace
	 * any blanks with '_' characters.  This will provide a window border.
	 */
	if (tw->cursy == tw->height - 1
	 && physy < o_ttyrows - 1
	 && (!US || !o_ttyunderline))
	{
		for (i = 0; i < len; i++)
		{
			ttych(text[i] == ' ' ? '_' : text[i]);
		}
	}
	else /* normal row */
	{
		for (i = 0; i < len; i++)
		{
			ttych(text[i]);
		}
	}

	/* drawing the characters has the side-effect of moving the cursor */
	tw->cursx += len;
	physx += len;
	if (physx == o_ttycolumns)
	{
		if (o_ttywrap && AM)
			physx = 0, physy++;
		else
			physx = physy = 9999;
	}
}



/* return ElvTrue if termcap is available. */
static int test()
{
	char	*term;
	char	dummy[40], *dummyptr;

	/* get terminal type.  If no type, then return 0 */
	term = ttytermtype();
	if (!term)
		return 0;

	/* find termcap entry.  If none, then return 0 */
	if (tgetent(ttybuf, term) != 1)
		return 0;
	gotentry = 1;

	/* check for some required strings and numbers */
	if (tgetnum("co") < 40
	 || tgetnum("li") < 3
	 || tgetstr("up", (dummyptr = dummy, &dummyptr)) == NULL
	 || tgetstr("cm", (dummyptr = dummy, &dummyptr)) == NULL
	 || tgetstr("ce", (dummyptr = dummy, &dummyptr)) == NULL)
		return 0;

	/* no obvious problems, so termcap should work */
	return 1;
}


/* initialize the termcap interface. */
static int init(argc, argv)
	int	argc;	/* number of command-line arguments */
	char	**argv;	/* values of command-line arguments */
{
	int	i;

	/* add the global options to the list known to :set */
	o_ttyunderline = ElvTrue;
	o_ttyitalic = ElvTrue;
	o_ttywrap = ElvTrue;
	optinsert("tcap", QTY(goptdesc), goptdesc, &ttygoptvals.term);

	/* initialize the termcap stuff */
	starttcap();

	/* pretend the cursor is in an impossible place, so we're guaranteed
	 * to move it on the first moveto() or draw() call.
	 */
	physx = physy = -100;

	/* map the arrow keys */
	for (i = 0; i < QTY(keys); i++)
	{
		if (keys[i].cooked && keys[i].rawin)
		{
			mapinsert(toCHAR(keys[i].rawin), (int)strlen(keys[i].rawin),
				toCHAR(keys[i].cooked), (int)strlen(keys[i].cooked),
				toCHAR(keys[i].label),
				keys[i].flags, NULL);
		}
	}

	return argc;
}

/* change the shape of the cursor */
static void cursorshape(shape)
	ELVCURSOR	shape;	/* desired cursor shape */
{
	char		*esc;		/* desired shape */
 static char		*prevesc;	/* current shape */

	/* find the desired shape's escape sequence */
	switch (shape)
	{
	  case CURSOR_INSERT:	esc = CI;	break;
	  case CURSOR_REPLACE:	esc = CR;	break;
	  case CURSOR_COMMAND:	esc = CV;	break;
	  case CURSOR_QUOTE:	esc = CX;	break;
	  default:		esc = CQ;	break;
	}

	/* output it, if changed and non-empty */
	if (esc != prevesc && *esc)
	{
		tputs(esc, 1, ttych);
		prevesc = esc;
	}
}

/* Repeatedly get events (keystrokes), and call elvis' event functions */
static void loop()
{
	char	buf[20];
	int	len;
	int	timeout = 0;
	MAPSTATE mst = MAP_CLEAR;
	TWIN	*scan;

	/* peform the -c command or -t tag */
	if (mainfirstcmd(windefault))
		return;

	while (twins)
	{
		/* reset the ttycaught bitmap */
		ttycaught = 0;

		/* if no window is current, then make the newest current */
		if (!current)
		{
			current = twins;
			eventfocus((GUIWIN *)current, ElvTrue);
		}

		/* redraw the window(s) */
		{
			/* redraw each window; the current one last */
			for (scan = twins; scan; scan = scan->next)
			{ 
				if (scan != current)
				{
					scan->shape = eventdraw((GUIWIN *)scan);
				}
			}
			current->shape = eventdraw((GUIWIN *)current);
			movecurs(current);

			/* make the cursor be this window's shape */
			cursorshape(current->shape);
		}

		/* choose a timeout value */
		switch (mst)
		{
		  case MAP_CLEAR:	timeout = 0;		break;
		  case MAP_USER:	timeout = o_usertime;	break;
		  case MAP_KEY:		timeout = o_keytime;	break;
		}

		/* read events */
		ttyflush();
		len = ttyread(buf, sizeof buf, timeout);

		/* process keystroke data */
		if (len == -2)
		{
			/* ttyread() itself did something.  We don't need to
			 * do anything except the usual screen updates.
			 */
		}
		else if (len == -1)
		{
			/* Maybe the screen was resized?  Get new size */
			ttygetsize();

			/* Resize the windows to match the new screen.  The
			 * easiest way to do this is to "change" the size of the
			 * current window to its original size and force the
			 * other windows to compensate.  If there is only one
			 * window, then should be resized to the screen size.
			 */
			chgsize(current, twins->next ? current->height : (int)o_ttyrows, ElvTrue);
		}
		else
		{
			/* If this is the erase key (as specified by the
			 * serial port's configuration) then make sure elvis
			 * sees it as a backspace character.
			 */
			if (len == 1 && buf[0] == ttyerasekey)
				buf[0] = '\b';

			mst = eventkeys((GUIWIN *)current, toCHAR(buf), len);

			/* if first keystroke after running an external
			 * program, then we need to expose every window.
			 *
			 * JohnW 13/08/96 - try to sort out keeping program
			 * output on screen until vi mode is entered again.
                         * We have some program output on the screen and we
                         * may want to keep looking at it while we do another
                         * command, so don't do a big refresh until we're
                         * out of bottom-line mode
			 *
			 * Steve 9/12/96 - switched it back, partially, because
			 * moving the ttyresume() call to the afterprg==2
			 * state caused problems with terminals which switch
			 * screens.
			 */
			if (afterprg == 1 && /* JohnW added next bit */
				  windefault && windefault->state &&
				  !(windefault->state->flags & ELVIS_BOTTOM))
			{
				/* reset the flag BEFORE exposing windows,
				 * or else the eventexpose() won't work right.
				 */
				afterprg = 0;
				ttyresume(ElvTrue);
				for (scan = twins; scan; scan = scan->next)
				{ 
					eventexpose((GUIWIN *)scan, 0, 0,
					    scan->height - 1, (int)(o_ttycolumns - 1));
				}
			}
			else if (afterprg == 2)
			{
				/* it became 2 while processing the earlier
				 * keystrokes.  Set it to 1 now, so we'll
				 * read one more keystroke before exposing
				 * all the windows.
				 */
				ttyresume(ElvFalse);
				afterprg = 1;
			}
		}
	}
}

/* shut down the termcap interface */
static void term()
{
	cursorshape(CURSOR_NONE);
	endtcap();
}


#ifndef FEATURE_SPLIT
# define drawborder(tw)

static void chgsize(tw, newheight, winch)
	TWIN	*tw;		/* window to be resized */
	int	newheight;	/* desired height of window */
	ELVBOOL	winch;		/* Did the whole screen change size? */
{
	tw->height = newheight;
	if (winch)
		eventresize((GUIWIN *)tw, tw->height, (int)o_ttycolumns);
}

/* This function creates a window */
static ELVBOOL creategw(name, firstcmd)
	char	*name;		/* name of new window's buffer */
	char	*firstcmd;	/* first command to run in window */
{
	/* only allow one window */
	if (twins)
		return ElvFalse;

	/* create a window */
	twins = (TWIN *)safealloc(1, sizeof(TWIN));

	/* initialize the window */
	twins->height = o_ttyrows;
	twins->pos = 0;
	twins->cursx = twins->cursy = 0;
	twins->shape = CURSOR_NONE;
	nwindows = 1;

	/* make elvis do its own initialization */
	if (!eventcreate((GUIWIN *)twins, NULL, name, twins->height, (int)o_ttycolumns))
	{
		/* elvis can't make it -- fail */
		safefree(twins);
		return ElvFalse;
	}

	/* make the new window be the current window */
	current = twins;
	eventfocus((GUIWIN *)current, ElvTrue);

	/* execute the first command, if any */
	if (firstcmd)
	{
		winoptions(winofgw((GUIWIN *)twins));
		exstring(windefault, toCHAR(firstcmd), "+cmd");
	}

	return ElvTrue;
}


/* This function deletes a window */
static void destroygw(gw, force)
	GUIWIN	*gw;	/* window to be destroyed */
	ELVBOOL	force;	/* if ElvTrue, try harder */
{
	/* delete the window */
	twins = NULL;
	nwindows = 0;

	/* If this is the last window, move the cursor to the last line, and
	 * erase it.  If the buffer is going to be written, this is where the
	 * "wrote..." message will appear.
	 */
	revert(NULL);
	tputs(tgoto(CM, 0, (int)(o_ttyrows - 1)), 1, ttych);
	tputs(CE, 1, ttych);

	/* simulate a "destroy" event */
	eventdestroy(gw);

	/* free the storage */
	safefree(gw);
}

#else /* FEATURE_SPLIT */

/* This draws a bunch of underscores on the physical screen on the bottom
 * row of a window, if that window doesn't end at the bottom of the screen.
 * This should be called after a window is resized or moved.
 */
static void drawborder(tw)
	TWIN	*tw;	/* window whose border needs to be redrawn */
{
	int	col;

	/* if this window ends at the bottom of the screen, then do nothing */
	if (tw->pos + tw->height == o_ttyrows)
	{
		return;
	}

	/* move the physical cursor to the bottom of the window */
	tw->cursx = 0;
	tw->cursy = tw->height - 1;
	movecurs(tw);
	revert(tw);
	change(0L, 0L, COLOR_UNDERLINED);
	if (currentbits & COLOR_UNDERLINED)
	{
		for (col = 0; col < o_ttycolumns; col++)
		{
			ttych(' ');
		}
	}
	else
	{
		for (col = 0; col < o_ttycolumns; col++)
		{
			ttych('_');
		}
	}

	/* figure out where the physical cursor would be after that */
	if (o_ttywrap && AM)
		physx = 0, physy++;
	else
		physx = physy = 9999;
}

/* This function changes the height of a given window.  The total heights of
 * all windows must be o_ttyrows and the minimum height of each window is
 * MINHEIGHT.
 */
static void chgsize(tw, newheight, winch)
	TWIN	*tw;		/* window to be resized */
	int	newheight;	/* desired height of window */
	ELVBOOL	winch;		/* Did the whole screen change size? */
{
	TWIN	*scan;
	int	pos;
	int	otherheight;
	int	oldheight;
	int	toosmall;

	/* if the current window can't be as large as requested, then reduce
	 * the requested size.
	 */
	if ((nwindows - 1) * MINHEIGHT + newheight > o_ttyrows)
	{
		newheight = o_ttyrows - (nwindows - 1) * MINHEIGHT;
	}

	/* if window is already the requested height, we're done */
	if (tw->height == newheight && !winch)
	{
		return;
	}

	/* Set the size of the current window.  Also, adjust the sizes of other
	 * windows, and maybe their positions.  If any window other than the
	 * requested one is moved, expose it.  If any window other than the
	 * requested one is resized, then resize it.
	 */
	toosmall = 0;
	do
	{
		for (oldheight = tw->height, pos = 0, scan = twins;
		     scan;
		     pos += scan->newheight, scan = scan->next)
		{
			/* the requested window? */
			if (scan == tw)
			{
				/* yes, set it */
				scan->newpos = pos;
				scan->newheight = newheight;
			}
			else
			{
				/* no, some other window */

				/* compute the size that this window should be */
				if (!scan->next)
				{
					scan->newheight = o_ttyrows - pos;
					toosmall = MINHEIGHT - scan->newheight;
				}
				else if (scan->next == tw && !scan->next->next)
				{
					scan->newheight = o_ttyrows - pos - newheight;
					toosmall = MINHEIGHT - scan->newheight;
				}
				else
				{
					if (winch)
						otherheight = MINHEIGHT;
					else
						otherheight = scan->height * (o_ttyrows - newheight) / (o_ttyrows - oldheight);
					if (otherheight < MINHEIGHT)
					{
						scan->newheight = MINHEIGHT;
					}
					else
					{
						scan->newheight = otherheight - toosmall;
						if (scan->newheight < MINHEIGHT)
						{
							scan->newheight = MINHEIGHT;
						}
						toosmall -= otherheight - scan->newheight;
					}
				}
				scan->newpos = pos;
			}
		}
	} while (toosmall > 0);

	/* resize/redraw the windows, as necessary */
	for (scan = twins; scan; scan = scan->next)
	{
		/* set the size & position of this window.  If its
		 * size has changed then resize the window; else if
		 * its position has changed resize the window.
		 */
		if (scan == tw && !winch)
		{
			/* just remember new stats.  Calling function will
			 * call eventredraw() or eventdraw(), as necessary.
			 */
			scan->height = scan->newheight;
			scan->pos = scan->newpos;
		}
		else if (scan->newheight != scan->height || winch)
		{
			scan->height = scan->newheight;
			scan->pos = scan->newpos;
			if (scan->pos + scan->height < o_ttyrows)
			{
				drawborder(scan);
			}
			else
			{
				/* draw border the hard way: erase last row */
				physy = o_ttyrows - 1;
				physx = 0;
				tputs(tgoto(CM, physx, physy), 1, ttych);
				tputs(CE, 1, ttych);
			}
			eventresize((GUIWIN *)scan, scan->height, (int)o_ttycolumns);
		}
		else if (scan->newpos != scan->pos)
		{
			scan->pos = scan->newpos;
			drawborder(scan);
			eventexpose((GUIWIN *)scan, 0, 0, scan->height - 1, (int)(o_ttycolumns - 1));
		}
	}
}


/* This function creates a window */
static ELVBOOL creategw(name, firstcmd)
	char	*name;		/* name of new window's buffer */
	char	*firstcmd;	/* first command to run in window */
{
	TWIN	*newp;
#ifdef FEATURE_MISC
	BUFFER	buf;
#endif

	/* if we don't have room for any more windows, then fail */
	if (o_ttyrows / (nwindows + 1) < MINHEIGHT)
	{
		return ElvFalse;
	}

	/* create a window */
	newp = (TWIN *)safealloc(1, sizeof(TWIN));

	/* initialize the window */
	if (twins)
	{
		newp->height = 0;
		newp->pos = o_ttyrows;
	}
	else
	{
		newp->height = o_ttyrows;
		newp->pos = 0;
	}
	newp->cursx = newp->cursy = 0;
	newp->shape = CURSOR_NONE;

	/* insert the new window into the list of windows */
	newp->next = twins;
	twins = newp;
	nwindows++;

	/* adjust the heights of the other windows to make room for this one */
	chgsize(newp, (int)(o_ttyrows / nwindows), ElvFalse);
	drawborder(newp);

	/* make elvis do its own initialization */
	if (!eventcreate((GUIWIN *)newp, NULL, name, newp->height, (int)o_ttycolumns))
	{
		/* elvis can't make it -- fail */
		safefree(newp);
		return ElvFalse;
	}

#ifdef FEATURE_MISC
	/* some versions of tcaphelp.c support retitle() */
	if (gui->retitle)
	{
		buf = buffind(toCHAR(name));
		if (buf && o_filename(buf))
			name = tochar8(o_filename(buf));
		(*gui->retitle)((GUIWIN *)newp, name);
	}
#endif

	/* make the new window be the current window */
	current = newp;
	eventfocus((GUIWIN *)current, ElvTrue);

	/* execute the first command, if any */
	if (firstcmd)
	{
		winoptions(winofgw((GUIWIN *)newp));
		exstring(windefault, toCHAR(firstcmd), "+cmd");
	}

	return ElvTrue;
}


/* This function deletes a window */
static void destroygw(gw, force)
	GUIWIN	*gw;	/* window to be destroyed */
	ELVBOOL	force;	/* if ElvTrue, try harder */
{
	TWIN	*scan, *lag;
	WINDOW	win;

	/* delete the window from the list of windows */
	for (lag = NULL, scan = twins; scan != (TWIN *)gw; lag = scan, scan = scan->next)
	{
	}
	if (lag)
	{
		lag->next = scan->next;
	}
	else
	{
		twins = scan->next;
	}

	/* adjust the sizes of other windows (if any) */
	nwindows--;
	if (nwindows > 0)
	{
		chgsize((TWIN *)gw, 0, ElvFalse);
	}

	/* If this is the last window, move the cursor to the last line, and
	 * erase it.  If the buffer is going to be written, this is where the
	 * "wrote..." message will appear.
	 */
	if (nwindows == 0)
	{
		revert(NULL);
		tputs(tgoto(CM, 0, (int)(o_ttyrows - 1)), 1, ttych);
		tputs(CE, 1, ttych);
	}

	/* simulate a "destroy" event */
	eventdestroy(gw);

	/* free the storage */
	safefree(gw);

	/* if it was the current window, it isn't now */
	if (scan == current)
	{
		current = twins;
		if (current)
			eventfocus((GUIWIN *)current, ElvTrue);
	}

	/* some versions of tcaphelp.c support retitle() */
	if (current && gui->retitle && (win=winofgw((GUIWIN *)current)) != NULL)
	{
		(*gui->retitle)((GUIWIN *)current,
			tochar8(o_filename(markbuffer(win->cursor)) ? o_filename(markbuffer(win->cursor)) : o_bufname(markbuffer(win->cursor))));
	}
}
#endif /* FEATURE_SPLIT */


/* This function changes window focus */
static ELVBOOL focusgw(gw)
	GUIWIN	*gw;	/* window to be the new "current" window */
{
#ifdef FEATURE_MISC
	WINDOW	win;
	BUFFER	buf;
#endif

	/* make this window current */
	current = (TWIN *)gw;
	eventfocus((GUIWIN *)current, ElvTrue);

#ifdef FEATURE_MISC
	/* some versions of tcaphelp.c support retitle() */
	win = winofgw(gw);
	if (gui->retitle && win != NULL)
	{
		buf = markbuffer(win->cursor);
		(*gui->retitle)(gw, tochar8(o_bufname(buf) ? o_bufname(buf) : o_bufname(buf)));
	}
#endif

	return ElvTrue;
}


/* This function handles the visual <Tab> command */
static ELVBOOL tabcmd(gw, key2, count)
	GUIWIN	*gw;	/* window that the command should affect */
	_CHAR_	key2;	/* second key of <Tab> command */
	long	count;	/* argument of the <Tab> command */
{
	TWIN	*tw = (TWIN *)gw;
	int	newheight;
	int	oldheight;
	int	oldpos;

	/* if only one window, then we can't change its size */
	if (nwindows == 1)
		return ElvFalse;

	/* remember the old position */
	newheight = oldheight = tw->height;
	oldpos = tw->pos;

	switch (key2)
	{
	  case '=':
		if (count >= MINHEIGHT)
		{
			newheight = count;
			break;
		}
		/* else fall through... */

	  case '+':
		newheight += (count ? count : 1);
		break;

	  case '-':
		newheight -= (count ? count : 1);
		if (newheight < MINHEIGHT)
		{
			newheight = MINHEIGHT;
		}
		break;

	  case '\\':
		newheight = o_ttyrows; /* will be reduced later */
		break;

	  default:	return ElvFalse;
	}

	/* try to change the heights of other windows to make this one fit */
	chgsize(tw, newheight, ElvFalse);
	newheight = tw->height;

	/* resize/expose this window */
	if (newheight != oldheight)
	{
		drawborder(tw);
		eventresize(tw, tw->height, (int)o_ttycolumns);
	}
	else if (tw->pos != oldpos)
	{
		drawborder(tw);
		eventexpose(tw, 0, 0, newheight - 1, (int)(o_ttycolumns - 1));
	}
	return ElvTrue;
}

/* This function rings the bell */
static void beep(gw)
	GUIWIN	*gw;	/* window that generated the beep */
{
	if (o_flash && VB)
		tputs(VB, 0, ttych);
	else
		ttych('\007');
}

/* This function converts key labels to raw codes */
static int keylabel(given, givenlen, label, rawptr)
	CHAR	*given;		/* what the user typed in as the key name */
	int	givenlen;	/* length of the "given" string */
	CHAR	**label;	/* standard name for that key */
	CHAR	**rawptr;	/* control code sent by that key */
{
	int	i;

	/* compare the given text to each key's strings */
	for (i = 0; i < QTY(keys); i++)
	{
		/* ignore unsupported keys */
		if (!keys[i].rawin)
			continue;

		/* does given string match key label or raw characters? */
		if ((!strncmp(keys[i].label, tochar8(given), (size_t)givenlen) && !keys[i].label[givenlen])
		 || (!strncmp(keys[i].rawin, tochar8(given), (size_t)givenlen) && !keys[i].rawin[givenlen]))
		{

			/* Set the label and rawptr pointers, return rawlen */
			*label = toCHAR(keys[i].label);
			*rawptr = toCHAR(keys[i].rawin);
			return CHARlen(*rawptr);
		}
	}

	/* We reached the end of the keys[] array without finding a match,
	 * so this given string is not a key.
	 */
	return 0;
}

/* This function defines colors for fonts */
static ELVBOOL color(fontcode, colornam, isfg, colorptr, rgb)
	int	fontcode;	/* name of font being changed */
	CHAR	*colornam;	/* name of new color */
	ELVBOOL	isfg;		/* ElvTrue for foreground, ElvFalse for background */
	long	*colorptr;	/* where to store the color number */
	unsigned char rgb[3];	/* color broken down into RGB components */
{
	/* Normal colors must be set first, so we have a way to switch back
	 * from specialized colors.
	 */
	if (fontcode != 1 && !(isfg ? fgcolored : bgcolored))
	{
		msg(MSG_ERROR, "must set normal colors first");
		return ElvFalse;
	}

	/* Set the colors, if we know how for this terminal type.  Since there
	 * is no standard termcap entry for identifying the coloring style for
	 * a terminal, we just try to guess whether a terminal is ANSI-like by
	 * comparing the :up=: string to the standard ANSI `CUP' sequence, or
	 * the :AF=: string to a typical ANSI foreground sequence.  Note that
	 * :up=: is mandatory in termcap with tgoto(), but :AF=: is not.
	 * Another complicating factor is that :AF=: uses a parameter, and
	 * if we're really using terminfo's termcap emulation functions instead
	 * of the real termcap library, this means the :AF=: string will be
	 * different.  We need to try both termcap & terminfo versions of that
	 * string.
	 */
	if ((strcmp(UP,"\033[A") && (!AF || (strcmp(AF, "\033[3%dm") && strcmp(AF, "\033[3%p1%dm"))))
	 || !coloransi(fontcode, colornam, isfg, colorptr, rgb))
		return ElvFalse;

	/* Success!  Remember if we've set foreground or background */
	if (isfg)
		fgcolored = ElvTrue;
	else
		bgcolored = ElvTrue;
	return ElvTrue;
}


static ELVBOOL isread;

/* Suspend curses while running an external program */
static ELVBOOL ttyprgopen(command, willwrite, willread)
	char	*command;	/* the shell command to run */
	ELVBOOL	willwrite;	/* redirect stdin from elvis */
	ELVBOOL	willread;	/* redirect stdiout back to elvis */
{
	/* unless stdout/stderr is going to be redirected, move the cursor
	 * to the bottom of the screen before running program.
	 */
	isread = willread;
	if (!isread)
	{
		/* suspend curses */
		ttysuspend();
	}

	/* try to call the regular prgopen(); if it fails, then clean up */
	if (!prgopen(command, willwrite, willread))
	{
		if (!isread)
			ttyresume(ElvTrue);
		return ElvFalse;
	}

	return ElvTrue;
}


/* After running a program, resume curses and redraw all screens */
static int ttyprgclose()
{
	int	status;
	WINDOW	win;
#ifdef FEATURE_MISC
	CHAR	*title;
#endif

	/* wait for the program to terminate */
	status = prgclose();

	/* resume curses */
	if (!isread)
	{
		ttyresume(ElvFalse);

		/* Okay, now we're in a weird sort of situation.  The screen is
		 * about to be forced to display "Hit <Enter> to continue" on
		 * the bottom of the window in open mode, and then wait for a
		 * keystroke.  That's a Good Thing.  But there are two problems
		 * we need to address:
		 *    * We want that prompt to appear at the bottom of the
		 *	screen, not the bottom of the window.
		 *    * After the user hits a key, we want to redraw all
		 *	windows.
		 *
		 * We'll set a flag indicating this situation.  The movecurs()
		 * function will test for that flag, and merely pretend to move
		 * the cursor when it is set.  The loop() function will test
		 * that flag after each keystroke, and expose all windows if
		 * it is set.
		 */
		afterprg = 2;
		afterscrl = 0;
	}

#ifdef FEATURE_MISC
	/* some versions of tcaphelp.c set the title */
	if (gui->retitle && (win = winofgw((GUIWIN *)current)) != NULL)
	{
		title = o_filename(markbuffer(win->cursor)); 
		if (!title)
			title = o_bufname(markbuffer(win->cursor)); 
		(*gui->retitle)((GUIWIN *)current, tochar8(title));
	}
#endif

	return status;
}


#ifdef SIGSTOP
/* This function starts an interactive shell.  It is called with the argument
 * (ElvTrue) for the :sh command, or (ElvFalse) for a :stop or :suspend command.
 * If successful it returns RESULT_COMPLETE after the shell exits; if
 * unsuccessful it issues an error message and returns RESULT_ERROR.  It
 * could also return RESULT_MORE to defer processing to the portable code
 * in ex_suspend().
 */
static RESULT stop(alwaysfork)
	ELVBOOL	alwaysfork;	/* fork even if SIGSTOP would work? */
{
	RESULT	result;

	/* move the cursor to the bottom of the screen, and scroll up */
	tputs(tgoto(CM, 0, (int)o_ttyrows - 1), 0, ttych);
	ttych('\n');
	reset();
	ttyflush();

	/* if we want to fork, then the default processing is good enough */
	if (alwaysfork)
		return RESULT_MORE;

	/* call the OS-dependent function for stopping the process */
	result = ttystop();
	if (result == RESULT_MORE)
		return RESULT_MORE;

	/* arrange for all windows to be refreshed */
	afterprg = 1;
	return result;
}
#endif


/* This function converts screen coordinates into a window, and coordinates
 * within that window.
 */
GUIWIN *ttywindow(ttyrow, ttycol, winrow, wincol)
	int	ttyrow, ttycol;		/* screen coordinates in */
	int	*winrow, *wincol;	/* window coordinates out */
{
#ifndef FEATURE_SPLIT
	*winrow = ttyrow;
	*wincol = ttycol;
	return twins;
#else
	TWIN	*tw;

	if (ttycol < 0 || ttycol >= o_ttycolumns)
		return NULL;
	for (tw = twins;
	     tw && (tw->pos > ttyrow || ttyrow >= tw->pos + tw->height);
	     tw = tw->next)
	{
	}
	if (tw)
	{
		*winrow = ttyrow - tw->pos;
		*wincol = ttycol;
	}
	return (GUIWIN *)tw;
#endif /* FEATURE_SPLIT */
}


/* structs of this type are used to describe each available GUI */
GUI guitermcap =
{
	"termcap",	/* name */
	"Termcap/Terminfo interface with windows & color",
	ElvFalse,		/* exonly */
	ElvFalse,		/* newblank */
	ElvFalse,		/* minimizeclr */
	ElvTrue,		/* scrolllast */
	ElvFalse,		/* shiftrows */
	3,		/* movecost */
	0,		/* opts */
	NULL,		/* optdescs */
	test,
	init,
	NULL,		/* usage */
	loop,
	ttypoll,
	term,
	creategw,
	destroygw,
	focusgw,
	NULL,		/* retitle */
	reset,
	flush,
	moveto,
	draw,
	shift,
	scroll,
	clrtoeol,
	NULL,		/* newline */
	beep,		/* beep */
	NULL,		/* msg */
	NULL,		/* scrollbar */
	NULL,		/* status */
	keylabel,
	NULL,		/* clipopen */
	NULL,		/* clipwrite */
	NULL,		/* clipread */
	NULL,		/* clipclose */
	color,		/* color */
	NULL,		/* colorfree */
	NULL,		/* setbg */
	NULL,		/* guicmd */
	tabcmd,
	NULL,		/* save */
	NULL,		/* wildcard */
	ttyprgopen,
	ttyprgclose,
#ifdef SIGSTOP
	stop
#else
	NULL		/* stop */
#endif
};
#endif
