/* This file contains stubs of most Xft library functions, used for rendering
 * antialiased text.  This is intended to serve as a reference; there's no
 * point in compiling it.  The idea is that you put this file in an accessible
 * but out-of-the-way location, and run ctags on it.  Then you set the TAGPATH
 * environment variable to include this file's directory.  From that point on,
 * you can * use the normal tag searching functions to see the declaration for
 * any * of these functions.  In particular, the "ref" program distributed with
 * elvis is handy for this.
 *
 * This file is derived from the <X11/Xft/Xft.h> header file.  See the
 * copyright notice at the end of this file.
 */


/* <X11/extensions/Xrender.h> */
typedef struct {
    short   red;
    short   redMask;
    short   green;
    short   greenMask;
    short   blue;
    short   blueMask;
    short   alpha;
    short   alphaMask;
} XRenderDirectFormat;

/* <X11/extensions/Xrender.h> */
typedef struct {
    PictFormat		id;
    int			type;
    int			depth;
    XRenderDirectFormat	direct;
    Colormap		colormap;
} XRenderPictFormat;

/* <X11/extensions/Xrender.h>  - indicates used fields XRenderPixtFormat */
#define PictFormatID	    (1 << 0)
#define PictFormatType	    (1 << 1)
#define PictFormatDepth	    (1 << 2)
#define PictFormatRed	    (1 << 3)
#define PictFormatRedMask   (1 << 4)
#define PictFormatGreen	    (1 << 5)
#define PictFormatGreenMask (1 << 6)
#define PictFormatBlue	    (1 << 7)
#define PictFormatBlueMask  (1 << 8)
#define PictFormatAlpha	    (1 << 9)
#define PictFormatAlphaMask (1 << 10)
#define PictFormatColormap  (1 << 11)

/* <X11/extensions/Xrender.h> */
typedef struct {
    Visual		*visual;
    XRenderPictFormat	*format;
} XRenderVisual;

/* <X11/extensions/Xrender.h> */
typedef struct {
    int			depth;
    int			nvisuals;
    XRenderVisual	*visuals;
} XRenderDepth;

/* <X11/extensions/Xrender.h> */
typedef struct {
    XRenderDepth	*depths;
    int			ndepths;
    XRenderPictFormat	*fallback;
} XRenderScreen;

/* <X11/extensions/Xrender.h> */
typedef struct _XRenderInfo {
    XRenderPictFormat	*format;
    int			nformat;
    XRenderScreen	*screen;
    int			nscreen;
    XRenderDepth	*depth;
    int			ndepth;
    XRenderVisual	*visual;
    int			nvisual;
} XRenderInfo;

/* <X11/extensions/Xrender.h> */
typedef struct _XRenderPictureAttributes {
    Bool		repeat;
    Picture		alpha_map;
    int			alpha_x_origin;
    int			alpha_y_origin;
    int			clip_x_origin;
    int			clip_y_origin;
    Pixmap		clip_mask;
    Bool		graphics_exposures;
    int			subwindow_mode;
    int			poly_edge;
    int			poly_mode;
    Atom		dither;
    Bool		component_alpha;
} XRenderPictureAttributes;

/* <X11/extensions/Xrender.h> */
typedef struct {
    unsigned short   red;
    unsigned short   green;
    unsigned short   blue;
    unsigned short   alpha;
} XRenderColor;

/* <X11/extensions/Xrender.h> */
typedef struct _XGlyphInfo {
    unsigned short  width;
    unsigned short  height;
    short	    x;
    short	    y;
    short	    xOff;
    short	    yOff;
} XGlyphInfo;

/* <X11/extensions/Xrender.h> */
Bool XRenderQueryExtension(Display *dpy, int *event_basep, int *error_basep)
{
}

/* <X11/extensions/Xrender.h> */
Status XRenderQueryVersion(Display *dpy, int *major_versionp, int *minor_versionp)
{
}

/* <X11/extensions/Xrender.h> */
Status XRenderQueryFormats(Display *dpy)
{
}

/* <X11/extensions/Xrender.h> */
XRenderPictFormat *XRenderFindVisualFormat(Display *dpy, Visual *visual)
{
}

