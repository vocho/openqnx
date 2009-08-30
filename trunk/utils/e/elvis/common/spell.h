/* spell.h */

typedef struct spells
{
	long		flags;		/* bitmap of flags for this word */
	unsigned short	min, max;	/* range of legal indicies into link[]*/
	struct spells	*link[1];	/* array of ptrs to succeeding nodes */
} spell_t;

typedef enum
{
	SPELL_GOOD, SPELL_BAD, SPELL_UNKNOWN, SPELL_INCOMPLETE
} spellresult_t;

/* These are the possible restrictions on spellchecking.  Note that the
 * least restrictive comes first, and the most restrictive comes last.
 */
typedef enum
{
	SPELL_CHECK_ALL, SPELL_CHECK_TAGONLY, SPELL_CHECK_NONE
} spellcheck_t;

/* some standard flags */
#define SPELL_FLAG_COMPLETE	0x40000000	/* whole word */
#define SPELL_FLAG_BAD		0x20000000	/* bad word */
#define SPELL_FLAG_PERSONAL	0x10000000	/* added via :spell */

/* a macro for determining whether a given node represents a valid word */
#define SPELL_IS_GOOD(node)	((node) && ((node)->flags & (SPELL_FLAG_COMPLETE|SPELL_FLAG_BAD)) == SPELL_FLAG_COMPLETE)

/* Functions for managing dictionaries.  These are used by descr.c as well as
 * the spell-checking feature.
 */
spell_t *spellletter P_((spell_t *node, _CHAR_	letter));
spell_t *spellfindword P_((spell_t *node, CHAR	*word, int len));
spell_t *spelladdword P_((spell_t *node, CHAR *word, long flags));
void spellfree P_((spell_t *node));

/* These functions are used only by the spell-checker */
spellresult_t spellcheck P_((MARK mark, ELVBOOL tagonly, long cursoff));
void spellbegin P_((void));
ELVBOOL spellsearch P_((CHAR *word));
void spellend P_((void));
void spellhighlight P_((WINDOW win));
void spellsave P_((BUFFER custom));
void spellfix P_((CHAR *word, CHAR *result, int resultlen, ELVBOOL tagonly));
CHAR *spellshow P_((MARK cursor, _char_ font));
ELVBOOL spellcount P_((MARK cursor, long count));
void spellcheckfont P_((CHAR *fontname, spellcheck_t check, ELVBOOL bang));
void spelltmp P_((int oldfont, int newfont, int combofont));
void spellload P_((char *filename, ELVBOOL personal));
MARK spellnext P_((WINDOW win, MARK curs));

/* some global variables */
extern spell_t	*spelltags;
extern spell_t	*spellwords;
