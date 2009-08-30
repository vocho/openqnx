/* tagelvis.c */

/* Elvis uses this file to scan a tags file, and built a list of the matching
 * tags, sorted by name and likelyhood that they're the intended tag.
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_tagelvis[] = "$Id: tagelvis.c,v 1.42 2003/10/18 04:47:18 steve Exp $";
#endif


/* Each call to tetag() sets this option, to indicate whether the cursor
 * position should be saved.
 */
static ELVBOOL	newtag = ElvTrue;

#ifdef FEATURE_TAGS
/* This option can be set via temodified to indicate that the current tag
 * couldn't be loaded in a previous attempt because some other buffer was
 * modified.  The current tag hasn't been rejected; the next tetag() call
 * should return the same tag and leave the history unchanged.
 */
static ELVBOOL	sametag = ElvFalse;

/* Cause the next tetag() to return the same tag as the previous tetag() call */
void tesametag()
{
	/* Note: taglist will always be non-NULL if we got here via a normal
	 * tag search.  But if we got here by trying to follow a hypertext
	 * link from a modified buffer, then it will be NULL... in which case
	 * the sametag variable is irrelevant.  So don't set it if taglist=NULL.
	 */
	if (taglist)
		sametag = ElvTrue;
}

/* Locate a tag.  Return the tag if it exists, or NULL if there is none.
 * Also, update the tag history for successful or unsuccessful searches.
 */
TAG *tetag(select)
	CHAR	*select;
{
 static	char	*tfilename;	/* pathname of the previous "tags" file */
	char	*tmp;
	CHAR	*scan;
	CHAR	*args[2];
	int	i;

	/* Decide whether the previous search was successful or not, by
	 * checking whether the current argument consists of only the tagname
	 * of the previous tag.
	 */
	if (taglist && !(select && *select && CHARcmp(select, toCHAR(taglist->TAGNAME))))
	{
		/* if supposed to return the same tag as last time, do that. */
		if (sametag && taglist != NULL)
		{
			sametag = ElvFalse;
			return taglist;
		}

		/* failed search */
		tsadjust(taglist, '-');

		/* try the next one in the list.  If we hit the end of the
		 * list for this particular tags file, then try the next one
		 * from the path.
		 */
		tagdelete(ElvFalse);
		if (!taglist && tfilename)
		{
			/* find the next tags file from tagpath */
			for (tmp = iopath(tochar8(o_tags), "tags", ElvTrue);
			     tmp && strcmp(tmp, tfilename);
			     tmp = iopath(NULL, "tags", ElvTrue))
			{
			}
			if (tmp)
				tmp = iopath(NULL, "tags", ElvTrue);

			/* process the following tag files until we get tags */
			while (tmp && !taglist)
			{
				tsfile(tmp, o_taglength);
				tmp = iopath(NULL, "tags", ElvTrue);
			}

			/* if we found tags, then remember the tags file name */
			if (tmp)
			{
				safefree(tfilename);
				tfilename = safedup(tmp);
			}
		}

		/* return the next matching tag, if any */
		newtag = ElvFalse;
		goto Finish;
	}
	newtag = ElvTrue;

	/* if no tag given on command line, then use o_previoustag */
	if (!select || !*select)
	{
		if (!o_previoustag)
		{
			msg(MSG_ERROR, "no previous tag");
			sametag = ElvFalse;
			return NULL;
		}
		select = o_previoustag;
	}
	else
	{
		/* tag given... or maybe it is a selection expression.
		 * Wipe out the old value of previoustag */
		if (o_previoustag)
			safefree(o_previoustag);
		o_previoustag = NULL;

		/* Determine whether we're given a tag or a more complex
		 * restriction expression.
		 */
		for (scan = select; *scan && *scan != ':' && !elvspace(*scan); scan++)
			if (*scan == '\\' && scan[1])
				scan++;

		/* If given a simple tag, then remember it as the value of
		 * o_previoustag.  Strip out any backslashes used as quotes.
		 *
		 * Note: If we find any matching tags, then the value set here
		 * will be overridden by the name of the found tag, so really
		 * the value we set here only matters if there is no matching
		 * tag.
		 */
		if (!*scan)
		{
			for (scan = select; *scan; scan++)
			{
				if (*scan == '\\' && scan[1])
					scan++;
				buildCHAR(&o_previoustag, *scan);
			}
		}
	}

	/* Previous tag search (if any) was apparently successful */
	if (taglist)
		tsadjust(taglist, '+');

	/* Delete all tags from previous search */
	tagdelete(ElvTrue);

	/* using internal tag search, or external? */
	if (o_tagprg && *o_tagprg)
	{
		/* external tag search */

		/* Wipe out the set of restrictions */
		tsreset();
		tmp = tochar8(calculate(toCHAR("file:+(filename)"),NULL, CALC_MSG));
		assert(tmp);
		tsparse(tmp);

		/* evaluate the tagprg string with $1 set to the args */
		args[0] = select;
		args[1] = NULL;
		scan = calculate(o_tagprg, args, CALC_MSG);
		if (!scan)
			goto Finish;

		/* add a "!" to the front of the command string, so tsfile
		 * will know it is a command and not a weird file name.
		 */
		tmp = (char *)safealloc(CHARlen(scan) + 2, sizeof(char));
		tmp[0] = '!';
		for (i = 1; *scan; i++, scan++)
			tmp[i] = *scan;

		/* read tags from the program */
		tsfile(tochar8(tmp), o_taglength);
		safefree(tmp);

		/* wipe out the tag file name */
		if (tfilename)
			safefree(tfilename);
		tfilename = NULL;
	}
	else
	{
		/* internal tag search */

		/* Build a new set of restrictions */
		tsreset();
		tsparse(tochar8(select));
		tmp = tochar8(calculate(toCHAR("file:+(filename)"),NULL, CALC_MSG));
		assert(tmp);
		tsparse(tmp);

		/* locate the first set of tags */
		for (tmp = iopath(tochar8(o_tags), "tags", ElvTrue);
		     tmp && !taglist;
		     tmp = iopath(NULL, "tags", ElvTrue))
		{
			tsfile(tmp, o_taglength);
		}

		/* remember the tag file name */
		if (tfilename)
			safefree(tfilename);
		tfilename = (tmp ? safedup(tmp) : NULL);
	}

Finish:
	/* return the first matching tag, if any */
	if (!taglist)
		msg(MSG_ERROR, newtag ? "no matching tags" : "no more matching tags");
	else if (!o_previoustag || CHARcmp(o_previoustag, toCHAR(taglist->TAGNAME)))
	{
		if (o_previoustag)
			safefree(o_previoustag);
		o_previoustag = CHARdup(toCHAR(taglist->TAGNAME));
	}
	sametag = ElvFalse;
	return taglist;
}