/* <X11/extensions/Xrender.h> */
XRenderPictFormat *XRenderFindFormat(Display *dpy, unsigned long mask, XRenderPictFormat *templ, int count)
{
}
    
/* <X11/extensions/Xrender.h> */
Picture XRenderCreatePicture(Display *dpy, Drawable drawable, XRenderPictFormat *format, unsigned long valuemask, XRenderPictureAttributes *attributes)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderChangePicture(Display *dpy, Picture picture, unsigned long valuemask, XRenderPictureAttributes *attributes)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderSetPictureClipRectangles(Display *dpy, Picture picture, int xOrigin, int yOrigin, XRectangle *rects, int n)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderSetPictureClipRegion(Display *dpy, Picture picture, Region r)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderFreePicture(Display *dpy, Picture picture)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderComposite(Display *dpy, int op, Picture src, Picture mask, Picture dst, int src_x, int src_y, int mask_x, int mask_y, int dst_x, int dst_y, unsigned int width, unsigned int height)
{
}

/* <X11/extensions/Xrender.h> */
GlyphSet XRenderCreateGlyphSet(Display *dpy, XRenderPictFormat *format)
{
}

/* <X11/extensions/Xrender.h> */
GlyphSet XRenderReferenceGlyphSet(Display *dpy, GlyphSet existing)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderFreeGlyphSet(Display *dpy, GlyphSet glyphset)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderAddGlyphs(Display *dpy, GlyphSet glyphset, Glyph *gids, XGlyphInfo *glyphs, int nglyphs, char *images, int nbyte_images)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderFreeGlyphs(Display *dpy, GlyphSet glyphset, Glyph *gids, int nglyphs)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderCompositeString8(Display *dpy, int op, Picture src, Picture dst, XRenderPictFormat *maskFormat, GlyphSet glyphset, int xSrc, int ySrc, int xDst, int yDst, char *string, int nchar)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderCompositeString16(Display *dpy, int op, Picture src, Picture dst, XRenderPictFormat *maskFormat, GlyphSet glyphset, int xSrc, int ySrc, int xDst, int yDst, unsigned short *string, int nchar)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderCompositeString32(Display *dpy, int op, Picture src, Picture dst, XRenderPictFormat *maskFormat, GlyphSet glyphset, int xSrc, int ySrc, int xDst, int yDst, unsigned int *string, int nchar)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderFillRectangle(Display *dpy, int op, Picture dst, XRenderColor *color, int x, int y, unsigned int width, unsigned int height)
{
}

/* <X11/extensions/Xrender.h> */
void XRenderFillRectangles(Display *dpy, int op, Picture dst, XRenderColor *color, XRectangle *rectangles, int n_rects)
{
}

/*--------------------------------------------------------------------*/

/* <X11/Xft/Xft.h> */
typedef unsigned char XftChar8;

/* <X11/Xft/Xft.h> */
typedef unsigned short XftChar16;

/* <X11/Xft/Xft.h> */
typedef unsigned int XftChar32;

/* <X11/Xft/Xft.h> */
#define XFT_FAMILY	    "family"	/* String */

/* <X11/Xft/Xft.h> */
#define XFT_STYLE	    "style"	/* String */

/* <X11/Xft/Xft.h> */
#define XFT_SLANT	    "slant"	/* Int */

/* <X11/Xft/Xft.h> */
#define XFT_WEIGHT	    "weight"	/* Int */

/* <X11/Xft/Xft.h> */
#define XFT_SIZE	    "size"	/* Double */

/* <X11/Xft/Xft.h> */
#define XFT_PIXEL_SIZE	    "pixelsize"	/* Double */

/* <X11/Xft/Xft.h> */
#define XFT_ENCODING	    "encoding"	/* String */

/* <X11/Xft/Xft.h> */
#define XFT_SPACING	    "spacing"	/* Int */

/* <X11/Xft/Xft.h> */
#define XFT_FOUNDRY	    "foundry"	/* String */

/* <X11/Xft/Xft.h> */
#define XFT_CORE	    "core"	/* Bool */

/* <X11/Xft/Xft.h> */
#define XFT_ANTIALIAS	    "antialias"	/* Bool */

/* <X11/Xft/Xft.h> */
#define XFT_XLFD	    "xlfd"	/* String */

