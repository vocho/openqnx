/* buffer.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_buffer[] = "$Id: buffer.c,v 2.150 2003/10/17 17:41:23 steve Exp $";
#endif

#define swaplong(x,y)	{long tmp; tmp = (x); (x) = (y); (y) = tmp;}

#if USE_PROTOTYPES
static void freeundo(BUFFER buffer);
static struct undo_s *allocundo(BUFFER buf);
static void bufdo(BUFFER buf, ELVBOOL wipe);
static void didmodify(BUFFER buf);
# ifdef FEATURE_MISC
  static void proc(_BLKNO_ bufinfo, long nchars, long nlines, long changes,
  			long prevloc, CHAR *name);
# endif
# ifdef DEBUG_ALLOC
  static void checkundo(char *where);
  static void removeundo(struct undo_s *undo);
# endif
#endif

/* This variable points to the head of a linked list of buffers */
BUFFER elvis_buffers;

/* This is the default buffer.  Its options have been inserted into the
 * list accessible via optset().  This variable should only be changed by
 * the bufoptions() function.
 */
BUFFER bufdefault;

/* This stores the message type that will be used for reporting the number
 * of lines read or written.  It is normally MSG_STATUS so that other messages
 * will be allowed to overwrite it; however, when quitting it is set to
 * MSG_INFO so messages will be queued and eventually displayed somewhere
 * else after the window is closed.  It is also set to MSG_INFO during
 * initialization so the "read..." message appears in the new window.
 */
MSGIMP bufmsgtype = MSG_INFO;

/* This buffer's contents are irrelevent.  The values of its options, though,
 * are significant because the values of its options are used as the default 
 * values of any new buffer.  This buffer is also used as the default buffer
 * during execution of the initialization scripts.
 */
BUFFER bufdefopts;

#ifdef FEATURE_AUTOCMD
/* This option is set while reading text into the buffer, so that it won't
 * trigger an Edit autocmd.
 */
static ELVBOOL bufnoedit;
#endif

/* This array describes buffer options */
static OPTDESC bdesc[] =
{
	{"filename", "file",	optsstring,	optisstring	},
	{"bufname", "buffer",	optsstring,	optisstring	},
	{"bufid", "bufferid",	optnstring,	optisnumber	},
	{"buflines", "bl",	optnstring,	optisnumber	},
	{"bufchars", "bc",	optnstring,	optisnumber	},
	{"retain", "ret",	NULL,		NULL		},
	{"modified", "mod",	NULL,		NULL,		},
	{"edited", "samename",	NULL,		NULL,		},
	{"newfile", "new",	NULL,		NULL,		},
	{"readonly", "ro",	NULL,		NULL,		},
	{"autoindent", "ai",	NULL,		NULL,		},
	{"inputtab", "it",	opt1string,	optisoneof,	"tab spaces ex filename identifier"},
	{"autotab", "at",	NULL,		NULL,		},
	{"tabstop", "ts",	opttstring,	optistab,	"8"},
	{"ccprg", "cp",		optsstring,	optisstring	},
	{"equalprg", "ep",	optsstring,	optisstring	},
	{"keywordprg", "kp",	optsstring,	optisstring	},
	{"makeprg", "mp",	optsstring,	optisstring	},
	{"paragraphs", "para",	optsstring,	optisstring	},
	{"sections", "sect",	optsstring,	optisstring	},
	{"shiftwidth", "sw",	opttstring,	optistab,	"8"},
	{"undolevels", "ul",	optnstring,	optisnumber	},
	{"textwidth", "tw",	optnstring,	optisnumber	},
	{"internal", "internal",NULL,		NULL		},
	{"bufdisplay", "bd",    optsstring,	optisstring	},
	{"initialsyntax","isyn",NULL,		NULL		},
	{"errlines", "errlines",optnstring,	optisnumber	},
	{"readeol", "reol",	opt1string,	optisoneof,	"unix dos mac text binary"},
	{"locked", "lock",	NULL,		NULL		},
	{"partiallastline","pll",NULL,		NULL		},
	{"putstyle", "ps",	opt1string,	optisoneof,	"character line rectangle"},
	{"timestamp", "time",	optsstring,	optisstring	},
	{"guidewidth", "gw",	opttstring,	optistab	},
	{"hlobject", "hlo",	optsstring,	optisstring	},
	{"spell", "sp",		NULL,		NULL		},
	{"lisp", "lisp",	NULL,		NULL		},
	{"mapmode", "mm",	optsstring,	optisstring	},
	{"smartargs", "sa",	NULL,		NULL		},
	{"userprotocol", "up",	NULL,		NULL		},
	{"bb", "bb",		optsstring,	optisstring	}
};

#ifdef DEBUG_ALLOC
/* This are used for maintaining a linked list of all undo versions. */
struct undo_s *undohead, *undotail;

/* This function is called after code which is suspected of leaking memory.
 * It checks all of the undo versions, making sure that each one is still
 * accessible via some buffer.
 */
static void checkundo(where)
	char	*where;
{
	struct undo_s	*scan, *undo;
	BUFFER		buf;

	/* for each undo version... */
	for (scan = undohead; scan; scan = scan->link1)
	{
		/* make sure the buffer still exists */
		for (buf = elvis_buffers; buf != scan->buf; buf = buf->next)
		{
			if (!buf)
				msg(MSG_FATAL, "[s]$1 - buffer disappeared, undo/redo not freed", where);
		}

		/* make sure this is an undo/redo for this buffer */
		if (scan->undoredo == 'l')
		{
			if (scan != buf->undolnptr)
			{
				msg(MSG_FATAL, "[s]$1 - undolnptr version leaked", where);
			}
		}
		else
		{
			for (undo = scan->undoredo=='u' ? buf->undo : buf->redo;
			     undo != scan;
			     undo = undo->next)
			{
				if (!undo)
					msg(MSG_FATAL, "[ss]$1 - $2 version leaked", where, scan->undoredo=='u'?"undo":"redo");
			}
		}
	}
}

static void removeundo(undo)
	struct undo_s	*undo;
{
	if (undo->link1)
		undo->link1->link2 = undo->link2;
	else
		undotail = undo->link2;
	if (undo->link2)
		undo->link2->link1 = undo->link1;
	else
		undohead = undo->link1;
}
#else
# define checkundo(s)
#endif

#ifdef FEATURE_MISC

/* This function is called during session file initialization.  It creates
 * a BUFFER struct for the buffer, and collects the undo versions.
 */
static void proc(bufinfo, nchars, nlines, changes, prevloc, name)
	_BLKNO_	bufinfo;	/* block describing the buffer */
	long	nchars;		/* number of characters in buffer */
	long	nlines;		/* number of lines in buffer */
	long	changes;	/* value of "changes" counter */
	long	prevloc;	/* offset of most recent change to buffer */
	CHAR	*name;		/* name of the buffer */
{
	BUFFER		buf;
	BLKNO		tmp;
	struct undo_s	*undo, *scan, *lag;
	ELVBOOL		internal;

	/* try to find a buffer by this name */
	for (buf = elvis_buffers; buf && CHARcmp(o_bufname(buf), name); buf = buf->next)
	{
	}

	/* if no buffer exists yet, then create one and make it use the old
	 * bufinfo block.
	 */
	if (!buf)
	{
		internal = (ELVBOOL)(!CHARncmp(name, toCHAR("Elvis "), 6) &&
				CHARncmp(name, toCHAR("Elvis untitled"), 14));
		buf = bufalloc(name, bufinfo, internal);
		buf->bufinfo = bufinfo;
		o_buflines(buf) = nlines;
		o_bufchars(buf) = nchars;
		buf->changes = changes;
		buf->changepos = prevloc;

		/* guess some values for a few other critical options */
		if (!CHARncmp(name, toCHAR("Elvis "), 6) &&
			CHARncmp(name, toCHAR("Elvis untitled"), 14))
		{
			/* probably an internal buffer */
			optpreset(o_internal(buf), ElvTrue, OPT_HIDE|OPT_NODFLT);
			optpreset(o_modified(buf), ElvFalse, OPT_HIDE|OPT_NODFLT);
			optpreset(o_filename(buf), NULL, OPT_HIDE|OPT_LOCK);
		}
		else
		{
			/* the filename is probably the same as the buffer name */
			optpreset(o_filename(buf), CHARdup(name), OPT_FREE|OPT_HIDE);
			optpreset(o_internal(buf), ElvFalse, OPT_HIDE);

			/* Mark it as readonly so the user will have to think
			 * before clobbering an existing file.
			 */
			optpreset(o_readonly(buf), ElvTrue, OPT_HIDE|OPT_NODFLT);

			/* Mark it as modified, so the user has to think
			 * before exitting and losing this session file.
			 */
			optpreset(o_modified(buf), ElvTrue, OPT_HIDE|OPT_NODFLT);
		}
		return;
	}

	/* We found the buffer.  Is this the newest version found so far? */
	if (changes > buf->changes)
	{
		/* yes, this is the newest.  Swap this one with the version
		 * currently in the buf struct.  That will leave this version
		 * (the newest) as the current version, and the current
		 * (second newest) in the arguements and ready to be added
		 * to the undo list.
		 */
		tmp = buf->bufinfo;
		buf->bufinfo = bufinfo;
		bufinfo = tmp;
		swaplong(o_buflines(buf), nlines);
		swaplong(o_bufchars(buf), nchars);
		swaplong(buf->changes, changes);
		swaplong(buf->changepos, prevloc);
	}

	/* insert as an "undo" version */
	undo = (struct undo_s *)safealloc(1, sizeof *undo);
	undo->changes = changes;
	undo->changepos = prevloc;
	undo->buflines = nlines;
	undo->bufchars = nchars;
	undo->bufinfo = bufinfo;
	for (scan = buf->undo, lag = NULL;
	     scan && scan->changes > changes;
	     lag = scan, scan = scan->next)
	{
	} 
	undo->next = scan;
	if (lag)
	{
		lag->next = undo;
	}
	else
	{
		buf->undo = undo;
	}
#ifdef DEBUG_ALLOC
	undo->link1 = undohead;
	undohead = undo;
	undo->link2 = NULL;
	if (undo->link1)
		undo->link1->link2 = undo;
	else
		undotail = undo;
	undo->buf = buf;
	undo->undoredo = 'u';
#endif
}
#endif /* FEATURE_MISC */