#ifdef FEATURE_BROWSE
/* Build a browser document for a given set of restrictions */
BUFFER tebrowse(all, select)
	ELVBOOL	all;		/* scan all tags files? (else only first) */
	CHAR	*select;	/* the restrictions, from command line */
{
	BUFFER	buf;		/* the buffer containing the new browser file */
	MARKBUF	from, to;	/* positions in the buffer */
	char	*proto;		/* name of tags file or prototype file */
	CHAR	*item;		/* format of a single item, from prototype */
	CHAR	*address;	/* the tagaddress of a tag, converted to HTML */
	CHAR	*url;		/* the URL of a tag */
	CHAR	*args[5];	/* tagname, tagfile, tagaddress, url, NULL */
	CHAR	*dflt = toCHAR("<ul>\n\n<li><a href=\"$4\">$1</a> $2, $3\n\n</ul>\n");
	CHAR	*cp;
	CHAR	prev, prev2;
	long	qty;
	CHAR	qtystr[20];
	TAG	*qtytag;
	char	*tmp;
	int	i;

	/* forget any old tag info */
	tagdelete(ElvTrue);
	tsreset();

	/* default args are none */
	if (!select)
		select = toCHAR("");

	if (o_tagprg && *o_tagprg)
	{
		/* external tag search */

		/* make an HTML copy of the string */
		cp = (CHAR *)safealloc(CHARlen(select) + 8, sizeof(CHAR));
		CHARcpy(cp, toCHAR("Browse "));
		CHARcat(cp, select);

		/* evaluate the tagprg string with $1 set to the args */
		args[0] = select;
		args[1] = NULL;
		select = cp;
		cp = calculate(o_tagprg, args, CALC_MSG);
		if (!cp)
			return NULL;

		/* add a "!" to the front of the command string, so tsfile
		 * will know it is a command and not a weird file name.
		 */
		tmp = (char *)safealloc(CHARlen(cp) + 2, sizeof(char));
		tmp[0] = '!';
		for (i = 1; *cp; i++, cp++)
			tmp[i] = *cp;

		/* read tags from the program */
		tsfile(tochar8(tmp), o_taglength);
		safefree(tmp);
	}
	else
	{
		/* internal tag search */

		/* parse the restrictions & make an HTML copy of the string */
		cp = (CHAR *)safealloc(CHARlen(select) + 8, sizeof(CHAR));
		CHARcpy(cp, toCHAR("Browse "));
		CHARcat(cp, select);
		tsparse(tochar8(select));
		select = cp;

		/* build the tags list */
		for (proto = iopath(tochar8(o_tags), "tags", ElvTrue);
		     proto && (!taglist || all);
		     proto = iopath(NULL, "tags", ElvTrue))
		{
			tsfile(proto, o_taglength);
		}
	}

	/* if no tags, then fail */
	if (!taglist)
	{
		return NULL;
	}

	/* count the tags */
	for (qty = 0L, qtytag = taglist; qtytag; qty++, qtytag = qtytag->next)
	{
	}
	sprintf(tochar8(qtystr), "%ld", qty);

	/* Create a buffer to hold the browser document.  If possible, load
	 * the document format from a file named "elvis.bro"
	 */
	proto = iopath(tochar8(o_elvispath), BROWSER_FILE, ElvFalse);
	if (proto)
		buf = bufload(select, proto, ElvTrue);
	else
		buf = bufalloc(select, 0, ElvFalse);

	/* Set some of the buffer's options */
	o_initialsyntax(buf) = ElvFalse;
	o_readonly(buf) = ElvTrue;
	if (o_filename(buf))
	{
		if (optflags(o_filename(buf)) & OPT_FREE)
			safefree(o_filename(buf));
		o_filename(buf) = NULL;
	}
	if (o_bufdisplay(buf))
	{
		if (optflags(o_bufdisplay(buf)) & OPT_FREE)
			safefree(o_bufdisplay(buf));
		optflags(o_bufdisplay(buf)) &= ~OPT_FREE;
	}
	o_bufdisplay(buf) = toCHAR("html");
	if (o_mapmode(buf))
	{
		if (optflags(o_mapmode(buf)) & OPT_FREE)
			safefree(o_mapmode(buf));
		optflags(o_mapmode(buf)) &= ~OPT_FREE;
	}
	o_mapmode(buf) = toCHAR("html");

	/* if no format was read from "elvis.bro" then use the default */
	if (o_bufchars(buf) == 0L)
		bufreplace(marktmp(from, buf, 0L), &from, dflt, CHARlen(dflt));

	/* Parse the document; i.e., locate the item section, copy it into RAM,
	 * and then delete it from the buffer.  The item section is delimited
	 * by blank lines.
	 */
	to = from;
	for (scanalloc(&cp, marktmp(from, buf, 0L)), prev = prev2 = '\0';
	     cp;
	     prev2 = prev, prev = *cp, scannext(&cp))
	{
		/* watch for multiple-newlines */
		if (prev2 == '\n' && prev == '\n' && *cp != '\n')
		{
			if (markoffset(&from) == 0L)
				from = *scanmark(&cp);
			else
				to = *scanmark(&cp);
		}

		/* watch for $1 or $2 in the header */
		if (markoffset(&from) == 0 && prev2 == '$' && (prev == '1' || prev == '2'))
		{
			to = *scanmark(&cp);
			scanfree(&cp);
			from = to;
			markaddoffset(&from, -2);
			switch (prev)
			{
			  case '1':
				args[0] = select;
				args[1] = NULL;
				cp = calculate(toCHAR("htmlsafe($1)"), args, CALC_MSG);
				break;

			  case '2':
				cp = qtystr;
				break;
			}
			bufreplace(&from, &to, cp, CHARlen(cp));
			markaddoffset(&from, CHARlen(cp));
			scanalloc(&cp, &from);
			marksetoffset(&from, 0);
			to = from;
		}
	}
	scanfree(&cp);
	if (markoffset(&to) == 0L)
	{
		msg(MSG_ERROR, "bad elvis.bdf");
		return NULL;
	}
	markaddoffset(&to, -1);
	item = bufmemory(&from, &to);
	markaddoffset(&from, -1);
	markaddoffset(&to, 1);
	bufreplace(&from, &to, NULL, 0L);

	/* for each tag in the list... */
	for ( ; taglist; tagdelete(ElvFalse))
	{
		/* Convert the address to a plaintext line */
		if (elvdigit(*taglist->TAGADDR))
		{
			/* line number -- make sure it isn't JUST a number! */
			address = NULL;
			buildstr(&address, "(line ");
			buildstr(&address, taglist->TAGADDR);
			buildCHAR(&address, ')');
		}
		else
		{
			/* strip /^ and $/, along with any backslashes */
			for (address = NULL, proto = taglist->TAGADDR + 2;
			     proto[2];
			     proto++)
			{
				if (*proto == '\\' && proto[1])
					proto++;
				buildCHAR(&address, *proto);
			}
			if (!address)
				address = (CHAR *)safealloc(1, sizeof(CHAR));
		}

		/* Generate an URL.  Note that the tagaddress is URL-encoded */
		url = NULL;
		buildstr(&url, taglist->TAGFILE);
		buildCHAR(&url, '?');
		for (cp = toCHAR(taglist->TAGADDR); *cp; cp++)
		{
			switch (*cp)
			{
			  case '\t':	buildstr(&url, "%09");	break;
			  case '+':	buildstr(&url, "%2B");	break;
			  case '"':	buildstr(&url, "%22");	break;
			  case '%':	buildstr(&url, "%25");	break;
			  case '<':	buildstr(&url, "%3C");	break;
			  case '>':	buildstr(&url, "%3E");	break;
			  case ' ':	buildCHAR(&url, '+');	break;
			  default:	buildCHAR(&url, *cp);
			}
		}

		/* evaluate the item line with this tag's values */
		args[0] = toCHAR(taglist->TAGNAME);
		args[1] = toCHAR(taglist->TAGFILE);
		args[2] = address;
		args[3] = url;
		args[4] = NULL;
		cp = calculate(item, args, CALC_MSG);
		if (!cp)
			cp = item; /* error -- but give user a clue */

		/* stuff the tag into the document */
		bufreplace(&from, &from, cp, CHARlen(cp));
		markaddoffset(&from, CHARlen(cp));

		/* free the temporary stuff */
		safefree(address);
		safefree(url);
	}

	/* turn off the "modified" flag. */
	o_modified(buf) = ElvFalse;
	buf->docursor = 0L;

	/* return the buffer */
	return buf;
}
#endif /* FEATURE_BROWSE */

