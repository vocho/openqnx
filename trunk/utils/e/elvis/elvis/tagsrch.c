/* tagsrch.c */

/* Elvis uses this file to scan a tags file, and built a list of the matching
 * tags, sorted by name and likelyhood that they're the intended tag.
 *
 * Entry points are:
 *   void tsreset()		forget old restrictions
 *   void tsparse(text)		add new restrictions
 *   void tsadjust(tag, oper)	adjust likelyhood heuristic data
 *   void tsfile(filename)	scan a file for tags, add to taglist
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_tagsrch[] = "$Id: tagsrch.c,v 1.16 2003/10/17 17:41:23 steve Exp $";
#endif
#ifdef FEATURE_TAGS

#define WEIGHT_SUCCESS	100
#define WEIGHT_FAIL	95
#define WEIGHT_AGING	10

/* These structures are used for storing a list of acceptable values for a
 * particular attribute.
 */
typedef struct value_s
{
	struct value_s	*next;		/* another possible value */
	char		*value;		/* attribute's possible value */
} value_t;


/* These structures are used for storing a list of restrictive names, and
 * their possible values.
 */
typedef struct name_s
{
	struct name_s	*next;		/* another restriction */
	char		*name;		/* attribute's name */
	long		weight;		/* 1=required, 0=optional */
	value_t		*values;	/* list of possible values */
} name_t;


#if USE_PROTOTYPES
static name_t *addrestrict(char *nametext, char *valuetext, _char_ oper);
static long likelyhood(TAG *tag, name_t *head, name_t *map[]);
static name_t *age(name_t *head);
static ELVBOOL chkrestrict(TAG *tag);

#endif /* USE PROTOTYPES */

/* These are the heads of a three of lists: one for restrictions, one for
 * attributes of recent successful searches, and one for attributes of recent
 * failed searches.  Also, there are map tables that allow a given name to be
 * found quickly via its attribute index in the current tags file.
 */
static name_t	*rhead, *rmap[MAXATTR];	/* restrictions */
static name_t	*shead, *smap[MAXATTR];	/* attributes from succeeded searches */
static name_t	*fhead, *fmap[MAXATTR];	/* attributes from failed searches */
static int	nmandatory;		/* number of mandatory restrictions */
#define NO_NAME	(((name_t *)0) + 1)


/* These store the first and last tagnames that we care about, sorted in
 * ASCII order like the tags file.  This helps us quickly skip tags that we
 * don't care about.  If there is no tagname restriction, these are NULL.
 * The longname option stores the length of the longest name.
 */
static char	*firstname, *lastname;
static int	longname;
static int	taglength;
static ELVBOOL	fulllength;


/* This function adds a name/value pair to the list of restrictions, and
 * returns a pointer to the name_t record.  It chooses which list to update
 * based on the operator.
 */
static name_t *addrestrict(nametext, valuetext, oper)
	char	*nametext;	/* text form of restrictive name */
	char	*valuetext;	/* text form of possible value, or NULL */
	_char_	oper;		/* one of {= : + -} from command line */
{
	name_t	**list;		/* the list to search */
	name_t	*name, *namelag;/* for scanning the names list */
	value_t	*value, *vlag;	/* for scanning the values list */
	long	i;
	int	len;

	/* choose a list */
	switch (oper)
	{
	  case '+':	list = &shead;	break;	/* the succeeded attributes */
	  case '-':	list = &fhead;	break;	/* the failed attributed */
	  default:	list = &rhead;		/* the restrictions */
	}

	/* search for the name in the list.  Add it if new */
	for (namelag = NULL, name = *list;
	     name && strcmp(nametext, name->name);
	     namelag = name, name = name->next)
	{
	}
	if (!name)
	{
		name = (name_t *)safealloc(1, sizeof(name_t));
		name->name = safedup(nametext);
		name->weight = 0;
		if (namelag)
			namelag->next = name;
		else
			*list = name;
	}

	/* insert the value into the list of acceptable values */
	if (valuetext)
	{
		value = (value_t *)safealloc(1, sizeof(value_t));
		value->value = safedup(valuetext);
		value->next = name->values;
		name->values = value;
	}

	/* adjust the weight */
	switch (oper)
	{
	  case '+':
	  case '-':
		if (valuetext)
		{
			name->weight = (oper == '+') ? WEIGHT_SUCCESS : WEIGHT_FAIL;
		}

		/* discard any older values which have no weight */
		for (i = name->weight, vlag = NULL, value = name->values;
		     i > 0 && value;
		     i -= WEIGHT_AGING, vlag = value, value = value->next)
		{
		}
		if (vlag)
			vlag->next = NULL;
		else
			name->values = NULL;
		while (value)
		{
			vlag = value->next;
			safefree(value->value);
			safefree(value);
			value = vlag;
		}
		break;

	  case ':':
		name->weight = 0;	/* optional */
		break;

	  case '/':
		name->weight = 2;	/* optional, but counts against mandatory */
		break;

	  case '=':
		name->weight = 1;	/* required */
		for (nmandatory = 0, namelag = rhead;
		     namelag;
		     namelag = namelag->next)
		{
			if (namelag->weight == 1)
				nmandatory++;
		}
		break;
	}

	/* if adding to the tagname attribute, then update firstname and
	 * lastname variables.
	 */
	if (!strcmp(name->name, "tagname"))
	{
		firstname = lastname = NULL;
		longname = 0;
		for (value = name->values; value; value = value->next)
		{
			if (!firstname || strcmp(firstname, value->value) > 0)
				firstname = value->value;
			if (!lastname || strcmp(lastname, value->value) < 0)
				lastname = value->value;
			len = strlen(value->value);
			if (len > longname)
				longname = len;
		}
	}

	/* return the name_t record of this item */
	return name;
}


