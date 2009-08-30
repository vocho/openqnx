/* region.c */

/* Copyright 2001 by Steve Kirkendall */

#include "elvis.h"

#ifdef FEATURE_REGION

# if USE_PROTOTYPES
  static region_t *rgnfind(MARK from, MARK to, _char_ font, region_t *after,
  			region_t **lag);
  static void rgnfree(region_t *region);
  static void rgnclean(BUFFER buf);
# endif

#ifndef DEBUG_REGION
# define RGNCHECK
# define RGNCHECKFULL
#else
# define RGNCHECK rgncheck(__FILE__, __LINE__, ElvFalse)
# define RGNCHECKFULL rgncheck(__FILE__, __LINE__, ElvTrue)
# if USE_PROTOTYPES
  static void rgncheck(char *file, int line, ELVBOOL full);
# endif

/* this stores the most recent places that called RGNCHECK */
static struct {
	char *file;
	int line;
} history[50];

/* this checks the regions for any weirdnes */
static void rgncheck(file, line, full)
	char	*file;
	int	line;
	ELVBOOL	full;	/* TRUE for extra checks */
{
	BUFFER	buf;
	region_t *rgn;

	/* shift the history */
	memmove(history + 1, history, sizeof history - sizeof history[0]);
	history[0].file = file;
	history[0].line = line;

	/* for each buffer... */
	for (buf = buflist((BUFFER)0); buf; buf = buflist(buf))
	{
		/* for each region in that buffer... */
		for (rgn = buf->regions; rgn; rgn = rgn->next)
		{
			/* check it */
			if ((int)rgn->from < 0x100000)			abort();
			if (rgn->from->buffer != buf)			abort();
			if (rgn->from->offset < 0)			abort();
			if (rgn->from->offset > o_bufchars(buf))	abort();
			if ((int)rgn->to < 0x100000)			abort();
			if (rgn->to->buffer != buf)			abort();
			if (rgn->to->offset < 0)			abort();
			if (rgn->to->offset > o_bufchars(buf))		abort();
			if (rgn->comment && *rgn->comment <= ' ')	abort();
			if (rgn->font && rgn->font > colornpermanent)	abort();

			/* check for overlaps & abutments */
			if (full && rgn->next)
			{
				if (markoffset(rgn->to) > markoffset(rgn->next->from))
									abort();
				if (markoffset(rgn->to) == markoffset(rgn->next->from)
				 && rgn->font == rgn->next->font
				 && !CHARcmp(rgn->comment, rgn->next->comment))
									abort();
			}
		}
	}
}
#endif

/* find an existing region between "from" and "to", of the given type */
static region_t *rgnfind(from, to, font, after, lag)
	MARK from;	/* start of the region to search */
	MARK to;	/* end of the region to search */
	_char_ font;	/* font to search for, or '\0' for any */
	region_t *after;/* NULL to start new search, else previous return val */
	region_t **lag;	/* reference to a pointer to region before found one */
{
	region_t *r;

	/* for each region... */
	if (lag) *lag = after;
	for (r = after ? after->next : markbuffer(from)->regions;
	     r && markoffset(r->to) <= markoffset(from);
	     r = r->next)
	{
		if (lag) *lag = r;
	}

	/* did we find a match? */
	if (r
	 && markoffset(r->from) < markoffset(to)
	 && (!font || r->font == font))
		/* Yes!  return it */
			return r;

	/* no matching region was found */
	return NULL;
}

/* free a region's resources */
static void rgnfree(region)
	region_t *region; /* the region to free */
{
	markfree(region->from);
	markfree(region->to);
	safefree(region->comment);
	safefree(region);
}


/* clean up various forms of ugliness in the region list */
static void rgnclean(buf)
	BUFFER	buf;	/* buffer whose list should be cleaned */
{
	region_t *r;	/* the new region */
	region_t *lag;	/* the region before it in the list */
	region_t *next;	/* the next region */

	RGNCHECK;

	/* for each region... */
	for (lag = NULL, r = buf->regions; r; lag = r, r = next)
	{
		next = r->next;

		/* if zero-length region, then delete it */
		if (markoffset(r->from) >= markoffset(r->to))
		{
			if (lag)
				lag->next = next;
			else
				buf->regions = next;
			rgnfree(r);

			/* loop, without advancing "lag" */
			r = lag;
			continue;
		}

		/* if next region is same as this one, merge them */
		if (next
		 && markoffset(r->to) == markoffset(next->from)
		 && r->font == next->font
		 && !CHARcmp(r->comment, next->comment))
		{
			marksetoffset(r->to, markoffset(next->to));
			r->next = next->next;
			rgnfree(next);

			/* loop, without advancing "lag" or "r" */
			next = r;
			r = lag;
			continue;
		}
	}

	RGNCHECKFULL;
}


