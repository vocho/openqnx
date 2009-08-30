/* color.c */
/* Copyright 1999 by Steve Kirkendall */

#include "elvis.h"

typedef struct guidot_s
{
	struct guidot_s *next;
	CHAR	*name;	/* "gui.role" form of some other gui's colors */
	CHAR	*descr;	/* description of the attributes */
} guidot_t;

COLORINFO colorinfo[128] =
{
	{NULL},			/* the null color */
	{toCHAR("normal")},	/* the "normal" color */
	{toCHAR("idle")},	/* the "idle" color */
	{toCHAR("bottom")},	/* the "bottom" color */
	{toCHAR("selection")},	/* the "selection" color */
	{toCHAR("hlsearch")},	/* the "hlsearch" color */
	{toCHAR("ruler")},	/* the "ruler" color -- ruler on bottom row */
	{toCHAR("showmode")},	/* the "showmode" colors -- showmode value */
	{toCHAR("lnum")},	/* the "lnum" colors -- line numbers */
	{toCHAR("nontext")}	/* the "nontext" colors -- tilde lines */
};
int colornpermanent = COLOR_FONT_QTY_SPECIALS;
int ntemporary;	/* index of next temporary colorinfo[] to use */

/* This array is used to store the sort sequence for the colors.  We don't
 * simply sort the colorinfo[] array in place because we don't want to move
 * any font after it has been assigned a position in colorinfo[].
 */
int colorsortorder[128] = {
	COLOR_FONT_BOTTOM,
	COLOR_FONT_HLSEARCH,
	COLOR_FONT_IDLE,
	COLOR_FONT_LNUM,
	COLOR_FONT_NONTEXT,
	COLOR_FONT_NORMAL,
	COLOR_FONT_RULER,
	COLOR_FONT_SELECTION,
	COLOR_FONT_SHOWMODE
};

/* This array is used for remembering the two fonts that got merged to make a
 * new temporary font, so that we can reuse it elsewhere on the screen during
 * the same refresh.
 */
static struct
{
	unsigned char oldfont, newfont;
} tmpfonts[128];

#ifdef FEATURE_MKEXRC
/* This is the head of a list of colors that would be used in a different
 * GUI.  We store this for the sole purpose of making :mkexrc work better
 * for multiple GUIs.
 */
static guidot_t *guidot;
#endif

/* Forward declarations for static functions */
static ELVBOOL orcolor P_((unsigned char cur[3], unsigned char or[3], unsigned char bg[3]));
static void colorlike P_((int fontcode));
static void recolor P_((int fontcode));

/* background colors in RGB format, used for choosing foreground color */
static unsigned char light[3] = {255, 255, 255};
static unsigned char dark[3] = {0, 0, 0};
#define LP_BG		light
#define VIDEO_BG	((colorinfo[COLOR_FONT_NORMAL].da.bits & COLOR_BG) \
				? colorinfo[COLOR_FONT_NORMAL].da.bg_rgb \
				: o_background == 'l' ? light : dark)

#ifdef FEATURE_IMAGE
/* Separate a background description into two parts: A solid color and an
 * image file name.  Search for the image file in the current directory, or
 * a "themes" subdirectory of an element in elvispath.  Return the image file
 * name or NULL if no file was given or it wasn't found.  Put a NUL character
 * at the end of the color part, so that upon return bgname[] contains only
 * the color name or "" if no color was given.
 */
char *colorimage(bgname)
	CHAR	*bgname;	/* buffer holding color name & image name */
{
	CHAR	*imgname;
	CHAR	*themes;
	static char *fullname;

	/* if there was a previous fullname, free it now */
	if (fullname)
		safefree(fullname);
	fullname = NULL;

	/* Look for an image name within the color name.  An image name always
	 * contains punctuation, but we can't use elvpunct() because this
	 * function may be called before elvis' digraphs are configured, so
	 * we just look for a few specific punctuation marks.
	 */
	for (imgname = bgname;
	     *imgname && CHARchr(toCHAR("./\\:"), *imgname) == NULL;
	     imgname++)
	{
	}

	/* if no image name, then return NULL. */
	if (!*imgname)
		return NULL;

	/* Found punctuation!  Go back to start of word */
	while (imgname != bgname && !elvspace(*imgname))
		imgname--;
	if (imgname != bgname)
		imgname++;

	/* If we were given an image name, and the name isn't that of a file in
	 * the current directory, then try searching through elvispath
	 */
	if (*imgname && dirperm(tochar8(imgname)) < DIR_READONLY)
	{
		themes = NULL;
		buildstr(&themes, "themes/");
		buildstr(&themes, tochar8(imgname));
		fullname = iopath(tochar8(o_elvispath), tochar8(themes), ElvFalse);
		safefree(themes);
	}
	else
		fullname = tochar8(imgname);

	/* allocate a copy of the fullname, unless it is null */
	if (fullname)
		fullname = safekdup(fullname);

	/* Place a NUL character before the image name, to separate the
	 * color name from the image name.  If there is no color name, then
	 * just place a NUL at the front of the buffer.
	 */
	if (imgname != bgname)
		imgname--;
	*imgname = '\0';

	/* return the full name of the image file */
	return fullname;
}
#endif /* FEATURE_IMAGE */


/* This is used for setting the "background" option.  We can't simply use
 * optisoneof() because we want to detect when the value is changed and the
 * exact background color is unknown, so we can calculate foreground colors.
 */
int colorisbg(desc, val, newval)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
	CHAR	*newval;/* value the option should have (as a string) */
{
	int	i;

	/* use optisoneof() to do the actual setting.  If invalid or unchanged,
	 * or if we know the real background, then return the result code.
	 */
	i = optisoneof(desc, val, newval);
	if (i != 1 || (colorinfo[COLOR_FONT_NORMAL].da.bits & COLOR_BG) != 0)
		return i;

	/* Changed! And we don't know the real background, so that matters! */
	for (i = 1; i < colornpermanent; i++)
		if (!colorinfo[i].like)
			recolor(i);

	/* return the "changed" result code */
	return 1;
}


/* This function chooses either a new RGB color or an old one, based on how
 * well it contrasts with a third RGB color.  If the new one is used, it is
 * copied over the old one and ElvTrue is returned; else ElvFalse is returned.
 */
