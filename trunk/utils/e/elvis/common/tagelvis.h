/* tagelvis.h */

extern ELVBOOL	tenewtag;

BEGIN_EXTERNC

#ifdef FEATURE_SHOWTAG
typedef struct tedef_s
{
	MARK	where;		/* where a tag is defined */
	CHAR	*label;		/* tag name, and maybe other info */
} TEDEF;
extern void tebuilddef P_((BUFFER buf));
extern void tefreedef P_((BUFFER buf));
extern CHAR *telabel P_((MARK cursor));
#endif

#ifdef DISPLAY_SYNTAX
spell_t *telibrary P_((char *tagfile, spell_t *dict, ELVBOOL ignorecase, CHAR *prefix));
#endif

#ifdef FEATURE_COMPLETE
CHAR *tagcomplete P_((WINDOW win, MARK m));
#endif

extern void tesametag P_((void));
extern TAG *tetag P_((CHAR *select));
extern BUFFER tebrowse P_((ELVBOOL all, CHAR *select));
extern void tepush P_((WINDOW win, CHAR *label));

END_EXTERNC
