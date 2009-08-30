/* This file contains stubs of most xlib library functions.
 * This is intended to serve as a reference; there's no point in compiling it.
 * The idea is that you put this file in an accessible but out-of-the-way
 * location, and run ctags on it.  Then you set the TAGPATH environment
 * variable to include this file's directory.  From that point on, you can
 * use the normal tag searching functions to see the declaration for any
 * of these functions.  In particular, the "ref" program distributed with
 * elvis is handy for this.
 *
 * This file is derived from the X11R6 header files.  See the copyright
 * notice at the end of this file.
 */

/* <X11/Xlib.h> */
#define BitmapBitOrder(dpy) 

/* <X11/Xlib.h> */
#define BitmapPad(dpy) 

/* <X11/Xlib.h> */
#define BitmapUnit(dpy) 

/* <X11/Xlib.h> */
#define BlackPixel(dpy, scr) 

/* <X11/Xlib.h> */
#define BlackPixelOfScreen(scr)

/* <X11/Xlib.h> */
#define CellsOfScreen(scr)

/* <X11/Xlib.h> */
#define ConnectionNumber(dpy) 

/* <X11/Xlib.h> */
#define DefaultColormap(dpy, scr)

/* <X11/Xlib.h> */
#define DefaultColormapOfScreen(scr)

/* <X11/Xlib.h> */
#define DefaultDepth(dpy, scr) 

/* <X11/Xlib.h> */
#define DefaultDepthOfScreen(scr)

/* <X11/Xlib.h> */
#define DefaultGC(dpy, scr) 

/* <X11/Xlib.h> */
#define DefaultGCOfScreen(scr)

/* <X11/Xlib.h> */
#define DefaultRootWindow(dpy) 

/* <X11/Xlib.h> */
#define DefaultScreen(dpy) 

/* <X11/Xlib.h> */
#define DefaultScreenOfDisplay(dpy)

/* <X11/Xlib.h> */
#define DefaultVisual(dpy, scr)

/* <X11/Xlib.h> */
#define DefaultVisualOfScreen(scr)

/* <X11/Xlib.h> */
#define DisplayCells(dpy, scr) 

/* <X11/Xlib.h> */
#define DisplayHeight(dpy, scr)

/* <X11/Xlib.h> */
#define DisplayHeightMM(dpy, scr)

/* <X11/Xlib.h> */
#define DisplayOfScreen(scr)

/* <X11/Xlib.h> */
#define DisplayPlanes(dpy, scr)

/* <X11/Xlib.h> */
#define DisplayString(dpy) 

/* <X11/Xlib.h> */
#define DisplayWidth(dpy, scr) 

/* <X11/Xlib.h> */
#define DisplayWidthMM(dpy, scr)

/* <X11/Xlib.h> */
#define DoesBackingStore(scr)

/* <X11/Xlib.h> */
#define DoesSaveUnders(scr)

/* <X11/Xlib.h> */
#define EventMaskOfScreen(scr)

/* <X11/Xlib.h> */
#define HeightMMOfScreen(scr)

/* <X11/Xlib.h> */
#define HeightOfScreen(scr)

/* <X11/Xlib.h> */
#define ImageByteOrder(dpy) 

/* <X11/Xlib.h> */
#define LastKnownRequestProcessed(dpy)

/* <X11/Xlib.h> */
#define MaxCmapsOfScreen(scr)

/* <X11/Xlib.h> */
#define MinCmapsOfScreen(scr)

/* <X11/Xlib.h> */
#define NextRequest(dpy)

/* <X11/Xlib.h> */
#define PlanesOfScreen(scr)

/* <X11/Xlib.h> */
#define ProtocolRevision(dpy) 

/* <X11/Xlib.h> */
#define ProtocolVersion(dpy) 

/* <X11/Xlib.h> */
#define QLength(dpy) 

/* <X11/Xlib.h> */
#define RootWindow(dpy, scr) 

/* <X11/Xlib.h> */
#define RootWindowOfScreen(scr)

/* <X11/Xlib.h> */
#define ScreenCount(dpy) 

/* <X11/Xlib.h> */
#define ScreenOfDisplay(dpy, scr)

/* <X11/Xlib.h> */
#define ServerVendor(dpy) 

/* <X11/Xlib.h> */
#define VendorRelease(dpy) 

/* <X11/Xlib.h> */
#define WhitePixel(dpy, scr) 

/* <X11/Xlib.h> */
#define WhitePixelOfScreen(scr)

/* <X11/Xlib.h> */
#define WidthMMOfScreen(scr)

/* <X11/Xlib.h> */
#define WidthOfScreen(scr)

/* <X11/Xlib.h> */
#define XAllocID(dpy)

/* <X11/Xutil.h> */
#define XDestroyImage(ximage)

/* <X11/Xutil.h> */
#define XGetPixel(ximage, x, y)

/* <X11/Xutil.h> */
#define XPutPixel(ximage, x, y, pixel)

/* <X11/Xutil.h> */
#define XSubImage(ximage, x, y, width, height)

/* <X11/Xutil.h> */
#define XAddPixel(ximage, value)

/* <X11/Xutil.h> */
#define IsKeypadKey(keysym)

/* <X11/Xutil.h> */
#define IsPrivateKeypadKey(keysym)

/* <X11/Xutil.h> */
#define IsCursorKey(keysym)

/* <X11/Xutil.h> */
#define IsPFKey(keysym)

/* <X11/Xutil.h> */
#define IsFunctionKey(keysym)

/* <X11/Xutil.h> */
#define IsMiscFunctionKey(keysym)

/* <X11/Xutil.h> */
#define IsModifierKey(keysym)

/* <X11/Xutil.h> */
#define XUniqueContext()

/* <X11/Xutil.h> */
#define XStringToContext(string)


/* <X11/X.h> */
typedef CARD32 XID;

/* <X11/X.h> */
typedef CARD32 Mask;

/* <X11/X.h> */
typedef CARD32 Atom;

/* <X11/X.h> */
typedef CARD32 VisualID;

/* <X11/X.h> */
typedef CARD32 Time;

/* <X11/X.h> */
typedef XID Window;

/* <X11/X.h> */
typedef XID Drawable;

/* <X11/X.h> */
typedef XID Font;

/* <X11/X.h> */
typedef XID Pixmap;

/* <X11/X.h> */
typedef XID Cursor;

/* <X11/X.h> */
typedef XID Colormap;

/* <X11/X.h>  You probably want GC, not GContext. */
typedef XID GContext;

/* <X11/X.h>  Hardware-independent keyboard codes.  The symbolic names for the
 * codes are defined in <X11/keysym.h>.  E.g., XK_space for the <Spacebar>
 */
typedef XID KeySym;

/* <X11/X.h>  Hardware-dependent keyboard codes */
typedef unsigned char KeyCode;

/* <X11/Xlib.h>  False or True */
typedef int Bool;

/* <X11/Xlib.h>  0 for failure, non-0 for success */
typedef int Status;


/* <X11/Xlib.h> */
typedef struct _XExtData {
	int number;		/* number returned by XRegisterExtension */
	struct _XExtData *next;	/* next item on list of data for structure */
	int (*free_private)(struct _XExtData *extension);
				/* called to free private storage */
	XPointer private_data;	/* data private to this extension. */
} XExtData;

/* <X11/Xlib.h> */
typedef struct {		/* public to extension, cannot be changed */
	int extension;		/* extension number */
	int major_opcode;	/* major op-code assigned by server */
	int first_event;	/* first event number for the extension */
	int first_error;	/* first error number for the extension */
} XExtCodes;

/* <X11/Xlib.h> */
typedef struct {
	int depth;
	int bits_per_pixel;
	int scanline_pad;
} XPixmapFormatValues;

/* <X11/Xlib.h> */
typedef struct {
	int function;		/* logical operation */
	unsigned long plane_mask;/* plane mask */
	unsigned long foreground;/* foreground pixel */
	unsigned long background;/* background pixel */
	int line_width;		/* line width */
	int line_style;	 	/* LineSolid, LineOnOffDash, LineDoubleDash */
	int cap_style;	  	/* CapNotLast, CapButt, 
				   CapRound, CapProjecting */
	int join_style;	 	/* JoinMiter, JoinRound, JoinBevel */
	int fill_style;	 	/* FillSolid, FillTiled, 
				   FillStippled, FillOpaeueStippled */
	int fill_rule;	  	/* EvenOddRule, WindingRule */
	int arc_mode;		/* ArcChord, ArcPieSlice */
	Pixmap tile;		/* tile pixmap for tiling operations */
	Pixmap stipple;		/* stipple 1 plane pixmap for stipping */
	int ts_x_origin;	/* offset for tile or stipple operations */
	int ts_y_origin;
        Font font;	        /* default text font for text operations */
	int subwindow_mode;     /* ClipByChildren, IncludeInferiors */
	Bool graphics_exposures;/* boolean, should exposures be generated */
	int clip_x_origin;	/* origin for clipping */
	int clip_y_origin;
	Pixmap clip_mask;	/* bitmap clipping; other calls for rects */
	int dash_offset;	/* patterned/dashed line information */
	char dashes;
} XGCValues;

/* <X11/Xlib.h> */
typedef struct _XGC
{
	/* Private */
} *GC;

/* <X11/Xlib.h> */
typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	VisualID visualid;	/* visual id of this visual */
	int class;		/* class of screen (monochrome, etc.) */
	unsigned long red_mask, green_mask, blue_mask;	/* mask values */
	int bits_per_rgb;	/* log base 2 of distinct color values */
	int map_entries;	/* color map entries */
} Visual;

/* <X11/Xlib.h> */
typedef struct {
	int depth;		/* this depth (Z) of the depth */
	int nvisuals;		/* number of Visual types at this depth */
	Visual *visuals;	/* list of visuals possible at this depth */
} Depth;

/* <X11/Xlib.h> */
typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	struct _XDisplay *display;/* back pointer to display structure */
	Window root;		/* Root window id. */
	int width, height;	/* width and height of screen */
	int mwidth, mheight;	/* width and height of  in millimeters */
	int ndepths;		/* number of depths possible */
	Depth *depths;		/* list of allowable depths on the screen */
	int root_depth;		/* bits per pixel */
	Visual *root_visual;	/* root visual */
	GC default_gc;		/* GC for the root root visual */
	Colormap cmap;		/* default color map */
	unsigned long white_pixel;
	unsigned long black_pixel;	/* White and Black pixel values */
	int max_maps, min_maps;	/* max and min color maps */
	int backing_store;	/* Never, WhenMapped, Always */
	Bool save_unders;	
	long root_input_mask;	/* initial root input mask */
} Screen;

/* <X11/Xlib.h> */
typedef struct {
	XExtData *ext_data;	/* hook for extension to hang data */
	int depth;		/* depth of this image format */
	int bits_per_pixel;	/* bits/pixel at this depth */
	int scanline_pad;	/* scanline must padded to this multiple */
} ScreenFormat;

/* <X11/Xlib.h> */
typedef struct {
    Pixmap background_pixmap;	/* background or None or ParentRelative */
    unsigned long background_pixel;	/* background pixel */
    Pixmap border_pixmap;	/* border of the window */
    unsigned long border_pixel;	/* border pixel value */
    int bit_gravity;		/* one of bit gravity values */
    int win_gravity;		/* one of the window gravity values */
    int backing_store;		/* NotUseful, WhenMapped, Always */
    unsigned long backing_planes;/* planes to be preseved if possible */
    unsigned long backing_pixel;/* value to use in restoring planes */
    Bool save_under;		/* should bits under be saved? (popups) */
    long event_mask;		/* set of events that should be saved */
    long do_not_propagate_mask;	/* set of events that should not propagate */
    Bool override_redirect;	/* boolean value for override-redirect */
    Colormap colormap;		/* color map to be associated with window */
    Cursor cursor;		/* cursor to be displayed (or None) */
} XSetWindowAttributes;