static ELVBOOL orcolor(cur, or, bg)
	unsigned char	cur[3];	/* current color */
	unsigned char	or[3];	/* new color */
	unsigned char	bg[3];	/* background color */
{
	long curdist;	/* distance between current fg and bg */
	long ordist;	/* distance between "or" fg and bg */
	long r, g, b;	/* partial distances */

	/* compute curdist */
	r = (long)cur[0] - (long)bg[0];
	g = (long)cur[1] - (long)bg[1];
	b = (long)cur[2] - (long)bg[2];
	curdist = 5 * r * r + 6 * g * g + 4 * b * b;

	/* compute ordist */
	r = (long)or[0] - (long)bg[0];
	g = (long)or[1] - (long)bg[1];
	b = (long)or[2] - (long)bg[2];
	ordist = 5 * r * r + 6 * g * g + 4 * b * b;

	/* if ordist contrasts better than curdist, then use it */
	if (ordist > curdist)
	{
		memcpy(cur, or, 3);
		return ElvTrue;
	}
	return ElvFalse;
}

/* Returns the index of a font name.  If the fontname isn't in the colorinfo[]
 * table already, then this adds it.  If it can't add it (because the table is
 * already full) then it outputs an error message and returns 0.
 */
int colorfind(fontname)
	CHAR *fontname;
{
	int	i;

	/* scan the list */
	for (i = 1; i < colornpermanent; i++)
		if (!CHARcmp(colorinfo[i].name, fontname))
			return i;

	/* must be a new color -- is there room? */
	if (i >= QTY(colorinfo))
	{
		msg(MSG_ERROR, "[S]color table full -- can't add $1", fontname);
		return 0;
	}

	/* allocate the color */
	colorinfo[i] = colorinfo[0];
	colorinfo[i].name = CHARkdup(fontname);
	while (--i >= 1 && CHARcmp(colorinfo[colorsortorder[i - 1]].name, colorinfo[colornpermanent].name) > 0)
		colorsortorder[i] = colorsortorder[i - 1];
	colorsortorder[i] = colornpermanent;

	return colornpermanent++;
}

/* Combine two fonts, and return the result in a static variable */
COLORINFO *colorcombine(oldfont, newcinfo)
	int	oldfont;	/* index of the old font */
	COLORINFO *newcinfo;	/* information about new font */
{
	static COLORINFO combo;
	COLORINFO	*old = &colorinfo[oldfont];

	if (&combo != newcinfo)
		combo = *newcinfo;
	if (~combo.da.bits & COLOR_FGSET)
	{
		combo.fg = old->fg;
		memcpy(combo.da.fg_rgb, old->da.fg_rgb, 3);
		memcpy(combo.lpfg_rgb, old->lpfg_rgb, 3);
		combo.da.bits |= old->da.bits & (COLOR_FG|COLOR_FGSET);
	}
	if (~combo.da.bits & COLOR_BGSET)
	{
		combo.bg = colorinfo[oldfont].bg;
		memcpy(combo.da.bg_rgb, colorinfo[oldfont].da.bg_rgb, 3);
		combo.da.bits |= old->da.bits & (COLOR_BG|COLOR_BGSET);
	}
	if (combo.da.bits & COLOR_PROPSET)
		combo.da.bits |= (colorinfo[oldfont].da.bits & ~(COLOR_SET|COLOR_FGSET|COLOR_BGSET|COLOR_PROP));
	else
		combo.da.bits |= (colorinfo[oldfont].da.bits & ~(COLOR_SET|COLOR_FGSET|COLOR_BGSET|COLOR_PROPSET));
	return &combo;
}


/* Parse a description string.  The flags are converted into bits in *bitsref,
 * but the other attributes are returned as strings.
 */
void colorparse(descr, fgref,bgref, likeref, bitsref)
	CHAR       	*descr;	/* description of the attributes */
	CHAR	       **fgref;	/* reference to foreground string pointer */
	CHAR	       **bgref;	/* reference to background string pointer */
	CHAR	       **likeref;/* reference to "like" string pointer */
	unsigned short *bitsref;/* reference to "bits" value */
{
	CHAR	*scan, *build, *word;
	enum	{EXPECT_LIKE, EXPECT_FG, EXPECT_BG} expect, nextexpect;
	int	i;

	/* by default, assume nothing */
	*fgref = NULL;
	*bgref = NULL;
	*likeref = '\0';
	*bitsref = 0;

	/* if no description, then we're done! */
	if (!descr)
		return;

	/* parse the words */
	build = word = NULL;
	expect = nextexpect = EXPECT_FG;
	for (scan = descr; ; scan++)
	{
		/* part of a word? */
		if (*scan && !elvspace(*scan))
		{
			(void)buildCHAR(&build, *scan);
			if (!word)
				word = build;
			goto Continue;
		}	

		/* Whitespace. If after other whitespace, then ignore */
		if (!build)
			goto Continue;

		/* okay, we've hit the end of a word.  If we were
		 * expecting a role name, then this is it.
		 */
		if (expect == EXPECT_LIKE)
		{
			/* if already had a "like", this one overrides it */
			if (*likeref)
				safefree(*likeref);
			*likeref = CHARdup(word);
			nextexpect = EXPECT_FG;
			goto ContinueNewWord;
		}

		/* Is it a word that has special meaning? */
		if (!CHARcmp(word, toCHAR("like")))
			nextexpect = EXPECT_LIKE;
		else if (!CHARcmp(word, toCHAR("on")))
			nextexpect = EXPECT_BG;
		else if (!CHARcmp(word, toCHAR("bold")))
			*bitsref |= COLOR_BOLD;
		else if (!CHARcmp(word, toCHAR("italic")))
			*bitsref |= COLOR_ITALIC;
		else if (!CHARcmp(word, toCHAR("underlined")))
			*bitsref |= COLOR_UNDERLINED;
		else if (!CHARcmp(word, toCHAR("boxed")))
			*bitsref |= COLOR_BOXED;
		else if (!CHARcmp(word, toCHAR("proportional")))
			*bitsref |= COLOR_PROP|COLOR_PROPSET;
		else if (!CHARcmp(word, toCHAR("fixed")))
			*bitsref = (*bitsref & ~COLOR_PROP) | COLOR_PROPSET;
		else if (!CHARcmp(word, toCHAR("graphic")))
			*bitsref |= COLOR_GRAPHIC;
		else
			word = NULL;

		/* If the preceding word was special or we've hit the
		 * end of the descr string, and some other text
		 * preceded the word, then that other text must be
		 * some kind of argument.
		 */
		if ((word || !*scan) && build != word)
		{
			/* If a special word appeared, strip it from
			 * the end of the other text.
			 */
			if (word)
				word[-1] = '\0';

			/* Strip any trailing whitespace.  (This can
			 * happen at the end of the command string.)
			 */
			for (i = CHARlen(build); elvspace(build[--i]); )
			{
			}
			build[i + 1] = '\0';

			/* What kind of text is it? */
			switch (expect)
			{
			  case EXPECT_LIKE:
				/* can't happen -- handled above */
				break;

			  case EXPECT_FG:
				if (*fgref)
					safefree(*fgref);
				*fgref = CHARdup(build);
				break;

			  case EXPECT_BG:
				if (*bgref)
					safefree(*bgref);
				*bgref = CHARdup(build);
				break;
			}
			expect = nextexpect;
		} /* end if ((word || !*scan) && build != word) */

		/* If we're continuing a long arg such as a color name,
		 * then add a space.  Otherwise free the string.
		 */
		if (!word && *scan)
		{
			/* continuing, so add a space */
			buildCHAR(&build, ' ');
			word = build + CHARlen(build);
		}
		else
		{
ContinueNewWord:
			/* free the old string, prepare for a new one */
			safefree(build);
			build = word = NULL;
			expect = nextexpect;

Continue:
			/* if end of string, then quit */
			if (!*scan)
				break;
		}

	}

	/* clean up */
	if (build)
		safefree(build);
}