/* <X11/Xft/Xft.h> */
#define XFT_FILE	    "file"	/* String */

/* <X11/Xft/Xft.h> */
#define XFT_INDEX	    "index"	/* Int */

/* <X11/Xft/Xft.h> */
#define XFT_RASTERIZER	    "rasterizer"/* String */

/* <X11/Xft/Xft.h> */
#define XFT_OUTLINE	    "outline"	/* Bool */

/* <X11/Xft/Xft.h> */
#define XFT_SCALABLE	    "scalable"	/* Bool */

/* <X11/Xft/Xft.h> */
#define XFT_RGBA	    "rgba"	/* Int */

/* <X11/Xft/Xft.h> */
#define XFT_SCALE	    "scale"	/* double */

/* <X11/Xft/Xft.h> */
#define XFT_RENDER	    "render"	/* Bool */

/* <X11/Xft/Xft.h> */
#define XFT_MINSPACE	    "minspace"	/* Bool use minimum line spacing */

/* <X11/Xft/Xft.h> */
#define XFT_DPI		    "dpi"	/* double */

/* <X11/Xft/Xft.h> */
#define XFT_CHAR_WIDTH	    "charwidth"	/* Int */

/* <X11/Xft/Xft.h> */
#define XFT_CHAR_HEIGHT	    "charheight"/* Int */

/* <X11/Xft/Xft.h> */
#define XFT_MATRIX	    "matrix"    /* XftMatrix */

/* <X11/Xft/Xft.h> */
#define XFT_WEIGHT_LIGHT	0

/* <X11/Xft/Xft.h> */
#define XFT_WEIGHT_MEDIUM	100

/* <X11/Xft/Xft.h> */
#define XFT_WEIGHT_DEMIBOLD	180

/* <X11/Xft/Xft.h> */
#define XFT_WEIGHT_BOLD		200

/* <X11/Xft/Xft.h> */
#define XFT_WEIGHT_BLACK	210

/* <X11/Xft/Xft.h> */
#define XFT_SLANT_ROMAN		0

/* <X11/Xft/Xft.h> */
#define XFT_SLANT_ITALIC	100

/* <X11/Xft/Xft.h> */
#define XFT_SLANT_OBLIQUE	110

/* <X11/Xft/Xft.h> */
#define XFT_PROPORTIONAL    0

/* <X11/Xft/Xft.h> */
#define XFT_MONO	    100

/* <X11/Xft/Xft.h> */
#define XFT_CHARCELL	    110

/* <X11/Xft/Xft.h> */
#define XFT_RGBA_NONE	    0

/* <X11/Xft/Xft.h> */
#define XFT_RGBA_RGB	    1

/* <X11/Xft/Xft.h> */
#define XFT_RGBA_BGR	    2

/* <X11/Xft/Xft.h> */
#define XFT_RGBA_VRGB	    3

/* <X11/Xft/Xft.h> */
#define XFT_RGBA_VBGR	    4

/* <X11/Xft/Xft.h> */
typedef enum _XftType {
    XftTypeVoid, 
    XftTypeInteger, 
    XftTypeDouble, 
    XftTypeString, 
    XftTypeBool,
    XftTypeMatrix
} XftType;

/* <X11/Xft/Xft.h> */
typedef struct _XftMatrix {
    double xx, xy, yx, yy;
} XftMatrix;

/* <X11/Xft/Xft.h> */
#define XftMatrixInit(m)	/* sets "m" to the identity matrix */

/* <X11/Xft/Xft.h> */
typedef enum _XftResult {
    XftResultMatch, XftResultNoMatch, XftResultTypeMismatch, XftResultNoId
} XftResult;

/* <X11/Xft/Xft.h> */
typedef struct _XftValue {
    XftType	type;
    union {
	char    *s;
	int	i;
	Bool	b;
	double	d;
	XftMatrix *m;
    } u;
} XftValue;

/* <X11/Xft/Xft.h> */
typedef struct _XftValueList {
    struct _XftValueList    *next;
    XftValue		    value;
} XftValueList;

/* <X11/Xft/Xft.h> */
typedef struct _XftPatternElt {
    const char	    *object;
    XftValueList    *values;
} XftPatternElt;

