/* optglob.c */
/* Copyright 1995 by Steve Kirkendall */


/* This file contains gobal options for the portable parts of elvis. */

#include "elvis.h"
#ifdef FEATURE_RCSID
char id_optglob[] = "$Id: optglob.c,v 2.110 2003/10/21 00:18:16 steve Exp $";
#endif

#ifdef FEATURE_LISTCHARS
static int optislistchars P_((struct optdesc_s *opt, OPTVAL *val, CHAR *newval));
#else
# define optislistchars optispacked
#endif

/* descriptions of the global options */
static OPTDESC ogdesc[] =
{
	{"blksize", "bsz",	optnstring,	optisnumber,	"256:8192" },
	{"blkhash", "hash",	optnstring,	optisnumber,	"1:500" },
	{"blkcache", "cache",	optnstring,	optisnumber,	"5:200" },
	{"blkgrow", "bgr",	optnstring,	optisnumber,	"1:32" },
	{"blkfill", "bfill",	optnstring,	optisnumber	},
	{"blkhit", "bh",	optnstring,	optisnumber	},
	{"blkmiss", "bm",	optnstring,	optisnumber	},
	{"blkwrite", "bw",	optnstring,	optisnumber	},
	{"version", "ver",	optsstring,	optisstring	},
	{"bitsperchar", "bits",	optnstring,	optisnumber	},
	{"gui", "gui",		optsstring,	optisstring	},
	{"os", "os",		optsstring,	optisstring	},
	{"session", "ses",	optsstring,	optisstring	},
	{"recovering", "rflag", NULL,		NULL		},
	{"digraph", "dig",	NULL,		NULL		},
	{"exrc", "ex",		NULL,		NULL		},
	{"modeline","ml",	NULL,		NULL		},
	{"modelines","mls",	optnstring,	optisnumber,	"1:100"},
	{"ignorecase", "ic",	NULL,		NULL		},
	{"magic", "ma",		NULL,		NULL		},
	{"magicchar", "mac",	optsstring,	optisstring	},
	{"magicname", "man",	NULL,		NULL,		},
	{"magicperl", "map",	NULL,		NULL,		},
	{"novice", "novice",	NULL,		NULL		},
	{"prompt", "prompt",	NULL,		NULL		},
	{"remap", "remap",	NULL,		NULL		},
	{"report", "report",	optnstring,	optisnumber	},
	{"shell", "sh",		optsstring,	optisstring	},
	{"sync", "sync",	NULL,		NULL		},
	{"taglength", "tl",	optnstring,	optisnumber	},
	{"tagkind", "tk",	NULL,		NULL		},
	{"taglibrary", "tlib",	NULL,		NULL		},
	{"tags", "tagpath",	optsstring,	optisstring	},
	{"tagstack", "tsk",	NULL,		NULL		},
	{"tagprg", "tp",	optsstring,	optisstring	},
	{"autoprint", "ap",	NULL,		NULL		},
	{"autowrite", "aw",	NULL,		NULL		},
	{"autoselect", "as",	NULL,		NULL,		},
	{"warn", "warn",	NULL,		NULL		},
	{"window", "wi",	optnstring,	optisnumber	},
	{"wrapscan", "ws",	NULL,		NULL		},
	{"writeany", "wa",	NULL,		NULL		},
	{"defaultreadonly","dro",NULL,		NULL		},
	{"initialstate","init",	opt1string,	optisoneof,	"input replace vi ex"},
	{"exitcode", "exit",	optnstring,	optisnumber,	"0:255"},
	{"keytime", "kt",	optnstring,	optisnumber,	"0:20"},
	{"usertime", "ut",	optnstring,	optisnumber,	"0:20"},
	{"security", "sec",	opt1string,	optisoneof,	"normal safer restricted"},
	{"tempsession", "temp",	NULL,		NULL		},
	{"newsession", "temp",	NULL,		NULL		},
	{"exrefresh", "er",	NULL,		NULL		},
	{"home", "home",	optsstring,	optisstring	},
	{"elvispath", "epath",	optsstring,	optisstring	},
	{"terse", "te",		NULL,		NULL		},
	{"previousdir", "pdir",	optsstring,	optisstring	},
	{"previousfile", "#",	optsstring,	optisstring	},
	{"previousfileline","@",optnstring,	optisnumber	},
	{"previouscommand", "!",optsstring,	optisstring	},
	{"previoustag", "ptag",	optsstring,	optisstring	},
	{"nearscroll", "ns",	optnstring,	optisnumber,	"0:100"},
	{"optimize", "op",	NULL,		NULL		},
	{"edcompatible", "ed",	NULL,		NULL		},
	{"pollfrequency", "pf",	optnstring,	optisnumber,	"1:1000"},
	{"sentenceend", "se",	optsstring,	optisstring	},
	{"sentencequote", "sq",	optsstring,	optisstring	},
	{"sentencegap",	"sg",	optnstring,	optisnumber,	"0:3"},
	{"verbose", "-v",	optnstring,	optisnumber,	"0:9"},
	{"anyerror", "ae",	NULL,		NULL		},
	{"directory", "dir",	optsstring,	optisstring	},
	{"errorbells", "eb",	NULL,		NULL		},
	{"warningbells", "wb",	NULL,		NULL		},
	{"flash", "vbell",	NULL,		NULL		},
	{"program", "argv0",	optsstring,	optisstring	},
	{"backup", "bk",	NULL,		NULL		},
	{"showmarkups", "smu",	NULL,		NULL		},
	{"nonascii", "asc",	opt1string,	optisoneof,	"all most none strip"},
	{"beautify", "bf",	NULL,		NULL		},
	{"mesg", "mesg",	NULL,		NULL		},
	{"sessionpath", "spath",optsstring,	optisstring	},
	{"maptrace", "mt",	opt1string,	optisoneof,	"off run step"},
	{"maplog", "mlog",	opt1string,	optisoneof,	"off reset append"},
	{"gdefault", "gd",	NULL,		NULL		},
	{"matchchar", "mc",	optsstring,	optisstring	},
	{"show", "show",	optsstring,	optisstring	},
	{"writeeol", "weol",	opt1string,	optisoneof,	"unix dos mac text binary same"},
	{"binary", "bin",	NULL,		NULL		},
	{"saveregexp", "sre",	NULL,		NULL		},
	{"true", "True",	optsstring,	optisstring	},
	{"false", "False",	optsstring,	optisstring	},
	{"animation", "anim",	optnstring,	optisnumber,	"1:100"},
	{"completebinary", "cob", NULL,		NULL		},
	{"optionwidth", "ow",	optnstring,	optisnumber,	},
	{"smarttab", "sta",	NULL,		NULL		},
	{"smartcase", "scs",	NULL,		NULL		},
	{"hlsearch", "hls",	NULL,		NULL		},
	{"background", "bg",	opt1string,	colorisbg,	"light dark"},
	{"incsearch", "is",	NULL,		NULL		},
	{"spelldict", "spd",	optsstring,	optisstring	},
	{"spellautoload","sal",	NULL,		NULL		},
	{"spellsuffix", "sps",	optsstring,	optisstring	},
	{"locale", "locale",	optsstring,	optisstring	},
	{"mkexrcfile", "rc",	optsstring,	optisstring	},
	{"prefersyntax","psyn",	opt1string,	optisoneof,	"never local writable always" },
	{"eventignore", "ei",	optsstring,	optisstring	},
	{"eventerrors", "ee",	NULL,		NULL,		},
	{"tweaksection", "twks",NULL,		NULL,		},
	{"timeout", "to",       NULL,		NULL		},
	{"listchars", "lcs",	optsstring,	optislistchars,	"eol:,tab:,trail:,ff:,cr:,esc:,bs:,del:,nul:,precedes:,extends:,markup" },
	{"cleantext", "ct",	optsstring,	optispacked,	"bs,input,short,long,ex" },
	{"filenamerules", "fnr",optsstring,	optispacked,	"tilde,dollar,paren,wildcard,special,space" },
	/* added these for the sake of backward compatibility : */
	{"more", "mo",		NULL,		NULL		},
	{"hardtabs", "ht",	optnstring,	optisnumber,	"1:1000"},
	{"redraw", "red",	NULL,		NULL		}
};


