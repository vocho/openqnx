/* draw2.h */
/* Copyright 1995 by Steve Kirkendall */


#define drawscratch(di)	((di)->mustredraw = True)

BEGIN_EXTERNC
extern DRAWINFO *drawalloc P_((int rows, int columns, MARK top));
extern void drawfree P_((DRAWINFO *di));
extern void drawimage P_((WINDOW win));
extern void drawexpose P_((WINDOW win, int top, int left, int bottom, int right));
extern void drawmsg P_((WINDOW win, MSGIMP imp, CHAR *verbose, int len));
extern void drawex P_((WINDOW win, CHAR *text, int len));
extern void drawfinish P_((WINDOW win));
extern void drawopenedit P_((WINDOW win));
extern void drawopencomplete P_((WINDOW win));
extern void drawextext P_((WINDOW win, CHAR *text, int len));
extern void drawexlist P_((WINDOW win, CHAR *text, int len));
END_EXTERNC
