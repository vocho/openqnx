/* tag.c */

/* This file contains functions and variables which are used by all tags
 * programs, including elvis and ctags.
 */

#include "elvis.h"

#ifdef FEATURE_TAGS
# if USE_PROTOTYPES
  static ELVBOOL tagbefore(TAG *t1, TAG *t2);
# endif

/* This array stores the (dynamically allocated) names of attributes. */
char *tagattrname[MAXATTR] = {"tagname", "tagfile", "tagaddress"};

/* This stores the search direction to be used with regular expressions */
ELVBOOL	tagforward;

/* This variable stores a list of tags.  Tags can be added to the list via
 * the tagadd() function, and deleted via tagdelete()
 */
TAG *taglist;

/* Tag comparison function, returns non-zero if t1 should be inserted before t2.
 */
static ELVBOOL tagbefore(t1, t2)
	TAG *t1, *t2;
{
	long cmp;
	char tmptime[20];

	/* First compare them by tag name.  If that doesn't resolve it, then
	 * compare them by how well they match previously accepted tags.  As a
	 * last resort, compare them by timestamp
	 */
	cmp = strcmp(t1->TAGNAME, t2->TAGNAME);
	if (cmp == 0)
	{
		cmp = t2->match - t1->match;
	}
	if (cmp == 0)
	{
		strcpy(tmptime, dirtime(t1->TAGFILE));
		cmp = strcmp(dirtime(t2->TAGFILE), tmptime);
	}

	/* return the comparison result */
	return (ELVBOOL)(cmp < 0);
}

/* This function frees any names in the tagattrname[] array.  It should be
 * called when switching to a different tag file, after the last TAG structure
 * from the previous file has been freed.
 */
void tagnamereset()
{
	int	i;

	for (i = 3; i < MAXATTR && tagattrname[i]; i++)
	{
		safefree(tagattrname[i]);
		tagattrname[i] = NULL;
	}
}

/* This function allocates a TAG structure, and fills in all fields with
 * copies of the attributes from another tag.  The string fields are also
 * dynamically allocated.
 */
TAG *tagdup(tag)
	TAG	*tag;		/* a tag to be duplicated */
{
	TAG	*ret;		/* tag to be returned */
	int	i;

	ret = (TAG *)safealloc(1, sizeof(TAG));
	memset(ret, 0, sizeof(TAG));
	for (i = 0; i < MAXATTR; i++)
	{
		if (tag->attr[i])
		{
			ret->attr[i] = safedup(tag->attr[i]);
		}
	}
	ret->match = tag->match;
	return ret;
}

/* This function adds a named attribute to a tag.  Returns ElvTrue if successful,
 * or ElvFalse if the tag already has too many named attributes.
 */
ELVBOOL tagattr(tag, name, value)
	TAG	*tag;	/* the tag to receive the value */
	char	*name;	/* name of the attribute */
	char	*value;	/* value of the attribute */
{
	int	i;

	/* search for the name in tagattrname[] */
	for (i = 0; i < MAXATTR && tagattrname[i] && strcmp(name, tagattrname[i]); i++)
	{
	}
	if (i >= MAXATTR)
		return ElvFalse;
	if (!tagattrname[i])
		tagattrname[i] = safedup(name);

	/* store the value */
	tag->attr[i] = value;
	return ElvTrue;
}


/* This function frees a tag.  Returns the "next" field from the deleted
 * tag, which can be handy in a loop.
 */
TAG *tagfree(tag)
	TAG	*tag;	/* the tag to be freed */
{
	TAG	*next;
	int	i;

	/* free this tag */
	next = tag->next;
	for (i = 0; i < MAXATTR; i++)
	{
		if (tag->attr[i]) safefree(tag->attr[i]);
	}
	safefree(tag);

	/* return its "next" field */
	return next;
}


/* This function deletes the first tag, or all tags, from the taglist.
 * After the last tag has been deleted, the tag attribute names are reset.
 */
void tagdelete(all)
	ELVBOOL all;	/* if ElvTrue, delete all tags (else just first tag) */
{
	/* delete the tag(s) */
	if (taglist)
	{
		do
		{
			taglist = tagfree(taglist);
		} while (all && taglist != NULL);
	}

	/* if taglist is empty, then we can discard the attribute names too */
	if (!taglist)
	{
		tagnamereset();
	}
}


/* This function inserts a tag into the tag list.  The list is sorted primarily
 * by name; within each name, they are sorted by the likelyhood factor (highest
 * first).  Returns ElvTrue of another tag with that name was previously in the
 * list, or ElvFalse if the new tag's name is unique so far.
 *
 * NOTE: This uses a special type of list, which uses an extra pointer to boost
 * its update effiency to be about the same as a binary tree.  Consequently,
 * it should be efficient, even for fairly large lists.
 */
