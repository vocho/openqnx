/* misc.h */
/* Copyright 1995 by Steve Kirkendall */

extern CHAR	empty[];
extern CHAR	blanks[80];
extern char	**arglist;
extern int	argnext;
BEGIN_EXTERNC
#ifdef DEBUG_ALLOC
# define buildCHAR(ref,ch)	_buildCHAR(__FILE__, __LINE__, (ref), (ch))
# define buildstr(ref,str)	_buildstr(__FILE__, __LINE__, (ref), (str))
extern int	_buildCHAR P_((char *file, int line, CHAR **refstr, _CHAR_ ch));
extern int	_buildstr P_((char *file, int line, CHAR **refstr, char *add));
#else
extern int	buildCHAR P_((CHAR **refstr, _CHAR_ ch));
extern int	buildstr P_((CHAR **refstr, char *add));
#endif
extern MARK	wordatcursor P_((MARK cursor, ELVBOOL apostrophe));
extern CHAR	*addquotes P_((CHAR *chars, CHAR *str));
extern CHAR	*removequotes P_((CHAR *chars, CHAR *str));
extern int	CHARncasecmp P_((CHAR *s1, CHAR *s2, int len));
END_EXTERNC