#ifdef DISPLAY_SYNTAX
/* Scan a tags file for any tags which are callable from the current language
 * (i.e., the most recently loaded via descr_open(), not descr_recall()), and
 * add them to a dictionary.  The tag's name is the word that gets added.  The
 * word's flags are set to denote the font; the font name is derived by adding
 * a prefix to the value of the tag's "kind".  For example, if prefix="lib"
 * and we're adding a function named "foo", then the dictionary will receive
 * the word "foo" with font "libf".
 *
 * Returns the new root of the spell dictionary.
 */
spell_t *telibrary(tagfile, dict, ignorecase, prefix)
	char	*tagfile;	/* name of the tags file */
	spell_t	*dict;		/* dictionary to which tags are added */
	ELVBOOL	ignorecase;	/* convert to lowercase before adding? */
	CHAR	*prefix;	/* prefix for font names */
{
	CHAR	tagline[1000];	/* input buffer */
	ELVBOOL	allnext;	/* does tagline[] contain the whole next line?*/
	int	bytes;		/* number of bytes in tagline */
	CHAR	*src, *dst;	/* for manipulating tagline[] */
	TAG	*tag;		/* a tag parsed from tagline[] */
	ELVBOOL	inside;		/* is tag's scope limited? */
	CHAR	fontname[50];
	CHAR	descr[30];
	int	font;
	char	prevkind[50];
	long	flags;
	int	genericfont, letterfont[26];
	int	i;

	/* open the file */
	if (!ioopen(tagfile, 'r', ElvFalse, ElvFalse, 't'))
		return dict;

	/* Arrange certain parameters to always appear in the same elements of
	 * the tag->attr[] array.
	 */
#define TEKIND	attr[3]
#define TEFILE	attr[4]
#define TEFIRST	6
	tagnamereset();
	CHARcpy(tagline, toCHAR("x\tx\t1;\"\tkind:x\tfile:x\tln:x\tenum:x"));
	tagparse(tochar8(tagline));

	/* create a generic font name */
	prevkind[0] = '\0';
	font = genericfont = colorfind(prefix);
	CHARcpy(descr, toCHAR("like normal"));
	if (genericfont)
	{
		colorset(genericfont, descr, ElvFalse);

		/* after this, others should default to be "like" this font */
		CHARcpy(descr, toCHAR("like "));
		CHARcat(descr, prefix);
	}
	flags = genericfont << 8;

	/* single-letter fonts aren't allocated yet */
	memset(letterfont, 0, sizeof letterfont);

	/* For each line from the tags file */
	bytes = ioread(tagline, QTY(tagline) - 1);
	while (bytes > 5) /* shortest possible legal tag line */
	{
		/* find the end of this line */
		for (src = tagline; src < &tagline[bytes] && *src != '\n'; src++)
		{
		}

		/* parse it */
		*src = '\0';
		tag = tagparse(tochar8(tagline));
		if (!tag)
			break;

		/* see if the tag's scope is limited, unless it's a function */
		inside = ElvFalse;
		if (!tag->TEKIND || tag->TEKIND[0] != 'f' || tag->TEKIND[1])
			for (i = TEFIRST; i < QTY(tag->attr); i++)
				if (tag->attr[i])
				{
					inside = ElvTrue;
					break;
				}

		/* if not static, not a non-function limited to some other
		 * scope, and not already in dictionary, and is defined in a
		 * file which is callable by this one...
		 */
		if (!tag->TEFILE
		 && !inside
		 && !SPELL_IS_GOOD(spellfindword(dict, toCHAR(tag->TAGNAME), 0))
		 && descr_cancall(toCHAR(tag->TAGFILE)))
		{
			/* some easy cases: no "kind" then use generic */
			if (!tag->TEKIND)
				flags = genericfont << 8;
			/* single-letter "kind" and letterfont is set, use it */
			else if (tag->TEKIND[0] >= 'a'
			      && tag->TEKIND[0] <= 'z'
			      && tag->TEKIND[1] == '\0'
			      && letterfont[tag->TEKIND[0] - 'a'] != 0)
				flags = letterfont[tag->TEKIND[0] - 'a'] << 8;
			/* same font as last time */
			else if (!strcmp(prevkind, tag->TEKIND))
				flags = font << 8;
			else
			{
				/* no special cases, just do it the hard way */
				strcpy(prevkind, tag->TEKIND);
				CHARcpy(fontname, prefix);
				CHARcat(fontname, toCHAR(prevkind));
				font = colorfind(fontname);
				if (font)
					colorset(font, descr, ElvFalse);

				/* if single-letter, remember it */
				if (tag->TEKIND[0] >= 'a'
				 && tag->TEKIND[0] <= 'z'
				 && tag->TEKIND[1] == '\0')
					letterfont[tag->TEKIND[0] - 'a'] = font;

				/* set the flags to use this font */
				flags = font << 8;
			}

			/* if ignorecase, then force the name to be lowercase */
			if (ignorecase)
			{
				char *conv;
				for (conv = tag->TAGNAME; *conv; conv++)
					*conv = elvtolower(*conv);
			}

			/* add the word, with the proper font flags */
			dict = spelladdword(dict, toCHAR(tag->TAGNAME), flags);
		}

		/* delete this line from tagline[] */
		for (dst = tagline, src++, allnext = ElvFalse; src < &tagline[bytes]; )
		{
			if (*src == '\n')
				allnext = ElvTrue;
			*dst++ = *src++;
		}
		bytes = (int)(dst - tagline);

		/* if the next line is incomplete, read some more text
		 * from the tags file.
		 */
		if (!allnext)
		{
			bytes += ioread(dst, (int)QTY(tagline) - bytes - 1);
		}
	}
	(void)ioclose();
	return dict;
}
#endif /* DISPLAY_SYNTAX */


