/* vi.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_vi[] = "$Id: vi.c 167211 2008-04-24 17:26:56Z sboucher $";
#endif

#if USE_PROTOTYPES
static RESULT parse(_CHAR_ key, void *info);
static ELVCURSOR shape(WINDOW win);
# ifdef FEATURE_TEXTOBJ
static long indentation(WINDOW win, long line);
static RESULT vixmlobj(WINDOW win, VIINFO *vinf, MARKBUF *from, MARKBUF *to);
# endif
#endif

/* This is a list of operator commands.  Note that 'g' commands are denoted by
 * setting the 8th bit of the character, so \365 is gu, \325 is gU, \376 is g~,
 * and \275 is g=.
 */
#define OPERATORS "cdy<=>!\275\365\325\376"

/* This is a list of letters and control characters that can be used as the
 * 2nd key in a text object's name.  In addition to the characters listed here,
 * elvis always allows any punctuation character to be used -- as a field
 * delimiter if nothing else.
 */
#define TEXTOBJECTS "BLSWXblpstwx\n"

/* These variables store the command which <.> is supposed to repeat.
 * If the command is supposed to switch to input mode, then those
 * commands will be smart enough to behave differently when executed
 * for the <.> command.  They'll paste from the "Elvis previous input"
 * buffer.
 */
VIINFO	dotcmd;	/* the command itself, already parsed */
ELVBOOL	dotviz;	/* was the dotcmd used as a visible operator? */
long	dotlines;/* number of lines for visible operator, or 0L if char/rect */

/* These are the "when" conditions in the array below */
#define WHEN_SEL	0x0001	/* legal while text is selected */
#define WHEN_ONCE	0x0002	/* legal during ^O */
#define WHEN_OPEN	0x0004	/* legal in "open" mode */
#define WHEN_HIST	0x0008	/* legal in "history" buffers */
#define WHEN_MOVE	0x0010	/* legal as target of an operator */
#define WHEN_ANY	0x001f	/* all of the above */
#define WHEN_NEVER	0x0000	/* none of the above */
#define WHEN_MORE	0x0020	/* legal as part of a "more" invocation */
#define WHEN_EMPTY	0x0040	/* legal when buffer is empty */

#define WHEN_SEL_ONCE		(WHEN_SEL|WHEN_ONCE)
#define WHEN_ONCE_OPEN		(WHEN_ONCE|WHEN_OPEN)
#define WHEN_ONCE_OPEN_HIST	(WHEN_ONCE|WHEN_OPEN|WHEN_HIST)
#define WHEN_SEL_ONCE_OPEN_HIST	(WHEN_SEL|WHEN_ONCE|WHEN_OPEN|WHEN_HIST)
#define WHEN_SEL_ONCE_OPEN	(WHEN_SEL|WHEN_ONCE|WHEN_OPEN)
#define WHEN_SEL_ONCE_MOVE	(WHEN_SEL|WHEN_ONCE|WHEN_MOVE)
#define WHEN_SEL_ONCE_OPEN_MOVE	(WHEN_SEL|WHEN_ONCE|WHEN_OPEN|WHEN_MOVE)
#define WHEN_SEL_ONCE_HIST_MOVE	(WHEN_SEL|WHEN_ONCE|WHEN_HIST|WHEN_MOVE)
#define WHEN_OPEN_HIST		(WHEN_OPEN|WHEN_HIST)