/* <X11/Xlib.h> */
typedef struct {
    int x, y;			/* location of window */
    int width, height;		/* width and height of window */
    int border_width;		/* border width of window */
    int depth;          	/* depth of window */
    Visual *visual;		/* the associated visual structure */
    Window root;        	/* root of screen containing window */
    int class;			/* InputOutput, InputOnly*/
    int bit_gravity;		/* one of bit gravity values */
    int win_gravity;		/* one of the window gravity values */
    int backing_store;		/* NotUseful, WhenMapped, Always */
    unsigned long backing_planes;/* planes to be preserved if possible */
    unsigned long backing_pixel;/* value to be used when restoring planes */
    Bool save_under;		/* boolean, should bits under be saved? */
    Colormap colormap;		/* color map to be associated with window */
    Bool map_installed;		/* boolean, is color map currently installed*/
    int map_state;		/* IsUnmapped, IsUnviewable, IsViewable */
    long all_event_masks;	/* set of events all people have interest in*/
    long your_event_mask;	/* my event mask */
    long do_not_propagate_mask; /* set of events that should not propagate */
    Bool override_redirect;	/* boolean value for override-redirect */
    Screen *screen;		/* back pointer to correct screen */
} XWindowAttributes;

/* <X11/Xlib.h> */
typedef struct {
	int family;		/* for example FamilyInternet */
	int length;		/* length of address, in bytes */
	char *address;		/* pointer to where to find the bytes */
} XHostAddress;

/* <X11/Xlib.h> */
typedef struct _XImage {
    int width, height;		/* size of image */
    int xoffset;		/* number of pixels offset in X direction */
    int format;			/* XYBitmap, XYPixmap, ZPixmap */
    char *data;			/* pointer to image data */
    int byte_order;		/* data byte order, LSBFirst, MSBFirst */
    int bitmap_unit;		/* quant. of scanline 8, 16, 32 */
    int bitmap_bit_order;	/* LSBFirst, MSBFirst */
    int bitmap_pad;		/* 8, 16, 32 either XY or ZPixmap */
    int depth;			/* depth of image */
    int bytes_per_line;		/* accelarator to next line */
    int bits_per_pixel;		/* bits per pixel (ZPixmap) */
    unsigned long red_mask;	/* bits in z arrangment */
    unsigned long green_mask;
    unsigned long blue_mask;
    XPointer obdata;		/* hook for the object routines to hang on */
    struct funcs {		/* image manipulation routines */
	struct _XImage *(*create_image)(struct _XDisplay*, Visual*, unsigned int, int, int, char*, unsigned int, unsigned int, int, int);
	int (*destroy_image)        (struct _XImage *);
	unsigned long (*get_pixel)  (struct _XImage *, int, int);
	int (*put_pixel)            (struct _XImage *, int, int, unsigned long);
	struct _XImage *(*sub_image)(struct _XImage *, int, int, unsigned int, unsigned int);
	int (*add_pixel)            (struct _XImage *, long);
	} f;
} XImage;

/* <X11/Xlib.h> */
typedef struct {
    int x, y;
    int width, height;
    int border_width;
    Window sibling;
    int stack_mode;
} XWindowChanges;

/* <X11/Xlib.h> */
typedef struct {
	unsigned long pixel;
	unsigned short red, green, blue;
	char flags;  /* DoRed, DoGreen, DoBlue */
	char pad;
} XColor;

/* <X11/Xlib.h> */
typedef struct {
    short x1, y1, x2, y2;
} XSegment;

/* <X11/Xlib.h> */
typedef struct {
    short x, y;
} XPoint;
    
/* <X11/Xlib.h> */
typedef struct {
    short x, y;
    unsigned short width, height;
} XRectangle;
    
/* <X11/Xlib.h> */
typedef struct {
    short x, y;
    unsigned short width, height;
    short angle1, angle2;
} XArc;

/* <X11/Xlib.h> */
typedef struct {
        int key_click_percent;
        int bell_percent;
        int bell_pitch;
        int bell_duration;
        int led;
        int led_mode;
        int key;
        int auto_repeat_mode;   /* On, Off, Default */
} XKeyboardControl;

/* <X11/Xlib.h> */
typedef struct {
        int key_click_percent;
	int bell_percent;
	unsigned int bell_pitch, bell_duration;
	unsigned long led_mask;
	int global_auto_repeat;
	char auto_repeats[32];
} XKeyboardState;

/* <X11/Xlib.h> */
typedef struct {
        Time time;
	short x, y;
} XTimeCoord;

/* <X11/Xlib.h> */
typedef struct {
 	int max_keypermod;	/* The server's max # of keys per modifier */
 	KeyCode *modifiermap;	/* An 8 by max_keypermod array of modifiers */
} XModifierKeymap;


/* <X11/Xlib.h> */
typedef struct {
	/* Private */
} Display;

/* <X11/Xlib.h> */
typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window it is reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time; milliseconds
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	unsigned int keycode; detail
	Bool same_screen;	/* same screen flag */
} XKeyEvent;

/* <X11/Xlib.h> */
typedef XKeyEvent XKeyPressedEvent;

/* <X11/Xlib.h> */
typedef XKeyEvent XKeyReleasedEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window it is reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time; milliseconds
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	unsigned int button; detail
	Bool same_screen;	/* same screen flag */
} XButtonEvent;

/* <X11/Xlib.h> */
typedef XButtonEvent XButtonPressedEvent;

/* <X11/Xlib.h> */
typedef XButtonEvent XButtonReleasedEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time; milliseconds
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	unsigned int state;	/* key or button mask */
	char is_hint; detail
	Bool same_screen;	/* same screen flag */
} XMotionEvent;

/* <X11/Xlib.h> */
typedef XMotionEvent XPointerMovedEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;		/* of event */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;	        /* "event" window reported relative to */
	Window root;	        /* root window that the event occurred on */
	Window subwindow;	/* child window */
	Time time; 		/* milliseconds */
	int x, y;		/* pointer x, y coordinates in event window */
	int x_root, y_root;	/* coordinates relative to root */
	int mode;		/* NotifyNormal, NotifyGrab, NotifyUngrab */
	int detail;		/* NotifyAncestor, NotifyVirtual,
				 * NotifyInferior, NotifyNonlinear,
				 * NotifyNonlinearVirtual */
	Bool same_screen;	/* same screen flag */
	Bool focus;		/* boolean focus */
	unsigned int state;	/* key or button mask */
} XCrossingEvent;

/* <X11/Xlib.h> */
typedef XCrossingEvent XEnterWindowEvent;

/* <X11/Xlib.h> */
typedef XCrossingEvent XLeaveWindowEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;		/* FocusIn or FocusOut */
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;		/* window of event */
	int mode;		/* NotifyNormal, NotifyGrab, NotifyUngrab */
	int detail;		/* NotifyAncestor, NotifyVirtual,
				 * NotifyInferior, NotifyNonlinear,
				 * NotifyNonlinearVirtual, NotifyPointer,
				 * NotifyPointerRoot, NotifyDetailNone */
} XFocusChangeEvent;

/* <X11/Xlib.h> */
typedef XFocusChangeEvent XFocusInEvent;

/* <X11/Xlib.h> */
typedef XFocusChangeEvent XFocusOutEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	char key_vector[32];
} XKeymapEvent;	

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	int x, y;
	int width, height;
	int count;		/* if non-zero, at least this many more */
} XExposeEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Drawable drawable;
	int x, y;
	int width, height;
	int count;		/* if non-zero, at least this many more */
	int major_code;		/* core is CopyArea or CopyPlane */
	int minor_code;		/* not defined in the core */
} XGraphicsExposeEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Drawable drawable;
	int major_code;		/* core is CopyArea or CopyPlane */
	int minor_code;		/* not defined in the core */
} XNoExposeEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	int state;		/* Visibility state */
} XVisibilityEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;		/* parent of the window */
	Window window;		/* window id of window created */
	int x, y;		/* window location */
	int width, height;	/* size of window */
	int border_width;	/* border width */
	Bool override_redirect;	/* creation should be overridden */
} XCreateWindowEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
} XDestroyWindowEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	Bool from_configure;
} XUnmapEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	Bool override_redirect;	/* boolean, is override set... */
} XMapEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;
	Window window;
} XMapRequestEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	Window parent;
	int x, y;
	Bool override_redirect;
} XReparentEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	int x, y;
	int width, height;
	int border_width;
	Window above;
	Bool override_redirect;
} XConfigureEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	int x, y;
} XGravityEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	int width, height;
} XResizeRequestEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;
	Window window;
	int x, y;
	int width, height;
	int border_width;
	Window above;
	int detail;		/* Above, Below, TopIf, BottomIf, Opposite */
	unsigned long value_mask;
} XConfigureRequestEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window event;
	Window window;
	int place;		/* PlaceOnTop, PlaceOnBottom */
} XCirculateEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window parent;
	Window window;
	int place;		/* PlaceOnTop, PlaceOnBottom */
} XCirculateRequestEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Atom atom;
	Time time;
	int state;		/* NewValue, Deleted */
} XPropertyEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Atom selection;
	Time time;
} XSelectionClearEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window owner;
	Window requestor;
	Atom selection;
	Atom target;
	Atom property;
	Time time;
} XSelectionRequestEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window requestor;
	Atom selection;
	Atom target;
	Atom property;		/* ATOM or None */
	Time time;
} XSelectionEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Colormap colormap;	/* COLORMAP or None */
	Bool new;
	int state;		/* ColormapInstalled, ColormapUninstalled */
} XColormapEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window;
	Atom message_type;
	int format;
	union {
		char b[20];
		short s[10];
		long l[5];
		} data;
} XClientMessageEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;	/* Display the event was read from */
	Window window; unused
	int request;		/* one of MappingModifier, MappingKeyboard,
				   MappingPointer */
	int first_keycode;	/* first keycode */
	int count;		/* defines range of change w. first_keycode*/
} XMappingEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	Display *display;	/* Display the event was read from */
	XID resourceid;		/* resource id */
	unsigned long serial;	/* serial number of failed request */
	unsigned char error_code;	/* error code of failed request */
	unsigned char request_code;	/* Major op-code of failed request */
	unsigned char minor_code;	/* Minor op-code of failed request */
} XErrorEvent;

/* <X11/Xlib.h> */
typedef struct {
	int type;
	unsigned long serial;	/* # of last request processed by server */
	Bool send_event;	/* true if this came from a SendEvent request */
	Display *display;/* Display the event was read from */
	Window window;	/* window on which event was requested in event mask */
} XAnyEvent;

/*
 * this union is defined so Xlib can always use the same sized
 * event structure internally, to avoid memory fragmentation.
 */
/* <X11/Xlib.h> */
typedef union _XEvent {
        int type;		/* All events begin with a "type" field */
	XAnyEvent xany;
	XKeyEvent xkey;
	XButtonEvent xbutton;
	XMotionEvent xmotion;
	XCrossingEvent xcrossing;
	XFocusChangeEvent xfocus;
	XExposeEvent xexpose;
	XGraphicsExposeEvent xgraphicsexpose;
	XNoExposeEvent xnoexpose;
	XVisibilityEvent xvisibility;
	XCreateWindowEvent xcreatewindow;
	XDestroyWindowEvent xdestroywindow;
	XUnmapEvent xunmap;
	XMapEvent xmap;
	XMapRequestEvent xmaprequest;
	XReparentEvent xreparent;
	XConfigureEvent xconfigure;
	XGravityEvent xgravity;
	XResizeRequestEvent xresizerequest;
	XConfigureRequestEvent xconfigurerequest;
	XCirculateEvent xcirculate;
	XCirculateRequestEvent xcirculaterequest;
	XPropertyEvent xproperty;
	XSelectionClearEvent xselectionclear;
	XSelectionRequestEvent xselectionrequest;
	XSelectionEvent xselection;
	XColormapEvent xcolormap;
	XClientMessageEvent xclient;
	XMappingEvent xmapping;
	XErrorEvent xerror;
	XKeymapEvent xkeymap;
	long pad[24];
} XEvent;


