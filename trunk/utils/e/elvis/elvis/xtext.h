/* xtext.h */

typedef struct
{
	Window		win;
	int		x, y;		/* position of text area within shell */
	unsigned	w, h;		/* size of text area, in pixels */
	Pixmap		undercurs;	/* image of character under cursor */
	int		cursx, cursy;	/* cursor position */
	unsigned	cellw, cellh;	/* size of a character cell, in pixels */
	int		cellbase;	/* baseline of characters */
	unsigned	rows, columns;	/* size of the window, in cells */
	ELVCURSOR	cursor;		/* current state of cursor */
	ELVCURSOR	nextcursor;	/* next state of cursor */
	long		bg;		/* background color of normal text */
#ifdef FEATURE_IMAGE
	int		scrollpixels;	/* number of pixels scrolled */
	Pixmap		pixmap;		/* background image, with scrolling */
#endif
#ifdef FEATURE_XFT
	XftDraw		*xftdraw;	/* Xft version of the window */
#endif
} X_TEXTAREA;

void x_ta_predict P_((X11WIN *xw, unsigned columns, unsigned rows));
void x_ta_create P_((X11WIN *xw, int x, int y));
void x_ta_destroy P_((X11WIN *xw));
void x_ta_drawcursor P_((X11WIN *xw));
void x_ta_erasecursor P_((X11WIN *xw));
void x_ta_moveto P_((X11WIN *xw, int column, int row));
void x_ta_draw P_((X11WIN *xw, long fg, long bg, int bits, CHAR *text, int len));
void x_ta_shift P_((X11WIN *xw, int qty, int rows));
void x_ta_scroll P_((X11WIN *xw, int qty, ELVBOOL notlast));
void x_ta_clrtoeol P_((X11WIN *xw));
void x_ta_event P_((X11WIN *xw, XEvent *event));
void x_ta_recolor P_((X11WIN *xw, _char_ font));
void x_ta_setbg P_((X11WIN *xw, long bg));