/* Restart a session */
void bufinit()
{
	assert(BUFOPTQTY == QTY(bdesc));

	/* find any buffers left over from a previous edit */
#ifndef FEATURE_MISC
	lowinit(NULL);
#else
	lowinit(proc);
#endif

	/* create the default options buffer, if it doesn't exist already */
	bufdefopts = bufalloc(toCHAR(DEFAULT_BUF), 0, ElvTrue);
	bufoptions(bufdefopts);
}

/* Create a buffer with a given name.  The buffer will initially be empty;
 * if it is meant to be associated with a particular file, then the file must
 * be copied into the buffer in a separate operation.  If there is already
 * a buffer with the desired name, it returns a pointer to the old buffer
 * instead of creating a new one.
 */
BUFFER bufalloc(name, bufinfo, internal)
	CHAR	*name;	/* name of the buffer */
	_BLKNO_	bufinfo;/* block number describing the buffer (0 to create) */
	ELVBOOL internal;/* is this supposed to be an internal buffer? */
{
	BUFFER	buffer;
	BUFFER	scan, lag;	/* used for inserting new buffer */
	char	unique[255];	/* name of untitled buffer */
 static long	bufid = 1;	/* for generating bufid values */
 static long	intbufid = -1;	/* used for generating internal names */

	/* if no name was specified, generate a unique untitled name */
	if (!name)
	{
		sprintf(unique, UNTITLED_BUF, (int)(internal ? intbufid : bufid));
		name = toCHAR(unique);
		if (internal)
			intbufid--;
		/* Note that bufid will be incremented below, when initializing
		 * the o_bufid(buf) option.
		 */
	}

	/* see if there's already a buffer with that name */
	buffer = buffind(name);
	if (buffer)
	{
		return buffer;
	}

	/* allocate buffer struct */
	buffer = (BUFFER)safekept(1, sizeof(*buffer));

	/* create a low-level buffer */
	if (bufinfo)
	{
		buffer->bufinfo = bufinfo;
	}
	else
	{
		buffer->bufinfo = lowalloc(tochar8(name));
	}

	/* initialize the buffer options */
	optpreset(o_readonly(buffer), o_defaultreadonly, OPT_HIDE|OPT_NODFLT);
	if (bufdefopts)
	{
		/* copy all options except the following: filename bufname bufid
		 * buflines bufchars modified edited newfile internal autotab
		 * partiallastline putstyle timestamp readeol.
		 */
		optpreset(buffer->retain, bufdefopts->retain, OPT_HIDE);
		buffer->autoindent = bufdefopts->autoindent;
		buffer->inputtab = bufdefopts->inputtab;
		optpreset(o_tabstop(buffer),
			(short *)safealloc(2 + o_tabstop(bufdefopts)[0],
			sizeof(short)), OPT_FREE|OPT_REDRAW);
		    memcpy(o_tabstop(buffer), o_tabstop(bufdefopts), 
			    (2 + o_tabstop(bufdefopts)[0]) * sizeof(short));
		optpreset(o_shiftwidth(buffer),
			(short *)safealloc(2 + o_shiftwidth(bufdefopts)[0],
			sizeof(short)), OPT_FREE);
		    memcpy(o_shiftwidth(buffer), o_shiftwidth(bufdefopts),
			    (2 + o_shiftwidth(bufdefopts)[0]) * sizeof(short));
		buffer->undolevels = bufdefopts->undolevels;
		buffer->initialsyntax = bufdefopts->initialsyntax;
		buffer->textwidth = bufdefopts->textwidth;
		buffer->autotab = bufdefopts->autotab;
		buffer->locked = bufdefopts->locked;
		buffer->guidewidth = bufdefopts->guidewidth;
		if (o_guidewidth(buffer))
		{
			optpreset(o_guidewidth(buffer),
				(short *)safealloc(2 + o_guidewidth(bufdefopts)[0],
				sizeof(short)), OPT_FREE|OPT_HIDE);
			    memcpy(o_guidewidth(buffer),
				o_guidewidth(bufdefopts),
				(2 + o_guidewidth(bufdefopts)[0]) * sizeof(short));
		}
		buffer->spell = bufdefopts->spell;
		buffer->lisp = bufdefopts->lisp;
		buffer->smartargs = bufdefopts->smartargs;
		buffer->userprotocol = bufdefopts->userprotocol;

		/* Strings are tricky, because we may need to allocate a
		 * duplicate of the value.
		 */
		buffer->cc = bufdefopts->cc;
		if (buffer->cc.flags & OPT_FREE)
			o_cc(buffer) = CHARdup(o_cc(bufdefopts));
		buffer->cc.flags |= OPT_HIDE;
		buffer->equalprg = bufdefopts->equalprg;
		if (buffer->equalprg.flags & OPT_FREE)
			o_equalprg(buffer) = CHARdup(o_equalprg(bufdefopts));
		buffer->keywordprg = bufdefopts->keywordprg;
		if (buffer->keywordprg.flags & OPT_FREE)
			o_keywordprg(buffer) = CHARdup(o_keywordprg(bufdefopts));
		buffer->make = bufdefopts->make;
		if (buffer->make.flags & OPT_FREE)
			o_make(buffer) = CHARdup(o_make(bufdefopts));
		buffer->make.flags |= OPT_HIDE;
		buffer->paragraphs = bufdefopts->paragraphs;
		if (buffer->paragraphs.flags & OPT_FREE)
			o_paragraphs(buffer) = CHARdup(o_paragraphs(bufdefopts));
		buffer->sections = bufdefopts->sections;
		if (buffer->sections.flags & OPT_FREE)
			o_sections(buffer) = CHARdup(o_sections(bufdefopts));
		buffer->bufdisplay = bufdefopts->bufdisplay;
		if (buffer->bufdisplay.flags & OPT_FREE)
			o_bufdisplay(buffer) = CHARdup(o_bufdisplay(bufdefopts));
		buffer->hlobject = bufdefopts->hlobject;
		if (buffer->hlobject.flags & OPT_FREE)
			o_hlobject(buffer) = CHARdup(o_hlobject(bufdefopts));

		buffer->mapmode = bufdefopts->mapmode;
		if (buffer->mapmode.flags & OPT_FREE)
			o_mapmode(buffer) = CHARdup(o_mapmode(bufdefopts));
	}
	else /* no default options -- set them explicitly */
	{
		o_inputtab(buffer) = 't';
		o_autotab(buffer) = ElvTrue;
		o_tabstop(buffer) = safealloc(2, sizeof(short));
			o_tabstop(buffer)[0] = 0;
			o_tabstop(buffer)[1] = 8;
			optflags(o_tabstop(buffer)) = OPT_FREE|OPT_REDRAW;
#ifndef OSCCPRG
# define OSCCPRG "cc ($1?$1:$2)"
#endif
		optpreset(o_cc(buffer), toCHAR(OSCCPRG), OPT_UNSAFE);
		optpreset(o_equalprg(buffer), toCHAR("fmt"), OPT_UNSAFE);
		optpreset(o_keywordprg(buffer), toCHAR("ref $1 file:$2"), OPT_UNSAFE);
#ifndef OSMAKEPRG
# define OSMAKEPRG "make $1"
#endif
		optpreset(o_make(buffer), toCHAR(OSMAKEPRG), OPT_UNSAFE);
		o_paragraphs(buffer) = toCHAR("PPppIPLPQPP");
		o_sections(buffer) = toCHAR("NHSHSSSEse");
		o_shiftwidth(buffer) = safealloc(2, sizeof(short));
			o_shiftwidth(buffer)[0] = 0;
			o_shiftwidth(buffer)[1] = 8;
			optflags(o_shiftwidth(buffer)) = OPT_FREE;
		o_undolevels(buffer) = 0;
		optpreset(o_bufdisplay(buffer), toCHAR("normal"), OPT_NODFLT);
		optpreset(o_initialsyntax(buffer), ElvFalse, OPT_HIDE|OPT_NODFLT);
		optflags(o_textwidth(buffer)) = OPT_REDRAW;
		optpreset(o_locked(buffer), ElvFalse, OPT_HIDE|OPT_NODFLT);
		optflags(o_guidewidth(buffer)) = OPT_HIDE|OPT_SCRATCH;
		optflags(o_hlobject(buffer)) = OPT_HIDE|OPT_REDRAW;
		optpreset(o_spell(buffer), ElvFalse, OPT_HIDE);
		optpreset(o_smartargs(buffer), ElvFalse, OPT_HIDE);
		optpreset(o_userprotocol(buffer), ElvFalse, OPT_HIDE|OPT_NODFLT);
		optflags(o_mapmode(buffer)) = OPT_HIDE|OPT_REDRAW;
	}

	/* set the name of this buffer, and limit access to some options */
	optpreset(o_bufname(buffer), CHARkdup(name), OPT_HIDE|OPT_LOCK|OPT_FREE);
	optpreset(o_internal(buffer), internal, OPT_HIDE|OPT_LOCK);
	optflags(o_buflines(buffer)) = OPT_HIDE|OPT_LOCK;
	optflags(o_bufchars(buffer)) = OPT_HIDE|OPT_LOCK;
	optflags(o_bufid(buffer)) = OPT_HIDE|OPT_LOCK;
	optflags(o_filename(buffer)) |= OPT_HIDE;
	optflags(o_edited(buffer)) |= OPT_HIDE|OPT_NODFLT;
	optflags(o_errlines(buffer)) |= OPT_HIDE|OPT_NODFLT;
	optflags(o_modified(buffer)) |= OPT_HIDE|OPT_NODFLT;
	optflags(o_newfile(buffer)) |= OPT_HIDE|OPT_NODFLT;
	if (!internal)
	{
		o_bufid(buffer) = bufid++;
	}
	optpreset(o_partiallastline(buffer), ElvFalse, OPT_HIDE|OPT_NODFLT);
	optpreset(o_putstyle(buffer), 'c', OPT_HIDE|OPT_LOCK);
	optpreset(o_timestamp(buffer), NULL, OPT_HIDE|OPT_NODFLT);
	optpreset(o_readeol(buffer), o_binary ? 'b' : 't', OPT_HIDE|OPT_NODFLT);
	optpreset(o_bb(buffer), NULL, OPT_HIDE|OPT_NODFLT);

	/* initialize the "willevent" field to a safe value */
	buffer->willevent = -1;

	/* Add the buffer to the linked list of buffers.  Keep it sorted. */
	for (lag = (BUFFER)0, scan = elvis_buffers;
	     scan && CHARcmp(o_bufname(scan), o_bufname(buffer)) < 0;
	     lag = scan, scan = scan->next)
	{
	}
	buffer->next = scan;
	if (lag)
	{
		lag->next = buffer;
	}
	else
	{
		elvis_buffers = buffer;
	}

#ifdef FEATURE_AUTOCMD
	if (!o_internal(buffer))
	{
		BUFFER oldbuf = bufdefault;
		bufoptions(buffer);
		auperform(windefault, ElvFalse, NULL, AU_BUFCREATE, NULL);
		bufoptions(oldbuf);
	}
#endif /* FEATURE_AUTOCMD */



	/* return the new buffer */
	return buffer;
}

