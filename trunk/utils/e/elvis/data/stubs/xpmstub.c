/* This file is derived from the <X11/xpm.h> header file, which is copyrighted
 * by GROUPE BULL.  See the full copyright notice at the end of this file.
 */

/* <X11/xpm.h> -lxpm Return codes */
#define XpmColorError    1	/* minor errors are positive */
#define XpmSuccess       0	/* success is 0 */
#define XpmOpenFailed   -1	/* major errors are negative */
#define XpmFileInvalid  -2
#define XpmNoMemory     -3
#define XpmColorFailed  -4

typedef struct {
    char *name;			/* Symbolic color name */
    char *value;		/* Color value */
    Pixel pixel;		/* Color pixel */
}      XpmColorSymbol;

typedef struct {
    char *name;			/* name of the extension */
    unsigned int nlines;	/* number of lines in this extension */
    char **lines;		/* pointer to the extension array of strings */
}      XpmExtension;

typedef struct {
    char *string;		/* characters string */
    char *symbolic;		/* symbolic name */
    char *m_color;		/* monochrom default */
    char *g4_color;		/* 4 level grayscale default */
    char *g_color;		/* other level grayscale default */
    char *c_color;		/* color default */
}      XpmColor;

typedef struct {
    unsigned int width;		/* image width */
    unsigned int height;	/* image height */
    unsigned int cpp;		/* number of characters per pixel */
    unsigned int ncolors;	/* number of colors */
    XpmColor *colorTable;	/* list of related colors */
    unsigned int *data;		/* image data */
}      XpmImage;

typedef struct {
    unsigned long valuemask;	/* Specifies which attributes are defined */
    char *hints_cmt;		/* Comment of the hints section */
    char *colors_cmt;		/* Comment of the colors section */
    char *pixels_cmt;		/* Comment of the pixels section */
    unsigned int x_hotspot;	/* Returns the x hotspot's coordinate */
    unsigned int y_hotspot;	/* Returns the y hotspot's coordinate */
    unsigned int nextensions;	/* number of extensions */
    XpmExtension *extensions;	/* pointer to array of extensions */
}      XpmInfo;

typedef int (*XpmAllocColorFunc)(Display *display, Colormap colormap, char *colorname, XColor *xcolor, void *closure);

typedef int (*XpmFreeColorsFunc)(Display *display, Colormap colormap, Pixel *pixels, int npixels, void *closure);

typedef struct {
    unsigned long valuemask;		/* Specifies which attributes are
					   defined */

    Visual *visual;			/* Specifies the visual to use */
    Colormap colormap;			/* Specifies the colormap to use */
    unsigned int depth;			/* Specifies the depth */
    unsigned int width;			/* Returns the width of the created
					   pixmap */
    unsigned int height;		/* Returns the height of the created
					   pixmap */
    unsigned int x_hotspot;		/* Returns the x hotspot's
					   coordinate */
    unsigned int y_hotspot;		/* Returns the y hotspot's
					   coordinate */
    unsigned int cpp;			/* Specifies the number of char per
					   pixel */
    Pixel *pixels;			/* List of used color pixels */
    unsigned int npixels;		/* Number of used pixels */
    XpmColorSymbol *colorsymbols;	/* List of color symbols to override */
    unsigned int numsymbols;		/* Number of symbols */
    char *rgb_fname;			/* RGB text file name */
    unsigned int nextensions;		/* Number of extensions */
    XpmExtension *extensions;		/* List of extensions */

    unsigned int ncolors;               /* Number of colors */
    XpmColor *colorTable;               /* List of colors */
/* 3.2 backward compatibility code */
    char *hints_cmt;                    /* Comment of the hints section */
    char *colors_cmt;                   /* Comment of the colors section */
    char *pixels_cmt;                   /* Comment of the pixels section */
/* end 3.2 bc */
    unsigned int mask_pixel;            /* Color table index of transparent
                                           color */

    /* Color Allocation Directives */
    Bool exactColors;			/* Only use exact colors for visual */
    unsigned int closeness;		/* Allowable RGB deviation */
    unsigned int red_closeness;		/* Allowable red deviation */
    unsigned int green_closeness;	/* Allowable green deviation */
    unsigned int blue_closeness;	/* Allowable blue deviation */
    int color_key;			/* Use colors from this color set */

    Pixel *alloc_pixels;		/* Returns the list of alloc'ed color
					   pixels */
    int nalloc_pixels;			/* Returns the number of alloc'ed
					   color pixels */

    Bool alloc_close_colors;    	/* Specify whether close colors should
					   be allocated using XAllocColor
					   or not */
    int bitmap_format;			/* Specify the format of 1bit depth
					   images: ZPixmap or XYBitmap */

    /* Color functions */
    XpmAllocColorFunc alloc_color;	/* Application color allocator */
    XpmFreeColorsFunc free_colors;	/* Application color de-allocator */
    void *color_closure;		/* Application private data to pass to
					   alloc_color and free_colors */

}      XpmAttributes;

