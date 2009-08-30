/* unix/osdir.c */

/* This file contains functions that deal with filenames.  The structure of
 * this file is a little unusual, because some of the support programs also
 * use functions from this file.  To minimize the size of those support
 * programs, this file may be compiled with JUST_DIRFIRST or JUST_DIRPATH
 * defined to exclude the unused functions.  If neither of those names is
 * defined then the whole file is compiled.
 */


#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <pwd.h>
#ifndef S_ISREG
# define S_ISREG(mode)	(((mode) & 0070000) == 0)
#endif
#if !defined(JUST_DIRFIRST) && !defined(JUST_DIRPATH)
# include "elvis.h"
#ifdef FEATURE_RCSID
char id_osdir[] = "$Id: osdir.c,v 2.29 2003/10/17 17:41:23 steve Exp $";
#endif
#endif

#ifndef NAME_MAX
# ifdef MAXNAMLEN
#  define NAME_MAX MAXNAMLEN
# else
#  define NAME_MAX 1024
# endif
#endif

#ifndef JUST_DIRPATH

/* This is the wildcard expression used by dirfirst() and dirnext() */
static char *wildfile;

/* This is the name of the directory being scanned by dirfirst() and dirnext() */
static char *wilddir;

/* This is the directory stream used by dirfirst() and dirnext() */
static DIR *dirfp;

/* This variable is ElvTrue if the wildcard expression only needs to match the
 * beginning of the name, or ElvFalse if it must match the entire filename.
 */
static ELVBOOL partial;

/* This recursive function checks a filename against a wildcard expression.
 * Returns ElvTrue for match, ElvFalse for mismatch.
 *
 * For Unix this is case-sensitive and supports the *, ?, and [] wildcards.
 */
ELVBOOL dirwildcmp(fname, wild)
	char	*fname;	/* an actual filename */
	char	*wild;	/* a wildcard expression */
{
	int	i;
	ELVBOOL	match, negate;

  TailRecursion:
	switch (*wild)
	{
	  case '\\':
		/* character after \ must match exactly, except \0 and / */
		if (*fname == '/' || !*fname || *fname != wild[1])
		{
			return ElvFalse;
		}
		fname++;
		wild += 2;
		goto TailRecursion;

	  case '?':
		/* match any single character except \0 and / */
		if (*fname == '/' || !*fname)
		{
			return ElvFalse;
		}
		fname++;
		wild++;
		goto TailRecursion;

	  case '[':
		/* if no matching ], then compare [ as literal character;
		 * else next char in fname must be in brackets
		 */

		/* if it starts with '^' we'll want to negate the result */
		match = negate = ElvFalse;
		i = 1;
		if (wild[i] == '^')
		{
			negate = ElvTrue;
			i++;
		}

		/* Compare this fname char to each bracketed char.  This is
		 * a little tricky because 1) the ']' char can be included in
		 * the brackets in unambiguous contexts, 2) we need to support
		 * ranges of characters, and 3) there might not be a closing ']'
		 */
		for (; wild[i] && (i == 1 || wild[i] != ']'); i++)
		{
			if (*fname == wild[i])
			{
				match = ElvTrue;
			}
			if (wild[i + 1] == '-' && wild[i + 2])
			{
				if (*fname > wild[i] && *fname <= wild[i + 2])
					match = ElvTrue;
				i += 2;
			}
		}

		/* If no ']' then this fname character must be '[' (i.e., treat
		 * the '[' like a literal character).  However, if there is a
		 * ']' then use the result of the bracket matching if not in
		 * the brackets (or not out if "negate" is set) then fail.
		 */
		if (wild[i] != ']' ? *fname != '[' : match == negate)
		{
			return ElvFalse;
		}

		/* It matched!  Advance fname and wild, then loop */
		fname++;
		wild += i + 1;
		goto TailRecursion;

	  case '*':
		/* The * should match as much text as possible.  Start by
		 * trying to make it match all of the name, and if that doesn't
		 * work then back off until it does match or no match is
		 * possible.
		 */
		for (i = strlen(fname);
		     i >= 0 && !dirwildcmp(fname + i, wild + 1);
		     i--)
		{
		}
		return (ELVBOOL)(i >= 0);

	  case '\0':
		return ((*fname && !partial) ? ElvFalse : ElvTrue);

	  default:
		if (*fname != *wild)
		{
			return ElvFalse;
		}
		fname++;
		wild++;
		goto TailRecursion;
	}
	/*NOTREACHED*/
}