/* Locate a buffer with a particular name.  (Note that this uses the buffer
 * name, not the filename.)  Returns a pointer to the buffer, or NULL for
 * unknown buffers.
 */
BUFFER buffind(name)
	CHAR	*name;	/* name of the buffer to find */
{
	BUFFER	buffer;
	long	bufid;
	CHAR	*b, *n;

	/* scan through buffers, looking for a match */
	for (buffer = elvis_buffers; buffer && CHARcmp(name, o_bufname(buffer)); buffer = buffer->next)
	{
	}

	/* If no exact match, and the name is a quote and some other char, then
	 * try searching for a cut buffer.
	 */
	if (!buffer && name[0] == '"' && name[1] && !name[2])
	{
		buffer = cutbuffer(name[1], ElvFalse);
	}

	/* If no exact match, and name is the initials of a buffer, then
	 * use that.
	 */
	if (!buffer && name[0] == '"')
	{
		for (buffer = elvis_buffers; buffer; buffer = buffer->next)
		{
			/* if first char doesn't match, skip it */
			if (*o_bufname(buffer) != name[1])
				continue;

			/* check other chars as initials */
			for (b = o_bufname(buffer) + 1, n = name + 2; *b; b++)
			{
				if (b[-1] == ' ')
				{
					if (*b == *n)
						n++;
					else
						break;
				}
			}

			/* if matched, then stop looking */
			if (!*n && !*b)
				break;
		}
	}

	/* If no exact match, and the name looks like a number, then try
	 * searching by bufid.
	 */
	if (!buffer && *name == '#' && calcnumber(++name))
	{
		for (bufid = atol(tochar8(name)), buffer = elvis_buffers;
		     buffer && o_bufid(buffer) != bufid;
		     buffer = buffer->next)
		{
		}
	}

	/* If no exact match by buffer name, then try converting the name to
	 * an absolute name.  This should allow users to say (Makefile) when
	 * they really mean (/home/steve/elvis-2.2/Makefile).
	 */
	if (!buffer)
	{
		name = toCHAR(ioabsolute(tochar8(name)));
		for (buffer = elvis_buffers;
		     buffer && CHARcmp(name, o_bufname(buffer));
		     buffer = buffer->next)
		{
		}
	}

	return buffer;
}


/* Look up the filename of a buffer, given a reference to a scan variable.
 * The scan variable should be in an existing scan context, and should point
 * to a '#' character.  If the filename is found, the scan variable is left
 * on the last character of the # expression, and the filename is returned.
 * Otherwise it returns NULL and the scan variable is undefined.
 *
 * This is used for expanding # in filenames.
 */
CHAR *buffilenumber(refp)
	CHAR	**refp;
{
	long	id;
	BUFFER	buf;
	MARKBUF	tmp;

	assert(*refp && **refp == '#');

	/* remember which buf we're scanning, so we can fix *refp after NULL */
	buf = markbuffer(scanmark(refp));
	if (buf)
	{
		/* the scanning context is in a buffer */

		/* get the buffer number */
		for (id = 0; scannext(refp) && elvdigit(**refp); )
		{
			id = id * 10 + **refp - '0';
		}

		/* move back one character, so *refp points to final char. This
		 * can be tricky if we bumped into the end of the buffer.
		 */
		if (*refp)
			(void)scanprev(refp);
		else
			(void)scanseek(refp, marktmp(tmp, buf, o_bufchars(buf) - 1));
	}
	else
	{
		/* The scanning context is in a string. */

		/* get the buffer number */
		for (id = 0; elvdigit(*++*refp); )
		{
			id = id * 10 + **refp - '0';
		}
		--*refp;
	}

	/* if 0, then use alternate file */
	if (id == 0)
		return o_previousfile;

	/* try to find a buffer with that value */
	for (buf = elvis_buffers; buf && o_bufid(buf) != id; buf = buf->next)
	{
	}
	return buf ? o_filename(buf) : NULL;
}



/* Read a text file or filter output into a specific place in the buffer */
ELVBOOL bufread(mark, rfile)
	MARK	mark;	/* where to insert the new next */
	char	*rfile;	/* file to read, or !cmd for filter */
{
	BUFFER	buf;		/* the buffer we're reading into */
	long	offset;		/* offset of mark before inserting text */
	long	origlines;	/* original number of lines in file */
	CHAR	chunk[4096];	/* I/O buffer */
	int	nread;		/* number of bytes in chunk[] */
	ELVBOOL	newbuf;		/* is this a new buffer? */
#ifdef FEATURE_AUTOCMD
	ELVBOOL	filter;
	ELVBOOL doevents;
	BUFFER	oldbuf = bufdefault;
	MARKBUF	from;
#endif

#ifdef FEATURE_PROTO
	switch (urlalias(mark, NULL, rfile))
	{
	  case RESULT_COMPLETE:	return ElvTrue;
	  case RESULT_ERROR:	return ElvFalse;
	  case RESULT_MORE:	; /* fall through to normal code */
	}
#endif /* FEATURE PROTO */

	/* initialize some vars */
	buf = markbuffer(mark);
	newbuf = (ELVBOOL)(o_bufchars(buf) == 0 && rfile[0] != '!' && (o_verbose || !o_internal(buf)));
	origlines = o_buflines(buf);
#ifdef FEATURE_AUTOCMD
	filter = (ELVBOOL)(*rfile == '!');
	doevents = (ELVBOOL)(o_newfile(buf) || !o_filename(buf) || o_bufchars(buf) > 0);
	from = *mark;

	if (doevents)
	{
		bufoptions(markbuffer(mark));
		if (auperform(windefault, ElvFalse, NULL,
			filter ? AU_FILTERREADPRE : AU_FILEREADPRE,
			toCHAR(rfile)))
			return ElvFalse;
	}
#endif

	/* open the file/filter */
	if (!ioopen(rfile, 'r', ElvFalse, ElvFalse, o_readeol(markbuffer(mark))))
	{
		/* failed -- error message already given */
		return ElvFalse;
	}

	/* read the text */
	if (newbuf)
		msg(MSG_STATUS, "[s]reading $1", rfile);
	while ((nread = ioread(chunk, QTY(chunk))) > 0)
	{
		if (guipoll(ElvFalse))
		{
			ioclose();
			return ElvFalse;
		}
		offset = markoffset(mark);
		bufreplace(mark, mark, chunk, nread);
		marksetoffset(mark, offset + nread);
	}
	ioclose();

#ifdef FEATURE_AUTOCMD
	if (doevents && o_buflines(buf) > origlines)
	{
		autop = markdup(&from);
		aubottom = markdup(mark);
		markaddoffset(aubottom, -1L);
		bufoptions(markbuffer(mark));
		(void)auperform(windefault, ElvFalse, NULL,
			filter ? AU_FILTERREADPOST : AU_FILEREADPOST,
			toCHAR(rfile));
		markfree(aubottom);
		markfree(autop);
		autop = aubottom = NULL;
		bufoptions(oldbuf);
	}
#endif

	if (newbuf)
		msg(bufmsgtype, "[sdd]read $1, $2 lines, $3 chars", rfile,
					o_buflines(buf), o_bufchars(buf));
	else if (!o_internal(buf)
	      && o_report != 0
	      && o_buflines(buf) - origlines >= o_report)
		msg(bufmsgtype, "[d]read $1 lines", o_buflines(buf) - origlines);

	return ElvTrue;
}

/* Create a buffer for a given file, and then load the file.  Return a pointer
 * to the buffer.  If the file can't be read for some reason, then complain and
 * leave the buffer empty, but still return the empty buffer.
 *
 * If the buffer already exists and contains text, then the "reload" option
 * can be used to force it to discard that text and reload the buffer; when
 * "reload" is ElvFalse, it would leave the buffer unchanged instead.
 */
