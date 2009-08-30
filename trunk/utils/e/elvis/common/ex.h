/* ex.h */
/* Copyright 1995 by Steve Kirkendall */

/* This is a list of all possible print flag combinations */
typedef enum
{
	PF_NONE,	/* don't print */
	PF_PRINT,	/* print the line */
	PF_NUMBER,	/* print line number, then print line */
	PF_LIST,	/* list the line (making control characters visible) */
	PF_NUMLIST	/* print line number, then list line */
} PFLAG;

typedef enum
{
	ELVFNR_TILDE    = 1,	/* perform tilde expansion */
	ELVFNR_DOLLAR   = 2,	/* expand $ENVVAR in filenames */
	ELVFNR_PAREN    = 4,	/* expand (expr) in filenames */
	ELVFNR_WILDCARD = 8,	/* perform wildcard expansion in filenames */
	ELVFNR_SPECIAL  = 16,	/* replace % and # with current/alternate name*/
	ELVFNR_SPACE    = 32	/* interpret <space> as filename delimiter */
} ELVFNR;
 
typedef enum
{
	EX_ABBR, EX_ALIAS, EX_ALL, EX_APPEND, EX_ARGS, EX_AT, EX_AUTOCMD,
		EX_AUEVENT, EX_AUGROUP,
	EX_BANG, EX_BBROWSE, EX_BREAK, EX_BROWSE, EX_BUFFER,
	EX_CALC, EX_CASE, EX_CC, EX_CD, EX_CHANGE, EX_CHECK, EX_CHREGION,
		EX_CLOSE, EX_COLOR, EX_COMMENT, EX_COPY,
	EX_DEFAULT, EX_DELETE, EX_DIGRAPH, EX_DISPLAY, EX_DO,
		EX_DOALIAS, EX_DOAUTOCMD, EX_DOPROTO,
	EX_ECHO, EX_EDIT, EX_ELSE, EX_EQUAL, EX_ERRLIST, EX_ERROR, EX_EVAL,
	EX_FILE, EX_FOLD, EX_FOR,
	EX_GOTO, EX_GLOBAL, EX_GUI,
	EX_HELP,
	EX_IF, EX_INSERT,
	EX_JOIN,
	EX_LAST, EX_LET, EX_LIST, EX_LOCAL, EX_LPR,
	EX_MAKE, EX_MAP, EX_MARK, EX_MESSAGE, EX_MKEXRC, EX_MOVE,
	EX_NEXT, EX_NOHLSEARCH, EX_NORMAL, EX_NUMBER,
	EX_ONLY, EX_OPEN,
	EX_POP, EX_PRESERVE, EX_PREVIOUS, EX_PRINT, EX_PUSH, EX_PUT,
	EX_QALL, EX_QUIT,
	EX_READ, EX_REDO, EX_REGION, EX_REWIND,
	EX_SALL, EX_SAFELY, EX_SBBROWSE, EX_SBROWSE, EX_SET, EX_SHELL,
		EX_SHIFTL, EX_SHIFTR, EX_SLAST, EX_SNEW, EX_SNEXT, EX_SOURCE,
		EX_SPELL, EX_SPLIT, EX_SPREVIOUS, EX_SREWIND, EX_STAG,
		EX_STACK, EX_STOP, EX_SUBAGAIN, EX_SUBRECENT, EX_SUBSTITUTE,
		EX_SUSPEND, EX_SWITCH,
	EX_TAG, EX_THEN, EX_TRY,
	EX_UNABBR, EX_UNALIAS, EX_UNBREAK, EX_UNDO, EX_UNFOLD, EX_UNMAP,
		EX_UNREGION,
	EX_VERSION, EX_VGLOBAL, EX_VISUAL,
	EX_WARNING, EX_WHILE, EX_WINDOW, EX_WNEXT, EX_WORDFILE, EX_WORDS,
		EX_WQUIT, EX_WRITE,
	EX_XIT,
	EX_YANK,
	EX_Z
} EXCMD;