#ifdef FEATURE_SHOWTAG
/* build a list of all top-level tags defined in this buffer */
void tebuilddef(buf)
	BUFFER	buf;
{
   /* for building the new tagdef array */
	TEDEF	*tagdef;	/* the new tagdef array */
	TEDEF	*bigger;	/* used while enlarging tagdef */
	int	allocated;	/* number of items allocated for tagdef */
	int	ntagdefs;	/* number of items in tagdef */
   /* for scanning the tags file... */
	CHAR	tagline[1000];	/* input buffer */
	ELVBOOL	allnext;	/* does tagline[] contain the whole next line?*/
	int	bytes;		/* number of bytes in tagline */
	CHAR	*src, *dst;	/* for manipulating tagline[] */
	TAG	*tag;		/* a tag parsed from tagline[] */
    /* for locating a tag defintion within this buffer */
	EXINFO	xinfb;		/* dummy ex command, for parsing tag address */
	ELVBOOL	wasmagic;	/* stores the normal value of o_magic */
	ELVBOOL wassaveregexp;	/* stores the normal value of o_saveregexp */
	ELVBOOL	wasmsghide;	/* stores the msghide() flag */
	CHAR	*cp;		/* for scanning the line */
	long	offset;		/* offset of the tag within this buffer */
	int	i;

	/* Destroy the old list, if any */
	tefreedef(buf);

	/* if the show option doesn't contain "tag", then do nothing more */
	if (!o_show)
		return;
	for (src = o_show; src && *src; src++)
		if (CHARncmp(src, "tag", 3))
			break;
	if (!*src)
		return;

	/* if this buffer contains no file, then do nothing */
	if (!o_filename(buf))
		return;

	/* Scan the "tags" file.  Note that we're using the lower-level
	 * tag reading functions for a couple of reasons: speed of course,
	 * but also because we don't want to clobber any existing tag list.
	 * That's important because tebuilddef() will be called whenever
	 * :tag causes a file to be loaded, and we want to keep the remainder
	 * of that tag list in case we just loaded the wrong one.
	 */

	/* If there is no tags file, then do nothing.  This check is
	 * necessary because the ioopen() function displays an error
	 * message when the file it's trying to read doesn't exist.
	 */
	if (dirperm("tags") == DIR_NEW)
		return;

	/* open the file */
	if (!ioopen("tags", 'r', ElvFalse, ElvFalse, 't'))
		return;

	/* For each line from the tags file */
	ntagdefs = allocated = 0;
	tagdef = NULL;
	wasmsghide = msghide(ElvTrue);
	bytes = ioread(tagline, QTY(tagline) - 1);
	while (bytes > 5) /* shortest possible legal tag line */
	{
		/* find the end of this line */
		for (src = tagline; src < &tagline[bytes] && *src != '\n'; src++)
		{
		}

		/* parse it */
		*src = '\0';
		tag = tagparse(tochar8(tagline));
		if (!tag)
			break;

		/* if for this file, and its definition isn't indented... */
		if (!strcmp(tag->TAGFILE, tochar8(o_filename(buf)))
		 && (elvdigit(tag->TAGADDR[0]) || !elvspace(tag->TAGADDR[2])))
		{
			/* if the tag has a "ln" attribute, start searching
			 * there -- saves *a lot* of time.
			 */
			memset((char *)&xinfb, 0, sizeof xinfb);
			(void)marktmp(xinfb.defaddr, buf, 0);
			for (i = 3; i < MAXATTR && tagattrname[i] && strcmp(tagattrname[i], "ln"); i++)
			{
			}
			if (i < MAXATTR && tag->attr[i] && (offset = atol(tag->attr[i])) > 1L)
				(void)marksetline(&xinfb.defaddr, offset - 1);

			/* locate the tag's definition within this buffer */
			scanstring(&cp, toCHAR(tag->TAGADDR));
			wasmagic = o_magic;
			o_magic = ElvFalse;
			wassaveregexp = o_saveregexp;
			o_saveregexp = ElvFalse;
			if (!exparseaddress(&cp, &xinfb))
			{
				scanfree(&cp);
				o_magic = wasmagic;
				o_saveregexp = wassaveregexp;
				goto NotFound;
			}
			scanfree(&cp);
			o_magic = wasmagic;
			o_saveregexp = wassaveregexp;
			offset = lowline(bufbufinfo(buf), xinfb.to);
			exfree(&xinfb);

			/* enlarge tagdef[] if necessary */
			if (ntagdefs + 1 > allocated)
			{
				allocated += 50;
				bigger = (TEDEF *)safealloc(allocated, sizeof(TEDEF));
				if (tagdef)
				{
					memcpy(bigger, tagdef, ntagdefs * sizeof(TEDEF));
					safefree(tagdef);
				}
				tagdef = bigger;
			}

			/* insert this tag into the array, sorted by offset */
			for (i = ntagdefs;
			     i > 0 && markoffset(tagdef[i - 1].where) > offset;
			     i--)
			{
					tagdef[i] = tagdef[i - 1];
			}
			tagdef[i].where = markalloc(buf, offset);
			tagdef[i].label = NULL;
			buildstr(&tagdef[i].label, tag->TAGNAME);
			ntagdefs++;
		}
NotFound:

		/* delete this line from tagline[] */
		for (dst = tagline, src++, allnext = ElvFalse; src < &tagline[bytes]; )
		{
			if (*src == '\n')
				allnext = ElvTrue;
			*dst++ = *src++;
		}
		bytes = (int)(dst - tagline);

		/* if the next line is incomplete, read some more text
		 * from the tags file.
		 */
		if (!allnext)
		{
			bytes += ioread(dst, (int)QTY(tagline) - bytes - 1);
		}
	}
	(void)ioclose();
	msghide(wasmsghide);

	/* store the list */
	buf->tagdef = tagdef;
	buf->ntagdefs = ntagdefs;
	safeinspect();
}