BUFFER bufload(bufname, filename, reload)
	CHAR	*bufname;	/* name of buffer, or NULL to derive from filename */
	char	*filename;	/* name of file to load into a buffer */
	ELVBOOL	reload;		/* load from file even if previously loaded? */
{
	BUFFER	buf;
	MARKBUF	top;
	MARKBUF	end;
	MARK	mark;
	int	i;
	ELVBOOL	internal;
#ifdef FEATURE_CALC
	BUFFER	initbuf;	/* buffer containing the initialization script*/
	RESULT	result;
	EXCTLSTATE oldctlstate;
# ifdef FEATURE_MISC
	void	*locals;
# endif
#endif


#ifdef FEATURE_AUTOCMD
	/* Loading a file shouldn't generate Edit autocmd events */
	bufnoedit = ElvTrue;
#endif

	/* Create a buffer.  The buffer's default name is derived from the
	 * given file name.  Note that we make a local copy of that name
	 * because ioabsolute() returns a static buffer which bufalloc()
	 * [really buffind()] clobbers the buffer before it uses the name.
	 */
	internal = (ELVBOOL)(bufname != NULL);
	if (!bufname)
		bufname = CHARdup(toCHAR(ioabsolute(filename)));
	buf = bufalloc(bufname, 0, internal);
	if (!internal)
		safefree(bufname);

	/* Does the buffer already contain text? */
	if (o_bufchars(buf) > 0)
	{
		/* If we aren't supposed to reload, then just return the
		 * buffer as-is.
		 */
		if (!reload)
		{
#ifdef FEATURE_AUTOMD
			bufnoedit = ElvFalse;
#endif
			return buf;
		}

		/* Save the text as an "undo" version, and then delete it */
		if (windefault && markbuffer(windefault->cursor) == buf)
			bufwilldo(windefault->cursor, ElvTrue);
		else
			bufwilldo(marktmp(top, buf, 0), ElvTrue);
		bufreplace(marktmp(top, buf, 0), marktmp(end, buf, o_bufchars(buf)), (CHAR *)0, 0);
	}

#ifdef FEATURE_REGION
	/* Initially the buffer should contain no regions */
	regiondel(marktmp(top, buf, 0), &top, '\0');
#endif

	/* Set the buffer's options */
	optpreset(o_filename(buf), CHARkdup(filename), OPT_HIDE|OPT_LOCK|OPT_FREE);
	optpreset(o_edited(buf), ElvTrue, OPT_HIDE|OPT_NODFLT);
	optpreset(o_readonly(buf), o_defaultreadonly, OPT_HIDE|OPT_NODFLT);
	optpreset(o_newfile(buf), ElvFalse, OPT_HIDE|OPT_NODFLT);
	switch (urlperm(filename))
	{
	  case DIR_INVALID:
	  case DIR_BADPATH:
	  case DIR_NOTFILE:
	  case DIR_DIRECTORY:
	  case DIR_UNREADABLE:
	  case DIR_READONLY:
		o_readonly(buf) = ElvTrue;
		break;

	  case DIR_NEW:
		o_newfile(buf) = ElvTrue;
		break;

	  case DIR_READWRITE:
		/* nothing needed */
		break;
	}

	/* Execute the "before read"  script, if it exists.  If the script
	 * fails, then don't load the newly-created buffer.
	 */
	if (!o_internal(buf))
	{
#ifdef FEATURE_CALC /* since it isn't real useful without :if */
		initbuf = buffind(toCHAR(BEFOREREAD_BUF));
		if (initbuf)
		{
			/* make the buffer available to :set */
			bufoptions(buf);

			/* execute the script */
# ifdef FEATURE_MISC
			locals = optlocal(NULL);
# endif
			exctlsave(oldctlstate);
# ifdef FEATURE_AUTOCMD
			(void)auperform(windefault, ElvFalse, NULL, AU_SCRIPTENTER,
				o_filename(initbuf));
# endif
			result = experform(windefault, marktmp(top, initbuf, 0),
				marktmp(end, initbuf, o_bufchars(initbuf)));
# ifdef FEATURE_AUTOCMD
			(void)auperform(windefault, ElvFalse, NULL, AU_SCRIPTLEAVE,
				o_filename(initbuf));
# endif
			exctlrestore(oldctlstate);
# ifdef FEATURE_MISC
			(void)optlocal(locals);
# endif
			if (result != RESULT_COMPLETE)
			{
# ifdef FEATURE_AUTOCMD
				bufnoedit = ElvFalse;
# endif
				return buf;
			}
		}
#endif /* FEATURE_CALC */

#ifdef FEATURE_AUTOCMD
		/* perform the pre-read script */
		(void)auperform(windefault, ElvFalse, NULL,
			o_newfile(buf) ? AU_BUFNEWFILE : AU_BUFREADPRE, NULL);
#endif
	}

	/* read the file's contents into the buffer */
	if (o_newfile(buf))
	{
		msg(bufmsgtype, "[sdd]$1 [NEW FILE]",
			filename, o_buflines(buf), o_bufchars(buf));
	}
	else if (!bufread(marktmp(top, buf, 0), filename))
	{
		o_edited(buf) = ElvFalse;
#ifdef FEATURE_AUTOCMD
		bufnoedit = ElvFalse;
#endif
		return buf;
	}

	/* if it ends with a partial last line, then add a newline */
	(void)marktmp(end, buf, o_bufchars(buf) - 1);
	if (o_bufchars(buf) > 0L && scanchar(&end) != '\n')
	{
		o_partiallastline(buf) = ElvTrue;
		marksetoffset(&end, markoffset(&end) + 1);
		bufreplace(&end, &end, toCHAR("\n"), 1L);
	}
	else
		o_partiallastline(buf) = ElvFalse;

	/* set other options to describe the file */
	o_modified(buf) = ElvFalse;
	optpreset(o_errlines(buf), o_buflines(buf), OPT_HIDE);
#ifdef PROTOCOL_FTP
	if (!strncmp(filename, "ftp:", 4))
		o_readonly(buf) = (ELVBOOL)(ftpperms == DIR_READONLY);
#endif

	/* Restore the marks to their previous offsets.  Otherwise any marks
	 * which refer to this buffer will be set to the end of the file.
	 * Restoring them isn't perfect, but it beats setting them all to EOF!
	 * (Note: New buffers won't have an "undo" version.)
	 */
	if (buf->undo && buf->undo->marklist)
	{
		for (i = 0; buf->undo->marklist[i].mark; i++)
		{
			/* is the mark still in this buffer? */
			for (mark = buf->marks;
			     mark && mark != buf->undo->marklist[i].mark;
			     mark = mark->next)
			{
			}
			if (!mark)
				/* not in this buffer anymore, so free it */
				continue;

			/* Restore its offset */
			marksetoffset(mark, buf->undo->marklist[i].offset);

			/* Some marks, newer than those in the marklist, may
			 * have offsets past the end of the buffer.  Set their
			 * offset to the end of the buffer.
			 */
			for (mark = buf->marks; mark; mark = mark->next)
			{
				if (markoffset(mark) > o_bufchars(buf))
					marksetoffset(mark, o_bufchars(buf));
			}
		}
	}

	/* execute the file initialization script, if it exists */
	if (!o_internal(buf))
	{
#ifndef FEATURE_CALC
		/* Whoa!  Since :if doesn't exist, the normal script won't
		 * work.  Just take a guess at the display mode.
		 */
		if (o_filename(buf))
		{
			char	*ext = strrchr(tochar8(o_filename(buf)), '.');
			bufoptions(buf);
			if (!ext)
				/* do nothing */;
# ifdef DISPLAY_HTML
			else if (!strncmp(ext, ".htm", 4)
			      || !strncmp(ext, ".HTM", 4)
			      || scanchar(marktmp(top, buf, 0)) == '<')
				optputstr(toCHAR("bd"), toCHAR("html"), ElvFalse);
# endif
# ifdef DISPLAY_MAN
			else if (!strcmp(ext, ".man")
			      || (elvdigit(ext[1]) && !ext[2])
			      || scanchar(marktmp(top, buf, 0)) == '.')
				optputstr(toCHAR("bd"), toCHAR("man"), ElvFalse);
# endif
# ifdef DISPLAY_SYNTAX
			else if (descr_known(tochar8(o_filename(buf)), SYNTAX_FILE))
				optputstr(toCHAR("bd"), toCHAR("syntax"), ElvFalse);
# endif
		}
#else
		initbuf = buffind(toCHAR(AFTERREAD_BUF));
		if (initbuf)
		{
			/* make the buffer available to :set */
			bufoptions(buf);

			/* Execute the script's contents. */
# ifdef FEATURE_MISC
			locals = optlocal(NULL);
# endif
			exctlsave(oldctlstate);
# ifdef FEATURE_AUTOCMD
			(void)auperform(windefault, ElvFalse, NULL, AU_SCRIPTENTER,
				o_filename(initbuf));
# endif
			(void)experform(windefault, marktmp(top, initbuf, 0),
				marktmp(end, initbuf, o_bufchars(initbuf)));
# ifdef FEATURE_AUTOCMD
			(void)auperform(windefault, ElvFalse, NULL, AU_SCRIPTLEAVE,
				o_filename(initbuf));
# endif
			exctlrestore(oldctlstate);
# ifdef FEATURE_MISC
			(void)optlocal(locals);
# endif
		}
#endif

#ifdef FEATURE_AUTOCMD
		/* perform the post-read script */
		if (!o_newfile(buf))
			(void)auperform(windefault, ElvFalse, NULL,
				AU_BUFREADPOST, NULL);
#endif
	}

#ifdef FEATURE_SHOWTAG
	/* load the tag definitions for this file */
	tebuilddef(buf);
#endif

	/* set the initial cursor offset to 0 */
	buf->docursor = 0L;

#ifdef FEATURE_AUTOCMD
	/* done loading.  Any later changes may generate Edit events */
	bufnoedit = ElvFalse;
#endif

	return buf;
}