/* <X11/Xft/Xft.h> */
typedef struct _XftPattern {
    int		    num;
    int		    size;
    XftPatternElt   *elts;
} XftPattern;

/* <X11/Xft/Xft.h> */
typedef struct _XftFontSet {
    int		nfont;
    int		sfont;
    XftPattern	**fonts;
} XftFontSet;

/* <X11/Xft/Xft.h> */
typedef struct _XftFontStruct	XftFontStruct;

/* <X11/Xft/Xft.h> */
typedef struct _XftFont {
    int		ascent;
    int		descent;
    int		height;
    int		max_advance_width;
    Bool	core;
    XftPattern	*pattern;
    union {
	struct {
	    XFontStruct	    *font;
	} core;
	struct {
	    XftFontStruct   *font;
	} ft;
    } u;
} XftFont;

/* <X11/Xft/Xft.h> */
typedef struct _XftDraw XftDraw;

/* <X11/Xft/Xft.h> */
typedef struct _XftColor {
    unsigned long   pixel;
    XRenderColor    color;
} XftColor;

/* <X11/Xft/Xft.h> */
typedef struct _XftObjectSet {
    int		nobject;
    int		sobject;
    const char	**objects;
} XftObjectSet;

/* <X11/Xft/Xft.h> */
Bool XftConfigSubstitute(XftPattern *p)
{
}

/* <X11/Xft/Xft.h> */
Bool XftColorAllocName(Display *dpy, Visual *visual, Colormap cmap, char *name, XftColor *result)
{
}

/* <X11/Xft/Xft.h> */
Bool XftColorAllocValue(Display *dpy, Visual*visual, Colormap cmap, XRenderColor *color, XftColor *result)
{
}

/* <X11/Xft/Xft.h> */
void XftColorFree(Display *dpy, Visual *visual, Colormap cmap, XftColor *color)
{
}

/* <X11/Xft/Xft.h> */
void XftValuePrint(XftValue v)
{
}

/* <X11/Xft/Xft.h> */
void XftValueListPrint(XftValueList *l)
{
}

/* <X11/Xft/Xft.h> */
void XftPatternPrint(XftPattern *p)
{
}

/* <X11/Xft/Xft.h> */
void XftFontSetPrint(XftFontSet *s)
{
}

/* <X11/Xft/Xft.h> */
Bool XftDefaultHasRender(Display *dpy)
{
}

/* <X11/Xft/Xft.h> */
Bool XftDefaultSet(Display *dpy, XftPattern *defaults)
{
}

/* <X11/Xft/Xft.h> */
void XftDefaultSubstitute(Display *dpy, int screen, XftPattern *pattern)
{
}

/* <X11/Xft/Xft.h> */
XftDraw *XftDrawCreate(Display *dpy, Drawable drawable, Visual *visual, Colormap colormap)
{
}

/* <X11/Xft/Xft.h> */
XftDraw *XftDrawCreateBitmap(Display *dpy, Pixmap bitmap)
{
}

/* <X11/Xft/Xft.h> */
void XftDrawChange(XftDraw *draw, Drawable drawable)
{
}

/* <X11/Xft/Xft.h> */
void XftDrawDestroy(XftDraw *draw)
{
}

/* <X11/Xft/Xft.h> */
void XftDrawString8(XftDraw *d, XftColor *color, XftFont *font, int x, int y, XftChar8 *string, int len)
{
}

/* <X11/Xft/Xft.h> */
void XftDrawString16(XftDraw *draw, XftColor *color, XftFont *font, int x, int y, XftChar16 *string, int len)
{
}

/* <X11/Xft/Xft.h> */
void XftDrawString32(XftDraw *draw, XftColor *color, XftFont *font, int x, int y, XftChar32 *string, int len)
{
}

/* <X11/Xft/Xft.h> */
void XftDrawStringUtf8(XftDraw *d, XftColor *color, XftFont *font, int x, int y, XftChar8 *string, int len)
{
}

/* <X11/Xft/Xft.h> */
void XftDrawRect(XftDraw *d, XftColor *color, int x, int y, unsigned int width, unsigned int height)
{
}

/* <X11/Xft/Xft.h> */
Bool XftDrawSetClip(XftDraw *d, Region r)
{
}

