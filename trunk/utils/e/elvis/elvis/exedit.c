/* exedit.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_exedit[] = "$Id: exedit.c,v 2.68 2003/10/17 17:41:23 steve Exp $";
#endif


/* This command implements the :insert, :append, and :change commands */
RESULT	ex_append(xinf)
	EXINFO	*xinf;
{
	MARK	where;

	assert(xinf->command == EX_APPEND || xinf->command == EX_CHANGE
		|| xinf->command == EX_INSERT);

	/* different behavior, depending on the command... */
	if (xinf->command == EX_CHANGE)
	{
		cutyank('\0', xinf->fromaddr, xinf->toaddr, 'L', 'd');
		where = markdup(xinf->fromaddr);
	}
	else if (xinf->command == EX_APPEND)
	{
		where = markdup(xinf->toaddr);
	}
	else
	{
		where = markdup(xinf->fromaddr);
	}

	/* Was the new text given as an argument to the command? */
	if (xinf->rhs)
	{
		/* yes, insert the new text at the appropriate place */
		bufreplace(where, where, xinf->rhs, (long)CHARlen(xinf->rhs));
	}
	xinf->newcurs = where;
	return RESULT_COMPLETE;
}


/* This function implements the :delete and :yank command */
RESULT	ex_delete(xinf)
	EXINFO	*xinf;
{
	/* check the cut buffer name */
	if (xinf->cutbuf && !elvalnum(xinf->cutbuf) &&
	    xinf->cutbuf != '^' && xinf->cutbuf != '<' && xinf->cutbuf != '>')
	{
		msg(MSG_ERROR, "bad cut buffer");
		return RESULT_ERROR;
	}

	/* do the command */
	cutyank(xinf->cutbuf, xinf->fromaddr, xinf->toaddr, (CHAR)'L', (ELVBOOL)(xinf->command == EX_DELETE) ? 'd' : 'y');
	return RESULT_COMPLETE;
}


RESULT	ex_global(xinf)
	EXINFO	*xinf;
{
	CHAR	*cp;
	long	lenln, endln;
	RESULT	ret = RESULT_COMPLETE;
	MARK	cursor;
	MARK	orig;
	MARKBUF	thisln;
	long	origlines;

	assert(xinf->command == EX_GLOBAL || xinf->command == EX_VGLOBAL);

	/* Default command is p, which should NOT be required */
	if (!xinf->rhs)
		xinf->rhs = CHARdup(toCHAR("p")); 

	/* ":g!" is like ":v" */
	if (xinf->bang)
		xinf->command = EX_VGLOBAL;

	/* remember the cursor's original position.  Inside the following loop,
	 * we'll force the cursor onto each matching line; when we're done,
	 * we'll move the cursor back and return the final position so the
	 * experform() function can move the cursor there in the conventional
	 * way -- that will be important if we switch buffers.
	 */
	if (xinf->window->state->acton)
		cursor = xinf->window->state->acton->cursor;
	else
		cursor = xinf->window->cursor;
	orig = markdup(cursor);
	if (markbuffer(cursor) != markbuffer(xinf->fromaddr))
	{
		marksetbuffer(cursor, markbuffer(xinf->fromaddr));
		marksetoffset(cursor, markoffset(xinf->fromaddr));
	}

	/* remember the number of lines before any changes... */
	origlines = o_buflines(markbuffer(xinf->fromaddr));

#ifdef FEATURE_AUTOCMD
	/* we want to trigger Edit events for change, even though we only save
	 * an undo version before the first change.
	 */
	markbuffer(cursor)->eachedit = ElvTrue;
#endif

	/* for each line... */
	(void)scanalloc(&cp, xinf->fromaddr);
	ret = RESULT_COMPLETE;
	while (cp && markoffset(xinf->fromaddr) < markoffset(xinf->toaddr))
	{
		/* find the end of this line */
		for (lenln = 0; cp && *cp != '\n'; lenln++)
			(void)scannext(&cp);
		if (cp)
		{
			(void)scannext(&cp);
			lenln++;
		}

		/* move "fromaddr" to end of this line (start of next line) */
		thisln = *xinf->fromaddr;
		endln = cp ? markoffset(scanmark(&cp)) : markoffset(xinf->toaddr);
		marksetoffset(xinf->fromaddr, endln);

		/* is this a selected line? */
		if ((regexec(xinf->re, &thisln, ElvTrue) ? EX_GLOBAL : EX_VGLOBAL)
			== xinf->command)
		{

			/* move the cursor to the matching line */
			marksetoffset(cursor, endln - lenln);

			/* free the scan pointer -- can't make changes
			 * while scanning.
			 */
			scanfree(&cp);

			/* execute the command */
			ret = exstring(xinf->window, xinf->rhs, NULL);

			/* reallocate the scan pointer */
			(void)scanalloc(&cp, xinf->fromaddr);

			/* if the ex command failed, then exit */
			if (ret != RESULT_COMPLETE)
				break;
		}

		/* if user wants to abort operation, then exit */
		if (guipoll(ElvFalse))
			break;
	}
	scanfree(&cp);

#ifdef FEATURE_AUTOCMD
	/* Revert to normal behavior of 1 Edit per "undo" version */
	markbuffer(cursor)->eachedit = ElvTrue;
#endif

	/* report any change in the number of lines */
	if (o_report != 0)
	{
		origlines -= o_buflines(markbuffer(cursor));
		if (origlines >= o_report)
			msg(MSG_INFO, "[d]$1 fewer lines", origlines);
		else if (-origlines >= o_report)
			msg(MSG_INFO, "[d]$1 more lines", -origlines);
	}

	/* move the cursor back to its original position, and then return
	 * the final position so the cursor will be moved there in a graceful
	 * way.
	 */
	xinf->newcurs = markdup(cursor);
	if (markbuffer(cursor) != markbuffer(orig))
		marksetbuffer(cursor, markbuffer(orig));
	marksetoffset(cursor, markoffset(orig));
	markfree(orig);
	return ret;
}