/* This structure is used to store a parsed ex command */
typedef struct
{
	WINDOW	window;		/* window where line was entered */
	MARKBUF	defaddr;	/* default address (includes buffer spec) */
	long	from, to;	/* range line numbers */
	long	fromoffset;	/* exact offset of an addressed point */
	MARK	fromaddr;	/* start of the "from" line */
	MARK	toaddr;		/* end of "to" line (start of following line) */
	ELVBOOL	anyaddr;	/* ElvTrue if any addresses given, ElvFalse if none */
	char	*cmdname;	/* name of command (for diagnostics) */
	EXCMD	command;	/* code for command name, e.g. EX_PRINT */
	int	cmdidx;		/* index into internal array of cmd attributes */
	int	multi;		/* number of times cmd name was stuttered */
	ELVBOOL	bang;		/* ElvTrue if '!' appended to command name */
	MARK	destaddr;	/* end of destination line */
	regexp	*re;		/* regular expression */
	CHAR	*lhs;		/* single-word argument or "+lineno" string */
	CHAR	*rhs;		/* multi-word argument, or command line */
	char	**file;		/* array of file names */
	int	nfiles;		/* size of "file" array */
	CHAR	cutbuf;		/* cut buffer name */
	long	count;		/* count argument, or plus value */
	PFLAG	pflag;		/* print flag, causes output of some lines */
	long	delta;		/* print offset */
	ELVBOOL	global;		/* executed as part of :global command? */
	ELVBOOL	undo;		/* save an "undo" version before first change? */
	MARK	newcurs;	/* where cursor should be left (NULL to not move) */
#ifdef FEATURE_V
	MARKBUF	putbacktop;	/* for rectangles, where to put changed text */
	MARKBUF	putbackbottom;	/* for rectangles, end to put changed text */
	long	putbackleft;	/* left edge of putback rectangle */
	long	putbackright;	/* right edge of putback rectangle */
	long	putbackchanges;	/* used to detect changes in original buffer */
	long	cbchanges;	/* used to detect changes in rect cut buffer */
#endif
} EXINFO;


/* This stores the current state of ex's control structures */
typedef struct
{
	ELVBOOL	thenflag;	/* result of an :if */
	ELVBOOL	switchcarry;	/* falling through to next :case? */
	CHAR	*switchvalue;	/* result of :switch, compare to :case value */
	CHAR	*dotest;	/* :while expression, or :for variable */
	CHAR	*list;		/* space-delimited list of :for values */
	char	**file;		/* array of file names */
	int	nfiles;		/* number of files in the array */
} EXCTLSTATE;


/* defined in exconfig.c */
extern EXCTLSTATE exctlstate;

/* macros for saving & restoring the control state in a local variable */
#define exctlsave(v)	{(v) = exctlstate; memset(&exctlstate, 0, sizeof exctlstate);}
#define exctlrestore(v)	{if (exctlstate.switchvalue) safefree(exctlstate.switchvalue);\
			if (exctlstate.dotest) safefree(exctlstate.dotest);\
			exctlstate = (v);}

/* defined in exmake.c */
extern ELVBOOL	makeflag;

BEGIN_EXTERNC
extern ELVBOOL	exparseaddress P_((CHAR **refp, EXINFO *xinf));
extern RESULT	experform P_((WINDOW win, MARK from, MARK to));
extern RESULT	exstring P_((WINDOW win, CHAR *str, char *name));
extern CHAR	*exname P_((CHAR *name));
extern RESULT	exenter P_((WINDOW win));
extern long	exprintlines P_((WINDOW win, MARK line, long qty, PFLAG pflag));
extern CHAR	*excomplete P_((WINDOW win, MARK from, MARK to));
extern void	exfree P_((EXINFO *xinf));
extern ELVFNR	exfilenamerules P_((CHAR *rulestr));
extern ELVBOOL	exaddfilearg P_((char ***file, int *nfiles, char *filename, ELVFNR rules));

extern char	*exaliasname P_((int i));
extern char	*exisalias P_((char *name, ELVBOOL inuse));
extern void	exaliassave P_((BUFFER custom));

