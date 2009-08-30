/* options.c */
/* Copyright 1995 by Steve Kirkendall */


/* This file contains functions which manipulate options.
 *
 * If compiled with -DTRY, it will include a main() function which can be
 * used to test these functions.
 */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_options[] = "$Id: options.c,v 2.77 2003/10/17 17:41:23 steve Exp $";
#endif

#ifndef OPT_MAXCOLS
# define OPT_MAXCOLS 7
#endif


/* This data type is used to record a collection of options that were added
 * via a call to optinsert().
 */
typedef struct domain_s
{
	struct domain_s	*next;	/* next domain in a linked list */
	char		*name;	/* domain name */
	int		nopts;	/* number of options in this domain */
	OPTDESC		*desc;	/* descriptions */
	OPTVAL		*val;	/* option values */
} OPTDOMAIN;

/* This data type is used to collect the names & values of options which are
 * supposed to be output.
 */
typedef struct optout_s
{
	struct optout_s *next;	/* another option to be output */
	OPTDOMAIN	*dom;	/* domain of option to be output */
	int		idx;	/* index of option to be output */
	int		width;	/* width of name+value */
} OPTOUT;


/* This is used for storing the names and values of :local options, so they can
 * be restored when the script/alias is done.
 */
typedef struct optstk_s
{
	struct optstk_s	*next;	/* another stack entry, or NULL */
	OPTDESC		*desc;	/* descriptions of options */
	OPTVAL		*val;	/* values of options */
	int		i;	/* index of the value being stored */
	ELVBOOL		wasset;	/* was the OPT_SET flag set originally? */
	CHAR		*value;	/* original value of the option, as a string */
	BUFFER		buffer;	/* original bufdefault -- this might not be */
				/* valid anymore when values get restored */
} OPTSTK;

#if USE_PROTOTYPES
static ELVBOOL optshow(char *name);
static void optoutput(ELVBOOL domain, ELVBOOL all, ELVBOOL set, CHAR *outbuf, size_t outsize);
# ifdef FEATURE_MISC
static void savelocal(OPTDESC *desc, OPTVAL *val, int i);
static OPTSTK *restorelocal(OPTSTK *item);
# endif
#endif


/* head of the list of current option domains */
static OPTDOMAIN	*head;


#ifdef FEATURE_MISC
/* stack of local options */
static OPTSTK *stack = (OPTSTK *)1;
#endif


/* Check a number's validity.  If valid & different, then set val and return
 * 1; if valid & same, then return 0; else give error message and return -1.
 */
int optisnumber(desc, val, newval)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
	CHAR	*newval;/* value the option should have (as a string) */
{
	long	min, max, value;

	/* convert value to binary */
	if (!calcnumber(newval))
	{
		msg(MSG_ERROR, "[s]$1 requires a numeric value", desc->longname);
		return -1;
	}
	value = CHAR2long(newval);

	/* compare against range string */
	if (desc->limit)
	{
		sscanf(desc->limit, "%ld:%ld", &min, &max);
		if (value < min || value > max)
		{
			msg(MSG_ERROR, "[sdd]$1 must be between $2 and $3", desc->longname, min, max);
			return -1;
		}
	}

	/* same value as before? */
	if (val->value.number == value)
	{
		return 0;
	}

	/* store the value */
	val->value.number = value;
	return 1;
}


/* Check a strings validity (all are valid) and set the option.  Return 1
 * if different, or 0 if same.
 */
int optisstring(desc, val, newval)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
	CHAR	*newval;/* value the option should have (as a string) */
{
	/* if value is the same, do nothing */
	if (val->value.string && !CHARcmp(val->value.string, newval))
	{
		return 0;
	}

	/* free the old string, if necessary */
	if (val->value.string && (val->flags & OPT_FREE))
	{
		safefree(val->value.string);
	}

	/* store a copy of the new string */
	val->value.string = CHARkdup(newval);
	val->flags |= OPT_FREE;
	return 1;
}


/* Check a strings validity (all are valid) and set the option.  Return 1
 * if different, or 0 if same.
 */
int optispacked(desc, val, newval)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
	CHAR	*newval;/* value the option should have (as a string) */
{
	CHAR	*scan, *next;
	ELVBOOL	takesvalue;
	ELVBOOL	hasvalue;
	CHAR	name[50];
	char	*errormsg;

	/* first make sure there are no duplicates */
	for (scan = newval; *scan; scan = next)
	{
		next = calcelement(newval, scan);
		if (next < scan)
		{
			errormsg = "[SS]$1.$2 appears more than once";
			goto Error;
		}
		while (*next && *next++ != ',')
		{
		}
	}

	/* make sure each field is legal, and has a value if it should,
	 * or doesn't have a value if it shouldn't
	 */
	for (scan = newval; *scan; scan = next)
	{
		/* locate this field in limit */
		next = calcelement(toCHAR(desc->limit), scan);
		if (!next)
		{
			errormsg = "[SS]$2 is unrecognized in $1";
			goto Error;
		}

		/* determine whether this field takes a value */
		takesvalue = (ELVBOOL)(*next == ':');

		/* advance to next field in newval */
		hasvalue = ElvFalse;
		next = scan;
		while (*next && *next++ != ',')
		{
			if (*next == ':')
				hasvalue = ElvTrue;
		}

		/* complain if it has a value and shouldn't, or
		 * vice versa.
		 */
		if (takesvalue != hasvalue)
		{
			/* output the error message */
			if (takesvalue)
				errormsg = "[SS]$1.$2 requires a value";
			else
				errormsg = "[SS]$1.$2 does not take a value";
			goto Error;
		}
	}

	/* the value is valid.  store it like a string */
	return optisstring(desc, val, newval);

Error:
	/* an error was detected.  "scan" points to field name, and "errormsg"
	 * is an appropriate error message, with [SS] arguments.
	 */

	/* extract the field name */
	next = name;
	while (next < &name[sizeof name - 1]
	    && *scan != ',' && *scan != ':' && *scan)
	{
		*next++ = *scan++;
	}
	*next = '\0';

	/* output the error message */
	msg(MSG_ERROR, errormsg, desc->longname, name);
	return -1;
}

/* Check a string's validity against a space-delimited list of legal values.
 * If valid & different, then set val to the string's first character, and
 * return 1; if valid & same, then return 0; else give error message and
 * return -1.  Note that each acceptable valid string must begin with a
 * unique character for this to work.
 */