/* <X11/Xft/Xft.h> */
void XftTextExtents8(Display *dpy, XftFont *font, XftChar8 *string, int len, XGlyphInfo *extents)
{
}

/* <X11/Xft/Xft.h> */
void XftTextExtents16(Display *dpy, XftFont *font, XftChar16 *string, int len, XGlyphInfo *extents)
{
}

/* <X11/Xft/Xft.h> */
void XftTextExtents32(Display *dpy, XftFont *font, XftChar32 *string, int len, XGlyphInfo *extents)
{
}

/* <X11/Xft/Xft.h> */
void XftTextExtentsUtf8(Display *dpy, XftFont *font, XftChar8 *string, int len, XGlyphInfo *extents)
{
}

/* <X11/Xft/Xft.h> */
XftPattern *XftFontMatch(Display *dpy, int screen, XftPattern *pattern, XftResult *result)
{
}

/* <X11/Xft/Xft.h> */
XftFont *XftFontOpenPattern(Display *dpy, XftPattern *pattern)
{
}

/* <X11/Xft/Xft.h> */
XftFont *XftFontOpen(Display *dpy, int screen, ...)
{
}

/* <X11/Xft/Xft.h> */
XftFont *XftFontOpenName(Display *dpy, int screen, const char *name)
{
}

/* <X11/Xft/Xft.h> */
XftFont *XftFontOpenXlfd(Display *dpy, int screen, const char *xlfd)
{
}

/* <X11/Xft/Xft.h> */
void XftFontClose(Display *dpy, XftFont *font)
{
}

/* <X11/Xft/Xft.h> */
Bool XftGlyphExists(Display *dpy, XftFont *font, XftChar32 glyph)
{
}

/* <X11/Xft/Xft.h> */
XftFontSet *XftFontSetCreate(void)
{
}

/* <X11/Xft/Xft.h> */
void XftFontSetDestroy(XftFontSet *s)
{
}

/* <X11/Xft/Xft.h> */
Bool XftFontSetAdd(XftFontSet *s, XftPattern *font)
{
}

/* <X11/Xft/Xft.h> */
Bool XftInit(char *config)
{
}

/* <X11/Xft/Xft.h> */
XftObjectSet *XftObjectSetCreate(void)
{
}

/* <X11/Xft/Xft.h> */
Bool XftObjectSetAdd(XftObjectSet *os, const char *object)
{
}

/* <X11/Xft/Xft.h> */
void XftObjectSetDestroy(XftObjectSet *os)
{
}

/* <X11/Xft/Xft.h> */
XftObjectSet *XftObjectSetVaBuild(const char *first, va_list va)
{
}

/* <X11/Xft/Xft.h> */
XftObjectSet *XftObjectSetBuild(const char *first, ...)
{
}

/* <X11/Xft/Xft.h> */
XftFontSet *XftListFontSets(XftFontSet **sets, int nsets, XftPattern *p, XftObjectSet *os)
{
}

/* <X11/Xft/Xft.h> */
XftFontSet *XftListFontsPatternObjects(Display *dpy, int screen, XftPattern *pattern, XftObjectSet *os)
{
}

/* <X11/Xft/Xft.h> */
XftFontSet *XftListFonts(Display *dpy, int screen, ...)
{
}

/* <X11/Xft/Xft.h> */
XftPattern *XftFontSetMatch(XftFontSet **sets, int nsets, XftPattern *p, XftResult *result)
{
}

/* <X11/Xft/Xft.h> */
int XftMatrixEqual(const XftMatrix *mat1, const XftMatrix *mat2)
{
}

/* <X11/Xft/Xft.h> */
void XftMatrixMultiply(XftMatrix *result, XftMatrix *a, XftMatrix *b)
{
}

/* <X11/Xft/Xft.h> */
void XftMatrixRotate(XftMatrix *m, double c, double s)
{
}

/* <X11/Xft/Xft.h> */
void XftMatrixScale(XftMatrix *m, double sx, double sy)
{
}

/* <X11/Xft/Xft.h> */
void XftMatrixShear(XftMatrix *m, double sh, double sv)
{
}

/* <X11/Xft/Xft.h> */
XftPattern *XftNameParse(const char *name)
{
}

