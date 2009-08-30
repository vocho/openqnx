/* cut.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_cut[] = "$Id: cut.c 167211 2008-04-24 17:26:56Z sboucher $";
#endif

static void shiftbufs P_((void));
static void dosideeffect P_((MARK from, MARK to, _CHAR_ sideeffect, _CHAR_ cbname));



/* This is the name of the most recently named buffer.  It is used to
 * implement the "" and "@ buffer names, and for incrementing "1 (etc.)
 * when pasting from numbered cut buffers.
 */
static CHAR previous;

/* This function locates or creates the BUFFER used for storing the contents
 * of a given cut buffer.  "cbname" is the single-character name of the cut
 * buffer.
 *
 * The cutyank() and cutput() functions both use this function to locate the
 * buffer, and then perform other name-dependent operations to determine how
 * the buffer should be used.  For example, 'a' and 'A' both refer to the same
 * buffer here, but cutyank() will treat them differently.
 */
BUFFER cutbuffer(cbname, create)
	_CHAR_	cbname;	/* name of cut buffer, or '\0' for anonymous */
	ELVBOOL	create;	/* create the edit buffer if it doesn't already exist? */
{
	char	tmpname[50];
	char	*bufname;
	BUFFER	buf;

	/* handle the "" buffer */
	if (cbname == '"' || cbname == '@')
	{
		if (!previous)
		{
			msg(MSG_ERROR, "no previous cut buffer");
			return NULL;
		}
		cbname = previous;
	}

	switch (cbname)
	{
	  case '\0':
		bufname = CUTANON_BUF;
		break;

	  case '<':
	  case '>':
	  case '^':
		bufname = CUTEXTERN_BUF;
		break;

	  case '.':
		bufname = CUTINPUT_BUF;
		break;

	  default:
		if ((cbname >= '1' && cbname <= '9') || elvlower(cbname) || cbname == '_')
		{
			sprintf(tmpname, CUTNAMED_BUF, cbname);
			bufname = tmpname;
		}
		else if (elvupper(cbname))
		{
			sprintf(tmpname, CUTNAMED_BUF, elvtolower((char)cbname));
			bufname = tmpname;
		}
		else
		{
			msg(MSG_ERROR, "[C]bad cutbuf $1", cbname);
			return NULL;
		}
	}

	/* find the buffer, or create it */
	previous = cbname;
	buf = (create ? bufalloc(toCHAR(bufname), 0, ElvTrue) : buffind(toCHAR(bufname)));
	if (buf)
	{
		o_internal(buf) = ElvTrue;	/* probably already set */
		o_locked(buf) = ElvFalse;
		o_bufid(buf) = 0;
	}
	return buf;
}


/* This function shifts the numbered cut buffers by renaming them. */
static void shiftbufs()
{
	CHAR	cbname;	/* buffer currently being considered. */
	BUFFER	buf;	/* the edit buffer used to store a cut buffer's contents */
	char	tmpname[50];

	/* We would like to delete "9 after this, but if it has any marks
	 * referring to it then we must leave it, and delete "8 instead.
	 * But "8 may have marks, forcing us to leave it too... search back
	 * until we find a buffer we can delete.
	 */
	for (cbname = '9'; cbname > '1'; cbname--)
	{
		/* Try to find the buffer.  If it doesn't exist then we
		 * won't really need to delete ANY numbered cut buffer!
		 */
		buf = cutbuffer(cbname, ElvFalse);
		if (!buf)
			break;

		/* If any marks refer to this buffer, then we can't
		 * delete this buffer.
		 */
		if (buf->marks)
			continue;

		/* Okay, this is the one!  Delete it and break out of loop */
		buffree(buf);
		break;
	}

	/* shift the lower-numbered buffers by renaming them */
	while (cbname > '1')
	{
		/* generate the name new name that the buffer should have */
		sprintf(tmpname, CUTNAMED_BUF, cbname);

		/* find the preceding-numbered buffer */
		cbname--;
		buf = cutbuffer(cbname, ElvFalse);

		/* if the buffer exists, rename it one number higher */
		if (buf)
		{
			buftitle(buf, toCHAR(tmpname));
		}
	}

	/* At this point, the buffers have been shifted and there probably
	 * is no "1 buffer.  The only way there could be a "1 buffer would be
	 * if every cut buffer from "1 to "9 was referred to by a mark and
	 * therefore undeleteable.  Even this case should be safe, though,
	 * since the cutyank() function will just replace the old contents
	 * of "1 with the new contents, causing the marks to be adjusted...
	 * to safe (though probably useless) offsets.
	 */
}