int optisoneof(desc, val, newval)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
	CHAR	*newval;/* value the option should have (as a string) */
{
	int	len;
	char	*scan;

	assert(desc->limit != NULL);

	/* compute the length of newval */
	len = CHARlen(newval);
	if (len <= 0)
	{
		goto NoMatch;
	}

	/* compare against each legal value */
	for (scan = desc->limit; scan - 1 != NULL; scan = strchr(scan, ' ') + 1)
	{
		/* does it match this value? */
		if (!CHARncmp(newval, toCHAR(scan), (size_t)len))
		{
			/* yes! Either save it & return 1, or just return 0 */
			if ((char)*newval != val->value.character)
			{
				val->value.character = (char)*newval;
				return 1;
			}
			return 0;
		}
	}

	/* no match.  Give an error message and exit */
NoMatch:
	msg(MSG_ERROR, "[ss]$1 must be one of {$2}", desc->longname, desc->limit);
	return -1;
}

/* Check if a string is an acceptable list of tabstop positions, and update an
 * option if they are.  Return 1 for success, -1 if error.
 */
int optistab(desc, val, newval)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
	CHAR	*newval;/* value the option should have (as a string) */
{
	int	i, j = 0;
	long	width, total;
	short	*tab;

	/* if no new value, then use the limit value.  If there is no limit,
	 * then discard the settings.
	 */
	if (!newval || !*newval)
	{
		newval = toCHAR(desc->limit);
		if (!newval)
		{
			if (!val->value.tab)
				return 0;
			if (val->flags & OPT_FREE)
				safefree(val->value.tab);
			val->value.tab = NULL;
			return 1;
		}
	}

	/* check the values, and compute the overall width for all but the last
	 * term.
	 */
	for (i = 0, width = total = 0L; ; i++)
	{
		if (elvdigit(newval[i]))
			width = width * 10 + newval[i] - '0';
		else if (newval[i] == ',' || !newval[i])
		{
			if (width < 1 || (short)width != (long)width)
			{
				msg(MSG_ERROR, "[s]bad width in $1 list", desc->longname);
				return -1;
			}
			if (!newval[i])
				break;
			total += width;
			width = 0L;
		}
		else
		{
			msg(MSG_ERROR, "[s]value of $1 should be comma-delimited numbers", desc->longname);
			return -1;
		}
	}

	/* free the old list, if any, and allocate a new list */
	if (val->value.tab && (val->flags & OPT_FREE))
		safefree(val->value.tab);
	tab = val->value.tab = (short *)safekept((int)(2 + total), sizeof(short));
	val->flags |= OPT_FREE;

	/* stuff the values into the array */
	tab[0] = (short)total;
	tab[1] = (short)width;
	for (i = 0, width = total = 0L; newval[i]; i++)
	{
		if (elvdigit(newval[i]))
			width = width * 10 + newval[i] - '0';
		else
		{
			total += width;
			tab[2 + (int)total - 1] = 1;
			width = 0L;
		}
	}
	for (i = tab[0]; --i >= 0;)
	{
		if (tab[2 + i] == 1)
			j = 1;
		else
			tab[2 + i] = ++j;
	}

	return 1;
}

/* convert a "number" value to a string */
CHAR *optnstring(desc, val)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
{
	static char	buf[30];

	/* convert the value to a string */
	sprintf(buf, "%ld", val->value.number);
	return toCHAR(buf);
}

/* convert a "string" value to a string.  For NULL strings, return "". */
CHAR *optsstring(desc, val)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
{
	if (val->value.string)
		return val->value.string;
	else
		return toCHAR("");
}

/* convert a "one of" value to a string */
CHAR *opt1string(desc, val)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
{
	static CHAR	buf[30], *build;
	char		*scan;

	/* locate the current value in the limit string */
	scan = desc->limit;
	assert(scan != NULL);
	while (*scan != val->value.character)
	{
		scan = strchr(scan, ' ');
		assert(scan != NULL);
		scan++;
	}

	/* copy the value to buf, and terminate it with a NUL */
	for (build = buf; *scan && *scan != ' '; )
	{
		*build++ = *scan++;
	}
	*build = '\0';

	return buf;
}

/* convert a "tab" value to a string */
CHAR *opttstring(desc, val)
	OPTDESC	*desc;	/* description of the option */
	OPTVAL	*val;	/* value of the option */
{
	static CHAR buf[100];
	CHAR	*build;
	short	*tab;
	int	i;

	/* get the value */
	tab = val->value.tab;

	/* if no value, return "" */
	if (!tab)
	{
		buf[0] = '\0';
		return buf;
	}

	/* build a list of specific tabstops */
	for (build = buf, i = 0; i < tab[0]; i += tab[2 + i])
	{
		if (build != buf)
			*build++ = ',';
		long2CHAR(build, (long)tab[2 + i]);
		while (*++build)
		{
		}
	}

	/* add the repeating width */
	if (build != buf)
		*build++ = ',';
	long2CHAR(build, (long)tab[1]);

	return buf;
}


/* Delete the options whose values are stored starting at val. */
void optdelete(val)
	OPTVAL	val[];	/* array of values to delete */
{
	OPTDOMAIN	*scan, *lag;

	assert(head != (OPTDOMAIN *)0);

	/* locate the domain in the list */
	for (scan = head, lag = (OPTDOMAIN *)0;
	     scan && scan->val != val;
	     lag = scan, scan = scan->next)
	{
	}

	/* if not found, then do nothing */
	if (!scan)
		return;

	/* remove the domain from the list */
	if (lag)
	{
		lag->next = scan->next;
	}
	else
	{
		head = scan->next;
	}

	/* free the domain structure */
	safefree(scan);
}

/* Add options to the list known to :set.  desc is an array of
 * option descriptions, as described below.  val is a parallel
 * array where the option values are stored.  nopts is the number
 * of options being added.
 *
 * Descriptions and values are stored separately to support the
 * situation multiple items such as buffers must have their own
 * values for the same options.
 */
void optinsert(domain, nopts, desc, val)
	char	*domain;	/* name of this set of options */
	int	nopts;		/* number of options being inserted */
	OPTDESC	desc[];		/* descriptions of options */
	OPTVAL	val[];		/* values of options */
{
	OPTDOMAIN *newp;

	/* create a new domain structure */
	newp = (OPTDOMAIN *)safekept(1, sizeof(OPTDOMAIN));
	assert(newp != (OPTDOMAIN *)0);
	newp->name = domain,
	newp->nopts = nopts;
	newp->desc = desc;
	newp->val = val;

	/* insert it at the start of the list, so its values take precedence
	 * over any other options that may have been declared with the same
	 * name.
	 */
	newp->next = head;
	head = newp;
}


