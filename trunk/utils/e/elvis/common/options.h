/* options.h */
/* Copyright 1995 by Steve Kirkendall */


/* This file contains extern declarations for the option maniplulation
 * functions.
 */

/* These values are ORed together in the "flags" field of an option value
 * to inform elvis how to treat it.  Initially, all flags should be 0.
 * Elvis maintains the "set" and "show" flags itself; the "save" flag is'
 * maintained by each option's store() function.
 */
#define OPT_SET		0x001	/* value has been changed */
#define OPT_SHOW	0x002	/* value should be shown */
#define OPT_HIDE	0x004	/* value should be hidden even if ":set all" */
#define OPT_LOCK	0x008	/* value can never be changed */
#define OPT_UNSAFE	0x010	/* value can't be changed if "safer" */
#define OPT_FREE	0x020	/* call safefree() on the value before freeing/changing it */
#define OPT_REDRAW	0x040	/* changing this option forces a redraw */
#define OPT_SCRATCH	0x080	/* changing forces redraw from scratch */
#define OPT_NODFLT	0x100	/* no default value */


/* REGARDING THE "tab" TYPE...
 *
 * To balance versatility and compatibility, elvis allows tabstop-like options
 * to have values that are lists of column widths.  The last width repeats.
 * This list is used to construct an array of short ints, in which the first
 * entry is the length of the list plus 2, the second is the repeating width,
 * and the remaining numbers distance from a given column to the next tabstop.
 * Thus, if you're in column 19 and you want to move to the start of the next
 * tab, you would add o_tabstop(buf)[2+19] to your column.  If the requested
 * column is >= than o_tabstop(buf)[0], then you would do the math to find
 * the repeated column.
 *
 * Here are some macros that test the value of a tabstop-like option.
 * opt_istab(t,col) returns TRUE if col at a tabstop.  opt_totab(t,col)
 * returns the number of spaces to the next tabstop.  Both of these assume
 * the left margin is 0 (as it is internally to elvis), not 1 (as on the ruler)
 */
#define opt_istab(t,col)  ((t)[0] > (col) \
				? (t)[2 + (col) - 1] == 1 \
				: (((col) - (t)[0]) % (t)[1] == 0))
#define opt_totab(t,col)  ((t)[0] > (col) \
				? (t)[2 + (col)] \
				: (t)[1] - ((col) - (t)[0]) % (t)[1])

/* Instances of this structure are used to store values of variables.  Values
 * are stored apart from their descriptions because for some options the
 * values will vary for each window or buffer, but their descriptions won't.
 */
typedef struct
{
	union
	{
		long	number;		/* for numeric options */
		ELVBOOL	boolean;	/* for boolean options */
		char	character;	/* for "one of" options */
		CHAR	*string;	/* for string options */
		short	*tab;		/* tabstop positions */
		void	*pointer;	/* for any other type of option */
	} value;
	short flags;		/* flags for the option */
} OPTVAL;

/* Instances of this structure are used to describe options. */
typedef struct optdesc_s
{
   char	*longname, *shortname;
   CHAR *(*asstring) P_((struct optdesc_s *opt, OPTVAL *val));
   int	(*isvalid) P_((struct optdesc_s *opt, OPTVAL *val, CHAR *newval));
   char *limit;
   int	(*store) P_((struct optdesc_s *opt, OPTVAL *val, CHAR *newval));
   CHAR	*dflt;
#ifdef FEATURE_AUTOCMD
   ELVBOOL event; /* if ElvTrue, send events when option changes */
#endif
} OPTDESC;


/* This macro returns the flags field of an optval, when given the lvalue
 * of the value field.  For example, optflags(o_blksize) accesses the flags
 * of the blksize option.
 */
#define optflags(o)		(((OPTVAL *)&(o))->flags)


/* This macro sets the value & flags of an option.  For example, the macro
 * optpreset(o_session, "session.elv", OPT_HIDE) sets the value of the
 * "session" option to "session.elv", and turns on its OPT_HIDE flag.
 */
#define optpreset(o, v, f)	((o) = (v), optflags(o) |= (f))


/* These functions are used to say which particular options are relevent at
 * the moment.
 */
BEGIN_EXTERNC
extern void optinsert P_((char *domain, int nopts, OPTDESC desc[], OPTVAL val[]));
extern void optdelete P_((OPTVAL val[]));
extern void optfree P_((int nopts, OPTVAL *val));
extern int optisnumber P_((OPTDESC *desc, OPTVAL *val, CHAR *newval));
extern int optisstring P_((OPTDESC *desc, OPTVAL *val, CHAR *newval));
extern int optispacked P_((OPTDESC *desc, OPTVAL *val, CHAR *newval));
extern int optisoneof P_((OPTDESC *desc, OPTVAL *val, CHAR *newval));
extern int optistab P_((OPTDESC *desc, OPTVAL *val, CHAR *newval));
extern CHAR *optnstring P_((OPTDESC *desc, OPTVAL *val));
extern CHAR *optsstring P_((OPTDESC *desc, OPTVAL *val));
extern CHAR *opt1string P_((OPTDESC *desc, OPTVAL *val));
extern CHAR *opttstring P_((OPTDESC *desc, OPTVAL *val));
extern void optautocmd P_(( char *name, OPTDESC *desc, OPTVAL *val));
extern ELVBOOL optset P_((ELVBOOL bang, CHAR *ARGS, CHAR *OUTBUF, size_t outsize));
extern CHAR *optgetstr P_((CHAR *name, OPTDESC **desc));
extern ELVBOOL optputstr P_((CHAR *name, CHAR *value, ELVBOOL bang));
extern OPTVAL *optval P_((char *name));
extern void optsetdflt P_((void));
extern void *optlocal P_((void *leval));
#ifdef FEATURE_AUTOCMD
extern char *optevent P_((CHAR *optname));
#endif
#if defined (GUI_WIN32)
extern int optiswinsize (OPTDESC *desc, OPTVAL *val, CHAR *newval);
#endif
END_EXTERNC