/* Return the first filename (in a static buffer) that matches wildexpr,
 * or wildexpr itself if none matches.  If wildexpr contains no wildcards,
 * then just return wildexpr without checking for files.
 */
char *dirfirst(wildexpr, ispartial)
	char	*wildexpr;	/* a wildcard expression to search for */
	ELVBOOL	ispartial;	/* is this just the front part of a name? */
{
	char	*found;

	/* If no wildcard characters, just return the expression */
	if (!diriswild(wildexpr) && !ispartial)
	{
		dirfp = (DIR *)0;
		return wildexpr;
	}

	/* If a previous dirfirst()/dirnext() was left unfinished, then
	 * abandon it now.
	 */
	if (dirfp)
	{
		closedir(dirfp);
	}

	/* Open the directory for scanning.  If can't open, then return
	 * the wildexpr unchanged.
	 */
	wilddir = dirdir(wildexpr);
	dirfp = opendir(wilddir);
	if (!dirfp)
	{
		return wildexpr;
	}

	/* Use dirnext to do most of the dirty work */
	wildfile = dirfile(wildexpr);
	partial = ispartial;
	found = dirnext();
	return (found ? found : wildexpr);
}

/* Return the next filename (in a static buffer) that matches the
 * wildexpr of the previous dirfirst(), or NULL if no more files match.
 */
char *dirnext()
{
	struct dirent *dent;

	/* if no directory is open, then fail immediately */
	if (!dirfp)
	{
		return (char *)0;
	}

	/* loop until we find a matching entry, or end of directory.
	 * Note that we're careful about not allowing the ? and * wildcards
	 * match a . at the start of a filename; those files are supposed to
	 * be hidden.
	 */

	while ((dent = readdir(dirfp)) != NULL
	    && ((dent->d_name[0] == '.' && (wildfile[0] != '.' && wildfile[0] != '['))
		|| !dirwildcmp(dent->d_name, wildfile)))
	{
	}

	/* if no entries matched, return NULL */
	if (!dent)
	{
		closedir(dirfp);
		dirfp = NULL;
		partial = ElvFalse;
		return NULL;
	}

	/* combine the found name with the wilddir */
	return dirpath(wilddir, dent->d_name);
}

#ifndef JUST_DIRFIRST

/* Return ElvTrue if wildexpr contains any wildcards; else ElvFalse */
ELVBOOL diriswild(wildexpr)
	char	*wildexpr;	/* either a filename or a wildcard expression */
{
#if 0
	char	*quote = '\0';

	while (*wildexpr)
	{
		if (!quote && strchr("?*[", *wildexpr))
		{
			return ElvTrue;
		}
		else if (quote == '\\' || *wildexpr == quote)
		{
			quote = '\0';
		}
		else if (!quote && strchr("\"'`\\", *wildexpr))
		{
			quote = *wildexpr;
		}
	}
#else
	if (strpbrk(wildexpr, "?*[\\"))
	{
		return ElvTrue;
	}
#endif
	return ElvFalse;
}


/* Check a the type & permissions of a file.  Return one of the
 * following to describe the file's type & permissions:
 *    DIR_INVALID    malformed filename (can't happen with UNIX)
 *    DIR_BADPATH    unable to check file
 *    DIR_NOTFILE    file exists but is neither normal nor a directory
 *    DIR_DIRECTORY  file is a directory
 *    DIR_NEW        file doesn't exist yet
 *    DIR_UNREADABLE file exists but is unreadable
 *    DIR_READONLY   file is readable but not writable
 *    DIR_READWRITE  file is readable and writable.
 */
DIRPERM dirperm(filename)
	char	*filename;	/* name of file to check */
{
	struct stat st;
	int	i;

	/* check for a protocol */
	for (i = 0; elvalpha(filename[i]); i++)
	{
	}
	if (i < 2 || filename[i] != ':')
		i = 0;

	/* Skip past "file:" protocol; assume ftp is read/write, and all others
	 * are readonly
	 */
	if (!strncmp(filename, "file:", 5))
		filename += i + 1;
	else if (!strncmp(filename, "ftp:", 4))
		return DIR_READWRITE;
	else if (i > 0)
		return DIR_READONLY;

	if (stat(filename, &st) < 0)
	{
		if (errno == ENOENT)
			return DIR_NEW;
		else
			return DIR_BADPATH;
	}
	if (S_ISDIR(st.st_mode))
		return DIR_DIRECTORY;
	else if (!S_ISREG(st.st_mode))
		return DIR_NOTFILE;
	else if (access(filename, 4) < 0)
		return DIR_UNREADABLE;
	else if (access(filename, 2) < 0)
		return DIR_READONLY;
	else
		return DIR_READWRITE;
}

