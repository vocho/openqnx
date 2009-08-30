/* autocmd.c */
/* Copyright 2001 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_autocmd[] = "$Id: autocmd.c,v 1.30 2003/10/19 23:13:33 steve Exp $";
#endif

#ifdef FEATURE_AUTOCMD

typedef struct
{
	long	fileevents;
	long	otherevents;
	long	userevents;
} aubits_t;

/* This is used for storing individual autocmds */
typedef struct au_s
{
	struct au_s	*next;
	aubits_t	bits;	/* which events trigger this command */
	CHAR		*ptrn;	/* filename pattern */
	CHAR		*excmd;	/* command line to run */
	ELVBOOL		busy;	/* don't nest or delete it */
} au_t;


/* This is used for storing the autocmd groups */
typedef struct aug_s
{
	struct aug_s	*next;
	CHAR		*group;	/* name of the group */
	au_t		*au;	/* list of autocmds in the group */
	ELVBOOL		internal;/* if True, don't save in .exrc file */
} aug_t;


/* These variables are used for storing the '[ and ]' marks, which are used
 * in some autocmd events.
 */
MARK autop, aubottom;

/* This variable indicates whether we're currently running an autocmd.
 * The experform() function checks this to enforce the rule that autocmd
 * commands can't change the current buffer.
 */
ELVBOOL aubusy;