/* free the tag definition info for a given buffer */
void tefreedef(buf)
	BUFFER	buf;
{
	int	i;

	safeinspect();
	if (buf->tagdef)
	{
		for (i = 0; i < buf->ntagdefs; i++)
		{
			markfree(buf->tagdef[i].where);
			safefree(buf->tagdef[i].label);
		}
		safefree((void *)buf->tagdef);
		buf->tagdef = NULL;
		buf->ntagdefs = 0;
	}
	safeinspect();
}



/* return the info about the tag that is defined at the cursor location. */
CHAR *telabel(cursor)
	MARK	cursor;
{
	int	i;
 static	CHAR	noinfo[1];
	TEDEF	*tagdef = markbuffer(cursor)->tagdef;

	/* if buffer has no tags, then return no info */
	if (!tagdef)
		return noinfo;

	/* search for this MARK in the list */
	for (i = 0; i < markbuffer(cursor)->ntagdefs; i++)
	{
		if (markoffset(tagdef[i].where) > markoffset(cursor))
			break;
	}

	/* report what (if anything) we found */
	if (i == 0)
		return noinfo;
	else
		return tagdef[i - 1].label;
}
#endif /* defined(FEATURE_SHOWTAG) */

#ifdef FEATURE_COMPLETE
/* This function is used for completing a tag name.  It searches backward
 * from the provided cursor position to collect the characters of a partial
 * tag name, and then it looks for any known tags whose name matches that
 * partial name.  It returns a string containing any new characters that it
 * could match.
 */