/* This function searches through a path for a file to load.  It then loads
 * that file into a buffer and returns the buffer.  If the file couldn't be
 * located, it returns NULL instead.  If a buffer already exists with the given
 * name, then it returns that buffer without attempting to load anything.
 */
BUFFER bufpath(path, filename, bufname)
	CHAR	*path;		/* path to search through */
	char	*filename;	/* file to search for */
	CHAR	*bufname;	/* name of buffer to store the file */
{
	char	*pathname;	/* full pathname of the loaded file */
	char	pathdup[256];	/* local copy of pathname */
	BUFFER	buf;

	/* if the buffer already exists, return it immediately */
	buf = buffind(bufname);
	if (buf)
	{
		return buf;
	}

	/* try to find the file */
	pathname = iopath(tochar8(path), filename, ElvFalse);
	if (!pathname)
	{
		return (BUFFER)0;
	}

	/* we need a local copy of the pathname, because bufload() will also
	 * call iopath() to find the "elvis.brf" and "elvis.arf" files, and
	 * iopath() only has a single static buffer that it uses for returning
	 * the found pathname.  We don't want our pathname clobbered.
	 */
	strcpy(pathdup, pathname);

	/* load the file */
	buf = bufload(bufname, pathdup, ElvTrue);
	return buf;
}


/* This function deletes the oldest undo versions of a given buffer */
static void freeundo(buffer)
	BUFFER	buffer;	/* buffer to be cleaned */
{
	struct undo_s *undo, *other;
	int	      i;

	checkundo("before freundo()");

	/* locate the most recent doomed version */
	i = o_undolevels(buffer);
	if (i < 1) i++;
	for (other = NULL, undo = buffer->undo;
	     i > 0 && undo;
	     i--, other = undo, undo = undo->next)
	{
	}

	/* if none are doomed, return now */
	if (!undo)
	{
		return;
	}

	/* Remove the most recent doomed version (and all following versions)
	 * from the linked list of undo versions.
	 */
	if (other)
	{
		other->next = NULL;
	}
	else
	{
		buffer->undo = NULL;
	}

	/* delete each doomed version */
	for (; undo; undo = other)
	{
		other = undo->next;
		/* free the lowbuf and the (struct undo_s) structure */
		lowfree(undo->bufinfo);
#ifdef DEBUG_ALLOC
		removeundo(undo);
#endif
		safefree(undo->marklist);
		safefree(undo);
	}
	checkundo("after freeundo");
}



/* Free a buffer which was created via bufalloc(). */
void buffree(buffer)
	BUFFER	buffer;	/* buffer to be destroyed */
{
	BUFFER	scan, lag;
	struct undo_s *undo;

	assert(buffer != bufdefopts);
	checkundo("before buffree");

	/* if any window is editing this buffer, then fail */
	if (wincount(buffer) > 0)
	{
		return;
	}

#ifdef FEATURE_AUTOCMD
	if (!o_internal(buffer))
	{
		BUFFER oldbuf = bufdefault;
		bufoptions(buffer);
		auperform(windefault, ElvFalse, NULL, AU_BUFUNLOAD, o_filename(buffer));
		auperform(windefault, ElvFalse, NULL, AU_BUFDELETE, o_filename(buffer));
		bufoptions(oldbuf);
	}
#endif /* FEATURE_AUTOCMD */

	/* transfer any marks to the dummy "bufdefopts" buffer */
	while (buffer->marks)
	{
		marksetoffset(buffer->marks, 0L);
		marksetbuffer(buffer->marks, bufdefopts);
	}

#ifdef FEATURE_SHOWTAG
	/* free the array of tag definitions */
	tefreedef(buffer);
#endif

	/* free any undo/redo versions of this buffer */
	while (buffer->undo)
	{
		undo = buffer->undo;
		buffer->undo = undo->next;
		lowfree(undo->bufinfo);
#ifdef DEBUG_ALLOC
		removeundo(undo);
#endif
		safefree(undo->marklist);
		safefree(undo);
	}
	while (buffer->redo)
	{
		undo = buffer->redo;
		buffer->redo = undo->next;
		lowfree(undo->bufinfo);
#ifdef DEBUG_ALLOC
		removeundo(undo);
#endif
		safefree(undo->marklist);
		safefree(undo);
	}
	if (buffer->undolnptr)
	{
		lowfree(buffer->undolnptr->bufinfo);
#ifdef DEBUG_ALLOC
		removeundo(buffer->undolnptr);
#endif
		safefree(buffer->undolnptr->marklist);
		safefree(buffer->undolnptr);
		buffer->undolnptr = NULL;
	}
	assert(buffer->undo == NULL && buffer->redo == NULL);

	/* locate the buffer in the linked list */
	for (lag = NULL, scan = elvis_buffers; scan != buffer; lag = scan, scan = scan->next)
	{
		assert(scan->next);
	}

	/* remove it from the linked list */
	if (lag)
	{
		lag->next = scan->next;
	}
	else
	{
		elvis_buffers = scan->next;
	}

	/* free the values of any string options which have been set */
	optfree(QTY(bdesc), &buffer->filename);

#if 0 /* this is pointless, since we already transfered marks to bufdefopts */
	/* free any marks in this buffer */
	while (buffer->marks)
	{
		markfree(buffer->marks);
	}
#endif

	/* free the low-level block */
	lowfree(buffer->bufinfo);

	/* free the buffer struct itself */
	safefree(buffer);
	checkundo("after buffree");
}


/* Free a buffer, if possible without losing anything important.  Return
 * ElvTrue if freed, ElvFalse if retained.  If "force" is ElvTrue, it tries
 * harder.
 */
ELVBOOL bufunload(buf, force, save)
	BUFFER	buf;	/* buffer to be unloaded */
	ELVBOOL	force;	/* if ElvTrue, try harder */
	ELVBOOL	save;	/* if ElvTrue, maybe save even if noautowrite */
{
	MARKBUF	top, bottom;

	/* if "internal" then retain it for now */
	if (o_internal(buf))
	{
		return ElvFalse;
	}

	/* if being used by some window, then retain it */
	if (wincount(buf) > 0)
	{
		return ElvFalse;
	}

	/* If supposed to retain, then keep it unless "force" is ElvTrue.
	 * If this is a temporary session, then no buffers will be retained,
	 * so we should fail in that situation too.
	 */
	if (o_retain(buf) && !force && !o_tempsession)
	{
		return ElvFalse;
	}

	/* if not modified, then discard it */
	if (!o_modified(buf))
	{
		buffree(buf);
		return ElvTrue;
	}

	/* if readonly, or no known filename then free it if "force" or
	 * retain it if not "force"
	 */
	if (o_readonly(buf) || o_filename(buf) == NULL)
	{
		if (force)
		{
			buffree(buf);
		}
		return force;
	}

	/* Try to save the buffer to a file */
	if (save && o_filename(buf)
	 && bufwrite(marktmp(top, buf, 0), marktmp(bottom, buf, o_bufchars(buf)), tochar8(o_filename(buf)), force))
	{
		buffree(buf);
		return ElvTrue;
	}
	return ElvFalse;
}


/* Return ElvTrue if "buf" can be deleted without loosing data, or ElvFalse if
 * it can't -- in which case it also emits a message describing why.  NOTE THAT
 * YOU ALSO NEED TO MAKE SURE THE BUFFER ISN'T BEING EDITED IN A WINDOW.
 *
 * This function has side-effects.  If the buffer has been modified, it may
 * try to write the buffer out to a file.
 */
ELVBOOL bufsave(buf, force, mustwr)
	BUFFER	buf;	/* the buffer to write */
	ELVBOOL	force;	/* passed to bufwrite() if writing is necessary */
	ELVBOOL	mustwr;	/* write to file even if buffer isn't modified */
{
	MARKBUF	top, bottom;

	/* Can never "save" or delete the internal buffers */
	if (o_internal(buf))
	{
		msg(MSG_ERROR, "[s]$1 is used internally by elvis", o_bufname(buf));
		return ElvFalse;
	}

	/* If writing wasn't explicitly demanded, and isn't needed, then
	 * return ElvTrue without doing anything else.
	 */
	if (!mustwr && !o_modified(buf))
	{
		return ElvTrue;
	}

	/* We know that we need to write this buffer.  If it has no filename
	 * then we can't write it.
	 */
	if (!o_filename(buf))
	{
		msg(MSG_ERROR, "[S]no file name for $1", o_bufname(buf));
		return ElvFalse;
	}

	/* try to write the buffer out to its file */
	return bufwrite(marktmp(top, buf, 0),
			marktmp(bottom, buf, o_bufchars(buf)),
			tochar8(o_filename(buf)), force);
}



/* Write a buffer, or part of a buffer, out to a file.  Return ElvTrue if
 * successful, or ElvFalse if error.
 */