/* where the values are stored */
OPTVAL optglob[QTY_GLOBAL_OPTS];


#ifdef FEATURE_LPR
/* printer options */
static OPTDESC lpdesc[] =
{
	{"lptype", "lpt",	optsstring,	optisstring	},
	{"lpcrlf", "lpc",	NULL,		NULL 		},
	{"lpout", "lpo",	optsstring,	optisstring	},
	{"lpcolumns", "lpcols",	optnstring,	optisnumber,	"0:300"},
	{"lpwrap", "lpw",	NULL,		NULL		},
	{"lplines", "lprows",	optnstring,	optisnumber,	"0:100"},
	{"lpconvert", "lpcvt",	NULL,		NULL		},
	{"lpformfeed", "lpff",	NULL,		NULL		},
	{"lpoptions", "lpopt",	optsstring,	optispacked,	"paper:,frame:,bar:,punch:,clip:"},
	{"lpnumber", "lpn",	NULL,		NULL		},
	{"lpheader", "lph",	NULL,		NULL		},
	{"lpcolor", "lpcl",	NULL,		NULL		},
	{"lpcontrast", "lpct",	optnstring,	optisnumber,	"0:100"}
};

/* where the values are stored */
OPTVAL lpval[QTY_LP_OPTS];
#endif /* FEATURE_LPR */