/* Interpret the "descr" string for the given fontcode.  Then call
 * colorlike(fontcode) to adjust any fonts which are "like" this one.
 */
static void recolor(fontcode)
	int	fontcode;
{
	COLORINFO newinfo, tmpinfo, *combo;
	long	oldfg[20], oldbg;
	int	noldfg, i, j;
	CHAR	*fgstr, *bgstr, *likestr;
	CHAR	*colornam, *scan;
	unsigned short oldbits;
	ELVBOOL	used;
#ifdef FEATURE_AUTOCMD
	ELVBOOL	bgchanged = ElvFalse;
#endif

	/* if there is no description, then don't change anything. */
	if (!colorinfo[fontcode].descr)
	{
		/* if this font has no "like" field, then do nothing */
		if (!colorinfo[fontcode].like)
			return;

		/* inherit some attributes from the "like" font */
		newinfo = colorinfo[fontcode];
		combo = colorcombine(newinfo.like, &newinfo);
		colorinfo[fontcode].fg = combo->fg;
		memcpy(colorinfo[fontcode].da.fg_rgb, combo->da.fg_rgb, 3);
		memcpy(colorinfo[fontcode].lpfg_rgb, combo->lpfg_rgb, 3);
		colorinfo[fontcode].bg = combo->bg;
		memcpy(colorinfo[fontcode].da.bg_rgb, combo->da.bg_rgb, 3);
		colorinfo[fontcode].da.bits = newinfo.da.bits | combo->da.bits;

		/* if there are other fonts that are "like" this, update them */
		colorlike(fontcode);

		return;
	}

	/* reset newinfo */
	newinfo = colorinfo[0];
	noldfg = 0;

	/* parse the description */
	colorparse(colorinfo[fontcode].descr,
		   &fgstr, &bgstr, &likestr, &newinfo.da.bits);

	/* were we given a "like" font? */
	if (likestr)
	{
		newinfo.like = colorfind(likestr);
		safefree(likestr);
	}

	/* were we given a background color? */
	if (bgstr)
	{
		/* parse it, if the GUI supports color */
		if (gui->color
		 && (*gui->color)(fontcode, bgstr, ElvFalse,
						&newinfo.bg, newinfo.da.bg_rgb))
		{
			newinfo.da.bits |= (COLOR_BG|COLOR_BGSET);
		}
		safefree(bgstr);
	}

	/* Were we given a foreground color? */
	if (fgstr)
	{
		/* It may actually be a series of colors, separated by "or" */
		for (colornam = scan = fgstr; gui->color && *scan; scan++)
		{
			/* If not the end of a color name (the word "or", or
			 * the end of the description string) then loop.  If
			 * "or" then stick a '\0' at the end of the color name.
			 */
			if (!CHARncmp(scan, toCHAR(" or "), 4))
			{
				*scan = '\0';
				scan += 3;
			}
			else if (scan[1])
				continue;

			/* Convert the color.  If can't convert, skip it */
			if (!(*gui->color)(fontcode, colornam, ElvTrue,
						&tmpinfo.fg, tmpinfo.da.fg_rgb))
			{
				colornam = scan + 1;
				continue;
			}

			/* was this the first color? */
			if ((newinfo.da.bits & COLOR_FGSET) == 0)
			{
				/* Yes!  First color is simply stored */
				newinfo.fg = tmpinfo.fg;
				memcpy(newinfo.da.fg_rgb, tmpinfo.da.fg_rgb, 3);
				memcpy(newinfo.lpfg_rgb, tmpinfo.da.fg_rgb, 3);
				newinfo.da.bits |= (COLOR_FG|COLOR_FGSET);
			}
			else
			{
				/* if contrasts with white, remember for :lpr */
				(void)orcolor(newinfo.lpfg_rgb,
					tmpinfo.da.fg_rgb, LP_BG);

				/* if contrasts with normal bg, remember it */
				if (orcolor(newinfo.da.fg_rgb,
					tmpinfo.da.fg_rgb, VIDEO_BG))
				{
					/* discard old fg, use the new one */
					oldfg[noldfg++] = newinfo.fg;
					newinfo.fg = tmpinfo.fg;
				}
				else
				{
					/* discard "or" fg */
					oldfg[noldfg++] = tmpinfo.fg;
				}
			}

			/* prepare for next color, if there is one */
			colornam = scan + 1;

		} /* end for each color in a list delimited by "or" words */

		safefree(fgstr);

	} /* end if (fgstr) */

	/* Remember the old colors, so we can free them later */
	oldfg[noldfg++] = colorinfo[fontcode].fg;
	oldbg = colorinfo[fontcode].bg;
	oldbits = colorinfo[fontcode].da.bits;

	/* combine the new info with the "like" info, and store it */
	combo = newinfo.like ? colorcombine(newinfo.like, &newinfo) : &newinfo;
	colorinfo[fontcode].fg = combo->fg;
	memcpy(colorinfo[fontcode].da.fg_rgb, combo->da.fg_rgb, 3);
	memcpy(colorinfo[fontcode].lpfg_rgb, combo->lpfg_rgb, 3);
	colorinfo[fontcode].bg = combo->bg;
	memcpy(colorinfo[fontcode].da.bg_rgb, combo->da.bg_rgb, 3);
	colorinfo[fontcode].da.bits = newinfo.da.bits | combo->da.bits;
	colorinfo[fontcode].like = newinfo.like;

	/* if it was explicitly set before, then it is still explicitly set */
	colorinfo[fontcode].da.bits |= oldbits & COLOR_SET;

	/* the "idle" font should take default fg & bg from "normal" */
	if (fontcode == COLOR_FONT_IDLE)
	{
		if (~colorinfo[fontcode].da.bits & COLOR_FG)
		{
			colorinfo[fontcode].fg = colorinfo[COLOR_FONT_NORMAL].fg;
			memcpy(colorinfo[fontcode].da.fg_rgb,
				colorinfo[COLOR_FONT_NORMAL].da.fg_rgb, 3);
			colorinfo[fontcode].da.bits |= COLOR_FG;
		}
		if (~colorinfo[fontcode].da.bits & COLOR_BG)
		{
			colorinfo[fontcode].bg = colorinfo[COLOR_FONT_NORMAL].bg;
			memcpy(colorinfo[fontcode].da.bg_rgb,
				colorinfo[COLOR_FONT_NORMAL].da.bg_rgb, 3);
			colorinfo[fontcode].da.bits |= COLOR_BG;
		}
	}

	/* if this GUI has a freecolor() function, then decide whether to
	 * free any colors.
	 */
	if (gui->freecolor)
	{
		/* maybe free the old foreground color */
		for (i = 0; i < noldfg; i++)
		{
			for (j=1, used=ElvFalse; j < colornpermanent && !used; j++)
				if (colorinfo[j].fg == oldfg[i]
				 || colorinfo[j].bg == oldfg[i])
					used = ElvTrue;
			if (!used)
			{
				(*gui->freecolor)(oldfg[i], ElvTrue);
			}
		}

		/* maybe free the old background color */
		if (oldbits & COLOR_BG)
		{
			for (i=1, used=ElvFalse; i < colornpermanent && !used; i++)
				if (colorinfo[i].fg == oldbg
				 || colorinfo[i].bg == oldbg)
					used = ElvTrue;
			if (!used)
			{
				(*gui->freecolor)(oldbg, ElvFalse);
			}
		}
	}

	/* call colorlike() to adjust any colors which are "like" this one */
	colorlike(fontcode);

#ifdef FEATURE_AUTOCMD
	if (bgchanged && fontcode == COLOR_FONT_NORMAL)
		(void)auperform(windefault, ElvFalse, NULL, AU_BGCHANGED,
			toCHAR(colorinfo[fontcode].da.bg_rgb[0]
			     + colorinfo[fontcode].da.bg_rgb[1]
			     + colorinfo[fontcode].da.bg_rgb[2] >= 384
			     				? "light"
			     				: "dark"));
#endif
}

