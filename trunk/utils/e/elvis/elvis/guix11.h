/* guix11.h */

#include <sys/types.h>
#include <sys/time.h>
#ifdef NEED_SELECT_H
# include <sys/select.h>
#endif
#include <unistd.h>
#include <fcntl.h>
#include <setjmp.h>
#include <signal.h>
#include <X11/Xlib.h>
#define XK_MISCELLANY
#define XK_XKB_KEYS
#include <X11/keysym.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xresource.h>
#ifdef NEED_XOS_H
# include <X11/Xos.h>
#endif
#ifndef NO_XLOCALE
# include <X11/Intrinsic.h>
# if (XtSpecificationRelease < 6)
#  define NO_XLOCALE
# else
#  include <X11/Xlocale.h>
# endif
#endif	/* NO_XLOCALE */
#ifdef FEATURE_XFT
# include <X11/Xft/Xft.h>
#endif

typedef struct x11win_s X11WIN;

#include "xmisc.h"
#include "xscroll.h"
#include "xtool.h"
#include "xtext.h"
#include "xstatus.h"
#include "xevent.h"
#include "xclip.h"
#include "xdialog.h"

#ifdef FEATURE_IMAGE

# define MAXPIXMAPS	20

extern struct x_bgmap_s
{
	Pixmap	pixmap;	/* the image */
	Pixel	pixel;	/* average color -- affects cursor rule */
	int	width;	/* image width */
	int	height;	/* image height */
} x_bgmap[MAXPIXMAPS];

#define ispixmap(color)		((color) < 0)
#define pixmapof(color)		(x_bgmap[-1 - (color)])

#endif

struct x11win_s
{
	struct x11win_s	*next;	   /* pointer to some other window */
	Window		win;	   /* top-level X window */
	X_SCROLLBAR	sb;	   /* scrollbar info */
	X_TOOLBAR	tb;	   /* toolbar info */
	X_TEXTAREA	ta;	   /* text area info */
	X_STATUSBAR	st;	   /* status bar info */
	GC		gc;	   /* graphic context for this window */
	unsigned long	fg, bg;	   /* current foreground & background */
	ELVBOOL		grexpose;  /* are graphic exposures allow now? */
	ELVBOOL		ismapped;  /* is window visible? */
	ELVBOOL		willraise; /* should next focus raise the window? */
	char		*title;	   /* name of the window */
	ELVISSTATE	state;	   /* command state of window */
	int		x, y;	   /* position of window */
	unsigned int	w, h;	   /* overall size of the window */
	ELVBOOL		nowarp;	   /* don't warp pointer into this window */
#ifndef NO_XLOCALE
	XIC		ic;	   /* input context, for composing chars */
#endif
};

extern Display		*x_display;	/* X11 display */
extern int		x_screen;	/* screen number */
extern Visual		*x_visual;	/* default visual of screen */
extern X11WIN		*x_winlist;	/* list of windows */
extern int		x_depth;	/* bits per pixel */
extern Colormap		x_colormap;	/* colormap shared by elvis windows */
extern ELVBOOL		x_ownselection;	/* does elvis own the X11 selection? */
extern X11WIN		*x_hasfocus;	/* window with kbd. focus, or NULL */
extern unsigned long	x_black;	/* black pixel color */
extern unsigned long	x_white;	/* white pixel color */
extern ELVBOOL		x_mono;		/* is this a monochrome display? */
extern Time		x_now;		/* timestamp of recent event */
extern Atom		x_elvis_cutbuffer;/* id for cut/paste buffer */
extern Atom		x_wm_protocols;	/* value for WM_PROTOCOLS atom */
extern Atom		x_wm_delete_window;/* value for WM_DELETE_WINDOW atom */
extern Atom		x_elvis_server;	/* value for ELVIS_SERVER atom */
extern Atom		x_resource_manager;/* value for MANAGER_RESOURCES atom*/
extern Atom		x_targets;	/* value for TARGETS atom */
extern Atom		x_compound_text;/* value for COMPOUND_TEXT atom */
extern Pixmap		x_gray;			/* gray background for mono */
extern Pixmap		x_elvis_icon;		/* elvis' window icon */
extern Pixmap		x_elvis_pin_icon;	/* elvis icon with pushpin */
extern unsigned		x_elvis_icon_width;	/* width of x_elvis_icon */
extern unsigned		x_elvis_icon_height;	/* height of x_elvis_icon */
extern X_LOADEDFONT	*x_defaultnormal;/* normal font */
extern X_LOADEDFONT	*x_defaultbold;	 /* bold font, or NULL to fake it */
extern X_LOADEDFONT	*x_defaultitalic;/* italic font, or NULL to fake it */
extern X_LOADEDFONT	*x_loadedcontrol;/* toolbar font */

