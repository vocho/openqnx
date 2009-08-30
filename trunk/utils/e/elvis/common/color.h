/* color.h */

typedef struct
{
	CHAR		*name;		/* name of this role */
	CHAR		*descr;		/* args of original :color command */
	long		fg;		/* foreground color code */
	long		bg;		/* background color code */
	char		like;		/* "like" index, or 0 for nothing */
	unsigned char	lpfg_rgb[3];	/* foreground when printing hardcopy */
	DRAWATTR 	da;		/* RGB colors & bitmap of attributes */
} COLORINFO;
#define COLOR_BOLD	 0x0001	/* add the bold attribute to the font */
#define COLOR_ITALIC	 0x0002	/* add the italic attribute to the font */
#define COLOR_UNDERLINED 0x0004	/* draw a line under the text */
#define COLOR_BOXED	 0x0008	/* draw a box around the text */
#define COLOR_GRAPHIC	 0x0010	/* replace text with graphical chars */
#define	COLOR_PROP	 0x0020	/* use proportionally-spaced font */
#define COLOR_PROPSET	 0x0040	/* don't inherit the COLOR_PROP flag */
#define	COLOR_SET	 0x0080	/* attributes were explicitly set by the user */
#define COLOR_FG	 0x0100	/* fg field contains a foreground color */
#define COLOR_BG	 0x0200	/* bg field contains a background color */
#define COLOR_FGSET	 0x0400	/* don't inherit the foreground color */
#define COLOR_BGSET	 0x0800	/* don't inherit the background color */
#define COLOR_LEFTBOX	 0x1000	/* boxed text should draw left edge */
#define COLOR_RIGHTBOX	 0x2000	/* boxed text should draw the right edge */
#define COLOR_SEL	 0x4000	/* text is "selected", in draw.c */

/* Some special font codes */
#define COLOR_FONT_NORMAL	1	/* normal colors */
#define COLOR_FONT_IDLE		2	/* colors for idle screen */
#define COLOR_FONT_BOTTOM	3	/* the bottom row of the window */
#define COLOR_FONT_SELECTION	4	/* colors for selections */
#define COLOR_FONT_HLSEARCH	5	/* colors for hlsearch option */
#define COLOR_FONT_RULER	6	/* colors of the ruler, in text gui */
#define COLOR_FONT_SHOWMODE	7	/* colors of the showmode, in text gui*/
#define COLOR_FONT_LNUM		8	/* colors of the line numbers "lnum" */
#define COLOR_FONT_NONTEXT	9	/* non-text characters such "~" lines */
#define COLOR_FONT_QTY_SPECIALS	10	/* first non-special font code */
/* NOTE: If you add any more special fonts here, you must increase the
 * COLOR_FONT_QTY_SPECIALS value, append your new value to the colorinfo[]
 * definition in color.c, and adjust the colorsortorder[] array in color.c
 */

extern COLORINFO colorinfo[128];
extern int colorsortorder[128];
extern int colornpermanent;
#ifdef FEATURE_IMAGE
extern char *colorimage P_((CHAR *bgname));
#endif
extern int colorisbg P_((OPTDESC *desc, OPTVAL *val, CHAR *newval));
extern int colorfind P_((CHAR *fontname));
extern COLORINFO *colorcombine P_((int oldfont, COLORINFO *newcinfo));
extern void colorparse P_((CHAR *descr, CHAR **fgref, CHAR **bgref, CHAR **likeref, unsigned short *bitsref));
extern void colorset P_((int fontcode, CHAR *descr, ELVBOOL explicit));
extern void colorforeign P_((CHAR *name, CHAR *descr));
extern void colorsetup P_((void));
extern int colortmp P_((int oldfont, int newfont));
extern void colorlist P_((WINDOW win, CHAR *name, ELVBOOL implicit));
#ifdef FEATURE_MKEXRC
extern void colorsave P_((BUFFER buf));
#endif
#ifdef FEATURE_COMPLETE
extern CHAR *colorcomplete P_((WINDOW win, MARK from, MARK to, ELVBOOL nameonly));
#endif
extern _char_ colorexpose P_((_char_ font, DRAWATTR *refattr));
extern int colorrgb2ansi P_((ELVBOOL isfg, unsigned char *rgb));
extern ELVBOOL coloransi P_((int fontcode, CHAR *name, ELVBOOL isfg, long *colorptr, unsigned char *rgb));
extern char *colorname P_((long ansi));
