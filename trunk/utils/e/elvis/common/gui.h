/* gui.h */
/* Copyright 1995 by Steve Kirkendall */


/* generic pointer to GUI-specific window information */
typedef void GUIWIN;

/* structs of this type are used to describe each available GUI */
typedef struct gui_s
{
	char	*name;
	char	*desc;
	ELVBOOL	exonly;
	ELVBOOL	newblank;
	ELVBOOL	minimizeclr;
	ELVBOOL	scrolllast;
	ELVBOOL	shiftrows;
	int	movecost;
	int	nopts;
	OPTDESC	*optdescs;
	int	(*test) P_((void));
	int	(*init) P_((int argc, char **argv));
	void	(*usage) P_((void));
	void	(*loop) P_((void));
	ELVBOOL	(*poll) P_((ELVBOOL reset));
	void	(*term) P_((void));
	ELVBOOL	(*creategw) P_((char *name, char *attributes));
	void	(*destroygw) P_((GUIWIN *gw, ELVBOOL force));
	ELVBOOL	(*focusgw) P_((GUIWIN *gw));
	void	(*retitle) P_((GUIWIN *gw, char *name));
	void	(*reset) P_((void));
	void	(*flush) P_((void));
	void	(*moveto) P_((GUIWIN *gw, int column, int row));
	void	(*draw) P_((GUIWIN *gw, long fg, long bg, int bits, CHAR *text, int len));
	ELVBOOL	(*shift) P_((GUIWIN *gw, int qty, int rows));
	ELVBOOL	(*scroll) P_((GUIWIN *gw, int qty, ELVBOOL notlast));
	ELVBOOL	(*clrtoeol) P_((GUIWIN *gw));
	void	(*textline) P_((GUIWIN *gw, CHAR *text, int len));
	void	(*beep) P_((GUIWIN *gw));
	ELVBOOL	(*msg) P_((GUIWIN *gw, MSGIMP imp, CHAR *text, int len));
	void	(*scrollbar) P_((GUIWIN *gw, long top, long bottom, long nlines));
	ELVBOOL	(*status) P_((GUIWIN *gw, CHAR *cmd, long line, long column, _CHAR_ learn, char *mode));
	int	(*keylabel) P_((CHAR *given, int givenlen, CHAR **label, CHAR **raw));
	ELVBOOL	(*clipopen) P_((ELVBOOL forwrite));
	int	(*clipwrite) P_((CHAR *text, int len));
	int	(*clipread) P_((CHAR *text, int len));
	void	(*clipclose) P_((void));
	ELVBOOL	(*color) P_((int fontcode, CHAR *colornam, ELVBOOL isfg, long *colorptr, unsigned char rgb[3]));
	void	(*freecolor) P_((long color, ELVBOOL isfg));
	void	(*setbg) P_((GUIWIN *gw, long bg));
	ELVBOOL	(*guicmd) P_((GUIWIN *gw, char *extra));
	ELVBOOL	(*tabcmd) P_((GUIWIN *gw, _CHAR_ key2, long count));
	void	(*save) P_((BUFFER buf, GUIWIN *gw));
	int	(*wildcard) P_((char *names, char *buf, int bufsize, ELVBOOL single));
	ELVBOOL	(*prgopen) P_((char *command, ELVBOOL willwrite, ELVBOOL willread));
	int	(*prgclose) P_((void));
	RESULT	(*stop) P_((ELVBOOL alwaysfork));
} GUI;



#if defined(GUI_TERMCAP) || defined(GUI_OPEN) || defined(GUI_VIO)
/* The "termcap" and "open" user interfaces use the following OS-dependent
 * functions.  These functions must be defined in "osXXXX/tcaphelp.c" if
 * you're going to use "termcap" or "open".
 */
BEGIN_EXTERNC
extern void	ttyinit P_((void));
extern void	ttyraw P_((char *erasekey));
extern void	ttynormal P_((void));
extern void	ttysuspend P_((void));
extern void	ttyresume P_((ELVBOOL sendstr));
extern int	ttyread P_((char *buf, int len, int timeout));
extern void	ttywrite P_((char *buf, int len));
extern char	*ttytermtype P_((void));
extern ELVBOOL	ttysize P_((int *linesptr, int *colsptr));
extern ELVBOOL	ttypoll P_((ELVBOOL reset));
extern RESULT	ttystop P_((void));
extern GUIWIN	*ttywindow P_((int ttyrow, int ttycol, int *winrow, int *wincol));
END_EXTERNC

/* These can be used for accessing the values of the tty options. */
extern struct ttygoptvals_s
{
	OPTVAL	term;		/* string - terminal type */
	OPTVAL	ttyrows;	/* number - rows of screen */
	OPTVAL	ttycolumns;	/* number - columns of screen */
	OPTVAL	ttyunderline;	/* boolean - whether colors and underline mix */
	OPTVAL	ttyitalic;	/* boolean - whether to allow italics */
	OPTVAL	ttywrap;	/* boolean - trust EOL wrapping? */
} ttygoptvals;
#define o_term		ttygoptvals.term.value.string
#define o_ttyrows	ttygoptvals.ttyrows.value.number
#define o_ttycolumns	ttygoptvals.ttycolumns.value.number
#define o_ttyunderline	ttygoptvals.ttyunderline.value.boolean
#define o_ttyitalic	ttygoptvals.ttyitalic.value.boolean
#define o_ttywrap	ttygoptvals.ttywrap.value.boolean

# ifdef NEED_SPEED_T
#  include <termcap.h>
# else
extern char	PC;
extern short	ospeed;		/* might be "speed_t" instead of "short" */
BEGIN_EXTERNC
extern int	tgetent P_((char *, char *));
extern int	tgetflag P_((char *));
extern int	tgetnum P_((char *));
extern char	*tgoto P_((char *, int, int));
extern char	*tgetstr P_((char*, char**));
extern void	tputs P_((char *, int, int (*)(int)));
END_EXTERNC
# endif /* !_POSIX_SOURCE */

#endif /* defined(GUI_TERMCAP) */