/* Find each entry in the color table which has "like" that matches "fontcode",
 * and reinterpret its "descr" field by calling recolor() on it.
 */
static void colorlike(fontcode)
	int	fontcode;
{
	int	i;
	int	oldlike;

	/* Set this font's "like" value to an impossible value temporarily,
	 * to avoid any chance of an endless loop due to cyclic dependencies.
	 */
	oldlike = colorinfo[fontcode].like;
	colorinfo[fontcode].like = 0;

	/* adjust any other colors which are "like" this one.  Also, if the
	 * fontcode is "normal" then we want to recolor everything, since
	 * a changed background color could cause the "or color" to be used
	 * for any font.
	 */
	for (i = 1; i < colornpermanent; i++)
		if (colorinfo[i].like == fontcode
		 || (fontcode == COLOR_FONT_NORMAL && i != fontcode))
			recolor(i);

	/* restore this font's "like" value */
	colorinfo[fontcode].like = oldlike;
}


/* Change the description for a given fontcode, and then recolor it so the
 * changes take effect.  The "explicit" parameter should be ElvTrue if called
 * from :color, or ElvFalse if set to a default value via elvis.syn or something
 * similar.
 */
void colorset(fontcode, descr, explicit)
	int	fontcode;
	CHAR	*descr;
	ELVBOOL	explicit;
{
	ELVBOOL	washidden;
	CHAR	*fg, *bg, *like;
	unsigned short bits;
	CHAR	*newfg, *newbg, *newlike;
	unsigned short newbits;
	CHAR	*oldfg, *oldbg, *oldlike;
	unsigned short oldbits;
	CHAR	oper;
	CHAR	*build;
#ifdef FEATURE_AUTOCMD
	ELVBOOL bgchanged = ElvFalse;
#endif

	/* If not explicit, and a previous colorset was explicit, then do
	 * nothing here.  Defaults never override explicit settings.
	 */
	if (!explicit && (colorinfo[fontcode].da.bits & COLOR_SET) != 0)
		return;

	/* parse an optional "+=" or "-=" at the front of the description */
	oper = '=';
	if (*descr == '+' || *descr == '-')
		oper = *descr++;
	if (*descr == '=')
		descr++;
	while (elvspace(*descr))
		descr++;

	/* parse the old description, and then free it */
	colorparse(colorinfo[fontcode].descr, &oldfg, &oldbg, &oldlike, &oldbits);

	/* parse the new description */
	colorparse(descr, &newfg, &newbg, &newlike, &newbits);

#ifdef FEATURE_AUTOCMD
	/* are we changing the normal background? */
	if (fontcode == COLOR_FONT_NORMAL
	 && newbg
	 && oper != '-'
	 && (!oldbg || CHARcmp(newbg, oldbg)))
		bgchanged = ElvTrue;
#endif

	/* "normal" can never be "like" another color.  "idle" can't be "like"
	 * anything except "normal"
	 */
	if (newlike
	 && (fontcode == COLOR_FONT_NORMAL
	 || (fontcode == COLOR_FONT_IDLE && CHARcmp(newlike, toCHAR("normal")))))
	{
		safefree(newlike);
		newlike = NULL;
	}

	/* combine them, under control of the operator */
	switch (oper)
	{
	  case '+':
		fg = newfg ? newfg : oldfg;
		bg = newbg ? newbg : oldbg;
		like = newlike ? newlike : oldlike;
		bits = oldbits | newbits;
		break;

	  case '-':
		fg = newfg ? NULL : oldfg;
		bg = newbg ? NULL : oldbg;
		like = newlike ? NULL : oldlike;
		bits = oldbits & ~newbits;
		break;
	  
	  default:
		if (fontcode==COLOR_FONT_NORMAL)
		{
			fg = newfg ? newfg : oldfg;
			bg = newbg ? newbg : oldbg;
		}
		else
		{
			fg = newfg;
			bg = newbg;
		}
		like = newlike;
		bits = newbits;
		break;
	}

	/* generate a whole new description string from the parsed info */
	build = NULL;
	if (like)
	{
		buildstr(&build, "like ");
		buildstr(&build, tochar8(like));
		buildCHAR(&build, ' ');
	}
	if (bits & COLOR_BOLD)
		buildstr(&build, "bold ");
	if (bits & COLOR_ITALIC)
		buildstr(&build, "italic ");
	if (bits & COLOR_UNDERLINED)
		buildstr(&build, "underlined ");
	if (bits & COLOR_BOXED)
		buildstr(&build, "boxed ");
	if (bits & COLOR_GRAPHIC)
		buildstr(&build, "graphic ");
	if ((bits & (COLOR_PROP|COLOR_PROPSET)) == COLOR_PROPSET)
		buildstr(&build, "fixed ");
	else if (bits & COLOR_PROPSET)
		buildstr(&build, "proportional ");
	if (fg)
	{
		buildstr(&build, tochar8(fg));
		buildCHAR(&build, ' ');
	}
	if (bg)
	{
		buildstr(&build, "on ");
		buildstr(&build, tochar8(bg));
		buildCHAR(&build, ' ');
	}

	/* remove the extra ' ' from the end of the description */
	if (build)
		build[CHARlen(build) - 1] = '\0';

	/* store the newly generated description string */
	if (colorinfo[fontcode].descr)
		safefree(colorinfo[fontcode].descr);
	colorinfo[fontcode].descr = build;

	/* free the parsed strings */
	if (oldfg)   safefree(oldfg);
	if (oldbg)   safefree(oldbg);
	if (oldlike) safefree(oldlike);
	if (newfg)   safefree(newfg);
	if (newbg)   safefree(newbg);
	if (newlike) safefree(newlike);

	/* force the COLOR_SET bit on, if necessary */
	if (explicit || (colorinfo[fontcode].da.bits & COLOR_SET))
		colorinfo[fontcode].da.bits |= COLOR_SET;

	/* Call recolor() to do the hard work.  If this is a non-explicit
	 * setting, then disable error messages while it runs, since there's
	 * no good reason to badger the user for my mistakes.
	 */
	washidden = msghide((ELVBOOL)!explicit);
	recolor(fontcode);
	(void)msghide(washidden);

#ifdef FEATURE_AUTOCMD
	/* If we changed the normal background, then do a BgChanged event */
	if (bgchanged)
		(void)auperform(windefault, ElvFalse, NULL, AU_BGCHANGED,
			toCHAR(colorinfo[fontcode].da.bg_rgb[0] +
			       colorinfo[fontcode].da.bg_rgb[1] +
			       colorinfo[fontcode].da.bg_rgb[2]>=384 ? "light" 
								     : "dark"));
#endif
}

