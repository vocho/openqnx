/* fold.c */
/* Copyright 2000 by Steve Kirkendall */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_fold[] = "$Id: fold.c,v 2.10 2003/10/17 17:41:23 steve Exp $";
#endif
#undef DEBUG_FOLD

#ifdef FEATURE_FOLD

static void foldfree P_((FOLD fold));
static long foldcmp P_((FOLD fold1, FOLD fold2));

/* Create a new FOLD.  After creation, it still needs to be added to either
 * the "fold" or "unfold" list by calling foldadd().
 */
FOLD foldalloc(from, to, name)
	MARK	from;	/* first char of region */
	MARK	to;	/* last char of region -- INCLUSIVE! */
	CHAR	*name;	/* name of the region */
{
	FOLD	fold;	/* the new fold */

#ifdef DEBUG_FOLD
	fprintf(stderr, "foldalloc(%ld-%ld, \"%s\")\n",
		markoffset(from), markoffset(to), tochar8(name));
#endif
	/* allocate the fold, and fill it with copies of the args */
	fold = (FOLD)safealloc(1, sizeof(struct fold_s));
	fold->from = markdup(from);
	fold->to = markdup(to);
	fold->name = CHARdup(name);

	return fold;
}

/* Free a FOLD and its resources.  This should only be done after it has
 * been deleted from the "fold" or "unfold" list.
 */
static void foldfree(fold)
	FOLD	fold;	/* the fold to be freed */
{
#ifdef DEBUG_FOLD
	fprintf(stderr, "foldfree({%s})\n", tochar8(fold->name));
#endif
	/* free the FOLD's resources, and then free the FOLD itself */
	markfree(fold->from);
	markfree(fold->to);
	safefree(fold->name);
	safefree(fold);
}

/* This function compares two FOLDs.  The "fold" and "unfold" lists are always
 * maintained in this order.  It returns 0 if identical, <0 if fold1 should
 * come before fold2 in the list, or >0 if fold1 should come after fold2.
 */
static long foldcmp(fold1, fold2)
	FOLD	fold1, fold2;	/* the FOLDs to compare. */
{
	long	diff;

	diff = markoffset(fold1->from) - markoffset(fold2->from);
	if (diff == 0)
		diff = markoffset(fold2->to) - markoffset(fold1->to);
	return diff;
}

/* Insert a FOLD into a buffer's "fold" or "unfold" list.  The buffer is
 * implied by the MARKs within the FOLD.  It is assumed that the any
 * identical or overlapping folds have been deleted from both the "fold"
 * and "unfold" lists already.
 */
void foldadd(fold, infold)
	FOLD	fold;	/* the FOLD to be added to "fold" list */
	ELVBOOL	infold;	/* add to "fold" list? (else "unfold") */
{
	BUFFER	buf = markbuffer(fold->from);
	FOLD	scan, lag;

#ifdef DEBUG_FOLD
	fprintf(stderr, "foldadd({%s}, %sinfold)\n",
		tochar8(fold->name), infold ? "" : "!");
#endif
	/* insert it at the proper point within the list, to keep the list
	 * sorted by markoffset(from) in ascending order, or markoffset(to)
	 * in decending order for equal from offsets.
	 */
	for (scan = infold ? buf->fold : buf->unfold, lag = NULL;
	     scan && foldcmp(fold, scan) > 0;
	     lag = scan, scan = scan->next)
	{
	}
	fold->next = scan;
	if (lag)
		lag->next = fold;
	else if (infold)
		buf->fold = fold;
	else
		buf->unfold = fold;
#ifdef DEBUG_FOLD
	fprintf(stderr, "    added between %s and %s\n",
		lag ? tochar8(lag->name) : "NULL",
		scan ? tochar8(scan->name) : "NULL");
#endif
}

/* Fold or unfold (depending on "infold" parameter) existing FOLDs which have
 * a given name.  Returns RESULT_COMPLETE if any were moved, else RESULT_ERROR.
 */