/* This array describes each command */
static struct
{
	RESULT		(*func) P_((WINDOW win, VIINFO *vinf));
	unsigned short	when;		/* when command is legal */
	unsigned short	tweak;		/* misc features */
	char		*helptopic;	/* label in "elvisvi.html" */
}
vikeys[] =
{
/* NUL not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE,		"nul"	},
/* ^A  search for word	 */	{ m_search,	WHEN_ANY,			TWEAK_MARK			},
/* ^B  page backward	 */	{ m_scroll,	WHEN_SEL_ONCE,			TWEAK_FRONT			},
/* ^C  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* ^D  scroll dn 1/2page */	{ m_scroll,	WHEN_SEL_ONCE_OPEN,		TWEAK_FIXCOL			},
/* ^E  scroll up	 */	{ m_scroll,	WHEN_SEL_ONCE,			TWEAK_FIXCOL			},
/* ^F  page forward	 */	{ m_scroll,	WHEN_SEL_ONCE,			TWEAK_FRONT			},
/* ^G  show file status	 */	{ m_absolute,	WHEN_SEL_ONCE_OPEN|WHEN_EMPTY,	TWEAK_NONE			},
/* ^H  move left, like h */	{ m_left,	WHEN_ANY,			TWEAK_NONE			},
/* ^I  move to next tag  */	{ m_tag,	WHEN_ONCE_OPEN,			TWEAK_NONE			},
/* ^J  move down	 */	{ m_updown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE		},
/* ^K  find misspelled   */	{ m_spell,	WHEN_NEVER,			TWEAK_NONE			},
/* ^L  redraw screen	 */	{ v_expose,	WHEN_SEL_ONCE|WHEN_EMPTY,	TWEAK_NONE			},
/* ^M  mv front next ln  */	{ m_updown,	WHEN_ANY,			TWEAK_FRONT_INCL_LINE		},
/* ^N  move down	 */	{ m_updown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE		},
/* ^O  ignored    	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* ^P  move up		 */	{ m_updown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE		},
/* ^Q  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* ^R  redo change	 */	{ v_undo,	WHEN_ONCE_OPEN|WHEN_EMPTY,	TWEAK_NONE			},
/* ^S  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* ^T  pop tagstack	 */	{ v_tag,	WHEN_OPEN|WHEN_EMPTY,		TWEAK_NONE			},
/* ^U  scroll up 1/2page */	{ m_scroll,	WHEN_SEL_ONCE_OPEN,		TWEAK_FIXCOL			},
/* ^V  start visible	 */	{ v_visible,	WHEN_SEL_ONCE,			TWEAK_LINE			},
/* ^W  window operations */	{ v_window,	WHEN_SEL_ONCE_OPEN_HIST|WHEN_EMPTY,TWEAK_NONE			},
/* ^X  move to phys col	 */	{ m_column,	WHEN_SEL_ONCE_OPEN_HIST|WHEN_MOVE,TWEAK_IGNCOL_MARK		},
/* ^Y  scroll down	 */	{ m_scroll,	WHEN_SEL_ONCE,			TWEAK_FIXCOL			},
/* ^Z  stop process	 */	{ v_notex,	WHEN_ANY,			TWEAK_NONE			},
/* ESC end visible mark	 */	{ v_visible,	WHEN_SEL,			TWEAK_NONE,		"^obra"	},
/* ^\  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE,		"^bksl"	},
/* ^]  keyword is tag	 */	{ v_tag,	WHEN_ONCE_OPEN,			TWEAK_NONE,		"^cbra"	},
/* ^^  previous file	 */	{ v_notex,	WHEN_ONCE_OPEN|WHEN_EMPTY,	TWEAK_FRONT			},
/* ^_  move to row	 */	{ NULL,		WHEN_SEL_ONCE,			TWEAK_FIXCOL_INCL		},
/* SPC move right,like l */	{ m_right,	WHEN_ANY,			TWEAK_NONE,		"l"	},
/*  !  run thru filter	 */	{ NULL,		WHEN_NEVER,			TWEAK_FRONT_LINE_DOT_OPER_UNDO,	"bang"	},
/*  "  select cut buffer */	{ NULL,		0,/* This is handled by the command parser */0,		"quote"	},
/*  #  increment number	 */	{ v_number,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO,		"hash"	},
/*  $  move to rear	 */	{ m_column,	WHEN_ANY,			TWEAK_IGNCOL_INCL,	"dollar"},
/*  %  move to match	 */	{ m_absolute,	WHEN_ANY,			TWEAK_INCL,		"pct"	},
/*  &  repeat subst	 */	{ v_notex,	WHEN_NEVER,			TWEAK_FIXCOL_INCL_LINE_DOT_UNDO, "amp"},
/*  '  move to a mark	 */	{ m_mark,	WHEN_SEL_ONCE_OPEN_MOVE,	TWEAK_FRONT_INCL_MARK_LINE, "apost"},
/*  (  mv back sentence	 */	{ m_bsentence,	WHEN_ANY,			TWEAK_NONE,		"open"	},
/*  )  mv fwd sentence	 */	{ m_fsentence,	WHEN_ANY,			TWEAK_NONE,		"close"	},
/*  *  errlist		 */	{ v_notex,	WHEN_OPEN|WHEN_EMPTY,		TWEAK_FRONT,			},
/*  +  mv front next ln  */	{ m_updown,	WHEN_ANY,			TWEAK_FRONT_INCL_LINE,		},
/*  ,  reverse [fFtT] cmd*/	{ m_csearch,	WHEN_ANY,			TWEAK_INCL,		"comma"	},
/*  -  mv front prev ln	 */	{ m_updown,	WHEN_ANY,			TWEAK_FRONT_INCL_LINE		},
/*  .  special...	 */	{ NULL,		0,/* This is handled by the command parser */0,		"stop"	},
/*  /  forward search	 */	{ m_search,	WHEN_ANY|WHEN_MORE,		TWEAK_MARK,		"slash"	},
/*  0  part of count?	 */	{ m_column,	WHEN_ANY,			TWEAK_IGNCOL_MARK		},
/*  1  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  2  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  3  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  4  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  5  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  6  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  7  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  8  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  9  part of count	 */	{ NULL		/* This is handled by the command parser */			},
/*  :  run single EX cmd */	{ v_ex,		WHEN_SEL_ONCE_OPEN_HIST|WHEN_EMPTY, TWEAK_NONE,		"colon"	},
/*  ;  repeat [fFtT] cmd */	{ m_csearch,	WHEN_ANY,			TWEAK_INCL,		"semi"	},
/*  <  shift text left	 */	{ NULL,		WHEN_OPEN,			TWEAK_FRONT_LINE_DOT_OPER_UNDO,"lt"},
/*  =  preset filter	 */	{ NULL,		WHEN_OPEN,			TWEAK_DOT_OPER_UNDO		},
/*  >  shift text right	 */	{ NULL,		WHEN_OPEN,			TWEAK_FRONT_LINE_DOT_OPER_UNDO,"gt"},
/*  ?  backward search	 */	{ m_search,	WHEN_ANY|WHEN_MORE,		TWEAK_MARK,		"quest"	},
/*  @  execute a cutbuf  */	{ v_at,		WHEN_SEL_ONCE_OPEN_HIST|WHEN_EMPTY,TWEAK_NONE,		"at"	},
/*  A  append at EOL	 */	{ v_input,	WHEN_ONCE_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO	},
/*  B  move back Word	 */	{ m_bigword,	WHEN_ANY,			TWEAK_NONE			},
/*  C  change to EOL	 */	{ v_notop,	WHEN_OPEN,			TWEAK_DOT_UNDO			},
/*  D  delete to EOL	 */	{ v_notop,	WHEN_OPEN,			TWEAK_DOT_UNDO			},
/*  E  move end of Word	 */	{ m_bigword,	WHEN_ANY,			TWEAK_INCL			},
/*  F  move bk to char	 */	{ m_csearch,	WHEN_ANY,			TWEAK_INCL			},
/*  G  move to line #	 */	{ m_absolute,	WHEN_ANY,			TWEAK_FRONT_INCL_MARK_LINE	},
/*  H  move to row	 */	{ m_scrnrel,	WHEN_SEL_ONCE_MOVE,		TWEAK_FRONT_INCL_LINE		},
/*  I  insert at front	 */	{ v_input,	WHEN_ONCE_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO	},
/*  J  join lines	 */	{ v_notex,	WHEN_ONCE_OPEN,			TWEAK_DOT_UNDO			},
/*  K  look up keyword	 */	{ v_notex,	WHEN_OPEN,			TWEAK_NONE			},
/*  L  move to last row	 */	{ m_scrnrel,	WHEN_SEL_ONCE_MOVE,		TWEAK_FRONT_INCL_LINE		},
/*  M  move to mid row	 */	{ m_scrnrel,	WHEN_SEL_ONCE_MOVE,		TWEAK_FRONT_INCL_LINE		},
/*  N  reverse prev srch */	{ m_search,	WHEN_ANY,			TWEAK_MARK			},
/*  O  insert above line */	{ v_input,	WHEN_ONCE_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO	},
/*  P  paste before	 */	{ v_paste,	WHEN_ONCE_OPEN_HIST|WHEN_EMPTY,	TWEAK_DOT_UNDO			},
/*  Q  quit to EX mode	 */	{ v_ex,		WHEN_OPEN|WHEN_EMPTY,		TWEAK_NONE			},
/*  R  overtype		 */	{ v_input,	WHEN_ONCE_OPEN_HIST|WHEN_MORE,	TWEAK_DOT_UNDO			},
/*  S  change line	 */	{ v_notop,	WHEN_OPEN,			TWEAK_DOT_UNDO			},
/*  T  move bk to char	 */	{ m_csearch,	WHEN_ANY,			TWEAK_INCL			},
/*  U  undo whole line	 */	{ v_undo,	WHEN_OPEN_HIST|WHEN_EMPTY,	TWEAK_FRONT_UNDO		},
/*  V  start visible	 */	{ v_visible,	WHEN_SEL_ONCE_OPEN,		TWEAK_LINE			},
/*  W  move forward Word */	{ m_bigword,	WHEN_ANY,			TWEAK_NONE			},
/*  X  delete to left	 */	{ v_delchar,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO			},
/*  Y  yank text	 */	{ v_notop,	WHEN_OPEN,			TWEAK_NONE			},
/*  Z  save file & exit	 */	{ v_quit,	WHEN_ONCE_OPEN|WHEN_EMPTY,	TWEAK_NONE			},
/*  [  move back section */	{ m_bsection,	WHEN_ANY,			TWEAK_MARK,		"obra"	},
/*  \  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE,		"bksl"	},
/*  ]  move fwd section  */	{ m_fsection,	WHEN_ANY,			TWEAK_MARK,		"cbra"	},
/*  ^  move to front	 */	{ m_front,	WHEN_ANY,			TWEAK_NONE			},
/*  _  current line	 */	{ m_updown,	WHEN_ANY,			TWEAK_FRONT_INCL_LINE		},
/*  `  move to mark	 */	{ m_mark,	WHEN_SEL_ONCE_OPEN_MOVE,	TWEAK_MARK,		"grave"	},
/*  a  append at cursor	 */	{ v_input,	WHEN_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO		},
/*  b  move back word	 */	{ m_word,	WHEN_ANY,			TWEAK_NONE			},
/*  c  change text	 */	{ NULL,		WHEN_OPEN,			TWEAK_DOT_OPER_UNDO		},
/*  d  delete op	 */	{ NULL,		WHEN_OPEN,			TWEAK_DOT_OPER_UNDO		},
/*  e  move end word	 */	{ m_word,	WHEN_ANY,			TWEAK_INCL			},
/*  f  move fwd for char */	{ m_csearch,	WHEN_ANY,			TWEAK_INCL			},
/*  g  command prefix	 */	{ NULL		/* This is handled by the command parser */			},
/*  h  move left	 */	{ m_left,	WHEN_ANY,			TWEAK_NONE			},
/*  i  insert at cursor	 */	{ v_input,	WHEN_ONCE_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO|TWEAK_IGNCOL	},
/*  j  move down	 */	{ m_updown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE		},
/*  k  move up		 */	{ m_updown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE		},
/*  l  move right	 */	{ m_right,	WHEN_ANY,			TWEAK_NONE			},
/*  m  define a mark	 */	{ v_setmark,	WHEN_ONCE_OPEN|WHEN_OPEN,	TWEAK_NONE			},
/*  n  repeat prev srch	 */	{ m_search,	WHEN_ANY,			TWEAK_MARK			},
/*  o  insert below line */	{ v_input,	WHEN_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO		},
/*  p  paste after	 */	{ v_paste,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO			},
/*  q  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/*  r  replace chars	 */	{ v_delchar,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO			},
/*  s  subst N chars	 */	{ v_notop,	WHEN_OPEN,			TWEAK_DOT_UNDO			},
/*  t  move fwd to char	 */	{ m_csearch,	WHEN_ANY,			TWEAK_INCL			},
/*  u  undo		 */	{ v_undo,	WHEN_ONCE_OPEN|WHEN_EMPTY,	TWEAK_NONE			},
/*  v  start visible	 */	{ v_visible,	WHEN_SEL_ONCE_OPEN,		TWEAK_NONE			},
/*  w  move fwd word	 */	{ m_word,	WHEN_ANY,			TWEAK_NONE			},
/*  x  delete character	 */	{ v_delchar,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO			},
/*  y  yank text	 */	{ NULL,		WHEN_OPEN,			TWEAK_OPER			},
/*  z  adjust scrn row	 */	{ m_z,		WHEN_SEL_ONCE,			TWEAK_FIXCOL			},
/*  {  back paragraph	 */	{ m_bsection,	WHEN_ANY,			TWEAK_NONE,		"ocur"	},
/*  |  move to column	 */	{ m_column,	WHEN_ANY,			TWEAK_IGNCOL_MARK,	"bar"	},
/*  }  fwd paragraph	 */	{ m_fsection,	WHEN_ANY,			TWEAK_NONE,		"ccur"	},
/*  ~  upper/lowercase	 */	{ v_delchar,	WHEN_ONCE_OPEN_HIST,		TWEAK_DOT_UNDO			},
/* DEL not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/****************************************************************************************************************/
/* gNUL not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^A  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^B  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^C  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^D  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^E  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^F  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^G  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^H  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^I  move to prev tag */	{ m_tag,	WHEN_ONCE_OPEN,			TWEAK_NONE			},
/* g^J  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^K  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^L  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^M  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^N  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^O  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^P  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^Q  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^R  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^S  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^T  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^U  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^V opposite sel end  */	{ v_visible,	WHEN_SEL,			TWEAK_FIXCOL			},
/* g^W  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^X  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^Y  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^Z  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gESC not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^\  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^]  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^^  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^_  not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gSPC not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g!   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g"   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g#   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g$  move to row rear	 */	{ m_g,		WHEN_ANY,			TWEAK_IGNCOL_INCL		},
/* g%  opposite sel end  */	{ v_visible,	WHEN_SEL,			TWEAK_FIXCOL			},
/* g&   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g'   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g(   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g)   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g*   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g+   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g,   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g-   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g.   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g/   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g0   not defined	 */	{ m_g,		WHEN_ANY,			TWEAK_IGNCOL_MARK		},
/* g1   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g2   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g3   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g4   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g5   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g6   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g7   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g8   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g9   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g:   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g;   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g<   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g=  replace char op	 */	{ NULL,		WHEN_OPEN,			TWEAK_DOT_OPER_UNDO		},
/* g>   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g?   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g@   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gA   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gB   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gC   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gD  find global def	 */	{ m_search,	WHEN_ANY,			TWEAK_MARK			},
/* gE   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gF   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gG   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gH   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gI  insert at front	 */	{ v_input,	WHEN_ONCE_OPEN_HIST|WHEN_EMPTY|WHEN_MORE, TWEAK_DOT_UNDO	},
/* gJ  join lines	 */	{ v_notex,	WHEN_ONCE_OPEN,			TWEAK_DOT_UNDO			},
/* gK   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gL   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gM   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gN   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gO   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gP   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gQ   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gR   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gS   not defined	 */	{ m_endspell,	WHEN_ANY,			TWEAK_INCL			},
/* gT   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gU  make uppercase op */	{ NULL,		WHEN_OPEN,			TWEAK_DOT_OPER_UNDO		},
/* gV   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gW   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gX   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gY   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gZ   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g[   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g\   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g]   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g^  move to row front */	{ m_g,		WHEN_ANY,			TWEAK_NONE			},
/* g_   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g`   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* ga   not defined	 */	{ v_ascii,	WHEN_ONCE_OPEN,			TWEAK_NONE			},
/* gb   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gc   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gd  find local def	 */	{ m_search,	WHEN_ANY,			TWEAK_MARK			},
/* ge   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gf   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gg   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gh   not defined	 */	{ m_g,		WHEN_ANY,			TWEAK_NONE			},
/* gi   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gj   not defined	 */	{ m_gupdown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE		},
/* gk   not defined	 */	{ m_gupdown,	WHEN_ANY,			TWEAK_IGNCOL_INCL_LINE		},
/* gl   not defined	 */	{ m_g,		WHEN_ANY,			TWEAK_NONE			},
/* gm   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gn   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* go   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gp   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gq   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gr   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gs  find misspelled	 */	{ m_spell,	WHEN_NEVER,			TWEAK_NONE			},
/* gt   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gu  make lowercase op */	{ NULL,		WHEN_OPEN,			TWEAK_DOT_OPER_UNDO		},
/* gv   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gw   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gx   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gy   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* gz   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g{   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g|   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g}   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g~   not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			},
/* g~  toggle case op	 */	{ NULL,		WHEN_OPEN,			TWEAK_DOT_OPER_UNDO		},
/* gDEL not defined	 */	{ NULL,		WHEN_NEVER,			TWEAK_NONE			}
};