/* This table converts names to bits, or vice versa */
static struct {
	CHAR	*name;
	aubits_t bits;
} nametbl[] =
{
  { toCHAR("*"),			{0x7fffffff, 0x3fffffff, 0x3fffffff}},
  { toCHAR("*"),/* without OPTBITS */	{0x7fffffff, 0x3ffff9ff, 0x3fffffff}},

  /* file events */
  { toCHAR("BufCreate"),		{0x00000001, 0x00000000, 0x00000000}},
  { toCHAR("BufDelete"),		{0x00000002, 0x00000000, 0x00000000}},
  { toCHAR("BufEnter"),			{0x00000004, 0x00000000, 0x00000000}},
  { toCHAR("BufFilePost"),		{0x00000008, 0x00000000, 0x00000000}},
  { toCHAR("BufFilePre"),		{0x00000010, 0x00000000, 0x00000000}},
  { toCHAR("BufHidden"),		{0x00000020, 0x00000000, 0x00000000}},
  { toCHAR("BufLeave"),			{0x00000040, 0x00000000, 0x00000000}},
  { toCHAR("BufNewFile"),		{0x00000080, 0x00000000, 0x00000000}},
  { toCHAR("BufRead"), /* BufReadPost */{0x00000100, 0x40000000, 0x00000000}},
  { toCHAR("BufReadPost"),		{0x00000100, 0x00000000, 0x00000000}},
  { toCHAR("BufReadPre"),		{0x00000200, 0x00000000, 0x00000000}},
  { toCHAR("BufUnload"),		{0x00000400, 0x00000000, 0x00000000}},
  { toCHAR("BufWrite"),/* BufWritePre */{0x00000800, 0x40000000, 0x00000000}},
  { toCHAR("BufWritePost"),		{0x00001000, 0x00000000, 0x00000000}},
  { toCHAR("BufWritePre"),		{0x00000800, 0x00000000, 0x00000000}},
  { toCHAR("FileAppendPost"),		{0x00002000, 0x00000000, 0x00000000}},
  { toCHAR("FileAppendPre"),		{0x00004000, 0x00000000, 0x00000000}},
  { toCHAR("FileChangedShell"),		{0x00008000, 0x00000000, 0x00000000}},
  { toCHAR("FileReadPost"),		{0x00010000, 0x00000000, 0x00000000}},
  { toCHAR("FileReadPre"),		{0x00020000, 0x00000000, 0x00000000}},
  { toCHAR("FileWritePost"),		{0x00040000, 0x00000000, 0x00000000}},
  { toCHAR("FileWritePre"),		{0x00080000, 0x00000000, 0x00000000}},
  { toCHAR("FilterReadPost"),		{0x00100000, 0x00000000, 0x00000000}},
  { toCHAR("FilterReadPre"),		{0x00200000, 0x00000000, 0x00000000}},
  { toCHAR("FilterWritePost"),		{0x00400000, 0x00000000, 0x00000000}},
  { toCHAR("FilterWritePre"),		{0x00800000, 0x00000000, 0x00000000}},
  { toCHAR("StdinReadPost"),		{0x01000000, 0x00000000, 0x00000000}},
  { toCHAR("StdinReadPre"),		{0x02000000, 0x00000000, 0x00000000}},

  /* other events */
  { toCHAR("AliasEnter"),		{0x00000000, 0x00000001, 0x00000000}},
  { toCHAR("AliasLeave"),		{0x00000000, 0x00000002, 0x00000000}},
  { toCHAR("BgChanged"),		{0x00000000, 0x00000004, 0x00000000}},
  { toCHAR("CursorHold"),		{0x00000000, 0x00000008, 0x00000000}},
  { toCHAR("DisplayEnter"),		{0x00000000, 0x00000010, 0x00000000}},
  { toCHAR("DisplayLeave"),		{0x00000000, 0x00000020, 0x00000000}},
  { toCHAR("DispMapEnter"),		{0x00000000, 0x00000040, 0x00000000}},
  { toCHAR("DispMapLeave"),		{0x00000000, 0x00000080, 0x00000000}},
  { toCHAR("Edit"),			{0x00000000, 0x00000100, 0x00000000}},
  { toCHAR("FileEncoding"),		{0x00000000, 0x00000200, 0x00000000}},
  { toCHAR("FileType"),			{0x00000000, 0x00000400, 0x00000000}},
  { toCHAR("FocusGained"),		{0x00000000, 0x00000800, 0x00000000}},
  { toCHAR("FocusLost"),		{0x00000000, 0x00001000, 0x00000000}},
  { toCHAR("GUIEnter"),			{0x00000000, 0x00002000, 0x00000000}},
  { toCHAR("OptChanged"),/* OPTBITS */	{0x00000000, 0x00004000, 0x00000000}},
  { toCHAR("OptSet"),	/* OPTBITS */	{0x00000000, 0x00008000, 0x00000000}},
  { toCHAR("ScriptEnter"),		{0x00000000, 0x00010000, 0x00000000}},
  { toCHAR("ScriptLeave"),		{0x00000000, 0x00020000, 0x00000000}},
  { toCHAR("Syntax"),			{0x00000000, 0x00040000, 0x00000000}},
  { toCHAR("TermChanged"),		{0x00000000, 0x00080000, 0x00000000}},
  { toCHAR("User"),			{0x00000000, 0x00100000, 0x00000000}},
  { toCHAR("VimEnter"),			{0x00000000, 0x00200000, 0x00000000}},
  { toCHAR("VimLeave"),			{0x00000000, 0x00400000, 0x00000000}},
  { toCHAR("VimLeavePre"),		{0x00000000, 0x00800000, 0x00000000}},
  { toCHAR("WinEnter"),			{0x00000000, 0x01000000, 0x00000000}},
  { toCHAR("WinLeave"),			{0x00000000, 0x02000000, 0x00000000}},

  /* user events */
  { NULL,	/* AU_USER01 */		{0x00000000, 0x00000000, 0x00000001}},
  { NULL,	/* AU_USER02 */		{0x00000000, 0x00000000, 0x00000002}},
  { NULL,	/* AU_USER03 */		{0x00000000, 0x00000000, 0x00000004}},
  { NULL,	/* AU_USER04 */		{0x00000000, 0x00000000, 0x00000008}},
  { NULL,	/* AU_USER05 */		{0x00000000, 0x00000000, 0x00000010}},
  { NULL,	/* AU_USER06 */		{0x00000000, 0x00000000, 0x00000020}},
  { NULL,	/* AU_USER07 */		{0x00000000, 0x00000000, 0x00000040}},
  { NULL,	/* AU_USER08 */		{0x00000000, 0x00000000, 0x00000080}},
  { NULL,	/* AU_USER09 */		{0x00000000, 0x00000000, 0x00000100}},
  { NULL,	/* AU_USER10 */		{0x00000000, 0x00000000, 0x00000200}},
  { NULL,	/* AU_USER11 */		{0x00000000, 0x00000000, 0x00000400}},
  { NULL,	/* AU_USER12 */		{0x00000000, 0x00000000, 0x00000800}},
  { NULL,	/* AU_USER13 */		{0x00000000, 0x00000000, 0x00001000}},
  { NULL,	/* AU_USER14 */		{0x00000000, 0x00000000, 0x00002000}},
  { NULL,	/* AU_USER15 */		{0x00000000, 0x00000000, 0x00004000}},
  { NULL,	/* AU_USER16 */		{0x00000000, 0x00000000, 0x00008000}},
  { NULL,	/* AU_USER17 */		{0x00000000, 0x00000000, 0x00010000}},
  { NULL,	/* AU_USER18 */		{0x00000000, 0x00000000, 0x00020000}},
  { NULL,	/* AU_USER19 */		{0x00000000, 0x00000000, 0x00040000}},
  { NULL,	/* AU_USER20 */		{0x00000000, 0x00000000, 0x00080000}},
  { NULL,	/* AU_USER21 */		{0x00000000, 0x00000000, 0x00100000}},
  { NULL,	/* AU_USER22 */		{0x00000000, 0x00000000, 0x00200000}},
  { NULL,	/* AU_USER23 */		{0x00000000, 0x00000000, 0x00400000}},
  { NULL,	/* AU_USER24 */		{0x00000000, 0x00000000, 0x00800000}},
  { NULL,	/* AU_USER25 */		{0x00000000, 0x00000000, 0x01000000}},
  { NULL,	/* AU_USER26 */		{0x00000000, 0x00000000, 0x02000000}},
  { NULL,	/* AU_USER27 */		{0x00000000, 0x00000000, 0x04000000}},
  { NULL,	/* AU_USER28 */		{0x00000000, 0x00000000, 0x08000000}},
  { NULL,	/* AU_USER29 */		{0x00000000, 0x00000000, 0x10000000}},
  { NULL,	/* AU_USER30 */		{0x00000000, 0x00000000, 0x20000000}},
};
#define OPTBITS 0x00003000

#ifdef USE_PROTOTYPES
static ELVBOOL wildmatch(char *fname, char *wildlist);
static aubits_t *nametobits(CHAR *name);
static CHAR *bitstoname(aubits_t *bits);
static CHAR *nextword(CHAR **refp);
static void listcmd(WINDOW win, CHAR *cmd);
#endif

/* This is the default autocmd group */
static aug_t defgroup = {NULL, toCHAR("END"), NULL};

/* This stores a list of groups */
static aug_t *groups = &defgroup;

/* This points to the current group */
static aug_t *curgroup = &defgroup;

/* This is the set of events that actually are actually being used */
static aubits_t usedbits;