/* Set a color for a GUI other than the one we're using.  This has no effect
 * except that it is incorporated into the :mkexrc script.
 */
void colorforeign(name, descr)
	CHAR	*name;	/* name, formatted as "gui.role" */
	CHAR	*descr;	/* the attributes for this color role */
{
#ifdef FEATURE_MKEXRC
	guidot_t *scan, *newp;

	/* If already set, then change the description */
	for (scan = guidot; scan; scan = scan->next)
	{
		if (!CHARcmp(scan->name, name))
		{
			safefree(scan->descr);
			scan->descr = CHARkdup(descr);
			return;
		}
	}

	/* else insert a new entry at the tail of the list */
	newp = safekept(1, sizeof(guidot_t));
	newp->name = CHARkdup(name);
	newp->descr = CHARkdup(descr);
	newp->next = NULL;
	if (!guidot)
		guidot = newp;
	else
	{
		for (scan = guidot; scan->next; scan = scan->next)
		{
		}
		newp->next = scan->next;
		scan->next = newp;
	}
#endif /* FEATURE_MKEXRC */
}


/* Reset the temporary color combination counter.  This is called by the
 * drawimage() function before the display->setup() function.
 */
void colorsetup()
{
	/* Set ntemporary to start allocating immediately after the
	 * permanent fonts.
	 */
	ntemporary = colornpermanent;
}

/* If possible, allocate a new temporary font for storing the result of
 * combining oldfont and newfont, and return the combined font's index.  If
 * that isn't possible (because we ran out of space in the color table) then
 * just return the newfont.
 */
int colortmp(oldfont, newfont)
	int	oldfont;
	int	newfont;
{
	int	i;
	int	sel;

	/* strip off the "selection" bit */
	sel = (oldfont | newfont) & 0x80;
	oldfont &= 0x7f;
	newfont &= 0x7f;

	/* is it one of the previously allocated temporary fonts? */
	for (i = ntemporary; --i >= colornpermanent; )
		if (tmpfonts[i].oldfont == oldfont && tmpfonts[i].newfont == newfont)
			return i | sel;

	/* if no more room for combinations, then return the newfont.  Also use
	 * newfont if there is no oldfont, or we're combining a font with
	 * itself.
	 */
	if (oldfont == 0 || oldfont == newfont || ntemporary >= QTY(colorinfo))
		return newfont | sel;

	/* combine the fonts to form a new one */
	i = ntemporary++;
	colorinfo[i] = *colorcombine(oldfont, &colorinfo[newfont]);

	/* remember info to help optimimize the next call */
	tmpfonts[i].oldfont = oldfont;
	tmpfonts[i].newfont = newfont;

#ifdef FEATURE_SPELL
	/* alert the spellchecker about the new temporary font */
	spelltmp(oldfont, newfont, i);
#endif

	/* return the combined font */
	return (i | sel);
}

