/* digraph.c */
/* Copyright 1995 by Steve Kirkendall */

#ifndef NO_DIGRAPH
BEGIN_EXTERNC
extern CHAR digraph P_((_CHAR_ key1, _CHAR_ key2));
extern void digaction P_((WINDOW win, ELVBOOL bang, CHAR *extra));
# ifdef FEATURE_MKEXRC
extern void digsave P_((BUFFER buf));
# endif
END_EXTERNC
#endif
