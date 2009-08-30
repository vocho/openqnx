/* regexp.h */

#define NSUBEXP  10	/* max # of subexpressions, plus 1 for whole expr */

/* DAG - hack to diff this from system regexp which this is incompatible with */
#define regexp regexp_elvis
#define regbuild regbuild_elvis
#define regcomp regcomp_elvis
#define regdup regdup_elvis
#define regexec regexec_elvis
#define regtilde regtilde_elvis
#define regsub regsub_elvis
#define regerror regerror_elvis

typedef struct regexp {
	long	startp[NSUBEXP];/* start of text matching a subexpr */
	long	endp[NSUBEXP];	/* end of a text matching a subexpr */
	long	leavep;		/* offset of text matching \= */
	long	nextlinep;	/* offset of start of following line */
	BUFFER	buffer;		/* buffer that the above offsets refer to */
	int	minlen;		/* length of shortest possible match */
	CHAR	first;		/* first character, if known; else \0 */
	ELVBOOL	bol;		/* must start at beginning of line? */
	ELVBOOL	literal;	/* contains no metacharacters? */
	ELVBOOL	upper;		/* contains some uppercase letters? */
	CHAR	program[1];	/* Unwarranted chumminess with compiler. */
} regexp;

BEGIN_EXTERNC
extern CHAR	*regbuild P_((_CHAR_ delim, CHAR **refp, ELVBOOL reg));
extern regexp	*regcomp P_((CHAR *retext, MARK cursor));
extern regexp	*regdup P_((regexp *re));
extern int	regexec P_((regexp *re, MARK from, ELVBOOL bol));
extern CHAR	*regtilde P_((CHAR *newp));
extern CHAR	*regsub P_((regexp *re, CHAR *newp, ELVBOOL doit));
extern void	regerror P_((char *errmsg));
END_EXTERNC

#ifndef REG
# define REG /* as nothing */
#endif
