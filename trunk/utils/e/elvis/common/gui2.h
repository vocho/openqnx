/* gui2.h */
/* Copyright 1995 by Steve Kirkendall */

/* Herbert: 
 * two more GUI's (by Lee).
 */

#define guiscrollbar(w,t,b,n)	if (gui->scrollbar != NULL) \
					(*scrollbar)((w)->gw, t, b, n); else (void)0
#define guiflush()		if (gui->flush) (*gui->flush)()

BEGIN_EXTERNC
extern ELVBOOL	guicolorsync P_((WINDOW win));
extern void	guimoveto P_((WINDOW win, int column, int row));
extern DRAWATTR	*guidraw P_((WINDOW win, _char_ font, CHAR *text, int len, int forcebits));
extern ELVBOOL	guishift P_((WINDOW win, int qty, int rows));
extern ELVBOOL	guiscroll P_((WINDOW win, int qty, ELVBOOL notlast));
extern void	guiclrtoeol P_((WINDOW win));
extern void	guireset P_((void));
extern ELVBOOL	guipoll P_((ELVBOOL reset));
extern void	guibeep P_((WINDOW win));
extern int	guikeylabel P_((CHAR *given, int givenlen, CHAR **labelptr, CHAR **rawptr));
END_EXTERNC

extern GUI	*gui;

#ifdef GUI_GNOME
extern GUI guignome;
#endif

#ifdef GUI_GNOME
extern GUI guignome;
#endif

#ifdef GUI_X11
extern GUI	guix11;
#endif

#ifdef GUI_PM
extern GUI	guipm;
#endif

#ifdef GUI_CURSES
extern GUI	guicurses;
#endif

#ifdef GUI_BIOS
extern GUI	guibios;
#endif

#ifdef GUI_VIO
extern GUI	guivio;
#endif

#ifdef GUI_TERMCAP
extern GUI	guitermcap;
#endif

#ifdef GUI_OPEN
extern GUI	guiopen;
extern GUI	guiquit;
extern GUI	guiscript;
#endif

#ifdef GUI_WIN32
extern GUI	guiwin32;
#endif