/* This function calls safefree() on any values which have the OPT_FREE
 * flag set.  This is handy when you intend to free a struct which contains
 * some option values.
 */
void optfree(nopts, vals)
	int	nopts;	/* number of options */
	OPTVAL	*vals;	/* values of options */
{
	int	i;

	for (i = 0; i < nopts; i++)
	{
		if (vals[i].flags & OPT_FREE)
		{
			safefree(vals[i].value.pointer);
		}
	}
}


/* This function sets the "show" flag for a given option.  Returns ElvTrue if
 * successful, or ElvFalse if the option doesn't exist.
 */
static ELVBOOL optshow(name)
	char	*name;	/* name of an option that should be shown */
{
	OPTDOMAIN *dom;
	ELVBOOL	  ret = ElvFalse;
	int	  i;

	/* for each domain of options... */
	for (dom = head; dom; dom = dom->next)
	{
		/* for each option in that domain... */
		for (i = 0; i < dom->nopts; i++)
		{
			/* if this is the one we're looking for... */
			if (!strcmp(dom->desc[i].longname, name)
			 || !strcmp(dom->desc[i].shortname, name)
			 || !strcmp(dom->name, name))
			{
				dom->val[i].flags |= OPT_SHOW;
				ret = ElvTrue;
			}
		}
	}

	/* complain if unknown */
	if (!ret)
	{
		msg(MSG_ERROR, "[s]bad option name $1", name);
	}

	return ret;
}

/* This function collects option names & values, sorts them, puts them into
 * columns, and outputs them.  Parameters are:
 *	domain	- include domain names as part of option name?
 *	all	- output all options?
 *	set	- output all options which have been set?
 *		  (If neither "all" nor "set" then only options which were
 *		   touched by a previous optshow() are output.)
 */
static void optoutput(domain, all, set, outbuf, outsize)
	ELVBOOL	  domain;	/* if ElvTrue, include domain names */
	ELVBOOL	  all;		/* if ElvTrue, output all non-hidden options */
	ELVBOOL	  set;		/* if ElvTrue, output all changed options */
	CHAR	  *outbuf;	/* where to place the values */
	size_t	  outsize;	/* size of outbuf */
{
	OPTOUT	  *out;		/* list of options to be output */
	OPTOUT	  *scan, *lag;	/* used for scanning through "out" list */
	OPTOUT	  *newp;	/* a new OPTOUT to be inserted into list */
	OPTDOMAIN *dom;		/* used for scanning through domains */
	int	  i, j, k;	/* used for scanning through opts in a domain */
	int	  cmp;		/* results of comparison */
	int	  nshown;	/* number of options to show */
	int	  maxwidth;	/* width of widest item */
	int	  ncols, nrows;	/* number of columns, and items per column */
	struct
	{
		OPTOUT	*opt;	/* first option in a column */
		int	width;	/* width of the column */
	}	  colinfo[OPT_MAXCOLS];
	CHAR	  *str, *build;

	/* if no output buffer, then do nothing */
	if (!outbuf)
		return;

	/* start with an empty list */
	out = (OPTOUT *)0;
	nshown = 0;
	maxwidth = 0;
	*outbuf = '\0';

	/* For each domain... */
	for (dom = head; dom; dom = dom->next)
	{
		/* For each option in the domain... */
		for (i = 0; i < dom->nopts; dom->val[i++].flags &= ~OPT_SHOW)
		{
			/* Skip if we aren't supposed to output this option */
			if (!all || !set)
			{
				if ((all || set) && dom->val[i].flags & OPT_HIDE)
					continue;
				if (!all && !(dom->val[i].flags & (set ? OPT_SET : OPT_SHOW)))
					continue;
			}

			/* See where this should be inserted into the output
			 * list.  Beware of duplicates; keep only first of each.
			 */
			for (scan = out, lag = (OPTOUT *)0, cmp = 1;
			     scan && (cmp = strcmp(scan->dom->desc[scan->idx].longname, dom->desc[i].longname)) < 0;
			     lag = scan, scan = scan->next)
			{
			}
			if (cmp == 0)
			{
				continue;
			}

			/* create a new OPTOUT structure for this option */
			newp = (OPTOUT *)safealloc(1, sizeof(OPTOUT));
			newp->dom = dom;
			newp->idx = i;
			if (domain) /* including domain name? */
				newp->width = strlen(dom->name) + 1 +
						strlen(dom->desc[i].shortname);
			else
				newp->width = strlen(dom->desc[i].longname);
			if (dom->desc[i].isvalid) /* non-boolean? */
			{
				newp->width += 1;
				if (dom->desc[i].asstring)
				{
					cmp = CHARlen((*dom->desc[i].asstring)(&dom->desc[i], &dom->val[i]));
					if (newp->width + cmp > o_optionwidth
					 && newp->width < o_optionwidth - 3
					 && (dom->val[i].flags & OPT_SHOW) == 0)
						newp->width = o_optionwidth;
					else
						newp->width += cmp;
				}
			}
			else if (!dom->val[i].value.boolean)
			{
				newp->width += 2;
			}

			/* is this the widest value so far? */
			if (newp->width > maxwidth)
			{
				maxwidth = newp->width;
			}

			/* insert the new OPTOUT into the list */
			if (lag)
			{
				newp->next = lag->next;
				lag->next = newp;
			}
			else
			{
				newp->next = out;
				out = newp;
			}
			nshown++;
		}
	}

	/* if nothing to show, then exit */
	if (nshown == 0)
	{
		return;
	}

	/* try to use as many columns as possible */
	for (ncols = (nshown > OPT_MAXCOLS ? OPT_MAXCOLS : nshown); ; ncols--)
	{
		/* how many options would go in each column? */
		nrows = (nshown + ncols - 1) / ncols;

		/* figure out the width of each column */
		for (scan = out, i = 0; i < ncols; i++)
		{
			colinfo[i].opt = scan;
			colinfo[i].width = 0;
			for (j = 0; j < nrows && scan; j++, scan = scan->next)
			{
				/* if this is the widest so far, widen col */
				if (scan->width > colinfo[i].width)
				{
					colinfo[i].width = scan->width;
				}
			}
			colinfo[i].width += 2;
		}

		/* if the total width is narrow enough, then use it */
		for (j = -2, i = 0; i < ncols; i++)
		{
			j += colinfo[i].width;
		}
		if (ncols == 1 || j < (windefault ? o_columns(windefault) : 80) - 1)
		{
			break;
		}
	}

	/* if the list is too large to fit in the output buffer, then just
	 * return an empty string.
	 */
	if ((size_t)j * (size_t)nrows >= outsize)
	{
		/* free the list */
		for (scan = out; scan; scan = lag)
		{
			lag = scan->next;
			safefree(scan);
		}

		CHARncpy(outbuf, toCHAR("too big!\n"), outsize);
		return;
	}

	/* show 'em */
	for (i = 0; i < nrows; i++)
	{
		for (j = 0; j < ncols && colinfo[j].opt; j++)
		{
			/* include the domain name, if we're supposed to */
			scan = colinfo[j].opt;
			if (domain)
			{
				(void)CHARcat(outbuf, scan->dom->name);
				(void)CHARcat(outbuf, ".");
			}

			/* booleans are special... */
			if (!scan->dom->desc[scan->idx].isvalid)
			{
				if (!scan->dom->val[scan->idx].value.boolean)
				{
					(void)CHARcat(outbuf, "no");
				}
				(void)CHARcat(outbuf, domain
					? scan->dom->desc[scan->idx].shortname
					: scan->dom->desc[scan->idx].longname);
			}
			else
			{

				str = (CHAR *)(domain
					? scan->dom->desc[scan->idx].shortname
					: scan->dom->desc[scan->idx].longname);
				(void)CHARcat(outbuf, str);
				(void)CHARcat(outbuf, toCHAR("="));
				cmp = CHARlen(str) + 1;
				str = (*scan->dom->desc[scan->idx].asstring)(
					&scan->dom->desc[scan->idx],
					&scan->dom->val[scan->idx]);
				if (cmp + CHARlen(str) > (unsigned)scan->width)
				{
					/* value too long, so just show part */
					build = &outbuf[CHARlen(outbuf)];
					for (; cmp < scan->width - 3; cmp++)
						*build++ = *str++;
					*build++ = '.';
					*build++ = '.';
					*build++ = '.';
					*build++ = '\0';
				}
				else
					/* show the whole value */
					(void)CHARcat(outbuf, str);
			}

			/* pad to max width, except at end of column */
			if (j + 1 == ncols || !colinfo[j + 1].opt)
			{
				(void)CHARcat(outbuf, "\n");
			}
			else
			{
				for (k = colinfo[j].width - scan->width; k > 0; k--)
				{
					(void)CHARcat(outbuf, " ");
				}
			}

			/* next time, use the next option */
			colinfo[j].opt = colinfo[j].opt->next;
		}
	}

	/* free the list */
	for (scan = out; scan; scan = lag)
	{
		lag = scan->next;
		safefree(scan);
	}
}