ELVBOOL bufwrite(from, to, wfile, force)
	MARK	from;	/* start of text to write */
	MARK	to;	/* end of text to write */
	char	*wfile;	/* output file, ">>file" to append, "!cmd" to filter */
	ELVBOOL	force;	/* write even if file already exists */
{
	BUFFER	buf = markbuffer(from);
	MARKBUF	next;		/* used for determining append location */
	CHAR	*cp;		/* used for scanning through file */
	ELVBOOL	append;		/* If ElvTrue, we're appending */
	ELVBOOL	filter;		/* If ElvTrue, we're writing to a filter */
	ELVBOOL	wholebuf;	/* if ElvTrue, we're writing the whole buffer */
	ELVBOOL	samefile;	/* If ElvTrue, we're writing the buffer to its original file */
	int	bytes;
	long	lines;
#if defined(FEATURE_CALC) || defined(FEATURE_AUTOCMD)
	RESULT	result;
#endif
#ifdef FEATURE_CALC
	BUFFER	initbuf;	/* one of the file initialization buffers */
	MARKBUF	top;		/* the endpoints of initbuf */
	EXCTLSTATE oldctlstate;
# ifdef FEATURE_MISC
	void	*locals;
# endif

	static OPTDESC	bangdesc[1] = { {"bang", "bang", NULL, NULL} };
	OPTVAL	bangval[1];
#endif

	assert(from && to && wfile);
	assert(markbuffer(from) == markbuffer(to));
	assert(markoffset(from) >= 0);
	assert(markoffset(from) <= markoffset(to));
	assert(markoffset(to) <= o_bufchars(buf));

#ifdef FEATURE_PROTO
	switch (urlalias(from, to, wfile))
	{
	  case RESULT_COMPLETE:	return ElvTrue;
	  case RESULT_ERROR:	return ElvFalse;
	  case RESULT_MORE:	; /* fall through to normal code */
	}
#endif /* FEATURE_PROTO */

	/* Determine some characteristics of this write */
	append = (ELVBOOL)(wfile[0] == '>' && wfile[1] == '>');
	filter = (ELVBOOL)(wfile[0] == '!');
	wholebuf = (ELVBOOL)(markoffset(from) == 0 && markoffset(to) == o_bufchars(buf));
	samefile = (ELVBOOL)(o_filename(buf) && !strcmp(wfile, tochar8(o_filename(buf))));

	/* if we're appending, skip the initial ">>" */
	if (append)
	{
		for (wfile += 2; elvspace(*wfile); wfile++)
		{
		}
	}

	/* If writing to the same file, and it is a readonly file, then fail
	 * unless we're forcing a write.
	 */
	if (!filter && wholebuf && samefile && o_readonly(buf) && !force)
	{
		msg(MSG_ERROR, "[s]$1 readonly" , wfile);
		return ElvFalse;
	}

	/* If this is supposed to be a new file, or we're writing to
	 * a name other than what was originally loaded, and we aren't
	 * forcing a write, then make sure the file doesn't already
	 * exist.
	 */
	if ((o_newfile(buf) || !o_edited(buf) || !samefile || !wholebuf)
	  && !force
	  && !append
	  && !filter
	  && urlperm(wfile) != DIR_NEW)
	{
		msg(MSG_ERROR, "[s]$1 exists", wfile);
		return ElvFalse;
	}

	/* If we're writing the whole file back over itself, then execute the
	 * "before write" script if it exists.  If this fails and "force" isn't
	 * true, then fail.
	 */
	if (wholebuf && samefile && !filter && !append)
	{
#ifdef FEATURE_CALC /* since it isn't real useful without :if */
		initbuf = buffind(toCHAR(BEFOREWRITE_BUF));
		if (initbuf)
		{
			/* make the buffer be the default buffer */
			bufoptions(buf);
			bangval[0].value.boolean = force;
			optinsert("bang", QTY(bangdesc), bangdesc, bangval);

			/* execute the script */
# ifdef FEATURE_MISC
			locals = optlocal(NULL);
# endif
			exctlsave(oldctlstate);
# ifdef FEATURE_AUTOCMD
			(void)auperform(windefault, ElvFalse, NULL, AU_SCRIPTENTER,
				o_filename(initbuf));
# endif
			result = experform(windefault, marktmp(top, initbuf, 0),
				marktmp(next, initbuf, o_bufchars(initbuf)));
# ifdef FEATURE_AUTOCMD
			(void)auperform(windefault, ElvFalse, NULL, AU_SCRIPTLEAVE,
				o_filename(initbuf));
# endif
			exctlrestore(oldctlstate);
# ifdef FEATURE_MISC
			(void)optlocal(locals);
# endif
			optdelete(bangval);
			if (result != RESULT_COMPLETE && !force)
				return ElvFalse;
		}
#endif /* FEATURE_CALC */

#ifdef FEATURE_AUTOCMD
		if (auperform(windefault, force, NULL, AU_BUFWRITEPRE,
				NULL) != RESULT_COMPLETE)
			return ElvFalse;
	}
	else
	{
		autop = markdup(from);
		aubottom = markdup(to);
		markaddoffset(aubottom, -1L); /* end of last line */
		result = auperform(windefault, force, NULL,
				filter ? AU_FILTERWRITEPRE
				       : append ? AU_FILEAPPENDPRE
						: AU_FILEWRITEPRE,
				toCHAR(wfile));
		markfree(aubottom);
		markfree(autop);
		autop = aubottom = NULL;
		if (result != RESULT_COMPLETE)
			return ElvFalse;
#endif
	}

	/* If "partiallastline" is set (indicating that the original file
	 * wasn't terminated with a newline so elvis added one) then we
	 * probably don't want to write that newline.  However, if this is a
	 * text-mode write or the added newline isn't there anymore then we
	 * want to write the whole file.
	 */
	if (wholebuf)
	{
		next = *to;
		marksetoffset(&next, markoffset(&next) - 1);
		if (!o_partiallastline(buf)
		 || (o_writeeol != 'b' && o_readeol(markbuffer(from)) != 'b')
		 || o_bufchars(markbuffer(from)) == 0
		 || scanchar(&next) != '\n')
			o_partiallastline(markbuffer(from)) = ElvFalse;
		else
			marksetoffset(to, markoffset(&next));
	}

	/* Try to write the file */
	if (ioopen(wfile, append ? 'a' : 'w', ElvFalse, ElvTrue,
		o_writeeol == 's' || o_readeol(markbuffer(from)) == 'b'
			? o_readeol(markbuffer(from))
			: o_writeeol))
	{
		if (wholebuf && !filter)
		{
			msg(MSG_STATUS, "[s]writing $1", wfile);
		}
		next = *from;
		if (o_bufchars(markbuffer(from)) > 0L)
		{
			scanalloc(&cp, &next);
			assert(cp);
			bytes = 1;
			do
			{
				/* check for ^C */
				if (guipoll(ElvFalse))
				{
					ioclose();
					scanfree(&cp);
					return ElvFalse;
				}

				bytes = scanright(&cp);
				if (markoffset(&next) + bytes > markoffset(to))
				{
					bytes = (int)(markoffset(to) - markoffset(&next));
				}
				if (iowrite(cp, bytes) < bytes)
				{
					msg(MSG_ERROR, (wfile[0] == '!') ? "broken pipe" : "disk full");
					ioclose();
					scanfree(&cp);
					return ElvFalse;
				}
				markaddoffset(&next, bytes);
				scanseek(&cp, &next);
			} while (cp != NULL && markoffset(&next) < markoffset(to));
			scanfree(&cp);
		}
		ioclose();

		if (!filter)
		{
			lines = markline(to) - markline(from);
			if (append)
				msg(bufmsgtype, "[ds]appended $1 lines to $2",
					lines, wfile);
			else if (o_report != 0 && lines >= o_report)
				msg(bufmsgtype, "[sdd]wrote $1, $2 lines, $3 chars",
					wfile, lines,
					markoffset(to) - markoffset(from));
		}
	}
	else
	{
		/* We had an error, and already wrote the error message */
		return ElvFalse;
	}

	/* Execute the "after write" script. */
	if (wholebuf && samefile && !filter && !append)
	{
#ifdef FEATURE_CALC /* since it isn't real useful without :if */
		initbuf = buffind(toCHAR(AFTERWRITE_BUF));
		if (initbuf)
		{
			/* make the buffer be the default buffer */
			bufoptions(buf);
			bangval[0].value.boolean = force;
			optinsert("bang", QTY(bangdesc), bangdesc, bangval);

			/* execute the script */
# ifdef FEATURE_MISC
			locals = optlocal(NULL);
# endif
			exctlsave(oldctlstate);
# ifdef FEATURE_AUTOCMD
			(void)auperform(windefault, ElvFalse, NULL, AU_SCRIPTENTER,
				o_filename(initbuf));
# endif
			(void)experform(windefault, marktmp(top, initbuf, 0),
				marktmp(next, initbuf, o_bufchars(initbuf)));
# ifdef FEATURE_AUTOCMD
			(void)auperform(windefault, ElvFalse, NULL, AU_SCRIPTLEAVE,
				o_filename(initbuf));
# endif
			exctlrestore(oldctlstate);
# ifdef FEATURE_MISC
			(void)optlocal(locals);
# endif
			optdelete(bangval);
		}
#endif /* FEATURE_CALC */

#ifdef FEATURE_AUTOCMD
		(void)auperform(windefault, force, NULL, AU_BUFWRITEPOST, NULL);
	}
	else
	{
		(void)auperform(windefault, force, NULL,
			filter ? AU_FILTERWRITEPOST
			       : append ? AU_FILEAPPENDPOST
			                : AU_FILEWRITEPOST,
			toCHAR(wfile));
#endif
	}

	/* Writing the whole file has some side-effects on options */
	if (wholebuf && !append && !filter)
	{
		/* if it had no filename before, it has one now */
		if (!o_filename(buf) && !o_internal(buf) && wholebuf)
		{
			o_filename(buf) = CHARdup(toCHAR(wfile));
			optflags(o_filename(buf)) |= OPT_FREE;
			buftitle(buf, toCHAR(ioabsolute(wfile)));
		}

		/* buffer is no longer modified */
		o_modified(buf) = ElvFalse;
		o_newfile(buf) = ElvFalse;
		
		/* if the original file was overwritten, then reset the
		 * readonly flag because apparently the file isn't readonly.
		 */
		if (o_filename(buf) && !CHARcmp(o_filename(buf), toCHAR(wfile)))
		{
			o_readonly(buf) = ElvFalse;
			o_edited(buf) = ElvTrue;
		}
	}

	/* success! */
	return ElvTrue;
}


/* Make "buf" be the default buffer.  The default buffer is the one whose
 * options are available to the :set command.
 */