/* Handle undo.  The tricky thing here is that a region may have been added
 * by operation that's about to be undone, and if so then we *MUST* delete
 * that region.  Trying to keep it would confuse other region code.
 */
void regionundo(buf, keep)
	BUFFER		buf;	/* buffer containing the regions */
	struct umark_s	*keep;	/* describes marks of regions to be kept */
{
	region_t *lag, *scan, *next;
	int	i;

	RGNCHECKFULL;

#ifdef REGION_UNDO
	/* if keep==NULL then we just wanted to inspect regions for bugs,
	 * and we've done that so return now.
	 */
	if (!keep)
		return;
#endif

	/* for each region... */
	for (lag = NULL, scan = buf->regions; scan; scan = next)
	{
		next = scan->next;

		/* is this region's marks listed in the undo version? */
		for (i = 0; keep[i].mark; i++)
			if (scan->from == keep[i].mark || scan->to == keep[i].mark)
				break;
		if (!keep[i].mark)
		{
			/* no -- delete this region */
			if (lag)
				lag->next = next;
			else
				buf->regions = next;
			rgnfree(scan);
		}
		else
		{
			/* yes -- keep it.  Advance to the next */
			lag = scan;
		}
	}
}

/* delete regions in a given range, optionally matching a given font */
void regiondel(from, to, font)
	MARK from;	/* start of region to delete */
	MARK to;	/* end of region to delete */
	_char_ font;	/* font to use, or '\0' for all fonts */
{
	region_t *doomed, *lag;
	MARKBUF	above, below;
	char	abovefont, belowfont;
	CHAR	*abovecomment, *belowcomment;

	RGNCHECK;

	/* for each doomed region... */
	abovecomment = belowcomment = NULL;
	lag = NULL;
	while ((doomed = rgnfind(from, to, font, lag, &lag)) != NULL)
	{
		/* remove it from the list */
		if (lag)
			lag->next = doomed->next;
		else
			markbuffer(from)->regions = doomed->next;

		/* if part of it extends outside the given region, remember it
		 * so we can add that portion back later.
		 */
		if (markoffset(doomed->from) < markoffset(from))
		{
			above = *doomed->from;
			abovefont = doomed->font;
			assert(abovecomment == NULL);
			abovecomment = CHARdup(doomed->comment);
		}
		if (markoffset(doomed->to) > markoffset(to))
		{
			below = *doomed->to;
			belowfont = doomed->font;
			assert(belowcomment == NULL);
			belowcomment = CHARdup(doomed->comment);
		}

		/* free this one's resources */
		rgnfree(doomed);
	}

	/* add back parts of regions above and below the change */
	if (abovecomment)
	{
		regionadd(&above, from, abovefont, abovecomment);
		safefree(abovecomment);
	}
	if (belowcomment)
	{
		regionadd(to, &below, belowfont, belowcomment);
		safefree(belowcomment);
	}

	RGNCHECK;
}

/* add a region, or extend an overlapping identical region */
void regionadd(from, to, font, comment)
	MARK from;	/* start of region to add */
	MARK to;	/* end of region to add */
	_char_ font;	/* font to use */
	CHAR *comment;	/* comment to use */
{
	region_t *r;	/* the new region */
	region_t *lag;	/* the region before it in the list */

	/* Do nothing if it would be zero-length */
	if (markoffset(from) >= markoffset(to))
		return;

	/* Delete any existing regions here.  NOTE: Although regionadd() and
	 * regiondel() are mutually recursive, there is no danger of infinite
	 * recursion since regiondel() only calls regionadd() for areas which
	 * are known to be free of existing regions.
	 */
	regiondel(from, to, '\0');

	/* create a new region */
	r = (region_t *)safealloc(1, sizeof(region_t));
	r->from = markdup(from);
	r->to = markdup(to);
	r->font = font;
	r->comment = CHARdup(comment);

	/* decide where to insert it */
	(void)rgnfind(from, to, '\0', NULL, &lag);

	/* insert it into the region list */
	if (lag)
	{
		r->next = lag->next;
		lag->next = r;
	}
	else
	{
		r->next = markbuffer(from)->regions;
		markbuffer(from)->regions = r;
	}

	/* clean up the list by deleting zero-length regions, and merging
	 * adjacent, identical regions.
	 */
	rgnclean(markbuffer(from));
}