/* This function returns the value of an option, as a statically allocated,
 * nul-terminated string.  For booleans, it returns "true" or "false".  For
 * invalid option names, it returns a NULL pointer.
 *
 * If the desc parameter is non-NULL, then the pointer that it refers to will
 * be altered to point to the option's OPTDESC struct.
 */
CHAR *optgetstr(name, desc)
	CHAR	*name;	/* NUL-terminated name */
	OPTDESC	**desc;	/* where to store a pointer to the OPTDESC struct */
{
	OPTDOMAIN *dom;	/* used for scanning through domains */
	int	  i;	/* used for scanning through opts in a domain */

	/* For each domain... */
	for (dom = head; dom; dom = dom->next)
	{
		/* For each option in the domain... */
		for (i = 0; i < dom->nopts; i++)
		{
			/* skip options with the wrong name */
			if (strcmp(dom->desc[i].longname, tochar8(name))
			 && strcmp(dom->desc[i].shortname, tochar8(name)))
			{
				continue;
			}

			/* if the caller wants to know the OPTDESC, tell it */
			if (desc)
				*desc = &dom->desc[i];

			/* convert it */
			if (dom->desc[i].isvalid) /* non-boolean? */
			{
				if (dom->desc[i].asstring)
				{
					return (CHAR *)(*dom->desc[i].asstring)(&dom->desc[i], &dom->val[i]);
				}
				return (CHAR *)"";
			}
			else if (dom->val[i].value.boolean)
			{
				return o_true;
			}
			else
			{
				return o_false;
			}
		}
	}

	/* if we get here, then we didn't find the option */
	return (CHAR *)0;
}

#ifdef FEATURE_AUTOCMD
/* Trigger OptChanged and/or OptSet events on an option.  You can leave either
 * name or desc set to NULL, but not both.  If you have the OPTDESC pointer
 * handy, then you should pass that and use NULL for the name; otherwise pass
 * the option's long name and use NULL for desc.  Either way, you must pass a
 * valid "val" pointer.
 */
void optautocmd(name, desc, val)
	char	*name;	/* option's long name, or NULL t use desc & val */
	OPTDESC	*desc;	/* description of option, if known */
	OPTVAL	*val;	/* value of option */
{
	CHAR	noname[30];

	/* if we have a name, look it up */
	if (name && !optgetstr(toCHAR(name), &desc))
		msg(MSG_FATAL, "[s]bufautocmd($1...) called for bad option name", name);

	/* if no events for this option, then do nothing */
	if (!desc->event)
		return;

	/* if supposed to send an event, then do that */
	if (desc->event)
	{
		/* "OptChanged" */
		(void)auperform(windefault, ElvFalse, NULL, AU_OPTCHANGED,
			toCHAR(desc->longname));

		/* "OptSet" */
		if (!desc->asstring)
		{
			noname[0] = '\0';
			if (!val->value.boolean)
				CHARcpy(noname, "no");
			CHARcat(noname, desc->longname);
			(void)auperform(windefault, ElvFalse, NULL,
				AU_OPTSET, noname);
		}
	}
}
#endif

/* This function assigns a new value to an option.  For booleans, it expects
 * "true" or "false".  For invalid option names or inappropriate values it
 * outputs an error message and returns ElvFalse.  If the value is NULL it
 * returns ElvFalse without issueing an error message, on the assumption that
 * whatever caused the NULL pointer already issued a message.
 */