#ifndef NO_USERVARS
/* descriptions of the user options */
static OPTDESC userdesc[] =
{
	{"a", "a",		optsstring,	optisstring	},
	{"b", "b",		optsstring,	optisstring	},
	{"c", "c",		optsstring,	optisstring	},
	{"d", "d",		optsstring,	optisstring	},
	{"e", "e",		optsstring,	optisstring	},
	{"f", "f",		optsstring,	optisstring	},
	{"g", "g",		optsstring,	optisstring	},
	{"h", "h",		optsstring,	optisstring	},
	{"i", "i",		optsstring,	optisstring	},
	{"j", "j",		optsstring,	optisstring	},
	{"k", "k",		optsstring,	optisstring	},
	{"l", "l",		optsstring,	optisstring	},
	{"m", "m",		optsstring,	optisstring	},
	{"n", "n",		optsstring,	optisstring	},
	{"o", "o",		optsstring,	optisstring	},
	{"p", "p",		optsstring,	optisstring	},
	{"q", "q",		optsstring,	optisstring	},
	{"r", "r",		optsstring,	optisstring	},
	{"s", "s",		optsstring,	optisstring	},
	{"t", "t",		optsstring,	optisstring	},
	{"u", "u",		optsstring,	optisstring	},
	{"v", "v",		optsstring,	optisstring	},
	{"w", "w",		optsstring,	optisstring	},
	{"x", "x",		optsstring,	optisstring	},
	{"y", "y",		optsstring,	optisstring	},
	{"z", "z",		optsstring,	optisstring	},
};

/* where the values are stored */
static OPTVAL optuser[QTY(userdesc)];
#endif /* not NO_USERVARS */


/* This function inializes the global options.  This can't be done in the
 * usual way, because the values are declared as a union, and C doesn't allow
 * unions to be initialized.
 */