/* return the file part of a pathname.  This particular implementation doesn't
 * use an internal buffer; it simply returns a pointer to the filename at the
 * end of the pathname.
 */
char *dirfile(pathname)
	char	*pathname;
{
	char	*slash;

	slash = strrchr(pathname, '/');
	if (slash)
	{
		return slash + 1;
	}
	else
	{
		return pathname;
	}
}

#endif /* !JUST_DIRFIRST */
#endif /* !JUST_DIRPATH */

/* return the directory part of a pathname */
char *dirdir(pathname)
	char	*pathname;
{
	static char dir[NAME_MAX + 1];
	char	*slash;

	strncpy(dir, pathname, sizeof dir);
	slash = strrchr(dir, '/');
	if (slash == dir)
	{
		return "/";
	}
	else if (slash)
	{
		*slash = '\0';
		return dir;
	}
	else
	{
		return ".";
	}
}

#ifndef JUST_DIRFIRST

/* combine a directory name and a filename, yielding a pathname. */
char *dirpath(dir, file)
	char	*dir;	/* directory name */
	char	*file;	/* filename */
{
#ifdef __QNXNTO__
	static char	path1[NAME_MAX + 1];
    static char *path2=0;
    static char *path=path1;
    int filelen=strlen(file);
#else    
	static char	path[NAME_MAX + 1];
#endif
	int		len;

	if (*file == '/' || !strcmp(dir, "."))
	{
#ifdef __QNXNTO__
        if (filelen>NAME_MAX)
            path=(path2=realloc(path2, len+filelen+1));
#endif
		strcpy(path, file);
	}
	else
	{
		len = strlen(dir);
		if (len > 0 && dir[len - 1] == '/')
			len--;
#ifdef __QNXNTO__
        if ((len+filelen)>NAME_MAX)
            path=(path2=realloc(path2, len+filelen+1));
#endif
		sprintf(path, "%.*s/%s", len, dir, file);
	}
	return path;
}

/* Return the timestamp of a file, or the current time if no file is specified.
 * If an invalid file is specified, return "".
 */
char *dirtime(filename)
	char	*filename;	/* filename to check */
{
	static char	str[20];/* "YYYY-MM-DDThh:mm:ss\0" */
	time_t		when;	/* the date/time */
	struct stat	st;	/* holds info from timestamp */
	struct tm	*tp;	/* time, broken down */

	/* Choose a time to return (if any) */
	if (!filename || !*filename)
		time(&when);
	else if (stat(filename, &st) == 0)
		when = (st.st_mtime > st.st_ctime) ? st.st_mtime : st.st_ctime;
	else
		return "";

	/* Convert it to a string */
	tp = localtime(&when);
	sprintf(str, "%04d-%02d-%02dT%02d:%02d:%02d",
		tp->tm_year + 1900, tp->tm_mon + 1, tp->tm_mday,
		tp->tm_hour, tp->tm_min, tp->tm_sec);

	/* return it */
	return str;
}

#ifndef JUST_DIRPATH

/* return the pathname of the current working directory */
char *dircwd()
{
	static char	cwd[NAME_MAX + 1];

	if (getcwd(cwd, sizeof cwd))
	{
		return cwd;
	}
	else
	{
		return ".";
	}
}

/* change the current directory, and return ElvTrue if successful (else ElvFalse) */
ELVBOOL dirchdir(pathname)
	char	*pathname;	/* new directory */
{
	return (ELVBOOL)(chdir(pathname) >= 0);
}

#ifdef FEATURE_MISC
/* This only handles ~user, because the ~/dir is handled elsewhere. */
char *expanduserhome(pathname, dest)
	char *pathname;
	char *dest;
{
	char *p;
	struct passwd *u;

	strcpy(dest, pathname+1);
	if ((p = strchr(dest, '/'))) /* yes, ASSIGNMENT! */
		*p = 0;

	if ((u = getpwnam(dest))) /* yes, ASSIGNMENT! */
	{
		strcpy(dest, u->pw_dir);
		if ((p = strchr(pathname, '/'))) /* yes, ASSIGNMENT! */
			strcat(dest, p);
		strcpy(pathname, dest);
	} else
	{
		strcpy(dest, pathname);
	}
	
	return(dest);
}
#endif /* FEATURE_MISC */
#endif /* !JUST_DIRPATH */
#endif /* !JUST_DIRFIRST */