/* <X11/Xlib.h> */
typedef struct {
	short		lbearing;	/* origin to left edge of raster */
	short		rbearing;	/* origin to right edge of raster */
	short		width;		/* advance to next char's origin */
	short		ascent;		/* baseline to top edge of raster */
	short		descent;	/* baseline to bottom edge of raster */
	unsigned short	attributes;	/* per char flags (not predefined) */
} XCharStruct;

/* <X11/Xlib.h> */
typedef struct {
	Atom name;
	unsigned long card32;
} XFontProp;

/* <X11/Xlib.h> */
typedef struct {
	XExtData	*ext_data;	/* hook for extension to hang data */
	Font		fid;            /* Font id for this font */
	unsigned	direction;	/* hint about direction the font is painted */
	unsigned	min_char_or_byte2;/* first character */
	unsigned	max_char_or_byte2;/* last character */
	unsigned	min_byte1;	/* first row that exists */
	unsigned	max_byte1;	/* last row that exists */
	Bool		all_chars_exist;/* flag if all characters have non-zero size*/
	unsigned	default_char;	/* char to print for undefined character */
	int		n_properties;   /* how many properties there are */
	XFontProp	*properties;	/* pointer to array of additional properties*/
	XCharStruct	min_bounds;	/* minimum bounds over all existing char*/
	XCharStruct	max_bounds;	/* maximum bounds over all existing char*/
	XCharStruct	*per_char;	/* first_char to last_char information */
	int		ascent;		/* log. extent above baseline for spacing */
	int		descent;	/* log. descent below baseline for spacing */
} XFontStruct;

/* <X11/Xlib.h> */
typedef struct {
	char *chars;		/* pointer to string */
	int nchars;			/* number of characters */
	int delta;			/* delta between strings */
	Font font;			/* font to print it in, None don't change */
} XTextItem;

/* <X11/Xlib.h> */
typedef struct {		/* normal 16 bit characters are two bytes */
	unsigned char byte1;
	unsigned char byte2;
} XChar2b;

/* <X11/Xlib.h> */
typedef struct {
	XChar2b *chars;		/* two byte characters */
	int nchars;			/* number of characters */
	int delta;			/* delta between strings */
	Font font;			/* font to print it in, None don't change */
} XTextItem16;


/* <X11/Xlib.h> */
typedef union {
	Display *display;
	GC gc;
	Visual *visual;
	Screen *screen;
	ScreenFormat *pixmap_format;
	XFontStruct *font;
} XEDataObject;

/* <X11/Xlib.h> */
typedef struct {
	XRectangle      max_ink_extent;
	XRectangle      max_logical_extent;
} XFontSetExtents;

/* <X11/Xlib.h> */
typedef struct {
	char           *chars;
	int             nchars;
	int             delta;
	XFontSet        font_set;
} XmbTextItem;

/* <X11/Xlib.h> */
typedef struct {
	wchar_t        *chars;
	int             nchars;
	int             delta;
	XFontSet        font_set;
} XwcTextItem;

/* <X11/Xlib.h> */
typedef struct {
	int charset_count;
	char **charset_list;
} XOMCharSetList;

/* <X11/Xlib.h> */
typedef enum {
	XOMOrientation_LTR_TTB,
	XOMOrientation_RTL_TTB,
	XOMOrientation_TTB_LTR,
	XOMOrientation_TTB_RTL,
	XOMOrientation_Context
} XOrientation;

/* <X11/Xlib.h> */
typedef struct {
	int num_orientation;
	XOrientation *orientation;	/* Input Text description */
    } XOMOrientation;

/* <X11/Xlib.h> */
typedef struct {
	int num_font;
	XFontStruct **font_struct_list;
	char **font_name_list;
} XOMFontInfo;

/* <X11/Xlib.h> */
typedef struct _XIM *XIM;
/* <X11/Xlib.h> */
typedef struct _XIC *XIC;

/* <X11/Xlib.h> */
typedef void (*XIMProc)(XIM, XPointer, XPointer);

/* <X11/Xlib.h> */
typedef Bool (*XICProc)(XIC, XPointer, XPointer);

/* <X11/Xlib.h> */
typedef void (*XIDProc)(Display*, XPointer, XPointer);

/* <X11/Xlib.h> */
typedef unsigned long XIMStyle;

/* <X11/Xlib.h> */
typedef struct {
	unsigned short count_styles;
	XIMStyle *supported_styles;
} XIMStyles;

/* <X11/Xlib.h> */
typedef void *XVaNestedList;

/* <X11/Xlib.h> */
typedef struct {
	XPointer client_data;
	XIMProc callback;
} XIMCallback;

/* <X11/Xlib.h> */
typedef struct {
	XPointer client_data;
	XICProc callback;
} XICCallback;

/* <X11/Xlib.h> */
typedef unsigned long XIMFeedback;

/* <X11/Xlib.h> */
typedef struct _XIMText {
	unsigned short length;
	XIMFeedback *feedback;
	Bool encoding_is_wchar; 
	union {
		char *multi_byte;
		wchar_t *wide_char;
	} string; 
} XIMText;

/* <X11/Xlib.h> */
typedef	unsigned long	 XIMPreeditState;

/* <X11/Xlib.h> */
typedef	struct	_XIMPreeditStateNotifyCallbackStruct {
	XIMPreeditState state;
} XIMPreeditStateNotifyCallbackStruct;

/* <X11/Xlib.h> */
typedef	unsigned long	 XIMResetState;

/* <X11/Xlib.h> */
typedef unsigned long XIMStringConversionFeedback;

/* <X11/Xlib.h> */
typedef struct _XIMStringConversionText {
	unsigned short length;
	XIMStringConversionFeedback *feedback;
	Bool encoding_is_wchar; 
	union {
		char *mbs;
		wchar_t *wcs;
	} string; 
} XIMStringConversionText;

/* <X11/Xlib.h> */
typedef	unsigned short	XIMStringConversionPosition;

/* <X11/Xlib.h> */
typedef	unsigned short	XIMStringConversionType;

/* <X11/Xlib.h> */
typedef	unsigned short	XIMStringConversionOperation;

/* <X11/Xlib.h> */
typedef enum {
	XIMForwardChar, XIMBackwardChar, XIMForwardWord, XIMBackwardWord,
	XIMCaretUp, XIMCaretDown, XIMNextLine, XIMPreviousLine,
	XIMLineStart, XIMLineEnd, XIMAbsolutePosition, XIMDontChange
} XIMCaretDirection;

/* <X11/Xlib.h> */
typedef struct _XIMStringConversionCallbackStruct {
	XIMStringConversionPosition	position;
	XIMCaretDirection		direction;
	XIMStringConversionOperation	operation;
	unsigned short			factor;
	XIMStringConversionText		*text;
} XIMStringConversionCallbackStruct;

/* <X11/Xlib.h> */
typedef struct _XIMPreeditDrawCallbackStruct {
	int caret;	/* Cursor offset within pre-edit string */
	int chg_first;	/* Starting change position */
	int chg_length;	/* Length of the change in character count */
	XIMText *text;
} XIMPreeditDrawCallbackStruct;

/* <X11/Xlib.h> */
typedef enum {
	XIMIsInvisible,	/* Disable caret feedback */ 
	XIMIsPrimary,	/* UI defined caret feedback */
	XIMIsSecondary	/* UI defined caret feedback */
} XIMCaretStyle;

/* <X11/Xlib.h> */
typedef struct _XIMPreeditCaretCallbackStruct {
	int position;			/* Caret offset within pre-edit string*/
	XIMCaretDirection direction;	/* Caret moves direction */
	XIMCaretStyle style;		/* Feedback of the caret */
} XIMPreeditCaretCallbackStruct;

/* <X11/Xlib.h> */
typedef enum {
	XIMTextType, XIMBitmapType
} XIMStatusDataType;
	
/* <X11/Xlib.h> */
typedef struct _XIMStatusDrawCallbackStruct {
	XIMStatusDataType type;
	union {
		XIMText *text;
		Pixmap  bitmap;
	} data;
} XIMStatusDrawCallbackStruct;

/* <X11/Xlib.h> */
typedef struct _XIMHotKeyTrigger {
	KeySym	 keysym;
	int	 modifier;
	int	 modifier_mask;
} XIMHotKeyTrigger;

/* <X11/Xlib.h> */
typedef struct _XIMHotKeyTriggers {
	int			num_hot_key;
	XIMHotKeyTrigger	*key;
} XIMHotKeyTriggers;

/* <X11/Xlib.h> */
typedef	unsigned long	 XIMHotKeyState;

/* <X11/Xlib.h> */
typedef struct {
	unsigned short count_values;
	char **supported_values;
} XIMValuesList;

/* <X11/Xlib.h> */
typedef void (*XConnectionWatchProc)(Display* dpy, XPointer client_data, int fd, Bool opening, XPointer* watch_data);

/* <X11/Xutil.h> */
typedef struct {
    	long flags;	/* marks which fields in this structure are defined */
	int x, y;		/* obsolete for new window mgrs, but clients */
	int width, height;	/* should set so old wm's don't mess up */
	int min_width, min_height;
	int max_width, max_height;
    	int width_inc, height_inc;
	struct {
		int x;		/* numerator */
		int y;		/* denominator */
	} min_aspect, max_aspect;
	int base_width, base_height;		/* added by ICCCM version 1 */
	int win_gravity;			/* added by ICCCM version 1 */
} XSizeHints;

/* <X11/Xutil.h> */
typedef struct {
	long flags;	/* marks which fields in this structure are defined */
	Bool input;	/* does this application rely on the window manager
			 * to get keyboard input? */
	int initial_state;	/* see below */
	Pixmap icon_pixmap;	/* pixmap to be used as icon */
	Window icon_window; 	/* window to be used as icon */
	int icon_x, icon_y; 	/* initial position of icon */
	Pixmap icon_mask;	/* icon mask bitmap */
	XID window_group;	/* id of related window group */
} XWMHints;

/* <X11/Xutil.h> */
typedef struct {
    unsigned char *value;		/* same as Property routines */
    Atom encoding;			/* prop type */
    int format;				/* prop data format: 8, 16, or 32 */
    unsigned long nitems;		/* number of data items in value */
} XTextProperty;

/* <X11/Xutil.h> */
typedef enum {
    XStringStyle, STRING
    XCompoundTextStyle, COMPOUND_TEXT
    XTextStyle,			/* text in owner's encoding (current locale)*/
    XStdICCTextStyle		/* STRING, else COMPOUND_TEXT */
} XICCEncodingStyle;

/* <X11/Xutil.h> */
typedef struct {
	int min_width, min_height;
	int max_width, max_height;
	int width_inc, height_inc;
} XIconSize;

/* <X11/Xutil.h> */
typedef struct {
	char *res_name;
	char *res_class;
} XClassHint;

/* <X11/Xutil.h> */
typedef struct _XComposeStatus {
    XPointer compose_ptr;	/* state table pointer */
    int chars_matched;		/* match state */
} XComposeStatus;

/* <X11/Xutil.h> */
typedef struct {
	Visual		*visual;
	VisualID	visualid;
	int		screen;
	int		depth;
	int		class;
	unsigned long	red_mask;
	unsigned long	green_mask;
	unsigned long	blue_mask;
	int		colormap_size;
	int		bits_per_rgb;
} XVisualInfo;

/* <X11/Xutil.h> */
typedef struct {
	Colormap colormap;
	unsigned long red_max;
	unsigned long red_mult;
	unsigned long green_max;
	unsigned long green_mult;
	unsigned long blue_max;
	unsigned long blue_mult;
	unsigned long base_pixel;
	VisualID visualid;		/* added by ICCCM version 1 */
	XID killid;			/* added by ICCCM version 1 */
} XStandardColormap;

/* <X11/Xutil.h> */
typedef int XContext;

/* <X11/Xlib.h> Set protocol error handler, where XErrorHandler is a pointer
 * to function which accepts a Display* and an XErrorEvent* as arguments, and
 * returns an int: typedef int (XErrorHandler)(Display*, XErrorEvent*);
 */