#ifdef FEATURE_TEXTOBJ
/* Return the indentation width (number of columns, after expanding tabs) for
 * a given line number.  For empty lines, return -1.  This is used for the 'L'
 * indentation blocks.
 */
static long indentation(win, line)
	WINDOW	win;	/* buffer */
	long	line;	/* line number */
{
	long	width;
	BUFFER	buf = markbuffer(win->cursor);
	MARKBUF	m;
	CHAR	*cp;

	/* start counting from the beginning of the line */
	scanalloc(&cp, marktmp(m, buf, lowline(bufbufinfo(buf), line)));
	width = 0;

	/* count the spaces and tabs */
	while (*cp && *cp != '\n' && elvspace(*cp))
	{
		if (*cp == ' ')
			width++;
		else if (*cp == '\t')
			width += opt_totab(o_tabstop(buf), width);
		scannext(&cp);
	}

	/* if empty line, then width = -1 */
	if (!cp || *cp == '\n'
#ifdef DISPLAY_SYNTAX
			       || *cp == dmspreprocessor(win)
#endif
							     )
		width = -1;

	/* clean up, and return the width */
	scanfree(&cp);
	return width;
}

/* Find the endpoints for an XML object, delimited by <tag>...</tag> pairs.
 * This function should only be called from the vitextobj() function.
 */