void tagadd(tag)
	TAG	*tag;
{
	TAG	*scan;

	/* if empty list, this is easy */
	if (!taglist)
	{
		taglist = tag;
		return;
	}

	/* if before head, then insert as new head */
	if (tagbefore(tag, taglist))
	{
		tag->next = taglist;
		tag->bighop = taglist->bighop;
		taglist->bighop = NULL;
		taglist = tag;
		return;
	}

	/* search for the insertion point */
	for (scan = taglist; scan->next && !tagbefore(tag, scan->next); )
	{
		/* if there is a bighop value, try it */
		if (scan->bighop && !tagbefore(tag, scan->bighop))
		{
			scan = scan->bighop;
		}
		else
		{
			scan = scan->next;
		}
	}

	/* insert the item after the "scan" item */
	tag->next = scan->next;
	scan->next = tag;
	if (!scan->bighop)
		scan->bighop = tag;
}


/* This function parses a line from a tag file, and returns the corresponding
 * tag.  Returns NULL if...
 *	+ the tag name is 0 characters long, or
 *	+ the file name is missing or 0 characters long, or
 *	+ the third field is neither a number nor a regular expression, or
 * If any tag attributes are malformed, or if there are too many of them, then
 * the remainder of the line is ignored but the tag data up to that point is
 * returned.
 *
 * THE LINE IS CLOBBERED!  THE RETURNED TAG IS STATICALLY-ALLOCATED, AND
 * WILL CONTAIN REFERENCES BACK TO THE LINE TEXT!  You can use tagdup() to
 * make a dynamically-allocated copy of the tag.
 */
TAG *tagparse(line)
	char	*line;		/* a text line from a tags file */
{
	char	*tagname;	/* name of tag (first field) */
	char	*filename;	/* name of file (second field) */
	char	*address;	/* address of the tag, as a string */
	char	*name, *value;	/* an attribute of the tag */
	char	*build;
	char	c;
	int	quoted;		/* 0=not quoted, 1=quoted, 2=backslash */
 static TAG	tag;		/* the tag to be returned */

	/* clobber any old data in "tag" */
	memset(&tag, 0, sizeof tag);

	/* parse the tag name */
	tagname = line;
	while (*line != '\t')
	{
		if (!*line || *line == '\n')
			return NULL;
		line++;
	}
	*line++ = '\0';

	/* parse the file name */
	filename = line;
	while (*line != '\t')
	{
		if (!*line || *line == '\n')
			return NULL;
		line++;
	}
	*line++ = '\0';

	/* parse the line address */
	address = line;
	if (elvdigit(*address))
	{
		/* number -- use all digits */
		for (; elvdigit(*line); line++)
		{
		}
	}
	else if (*address == '?' || *address == '/')
	{
		/* regexp -- use chars up to EOL or next unquoted delimiter */
		c = *line;
		quoted = 0;
		do
		{
			if (*line == '\0' || *line == '\n')
				return NULL;
			line++;
			if (quoted > 0)
				quoted--;
			if (quoted == 0 && *line == '\\')
				quoted = 2;
		} while (*line != c
		     || quoted != 0
		     || (line[1] == ';' && (line[2] == '/' || line[2] == '?')));
		line++;
	}
	else
	{
		/* bad line address */
		return NULL;
	}

	/* allow an optional semicolon-quote after the address */
	if (line[0] == ';' && line[1] == '"')
	{
		*line = '\0';
		line += 2;
	}

	/* must be EOL or a tab after the address */
	if (*line && *line != '\n' && *line != '\t')
		return NULL;

	/* stuff the standard attributes into the tag */
	tag.attr[0] = tagname;
	tag.attr[1] = filename;
	tag.attr[2] = address;

	/* If there are any extra attributes, stuff them into the tag */
	for (c = *line; c == '\t'; )
	{
		/* mark the end of the previous field */
		*line++ = '\0';

		/* parse the name of the attribute */
		name = line;
		while (*line && *line != '\n' && *line != '\t' && *line != ':')
		{
			line++;
		}
		if (*line != ':')
		{
			/* use the "name" as the value of `kind' */
			c = *line;
			*line = '\0';
			if (!tagattr(&tag, "kind", name))
			{
				break;
			}
			*line = c;
			continue;
		}
		*line++ = '\0';

		/* Parse the value of the attribute, and translate "\x" into
		 * the corresponding character.
		 */
		build = value = line;
		while (*line && *line != '\n' && *line != '\t')
		{
			if (line[0] == '\\' && line[1] == 't')
			{
				*build++ = '\t';
				line += 2;
			}
			else if (line[0] == '\\' && line[1] == 'n')
			{
				*build++ = '\n';
				line += 2;
			}
			else if (line[0] == '\\' && line[1] == '\\')
			{
				*build++ = '\\';
				line += 2;
			}
			else
			{
				*build++ = *line++;
			}
		}

		/* Mark the end of the value.  Before doing that, though,
		 * remember the last character scanned so far.  We must
		 * remember that character before marking the end of the
		 * value, because it is possible that the end of the value
		 * will be at the same location as the last scanned char,
		 * and we don't want to clobber it until after we have a
		 * copy elsewhere.
		 */
		c = *line;
		*build = '\0';

		/* For "file" attribute with no value, assume filename */
		if (!*value && !strcmp(name, "file"))
			value = filename;

		/* Store it.  If too many hints, then skip the rest */
		if (!tagattr(&tag, name, value))
		{
			break;
		}
	}

	/* mark the end of the last field */
	*line = '\0';

	return &tag;
}
#endif /* FEATURE_TAGS */