/* These options are defined while an autocommand is executing */
static OPTDESC audesc[] =
{
	{ "aufilename", "afile",optsstring,	optisstring },
	{ "auevent", "ev",	optsstring,	optisstring },
	{ "auforce", "bang",	NULL,		NULL	    }
};
#define o_aufilename	auval[0].value.string
#define o_auevent	auval[1].value.string
#define o_auforce	auval[2].value.boolean

/* Compare a given filename to a list of wildcard patterns.  This uses
 * dirwildcmp() to perform the wildcard matching, and also implements the
 * logic to extract patterns from a comma-delimited list.  The wildlist
 * must be in writable memory; it is temporarily altered by this function.
 *
 * Returns ElvTrue for a match, or ElvFalse otherwise.
 */
static ELVBOOL wildmatch(fname, wildlist)
	char	*fname;		/* a given file name */
	char	*wildlist;	/* comma-delimited list of wildcard patterns */
{
	char	*scan, *start;
	ELVBOOL	match;

	/* for each character in wildlist... */
	for (scan = start = wildlist; *scan; scan++)
	{
		/* if this is the end of a wildcard pattern... */
		if (*scan == ',')
		{
			/* compare the filename to the wildcard pattern */
			*scan = '\0';
			match = dirwildcmp(fname, start);
			*scan = ',';
			if (match)
				return ElvTrue;

			/* no match yet -- prepare for next pattern */
			start = scan + 1;
		}
	}

	/* handle the last pattern too */
	return (dirwildcmp(fname, start));
}

/* Implement :augroup -- Select a group of autocommands */
RESULT ex_augroup(xinf)
	EXINFO	*xinf;
{
	aug_t	*scan, *lag;
	int	col, len;
	CHAR	gap[2];

	/* were we given a new group name? */
	if (!xinf->lhs)
	{
		/* no group given, so list the current groups */
		for (col = 0, scan = groups; scan; scan = scan->next)
		{
			if (scan == &defgroup)
				continue;
			len = CHARlen(scan->group);
			col += len + 1;
			gap[0] = ' ';
			if (col >= o_columns(xinf->window))
			{
				gap[0] = '\n';
				col = len;
			}
			if (col > 0)
				drawextext(xinf->window, gap, 1);
			drawextext(xinf->window, scan->group, len);
		}
		if (col > 0)
		{
			gap[0] = '\n';
			drawextext(xinf->window, gap, 1);
		}
	}
	else
	{
		/* As a special case, "end" is treated like "END" */
		if (!CHARcmp(xinf->lhs, toCHAR("end")))
		{
			curgroup = &defgroup;
			return RESULT_COMPLETE;
		}

		/* Find the given group.  If it doesn't exist, create it */
		for (scan = groups, lag = NULL;
		     scan && CHARcmp(xinf->lhs, scan->group) > 0;
		     lag = scan, scan = scan->next)
		{
		}
		if (scan && !CHARcmp(xinf->lhs, scan->group))
		{
			/* found -- use it */
			curgroup = scan;
		}
		else
		{
			/* not found -- need to create it */
			curgroup = (aug_t *)safealloc(1, sizeof(aug_t));
			curgroup->group = CHARdup(xinf->lhs);
			if (lag)
				lag->next = curgroup;
			else
				groups = curgroup;
			curgroup->next = scan;
		}

		/* remember whether internal (":aug!") */
		curgroup->internal = xinf->bang;
	}

	return RESULT_COMPLETE;
}

/* convert an event name string into event bits */
static aubits_t *nametobits(name)
	CHAR *name;
{
	static aubits_t bits;
	int	len;
	auevent_t i;

	assert(AU_QTY_EVENTS == QTY(nametbl));

	/* clobber the bits */
	bits.fileevents = bits.otherevents = bits.userevents = 0;

	/* while we have more event names to convert... */
	while (*name)
	{
		/* for each possible name... */
		for (i = (auevent_t)0;
		     i < (auevent_t)QTY(nametbl) && nametbl[i].name;
		     i++)
		{
			/* count the matching chars, ignoring case */
			for (len = 0; name[len] && elvtolower(nametbl[i].name[len]) == elvtolower(name[len]); len++)
			{
			}

			/* if it matches ... */
			if (nametbl[i].name[len] == '\0'
			 && (name[len] == ',' || name[len] == '\0'))
			{
				/* include its bit */
				bits.fileevents |= nametbl[i].bits.fileevents;
				bits.otherevents |= nametbl[i].bits.otherevents;
				bits.userevents |= nametbl[i].bits.userevents;

				/* skip to the end of the name.  Skip ',' too */
				name += len;
				if (*name == ',')
					name++;
				break;
			}
		}

		/* if we hit the end without a match, then complain */
		if (i >= QTY(nametbl) || !nametbl[i].name)
		{
			msg(MSG_ERROR, "bad autocmd event name");
			return NULL;
		}
	}

	/* strip off the "ugly" bit */
	bits.otherevents &= ~0x40000000;

	/* strip off the "internal" bit */
	bits.userevents &= ~0x40000000;

	/* return the combined bits */
	return &bits;
}

