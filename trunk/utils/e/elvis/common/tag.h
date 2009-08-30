/* tag.h */

/* maximum number of attributes */
#define MAXATTR	10

/* The indicies of the standard fields */
#define TAGNAME	attr[0]
#define TAGFILE attr[1]
#define TAGADDR	attr[2]

/* values of a single tag */
typedef struct tag_s
{
	struct tag_s	*next;	/* next tag in sorted order, or NULL */
	struct tag_s	*bighop;/* some other tag, or NULL */
	long	match;		/* likelyhood that this is desired tag */
	char	*attr[MAXATTR];	/* other attribute values; see tagattrname[] for names */
} TAG;

#if 0
/* Stuff normally defined in elvis.h, but elvis.h might not be included */
# ifndef QTY
#  include <stdio.h>
typedef enum {ElvFalse, ElvTrue} ELVBOOL;
#  define P_(args)	args
# endif
#endif

extern char *tagattrname[MAXATTR];
extern TAG *taglist;
extern ELVBOOL tagforward;

BEGIN_EXTERNC

extern void tagnamereset P_((void));
extern TAG *tagdup P_((TAG *tag));
extern ELVBOOL tagattr P_((TAG *tag, char *name, char *value));
extern TAG *tagfree P_((TAG *tag));
extern void tagdelete P_((ELVBOOL all));
extern void tagadd P_((TAG *tag));
extern TAG *tagparse P_((char *line));

END_EXTERNC