RESULT	ex_join(xinf)
	EXINFO	*xinf;
{
	CHAR	prevchar;	/* character before newline */
	CHAR	*cp;		/* used while scanning for newlines */
	long	njoined;	/* number of lines joined */
	long	newlines;	/* number of newlines to be clobbered */
	long	nspaces;	/* number of spaces to insert */
	MARK	start, end;	/* region around a newline */
	long	offset;		/* position of last change */
	CHAR	*endlist;	/* string of sentence ending punctuation */
 static CHAR	spaces[3] = {' ', ' ', ' '};

	/* initialize "offset" just to silence a compiler warning */
	offset = 0;

	/* initialize endlist from options (if set) or literals */
	endlist = (o_sentenceend ? o_sentenceend : toCHAR(".!?"));

	/* We're going to be replacing newlines with blanks.  The number of
	 * newlines we want to replace is equal to the number lines affected
	 * minus one... except that if the user requested a "join" of a single
	 * line then we should assume we're supposed to join two lines.
	 */
	newlines = xinf->to - xinf->from;
	if (xinf->from + newlines > o_buflines(markbuffer(xinf->fromaddr)))
	{
		msg(MSG_ERROR, "nothing to join with this line");
		return RESULT_ERROR;;
	}
	njoined = newlines + 1;

	/* scan the text for newlines */
	prevchar = ' ';
	for (scanalloc(&cp, xinf->fromaddr); cp && newlines > 0; scannext(&cp))
	{
		/* if newline, then clobber it */
		if (*cp == '\n')
		{
			start = markdup(scanmark(&cp));
			offset = markoffset(start);
			if (scannext(&cp))
			{
				/* figure out how many spaces to insert */
				if (xinf->bang || *cp == ')' || elvspace(prevchar))
					nspaces = 0;
				else if (CHARchr(toCHAR(endlist), prevchar))
					nspaces = o_sentencegap;
				else
					nspaces = 1;

				/* skip any leading whitespace in next line */
				while (!xinf->bang && cp && (*cp == ' ' || *cp == '\t'))
				{
					scannext(&cp);
				}

				/* Find the end mark */
				end = (cp ? markdup(scanmark(&cp))
					  : markalloc(markbuffer(start), o_bufchars(markbuffer(start))));

				/* free the scan context during the change;
				 * can't mix scanning & updates.
				 */
				scanfree(&cp);

				/* replace the newline (and trailing whitespace)
				 * with a given number of spaces.
				 */
				bufreplace(start, end, spaces, nspaces);
				marksetoffset(end, offset + nspaces - 1);
					/* NOTE: the "- 1" is to compensate for
					 * the scannext() at the top of the loop
					 */

				/* resume scanning */
				scanalloc(&cp, end);
				markfree(end);
				if (nspaces > 0)
				{
					prevchar = ' ';
				}
			}
			markfree(start);

			/* All that to clobber one newline! */
			newlines--;
		}
		else if ((*cp != '"' && *cp != ')') || elvspace(prevchar))
		{
			/* remember the character.  When we hit a newline,
			 * the previous character will be checked to determine
			 * how many spaces to insert.  Note that we ignore
			 * quote and parenthesis characters except after a
			 * blank.
			 */
			prevchar = *cp;
		}
	}
	scanfree(&cp);

	/* Choose a new cursor position: at the start of last joint */
	xinf->newcurs = markalloc(markbuffer(xinf->fromaddr), offset);

	/* Report it */
	if (o_report != 0 && njoined >= o_report)
	{
		msg(MSG_INFO, "[d]$1 lines joined", njoined);
	}

	return RESULT_COMPLETE;
}


