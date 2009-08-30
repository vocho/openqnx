/* elvctype.h */

#ifdef NEED_CTYPE
# ifndef elvupper
#  define ELVCT_UPPER	0x01
#  define ELVCT_LOWER	0x02
#  define ELVCT_DIGIT	0x04
#  define ELVCT_XDIGIT	0x08
#  define ELVCT_SPACE	0x10
#  define ELVCT_PUNCT	0x20
#  define ELVCT_CNTRL	0x40

#  define ELVCT_ALPHA	(ELVCT_UPPER|ELVCT_LOWER)
#  define ELVCT_ALNUM	(ELVCT_ALPHA|ELVCT_DIGIT)
#  define ELVCT_GRAPH	(ELVCT_ALNUM|ELVCT_PUNCT)

#  define elvupper(c)	((elvct_class[(CHAR)(c)] & ELVCT_UPPER) != 0)
#  define elvlower(c)	((elvct_class[(CHAR)(c)] & ELVCT_LOWER) != 0)
#  define elvdigit(c)	((elvct_class[(CHAR)(c)] & ELVCT_DIGIT) != 0)
#  define elvxdigit(c)	((elvct_class[(CHAR)(c)] & ELVCT_XDIGIT) != 0)
#  define elvspace(c)	((elvct_class[(CHAR)(c)] & ELVCT_SPACE) != 0)
#  define elvpunct(c)	((elvct_class[(CHAR)(c)] & ELVCT_PUNCT) != 0)
#  define elvgraph(c)	((elvct_class[(CHAR)(c)] & ELVCT_GRAPH) != 0)
#  define elvcntrl(c)	((elvct_class[(CHAR)(c)] & ELVCT_CNTRL) != 0)
#  define elvalpha(c)	((elvct_class[(CHAR)(c)] & ELVCT_ALPHA) != 0)
#  define elvalnum(c)	((elvct_class[(CHAR)(c)] & ELVCT_ALNUM) != 0)

#  define setupper(c)	(elvct_class[(CHAR)(c)] |= ELVCT_UPPER)
#  define setlower(c)	(elvct_class[(CHAR)(c)] |= ELVCT_LOWER)
#  define setpunct(c)	(elvct_class[(CHAR)(c)] |= ELVCT_PUNCT)

#  define clrupper(c)	(elvct_class[(CHAR)(c)] &= ~ELVCT_UPPER)
#  define clrlower(c)	(elvct_class[(CHAR)(c)] &= ~ELVCT_LOWER)
#  define clrpunct(c)	(elvct_class[(CHAR)(c)] &= ~ELVCT_PUNCT)

#  define elvtoupper(c)	elvct_upper[(CHAR)(c)]
#  define elvtolower(c)	elvct_lower[(CHAR)(c)]

extern CHAR elvct_upper[256];
extern CHAR elvct_lower[256];
extern CHAR elvct_class[256];

# endif /* ndef isupper */

#else /* use the standard macros */
# include <ctype.h>
# define elvupper(c)	isupper(c)
# define elvlower(c)	islower(c)
# define elvdigit(c)	isdigit(c)
# define elvxdigit(c)	isxdigit(c)
# define elvspace(c)	isspace(c)
# define elvpunct(c)	ispunct(c)
# define elvgraph(c)	isgraph(c)
# define elvcntrl(c)	iscntrl(c)
# define elvalpha(c)	isalpha(c)
# define elvalnum(c)	isalnum(c)
# define elvtoupper(c)	toupper(c)
# define elvtolower(c)	tolower(c)
#endif /* not NEED_CTYPE */
