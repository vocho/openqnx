/* ex.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_ex[] = "$Id: ex.c 167211 2008-04-24 17:26:56Z sboucher $";
#endif

#if USE_PROTOTYPES
static void skipwhitespace(CHAR **refp);
static ELVBOOL parsewindowid(CHAR **refp, EXINFO *xinf);
static ELVBOOL parsebuffername(CHAR **refp, EXINFO *xinf);
static ELVBOOL parseregexp(CHAR **refp, EXINFO *xinf, long flags);
static ELVBOOL parsecommandname(CHAR **refp, EXINFO *xinf);
static void parsemulti(CHAR **refp, EXINFO *xinf);
static void parsebang(CHAR **refp, EXINFO *xinf, long flags);
static void parseplus(CHAR **refp, EXINFO *xinf, long flags);
static void parseprintflag(CHAR **refp, EXINFO *xinf, long flags, long quirks);
static void parselhs(CHAR **refp, EXINFO *xinf, long flags, long quirks);
static void parserhs(CHAR **refp, EXINFO *xinf, long flags, long quirks);
static void parsecutbuffer(CHAR **refp, EXINFO *xinf, long flags);
static void parsecount(CHAR **refp, EXINFO *xinf, long flags);
static RESULT parsenested(CHAR	**refp, EXINFO	*xinf);
static RESULT parsecmds(CHAR **refp, EXINFO *xinf, long flags);
static ELVBOOL parsefileargs(CHAR **refp, EXINFO *xinf, long flags);
static RESULT parse(WINDOW win, CHAR **refp, EXINFO *xinf);
static RESULT execute(EXINFO *xinf);
# ifdef FEATURE_ALIAS
static char *cname(int i);
# endif
#endif

/* Minimum number of filename pointers to allocate at a time. */
#define FILEGRAN	8



/* These are the possible arguments for a command.  These may be combined
 * via the bitwise OR operator to describe arbitrary combinations of these.
 */
#define a_Line	  0x00000001L	/* single line specifier */
#define a_Range	  0x00000002L	/* range of lines */
#define a_Multi	  0x00000004L	/* command character may be stuttered */
#define a_Bang	  0x00000008L	/* '!' after the command name */
#define a_Target  0x00000010L	/* extra line specifier after the command name */
#define a_Lhs	  0x00000020L	/* single word after the command */
#define a_Rhs	  0x00000040L	/* extra words (after Lhs, if Lhs allowed) */
#define a_Buffer  0x00000080L	/* cut buffer name */
#define a_Count	  0x00000100L	/* number of lines affected */
#define a_Pflag	  0x00000200L	/* print flag (p, l, or #) */
#define a_Plus	  0x00000400L	/* line offset (as for ":e +20 foo") */
#define a_Append  0x00000800L	/* ">>" indicating append mode */
#define a_Filter  0x00001000L	/* "!cmd", where cmd may include '|' characters */
#define a_File	  0x00002000L	/* a single file name */
#define a_Files	  0x00004000L	/* Zero or more filenames */
#define a_Cmds	  0x00008000L	/* text which may contain '|' characters */
#define a_RegExp  0x00010000L	/* regular expression */
#define a_RegSub  0x00020000L	/* substitution text (requires RegExp) */
#define a_Text	  0x00040000L	/* If no Rhs, following lines are part of cmd */

/* The following values indicate the default values of arguments */
#define d_All	  0x00080000L	/* default range is all lines */
#define d_2Lines  0x00100000L	/* default range is two lines (for :join) */
#define d_None	  0x00180000L	/* default range is no lines (else current line) */
#define d_LnMask  0x00180000L	/* mask value for extracting line defaults */
#define d_File	  0x00200000L	/* default file is current file (else no default) */

/* The following indicate other quirks of commands */
#define q_Unsafe      0x0001L	/* command can't be executed if "safer" */
#define q_FileUnsafe  0x0002L	/* command can't take filename if "safer" */
#define q_Restricted  0x0004L	/* "Unsafe" flags only apply if "restricted" */
#define q_Autop	      0x0008L	/* autoprint current line */
#define q_Zero	      0x0010L	/* allow Target or Line to be 0 */
#define q_Exrc	      0x0020L	/* command may be run in a .exrc file */
#define q_Undo	      0x0040L	/* save an "undo" version before this command */
#define q_Custom      0x0080L	/* save options/maps after command */
#define q_Ex	      0x0100L	/* only allowed in ex mode, not vi's <:> cmd */
#define q_CtrlV	      0x0200L	/* use ^V for quote char, instead of \ */
#define q_MayQuit     0x0400L	/* may cause window to disappear */
#define q_SwitchB     0x0800L	/* may move cursor to a different buffer */


/* This array maps ex command names to command codes. The order in which
 * command names are listed below is significant --  ambiguous abbreviations
 * are always resolved to be the first possible match.  (e.g. "r" is taken
 * to mean "read", not "rewind", because "read" comes before "rewind")
 *
 * Also, commands which share the same first letter are grouped together.
 * Similarly, within each one-letter group, the commands are ordered so that
 * the commands with the same two letters are grouped together, and those
 * groups are then divided into 3-letter groups, and so on.  This allows
 * the command list to be searched faster.
 * 
 * The comment at the start of each line below gives the shortest abbreviation.
 * HOWEVER, YOU CAN'T SIMPLY SORT THOSE ABBREVIATIONS to produce the correct
 * order for the commands.  Consider the change/chdir/calculate commands, for
 * an example of why that wouldn't work.
 */