/* convert event bits into a string */
static CHAR *bitstoname(aubits)
	aubits_t *aubits;
{
	static CHAR	name[500];
	int		i, len;
	aubits_t	bits, *nbits;

	/* for each possible event */
	bits = *aubits;
	for (i = len = 0;
	     i < QTY(nametbl) && nametbl[i].name;
	     i++)
	{
		/* skip if not part of bits */
		nbits = &nametbl[i].bits;
		if ((bits.fileevents & nbits->fileevents) != nbits->fileevents
		 || (bits.otherevents & nbits->otherevents) != nbits->otherevents
		 || (bits.userevents & nbits->userevents) != (nbits->userevents & ~0x40000000))
			continue;

		/* add this event to the name */
		if (len > 0)
			name[len++] = ',';
		CHARcpy(&name[len], nametbl[i].name);
		len += CHARlen(&name[len]);

		/* remove this event from the bits to show */
		bits.fileevents &= ~nbits->fileevents;
		bits.otherevents &= ~nbits->otherevents;
		bits.userevents &= ~nbits->userevents;
	}

	return name;
}

/* return a word.  Advance refp past end of the word, and stick a NUL there */
static CHAR *nextword(refp)
	CHAR	**refp;
{
	CHAR	*word;

	/* if no more words, then return NULL */
	if (!*refp || !**refp)
		return NULL;

	/* remember where this word starts */
	word = *refp;

	/* skip to the end of the line */
	while (**refp && !elvspace(**refp))
		(*refp)++;

	/* stick a NUL there, if not NUL already */
	if (**refp)
		*(*refp)++ = '\0';

	/* skip trailing whitespace, if any */
	while (elvspace(**refp))
		(*refp)++;

	/* return the word */
	return word;
}


/* Implement :auevent -- create a new event name */
RESULT ex_auevent(xinf)
	EXINFO *xinf;
{
	CHAR	*scan, *word;
	int	i, len, col; 
	CHAR	spc[1];

	/* were we given any words? */
	scan = xinf->rhs;
	word = nextword(&scan);
	if (!word)
	{
		/* no -- so just list the previously added events */
		for (col = 0, i = 2; i < QTY(nametbl) && nametbl[i].name; i++)
		{
			/* skip the builtins */
			if (nametbl[i].bits.userevents == 0)
				continue;

			/* skip if is/isn't set up using standard scripts */
			if ((nametbl[i].bits.userevents & 0x40000000) !=
			    (long)(xinf->bang ? 0x40000000 : 0x0))
				continue;

			/* output a space or newline, if necessary */
			len = CHARlen(nametbl[i].name);
			if (col > 0)
			{
				if (col + 1 + len >= o_columns(xinf->window))
					spc[0] = '\n', col = 0;
				else
					spc[0] = ' ', col++;
				drawextext(xinf->window, spc, 1);
			}

			/* output the name */
			drawextext(xinf->window, nametbl[i].name, len);
			col += len;
		}

		/* if we printed anything, then end the last line */
		if (col > 0)
		{
			  spc[0] = '\n';
			  drawextext(xinf->window, spc, 1);
		}
		return RESULT_COMPLETE;
	}

	/* for each word... */
	for ( ; word; word = nextword(&scan))
	{
		/* look for an open spot in the nametbl */
		/* if already a known event, then do nothing */
		for (i = 0; i < QTY(nametbl) && nametbl[i].name; i++)
		{
			for (len = 0; word[len]; len++)
				if (elvtolower(nametbl[i].name[len]) != elvtolower(word[len]))
					break;
			if (!word[len] && !nametbl[i].name[len])
				goto ContinueContinue;
		}
		if (i >= QTY(nametbl))
		{
			msg(MSG_ERROR, "too many user-defined event names");
			return RESULT_ERROR;
		}

		/* store the name there */
		nametbl[i].name = CHARdup(word);

		/* also mark it as being "internal" if ! flag */
		if (xinf->bang)
			nametbl[i].bits.userevents |= 0x40000000;
ContinueContinue:
		;
	}
	return RESULT_COMPLETE;
}

static void listcmd(win, cmd)
	WINDOW	win;
	CHAR	*cmd;
{
	CHAR	ch[4];
	CHAR	*start;
	int	len;
	int	indent;

	if (CHARchr(cmd, '\n') == cmd + CHARlen(cmd) - 1)
	{
		/* single-line command simply follows the alias name */
		drawexlist(win, cmd, CHARlen(cmd));
	}
	else
	{
		/* multi-line command is output in a fancy way */
		ch[0] = '{';
		ch[1] = '\n';
		drawexlist(win, ch, 2);
		indent = 4;
		for (start = cmd, len = 0; *start; len++)
		{
			if (start[len] == '\n')
			{
				drawexlist(win, blanks, indent);
				drawexlist(win, start, len);
				ch[0] = '\n';
				drawexlist(win, ch, 1);
				start += len + 1;
				if (len > 1 && indent + 2 < QTY(blanks) && start[-2] == '{')
					indent += 2;
				if (indent > 4 && *start == '}')
					indent -= 2;
				len = -1; /* will be incremented to 0 by for()*/
			}
		}
		ch[0] = '}';
		ch[1] = '\n';
		drawexlist(win, ch, 2);
	}
}