/* Assign a likelyhood value to a tag, by comparing its attributes to those
 * of recent successful or failed tag searches.  The tag's total weight can
 * be computed by taking the likelyhood of success minus the likelyhood of
 * failure.
 *
 * In addition to returning the partial likelyhood, this function also updates
 * the smap[] or fmap[] array.
 */
static long likelyhood(tag, head, map)
	TAG	*tag;	/* a tag to check */
	name_t	*head;	/* either shead or fhead */
	name_t	*map[];	/* either smap[] or fmap[] */
{
	long	weight, total;
	name_t	*name;
	value_t	*value;
	int	i;

	/* for all attributes including the standard ones... */
	total = 0;
	for (i = 0; i < MAXATTR && tagattrname[i]; i++)
	{
		/* if this tag lacks this attribute, skip it */
		if (!tag->attr[i])
			continue;

		/* locate the name_t record */
		if (map[i])
			name = map[i];
		else
		{
			for (name = head;
			     name && strcmp(name->name, tagattrname[i]);
			     name = name->next)
			{
			}
			if (!name)
			{
				name = NO_NAME;
			}
			map[i] = name;
		}

		/* if there is no name_t record, ignore this attribute */
		if (name == NO_NAME)
		{
			continue;
		}

		/* try to find the tag's value in recent values */
		for (weight = name->weight, value = name->values;
		     value && *value->value && strcmp(value->value, tag->attr[i]);
		     weight -= WEIGHT_AGING,  value = value->next)
		{
		}
		if (value)
		{
			total += weight;
		}
	}

	/* return the total weight (for either success or failure) */
	return total;
}


/* Age the successful/failed tag search data.  This will eventually cause old
 * data to be freed.  Returns the new head of the list.  Doesn't depend on or
 * update smap[] or fmap[].
 */
static name_t *age(head)
	name_t	*head;	/* either shead or fhead */
{
	name_t	*scan, *lag;
	value_t	*value, *vlag;
	long	weight;

	/* for each named attribute... */
	for (scan = head, lag = NULL; scan; lag = scan, scan = scan->next)
	{
		/* decrement its weight */
		scan->weight -= WEIGHT_AGING;

		/* free any older values which would have zero weight */
		for (weight = scan->weight, value = scan->values, vlag = NULL;
		     value && weight > 0;
		     weight--, vlag = value, value = value->next)
		{
		}
		if (vlag)
			vlag->next = NULL;
		else
			scan->values = NULL;
		while (value)
		{
			vlag = value->next;
			safefree(value->value);
			safefree(value);
			value = vlag;
		}

		/* if this name has no values left, then delete it */
		if (!scan->values)
		{
			if (lag)
				lag->next = scan->next;
			else
				head = scan->next;
			safefree(scan->name);
			safefree(scan);
			if (lag)
				scan = lag;
			else if (head)
				scan = head;
			else
				break;
		}
	}

	/* return the new head */
	return head;
}