/* <X11/Xft/Xft.h> */
Bool XftNameUnparse(XftPattern *pat, char *dest, int len)
{
}

/* <X11/Xft/Xft.h> */
XftPattern *XftPatternCreate(void)
{
}

/* <X11/Xft/Xft.h> */
XftPattern *XftPatternDuplicate(XftPattern *p)
{
}

/* <X11/Xft/Xft.h> */
void XftValueDestroy(XftValue v)
{
}

/* <X11/Xft/Xft.h> */
void XftValueListDestroy(XftValueList *l)
{
}

/* <X11/Xft/Xft.h> */
void XftPatternDestroy(XftPattern *p)
{
}

/* <X11/Xft/Xft.h> */
XftPatternElt *XftPatternFind(XftPattern *p, const char *object, Bool insert)
{
}

/* <X11/Xft/Xft.h> */
Bool XftPatternAdd(XftPattern *p, const char *object, XftValue value, Bool append)
{
}

/* <X11/Xft/Xft.h> */
XftResult XftPatternGet(XftPattern *p, const char *object, int id, XftValue *v)
{
}

/* <X11/Xft/Xft.h> */
Bool XftPatternDel(XftPattern *p, const char *object)
{
}

/* <X11/Xft/Xft.h> */
Bool XftPatternAddInteger(XftPattern *p, const char *object, int i)
{
}

/* <X11/Xft/Xft.h> */
Bool XftPatternAddDouble(XftPattern *p, const char *object, double d)
{
}

/* <X11/Xft/Xft.h> */
Bool XftPatternAddString(XftPattern *p, const char *object, const char *s)
{
}

/* <X11/Xft/Xft.h> */
Bool XftPatternAddMatrix(XftPattern *p, const char *object, const XftMatrix *s)
{
}

/* <X11/Xft/Xft.h> */
Bool XftPatternAddBool(XftPattern *p, const char *object, Bool b)
{
}

/* <X11/Xft/Xft.h> */
XftResult XftPatternGetInteger(XftPattern *p, const char *object, int n, int *i)
{
}

/* <X11/Xft/Xft.h> */
XftResult XftPatternGetDouble(XftPattern *p, const char *object, int n, double *d)
{
}

/* <X11/Xft/Xft.h> */
XftResult XftPatternGetString(XftPattern *p, const char *object, int n, char **s)
{
}

/* <X11/Xft/Xft.h> */
XftResult XftPatternGetMatrix(XftPattern *p, const char *object, int n, XftMatrix **s)
{
}

/* <X11/Xft/Xft.h> */
XftResult XftPatternGetBool(XftPattern *p, const char *object, int n, Bool *b)
{
}

/* <X11/Xft/Xft.h> */
XftPattern *XftPatternVaBuild(XftPattern *orig, va_list va)
{
}

/* <X11/Xft/Xft.h> */
XftPattern *XftPatternBuild(XftPattern *orig, ...)
{
}

/* <X11/Xft/Xft.h> */
int XftUtf8ToUcs4(XftChar8 *src_orig, XftChar32 *dst, int len)
{
}

/* <X11/Xft/Xft.h> */
Bool XftUtf8Len(XftChar8 *string, int len, int *nchar, int *wchar)
{
}

/* <X11/Xft/Xft.h> */
XftPattern *XftXlfdParse(const char *xlfd_orig, Bool ignore_scalable, Bool complete)
{
}

/* <X11/Xft/Xft.h> */
XFontStruct *XftCoreOpen(Display *dpy, XftPattern *pattern)
{
}

/* <X11/Xft/Xft.h> */
void XftCoreClose(Display *dpy, XFontStruct *font)
{
}

/*
 * $XFree86: xc/lib/Xft/Xft.h,v 1.19 2001/04/29 03:21:17 keithp Exp $
 *
 * Copyright © 2000 Keith Packard, member of The XFree86 Project, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation, and that the name of Keith Packard not be used in
 * advertising or publicity pertaining to distribution of the software without
 * specific, written prior permission.  Keith Packard makes no
 * representations about the suitability of this software for any purpose.  It
 * is provided "as is" without express or implied warranty.
 *
 * KEITH PACKARD DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL KEITH PACKARD BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */
