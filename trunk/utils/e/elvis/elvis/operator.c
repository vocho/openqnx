/* operator.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_operator[] = "$Id: operator.c,v 2.47 2003/10/17 17:41:23 steve Exp $";
#endif


#if USE_PROTOTYPES
static RESULT	filterenter(WINDOW win);
#endif

static MARKBUF	fromdup, todup;

/* This function sends a portion of some buffer through an external filter
 * program, and replaces the old text with the output of the filter program.
 */
RESULT opfilter(from, to, prog)
	MARK	from;	/* start of text to send through filter */
	MARK	to;	/* end of text to send through filter */
	CHAR	*prog;	/* command-line of filter program */
{
	int	nchars;	/* number of chars for current I/O operation */
	int	totchars;/* total number of characters */
	MARKBUF	mark;	/* insertion point for replacement text */
	CHAR	*p;	/* used for scanning through text, or ptr to input buf */

	/* if no filter, then fail */
	if (!prog || !*prog)
	{
		msg(MSG_ERROR, "no filter");
		return RESULT_ERROR;
	}

	/* start the filter */
	if (gui->prgopen
		? !(*gui->prgopen)(tochar8(prog), ElvTrue, ElvTrue)
		: !prgopen(tochar8(prog), ElvTrue, ElvTrue))
	{
		return RESULT_ERROR;
	}

	/* write the old text out to the filter */
	for (scanalloc(&p, from), totchars = markoffset(to) - markoffset(from);
	     p && *p && totchars > 0;
	     totchars -= nchars, scanseek(&p, marktmp(mark, markbuffer(to), markoffset(to) - totchars)))
	{
		/* how many contiguous characters do we have buffered? */
		nchars = scanright(&p);
		if (nchars > totchars)
			nchars = totchars;

		/* write the characters out to the filter */
		if (prgwrite(p, nchars) < nchars)
		{
			msg(MSG_ERROR, "broken pipe");
			ioclose();
			scanfree(&p);
			return RESULT_ERROR;
		}
	}
	scanfree(&p);

	/* switch from writing to reading */
	if (!prggo())
	{
		return RESULT_ERROR;
	}

	/* read in the new text */
	for (mark = *to, p = (CHAR *)safealloc((int)o_blksize, sizeof(CHAR));
	     (nchars = prgread(p, (int)o_blksize)) > 0;
	     markaddoffset(&mark, nchars))
	    
	{
		bufreplace(&mark, &mark, p, nchars);
	}
	safefree(p);

	/* delete the old text */
	bufreplace(from, to, NULL, 0);

	/* Finish I/O to the filter program.  If the filter program had a
	 * non-zero exit status then return RESULT_ERROR (but still leave
	 * the effects of the attempted filter run in the edit buffer).
	 */
	return (gui->prgclose ? (*gui->prgclose)() : prgclose()) == 0 ? RESULT_COMPLETE : RESULT_ERROR;
}


/* This function is called when the user hits <Enter> after typing in a
 * command line for the visual <!> command.  It collects the characters
 * of the command-line into a dynamic string, and performs substitutions
 * on the the special characters !, %, and #.  The resulting command-line
 * is then stored as the "previouscommand" option's value.
 */