ELVBOOL optputstr(name, value, bang)
	CHAR	*name;	/* NUL-terminated name */
	CHAR	*value;	/* NUL-terminated value */
	ELVBOOL	bang;	/* don't set the OPT_SET flag? */
{
	OPTDOMAIN *dom;	/* used for scanning through domains */
	int	  i;	/* used for scanning through opts in a domain */
	ELVBOOL	  ret;	/* return code */
	WINDOW	  w;

	/* For each domain... */
	for (dom = head; dom; dom = dom->next)
	{
		/* For each option in the domain... */
		for (i = 0; i < dom->nopts; i++)
		{
			/* skip options with the wrong name */
			if (strcmp(dom->desc[i].longname, tochar8(name))
			 && strcmp(dom->desc[i].shortname, tochar8(name)))
			{
				continue;
			}

			/* if the option is locked, then fail */
			if (dom->val[i].flags & OPT_LOCK)
			{
				if (!bang)
					msg(MSG_ERROR, "[S]$1 is locked", name);
				return ElvFalse;
			}

			/* if the option is unsafe and "safer" is set, fail */
			if (o_security != 'n' /* normal */
			 && (dom->val[i].flags & OPT_UNSAFE) != 0)
			{
				if (!bang)
					msg(MSG_ERROR, "[S]unsafe to change $1", name);
				return ElvFalse;
			}

			/* if we haven't save the default value before, and
			 * this isn't a :set! command (with a bang) then save
			 * the old value as the default.
			 */
			if (!dom->desc[i].dflt && !bang)
			{
				dom->desc[i].dflt = CHARkdup(optgetstr(toCHAR(dom->desc[i].longname), NULL));
			}

			/* convert it */
			ret = ElvTrue;
			if (dom->desc[i].isvalid) /* non-boolean? */
			{
				/* if the value is valid & different and we need
				 * to call a store function, then call it.
				 */
				if ((*dom->desc[i].isvalid)(&dom->desc[i], &dom->val[i], value) == 1
				 && dom->desc[i].store)
				{
					ret = (ELVBOOL)((*dom->desc[i].store)(&dom->desc[i], &dom->val[i], value) >= 0);
				}
			}
			else
			{
				dom->val[i].value.boolean = calctrue(value);
			}

			/* set or clear the "set" flag */
			if (!bang)
			{
				if (CHARcmp(optgetstr(toCHAR(dom->desc[i].longname), NULL), dom->desc[i].dflt))
					dom->val[i].flags |= OPT_SET;
				else
					dom->val[i].flags &= ~OPT_SET;
			}
			else
			{
				/* store the new value as the default */
				dom->desc[i].dflt = CHARkdup(optgetstr(toCHAR(dom->desc[i].longname), NULL));
				dom->val[i].flags &= ~OPT_SET;
			}

#ifdef FEATURE_AUTOCMD
			/* if supposed to send an event, then do that */
			optautocmd(NULL, &dom->desc[i], &dom->val[i]);
#endif

			/* if the "redraw" flag is set, then force redraw */
			if (dom->val[i].flags & (OPT_REDRAW|OPT_SCRATCH))
			{
				for (w = winofbuf(NULL, NULL); w; w = winofbuf(w, NULL))
				{
					if (dom->val[i].flags & OPT_SCRATCH)
						w->di->logic = DRAW_SCRATCH;
					else if (w->di->logic == DRAW_NORMAL)
						w->di->logic = DRAW_CHANGED;
				}
			}

			return ret;
		}
	}

	/* if we get here, then we didn't find the option */
	if (!bang)
		msg(MSG_ERROR, "[S]bad option name $1", name);
	return ElvFalse;
}

#ifdef FEATURE_MISC
/* Save an option on the :local stack */
static void savelocal(desc, val, i)
	OPTDESC	*desc;	/* descriptions of the option's domain */
	OPTVAL	*val;	/* values of the option's domain */
	int	i;	/* index of the particular option we want to save */
{
	OPTSTK	*s;

	/* create a stack entry */
	s = (OPTSTK *)safealloc(1, sizeof(OPTSTK));

	/* store the data into it */
	s->desc = desc;
	s->val = val;
	s->i = i;
	s->wasset = (ELVBOOL)((val[i].flags & OPT_SET) != 0);
	if (desc[i].isvalid) /* non-boolean? */
	{
		if (desc[i].asstring)
			s->value = (*desc[i].asstring)(&desc[i], &val[i]);
		else
			s->value = toCHAR("");
	}
	else if (val[i].value.boolean)
	{
		s->value = o_true;
	}
	else
	{
		s->value = o_false;
	}
	s->value = CHARdup(s->value);
	s->buffer = (bufdefault && val == &bufdefault->filename) ? bufdefault : NULL;

	/* insert it onto the stack */
	s->next = stack;
	stack = s;
}

/* Restore a single option from the :local stack, and then free that item and
 * return the one after it.
 */
static OPTSTK *restorelocal(item)
	OPTSTK	*item;
{
	OPTSTK	*next;
	OPTDESC	*desc = &item->desc[item->i];
	OPTVAL	*val = &item->val[item->i];
	OPTDOMAIN *d;
	ELVBOOL	newbool, changed = ElvFalse;
	BUFFER	b;

	/* verify that the option is still active */
	for (d = head; d; d = d->next)
		if (d->val == item->val)
			break;

	/* if not active, maybe it is a buffer option that we can make
	 * active again temporarily.
	 */
	b = NULL;
	if (!d && item->buffer && bufdefault != item->buffer)
	{
		/* verify that the buffer still exists */
		while ((b = buflist(b)) != NULL)
			if (b == item->buffer)
				break;
		if (b)
		{
			/* It exists.  Make a 1-element group for it */
			optinsert("local", 1, desc, val);
		}
	}

	/* if active now, then restore it */
	if (d)
	{
		/* restore the value */
		if (desc->isvalid) /* non-boolean? */
		{
			/* if the value is valid & different and we need
			 * to call a store function, then call it.
			 */
			if ((*desc->isvalid)(desc, val, item->value) == 1)

			{
				if (!desc->store || (*desc->store)(desc, val, stack->value) != 0)
					changed = ElvTrue;
			}
		}
		else
		{
			newbool = calctrue(stack->value);
			changed = (ELVBOOL)(newbool != val->value.boolean);
			val->value.boolean = newbool;
		}

		/* restore the "was set" flag */
		if (stack->wasset)
			val->flags |= OPT_SET;
		else
			val->flags &= ~OPT_SET;

#ifdef FEATURE_AUTOCMD
		/* if supposed to send an event, then do that */
		if (changed)
			optautocmd(NULL, desc, val);
#endif
	}
	else
	{
		msg(MSG_WARNING, "[s]can't restore local $1", desc->longname);
	}

	/* if we had to insert a temporary local group (to restore a buffer
	 * option after we've switched buffers) then delete that group now.
	 */
	if (b)
		optdelete(val);

	/* free it */
	next = item->next;
	safefree(item->value);
	safefree(item);

	/* return the item after it */
	return next;
}


/* This function serves two purposes.  It should be called with NULL at the
 * start of a script or alias, to find the current location of the :local
 * stack.  It should be called again at the end of the script/alias, with the
 * value returned by the first call, to restore all local variables from that
 * stack.
 */
