/* cut.h */
/* Copyright 1995 by Steve Kirkendall */


BEGIN_EXTERNC
extern BUFFER cutbuffer P_((_CHAR_ cbname, ELVBOOL create));
extern void cutyank P_((_CHAR_ cbname, MARK from, MARK to, _CHAR_ type, _CHAR_ aideeffect));
extern MARK cutput P_((_CHAR_ cbname, WINDOW win, MARK at, ELVBOOL after, ELVBOOL cretend, ELVBOOL lretend));
extern CHAR *cutmemory P_((_CHAR_ cbname));
END_EXTERNC