/* Implement :autocmd -- add or delete autocmds */
RESULT ex_autocmd(xinf)
	EXINFO	*xinf;
{
	CHAR	*word, *scan;
	CHAR	*ptrn, *excmd;
	char	*optname;
	aug_t	*group = curgroup;
	aubits_t *givenbits, bits;
	au_t	*au, *lag, *next;
	ELVBOOL	anybusy;

	/* parse the optional group name */
	scan = xinf->rhs;
	word = nextword(&scan);
	if (word)
	{
		for (group = groups;
		     group && CHARcmp(group->group, word);
		     group = group->next)
		{
		}
		if (group)
			/* yes this was group name - we used this word, so get
			 * another.
			 */
			word = nextword(&scan);
		else
			/* no, so use the default group */
			group = curgroup;
	}

	/* parse the event -- if none then assume all bits */
	if (word)
	{
		givenbits = nametobits(word);
		if (!givenbits)
			return RESULT_ERROR;
		bits = *givenbits;
		word = nextword(&scan);
	}
	else
	{
		bits = nametbl[AU_ALL_EVENTS].bits;
	}

	/* the last word is the filename pattern, or NULL.  For NULL we want
	 * to use "*".
	 */
	ptrn = word ? word : toCHAR("*");

	/* now "scan" points to the remainder of the line -- the command to
	 * be executed.
	 */
	if (scan && !CHARncmp(scan, toCHAR("nested "), 7))
		scan += 7;
	if (scan && !*scan)
		scan = NULL;
	excmd = scan;

	/* okay, at this point we have parsed everything.  Sanity check */

	/* do we want to add an autocmd or delete some of them?  */
	anybusy = ElvFalse;
	if (xinf->bang || !excmd)
	{
		/* List/Del all items in group that don't clash with given info */
		for (au = group->au, lag = NULL; au; lag = au, au = next)
		{
			/* remember the next item -- we can't expect au->next
			 * to still be valid if we free au.
			 */
			next = au->next;

			/* if no matching event bits, then skip it */
			if (((bits.fileevents & au->bits.fileevents)
			    | (bits.otherevents & au->bits.otherevents)
			    | (bits.userevents & au->bits.userevents)) == 0)
				continue;

			/* If the filename pattern clashes, then skip it.
			 * NOTE: This is a little weird because we're comparing
			 * a pattern to a pattern, instead of a filename to a
			 * pattern.
			 */
			if (!wildmatch(tochar8(au->ptrn), tochar8(ptrn)))
				continue;

			/* if just supposed to list it, then do that */
			if (!xinf->bang)
			{
				drawextext(xinf->window, toCHAR("au "), 3);
				word = bitstoname(&au->bits);
				drawextext(xinf->window, word, CHARlen(word));
				drawextext(xinf->window, blanks, 1);
				drawextext(xinf->window, au->ptrn, CHARlen(au->ptrn));
				drawextext(xinf->window, blanks, 1);
				listcmd(xinf->window, au->excmd);
				continue;
			}

			/* if busy, then set a flag and skip it */
			if (au->busy)
			{
				anybusy = ElvTrue;
				continue;
			}

			/* remove these events from this item.  If there are
			 * any other bits still set after that, then leave it.
			 */
			au->bits.fileevents &= ~bits.fileevents;
			au->bits.otherevents &= ~bits.otherevents;
			au->bits.userevents &= ~bits.userevents;
			if (au->bits.fileevents | au->bits.otherevents | au->bits.userevents)
				continue;

			/* this autocmd has no reason to live */
			if (lag)
				lag->next = next;
			else
				group->au = next;
			safefree(au->ptrn);
			safefree(au->excmd);
			safefree(au);

			/* in the for-loop's increment clause, we'll be setting
			 * lag=au... but we really don't want to move lag since
			 * au was just deleted.  Consequently, we set au=lag
			 * here so lag won't change there.
			 */
			au = lag;
		}
	}

	/* When adding auto commands with "*" in pattern, omit option events */
	if (excmd && CHARchr(ptrn, '*') && (bits.otherevents & OPTBITS) != 0)
	{
		bits.otherevents &= ~OPTBITS;
		if ((bits.fileevents | bits.otherevents | bits.userevents) == 0)
		{
			msg(MSG_WARNING, "can't add opt events with * pattern");
			excmd = NULL;
		}
	}

	/* if a new command was given, then add it now */
	if (excmd)
	{
		/* Create a new autocmd. */
		next = (au_t *)safealloc(1, sizeof(au_t));
		next->bits = bits;
		next->ptrn = CHARdup(ptrn);
		next->excmd = CHARdup(excmd);

		/* If ptrn is option names, then convert short names to long */
		if (bits.otherevents & OPTBITS)
		{
			/* for each option that it tests... */
			ptrn = NULL;
			for (word = next->ptrn, scan = NULL; word; word = scan)
			{
				/* if previous comma, then restore it */
				if (scan)
					scan[-1] = ',';

				/* look for a comma */
				scan = CHARchr(word, ',');
				if (scan)
					*scan++ = '\0';

				/* convert short name to long name */
				optname = optevent(word);
				if (optname)
				{
					if (ptrn)
						buildCHAR(&ptrn, ',');
					buildstr(&ptrn, optname);
				}
				else
					msg(MSG_WARNING, "[S]bad option name $1", word);
			}

			/* free the old pattern, use the new one */
			safefree(next->ptrn);
			next->ptrn = ptrn;
		}

		/* the above OptSet and OptChanged handler may have left no
		 * valid options in the pattern.  If so, then don't add it.
		 */
		if (!next->ptrn)
		{
			safefree(next->excmd);
			safefree(next);
			bits.otherevents &= OPTBITS;
		}
		else /* add it */
		{

			/* Locate the end of the group.  We always add to
			 * the end of a group, so that the commands can be
			 * executed in a predictable order.
			 */
			for (au = group->au, lag = NULL; au; lag = au, au = au->next)
			{
			}
			if (lag)
				lag->next = next;
			else
				group->au = next;

			/* merge this command's event bits into usedbits */
			usedbits.fileevents |= bits.fileevents;
			usedbits.otherevents |= bits.otherevents;
			usedbits.userevents |= bits.userevents;
		}
	}

	/* tell user if some couldn't be deleted */
	if (anybusy)
	{
		msg(MSG_WARNING, "some autocmds couldn't be deleted because they were running");
	}

	/* if any auto commands were deleted (not replaced or listed), then
	 * we need to recreate the usedbits variable from scratch.
	 */
	if (!xinf->bang && !excmd)
	{
		usedbits.fileevents = usedbits.otherevents = usedbits.userevents = 0;
		for (group = groups; group; group = group->next)
		{
			for (au = group->au; au; au = au->next)
			{
				usedbits.fileevents |= au->bits.fileevents;
				usedbits.otherevents |= au->bits.otherevents;
				usedbits.userevents |= au->bits.userevents;
			}
		}
	}

	/* If we added or deleted any auto commands that are sensitive to
	 * options, then we need to adjust the options' event flags
	 */
	if ((bits.otherevents & OPTBITS) != 0
	 && (xinf->bang || excmd))
	{
		(void)optevent(NULL);
		for (group = groups; group; group = group->next)
		{
			for (au = group->au; au; au = au->next)
			{
				/* skip if not for options */
				if ((au->bits.otherevents & OPTBITS) == 0)
					continue;

				/* for each option that it tests... */
				for (word = ptrn, scan = NULL; word; word = scan)
				{
					/* if previous comma, then restore it */
					if (scan)
						scan[-1] = ',';

					/* look for a comma */
					scan = CHARchr(word, ',');
					if (scan)
						*scan++ = '\0';

					/* option's name is isolated - watch
					 * for events on it.
					 */
					(void)optevent(word);
				}
			}
		}
	}

	return RESULT_COMPLETE;
}

