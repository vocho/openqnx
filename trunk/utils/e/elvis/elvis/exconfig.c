/* exconfig.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_exconfig[] = "$Id: exconfig.c,v 2.147 2003/10/19 23:13:33 steve Exp $";
#endif



/* This variable stores the current state of ex's control structures. */
EXCTLSTATE exctlstate;

static MAPFLAGS maphelp2 P_((CHAR **refcp, char *word, MAPFLAGS flag));
static MAPFLAGS maphelp P_((CHAR **refcp, CHAR **mode));
static CHAR *equaltilde P_((CHAR *value, int len, CHAR	*cmd));


#ifdef FEATURE_ALIAS
/* These are used for storing aliases */
typedef struct alias_s
{
	struct alias_s	*next;		/* some other alias */
	char		*name;		/* name of this alias */
	CHAR		*command;	/* commands for this alias */
	ELVBOOL		inuse;		/* is this alias already being run? */
	ELVBOOL		internal;	/* save in .exrc file? */
} alias_t;


BEGIN_EXTERNC
static void listalias P_((WINDOW win, alias_t *alias, ELVBOOL shortformat));
static void buildarg P_((CHAR **cmd, CHAR *arg, long len, CHAR *defarg, long deflen, ELVBOOL quote));
END_EXTERNC
static alias_t	*aliases;	/* This is the head of a list of aliases */


# ifdef FEATURE_COMPLETE
/* Return the name of an alias, by number.  If there is no such alias then
 * return NULL.  This is used only by the name completion code in ex.c.
 */
char *exaliasname(i)
	int	i;
{
	alias_t *alias;

	for (alias = aliases; --i >= 0 && alias; alias = alias->next)
	{
	}
	return alias ? alias->name : NULL;
}
# endif /* FEATURE_COMPLETE */

/* look up a name in the alias list.  The name can be terminated with any
 * non-alphanumeric character, not just '\0'.  Return its name if alias,
 * or NULL otherwise.  Optionally ignore if already in use.
 */
char *exisalias(name, inuse)
	char	*name;	/* name of a command, maybe an alias */
	ELVBOOL	inuse;	/* find even if in use? (else hide in-use aliases) */
{
	alias_t	*alias;

	/* look for alias */
	for (alias = aliases; alias; alias = alias->next)
		if (!strcmp(name, alias->name))
			return (alias->inuse && !inuse) ? NULL : alias->name;
	return NULL;
}