/* Create a temporary font with the given attributes, and return its font code.
 * This is used by the drawexpose() function.  It is also called from
 * updateimage(), but should be superfluous unless updateimage() was itself
 * called from drawopenedit().  Note that drawopenedit() calls colorsetup(),
 * so ntemporary=colornpermanent.
 */
int colorexpose(font, refattr)
	_char_		font;		/* old font code */
	DRAWATTR	*refattr;	/* attributes to show */
{
	int	i;
	int	select;

	/* if old font code is still valid, then just return it */
	select = font & 0x80;
	if ((font & 0x7f) < ntemporary)
		return font;

	/* else we need to choose a temporary font code */
	font = QTY(colorinfo) - 1;

	/* stuff the attributes into the color */
	memset(&colorinfo[font], 0, sizeof colorinfo[font]);
	colorinfo[font].da = *refattr;

	/* now we need to convert foreground & background colors from RGB to
	 * the corresponding color codes.
	 */
	for (i = 1; memcmp(colorinfo[i].da.fg_rgb, refattr->fg_rgb, 3); i++)
	{
	}
	colorinfo[font].fg = colorinfo[i].fg;
	for (i = 1; memcmp(colorinfo[i].da.bg_rgb, refattr->bg_rgb, 3); i++)
	{
	}
	colorinfo[font].bg = colorinfo[i].bg;

	/* return the font code */
	return font | select;
}

/* List the current color definitions out to a given window */
void colorlist(win, name, implicit)
	WINDOW	win;	/* which window to write to */
	CHAR	*name;	/* name of color, or NULL for all */
	ELVBOOL	implicit;/* list all colors (else just explicitly set ones) */
{
	int	sorted, i;
	ELVBOOL	found;

	for (sorted = 0, found = ElvFalse; sorted < colornpermanent - 1; sorted++)
	{
		/* skip if not the named color */
		i = colorsortorder[sorted];
		if (name ? CHARcmp(name, colorinfo[i].name)
			 : (!implicit && (~colorinfo[i].da.bits & COLOR_SET)))
			continue;

		/* output the description */
		if (colorinfo[i].descr)
			msg(MSG_INFO, "[SS]color $1 $2",
				colorinfo[i].name, colorinfo[i].descr);
		else
			msg(MSG_INFO, "[S]color $1 has not been set",
				colorinfo[i].name);
		found = ElvTrue;
	}
	if (name && !found)
		msg(MSG_INFO, "[S]nothing is using colors for $1", name);
}


#ifdef FEATURE_MKEXRC
/* Append a series of ex commands to "buf", to recreate the current color
 * scheme.  Ignore any colors which don't have the COLOR_SET bit set.
 */
void colorsave(buf)
	BUFFER	buf;
{
	int	i;
	CHAR	cmd[200];
	MARKBUF	end;
	guidot_t *scan;

	/* for each color role... */
	for (i = 1; i < colornpermanent; i++)
	{
		/* skip if only set to a default value */
		if ((colorinfo[i].da.bits & COLOR_SET) == 0
		 || colorinfo[i].descr == NULL)
			continue;

		/* construct a command line */
		CHARcpy(cmd, toCHAR("color "));
		CHARcat(cmd, o_gui);
		CHARcat(cmd, toCHAR("."));
		CHARcat(cmd, colorinfo[i].name);
		CHARcat(cmd, toCHAR(" "));
		CHARcat(cmd, colorinfo[i].descr);
		CHARcat(cmd, toCHAR("\n"));

		/* append it to the buffer */
		(void)marktmp(end, buf, o_bufchars(buf));
		bufreplace(&end, &end, cmd, CHARlen(cmd));
	}

	/* Append the colors for other GUIs too */
	for (scan = guidot; scan; scan = scan->next)
	{
		CHARcpy(cmd, toCHAR("color "));
		CHARcat(cmd, scan->name);
		CHARcat(cmd, toCHAR(" "));
		CHARcat(cmd, scan->descr);
		CHARcat(cmd, toCHAR("\n"));

		/* append it to the buffer */
		(void)marktmp(end, buf, o_bufchars(buf));
		bufreplace(&end, &end, cmd, CHARlen(cmd));
	}
}
#endif /* FEATURE_MKEXRC */

#ifdef FEATURE_COMPLETE
/* Attempt to complete an argument to :color.  This may be role name, or one
 * of the special words.  The names of actual colors are not expanded.  As a
 * last resort, it may return the current args.
 */