static RESULT vixmlobj(win, vinf, from, to)
	WINDOW	win;	/* window where the command is to be run */
	VIINFO	*vinf;	/* identifies the type of text object */
	MARKBUF *from;	/* output: move this mark to top of affected text */
	MARKBUF	*to;	/* output: move this mark to bottom of affected text */
{
	CHAR	*scan, *intag;
	CHAR	prev;
	CHAR	*name = NULL, *inname;
	int	nest;
	ELVBOOL	slash;
	MARKBUF	middle;
	static struct xmlstack_s
	{
		struct xmlstack_s *next;
		CHAR	*name;
	}	*stack = NULL, *st;

	DEFAULT(1);

	/* start scanning at cursor */
	scanalloc(&scan, win->cursor);

	/* Adjust the scanning point.  If in a </tag> then we want to start
	 * at its "<".  If in a <tag> then we want to start after the "<".
	 * Otherwise it doesn't matter much, so go back to the cursor position.
	 */
	while (scan && *scan != '<' && scanprev(&scan) && *scan != '>')
	{
	}
	if (!scan || *scan != '<')
		scanseek(&scan, win->cursor);
	else if (scannext(&scan) && *scan == '/')
		scanprev(&scan);

	/* scan forward for the count'th unmatched "</".  This is tricky!
	 * Not all XML tags come in pairs, so can't simply count tags with
	 * and without slashes.  We must push <tag> names onto a stack, and
	 * pop them when we get a </tag>.  To handle unpaired tags, </tag>
	 * should always pop back to the matching name; or, if there is no
	 * name, then the match must be before the cursor so WE'RE IN THAT
	 * TAG PAIR!  THAT'S ONE OF THE TAGS WE SHOULD COUNT!
	 */
	for (prev = *scan, nest = vinf->count; nest > 0; prev = *scan)
	{
		/* we scan pairs of characters -- one in "prev" and one at
		 * "*scan".  We already have "prev", but now we need to
		 * increment the scan pointer.
		 */
		if (!scannext(&scan))
			goto Fail;

		/* Is this the start of a tag? */
		if (prev == '<')
		{
			/* Remember the location of the "<".  If this turns out
			 * to be the ending tag for an ix object, we'll need it.
			 */
			*to = *scanmark(&scan);
			to->offset--;
			middle = *to;

			/* detect a slash */
			slash = (ELVBOOL)(*scan == '/');
			if (slash)
				if (!scannext(&scan)) goto Fail ;

			/* collect chars of the name */
			assert(name == NULL);
			for (; scan && elvalnum(*scan); scannext(&scan))
				buildCHAR(&name, vinf->key2 == 'X' ? elvtolower(*scan) : *scan);
			if (!scan || !name)
				goto Fail;

			/* if slash, pop to match.  If no match decrement nest*/
			if (slash)
			{
				/* pop any tags that don't nest properly */
				while (stack && CHARcmp(stack->name, name))
				{
					st = stack;
					stack = st->next;
					safefree(st->name);
					safefree(st);
				}

				/* did we find a match? */
				if (stack)
				{
					/* pop the match too. */
					st = stack;
					stack = st->next;
					safefree(st->name);
					safefree(st);

					/* free the name */
					safefree(name);
					name = NULL;
				}
				else /* must extend back past the cursor */
				{
					nest--;

					/* free the name, unless this is
					 * the tag we want.
					 */
					if (nest > 0)
					{
						safefree(name);
						name = NULL;
					}
				}
			}
			else /* push onto stack */
			{
				st = (struct xmlstack_s *)safealloc(1, sizeof *st);
				st->name = name;
				st->next = stack;
				stack = st;
				name = NULL;
			}
		}
	}

	/* if "ax" object, remember where it ends to help find the right end */
	if (vinf->command == 'a')
		*to = *scanmark(&scan);

	/* Scan backward for the starting tag, skipping over nested pairs.
	 * This is easier than the search for the ending tag, because now we
	 * only need to worry about nested pairs of THAT PARTICULAR TAG NAME.
	 */
	scanseek(&scan, &middle);
	for (nest = 1; scan && nest > 0; )
	{
		if (!scanprev(&scan))
			goto Fail;
		if (*scan != '<')
			continue;

		/* we're at the start of a tag.  See if it's a starter or not */
		scandup(&intag, &scan);
		scannext(&intag);
		slash = (ELVBOOL)(*intag == '/');
		if (slash)
			scannext(&intag);

		/* does the name match? */
		inname = name;
		if (vinf->key2 == 'X')
			while (*inname && elvalnum(*intag) && *inname++ == elvtolower(*intag))
				scannext(&intag);
		else
			while (*inname && elvalnum(*intag) && *inname++ == *intag)
				scannext(&intag);
		if (!*inname && (!intag || !elvalnum(*intag)))
		{
			/* yes, the name matches -- adjust nest */
			if (slash)
				nest++;
			else
				nest--;
		}
		scanfree(&intag);
	}

	/* now we can compute the endpoints */
	if (vinf->command == 'i')
	{
		/* skip to the end of the starting tag */
		while (*scan != '>')
			if (!scannext(&scan)) goto Fail;

		/* this is probably the start of the inner tag */
		*from = *scanmark(&scan);
		from->offset++;

		/* skip any spaces and tabs */
		do
		{
			if (!scannext(&scan)) goto Fail;
		} while (*scan == ' ' || *scan == '\t');

		/* if we hit a newline, then starting point is after newline */
		if (*scan == '\n')
		{
			scannext(&scan);
			*from = *scanmark(&scan);
		}

		/* Go back to the "<" of the ending tag.  This is probably the
		 * end of the inner tag.
		 */
		scanseek(&scan, to);

		/* skip backward past any spaces and tabs */
		while (scanprev(&scan) && (*scan == ' ' || *scan == '\t'))
		{
		}

		/* if we hit a newline, then that's the real end of the object*/
		if (scan && *scan == '\n')
		{
			*to = *scanmark(&scan);
			to->offset++;
		}
	}
	else /* vinf->command == 'a' */
	{
		/* The "<" of the start tag is the start of the object */
		*from = *scanmark(&scan);

		/* go back to the ending tag */
		scanseek(&scan, to);

		/* scan forward for the ">" at the end of that tag */
		while (*scan != '>')
			if (!scannext(&scan)) goto Fail;

		/* This is the end of the object */
		*to = *scanmark(&scan);
		to->offset++;
	}

	/* if the endpoints would be the same, then fail */
	if (from->offset >= to->offset)
		goto Fail;

	/* success! */
	safefree(name);
	scanfree(&scan);
	return RESULT_COMPLETE;

Fail:
	if (name)
		safefree(name);
	scanfree(&scan);
	while (stack)
	{
		st = stack;
		stack = st->next;
		safefree(st->name);
		safefree(st);
	}
	return RESULT_ERROR;
}

/* Find the endpoints of a text object.  The "from" and "to" parameters should
 * pointers to MARKBUF structures -- not dynamically allocated MARKs!  Returns
 * RESULT_COMPLETE normally, RESULT_MORE if only the "from" end could be found,
 * or RESULT_ERROR if neither end could be found or the text object is not
 * understood.
 */
RESULT vitextobj(win, vinf, from, to)
	WINDOW	win;	/* window where the command is to be run */
	VIINFO	*vinf;	/* identifies the type of text object */
	MARKBUF *from;	/* output: move this mark to top of affected text */
	MARKBUF	*to;	/* output: move this mark to bottom of affected text */
{
	CHAR	*oldmatchchar;
	RESULT	result = RESULT_COMPLETE;
	BUFFER	buf;
	CHAR	*scan;
	long	count;
	VIINFO	cmd;	/* simulated command, to find characters */
	CHAR	fromkey;/* keystroke used to find top of object */
	CHAR	tokey;	/* keystroke used to find top of next object */
	CHAR	opposite;/* for block delimiters, this is the other delimiter */
	CHAR	prev;	/* previously scanned character */
	MARKBUF	tmp;
	long	cursoff;
	long	ln, fromln, in, nextin;

	/* if the buffer is empty, then there can't be any text object */
	buf = markbuffer(win->cursor);
	if (o_buflines(buf) == 0L)
		return RESULT_ERROR;

	/* temporarily change matchchar to a hardcoded value so we can handle
	 * blocks in a consistent way.
	 */
	oldmatchchar = o_matchchar;
	o_matchchar = toCHAR("{}[]()<>");

	/* change b, B, or l to the equivelent block or field delimiter */
	if (vinf->key2 == 'b')
		vinf->key2 = '(';
	else if (vinf->key2 == 'B')
		vinf->key2 = '{';
	else if (vinf->key2 == 'l')
		vinf->key2 = '\n';
	else if (vinf->key2 == 't')
		vinf->key2 = '\t';

	/* if block delimiter, then find its opposite */
	for (scan = o_matchchar, opposite = '\0'; !opposite && *scan; scan += 2)
		if (scan[0] == vinf->key2)
			opposite = scan[1];
		else if (scan[1] == vinf->key2)
			opposite = scan[0];

	/* for now, assume we won't need fromkey/tokey */
	fromkey = tokey = '\0';

	/* "from" and "to" buffers are in the window's buffer */
	from->buffer = to->buffer = buf;

	/* the default count is always 1, for all types of text object */
	DEFAULT(1);

	/* handle each type of object separately */
	switch (vinf->key2)
	{

	  case '{':
	  case '[':
	  case '(':
	  case '<':
	  case '%':
		/* search backward for nth character */
		scanalloc(&scan, win->cursor);
		if (vinf->key2 != '%')
		{
			for (count = vinf->count, prev = ' '; ; )
			{
				if (*scan == vinf->key2)
					count--;
				else if (*scan == '\n' && prev == vinf->key2)
					break;
				prev = *scan;
				if (count == 0 || !scanprev(&scan))
					break;
				if (*scan == opposite)
					count++;
			}
		}
		else /* % */
		{
			for (count = vinf->count, prev = ' '; ; )
			{
				if (*scan=='(' || *scan=='{' || *scan=='[')
					count--;
				else if (*scan == '\n' &&
					(prev=='(' || prev=='{' || prev=='['))
					break;
				prev = *scan;
				if (count == 0 || !scanprev(&scan))
					break;
				if (*scan==')' || *scan=='}' || *scan==']')
					count++;
			}
		}
		if (count > 0L)
		{
			/* oops, there was no such character */
			result = RESULT_ERROR;
			scanfree(&scan);
			goto Error;
		}

		/* This is the top of the object */
		from->offset = markoffset(scanmark(&scan));
		scanfree(&scan);
		cursoff = markoffset(win->cursor);
		marksetoffset(win->cursor, from->offset);

		/* find the matching character by simulating a % command */
		cmd.command = '%';
		cmd.count = 0L;
		result = m_absolute(win, &cmd);
		if (result != RESULT_COMPLETE)
		{
			result = RESULT_MORE;
			marksetoffset(win->cursor, cursoff);
			goto Error;
		}

		/* the cursor is now at the bottom of the object */
		to->offset = markoffset(win->cursor);

		/* include delimiter chars if 'a', exclude if 'i' */
		if (vinf->command == 'a')
			to->offset++;
		else
		{
			markaddoffset(from, 1L);
			if (scanchar(from) == '\n')
				markaddoffset(from, 1L);
			do
			{
				markaddoffset(to, -1L);
			} while (elvspace(scanchar(to)));
			markaddoffset(to, 1L);

		}
		break;

	  case '}':
	  case ']':
	  case ')':
	  case '>':
		/* search forward for nth character */
		scanalloc(&scan, win->cursor);
		scanprev(&scan);
		for (count = vinf->count; count > 0L && scannext(&scan); )
			if (*scan == vinf->key2)
				count--;
			else if (*scan == opposite)
				count++;
		if (count > 0L)
		{
			/* oops, there was no such character */
			result = RESULT_ERROR;
			scanfree(&scan);
			goto Error;
		}

		/* This is the bottom of the object */
		to->offset = markoffset(scanmark(&scan));
		scanfree(&scan);
		marksetoffset(win->cursor, to->offset);

		/* find the matching character by simulating a % command */
		cmd.command = '%';
		cmd.count = 0L;
		result = m_absolute(win, &cmd);
		if (result != RESULT_COMPLETE)
			goto Error;

		/* the cursor is now at the bottom of the object */
		from->offset = markoffset(win->cursor);

		/* include delimiter chars if 'a', exclude if 'i' */
		if (vinf->command == 'a')
			to->offset++;
		else
		{
			markaddoffset(from, 1L);
			if (scanchar(from) == '\n')
				markaddoffset(from, 1L);
			do
			{
				markaddoffset(to, -1L);
			} while (elvspace(scanchar(to)));
			markaddoffset(to, 1L);

		}
		break;

	  case 'L':
		fromln = markline(win->cursor);
		in = indentation(win, fromln);
		count = vinf->count;

		/* if this line is blank, then scan forward and backward for
		 * the nearest nonblank lines.  Use the one with more
		 * indentation.
		 */
		if (in < 0L)
		{
			/* scan backward for a non-blank line */
			for (ln = fromln, nextin = -1L; nextin < 0 && --ln > 0;)
				nextin = indentation(win, ln);

			/* scan forward for a non-blank line */
			for (; in < 0 && ++fromln < o_buflines(buf); )
				in = indentation(win, fromln);

			/* choose the one with more indentation.  If the entire
			 * file consists of blank lines, then this will leave
			 * fromln=0, in=-1L, which will cause the next loop to
			 * detect a failure condition.
			 */
			if (nextin > in)
				fromln = ln, in = nextin;
		}

		/* search backward for n'th line with a smaller indent */
		for (ln = fromln - 1; ln >= 1 && in > 0 && count > 0; ln--)
		{
			nextin = indentation(win, ln);
			if (nextin >= 0 && nextin < in)
			{
				in = nextin;
				count--;
				if (in == 0 || count == 0)
					break;
			}
		}
		if (count > 1 || (count == 1 && in == 0))
		{
			result = RESULT_ERROR;
			goto Error;
		}

		/* we went one line too far -- range starts one line down */
		marksetline(from, ln + 1);

		/* search forward past lines with more indentation */
		for (ln = fromln + 1; ln < o_buflines(buf); ln++)
		{
			nextin = indentation(win, ln);
			if (nextin >= 0 && nextin <= in)
				break;
		}

		/* ln is now either past the end of the buffer, or on the line
		 * after the end of the indentation block.  Either way, that's
		 * the end of the block.
		 */
		marksetline(to, ln);

		/* the 'i' version leaves off the whitespace */
		if (vinf->command == 'i')
		{
			/* trim the leading space */
			for (scanalloc(&scan, from);
			     *scan && elvspace(*scan);
			     scannext(&scan))
			{
			}
			if (scan)
				marksetoffset(from, markoffset(scanmark(&scan)));
			scanfree(&scan);

			/* trim the trailing space */
			markaddoffset(to, -1L);
			for (scanalloc(&scan, to);
			     *scan && elvspace(*scan);
			     scanprev(&scan))
			{
			}
			if (scan)
				marksetoffset(to, markoffset(scanmark(&scan)) + 1);
			scanfree(&scan);
		}
		break;

	  case 'w':
		fromkey = 'b';
		tokey = 'w';
		break;

	  case 'W':
		fromkey = 'B';
		tokey = 'W';
		break;

	  case 's':
		fromkey = '(';
		tokey = ')';
		break;

	  case 'S':
		fromkey = '[';
		tokey = ']';
		break;

	  case 'p':
		fromkey = '{';
		tokey = '}';
		break;

	  case 'x':
	  case 'X':
		o_matchchar = oldmatchchar;
		return vixmlobj(win, vinf, from, to);

	  default:
		if (elvalnum(vinf->key2))
		{
			/* bogus letter */
			result = RESULT_ERROR;
			goto Error;
		}

		/* Other characters can be used as delimiters though... */

		/* search backward for character */
		scanalloc(&scan, win->cursor);
		for (count = vinf->count; count && scanprev(&scan); )
		{
			if (*scan == vinf->key2 || *scan == '\n')
				count--;
		}
		if (!scan && count > 1)
		{
			/* oops, there was no such character */
			result = RESULT_ERROR;
			scanfree(&scan);
			goto Error;
		}

		/* This is the top of the object */
		if (scan)
		{
			opposite = *scan;
			marksetoffset(from, markoffset(scanmark(&scan)));
		}
		else
		{
			marksetoffset(from, 0L);
			scanseek(&scan, from);
			opposite = '\0';
		}

		/* search forward the matching character */
		for (count = vinf->count * 2 - 1; count > 0 && scannext(&scan);)
			if (*scan == vinf->key2 || *scan == '\n')
				count--;
		if (!scan)
		{
			/* oops, there was no such character */
			result = RESULT_ERROR;
			scanfree(&scan);
			goto Error;
		}

		/* this is the bottom of the object */
		to->offset = markoffset(scanmark(&scan));

		/* 'a' includes the trailing delimiter, unless there was no
		 * trailing delimiter in which case it includes the leading
		 * delimiter.  'i' excludes both delimiters, and also leading
		 * whitespace.
		 */
		if (vinf->command == 'a')
		{
			if (*scan == vinf->key2)
			{
				to->offset++;
				if (opposite)
					from->offset++;
			}
			else if (opposite && opposite != vinf->key2)
				from->offset++;
		}
		else
		{
			if (opposite)
				from->offset++;
			scanseek(&scan, from);
			while (elvspace(*scan) && scannext(&scan))
			{
			}
			if (scan && markoffset(scanmark(&scan)) < markoffset(to))
				marksetoffset(from, markoffset(scanmark(&scan)));
		}
		scanfree(&scan);

		/* If ends are on different lines, and delimiter isn't '\n',
		 * then treat that as an error.
		 */
		if (vinf->key2 != '\n' && markline(from) != markline(to))
		{
			result = RESULT_ERROR;
			goto Error;
		}
	}

	/* maybe use a simulated command to find the top of the object */
	if (fromkey && tokey)
	{
		/* To find the start of the object, add 1 (unless already at
		 * end of buffer) and then simulate a `fromkey' command.
		 */
		if (markoffset(win->cursor) + 1L < o_bufchars(buf))
			markaddoffset(win->cursor, 1L);
		cmd.command = cmd.key2 = fromkey;
		cmd.count = 1L;
		result = (*vikeys[cmd.command].func)(win, &cmd);
		if (result != RESULT_COMPLETE)
			goto Error;
		from->offset = markoffset(win->cursor);

		/* To find the end of the object, simulate a `tokey' command. */
		cmd.command = cmd.key2 = tokey;
		cmd.count = vinf->count;
		result = (*vikeys[cmd.command].func)(win, &cmd);
		if (result != RESULT_COMPLETE)
			goto Error;
		to->offset = markoffset(win->cursor);

		/* adjust whitespace */
		if (vinf->command == 'a')
		{
			/* We want to include whitespace.  If there is no
			 * trailing whitespace, then try to use leading.
			 */
			tmp = *to;
			tmp.offset--;
			if (!elvspace(scanchar(&tmp)))
			{
				tmp.offset = from->offset - 1;
				while (tmp.offset > 0 && elvspace(scanchar(&tmp)))
					tmp.offset--;
				from->offset = tmp.offset + 1;
			}
		}
		else
		{
			/* if leading/trailing whitespace, then exclude it */
			tmp = *to;
			do
				tmp.offset--;
			while (tmp.offset > from->offset && elvspace(scanchar(&tmp)));
			to->offset = tmp.offset + 1;
			while (elvspace(scanchar(from)))
				from->offset++;
		}
	}

	/* text objects never include the last *CHARACTER*, but they always
	 * include the last *LINE*.
	 */
	if (vinf->tweak & TWEAK_LINE)
		vinf->tweak |= TWEAK_INCL;
	else
		vinf->tweak &= ~TWEAK_INCL;

Error:
	/* restore the matchchar option */
	o_matchchar = oldmatchchar;

	assert(result != RESULT_COMPLETE || from->offset < to->offset);
	return result;
}
#endif /* FEATURE_TEXTOBJ */

/* This initializes the "info" field to represent the start of a new command */
void viinitcmd(info)
	VIINFO	*info;	/* vi command to be blanked */
{
	info->phase = VI_START;
	info->count = info->count2 = 0;
	info->cutbuf = info->oper = info->command = info->key2 = '\0';
}