extern RESULT	ex_alias P_((EXINFO *xinf));
extern RESULT	ex_doalias P_((EXINFO *xinf));
extern RESULT	ex_all P_((EXINFO *xinf));
extern RESULT	ex_append P_((EXINFO *xinf));
extern RESULT	ex_args P_((EXINFO *xinf));
extern RESULT	ex_at P_((EXINFO *xinf));
extern RESULT	ex_bang P_((EXINFO *xinf));
extern RESULT	ex_browse P_((EXINFO *xinf));
extern RESULT	ex_buffer P_((EXINFO *xinf));
extern RESULT	ex_case P_((EXINFO *xinf));
extern RESULT	ex_cd P_((EXINFO *xinf));
extern RESULT	ex_check P_((EXINFO *xinf));
extern RESULT	ex_color P_((EXINFO *xinf));
extern RESULT	ex_comment P_((EXINFO *xinf));
extern RESULT	ex_delete P_((EXINFO *xinf));
extern RESULT	ex_digraph P_((EXINFO *xinf));
extern RESULT	ex_display P_((EXINFO *xinf));
extern RESULT	ex_do P_((EXINFO *xinf));
extern RESULT	ex_edit P_((EXINFO *xinf));
extern RESULT	ex_errlist P_((EXINFO *xinf));
extern RESULT	ex_file P_((EXINFO *xinf));
extern RESULT	ex_fold P_((EXINFO *xinf));
extern RESULT	ex_global P_((EXINFO *xinf));
extern RESULT	ex_gui P_((EXINFO *xinf));
extern RESULT	ex_help P_((EXINFO *xinf));
extern RESULT	ex_if P_((EXINFO *xinf));
extern RESULT	ex_join P_((EXINFO *xinf));
extern RESULT	ex_lpr P_((EXINFO *xinf));
extern RESULT	ex_make P_((EXINFO *xinf));
extern RESULT	ex_map P_((EXINFO *xinf));
extern RESULT	ex_mark P_((EXINFO *xinf));
extern RESULT	ex_message P_((EXINFO *xinf));
extern RESULT	ex_mkexrc P_((EXINFO *xinf));
extern RESULT	ex_move P_((EXINFO *xinf));
extern RESULT	ex_next P_((EXINFO *xinf));
extern RESULT	ex_nohlsearch P_((EXINFO *xinf));
extern RESULT	ex_pop P_((EXINFO *xinf));
extern RESULT	ex_print P_((EXINFO *xinf));
extern RESULT	ex_put P_((EXINFO *xinf));
extern RESULT	ex_qall P_((EXINFO *xinf));
extern RESULT	ex_read P_((EXINFO *xinf));
extern RESULT	ex_region P_((EXINFO *xinf));
extern RESULT	ex_sall P_((EXINFO *xinf));
extern RESULT	ex_set P_((EXINFO *xinf));
extern RESULT	ex_shift P_((EXINFO *xinf));
extern RESULT	ex_source P_((EXINFO *xinf));
extern RESULT	ex_stack P_((EXINFO *xinf));
extern RESULT	ex_substitute P_((EXINFO *xinf));
extern RESULT	ex_suspend P_((EXINFO *xinf));
extern RESULT	ex_switch P_((EXINFO *xinf));
extern RESULT	ex_tag P_((EXINFO *xinf));
extern RESULT	ex_then P_((EXINFO *xinf));
extern RESULT	ex_undo P_((EXINFO *xinf));
extern RESULT	ex_version P_((EXINFO *xinf));
extern RESULT	ex_wordfile P_((EXINFO *xinf));
extern RESULT	ex_split P_((EXINFO *xinf));
extern RESULT	ex_while P_((EXINFO *xinf));
extern RESULT	ex_window P_((EXINFO *xinf));
extern RESULT	ex_write P_((EXINFO *xinf));
extern RESULT	ex_xit P_((EXINFO *xinf));
extern RESULT	ex_z P_((EXINFO *xinf));

extern void	colorsave P_((BUFFER custom));
END_EXTERNC