CHAR *colorcomplete(win, from, to, nameonly)
	WINDOW	win;	/* where to list the possible matches */
	MARK	from;	/* start of args */
	MARK	to;	/* end of args, where completion takes place */
	ELVBOOL	nameonly;/* only complete font names, never any other arg */
{
	CHAR	*cp, *word, *role;
	long	len, width;
	ELVBOOL	wantrole;
	int	i, sorted;
	int	matches, firstmatch, common;
 static CHAR	retbuf[100];
 static CHAR	*magic[] = {
		toCHAR("bold "),
		toCHAR("boxed "),
		toCHAR("proportional "),
		toCHAR("fixed "),
		toCHAR("graphic "),
		toCHAR("italic "),
		toCHAR("like "),
		toCHAR("on "),
		toCHAR("underlined ")};
	

	/* just to keep the compiler happy */
	firstmatch = common = 0;

	/* scan the words leading up to current word */
	word = NULL;
	wantrole = ElvTrue;
	role = NULL;
	len = markoffset(to) - markoffset(from);
	for (scanalloc(&cp, from); cp && len > 0; scannext(&cp), len--)
	{
		/* if in word, accumulate chars */
		if (nameonly ? elvalnum(*cp) : !elvspace(*cp))
		{
			buildCHAR(&word, *cp);
			continue;
		}

		/* ignore redundant spaces */
		if (!word)
			continue;

		/* end of a complete word -- if it was "like" then we expect
		 * a role. */
		wantrole = (ELVBOOL)!CHARcmp(word, toCHAR("like"));
		if (!role)
			role = word;
		else
			safefree(word);
		word = NULL;
	}
	scanfree(&cp);

	/* At this point, we've reached the point where completion is to take
	 * place.  If we're in the middle of a partial word, then "word" points
	 * to that partial word.  The "wantrole" variable indicates whether
	 * we'll be completing a role name or something else.
	 */

	/* Are we completing a role name? */
	if (wantrole || nameonly)
	{
		/* we don't care any complete role name */
		if (role)
			safefree(role);

		/* if no chars, then list all roles */
		if (!word)
		{
			width = o_columns(win);
			for (sorted = 0; sorted < colornpermanent - 1; sorted++)
			{
				i = colorsortorder[sorted];
				if (CHARlen(colorinfo[i].name) + 2 > (unsigned)width)
				{
					drawextext(win, toCHAR("\n"), 1);
					width = o_columns(win);
				}
				else if (width < o_columns(win))
				{
					drawextext(win, toCHAR(" "), 1);
					width--;
				}
				drawextext(win, colorinfo[i].name, CHARlen(colorinfo[i].name));
				width -= CHARlen(colorinfo[i].name);
			}
			drawextext(win, toCHAR("\n"), 1);
			return toCHAR("");
		}

		/* see how many matches there are, and how many more characters
		 * they have in common.
		 */
		matches = 0;
		len = CHARlen(word);
		for (i = 1; i < colornpermanent; i++)
		{
			/* skip non-matching role names */
			if (CHARncmp(word, colorinfo[i].name, len))
				continue;

			/* first match? */
			if (matches++ == 0)
			{
				/* yes, remember it */
				firstmatch = i;
				common = CHARlen(colorinfo[i].name);
			}
			else
			{
				/* no, check common chars */
				while (CHARncmp(colorinfo[firstmatch].name,
					 	colorinfo[i].name, common))
					common--;
			}
		}

		/* don't need "word" anymore */
		safefree(word);

		/* If no matches, then beep and return nothing. */
		if (matches == 0)
		{
			guibeep(win);
			return toCHAR("");
		}

		/* If unique match, then return its tail plus a space */
		if (matches == 1)
		{
			CHARcpy(retbuf, colorinfo[firstmatch].name + len);
			CHARcat(retbuf, toCHAR(" "));
			return retbuf;
		}

		/* It isn't unique, but maybe we can complete some chars */
		if (common > len)
		{
			CHARcpy(retbuf, colorinfo[firstmatch].name + len);
			retbuf[common - len] = '\0';
			return retbuf;
		}

		/* List the partially-matching role names */
		width = o_columns(win);
		for (sorted = 0; sorted < colornpermanent - 1; sorted++)
		{
			/* skip non-matching names */
			i = colorsortorder[sorted];
			if (CHARncmp(colorinfo[i].name, colorinfo[firstmatch].name, len))
				continue;
			if (CHARlen(colorinfo[i].name) + 2 > (unsigned)width)
			{
				drawextext(win, toCHAR("\n"), 1);
				width = o_columns(win);
			}
			else if (width < o_columns(win))
			{
				drawextext(win, toCHAR(" "), 1);
				width--;
			}
			drawextext(win, colorinfo[i].name, CHARlen(colorinfo[i].name));
			width -= CHARlen(colorinfo[i].name);
		}
		drawextext(win, toCHAR("\n"), 1);
		return toCHAR("");
	}

	/* if we get here, then it wasn't a role name, so we want to complete
	 * other stuff.
	 */

	/* if no partial word, then we return this role's settings.  Note that
	 * we are guaranteed to know the role name, since otherwise we would
	 * have performed role name completion, above.
	 */
	if (!word)
	{
		/* locate this role */
		for (i = 1; i < colornpermanent; i++)
			if (!CHARcmp(role, colorinfo[i].name))
				break;
		safefree(role);
		
		/* if not found, or has no settings, then just beep */
		if (i >= colornpermanent || !colorinfo[i].descr)
		{
			guibeep(win);
			return toCHAR("");
		}

		/* else return the settings */
		return colorinfo[i].descr;
	}

	/* don't care about role drom here on out */
	safefree(role);

	/* if we get here, then we have a partial word.  Since we can't complete
	 * color names, we can only hope to complete magic words.  All of them
	 * start with different letters except "bold" and "boxed", so this is
	 * easy.
	 */
	if (!CHARcmp(word, toCHAR("b")))
		return toCHAR("o");
	else if (!CHARcmp(word, toCHAR("bo")))
	{
		drawextext(win, toCHAR("bold boxed\n"), 11);
		return toCHAR("");
	}
	len = CHARlen(word);
	for (i = 0; i < QTY(magic); i++)
		if (!CHARncmp(word, magic[i], len))
			return magic[i] + len;

	/* it didn't match anything. */
	guibeep(win);
	return toCHAR("");
}
#endif /* FEATURE_COMPLETE */

static struct
{
	char	*name;	/* name of the color */
	long	ansi;	/* ANSI.SYS color; if >10, then set "bold" attribute */
	unsigned char rgb[3];	/* approximate RGB versions of colors */
} ansicolors[] =
{
	{"black",	0,	{  0,   0,   0}},
	{"red",		1,	{170,   0,   0}},
	{"green",	2,	{  0, 170,   0}},
	{"brown",	3,	{170, 170,   0}},
	{"blue",	4,	{  0,   0, 170}},
	{"magenta",	5,	{170,   0, 170}},
	{"cyan",	6,	{  0, 170, 170}},
	{"white",	7,	{170, 170, 170}},
	{"gray",	10,	{ 85,  85,  85}},
	{"grey",	10,	{ 85,  85,  85}},
	{"yellow",	13,	{255, 255,  85}},
	/* default & transparent must be the last two items in the list! */
	{"default",	9,	{  0,   0,   0}},
	{"transparent",	20,	{170, 170, 170}}
};