/* XpmAttributes value masks bits */
#define XpmVisual	   (1L<<0)
#define XpmColormap	   (1L<<1)
#define XpmDepth	   (1L<<2)
#define XpmSize		   (1L<<3)	/* width & height */
#define XpmHotspot	   (1L<<4)	/* x_hotspot & y_hotspot */
#define XpmCharsPerPixel   (1L<<5)
#define XpmColorSymbols	   (1L<<6)
#define XpmRgbFilename	   (1L<<7)
/* 3.2 backward compatibility code */
#define XpmInfos	   (1L<<8)
#define XpmReturnInfos	   XpmInfos
/* end 3.2 bc */
#define XpmReturnPixels	   (1L<<9)
#define XpmExtensions      (1L<<10)
#define XpmReturnExtensions XpmExtensions

#define XpmExactColors     (1L<<11)
#define XpmCloseness	   (1L<<12)
#define XpmRGBCloseness	   (1L<<13)
#define XpmColorKey	   (1L<<14)

#define XpmColorTable      (1L<<15)
#define XpmReturnColorTable XpmColorTable

#define XpmReturnAllocPixels (1L<<16)
#define XpmAllocCloseColors (1L<<17)
#define XpmBitmapFormat    (1L<<18)

#define XpmAllocColor      (1L<<19)
#define XpmFreeColors      (1L<<20)
#define XpmColorClosure    (1L<<21)


/* XpmInfo value masks bits */
#define XpmComments        XpmInfos
#define XpmReturnComments  XpmComments

/* XpmAttributes mask_pixel value when there is no mask */
#define XpmUndefPixel 0x80000000

/*
 * color keys for visual type, they must fit along with the number key of
 * each related element in xpmColorKeys[] defined in XpmI.h
 */
#define XPM_MONO	2
#define XPM_GREY4	3
#define XPM_GRAY4	3
#define XPM_GREY 	4
#define XPM_GRAY 	4
#define XPM_COLOR	5