/* This performs a single vi command for a given window.  The vi command
 * is stored in the current state.
 */
RESULT _viperform(win)
	WINDOW	win;	/* window whose command is to be executed */
{
	return viperform(win, (VIINFO *)win->state->info);
}

/* This performs a single vi command, and returns RESULT_COMPLETE if
 * successful, RESULT_ERROR if not error, or RESULT_MORE if it has pushed
 * a new state and can't be resolved until that mode is popped.
 */
RESULT	viperform(win, vinf)
	WINDOW	win;	/* window where the command should be performed */
	VIINFO	*vinf;	/* the command itself */
{
	STATE	*state = win->state;
	RESULT	result = RESULT_COMPLETE;
	MARKBUF	from, to;
	unsigned short flags;
	long	tmp;

	/* Remember the cursor position */
	bufwilldo(win->cursor, ElvFalse);

	/* If a vi command runs an ex command, and that ex command generates
	 * an error message, then we don't want it to look like a script.
	 */
	msgscriptline(NULL, EX_BUF);

	/* The next map will be in "command" context, unless we alter the state
	 * stack.  Might as well set the mapflags now.
	 */
	win->state->mapflags = MAP_COMMAND;

	/* if the command is '.' then recall the previous repeatable command */
	vinf->tweak = vikeys[vinf->command].tweak;
	if (vinf->command == '.')
	{
		/* change the count, if a new count was supplied */
		if (vinf->count)
		{
			dotcmd.count = vinf->count;
		}

		/* recall the command */
		*vinf = dotcmd;

		/* If it was a visible operator the first time, then it must
		 * be visible now too.  Exception: if the earlier command was
		 * applied to whole lines, then reapply it now to the same
		 * quantity of whole lines.
		 */
		if (dotviz && !win->seltop)
		{
			if (dotlines)
			{
				vinf->count = dotlines;
			}
			else
			{
				return RESULT_ERROR;
			}
		}
	}

	/* see if the command is legal in this context */
	flags = vikeys[vinf->command].when;
	if ((win->seltop && !(flags & WHEN_SEL)
#ifdef FEATURE_TEXTOBJ
	 			&& vinf->command != 'i' && vinf->command != 'a'
#endif
				)
	 || ((state->flags & (ELVIS_ONCE | ELVIS_POP)) && !(flags & WHEN_ONCE))
	 || ((state->flags & ELVIS_BOTTOM) && !(flags & WHEN_OPEN))
	 || (state->acton && !(flags & WHEN_HIST))
	 || (vinf->oper && !(flags & WHEN_MOVE)
#ifdef FEATURE_TEXTOBJ
	 			&& vinf->command != 'i' && vinf->command != 'a'
#endif
				)
	 || (o_locked(markbuffer(state->cursor))
	 	&& ((vikeys[vinf->command].tweak & TWEAK_UNDO) || (vinf->oper && vinf->oper != 'y')))
	 || vikeys[vinf->command].func == NULL)
	{
		viinitcmd(vinf);
		return RESULT_ERROR;
	}

	/* if the buffer is empty, and this command doesn't work on an empty
	 * buffer, then stuff a newline into the buffer.
	 */
	if (o_bufchars(markbuffer(state->cursor)) == 0 && !(flags & WHEN_EMPTY))
	{
		bufreplace(marktmp(from, markbuffer(state->cursor), 0L), &from, toCHAR("\n"), 1L);
		o_modified(markbuffer(state->cursor)) = ElvFalse;
	}

	/* If we're using an operator, then combine the operator's flags
	 * with the movement command's flags.
	 */
	if (vinf->oper)
	{
		vinf->tweak |= vikeys[vinf->oper].tweak;
	}

	/* If the command doesn't accept two separate counts, and we got two,
	 * then multiply them and store the result as the count.
	 */
	if ((vinf->tweak & TWEAK_TWONUM) == 0 && vinf->count2 != 0)
	{
		if (vinf->count)
		{
			vinf->count *= vinf->count2;
		}
		else
		{
			vinf->count = vinf->count2;
		}
	}

	/* if TWEAK_UNDO, then set the buffer's "willdo" flag.  This will cause
	 * an "undo" version of the buffer to be created if the command does
	 * indeed modify the buffer.  Exception: If this is the second phase
	 * of a complex command, then we don't want to set the flag because
	 * we already set it in the first phase.
	 */
	if ((vinf->tweak & TWEAK_UNDO) != 0 && (win->state->flags & ELVIS_MORE) == 0)
	{
		bufwilldo(state->cursor, ElvTrue);
	}

#ifdef FEATURE_TEXTOBJ
	/* if text object used in during visual selection, adjust endpoints */
	if (win->seltop && (vinf->command == 'a' || vinf->command == 'i'))
	{
		/* Find the object's endpoints */
		if (vitextobj(win, vinf, &from, &to) != RESULT_COMPLETE)
		{
			viinitcmd(vinf);
			return RESULT_ERROR;
		}

		/* Set the highlighted region's endpoints to the object's
		 * endpoints.  Leave the cursor at the bottom of the text
		 * object.  Note that we must subtract 1 from the endpoint
		 * since 'v' includes the cursor but text objects assume the
		 * end is excluded.
		 */
		assert(from.buffer == markbuffer(win->seltop));
		marksetoffset(win->seltop, from.offset);
		marksetoffset(win->selbottom, to.offset - 1L);
		marksetoffset(win->cursor, to.offset - 1L);
		win->selattop = ElvFalse;
		win->wantcol = (*win->md->mark2col)(win, win->cursor, ElvTrue);

		/* The end at the cursor will be adjusted for line/rectangle
		 * modes automatically, elsewhere.  But the other end isn't
		 * expected to move, so it isn't adjusted automatically; we
		 * must adjust it here explicitly.
		 */
		switch (win->seltype)
		{
		  case 'l': /* line mode */
			marksetoffset(win->seltop, markoffset((*win->md->move)(win, win->seltop, 0L, 0, ElvTrue)));
			break;

		  case 'r': /* rectangle mode */
			win->selleft = (*win->md->mark2col)(win, win->seltop, ElvTrue);
			if (win->selleft < win->wantcol)
			{
				win->selright = win->wantcol;
				win->selorigcol = win->selleft;
			}
			else
			{
				win->selright = win->selorigcol = win->selleft;
				win->selleft = win->wantcol;
			}
			marksetoffset(win->seltop, markoffset((*win->md->move)(win, win->seltop, 0L, win->selleft, ElvTrue)));
			break;
		}

		/* prepare for next command */
		viinitcmd(vinf);
		return RESULT_COMPLETE;
	}

	/* if text object used as target of operator, then compute endpoints */
	if (vinf->oper && (vinf->command == 'a' || vinf->command == 'i'))
	{
		if (vitextobj(win, vinf, &from, &to) != RESULT_COMPLETE)
		{
			viinitcmd(vinf);
			return RESULT_ERROR;
		}

		/* if TWEAK_DOT, on the main buffer, then remember command. */
		if ((vinf->tweak & TWEAK_DOT) != 0 && state->acton == NULL)
		{
			dotcmd = *vinf;
			dotcmd.tweak |= TWEAK_DOTTING;
			dotviz = ElvFalse;
			dotlines = 0L;
		}
	}
	else
#endif
	/* unless we're applying an operator to a visible selection... */
	if (!vinf->oper || !win->seltop)
	{
		/* save the cursor position in a local buffer, in case we need
		 * to do a TWEAK_MARK after calling the function, or if this
		 * command will be used as the target of an operator.
		 */
		from = *state->cursor;

		/* if TWEAK_DOT, on the main buffer, then remember command. */
		if ((vinf->tweak & TWEAK_DOT) != 0 && state->acton == NULL)
		{
			dotcmd = *vinf;
			dotcmd.tweak |= TWEAK_DOTTING;
			dotviz = ElvFalse;
			dotlines = 0L;
		}

		/* call the function */
		switch ((*vikeys[vinf->command].func)(win, vinf))
		{
		  case RESULT_ERROR:
			viinitcmd(vinf);
			return RESULT_ERROR;

		  case RESULT_MORE:
			assert(vikeys[vinf->command].when & WHEN_MORE);
			return RESULT_MORE;

		  case RESULT_COMPLETE:
			/* do nothing -- just fall through */;
		}

		/* apply the tweaks */
		flags = vinf->tweak;
		if (flags & TWEAK_FRONT)
		{
			(void)m_front(win, vinf);
		}
		if (flags & TWEAK_FIXCOL)
		{
			marksetoffset(state->cursor, markoffset((*win->md->move)(win, state->cursor, 0, win->wantcol, viiscmd(win))));
		}
		else if (!vinf->oper && !(flags & TWEAK_IGNCOL))
		{
			win->wantcol = (*win->md->mark2col)(win, state->cursor, viiscmd(win));
		}

		/* use the final location as the target of an operator, maybe */
		to = *state->cursor;
	}
	else /* visible operator */
	{
		/* Remember this command, and the fact that it is being
		 * applied to a visible selection.
		 */
		if (state->acton == NULL)
		{
			dotcmd = *vinf;
			dotcmd.tweak |= TWEAK_DOTTING;
			dotviz = ElvTrue;
			if (win->seltype == 'l' || win->seltype == 'L')
			{
				dotlines = markline(win->selbottom) - markline(win->seltop) + 1;
			}
			else
			{
				dotlines = 0L;
			}

		}

		from = *win->seltop;
		to = *win->selbottom;
	}

	/* if TWEAK_MARK, on main buffer, then remember the cursor position */
	if ((vinf->tweak & TWEAK_MARK) != 0 && state->acton == NULL)
	{
		if (win->prevcursor)
		{
			markfree(win->prevcursor);
		}
		win->prevcursor = markdup(&from);
	}

	/* if we have an operator, apply it */
	if (vinf->oper)
	{
		/* make sure both endpoints are in the same buffer */
		if (to.buffer != from.buffer)
		{
			msg(MSG_ERROR, "would span buffers");
			viinitcmd(vinf);
			return RESULT_ERROR;
		}

		/* swap endpoints, if necessary, to make from <= to */
		if (to.offset < from.offset)
		{
			tmp = to.offset;
			to.offset = from.offset;
			from.offset = tmp;
		}

		/* move the cursor to the top of the affected region */
		marksetoffset(state->cursor, from.offset);

		/* The = operator needs special treatment.  If a multi-line
		 * region is selected, then = needs to work in line mode;
		 * otherwise it should work in character mode.
		 */
		if (vinf->oper == '='
		 && markoffset(dispmove(win, 0L, INFINITY)) < to.offset)
		{
			vinf->tweak |= TWEAK_LINE|TWEAK_FRONT;
		}

		/* do we need to adjust for line-mode or inclusion? */
		if (vinf->tweak & TWEAK_LINE)
		{
			from.offset = markoffset((*win->md->move)(win, &from, 0, 0, ElvTrue));
			if (vinf->tweak & TWEAK_INCL)
			{
				to.offset = markoffset((*win->md->move)(win, &to, 0, INFINITY, ElvFalse)) + 1;
			}
			else
			{
				to.offset = markoffset((*win->md->move)(win, &to, 0, 0, ElvTrue));
			}
		}
		else
		{
			if ((vinf->tweak & TWEAK_INCL) != 0 && scanchar(&to) != '\n')
			{
				to.offset++;
			}
			else if (--to.offset < from.offset
				|| from.offset == markoffset((*win->md->move)(win, &from, 0, 0, ElvTrue))
				|| scanchar(&to) != '\n')
			{
				to.offset++;
			}
		}

		/* guard against the possibility of overflowing the buffer */
		if (markoffset(&to) > o_bufchars(markbuffer(&to)))
			marksetoffset(&to, o_bufchars(markbuffer(&to)));

		/* do the operator */
		result = oper(win, vinf, &from, &to);

#ifdef FEATURE_V
		/* if we were doing visible marking before, end it now */
		if (win->seltop)
		{
			vinf->command = ELVCTRL('[');
			(void)v_visible(win, vinf);
		}
#endif

		/* set the desired cursor column to its current column */
		if (!state->acton)
		{
			win->wantcol = (*win->md->mark2col)(win, state->cursor, viiscmd(win));
		}
	}

	/* done! */
	if (result != RESULT_MORE)
		viinitcmd(vinf);
	return result;
}