/* list a single alias */
static void listalias(win, alias, shortformat)
	WINDOW	win;
	alias_t	*alias;
	ELVBOOL	shortformat;
{
	CHAR	ch[4];
	CHAR	*start;
	int	len;
	int	indent;

	drawexlist(win, toCHAR(alias->name), strlen(alias->name));
	if (CHARchr(alias->command, '\n') == alias->command + CHARlen(alias->command) - 1)
	{
		/* single-line command simply follows the alias name */
		drawexlist(win, blanks, 10 - (CHARlen(alias->name) % 10));
		drawexlist(win, alias->command, CHARlen(alias->command));
	}
	else if (shortformat)
	{
		/* multi-line command, but only show first line */
		drawexlist(win, blanks, 10 - (CHARlen(alias->name) % 10));
		ch[0] = '{';
		ch[1] = ' ';
		drawexlist(win, ch, 2);
		for (start = alias->command, len = 0;
		     *start++ != '\n' && len < o_columns(win) - 16;
		   len++)
		{
		}
		drawexlist(win, alias->command, len);
		ch[0] = ch[1] = ch[2] = '.';
		ch[3] = '\n';
		drawexlist(win, ch, 4);
	}
	else
	{
		/* multi-line command is output in a fancy way */
		ch[0] = ' ';
		ch[1] = '{';
		ch[2] = '\n';
		drawexlist(win, ch, 3);
		indent = 4;
		for (start = alias->command, len = 0; *start; len++)
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


/* maintain the alias list */
RESULT	ex_alias(xinf)
	EXINFO	*xinf;
{
	alias_t	*newalias, *alias, *lag;
	int	i;

	/* if no aliases named, then list all */
	if (!xinf->lhs)
	{
		for (alias = aliases; alias; alias = alias->next)
		{
			if (xinf->bang == alias->internal)
				listalias(xinf->window, alias, ElvTrue);
		}
		return RESULT_COMPLETE;
	}

	/* Try to find the named alias */
	for (lag = NULL, alias = aliases;
	     alias && CHARcmp(xinf->lhs, toCHAR(alias->name));
	     lag = alias, alias = alias->next)
	{
	}

	/* Unaliasing? */
	if (xinf->command == EX_UNALIAS)
	{
		if (alias)
		{
			/* safety check */
			if (alias->inuse)
			{
				msg(MSG_ERROR, "[s]can't unalias $1 because it is in use", alias->name);
				return RESULT_ERROR;
			}

			/* remove it from the list, and free it */
			if (lag)
				lag->next = alias->next;
			else
				aliases = alias->next;
			safefree(alias->name);
			safefree(alias->command);
			safefree(alias);
		}
		return RESULT_COMPLETE;
	}

	/* listing one specific alias? */
	if (!xinf->rhs)
	{
		if (!alias)
			msg(MSG_WARNING, "[S]no alias named $1", xinf->lhs);
		else
			listalias(xinf->window, alias, ElvFalse);
		return RESULT_COMPLETE;
	}

	/* safety check */
	if (alias && alias->inuse)
	{
		msg(MSG_ERROR, "[s]can't redefine $1 because it is in use", alias->name);
		return RESULT_ERROR;
	}

	/* verify that the name contains only alphanumeric characters */
	for (i = 0; xinf->lhs[i]; i++)
	{
		if (!elvalnum(xinf->lhs[i]))
		{
			msg(MSG_ERROR, "alias names must be alphanumeric");
			return RESULT_ERROR;
		}
	}

	/* create or alter an alias */
	if (alias)
		safefree(alias->command);
	else
	{
		/* find the aliases before & after it, in ASCII order */
		for (lag = NULL, alias = aliases;
		     alias && CHARcmp(xinf->lhs, toCHAR(alias->name)) > 0;
		     lag = alias, alias = alias->next)
		{
		}

		/* allocate the new alias, and insert it into the list */
		newalias = (alias_t *)safekept(1, sizeof *alias);
		newalias->next = alias;
		if (lag)
			lag->next = newalias;
		else
			aliases = newalias;
		newalias->name = safekdup(tochar8(xinf->lhs));
		alias = newalias;
	}
#ifdef DEBUG_ALLOC
	alias->command = CHARkdup(xinf->rhs);
#else
	alias->command = xinf->rhs;
	xinf->rhs = NULL;
#endif
	alias->internal = xinf->bang;
	return RESULT_COMPLETE;
}


/* Add an argument to cmd, by calling buildCHAR() repeatedly. */
static void buildarg(cmd, arg, len, defarg, deflen, quote)
	CHAR	**cmd;	/* the resulting string */
	CHAR	*arg;	/* the arg to add */
	long	len;	/* length of arg */
	CHAR	*defarg;/* default, used if len == 0 */
	long	deflen;	/* length of defarg */
	ELVBOOL	quote;	/* should backslashes be inserted? */
{
	long	i;

	/* if normal arg is empty, then use defarg without quoting */
	if (len == 0)
	{
		arg = defarg;
		len = deflen;
		quote = ElvFalse;
	}

	/* copy characters, with optional quoting */
	for (i = 0; i < len; i++, arg++)
	{
		if (quote && (*arg == '\\' || (o_magicchar && CHARchr(o_magicchar, *arg))))
			buildCHAR(cmd, '\\');
		buildCHAR(cmd, *arg);
	}
}


/* Execute an alias */
RESULT	ex_doalias(xinf)
	EXINFO	*xinf;
{
	alias_t	*alias;
	CHAR	*cmd, *str, *defarg, *name;
	CHAR	*mustfree;
	CHAR	*args[11];
	long	lens[11];
	long	deflen;
	long	namelen;
	int	i, j;
	char	num[24];
	ELVBOOL	inword;
	ELVBOOL	anyargs, anyaddr, anybang;
	ELVBOOL	multiline;
	ELVBOOL	quote;
	RESULT	result = RESULT_ERROR;
#ifdef FEATURE_AUTOCMD
	void	*popopt;
#endif

	/* Find the alias.  It *will* exist, and use the same name pointer */
	for (alias = aliases; alias->name != xinf->cmdname; alias = alias->next)
	{
	}

	/* parse the args, if any.  args[0] is the whole argument string, and
	 * args[1] through args[9] are the first 9 words from that string.
	 */
	memset(lens, 0, sizeof lens);
	mustfree = NULL;
	if (xinf->rhs)
	{
		args[0] = xinf->rhs;
		lens[0] = CHARlen(args[0]);
#ifdef FEATURE_PROTO
		if (xinf->command == EX_DOPROTO)
		{
			/* parsing a URL -- skip the protocol, if given */
			str = xinf->rhs;
			for (i = 0; elvalpha(str[i]); i++)
			{
			}
			if (i > 0 && str[i] == ':')
				str += i + 1;

			/* parsing a URL -- first part is machine name */
			if (str[0] == '/' && str[1] == '/')
			{
				args[1] = str += 2;
				while (*str && *str != ':' && *str != '/' && *str != '?')
					str++;
				lens[1] = (int)(str - args[1]);
				if (*str == ':')
					do
					{
						str++;
					} while (elvdigit(*str));
			}

			/* next is the resource name */
			args[2] = str;
			while (*str && *str != '#' && *str != '?')
				str++;
			lens[2] = (int)(str - args[2]);

			/* next is the anchor name */
			if (*str == '#')
			{
				str++;
				args[3] = str;
				lens[3] = CHARlen(str);
				str += lens[3];
			}

			/* remainder is '&'-delimited, URL-encoded params */
			if (*str == '?' && str[1])
			{
				/* URL-decode the params, remembering lengths */
				for (str++, i = 4; i < 9 && *str; str++)
				{
					switch (*str)
					{
					  case '&':
						if (lens[i] > 0)
							i++;
						break;

					  case '+':
						buildCHAR(&mustfree, ' ');
						lens[i]++;
						break;

					  case '%':
						if (elvxdigit(str[1]) && elvxdigit(str[2]))
						{
							str++;
							if (elvdigit(*str))
								j = *str - '0';
							else
								j = (*str - 'A' + 10) & 0x0f;
							j <<= 4;
							str++;
							if (elvdigit(*str))
								j = *str - '0';
							else
								j = (*str - 'A' + 10) & 0x0f;
							buildCHAR(&mustfree, j);
							lens[i]++;
							break;
						}
						/* else fall through... */

					  default:
						buildCHAR(&mustfree, *str);
						lens[i]++;
					}
				}

				/* use lengths to find start of each param */
				for (j = 4, str = mustfree; j <= i; j++)
				{
					args[j] = str;
					str += lens[j];
				}
			}
		}
		else /* EX_DOALIAS */
#endif /* FEATURE_PROTO */
		{
			args[1] = args[0];
			for (i = 1, inword = ElvTrue, str = args[0]; *str && i < 10; str++)
			{
				if (inword)
				{
					if (elvspace(*str))
						inword = ElvFalse;
					else
						lens[i]++;
				}
				else if (!elvspace(*str))
				{
					args[++i] = str; 
					lens[i] = 1;
					inword = ElvTrue;
				}
			}
		}
	}

	/* Any missing args are zero-length */
	for (i = 1; i < QTY(args); i++)
		if (lens[i] == 0)
			args[i] = args[0] + lens[0];

	/* Build a copy of the command string, with !0-!9 replaced by args[0]
	 * through args[9].
	 */
	anyargs = anyaddr = anybang = multiline = ElvFalse;
	for (cmd = NULL, str = alias->command; *str; str++)
	{
		/* if not '!' then it can't be an arg substitution */
		if (*str != '!')
		{
			buildCHAR(&cmd, *str);
			if (*str == '\n' && str[1])
				multiline = ElvTrue;
			continue;
		}

		/* Allow an optional ':' after the '!'.  Also allow an optional
		 * \ which causes backslashes to be inserted before certain
		 * characters in the expansion.
		 */
		quote = ElvFalse;
		deflen = 0;
		defarg = args[0]; /* anything but NULL, really */
		str++;
		name = NULL;
		namelen = 0;
		while (*str == ':' || *str == '\\' || *str == '(' || elvalpha(*str))
		{
			if (*str == ':')
				str++;
			else if (*str == '\\')
				str++, quote = ElvTrue;
			else if (*str == '(')
			{
				str++;
				defarg = str;
				for (deflen = 0; *str != ')'; deflen++)
				{
					if (!*str || *str == '\n')
					{
						msg(MSG_ERROR, "malformed !() in alias $1", alias->name);
						if (cmd)
							safefree(cmd);
						if (mustfree)
							safefree(mustfree);
						return RESULT_ERROR;
					}
					str++;
				}
				str++;
			}
			else /* elvalpha(*str) -- only works for !name= and !name& */
			{
				name = str;
				for (namelen = 1; elvalnum(name[namelen]); namelen++)
				{
				}
				if (name[namelen]=='=' || name[namelen]=='&')
					str += namelen;
				else
				{
					name = NULL;
					break;
				}
			}
		}

		/* which substitution is being requested? */
		switch (*str)
		{
		  case '1':
		  case '2':
		  case '3':
		  case '4':
		  case '5':
		  case '6':
		  case '7':
		  case '8':
		  case '9':
			/* insert an argument */
			i = *str - '0';
			if (str[1] == '*')
			{
				/* !n* includes all items from !n to !$ */
				str++;
				buildarg(&cmd, args[i], lens[0] - (int)(args[i] - args[0]), defarg, deflen, quote);
			}
			else
				buildarg(&cmd, args[i], lens[i], defarg, deflen, quote);
			anyargs = ElvTrue;
			break;

		  case '*':
			/* insert the whole argument string */
			buildarg(&cmd, args[0], lens[0], defarg, deflen, quote);
			anyargs = ElvTrue;
			break;

		  case '^':
			/* insert the first argument string */
			buildarg(&cmd, args[1], lens[1], defarg, deflen, quote);
			anyargs = ElvTrue;
			break;

		  case '$':
			/* insert the last argument string */
			for (i = 1; i < QTY(lens) - 1 && lens[i + 1] > 0; i++)
			{
			}
			if (str[1] == '*')
			{
				/* !$* includes all items from !1 to !$-1 */
				str++;
				if (i != 1)
					buildarg(&cmd, args[0], lens[i - 1] + (int)(args[i - 1] - args[0]), defarg, deflen, quote);
			}
			else
				buildarg(&cmd, args[i], lens[i], defarg, deflen, quote);
			anyargs = ElvTrue;
			break;

		  case '!':
			buildCHAR(&cmd, '!');
			break;

		  case '?':
			if (xinf->bang)
				buildCHAR(&cmd, '!');
			anybang = ElvTrue;
			break;

		  case '<':
			if (xinf->anyaddr)
			{
				sprintf(num, "%ld", xinf->from);
				buildstr(&cmd, num);
			}
			else
				buildarg(&cmd, NULL, 0, defarg, deflen, quote);
			anyaddr = ElvTrue;
			break;

		  case '>':
			if (xinf->anyaddr)
			{
				sprintf(num, "%ld", xinf->to);
				buildstr(&cmd, num);
			}
			else
				buildarg(&cmd, NULL, 0, defarg, deflen, quote);
			anyaddr = ElvTrue;
			break;

		  case '%':
			if (xinf->anyaddr)
			{
				sprintf(num, "%ld,%ld", xinf->from, xinf->to);
				buildstr(&cmd, num);
			}
			else
				buildarg(&cmd, NULL, 0, defarg, deflen, quote);
			anyaddr = ElvTrue;
			break;

		  case '=':
		  case '&':
			/* If just "!=" with no name, then insert "!=" */
			if (!name)
			{
				buildCHAR(&cmd, '!');
				buildCHAR(&cmd, *str);
				break;
			}

			/* find a named arg & insert it (else use default) */
			for (i = 1;
			     i <= 9 && (lens[i] <= namelen
					|| CHARncmp(args[i], name, (int)namelen)
					|| args[i][namelen] != '=');
			     i++)
			{
			}
			if (i <= 9)
			{
				if (*str == '=')
				{
					buildarg(&cmd, args[i] + namelen+1,
						lens[i] - (namelen+1),
						defarg, deflen, quote);
				}
				else
				{
					for (name = args[i] + (namelen+1),
						j = lens[i] - (namelen+1);
					     --j >= 0;
					     name++)
					{
						switch (*name)
						{
						  case '\t':	buildstr(&cmd, "%09");	break;
						  case '+':	buildstr(&cmd, "%2B");	break;
						  case '"':	buildstr(&cmd, "%22");	break;
						  case '%':	buildstr(&cmd, "%25");	break;
						  case '<':	buildstr(&cmd, "%3C");	break;
						  case '>':	buildstr(&cmd, "%3E");	break;
						  case ' ':	buildCHAR(&cmd, '+');	break;
						  default:	buildCHAR(&cmd, *name);
						}
					
					}
				}
			}
			else
				buildarg(&cmd, NULL, 0, defarg, deflen, quote);
			anyargs = ElvTrue;
			break;

		  default:
			/* no substitution -- use a literal ! character */
			if (str[-1] == ':')
				buildCHAR(&cmd, '!');
			buildCHAR(&cmd, str[-1]);
			buildCHAR(&cmd, *str);
		}
	}

	/* don't need the mustfree strings anymore */
	if (mustfree)
		safefree(mustfree);

	/* If command contained no !n strings, but the alias was invoked with
	 * arguments, then append the arguments to the last command line.
	 */
	if (xinf->command == EX_DOALIAS && !anyargs && !multiline && lens[0] > 0)
	{
		cmd[CHARlen(cmd) - 1] = ' '; /* convert newline to space */
		buildarg(&cmd, args[0], lens[0], args[0], lens[0], ElvFalse);
		buildCHAR(&cmd, '\n');
		anyargs = ElvTrue;
	}

	/* Detect usage errors */
	if (xinf->command == EX_DOALIAS && xinf->bang && !anybang)
		msg(MSG_ERROR, "[s]the $1 alias doesn't use a ! suffix", alias->name);
	else if (xinf->command == EX_DOALIAS && xinf->anyaddr && !anyaddr)
		msg(MSG_ERROR, "[s]the $1 alias doesn't use addresses", alias->name);
	else if (xinf->command == EX_DOALIAS && lens[0] > 0 && !anyargs)
		msg(MSG_ERROR, "[s]the $1 alias doesn't use arguments", alias->name);
	else
	{
		/* No errors - Run the command.  Mark it as being "in use"
		 * while it is running, to prevent recursion.  If autocmd is
		 * supported, then do an AliasEnter event immediately after
		 * starting a :local context, and AliasLeave before restoring
		 * options to their non-local values.  This allws AliasEnter
		 * to use :local for settings that only affect the alias.  Note
		 * that exstring() also has its own internal :local logic, but
		 * the duplication doesn't cost much and keeps exstring()'s
		 * interface simple.
		 */
		alias->inuse = ElvTrue;
# ifdef FEATURE_AUTOCMD
#  ifdef FEATURE_MISC
		popopt = optlocal(NULL);
#  endif
		(void)auperform(xinf->window, ElvFalse, NULL, AU_ALIASENTER, toCHAR(alias->name));
# endif
		result = exstring(xinf->window, cmd, alias->name);
# ifdef FEATURE_AUTOCMD
		(void)auperform(xinf->window, ElvFalse, NULL, AU_ALIASENTER, toCHAR(alias->name));
#  ifdef FEATURE_MISC
		(void)optlocal(popopt);
#  endif
# endif
		alias->inuse = ElvFalse;
	}

	/* Free the copy of the command string */
	safefree(cmd);

	return result;
}

# ifdef FEATURE_MKEXRC

/* add user aliases to end of buffer */
void exaliassave(custom)
	BUFFER	custom;	/* the buffer to which the :au commands are added */
{
	ELVBOOL anyout;	/* any groups output yet? */
	MARKBUF	end;
	alias_t	*alias;
	CHAR	*cmd, *word;

	/* for each alias... */
	anyout = ElvFalse;
	for (alias = aliases; alias; alias = alias->next)
	{
		/* skip if standard - we don't need to save it since the
		 * standard initialization scripts will recreate it each time
		 * elvis is run anyway.
		 */
		if (alias->internal)
			continue;

		/* if first alias, output "try {" */
		if (!anyout)
		{
			anyout = ElvTrue;
			(void)marktmp(end, custom, o_bufchars(custom));
			bufreplace(&end, &end,
			    toCHAR("try {\n"), 6L);
		}

		/* construct an alias command string */
		cmd = NULL;
		buildstr(&cmd, " alias ");
		buildstr(&cmd, alias->name);
		buildCHAR(&cmd, ' ');
		word = alias->command;
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

	} /* for alias */

	/* if any groups were output, then we need to end "try {" */
	if (anyout)
	{
		end.offset = o_bufchars(custom);
		bufreplace(&end, &end, toCHAR("}\n"), 2L);
	}
}

# endif /* FEATURE_MKEXRC */
#endif /* FEATURE_ALIAS */

RESULT	ex_args(xinf)
	EXINFO	*xinf;
{
	int	i, col, len;
	char	**tmp;

	/* were we given a new args list? */
	if (xinf->nfiles > 0)
	{
		/* yes, use it */
		tmp = arglist;
		arglist = xinf->file;
		xinf->file = tmp;
		for (xinf->nfiles = 0; xinf->file[xinf->nfiles]; xinf->nfiles++)
		{
		}
		argnext = 0;
	}
	else
	{
		/* show current args list */
		for (i = col = 0; arglist[i]; i++)
		{
			len = strlen(arglist[i]);

			/* whitespace between args */
			if (i == 0)
				; /* no space is needed */
			else if (col + len + 4 > o_columns(xinf->window))
			{
				drawexlist(xinf->window, toCHAR("\n"), 1);
				col = 0;
			}
			else
			{
				drawexlist(xinf->window, toCHAR(" "), 1);
				col++;
			}

			/* Output the arg.  If current, enclose in '[' */
			if (i == argnext - 1)
				drawexlist(xinf->window, toCHAR("["), 1);
			drawexlist(xinf->window, toCHAR(arglist[i]), len);
			if (i == argnext - 1)
				drawexlist(xinf->window, toCHAR("]"), 1);
			col += len;
		}

		/* the final newline */
		drawexlist(xinf->window, toCHAR("\n"), 1);
	}
	return RESULT_COMPLETE;
}


/* This function implements the :color command. */
RESULT	ex_color(xinf)
	EXINFO	*xinf;
{
	int	i;
	WINDOW	win;
	CHAR	*lhs;

	/* If this GUI doesn't support colors, then do nothing */
	if (!gui->color)
	{
		if (xinf->bang)
			return RESULT_COMPLETE;
		msg(MSG_ERROR, "the (gui) user interface doesn't support colors");
		return RESULT_ERROR;
	}

	/* If "gui.role", where "gui" is the current GUI, then strip that off */
	lhs = xinf->lhs;
	i = CHARlen(o_gui);
	if (lhs && !CHARncmp(lhs, o_gui, i) && lhs[i] == '.' && lhs[i + 1])
		lhs += i + 1;

	if (!xinf->rhs)
	{
		/* no args, or just a role name - list the colors */
		colorlist(xinf->window, lhs, xinf->bang);
	}
	else if (CHARchr(lhs, '.'))
	{
		/* setting some other GUI's colors */
		if (!xinf->bang)
			colorforeign(lhs, xinf->rhs);
	}
	else 
	{
		/* both a role and a new color definition - set the colors */
		i = colorfind(lhs);
		if (i == 0)
			return RESULT_ERROR;
		colorset(i, xinf->rhs, (ELVBOOL)!xinf->bang);

		/* reset the GUI, to bypass any optimizations */
		guireset();

		/* Cause all windows to be redrawn from scratch */
		for (win = windows; win; win = win->next)
			win->di->logic = DRAW_SCRATCH;
	}
	return RESULT_COMPLETE;
}


RESULT	ex_comment(xinf)
	EXINFO	*xinf;
{
#ifdef FEATURE_CALC
	CHAR	*result;
#endif

	assert(xinf->command == EX_COMMENT || xinf->command == EX_ECHO
		|| xinf->command == EX_CALC || xinf->command == EX_GOTO);

	if (xinf->command == EX_ECHO && xinf->rhs)
	{
		drawextext(xinf->window, xinf->rhs, (int)CHARlen(xinf->rhs));
		drawextext(xinf->window, toCHAR("\n"), 1);
	}
#ifdef FEATURE_CALC
	else if (xinf->command == EX_CALC && xinf->rhs)
	{
		result = calculate(xinf->rhs, NULL, CALC_ALL);
		if (!result)
		{
			return RESULT_ERROR;
		}
		drawextext(xinf->window, result, (int)CHARlen(result));
		drawextext(xinf->window, toCHAR("\n"), 1);
	}
#endif
	else if (xinf->command == EX_GOTO && xinf->fromaddr)
	{
		if (xinf->fromoffset > markoffset(xinf->fromaddr)
		 && xinf->fromoffset <= markoffset(xinf->toaddr))
			xinf->newcurs = markalloc(markbuffer(xinf->fromaddr), xinf->fromoffset);
		else
			xinf->newcurs = markdup(xinf->fromaddr);
	}
	return RESULT_COMPLETE;
}


RESULT ex_message(xinf)
	EXINFO	*xinf;
{
	MSGIMP	imp;
	RESULT	result;

	/* choose an importance level for this message */
	switch (xinf->command)
	{
	  case EX_MESSAGE: imp = MSG_INFO;	result = RESULT_COMPLETE; break;
	  case EX_WARNING: imp = MSG_WARNING;	result = RESULT_COMPLETE; break;
	  case EX_ERROR:   imp = MSG_ERROR;	result = RESULT_ERROR;	  break;
	  default:
		/* this should never happen. */
#ifndef NDEBUG
		abort();
#endif
		return RESULT_ERROR;
	}

	/* do we have a message? */
	if (!xinf->rhs)
	{
		/* no - fake it for :error, else just return */
		if (xinf->command == MSG_ERROR)
			xinf->rhs = CHARdup(toCHAR("error"));
		else
			return result;
	}

	/* don't allow bracket at the beginning -- would look like args */
	if (*xinf->rhs == '[')
		*xinf->rhs = '{';

	/* output the message, or queue it */
	msg(imp, tochar8(xinf->rhs));

	/* return the result code */
	return result;
}



RESULT	ex_digraph(xinf)
	EXINFO	*xinf;
{
	digaction(xinf->window, xinf->bang, xinf->rhs);
	return RESULT_COMPLETE;
}


RESULT	ex_display(xinf)
	EXINFO	*xinf;
{
	int	len;
#ifdef FEATURE_NORMAL
	long	lback, last;
	BUFFER	buf;
	RESULT	result;
	MARK	cursor, orig;

	/* As a special case, ":normal vicmds" interprets "vicmds" as a series
	 * of vi commands.
	 */
	if (xinf->command == EX_NORMAL && xinf->rhs)
	{
		/* Count the chars in rhs.  Skip the last character, since it
		 * is just a newline marking the end of the string.
		 */
		len = CHARlen(xinf->rhs) - 1;

		/* remember the cursor position, we can restore it and return
		 * the new position the usual way (via xinf->newcurs).
		 */
		if (xinf->window->state->acton)
			cursor = xinf->window->state->acton->cursor;
		else
			cursor = xinf->window->cursor;
		orig = markdup(cursor);

		/* If no address was given, then run the command once */
		if (!xinf->anyaddr)
		{
			/* no addresses, run the command once, here */
			result = vinormal(xinf->window, len, xinf->rhs);
		}
		else
		{
			/* Otherwise we need to repeat the command on each
			 * line.  We want to do this in a top-to-bottom
			 * sequence, but the command may insert or delete
			 * lines so simply stepping by line number won't
			 * work.  Instead, we count relative to the end of
			 * the buffer.
			 */
			buf = xinf->defaddr.buffer;
			lback = o_buflines(buf) - xinf->from;
			last = o_buflines(buf) - xinf->to;
			result = RESULT_COMPLETE;
			for ( ;
			     lback >= last
				&& marksetline(cursor, o_buflines(buf) - lback);
			     lback--)
			{
				result = vinormal(xinf->window, len, xinf->rhs);
				if (result != RESULT_COMPLETE)
					break;
			}
		}

		/* if success, then stuff the new position into xinf->newcurs */
		if (result == RESULT_COMPLETE)
			xinf->newcurs = markdup(cursor);

		/* move the cursor back to its original position, so
		 * xinf->newcurs can change it gracefully
		 */
		marksetbuffer(cursor, markbuffer(orig));
		marksetoffset(cursor, markoffset(orig));

		return result;
	}
#endif

	/* List or change the display mode */
	if (xinf->command == EX_DISPLAY && !xinf->rhs)
	{
		displist(xinf->window);
	}
	else
	{
		/* trim trailing whitespace from the mode name */
		if (xinf->rhs)
		{
			for (len = CHARlen(xinf->rhs); xinf->rhs[--len] == ' ';)
				xinf->rhs[len] = '\0';
		}

		/* set the mode */
		if (!dispset(xinf->window, tochar8(xinf->rhs)))
			return RESULT_ERROR;
	}
	xinf->window->di->logic = DRAW_CHANGED;
	return RESULT_COMPLETE;
}


RESULT	ex_gui(xinf)
	EXINFO	*xinf;
{
	if (!gui->guicmd)
	{
		msg(MSG_ERROR, "gui-specific commands not supported");
		return RESULT_ERROR;
	}

	return (*gui->guicmd)(xinf->window ? xinf->window->gw : NULL, tochar8(xinf->rhs))
		? RESULT_COMPLETE
		: RESULT_ERROR;
}


#ifdef FEATURE_MISC
RESULT	ex_help(xinf)
	EXINFO	*xinf;
{
# ifndef DISPLAY_HTML
	msg(MSG_ERROR, "help unavailable; html mode is disabled");
	return RESULT_ERROR;
# else
	CHAR	*topic;	/* topic to search for; a tag name */
	char	*section;/* name of help file containing topic */
	CHAR	*tag;	/* name of tag to search for -- section#topic */
	BUFFER	buf;	/* buffer containing help text */
	MARK	tagdefn;/* result of search; where cursor should move to */
	OPTDESC	*od;	/* description struct of an option */
	int	len;
	int	i;
#ifdef FEATURE_ALIAS
	alias_t	*alias;
#endif

	/* remove trailing whitespace from args */
	if (xinf->lhs)
		for (topic = &xinf->lhs[CHARlen(xinf->lhs)];
		     topic-- != xinf->lhs && elvspace(*topic);
		     )
			*topic = '\0';
	if (xinf->rhs)
		for (topic = &xinf->rhs[CHARlen(xinf->rhs)];
		     topic-- != xinf->rhs && elvspace(*topic);
		     )
			*topic = '\0';

	/* construct a tag name for the requested topic */
	topic = NULL;
	len = xinf->lhs ? CHARlen(xinf->lhs) : 0;
	if (!xinf->lhs)
	{
		/* :help */
		topic = toCHAR("CONTENTS");
		section = "elvis.html";
	}
	else if (!CHARcmp(xinf->lhs, toCHAR("ex")))
	{
		/* :help ex */
		section = "elvisex.html";
	}
	else if (!CHARcmp(xinf->lhs, toCHAR("vi")))
	{
		/* :help vi */
		section = "elvisvi.html";
	}
	else if ((!CHARncmp(xinf->lhs, toCHAR("se"), 2)
			|| !CHARncmp(xinf->lhs, toCHAR(":se"), 3))
		&& xinf->rhs)
	{
		/* :help set optionname */

		/* strip off trailing =, if any */
		topic = xinf->rhs + CHARlen(xinf->rhs);
		while (*--topic == ' ' || *topic == '=')
		{
		}
		topic[1] = '\0';

		/* topic is option name */
		if (optgetstr(xinf->rhs, &od))
			topic = toCHAR(od->longname);
		else if (xinf->rhs[0] == 'n' && xinf->rhs[1] == 'o'
					&& (optgetstr(xinf->rhs + 2, &od)))
			topic = toCHAR(od->longname);
		else
			topic = xinf->rhs;
		section = "elvisopt.html";
	}
	else if ((!CHARncmp(xinf->lhs, toCHAR("di"), 2)
			|| !CHARncmp(xinf->lhs, toCHAR(":di"), 3))
		&& xinf->rhs)
	{
		/* :help display mode */

		/* topic is option name */
		topic = xinf->rhs;
		section = "elvisdm.html";
	}
#  ifdef FEATURE_AUTOCMD
	else if ((!CHARncmp(xinf->lhs, toCHAR("au"), 2)
			|| !CHARncmp(xinf->lhs, toCHAR(":au"), 3))
		&& xinf->rhs)
	{
		/* :help autocmd event */

		/* topic is event name */
		topic = auname(xinf->rhs);
		if (!topic)
			topic = toCHAR("AUTOCMD");
		section = "elvistip.html";
	}
#  endif
	else if (len > 1 && xinf->lhs[0] == ':')
	{
		/* :help :exname */
		section = "elvisex.html";
		topic = exname(xinf->lhs + 1);
#ifdef FEATURE_ALIAS
		if (!topic)
		{
			/* Try to find the named alias */
			for (alias = aliases;
			     alias && CHARcmp(xinf->lhs+1, toCHAR(alias->name));
			     alias = alias->next)
			{
			}

			/* if it's a ! alias then look in elvistip.html;
			 * else just tell the user it's an alias.
			 */
			if (alias)
			{
				if (!alias->internal)
				{
					msg(MSG_INFO, "[S]$1 is an alias.", xinf->lhs + 1);
					return RESULT_COMPLETE;
				}
				section = "elvistip.html";
				topic = xinf->lhs + 1;
			}
		}
#endif
		if (!topic)
			topic = toCHAR("GROUP");
	}
	else if (len > 1 && xinf->lhs[0] == '<' && xinf->lhs[1])
	{
		/* :help <name */

		/* strip off trailing > and leading </ */
		topic = xinf->lhs + CHARlen(xinf->lhs) - 1;
		if (*topic == '>')
			*topic = '\0';
		topic = xinf->lhs;
		if (topic[1] == '/')
			*++topic = '<';
		if (!elvalnum(topic[1]))
			topic = toCHAR("html");
		section = "elvisdm.html";
	}
	else if (len > 1 && elvalpha(xinf->lhs[0]) 
	      && (xinf->lhs[len - 1] == '(' || xinf->lhs[len - 1] == ')'))
	{
		/* :help function()  (where function is a name) */
		section = "elvisexp.html";
		do
		{
			len--;
		} while (!elvalpha(xinf->lhs[len - 1]));
		xinf->lhs[len] = '\0';
		topic = xinf->lhs;
	}
	else if ((topic = viname(xinf->lhs)) != NULL)
	{
		/* :help c  (where c is a vi command, usually single-char) */
		section = "elvisvi.html";
	}
	else if (optgetstr(xinf->lhs, &od) != NULL)
	{
		/* :help optionname */
		topic = toCHAR(od->longname);
		section = "elvisopt.html";
	}
	else if ((topic = exname(xinf->lhs)) != NULL)
	{
		/* :help exname */
		section = "elvisex.html";
	}
	else
	{
#ifdef FEATURE_ALIAS
		/* Try to find the named alias */
		for (alias = aliases;
		     alias && CHARcmp(xinf->lhs, toCHAR(alias->name));
		     alias = alias->next)
		{
		}

		/* if it's a ! alias then look in elvistip.html;
		 * else just tell the user it's an alias.
		 */
		if (alias)
		{
			if (!alias->internal)
			{
				msg(MSG_INFO, "[S]$1 is an alias.", xinf->lhs);
				return RESULT_COMPLETE;
			}
			section = "elvistip.html";
			topic = xinf->lhs;
		}
		else
#endif
		{
			/* Can't tell what user is looking for; perhaps the user
			 * doesn't know the syntax of :help ?  Teach them!
			 */
			topic = toCHAR("help");
			section = "elvisex.html";
		}
	}

	/* if help text not found, then give up */
	buf = bufpath(o_elvispath, section, toCHAR(section));
	if (!buf)
	{
		msg(MSG_ERROR, "[s]help not available; couldn't load $1", section);
		return RESULT_ERROR;
	}

	/* help text uses "html" display mode */
	if (optflags(o_bufdisplay(buf)) & OPT_FREE)
	{
		safefree(o_bufdisplay(buf));
		optflags(o_bufdisplay(buf)) &= ~OPT_FREE;
	}
	o_bufdisplay(buf) = toCHAR("html");
	o_initialsyntax(buf) = ElvFalse;

	/* combine section name and topic name to form a tag */
	if (topic)
	{
		tag = (CHAR *)safealloc((int)(CHARlen(o_filename(buf)) + CHARlen(topic) + 2), sizeof(CHAR));
		CHARcpy(tag, o_filename(buf));
		CHARcat(tag, toCHAR("#"));
		CHARcat(tag, topic);
	}
	else
	{
		tag = CHARdup(o_filename(buf));
	}

	/* perform tag lookup to find the the topic in the help file */
	tagdefn = (*dmhtml.tagload)(tag, NULL);
	if (!tagdefn)
	{
		msg(MSG_ERROR, "[S]no help available for $1", topic);
		safefree(tag);
		return RESULT_ERROR;
	}
	safefree(tag);

	/* Try to create a new window for the help text.  If that doesn't
	 * work, then use the original window and push the old cursor onto
	 * the tag stack.
	 */
	bufwilldo(tagdefn, ElvFalse);
	if ((*gui->creategw)(tochar8(o_bufname(markbuffer(tagdefn))), ""))
	{
		return RESULT_COMPLETE;
	}

	/* push the current cursor position and display mode onto tag stack */
	if (o_tagstack &&
		!o_internal(markbuffer(xinf->window->cursor)) &&
		o_filename(markbuffer(xinf->window->cursor)))
	{
		if (xinf->window->tagstack[TAGSTK - 1].prevtag)
			safefree(xinf->window->tagstack[TAGSTK - 1].prevtag);
		if (xinf->window->tagstack[TAGSTK - 1].origin)
			markfree(xinf->window->tagstack[TAGSTK - 1].origin);
		for (i = TAGSTK - 1; i > 0; i--)
		{
			xinf->window->tagstack[i] = xinf->window->tagstack[i - 1];
		}
		xinf->window->tagstack[0].prevtag = (o_previoustag ? CHARdup(o_previoustag) : NULL);
		xinf->window->tagstack[0].origin = markdup(xinf->window->cursor);
		xinf->window->tagstack[0].display = xinf->window->md->name;
	}

	/* arrange for the cursor to move to the tag position */
	xinf->newcurs = markdup(tagdefn);
	return RESULT_COMPLETE;
# endif /* DISPLAY_HTML */
}
#endif /* FEATURE_MISC */


#ifdef FEATURE_CALC
/* Evaluate an expression, and set the "then" flag according to result */
RESULT	ex_if(xinf)
	EXINFO	*xinf;
{
	CHAR	*result;

	/* expression is required */
	if (!xinf->rhs)
	{
		msg(MSG_ERROR, "missing rhs");
		return RESULT_ERROR;
	}

	/* evaluate expression */
	result = calculate(xinf->rhs, NULL, xinf->command == EX_EVAL ? CALC_MSG : CALC_ALL);
	if (!result)
	{
		return RESULT_ERROR;
	}

	if (xinf->command == EX_IF)
	{
		/* set "exthenflag" based on result of evaluation */
		exctlstate.thenflag = calctrue(result);
		return RESULT_COMPLETE;
	}
	else /* command == EX_EVAL */
	{
		/* execute the result as an ex command */
		return exstring(xinf->window, result, NULL);
	}
}
#endif /* FEATURE_CALC */

/* implements the :try, :then, and :else commands */
RESULT	ex_then(xinf)
	EXINFO	*xinf;
{
	RESULT	result = RESULT_COMPLETE;
	ELVBOOL	origmsg;
	CHAR	origsecurity;

	assert(xinf->command == EX_THEN || xinf->command == EX_ELSE
		|| xinf->command == EX_TRY || xinf->command == EX_SAFELY);

	/* If no commands, then do nothing */
	if (!xinf->rhs)
		return result;

	/* For :try, execute the commands unconditionally and then set the
	 * "exthenflag" to indicate whether the command succeeded.  For :safely,
	 * execute the commands unconditionally with security=safer temporarily.
	 * Otherwise (for :then and :else) execute the commands if "exthenflag"
	 * is set appropriately.
	 */
	if (xinf->command == EX_TRY)
	{
		origmsg = msghide(ElvTrue);
		exctlstate.thenflag = (ELVBOOL)(exstring(xinf->window, xinf->rhs, NULL) == RESULT_COMPLETE);
		(void)msghide(origmsg);
	}
	else if (xinf->command == EX_SAFELY)
	{
		origsecurity = o_security;
		if (o_security == 'n' /* normal */)
			o_security = 's' /* safer */;
		result = exstring(xinf->window, xinf->rhs, NULL);
		o_security = origsecurity;
	}
	else if (xinf->command == EX_THEN ? exctlstate.thenflag : !exctlstate.thenflag)
	{
		result = exstring(xinf->window, xinf->rhs, NULL);
	}

	return result;
}


#ifdef FEATURE_CALC
RESULT ex_while(xinf)
	EXINFO	*xinf;
{
	CHAR	*vals;
	int	thisfile;

	assert(xinf->command == EX_WHILE || xinf->command == EX_FOR);

	/* If there was some other, unused test lying around, then free it. */
	if (exctlstate.dotest)
		safefree(exctlstate.dotest);
	exctlstate.dotest = NULL;
	if (exctlstate.list)
	{
		safefree(exctlstate.list);
		exctlstate.list = NULL;
	}
	if (exctlstate.nfiles > 0)
	{
		for (thisfile = 0; thisfile < exctlstate.nfiles; thisfile++)
			safefree(exctlstate.file[thisfile]);
		safefree(exctlstate.file);
		exctlstate.nfiles = 0;
	}

	/* :for requires a variable name, which must be an option */
	if (xinf->command == EX_FOR)
	{
		if (!xinf->lhs)
		{
			msg(MSG_ERROR, "missing lhs");
			return RESULT_ERROR;
		}
		if (!optval(tochar8(xinf->lhs)))
		{
			msg(MSG_ERROR, "[S]bad option $1", xinf->lhs);
			return RESULT_ERROR;
		}
	}

	/* expression is required for :while, and the non-file version of :for*/
	if (!xinf->rhs && !(xinf->command == EX_FOR && xinf->nfiles > 0))
	{
		msg(MSG_ERROR, "missing rhs");
		return RESULT_ERROR;
	}

	if (xinf->command == EX_WHILE)
	{
		/* store the new test */
		exctlstate.dotest = xinf->rhs;
		xinf->rhs = NULL;
	}
	else if (xinf->nfiles > 0)
	{
		/* remember the files list */
		exctlstate.file = xinf->file;
		exctlstate.nfiles = xinf->nfiles;

		/* don't let the main ex code free this array */
		xinf->nfiles = 0;
		xinf->file = NULL;

		/* remember the name of the variable */
		exctlstate.dotest = xinf->lhs;
		xinf->lhs = NULL;
	}
	else
	{
		/* evaluate the for-list expression */
		vals = calculate(xinf->rhs, NULL, CALC_ALL);
		if (!vals)
			/* error message was already output by calculate() */
			return RESULT_ERROR;
		exctlstate.list = CHARdup(vals);

		/* remember the name of the variable */
		exctlstate.dotest = xinf->lhs;
		xinf->lhs = NULL;
	}

	return RESULT_COMPLETE;
}


RESULT ex_do(xinf)
	EXINFO	*xinf;
{
	CHAR	*value;
	CHAR	*end, atend;
	RESULT	result = RESULT_COMPLETE;
	int	thisfile;

	/* if no :while was executed before this, then fail */
	if (!exctlstate.dotest)
	{
		msg(MSG_ERROR, "missing :while or :for");
		return RESULT_ERROR;
	}

	/* while the expression is true and valid... */
	value = exctlstate.list;
	end = NULL;
	atend = '\0';
	thisfile = 0;
	while (result == RESULT_COMPLETE)
	{
		/* distinguish between "for" and "while" */
		if (exctlstate.list)
		{
			/* :for i (expr) */

			/* restore the character at the end of previous word */
			if (end)
			{
				*end = atend;
				value = end;
			}

			/* skip ahead to next word.  Break if none */
			while (*value && elvspace(*value))
				value++;
			if (!*value)
				break;

			/* locate the end of the word, and mark it */
			for (end = value; *end && !elvspace(*end); end++)
			{
			}
			atend = *end;
			*end = '\0';

			/* store the value in the variable */
			if (!optputstr(exctlstate.dotest, value, ElvFalse))
			{
				value = NULL;
				break;
			}
		}
		else if (exctlstate.nfiles > 0)
		{
			/* for i in files... */

			/* if no more files, then break */
			if (thisfile >= exctlstate.nfiles)
			{
				/* make sure this doesn't look like error */
				if (!value)
					value = toCHAR(exctlstate.file[0]);
				break;
			}

			/* copy the next file name into the variable */
			value = toCHAR(exctlstate.file[thisfile++]);
			if (!optputstr(exctlstate.dotest, value, ElvFalse))
			{
				value = NULL;
				break;
			}
		}
		else
		{
			/* :while */

			/* evaluate the expression.  Break if failed or false */
			value = calculate(exctlstate.dotest, NULL, CALC_ALL);
			if (!value || !calctrue(value))
				break;
		}

		/* Run the command.  If no command, then display result */
		if (xinf->rhs)
			result = exstring(xinf->window, xinf->rhs, NULL);
		else
		{
			drawextext(xinf->window, value, CHARlen(value));
			drawextext(xinf->window, toCHAR("\n"), 1);
		}

		/* is the user getting bored? */
		if (guipoll(ElvFalse))
			break;
	}

	/* free the dotest expression, and (if used) the list */
	safefree(exctlstate.dotest);
	exctlstate.dotest = NULL;
	if (exctlstate.list)
	{
		safefree(exctlstate.list);
		exctlstate.list = NULL;
	}
	if (exctlstate.nfiles > 0)
	{
		for (thisfile = 0; thisfile < exctlstate.nfiles; thisfile++)
			safefree(exctlstate.file[thisfile]);
		safefree(exctlstate.file);
		exctlstate.file = NULL;
		exctlstate.nfiles = 0;
	}

	/* if test could not be evaluated, then this command fails */
	if (!value)
		return RESULT_ERROR;
	return result;
}


/* Implements the :switch command -- evaluate an expression, and store the
 * result for use in later :case statements.
 */
RESULT ex_switch(xinf)
	EXINFO	*xinf;
{
	/* free the old switch value, if any */
	if (exctlstate.switchvalue)
		safefree(exctlstate.switchvalue);
	exctlstate.switchvalue = NULL;
	exctlstate.switchcarry = ElvFalse;

	/* verify that we were given an expression */
	if (!xinf->rhs)
	{
		msg(MSG_ERROR, "missing rhs");
		return RESULT_ERROR;
	}

	/* compute a new value */
	exctlstate.switchvalue = calculate(xinf->rhs, NULL, CALC_ALL);
	if (exctlstate.switchvalue)
	{
		exctlstate.switchvalue = CHARdup(exctlstate.switchvalue);
		return RESULT_COMPLETE;
	}
	return RESULT_ERROR;
}


/* This implements the :case and :default commands.  It tests the value from
 * a previous :switch command, and conditionally executes a command.
 */
RESULT ex_case(xinf)
	EXINFO	*xinf;
{
	/* check syntax */
	if (xinf->command == EX_CASE && !xinf->lhs)
	{
		msg(MSG_ERROR, "missing lhs");
		return RESULT_ERROR;
	}
	if (xinf->command == EX_DEFAULT && !xinf->rhs)
	{
		msg(MSG_ERROR, "missing ex command");
		return RESULT_ERROR;
	}

	/* if a previous case matched, then do nothing here... unless the
	 * previous match had no ex command line, and is therefor trying to
	 * execute this case's command line.
	 */
	if (!exctlstate.switchvalue && !exctlstate.switchcarry)
		return RESULT_COMPLETE;

	/* The :default command doesn't care about values, and we don't care
	 * when a previous match had no ex command line eithe.  Otherwise we
	 * need to check the :switch value against this case's value.
	 */
	if (!exctlstate.switchcarry && xinf->command != EX_DEFAULT)
	{
		if (CHARcmp(exctlstate.switchvalue, xinf->lhs))
			/* no match, so skip this case */
			return RESULT_COMPLETE;
	}

	/* It *DOES* match.  Clobber the exswitchvalue so each :switch will
	 * only match (at most) one case.  This also allows is to detect
	 * the default condition later.
	 */
	if (exctlstate.switchvalue)
	{
		safefree(exctlstate.switchvalue);
		exctlstate.switchvalue = NULL;
	}
	exctlstate.switchcarry = ElvFalse;

	/* Execute the command for this case.  If there is no command, then
	 * set a flag to indicate that the next case should match.
	 */
	if (xinf->rhs)
		return exstring(xinf->window, xinf->rhs, NULL);
	exctlstate.switchcarry = ElvTrue;		
	return RESULT_COMPLETE;
}
#endif /* FEATURE_CALC */


/* Test for a given word at the front of the map text.  If present, then advance
 * past it and return its flag bit.
 */
static MAPFLAGS maphelp2(refcp, word, flag)
	CHAR	**refcp;	/* front of the map text */
	char	*word;		/* a word to test for */
	MAPFLAGS flag;		/* flag to return if word is found */
{
	int	len = strlen(word);
	CHAR	*bigword = toCHAR(word);

	if (!CHARncmp(*refcp, bigword, len)
	 && (!(*refcp)[len] || elvspace((*refcp)[len])))
	{
		*refcp += len;
		while (elvspace(**refcp))
			(*refcp)++;
		return flag;
	}
	return (MAPFLAGS)0;
}

/* parse any number of map flags.  Return the MAPFLAGS value, and advance
 * *refcp past the flags.  If "mode=..." is seen, then set *refmode to point
 * to a static string giving the mode name.
 */
static MAPFLAGS maphelp(refcp, refmode)
	CHAR	**refcp;
	CHAR	**refmode;
{
	MAPFLAGS flags = (MAPFLAGS)0;
	CHAR	*start;
 static CHAR	modebuf[30];

	/* if no arg, then return no flags */
	if (!*refcp)
		return flags;

	/* parse any flags */
	do
	{
		/* skip extra whitespace */
		while (elvspace(**refcp))
			(*refcp)++;

		/* check for a word */
		start = *refcp;
		flags |= maphelp2(refcp, "input", MAP_INPUT);
		flags |= maphelp2(refcp, "history", MAP_HISTORY);
		flags |= maphelp2(refcp, "command", MAP_COMMAND);
		flags |= maphelp2(refcp, "motion", MAP_MOTION);
		flags |= maphelp2(refcp, "select", MAP_SELECT);
		flags |= maphelp2(refcp, "visual", MAP_ASCMD);
		flags |= maphelp2(refcp, "noremap", MAP_NOREMAP);
		flags |= maphelp2(refcp, "nosave", MAP_NOSAVE);
		if (!CHARncmp(*refcp, "mode=", 5))
		{
			for (start = modebuf, *refcp += 5;
			     start < &modebuf[sizeof modebuf -1]
				&& !elvspace(**refcp);
			     )
				*start++ = *(*refcp)++;
			*start = '\0';
			*refmode = modebuf;
		}
	} while (start != *refcp);

	return flags;
}

/* This implemented :map, :unmap, :abbr, :unabbr, :break, and :unbreak */
RESULT	ex_map(xinf)
	EXINFO	*xinf;
{
	CHAR	*line, *build;
	MAPFLAGS flags;
	int	len;
	CHAR	*mode, *lhs, *rhs;
	ELVBOOL	all;

	assert(xinf->command == EX_MAP || xinf->command == EX_ABBR
		|| xinf->command == EX_UNMAP || xinf->command == EX_UNABBR
		|| xinf->command == EX_BREAK || xinf->command == EX_UNBREAK);

	/* the lhs and rhs of maps are complicated by the fact that they may
	 * be prefixed by context flags.  This doesn't affect abbreviations.
	 */
	flags = 0;
	lhs = rhs = mode = NULL;
	if (xinf->command == EX_ABBR || xinf->command == EX_UNABBR)
	{
		/* standard parser is smart enough for :abbr */
		lhs = xinf->lhs;
		rhs = xinf->rhs;
	}
	else if (xinf->rhs)
	{
		/* for :map, everything is passed as part of the rhs, and we
		 * parse it out here.
		 */
		lhs = xinf->rhs;
		flags = maphelp(&lhs, &mode);
		for (build = rhs = lhs; *rhs && !elvspace(*rhs); )
		{
			if (*rhs == ELVCTRL('V') && rhs[1])
				rhs++;
			*build++ = *rhs++;
		}
		if (*rhs)
			*rhs++ = '\0';
		*build = '\0';
		flags |= maphelp(&rhs, &mode);
		for (build = line = rhs; *line; )
		{
			if (*line == ELVCTRL('V') && line[1])
				line++;
			*build++ = *line++;
		}
		*build = '\0';
		if (!*rhs)
			rhs = NULL;
		if (!*lhs)
			lhs = NULL;
	}

	/* detect "map all" */
	all = ElvFalse;
	if (xinf->command == EX_MAP && lhs && !rhs && !CHARcmp(lhs, toCHAR("all")))
	{
		lhs = NULL;
		all = ElvTrue;
	}

	/* check for missing mandatory arguments */
	if ((xinf->command == EX_MAP || xinf->command == EX_ABBR)
		&& lhs && !rhs)
	{
		msg(MSG_ERROR, "missing rhs");
		return RESULT_ERROR;
	}
	if ((xinf->command == EX_UNMAP || xinf->command == EX_UNABBR
		   || xinf->command == EX_BREAK || xinf->command == EX_UNBREAK)
		&& !lhs)
	{
		msg(MSG_ERROR, "missing lhs");
		return RESULT_ERROR;
	}

	/* choose which flags to map */
	if (xinf->command == EX_ABBR || xinf->command == EX_UNABBR)
	{
		flags = MAP_ABBR|(xinf->bang ? MAP_HISTORY : MAP_INPUT);
	}
	else
	{
		/* using :map! implies "input history" */
		if (xinf->bang)
			flags |= MAP_INPUT|MAP_HISTORY;

		/* "visual" implies at least one of "input history" */
		if ((flags & (MAP_INPUT|MAP_HISTORY|MAP_ASCMD)) == MAP_ASCMD)
			flags |= MAP_INPUT|MAP_HISTORY;

		/* if no other context specified, assume "command motion select" */
		if ((flags & MAP_WHEN) == 0)
			flags |= MAP_COMMAND|MAP_MOTION|MAP_SELECT;
	}

	/* either list, unmap, or map */
	if (!lhs)
	{
		/* can't list maps before the first window is created */
		if (xinf->window)
			while ((line = maplist(flags & (MAP_WHEN|MAP_ABBR), mode, &len)) != (CHAR *)0)
			{
				if (*line == ' ' || all)
					drawextext(xinf->window, line, len);
			}
	}
	else if (!rhs)
	{
		(void)mapdelete(lhs, (int)CHARlen(lhs), flags, mode,
			(ELVBOOL)(xinf->command == EX_UNMAP || xinf->command == EX_UNABBR),
			(ELVBOOL)(xinf->command == EX_BREAK));
	}
	else 
	{
		mapinsert(lhs, (int)CHARlen(lhs), rhs, (int)CHARlen(rhs), (CHAR *)0, flags, mode);
	}
	return RESULT_COMPLETE;
}


#ifdef FEATURE_EQUALTILDE
static CHAR *equaltilde(value, len, cmd)
	CHAR	*value;	/*value to act on */
	int	len;	/* length of value (which might not be NUL terminated)*/
	CHAR	*cmd;	/* ex command to apply to the value */
{
	BUFFER	buf, prevdef;
	MARKBUF top, bottom;
	RESULT	result;

	/* create a temporary buffer */
	buf = bufalloc(toCHAR(EQUALTILDE_BUF), 0, ElvTrue);

	/* store the value into the buffer */
	bufreplace(marktmp(top, buf, 0L), marktmp(bottom, buf, o_bufchars(buf)),
		value, len);
	bufreplace(marktmp(bottom, buf, o_bufchars(buf)), &bottom, toCHAR("\n"), 1);

#if 0
	/* temporarily move the cursor to this buffer */
	top = *windefault->cursor;
	marksetoffset(windefault->cursor, 0L);
	marksetbuffer(windefault->cursor, buf);
#else
	/* temporarily make this the default buffer */
	prevdef = bufdefault;
	bufoptions(buf);
#endif

	/* apply the ex command line to the buffer */
	result = exstring(windefault, cmd, "equaltilde");

#if 0
	/* restore the cursor */
	marksetbuffer(windefault->cursor, top.buffer);
	marksetoffset(windefault->cursor, top.offset);
#else
	/* restore the buffer */
	bufoptions(prevdef);
#endif

	/* if the command failed or has no characters, return NULL */
	if (result != RESULT_COMPLETE || o_bufchars(buf) == 0L)
		return NULL;

	/* else return the buffer's contents as a string */
	return bufmemory(marktmp(top, buf, 0L), marktmp(bottom, buf, o_bufchars(buf) - 1L));
}
#endif

RESULT	ex_set(xinf)
	EXINFO	*xinf;
{
	CHAR	outbuf[10000];
	static CHAR empty[1];
#ifdef FEATURE_CALC
	CHAR	*value;
	CHAR	*opereq = NULL;
	int	i;
	int	j;

# ifdef FEATURE_ARRAY
	CHUNK	chunks[3];
	CHAR	*build;
	CHAR	*element;
	CHAR	*oldval = NULL;
# endif

#ifdef FEATURE_EQUALTILDE
	ELVBOOL	doequaltilde = ElvFalse;
	CHAR	*mustfree = NULL;
#endif

	if (xinf->command == EX_LET)
	{
		i = 0;
		if (!xinf->rhs)
		{
			goto MissingRHS;
		}

		/* copy name into outbuf[], so we can nul-terminate it */
		for ( ; xinf->rhs && elvalnum(xinf->rhs[i]); i++)
		{
			outbuf[i] = xinf->rhs[i];
		}
		outbuf[i] = '\0';

# ifdef FEATURE_ARRAY
		/* if "[" then get the array subscript */
		if (xinf->rhs[i] == '[' || xinf->rhs[i] == '.')
		{
			/* get the option's value, as a string.  We need to
			 * save it in a private buffer since optgetstr()
			 * returns a static buffer, and calculate() also uses
			 * optgetstr.  So we copy it into outbuf.
			 */
			value = optgetstr(outbuf, NULL);
			if (!value)
			{
				msg(MSG_ERROR, "[S]bad option $1", outbuf);
				return RESULT_ERROR;
			}
			oldval = outbuf + i + 1;
			CHARncpy(oldval, value, QTY(outbuf) - 100);
			oldval[QTY(oldval) - 100] = '\0';

			/* parse the subscript -- it will be alphanumeric */
			if (xinf->rhs[i] == '.')
			{
				/* collect the chars */
				for (build = NULL;
				     xinf->rhs[++i] && elvalnum(xinf->rhs[i]);
				     buildCHAR(&build, xinf->rhs[i]))
				{
				}

				/* if the string is a set with a valueless
				 * element of that name, then delete the name.
				 */
				element = calcelement(oldval, build);
				if (element && *element != ':')
				{
					j = CHARlen(build);
					if (*element == ',')
						element++, j++;
					else if (oldval + j != element)
						j++;
					memmove(element - j, element, (CHARlen(element) + 1) * sizeof(CHAR));
				}

				/* find the chunks in the option's value */
				(void)calcsubscript(oldval, build, QTY(chunks), chunks);

				/* if no chunks, then append the name and try
				 * again.
				 */
				if (chunks[0].ptr == NULL)
				{
					if (oldval[0])
						CHARcat(oldval, toCHAR(","));
					CHARcat(oldval, build);
					CHARcat(oldval, toCHAR(":"));
					(void)calcsubscript(oldval, build, QTY(chunks), chunks);
				}
				safefree(build);
			}
			else /* it will end with "] = " */
			{
				for (build = NULL; xinf->rhs[++i]; buildCHAR(&build, xinf->rhs[i]))
				{
					/* if "] = " then stop */
					if (xinf->rhs[i] == ']')
					{
						j = i + 1;
						while (xinf->rhs[j] && elvspace(xinf->rhs[j]))
							j++;
						if (xinf->rhs[j] == '=' && xinf->rhs[j + 1] != '=')
							break;
					}
				}
				i++;

				/* evaluate it */
				if (build)
				{
					value = calculate(build, NULL, CALC_ALL);
					safefree(build);
					if (!value)
						return RESULT_ERROR;

					/* find the chunks in the option's value */
					(void)calcsubscript(oldval, value, QTY(chunks), chunks);
				}
				else
					value = NULL;
			}

			/* can't assign to element 0 (the length), or to
			 * scattered chunks.
			 */
			if (!value
			 || chunks[0].ptr == NULL
			 || chunks[0].ptr < oldval
			 || chunks[0].ptr + chunks[0].len > oldval + CHARlen(oldval)
			 || chunks[1].ptr)
			{
				msg(MSG_ERROR, "invalid subscript for assignment");
				return RESULT_ERROR;
			}
		}
# endif /* FEATURE_ARRAY */

		/* skip whitespace */
		while (elvspace(xinf->rhs[i]))
		{
			i++;
		}

		/* if we're using an <oper>= then adjust the expression */
		if (xinf->rhs[i] != '=' && elvpunct(xinf->rhs[i]))
		{
			/* convert ":let option oper= expr" to
			 * ":let option = option oper (expr)"
			 */
			for (j = 0; j < i; j++)
				buildCHAR(&opereq, xinf->rhs[j]);
			while (xinf->rhs[i] != '=' && xinf->rhs[i])
				buildCHAR(&opereq, xinf->rhs[i++]);
			buildCHAR(&opereq, ' ');
			buildCHAR(&opereq, '(');
			for (j = i + 1; xinf->rhs[j]; j++)
				buildCHAR(&opereq, xinf->rhs[j]);
			buildCHAR(&opereq, ')');
		}

		/* skip '=' */
		if (xinf->rhs[i] != '=' || !outbuf[0])
		{
			goto MissingRHS;
		}
		i++;

# ifdef FEATURE_EQUALTILDE
		/* detect if we're using "=~" */
		if (!opereq && xinf->rhs[i] == '~')
		{
			doequaltilde = ElvTrue;
			i++;
		}
# endif

		/* skip whitespace after the '=' */
		while (elvspace(xinf->rhs[i]))
		{
			i++;
		}
		if (!xinf->rhs[i])
		{
MissingRHS:
			if (opereq)
				safefree(opereq);
			msg(MSG_ERROR, "missing rhs");
			return RESULT_ERROR;
		}

		/* evaluate the expression */
# ifdef FEATURE_EQUALTILDE
		if (doequaltilde)
		{
#  ifdef FEATURE_ARRAY
			if (oldval)
			{
				mustfree = value = equaltilde(chunks[0].ptr, chunks[0].len, &xinf->rhs[i]);
			}
			else
#  endif
			{
				value = optgetstr(outbuf, NULL);
				mustfree = value = equaltilde(value, CHARlen(value), &xinf->rhs[i]);
			}
		}
		else
# endif
		value = calculate(opereq ? opereq : &xinf->rhs[i], NULL, CALC_ALL);
		if (!value)
		{
			/* error message already given */
			if (opereq)
				safefree(opereq);
#ifdef FEATURE_EQUALTILDE
			if (mustfree)
				safefree(mustfree);
#endif
			return RESULT_ERROR;
		}

		/* store the result */
# ifdef FEATURE_ARRAY
		if (oldval)
		{
			/* the subscripted portion of the value is always one
			 * chunk, and it excludes any required delimiters.  So
			 * all we need to do is copy text before the chunk,
			 * copy the new text, and copy the text after the chunk.
			 */
			build = NULL;
			for (i = 0; i < (int)(chunks[0].ptr - oldval); i++)
				buildCHAR(&build, oldval[i]);
			while (*value)
				buildCHAR(&build, *value++);
			for (i += chunks[0].len; oldval[i]; i++)
				buildCHAR(&build, oldval[i]);
			if (!optputstr(outbuf, build, xinf->bang))
			{
				safefree(build);
				if (opereq)
					safefree(opereq);
#  ifdef FEATURE_EQUALTILDE
				if (mustfree)
					safefree(mustfree);
#  endif
				return RESULT_ERROR;
			}
			safefree(build);
		}
		else /* no subscripts */
# endif /* FEATURE_ARRAY */
		{
			if (!optputstr(outbuf, value, xinf->bang))
			{
				/* error message already given */
				if (opereq)
					safefree(opereq);
#  ifdef FEATURE_EQUALTILDE
				if (mustfree)
					safefree(mustfree);
#  endif
				return RESULT_ERROR;
			}
		}
	}
	else
#endif /* FEATURE_CALC */
#ifdef FEATURE_MISC
	if (xinf->command == EX_LOCAL)
	{
		if (!xinf->rhs)
		{
			msg(MSG_ERROR, "missing rhs");
			return RESULT_ERROR;
		}
		if (!optset(xinf->bang, xinf->rhs, NULL, 0))
		{
			return RESULT_ERROR;
		}
	}
	else
#endif /* FEATURE_MISC */
	/* command == EX_SET */
	{
		if (!optset(xinf->bang, xinf->rhs ? xinf->rhs : empty, outbuf, QTY(outbuf)))
		{
			return RESULT_ERROR;
		}
		if (*outbuf)
		{
			drawexlist(windefault, outbuf, (int)CHARlen(outbuf));
		}
	}
	return RESULT_COMPLETE;
}


RESULT	ex_version(xinf)
	EXINFO	*xinf;
{
	msg(MSG_INFO, "[s]elvis $1", VERSION);
#ifdef COPY1
	msg(MSG_INFO, "[s]$1", COPY1);
#endif
#ifdef COPY2
	msg(MSG_INFO, "[s]$1", COPY2);
#endif
#ifdef COPY3
	msg(MSG_INFO, "[s]$1", COPY3);
#endif
#ifdef COPY4
	msg(MSG_INFO, "[s]$1", COPY4);
#endif
#ifdef COPY5
	msg(MSG_INFO, "[s]$1", COPY5);
#endif
#ifdef PORTEDBY
	msg(MSG_INFO, "[s]Ported to (os) by $1", PORTEDBY);
#endif
	return RESULT_COMPLETE;
}


RESULT	ex_qall(xinf)
	EXINFO	*xinf;
{
	WINDOW	win, except;
	WINDOW	orig;
	RESULT	result;
	ELVBOOL	didorig;

	assert(xinf->command == EX_QALL || xinf->command == EX_PRESERVE
		|| xinf->command == EX_ONLY);

	/* If :preserve, then turn off the tempsession flag */
	if (xinf->command == EX_PRESERVE)
	{
		o_tempsession = ElvFalse;
	}

	/* if :only, then use EX_CLOSE but don't close this window */
	if (xinf->command == EX_ONLY)
	{
		xinf->command = EX_CLOSE;
		except = xinf->window;
	}
	else
	{
		xinf->command = EX_QUIT;
		except = NULL;
	}

	/* run the command on each window, except possibly this one */
	orig = xinf->window;
	for (win = winofbuf(NULL, NULL), result = RESULT_COMPLETE, didorig = ElvFalse;
	     win;
	     win = winofbuf(win, NULL))
	{
		/* maybe skip the current window */
		if (win == except)
		{
			didorig = ElvTrue;
			continue;
		}

		xinf->window = win;
		if (ex_xit(xinf) != RESULT_COMPLETE)
		{
			result = RESULT_ERROR;
		}
		else if (win != orig)
		{
			/* Need to explicitly delete all windows except the
			 * current one.  The current one will go away
			 * automatically when the ex_qall() function exits.
			 */
			(*gui->destroygw)(win->gw, ElvFalse);
			win = didorig ? orig : NULL;
		}
		else
		{
			didorig = ElvTrue;
		}
	}
	return result;
}


RESULT	ex_xit(xinf)
	EXINFO	*xinf;
{
	BUFFER	buf;		/* the buffer to be saved */
	MARKBUF	top, bottom;
	STATE	*state;
	BUFFER	b;		/* other buffers */
 static	long	morechgs;	/* change counter at last "more files" warning */
 static	BUFFER	morebuf;	/* buffer which morechgs value refers to */

	assert(xinf->command == EX_CLOSE || xinf->command == EX_QUIT
		|| xinf->command == EX_XIT || xinf->command == EX_WQUIT);

	/* Save the buffer, if :wquit or modified and :xit */
	buf = markbuffer(xinf->window->cursor);
	if (xinf->command == EX_WQUIT ||
		(o_modified(buf) && xinf->command == EX_XIT))
	{
		/* Write to named file or the buffer's original file.
		 * If can't write, then fail.
		 */
		if (xinf->nfiles == 1
			? !bufwrite(marktmp(top, buf, 0), marktmp(bottom, buf, o_bufchars(buf)), xinf->file[0], xinf->bang)
			: !bufsave(buf, xinf->bang, ElvTrue))
		{
			/* an error message has already been output */
			return RESULT_ERROR;
		}
	}

	/* if :q on a modified buffer, and no other window is showing this
	 * buffer, then either (without !) complain or (with !) turn off the
	 * modified flag.
	 */
	if (xinf->command == EX_QUIT
	 && o_modified(buf)
	 && winofbuf(NULL, buf) == xinf->window
	 && winofbuf(xinf->window, buf) == NULL)
	{
		if (xinf->bang)
		{
			o_modified(buf) = ElvFalse;
		}
		else
		{
			msg(MSG_ERROR, "[S]$1 modified, not saved", o_filename(buf) ? o_filename(buf) : o_bufname(buf));
			return RESULT_ERROR;
		}
	}

	/* If this is the last window, then make sure *ALL* user buffers
	 * have been saved.  Exception: If :close! and this session isn't
	 * temporary, then we don't need to check buffers.
	 */
	if ((morebuf != markbuffer(xinf->window->cursor) || morebuf->changes != morechgs || xinf->command == EX_CLOSE)
	 && !(xinf->command == EX_CLOSE && xinf->bang && o_tempsession)
	 && winofbuf(NULL, NULL) == xinf->window && !winofbuf(xinf->window, NULL))
	{
		/* remember some stuff which should prevent us from warning
		 * the user more than once.
		 */
		morebuf = markbuffer(xinf->window->cursor);
		morechgs = morebuf->changes;

		/* check all buffers */
		for (b = elvis_buffers; b; b = buflist(b))
		{
			if (!o_internal(b) && o_modified(b)
				&& (xinf->command == EX_CLOSE || b != buf))
			{
				/* We've found a modified, unsaved user buffer.
				 * If the command is :q! then we want to
				 * discard all buffers; otherwise we want to
				 * warn the user that other buffers need to
				 * be checked.
				 */
				if (xinf->command == EX_QUIT && xinf->bang)
					o_modified(b) = ElvFalse;
				else
				{
					msg(MSG_ERROR, "check other buffers");
					return RESULT_ERROR;
				}
			}
		}

		/* also check for more files to edit */
		if (arglist && (argnext < 0 || arglist[argnext])
		 && (xinf->command != EX_QUIT || !xinf->bang))
		{
			msg(MSG_WARNING, "more files");
			return RESULT_ERROR;
		}
	}

	/* If :close, then set the "retain" flag on the window's main buffer. */
	if (xinf->command == EX_CLOSE)
	{
		o_retain(markbuffer(xinf->window->cursor)) = ElvTrue;
	}

	/* Arrange for the state stack to pop everything.  This will cause
	 * the window to be closed, eventually.
	 */
	for (state = xinf->window->state; state; state = state->pop)
	{
		state->flags |= ELVIS_POP;
	}
	return RESULT_COMPLETE;
}

#ifdef FEATURE_SPELL

RESULT	ex_check(xinf)
	EXINFO	*xinf;
{
	long	flags = 0;
	CHAR	*word, *end, atend;
	int	i;
	ELVBOOL	casesensitive;

	assert(xinf->command == EX_CHECK || xinf->command == EX_WORDS);

	/* if no words then dump current personal words */
	if (!xinf->rhs)
	{
		if (xinf->command == EX_CHECK)
			spellcheckfont(NULL, xinf->bang ? SPELL_CHECK_TAGONLY : SPELL_CHECK_ALL, xinf->bang);
		else
			spellsave(NULL);
		return RESULT_COMPLETE;
	}

	/* find the start of each word.  Also notice "+" or "-" */
	for (word = xinf->rhs; *word; word++)
	{
		if (*word == '+')
			flags = 0;
		else if (*word == '-')
			flags = SPELL_FLAG_BAD;
		else if (*word == '*' && xinf->command == EX_CHECK)
			flags = -1;
		else if (!elvspace(*word))
		{
			/* found a word!  now see how long it is */
			casesensitive = (ELVBOOL)elvupper(*word);
			for (end = word + 1;
			     elvalnum(*end) || *end == '_' || *end == '\'';
			     end++)
			{
				casesensitive |= elvupper(*word);
			}

			/* temporarily mark the end with a NUL character */
			atend = *end;
			*end = '\0';

			/* add font or word, depending on the command */
			if (xinf->command == EX_CHECK)
			{
				/* add the font */
				spellcheckfont(word,
					flags == -1 ? SPELL_CHECK_ALL :
					(flags & SPELL_FLAG_BAD) ? SPELL_CHECK_NONE :
					SPELL_CHECK_TAGONLY, xinf->bang);
			}
			else if (CHARlen(word) < 100) /* personal word */
			{
				/* add the word */
				if (!xinf->bang)
					flags |= SPELL_FLAG_PERSONAL;
				spellwords = spelladdword(spellwords, word,
							flags);

				/* if case sensitive, then make the
				 * wrong-case version be a "wrong" word.
				 * There's no need to make this be treated
				 * as a personal word.
				 */
				if (casesensitive)
				{
					for (i = 0; &word[i] != end; i++)
						word[i] = elvtolower(word[i]);
					spellwords = spelladdword(spellwords,
							word, SPELL_FLAG_BAD);
				}
			}

			/* prepare for next word */
			*end = atend;
			word = end - 1; /* advances to end in for(;;) stmt */
		}
	}
	return RESULT_COMPLETE;
}

RESULT	ex_wordfile(xinf)
	EXINFO	*xinf;
{
	int	i;

	assert(xinf->command == EX_WORDFILE);

	/* check arguments */
	if (xinf->nfiles == 0)
	{
		if (!o_spelldict || !*o_spelldict)
		{
			msg(MSG_ERROR, "missing file name");
			return RESULT_ERROR;
		}
		spellload(tochar8(o_spelldict), ElvFalse);
	}
	else
	{
		for (i = 0; i < xinf->nfiles; i++)
		{
			spellload(xinf->file[i], (ELVBOOL)!xinf->bang);
		}
	}

	return RESULT_COMPLETE;
}

#endif /* FEATURE_SPELL */