XErrorHandler XSetErrorHandler (XErrorHandler handler)
{
}

/* <X11/Xlib.h> Set I/O error handler, where XIOErrorHandler is a pointer to
 * a function which accepts a Display* as its argument, and returns an int:
 * typedef int (XIOErrorHandler)(Display*);
 */
XIOErrorHandler XSetIOErrorHandler (XIOErrorHandler handler)
{
}


/* <X11/auth.h> */
char *XauFileName(void)
{
}

/* <X11/auth.h> */
Xauth *XauReadAuth(FILE* auth_file)
{
}

/* <X11/auth.h> */
int XauLockAuth(const char* file_name, int retries, int timeout, long dead)
{
}

/* <X11/auth.h> */
int XauUnlockAuth(const char* file_name)
{
}

/* <X11/auth.h> */
int XauWriteAuth(FILE* auth_file, Xauth* auth)
{
}

/* <X11/auth.h> */
Xauth *XauGetAuthByName(const char* display_name)
{
}

/* <X11/xauth.h> */
Xauth *XauGetAuthByAddr(unsigned short family, unsigned short address_length, const char* address, unsigned short number_length, const char* number, unsigned short name_length, const char* name)
{
}

/* <X11/xauth.h> */
Xauth *XauGetBestAuthByAddr(unsigned short family, unsigned short address_length, const char* address, unsigned short number_length, const char* number, int types_length, char** type_names, const int* type_lengths)
{
}

/* <X11/xauth.h> */
void XauDisposeAuth(Xauth* auth)
{
}

/* <X11/xauth.h> */
int XauKrb5Encode(krb5_principal princ, krb5_data *outbuf)
{
}

/* <X11/xauth.h> */
int XauKrb5Decode(krb5_data inbuf, krb5_principal *princ)
{
}

/* <X11/Xlib.h> */
XFontStruct *XLoadQueryFont(Display *display, const char *name)
{
}

/* <X11/Xlib.h> */
XFontStruct *XQueryFont(Display *display, XID font_ID)
{
}

/* <X11/Xlib.h> */
XTimeCoord *XGetMotionEvents(Display *display, Window w, Time start, Time stop, int *nevents_return)
{
}

/* <X11/Xlib.h> */
XModifierKeymap *XDeleteModifiermapEntry(XModifierKeymap *modmap, KeyCode keycode_entry, int modifier)
{
}

/* <X11/Xlib.h> */
XModifierKeymap *XGetModifierMapping(Display *display)
{
}

/* <X11/Xlib.h> */
XModifierKeymap	*XInsertModifiermapEntry(XModifierKeymap *modmap, KeyCode keycode_entry, int modifier)
{
}

/* <X11/Xlib.h> */
XModifierKeymap *XNewModifiermap(int max_keys_per_mod)
{
}

/* <X11/Xlib.h> */
XImage *XCreateImage(Display *display, Visual *visual, unsigned int depth, int format, int offset, char *data, unsigned int width, unsigned int height, int bitmap_pad, int bytes_per_line)
{
}

/* <X11/Xlib.h> */
Status XInitImage(XImage *image)
{
}

/* <X11/Xlib.h> */
XImage *XGetImage(Display *display, Drawable d, int x, int y, unsigned int width, unsigned int height, unsigned long plane_mask, int format)
{
}

/* <X11/Xlib.h> */
XImage *XGetSubImage(Display *display, Drawable d, int x, int y, unsigned int width, unsigned int height, unsigned long plane_mask, int format, XImage *dest_image, int dest_x, int dest_y)
{
}

/* <X11/Xlib.h> */
Display *XOpenDisplay(const char *display_name)
{
}

/* <X11/Xlib.h> */
void XrmInitialize(void)
{
}

/* <X11/Xlib.h> */
char *XFetchBytes(Display *display, int *nbytes_return)
{
}

/* <X11/Xlib.h> */
char *XFetchBuffer(Display *display, int *nbytes_return, int buffer)
{
}

/* <X11/Xlib.h> */
char *XGetAtomName(Display *display, Atom atom)
{
}

/* <X11/Xlib.h> */
Status XGetAtomNames(Display *dpy, Atom *atoms, int count, char **names_return)
{
}

/* <X11/Xlib.h> */
char *XGetDefault(Display *display, const char *program, const char *option)
{
}

/* <X11/Xlib.h> */
char *XDisplayName(const char *string)
{
}

/* <X11/Xlib.h> */
char *XKeysymToString(KeySym keysym)
{
}

/* <X11/Xlib.h> */
int (*XSynchronize(Display *display, Bool onoff))(Display *display)
{
}

/* <X11/Xlib.h> */
int (*XSetAfterFunction(Display *display, int (*) (Display *display) procedure))(Display *display)
{
}

/* <X11/Xlib.h> */
Atom XInternAtom(Display *display, const char *atom_name, Bool only_if_exists)
{
}

/* <X11/Xlib.h> */
Status XInternAtoms(Display *dpy, char **names, int count, Bool onlyIfExists, Atom *atoms_return)
{
}

/* <X11/Xlib.h> */
Colormap XCopyColormapAndFree(Display *display, Colormap colormap)
{
}

/* <X11/Xlib.h> */
Colormap XCreateColormap(Display *display, Window w, Visual *visual, int	alloc)
{
}

/* <X11/Xlib.h> */
Cursor XCreatePixmapCursor(Display *display, Pixmap source, Pixmap mask, XColor *foreground_color, XColor *background_color, unsigned int x, unsigned int y)
{
}

/* <X11/Xlib.h> */
Cursor XCreateGlyphCursor(Display *display, Font source_font, Font mask_font, unsigned int source_char, unsigned int mask_char, XColor *foreground_color, XColor *background_color)
{
}

/* <X11/Xlib.h> */
Cursor XCreateFontCursor(Display *display, unsigned int shape)
{
}

/* <X11/Xlib.h> */
Font XLoadFont(Display *display, const char *name)
{
}

/* <X11/Xlib.h> */
GC XCreateGC(Display *display, Drawable d, unsigned long valuemask, XGCValues *values)
{
}

/* <X11/Xlib.h> */
GContext XGContextFromGC(GC gc)
{
}

/* <X11/Xlib.h> */
void XFlushGC(Display *display, GC gc)
{
}

/* <X11/Xlib.h> */
Pixmap XCreatePixmap(Display *display, Drawable d, unsigned int width, unsigned int height, unsigned int depth)
{
}

/* <X11/Xlib.h> */
Pixmap XCreateBitmapFromData(Display *display, Drawable d, const char *data, unsigned int width, unsigned int height)
{
}

/* <X11/Xlib.h> */
Pixmap XCreatePixmapFromBitmapData(Display *display, Drawable d, char *data, unsigned int width, unsigned int height, unsigned long fg, unsigned long bg, unsigned int depth)
{
}

/* <X11/Xlib.h> */
Window XCreateSimpleWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, unsigned long border, unsigned long background)
{
}

/* <X11/Xlib.h> */
Window XGetSelectionOwner(Display *display, Atom selection)
{
}

/* <X11/Xlib.h> */
Window XCreateWindow(Display *display, Window parent, int x, int y, unsigned int width, unsigned int height, unsigned int border_width, int depth, unsigned int class, Visual *visual, unsigned long valuemask, XSetWindowAttributes *attributes)
{
}

/* <X11/Xlib.h> */
Colormap *XListInstalledColormaps(Display *display, Window w, int *num_return)
{
}

/* <X11/Xlib.h> */
char **XListFonts(Display *display, const char *pattern, int maxnames, int *actual_count_return)
{
}

/* <X11/Xlib.h> */
char **XListFontsWithInfo(Display *display, const char *pattern, int maxnames, int *count_return, XFontStruct **info_return)
{
}

/* <X11/Xlib.h> */
char **XGetFontPath(Display *display, int *npaths_return)
{
}

/* <X11/Xlib.h> */
char **XListExtensions(Display *display, int *nextensions_return)
{
}

/* <X11/Xlib.h> */
Atom *XListProperties(Display *display, Window w, int *num_prop_return)
{
}

/* <X11/Xlib.h> */
XHostAddress *XListHosts(Display *display, int *nhosts_return, Bool *state_return)
{
}

/* <X11/Xlib.h> */
KeySym XKeycodeToKeysym(Display *display, KeyCode keycode, int index)
{
}

/* <X11/Xlib.h> */
KeySym XLookupKeysym(XKeyEvent *key_event, int index)
{
}

/* <X11/Xlib.h> */
KeySym *XGetKeyboardMapping(Display *display, KeyCode first_keycode, int keycode_count, int *keysyms_per_keycode_return)
{
}

/* <X11/Xlib.h> */
KeySym XStringToKeysym(const char *string)
{
}

/* <X11/Xlib.h> */
long XMaxRequestSize(Display *display)
{
}

/* <X11/Xlib.h> */
long XExtendedMaxRequestSize(Display *display)
{
}

/* <X11/Xlib.h> */
char *XResourceManagerString(Display *display)
{
}

/* <X11/Xlib.h> */
char *XScreenResourceString(Screen *screen)
{
}

/* <X11/Xlib.h> */
unsigned long XDisplayMotionBufferSize(Display *display)
{
}

/* <X11/Xlib.h> */
VisualID XVisualIDFromVisual(Visual *visual)
{
}

/* <X11/Xlib.h> */
Status XInitThreads(void)
{
}

/* <X11/Xlib.h> */
void XLockDisplay(Display *display)
{
}

/* <X11/Xlib.h> */
void XUnlockDisplay(Display *display)
{
}

/* <X11/Xlib.h> */
XExtCodes *XInitExtension(Display *display, const char *name)
{
}

/* <X11/Xlib.h> */
XExtCodes *XAddExtension(Display *display)
{
}

/* <X11/Xlib.h> */
XExtData *XFindOnExtensionList(XExtData **structure, int number)
{
}

/* <X11/Xlib.h> */
XExtData **XEHeadOfExtensionList(XEDataObject object)
{
}