/* Convert an RGB value into an ANSI color code */
int colorrgb2ansi(isfg, rgb)
	ELVBOOL		isfg;	/* is this foreground? (else background) */
	unsigned char	*rgb;	/* the RGB values to convert to an ANSI color */
{
	int	idx, nearidx;
	int	idxdist, neardist;
	ELVBOOL	nearbright;
	int	r, g, b;

	/* reset the near variables */
	neardist = 3 * 255 * 255 + 1; /* impossibly far away */

	/* for each non-bright color in the color table... */
	nearbright = ElvTrue;
	for (nearidx = idx = 0; ansicolors[idx].ansi < 10; idx++)
	{
		/* compute the non-bright distance */
		r = (int)rgb[0] - (int)ansicolors[idx].rgb[0];
		g = (int)rgb[1] - (int)ansicolors[idx].rgb[1];
		b = (int)rgb[2] - (int)ansicolors[idx].rgb[2];
		idxdist = r * r + g * g + b * b;

		/* if closer than any previous color, remember it */
		if (idxdist < neardist)
		{
			neardist = idxdist;
			nearidx = idx;
			nearbright = ElvFalse;
		}

		/* for foreground, also allow "bright" versions */
		if (isfg)
		{
			/* compute the bright distance */
			r = (int)rgb[0] - (int)ansicolors[idx].rgb[0] - 85;
			g = (int)rgb[1] - (int)ansicolors[idx].rgb[1] - 85;
			b = (int)rgb[2] - (int)ansicolors[idx].rgb[2] - 85;
			idxdist = r * r + g * g + b * b;

			/* if closer than any previous color, remember it */
			if (idxdist < neardist)
			{
				neardist = idxdist;
				nearidx = idx;
				nearbright = ElvTrue;
			}
		}
	}

	/* Return the ansi color.  To denote brightness, add 10 to the value */
	if (nearbright)
		nearidx += 10;
	return nearidx;
}

/* Convert a color name to an ANSI color value.  This function has the same
 * signature as the gui->color() function, and is intended to be used by the
 * termcap, open, script, and quit user interfaces.
 */
ELVBOOL coloransi(fontcode, name, isfg, colorptr, rgb)
	int	fontcode;	/* index into colorinfo[] (ignored) */
	CHAR	*name;		/* name of the color */
	ELVBOOL	isfg;		/* will this be a foreground color? */
	long	*colorptr;	/* where to store the color code */
	unsigned char	*rgb;	/* where to store the RGB value */
{
	ELVBOOL	bright;	/* set the brightness bit? */
	int	i;
	unsigned int	r, g, b;

	/* tweak the "transparent" color */
	if (isfg)
	{
		i = (o_background == 'l') ? 0 : 255;
	}
	else
	{
		i = (o_background == 'l') ? 170 : 85;
		memset(ansicolors[QTY(ansicolors) - 1].rgb, i, 3);
	}
	memset(ansicolors[QTY(ansicolors) - 2].rgb, i, 3);

	/* If first character is '#' then try to convert hex to RGB, and then
	 * convert that to an ANSI color code.
	 */
	if (*name == '#')
	{
		/* Convert string to RGB values */
		if (sscanf(tochar8(name), "#%4x%4x%4x", &r, &g, &b) == 3)
			r >>= 8, g >>= 8, b >>= 8;
		if (sscanf(tochar8(name), "#%2x%2x%2x", &r, &g, &b) == 3)
			/* do nothing */;
		else if (sscanf(tochar8(name), "#%1x%1x%1x", &r, &g, &b) == 3)
			r *= 0x11, g *= 0x11, b *= 0x11;
		else
		{
			msg(MSG_ERROR, "[S]malformed #RRGGBB string $1", name);
			return ElvFalse;
		}
		rgb[0] = (unsigned char)r;
		rgb[1] = (unsigned char)g;
		rgb[2] = (unsigned char)b;

		/* Convert RGB values to ANSI color */
		*colorptr = colorrgb2ansi(isfg, rgb);
		return ElvTrue;
	}

	/* see if we're supposed to set the brightness bit */
	bright = ElvFalse;
	if (!CHARncmp(name, toCHAR("light"), 5)) bright = ElvTrue, name += 5;
	if (!CHARncmp(name, toCHAR("lt"), 2)) bright = ElvTrue, name += 2;
	if (!CHARncmp(name, toCHAR("bright"), 6)) bright = ElvTrue, name += 6;

	/* skip leading garbage characters */
	while (*name && !elvalpha(*name))
	{
		name++;
	}

	/* try to find the color */
	for (i = 0; i < QTY(ansicolors) && CHARcmp(toCHAR(ansicolors[i].name), name); i++)
	{
	}
	if (i >= QTY(ansicolors))
	{
		msg(MSG_ERROR, "[s]invalid color $1", name);
		return ElvFalse;
	}

	/* background can't be bright */
	if (!isfg && (bright || ansicolors[i].ansi >= 10) && ansicolors[i].ansi != 20)
	{
		msg(MSG_ERROR, "background color can't be bright");
		return ElvFalse;
	}

	/* foreground can't be transparent */
	if (isfg && ansicolors[i].ansi >= 20)
	{
		msg(MSG_ERROR, "foreground color can't be transparent");
		return ElvFalse;
	}

	/* Copy the results out to caller's vars */
	if (bright && ansicolors[i].ansi < 10)
	{
		/* Copy bright version of color */
		*colorptr = ansicolors[i].ansi + 10;
		rgb[0] = ansicolors[i].rgb[0] + 85;
		rgb[1] = ansicolors[i].rgb[1] + 85;
		rgb[2] = ansicolors[i].rgb[2] + 85;
	}
	else
	{
		/* Copy normal version of color */
		*colorptr = ansicolors[i].ansi;
		memcpy(rgb, ansicolors[i].rgb, sizeof ansicolors[i].rgb);
	}

	/* Success! */
	return ElvTrue;
}

/* xx herbert */
/* return the name for an ansi code */
char *colorname (ansi)
  long ansi;
{
  int i = 0;
  for (; i < QTY(ansicolors) && ansicolors[i].ansi != ansi; i++)
    {
    }
  if (i >= QTY(ansicolors) && ansi > 10)
    {
      return colorname (ansi - 10);
    }
  return (i < QTY(ansicolors))? ansicolors[i].name : "invalid color";
}
/* xx herbert */