void *optlocal(level)
	void 	*level;	/* level of stack to pop to */
{
	/* if NULL, then return the current stack */
	if (!level)
		return stack;

	/* else we need to restore items until we reach the old stack point */
	while ((void *)stack != level)
		stack = restorelocal(stack);

	return NULL;
}
#endif /* FEATURE_MISC */

/* This function parses the arguments to a ":set" command.  Returns ElvTrue if
 * successful.  For errors, it issues an error message via msg() and returns
 * ElvFalse.  If any options are to be output, their values will be stored in
 * a null-terminated string in outbuf.
 *
 * If outbuf is NULL, then no options will be output, and any options mentioned
 * in the "args" string will be pushed onto the :local stack.
 */
ELVBOOL optset(bang, args, outbuf, outsize)
	ELVBOOL	  bang;		/* if ElvTrue, any options displayed will include domain */
	CHAR	  *args;	/* arguments of ":set" command */
	CHAR	  *outbuf;	/* buffer for storing output string */
	size_t	  outsize;	/* size of outbuf */
{
	CHAR	  *name;	/* name of option in args */
	CHAR	  *value;	/* value of the variable */
	CHAR	  *scan;	/* used for moving through strings */
	CHAR	  *build;	/* used for copying chars from "scan" */
	CHAR	  *prefix;	/* pointer to "neg" or "no" at front of a boolean */
	ELVBOOL	  quote;	/* boolean: inside '"' quotes? */
	OPTDOMAIN *dom;		/* used for scanning through domains list */
	ELVBOOL	  ret;		/* return code */
	WINDOW	  w;
	ELVBOOL	  b;
        int       i;

	/* be optimistic.  Begin by assuming this will succeed. */
	ret = ElvTrue;

	/* initialize "prefix" just to avoid a compiler warning */
	prefix = NULL;

	/* if no arguments, list values of any set options */
	if (!*args)
	{
		optoutput(bang, ElvFalse, ElvTrue, outbuf, outsize);
		return ElvTrue;
	}

	/* if "all", list values of all options */
	if (!CHARcmp(args, toCHAR("all")))
	{
		optoutput(bang, ElvTrue, ElvFalse, outbuf, outsize);
		return ElvTrue;
	}
	if (!CHARcmp(args, toCHAR("everything")))
	{
		optoutput(bang, ElvTrue, ElvTrue, outbuf, outsize);
		return ElvTrue;
	}

	/* for each assignment... */
	for (name = args; *name; name = scan)
	{
		/* skip whitespace */
		while (*name == ' ' || *name == '\t')
		{
			name++;
		}
		if (!*name)
			break;

		/* after the name, find the value (if any) */
		for (scan = name; elvalnum(*scan); scan++)
		{
		}
		if (*scan == '=')
		{
			*scan++ = '\0';
			value = build = scan;
			for (quote = ElvFalse; *scan && (quote || !elvspace(*scan)); scan++)
			{
				if (*scan == '"')
				{
					quote = (ELVBOOL)!quote;
				}
				else if (*scan == '\\' && scan[1] && !elvalnum(scan[1]))
				{
					*build++ = *++scan;
				}
				else
				{
					*build++ = *scan;
				}
			}
			if (*scan)
				scan++;
			*build = '\0';
		}
		else if (*scan == '?')
		{
			/* mark the option for showing */
			*scan++ = '\0';
			if (!optshow(tochar8(name)))
				ret = ElvFalse;
			continue;
		}
		else /* no "=" or "?" */
		{
			if (*scan)
			{
				*scan++ = '\0';
			}
			value = NULL;
			prefix = name;
			if (!CHARcmp(name, toCHAR("novice"))
			 || !CHARcmp(name, toCHAR("nonascii")))
				/* don't check for a "no" prefix */;
			else if (prefix[0] == 'n' && prefix[1] == 'o')
				name += 2;
			else if (prefix[0] == 'n' && prefix[1] == 'e' && prefix[2] == 'g')
				name += 3;
		}

		/* find the option */
		for (dom = head; dom; dom = dom->next)
		{
			/* check each option in this domain */
			for (i = 0; i < dom->nopts; i++)
			{
				if (!CHARcmp(name, toCHAR(dom->desc[i].longname))
				 || !CHARcmp(name, toCHAR(dom->desc[i].shortname)))
				{
					goto BreakBreak;
				}
			}
		}
BreakBreak:

		/* if not found, complain */
		if (!dom)
		{
			msg(MSG_ERROR, "[S]bad option name $1", name);
			ret = ElvFalse;
			continue;
		}

		/* if non-boolean & we got no value, then assume '?' */
		if (dom->desc[i].isvalid && !value)
		{
			if (prefix == name)
			{
				if (outbuf)
					optshow(tochar8(name));
#ifdef FEATURE_MISC
				else if (dom->val[i].flags & OPT_LOCK)
				{
					msg(MSG_ERROR, "[S]$1 is locked", name);
					name = scan;
					ret = ElvFalse;
					continue;
				}
				else
					savelocal(dom->desc, dom->val, i);
#endif
			}
			else
			{
				msg(MSG_ERROR, "[S]$1 is not a boolean option", name);
				ret = ElvFalse;
			}
			continue;
		}

		/* if option is locked, then complain */
		if (dom->val[i].flags & OPT_LOCK)
		{
			msg(MSG_ERROR, "[S]$1 is locked", name);
			name = scan;
			ret = ElvFalse;
			continue;
		}

		/* if unsafe, then complain */
		if (o_security != 'n' /* normal */
		 && (dom->val[i].flags & OPT_UNSAFE) != 0)
		{
			msg(MSG_ERROR, "[S]unsafe to change $1", name);
			name = scan;
			ret = ElvFalse;
			continue;
		}

		/* if boolean & we got a value, then complain */
		if (!dom->desc[i].isvalid && value)
		{
			msg(MSG_ERROR, "[S]$1 is a boolean option", name);
			name = scan;
			ret = ElvFalse;
			continue;
		}

		/* if :set! and the option was already explicitly set, then
		 * don't set it now.
		 */
		if (bang && (dom->val[i].flags & OPT_SET))
		{
			if (o_verbose >= 3)
				msg(MSG_INFO, "[s]skipping \":set! $1\" because already set", dom->desc[i].longname);
			name = scan;
			continue;
		}

#ifdef FEATURE_MISC
		/* if :local then save its original value */
		if (!outbuf)
			savelocal(dom->desc, dom->val, i);
#endif

		/* if boolean, set it */
		if (!dom->desc[i].isvalid)
		{
			/* set the value */
			if (prefix == name)
				b = ElvTrue;
			else if (prefix[0] == 'n' && prefix[1] == 'o')
				b = ElvFalse;
			else /* "neg" */
				b = (ELVBOOL)!dom->val[i].value.boolean;

			/* if there's a store function, then call it */
			if (dom->desc[i].store)
			{
				if ((*dom->desc[i].store)(&dom->desc[i], &dom->val[i], b ? o_true : o_false) < 0)
					ret = ElvFalse;
			}
			else
				dom->val[i].value.boolean = b;
		}
		else /* non-boolean with a value */
		{
			/* if the value is valid & different and we need to 
			 * call a store function, then call it.
			 */
			if ((*dom->desc[i].isvalid)(&dom->desc[i], &dom->val[i], value) == 1
			 && dom->desc[i].store)
			{
				if ((*dom->desc[i].store)(&dom->desc[i], &dom->val[i], value) < 0)
					ret = ElvFalse;
			}
		}

		/* set the "set" flag */
		if (!bang)
		{
			/* set or clear the "set" flag */
			if (!dom->desc[i].dflt
			 || CHARcmp(optgetstr(toCHAR(dom->desc[i].longname), NULL), dom->desc[i].dflt))
				dom->val[i].flags |= OPT_SET;
			else
				dom->val[i].flags &= ~OPT_SET;

#ifdef FEATURE_AUTOCMD
			/* if supposed to send an event, then do that */
			optautocmd(NULL, &dom->desc[i], &dom->val[i]);
#endif
		}

		/* If the "redraw" flag is set, then force redrawing (or at
		 * least regeneration) of all windows.
		 */
		if (dom->val[i].flags & (OPT_REDRAW|OPT_SCRATCH))
		{
			for (w = winofbuf(NULL, NULL); w; w = winofbuf(w, NULL))
			{
				if (dom->val[i].flags & OPT_SCRATCH)
					w->di->logic = DRAW_SCRATCH;
				else if (w->di->logic == DRAW_NORMAL)
					w->di->logic = DRAW_CHANGED;
			}
		}
	}

	/* show any options which we're supposed to show */
	optoutput(bang, ElvFalse, ElvFalse, outbuf, outsize);
	return ret;
}