void bufoptions(buf)
	BUFFER	buf;	/* the buffer to become the new default buffer */
{
	/* if same as before, then do nothing */
	if (buf == bufdefault)
	{
		return;
	}

	/* if there is a previous buffer, then delete its options */
	if (bufdefault)
	{
#ifdef FEATURE_AUTOCMD
		(void)auperform(windefault, ElvFalse, NULL, AU_BUFLEAVE, NULL);
#endif
		optdelete(&bufdefault->filename);
	}

	/* make this buffer be the default */
	bufdefault = buf;

	/* if bufdefault is not NULL, then insert its options */
	if (buf)
	{
		optinsert("buf", QTY(bdesc), bdesc, &buf->filename);
#ifdef FEATURE_AUTOCMD
		(void)auperform(windefault, ElvFalse, NULL, AU_BUFENTER, NULL);
#endif
	}
}


/* This function changes the name of a buffer.  If the buffer happens to
 * be the main buffer of one or more windows, then it will also retitle
 * those windows.
 */
void buftitle(buffer, title)
	BUFFER	buffer;	/* the buffer whose name is to be changed */
	CHAR	*title;	/* the new name of the buffer */
{
	WINDOW	win;
#ifdef FEATURE_AUTOCMD
	BUFFER	oldbuf = bufdefault;
#endif

	/* Make a local copy of the title */
	title = CHARkdup(title);

	/* if another buffer already has this name, then fail silently */
	if (buffind(title))
	{
		safefree(title);
		return;
	}

#ifdef FEATURE_AUTOCMD
	bufoptions(buffer);
	(void)auperform(windefault, ElvFalse, NULL, AU_BUFFILEPRE, title);
#endif

	/* change the name on disk */
	lowtitle(buffer->bufinfo, title);

	/* change the name in RAM */
	safefree(o_bufname(buffer));
	o_bufname(buffer) = title;

	/* change the window titles, if the gui supports that */
	if (gui->retitle)
	{
		for (win = winofbuf((WINDOW)0, buffer);
		     win;
		     win = winofbuf(win, buffer))
		{
			(*gui->retitle)(win->gw, tochar8(o_filename(buffer)
						 ? o_filename(buffer)
						 : o_bufname(buffer)));
		}
	}

#ifdef FEATURE_AUTOCMD
	(void)auperform(windefault, ElvFalse, NULL, AU_BUFFILEPOST, title);
	bufoptions(oldbuf);
#endif
}


/* Set the buffer's flag that will eventually cause an undo version of to
 * be saved.
 */
void bufwilldo(cursor, will)
	MARK	cursor;	/* where to put cursor if we return to this "undo" version */
	ELVBOOL	will;	/* ElvTrue to set flag, ElvFalse to merely remember cursor */
{
	if (will && markbuffer(cursor)->willevent != eventcounter)
	{
		markbuffer(cursor)->willdo = ElvTrue;
		markbuffer(cursor)->willevent = eventcounter;
	}
	markbuffer(cursor)->docursor = markoffset(cursor);
}


/* allocate an "undo" version for a buffer, but don't insert it into the
 * buffer's undo list.  This function is called only from the bufdo() function,
 * which handles any other processing that may be necessary.
 */
static struct undo_s *allocundo(buf)
	BUFFER	buf;
{
	struct undo_s *undo;
	int	i;
	MARK	mark;

	/* allocate a structure */
	undo = (struct undo_s *)safealloc(1, sizeof *undo);
#ifdef DEBUG_ALLOC
	undo->link1 = undohead;
	undohead = undo;
	undo->link2 = NULL;
	if (undo->link1)
		undo->link1->link2 = undo;
	else
		undotail = undo;
	undo->buf = buf;
	undo->undoredo = 'u';
#endif

	/* fill it in */
	undo->changepos = buf->changepos = buf->docursor;
	undo->buflines = o_buflines(buf);
	undo->bufchars = o_bufchars(buf);
	undo->bufinfo = lowdup(buf->bufinfo);
	undo->next = NULL;

	/* the undo->marklist field is tricky.  First step is to count the marks
	 * in this buffer.  Then allocate an array that large plus 1 for an
	 * end marker.  Then fill it in.
	 */
	for (mark = buf->marks, i = 0; mark; mark = mark->next, i++)
	{
	}
	undo->marklist = (struct umark_s *)safealloc(i + 1, sizeof(struct umark_s));
	for (mark = buf->marks, i = 0; mark; mark = mark->next, i++)
	{
		undo->marklist[i].mark = mark;
		undo->marklist[i].offset = markoffset(mark);
	}
	undo->marklist[i].mark = NULL;

	/* return it */
	return undo;
}


/* Save an "undo" version of a buffer */
static void bufdo(buf, wipe)
	BUFFER	buf;	/* buffer to make an "undo" version for */
	ELVBOOL	wipe;	/* if ElvTrue, then delete all "redo" versions */
{
	struct undo_s	*undo;
	long		linenum;

	checkundo("before bufdo");

	/* never save an undo version of an internal buffer */
	if (o_internal(buf))
		return;

	/* allocate an undo structure and fill it in */
	undo = allocundo(buf);

	/* insert it into the buffer's "undo" list */
	undo->next = buf->undo;
	buf->undo = undo;

	checkundo("in bufdo, before changing undolnptr");

	/* If this is on a different line from previous change, then allocate
	 * another undo structure to use as the line-undo version.
	 */
	(void)lowoffset(undo->bufinfo, undo->changepos,
		(COUNT *)0, (COUNT *)0, (LBLKNO *)0, &linenum);
	if (linenum != buf->undoline)
	{
		/* free the old line-undo version, if any */
		if (buf->undolnptr)
		{
			lowfree(buf->undolnptr->bufinfo);
#ifdef DEBUG_ALLOC
			removeundo(buf->undolnptr);
#endif
			safefree(buf->undolnptr->marklist);
			safefree(buf->undolnptr);
		}

		/* allocate & store the new line-undo version */
		buf->undolnptr = allocundo(buf);
		buf->undoline = linenum;
#ifdef DEBUG_ALLOC
		buf->undolnptr->undoredo = 'l';
#endif
	}

	checkundo("in bufdo, after changing undolnptr");

	/* discard the redo versions, if we're supposed to */
	if (wipe)
	{
		while (buf->redo && buf->redo != buf->undolnptr)
		{
			undo = buf->redo;
			buf->redo = undo->next;
			lowfree(undo->bufinfo);
#ifdef DEBUG_ALLOC
			removeundo(undo);
#endif
			safefree(undo->marklist);
			safefree(undo);
		}

		checkundo("in bufdo, after wiping the redo versions");
	}

	/* discard the oldest version[s] from the undo list */
	freeundo(buf);

	/* make sure this is written to disk */
	if (!o_internal(buf) && eventcounter > 2)
		sessync();
}

/* Set the "modified" option for a buffer, unless it is internal */
static void didmodify(buf)
	BUFFER	buf;	/* a buffer to mark as "modified", unless internal */
{
	/* if internal or already modified, do nothing */
	if (o_internal(buf) || o_modified(buf))
		return;

	/* set the modified flag */
	o_modified(buf) = ElvTrue;

#ifdef FEATURE_AUTOCMD
	/* do OptSet and OptChanged events on the "modified" option */
	optautocmd("modified", NULL, &buf->modified);
#endif
}

/* Recall a previous "undo" version of a given buffer.  The "back" argument
 * should be positive to undo, negative to redo, or 0 to undo all changes
 * to the current line.  Returns the cursor offset if successful, or -1 if
 * the requested undo version doesn't exist.
 */