/* This function wipes out the restrictions list.  The succeeded and failed
 * attribute lists are unaffected.
 */
void tsreset()
{
	name_t	*nextname;
	value_t	*nextvalue;

	/* for each name... */
	while (rhead)
	{
		/* for each value */
		while (rhead->values)
		{
			/* free the value */
			nextvalue = rhead->values->next;
			safefree(rhead->values->value);
			safefree(rhead->values);
			rhead->values = nextvalue;
		}

		/* free the name */
		nextname = rhead->next;
		safefree(rhead->name);
		safefree(rhead);
		rhead = nextname;
	}

	/* clobber the rmap[] array, too */
	memset(rmap, 0, sizeof rmap);

	/* clobber the firstname and lastname variables */
	firstname = lastname = NULL;
	longname = 0;

	/* clobber the nmandatory variable */
	nmandatory = 0;
}


/* This function parses a restrictions command line.  The command line may have
 * any number of restrictions.  You can call this function repeatedly to combine
 * multiple restrictions lines; You will usually call tsreset() before the first
 * tsparse().
 *
 * A typical input: tsparse("mytag class:=DbItem,DbCustomer file:+myfile.cpp")
 *
 * Supported operators are:
 * 	name:value	Add a value for an optional attribute
 *	name:=value	Add a value for a mandatory attribute
 *	name:/value	Add a value for an optional attribute, but require the
 *			value to be a substring of the tagaddress value
 *	name:+value	Pretend name:value was part of a recent successful tag
 *	name:-value	Pretend name:value was part of a recent failed tag
 * Also, a ',' character can be used to repeat the prevous operator; e.g.,
 * "class:/DbItem,DbCust", "class:/DbItem:/DbCust" are both identical in effect
 * to "class:/DbItem class:/DbCustomer".
 *
 * THE TEXT IS CLOBBERED!
 */
void tsparse(text)
	char	*text;	/* a "name:value name:value" string. */
{
	char	*name;		/* start of a name within the text */
	char	*value;		/* start of a value within the text */
	char	*copy;		/* used while deleting backslashes */
	char	oper;		/* most recent operator character */
	char	nextoper;	/* operator for the next value */

	/* for each word (delimited by whitespace or an operator) ... */
	for (name = NULL, nextoper = oper = ' '; text && *text; oper = nextoper)
	{
		/* skip redundant whitespace */
		while (*text == ' ' || *text == '\t')
		{
			text++;
		}
		if (!*text) break;

		/* value (or maybe name?) starts here */
		value = text;

		/* skip over characters of value */
		for (copy = text; *text && !strchr(":, \t", *text); )
		{
			if (*text == '\\' && text[1])
				text++;
			*copy++ = *text++;
		}

		/* Get the NEXT operator and mark the end of the name.  ',' is
		 * treated as a repeat of previous.
		 */
		if (*text != ',')
			nextoper = *text;
		if (*text)
			*text++ = '\0';
		*copy = '\0';
		if (!nextoper || nextoper == '\t')
			nextoper = ' ';
		if (nextoper == ':' && *text && strchr("=+-/", *text))
			nextoper = *text++;

		/* Use THIS operator to decide how to handle this value */
		switch (oper)
		{
		  case ' ': /* value or name<nextoper>... */
			/* Two possible cases: If the NEXT operator is
			 * whitespace then this value is assumed to be the
			 * value of the "tagname" attribute.  Otherwise, this
			 * value is actually an attribute name which will be
			 * used for later values (up to the next whitespace)
			 */
			if (nextoper == ' ')
				(void)addrestrict("tagname", value, '=');
			else if (!*text)
				(void)addrestrict(value, "", nextoper);
			else
				name = value;
			break;

		  case '/': /* name:/value */
			/* use this value with the previously mentioned name */
			(void)addrestrict(name, value, '/');

			/* also use as a mandatory address substring */
			(void)addrestrict("tagaddress", value, '=');
			break;

		  default: /* name:value name:=value name:+value name:-value */
			/* use this value with the previously mentioned name */
			(void)addrestrict(name, value, oper);
		}
	}
}