static void dosideeffect(from, to, sideeffect, cbname)
	MARK	from;		/* start of affected text */
	MARK	to;		/* end of affected text */
	_CHAR_	sideeffect;	/* what to do to it */
	_CHAR_	cbname;		/* replacement char, if sideeffect='g=' */
{
#ifdef FEATURE_G
	CHAR	*cp, *mem;
	long	len, i;
#endif

	switch (sideeffect)
	{
	  case 'd':
		bufreplace(from, to, NULL, 0L);
		break;

	  case 'y':
	  	/* do nothing */
	  	break;

#ifdef FEATURE_G
	  case ELVG('u'):
		/* convert to lowercase */
		mem = bufmemory(from, to);
		len = markoffset(to) - markoffset(from);
		for (cp = mem, i = len; --i >= 0; cp++)
			*cp = elvtolower(*cp);
		bufreplace(from, to, mem, len);
		safefree(mem);
		break;

	  case ELVG('U'):
		/* convert to uppercase */
		mem = bufmemory(from, to);
		len = markoffset(to) - markoffset(from);
		for (cp = mem, i = len; --i >= 0; cp++)
			*cp = elvtoupper(*cp);
		bufreplace(from, to, mem, len);
		safefree(mem);
		break;

	  case ELVG('~'):
		/* toggle between uppercase & lowercase */
		mem = bufmemory(from, to);
		len = markoffset(to) - markoffset(from);
		for (cp = mem, i = len; --i >= 0; cp++)
			if (elvlower(*cp))
				*cp = elvtoupper(*cp);
			else
				*cp = elvtolower(*cp);
		bufreplace(from, to, mem, len);
		safefree(mem);
		break;

	  case ELVG('='):
		/* replace with cbname (but leave newline alone) */
		mem = bufmemory(from, to);
		len = markoffset(to) - markoffset(from);
		for (cp = mem, i = len; --i >= 0; cp++)
			if (*cp != '\n')
				*cp = cbname;
		bufreplace(from, to, mem, len);
		safefree(mem);
		break;
#endif /* FEATURE_G */
	}
}

/* This function copies text between two marks into a cut buffer.  "cbname"
 * is the single-character name of the cut buffer.  "from" and "to" delimit
 * the source of the text.  "type" is 'c' for character cuts, 'l' for line
 * cuts, and 'r' for rectangular cuts; for rectangular cuts only, the left
 * and right limits are taken from the current window.
 *
 * "type" can also be 'L' for line-mode cuts which come from visual command
 * mode operators.  This is different from 'l' in that 'L' boundaries have
 * already been adjusted to match line boundaries, but for 'l' the cutyank()
 * function will need to adjust the boundaries itself.
 *
 * The "sideeffect" parameter is 'y' to keep the text, or 'd' to delete it.
 * If FEATURE_G is defined then it can also be one of the gr, gu, gu, or g~
 * operators, in which case no actual yank takes place and "cbname" is ignored.
 */
