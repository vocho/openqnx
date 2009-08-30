/* screen.h */

/* This structure stores information about a strip of rectangles within a
 * screen.  Each STRIP is either horizontal or vertical.  Horizontal STRIPS
 * contain either a single WINDOW, or an array of vertical STRIPs; Vertical
 * strips contain either a single WINDOW or an array of horizontal STRIPs.
 *
 * Each screen contains a single top-level strip.  A flag in the SCREEN
 * structure indicates whether that strip is horizontal or vertical.
 * The orientation of any subSTRIPs is implied from this.
 */
typedef struct strip_s
{
	long	width, height;	/* size of the strip, as a whole */
	long	x, y;		/* position of the strip within screen */
	WINDOW	window;		/* window, or NULL if strip of windows */
	struct strip_s *sub[1];	/* array of subwindows */
} *STRIP;

/* This is the structure of a SCREEN */
typedef struct screen_s
{
	struct screen_s	*next;	/* some other screen, or NULL */

	GUISCR		*guiscr;	/* GUI's screen identifier */
	STRIP		contents;	/* list of windows in the screen */
	ELVBOOL		horizontal;	/* is "contents" horizontally tiled? */

	WINDOW		active;		/* most recent active window in screen */
	OPTVAL		screenwidth;	/* in pixels */
	OPTVAL		screenheight;	/* in pixels */

} *SCREEN;

#define o_screenwidth(scr)	(scr)->screenwidth.value.number
#define o_screenheight(scr)	(scr)->screenheight.value.number

extern void screencreate P_((GUISCR *guiscr, int width, int height));
extern void screendestroy P_((GUISCR *guiscr));
extern void screenfocus P_((GUISCR *guiscr));
extern void screenkey P_((GUISCR *guiscr, _CHAR_ key));
extern void screenmouse P_((GUISCR *guiscr, int x, int y, int what));
