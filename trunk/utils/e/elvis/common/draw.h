/* draw.h */
/* Copyright 1995 by Steve Kirkendall */


/* Attributes of characters currently shown on the screen.  Note that the new
 * image stores 1-byte font codes; the DRAWATTR struct is only used for the
 * old image.  This is because the new image needs some extra information
 * (especially the GUI's numbers for the colors) which isn't stored in DRAWATTR.
 * Also, any temporary font codes (created via colortmp()) are still allocated
 * while the new image is being used, but could be freed by the time the old
 * image is used.
 */
typedef struct
{
	unsigned short	bits;		/* attribute bits of a font code */
	unsigned char	fg_rgb[3];	/* foreground color */
	unsigned char	bg_rgb[3];	/* background color */
} DRAWATTR;

/* This macro returns a font's attributes.  It is clever enough to convert
 * "selected" font codes 0x80-0xff into real font codes with the COLOR_SEL
 * attribute bit set.
 */
#define drawfontbits(f)		(colorinfo[0x7f & (f)].da.bits | ((0x80 & (f)) ? COLOR_SEL : 0))

/* This macro compares new attributes to old attributes, at a given index.
 * This is non-trivial since they're stored differently.  It also compares
 * the character.  Return 0 if same, else non-0.
 */
#define drawnochange(di,i)	((di)->newchar[i] == (di)->curchar[i] \
				&& drawfontbits((di)->newfont[i]) == (di)->curattr[i].bits \
				&& !memcmp(&colorinfo[0x7f & (int)(di)->newfont[i]].da.fg_rgb, \
						&(di)->curattr[i].fg_rgb, 6))

/* Return TRUE if current attributes match the default font */
#define drawdeffont(di,i)	!memcmp(&colorinfo[0].da, \
					&(di)->curattr[i], sizeof(DRAWATTR))

/* Return TRUE if the current attributes at two locations are the same */
#define drawspan(di,i,j)	!memcmp(&(di)->curattr[i], \
					&(di)->curattr[j], sizeof(DRAWATTR))

/* information about a row of the screen */
typedef struct
{
	long	lineoffset; /* which line this row is for */
	int	insrows;
	int	shiftright;
	int	inschars;
} DRAWROW;

/* information about a line that's drawn on the screen.  Note that this is
 * different from rows; rows are screen-oriented, while lines are defined by
 * the buffer and the display mode.
 */
typedef struct
{
	long	start;	/* offset from start of buffer to start of line */
	int	width;	/* width of the line */
	int	startrow;/* where it appears on the screen */
} DRAWLINE;

/* possible states of a window, affecting the way it is updated */
typedef enum
{
	DRAW_VISUAL,    /* visual, no message or status msg */
	DRAW_VMSG,      /* visual, non-status message */
	DRAW_OPENEDIT,  /* open, editing a line */
	DRAW_OPENOUTPUT /* open, after outputing a message */
} DRAWSTATE;

/* affects efficiency of screen updates.  The DRAW_CENTER value also affects
 * the way the new image is generated.
 */
typedef enum
{
	DRAW_NORMAL,	/* optimizable, refreshable, cursor on screen */
	DRAW_CHANGED,	/* non-optimizable, refreshable, cursor on screen */
	DRAW_CENTER,	/* non-optimizable, refreshable, cursor in top half */
	DRAW_SCRATCH 	/* non-optimizable, non-refreshable, cursor on screen */
} DRAWLOGIC;


/* This collects all of the information that the "draw.c" module requires to
 * update a window's image efficiently.
 */
typedef struct
{
	DRAWSTATE drawstate;	/* drawing state */
	DRAWROW	 *newrow;	/* info about new rows */
	DRAWLINE *newline;	/* info about new lines */
	DRAWLINE *curline;	/* info about current lines */
	CHAR	 *newchar;	/* characters of new image */
	char	 *newfont;	/* fonts of new image */
	CHAR	 *curchar;	/* characters of current image */
	DRAWATTR *curattr;	/* font info of current image */
	long	 *offsets;	/* buffer offsets of each individual cell */
	MARK	 topmark;	/* first line drawn */
	MARK	 bottommark;	/* line after last drawn */
	BUFFER	 curbuf;	/* current buffer */
	long	 curnbytes;	/* size of buffer when current image drawn */
	long	 curchgs;	/* buffer's "changes" counter when image drawn */
	int	 rows, columns; /* dimensions of screen */
	int	 cursrow, curscol;/* position of cursor */
	int	 skipped;	/* number of columns skipped from first line */
	int	 nlines;	/* number of lines */
	DRAWLOGIC logic;	/* ignore current image? */
	ELVBOOL	 newmsg;	/* does msg row contain anything important? */
	MARK	 openline;	/* current line (open mode only) */
	CHAR	 *openimage;	/* image of current line, '\0'-terminated */
	long	 opencursor;	/* where cursor is within line */
	long	 opencnt;	/* width of line in openimage */
	int	 opencell;	/* tty simulator's cursor position */
	ELVBOOL	 tmpmsg;	/* blank out the status line, unless newmsg */
	char	 cursface;	/* text face of cursor, in visual mode only */
} DRAWINFO;