/* This function returns the OPTVAL struct of an option, given a name.
 * If the string is not the name of an option, it returns NULL.
 */
OPTVAL *optval(name)
	char	*name;
{
	OPTDOMAIN *dom;
	int	  i;

	/* for each domain of options... */
	for (dom = head; dom; dom = dom->next)
	{
		/* for each option in that domain... */
		for (i = 0; i < dom->nopts; i++)
		{
			/* if this is the one we're looking for... */
			if (!strcmp(dom->desc[i].longname, name)
			 || !strcmp(dom->desc[i].shortname, name))
			{
				/* return the value struct */
				return &dom->val[i];
			}
		}
	}

	return NULL;
}

#ifdef FEATURE_AUTOCMD
/* This function is used for setting the "event" flag of a specific option,
 * or clearing the "event" flags for all options.  Returns the long name of
 * the option -- the one that should be checked for in the pattern.
 */
char *optevent(name)
	CHAR	*name;	/* the option that triggers events, NULL to clear all */
{
	OPTDOMAIN	*dom;
	int		i;
	static char	noname[30];

	/* for each domain of options... */
	for (dom = head; dom; dom = dom->next)
	{
		/* for each option in that domain... */
		for (i = 0; i < dom->nopts; i++)
		{
			/* if supposed to clear flags... */
			if (!name)
			{
				/* clear it */
				dom->desc[i].event = ElvFalse;
			}
			else if (!strcmp(dom->desc[i].longname, tochar8(name))
			 || !strcmp(dom->desc[i].shortname, tochar8(name)))
			{
				/* set this particular option's flag */
				dom->desc[i].event = ElvTrue;
				return dom->desc[i].longname;
			}
		}
	}

	/* maybe it has a "no" prefix? */
	if (!name || CHARncmp(name, toCHAR("no"), 2))
		return NULL;
	name += 2;

	/* for each domain of options... */
	for (dom = head; dom; dom = dom->next)
	{
		/* for each option in that domain... */
		for (i = 0; i < dom->nopts; i++)
		{
			if (!strcmp(dom->desc[i].longname, tochar8(name))
			 || !strcmp(dom->desc[i].shortname, tochar8(name)))
			{
				/* set this particular option's flag */
				dom->desc[i].event = ElvTrue;
				noname[0] = 'n';
				noname[1] = 'o';
				strcpy(noname+2, dom->desc[i].longname);
				return noname;
			}
		}
	}

	/* not found - return NULL */
	return NULL;
}
#endif /* FEATURE_AUTOCMD */

#ifdef FEATURE_MKEXRC
/* This function saves the values of some options.  It only does this for
 * options whose values have been changed, and which are in the "global",
 * "buf", "win", "syntax", or "lp" domains.
 */
void optsave(custom)
	BUFFER	custom;	/* where to stuff the "set" commands */
{
	MARKBUF   m;
	OPTDOMAIN *dom;
	int	  i, j, pass;
	CHAR	  *str;
	char	  *tmp;

	/* make two passes - one for universal options, and one for options
	 * which only apply to this GUI.
	 */
	for (pass = 1; pass <= 2; pass++)
	{
		/* for each domain of options... */
		for (dom = head; dom; dom = dom->next)
		{
			if (pass == 1)
			{
				/* ignore if not "global", "buf", "win", "lp",
				 * or "syntax"
				 */
				if (strcmp(dom->name, "global")
				 && strcmp(dom->name, "buf")
				 && strcmp(dom->name, "win")
				 && strcmp(dom->name, "lp")
				 && strcmp(dom->name, "syntax"))
					continue;
			}
			else /* pass == 2 */
			{
				/* ignore if not GUI global options or GUI
				 * window options.
				 */
				if (strcmp(dom->name, tochar8(o_gui))
				 && gui->optdescs != dom->desc)
					continue;
			}

			/* for each option in that domain... */
			for (i = 0; i < dom->nopts; i++)
			{
				/* if its value has been set... */
				if ((dom->val[i].flags & (OPT_SET|OPT_LOCK|OPT_NODFLT)) == OPT_SET)
				{
					/* then add it to the custom buffer */
					if (dom->desc[i].asstring)
					{
						str = (*dom->desc[i].asstring)(&dom->desc[i], &dom->val[i]);
						tmp = (char *)safealloc(11 + strlen(dom->desc[i].longname) + 2 * CHARlen(str), sizeof(char));
						if (pass == 2 || (dom->val[i].flags & OPT_UNSAFE))
							strcpy(tmp, "try set ");
						else
							strcpy(tmp, "set ");
						strcat(tmp, dom->desc[i].longname);
						strcat(tmp, "=");
						for (j = strlen(tmp); *str; )
						{
							if (*str == ' ' || *str == '\t' || *str == '|' || *str == '\\')
							{
								tmp[j++] = '\\';
							}
							tmp[j++] = (char)*str++;
						}
						tmp[j++] = '\n';
						tmp[j] = '\0';
					}
					else
					{
						tmp = (char *)safealloc(12 + strlen(dom->desc[i].longname), sizeof(char));
						sprintf(tmp, "%sset %s%s\n",
							(pass == 2 || (dom->val[i].flags & OPT_UNSAFE)) ? "try " : "",
							dom->val[i].value.boolean ? "" : "no",
							dom->desc[i].longname);
					}
					bufreplace(marktmp(m, custom, o_bufchars(custom)), &m, toCHAR(tmp), (long)strlen(tmp));
					safefree((void *)tmp);
				}
			}
		}
	}
}
#endif /* FEATURE_MKEXRC */