RESULT	ex_move(xinf)
	EXINFO	*xinf;
{
	long	oldfrom, oldto, olddest, len;

	/* detect errors */
	oldfrom = markoffset(xinf->fromaddr);
	oldto = markoffset(xinf->toaddr);
	olddest = markoffset(xinf->destaddr);
	if (markbuffer(xinf->fromaddr) == markbuffer(xinf->destaddr)
	 && oldfrom < olddest
	 && olddest < oldto)
	{
		msg(MSG_ERROR, "destination can't be inside source");
		return RESULT_ERROR;
	}

	/* copy the text.  Adjust the offsets explicitly */
	bufpaste(xinf->destaddr, xinf->fromaddr, xinf->toaddr);
	len = oldto - oldfrom;
	if (olddest <= oldfrom)
	{
		oldfrom += len;
		oldto += len;
	}

	/* leave the cursor on the last line of the destination */
	xinf->newcurs = markalloc(markbuffer(xinf->fromaddr), olddest+len-1);
	marksetoffset(xinf->newcurs, markoffset(
		(*dmnormal.move)(xinf->window, xinf->newcurs, 0L, 0L, ElvFalse)));

	/* If moving (not copying) then delete source */
	if (xinf->command == EX_MOVE)
	{
		marksetoffset(xinf->fromaddr, oldfrom);
		marksetoffset(xinf->toaddr, oldto);
		bufreplace(xinf->fromaddr, xinf->toaddr, NULL, 0);
	}

	return RESULT_COMPLETE;
}


RESULT	ex_print(xinf)
	EXINFO	*xinf;
{
	long	last;	/* offset of start of last line */
	PFLAG	pflag;	/* how to print */

	/* generate a pflag from the command name and any supplied pflag */
	switch (xinf->pflag)
	{
	  case PF_NONE:
	  case PF_PRINT:
		pflag = (xinf->command == EX_NUMBER || o_number(xinf->window))
			    ? ((xinf->command == EX_LIST || o_list(xinf->window)) ? PF_NUMLIST : PF_NUMBER)
			    : ((xinf->command == EX_LIST || o_list(xinf->window)) ? PF_LIST : PF_PRINT);
		break;

	  case PF_LIST:
		pflag = (xinf->command == EX_NUMBER || o_number(xinf->window)) ? PF_NUMLIST : PF_LIST;
		break;

	  case PF_NUMBER:
		pflag = (xinf->command == EX_LIST || o_list(xinf->window))? PF_NUMLIST : PF_NUMBER;
		break;

	  default:
		pflag = PF_NUMLIST;
		break;
	}

	/* print the lines */
	last = exprintlines(xinf->window, xinf->fromaddr, xinf->to - xinf->from + 1, pflag);

	/* disable autoprinting for this command */
	xinf->pflag = PF_NONE;

	/* leave the cursor at the start of the last line */
	xinf->newcurs = markalloc(markbuffer(xinf->fromaddr), last);
	return RESULT_COMPLETE;
}