void cutyank(cbname, from, to, type, sideeffect)
	_CHAR_	cbname;	/* name of cut buffer to yank into */
	MARK	from;	/* start of source */
	MARK	to;	/* end of source */
	_CHAR_	type;	/* yank style: c=character, l=line, r=rectangle */
	_CHAR_	sideeffect;/* what to do to original text */
{
	BUFFER	dest;		/* cut buffer we're writing into */
	MARKBUF	dfrom, dto;	/* region of destination buffer */
	MARKBUF	sfrom, sto;	/* region of source buffer */
	MARK	line;		/* end of current line, when type='r' */
	long	prevline;	/* used for detecting failed move of "line" */
	long	origlines;	/* number of lines in cut buffer before yank */
	long	nlines;
	CHAR	*cp;

	assert(markbuffer(from) == markbuffer(to) && markoffset(from) <= markoffset(to));
	assert(type == 'c' || type == 'l' || type == 'r' || type == 'L');

	/* this is just to silence a bogus compiler warning */
	origlines = 0L;

	/* if yanking into the anonymous cut buffer, then shift numbered */
	if (!cbname && (sideeffect == 'y' || sideeffect == 'd'))
		shiftbufs();

	/* If this is a character-mode cut, and both ends happen to be the
	 * start of lines, then treat this as a line-mode cut.  Note that
	 * we really should know what display mode is being used, but that
	 * wasn't passed as an argument so we'll have to fudge it a little.
	 */
	if (type == 'c')
	{
		if (windefault && markbuffer(from) == markbuffer(windefault->cursor))
		{
			if (markoffset((*windefault->md->move)(windefault, from, 0L, 0L, ElvTrue)) == markoffset(from)
			 && (markoffset(to) == o_bufchars(markbuffer(to))
				|| markoffset((*windefault->md->move)(windefault, to, 0L, 0L, ElvTrue)) == markoffset(to)))
			{
				type = 'L';
			}
		}
		else
		{
			if (markoffset((*dmnormal.move)(windefault, from, 0L, 0L, ElvTrue)) == markoffset(from)
			 && (markoffset(to) == o_bufchars(markbuffer(to))
				|| markoffset((*dmnormal.move)(windefault, to, 0L, 0L, ElvTrue)) == markoffset(to)))
			{
				type = 'L';
			}
		}
	}

	/* find the cut buffer */
	if (sideeffect != 'y' && sideeffect != 'd')
		dest = NULL;
	else
	{
		dest = cutbuffer(cbname, ElvTrue);
		if (!dest)
			return;

		/* when editing a cut buffer, you can't yank it into itself */
		if (markbuffer(from) == dest)
		{
			/* for the anonymous cut buffer, just ignore it */
			if (cbname)
			{
				msg(MSG_ERROR, "can't yank a buffer into itself");
			}
			return;
		}

		/* discard the old contents, unless we want to append */
		if (!elvupper(cbname))
		{
			(void)marktmp(dfrom, dest, 0);
			(void)marktmp(dto, dest, o_bufchars(dest));
			bufreplace(&dfrom, &dto, NULL, 0L);
			o_putstyle(dest) = elvtolower(type);
			origlines = 0;
		}
		else
		{
			if (o_partiallastline(dest) && type == 'c')
			{
				(void)marktmp(dfrom, dest, o_bufchars(dest) - 1);
				(void)marktmp(dto, dest, o_bufchars(dest));
				bufreplace(&dfrom, &dto, NULL, 0L);
			}
			origlines = o_buflines(dest);
		}
	}

	/* copy the text into the buffer. */
	if (dest)
		(void)marktmp(dfrom, dest, o_bufchars(dest));
	switch (type)
	{
	  case 'c':
		if (dest)
			bufpaste(&dfrom, from, to);
		dosideeffect(from, to, sideeffect, cbname);
		break;

	  case 'l':
		sfrom = *(*dmnormal.move)(windefault, from, 0, 0, ElvTrue);
		markaddoffset(to, -1);
		sto = *(*dmnormal.move)(windefault, to, 1, INFINITY, ElvTrue);
		markaddoffset(&sto, 1);
		if (dest)
			bufpaste(&dfrom, &sfrom, &sto);
		dosideeffect(from, to, sideeffect, cbname);
		break;

	  case 'L':
		if (dest)
			bufpaste(&dfrom, from, to);
		dosideeffect(from, to, sideeffect, cbname);
		break;

	  case 'r':
		/* NOTE: the only way to yank a rectangle is by visibly
		 * selecting it.  So we know that we're yanking from the
		 * current window, and can find the left & right limits
		 * there, and use the window's edit mode to determine how
		 * the text is formatted.
		 */
		assert(windefault && from && markbuffer(from) == markbuffer(windefault->cursor));

		/* we'll start at the bottom and work backward.  All text
		 * will therefore be inserted into the cut-buffer at what
		 * is currently its end.
		 */
		if (dest)
			(void)marktmp(dfrom, dest, o_bufchars(dest));

		/* The "to" mark is actually the start of the line *AFTER* the
		 * last line to be included in the cut.  This makes display
		 * updates easier, but we need to decrement the "to" mark
		 * here or else we'll be cutting one line too many.
		 */
		line = markdup(to);
		marksetoffset(line, markoffset((*windefault->md->move)(windefault, line, -1, INFINITY, ElvTrue)));

		/* for each line of the rectangle... */
		do
		{
			/* Choose the starting point on this line.  Make sure
			 * the left edge of the character is in the rectangle
			 */
			sfrom = *(*windefault->md->move)(windefault, line, 0, windefault->selleft, ElvFalse);
			if ((*windefault->md->mark2col)(windefault, &sfrom, ElvFalse) < windefault->selleft)
			{
				markaddoffset(&sfrom, 1);
			}

			/* Choose the ending point on this line.  Add 1 so that
			 * the final character is included in the yanking, but
			 * be careful never to yank a newline.
			 */
			sto = *(*windefault->md->move)(windefault, line, 0, windefault->selright, ElvFalse);
			if (scanchar(&sto) != '\n')
			{
				markaddoffset(&sto, 1);
			}

			/* append this slice of the rectangle */
			if (dest)
				bufreplace(&dfrom, &dfrom, toCHAR("\n"), 1);
			if (markoffset(&sfrom) < markoffset(&sto))
			{
				if (dest)
					bufpaste(&dfrom, &sfrom, &sto);
				dosideeffect(&sfrom, &sto, sideeffect, cbname);
			}

			/* locate the next line */
			prevline = markoffset(line);
			marksetoffset(line, markoffset((*windefault->md->move)(windefault, line, -1, INFINITY, ElvTrue)));
			if (prevline == markoffset(line))
			{
				marksetoffset(line, markoffset(from));
			}

		} while (markoffset(line) > markoffset(from));
		markfree(line);
		break;
	}

	/* if this the external cut buffer, then write it */
	if (dest && (cbname == '>' || cbname == '^') && gui->clipopen && (*gui->clipopen)(ElvTrue))
	{
		assert(dest);
		for (scanalloc(&cp, marktmp(dfrom, dest, 0L));
		     cp;
		     markaddoffset(&dfrom, scanright(&cp)), scanseek(&cp, &dfrom))
		{
			(*gui->clipwrite)(cp, scanright(&cp));
		}
		(*gui->clipclose)();
		scanfree(&cp);
	}

	/* if it doesn't end with a newline, then slap a newline onto the
	 * end so the last line can be edited.
	 */
	if (dest)
	{
		o_partiallastline(dest) = (ELVBOOL)(o_bufchars(dest) > 0L
			&& scanchar(marktmp(dto, dest, o_bufchars(dest) - 1)) != '\n');
		if (o_partiallastline(dest))
		{
			markaddoffset(&dto, 1L);
			bufreplace(&dto, &dto, toCHAR("\n"), 1);
		}
	}

	/* Report.  Except that we don't need to report how many new input
	 * lines we've copied to the ELVIS_PREVIOUS_INPUT buffer.  Also, when
	 * the mouse is used to mark text under X11, it is immediately copied
	 * to the clipboard and we don't want to report that.
	 */
	if (!dest)
		nlines = markline(to) - markline(from);
	else
		nlines = o_buflines(dest) - origlines;
	if (o_report != 0
	 && nlines >= o_report
	 && cbname != '.'
	 && ((cbname != '>' && cbname != '^') || !windefault || !windefault->seltop))
	{
		switch (sideeffect)
		{
		  case 'd':
			msg(MSG_INFO, "[d]$1 lines deleted", nlines);
			break;

		  case ELVG('='):
		  case ELVG('u'):
		  case ELVG('U'):
		  case ELVG('~'):
			msg(MSG_INFO, "[d]$1 lines changed", nlines);
			break;

		  default:
			if (elvupper(cbname))
				msg(MSG_INFO, "[d]$1 more lines yanked", nlines);
			else
				msg(MSG_INFO, "[d]$1 lines yanked", nlines);
		}
	}
}