/* <X11/Xlib.h> */
Window XRootWindow(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
Window XDefaultRootWindow(Display *display)
{
}

/* <X11/Xlib.h> */
Window XRootWindowOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
Visual *XDefaultVisual(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
Visual *XDefaultVisualOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
GC XDefaultGC(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
GC XDefaultGCOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
unsigned long XBlackPixel(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
unsigned long XWhitePixel(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
unsigned long XAllPlanes(void)
{
}

/* <X11/Xlib.h> */
unsigned long XBlackPixelOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
unsigned long XWhitePixelOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
unsigned long XNextRequest(Display *display)
{
}

/* <X11/Xlib.h> */
unsigned long XLastKnownRequestProcessed(Display *display)
{
}

/* <X11/Xlib.h> */
char *XServerVendor(Display *display)
{
}

/* <X11/Xlib.h> */
char *XDisplayString(Display *display)
{
}

/* <X11/Xlib.h> */
Colormap XDefaultColormap(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
Colormap XDefaultColormapOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
Display *XDisplayOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
Screen *XScreenOfDisplay(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
Screen *XDefaultScreenOfDisplay(Display *display)
{
}

/* <X11/Xlib.h> */
long XEventMaskOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
int XScreenNumberOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
XPixmapFormatValues *XListPixmapFormats(Display *display, int *count_return)
{
}

/* <X11/Xlib.h> */
int *XListDepths(Display *display, int screen_number, int *count_return)
{
}

/* <X11/Xlib.h> */
Status XReconfigureWMWindow(Display *display, Window w, int screen_number, unsigned int mask, XWindowChanges *changes)
{
}

/* <X11/Xlib.h> */
Status XGetWMProtocols(Display *display, Window w, Atom **protocols_return, int *count_return)
{
}

/* <X11/Xlib.h> */
Status XSetWMProtocols(Display *display, Window w, Atom *protocols, int count)
{
}

/* <X11/Xlib.h> */
Status XIconifyWindow(Display *display, Window w, int screen_number)
{
}

/* <X11/Xlib.h> */
Status XWithdrawWindow(Display *display, Window w, int screen_number)
{
}

/* <X11/Xlib.h> */
Status XGetCommand(Display *display, Window w, char ***argv_return, int *argc_return)
{
}

/* <X11/Xlib.h> */
Status XGetWMColormapWindows(Display *display, Window w, Window **windows_return, int *count_return)
{
}

/* <X11/Xlib.h> */
Status XSetWMColormapWindows(Display *display, Window w, Window *colormap_windows, int count)
{
}

/* <X11/Xlib.h> */
void XFreeStringList(char **list)
{
}

/* <X11/Xlib.h> */
int XSetTransientForHint(Display *display, Window w, Window prop_window)
{
}

/* <X11/Xlib.h> */
int XActivateScreenSaver(Display *display)
{
}

/* <X11/Xlib.h> */
int XAddHost(Display *display, XHostAddress *host)
{
}

/* <X11/Xlib.h> */
int XAddHosts(Display *display, XHostAddress *hosts, int	num_hosts)
{
}

/* <X11/Xlib.h> */
int XAddToExtensionList(struct _XExtData **structure, XExtData *ext_data)
{
}

/* <X11/Xlib.h> */
int XAddToSaveSet(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
Status XAllocColor(Display *display, Colormap colormap, XColor *screen_in_out)
{
}

/* <X11/Xlib.h> */
Status XAllocColorCells(Display *display, Colormap colormap, Bool contig, unsigned long *plane_masks_return, unsigned int nplanes, unsigned long *pixels_return, unsigned int  npixels)
{
}

/* <X11/Xlib.h> */
Status XAllocColorPlanes(Display *display, Colormap colormap, Bool contig, unsigned long *pixels_return, int ncolors, int nreds, int ngreens, int nblues, unsigned long *rmask_return, unsigned long *gmask_return, unsigned long *bmask_return)
{
}

/* <X11/Xlib.h> */
Status XAllocNamedColor(Display *display, Colormap colormap, const char *color_name, XColor *screen_def_return, XColor *exact_def_return)
{
}

/* <X11/Xlib.h> */
int XAllowEvents(Display *display, int event_mode, Time time)
{
}

/* <X11/Xlib.h> */
int XAutoRepeatOff(Display *display)
{
}

/* <X11/Xlib.h> */
int XAutoRepeatOn(Display *display)
{
}

/* <X11/Xlib.h> */
int XBell(Display *display, int percent)
{
}

/* <X11/Xlib.h> */
int XBitmapBitOrder(Display *display)
{
}

/* <X11/Xlib.h> */
int XBitmapPad(Display *display)
{
}

/* <X11/Xlib.h> */
int XBitmapUnit(Display *display)
{
}

/* <X11/Xlib.h> */
int XCellsOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
int XChangeActivePointerGrab(Display *display, unsigned int event_mask, Cursor cursor, Time time)
{
}

/* <X11/Xlib.h> */
int XChangeGC(Display *display, GC gc, unsigned long valuemask, XGCValues *values)
{
}

/* <X11/Xlib.h> */
int XChangeKeyboardControl(Display *display, unsigned long value_mask, XKeyboardControl *values)
{
}

/* <X11/Xlib.h> */
int XChangeKeyboardMapping(Display *display, int first_keycode, int keysyms_per_keycode, KeySym *keysyms, int num_codes)
{
}

/* <X11/Xlib.h> */
int XChangePointerControl(Display *display, Bool do_accel, Bool do_threshold, int accel_numerator, int accel_denominator, int threshold)
{
}

/* <X11/Xlib.h> */
int XChangeProperty(Display *display, Window w, Atom property, Atom type, int format, int mode, const unsigned char *data, int nelements)
{
}

/* <X11/Xlib.h> */
int XChangeSaveSet(Display *display, Window w, int change_mode)
{
}

/* <X11/Xlib.h> */
int XChangeWindowAttributes(Display *display, Window w, unsigned long valuemask, XSetWindowAttributes *attributes)
{
}

/* <X11/Xlib.h> */
Bool XCheckIfEvent(Display *display, XEvent *event_return, Bool (*)(Display *display, XEvent *event, XPointer arg) predicate, XPointer arg)
{
}

/* <X11/Xlib.h> */
Bool XCheckMaskEvent(Display *display, long event_mask, XEvent *event_return)
{
}

/* <X11/Xlib.h> */
Bool XCheckTypedEvent(Display *display, int event_type, XEvent *event_return)
{
}

/* <X11/Xlib.h> */
Bool XCheckTypedWindowEvent(Display *display, Window w, int event_type, XEvent *event_return)
{
}

/* <X11/Xlib.h> */
Bool XCheckWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
}

/* <X11/Xlib.h> */
int XCirculateSubwindows(Display *display, Window w, int direction)
{
}

/* <X11/Xlib.h> */
int XCirculateSubwindowsDown(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XCirculateSubwindowsUp(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XClearArea(Display *display, Window w, int x, int y, unsigned int width, unsigned int height, Bool exposures)
{
}

/* <X11/Xlib.h> */
int XClearWindow(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XCloseDisplay(Display *display)
{
}

/* <X11/Xlib.h> */
int XConfigureWindow(Display *display, Window w, unsigned int value_mask, XWindowChanges *values)
{
}

/* <X11/Xlib.h> */
int XConnectionNumber(Display *display)
{
}

/* <X11/Xlib.h> */
int XConvertSelection(Display *display, Atom selection, Atom  target, Atom property, Window requestor, Time time)
{
}

/* <X11/Xlib.h> */
int XCopyArea(Display *display, Drawable src, Drawable dest, GC gc, int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y)
{
}

/* <X11/Xlib.h> */
int XCopyGC(Display *display, GC src, unsigned long valuemask, GC dest)
{
}

/* <X11/Xlib.h> */
int XCopyPlane(Display *display, Drawable src, Drawable dest, GC gc, int src_x, int src_y, unsigned int width, unsigned int height, int dest_x, int dest_y, unsigned long plane)
{
}

/* <X11/Xlib.h> */
int XDefaultDepth(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
int XDefaultDepthOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
int XDefaultScreen(Display *display)
{
}

/* <X11/Xlib.h> */
int XDefineCursor(Display *display, Window w, Cursor cursor)
{
}

/* <X11/Xlib.h> */
int XDeleteProperty(Display *display, Window w, Atom property)
{
}

/* <X11/Xlib.h> */
int XDestroyWindow(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XDestroySubwindows(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XDoesBackingStore(Screen *screen)
{
}

/* <X11/Xlib.h> */
Bool XDoesSaveUnders(Screen *screen)
{
}

/* <X11/Xlib.h> */
int XDisableAccessControl(Display *display)
{
}

/* <X11/Xlib.h> */
int XDisplayCells(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
int XDisplayHeight(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
int XDisplayHeightMM(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
int XDisplayKeycodes(Display *display, int *min_keycodes_return, int *max_keycodes_return)
{
}

/* <X11/Xlib.h> */
int XDisplayPlanes(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
int XDisplayWidth(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
int XDisplayWidthMM(Display *display, int screen_number)
{
}

/* <X11/Xlib.h> */
int XDrawArc(Display *display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height, int angle1, int angle2)
{
}

/* <X11/Xlib.h> */
int XDrawArcs(Display *display, Drawable d, GC gc, XArc *arcs, int narcs)
{
}

/* <X11/Xlib.h> */
int XDrawImageString(Display *display, Drawable d, GC gc, int x, int y, const char *string, int length)
{
}

/* <X11/Xlib.h> */
int XDrawImageString16(Display *display, Drawable d, GC gc, int x, int y, const XChar2b *string, int length)
{
}

/* <X11/Xlib.h> */
int XDrawLine(Display *display, Drawable d, GC gc, int x1, int x2, int y1, int y2)
{
}

/* <X11/Xlib.h> */
int XDrawLines(Display *display, Drawable d, GC gc, XPoint *points, int npoints, int mode)
{
}

/* <X11/Xlib.h> */
int XDrawPoint(Display *display, Drawable d, GC gc, int x, int y)
{
}

/* <X11/Xlib.h> */
int XDrawPoints(Display *display, Drawable d, GC gc, XPoint *points, int npoints, int mode)
{
}

/* <X11/Xlib.h> */
int XDrawRectangle(Display *display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height)
{
}

/* <X11/Xlib.h> */
int XDrawRectangles(Display *display, Drawable d, GC gc, XRectangle *rectangles, int nrectangles)
{
}

/* <X11/Xlib.h> */
int XDrawSegments(Display *display, Drawable d, GC gc, XSegment *segments, int nsegments)
{
}

/* <X11/Xlib.h> */
int XDrawString(Display *display, Drawable d, GC gc, int x, int y, const char *string, int length)
{
}

/* <X11/Xlib.h> */
int XDrawString16(Display *display, Drawable d, GC gc, int x, int y, const XChar2b *string, int length)
{
}

/* <X11/Xlib.h> */
int XDrawText(Display *display, Drawable d, GC gc, int x, int y, XTextItem *items, int nitems)
{
}

/* <X11/Xlib.h> */
int XDrawText16(Display *display, Drawable d, GC gc, int x, int y, XTextItem16 *items, int nitems)
{
}

/* <X11/Xlib.h> */
int XEnableAccessControl(Display *display)
{
}

/* <X11/Xlib.h> */
int XEventsQueued(Display *display, int mode)
{
}

/* <X11/Xlib.h> */
Status XFetchName(Display *display, Window w, char **window_name_return)
{
}

/* <X11/Xlib.h> */
int XFillArc(Display *display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height, int angle1, int angle2)
{
}

/* <X11/Xlib.h> */
int XFillArcs(Display *display, Drawable d, GC gc, XArc *arcs, int narcs)
{
}

/* <X11/Xlib.h> */
int XFillPolygon(Display *display, Drawable d, GC gc, XPoint *points, int npoints, int shape, int mode)
{
}

/* <X11/Xlib.h> */
int XFillRectangle(Display *display, Drawable d, GC gc, int x, int y, unsigned int width, unsigned int height)
{
}

/* <X11/Xlib.h> */
int XFillRectangles(Display *display, Drawable d, GC gc, XRectangle *rectangles, int nrectangles)
{
}

/* <X11/Xlib.h> */
int XFlush(Display *display)
{
}

/* <X11/Xlib.h> */
int XForceScreenSaver(Display *display, int mode)
{
}

/* <X11/Xlib.h> */
int XFree(void *data)
{
}

/* <X11/Xlib.h> */
int XFreeColormap(Display *display, Colormap colormap)
{
}

/* <X11/Xlib.h> */
int XFreeColors(Display *display, Colormap colormap, unsigned long *pixels, int npixels, unsigned long planes)
{
}

/* <X11/Xlib.h> */
int XFreeCursor(Display *display, Cursor cursor)
{
}

/* <X11/Xlib.h> */
int XFreeExtensionList(char **list)
{
}

/* <X11/Xlib.h> */
int XFreeFont(Display *display, XFontStruct *font_struct)
{
}

/* <X11/Xlib.h> */
int XFreeFontInfo(char **names, XFontStruct *free_info, int actual_count)
{
}

/* <X11/Xlib.h> */
int XFreeFontNames(char **list)
{
}

/* <X11/Xlib.h> */
int XFreeFontPath(char **list)
{
}

/* <X11/Xlib.h> */
int XFreeGC(Display *display, GC gc)
{
}

/* <X11/Xlib.h> */
int XFreeModifiermap(XModifierKeymap *modmap)
{
}

/* <X11/Xlib.h> */
int XFreePixmap(Display *display, Pixmap pixmap)
{
}

/* <X11/Xlib.h> */
int XGeometry(Display *display, int screen, const char *position, const char *default_position, unsigned int bwidth, unsigned int fwidth, unsigned int fheight, int xadder, int yadder, int *x_return, int *y_return, int *width_return, int *height_return)
{
}

/* <X11/Xlib.h> */
int XGetErrorDatabaseText(Display *display, const char *name, const char *message, const char *default_string, char *buffer_return, int length)
{
}

/* <X11/Xlib.h> */
int XGetErrorText(Display *display, int code, char *buffer_return, int length)
{
}

/* <X11/Xlib.h> */
Bool XGetFontProperty(XFontStruct *font_struct, Atom atom, unsigned long *value_return)
{
}

/* <X11/Xlib.h> */
Status XGetGCValues(Display *display, GC gc, unsigned long valuemask, XGCValues *values_return)
{
}

/* <X11/Xlib.h> */
Status XGetGeometry(Display *display, Drawable d, Window *root_return, int *x_return, int *y_return, unsigned int *width_return, unsigned int *height_return, unsigned int *border_width_return, unsigned int *depth_return)
{
}

/* <X11/Xlib.h> */
Status XGetIconName(Display *display, Window w, char **icon_name_return)
{
}

/* <X11/Xlib.h> */
int XGetInputFocus(Display *display, Window *focus_return, int *revert_to_return)
{
}

/* <X11/Xlib.h> */
int XGetKeyboardControl(Display *display, XKeyboardState *values_return)
{
}

/* <X11/Xlib.h> */
int XGetPointerControl(Display *display, int *accel_numerator_return, int *accel_denominator_return, int *threshold_return)
{
}

/* <X11/Xlib.h> */
int XGetPointerMapping(Display *display, unsigned char *map_return, int nmap)
{
}

/* <X11/Xlib.h> */
int XGetScreenSaver(Display *display, int *timeout_return, int *interval_return, int *prefer_blanking_return, int *allow_exposures_return)
{
}

/* <X11/Xlib.h> */
Status XGetTransientForHint(Display *display, Window w, Window *prop_window_return)
{
}

/* <X11/Xlib.h> */
int XGetWindowProperty(Display *display, Window w, Atom property, long long_offset, long long_length, Bool delete, Atom req_type, Atom *actual_type_return, int *actual_format_return, unsigned long *nitems_return, unsigned long *bytes_after_return, unsigned char **prop_return)
{
}

/* <X11/Xlib.h> */
Status XGetWindowAttributes(Display *display, Window w, XWindowAttributes *window_attributes_return)
{
}

/* <X11/Xlib.h> */
int XGrabButton(Display *display, unsigned int button, unsigned int modifiers, Window grab_window, Bool owner_events, unsigned int event_mask, int pointer_mode, int keyboard_mode, Window confine_to, Cursor cursor)
{
}

/* <X11/Xlib.h> */
int XGrabKey(Display *display, int keycode, unsigned int modifiers, Window grab_window, Bool owner_events, int pointer_mode, int keyboard_mode)
{
}

/* <X11/Xlib.h> */
int XGrabKeyboard(Display *display, Window grab_window, Bool owner_events, int pointer_mode, int keyboard_mode, Time time)
{
}

/* <X11/Xlib.h> */
int XGrabPointer(Display *display, Window grab_window, Bool owner_events, unsigned int event_mask, int pointer_mode, int keyboard_mode, Window confine_to, Cursor cursor, Time time)
{
}

/* <X11/Xlib.h> */
int XGrabServer(Display *display)
{
}

/* <X11/Xlib.h> */
int XHeightMMOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
int XHeightOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
int XIfEvent(Display *display, XEvent *event_return, Bool (*)(Display *display, XEvent *event, XPointer arg ) predicate, XPointer arg)
{
}

/* <X11/Xlib.h> */
int XImageByteOrder(Display *display)
{
}

/* <X11/Xlib.h> */
int XInstallColormap(Display *display, Colormap colormap)
{
}

/* <X11/Xlib.h> */
KeyCode XKeysymToKeycode(Display *display, KeySym keysym)
{
}

/* <X11/Xlib.h> */
int XKillClient(Display *display, XID resource)
{
}

/* <X11/Xlib.h> */
Status XLookupColor(Display *display, Colormap colormap, const char *color_name, XColor *exact_def_return, XColor *screen_def_return)
{
}

/* <X11/Xlib.h> */
int XLowerWindow(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XMapRaised(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XMapSubwindows(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XMapWindow(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XMaskEvent(Display *display, long event_mask, XEvent *event_return)
{
}

/* <X11/Xlib.h> */
int XMaxCmapsOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
int XMinCmapsOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
int XMoveResizeWindow(Display *display, Window w, int x, int y, unsigned int width, unsigned int height)
{
}

/* <X11/Xlib.h> */
int XMoveWindow(Display *display, Window w, int x, int y)
{
}

/* <X11/Xlib.h> */
int XNextEvent(Display *display, XEvent *event_return)
{
}

/* <X11/Xlib.h> */
int XNoOp(Display *display)
{
}

/* <X11/Xlib.h> */
Status XParseColor(Display *display, Colormap colormap, const char *spec, XColor *exact_def_return)
{
}

/* <X11/Xlib.h> */
int XParseGeometry(const char *parsestring, int *x_return, int *y_return, unsigned int *width_return, unsigned int *height_return)
{
}

/* <X11/Xlib.h> */
int XPeekEvent(Display *display, XEvent *event_return)
{
}

/* <X11/Xlib.h> */
int XPeekIfEvent(Display *display, XEvent *event_return, Bool (*)(Display *display, XEvent *event, XPointer arg ) predicate, XPointer arg)
{
}

/* <X11/Xlib.h> */
int XPending(Display *display)
{
}

/* <X11/Xlib.h> */
int XPlanesOfScreen(Screen *screen )
{
}

/* <X11/Xlib.h> */
int XProtocolRevision(Display *display)
{
}

/* <X11/Xlib.h> */
int XProtocolVersion(Display *display)
{
}

/* <X11/Xlib.h> */
int XPutBackEvent(Display *display, XEvent *event)
{
}

/* <X11/Xlib.h> */
int XPutImage(Display *display, Drawable d, GC gc, XImage *image, int src_x, int src_y, int dest_x, int dest_y, unsigned int width, unsigned int height)
{
}

/* <X11/Xlib.h> */
int XQLength(Display *display)
{
}

/* <X11/Xlib.h> */
Status XQueryBestCursor(Display *display, Drawable d, unsigned int width, unsigned int height, unsigned int *width_return, unsigned int *height_return)
{
}

/* <X11/Xlib.h> */
Status XQueryBestSize(Display *display, int class, Drawable which_screen, unsigned int width, unsigned int height, unsigned int *width_return, unsigned int *height_return)
{
}

/* <X11/Xlib.h> */
Status XQueryBestStipple(Display *display, Drawable which_screen, unsigned int width, unsigned int height, unsigned int *width_return, unsigned int *height_return)
{
}

/* <X11/Xlib.h> */
Status XQueryBestTile(Display *display, Drawable which_screen, unsigned int width, unsigned int height, unsigned int *width_return, unsigned int *height_return)
{
}

/* <X11/Xlib.h> */
int XQueryColor(Display *display, Colormap colormap, XColor *def_in_out)
{
}

/* <X11/Xlib.h> */
int XQueryColors(Display *display, Colormap colormap, XColor *defs_in_out, int ncolors)
{
}

/* <X11/Xlib.h> */
Bool XQueryExtension(Display *display, const char *name, int *major_opcode_return, int *first_event_return, int *first_error_return)
{
}

/* <X11/Xlib.h> */
int XQueryKeymap(Display *display, char [32] keys_return)
{
}

/* <X11/Xlib.h> */
Bool XQueryPointer(Display *display, Window w, Window *root_return, Window *child_return, int *root_x_return, int *root_y_return, int *win_x_return, int *win_y_return, unsigned int *mask_return)
{
}

/* <X11/Xlib.h> */
int XQueryTextExtents(Display *display, XID font_ID, const char *string, int nchars, int *direction_return, int *font_ascent_return, int *font_descent_return, XCharStruct *overall_return)
{
}

/* <X11/Xlib.h> */
int XQueryTextExtents16(Display *display, XID font_ID, const XChar2b *string, int nchars, int *direction_return, int *font_ascent_return, int *font_descent_return, XCharStruct *overall_return)
{
}

/* <X11/Xlib.h> */
Status XQueryTree(Display *display, Window w, Window *root_return, Window *parent_return, Window **children_return, unsigned int *nchildren_return)
{
}

/* <X11/Xlib.h> */
int XRaiseWindow(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XReadBitmapFile(Display *display, Drawable  d, const char *filename, unsigned int *width_return, unsigned int *height_return, Pixmap *bitmap_return, int *x_hot_return, int *y_hot_return)
{
}

/* <X11/Xlib.h> */
int XReadBitmapFileData(const char *filename, unsigned int *width_return, unsigned int *height_return, unsigned char **data_return, int *x_hot_return, int *y_hot_return)
{
}

/* <X11/Xlib.h> */
int XRebindKeysym(Display *display, KeySym keysym, KeySym *list, int mod_count, const unsigned char *string, int bytes_string)
{
}

/* <X11/Xlib.h> */
int XRecolorCursor(Display *display, Cursor cursor, XColor *foreground_color, XColor *background_color)
{
}

/* <X11/Xlib.h> */
int XRefreshKeyboardMapping(XMappingEvent *event_map)
{
}

/* <X11/Xlib.h> */
int XRemoveFromSaveSet(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XRemoveHost(Display *display, XHostAddress *host)
{
}

/* <X11/Xlib.h> */
int XRemoveHosts(Display *display, XHostAddress *hosts, int num_hosts)
{
}

/* <X11/Xlib.h> */
int XReparentWindow(Display *display, Window w, Window parent, int x, int y)
{
}

/* <X11/Xlib.h> */
int XResetScreenSaver(Display *display)
{
}

/* <X11/Xlib.h> */
int XResizeWindow(Display *display, Window w, unsigned int width, unsigned int height)
{
}

/* <X11/Xlib.h> */
int XRestackWindows(Display *display, Window *windows, int nwindows)
{
}

/* <X11/Xlib.h> */
int XRotateBuffers(Display *display, int rotate)
{
}

/* <X11/Xlib.h> */
int XRotateWindowProperties(Display *display, Window w, Atom *properties, int num_prop, int npositions)
{
}

/* <X11/Xlib.h> */
int XScreenCount(Display *display)
{
}

/* <X11/Xlib.h> */
int XSelectInput(Display *display, Window w, long event_mask)
{
}

/* <X11/Xlib.h> */
Status XSendEvent(Display *display, Window w, Bool propagate, long event_mask, XEvent *event_send)
{
}

/* <X11/Xlib.h> */
int XSetAccessControl(Display *display, int mode)
{
}

/* <X11/Xlib.h> */
int XSetArcMode(Display *display, GC gc, int arc_mode)
{
}

/* <X11/Xlib.h> */
int XSetBackground(Display *display, GC gc, unsigned long background)
{
}

/* <X11/Xlib.h> */
int XSetClipMask(Display *display, GC gc, Pixmap pixmap)
{
}

/* <X11/Xlib.h> */
int XSetClipOrigin(Display *display, GC gc, int clip_x_origin, int clip_y_origin)
{
}

/* <X11/Xlib.h> */
int XSetClipRectangles(Display *display, GC gc, int clip_x_origin, int clip_y_origin, XRectangle *rectangles, int n, int ordering)
{
}

/* <X11/Xlib.h> */
int XSetCloseDownMode(Display *display, int close_mode)
{
}

/* <X11/Xlib.h> */
int XSetCommand(Display *display, Window w, char **argv, int argc)
{
}

/* <X11/Xlib.h> */
int XSetDashes(Display *display, GC gc, int dash_offset, const char *dash_list, int n)
{
}

/* <X11/Xlib.h> */
int XSetFillRule(Display *display, GC gc, int fill_rule)
{
}

/* <X11/Xlib.h> */
int XSetFillStyle(Display *display, GC gc, int fill_style)
{
}

/* <X11/Xlib.h> */
int XSetFont(Display *display, GC gc, Font font)
{
}

/* <X11/Xlib.h> */
int XSetFontPath(Display *display, char **directories, int	ndirs)
{
}

/* <X11/Xlib.h> */
int XSetForeground(Display *display, GC gc, unsigned long foreground)
{
}

/* <X11/Xlib.h> */
int XSetFunction(Display *display, GC gc, int function)
{
}

/* <X11/Xlib.h> */
int XSetGraphicsExposures(Display *display, GC gc, Bool graphics_exposures)
{
}

/* <X11/Xlib.h> */
int XSetIconName(Display *display, Window w, const char *icon_name)
{
}

/* <X11/Xlib.h> */
int XSetInputFocus(Display *display, Window focus, int revert_to, Time time)
{
}

/* <X11/Xlib.h> */
int XSetLineAttributes(Display *display, GC gc, unsigned int line_width, int line_style, int cap_style, int join_style)
{
}

/* <X11/Xlib.h> */
int XSetModifierMapping(Display *display, XModifierKeymap *modmap)
{
}

/* <X11/Xlib.h> */
int XSetPlaneMask(Display *display, GC gc, unsigned long plane_mask)
{
}

/* <X11/Xlib.h> */
int XSetPointerMapping(Display *display, const unsigned char *map, int nmap)
{
}

/* <X11/Xlib.h> */
int XSetScreenSaver(Display *display, int timeout, int interval, int prefer_blanking, int allow_exposures)
{
}

/* <X11/Xlib.h> */
int XSetSelectionOwner(Display *display, Atom selection, Window owner, Time time)
{
}

/* <X11/Xlib.h> */
int XSetState(Display *display, GC gc, unsigned long  foreground, unsigned long background, int function, unsigned long plane_mask)
{
}

/* <X11/Xlib.h> */
int XSetStipple(Display *display, GC gc, Pixmap stipple)
{
}

/* <X11/Xlib.h> */
int XSetSubwindowMode(Display *display, GC gc, int subwindow_mode)
{
}

/* <X11/Xlib.h> */
int XSetTSOrigin(Display *display, GC gc, int ts_x_origin, int ts_y_origin)
{
}

/* <X11/Xlib.h> */
int XSetTile(Display *display, GC gc, Pixmap tile)
{
}

/* <X11/Xlib.h> */
int XSetWindowBackground(Display *display, Window w, unsigned long background_pixel)
{
}

/* <X11/Xlib.h> */
int XSetWindowBackgroundPixmap(Display *display, Window w, Pixmap background_pixmap)
{
}

/* <X11/Xlib.h> */
int XSetWindowBorder(Display *display, Window w, unsigned long border_pixel)
{
}

/* <X11/Xlib.h> */
int XSetWindowBorderPixmap(Display *display, Window w, Pixmap border_pixmap)
{
}

/* <X11/Xlib.h> */
int XSetWindowBorderWidth(Display *display, Window w, unsigned int width)
{
}

/* <X11/Xlib.h> */
int XSetWindowColormap(Display *display, Window w, Colormap colormap)
{
}

/* <X11/Xlib.h> */
int XStoreBuffer(Display *display, const char *bytes, int nbytes, int buffer)
{
}

/* <X11/Xlib.h> */
int XStoreBytes(Display *display, const char *bytes, int nbytes)
{
}

/* <X11/Xlib.h> */
int XStoreColor(Display *display, Colormap colormap, XColor *color)
{
}

/* <X11/Xlib.h> */
int XStoreColors(Display *display, Colormap colormap, XColor *color, int ncolors)
{
}

/* <X11/Xlib.h> */
int XStoreName(Display *display, Window w, const char *window_name)
{
}

/* <X11/Xlib.h> */
int XStoreNamedColor(Display *display, Colormap colormap, const char *color, unsigned long pixel, int flags)
{
}

/* <X11/Xlib.h> */
int XSync(Display *display, Bool discard)
{
}

/* <X11/Xlib.h> */
int XTextExtents(XFontStruct *font_struct, const char *string, int nchars, int *direction_return, int *font_ascent_return, int *font_descent_return, XCharStruct *overall_return)
{
}

/* <X11/Xlib.h> */
int XTextExtents16(XFontStruct *font_struct, const XChar2b *string, int nchars, int *direction_return, int *font_ascent_return, int *font_descent_return, XCharStruct *overall_return)
{
}

/* <X11/Xlib.h> */
int XTextWidth(XFontStruct *font_struct, const char *string, int count)
{
}

/* <X11/Xlib.h> */
int XTextWidth16(XFontStruct *font_struct, const XChar2b *string, int count)
{
}

/* <X11/Xlib.h> */
Bool XTranslateCoordinates(Display *display, Window src_w, Window dest_w, int src_x, int src_y, int *dest_x_return, int *dest_y_return, Window *child_return)
{
}

/* <X11/Xlib.h> */
int XUndefineCursor(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XUngrabButton(Display *display, unsigned int button, unsigned int modifiers, Window grab_window)
{
}

/* <X11/Xlib.h> */
int XUngrabKey(Display *display, int keycode, unsigned int modifiers, Window grab_window)
{
}

/* <X11/Xlib.h> */
int XUngrabKeyboard(Display *display, Time time)
{
}

/* <X11/Xlib.h> */
int XUngrabPointer(Display *display, Time time)
{
}

/* <X11/Xlib.h> */
int XUngrabServer(Display *display)
{
}

/* <X11/Xlib.h> */
int XUninstallColormap(Display *display, Colormap colormap)
{
}

/* <X11/Xlib.h> */
int XUnloadFont(Display *display, Font font)
{
}

/* <X11/Xlib.h> */
int XUnmapSubwindows(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XUnmapWindow(Display *display, Window w)
{
}

/* <X11/Xlib.h> */
int XVendorRelease(Display *display)
{
}

/* <X11/Xlib.h> */
int XWarpPointer(Display *display, Window src_w, Window dest_w, int src_x, int src_y, unsigned int src_width, unsigned int src_height, int dest_x, int			/ *dest_y */	     )
{
}

/* <X11/Xlib.h> */
int XWidthMMOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
int XWidthOfScreen(Screen *screen)
{
}

/* <X11/Xlib.h> */
int XWindowEvent(Display *display, Window w, long event_mask, XEvent *event_return)
{
}

/* <X11/Xlib.h> */
int XWriteBitmapFile(Display *display, const char *filename, Pixmap bitmap, unsigned int width, unsigned int height, int x_hot, int	y_hot)
{
}

/* <X11/Xlib.h> */
Bool XSupportsLocale(void)
{
}

/* <X11/Xlib.h> */
char *XSetLocaleModifiers(const char *modifier_list)
{
}

/* <X11/Xlib.h> */
XOM XOpenOM(Display *display, struct _XrmHashBucketRec *rdb, const char *res_name, const char *res_class)
{
}

/* <X11/Xlib.h> */
Status XCloseOM(XOM om)
{
}

/* <X11/Xlib.h> */
char *XSetOMValues(XOM om, ...)
{
}

/* <X11/Xlib.h> */
char *XGetOMValues(XOM om, ...)
{
}

/* <X11/Xlib.h> */
Display *XDisplayOfOM(XOM om)
{
}

/* <X11/Xlib.h> */
char *XLocaleOfOM(XOM om)
{
}

/* <X11/Xlib.h> */
XOC XCreateOC(XOM om, ...)
{
}

/* <X11/Xlib.h> */
void XDestroyOC(XOC oc)
{
}

/* <X11/Xlib.h> */
XOM XOMOfOC(XOC oc)
{
}

/* <X11/Xlib.h> */
char *XSetOCValues(XOC oc, ...)
{
}

/* <X11/Xlib.h> */
char *XGetOCValues(XOC oc, ...)
{
}

/* <X11/Xlib.h> */
XFontSet XCreateFontSet(Display *display, const char *base_font_name_list, char ***missing_charset_list, int *missing_charset_count, char **def_string)
{
}

/* <X11/Xlib.h> */
void XFreeFontSet(Display *display, XFontSet font_set)
{
}

/* <X11/Xlib.h> */
int XFontsOfFontSet(XFontSet font_set, XFontStruct ***font_struct_list, char ***font_name_list)
{
}

/* <X11/Xlib.h> */
char *XBaseFontNameListOfFontSet(XFontSet font_set)
{
}

/* <X11/Xlib.h> */
char *XLocaleOfFontSet(XFontSet font_set)
{
}

/* <X11/Xlib.h> */
Bool XContextDependentDrawing(XFontSet font_set)
{
}

/* <X11/Xlib.h> */
Bool XDirectionalDependentDrawing(XFontSet font_set)
{
}

/* <X11/Xlib.h> */
Bool XContextualDrawing(XFontSet font_set)
{
}

/* <X11/Xlib.h> */
XFontSetExtents *XExtentsOfFontSet(XFontSet font_set)
{
}

/* <X11/Xlib.h> */
int XmbTextEscapement(XFontSet font_set, const char *text, int bytes_text)
{
}

/* <X11/Xlib.h> */
int XwcTextEscapement(XFontSet font_set, const wchar_t *text, int num_wchars)
{
}

/* <X11/Xlib.h> */
int XmbTextExtents(XFontSet font_set, const char *text, int bytes_text, XRectangle *overall_ink_return, XRectangle *overall_logical_return)
{
}

/* <X11/Xlib.h> */
int XwcTextExtents(XFontSet font_set, const wchar_t *text, int num_wchars, XRectangle *overall_ink_return, XRectangle *overall_logical_return)
{
}

/* <X11/Xlib.h> */
Status XmbTextPerCharExtents(XFontSet font_set, const char *text, int bytes_text, XRectangle *ink_extents_buffer, XRectangle *logical_extents_buffer, int buffer_size, int *num_chars, XRectangle *overall_ink_return, XRectangle *overall_logical_return)
{
}

/* <X11/Xlib.h> */
Status XwcTextPerCharExtents(XFontSet font_set, const wchar_t *text, int num_wchars, XRectangle *ink_extents_buffer, XRectangle *logical_extents_buffer, int buffer_size, int *num_chars, XRectangle *overall_ink_return, XRectangle *overall_logical_return)
{
}

/* <X11/Xlib.h> */
void XmbDrawText(Display *display, Drawable d, GC gc, int x, int y, XmbTextItem *text_items, int nitems)
{
}

/* <X11/Xlib.h> */
void XwcDrawText(Display *display, Drawable d, GC gc, int x, int y, XwcTextItem *text_items, int nitems)
{
}

/* <X11/Xlib.h> */
void XmbDrawString(Display *display, Drawable d, XFontSet font_set, GC gc, int x, int y, const char *text, int bytes_text)
{
}

/* <X11/Xlib.h> */
void XwcDrawString(Display *display, Drawable d, XFontSet font_set, GC gc, int x, int y, const wchar_t *text, int num_wchars)
{
}

/* <X11/Xlib.h> */
void XmbDrawImageString(Display *display, Drawable d, XFontSet font_set, GC gc, int x, int y, const char *text, int bytes_text)
{
}

/* <X11/Xlib.h> */
void XwcDrawImageString(Display *display, Drawable d, XFontSet font_set, GC gc, int x, int y, const wchar_t *text, int num_wchars)
{
}

/* <X11/Xlib.h> */
XIM XOpenIM(Display *dpy, struct _XrmHashBucketRec *rdb, char *res_name, char *res_class)
{
}

/* <X11/Xlib.h> */
Status XCloseIM(XIM im)
{
}

/* <X11/Xlib.h> */
char *XGetIMValues(XIM im, ...)
{
}

/* <X11/Xlib.h> */
char *XSetIMValues(XIM im, ...)
{
}

/* <X11/Xlib.h> */
Display *XDisplayOfIM(XIM im)
{
}

/* <X11/Xlib.h> */
char *XLocaleOfIM(XIM im)
{
}

/* <X11/Xlib.h> */
XIC XCreateIC(XIM im, ...)
{
}

/* <X11/Xlib.h> */
void XDestroyIC(XIC ic)
{
}

/* <X11/Xlib.h> */
void XSetICFocus(XIC ic)
{
}

/* <X11/Xlib.h> */
void XUnsetICFocus(XIC ic)
{
}

/* <X11/Xlib.h> */
wchar_t *XwcResetIC(XIC ic)
{
}

/* <X11/Xlib.h> */
char *XmbResetIC(XIC ic)
{
}

/* <X11/Xlib.h> */
char *XSetICValues(XIC ic, ...)
{
}

/* <X11/Xlib.h> */
char *XGetICValues(XIC ic, ...)
{
}

/* <X11/Xlib.h> */
XIM XIMOfIC(XIC ic)
{
}

/* <X11/Xlib.h> */
Bool XFilterEvent(XEvent *event, Window window)
{
}

/* <X11/Xlib.h> */
int XmbLookupString(XIC ic, XKeyPressedEvent *event, char *buffer_return, int bytes_buffer, KeySym *keysym_return, Status *status_return)
{
}

/* <X11/Xlib.h> */
int XwcLookupString(XIC ic, XKeyPressedEvent *event, wchar_t *buffer_return, int wchars_buffer, KeySym *keysym_return, Status *status_return)
{
}

/* <X11/Xlib.h> */
XVaNestedList XVaCreateNestedList(int dummy, ...)
{
}

/* <X11/Xlib.h> */
Bool XRegisterIMInstantiateCallback(Display *dpy, struct _XrmHashBucketRec *rdb, char *res_name, char *res_class, XIDProc callback, XPointer client_data)
{
}

/* <X11/Xlib.h> */
Bool XUnregisterIMInstantiateCallback(Display *dpy, struct _XrmHashBucketRec *rdb, char *res_name, char *res_class, XIDProc callback, XPointer client_data)
{
}

/* <X11/Xlib.h> */
Status XInternalConnectionNumbers(Display *dpy, int **fd_return, int *count_return)
{
}

/* <X11/Xlib.h> */
void XProcessInternalConnection(Display *dpy, int fd)
{
}

/* <X11/Xlib.h> */
Status XAddConnectionWatch(Display *dpy, XConnectionWatchProc callback, XPointer client_data)
{
}

/* <X11/Xlib.h> */
void XRemoveConnectionWatch(Display *dpy, XConnectionWatchProc callback, XPointer client_data)
{
}

/* <X11/Xutil.h> */
XClassHint *XAllocClassHint (void)
{
}

/* <X11/Xutil.h> */
XIconSize *XAllocIconSize (void)
{
}

/* <X11/Xutil.h> */
XSizeHints *XAllocSizeHints (void)
{
}

/* <X11/Xutil.h> */
XStandardColormap *XAllocStandardColormap (void)
{
}

/* <X11/Xutil.h> */
XWMHints *XAllocWMHints (void)
{
}

/* <X11/Xutil.h> */
int XClipBox(Region r, XRectangle* rect_return)
{
}

/* <X11/Xutil.h> */
Region XCreateRegion(void)
{
}

/* <X11/Xutil.h> */
char *XDefaultString(void)
{
}

/* <X11/Xutil.h> */
int XDeleteContext(Display* display, XID rid, XContext context)
{
}

/* <X11/Xutil.h> */
int XDestroyRegion(Region r)
{
}

/* <X11/Xutil.h> */
int XEmptyRegion(Region r)
{
}

/* <X11/Xutil.h> */
int XEqualRegion(Region r1, Region r2)
{
}

/* <X11/Xutil.h> */
int XFindContext(Display* display, XID rid, XContext context, XPointer* data_return)
{
}

/* <X11/Xutil.h> */
Status XGetClassHint(Display* display, Window w, XClassHint* class_hints_return)
{
}

/* <X11/Xutil.h> */
Status XGetIconSizes(Display* display, Window w, XIconSize** size_list_return, int* count_return)
{
}

/* <X11/Xutil.h> */
Status XGetNormalHints(Display* display, Window w, XSizeHints* hints_return)
{
}

/* <X11/Xutil.h> */
Status XGetRGBColormaps(Display* display, Window w, XStandardColormap** stdcmap_return, int* count_return, Atom property)
{
}

/* <X11/Xutil.h> */
Status XGetSizeHints(Display* display, Window w, XSizeHints* hints_return, Atom property)
{
}

/* <X11/Xutil.h> */
Status XGetStandardColormap(Display* display, Window w, XStandardColormap* colormap_return, Atom		/* property */			    )
{
}

/* <X11/Xutil.h> */
Status XGetTextProperty(Display* display, Window window, XTextProperty* text_prop_return, Atom property)
{
}

/* <X11/Xutil.h> */
XVisualInfo *XGetVisualInfo(Display* display, long vinfo_mask, XVisualInfo* vinfo_template, int* nitems_return)
{
}

/* <X11/Xutil.h> */
Status XGetWMClientMachine(Display* display, Window w, XTextProperty* text_prop_return)
{
}

/* <X11/Xutil.h> */
XWMHints *XGetWMHints(Display* display, Window w)
{
}

/* <X11/Xutil.h> */
Status XGetWMIconName(Display* display, Window w, XTextProperty* text_prop_return)
{
}

/* <X11/Xutil.h> */
Status XGetWMName(Display* display, Window w, XTextProperty* text_prop_return)
{
}

/* <X11/Xutil.h> */
Status XGetWMNormalHints(Display* display, Window w, XSizeHints* hints_return, long* supplied_return)
{
}

/* <X11/Xutil.h> */
Status XGetWMSizeHints(Display* display, Window w, XSizeHints* hints_return, long* supplied_return, Atom property)
{
}

/* <X11/Xutil.h> */
Status XGetZoomHints(Display* display, Window w, XSizeHints* zhints_return)
{
}

/* <X11/Xutil.h> */
int XIntersectRegion(Region sra, Region srb, Region dr_return)
{
}

/* <X11/Xutil.h> */
void XConvertCase(KeySym sym, KeySym* lower, KeySym* upper)
{
}

/* <X11/Xutil.h> */
int XLookupString(XKeyEvent* event_struct, char* buffer_return, int bytes_buffer, KeySym* keysym_return, XComposeStatus* status_in_out)
{
}

/* <X11/Xutil.h> */
Status XMatchVisualInfo(Display* display, int screen, int depth, int class, XVisualInfo* vinfo_return)
{
}

/* <X11/Xutil.h> */
int XOffsetRegion(Region r, int dx, int dy)
{
}

/* <X11/Xutil.h> */
Bool XPointInRegion(Region r, int x, int y)
{
}

/* <X11/Xutil.h> */
Region XPolygonRegion(XPoint* points, int n, int fill_rule)
{
}

/* <X11/Xutil.h> */
int XRectInRegion(Region r, int x, int y, unsigned int width, unsigned int height)
{
}

/* <X11/Xutil.h> */
int XSaveContext(Display* display, XID rid, XContext context, const char* data)
{
}

/* <X11/Xutil.h> */
int XSetClassHint(Display* display, Window w, XClassHint* class_hints)
{
}

/* <X11/Xutil.h> */
int XSetIconSizes(Display* display, Window w, XIconSize* size_list, int	count)
{
}

/* <X11/Xutil.h> */
int XSetNormalHints(Display* display, Window w, XSizeHints* hints)
{
}

/* <X11/Xutil.h> */
void XSetRGBColormaps(Display* display, Window w, XStandardColormap* stdcmaps, int count, Atom property)
{
}

/* <X11/Xutil.h> */
int XSetSizeHints(Display* display, Window w, XSizeHints* hints, Atom property)
{
}

/* <X11/Xutil.h> */
int XSetStandardProperties(Display* display, Window w, const char* window_name, const char* icon_name, Pixmap icon_pixmap, char** argv, int argc, XSizeHints* hints)
{
}

/* <X11/Xutil.h> */
void XSetTextProperty(Display* display, Window w, XTextProperty* text_prop, Atom property)
{
}

/* <X11/Xutil.h> */
void XSetWMClientMachine(Display* display, Window w, XTextProperty* text_prop)
{
}

/* <X11/Xutil.h> */
int XSetWMHints(Display* display, Window w, XWMHints* wm_hints)
{
}

/* <X11/Xutil.h> */
void XSetWMIconName(Display* display, Window w, XTextProperty* text_prop)
{
}

/* <X11/Xutil.h> */
void XSetWMName(Display* display, Window w, XTextProperty* text_prop)
{
}

/* <X11/Xutil.h> */
void XSetWMNormalHints(Display* display, Window w, XSizeHints* hints)
{
}

/* <X11/Xutil.h> */
void XSetWMProperties(Display* display, Window w, XTextProperty* window_name, XTextProperty* icon_name, char** argv, int argc, XSizeHints* normal_hints, XWMHints* wm_hints, XClassHint* class_hints)
{
}

/* <X11/Xutil.h> */
void XmbSetWMProperties(Display* display, Window w, const char* window_name, const char* icon_name, char** argv, int argc, XSizeHints* normal_hints, XWMHints* wm_hints, XClassHint* class_hints)
{
}

/* <X11/Xutil.h> */
void XSetWMSizeHints(Display* display, Window w, XSizeHints* hints, Atom property)
{
}

/* <X11/Xutil.h> */
int XSetRegion(Display* display, GC gc, Region r)
{
}

/* <X11/Xutil.h> */
void XSetStandardColormap(Display* display, Window w, XStandardColormap* colormap, Atom property)
{
}

/* <X11/Xutil.h> */
int XSetZoomHints(Display* display, Window w, XSizeHints* zhints)
{
}

/* <X11/Xutil.h> */
int XShrinkRegion(Region r, int dx, int dy)
{
}

/* <X11/Xutil.h> */
Status XStringListToTextProperty(char** list, int count, XTextProperty* text_prop_return)
{
}

/* <X11/Xutil.h> */
int XSubtractRegion(Region sra, Region srb, Region dr_return)
{
}

/* <X11/Xutil.h> */
int XmbTextListToTextProperty(Display* display, char** list, int count, XICCEncodingStyle style, XTextProperty* text_prop_return)
{
}

/* <X11/Xutil.h> */
int XwcTextListToTextProperty(Display* display, wchar_t** list, int count, XICCEncodingStyle style, XTextProperty* text_prop_return)
{
}

/* <X11/Xutil.h> */
void XwcFreeStringList(wchar_t** list)
{
}

/* <X11/Xutil.h> */
Status XTextPropertyToStringList(XTextProperty* text_prop, char*** list_return, int* count_return)
{
}

/* <X11/Xutil.h> */
int XmbTextPropertyToTextList(Display* display, XTextProperty* text_prop, char*** list_return, int* count_return)
{
}

/* <X11/Xutil.h> */
int XwcTextPropertyToTextList(Display* display, XTextProperty* text_prop, wchar_t*** list_return, int* count_return)
{
}

/* <X11/Xutil.h> */
int XUnionRectWithRegion(XRectangle* rectangle, Region src_region, Region dest_region_return)
{
}

/* <X11/Xutil.h> */
int XUnionRegion(Region sra, Region srb, Region dr_return)
{
}

/* <X11/Xutil.h> */
int XWMGeometry(Display* display, int screen_number, const char* user_geometry, const char* default_geometry, unsigned int border_width, XSizeHints* hints, int* x_return, int* y_return, int* width_return, int* height_return, int* gravity_return)
{
}

/* <X11/Xutil.h> */
int XXorRegion(Region sra, Region srb, Region dr_return)
{
}

/***********************************************************

Copyright (c) 1987  X Consortium

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
X CONSORTIUM BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of the X Consortium shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from the X Consortium.


Copyright 1987 by Digital Equipment Corporation, Maynard, Massachusetts.

                        All Rights Reserved

Permission to use, copy, modify, and distribute this software and its 
documentation for any purpose and without fee is hereby granted, 
provided that the above copyright notice appear in all copies and that
both that copyright notice and this permission notice appear in 
supporting documentation, and that the name of Digital not be
used in advertising or publicity pertaining to distribution of the
software without specific, written prior permission.  

DIGITAL DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE, INCLUDING
ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO EVENT SHALL
DIGITAL BE LIABLE FOR ANY SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION,
ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS
SOFTWARE.

******************************************************************/