extern void x_reconfig P_((X11WIN *xw, unsigned columns, unsigned rows));

extern struct x_optvals_s
{
	OPTVAL	font, boldfont, italicfont, controlfont, toolbar,
		scrollbarwidth, scrollbartime, scrollbarleft, scrollbar,
		statusbar, dblclicktime, blinktime, xrows, xcolumns,
		firstx, firsty, icon, iconic, iconimage, stopshell,
		autoiconify, altkey, stagger, warpback, warpto, focusnew,
		textcursor, outlinemono, borderwidth, xrootwidth, xrootheight,
		xencoding, scrollwheelspeed, submit, cancel, help, synccursor,
		scrollbgimage;
#ifdef FEATURE_XFT
	OPTVAL	antialias, aasqueeze;
#endif
} x_optvals;
#define o_font		 x_optvals.font.value.string
#define o_boldfont	 x_optvals.boldfont.value.string
#define o_italicfont	 x_optvals.italicfont.value.string
#define o_controlfont	 x_optvals.controlfont.value.string
#define o_toolbar	 x_optvals.toolbar.value.boolean
#define o_scrollbarwidth x_optvals.scrollbarwidth.value.number
#define o_scrollbartime	 x_optvals.scrollbartime.value.number
#define o_scrollbarleft	 x_optvals.scrollbarleft.value.boolean
#define o_scrollbar	 x_optvals.scrollbar.value.boolean
#define o_statusbar	 x_optvals.statusbar.value.boolean
#define o_dblclicktime	 x_optvals.dblclicktime.value.number
#define o_blinktime	 x_optvals.blinktime.value.number
#define o_xrows		 x_optvals.xrows.value.number
#define o_xcolumns	 x_optvals.xcolumns.value.number
#define o_firstx	 x_optvals.firstx.value.number
#define o_firsty	 x_optvals.firsty.value.number
#define o_icon		 x_optvals.icon.value.boolean
#define o_iconic	 x_optvals.iconic.value.boolean
#define o_iconimage	 x_optvals.iconimage.value.string
#define o_stopshell	 x_optvals.stopshell.value.string
#define o_autoiconify	 x_optvals.autoiconify.value.boolean
#define o_altkey	 x_optvals.altkey.value.character
#define o_stagger	 x_optvals.stagger.value.number
#define o_warpback	 x_optvals.warpback.value.boolean
#define o_warpto	 x_optvals.warpto.value.character
#define o_focusnew	 x_optvals.focusnew.value.boolean
#define o_textcursor	 x_optvals.textcursor.value.character
#define o_outlinemono	 x_optvals.outlinemono.value.number
#define o_borderwidth	 x_optvals.borderwidth.value.number
#define o_xrootwidth	 x_optvals.xrootwidth.value.number
#define o_xrootheight	 x_optvals.xrootheight.value.number
#define o_scrollwheelspeed x_optvals.scrollwheelspeed.value.number
#define o_xencoding	 x_optvals.xencoding.value.string
#define o_submit	 x_optvals.submit.value.string
#define o_cancel	 x_optvals.cancel.value.string
#define o_help		 x_optvals.help.value.string
#define o_synccursor	 x_optvals.synccursor.value.boolean
#define o_scrollbgimage	 x_optvals.scrollbgimage.value.boolean
#ifdef FEATURE_XFT
# define o_antialias	 x_optvals.antialias.value.boolean
# define o_aasqueeze	 x_optvals.aasqueeze.value.number
#endif

/* The following store font codes return by colorfind() */
extern int x_cursorcolors;
extern int x_toolcolors;
extern int x_toolbarcolors;
extern int x_scrollcolors;
extern int x_scrollbarcolors;
extern int x_statuscolors;
extern int x_statusbarcolors;
extern int x_guidecolors;