/* Check a given tag against the restrictions.  Return ElvTrue if it satisfies. */
static ELVBOOL chkrestrict(tag)
	TAG	*tag;
{
	int	mandcnt;/* number of mandatory restrictions which matched */
	name_t	*name;	/* a "name" struct for a restriction */
	value_t	*value;	/* a "value" struct from within "name"'s list */
	char	*scan;
	int	i;

	/* for each attribute... */
	for (i = mandcnt = 0; i < MAXATTR && tagattrname[i]; i++)
	{
		/* locate the name_t for this attribute.  If there is none,
		 * then ignore this attribute.
		 */
		if (rmap[i] == NO_NAME)
			continue;
		else if (rmap[i])
			name = rmap[i];
		else
		{
			for (name = rhead;
			     name && strcmp(name->name, tagattrname[i]);
			     name = name->next)
			{
			}
			if (!name)
			{
				rmap[i] = NO_NAME;
				continue;
			}
			rmap[i] = name;
		}

		/* skip the TAGADDR attribute until later */
		if (&tag->attr[i] == &tag->TAGADDR)
			continue;

		/* if required and tag doesn't have it, then reject */
		if (tag->attr[i])
			mandcnt += name->weight;
		else if (name->weight == 1)
			return ElvFalse;

		/* if optional and tag doesn't have it, ignore it */
		if (!tag->attr[i])
			continue;

		/* check against all acceptable values */
		if (fulllength || tag->attr[i] != tag->TAGNAME)
		{
			for (value = name->values;
			     value && *value->value && strcmp(value->value, tag->attr[i]);
			     value = value->next)
			{
			}
		}
		else
		{
			for (value = name->values;
			     value && strncmp(value->value, tag->attr[i], (int)taglength);
			     value = value->next)
			{
			}
		}
		if (!value)
			return ElvFalse;
	}

	/* As a special case, if there are tagaddress restrictions, then the
	 * tag's address field must contain one of them as a substring.
	 */
	name = rmap[2];
	if (name != NO_NAME && name && name->values && mandcnt < nmandatory)
	{
		/* for each possible address substring... */
		for (value = name->values; value; value = value->next)
		{
			/* search for the substring within the tag's address */
			i = strlen(value->value);
			if ((int)strlen(tag->TAGADDR) <= i + 2)
				continue;
			for (scan = tag->TAGADDR + 1;
			     scan[i] && 
				(elvalnum(scan[-1])
					|| elvalnum(scan[i])
					|| *scan != *value->value
			    		|| strncmp(scan, value->value, i));
			     scan++)
			{
			}

			/* if found, then stop looking */
			if (scan[i])
				break;
		}

		/* if no substring was found, then reject */
		if (!value)
			return ElvFalse;

		/* If mandatory, count it */
		if (name->weight > 0)
			mandcnt++;
	}

	/* if some mandatory values were missing, reject it */
	if (mandcnt < nmandatory)
		return ElvFalse;

	/* if nothing wrong with it, then it is acceptable */
	return ElvTrue;
}




/* Adjust the histories of failed or successful searches.  Returns the head of
 * the list.  Doesn't depend on or update smap[] or fmap[].
 */
void tsadjust(tag, oper)
	TAG	*tag;	/* a tag that recently succeeded or failed */
	_char_	oper;	/* '+' if succeeded, '-' if failed */
{
	int	i;

	/* age both histories */
	shead = age(shead);
	fhead = age(fhead);

	/* for each attribute, except the standard ones... */
	for (i = 3; i < MAXATTR && tagattrname[i]; i++)
	{
		/* if this tag had such an attribute... */
		if (tag->attr[i])
		{
			/* add it to the history */
			addrestrict(tagattrname[i], tag->attr[i], oper);
		}
	}
}