/* <X11/xpm.h> -lX11 */
int XpmCreatePixmapFromData(Display *display, Drawable d, char **data, Pixmap *pixmap_return, Pixmap *shapemask_return, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateDataFromPixmap(Display *display, char ***data_return, Pixmap pixmap, Pixmap shapemask, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmReadFileToPixmap(Display *display, Drawable d, char *filename, Pixmap *pixmap_return, Pixmap *shapemask_return, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmWriteFileFromPixmap(Display *display, char *filename, Pixmap pixmap, Pixmap shapemask, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateImageFromData(Display *display, char **data, XImage **image_return, XImage **shapemask_return, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateDataFromImage(Display *display, char ***data_return, XImage *image, XImage *shapeimage, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmReadFileToImage(Display *display, char *filename, XImage **image_return, XImage **shapeimage_return, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmWriteFileFromImage(Display *display, char *filename, XImage *image, XImage *shapeimage, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateImageFromBuffer(Display *display, char *buffer, XImage **image_return, XImage **shapemask_return, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreatePixmapFromBuffer(Display *display, Drawable d, char *buffer, Pixmap *pixmap_return, Pixmap *shapemask_return, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateBufferFromImage(Display *display, char **buffer_return, XImage *image, XImage *shapeimage, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateBufferFromPixmap(Display *display, char **buffer_return, Pixmap pixmap, Pixmap shapemask, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmReadFileToBuffer(char *filename, char **buffer_return)
{
}

/* <X11/xpm.h> -lX11 */
int XpmWriteFileFromBuffer(char *filename, char *buffer)
{
}

/* <X11/xpm.h> -lX11 */
int XpmReadFileToData(char *filename, char ***data_return)
{
}

/* <X11/xpm.h> -lX11 */
int XpmWriteFileFromData(char *filename, char **data)
{
}

/* <X11/xpm.h> -lX11 */
int XpmAttributesSize(void)
{
}

/* <X11/xpm.h> -lX11 */
void XpmFreeAttributes(XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
void XpmFreeExtensions(XpmExtension *extensions, int nextensions)
{
}

/* <X11/xpm.h> -lX11 */
void XpmFreeXpmImage(XpmImage *image)
{
}

/* <X11/xpm.h> -lX11 */
void XpmFreeXpmInfo(XpmInfo *info)
{
}

/* <X11/xpm.h> -lX11 */
char * XpmGetErrorString(int errcode)
{
}

/* <X11/xpm.h> -lX11 */
int XpmLibraryVersion(void)
{
}

/* <X11/xpm.h> -lX11 */
int XpmReadFileToXpmImage(char *filename, XpmImage *image, XpmInfo *info)
{
}

/* <X11/xpm.h> -lX11 */
int XpmWriteFileFromXpmImage(char *filename, XpmImage *image, XpmInfo *info)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreatePixmapFromXpmImage(Display *display, Drawable d, XpmImage *image, Pixmap *pixmap_return, Pixmap *shapemask_return, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateImageFromXpmImage(Display *display, XpmImage *image, XImage **image_return, XImage **shapeimage_return, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateXpmImageFromImage(Display *display, XImage *image, XImage *shapeimage, XpmImage *xpmimage, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateXpmImageFromPixmap(Display *display, Pixmap pixmap, Pixmap shapemask, XpmImage *xpmimage, XpmAttributes *attributes)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateDataFromXpmImage(char ***data_return, XpmImage *image, XpmInfo *info)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateXpmImageFromData(char **data, XpmImage *image, XpmInfo *info)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateXpmImageFromBuffer(char *buffer, XpmImage *image, XpmInfo *info)
{
}

/* <X11/xpm.h> -lX11 */
int XpmCreateBufferFromXpmImage(char **buffer_return, XpmImage *image, XpmInfo *info)
{
}

/* <X11/xpm.h> -lX11 */
int XpmGetParseError(char *filename, int *linenum_return, int *charnum_return)
{
}

/* <X11/xpm.h> -lX11 */
void XpmFree(void *ptr)
{
}

/* <Xlib.h> backward compatibility, for version 3.0c */
#define XpmPixmapColorError   XpmColorError
#define XpmPixmapSuccess      XpmSuccess
#define XpmPixmapOpenFailed   XpmOpenFailed
#define XpmPixmapFileInvalid  XpmFileInvalid
#define XpmPixmapNoMemory     XpmNoMemory
#define XpmPixmapColorFailed  XpmColorFailed
#define XpmReadPixmapFile     XpmReadFileToPixmap
#define XpmWritePixmapFile    XpmWriteFileFromPixmap

/* <Xlib.h> backward compatibility, for version 3.0b */
#define PixmapColorError      XpmColorError
#define PixmapSuccess         XpmSuccess
#define PixmapOpenFailed      XpmOpenFailed
#define PixmapFileInvalid     XpmFileInvalid
#define PixmapNoMemory        XpmNoMemory
#define PixmapColorFailed     XpmColorFailed
#define ColorSymbol           XpmColorSymbol
#define XReadPixmapFile       XpmReadFileToPixmap
#define XWritePixmapFile      XpmWriteFileFromPixmap
#define XCreatePixmapFromData XpmCreatePixmapFromData
#define XCreateDataFromPixmap XpmCreateDataFromPixmap

/*
 * Copyright (C) 1989-95 GROUPE BULL
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * GROUPE BULL BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of GROUPE BULL shall not be
 * used in advertising or otherwise to promote the sale, use or other dealings
 * in this Software without prior written authorization from GROUPE BULL.
 */