static struct
{
	char	*name;	/* name of the command */
	EXCMD	code;	/* enum code of the command */
	RESULT	(*fn) P_((EXINFO *));
			/* function which executes the command */
	long	flags;	/* command line arguments, defaults */
	long	quirks;	/* command line quirks */
}
	cmdnames[] =
{   /*	 cmd name	cmd code	function	flags (describing arguments)            quirks */
/*!   */{"!",		EX_BANG,	ex_bang,	a_Range | a_Cmds | d_None,		q_Exrc | q_Unsafe | q_Undo	},
/*#   */{"#",		EX_NUMBER,	ex_print,	a_Range | a_Count | a_Pflag						},
/*&   */{"&",		EX_SUBAGAIN,	ex_substitute,	a_Range | a_Rhs,			q_Undo				},
/*<   */{"<",		EX_SHIFTL,	ex_shift,	a_Range | a_Multi | a_Bang | a_Count,	q_Autop | q_Undo		},
/*=   */{"=",		EX_EQUAL,	ex_file,	a_Range									},
/*>   */{">",		EX_SHIFTR,	ex_shift,	a_Range | a_Multi | a_Bang | a_Count,	q_Autop | q_Undo		},
#ifdef FEATURE_MISC
/*@   */{"@",		EX_AT,		ex_at,		a_Buffer								},
#endif
/*N   */{"Next",	EX_PREVIOUS,	ex_next,	a_Bang,					q_Unsafe | q_SwitchB		},
/*"   */{"\"",		EX_COMMENT,	ex_comment,	a_Cmds,					q_Exrc				},
/*a   */{"append",	EX_APPEND,	ex_append,	a_Line | a_Bang | a_Cmds | a_Text,	q_Zero | q_Undo | q_Ex		},
/*ab  */{"abbreviate",	EX_ABBR,	ex_map,		a_Bang | a_Lhs | a_Rhs,			q_Exrc|q_Custom|q_Unsafe|q_CtrlV},
#ifdef FEATURE_MISC
/*al  */{"all",		EX_ALL,		ex_all,		a_Bang | a_Cmds,			q_Exrc				},
#endif
#ifdef FEATURE_ALIAS
/*ali */{"alias",	EX_ALIAS,	ex_alias,	a_Bang | a_Lhs | a_Cmds,		q_Exrc | q_Unsafe | q_Custom	},
#endif
/*ar  */{"args",	EX_ARGS,	ex_args,	a_Files,				q_Exrc | q_Unsafe		},
#ifdef FEATURE_AUTOCMD
/*au  */{"autocmd",	EX_AUTOCMD,	ex_autocmd,	a_Bang | a_Cmds,			q_Exrc | q_Unsafe | q_Custom	},
/*aue */{"auevent",	EX_AUEVENT,	ex_auevent,	a_Bang | a_Rhs,				q_Exrc | q_Custom		},
/*aug */{"augroup",	EX_AUGROUP,	ex_augroup,	a_Bang | a_Lhs,				q_Exrc | q_Unsafe		},
#endif
#ifdef FEATURE_MISC
/*b   */{"buffer",	EX_BUFFER,	ex_buffer,	a_Bang | a_Rhs,				q_SwitchB			},
/*bb  */{"bbrowse",	EX_BBROWSE,	ex_buffer,	a_Bang,					q_SwitchB			},
#endif
#ifdef FEATURE_BROWSE
/*br  */{"browse",	EX_BROWSE,	ex_browse,	a_Bang|a_Rhs,				q_SwitchB			},
#endif
#ifdef FEATURE_MAPDB
/*bre */{"break",	EX_BREAK,	ex_map,		a_Bang | a_Rhs,				q_CtrlV				},
#endif
/*c   */{"change",	EX_CHANGE,	ex_append,	a_Range | a_Bang | a_Count | a_Text,	q_Undo | q_Ex			},
/*cha */{"chdir",	EX_CD,		ex_cd,		a_Bang | a_File,			q_Exrc | q_Unsafe		},
#ifdef FEATURE_SPELL
/*che */{"check",	EX_CHECK,	ex_check,	a_Bang | a_Rhs,				q_Exrc | q_Custom		},
#endif
#ifdef FEATURE_REGION
/*chr */{"chregion",	EX_CHREGION,	ex_region,	a_Range | a_Rhs | a_Lhs							},
#endif
#ifdef FEATURE_CALC
/*ca  */{"calculate",	EX_CALC,	ex_comment,	a_Cmds,					q_Exrc				},
/*cas */{"case",	EX_CASE,	ex_case,	a_Lhs | a_Cmds,				q_Exrc				},
#endif
#ifdef FEATURE_MAKE
/*cc  */{"cc",		EX_CC,		ex_make,	a_Bang | a_Rhs,				q_Unsafe | q_SwitchB		},
#endif
/*cd  */{"cd",		EX_CD,		ex_cd,		a_Bang | a_File,			q_Exrc | q_Unsafe		},
/*cl  */{"close",	EX_CLOSE,	ex_xit,		a_Bang,					q_MayQuit			},
/*co  */{"copy",	EX_COPY,	ex_move,	a_Range | a_Target | a_Pflag,		q_Autop | q_Undo		},
/*col */{"color",	EX_COLOR,	ex_color,	a_Bang | a_Lhs | a_Rhs,			q_Exrc | q_Custom		},
/*d   */{"delete",	EX_DELETE,	ex_delete,	a_Range | a_Buffer | a_Count | a_Pflag,	q_Undo | q_Autop		},
#ifdef FEATURE_CALC
/*de  */{"default",	EX_DEFAULT,	ex_case,	a_Cmds,					q_Exrc				},
#endif
/*di  */{"display",	EX_DISPLAY,	ex_display,	a_Rhs									},
/*dig */{"digraph",	EX_DIGRAPH,	ex_digraph,	a_Bang | a_Rhs,				q_Exrc | q_Custom		},
#ifdef FEATURE_CALC
/*do  */{"doloop",	EX_DO,		ex_do,		a_Cmds,					q_Exrc				},
#endif
#ifdef FEATURE_AUTOCMD
/*doa */{"doautocmd",	EX_DOAUTOCMD,	ex_doautocmd,	a_Rhs,					q_Exrc				},
#endif
/*e   */{"edit",	EX_EDIT,	ex_edit,	a_Bang | a_Plus | a_File | d_File,	q_FileUnsafe | q_SwitchB	},
/*ec  */{"echo",	EX_ECHO,	ex_comment,	a_Rhs,					q_Exrc				},
/*el  */{"else",	EX_ELSE,	ex_then,	a_Cmds,					q_Exrc				},
#ifdef FEATURE_MAKE
/*er  */{"errlist",	EX_ERRLIST,	ex_errlist,	a_Bang | a_File | a_Filter,		q_SwitchB|q_FileUnsafe|q_Restricted},
#endif
/*erro*/{"error",	EX_ERROR,	ex_message,	a_Rhs,					q_Exrc				},
#ifdef FEATURE_CALC
/*ev  */{"eval",	EX_EVAL,	ex_if,		a_Cmds,					q_Exrc				},
#endif
/*ex  */{"ex",		EX_EDIT,	ex_edit,	a_Bang | a_Plus | a_File | d_File,	q_FileUnsafe | q_SwitchB	},
/*f   */{"file",	EX_FILE,	ex_file,	a_Bang | a_File,			q_FileUnsafe			},
#ifdef FEATURE_FOLD
/*fo  */{"fold",	EX_FOLD,	ex_fold,	a_Range | a_Bang | a_Rhs 						},
#endif
#ifdef FEATURE_CALC
/*for */{"foreach",	EX_FOR,		ex_while,	a_Lhs | a_Cmds,				q_Exrc				},
#endif
/*g   */{"global",	EX_GLOBAL,	ex_global,	a_Range|a_Bang|a_RegExp|a_Cmds|d_All,	q_Undo				},
/*go  */{"goto",	EX_GOTO,	ex_comment,	a_Line,					q_Zero | q_Autop | q_SwitchB	},
/*gu  */{"gui",		EX_GUI,		ex_gui,		a_Cmds,					q_Exrc				},
#ifdef FEATURE_MISC
/*h   */{"help",	EX_HELP,	ex_help,	a_Lhs | a_Rhs,				q_SwitchB			},
#endif
/*i   */{"insert",	EX_INSERT,	ex_append,	a_Line | a_Bang | a_Cmds | a_Text,	q_Undo | q_Ex			},
#ifdef FEATURE_CALC
/*if  */{"if",		EX_IF,		ex_if,		a_Cmds,					q_Exrc				},
#endif
/*j   */{"join",	EX_JOIN,	ex_join,	a_Range | a_Bang | a_Count | d_2Lines,	q_Autop | q_Undo		},
/*k   */{"k",		EX_MARK,	ex_mark,	a_Line | a_Lhs								},
/*l   */{"list",	EX_LIST,	ex_print,	a_Range | a_Count | a_Pflag						},
/*la  */{"last",	EX_LAST,	ex_next,	0,					q_Unsafe | q_SwitchB		},
#ifdef FEATURE_CALC
/*le  */{"let",		EX_LET,		ex_set,		a_Bang | a_Cmds,			q_Exrc | q_Custom		},
#endif
#ifdef FEATURE_MISC
/*se  */{"local",	EX_LOCAL,	ex_set,		a_Rhs,					q_Exrc		 		},
#endif
#ifdef FEATURE_LPR
/*lp  */{"lpr",		EX_LPR,		ex_lpr,		a_Range|a_Bang|a_Append|a_File|a_Filter|d_All,	q_Unsafe		},
#endif
/*m   */{"move",	EX_MOVE,	ex_move,	a_Range | a_Target,			q_Autop | q_Undo		},
/*ma  */{"mark",	EX_MARK,	ex_mark,	a_Line | a_Lhs								},
#ifdef FEATURE_MAKE
/*mak */{"make",	EX_MAKE,	ex_make,	a_Bang | a_Rhs,				q_Unsafe | q_SwitchB		},
#endif
/*map */{"map",		EX_MAP,		ex_map,		a_Bang | a_Rhs,				q_Exrc|q_Custom|q_Unsafe|q_CtrlV},
/*me  */{"message",	EX_MESSAGE,	ex_message,	a_Rhs,					q_Exrc				},
#ifdef FEATURE_MKEXRC
/*mk  */{"mkexrc",	EX_MKEXRC,	ex_mkexrc,	a_Bang | a_File,			q_Unsafe			},
#endif
/*n   */{"next",	EX_NEXT,	ex_next,	a_Bang | a_Files,			q_Unsafe | q_SwitchB		},
#ifdef FEATURE_SPLIT
/*ne  */{"new",		EX_SNEW,	ex_split										},
#endif
/*no  */{"normal",	EX_NORMAL,	ex_display,	a_Range | a_Bang | a_Cmds | d_None,	q_Undo 				},
#ifdef FEATURE_HLSEARCH
/*noh */{"nohlsearch",	EX_NOHLSEARCH,	ex_nohlsearch										},
#endif
/*nu  */{"number",	EX_NUMBER,	ex_print,	a_Range | a_Count | a_Pflag						},
/*o   */{"open",	EX_OPEN,	ex_edit,	a_Bang | a_Plus | a_File,		q_SwitchB | q_FileUnsafe	},
/*on  */{"only",	EX_ONLY,	ex_qall,	a_Bang | d_None								},
/*p   */{"print",	EX_PRINT,	ex_print,	a_Range | a_Count | a_Pflag						},
/*pre */{"previous",	EX_PREVIOUS,	ex_next,	a_Bang,					q_Unsafe | q_SwitchB		},
/*pres*/{"preserve",	EX_PRESERVE,	ex_qall,	d_None,					q_MayQuit			},
/*po  */{"pop",		EX_POP,		ex_pop,		a_Bang,					q_SwitchB			},
/*pu  */{"put",		EX_PUT,		ex_put,		a_Line,					q_Zero | a_Buffer | q_Undo	},
/*pus */{"push",	EX_PUSH,	ex_edit,	a_Bang | a_Plus | a_File,		q_FileUnsafe | q_SwitchB	},
/*q   */{"quit",	EX_QUIT,	ex_xit,		a_Bang,					q_MayQuit			},
/*qa  */{"qall",	EX_QALL,	ex_qall,	a_Bang,					q_MayQuit			},
/*r   */{"read",	EX_READ,	ex_read,	a_Line | a_File | a_Filter,		q_Zero|q_Undo|q_Unsafe|q_Restricted},
/*red */{"redo",	EX_REDO,	ex_undo,	a_Lhs,					q_Autop				},
#ifdef FEATURE_REGION
/*reg */{"region",	EX_REGION,	ex_region,	a_Range | a_Lhs | a_Rhs							},
#endif
/*rew */{"rewind",	EX_REWIND,	ex_next,	a_Bang,					q_Unsafe | q_SwitchB		},
/*s   */{"substitute",	EX_SUBSTITUTE,	ex_substitute,	a_Range|a_RegExp|a_RegSub|a_Rhs|a_Count|a_Pflag, q_Autop|q_Undo|q_Exrc	},
#ifdef FEATURE_SPLIT
/*sN  */{"sNext",	EX_SPREVIOUS,	ex_next,	0,					q_Unsafe			},
/*sa  */{"sall",	EX_SALL,	ex_sall,	0									},
#endif
/*saf */{"safely",	EX_SAFELY,	ex_then,	a_Cmds,					q_Exrc				},
#if defined(FEATURE_SPLIT) && defined(FEATURE_MISC)
/*sbb */{"sbbrowse",	EX_SBBROWSE,	ex_buffer,	a_Bang									},
#endif
#ifdef FEATURE_BROWSE
/*sbr */{"sbrowse",	EX_SBROWSE,	ex_browse,	a_Bang|a_Rhs								},
#endif
/*se  */{"set",		EX_SET,		ex_set,		a_Bang | a_Rhs,				q_Exrc | q_Custom		},
/*sh  */{"shell",	EX_SHELL,	ex_suspend,	0,					q_Unsafe			},
#ifdef FEATURE_SPLIT
/*sl  */{"slast",	EX_SLAST,	ex_next,	0,					q_Unsafe			},
/*sn  */{"snext",	EX_SNEXT,	ex_next,	a_Files,				q_Unsafe			},
/*snew*/{"snew",	EX_SNEW,	ex_split,	0,					q_Unsafe			},
#endif
/*so  */{"source",	EX_SOURCE,	ex_source,	a_Bang | a_File,			q_Exrc | q_Unsafe | q_Restricted	},
#ifdef FEATURE_SPLIT
/*sp  */{"split",	EX_SPLIT,	ex_split,	a_Line | a_Plus | a_File | a_Filter,	q_FileUnsafe			},
/*sr  */{"srewind",	EX_SREWIND,	ex_next,	a_Bang,					q_Unsafe			},
#endif
/*st  */{"stop",	EX_STOP,	ex_suspend,	a_Bang,					q_Unsafe | q_Restricted		},
#ifdef FEATURE_SPLIT
/*sta */{"stag",	EX_STAG,	ex_tag,		a_Rhs									},
#endif
#ifdef FEATURE_MISC
/*stac*/{"stack",	EX_STACK,	ex_stack,	d_None									},
#endif
/*su  */{"suspend",	EX_SUSPEND,	ex_suspend,	a_Bang,					q_Unsafe | q_Restricted		},
#ifdef FEATURE_CALC
/*sw  */{"switch",	EX_SWITCH,	ex_switch,	a_Cmds,					q_Exrc				},
#endif
/*t   */{"to",		EX_COPY,	ex_move,	a_Range | a_Target | a_Pflag,		q_Autop | q_Undo		},
/*ta  */{"tag",		EX_TAG,		ex_tag,		a_Bang | a_Rhs,				q_SwitchB			},
/*th  */{"then",	EX_THEN,	ex_then,	a_Cmds,					q_Exrc			 	},
/*try */{"try",		EX_TRY,		ex_then,	a_Cmds,					q_Exrc			 	},
/*u   */{"undo",	EX_UNDO,	ex_undo,	a_Lhs,					q_Autop				},
/*una */{"unabbreviate",EX_UNABBR,	ex_map,		a_Bang | a_Lhs,				q_Exrc | q_Custom | q_CtrlV	},
#ifdef FEATURE_ALIAS
/*unal*/{"unalias",	EX_UNALIAS,	ex_alias,	a_Lhs,					q_Exrc | q_Custom		},
#endif
#ifdef FEATURE_MAPDB
/*unb */{"unbreak",	EX_UNBREAK,	ex_map,		a_Bang | a_Rhs,				q_CtrlV				},
#endif
#ifdef FEATURE_FOLD
/*unf */{"unfold",	EX_UNFOLD,	ex_fold,	a_Range | a_Bang | a_Rhs 						},
#endif
/*unm */{"unmap",	EX_UNMAP,	ex_map,		a_Bang | a_Rhs,				q_Exrc | q_Custom | q_CtrlV	},
#ifdef FEATURE_REGION
/*unr */{"unregion",	EX_UNREGION,	ex_region,	a_Range | a_Lhs								},
#endif
/*v   */{"vglobal",	EX_VGLOBAL,	ex_global,	a_Range|a_Bang|a_RegExp|a_Cmds|d_All,	q_Undo				},
/*ve  */{"version",	EX_VERSION,	ex_version,	0,					q_Exrc								},
/*vi  */{"visual",	EX_VISUAL,	ex_edit,	a_Bang | a_Plus | a_File,		q_FileUnsafe | q_SwitchB	},
/*w   */{"write",	EX_WRITE,	ex_write,	a_Range|a_Bang|a_Append|a_File|a_Filter|d_All,	q_Unsafe		},
/*wa  */{"warning",	EX_WARNING,	ex_message,	a_Rhs,					q_Exrc				},
#ifdef FEATURE_CALC
/*wh  */{"while",	EX_WHILE,	ex_while,	a_Cmds,					q_Exrc				},
#endif
#ifdef FEATURE_MISC
/*wi  */{"window",	EX_WINDOW,	ex_window,	a_Lhs									},
#endif
/*wn  */{"wnext",	EX_WNEXT,	ex_next,	a_Bang,					q_SwitchB			},
#ifdef FEATURE_SPELL
/*wo  */{"words",	EX_WORDS,	ex_check,	a_Bang | a_Rhs,				q_Exrc | q_Custom		},
/*wordf*/{"wordfile",	EX_WORDFILE,	ex_wordfile,	a_Bang | a_Files,			q_Exrc|q_Custom|q_FileUnsafe|q_Restricted},
#endif
/*wq  */{"wquit",	EX_WQUIT,	ex_xit,		a_Bang | a_File | d_File,		q_FileUnsafe | q_MayQuit	},
/*x   */{"xit",		EX_XIT,		ex_xit,		a_Bang | a_File ,			q_FileUnsafe | q_MayQuit	},
/*y   */{"yank",	EX_YANK,	ex_delete,	a_Range | a_Buffer | a_Count						},
#ifdef FEATURE_MISC
/*z   */{"z",		EX_Z,		ex_z,		a_Line | a_Rhs								},
#endif
/*~   */{"~",		EX_SUBRECENT,	ex_substitute,	a_Range | a_Rhs,			q_Undo				},
#ifdef FEATURE_ALIAS
/*~   */{" "/*dummy name*/,EX_DOALIAS,	ex_doalias,	a_Range | a_Bang | a_Rhs,		q_Exrc 				},
#endif
};


/* This variable is used for detecting nested global statements */
static int 	globaldepth;



/* This function discards info from an EXINFO struct.  The struct itself
 * is not freed, since it is usually just a local variable in some function.
 */
void exfree(xinf)
	EXINFO	*xinf;	/* the command to be freed */
{
	int	i;

	if (xinf->fromaddr)	markfree(xinf->fromaddr);
	if (xinf->toaddr)	markfree(xinf->toaddr);
	if (xinf->destaddr)	markfree(xinf->destaddr);
	if (xinf->re)		safefree(xinf->re);
	if (xinf->lhs)		safefree(xinf->lhs);
	if (xinf->rhs)		safefree(xinf->rhs);
	for (i = 0; i < xinf->nfiles; i++)
	{
		assert(xinf->file && xinf->file[i]);
		safefree(xinf->file[i]);
	}
	if (xinf->file)		safefree(xinf->file);
}



/* This function skips over blanks and tabs */
static void skipwhitespace(refp)
	CHAR	**refp;	/* pointer to scan variable */
{
	while (*refp && (**refp == ' ' || **refp == '\t'))
	{
		scannext(refp);
	}
}




/* This function attempts to parse a window ID.  If there is no window ID,
 * then it leaves xinf->win unchanged; else it sets xinf->win to the given
 * window.  Returns ElvTrue unless there was an error.
 *
 * This function also sets the default buffer to the default window's state
 * buffer, or the specified window's main buffer.  It is assumed that
 * xinf->window has already been initialized to the default window.
 */
static ELVBOOL parsewindowid(refp, xinf)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
{
	long	num;
	MARK	start;
	WINDOW	win;

	/* set default buffer, assuming default window. */
	if (xinf->window->state && xinf->window->state->pop)
		xinf->defaddr = *xinf->window->state->pop->cursor;
	else
		xinf->defaddr = *xinf->window->cursor;
	if (markbuffer(&xinf->defaddr) != bufdefault)
	{
		xinf->defaddr.buffer = bufdefault;
		xinf->defaddr.offset = 0L;
	}

	/* if doesn't start with a digit, ignore it */
	if (!*refp || !elvdigit(**refp))
	{
		return ElvTrue;
	}

	/* convert the number */
	start = scanmark(refp);
	for (num = 0; *refp && elvdigit(**refp); scannext(refp))
	{
		num = num * 10 + **refp - '0';
	}

	/* if doesn't end with a ':', then it's not meant to be a window id */
	if (!*refp || **refp != ':' || num <= 0)
	{
		scanseek(refp, start);
		return ElvTrue;
	}

	/* eat the ':' character */
	scannext(refp);

	/* convert the number to a WINDOW */
	for (win = winofbuf((WINDOW)0, (BUFFER)0);
	     win && o_windowid(win) != num;
	     win = winofbuf(win, (BUFFER)0))
	{
	}
	if (!win)
	{
		msg(MSG_ERROR, "bad window");
		return ElvFalse;
	}

	/* use the named window */
	xinf->window = win;
	xinf->defaddr = *win->cursor;
	return ElvTrue;
}

/* This function attempts to parse a single buffer name.  It returns True
 * unless an invalid buffer is specified.
 *
 * If an explicit buffer is named, then it sets the default address to that
 * buffer's undo point.  (Else it is left set window's cursor, as set by
 * the parsewindowid() function.)
 */
static ELVBOOL parsebuffername(refp, xinf)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
{
	CHAR	*tmp;
	CHAR	bufname[200];/* buffer name, as a string */
	int	len, nest;
	BUFFER	buf;

	/* if the first character isn't '(' then this isn't a buffer name */
	if (!*refp || **refp != '(')
	{
		return ElvTrue;
	}

	/* collect the characters into buffer */
	for (len = 0, nest = 1; scannext(refp) && nest > 0; )
	{
		/* Treat some characters specially */
		if (**refp == '(')
			nest++;
		else if (**refp == ')')
			nest--;
		else if (**refp == '|' && nest == 1)
			nest = 0;
		else if (**refp == '\n')
			break; /* and don't do scannext() in for-loop */

		/* add this character, unless we hit end */
		if (nest > 0)
			bufname[len++] = **refp;
	}
	bufname[len] = '\0';

#ifdef FEATURE_CALC
	/* if the name begins with a '=' then evaluate it */
	if (*bufname == '=')
	{
		tmp = calculate(bufname + 1, NULL, CALC_ALL);
		if (!tmp)
			return ElvFalse;
	}
	else
#endif
		tmp = bufname;

	/* try to find the buffer */
	buf = buffind(tmp);
	if (!buf)
	{
		msg(MSG_ERROR, "[S]no buffer named $1", tmp);
		return ElvFalse;
	}
	xinf->defaddr.buffer = buf;
	marksetoffset(&xinf->defaddr, buf->docursor);
	return ElvTrue;
}


/* If the command supports a regular expression, then this function parses
 * that regular expression from the command line and compiles it.  If the
 * command also supports replacement text, then that is parsed too.
 */
static ELVBOOL parseregexp(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	CHAR	delim;	/* delimiter character */
	CHAR	*retext;/* source code of the regular expression */

	/* If this command doesn't use regular expressions, then do nothing.
	 * (By the way, not using regexps implies that it doesn't use
	 * replacement text either.)
	 */
	if ((flags & a_RegExp) == 0)
	{
		return ElvTrue;
	}

	/* get the delimiter character.  Can be any punctuation character
	 * except '|'.  If no delimiter is given, then use an empty string.
	 */
	if (*refp && elvpunct(**refp) && **refp != '|')
	{
		/* remember the delimiter */
		delim = **refp;
		scannext(refp);

		/* collect the characters of the regexp source code */
		retext = (*refp && **refp) ? regbuild(delim, refp, ElvTrue) : NULL;

		/* if an empty regexp was explicitly given (":s//" rather than
		 * ":s"), then remember that.  We need to distinguish between
		 * an explicit empty regexp and an implicit one, because if
		 * there is no explicit regexp then there can't be any
		 * replacement text either.
		 */
		if (!retext)
		{
			retext = empty;
			empty[0] = (CHAR)0;
		}
	}
	else
	{
		retext = NULL;
		delim = '\n'; /* illegal delimiter, affects a_RegSub text below */
		if (xinf->command == EX_SUBSTITUTE)
			xinf->command = EX_SUBAGAIN;
	}

	/* compile the regular expression */
	xinf->re = regcomp(retext ? retext : empty,
			   xinf->window ? xinf->window->state->cursor : NULL);

	/* We don't need the source to the regexp anymore.  If the source was
	 * anything other than the empty string, then free its memory.
	 */
	if (retext && retext != empty)
		safefree(retext);

	/* detect errors in the regexp */
	if (!xinf->re)
	{
		/* error message already written out by regcomp() */
		return ElvFalse;
	}

	/* if no substitution text is needed, then we're done. */
	if ((flags & a_RegSub) == 0)
	{
		return ElvTrue;
	}

	/* We do use replacement text.  If there was no regexp, then the
	 * replacement text is implied to be "~" -- so :s with no args will
	 * repeat the previous replacement command.
	 */
	if (!retext)
	{
		xinf->lhs = CHARdup(toCHAR(o_magic ? "~" : "\\~"));
		return ElvTrue;
	}

	/* Collect characters up to the next delimiter to be the replacement
	 * text.  Same rules as the regular expression.  The first delimiter
	 * has already been comsumed.
	 */
	if (delim == '\n')
	{
		/* no delimiter implies that there is no replacement text */
		xinf->lhs = (CHAR *)safealloc(1, sizeof(CHAR));
	}
	else
	{
		xinf->lhs = regbuild(delim, refp, ElvFalse);
		if (!xinf->lhs)
		{
			/* zero-length string, if nothing else */
			xinf->lhs = (CHAR *)safealloc(1, sizeof(CHAR));
		}

		/* consume the closing delimiter */
		if (*refp && **refp == delim)
		{
			scannext(refp);
		}
	}

	return ElvTrue;
}


/* This function parses an address, and places the results in xinf->to.
 * Returns ElvTrue unless there is an error.
 */
ELVBOOL exparseaddress(refp, xinf)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
{
	BLKNO	bi;	/* bufinfo block of the buffer being addressed */
	long	lnum;	/* line number */
	long	delta;	/* movement amount */
	CHAR	sign;	/* '+' or '-' for delta */
	long	start;	/* where search stared, so we can detect wrap */
	long	buflines;/* number of lines in buffer */
	MARKBUF	m;	/* start of a line */
	MARK	mark;
	CHAR	ch;
	regexp	*re;

	/* if nothing, do nothing */
	if (!*refp)
		return ElvTrue;

	/* find the default line number */
	bi = bufbufinfo(markbuffer(&xinf->defaddr));
	lnum = markline(&xinf->defaddr);
	xinf->fromoffset = 0;

	/* parse the address */
	switch (**refp)
	{
	  case '.':
		/* use the line containing the default address */
		scannext(refp);
		break;

	  case '/':
	  case '?':
		/* if buffer is empty then fail */
		if (o_bufchars(markbuffer(&xinf->defaddr)) <= 1)
		{
			msg(MSG_ERROR, "no match");
			return ElvFalse;
		}

		/* parse & compile the regular expression */
		delta = (**refp == '?') ? -1 : 1;
		if (!parseregexp(refp, xinf, a_RegExp))
		{
			return ElvFalse;
		}

		/* allow the visual 'n' and 'N' commands to search for the
		 * same regexp, unless the "saveregexp" option is turned off.
		 */
		re = xinf->re;
		if (o_saveregexp)
		{
			if (searchre)
			{
				safefree(searchre);
			}
			searchre = xinf->re;
			searchforward = (ELVBOOL)(delta > 0);
			searchhasdelta = ElvFalse;
			xinf->re = NULL;
		}

		/* search for the regular expression */
		start = lnum;
		buflines = o_buflines(markbuffer(&xinf->defaddr));
		do
		{
			/* find next line */
			lnum += delta;
			if (o_wrapscan)
			{
				if (lnum == 0)
					lnum = buflines;
				else if (lnum > buflines)
					lnum = 1;
			}
			else if (lnum == 0)
			{
				msg(MSG_ERROR, "no match above");
				return ElvFalse;
			}
			else if (lnum > buflines)
			{
				msg(MSG_ERROR, "no match below");
				return ElvFalse;
			}

			/* see if this line contains the regexp */
			if (regexec(re, marktmp(m, markbuffer(&xinf->defaddr), lowline(bi, lnum)), ElvTrue))
			{
				delta = 0;
			}

			/* if we've wrapped back to our starting point,
			 * then we've failed.
			 */
			if (lnum == start)
			{
				msg(MSG_ERROR, "no match");
				return ElvFalse;
			}

			/* did the user cancel the search? */
			if (guipoll(ElvFalse))
			{
				return ElvFalse;
			}

		} while (delta != 0);

		/* If we get here, then we've found a match, and lnum is set
		 * to its line number.  Good!
		 */
		xinf->fromoffset = (re->leavep >= 0 ? re->leavep : re->startp[0]);
		break;

	  case '\'':
	  case '`':
		ch = **refp;
		if (!scannext(refp))
		{
			msg(MSG_ERROR, "incomplete mark name");
			return ElvFalse;
		}
#ifdef FEATURE_AUTOCMD
		if (**refp == '[')
			mark = autop;
		else if (**refp == ']')
			mark = aubottom;
		else
#endif
		if (**refp >= 'a' && **refp <= 'z')
			mark = namedmark[**refp - 'a'];
		else
		{
			msg(MSG_ERROR, "bad mark name");
			return ElvFalse;
		}

		/* sanity checks */
		if (!mark)
		{
			msg(MSG_ERROR, "[C]'$1 unset", **refp);
			return ElvFalse;
		}
		if (markbuffer(&xinf->defaddr) != markbuffer(mark))
		{
			if (xinf->anyaddr)
			{
				msg(MSG_ERROR, "would span buffers");
				return ElvFalse;
			}
			xinf->defaddr = *mark;
		}

		/* use it */
		lnum = markline(mark);
		if (ch == '`')
			xinf->fromoffset = markoffset(mark);
		scannext(refp);
		break;

	  case '0':
	  case '1':
	  case '2':
	  case '3':
	  case '4':
	  case '5':
	  case '6':
	  case '7':
	  case '8':
	  case '9':
		for (lnum = 0; *refp && elvdigit(**refp); scannext(refp))
		{
			lnum = lnum * 10 + **refp - '0';
		}
		if (lnum < 0 || lnum > o_buflines(markbuffer(&xinf->defaddr)))
		{
			msg(MSG_ERROR, "bad line number");
			return ElvFalse;
		}
		break;

	  case '$':
		scannext(refp);
		lnum = o_buflines(markbuffer(&xinf->defaddr));
		break;

	  default:
		/* use the default address, but don't consume any chars */
		lnum = markline(&xinf->defaddr);
	}

	/* followed by a delta? */
	skipwhitespace(refp);
	if (*refp && (**refp == '+' || **refp == '-' || elvdigit(**refp)))
	{
		/* delta implies whole-line addressing */
		xinf->fromoffset = 0;

		/* find the sign & magnitude of the delta */
		sign = **refp;
		if (elvdigit(sign))
			sign = '+';
		else
			scannext(refp);
		for (delta = 1; **refp == sign; scannext(refp), delta++)
		{
		}
		if (delta == 1 && *refp && elvdigit(**refp))
		{
			for (delta = **refp - '0';
			     scannext(refp) && elvdigit(**refp);
			     delta = delta * 10 + **refp - '0')
			{
			}
		}

		/* add the delta to the line number */
		if (sign == '+')
			lnum += delta;
		else
			lnum -= delta;

		/* if sum is invalid, complain */
		if (lnum < 1 || lnum > o_buflines(markbuffer(&xinf->defaddr)))
		{
			msg(MSG_ERROR, "bad delta");
		}
	}

	xinf->to = lnum;
	return ElvTrue;
}


/* This parses a command name, and sets xinf->command accordingly.  Returns
 * ElvTrue if everything is okay, or ElvFalse if the command is unrecognized or
 * was given addresses but doesn't allow addresses.
 */
static ELVBOOL parsecommandname(refp, xinf)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
{
	ELVBOOL	anymatch;
	int	matches;	/* number of commands that match so far */
	int	firstmatch;	/* first command that matched */
	char	cmdname[20];	/* command name */
	int	len;		/* number of characters in name so far */
	MARK	start;		/* where the command name started */
	int	i;

	/* if no command, then assume either "goto" or comment */
	if (!*refp || **refp == '\n' || **refp == '|')
	{
		strcpy(cmdname, (xinf->anyaddr || (xinf->window && markbuffer(xinf->window->cursor) != xinf->defaddr.buffer)) ? "goto" : "\"");
		for (firstmatch = QTY(cmdnames) - 1;
		     firstmatch >= 0 && strcmp(cmdnames[firstmatch].name, cmdname);
		     firstmatch--)
		{
		}
		goto Found;
	}

	/* begin by checking for aliases */
	start = scanmark(refp);
#ifdef FEATURE_ALIAS
	for (len = 0; len < QTY(cmdname) - 1 && *refp && elvalnum(**refp); scannext(refp))
	{
		cmdname[len++] = **refp;
	}
	cmdname[len] = '\0';
	xinf->cmdname = exisalias(cmdname, ElvFalse);
	if (xinf->cmdname)
	{
		xinf->cmdidx = QTY(cmdnames) - 1;
		xinf->command = EX_DOALIAS;
		return ElvTrue;
	}
	scanseek(refp, start);
#endif /* FEATURE_ALIAS */

	/* start with shortest possible command name, and extend the command
	 * name as much as possible without eliminating all commands from
	 * matching.  When we get it as long as possible, then the first
	 * matching name is the one we'll use.
	 */
	len = 0;
	firstmatch = 0;
	anymatch = ElvFalse;
	do
	{
		/* copy the first (or next) character of the name into a buffer */
		cmdname[len++] = **refp;

		/* see how many commands match this command so far */
		for (matches = 0, i = firstmatch; i < QTY(cmdnames); i++)
		{
			/* In the cmdnames[] array, commands with the same
			 * initial letters are grouped together.  Have we
			 * reached the end of the group that interests us?
			 */
			if (len > 1 && (cmdnames[i].name[0] != cmdname[0] || strncmp(cmdnames[i].name, cmdname, (size_t)(len - 1))))
			{
				/* no more matches are possible -- we reached
				 * the end of this (len-1) letter group.
				 */
				break;
			}
			if ((CHAR)cmdnames[i].name[len - 1] == **refp)
			{
				/* Partial match, but keep looking.  If this
				 * was the first match with this length then
				 * remember it.
				 */
				matches++;
				if (matches == 1)
				{
					firstmatch = i;
					anymatch = ElvTrue;
				}
			}
		}

	} while (matches > 0 && scannext(refp) && elvalnum(**refp));

	/* at this point, if "matches" is zero and "firstmatch" has been set,
	 * then we may have read one too many characters for the command name,
	 * so we need to adjust the position of the *refp.
	 */
	if (matches == 0
		&& anymatch
		&& (strlen(cmdnames[firstmatch].name) == (unsigned)(len - 1)
			|| !*refp
			|| !elvalpha(**refp)))
	{
		len--;
		markaddoffset(start, len);
		scanseek(refp, start);
		matches = 1;
	}

	/* If we still haven't found anything, give up! */
	if (matches == 0)
	{
		cmdname[len] = '\0';
		msg(MSG_ERROR, "[s]bad command name $1", cmdname);
		return ElvFalse;
	}

	/* so I guess we found a match. */
Found:
	assert(firstmatch >= 0);
	xinf->cmdidx = firstmatch;
	xinf->command = cmdnames[firstmatch].code;
	xinf->cmdname = cmdnames[firstmatch].name;
	return ElvTrue;
}


/* This function parses multiplied command names, as in :<<<<.  This can't
 * possibly fail, so this isn't a boolean function.
 */
static void parsemulti(refp, xinf)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
{
	/* the default for all commands is 1. */
	xinf->multi = 1;

	/* if this command can be multiplied, then see how many we have */
	if (cmdnames[xinf->cmdidx].flags & a_Multi)
	{
		while (*refp && **refp == (CHAR)xinf->cmdname[0])
		{
			xinf->multi++;
			scannext(refp);
		}
	}
}

/* This function parses an optional '!' character for some commands.  This
 * can't possibly fail, so this isn't a boolean function.
 */
static void parsebang(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	/* if this supports '!', and a '!' character really is given, then
	 * consume the '!' character and set the "bang" flag.
	 */
	if ((flags & a_Bang) != 0 && *refp && **refp == '!')
	{
		scannext(refp);
		xinf->bang = ElvTrue;
	}
}


/* This function parses an optional "+line" argument, for commands which
 * support it.
 */
static void parseplus(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	/* if the command doesn't support "+line" or the next character isn't
	 * '+' then do nothing.
	 */
	if ((flags & a_Plus) == 0 || !*refp || **refp != '+')
	{
		return;
	}

	/* nothing else should have used the xinf->lhs field yet */
	assert(!xinf->lhs);

	/* skip the '+' */
	scannext(refp);

	/* collect the chars of the command string */
	if (*refp && **refp == '"')
	{
		/* collect all characters within quotes */
		scannext(refp);
		while (*refp && **refp != '\n' && **refp != '"')
		{
			buildCHAR(&xinf->lhs, **refp);
			scannext(refp);
		}
		if (*refp && **refp == '"')
			scannext(refp);
	}
	else
	{
		/* collect following characters up to next whitespace or '|' */
		while (*refp && !elvspace(**refp) && **refp != '|')
		{
			buildCHAR(&xinf->lhs, **refp);
			scannext(refp);
		}
	}
}


/* This function parses an optional print flag and delta, if the command
 * supports them.
 */
static void parseprintflag(refp, xinf, flags, quirks)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command arguments */
	long	quirks;	/* bitmap of command quirks */
{
	CHAR		sign;
	static PFLAG	pflag = PF_NONE;

	/* if no print flag is possible, skip parsing this */
	if (!*refp || (flags & a_Pflag) == 0)
	{
		goto NoFlag;
	}

	/* try to parse a print flag here */
	switch (**refp)
	{
	  case 'p':
		scannext(refp);
		xinf->pflag = PF_PRINT;
		break;

	  case 'l':
		scannext(refp);
		if (*refp && **refp == '#')
		{
			scannext(refp);
			xinf->pflag = PF_NUMLIST;
		}
		else
		{
			xinf->pflag = PF_LIST;
		}
		break;

	  case '#':
		scannext(refp);
		if (*refp && **refp == 'l')
		{
			scannext(refp);
			xinf->pflag = PF_NUMLIST;
		}
		else
		{
			xinf->pflag = PF_NUMBER;
		}
		break;

	  default:
		goto NoFlag;
	}

	/* we have a print flag -- tweak it via "list" and "number" options */
	if (o_number(xinf->window))
		xinf->pflag = (xinf->pflag == PF_LIST || xinf->pflag == PF_NUMLIST) ? PF_NUMLIST : PF_NUMBER;
	if (o_list(xinf->window))
		xinf->pflag = (xinf->pflag == PF_NUMBER || xinf->pflag == PF_NUMLIST) ? PF_NUMLIST : PF_LIST;

	/* now see if we have a delta */
	if (refp && (**refp == '+' || **refp == '-'))
	{
		sign = **refp;
		while (scannext(refp) && elvdigit(**refp))
		{
			xinf->delta = xinf->delta * 10 + **refp - '0';
		}
		if (sign == '-')
		{
			xinf->delta = -xinf->delta;
		}
	}

	/* remember this print flag as the default for next time */
	pflag = xinf->pflag;
	return;

NoFlag:	/* see if we're supposed to autoprint this command.  If so, then
	 * assume the previous print flag is the default to use here.
	 */
	if ((quirks & q_Autop) != 0
	 && o_autoprint
	 && globaldepth == 0
	 && xinf->window
	 && xinf->window->state
	 && (xinf->window->state->flags & (ELVIS_POP|ELVIS_1LINE)) == 0
	 && xinf->window->state->enter
	 && markbuffer(scanmark(refp)) == buffind(toCHAR(EX_BUF)))
	{
		if (pflag == PF_NONE)
			pflag = PF_PRINT;
		xinf->pflag = pflag;
	}
}

/* This function parses single word which doesn't contain any unquoted
 * whitespace (if the command supports this).
 */
static void parselhs(refp, xinf, flags, quirks)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command arguments */
	long	quirks;	/* bitmap of command quirks */
{
	CHAR	quote;	/* quote character -- either ^V or \ */

	/* if the command doesn't use lhs, then do nothing */
	if ((flags & a_Lhs) == 0)
		return;

	/* choose the proper quote character */
	quote = (quirks & q_CtrlV) ? ELVCTRL('V') : '\\';

	/* collect characters up to next whitespace or end of command */
	while (*refp && **refp != ' ' && **refp != '\t' && **refp != '\n'
		&& (**refp != '|' || (*refp)[1] == '\n'))
	{
		/* backslash followed by whitespace becomes just whitespace */
		if (**refp == quote)
		{
			scannext(refp);
			if (!*refp)
			{
				buildCHAR(&xinf->lhs, quote);
			}
			else if (**refp == ' ' || **refp == '\t' || **refp == '|')
			{
				buildCHAR(&xinf->lhs, **refp);
			}
			else
			{
				buildCHAR(&xinf->lhs, quote);
				buildCHAR(&xinf->lhs, **refp);
			}
		}
		else
		{
			/* unquoted character */
			buildCHAR(&xinf->lhs, **refp);
		}
		scannext(refp);
	}
}

/* This function parses an optional print flag and delta, if the command
 * supports them.
 */
static void parserhs(refp, xinf, flags, quirks)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
	long	quirks;	/* bitmap of command quirks */
{
	CHAR	quote;	/* quote character -- either ^V or \ */

	/* if the command doesn't use rhs, then do nothing */
	if ((flags & a_Rhs) == 0)
		return;

	/* choose the proper quote character */
	quote = (quirks & q_CtrlV) ? ELVCTRL('V') : '\\';

	/* collect characters up to end of command */
	while (*refp && **refp != '\n' && **refp != '|')
	{
		/* backslash followed by whitespace becomes just whitespace */
		if (**refp == quote)
		{
			scannext(refp);
			if (!*refp)
			{
				buildCHAR(&xinf->rhs, quote);
			}
			else if (**refp == '|')
			{
				buildCHAR(&xinf->rhs, **refp);
			}
			else
			{
				buildCHAR(&xinf->rhs, quote);
				buildCHAR(&xinf->rhs, **refp);
			}
		}
		else
		{
			/* unquoted character */
			buildCHAR(&xinf->rhs, **refp);
		}
		scannext(refp);
	}
}

/* This function parses an optional cut buffer name, if the command
 * supports them.  A cut buffer name is a single letter, or a " followed
 * a letter, digit, or another ".
 */
static void parsecutbuffer(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	/* if the command doesn't support cut buffer names, do nothing */
	if ((flags & a_Buffer) == 0)
		return;

	/* if @ then use the anonymous cut buffer */
	if (*refp && **refp == '@')
	{
		xinf->cutbuf = '1';
		scannext(refp);
	}

	/* if double-quote, then following character is buffer name.  Also,
	 * letters letters, digits, <, and > don't require a " character.
	 */
	if (*refp && (elvalnum(**refp) || **refp == '<' || **refp == '>' || (**refp == '"' && scannext(refp))))
	{
		xinf->cutbuf = **refp;
		scannext(refp);
	}
}

/* This function parses an optional count, if the command supports them.
 * A count is a series of digits.  If no count is given, then the count
 * field is set to -1.
 */
static void parsecount(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	if ((flags & a_Count) == 0 || !*refp || !elvdigit(**refp))
	{
		/* no count given */
		xinf->count = -1;
	}
	else
	{
		/* count given -- convert it to binary */
		do
		{
			xinf->count = xinf->count * 10 + **refp - '0';
			scannext(refp);
		} while (*refp && elvdigit(**refp));
	}
}


/* This is a "helper" function for parsecmds().  This function parses { ... } */
static RESULT parsenested(refp, xinf)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
{
	CHAR	ch;
	MARK	start;
	int	nest;

	/* if there was text before the {...}, then add a newline in between */
	if (xinf->rhs)
		buildCHAR(&xinf->rhs, '\n');

	/* collect characters up to next } character which appears
	 * at the front of a line.  Ignore blank lines.
	 */
	start = scanmark(refp);
	for (ch = '\n', nest = 0; scannext(refp); )
	{
		if (ch == '\n' && **refp == '}')
		{
			if (nest == 0)
				break;
			nest--;
		}
		if (ch == '{' && **refp == '\n')
			nest++;
		if (ch == '\n' && (**refp == ' ' || **refp == '\t'))
			continue;
		if (ch != '\n' || **refp != '\n')
			buildCHAR(&xinf->rhs, **refp);
		ch = **refp;
	}

	/* if we hit end without finding } then we need more. */
	if (!*refp)
	{
		/* if not interactive, then there won't be more so we
		 * should give an error message.
		 */
		if (!markbuffer(start)
		 || CHARcmp(o_bufname(markbuffer(start)), toCHAR(EX_BUF)))
		{
			msg(MSG_ERROR, "missing }");
		}
		return RESULT_MORE;
	}

	/* eat the closing } */
	scannext(refp);

	return RESULT_COMPLETE;
}

/* This function parses an optional command list, if the command supports them.
 * A command list is a string which may contain any characters except newline.
 * (In particular, '|' is allowed.)  Returns RESULT_COMPLETE if successful,
 * RESULT_ERROR if unsuccessful, or RESULT_MORE if we hit the end of the buffer
 * while looking for the end of a multi-line command.
 */
static RESULT parsecmds(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	ELVBOOL	shell;	/* is this command for a shell? */
	ELVBOOL	bol;	/* at front of a line? */
	CHAR	*scan;
	CHAR	ch;
	MARKBUF	end;
	MARK	start;
	int	i;

	/* if the command doesn't use cmds, then do nothing */
	if ((flags & a_Cmds) == 0)
		return RESULT_COMPLETE;

	/* determine whether this command is for a shell or for ex */
	shell = (ELVBOOL)(xinf->command == EX_BANG || (flags & a_Filter) != 0);

	/* skip whitespace */
	skipwhitespace(refp);

	/* ex commands allow a ...backslash-newline... syntax */
	if (!shell)
	{
		for (bol = ElvFalse; *refp && **refp && **refp != '\n'; scannext(refp))
		{
			/* if backslash which preceeds newline, then delete
			 * the backslash and treat the newline literally.
			 */
			bol = ElvFalse;
			ch = **refp;
			if (ch == '\\' || ch == '{')
			{
				if (!scannext(refp))
					continue;
				if (**refp != '\n')
					buildCHAR(&xinf->rhs, ch);
				else if (ch == '{')
					return parsenested(refp, xinf);
				else
					bol = ElvTrue;
			}
			buildCHAR(&xinf->rhs, **refp);
		}
		if (xinf->rhs)
			buildCHAR(&xinf->rhs, '\n');

		return bol ? RESULT_MORE : RESULT_COMPLETE;
	}

	/* shell commands are more complex, due to substitutions... */
	bol = ElvTrue;
	while (*refp && **refp && **refp != '\n')
	{
		/* copy most characters literally, but perform
		 * substitutions for !, %, #
		 */
		ch = **refp;
		switch (ch)
		{
		  case '%':
			scan = o_filename(markbuffer(&xinf->defaddr));
			break;

		  case '#':
			scan = buffilenumber(refp);
			break;

		  case '!':
			if (bol)
			{
				scan = o_previouscommand;
			}
			else
			{
				buildCHAR(&xinf->rhs, **refp);
				scan = xinf->rhs;
			}
			break;

		  case '\\':
			scannext(refp);
			if (*refp && !CHARchr(toCHAR("%#!@"), **refp)) /* !!!removed '\\' */
			{
				buildCHAR(&xinf->rhs, (_CHAR_)'\\');
				ch = **refp;
			}
			else if (*refp && **refp == '@')
			{
				if (!xinf->window)
				{
					msg(MSG_ERROR, "can't use \\\\@ during initialization");
				}
				end = *xinf->window->cursor;
				start = wordatcursor(&end, ElvFalse);
				if (!start)
				{
					msg(MSG_ERROR, "cursor not on word");
					return RESULT_ERROR;
				}
				for (i = markoffset(&end) - markoffset(start),
					scanalloc(&scan, start);
				     scan && i > 0;
				     scannext(&scan), i--)
				{
					buildCHAR(&xinf->rhs, *scan);
				}
				scanfree(&scan);

				/* don't add anything else */
				scan = xinf->rhs;
				break;
			}
			/* fall through for all but \@ ... */

		  default:
			buildCHAR(&xinf->rhs, **refp);
			scan = xinf->rhs;
		}

		/* are we trying to add a string? */
		if (scan != xinf->rhs)
		{
			/* we want to... but do we have the string? */
			if (scan)
			{
				for ( ; *scan; scan++)
					buildCHAR(&xinf->rhs, *scan);
			}
			else
			{
				msg(MSG_ERROR, "[C]no value to substitute for $1", ch);
				return RESULT_ERROR;
			}
		}

		/* Move on to the next character.  Note that we need to
		 * check (*refp) because it may have become NULL during
		 * the processing of a backslash.
		 */
		if (*refp)
			scannext(refp);
		bol = ElvFalse;
	}

	/* if shell command, then remember it for later ! substitutions */
	if (shell)
	{
		if (!xinf->rhs)
			return RESULT_ERROR; /* blank commands are illegal */
		if (o_previouscommand)
			safefree(o_previouscommand);
		o_previouscommand = CHARkdup(xinf->rhs);
	}

	return RESULT_COMPLETE;
}


/* Convert a filenamerules string into a bitmap of rules */
ELVFNR exfilenamerules(rulestr)
	CHAR	*rulestr;	/* the rules string to convert */
{
	ELVFNR	rules = (ELVFNR)0;

	if (calcelement(rulestr, toCHAR("tilde")))
		rules |= ELVFNR_TILDE;
	if (calcelement(rulestr, toCHAR("dollar")))
		rules |= ELVFNR_DOLLAR;
	if (calcelement(rulestr, toCHAR("paren")))
		rules |= ELVFNR_PAREN;
	if (calcelement(rulestr, toCHAR("wildcard")))
		rules |= ELVFNR_WILDCARD;
	if (calcelement(rulestr, toCHAR("special")))
		rules |= ELVFNR_SPECIAL;
	if (calcelement(rulestr, toCHAR("space")))
		rules |= ELVFNR_SPACE;
	return rules;
}

/* This is a "helper" function for parsefileargs().  This function adds a
 * single item to the list of filename arguments.  Optionally, it can
 * attempt to expand wildcards and recursively add each matching filename.
 * Returns ElvTrue if successful, ElvFalse if error.
 */
ELVBOOL exaddfilearg(file, nfiles, filename, rules)
	char	***file;	/* ptr to a dynamic array of char* pointers */
	int	*nfiles;	/* number of files in file array */
	char	*filename;	/* the filename (or wildcard expression) to add */
	ELVFNR	rules;		/* describes how names should be manipulated */
{
	char	*match;		/* a name matching a wildcard */
	char	**old;		/* previous value of xinf->file pointer */
	int	start;		/* index into xinf->file of wildcard's expansion */
	ELVBOOL	mustfree=ElvFalse;	/* if ElvTrue, then free "match" before returning*/
	int	i = 0;
#if defined(ANY_UNIX) && defined(FEATURE_MISC)
	char	tmp[256];
#endif

	/* never use wildcards for URLs */
	if (((match = urllocal(filename)) != NULL && match != filename)
#if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
		|| urlremote(filename)
#endif
	)
	{
		rules = (ELVFNR)0;
	}

	if (rules & (ELVFNR_DOLLAR | ELVFNR_PAREN))
	{
		/* expand any environment variables */
		filename = tochar8(calculate(toCHAR(filename), NULL,
			 (ELVFNR)(((rules & ELVFNR_DOLLAR) ? CALC_DOLLAR : 0) | 
				  ((rules & ELVFNR_PAREN) ? CALC_PAREN : 0))));
		if (!filename)
			return ElvFalse;
	}

	/* If this operating system uses backslashes between names,
	 * then replace any forward slashes with backslashes.  This is
	 * intended to allow MS-DOS users to type forward slashes in
	 * their filenames, if they so prefer.
	 */
#if OSDIRDELIM != '/'
# if defined(PROTOCOL_HTTP) || defined(PROTOCOL_FTP)
	if (!urlremote(filename))
# endif
	{
		for (i = 0; filename[i]; i++)
		{
			if (filename[i] == '/')
			{
				filename[i] = OSDIRDELIM;
			}
		}
	}
#endif /* OSDIRDELIM != '/' */

	if (rules & ELVFNR_TILDE)
	{
		/* replace ~ with the name of the home directory */
		match = NULL;
		if (filename[0] == '~' && filename[1] == '+')
			match = dircwd(), i = 2;
		else if (filename[0] == '~' && filename[1] == '-')
			match = tochar8(o_previousdir), i = 2;
		else if (filename[0] == '~')
			match = tochar8(o_home), i = 1;
		if (match && filename[i] == OSDIRDELIM && filename[i + 1])
		{
			filename = safedup(dirpath(match, filename + i + 1));
			mustfree = ElvTrue;
		}
		else if (match && (filename[i] == OSDIRDELIM || !filename[i]))
		{
			filename = safedup(match);
			mustfree = ElvTrue;
		}
#if defined(ANY_UNIX) && defined(FEATURE_MISC)
		else if (filename[0] == '~' && elvalpha(filename[1]))
		{
			filename = safedup(expanduserhome(tochar8(filename), tmp));
			mustfree = ElvTrue;
		}
#endif /* ANY_UNIX && FEATURE_MISC */
	}

	if (rules & ELVFNR_WILDCARD)
	{
		/* If there are any matches... */
		if ((match = dirfirst(filename, ElvFalse)) != NULL)
		{
			/* for each match... */
			start = *nfiles;
			do
			{
				/* add the matching name */
				exaddfilearg(file, nfiles, match, (ELVFNR)0);

				/* ripple the new name back, to keep things sorted */
				for (i = *nfiles - 1;
				     i > start && strcmp((*file)[i], (*file)[i - 1]) < 0;
				     i--)
				{
					match = (*file)[i];
					(*file)[i] = (*file)[i - 1];
					(*file)[i - 1] = match;
				}
			} while ((match = dirnext()) != (char *)0);
		}
	}
	else /* literal filename */
	{
		/* [re-]allocate the *files array if necessary.  Note that
		 * we've arranged it so there will always be at least one
		 * NULL pointer at the end of the list.
		 */
		if (*nfiles == 0 || (*nfiles + 1) % FILEGRAN == 0)
		{
			old = *file;
			*file = (char **)safealloc(*nfiles + 1 + FILEGRAN, sizeof(char *));
			if (old)
			{
				memcpy(*file, old, *nfiles * sizeof(char *));
				safefree(old);
			}
		}

		/* append the new filename */
		(*file)[(*nfiles)++] = safedup(filename);
	}

	/* if necessary, free the filename */
	if (mustfree)
	{
		safefree(filename);
	}

	return ElvTrue;
}

/* This function parses filenames, if the command supports them. */
static ELVBOOL parsefileargs(refp, xinf, flags)
	CHAR	**refp;	/* pointer to the (CHAR *) used for scanning command */
	EXINFO	*xinf;	/* info about the command being parsed */
	long	flags;	/* bitmap of command attributes */
{
	CHAR	*filename;
	CHAR	*scan, *val;
	ELVFNR	rules;
#ifdef FEATURE_BACKTICK
	CHAR	*expr;
	int	i;
#endif

	/* if no filenames expected, or hit end of cmd, then do nothing */
	if ((flags & (a_File|a_Files|a_Filter|a_Append)) == 0 || !*refp)
	{
		return ElvTrue;
	}

	/* If filter is allowed, and next character is a '!' the we have
	 * a filter command.
	 */
	if ((flags & a_Filter) != 0 && **refp == '!')
	{
		/* skip the initial '!' */
		if (!scannext(refp))
			return ElvTrue;	/* missing filter will be detected later */

		/* begin the rhs argument with a bang */
		buildCHAR(&xinf->rhs, (_CHAR_)'!');

		/* collect characters up to next newline */
		return (ELVBOOL)(RESULT_COMPLETE == parsecmds(refp, xinf, flags | a_Cmds));
	}

	/* An initial backslash could have been used to quote a ! or > at
	 * the start of a filename.  If the first character is backslash,
	 * and the second is either ! or > then skip the backslash.  Also,
	 * if the second is another backslash and backslash isn't used as
	 * the directory separator on this operating system, then skip the
	 * backslash; this should allow "\\foo" to be "\foo" under UNIX,
	 * while still allowing "\\machine\dir\file" to be parsed as a UNC
	 * name under Win32.
	 */
	if (*refp && **refp == '\\')
	{
		scannext(refp);
		if (!*refp)
		{
			msg(MSG_ERROR, "oops");
			return ElvFalse;
		}
		if (**refp != '!' && **refp != '>' &&
			(**refp != '\\' || OSDIRDELIM == '\\'))
		{
			scanprev(refp);
		}
	}

	/* parse the value of the filenamerules option */
	rules = exfilenamerules(o_filenamerules);

	/* collect each whitespace-delimited filename argument */
	for (filename = val = NULL;
	     *refp && **refp != '|' && **refp != '\n';
	     scannext(refp))
	{
		/* if whitespace, then process filename (if any) and then skip */
		switch (**refp)
		{
		  case ' ':
			if (filename && !(rules & ELVFNR_SPACE))
			{
				buildCHAR(&filename, **refp);
				break;
			}
			/* else fall through... */

		  case '\t':
			/* do we have a filename? */
			if (filename)
			{
				/* store the name */
				if (!exaddfilearg(&xinf->file, &xinf->nfiles, tochar8(filename), rules))
					goto Error;

				/* free the buildCHAR copy of the name */
				safefree(filename);
				filename = NULL;
			}
			break;

		  case '\\':
			/* backslash can be used to quote some characters. */
			scannext(refp);
			if (!*refp)
				break; /* hit end of string */
			if (**refp == '*' && OSDIRDELIM == '\\')
			{
				/* This operating system seems use backslashes
				 * in filenames.  So an expression such as
				 * "foo\*.c" should NOT become "foo*.c"
				 */
				scanprev(refp);
			}
			else if (!CHARchr(toCHAR(" \t\\#!*`"), **refp))
			{
				/* backslash isn't followed by a char that might
				 * require backslash quoting, so the backslash
				 * must be here for some other reason.  Keep it.
				 */
				scanprev(refp);
			}
			else if (**refp == '\\' && filename == NULL)
			{
				/* A double-backslash at the start of a name
				 * is kept, so Win32 users can have names like
				 * \\machine\directory\file
				 */
				scanprev(refp);
			}
			buildCHAR(&filename, **refp);
			break;

		  case '#':
			/* if filenamerules doesn't contain "special" just add char */
			if (!(rules & ELVFNR_SPECIAL))
			{
				buildCHAR(&filename, **refp);
				break;
			}

			/* replace # with name of alternate file, or a numbered
			 * file.
			 */
			scan = buffilenumber(refp);
			if (!scan)
			{
				msg(MSG_ERROR, "[c]no value to substitute for $1", '#');
				goto Error;
			}
			for ( ; *scan; scan++)
			{
				buildCHAR(&filename, *scan);
			}
			break;

		  case '%':
			/* filenamerules doesn't contain "special" just add char */
			if (!(rules & ELVFNR_SPECIAL))
			{
				buildCHAR(&filename, **refp);
				break;
			}

			/* replace % with name of current file */
			scan = o_filename(markbuffer(&xinf->defaddr));
			if (scan)
			{
				for ( ; *scan; scan++)
				{
					buildCHAR(&filename, *scan);
				}
			}
			else
			{
				msg(MSG_ERROR, "[c]no value to substitute for $1", '%');
				goto Error;
			}
			break;

		  case '\0':
			msg(MSG_ERROR, "NUL not allowed in file name");
			goto Error;

#ifdef FEATURE_BACKTICK
		  case '`':
			if (!(rules & ELVFNR_WILDCARD))
			{
				buildCHAR(&filename, **refp);
				break;
			}

			/* build an expression which reads from a program */
			expr = NULL; 
			buildstr(&expr, "shell(\"");
			scannext(refp);
			for (; *refp && **refp && **refp != '|' && **refp != '\n' && **refp != '`'; scannext(refp))
			{
				if (**refp == '\\')
				{
					if (!scannext(refp) || !**refp || **refp == '\n')
					{
						msg(MSG_ERROR, "unterminated `prog` expression");
						goto Error;
					}
					if (**refp != '`' || **refp != '\\')
						buildCHAR(&expr, '\\');
				}
				else if (**refp == '"')
					buildCHAR(&expr, '\\');
				buildCHAR(&expr, **refp);
			}
			buildstr(&expr, "\")");

			/* evaluate the expression */
			val = calculate(expr, NULL, CALC_ALL);
			safefree(expr);
			if (!val)
				/* error message already given */
				goto Error;

			/* resume previous filename, if any */
			if (filename)
			{
				expr = filename;
				filename = NULL;
				for (i = 0; (unsigned)i < CHARlen(expr); i++)
					buildCHAR(&filename, expr[i]);
				safefree(expr);
			}

			/* Find whitespace-delimited names.  Treat any special
			 * chars as normal chars.
			 */
			for (; *val; val++)
			{
				if (*val == '\t' || (*val == ' ' && (rules & ELVFNR_SPACE)))
				{
					if (filename)
					{
						/* store the name */
						if (!exaddfilearg(&xinf->file, &xinf->nfiles, tochar8(filename), rules))
							goto Error;

						/* free the buildCHAR copy of the name */
						safefree(filename);
						filename = NULL;
					}
				}
				else
					buildCHAR(&filename, *val);
			}
			/* NOTE: val (returned by calculate()) is static so
			 * we don't need to worry about freeing it.
			 */
			break;
#endif /* FEATURE_BACKTICK */

		  default:
			/* Append the character to the name.  If this results
			 * in a partial name of ">>" and either this isn't the
			 * first file, or this command doesn't support appending
			 * then complain.
			 */
			if (buildCHAR(&filename, **refp) == 2
			 && !CHARcmp(filename, ">>")
			 && (xinf->nfiles > 0 || (flags & a_Append) == 0))
			{
				msg(MSG_ERROR, "bad >>", xinf->cmdname);
				goto Error;
			}
		}
	}

	/* if we were working on a filename when we ended the loop, add it */
	if (filename)
	{
		if (!exaddfilearg(&xinf->file, &xinf->nfiles, tochar8(filename), rules))
			goto Error;
		safefree(filename);
		filename = (CHAR *)0;
	}

	/* complain if we have multiple filenames and only support one */
	if (xinf->nfiles > 1 && (flags & a_Files) == 0)
	{
		msg(MSG_ERROR, "too many files");
		goto Error;
	}

	/* If no files named, maybe go for a default name */
	if (xinf->nfiles == 0 && (flags & d_File) != 0)
	{
		if (o_filename(markbuffer(&xinf->defaddr)) == NULL)
		{ /* nishi */
			msg(MSG_ERROR, "[S]no file name for $1",
				o_bufname(markbuffer(&xinf->defaddr)));
			goto Error;
		}
		exaddfilearg(&xinf->file, &xinf->nfiles, tochar8(o_filename(markbuffer(&xinf->defaddr))), (ELVFNR)0);
	}

	return ElvTrue;


Error:
	/* If we were in the middle of a filename, free the filename.
	 * Other stuff will be freed by the calling function.
	 */
	if (filename)
	{
		safefree(filename);
	}
	return ElvFalse;
}


/* Parse a single command, and leave *refp pointing past the last character of
 * the command.  Return RESULT_COMPLETE normally, RESULT_MORE if the command is
 * incomplete, or RESULT_ERROR for an error (after outputting an error message).
 */
static RESULT parse(win, refp, xinf)
	WINDOW	win;	/* window that the command applies to */
	CHAR	**refp;	/* pointer to (CHAR *) used for scanning the command */
	EXINFO	*xinf;	/* where to place the results of the parse */
{
	CHAR	sep;	/* separator character, from scanning */
	MARKBUF	orig;	/* original mark of "refp", so we can seek back later */
	MARKBUF	rngdef;	/* default range */
	long	rngfrom;/* "from" line number of range */
	long	rngto;	/* "to" line number of range */
	long	flags;	/* bitmap of command arguments */
	long	quirks;	/* bitmap of command quirks */
	ELVBOOL	sel;	/* did address come from selection? */
	CHAR	*p2;	/* a second scanning variable */
	CHAR	*lntext;/* text of current line, used for trace */
	RESULT	result;	/* result of parsing */
	ELVBOOL	twoaddrs;/* have two addresses been seen on this line? */
	int	i;
	MARK	m;
#ifdef FEATURE_MAPDB
	BUFFER	log;	/* log buffer, for debugging */
	MARKBUF	logstart; /* start of the log buffer, for debugging */
	MARKBUF	logend;	/* end of the log buffer, for debugging */
	long	loglen;	/* length of log message */
	CHAR	*logstr;/* start of string command, or NULL if from buffer */
#endif
#ifdef FEATURE_V
	long	oldreport;
#endif

	/* if verbose, then display this line in window, or on stderr */
	if (o_verbose >= (win ? 5 : 3))
	{
		lntext = NULL;
		for (scandup(&p2, refp); p2 && *p2 != '\n'; scannext(&p2))
		{
			buildCHAR(&lntext, *p2);
		}
		scanfree(&p2);
		if (lntext)
		{
			msg(MSG_INFO, "[S]:$1", lntext);
			safefree(lntext);
		}
	}

	/* set defaults */
	memset((char *)xinf, 0, sizeof *xinf);
	xinf->window = win;
	twoaddrs = ElvFalse;

	/* skip leading ':' characters and whitespace */
	while (*refp && (**refp == ':' || **refp == ' ' || **refp == '\t'))
	{
		scannext(refp);
	}

	/* remember where this command started */
	orig = *scanmark(refp);
#ifdef FEATURE_MAPDB
	logstr = markbuffer(&orig) ? NULL : *refp;
#endif

	/* If no command was entered, then assume the command line should be
	 * "+p", so the user can simply hit <Enter> to step through the file.
	 * Only do this for interactively entered commands in ex mode, though!
	 */
	if ((!*refp || **refp == '\n')
		&& win
		&& 0 == (win->state->flags & (ELVIS_POP|ELVIS_ONCE|ELVIS_1LINE))
		&& markbuffer(&orig) == buffind(toCHAR(EX_BUF)))
	{
		/* parse the "+p" command, unless at end of buffer */
		if (markline(win->cursor) < o_buflines(markbuffer(win->cursor)))
		{
			scanstring(&p2, toCHAR("+p"));
			result = parse(win, &p2, xinf);
			scanfree(&p2);
		}
		else
		{
			msg(MSG_ERROR, "no more lines");
			result = RESULT_ERROR;
		}

		return result;
	}

	/* parse the window id */
	if (win)
	{
		skipwhitespace(refp);
		if (!parsewindowid(refp, xinf))
		{
			return RESULT_ERROR;
		}
	}
	else
	{
		xinf->defaddr.buffer = bufdefault;
	}

	/* parse the buffer name (unless window has visible selection) */
	skipwhitespace(refp);
	if ((!xinf->window || !xinf->window->seltop) && !parsebuffername(refp, xinf))
	{
		return RESULT_ERROR;
	}

	/* parse addresses */
	sel = ElvFalse;
#ifdef FEATURE_V
	xinf->putbacktop.buffer = NULL;
#endif
	if (xinf->defaddr.buffer)
	{
		skipwhitespace(refp);
		xinf->to = markline(&xinf->defaddr);
		xinf->from = xinf->to;
		if (*refp && **refp == '%')
		{
			scannext(refp);
			xinf->from = 1;
			xinf->to = o_buflines(markbuffer(&xinf->defaddr));
			xinf->anyaddr = ElvTrue;
			sel = ElvFalse;
		}
		else if ((*refp && strchr("./?'0123456789$+-,;", (char)**refp)))
		{
			do
			{
				/* Shift addresses, so we always remember the
				 * last two addresses.
				 */
				xinf->from = xinf->to;

				/* Parse the address */
				if (!exparseaddress(refp, xinf))
				{
					return RESULT_ERROR;
				}

				/* We now have at least one address.  If this
				 * is the first address we've encountered,
				 * then copy it into "from" as well.
				 */
				if (!xinf->anyaddr)
				{
					xinf->from = xinf->to;
					xinf->anyaddr = ElvTrue;
				}
				else
				{
					twoaddrs = ElvTrue;
				}

				/* if followed by a semicolon, then this
				 * address becomes default.
				 */
				skipwhitespace(refp);
				if (*refp)
				{
					sep = **refp;
				}
				else
				{
					sep = '\0';
				}
				if (sep == ';')
				{
					marksetoffset(&xinf->defaddr,
						lowline(bufbufinfo(markbuffer(&xinf->defaddr)), xinf->to));
				}
				else if (xinf->window && xinf->window->selbottom)
				{
					xinf->defaddr = *xinf->window->selbottom;
				}
			} while (*refp && (**refp == ',' || **refp == ';') && scannext(refp));

			/* "from" can't come after "to" */
			if (xinf->from > xinf->to)
			{
				msg(MSG_ERROR, "bad range");
				return RESULT_ERROR;
			}
		}
#ifdef FEATURE_V
		else if (xinf->window && xinf->window->seltop)
		{
			if (xinf->window->seltype == 'r')
			{
				/* remember where the rectangle begins */
				xinf->putbackleft = xinf->window->selleft;
				xinf->putbackright = xinf->window->selright;
				xinf->putbacktop = *xinf->window->seltop;
				xinf->putbackbottom = *(*win->md->move)(
					xinf->window, xinf->window->selbottom,
					1L, 0L, ElvFalse);
				xinf->putbackchanges = markbuffer(&xinf->putbacktop)->changes;

				/* move the text into a cut buffer */
				xinf->defaddr.buffer = cutbuffer('_', ElvTrue);
				xinf->defaddr.offset = 0L;
				m = scanmark(refp);
				scanfree(refp);
				oldreport = o_report;
				o_report = 0L;
				cutyank('_', &xinf->putbacktop, &xinf->putbackbottom, 'r', 'y');
				o_report = oldreport;
				scanalloc(refp, m);
				xinf->cbchanges = xinf->defaddr.buffer->changes;

				/* use the entire cut buffer as the default */
				xinf->from = 1L;
				xinf->to = o_bufchars(xinf->defaddr.buffer);
			}
			else /* line or char */
			{
				/* act directly on the edit buffer */
				xinf->defaddr = *xinf->window->seltop;
				xinf->from = markline(xinf->window->seltop);
				xinf->to = markline(xinf->window->selbottom);
			}
			xinf->anyaddr = ElvTrue;
			twoaddrs = ElvTrue;
			sel = ElvTrue;

		}

		/* if a visible selection was marked, then unmark it now */
		if (xinf->window && xinf->window->seltop)
		{
			markfree(xinf->window->seltop);
			markfree(xinf->window->selbottom);
			xinf->window->seltop = xinf->window->selbottom = NULL;
		}
#endif /* FEATURE_V */
	}

	/* parse command name */
	skipwhitespace(refp);
	if (!parsecommandname(refp, xinf))
	{
		return RESULT_ERROR;
	}
	flags = cmdnames[xinf->cmdidx].flags;
	quirks = cmdnames[xinf->cmdidx].quirks;

	/* is the command legal in this context? */
	if (!win && 0 == (quirks & q_Exrc))
	{
		msg(MSG_ERROR, "[s]$1 is illegal during initialization", xinf->cmdname);
		return RESULT_ERROR;
	}
	if (((0 == (quirks & q_Restricted)) ? o_security != 'n' /* !normal */
					    : o_security == 'r')/* restricted */
	 && 0 != (quirks & q_Unsafe))
	{
		msg(MSG_ERROR, "[s]$1 is unsafe", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* if given addresses for a command which doesn't support addresses,
	 * then complain.  If the addresses were due to a visible selection,
	 * then ignore them silently.
	 */
	if (xinf->anyaddr && 0 == (flags & (a_Line|a_Range)))
	{
		if (sel)
		{
			/* visible selection; pretend no addresses were given */
			xinf->anyaddr = ElvFalse;
		}
		else
		{
			/* explicit addresses; complain */
			msg(MSG_ERROR, "[s]$1 doesn't use addresses", xinf->cmdname);
			return RESULT_ERROR;
		}
	}

	/* if "from" is 0 for a command which doesn't allow 0, then
	 * complain.
	 */
	if (xinf->anyaddr && xinf->from == 0 && 0 == (quirks & q_Zero))
	{
		msg(MSG_ERROR, "[s]$1 doesn't allow address 0", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* parse multiplied command names */
	parsemulti(refp, xinf);

	/* If a command allows both a print flag and a buffer name
	 * then parsing them can be tricky because this can lead
	 * to ambiguous situations.  According to POSIX docs,
	 * if there is no space after the command name (:dp) then
	 * the print flag is next; in all other situations, the
	 * cut buffer name comes first.  THEREFORE, if this command
	 * allows both a print flag and a buffer, and we appear to
	 * have a print flag appended to the command name, then
	 * we should parse the print flag now.
	 */
	if ((flags & (a_Pflag|a_Buffer)) == (a_Pflag|a_Buffer)
		&& *refp && (**refp == 'p' || **refp == 'l' || **refp == '#'))
	{
		parseprintflag(refp, xinf, flags, quirks);
	}

	/* parse an optional '!' appended to the name */
	parsebang(refp, xinf, flags);

	/* maybe parse a regular expression & replacement text */
	skipwhitespace(refp);
	if (!parseregexp(refp, xinf, flags))
	{
		return RESULT_ERROR;
	}

	/* maybe parse a target buffer and address */
	if (flags & a_Target)
	{
		/* The target allows a window, a buffer, and an
		 * address all to be specified.  Parsing these
		 * could clobber the source range fields, so we'll
		 * copy them into local variables while we parse.
		 */
		rngdef = xinf->defaddr;
		rngfrom = xinf->from;
		rngto = xinf->to;

		/* parse the window id */
		skipwhitespace(refp);
		if (!parsewindowid(refp, xinf))
		{
			return RESULT_ERROR;
		}

		/* parse the buffer name */
		skipwhitespace(refp);
		if (!parsebuffername(refp, xinf))
		{
			return RESULT_ERROR;
		}

		/* parse an address.  Note that we don't allow
		 * looping here, and the address is mandatory
		 * for commands that allow it.
		 */
		skipwhitespace(refp);
		if (*refp && (**refp == '\n' || **refp == '|'))
		{
			msg(MSG_ERROR, "[s]$1 requires destination", xinf->cmdname);
			return RESULT_ERROR;
		}
		if (!exparseaddress(refp, xinf))
		{
			return RESULT_ERROR;
		}

		/* create the "destaddr" mark */
		xinf->destaddr = markalloc(markbuffer(&xinf->defaddr),
			lowline(bufbufinfo(markbuffer(&xinf->defaddr)), xinf->to + 1));

		/* move the range info back where it belongs */
		xinf->defaddr = rngdef;
		xinf->from = rngfrom;
		xinf->to = rngto;
	}

	/* for some commands, parse an LHS */
	parselhs(refp, xinf, flags, quirks);
	skipwhitespace(refp);

#ifdef FEATURE_CALC /* if the :for command exists in this configuration... */
	/* as a special case, ":for LHS in files" accepts files arguments */
	if (xinf->command == EX_FOR && *refp && **refp == 'i')
	{
		scandup(&p2, refp);
		if (scannext(&p2) && *p2 == 'n'
		 && scannext(&p2) && (*p2 == ' ' || *p2 == '\t'))
		{
			/* move the scan pointer past "in " */
			scannext(refp);
			scannext(refp);
			skipwhitespace(refp);

			/* parse files instead of RHS */
			flags &= ~a_Cmds;
			flags |= a_Files;
		}
		scanfree(&p2);
	}
#endif /* FEATURE_CALC */

	/* for some commands, parse a RHS */
	parserhs(refp, xinf, flags, quirks);

	/* for some commands, parse an optional cut buffer name,
	 * optional count, optional "+arg", and optional print flag.
	 */
	skipwhitespace(refp);
	parsecutbuffer(refp, xinf, flags);
	skipwhitespace(refp);
	parsecount(refp, xinf, flags);
	skipwhitespace(refp);
	parseplus(refp, xinf, flags);
	skipwhitespace(refp);
	parseprintflag(refp, xinf, flags, quirks);

	/* for some commands, parse file arguments.  This includes
	 * filenames, "!cmd" filter commands, ">>" append tokens,
	 * and wildcard expansion.
	 */
	skipwhitespace(refp);
	if (!parsefileargs(refp, xinf, flags))
	{
		return RESULT_ERROR;
	}
	if (((0 == (quirks & q_Restricted)) ? o_security != 'n' /* !normal */
					    : o_security == 'r')/* restricted */
	 && 0 != (quirks & q_FileUnsafe)
	 && xinf->nfiles > 0)
	{
		msg(MSG_ERROR, "[s]$1 filename is unsafe", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* for some commands, parse a long argument string which
	 * may include the '|' character but not newline
	 */
	skipwhitespace(refp);
	result = parsecmds(refp, xinf, flags);
	if (result != RESULT_COMPLETE)
	{
		return result;
	}

	/* By this point, there should be no more arguments on this line. */
	skipwhitespace(refp);
	if (*refp && **refp != '|' && **refp != '\n')
	{
		msg(MSG_ERROR, "[s]too many arguments for $1", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* beware of EX-only commands */
	if (0 != (quirks & q_Ex)
		&& !xinf->rhs
		&& (!win || 0 != (win->state->flags & (ELVIS_POP|ELVIS_ONCE|ELVIS_1LINE))))
	{
		msg(MSG_ERROR, "[s]$1 is illegal in vi mode", xinf->cmdname);
		return RESULT_ERROR;
	}

	/* If the buffer is locked, then complain about edit commands.  Note
	 * that ":s/???/???/x" and ":!cmd" (with no addresses) are specifically
	 * permitted.
	 */
	if (0 != (quirks & q_Undo)
	 && o_locked(markbuffer(&xinf->defaddr))
	 && !(xinf->command == EX_SUBSTITUTE && xinf->rhs && CHARchr(xinf->rhs, 'x'))
	 && !(xinf->command == EX_BANG && !xinf->anyaddr) )
	{
		msg(MSG_ERROR, "[s]buffer $1 is locked", o_bufname(markbuffer(&xinf->defaddr)));
		return RESULT_ERROR;
	}

	/* Maybe parse extra text lines which are arguments to this command */
	if (!xinf->rhs && 0 != (flags & a_Text) && *refp)
	{
		/* skip past the newline on the command line */
		assert(**refp == '\n');
		scannext(refp);

		/* collect characters up to the next line containing only "." */
		for (i = 0;
		     *refp
			&& (**refp != '\n'
			    || (i >= 1 && xinf->rhs[i-1] != '.')
			    || (i >= 2 && xinf->rhs[i-2] != '\n'));
		    i++)
		{
			buildCHAR(&xinf->rhs, **refp);
			scannext(refp);
		}

		/* if all went well, then strip the "." from the end */
		if (*refp)
		{
			assert(i > 2);
			xinf->rhs[i - 1] = '\0';
		}
		else if (markbuffer(&orig) != NULL) /* not scanning string */
		{
			/* end not found, need more text */
			scanseek(refp, &orig);
			drawopencomplete(win);
			return RESULT_MORE;
		}
		assert(!*refp || **refp == '\n');
	}

	/* convert line numbers to marks (if there are any) */
	if ((flags & (a_Line|a_Range)) != 0
		&& (xinf->anyaddr || (flags & d_LnMask) != d_None))
	{
		/* if no lines, set the default */
		if (!xinf->anyaddr && (flags & d_LnMask) == d_All)
		{
			xinf->from = 1;
			xinf->to = o_buflines(markbuffer(&xinf->defaddr));
		}

		/* if only one address was given, and the default range
		 * include should include 2 lines, then add second line.
		 */
		if (!twoaddrs && (flags & d_LnMask) == d_2Lines)
		{
			xinf->to = xinf->from + 1;
		}

		/* if there was a count, add it to "from" to make "to" */
		if (xinf->count > 0)
		{
			xinf->to = xinf->from + xinf->count - 1;
		}

		/* create the "fromaddr" mark -- start of "from" line */
		xinf->fromaddr = markalloc(markbuffer(&xinf->defaddr), 
			lowline(bufbufinfo(markbuffer(&xinf->defaddr)), xinf->from));

		/* create the "toaddr" mark -- end of "to" line.  If that's
		 * the last line, then the computation can be tricky.
		 */
		if (xinf->to == o_buflines(markbuffer(&xinf->defaddr)))
			xinf->toaddr = markalloc(markbuffer(&xinf->defaddr),
					o_bufchars(markbuffer(&xinf->defaddr)));
		else
			xinf->toaddr = markalloc(markbuffer(&xinf->defaddr),
					lowline(bufbufinfo(markbuffer(&xinf->defaddr)), xinf->to + 1));
	}
	else
	{
		/* the cursor won't move */
		xinf->newcurs = (MARK)0;
	}

#ifdef FEATURE_MAPDB
	/* maybe add the commands to the map log */
	if (o_maplog != 'o' && (logstr || !o_internal(markbuffer(&orig)) ) )
	{
		log = bufalloc(toCHAR(TRACE_BUF), 0, ElvTrue);
		if (o_maplog == 'r')
		{
			bufreplace(marktmp(logstart, log, 0L), marktmp(logend, log, o_bufchars(log)), NULL, 0L);
			o_maplog = 'a';
		}
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, toCHAR(":"), 1L);
		if (logstr)
		{
			if (*refp)
				loglen = (int)(*refp - logstr);
			else
				loglen = CHARlen(logstr);
			assert(loglen > 0L);
			bufreplace(marktmp(logend, log, o_bufchars(log)),
					&logend, logstr, loglen);
		}
		else
			bufpaste(marktmp(logend, log, o_bufchars(log)),
					&orig, scanmark(refp));
		bufreplace(marktmp(logend, log, o_bufchars(log)), &logend, toCHAR("\n"), 1L);
	}
#endif /* FEATURE_MAPDB */

	/* move the scan point past the command separator */
	if (*refp)
		scannext(refp);

	return RESULT_COMPLETE;
}


/* Execute an ex command, after it has been parsed by the parse() function.
 * Return RESULT_COMPLETE normally, or RESULT_ERROR for errors (after outputting
 * an error message).
 */
static RESULT execute(xinf)
	EXINFO	*xinf;	/* the parsed command to execute */
{
	MARK		pline;	/* line to autoprint */
	RESULT		ret;
	BUFFER		origdef;/* original value of bufdefault */
	struct state_s	*state;
	long		quirks;	/* bitmap of command quirks */
	WINDOW		found;
#ifdef FEATURE_V
	long		oldreport;
#endif
#ifdef FEATURE_MAPDB
	long		loglines = 0;
	BUFFER		log;

	if (o_maplog != 'o' && (log = buffind(toCHAR(TRACE_BUF))) != NULL)
		loglines = o_buflines(log);
#endif

	/* Figure out the command's quirks.  Mostly this comes straight out of
	 * the cmdname[] table, but it can also be influenced by whether the
	 * command is running as part of an autocmd sequence.
	 */
	quirks = cmdnames[xinf->cmdidx].quirks;
#ifdef FEATURE_AUTOCMD
	if (aubusy)
		quirks &= ~(q_SwitchB|q_MayQuit);
#endif
#ifdef FEATURE_PROTO
	if (urlprotobusy)
		quirks &= ~(q_SwitchB|q_MayQuit);
#endif

	/* If we need to save an "undo" version of the buffer, then
	 * remember that fact.
	 */
	if (xinf->window && xinf->window->cursor && globaldepth == 0)
		bufwilldo(xinf->window->cursor, ElvFalse);
	if (globaldepth == 0 && (quirks & q_Undo))
	{
		if (xinf->destaddr)
			bufwilldo(xinf->destaddr, ElvTrue);
		else if (xinf->window)
			bufwilldo(xinf->window->cursor, ElvTrue);
		else if (xinf->toaddr)
			bufwilldo(xinf->toaddr, ElvTrue);
	}

	/* if global command, then increment globaldepth */
	if (xinf->command == EX_GLOBAL || xinf->command == EX_VGLOBAL)
	{
		globaldepth++;
	}

	/* if some other command executed as part of global, then set its
	 * "global" flag.  This is important for :g/re/s/re2/text/ commands.
	 */
	xinf->global = (ELVBOOL)(globaldepth > 0);

	/* if quit command, then cause "wrote..." messages to be displayed
	 * as INFO instead of the normal STATUS.  This is so they'll be queued
	 * and can be printed someplace else after the window is closed.
	 */
	if (quirks & q_MayQuit)
	{
		bufmsgtype = MSG_INFO;
	}

	/* if the command's default buffer isn't the window's default buffer,
	 * then make the command's default buffer be the one used by :set.
	 */
	origdef = bufdefault;
	if (xinf->window && markbuffer(&xinf->defaddr) !=
		markbuffer(xinf->window->state && xinf->window->state->pop
			? xinf->window->state->pop->cursor
			: xinf->window->cursor))
	{
		bufoptions(markbuffer(&xinf->defaddr));
	}

	/* Hooray!  Now all we need to do is execute the damn thing */
	ret = (*cmdnames[xinf->cmdidx].fn)(xinf);

	/* restore the options buffer to what it was before */
	bufoptions(origdef);

	/* if global command, then decrement globaldepth */
	if (xinf->command == EX_GLOBAL || xinf->command == EX_VGLOBAL)
	{
		globaldepth--;
	}

	/* if command failed, we're done. */
	if (ret != RESULT_COMPLETE)
	{
		/* !!! Should we call exfree(xinf) here? */
		return RESULT_ERROR;
	}

	/* if the window was deleted, we're done */
	if (xinf->window)
	{
		for (found = winofbuf(NULL, NULL);
		     found && found != xinf->window;
		     found = winofbuf(found, NULL))
		{
		}
		if (!found)
		{
			exfree(xinf);
			return RESULT_COMPLETE;
		}
	}

	/* most commands aren't allowed to switch buffers */
	if (xinf->newcurs
	 && markbuffer(xinf->window->cursor) != markbuffer(xinf->newcurs)
	 && !(quirks & q_SwitchB))
	{
		markfree(xinf->newcurs);
		xinf->newcurs = NULL;
	}

#ifdef FEATURE_V
	/* if rectangle, then copy any changes back into the edit buffer */
	if (xinf->putbacktop.buffer
	 && xinf->putbackchanges == xinf->putbacktop.buffer->changes
	 && xinf->cbchanges != xinf->defaddr.buffer->changes)
	{
		/* reselect the rectangle */
		found = xinf->window;
		found->seltop = markdup(&xinf->putbacktop);
		found->selbottom = markdup(&xinf->putbackbottom);
		found->selleft = xinf->putbackleft;
		found->selright = xinf->putbackright;
		found->seltype = 'r';

		/* delete the rectangle */
		oldreport = o_report;
		o_report = 0;
		cutyank('\0', found->seltop, found->selbottom, 'r', 'd');

		/* unselect it again */
		markfree(found->seltop);
		markfree(found->selbottom);
		found->seltop = found->selbottom = NULL;

		/* paste the changed buffer */
		pline = (xinf->window->md->move)(xinf->window,
						 &xinf->putbacktop, 0L,
						 xinf->putbackleft, ElvFalse);
		pline = markdup(pline);
		cutput('_', xinf->window, pline, ElvFalse, ElvFalse, ElvFalse);
		markfree(pline);
		o_report = oldreport;

		/* Leave the cursor at the top of the changed line */
		if (xinf->newcurs)
			markfree(xinf->newcurs);
		xinf->newcurs = markdup(&xinf->putbacktop);
	}
#endif /* FEATURE_V */

	/* move the cursor to where it wants to be */
	if (xinf->newcurs)
	{
		/* find the window's main state */
		for (state = xinf->window->state; state->acton; state = state->acton)
		{
		}

		/* if we're switching buffers, then we need to
		 * switch names, too.
		 */
		if (markbuffer(xinf->window->cursor) != markbuffer(xinf->newcurs))
		{
#ifdef FEATURE_MAPDB
			/* maybe add a note to the map log */
			if (o_maplog != 'o')
			{
				BUFFER	log;
				MARKBUF	logstart, logend;
				char	line[200];

				/* allocate the buffer */
				log = bufalloc(toCHAR(TRACE_BUF), 0, ElvTrue);

				/* if "reset", truncate & switch to "append" */
				if (o_maplog == 'r')
				{
					bufreplace(marktmp(logstart, log, 0L), marktmp(logend, log, o_bufchars(log)), NULL, 0L);
					o_maplog = 'a';
				}

				/* add a line */
				sprintf(line, "Buffer switch from (%s) to (%s) since log line %ld\n",
					tochar8(o_bufname(markbuffer(xinf->window->cursor))),
					tochar8(o_bufname(markbuffer(xinf->newcurs))),
					loglines);
				bufreplace(marktmp(logend, log, o_bufchars(log)),
					&logend,
					toCHAR(line),
					(long)strlen(line));
			}
#endif /* FEATURE_MAPDB */

			optprevfile(o_filename(markbuffer(xinf->window->cursor)),
				    markline(xinf->window->cursor));
			if (gui->retitle)
			{
				(*gui->retitle)(xinf->window->gw,
				 tochar8(o_filename(markbuffer(xinf->newcurs))
				    ? o_filename(markbuffer(xinf->newcurs))
				    : o_bufname(markbuffer(xinf->newcurs))));
			}
			marksetbuffer(state->cursor, markbuffer(xinf->newcurs));
			dispset(xinf->window, o_initialsyntax(markbuffer(xinf->newcurs))
			    ? "syntax"
			    : tochar8(o_bufdisplay(markbuffer(xinf->newcurs))));
			bufoptions(markbuffer(xinf->newcurs));

		}

		/* other stuff is easy */
		marksetoffset(state->cursor, markoffset(xinf->newcurs));
		marksetbuffer(state->top, markbuffer(xinf->newcurs));
		marksetoffset(state->top, markoffset(xinf->newcurs));
		marksetbuffer(state->bottom, markbuffer(xinf->newcurs));
		marksetoffset(state->bottom, markoffset(xinf->newcurs));
		markfree(xinf->newcurs);
		xinf->window->wantcol = state->wantcol = (*xinf->window->md->mark2col)(xinf->window, xinf->window->cursor, ElvTrue);

		assert(markbuffer(state->cursor) == markbuffer(state->top));
		assert(markbuffer(state->cursor) == markbuffer(state->bottom));
	}

	/* if the new cursor position is off the end of the buffer,
	 * then move it to the start of the last line, instead.
	 */
	if (xinf->window && markoffset(xinf->window->cursor) >= o_bufchars(markbuffer(xinf->window->cursor)))
	{
		marksetoffset(xinf->window->cursor, o_bufchars(markbuffer(xinf->window->cursor)));
		if (markoffset(xinf->window->cursor) > 0L)
		{
			markaddoffset(xinf->window->cursor, -1L);
			marksetoffset(xinf->window->cursor,
				markoffset((*xinf->window->md->move)
					(xinf->window, xinf->window->cursor, 0L, 0L, ElvFalse)));
		}
	}

	/* print flags and autoprinting happen here */
	if (xinf->pflag != PF_NONE)
	{
		/* Find the start of the line which contains the cursor.
		 * Also add delta.
		 */
		pline = (*xinf->window->md->move)(xinf->window,
				xinf->window->cursor, xinf->delta, 0L, ElvFalse);

		/* print the line */
		exprintlines(xinf->window, pline, 1, xinf->pflag);
	}

	/* some commands force us to regenerate the CUSTOM_BUF script. */
	if (quirks & q_Custom)
		eventupdatecustom(ElvTrue);

	/* free the data associated with that command */
	exfree(xinf);

	return RESULT_COMPLETE;
}


/* This function parses one or more ex commands, and executes them.   It
 * returns RESULT_COMPLETE if successful, RESULT_ERROR (after printing an
 * error message) if unsuccessful, or RESULT_MORE if the command is
 * incomplete as entered.  As a side-effect, the offset of the "top" mark
 * is moved passed any commands which have been completely parsed and
 * executed.
 *
 * If the "win" argument is NULL, then the parsing uses the parsing style
 * of a ".exrc" file: no window or line numbers are allowed, only commands
 * which have q_Exrc set and q_Unsafe cleared are allowed, the | character
 * is quoted via ^V instead of backslash, and blank lines are ignored (instead
 * of being interpretted as "+p").
 */
RESULT experform(win, top, bottom)
	WINDOW	win;	/* default window (implies default buffer) */
	MARK	top;	/* start of commands */
	MARK	bottom;	/* end of commands */
{
	EXINFO	xinfb;	/* buffer, holds info about command being parsed */
	CHAR	*p;	/* pointer used for scanning command line */
	long	next;	/* where the next command starts */

	/* start reading commands */
	scanalloc(&p, top);

	/* for each command... */
	while (p && markoffset(top) < markoffset(bottom))
	{
		/* remember the location, for error reporting */
		msgscriptline(top, NULL);

		/* parse an ex command */
		switch (parse(win, &p, &xinfb))
		{
		  case RESULT_ERROR:
			goto Fail;

		  case RESULT_MORE:
			goto More;

		  case RESULT_COMPLETE:
			; /* continue processing */
		}

		/* Suspend the scanning while we execute this command.
		 * We aren't allowed to change text while scanning.
		 */
		next = (p ? markoffset(scanmark(&p)) : markoffset(bottom));
		scanfree(&p);

		/* execute the command */
		if (execute(&xinfb) != RESULT_COMPLETE)
		{
			goto Fail2;
		}

		/* adjust "top" to point after the command, and resume scan */
		marksetoffset(top, next);
		scanalloc(&p, top);
	}
	scanfree(&p);
	return RESULT_COMPLETE;

Fail:
	scanfree(&p);
Fail2:
	exfree(&xinfb);
	return RESULT_ERROR;

More:
	scanfree(&p);
	exfree(&xinfb);
	return RESULT_MORE;
}

/* This function resembles experform(), except that this function parses
 * from a string instead of from a buffer.
 */
RESULT exstring(win, str, name)
	WINDOW	win;	/* default window (implies default buffer) */
	CHAR	*str;	/* the string containing ex commands */
	char	*name;	/* name for error reporting, or NULL if not known */
{
	EXINFO	xinfb;	/* buffer, holds info about command being parsed */
	CHAR	*p;	/* pointer used for scanning command line */
	EXCTLSTATE oldctlstate;
	RESULT	result;
#ifdef FEATURE_MISC
	void	*locals;
#endif

	/* commands run this way don't affect :if/:while/:switch expressions */
	exctlsave(oldctlstate);
#ifdef FEATURE_MISC
	locals = optlocal(NULL);
#endif

	/* start reading commands */
	scanstring(&p, str);

	/* for each command... */
	while (p && *p)
	{
		/* remember its location, for error reporting */
		msgscriptline(scanmark(&p), name);

		/* parse and execute one ex command.
		 *
		 * NOTE: Generally you shouldn't alter a buffer while a scan
		 * is in progress, but in this case we're only scanning a
		 * string, which shouldn't be affected by changing buffers,
		 * so it's okay.  We don't need to suspend the scan while
		 * we call execute().
		 */
		if (parse(win, &p, &xinfb) != RESULT_COMPLETE
		 || execute(&xinfb) != RESULT_COMPLETE)
		{
			goto Fail;
		}
	}
	scanfree(&p);
	result = RESULT_COMPLETE;
	goto Done;

Fail:
	scanfree(&p);
	exfree(&xinfb);
	result = RESULT_ERROR;
	/* and fall through... */

Done:
#ifdef FEATURE_MISC
	(void)optlocal(locals);
#endif
	exctlrestore(oldctlstate);
	return result;
}



/* This function checks to see whether a given string is an acceptable
 * abbreviation for a command name.  If so, it returns the full command name;
 * if not, it returns NULL
 */
CHAR *exname(name)
	CHAR	*name;	/* possible name of an ex command */
{
	int	i, len;

	/* non-alphabetic names get special treatment */
	len = CHARlen(name);
	if (len == 1 && !elvalpha(*name))
	{
		switch (*name)
		{
		  case '!':	return toCHAR("BANG");
		  case '"':	return toCHAR("QUOTE");
		  case '#':	return toCHAR("HASH");
		  case '<':	return toCHAR("LT");
		  case '=':	return toCHAR("EQ");
		  case '>':	return toCHAR("GT");
		  case '&':	return toCHAR("AMP");
		  case '~':	return toCHAR("TILDE");
		  case '@':	return toCHAR("AT");
		  case '(':	return toCHAR("OPEN"); /* not a real command */
		  case '{':	return toCHAR("OCUR"); /* not a real command */
		}
	}

	/* else look up the name in the cmdnames[] array */
	for (i = 0;
	     i < QTY(cmdnames) && strncmp(cmdnames[i].name, tochar8(name), (size_t)len);
	     i++)
	{
	}
	if (i < QTY(cmdnames))
	{
		return toCHAR(cmdnames[i].name);
	}
	return NULL;
}


/* This function is called when the user hits <Enter> after entering an
 * ex command line.  It returns RESULT_COMPLETE if successful, RESULT_ERROR
 * (after printing an error message) if unsuccessful, or RESULT_MORE if
 * the command is incomplete as entered.  As a side-effect, the offset of
 * the "top" mark is moved passed any commands which have been completely
 * parsed and executed.
 */
RESULT exenter(win)
	WINDOW	win;	/* window where an ex command has been entered */
{
	STATE	*state = win->state;

	return experform(win, state->top, state->bottom);
}


/* This function prints single line as ex output text.  If "number" is true,
 * it will precede each line with a line number.  If "list" is true, it will
 * make all characters visible (including tab) and show a '$' at the end of
 * each line.  Returns the offset of the last line output.
 */
long exprintlines(win, mark, qty, pflag)
	WINDOW	win;	/* window to write to */
	MARK	mark;	/* start of line to output */
	long	qty;	/* number of lines to print */
	PFLAG	pflag;	/* controls how the line is printed */
{
	CHAR	tmp[24];/* temp strings */
	CHAR	*scan;	/* used for scanning */
	long	last;	/* offset of start of last line */
	long	lnum;	/* line number */
	long	col;	/* output column number */
	ELVBOOL	number;	/* show line number? */
	ELVBOOL	list;	/* show all characters? */
	long	i;

	/* initialize "last" just to silence a compiler warning */
	last = 0;

	/* figure out how we'll show the lines */
	if (pflag == PF_NONE)
	{
		return markoffset(mark);
	}
	number = (ELVBOOL)(pflag == PF_NUMBER || pflag == PF_NUMLIST);
	list = (ELVBOOL)(pflag == PF_LIST || pflag == PF_NUMLIST);

	/* If we'll be showing line numbers, then find the first one now */
	if (number)
	{
		(void)lowoffset(bufbufinfo(markbuffer(mark)), markoffset(mark),
			(COUNT *)0, (COUNT *)0, (LBLKNO *)0, &lnum);
	}

	/* for each line... */
	for (scanalloc(&scan, mark); scan && qty > 0 && !guipoll(ElvFalse); scannext(&scan), qty--)
	{
		/* remember where this line started */
		last = markoffset(scanmark(&scan));

		/* output the line number, if we're supposed to */
		if (number)
		{
			memset(tmp, ' ', QTY(tmp));
			long2CHAR(tmp + 8, lnum);
			CHARcat(tmp, toCHAR("  "));
			drawextext(win, tmp + CHARlen(tmp) - 8, 8);
			lnum++;
		}

		/* scan the line and output each character */
		col = 0;
		for (; scan && *scan != '\n'; scannext(&scan))
		{
			if (*scan == '\t' && !list)
			{
				/* expand tab into a bunch of spaces */
				i = opt_totab(o_tabstop(markbuffer(mark)), col);
				col += i;
				memset(tmp, ' ', QTY(tmp));
				while (i > QTY(tmp))
				{
					drawextext(win, tmp, QTY(tmp));
					i -= QTY(tmp);
				}
				if (i > 0)
				{
					drawextext(win, tmp, (int)i);
				}
			}
			else if (*scan < ' ' || *scan == '\177')
			{
				tmp[0] = '^';
				tmp[1] = ELVCTRL(*scan);
				drawextext(win, tmp, 2);
				col += 2;
			}
			else if (*scan > '\177' && list)
			{
				sprintf((char *)tmp, "\\x%02x", *scan);
				i = CHARlen(tmp);
				drawextext(win, tmp, (int)i);
				col += i;
			}
			else
			{
				/* count consecutive normal chars */
				for (i = 1; i < scanright(&scan) && scan[i] >= ' ' && scan[i] < '\177'; i++)
				{
				}
				drawextext(win, scan, (int)i);
				col += i;
				scan += i - 1; /* plus one more @ top of loop */
			}
		}

		/* for "list", append a '$' */
		if (list)
		{
			tmp[0] = '$';
			tmp[1] = '\n';
			drawextext(win, tmp, 2);
		}
		else
		{
			tmp[0] = '\n';
			drawextext(win, tmp, 1);
		}
	}
	scanfree(&scan);
	return last;
}


#ifdef FEATURE_COMPLETE

# ifdef FEATURE_ALIAS
static char *cname(i)
	int	i;
{
	if (i < QTY(cmdnames) - 1)
		return cmdnames[i].name;
	else
		return exaliasname(i - (QTY(cmdnames) - 1));
}
# else
#  define cname(i)	cmdnames[i].name
#endif

/* Return the remainder of a command name, buffer name, or tag name.  If file
 * name completion is appropriate, then return NULL.  If nothing can be
 * completed then just return a string containing a single tab character.
 *
 * This function may turn on the completebinary option.  It does this when
 * it returns NULL (to denote file name completion) in a shell command line.
 */
CHAR *excomplete(win, from, to)
	WINDOW	win;	/* window where multiple matches are listed */
	MARK	from;	/* start of the line where completion is needed */
	MARK	to;	/* position of the cursor within that line */
{
	CHAR	*s;
	long	offset;
 static CHAR	tab[2] = "\t";
 static CHAR	common[10];/* long enough for any ex command name */
 	MARKBUF	tmp;
	CHAR	*cmdname;
	char	*scan, *found;
	int	len, mlen;
	int	i, j, match;
	ELVBOOL	filter;

	(void)scanalloc(&s, from);
	offset = markoffset(from);

	/* skip leading ':' characters and whitespace */
	while (*s && offset < markoffset(to) && (*s == ':' || elvspace(*s)))
	{
		scannext(&s);
		offset++;
	}

	/* if '(' then skip forward for matching ')'.  If we hit the cursor
	 * before that, then we want to perform filename completion (really
	 * buffername completion)
	 */
	if (*s == '(')
	{
		while (*s && offset < markoffset(to) && *s != ')')
		{
			scannext(&s);
			offset++;
		}
		if (offset == markoffset(to))
		{
			scanfree(&s);
			return NULL;
		}
	}

	/* if at start of line, or next char isn't a command, then always
	 * use a literal tab.  If at start of a ! command, then do filename
	 * completion.
	 */
	if (!s || offset >= markoffset(to) || !elvalpha(*s))
	{
		if (s && offset < markoffset(to) && *s == '!')
		{
			o_completebinary = ElvTrue;
			cmdname = NULL;
		}
		else
			cmdname = tab;
		scanfree(&s);
		return cmdname;
	}

	/* collect the characters of the command name */
	cmdname = NULL;
	do
	{
		len = buildCHAR(&cmdname, *s);
		offset++;
	} while (scannext(&s) && offset < markoffset(to) && elvalpha(*s));

	/* if followed by whitespace and then a !, then remember that */
	filter = ElvFalse;
	if (s && elvspace(*s))
	{
		while (scannext(&s) && elvspace(*s))
		{
		}
		if (s && *s == '!')
			filter = ElvTrue;
	}
	scanfree(&s);

	/* if cursor is at end of command name, then try to expand the name.
	 * This doesn't know about aliases!
	 */
	if (offset == markoffset(to))
	{
		/* count the matches */
		match = mlen = 0;
		for (i = j = 0; (scan = cname(i)) != NULL; i++)
			if (!CHARncmp(cmdname, toCHAR(scan), len))
			{
				if (j == 0)
				{
					match = i;
					mlen = strlen(scan);
				}
				else
				{
					while (strncmp(scan, cname(match), mlen))
						mlen--;
				}
				j++;
			}

		/* don't need our copy of the partial name anymore */
		safefree(cmdname);

		/* if no matches, then do a literal tab */
		if (j == 0)
			return tab;

		/* If one match, then return the untyped portion of it followed
		 * by a space.  The user may want to back up over the space
		 * to append a ! but the space is still nice because it
		 * reassures the user that the name really is complete.
		 */
		found = cname(match);
		if (j == 1)
		{
			for (i = 0, j = len; found[j]; i++, j++)
				common[i] = found[j];
			common[i++] = ' ';
			common[i] = '\0';
			return common;
		}

		/* Else multiple matches.  Can we add to their common chars?
		 * If so, great!  If not, then list all matches.
		 */
		for (i = 0, j = len; j < mlen; j++, i++)
			common[i] = found[j];
		common[i] = '\0';
		if (mlen == len)
		{
			for (i = j = 0; (scan = cname(i)) != NULL; i++)
				if (!strncmp(found, scan, len))
				{
					mlen = strlen(cname(i));
					if (j + mlen + 1 >= o_columns(win))
					{
						drawextext(win, toCHAR("\n"), 1);
						j = 0;
					}
					else if (j != 0)
						drawextext(win, blanks, 1);
					drawextext(win,
						toCHAR(scan),
						mlen);
					j += mlen + 1;
				}
			if (j != 0)
				drawextext(win, toCHAR("\n"), 1);
		}
		return common;
	}

	/* if we get here, then we're completing the args of a command,
	 * instead of the command itself.
	 */

#ifdef FEATURE_ALIAS
	/* macros probably take filenames for args */
	if (exisalias(tochar8(cmdname), ElvFalse))
	{
		safefree(cmdname);
		return NULL;
	}
#endif

	/* look up the command */
	for (i = 0; i < QTY(cmdnames) && CHARncmp(cmdname, toCHAR(cmdnames[i].name), len); i++)
	{
	}
	safefree(cmdname);
	if (i >= QTY(cmdnames))
	{
		/* Unknown command.  This is probably either a typo (in which
		 * case the behavior of <Tab> isn't that critical) or it is
		 * really a shell command via the recursive excomplete() call,
		 * below.  In the latter case, filename completion is the
		 * best action, so we do that.
		 */
		return NULL;
	}

	/* the :set command completes option names */
	if (cmdnames[i].code == EX_SET)
		return optcomplete(win, to);

#ifdef FEATURE_TAGS
	/* The :tag and :stag commands complete tag names.  I'm tempted to
	 * add :browse and :sbrowse here, but that might be confusing.
	 */
	if (cmdnames[i].code == EX_TAG || cmdnames[i].code == EX_STAG)
		return tagcomplete(win, to);
#endif

	/* The :cc and :make commands take filename arguments. */
	if (cmdnames[i].code == EX_CC || cmdnames[i].code == EX_MAKE)
		return NULL; /* do filename expansion */

	/* The :color command takes some complex arguments. */
	if (cmdnames[i].code == EX_COLOR)
		return colorcomplete(win, marktmp(tmp, markbuffer(to), offset), to, ElvFalse);

#ifdef FEATURE_SPELL
	/* The :check command mostly uses font names. */
	if (cmdnames[i].code == EX_CHECK)
		return colorcomplete(win, marktmp(tmp, markbuffer(to), offset), to, ElvTrue);
#endif

#ifdef FEATURE_REGION
	/* The :region and :unregion commands mostly use font names */
	if (cmdnames[i].code == EX_REGION || cmdnames[i].code == EX_UNREGION)
		return colorcomplete(win, marktmp(tmp, markbuffer(to), offset), to, ElvTrue);
#endif

#ifdef FEATURE_AUTOCMD
	/* The :au and :doau commands take complex arguments.  We'll settle
	 * for event name completion though.
	 */
	if (cmdnames[i].code == EX_AUTOCMD || cmdnames[i].code == EX_DOAUTOCMD)
		return aucomplete(win, marktmp(tmp, markbuffer(to), offset), to);
#endif

	/* Many commands take ex commands as their arguments.  The a_Cmds
	 * flag is set for these commands, but it is also set for a few
	 * others.  Oh well, it's close enough.  Also, the :help command's
	 * arguments resemble an ex command.
	 */
	if ((cmdnames[i].flags & a_Cmds) != 0 || cmdnames[i].code == EX_HELP)
		return excomplete(win, marktmp(tmp, markbuffer(to), offset), to);

	/* does the command take filename arguments or a target address? */
	if ((cmdnames[i].flags & (a_Target | a_Filter | a_File | a_Files)) != 0)
	{
		if (filter)
			o_completebinary = ElvTrue;
		return NULL;/* do filename expansion */
	}

	/* if all else fails, just treat it literally */
	return tab;
}
#endif /* FEATURE_COMPLETE */