/* Scan a file for tags which meet the restrictions, and add them to the list */
void tsfile(filename, maxlength)
	char	*filename;	/* name of a file to scan */
	long	maxlength;	/* maximum significant length, or 0 for all */
{
	CHAR	tagline[1000];	/* input buffer */
	ELVBOOL	allnext;	/* does tagline[] contain the whole next line?*/
	int	bytes;		/* number of bytes in tagline */
	CHAR	*src, *dst;	/* for manipulating tagline[] */
	TAG	*tag;		/* a tag parsed from tagline[] */
	ELVBOOL	skipped;	/* have we already skipped as much as possible? */
	int	i;

	/* clobber the rmap[], smap[], and fmap[] arrays */
	memset(rmap, 0, sizeof rmap);
	memset(smap, 0, sizeof smap);
	memset(fmap, 0, sizeof fmap);

	/* clobber the attribute names */
	tagnamereset();

	/* choose a significant length */
	if (maxlength == 0 || maxlength > longname)
		taglength = longname, fulllength = ElvTrue;
	else
		taglength = maxlength, fulllength = ElvFalse;

	/* open the file */
	if (!ioopen(filename, 'r', ElvTrue, ElvFalse, 't'))
	{
		return;
	}

	/* Make a local copy of the filename.  This is important because the
	 * value passed into this function is usually the static buffer used
	 * by the dirpath() function, and we want to use that buffer ourselves
	 * later in this function.
	 */
	filename = safedup(filename);

	/* Compare the tag of each line against the tagname */
	bytes = ioread(tagline, QTY(tagline) - 1);
	skipped = ElvFalse;
	while (bytes > taglength
		&& (!lastname || CHARncmp(lastname, tagline, (size_t)taglength) >= 0))
	{
		/* Except for the first time, we would like to avoid scanning
		 * all of the tag lines currently in the tagline[] buffer if we
		 * know for a fact that the last tag in the buffer is before
		 * the first tag that we care about.  (We can't do the first
		 * block, because at that point we don't know for sure that
		 * the tags are sorted.)
		 */
		if (*tagline != '!' && firstname && !skipped)
		{
			/* Locate the last complete tag name in the buffer */
			for (src = &tagline[bytes], dst = NULL;
			     --src > tagline && (src[-1] != '\n' || !dst);
			     )
			{
				if (*src == '\t')
					dst = src;
			}

			/* Is it before the first one we care about? */
			if (dst && CHARncmp(src, firstname, (int)(dst - src)) < 0)
			{
				/* Yes, so we want to skip as much as possible.
				 * Since we didn't bother to remember whether
				 * we saw a newline before this, or where it
				 * might have belonged, we'll just shift this
				 * tag to the front of tagline[], and fill the
				 * buffer after that.
				 */
				bytes = (int)(&tagline[bytes] - src);
				memmove(tagline, src, bytes * sizeof(CHAR));
				i = ioread(tagline + bytes, (int)QTY(tagline) - bytes - 1);
				bytes += i;

				/* If we managed to read some more text, then
				 * we can loop and repeat the "skip" test with
				 * the new data.  Otherwise (at the end of
				 * the tags file) we must process the tagline
				 * normally, without skipping.
				 */
				if (i > 0)
					continue;
			}
			else
			{
				/* we never want to skip again, in this file */
				skipped = ElvTrue;
			}
		}

		/* disable firstname/lastname checks if tags file claims to
		 * be unsorted.
		 */
		if (lastname && *tagline == '!' && !CHARncmp(tagline, toCHAR("!_TAG_FILE_SORTED\t0\t"), 20))
		{
			lastname = firstname = NULL;
		}

		/* find the end of this line */
		for (src = tagline; src < &tagline[bytes] && *src != '\n'; src++)
		{
		}
		*src = '\0';

		/* if not obviously too early to be of interest, parse it and
		 * process it...
		 */
		if ((!firstname || CHARncmp(firstname, tagline, taglength) <= 0)
			&& (tag = tagparse(tochar8(tagline))) != NULL)
		{
			/* do we want to keep this tag? */
			if (*filename == '!')
			{
				/* Yes! compute its likelyhood factor */
				tag->match = likelyhood(tag, shead, smap)
						- likelyhood(tag, fhead, fmap);
				for (i = 3; i < MAXATTR; i++)
					if (tag->attr[i])
						tag->match++;

				/* save a copy of it */
				(void)tagadd(tagdup(tag));
			}
			else if (chkrestrict(tag))
			{
				/* Yes! compute its likelyhood factor */
				tag->match = likelyhood(tag, shead, smap)
						- likelyhood(tag, fhead, fmap);
				for (i = 3; i < MAXATTR; i++)
					if (tag->attr[i])
						tag->match++;

				/* replace the filename with full pathname */
				tag->TAGFILE = dirpath(dirdir(filename), tag->TAGFILE);
				/* save a copy of it */
				(void)tagadd(tagdup(tag));
			}
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
		if (!allnext || bytes <= taglength)
		{
			bytes += ioread(dst, (int)QTY(tagline) - bytes - 1);
		}
	}
	safefree(filename);
	(void)ioclose();
}
#endif /* FEATURE_TAGS */