RESULT	ex_put(xinf)
	EXINFO	*xinf;
{
	MARK	newcurs;

	newcurs = cutput(xinf->cutbuf, xinf->window, xinf->fromaddr, (ELVBOOL)(xinf->from > 0), ElvFalse, ElvFalse);
	if (newcurs)
	{
		xinf->newcurs = markdup(newcurs);
		return RESULT_COMPLETE;
	}
	return RESULT_ERROR;
}


RESULT	ex_read(xinf)
	EXINFO	*xinf;
{
	long	offset;

	if (!xinf->rhs && xinf->nfiles != 1)
	{
		msg(MSG_ERROR, "filename required");
		return RESULT_ERROR;
	}

	/* remember where we started inserting */
	offset = markoffset(xinf->toaddr);
	xinf->newcurs = markdup(xinf->toaddr);

	/* read in the text */
	if (!bufread(xinf->toaddr, xinf->rhs ? tochar8(xinf->rhs) : xinf->file[0]))
	{
		return RESULT_ERROR;
	}

	/* Choose a place to leave the cursor.  If reading due to visual <:>
	 * command, this should be the start of the first line read; else it
	 * should be the start of the last line read.
	 */
	if (xinf->window->state->flags & ELVIS_1LINE)
	{
		marksetoffset(xinf->newcurs, offset);
	}
	else
	{
		marksetoffset(xinf->newcurs, markoffset(
			(*xinf->window->md->move)(xinf->window, xinf->newcurs, -1, 0, ElvFalse)));
	}
	return RESULT_COMPLETE;
}


/* This function implements the :< and :> commands.  It is also used to do
 * the real work for the visual <<> and <>> operators.
 */
RESULT	ex_shift(xinf)
	EXINFO	*xinf;
{
	long	ws;	/* amount of whitespace currently */
	CHAR	*cp;	/* used for scanning through a line's whitespace */
	long	line;	/* used for counting through line numbers */
	MARKBUF	start;	/* start of the line */
	MARKBUF	end;	/* end of the line's whitespace */
	CHAR	str[50];/* buffer for holding whitespace */
	short	*sw;	/* shiftwidth table */
	short	*ts;	/* tabstop table */
	long	i, col;
	long	multi;

	/* for each line... */
	start = xinf->defaddr;
	sw = o_shiftwidth(markbuffer(&start));
	ts = o_tabstop(markbuffer(&start));
	for (line = xinf->from; line <= xinf->to; line++)
	{
		/* count the current whitespace */
		scanalloc(&cp, marksetline(&start, line));
		for (ws = 0; cp && (*cp == ' ' || *cp == '\t'); scannext(&cp))
		{
			if (*cp == ' ')
				ws++;
			else
				ws += opt_totab(ts, ws);
		}
		end = *scanmark(&cp);

		/* if this is an empty line, and no ! was given on the command
		 * line, then do nothing to this line.
		 */
		if (ws == 0 && *cp == '\n' && !xinf->bang)
		{
			scanfree(&cp);
			continue;
		}

		/* if this is a preprocessor line, then don't shift it
		 * unless "!" was given.
		 */
		if (!xinf->bang
		 && xinf->window
		 && *cp == dmspreprocessor(xinf->window))
		{
			scanfree(&cp);
			continue;
		}
		scanfree(&cp);

		/* compute the amount of whitespace we want to have */
		if (xinf->bang)
		{
			/* For the ^T/^D commands, align the text with the
			 * next shift column.
			 */
			if (xinf->command == EX_SHIFTL)
			{
                                do
                                {
					ws--;
                                } while (ws > 0 && !opt_istab(sw, ws));
			}
			else
				ws += opt_totab(sw, ws);
		}
		else if (xinf->command == EX_SHIFTL)
		{
			for (multi = xinf->multi; ws > 0 && --multi >= 0; )
			{
				/* For the :< command, or < visual
				 * operator, subtract this columns width
				 * unless we're on the first char of the
				 * column, in which case use the width of
				 * the previous column.
				 */
				for (col = ws - 1; col > 0 && !opt_istab(sw, col); col--)
				{
				}
				ws -= opt_totab(sw, col);
			}
			if (ws < 0)
				ws = 0;
                }
                else /* xinf->command == EX_SHIFTR */
		{
			for (multi = xinf->multi; --multi >= 0; )
                	{
				/* For the :> command, or > visual
				 * operator, add this shift column's width.
				 * To do that, we must first find its
				 * width.
				 */
				for (col = ws; col > 0 && !opt_istab(sw, col); col--)
				{
				}
				ws += opt_totab(sw, col);
			}
		}

		/* Replace the old whitespace with new whitespace.  Since our
		 * buffer for holding new whitespace is of limited size, we
		 * may need to make several bufreplace() calls to do this.
		 */
		col = 0;
		while (markoffset(&start) != markoffset(&end) || ws > col)
		{
			/* build new whitespace (as much of it as possible) */
			i = 0;
			if (o_autotab(markbuffer(&xinf->defaddr)))
			{
				while (ws >= col + opt_totab(ts, col)
					&& i < QTY(str))
				{
					str[i++] = '\t';
					col += opt_totab(ts, col);
				}
			}
			while (ws > col && i < QTY(str))
			{
				str[i++] = ' ';
				col++;
			}

			/* replace old whitespace with new */
			bufreplace(&start, &end, str, i);
			markaddoffset(&start, i);
			marksetoffset(&end, markoffset(&start));
		}
	}

	if (o_report != 0 && xinf->to - xinf->from + 1 >= o_report)
		msg(MSG_INFO,"[ds]$1 lines $2ed",
			xinf->to - xinf->from + 1, xinf->cmdname);

	return RESULT_COMPLETE;
}