RESULT foldbyname(buf, name, infold)
	BUFFER	buf;	/* buffer whose FOLDs are to be affected */
	CHAR	*name;	/* name to search for */
	ELVBOOL	infold;	/* unfold them? (else refold them) */
{
	RESULT	result = RESULT_ERROR;
	FOLD	next, scan, lag;

	/* scan for the name */
	for (scan = infold ? buf->fold : buf->unfold, lag = NULL; scan; )
	{
		if (!CHARcmp(scan->name, name))
		{
			/* delete from this list */
			next = scan->next;
			if (lag)
				lag->next = next;
			else if (infold)
				buf->fold = next;
			else
				buf->unfold = next;

			/* add to other list */
			if (infold)
				foldadd(scan, ElvFalse);
			else
				foldadd(scan, ElvTrue);

			/* remember that we moved an item */
			result = RESULT_COMPLETE;

			/* move on to next item */
			scan = next;
		}
		else
		{
			/* move on to next item */
			lag = scan;
			scan = scan->next;
		}
	}

	/* return the result */
	return result;
}

/* Locate FOLDs by region, and either delete them or unfold/refold them.
 * Return RESULT_COMPLETE if at least one region was deleted/moved, or
 * RESULT_COMPLETE if it had no effect.
 *
 * This search always acts on regions which overlap the from/to range,
 * or which exactly match the from/to range.  They can also optionally
 * affected wholely enclosed FOLDs, and/or FOLDs which are nested inside
 * other FOLDs which are affected by this command.
 */