static RESULT	filterenter(win)
	WINDOW	win;	/* window where a command-line has been entered */
{
	CHAR	*cmd;	/* used for collecting the characters of the command line */
	CHAR	*cp;	/* used for scanning the command-line buffer */
	CHAR	*sub;	/* used for scanning a substitution value (for ! % #) */
	char	err;	/* error message (if this is an error) */
	long	i;

	assert(markoffset(win->state->top) <= markoffset(win->state->cursor));
	assert(markoffset(win->state->cursor) <= markoffset(win->state->bottom));

	/* skip over the '!' at the beginning of the command line, and any
	 * whitespace.
	 */
	scanalloc(&cp, win->state->top);
	if (cp && *cp == '!')
		scannext(&cp);
	while (cp && elvspace(*cp))
		scannext(&cp);

	/* Copy the command line into a buffer.  Note that the looping
	 * condition is "i > 1", not "i > 0", because we want to leave off
	 * the newline character at the end of the line.
	 */
	cmd = NULL;
	for (i = markoffset(win->state->bottom) - markoffset(win->state->top);
	     cp != NULL && i > 1;
	     scannext(&cp), i--)
	{
		err = *cp;
		switch (*cp)
		{
		  case '!':
			sub = o_previouscommand;
			break;

		  case '%':
			sub = o_filename(markbuffer(win->cursor));
			break;

		  case '#':
			sub = buffilenumber(&cp);
			break;

		  case '\\':
			if (!scannext(&cp))
			{
				goto Fail;
			}
			if (*cp != '!' && *cp != '%' && *cp != '#')
				(void)buildCHAR(&cmd, '\\');
			sub = cp;
			break;

		  default:
			sub = cp;
		}

		/* perform a substitution, if necessary */
		if (sub == cp)
			(void)buildCHAR(&cmd, *cp);
		else if (!sub)
		{
			msg(MSG_ERROR, "[c]no value to substitute for $1", err);
			goto Fail;
		}
		else
		{
			for (; *sub; sub++)
			{
				buildCHAR(&cmd, *sub);
			}
		}
	}
	scanfree(&cp);

	/* if no command was given, then fail */
	if (!cmd) goto Fail2;

	/* the new command becomes the "previouscommand" for next time */
	if (o_previouscommand)
		safefree(o_previouscommand);
	o_previouscommand = cmd;

	/* blank out the parsing info */
	assert(win->state->pop && win->state->acton == win->state->pop);
	viinitcmd((VIINFO *)win->state->pop->info);
	win->state->pop->flags &= ~ELVIS_MORE; /* !!! why is this necessary? */

	/* do the filtering */
	assert(markbuffer(&fromdup) == markbuffer(&todup));
	return opfilter(&fromdup, &todup, o_previouscommand);

Fail:	if (cmd)
		safefree(cmd);
	scanfree(&cp);
Fail2:
	assert(win->state->pop && win->state->acton == win->state->pop);
	viinitcmd((VIINFO *)win->state->pop->info);
	return RESULT_ERROR;
}