CHAR *tagcomplete(win, m)
	WINDOW	win;	/* the window where multiple matches are listed */
	MARK	m;	/* the cursor position (end of partial name) */
{
	char	rest[300];
 static CHAR	retbuf[100];
	int	plen;		/* length of partial string */
	CHAR	*cp;
	int	mlen;
	TAG	*tag, *scan;
	long	oldtaglength;
	TAG	*oldtaglist;
	ELVBOOL	oldmsghide;
	ELVBOOL	oldexrefresh;
	CHAR	*oldprevioustag;
	DRAWSTATE olddrawstate;

	/* Ignore the inputtab=identifier setting unless there is a "tags"
	 * file in the current directory.
	 */
	if (o_inputtab(markbuffer(win->cursor)) == 'i'
	 && dirperm("tags") < DIR_READONLY)
	{
		/* return an ordinary tab character */
		retbuf[0] = '\t';
		retbuf[1] = '\0';
		return retbuf;
	}

	/* collect the characters of the partial name */
	rest[0] = '\0';
	for (scanalloc(&cp, m), plen = 0;
	     scanprev(&cp) && (elvalnum(*cp) || *cp == '_') && plen < QTY(rest) - 1;
	     )
	{
		memmove(rest + 1, rest, QTY(rest) - 1);
		rest[0] = *cp;
		plen++;
	}
	scanfree(&cp);

	/* if no text, or a keyword, then just return a tab character */
	if (plen == 0 || dmskeyword(win, toCHAR(rest)))
	{
		retbuf[0] = '\t';
		retbuf[1] = '\0';
		return retbuf;
	}

	/* find the matching tags */
	oldtaglist = taglist;
	taglist = NULL;
	oldtaglength = o_taglength;
	oldmsghide = msghide(ElvTrue);
	oldprevioustag = o_previoustag ? CHARdup(o_previoustag) : NULL;
	o_previoustag = NULL;
	o_taglength = plen;
	if (o_filename(markbuffer(m))
	 && strlen(rest) + 2 + CHARlen(o_filename(markbuffer(m))) < QTY(rest))
		sprintf(rest + strlen(rest), " file:%s", tochar8(o_filename(markbuffer(m))) );
	tag = tetag(toCHAR(rest));
	taglist = oldtaglist;
	msghide(oldmsghide);
	o_taglength = oldtaglength;
	if (o_previoustag)
		safefree(o_previoustag);
	o_previoustag = oldprevioustag;

	/* if no matches, then return a space */
	if (!tag)
	{
		CHARcpy(retbuf, toCHAR(" "));
		if (plen == 0)
			*retbuf = '\0';
		return retbuf;
	}

	/* eliminate duplicates */
	for (scan = tag; scan->next; )
	{
		if (!strcmp(scan->TAGNAME, scan->next->TAGNAME))
			scan->next = tagfree(scan->next);
		else
			scan = scan->next;
	}

	/* if only one match, then return the remainder of its name plus
	 * a space.
	 */
	if (!tag->next)
	{
		CHARcpy(retbuf, toCHAR(tag->TAGNAME + plen));
		CHARcat(retbuf, toCHAR(" "));
		while (tag)
			tag = tagfree(tag);
		return retbuf;
	}

	/* We have multiple matches.  Can we add any characters? */
	mlen = strlen(tag->TAGNAME);
	for (scan = tag->next; scan; scan = scan->next)
	{
		while (strncmp(tag->TAGNAME, scan->TAGNAME, mlen) != 0)
			mlen--;
	}

	/* If we can add some chars then do so */
	if (mlen > plen)
	{
		CHARncpy(retbuf, toCHAR(tag->TAGNAME + plen), mlen - plen);
		retbuf[mlen - plen] = '\0';
		while (tag)
			tag = tagfree(tag);
		return retbuf;
	}

	/* Else list all matches */
	plen = 0;
	olddrawstate = win->di->drawstate;
	for (scan = tag; scan; scan = scan->next)
	{
		mlen = strlen(scan->TAGNAME);
		if (plen + mlen + 1 >= o_columns(win))
		{
			drawextext(win, toCHAR("\n"), 1);
			plen = 0;
			olddrawstate = win->di->drawstate;
		}
		else if (plen > 0)
		{
			drawextext(win, blanks, 1);
			plen++;
		}
		drawextext(win, toCHAR(scan->TAGNAME), mlen);
		plen += mlen;
	}

	/* complete the last output line.  Note that we try to do this in
	 * a clever way which avoids prompting the user to "Hit <Enter> to
	 * continue" if the whole list fits on a single line.
	 */
	if (olddrawstate == DRAW_VISUAL)
	{
		oldexrefresh = o_exrefresh;
		o_exrefresh = ElvTrue;
		drawextext(win, retbuf, 0);
		o_exrefresh = oldexrefresh;
		win->di->drawstate = DRAW_VMSG;
	}
	else
		drawextext(win, toCHAR("\n"), 1);

	/* we weren't able to extend the partial name at all */
	*retbuf = '\0';
	return retbuf;
}
#endif