RESULT	ex_undo(xinf)
	EXINFO	*xinf;
{
	long	l = 1;

	assert(xinf->command == EX_UNDO || xinf->command == EX_REDO);

	/* choose an undo/redo level to recover */
	if (xinf->lhs)
	{
		if (!calcnumber(xinf->lhs) || (l = CHAR2long(xinf->lhs)) < 1)
		{
			msg(MSG_ERROR, "bad undo level");
			return RESULT_ERROR;
		}
	}

	/* if redo, then negate the undo value */
	if (xinf->command == EX_REDO)
		l = -l;

	/* try to revert to the undo level */
	l = bufundo(xinf->window->cursor, l);

	/* if successful, adjust the cursor position */
	if (l >= 0)
	{
		marksetoffset(xinf->window->cursor, l);
		return RESULT_COMPLETE;
	}

	return RESULT_ERROR;
}


RESULT	ex_write(xinf)
	EXINFO	*xinf;
{
	char	*name;
	ELVBOOL	success;

	if (xinf->rhs)
	{
		name = tochar8(xinf->rhs);
	}
	else if (xinf->nfiles >= 1)
	{
		assert(xinf->nfiles == 1);
		name = xinf->file[0];
	}
	else
	{
		name = tochar8(o_filename(markbuffer(xinf->fromaddr)));
		if (!name)
		{
			msg(MSG_ERROR, "[S]no file name for $1",
				o_bufname(markbuffer(xinf->fromaddr)));
			return RESULT_ERROR;	/* nishi */
		}
	}

	/* if writing to a different filename, remember that name */
	if (name[0] != '!'
	 && o_filename(markbuffer(xinf->fromaddr))
	 && CHARcmp(name, o_filename(markbuffer(xinf->fromaddr))))
	{
		if (name[0] == '>' && name[1] == '>')
			optprevfile(toCHAR(name + 2), 1);
		else
			optprevfile(toCHAR(name), 1);
	}

	/* actually write the file */
	success = bufwrite(xinf->fromaddr, xinf->toaddr, name, xinf->bang);
	return success ? RESULT_COMPLETE : RESULT_ERROR;
}


