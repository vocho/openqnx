/* vi.h */
/* Copyright 1995 by Steve Kirkendall */



/* This is a list of all possible parsing phases */
typedef enum {VI_CUTBUF, VI_START, VI_AFTERG, VI_COUNT2, VI_KEY2, VI_QUOTE, VI_HEX1, VI_HEX2, VI_COMPLETE} PHASE;

/* These are the "tweak" flags */
#define TWEAK_DOT	0x0001	/* remember command so <.> can repeat it */
#define TWEAK_DOTTING	0x0002	/* we're doing this command as a <.> */
#define TWEAK_FIXCOL	0x0004	/* adjust mark to the "wantcol" column */
#define TWEAK_FRONT	0x0008	/* leave cursor at front of line */
#define TWEAK_IGNCOL	0x0010	/* don't set the "wantcol" for the window */
#define TWEAK_INCL	0x0020	/* operated region includes last char/line */
#define TWEAK_LINE	0x0040	/* implies operators act in "line" mode */
#define TWEAK_MARK	0x0080	/* remember starting point so `` can go back */
#define TWEAK_OPER	0x0100	/* operator - ends a visible mark */
#define TWEAK_TWONUM	0x0200	/* command accepts 2 separate counts */
#define TWEAK_UNDO	0x0400	/* make an "undo" version if this changes buffer */

#define TWEAK_NONE			0x0000	/* none of the above */
#define TWEAK_DOT_OPER_UNDO		(TWEAK_DOT|TWEAK_OPER|TWEAK_UNDO)
#define TWEAK_DOT_UNDO			(TWEAK_DOT|TWEAK_UNDO)
#define TWEAK_FIXCOL_INCL		(TWEAK_FIXCOL|TWEAK_INCL)
#define TWEAK_FIXCOL_INCL_LINE		(TWEAK_FIXCOL|TWEAK_INCL|TWEAK_LINE)
#define TWEAK_FIXCOL_INCL_LINE_DOT_UNDO	(TWEAK_FIXCOL_INCL_LINE|TWEAK_DOT|TWEAK_UNDO)
#define TWEAK_FRONT_INCL_LINE		(TWEAK_FRONT|TWEAK_INCL|TWEAK_LINE)
#define TWEAK_FRONT_INCL_MARK_LINE	(TWEAK_FRONT|TWEAK_INCL|TWEAK_MARK|TWEAK_LINE)
#define TWEAK_FRONT_LINE_DOT		(TWEAK_FRONT|TWEAK_LINE|TWEAK_DOT)
#define TWEAK_FRONT_LINE_DOT_OPER_UNDO	(TWEAK_FRONT_LINE_DOT|TWEAK_OPER|TWEAK_UNDO)
#define TWEAK_FRONT_UNDO		(TWEAK_FRONT|TWEAK_UNDO)
#define TWEAK_IGNCOL_INCL		(TWEAK_IGNCOL|TWEAK_INCL)
#define TWEAK_IGNCOL_INCL_LINE		(TWEAK_IGNCOL|TWEAK_INCL|TWEAK_LINE)
#define TWEAK_IGNCOL_MARK		(TWEAK_IGNCOL|TWEAK_MARK)
#define TWEAK_LINE_MARK			(TWEAK_LINE|TWEAK_MARK)
#define TWEAK_LINE_UNDO			(TWEAK_LINE|TWEAK_UNDO)


/* This data type is used to store a single parsed command */
typedef struct
{
	long	count;		/* numeric argument */
	long	count2;		/* secondary numeric argument (rarely used) */
	CHAR	cutbuf;		/* name of cut buffer, or '\0' for anonymous */
	CHAR	oper;		/* an operator command, or '\0' */
	CHAR	command;	/* a command keystroke */
	CHAR	key2;		/* argument keystroke, if appropriate */
	PHASE	phase;		/* parsing phase */
	ELVBOOL	control_o;	/* previous char was ^O */
	unsigned short tweak;	/* tweak flags */
} VIINFO;

/* This macro is used to set the default count value */
#define DEFAULT(x)	if (vinf->count == 0) vinf->count = (x)

/* This macro converts a non-'g' ascii command into a 'g' command. */
#define ELVG(x)		((x) | 0x80)
#define ELVUNG(x)	((x) & ~0x80)

/* This macro returns ElvTrue if the window's current state is "multiple vi
 * command mode."  Note that the _viperform() function is declared globally
 * solely so that it can be referenced by this macro.
 */
#define viiscmd(win)	(ELVBOOL)((win)->state->perform == _viperform \
			    && ((win)->state->flags & ELVIS_ONCE) == 0)
BEGIN_EXTERNC
extern RESULT	_viperform P_((WINDOW win));



extern void	vipush P_((WINDOW win, ELVISSTATE flags, MARK cursor));
extern void	viinitcmd P_((VIINFO *info));
extern RESULT	viperform P_((WINDOW win, VIINFO *vinf));
extern CHAR	*viname P_((CHAR *name));
#ifdef FEATURE_NORMAL
extern RESULT	vinormal P_((WINDOW win, int nkeys, CHAR *keys));
#endif
#ifdef FEATURE_TEXTOBJ
extern RESULT vitextobj(WINDOW win, VIINFO *vinf, MARKBUF *from, MARKBUF *to);
#endif
END_EXTERNC