/* This parses a keystroke as part of a vi command. */
static RESULT parse(key, info)
	_CHAR_	key;	/* keystroke from user */
	void	*info;	/* current vi command parsing state */
{
	VIINFO	*vinf = (VIINFO *)info;
	CHAR	digit;


	/* if ^O, and we aren't already in a ^O, then this ^O does nothing */
	if (key == ELVCTRL('O') && !vinf->control_o)
	{
		vinf->control_o = ElvTrue;
		windefault->state->mapflags |= MAP_DISABLE;
		return RESULT_MORE;
	}
	else
	{
		vinf->control_o = ElvFalse;
	}

	/* g is special for commands -- it causes the following keystroke to
	 * be interpreted as a command beyond the normal ascii range.
	 */
	if (vinf->phase == VI_START && key == 'g')
	{
		vinf->phase = VI_AFTERG;
		windefault->state->mapflags |= MAP_DISABLE;
		return RESULT_MORE;
	}
	else if (vinf->phase == VI_AFTERG
	      || (vinf->phase == VI_START && vinf->oper == ELVG(key)))
	{
		key = ELVG(key);
		vinf->phase = VI_START;
	}

	if (vinf->phase == VI_CUTBUF)
	{
		if (key == '\033')
		{
			viinitcmd(vinf);
			windefault->state->mapflags = MAP_COMMAND;
			return RESULT_ERROR;
		}
		vinf->cutbuf = key;
		if (windefault->seltop && vinf->oper)
			vinf->phase = VI_COMPLETE;
		else if (vinf->count == 0)
			vinf->phase = VI_START;
		else
			vinf->phase = VI_COUNT2;
	}
	else if (vinf->phase == VI_COUNT2 && elvdigit(key))
	{
		vinf->count2 = vinf->count2 * 10 + key - '0';
	}
	else if (vinf->phase == VI_START && vinf->count == 0 && key == '0')
	{
		vinf->command = key;
		vinf->phase = VI_COMPLETE;
	}
	else if (vinf->phase == VI_START && elvdigit(key))
	{
		vinf->count = vinf->count * 10 + key - '0';
	}
	else if (vinf->phase == VI_COUNT2 || vinf->phase == VI_KEY2)
	{
		switch (key)
		{
		  case '\033':
			viinitcmd(vinf);
			windefault->state->mapflags = MAP_COMMAND;
			return RESULT_ERROR;

		  case ELVCTRL('V'):
			vinf->phase = VI_QUOTE;
		  	windefault->state->mapflags |= MAP_DISABLE;
			break;

		  case ELVCTRL('X'):
			vinf->phase = VI_HEX1;
		  	windefault->state->mapflags |= MAP_DISABLE;
			break;

		  default:
			vinf->key2 = key;
			vinf->phase = VI_COMPLETE;
		  	windefault->state->mapflags |= MAP_DISABLE;
		}
	}
	else if (vinf->phase == VI_QUOTE)
	{
		vinf->key2 = key;
		vinf->phase = VI_COMPLETE;
		windefault->state->mapflags |= MAP_DISABLE;
	}
	else if (vinf->phase == VI_HEX1 || vinf->phase == VI_HEX2)
	{
		/* convert digit from ASCII to binary */
		if (key >= '0' && key <= '9')
		{
			digit = key - '0';
		}
		else if (key >= 'a' && key <= 'f')
		{
			digit = key - 'a' + 10;
		}
		else if (key >= 'A' && key <= 'F')
		{
			digit = key - 'A' + 10;
		}
		else
		{
			viinitcmd(vinf);
			windefault->state->mapflags = MAP_COMMAND;
			return RESULT_ERROR;
		}

		/* merge it into the key */
		if (vinf->phase == VI_HEX1)
		{
			vinf->key2 = digit << 4;
			vinf->phase = VI_HEX2;
		  	windefault->state->mapflags |= MAP_DISABLE;
		}
		else
		{
			vinf->key2 |= digit;
			vinf->phase = VI_COMPLETE;
		}
	}
	else if (strchr(OPERATORS, (char)key))
	{
		if (windefault && windefault->seltop)
		{
			vinf->oper = key;
			vinf->command = (windefault->seltype == 'c' ? 'e' : '_');
#ifdef FEATURE_G
			/* The g= operator needs a second keystroke, which is
			 * stored as though it was a cutbuffer name.  It can't
			 * be stored as key2, because that would prevent things
			 * like g=xfy -- "change everything from here to the
			 * next y into an x".
			 */
			if (vinf->oper == ELVG('='))
			{
				vinf->phase = VI_CUTBUF;
				windefault->state->mapflags |= MAP_DISABLE;
			}
			else
#endif
				vinf->phase = VI_COMPLETE;
		}
		else if (vinf->oper == key)
		{
			vinf->command = '_';
			vinf->phase = VI_COMPLETE;
		}
		else if (vinf->oper == '\0')
		{
			vinf->oper = key;
			assert(vinf->phase == VI_START);

#ifdef FEATURE_G
			/* The g= operator needs a second keystroke, which is
			 * stored as though it was a cutbuffer name.  See the
			 * above comment for a description of why.
			 */
			if (vinf->oper == ELVG('='))
			{
				vinf->phase = VI_CUTBUF;
				windefault->state->mapflags |= MAP_DISABLE;
			}
#endif
		}
		else
		{
			viinitcmd(vinf);
			windefault->state->mapflags = MAP_COMMAND;
			return RESULT_ERROR;
		}
	}
	else if (key == '"')
	{
		vinf->phase = VI_CUTBUF;
	  	windefault->state->mapflags |= MAP_DISABLE;
	}
	else if (key == '#' || key == 'z')
	{
		vinf->command = key;
		vinf->phase = VI_COUNT2;
	  	windefault->state->mapflags |= MAP_DISABLE;
	}
	else if (strchr("[]@#`'rtfmTF\"Z\027", (char)key))	/* \027 is ^W */
	{
		vinf->command = key;
		vinf->phase = VI_KEY2;
	  	windefault->state->mapflags |= MAP_DISABLE;
	}
#ifdef FEATURE_TEXTOBJ
	else if ((windefault->seltop || vinf->oper) && (key == 'i' || key == 'a'))
	{
		vinf->command = key;
		vinf->phase = VI_KEY2;
	  	windefault->state->mapflags |= MAP_DISABLE;
	}
#endif
	else
	{
		vinf->command = key & 0xff;
		vinf->phase = VI_COMPLETE;
	}

	/* set the map context to either MAP_COMMAND or MAP_MOTION */
	windefault->state->mapflags &= ~(MAP_COMMAND|MAP_MOTION);
	if (vinf->oper == '\0')
		windefault->state->mapflags |= MAP_COMMAND;
	else
		windefault->state->mapflags |= MAP_MOTION;

	return (vinf->phase == VI_COMPLETE ? RESULT_COMPLETE : RESULT_MORE);
}