/* implement :doautocmd -- simulate an event */
RESULT ex_doautocmd(xinf)
	EXINFO	*xinf;
{
	CHAR	*word, *scan;
	aug_t	*group = curgroup;
	auevent_t event;
	int	len;

	/* parse the optional group name */
	scan = xinf->rhs;
	word = nextword(&scan);
	if (word)
	{
		for (group = groups;
		     group && CHARcmp(group->group, word);
		     group = group->next)
		{
		}
		if (group)
			/* yes this was group name - we used this word, so get
			 * another.
			 */
			word = nextword(&scan);
	}

	/* parse the event - must be a single event */
	if (!word)
	{
		msg(MSG_ERROR, "missing event name");
		return RESULT_ERROR;
	}
	for (event = (auevent_t)0;
	     event < AU_QTY_EVENTS && nametbl[event].name;
	     event++)
	{
		for (len = 0; word[len] && elvtolower(nametbl[event].name[len]) == elvtolower(word[len]); len++)
		{
		}
		if (!word[len] && !nametbl[event].name[len])
			break;
	}
	if (event == AU_QTY_EVENTS)
	{
		msg(MSG_ERROR, "unknown event");
		return RESULT_ERROR;
	}

	/* remainder is filename; defaults to current filename */
	if (!*scan)
		scan = o_filename(markbuffer(&xinf->defaddr));

	/* simulate the event */
	return auperform(xinf->window, xinf->bang, group ? group->group : NULL,
		event, scan);
}

/* perform the commands for a given event */
RESULT auperform(win, bang, groupname, event, filename)
	WINDOW	win;		/* window to run in */
	ELVBOOL bang;		/* invoked with "!" ? */
	CHAR	*groupname;	/* name of the group -- usually NULL for all */
	auevent_t event;	/* event to run */
	CHAR	*filename;	/* filename-- usually NULL for default */
{
	aug_t	*group;		/* used for scanning groups */
	au_t	*au;		/* used for scanning autocmds within group */
	aubits_t *bits;		/* bitmask of this event */
	aubits_t *ignore;	/* bitmask of events to ignore */
	ELVBOOL	anygrp;		/* was any matching group name found? */
	ELVBOOL oldhide;
	RESULT	result;
	OPTVAL	auval[QTY(audesc)];

	/* locate the event's bitmask */
	bits = &nametbl[event].bits;

	/* if we know there aren't any auto commands for this event, skip it */
	if ((usedbits.fileevents & bits->fileevents) == 0
	 && (usedbits.otherevents & bits->otherevents) == 0
	 && (usedbits.userevents & bits->userevents) == 0)
		return RESULT_COMPLETE;

	/* if any events are supposed to be ignored, then ignore them */
	if (o_eventignore)
	{
		if (!CHARcmp(o_eventignore, toCHAR("all")))
			return RESULT_COMPLETE;
		ignore = nametobits(o_eventignore);
		if (ignore &&
		     ((bits->fileevents & ignore->fileevents)
		    | (bits->otherevents & ignore->otherevents)
		    | (bits->userevents & ignore->userevents)) != 0)
			return RESULT_COMPLETE;
	}

	/* set the "aubusy" flag to indicate that we're running an autocmd */
	aubusy = ElvTrue;

	/* if no filename specified, then use the default filename */
	if (!filename && bufdefault)
	{
		filename = o_filename(bufdefault);
		if (!filename)
			filename = o_bufname(bufdefault);
	}
	if (!filename)
	{
		filename = toCHAR("no file yet");
	}

	/* set up the options that the commands will need */
	memset(auval, 0, sizeof auval);
	optpreset(o_aufilename, CHARdup(filename), OPT_LOCK|OPT_HIDE);
	optpreset(o_auevent, nametbl[event].name, OPT_LOCK|OPT_HIDE);
	optpreset(o_auforce, bang, OPT_LOCK|OPT_HIDE);
	optinsert("au", QTY(audesc), audesc, auval);

	/* for each group ... */
	anygrp = ElvFalse;
	for (group = groups; group; group = group->next)
	{
		/* if a specific group was requested, then skip all others */
		if (groupname && CHARcmp(groupname, group->group))
			continue;
		anygrp = ElvTrue;

		/* for each command in this group... */
		for (au = group->au; au; au = au->next)
		{
			/* skip if it doesn't include this event */
			if (((bits->fileevents & au->bits.fileevents)
			   | (bits->otherevents & au->bits.otherevents)
			   | (bits->userevents & au->bits.userevents)) == 0)
				continue;

			/* skip if for a different file pattern */
			if (!wildmatch(tochar8(o_aufilename), tochar8(au->ptrn)))
				continue;

			/* skip if busy */
			if (au->busy)
				continue;

			/* execute the command */
			au->busy = ElvTrue;
			oldhide = msghide((ELVBOOL)!o_eventerrors);
			result = exstring(win, au->excmd, NULL);
			(void)msghide(oldhide);
			au->busy = ElvFalse;
			if (o_eventerrors && result != RESULT_COMPLETE)
				goto Error;
		}
	}

	/* if requested group not found, fail */
	if (!anygrp)
	{
		msg(MSG_ERROR, "no such augroup");
		goto Error;
	}

	optdelete(auval);
	safefree(o_aufilename);
	aubusy = ElvFalse;
	return RESULT_COMPLETE;

Error:	optdelete(auval);
	safefree(o_aufilename);
	aubusy = ElvFalse;
	return RESULT_ERROR;
}