RESULT foldbyrange(from, to, infold, flags)
	MARK	from;	/* start of region to be tested against */
	MARK	to;	/* end of region to be tested against, inclusive */
	ELVBOOL	infold;	/* search the "fold" list? (else the "unfold" list) */
	int	flags;	/* mixture of FOLD_{INSIDE,OUTSIDE,NESTED,TOGGLE,DESTROY} */
{
	FOLD	scan, lag, next, lagnext;
	BUFFER	buf = markbuffer(from);
	long	fromoff = markoffset(from);
	RESULT	result = RESULT_ERROR;
	enum {FOLD_IS_IDENTICAL, FOLD_IS_INSIDE, FOLD_IS_OUTSIDE, FOLD_IS_OVERLAP} relation;

#ifdef DEBUG_FOLD
	fprintf(stderr, "foldbyrange(%ld-%ld, %sinfold,%s%s%s%s%s%s)\n",
		markoffset(from), markoffset(to),
		infold ? "" : "!",
		(flags & FOLD_INSIDE) ? " Inside" : "",
		(flags & FOLD_OUTSIDE) ? " Outside" : "",
		(flags & FOLD_NESTED) ? " Nested" : "",
		(flags & FOLD_TOGGLE) ? " Toggle" : "",
		(flags & FOLD_DESTROY) ? " Destroy" : "",
		(flags & FOLD_TEST) ? " Test" : "");
#endif

	for (scan = infold ? buf->fold : buf->unfold, lag = NULL;
	     scan && fromoff <= markoffset(to);
	     lag = scan, scan = next)
	{
		/* remember scan->next, so we can go there even if scan is
		 * deleted or moved to the other list.
		 */
		next = scan->next;

		/* if totally outside the affected range, then skip it */
		if (markoffset(scan->to) < fromoff
		 || markoffset(to) < markoffset(scan->from))
		{
#ifdef DEBUG_FOLD
		fprintf(stderr, "    {%s,%ld-%ld} is disjoint from the range %ld-%ld -- skipping\n",
			scan->name,
			markoffset(scan->from), markoffset(scan->to),
			fromoff, markoffset(to));
#endif
			continue;
		}

		/* classify the relationship between the region and this FOLD */
		if (markoffset(scan->from) == fromoff
		 && markoffset(scan->to) == markoffset(to))
			relation = FOLD_IS_IDENTICAL;
		else if (markoffset(scan->from) >= fromoff
		 && markoffset(scan->to) <= markoffset(to))
			relation = FOLD_IS_INSIDE;
		else if (markoffset(scan->from) <= fromoff
		 && markoffset(scan->to) >= markoffset(to))
			relation = FOLD_IS_OUTSIDE;
		else
			relation = FOLD_IS_OVERLAP;

#ifdef DEBUG_FOLD
		fprintf(stderr, "    {%s,%ld-%ld} is %s the range %ld-%ld\n",
			scan->name,
			markoffset(scan->from), markoffset(scan->to),
			relation == FOLD_IS_IDENTICAL ? "identical to" :
			  relation == FOLD_IS_INSIDE ? "inside" :
			  relation == FOLD_IS_OUTSIDE ? "outside" :
			  "overlapping",
			fromoff, markoffset(to));
#endif

		/* Is it a relationship that we care about?  We always want
		 * FOLD_IS_IDENTICAL and FOLD_IS_OVERLAP, but we only want
		 * FOLD_IS_INSIDE if the 'inside' flag is set, and we only want
		 * FOLD_IS_OUTSIDE  if the 'outside' flag is set.
		 */
		if (((flags & FOLD_INSIDE) == 0 && relation == FOLD_IS_INSIDE)
		 || ((flags & FOLD_OUTSIDE)== 0 && relation == FOLD_IS_OUTSIDE))
		{
#ifdef DEBUG_FOLD
			fprintf(stderr, "    skipping %s because !inside or !outside\n",
				scan->name);
#endif
			continue;
		}

		/* if we don't want to process whole nested trees, and we're
		 * scanning the "unfold" list, then we want to skip this item
		 * if the following item is inside it, and is part of the
		 * region we're scanning.
		 */
		if (!infold && (flags & FOLD_NESTED) == 0)
		{
			/* oops, we may need to skip some intervening FOLDs
			 * that are disjoint from the range.
			 */
			lagnext = scan;
			while (next && markoffset(next->to) < fromoff)
			{
#ifdef DEBUG_FOLD
				fprintf(stderr, "    bypassing %s because disjoint\n",
					next->name);
#endif
				lagnext = next;
				next = next->next;
			}

			/* okay, now finish the test */
			if (next
			 && markoffset(scan->to) > markoffset(next->from)
			 && markoffset(next->from) <= markoffset(to)
			 && markoffset(next->to) >= fromoff)
			{
#ifdef DEBUG_FOLD
				fprintf(stderr, "    skipping %s because !nested and %s is better\n", 
					scan->name, next->name);
#endif
				/* need to tweak scan, in case we bypassed any
				 * FOLDs in the little while-loop, above.
				 */
				scan = lagnext;
				continue;
			}
		}

		if (flags & (FOLD_DESTROY|FOLD_TOGGLE))
		{
			/* delete the item from this list */
			if (lag)
				lag->next = scan->next;
			else if (infold)
				buf->fold = scan->next;
			else
				buf->unfold = scan->next;

			/* if we don't want to process whole nested trees, and
			 * we're scanning the "fold" list, then we only want
			 * the outermost folds.  Tweak the "fromoff" value to
			 * prevent any nested FOLDs from being processed.
			 */
			if ((flags & FOLD_NESTED) == 0 && infold)
				fromoff = markoffset(scan->to) + 1;

			/* either free the item, or move it to the other list */
			if (flags & FOLD_DESTROY)
				foldfree(scan);
			else if (infold)
				foldadd(scan, ElvFalse);
			else
				foldadd(scan, ElvTrue);

			/* remember that we found at least one FOLD */
			result = RESULT_COMPLETE;

			/* normally we set 'lag' to 'scan', but since we
			 * deleted 'scan' we want to leave 'lag' unchanged.
			 * The easiest way to do this is to set 'scan' to
			 * 'lag' now.
			 */
			scan = lag;
		}
		else /* FOLD_TEST */
		{
			/* Don't change anything, but still remember that a
			 * a fold was found.  Also, there's no reason to stay
			 * in the loop, so might as well break out of it.
			 */
			result = RESULT_COMPLETE;
			break;
		}
	}

	return result;
}

