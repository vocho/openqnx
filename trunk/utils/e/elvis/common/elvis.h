/* elvis.h */
/* Copyright 1995 by Steve Kirkendall */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "config.h"
#include "version.h"
#include "osdef.h"

/* The FTP protocol requires HTTP */
#if defined(PROTOCOL_FTP) && !defined(PROTOCOL_HTTP)
# define PROTOCOL_HTTP
#endif

/* The hlobject feature requires text objects */
#if defined(FEATURE_HLOBJECT) && !defined(FEATURE_TEXTOBJ)
# define FEATURE_TEXTOBJ
#endif

/* Some handy macros */
#define QTY(array)	(int)(sizeof(array) / sizeof((array)[0]))
#define ELVCTRL(ch)	((ch) ^ 0x40)

/* Names of some special buffers */
#define INIT_BUF	"Elvis initialization"
#define CUSTOM_BUF	"Elvis custom initialization"
#define BEFOREREAD_BUF	"Elvis before reading"
#define AFTERREAD_BUF	"Elvis after reading"
#define BEFOREWRITE_BUF	"Elvis before writing"
#define AFTERWRITE_BUF	"Elvis after writing"
#define MSG_BUF		"Elvis messages"
#define MSGQUEUE_BUF	"Elvis message queue"
#define UNTITLED_BUF	"Elvis untitled #%d"
#define EX_BUF		"Elvis ex history"
#define REGEXP_BUF	"Elvis regexp history"
#define FILTER_BUF	"Elvis filter history"
#define MORE_BUF	"Elvis more"
#define CUTANON_BUF	"Elvis cut buffer 1"	/* anonymous cut buffer */
#define CUTNAMED_BUF	"Elvis cut buffer %c"	/* cut buffers a-z and 1-9 */
#define CUTEXTERN_BUF	"Elvis clipboard"	/* cut buffers < and > */
#define CUTINPUT_BUF	"Elvis previous input"	/* cut buffer . */
#define DEFAULT_BUF	"Elvis default options"
#define HELP_BUF	"Elvis documentation"
#define ERRLIST_BUF	"Elvis error list"
#define TRACE_BUF	"Elvis map log"
#define BBROWSE_BUF	"Elvis buffer list"
#define EQUALTILDE_BUF	"Elvis equal tilde"

/* Names of files that store default contents of buffers */
#define INIT_FILE	"elvis.ini"	/* executed before first file is loaded */
#if ANY_UNIX
# define CUSTOM_FILE	".exrc"		/* custom file for each user */
#else
# define CUSTOM_FILE	"elvis.rc"	/* custom file for each user */
#endif
#define BEFOREREAD_FILE	"elvis.brf"	/* executed before each file is loaded */
#define AFTERREAD_FILE	"elvis.arf"	/* executed after each file is loaded */
#define BEFOREWRITE_FILE "elvis.bwf"	/* executed before writing a file */
#define AFTERWRITE_FILE	"elvis.awf"	/* executed after writing a file */
#define MSG_FILE	"elvis.msg"	/* verbose message translations */
#define HELP_FILE	"elvis.html"	/* elvis online documentation */
#define SYNTAX_FILE	"elvis.syn"	/* syntax descriptions for languages */
#define MARKUP_FILE	"elvis.mar"	/* markup descriptions for languages */
#define BROWSER_FILE	"elvis.bro"	/* prototype of browser document */
#define NET_FILE	"elvis.net"	/* network proxy list */
#define FTP_FILE	"elvis.ftp"	/* ftp account information */

/* a very large number */
#define INFINITY	2147483647L

/* default size of the tag stack (for each window) */
#ifndef TAGSTK
# define TAGSTK		10
#endif


/* Some useful data types */
typedef enum {ElvFalse, ElvTrue} ELVBOOL;
typedef enum { RESULT_COMPLETE, RESULT_MORE, RESULT_ERROR } RESULT;
typedef unsigned char CHAR;
typedef unsigned short COUNT;
typedef unsigned int	_COUNT_;
typedef unsigned int	_CHAR_;
typedef int		_char_;


/* Include ctype macros -- either elvis' version or the standard version */
#include "elvctype.h"

/* Character conversions, and other operations */
#define toCHAR(s)	((CHAR *)(s))
#define tochar8(s)	((char *)(s))
#define CHARcpy(d,s)	((void)strcpy((char *)(d), (char *)(s)))
#define CHARcat(d,s)	((void)strcat((char *)(d), (char *)(s)))
#define CHARncpy(d,s,n)	((void)strncpy((char *)(d), (char *)(s), (n)))
#define CHARncat(d,s,n)	((void)strncat((char *)(d), (char *)(s), (n)))
#define CHARlen(s)	strlen((char *)(s))
#define CHARchr(s,c)	((CHAR *)strchr((char *)(s), (char)(c)))
#define CHARrchr(s,c)	((CHAR *)strrchr((char *)(s), (char)(c)))
#define CHARcmp(s,t)	(strcmp((char *)(s), (char *)(t)))
#define CHARncmp(s,t,n) (strncmp((char *)(s), (char *)(t), (n)))
#define CHARdup(s)	((CHAR *)safedup(tochar8(s)))
#define CHARkdup(s)	((CHAR *)safekdup(tochar8(s)))
#define long2CHAR(s,l)	((void)sprintf((char *)(s), "%ld", (l)))
#define CHAR2long(s)	(atol(tochar8(s)))

#ifndef USE_PROTOTYPES
# if defined(__STDC__) || defined(__cplusplus)
#  define USE_PROTOTYPES	1
# else
#  define USE_PROTOTYPES	0
# endif
#endif
#if USE_PROTOTYPES
# define P_(args)	args
# define P_VOID		(void)
#else
# define P_(args)	()
# define P_VOID		()
#endif

/* Some macros to handle C++ in a graceful way */
#if defined (__cplusplus)
#define BEGIN_EXTERNC	extern "C" {
#define END_EXTERNC	}
#else
#define BEGIN_EXTERNC
#define END_EXTERNC
#endif

#ifdef FEATURE_STDIN
extern FILE *origstdin;
extern ELVBOOL stdin_not_kbd;
#endif

/* Header files for the modules */
#include "safe.h"
#include "options.h"
#include "optglob.h"
#include "session.h"
#include "lowbuf.h"
#include "message.h"
#include "buffer.h"
#include "mark.h"
#include "buffer2.h"
#include "scan.h"
#include "opsys.h"
#include "map.h"
#include "gui.h"
#include "display.h"
#include "draw.h"
#include "state.h"
#include "window.h"
#include "spell.h"
#include "color.h"
#include "options2.h"
#include "gui2.h"
#include "display2.h"
#include "draw2.h"
#include "state2.h"
#include "event.h"
#include "input.h"
#include "vi.h"
#include "regexp.h"
#include "ex.h"
#include "move.h"
#include "vicmd.h"
#include "operator.h"
#include "cut.h"
#include "elvisio.h"
#include "lp.h"
#include "calc.h"
#include "more.h"
#include "digraph.h"
#include "tag.h"
#include "tagsrch.h"
#include "tagelvis.h"
#include "descr.h"
#include "need.h"
#include "misc.h"
#include "message2.h"
#include "fold.h"
#include "autocmd.h"
#include "region.h"

/* The following are defined in main.c */
extern GUI *chosengui;
BEGIN_EXTERNC
extern ELVBOOL mainfirstcmd P_((WINDOW win));
END_EXTERNC