#ifdef FEATURE_MISC
RESULT	ex_z(xinf)
	EXINFO	*xinf;
{
	CHAR	type = '+';		/* type of window to show */
	long	count = o_window;	/* number of lines to show */
	PFLAG	pflag;			/* how to show */
	long	line = xinf->from;	/* first line to show */
	long	offset;
	CHAR	*scan;

	/* choose a default printing style, from options */
	if (o_number(xinf->window))
		if (o_list(xinf->window))
			pflag = PF_NUMLIST;
		else
			pflag = PF_NUMBER;
	else
		if (o_list(xinf->window))
			pflag = PF_LIST;
		else
			pflag = PF_NUMBER;

	/* If we were given arguments, then parse them */
	for (scan = xinf->rhs; scan && *scan; scan++)
	{
		switch (*scan)
		{
		  case '-':
		  case '+':
		  case '.':
		  case '^':
		  case '=':
			type = *scan;
			break;

		  case '0':
		  case '1':
		  case '2':
		  case '3':
		  case '4':
		  case '5':
		  case '6':
		  case '7':
		  case '8':
		  case '9':
			for (count = 0; elvdigit(*scan); scan++)
			{
				count = count * 10 + *scan - '0';
			}
			scan--; /* we went one character too far */
			break;

		  case 'l':
			if (pflag == PF_NUMBER || pflag == PF_NUMLIST)
				pflag = PF_NUMLIST;
			else
				pflag = PF_LIST;
			break;

		  case '#':
			if (pflag == PF_LIST || pflag == PF_NUMLIST)
				pflag = PF_NUMLIST;
			else
				pflag = PF_NUMBER;
			break;

		  case ' ':
		  case '\t':
		  case 'p':
			/* ignore */
			break;

		  default:
			msg(MSG_ERROR, "bad argument to :z");
			return RESULT_ERROR;
		}
	}

	/* choose the first line, based on the type */
	switch (type)
	{
	  case '-': /* show the given line at the bottom */
		line = xinf->from - count + 1;
		break;

	  case '+': /* show the given line at the top */
		line = xinf->from;
		break;

	  case '.': /* show the given line in the middle */
		line = xinf->from - count/2;
		break;

	  case '^': /* show the window before the current line */
		line = xinf->from - count * 2 + 1;
		break;
		
	  case '=': /* show it in the middle, surrounded by lines of hyphens */
		count -= 2;
		line = xinf->from - count / 2;
		break;
	}

	/* protect against readed past top or bottom of buffer */
	if (line < 1)
	{
		count -= 1 - line;
		line = 1;
	}
	if (line + count > o_buflines(markbuffer(xinf->fromaddr)))
	{
		count -= line - o_buflines(markbuffer(xinf->fromaddr));
	}

	/* construct a mark for the first line */
	xinf->newcurs = markdup(xinf->fromaddr);
	marksetline(xinf->newcurs, line);

	/* print the lines */
	if (type == '=')
	{
		/* for '=', we need to add lines of hyphens around given line */
		if (line < xinf->from)
		{
			exprintlines(xinf->window, xinf->newcurs, xinf->from - line, pflag);
		}
		drawextext(xinf->window, toCHAR("-------------------------------------------------------------------------------\n"), 80);
		exprintlines(xinf->window, xinf->fromaddr, 1, pflag);
		drawextext(xinf->window, toCHAR("-------------------------------------------------------------------------------\n"), 80);
		count -= (xinf->from - line) + 1;
		if (count > 0)
		{
			marksetline(xinf->newcurs, xinf->from + 1);
			exprintlines(xinf->window, xinf->newcurs, count, pflag);
		}

		/* leave the cursor on the given line */
		marksetoffset(xinf->newcurs, markoffset(xinf->fromaddr));
	}
	else
	{
		/* print the lines all at once */
		offset = exprintlines(xinf->window, xinf->newcurs, count, pflag);

		/* leave the cursor at the start of the last line */
		marksetoffset(xinf->newcurs, offset);
	}

	return RESULT_COMPLETE;
}
#endif /* FEATURE_MISC */