/* This is called periodically to detect changes in the display mode.  When it
 * detects a change, it triggers DispMapLeave and DispMapEnter autocmds.
 */
void audispmap()
{
	static CHAR *prevdisplay;

	/* if there is no default window, then do nothing */
	if (!windefault)
		return;
	assert(o_display(windefault));

	/* if there was a previous display mode, and it is different from the
	 * current window's display mode, then do DispMapLeave on it.
	 */
	if (prevdisplay && CHARcmp(prevdisplay, o_display(windefault)))
	{
		(void)auperform(windefault, ElvFalse, NULL, AU_DISPMAPLEAVE, prevdisplay);
		safefree(prevdisplay);
		prevdisplay = NULL;
	}

	/* if no previous display (or we just clobbered it), then do a
	 * DispMapEnter for this window's display mode.
	 */
	if (!prevdisplay)
	{
		prevdisplay = CHARkdup(o_display(windefault));
		(void)auperform(windefault, ElvFalse, NULL, AU_DISPMAPENTER, prevdisplay);
	}
}

/* Convert a case-insensitive event name into its canonical form.  Returns
 * the canonical name, or NULL if invalid.  This is used by the :help command.
 */
CHAR *auname(name)
	CHAR	*name;
{
	aubits_t *bits;
	bits = nametobits(name);
	if (bits)
		return bitstoname(bits);
	return NULL;
}

# ifdef FEATURE_MKEXRC
void ausave(custom)
	BUFFER	custom;	/* the buffer to which the :au commands are added */
{
	ELVBOOL anygrp;	/* any groups output yet? */
	MARKBUF	end;
	aug_t	*group;
	au_t	*au;
	CHAR	*cmd, *word;
	int	i;

	end.buffer = custom;

	/* for each event... */
	for (i = 2; i < QTY(nametbl) && nametbl[i].name; i++)
	{
		/* skip if built-in, or defined in a standard script */
		if (nametbl[i].bits.userevents == 0
		 || (nametbl[i].bits.userevents & 0x40000000) != 0)
			continue;

		/* output a command to recreate this event type */
		end.offset = o_bufchars(custom);
		bufreplace(&end, &end, toCHAR("try aue "), 8L);
		end.offset = o_bufchars(custom);
		bufreplace(&end, &end, nametbl[i].name, (long)CHARlen(nametbl[i].name));
		end.offset = o_bufchars(custom);
		bufreplace(&end, &end, toCHAR("\n"), 1L);
	}

	/* for each group... */
	for (anygrp = ElvFalse, group = groups; group; group = group->next)
	{
		/* skip if internal or empty */
		if (group->internal || !group->au)
			continue;

		/* remember that we have at least one group */
		anygrp = ElvTrue;

		/* start this group */
		end.offset = o_bufchars(custom);
		bufreplace(&end, &end, toCHAR("try aug "), 8L);
		end.offset = o_bufchars(custom);
		bufreplace(&end, &end, group->group, (long)CHARlen(group->group));
		end.offset = o_bufchars(custom);
		bufreplace(&end, &end, toCHAR("\nthen {\n au!\n"), 13L);

		/* for each au in this group... */
		for (au = group->au; au; au = au->next)
		{
			/* construct an au command string */
			cmd = NULL;
			buildstr(&cmd, " au ");
			for (word = bitstoname(&au->bits); *word; word++)
				buildCHAR(&cmd, *word);
			buildCHAR(&cmd, ' ');
			for (word = au->ptrn; *word; word++)
				buildCHAR(&cmd, *word);
			buildCHAR(&cmd, ' ');
			word = au->excmd;
			if (CHARchr(word, '\n') == word + CHARlen(word) - 1)
			{
				/* single line - output it including final \n */
				for ( ; *word; word++)
				{
					if (*word == ELVCTRL('V') || *word == '\033')
						buildCHAR(&cmd, ELVCTRL('V'));
					buildCHAR(&cmd, *word);
				}
			}
			else
			{
				/* multi-line - add explicit {} around it */
				buildstr(&cmd, "{\n  ");
				for ( ; *word; word++)
				{
					if (*word == ELVCTRL('V') || *word == '\033')
						buildCHAR(&cmd, ELVCTRL('V'));
					buildCHAR(&cmd, *word);
					if (*word == '\n' && word[1])
						buildstr(&cmd, "  ");
				}
				buildstr(&cmd, " }\n");
			}

			/* add the command to the buffer */
			end.offset = o_bufchars(custom);
			bufreplace(&end, &end, cmd, CHARlen(cmd));

			/* free the string form of the command */
			safefree(cmd);

		} /* for au */

		/* mark the end of this group */
		end.offset = o_bufchars(custom);
		bufreplace(&end, &end, toCHAR("}\n"), 2);

	} /* for group */

	/* if any groups were output, then we need to end "if feature(...)" */
	if (anygrp)
	{
		end.offset = o_bufchars(custom);
		bufreplace(&end, &end, toCHAR("try aug END\n"), 12L);
	}
}
# endif /* FEATURE_MKEXRC */