long bufundo(cursor, back)
	MARK	cursor;	/* buffer to be undone, plus cursor offset of current version */
	long	back;	/* number of versions back (negative to redo) */
{
	struct undo_s	*undo;
	struct undo_s	*tmp;
	long		i;
	BUFFER		buffer;
	MARKBUF		from, to;
	MARK		mark;
	long		delta;
	long		origulev;

	checkundo("before bufundo");

	/* locate the desired undo version */
	buffer = markbuffer(cursor);
	if (back == 0)
	{
		/* line undo */
		undo = buffer->undolnptr;
	}
	else if (o_undolevels(buffer) == 0)
	{
		/* Can only oscillate between previous version and this one,
		 * but it has the advantage that <u> and <^R> both do exactly
		 * the same thing.
		 */
		if (buffer->redo)
			undo = buffer->redo, back = -1;
		else
			undo = buffer->undo, back = 1;
	}
	else if (back > 0)
	{
		/* undo */
		for (i = back, undo = buffer->undo; undo && i > 1; i--, undo = undo->next)
		{
		}
	}
	else
	{
		/* redo */
		for (i = -back, undo = buffer->redo; undo && i > 1; i--, undo = undo->next)
		{
		}
	}

	/* if the requested version doesn't exist, then fail */
	if (!undo)
	{
		return -1;
	}

	/* save the current version as either an undo version or a redo version,
	 * so we can revert to it.  Note that we increase the number of undo
	 * levels temporarily, so the oldest undo version won't be discarded
	 * yet.
	 */
	origulev = o_undolevels(buffer);
	o_undolevels(buffer)++;
	if (o_undolevels(buffer) < 2)
		o_undolevels(buffer) = 2;
	bufwilldo(marktmp(from, buffer, undo->changepos), ElvTrue);
	bufdo(buffer, ElvFalse);
	if (back > 0)
	{
		/* undoing: move from "undo" to "redo" */
		while (buffer->undo != undo)
		{
			assert(buffer->undo);
			tmp = buffer->undo;
			buffer->undo = tmp->next;
			tmp->next = buffer->redo;
			buffer->redo = tmp;
#ifdef DEBUG_ALLOC
			tmp->undoredo = 'r';
#endif
		}

		/* remove the selected version from the "undo" list */
		buffer->undo = undo->next;
	}
	else if (back < 0)
	{
		/* redoing: move from "redo" to "undo" */
		while (buffer->redo != undo)
		{
			assert(buffer->redo);
			tmp = buffer->redo;
			buffer->redo = tmp->next;
			tmp->next = buffer->undo;
			buffer->undo = tmp;
#ifdef DEBUG_ALLOC
			tmp->undoredo = 'u';
#endif
		}

		/* remove the selected version from the "redo" list */
		buffer->redo = undo->next;
	}
	else
	{
		/* line-undo: Remove the selected undo version from the
		 * undolnptr pointer.
		 */
		buffer->undolnptr = NULL;
		buffer->undoline = 0;
	}

	/* replace the current version with the selected undo version */
	lowfree(buffer->bufinfo);
	buffer->bufinfo = undo->bufinfo;
	delta = undo->bufchars - o_bufchars(buffer);
	buffer->changepos = undo->changepos;
	o_buflines(buffer) = undo->buflines;
	o_bufchars(buffer) = undo->bufchars;
	buffer->changes++;
	didmodify(buffer);
	/*!!! But since internal buffers never store any undo versions, if
	 * we get here we know that o_internal(buffer) is ElvFalse.
	 */

#ifdef FEATURE_REGION
	/* mark regions to be kept after the Undo */
	regionundo(buffer, undo->marklist);
#endif

	/* Adjust the values of any marks.  Most marks can be fixed just by
	 * calling markadjust().
	 */
	if (delta < 0)
	{
		markadjust(marktmp(from, buffer, undo->changepos),
			marktmp(to, buffer, undo->changepos - delta),
			delta);
	}
	else
	{
		markadjust(marktmp(from, buffer, undo->changepos), &from,delta);
	}

	/* For any marks which happened to point to this buffer when the undo
	 * state was saved, we can restore them exactly.
	 */
	for (i = 0; undo->marklist[i].mark; i++)
	{
		for (mark = buffer->marks;
		     mark && undo->marklist[i].mark != mark;
		     mark = mark->next)
		{
		}
		if (mark)
			marksetoffset(mark, undo->marklist[i].offset);
	}
#ifdef DISPLAY_ANYMARKUP
	dmmuadjust(marktmp(from, buffer, 0), marktmp(to, buffer, o_bufchars(buffer)), 0);
#endif
#ifdef FEATURE_REGION
# ifdef DEBUG_REGION
	regionundo(buffer, NULL);
# endif
#endif

	/* We can free the undo_s structure now. */
#ifdef DEBUG_ALLOC
	removeundo(undo);
#endif
	safefree(undo->marklist);
	safefree(undo);

	/* Okay to free the oldest "undo" version now */
	o_undolevels(buffer) = origulev;
	freeundo(buffer);	/* Is this really necessary? */

	/* Return the offset of the last change, so the cursor can be moved
	 * there.  Never return a point past the end of the buffer, though.
	 */
	checkundo("after bufundo");
	if (o_bufchars(buffer) == 0)
		buffer->changepos = 0;
	else if (buffer->changepos >= o_bufchars(buffer))
		buffer->changepos = o_bufchars(buffer) - 1;
	return buffer->changepos;
}

/* This function replaces part of a buffer with new text.  In addition to
 * replacement, it can also be used to implement insertion (by having "from"
 * and "to" be identical) or deletion (by having "newlen" be zero).
 *
 * It uses markadjust() to automatically update marks.  If the buffer's
 * "willdo" flag is set, then it will automatically create an "undo" version
 * of the buffer before making the change.
 */
void bufreplace(from, to, newp, newlen)
	MARK	from;	/* starting position of old text */
	MARK	to;	/* ending position of old text */
	CHAR	*newp;	/* pointer to new text (in RAM) */
	long	newlen;	/* length of new text */
{
	long	chglines;
	long	chgchars;
#ifdef FEATURE_AUTOCMD
	ELVBOOL	doing = markbuffer(from)->willdo;
#endif

	assert(markbuffer(from) == markbuffer(to) && newlen >= 0);

	/* if the destination's "willdo" flag is set, then save an "undo"
	 * version of it before doing the change
	 */
	if (markbuffer(from)->willdo)
	{
		bufdo(markbuffer(from), ElvTrue);
		markbuffer(from)->willdo = ElvFalse;
	}

#ifdef FEATURE_FOLD
	/* destroy any FOLDs whose endpoint is going to go away */
	if (markoffset(from) != markoffset(to))
		foldedit(from, to, NULL);
#endif

#ifdef FEATURE_AUTOCMD
	/* After the change, we'll want to perform an Edit autocmd on the
	 * altered text.  Save duplicates of the marks.  We use duplicates
	 * so they'll be adjusted automatically in response to the changes.
	 */
	if (!o_internal(markbuffer(from))
	 && !bufnoedit
	 && (doing || markbuffer(from)->eachedit))
	{
		autop = markdup(from);
		aubottom = markdup(to);
	}
#endif

	/* make the change to the buffer contents */
	if (markoffset(from) == markoffset(to))
	{
		/* maybe we aren't really changing anything? */
		if (newlen == 0)
		{
			return;
		}

		/* we're inserting */
		chglines = lowinsert(markbuffer(from)->bufinfo, markoffset(from), newp, newlen);
	}
	else if (newlen == 0)
	{
		/* we're deleting */
		chglines = lowdelete(markbuffer(from)->bufinfo, markoffset(from), markoffset(to));
	}
	else
	{
		/* we're replacing */
		chglines = lowreplace(markbuffer(from)->bufinfo, markoffset(from), markoffset(to), newp, newlen);
	}

	/* adjust the buffer totals */
	chgchars = newlen - (markoffset(to) - markoffset(from));
	o_buflines(markbuffer(from)) += chglines;
	o_bufchars(markbuffer(from)) += chgchars;
	markbuffer(from)->changes++;

	/* adjust the marks */
#ifdef DISPLAY_ANYMARKUP
	dmmuadjust(from, to, chgchars);
#endif
	markadjust(from, to, chgchars);

#ifdef FEATURE_AUTOCMD
	/* allow an autocmd to update regions */
	if (!o_internal(markbuffer(from))
	 && !bufnoedit
	 && (doing || markbuffer(from)->eachedit))
	{
		auperform(windefault, ElvFalse, NULL, AU_EDIT, NULL);
		markfree(autop);
		autop = NULL;
		markfree(aubottom);
		aubottom = NULL;
	}
#endif

	/* mark it as modified.  Note that we do this after autocmds, so the
	 * autocmd can detect whether this is the *FIRST* change to the buffer.
	 */
	didmodify(markbuffer(from));
}

/* Copy part of one buffer into another.  "dst" is the destination (where
 * the pasted text will be inserted), and "from" and "to" describe the
 * portion of the source buffer to insert.
 *
 * This calls markadjust() to automatically adjust marks.  If the destination
 * buffer's "willdo" flag is set, it will save an "undo" version before
 * making the change.
 */
void bufpaste(dst, from, to)
	MARK	dst;	/* destination */
	MARK	from;	/* start of source */
	MARK	to;	/* end of source */
{
	long	chglines;
	long	chgchars;
#ifdef FEATURE_AUTOCMD
	long	dstoffset = markoffset(dst);
	ELVBOOL	doing = markbuffer(dst)->willdo;
#endif

	assert(markbuffer(from) == markbuffer(to));

	/* if the destination's "willdo" flag is set, then save an "undo"
	 * version of it before doing the paste
	 */
	if (markbuffer(dst)->willdo)
	{
		bufdo(markbuffer(dst), ElvTrue);
		markbuffer(dst)->willdo = ElvFalse;
	}

	/* make the change to the buffer's contents */
	chglines = lowpaste(markbuffer(dst)->bufinfo, markoffset(dst),
		markbuffer(from)->bufinfo, markoffset(from), markoffset(to));

	/* adjust the destination's counters */
	chgchars = markoffset(to) - markoffset(from);
	o_buflines(markbuffer(dst)) += chglines;
	o_bufchars(markbuffer(dst)) += chgchars;
	markbuffer(dst)->changes++;

	/* adjust marks */
	markadjust(dst, dst, chgchars);

#ifdef FEATURE_FOLD
	/* duplicate any folds in the copied region */
	foldedit(from, to, dst);
#endif

#ifdef FEATURE_AUTOCMD
	/* do an Edit event on the destination */
	if (!o_internal(markbuffer(dst))
	 && !bufnoedit
	 && (doing || markbuffer(dst)->eachedit))
	{
		autop = markalloc(markbuffer(dst), dstoffset);
		aubottom = markalloc(markbuffer(dst), dstoffset + chgchars - 1);
		auperform(windefault, ElvFalse, NULL, AU_EDIT, NULL);
		markfree(autop);
		markfree(aubottom);
		autop = aubottom = NULL;
	}
#endif

	/* mark it as modified.  Note that we do this after the autocmds so an
	 * autocmd line can detect whether this is the *FIRST* change.
	 */
	didmodify(markbuffer(dst));
}


/* Copy a section of some buffer into dynamically-allocated RAM, and append
 * a NUL to the end of the copy.  The calling function must call safefree()
 * on the returned memory.
 */
CHAR *bufmemory(from, to)
	MARK	from, to;	/* the section of the buffer to fetch */
{
	CHAR	*memory;	/* the allocated memory */
	CHAR	*scan, *build;	/* used for copying text from buffer to memory */
	long	i;

	assert(markbuffer(from) == markbuffer(to)
		&& markoffset(from) <= markoffset(to));

	/* allocate space for the copy */
	i = markoffset(to) - markoffset(from);
	memory = (CHAR *)safealloc((int)(i + 1), sizeof(CHAR));

	/* copy the text into it */
	for (scanalloc(&scan, from), build = memory; i > 0; scannext(&scan), i--)
	{
		assert(scan);
		*build++ = *scan;
	}
	scanfree(&scan);
	*build = '\0';

	return memory;
}