void optglobinit()
{
	int	i;
	char	*envval;

	assert(QTY(ogdesc) == QTY_GLOBAL_OPTS);
#ifdef FEATURE_LPR
	assert(QTY(lpdesc) == QTY_LP_OPTS);
#endif

	/* set each option to a reasonable default */
	optpreset(o_blksize, BLKSIZE, OPT_LOCK|OPT_HIDE);
	optpreset(o_blkhash, BLKHASH, OPT_HIDE);
	optpreset(o_blkcache, BLKCACHE, OPT_HIDE);
	optpreset(o_blkgrow, BLKGROW, OPT_HIDE);
	optflags(o_blkfill) = OPT_LOCK|OPT_HIDE;
	optflags(o_blkhit) = OPT_LOCK|OPT_HIDE;
	optflags(o_blkmiss) = OPT_LOCK|OPT_HIDE;
	optflags(o_blkwrite) = OPT_LOCK|OPT_HIDE;
	optpreset(o_version, toCHAR(VERSION), OPT_LOCK|OPT_HIDE);
	optpreset(o_bitsperchar, 8 * sizeof(CHAR), OPT_LOCK|OPT_HIDE);
	optpreset(o_os, toCHAR(OSNAME), OPT_LOCK|OPT_HIDE);
	optflags(o_gui) = OPT_LOCK|OPT_HIDE;
	optflags(o_session) = OPT_LOCK|OPT_HIDE;
	optflags(o_recovering) = OPT_HIDE;
	o_magic = ElvTrue;
	optpreset(o_magicchar, toCHAR("^$.[*"), OPT_HIDE);
	optpreset(o_magicname, ElvFalse, OPT_HIDE);
	optpreset(o_magicperl, ElvFalse, OPT_HIDE);
	o_prompt = ElvTrue;
	o_autoprint = ElvTrue;
	o_remap = ElvTrue;
	o_report = 5;
	optpreset(o_modeline, ElvFalse, OPT_UNSAFE);
	optpreset(o_modelines, 5, OPT_HIDE);
#ifdef OSSHELLENV
	optpreset(o_shell, toCHAR(getenv(OSSHELLENV)), OPT_HIDE | OPT_UNSAFE);
#else
	optpreset(o_shell, toCHAR(getenv("SHELL")), OPT_HIDE | OPT_UNSAFE);
#endif
	if (!o_shell)
		o_shell = toCHAR(OSSHELL);
	o_tagstack = ElvTrue;
	o_tagkind = ElvFalse;
	o_taglibrary = ElvFalse;
	o_warn = ElvTrue;
	o_window = 12;
	o_wrapscan = ElvTrue;
	optpreset(o_initialstate,'v', OPT_HIDE | OPT_NODFLT); /* vi */
	o_keytime = 3;
	o_usertime = 15;
	optflags(o_exitcode) = OPT_HIDE;
	optpreset(o_security, 'n', OPT_HIDE|OPT_UNSAFE|OPT_NODFLT); /* normal */
	optflags(o_tempsession) = OPT_HIDE;
	optflags(o_newsession) = OPT_HIDE;
	optpreset(o_nearscroll, 10, OPT_HIDE);
	optflags(o_previousfile) = OPT_HIDE|OPT_LOCK;
	optflags(o_previousfileline) = OPT_HIDE|OPT_LOCK;
	optflags(o_previouscommand) = OPT_HIDE|OPT_LOCK;
	optflags(o_previoustag) = OPT_HIDE|OPT_LOCK;
	optflags(o_tagprg) = OPT_HIDE|OPT_UNSAFE;
	o_optimize = ElvTrue;
	optpreset(o_pollfrequency, 20, OPT_HIDE);
	optflags(o_sentenceend) = OPT_HIDE;
	optflags(o_sentencequote) = OPT_HIDE;
	optpreset(o_sentencegap, 2, OPT_HIDE);
	if (!o_directory)
		o_directory = toCHAR(getenv("TMP"));
#ifdef OSDIRECTORY
	if (!o_directory)
		o_directory = toCHAR(OSDIRECTORY);
#endif
	o_errorbells = ElvTrue;
	optpreset(o_nonascii, 'm', OPT_HIDE); /* most */
	optflags(o_digraph) = OPT_HIDE;
	optpreset(o_sync, ElvFalse, OPT_HIDE);
	optflags(o_autoselect) = OPT_HIDE;
	optflags(o_defaultreadonly) = OPT_HIDE;
	optflags(o_exrefresh) = OPT_HIDE;
	optflags(o_verbose) = OPT_HIDE;
	optflags(o_anyerror) = OPT_HIDE;
	optflags(o_program) = OPT_HIDE;
	optpreset(o_mesg, ElvTrue, OPT_HIDE);
	optpreset(o_maptrace, 'o', OPT_HIDE); /* off */
	optpreset(o_maplog, 'o', OPT_HIDE); /* off */
	o_timeout = ElvTrue;
	optpreset(o_matchchar, toCHAR("[]{}()"), OPT_HIDE);
	optpreset(o_show, toCHAR("spell/tag,region"), OPT_HIDE);
	o_writeeol = 's'; /* same */
	optpreset(o_binary, ElvFalse, OPT_HIDE);
	optpreset(o_saveregexp, ElvTrue, OPT_HIDE);
	optpreset(o_hardtabs, 8, OPT_HIDE);
	optpreset(o_redraw, ElvFalse, OPT_HIDE);
	optpreset(o_true, msgtranslate("True"), OPT_HIDE|OPT_FREE);
	optpreset(o_false, msgtranslate("False"), OPT_HIDE|OPT_FREE);
	optpreset(o_animation, 3, OPT_HIDE);
	optpreset(o_completebinary, ElvFalse, OPT_HIDE);
	optpreset(o_optionwidth, 24, OPT_HIDE);
	optpreset(o_smarttab, ElvFalse, OPT_HIDE);
	optpreset(o_smartcase, ElvFalse, OPT_HIDE);
	optpreset(o_hlsearch, ElvFalse, OPT_HIDE);
	o_incsearch = ElvFalse;
	optpreset(o_spelldict, NULL, OPT_HIDE | OPT_UNSAFE);
	optpreset(o_spellsuffix, NULL, OPT_HIDE);
	optpreset(o_locale, NULL, OPT_HIDE);
	optpreset(o_mkexrcfile, NULL, OPT_HIDE);
	optpreset(o_prefersyntax, 'n', OPT_HIDE); /* never */
	optpreset(o_eventignore, NULL, OPT_HIDE);
	optpreset(o_eventerrors, ElvFalse, OPT_HIDE);
	optpreset(o_tweaksection, ElvTrue, OPT_HIDE);
	optpreset(o_listchars, toCHAR("eol:$"), OPT_HIDE);
#ifdef FEATURE_LISTCHARS
	(void)dmnlistchars('<', -1L, 0, NULL, NULL);
#endif
	optpreset(o_cleantext, toCHAR("long"), OPT_HIDE);
	optpreset(o_filenamerules, toCHAR("tilde,dollar,paren,wildcard,special,space"), OPT_HIDE);

	/* Set the "home" option from $HOME */
	envval = getenv("HOME");
	if (envval)
	{
		if (optflags(o_home) & OPT_FREE)
			safefree(o_home);
		o_home = toCHAR(envval);
	}
	else if (!o_home)
	{
		o_home = toCHAR(".");
	}
	optflags(o_home) |= OPT_HIDE | OPT_UNSAFE;

	/* Set the "previousdir" option from $OLDPWD */
	envval = getenv("OLDPWD");
	if (envval)
		o_previousdir = toCHAR(envval);
	else
		o_previousdir = toCHAR(".");
	optflags(o_previousdir) |= OPT_HIDE;

	/* Set the "tags" option from $TAGPATH */
	o_tags = toCHAR(getenv("TAGPATH"));
	if (!o_tags)
	{
		o_tags = toCHAR("tags");
	}

	/* Set the "background" option from $ELVISBG */
	envval = getenv("ELVISBG");
	if (!envval || (strcmp(envval, "dark") && strcmp(envval, "light")))
		envval = "dark";
	optpreset(o_background, *envval, OPT_HIDE|OPT_NODFLT);

	/* Generate the default elvispath value. */
	envval = getenv("ELVISPATH");
	if (envval)
	{
		if (optflags(o_elvispath) & OPT_FREE)
			safefree(o_elvispath);
		o_elvispath = toCHAR(envval);
	}
	else if (!o_elvispath)
	{
		o_elvispath = toCHAR(OSLIBPATH);
	}
	optflags(o_elvispath) |= OPT_HIDE | OPT_UNSAFE;

	/* Generate the default sessionpath value. */
	envval = getenv("SESSIONPATH");
	if (envval)
	{
		if (optflags(o_sessionpath) & OPT_FREE)
			safefree(o_sessionpath);
		o_sessionpath = toCHAR(envval);
	}
	else if (!o_sessionpath)
	{
#ifdef OSSESSIONPATH
		o_sessionpath = toCHAR(OSSESSIONPATH);
#else
		o_sessionpath = toCHAR("~:.");
#endif
	}
	optflags(o_sessionpath) |= (OPT_HIDE | OPT_LOCK);

#ifdef FEATURE_LPR
	/* initialize the printing options */
# ifdef OSLPTYPE
	optpreset(o_lptype, toCHAR(OSLPTYPE), OPT_HIDE);
# else
	optpreset(o_lptype, toCHAR("dumb"), OPT_HIDE);
# endif
	optpreset(o_lpcrlf, ElvFalse, OPT_HIDE);
	optpreset(o_lpout, toCHAR(OSLPOUT), OPT_HIDE | OPT_UNSAFE);
	optpreset(o_lpcolumns, 80, OPT_HIDE);
	optpreset(o_lpwrap, ElvTrue, OPT_HIDE);
	optpreset(o_lplines, 60, OPT_HIDE);
	optpreset(o_lpconvert, ElvFalse, OPT_HIDE);
	optpreset(o_lpformfeed, ElvFalse, OPT_HIDE);
	optpreset(o_lpoptions, NULL, OPT_HIDE);
	optpreset(o_lpnumber, ElvFalse, OPT_HIDE);
	optpreset(o_lpheader, ElvTrue, OPT_HIDE);
	optpreset(o_lpcolor, ElvFalse, OPT_HIDE);
	optpreset(o_lpcontrast, 50, OPT_HIDE);
#endif /* FEATURE_LPR */

	/* inform the options code about these options */
	optinsert("global", QTY(ogdesc), ogdesc, optglob);
#ifdef FEATURE_LPR
	optinsert("lp", QTY(lpdesc), lpdesc, lpval);
#endif

#ifndef NO_USERVARS
	/* initialize user variables */
	for (i = 0; i < QTY(userdesc); i++)
	{
		optflags(optuser[i]) = OPT_HIDE;
	}
	optinsert("user", QTY(userdesc), userdesc, optuser);
#endif
}

#ifdef FEATURE_LISTCHARS
/* Store a value for the "listchars" option, and then parse it */
static int optislistchars(opt, val, newval)
	struct optdesc_s *opt;
	OPTVAL *val;
	CHAR *newval;
{
	int	result;

	/* try to change it as a packed list */
	result = optispacked(opt, val, newval);
	if (result <= 0)
		return result;

	/* it succeeded and was different -- use dmnlistchars to parse it */
	(void)dmnlistchars('<', -1L, 0L, NULL, NULL);

	/* return 1 to indicate a successful change */
	return 1;
}
#endif

/* This function sets the "previousfile" and "previousfileline" options. */
void optprevfile(filename, line)
	CHAR	*filename;	/* new value for "previousfile" */
	long	line;		/* new value for "previousfileline" */
{
	/* if no new filename, then don't clobber the old one */
	if (!filename)
		return;

	/* if there was an old previousfile, then free its storage space */
	if (o_previousfile)
		safefree(o_previousfile);

	/* store the new filename and line number */
	o_previousfile = CHARdup(filename);
	o_previousfileline = line;
}
