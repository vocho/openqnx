/* lp.h */
/* Copyright 1995 by Steve Kirkendall */

#ifdef FEATURE_LPR

typedef struct
{
    char *name;				/* printer type, e.g. "epson" */
    int  minorno;			/* value to pass to `before' */
    ELVBOOL spooled;			/* uses lpout spooler */
    void (*before) P_((int minorno, void (*draw)(_CHAR_ ch)));/* called before print job */
    void (*fontch) P_((_char_ font, _CHAR_ ch)); /* output a single char */
    void (*page) P_((int linesleft));	/* called at end of each page */
    void (*after) P_((int linesleft));	/* called at end of print job */
} LPTYPE;

BEGIN_EXTERNC
extern unsigned char *lpfg P_((_char_ fontcode));
extern char *lpoptfield P_((char *field, char *dflt));
extern RESULT lp P_((WINDOW win, MARK top, MARK bottom, ELVBOOL force));
END_EXTERNC
extern LPTYPE lpepson, lppana, lpibm, lphp, lpdumb, lpansi, lphtml;
extern LPTYPE lpcr, lpbs;
extern LPTYPE lpps, lpps2;
#ifdef GUI_WIN32
extern LPTYPE lpwindows;
#endif

#endif /* FEATURE_LPR */