/* Check to see whether a given mark is part of a currently-folded region.
 * If so, then return the mark; else return NULL.
 */
FOLD foldmark(mark, infold)
	MARK	mark;	/* the mark to check (implies which buffer to check) */
	ELVBOOL	infold;	/* search in "fold" list? (else search "unfold") */
{
	FOLD	scan, smallest = NULL;
	BUFFER	buf = markbuffer(mark);

	/* scan the "fold" or "unfold" list of the mark's buffer */
	for (scan = infold ? buf->fold : buf->unfold; scan; scan = scan->next)
	{
		/* does this FOLD contain the MARK? */
		if (markoffset(scan->from) <= markoffset(mark)
		 && markoffset(mark) <= markoffset(scan->to))
		{
			if (infold)
				/* return the first (largest) fold */
				return scan;
			else
				smallest = scan;
		}
	}

	/* return the smallest "unfold", or NULL if there were none or we were
	 * trying to search the "fold" list.
	 */
	return smallest;
}

/* Adjust the "fold" and "unfold" lists in response to editing.
 *
 * There are two cases that we care about: Deleting text which includes the
 * endpoints of a FOLD, and copying a region which includes a FOLD.  We
 * distinguish between these cases via the "dest" parameter -- it is NULL
 * if deleting, or the destination if copying.  (*Moving* text is accomplished
 * by first copying, and then deleting the original text.)
 *
 * Note that this function should be called immediately BEFORE deleting, but
 * immediately AFTER copying.
 */
void foldedit(from, to, dest)
	MARK	from;	/* first character to be deleted */
	MARK	to;	/* first char AFTER the affected region */
	MARK	dest;	/* where the region will be copied; NULL if no copy */
{
	FOLD	scan, fold;
	MARKBUF	foldfrom, foldto;
	BUFFER	buf = markbuffer(from);
	ELVBOOL	infold;

#ifdef DEBUG_FOLD
	if (dest)
		fprintf(stderr, "foldedit(%ld-%ld, {%s,%ld})\n",
			markoffset(from), markoffset(to),
			o_bufname(markbuffer(dest)), markoffset(dest));
	else
		fprintf(stderr, "foldedit(%ld-%ld, NULL)\n",
			markoffset(from), markoffset(to));
#endif

	/* if no folds in this buffer, then we certainly have nothing to do */
	if (!buf->fold && !buf->unfold)
		return;

	/* Are we copying or deleting? */
	if (dest)
	{
		/* After copy -- Duplicate any folds which are entirely within
		 * the source range.  The duplicates are in the destination.
		 */
		infold = (ELVBOOL)buf->fold;
		for (scan = infold ? buf->fold : buf->unfold; scan; scan = fold)
		{
			/* if entirely within source region, then duplicate */
			if (markoffset(scan->from) >= markoffset(from)
			 && markoffset(scan->to) < markoffset(to))
			{
				(void)marktmp(foldfrom, markbuffer(dest),
					markoffset(dest) + markoffset(scan->from) - markoffset(from));
				(void)marktmp(foldto, markbuffer(dest),
					markoffset(dest) + markoffset(scan->to) - markoffset(from));
				fold = foldalloc(&foldfrom, &foldto, scan->name);
				if (infold)
					foldadd(fold, ElvTrue);
				else
					foldadd(fold, ElvFalse);
			}

			/* choose next FOLD, wrapping from "fold" to "unfold" */
			fold = scan->next;
			if (!fold && infold)
			{
				fold = buf->unfold;
				infold = ElvFalse;
			}
		}
	}
	else
	{
		/* Before delete -- Destroy any folds whose endpoints are in
		 * the deleted range.
		 */
		foldto = *to;
		markaddoffset(&foldto, -1);
		foldbyrange(from, &foldto, ElvTrue, FOLD_INSIDE|FOLD_NESTED|FOLD_DESTROY);
		foldbyrange(from, &foldto, ElvFalse, FOLD_INSIDE|FOLD_NESTED|FOLD_DESTROY);
	}
}
#endif /* defined(FEATURE_FOLD) */