#endif /* FEATURE_TAGS */

/* Save the current cursor position on the tag stack.  */
void tepush(win, label)
	WINDOW	win;	/* window where push should occur */
	CHAR	*label;	/* dynamically-allocated name of old position */
{
	int	i;

	if (o_tagstack
	 && newtag
	 && (o_filename(markbuffer(win->cursor))
		|| o_bufchars(markbuffer(win->cursor))))
	{
		/* The oldest tag will be lost.  If it had pointers to any
		 * dynamically allocated memory, then free that memory now.
		 */
		if (win->tagstack[TAGSTK - 1].prevtag)
			safefree(win->tagstack[TAGSTK - 1].prevtag);
		if (win->tagstack[TAGSTK - 1].origin)
			markfree(win->tagstack[TAGSTK - 1].origin);

		/* Shift the tag stack; top is always win->tagstack[0] */
		for (i = TAGSTK - 1; i > 0; i--)
		{
			win->tagstack[i] = win->tagstack[i - 1];
		}

		/* insert data into the top slot */
		win->tagstack[0].origin = markdup(win->cursor);
		win->tagstack[0].display = win->md->name;
		win->tagstack[0].prevtag = label;
	}
	else
	{
		safefree(label);
	}

	/* always leave newtag set to "true" */
	newtag = ElvTrue;
}