/* This function pastes text that was yanked by cutyank.  Returns NULL on
 * errors, or the final cursor position if successful.
 */
MARK cutput(cbname, win, at, after, cretend, lretend)
	_CHAR_	cbname;	/* cut buffer name */
	WINDOW	win;	/* window showing that buffer */
	MARK	at;	/* where to insert the text */
	ELVBOOL	after;	/* if ElvTrue, insert after "at"; else insert before */
	ELVBOOL	cretend;/* if character-mode: ElvTrue=return first, ElvFalse=return last */
	ELVBOOL	lretend;/* if not character-mode: ElvTrue=return first, ElvFalse=return last */
{
	BUFFER	src;
	CHAR	iobuf[1000];
	CHAR	*cp;
	MARKBUF	sfrom, sto;
	static MARKBUF ret;
	int	i;
	long	line, col, len;
	ELVBOOL	cmd;
	long     location;

	/* If anonymous buffer, and most recent paste was from a numbered
	 * cut buffer, then use the successive numbered buffer by default.
	 */
	if (!cbname)
	{
		if (previous >= '1' && previous < '9')
			cbname = previous + 1;
		else if (previous == '9')
			cbname = '9';
	}

	/* find the cut buffer */
	src = cutbuffer(cbname, ElvTrue);
	if (!src)
	{
		return NULL;
	}

	/* when editing a cut buffer, you can't paste it into itself */
	if (markbuffer(at) == src)
	{
		/* for the anonymous cut buffer, just ignore it */
		if (!CHARcmp(o_bufname(src), toCHAR(CUTANON_BUF)))
		{
			ret = *at;
			return &ret;
		}
		msg(MSG_ERROR, "can't paste a buffer into itself");
		return NULL;
	}

	/* if external cut buffer, then fill it from GUI */
	if ((cbname == '<' || cbname == '^') && gui->clipopen && (*gui->clipopen)(ElvFalse))
	{
		bufreplace(marktmp(sfrom, src, 0), marktmp(sto, src, o_bufchars(src)), NULL, 0L);
		location = 0L;
		while ((i = (*gui->clipread)(iobuf, sizeof(iobuf))) > 0)
		{
			bufreplace(marktmp(sfrom, src, location), &sfrom, iobuf, i);
			location += i;
		}
		(*gui->clipclose)();
		o_putstyle(src) = 'c';
		o_partiallastline(src) = (ELVBOOL)(location > 0L
			&& scanchar(marktmp(sfrom, src, location - 1)) != 'n');
		if (o_partiallastline(src))
		{
			bufreplace(marktmp(sfrom, src, location), &sfrom, toCHAR("\n"), 1L);
		}
	}

	/* if the buffer is empty, fail */
	if (o_bufchars(src) == 0L)
	{
		/* well, the '.' buffer is okay, but all others fail */
		if (cbname == '.')
		{
			ret = *at;
			return &ret;
		}
		msg(MSG_ERROR, "[C]cut buffer $1 empty", cbname);
		return NULL;
	}

	/* do the paste */
	switch (o_putstyle(src))
	{
	  case 'c': /* CHARACTER MODE */
		/* choose the insertion point */
		ret = *at;
		if (after && scanchar(at) != '\n')
		{
			markaddoffset(&ret, 1);
		}

		/* paste it & set "ret" to the new cursor cursor */
		len = o_bufchars(src);
		if (o_partiallastline(src))
			len--;
		bufpaste(&ret, marktmp(sfrom, src, 0L), marktmp(sto, src, len));
		if (cretend)
		{
			markaddoffset(&ret, len - 1);
		}
		break;

	  case 'l': /* LINE MODE */
		/* choose the insertion point */
		if (after)
		{
			ret = *(win->md->move)(win, at, 0, INFINITY, ElvFalse);
			markaddoffset(&ret, 1);
		}
		else
		{
			ret = *(win->md->move)(win, at, 0, 0, ElvFalse);
		}

		/* paste it & set "ret" to the start of the new cursor line */
		bufpaste(&ret, marktmp(sfrom, src, 0L), marktmp(sto, src, o_bufchars(src)));
		if (lretend)
		{
			markaddoffset(&ret, o_bufchars(src));
			ret = *(win->md->move)(win, &ret, -1, 0, ElvTrue);
		}

		/* move new cursor past any whitespace at start of line */
		for (scanalloc(&cp, &ret);
		     cp && (*cp == '\t' || *cp == ' ');
		     scannext(&cp))
		{
		}
		if (cp)
			ret = *scanmark(&cp);
		scanfree(&cp);
		break;

	  default: /* 'r' -- RECTANGLE MODE */
		/* choose a starting point, and a column to try for */
		if (after)
		{
			cmd = ElvTrue;
			col = (*win->md->mark2col)(win, at, cmd) + 1;
		}
		else
		{
			cmd = ElvFalse;
			col = (*win->md->mark2col)(win, at, cmd);
		}
		ret = *(*win->md->move)(win, at, 0, col, cmd);
		(void)marktmp(sto, src, lowline(bufbufinfo(src), 1) - 1);

		/* for each data line in the cut buffer... */
		for (line = 1;
		     line <= o_buflines(src) && markoffset(&ret) < o_bufchars(markbuffer(&ret));
		     line++)
		{
			/* delimit the contents of the next line in this cutbuf */
			sfrom = sto;
			markaddoffset(&sfrom, 1);
			(void)marktmp(sto, src, lowline(bufbufinfo(src), line + 1) - 1);

			/* paste it */
			bufpaste(&ret, &sfrom, &sto);

			/* move to the next line in destination buffer */
			ret = *(*win->md->move)(win, &ret, 1, col, cmd);
		}
		if (!lretend)
		{
			ret = *at;
		}
		break;
	}


	/* report */
	if (o_report != 0 && o_buflines(src) >= o_report && cbname != '.')
	{
		msg(MSG_INFO, "[d]$1 lines pasted", o_buflines(src));
	}

	return &ret;
}

/* This function copies the contents of a cut buffer into RAM.  The memory
 * image contains no hint as to whether it was a line mode cut, or character
 * cut, or rectangle.  The calling function is responsible for calling
 * safefree() when the memory image is no longer needed.  Returns NULL if
 * the buffer is empty, doesn't exist, or appears to be corrupt.  The
 * "< cut buffer is illegal in this context, and will also return NULL.
 */
CHAR *cutmemory(cbname)
	_CHAR_	cbname;	/* cut buffer name */
{
	BUFFER	src;
	MARKBUF	from, to;
	long	len;

	/* Find the cut buffer.  If it looks wrong, then return NULL. */
	src = cutbuffer(cbname, ElvFalse);
	if ((cbname == '<' || cbname == '^') || !src || o_bufchars(src) == 0L)
	{
		return NULL;
	}

	/* copy the contents into the memory */
	len = o_bufchars(src);
	if (o_putstyle(src) == 'c' && o_partiallastline(src))
		len--;
	return bufmemory(marktmp(from, src, 0L), marktmp(to, src, len));
}