#ifdef FEATURE_COMPLETE
/* This function is used for completing an option name.  It searches
 * backward from the provided cursor position to collect the characters
 * of a partial option name, and then it looks for any known options
 * whose name matches that partial name.  It returns a statically-allocated
 * string containing any new characters that it could match.
 *
 * It only complete the long names.  Short names aren't worth the effort.
 */
CHAR *optcomplete(win, m)
	WINDOW	win;	/* the window where multiple matches are listed */
	MARK	m;	/* the cursor position (end of partial name) */
{
	char	partial[20];
 static CHAR	retbuf[100];
	int	plen;		/* length of partial string */
	CHAR	*cp;
	OPTDOMAIN *dom;
	char	*name;
	int	i, j, mlen = 0, mcount;
	ELVBOOL	isbool = ElvFalse;
	ELVBOOL	getvalue;

	/* if the cursor is located immediately after a '=' then skip back
	 * before the '=' so we can still get the option name.  Also set a
	 * flag we can test later so we know how to treat that name.
	 */
	getvalue = ElvFalse;
	scanalloc(&cp, m);
	if (!scanprev(&cp))
	{
		retbuf[0] = '\t';
		retbuf[1] = '\0';
		scanfree(&cp);
		return retbuf;
	}
	if (*cp == '=')
		getvalue = ElvTrue;
	else
		scannext(&cp);

	/* collect the characters of the partial name */
	partial[0] = '\0';
	for (plen = 0; scanprev(&cp) && elvalnum(*cp) && plen < QTY(partial)-1; )
	{
		memmove(partial + 1, partial, QTY(partial) - 1);
		partial[0] = *cp;
		plen++;
	}
	scanfree(&cp);

	/* if '=' then just get the value */
	if (getvalue)
	{
		memset(retbuf, 0, sizeof retbuf);
		cp = optgetstr(toCHAR(partial), NULL);
		if (!cp)
		{
			retbuf[0] = '\t';
			retbuf[1] = '\0';
		}
		else
		{
			cp = addquotes(toCHAR("\"|"), cp);
			if (CHARchr(cp, ' ') || CHARchr(cp, '\t'))
			{
				retbuf[0] = '"';
				CHARncpy(&retbuf[1], cp, QTY(retbuf) - 3);
				CHARcat(retbuf, toCHAR("\""));
			}
			else
			{
				CHARncpy(retbuf, cp, QTY(retbuf) - 1);
			}
			safefree(cp);
		}
		return retbuf;
	}

	/* look for matches */
	for (mcount = 0, dom = head; dom; dom = dom->next)
	{
		/* check every option in this domain */
		for (i = 0; i < dom->nopts; i++)
		{
			name = dom->desc[i].longname;
			if (!dom->desc[i].asstring /* boolean option */
			 && plen >= 2
			 && partial[0] == 'n' && partial[1] == 'o'
			 && !strncmp(name, partial + 2, plen - 2))
			{
				mcount++;
				isbool = ElvTrue;
				if (mcount == 1)
				{
					mlen = strlen(name) + 2;
					strcpy(partial + 2, name);
				}
				else
				{
					while (strncmp(partial + 2, name, mlen) != 0)
						mlen--;
				}
			}
			else if (!strncmp(name, partial, plen))
			{
				mcount++;
				isbool = (ELVBOOL)(!dom->desc[i].asstring);
				if (mcount == 1)
				{
					mlen = strlen(name);
					strcpy(partial, name);
				}
				else
				{
					while (strncmp(partial, name, mlen) != 0)
						mlen--;
				}
			}
		}
	}

	/* Copy the new matching chars into the return buffer */
	i = 0;
	if (mcount >= 1)
		for (i = 0, j = plen; j < mlen; )
			retbuf[i++] = partial[j++];

	/* Unless there are multiple matches, add a space or '=' */
	if (mcount <= 1)
		retbuf[i++] = isbool ? ' ' : '=';
	retbuf[i] = '\0';

	/* if no chars could be added, then list all matches */
	if (!retbuf[0])
	{
		j = 0;
		for (dom = head; dom; dom = dom->next)
			for (i = 0; i < dom->nopts; i++)
			{
				name = dom->desc[i].longname;
				if (!dom->desc[i].asstring /* boolean option */
				 && plen >= 2
				 && partial[0] == 'n' && partial[1] == 'o'
				 && !strncmp(name, partial + 2, plen - 2))
				{
					mlen = strlen(name);
					if (2 + j + 1 + mlen >= o_columns(win))
					{
						drawextext(win, toCHAR("\n"), 1);
						j = 0;
					}
					else if (j > 0)
					{
						drawextext(win, blanks, 1);
						j++;
					}
					drawextext(win, toCHAR("no"), 2);
					drawextext(win, toCHAR(name), mlen);
					j += 2 + mlen;
				}
				else if (!strncmp(name, partial, plen))
				{
					mlen = strlen(name);
					if (j + 1 + mlen >= o_columns(win))
					{
						drawextext(win, toCHAR("\n"), 1);
						j = 0;
					}
					else if (j > 0)
					{
						drawextext(win, blanks, 1);
						j++;
					}
					drawextext(win, toCHAR(name), mlen);
					j += mlen;
				}
			}
		if (j > 0)
			drawextext(win, toCHAR("\n"), 1);
	}

	/* return the matching chars */
	return retbuf;
}
#endif