# ifdef FEATURE_COMPLETE
CHAR *aucomplete(win, from, to)
	WINDOW	win;	/* where to list multiples */
	MARK	from;	/* start of cmd */
	MARK	to;	/* end of partial word */
{
	CHAR	*cp;
	ELVBOOL	inword;
	int	nwords;
	long	len;
	MARKBUF word;
	auevent_t event, match;
	int	i, matches;
	int	wlen, mlen;
	static CHAR retbuf[20];

	/* this is just to avoid a bogus compiler warning */
	match = (auevent_t)0;
	mlen = 0;

	/* count the words leading up to the current word */
	inword = ElvFalse;
	nwords = 0;
	len = markoffset(to) - markoffset(from);
	word = *from;
	for (scanalloc(&cp, from); cp && len > 0 && nwords < 3; scannext(&cp), len--)
	{
		/* detect word edges */
		if (inword && elvspace(*cp))
			inword = ElvFalse;
		else if (!inword && !elvspace(*cp))
		{
			inword = ElvTrue;
			nwords++;
			word = *scanmark(&cp);
		}
		else if (*cp == ',')
		{
			word = *scanmark(&cp);
			markaddoffset(&word, 1L);
		}
	}
	scanfree(&cp);

	/* if cursor is after whitespace, then assume it is part of next word */
	if (!inword)
		nwords++;

	/* Process each word in a different way.  We ignore the possibility
	 * of giving a group name before the event name, for the sake of
	 * simplicity.
	 */
	switch (nwords)
	{
	  case 1: /* event name */
		/* collect the chars of the partial name */
		cp = bufmemory(&word, to);

		/* scan the event names, ignoring case */
		matches = 0;
		wlen = CHARlen(cp);
		for (event = (auevent_t)0;
		     event < AU_QTY_EVENTS && nametbl[event].name;
		     event++)
		{
			/* skip if ugly */
			if (nametbl[event].bits.otherevents & 0x40000000)
				continue;

			/* skip if it doesn't match */
			for (i = 0; i < wlen && elvtolower(cp[i]) == elvtolower(nametbl[event].name[i]); i++)
			{
			}
			if (i < wlen)
				continue;

			/* found a match.  Is it the first one? */
			matches++;
			if (matches == 1)
			{
				/* yes -- its length is maximum possible */
				mlen = CHARlen(nametbl[event].name);
				match = event;
			}
			else
			{
				/* no, not first -- maybe reduce maximum? */
				for (i = wlen; i < mlen && nametbl[event].name[i] == nametbl[match].name[i]; i++)
				{
				}
				mlen = i;
			}
		}

		/* if no matches, then beep and return nothing */
		if (matches == 0)
		{
			guibeep(win);
			return toCHAR("");
		}

		/* if unique match, then return its tail plus a space */
		if (matches == 1)
		{
			CHARcpy(retbuf, nametbl[match].name + wlen);
			CHARcat(retbuf, toCHAR(" "));
			return retbuf;
		}

		/* it isn't unique, but maybe we can complete some chars */
		if (mlen > wlen)
		{
			CHARcpy(retbuf, nametbl[match].name + wlen);
			retbuf[mlen - wlen] = '\0';
			return retbuf;
		}

		/* list the partially matching event names */
		len = 0L;
		for (event = (auevent_t)0;
		     event < AU_QTY_EVENTS && nametbl[event].name;
		     event++)
		{
			/* skip if ugly */
			if (nametbl[event].bits.otherevents & 0x40000000)
				continue;

			/* skip if it doesn't match */
			for (i = 0; i < wlen && elvtolower(cp[i]) == elvtolower(nametbl[event].name[i]); i++)
			{
			}
			if (i < wlen)
				continue;

			/* if this event won't fit on this line, then wrap */
			mlen = CHARlen(nametbl[event].name);
			if (len + mlen + 2 >= o_columns(win))
			{
				drawextext(win, toCHAR("\n"), 1);
				len = 0;
			}
			else if (len > 0)
			{
				drawextext(win, blanks, 1);
				len++;
			}

			/* draw the name */
			drawextext(win, nametbl[event].name, (long)mlen);
			len += mlen;
		}
		if (len > 0)
			drawextext(win, toCHAR("\n"), 1);
		return toCHAR("");

	  case 2: /* file type or pattern -- treat like filename */
		return NULL;

	  default: /* command -- treat like ex command */
		return excomplete(win, &word, to);
	}
}
# endif /* FEATURE_COMPLETE */

#endif /* FEATURE_AUTOCMD */