/* This function applies an operator to a chunk of text */
RESULT oper(win, vinf, from, to)
	WINDOW	win;	/* where the command was typed */
	VIINFO	*vinf;	/* information about the command */
	MARK	from;	/* start of the affected text */
	MARK	to;	/* end of the affected text */
{
	RESULT	result;
	EXINFO	xinfb;
	ELVBOOL	dot;
	MARK	curs;

	assert(markbuffer(from) == markbuffer(to));
	assert(markbuffer(from) == markbuffer(win->state->cursor));
	assert(markoffset(from) <= markoffset(to));

	/* are we doing this for a "." command? */
	dot = (ELVBOOL)((vinf->tweak & TWEAK_DOTTING) != 0);

	/* If this operator is being applied to a rectangular selection which
	 * happens to use only a single row, then treat it as a character
	 * selection.
	 */
	if (win->seltop
	 && win->seltype == 'r'
	 && markoffset((*win->md->move)(win, win->seltop, 0L, INFINITY, ElvFalse)) >= markoffset(win->selbottom))
	{
		win->seltype = 'c';
		marksetoffset(win->seltop, markoffset((*win->md->move)(win, win->seltop, 0L, win->selleft, ElvFalse)));
		marksetoffset(win->selbottom, markoffset((*win->md->move)(win, win->selbottom, 0L, win->selright + 1, ElvFalse)));
		marksetoffset(from, markoffset(win->seltop));
		marksetoffset(to, markoffset(win->selbottom));
	}

	/* very few rectangle operations for now */
	if (win->seltop && win->seltype == 'r'
		&& vinf->oper != 'y' && vinf->oper != 'd' && vinf->oper < 0x80)
	{
		msg(MSG_ERROR, "[C]can't $1 rectangles", vinf->oper);
		return RESULT_ERROR;
	}

	/* If this is a ! operator, then we have two modes of operation.  The
	 * first time this function is called, we need to push a new stratum
	 * to use for inputting the command line, and return RESULT_MORE.
	 * The second time (when ELVIS_MORE is set) we handle the filtering
	 * in the usual way.
	 */
	if (vinf->oper == '!' && !dot && !(win->state->flags & ELVIS_MORE))
	{
		statestratum(win, toCHAR(FILTER_BUF), vinf->oper, filterenter);
		o_inputtab(markbuffer(win->state->cursor)) = 'f'; /* filename */

		/* remember the range */
		fromdup = *from;
		todup = *to;
		return RESULT_MORE;
	}

	/* perform the operator */
	switch (vinf->oper)
	{
	  case 'y':
	  case 'd':
#ifdef FEATURE_G
	  case ELVG('u'):
	  case ELVG('U'):
	  case ELVG('~'):
	  case ELVG('='):
#endif
		if (win->seltop)
		{
			cutyank(vinf->cutbuf, from, to,
				(_CHAR_)(win->seltype == 'l' ? 'L' : win->seltype),
				vinf->oper);
			vinf->tweak |= (TWEAK_LINE | TWEAK_FRONT);
		}
		else
		{
			cutyank(vinf->cutbuf, from, to, (CHAR)((vinf->tweak & TWEAK_LINE) ? 'L' : 'c'), vinf->oper);
		}
		result = RESULT_COMPLETE;
		break;

	  case 'c':
		/* yank the text that we're about to change */
		if (win->seltop)
		{
			cutyank(vinf->cutbuf, from, to, win->seltype, dot ? 'd' : 'y');
			if (win->seltype == 'l')
			{
				vinf->tweak |= (TWEAK_LINE | TWEAK_FRONT);
			}
		}
		else
		{
			cutyank(vinf->cutbuf, from, to, (CHAR)((vinf->tweak & TWEAK_LINE) ? 'L' : 'c'), dot ? 'd' : 'y');
		}

		if (dot)
		{
			/* replace the text with previous input */
			curs = cutput('.', win, from, ElvFalse, ElvTrue, ElvTrue);

			/* if line mode, then append a newline */
			if (vinf->tweak & TWEAK_LINE)
			{
				markaddoffset(curs, 1L);
				bufreplace(curs, curs, toCHAR("\n"), 1);
				markaddoffset(curs, -1L);
			}

			/* leave cursor at the end of the replacement text */
			marksetoffset(win->state->cursor, markoffset(curs));
		}
		else
		{
			/* set the edit boundaries */
			inputchange(win, from, to, (vinf->tweak & TWEAK_LINE) ? ElvTrue : ElvFalse);
		}

		result = RESULT_COMPLETE;
		break;

	  case '<':
	  case '>':
		/* build a :< or :> ex command */
		memset((char *)&xinfb, 0, sizeof xinfb);
		xinfb.window = win;
		xinfb.cmdname = (vinf->oper == '<') ? "<" : ">";
		xinfb.command = (vinf->oper == '<') ? EX_SHIFTL : EX_SHIFTR;
		xinfb.defaddr = *from;
		xinfb.fromaddr = from;
		xinfb.toaddr = to;
		xinfb.from = markline(from);
		xinfb.to = markline(to) - 1;
		xinfb.multi = 1;

		/* execute the command */
		result = ex_shift(&xinfb);
		break;

	  case '=':
		if ((win->seltop && win->seltype == 'c')
		 || (!win->seltype && (vinf->tweak & TWEAK_LINE) == 0))
		{
#ifdef FEATURE_CALC
			result = calcsel(from, to) ? RESULT_COMPLETE : RESULT_ERROR;
#else
			result = RESULT_ERROR;
#endif
		}
		else
		{
			result = opfilter(from, to, o_equalprg(markbuffer(to)));
		}
		break;

	  case '!':
		if (!dot)
		{
			/* NOTE: This is the second call to operator() for this
			 * ! command.  The first call pushed a stratum so the
			 * command line could be read, and it remembered the
			 * range to affect.  On this call, the command line
			 * has been stored in o_previouscommand, but the range
			 * is messed up due to a limitation of the parser.
			 * USE THE OFFSETS FROM THE FIRST CALL.
			 */
			assert(markbuffer(from) == markbuffer(&fromdup));
			marksetoffset(from, markoffset(&fromdup));
			marksetoffset(to, markoffset(&todup));
		}
		result = opfilter(from, to, o_previouscommand);
		break;

	  default:
		/* other operators aren't implemented yet */
		result = RESULT_ERROR;
	}

	/* If the cursor is left after the end of the buffer, then move it
	 * to either the front of the last line (if line-mode) or the end
	 * of the last line (if not line-mode).
	 */
	if (result == RESULT_COMPLETE
	 && o_bufchars(markbuffer(win->state->cursor)) != 0
	 && markoffset(win->state->cursor) >= o_bufchars(markbuffer(win->state->cursor)))
	{
		markaddoffset(win->state->cursor, -1);
		if (vinf->tweak & TWEAK_LINE)
		{
			(void)m_front(win, vinf);
		}
	}

	/* We don't want to leave the cursor after the last character of the
	 * line, unless there are no characters on the line.
	 */
	if (result == RESULT_COMPLETE
	 && markoffset(win->state->cursor) > markoffset(dispmove(win, 0, INFINITY)))
	{
		markaddoffset(win->state->cursor, -1);
	}
	return result;
}
