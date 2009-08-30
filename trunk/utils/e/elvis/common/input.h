/* input.h */
/* Copyright 1995 by Steve Kirkendall */


BEGIN_EXTERNC
extern void inputpush P_((WINDOW win, ELVISSTATE flags, _char_ mode));
extern void inputtoggle P_((WINDOW win, _char_ mode));
extern void inputchange P_((WINDOW win, MARK from, MARK to, ELVBOOL linemd));
extern void inputbeforeenter P_((WINDOW win));
END_EXTERNC
