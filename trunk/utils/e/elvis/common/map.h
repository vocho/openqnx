/* map.h */
/* Copyright 1995 by Steve Kirkendall */


/* The current state of the keystroke mapping queue */
typedef enum
{
	MAP_CLEAR,	/* not in the middle of any map */
	MAP_USER,	/* at least 1 unresolved user map */
	MAP_KEY		/* at least 1 unresolved key map, but no user maps */
} MAPSTATE;

/* This data type is a bitmap of the following flags.  It is used to describe
 * when a map takes effect, and how it is interpreted.
 */
typedef unsigned int MAPFLAGS;
#define MAP_INPUT	0x0001	/* map when in input mode */
#define	MAP_HISTORY	0x0002	/* map when in input on history buffer */
#define MAP_COMMAND	0x0004	/* map when in visual command mode */
#define MAP_MOTION	0x0008	/* map in command mode when motion expected */
#define MAP_SELECT	0x0010	/* map when visible selection is pending */
#define MAP_WHEN	0x00ff	/* mask for all of the "map when" bits */
#define MAP_ASCMD	0x0100	/* always execute map as visual commands */
#define MAP_ALL		0x011f	/* all of the above */
#define MAP_ABBR	0x0200	/* this is an abbr, not a map */
#define MAP_DISABLE	0x0400	/* disable all maps for next keystroke */
#define MAP_BREAK	0x0800	/* switch from "run" to "step" trace mode */
#define MAP_NOREMAP	0x1000	/* disable remaps for rhs of this map */
#define MAP_NOSAVE	0x2000	/* prevent :mkexrc from saving this map */

BEGIN_EXTERNC
extern void	mapinsert P_((CHAR *rawin, int rawlen, CHAR *cooked, int cooklen, CHAR *label, MAPFLAGS flags, CHAR *mode));
extern ELVBOOL	mapdelete P_((CHAR *rawin, int rawlen, MAPFLAGS flags, CHAR *mode, ELVBOOL del, ELVBOOL brk));
extern MAPSTATE	mapdo P_((CHAR *keys, int nkeys));
extern void	mapunget P_((CHAR *keys, int nkeys, ELVBOOL remap));
extern CHAR	*maplist P_((MAPFLAGS flags, CHAR *mode, int *reflen));
extern RESULT	maplearn P_((_CHAR_ buf, ELVBOOL starting));
extern CHAR	maplrnchar P_((_CHAR_ dflt));
extern ELVBOOL	mapbusy P_((void));
extern void	mapalert P_((void));
extern CHAR	*mapabbr P_((CHAR *bkwd, long *oldptr, long *newptr, ELVBOOL exline));
#ifdef FEATURE_MKEXRC
extern void	mapsave P_((BUFFER buf));
#endif
END_EXTERNC
