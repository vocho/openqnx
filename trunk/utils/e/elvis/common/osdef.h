/* osunix/osdef.h */

/*=============================================================================
 * This is the name of the OS, as reported by ":set os?"
 */
#ifndef OSNAME
# define OSNAME	"unix"
#endif
#define ANY_UNIX 1

/*=============================================================================
 * This is the default shell, as reported by ":set shell?"
 */
#ifndef OSSHELL
# define OSSHELL "/bin/sh"
#endif

/*=============================================================================
 * This is the default printer, as report by ":set lpout?".  For unix, this
 * should be the name of a spooler program: usually "!lp -s", but BSD and
 * Linux use "!lpr".
 *
 * NOTE: The "configure" script generates a "config.h" file which overrides
 * the value shown here.
 */
#ifndef OSLPOUT
# if defined(bsd) || defined(linux)
#  define OSLPOUT "!lpr"
# else
#  define OSLPOUT "!lp -s"
# endif
#endif

/*=============================================================================
 * This is used as the delimiter inside a "path" string.  For UNIX, this is
 * traditionally a ':' character.  Most other OSes use a ';' character.
 */
#define OSPATHDELIM	':'

/*=============================================================================
 * This is used as the directory delimiter inside a file name.  For UNIX, this
 * is traditionally a '/' character.  Most other OSes use a '\\' character.
 */
#define OSDIRDELIM	'/'

/*=============================================================================
 * This is a list of directories where elvis might store its session file.
 */
#define OSSESSIONPATH	"/var/tmp:/tmp:~:."

/* This is the default value of the "syntax" display mode's includepath option,
 * which tells elvis where to look for #include files.  It can be overridden
 * at run time by an INCLUDEPATH environment variable.
 */
#ifndef OSINCLUDEPATH
# define OSINCLUDEPATH	"/usr/local/include:/usr/include"
#endif

/*=============================================================================
 * This is the name of a directory where elvis stores some default files.
 * It is incorporated into the default value of the "loadpath" option.  This
 * macro is optional; if undefined, then it is simply omitted from loadpath.
 *
 * NOTE: The "configure" script generates a "config.h" file which overrides
 * the value shown here.
 */
#ifndef OSLIBPATH
# define OSLIBPATH	"/usr/local/lib/elvis"
#endif

/*=============================================================================
 * This should be defined if there is an osinit() function.  When defined,
 * this function will be called after a GUI has been selected but before any
 * other initialization.  It is used mostly to initialize options.
 */
#undef OSINIT

/*=============================================================================
 * OSFILENAMERULES should be a bitwise-OR of ELVFNR flags, indicating how
 * file names passed to elvis should be interpreted.  Usually this will be
 * "(ELVFNR_TILDE|ELVFNR_DOLLAR|ELVFNR_WILDCARD)" to do the kind of processing
 * that the shell would do on Unix, or "(ELVFNR)0" on Unix systems.
 *
 * OSEXPANDARGS should be 1 if the args ever need expansion, or 0 if they
 * never do.
 */
#define OSFILENAMERULES	(ELVFNR)0
#define OSEXPANDARGS	0

/*=============================================================================
 * This is the default terminal type, used by the "termcap" GUI whenever the
 * TERM environment variable is unset.
 */
#define TTY_DEFAULT	"unknown"

/*=============================================================================
 * This determines whether filename completion should ignore case differences.
 */
#define FILES_IGNORE_CASE 0

/*=============================================================================
 * Miscellaneous tweaks
 */

#if defined(ultrix)
# define NEED_STRDUP
#endif

/* Some newer Linux systems have speed_t defined in <termios.h> instead of
 * the traditional <sys/types.h>.
 */
#if defined(linux) && !defined(USE_TERMIO) && !defined(USE_SGTTY)
# include <termios.h>
#endif

#if (defined(M_XENIX) || defined(__QNX__)) && defined(__STDC__)
extern char	PC;		/* Pad char */
extern char	*BC;		/* backspace */
extern char	*UP;		/* cursor up */
extern short	ospeed;		/* tty speed, eg B2400 */

# if defined (__cplusplus)
extern "C" {
# endif
# ifdef __QNX__
#  include <termcap.h>
# else
extern int	tgetent(char *, char *);
extern int	tgetnum(char *);
extern int	tgetflag(char *);
extern char	*tgoto(char *, int, int);
extern char	*tgetstr(char*, char**);
extern void	tputs(char *, int, int (*)(int));
# endif
# if defined (__cplusplus)
}
# endif

#endif