/* return the region containing a given MARK, or NULL if there is none */
region_t *regionfind(mark)
	MARK	mark;	/* position, possibly within a region */
{
	MARKBUF	tmp;
	(void)marktmp(tmp, markbuffer(mark), markoffset(mark) + 1);
	return rgnfind(mark, &tmp, '\0', NULL, NULL);
}

/* This implements the :region and :unregion commands. */
RESULT ex_region(xinf)
	EXINFO	*xinf;	/* details about the command */
{
	char	font, newfont;
	CHAR	*comment;
	region_t *r;
	CHAR	buf[100], *build;
	ELVBOOL	any;

	assert(xinf->command == EX_REGION || xinf->command == EX_UNREGION ||
	       xinf->command == EX_CHREGION);
	RGNCHECKFULL;

	/* convert the font name to a font code */
	if (xinf->lhs)
		font = colorfind(xinf->lhs);
	else if (xinf->command == EX_UNREGION)
		font = '\0';
	else if (xinf->command == EX_CHREGION)
	{
		msg(MSG_ERROR, "missing lhs");
		return RESULT_ERROR;
	}
	else
	{
		/* :region with no args -- list the regions */

		/* now list it */
		any = ElvFalse;
		r = NULL;
		while ((r = rgnfind(xinf->fromaddr, xinf->toaddr, '\0', r, NULL)) != NULL)
		{
			/* list this region */
			any = ElvTrue;
			long2CHAR(buf, markline(r->from));
			build = buf + CHARlen(buf);
			*build++ = ',';
			long2CHAR(build, markline(r->to) - 1);
			CHARcat(build, toCHAR(" region "));
			CHARcat(build, colorinfo[(int)r->font].name);
			build += CHARlen(build);
			if (CHARcmp(r->comment, colorinfo[(int)r->font].name))
			{
				*build++ = ' ';
				CHARcpy(build,  r->comment);
				build += CHARlen(build);
			}
			*build++ = '\n';
			drawextext(xinf->window, buf, (int)(build - buf));
		}
		if (!any)
			msg(MSG_INFO, "no regions");
		return RESULT_COMPLETE;
	}

	/* default comment is same as face name */
	comment = xinf->rhs ? xinf->rhs : xinf->lhs;

	/* add or delete regions */
	if (xinf->command == EX_REGION)
		regionadd(xinf->fromaddr, xinf->toaddr, font, comment);
	else if (xinf->command == EX_UNREGION)
		regiondel(xinf->fromaddr, xinf->toaddr, font);
	else /* EX_CHREGION */
	{
		/* parse the newfont and comment */
		if (!xinf->rhs)
		{
			msg(MSG_ERROR, "missing rhs");
			return RESULT_ERROR;
		}
		comment = CHARchr(xinf->rhs, ' ');
		if (comment)
		{
			*comment++ = '\0';
			while (*comment == ' ');
				comment++;
			if (*comment == '\0')
				comment = xinf->rhs;
		}
		else
			comment = xinf->rhs;
		newfont = colorfind(xinf->rhs);

		/* for each matching region... */
		r = NULL;
		while ((r = rgnfind(xinf->fromaddr, xinf->toaddr, font, r, NULL)) != NULL)
		{
			/* change the font & comment */
			r->font = newfont;
			safefree(r->comment);
			r->comment = CHARdup(comment);
		}

		/* clean up abutting regions, etc. */
		rgnclean(markbuffer(xinf->fromaddr));
	}

	RGNCHECKFULL;

	return RESULT_COMPLETE;
}

#endif /* FEATURE_REGION */