/* This function decides what shape the cursor should be */
static ELVCURSOR shape(win)
	WINDOW	win;	/* the window whose shape should be fetched */
{
	return CURSOR_COMMAND;
}


/* Push a vi command state onto the stack */
void vipush(win, flags, cursor)
	WINDOW		win;	/* the window to receive the new state */
	ELVISSTATE	flags;	/* flags of the new state */
	MARK		cursor;	/* the cursor to use in the new state */
{
	/* push a state */
	statepush(win, flags);

	/* initialize it to look like a vi state */
	win->state->parse = parse;
	win->state->perform = _viperform;
	win->state->shape = shape;
	win->state->info = safealloc(1, sizeof (VIINFO));
	win->state->modename = viiscmd(win) ? "Command" : "One Cmd";
	win->state->mapflags |= MAP_COMMAND;
	viinitcmd((VIINFO *)win->state->info);

	/* if a specific cursor was given, use it */
	if (cursor)
	{
		win->state->cursor = cursor;
		win->state->top = markdup(cursor);
		win->state->bottom = markdup(cursor);
	}

	/* If this is the main buffer for this window, then make the cursor's
	 * current column the default.
	 */
	if (!win->state->acton && o_bufchars(markbuffer(win->state->cursor)) > 0)
	{
		win->wantcol = (*win->md->mark2col)(win, win->state->cursor, ElvTrue);
	}
}


/* This function translates a potentially-abbreviated vi key name into 
 * standardized name which can be used as a help topic.  If the name can't
 * be standardized, then it returns NULL instead.
 */
CHAR *viname(name)
	CHAR	*name;
{
	CHAR	key, key2;
static	CHAR	stdname[12];

	/* Get the first character.  gX is seen as ELVG(X), and ^X is seen as
	 * ELVCTRL(X).  Other characters are taken literally.
	 */
	if (name[0] == 'g' && name[1])
	{
		key = *++name;
		if (key == '^' && name[1])
		{
			name++;
			key = ELVCTRL(*name);
		}
		key = ELVG(key);
		name++;
	}
	else if (name[0] == '^' && name[1])
	{
		key = *++name;
		key = ELVCTRL(key);
		name++;
	}
	else
	{
		key = *name++;
	}

	/* Get the second character, if there is one. */
	if (name[0] == '^' && name[1])
	{
		key2 = *++name;
		key2 = ELVCTRL(key2);
		name++;
	}
	else if (name[0])
		key2 = *name++;
	else
		key2 = '\0';

	/* if any extra chars, then not a vi command */
	if (*name)
		return NULL;

	/* some commands require two characters */
	if (!key2 && key == 'z')
		return NULL;

	/* some commands ignore a second character */ 
	if ((key == '[' || key == ']') && key2 == key)
	{
		key2 = '\0';
	}
	/* some commands allow a second character, but don't require it */
	else if (key2 && (key == ELVCTRL('W') || key == 'i' || key == 'a'))
	{
		/* do nothing */
	}
	else if (key2)
		return NULL;

	/* now we have what appears to be a well-formed key name, with the main
	 * keystroke in "key", and an optional second char in "key2".  Is the
	 * main key defined?
	 */
	if (!vikeys[key].func && (!key || !strchr(OPERATORS, key)) && key != '.')
		return NULL;

	/* format the name */
	if ((key == 'i' || key == 'a') && key2)
	{
		if (!strchr(TEXTOBJECTS, key2) && !elvpunct(key2))
			return NULL;
		CHARcpy(stdname, toCHAR("textobject"));
	}
	else
	{
		memset(stdname, 0, sizeof stdname);
		if (key >= 0x80)
		{
			CHARcpy(stdname, toCHAR("g"));
			key = ELVUNG(key);
		}
		if (vikeys[key].helptopic)
			CHARcat(stdname, toCHAR(vikeys[key].helptopic));
		else if (key < ' ')
		{
			stdname[CHARlen(stdname)] = '^';
			stdname[CHARlen(stdname)] = ELVCTRL(key);
		}
		else
		{
			stdname[CHARlen(stdname)] = key;
		}

		/* format the 2nd character, if any */
		if (key2)
		{
			if (key2 < ' ')
			{
				stdname[CHARlen(stdname)] = '^';
				stdname[CHARlen(stdname)] = ELVCTRL(key2);
			}
			else
			{
				stdname[CHARlen(stdname)] = key2;
			}
		}
	}

	return stdname;
}

#ifdef FEATURE_NORMAL
/* Interpret vi keystokes in a window */
RESULT vinormal(win, nkeys, keys)
	WINDOW	win;	/* window where keystrokes are to be run */
	int	nkeys;	/* number of keystrokes in keys[] array */
	CHAR	*keys;	/* the keystrokes to run */
{
	STATE	*ex;	/* ex input state, or NULL if none */
	STATE	*top;	/* the state that ex acted on */
	STATE	*vi;	/* the newly-pushed vi command interpreter used here */
	int	i;
	RESULT	result;

	/* This is ordinarily executed from an ex command line.  We want to
	 * suspend the ex input state, so the vi commands will run on the
	 * window's main buffer instead of the (Elvis ex history) buffer.
	 */
	if (win->state->acton)
	{
		ex = win->state;
		win->state = win->state->acton;
	}
	else
		ex = NULL;

	/* remember the top of the stack, so we can restore the stack later */
	top = win->state;

	/* push a vi command interpreter onto the input state stack */
	vipush(win, 0, NULL);
	vi = win->state;

	/* interpret each keystroke */
	for (result = RESULT_COMPLETE, i = 0; i < nkeys; i++)
	{
		/* don't allow it to switch windows */
		if (windefault != win)
		{
			if (o_verbose >= 1)
				msg(MSG_INFO, ":normal switched windows");
			result = RESULT_ERROR;
			break;
		}

		/* run the keystroke -- catch errors */
		result = statekey(keys[i]);
		if (result == RESULT_ERROR)
		{
			if (o_verbose >= 1)
				msg(MSG_INFO, ":normal vi command failed");
			result = RESULT_ERROR;
			break;
		}
	}

	/* arrange for "vi" to be popped immediately, or after the completion
	 * of the next command.  If other states have been pushed during the
	 * :normal command, then leave those unchanged.
	 */
	vi->flags |= (result == RESULT_MORE ? ELVIS_ONCE : ELVIS_POP);

	/* patch the ex input state back onto the stack */
	if (ex)
	{
		/*!!! Are there any intermediate states I should worry about?
		 * I don't think so.  This code assumes there are none.
		 */
		ex->pop = win->state;
		win->state = ex;
	}

	/* return the result */
	return result;
}
#endif /* defined(FEATURE_NORMAL) */
