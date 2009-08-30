/* xmisc.h */

typedef struct x_loadedfont_s
{
	struct x_loadedfont_s *next;	/* next font in linked list */
	int		links;		/* number of windows using this font */
	XFontStruct	*fontinfo;	/* X font structure */
	char		*name;		/* name of the font */
#ifdef FEATURE_XFT
	XftFont		*xftfont;	/* Xft version of the font */
#endif
} X_LOADEDFONT;

typedef struct x_loadedcolor_s
{
	struct x_loadedcolor_s *next;	/* next color in linked list */
	unsigned long	pixel;		/* the color code */
	unsigned char 	rgb[3];		/* the color, broken down into RGB */
	CHAR		*name;		/* name of the color */
#ifdef FEATURE_XFT
	XftColor	xftcolor;	/* Xft version of the color */
#endif
} X_LOADEDCOLOR;

#ifdef FEATURE_XFT
XftColor *x_xftpixel P_((long pixel));
#endif
X_LOADEDFONT *x_loadfont P_((char *name));
void x_unloadfont P_((X_LOADEDFONT *font));
unsigned long x_loadcolor P_((CHAR *name, unsigned long def, unsigned char rgb[3]));
void x_unloadcolor P_((unsigned long pixel));
void x_drawbevel P_((X11WIN *xw, Window win, int x, int y, unsigned w, unsigned h, _char_ dir, int height));
void x_drawstring P_((Display *display, Window win, GC gc, int x, int y, char *str, int len));
