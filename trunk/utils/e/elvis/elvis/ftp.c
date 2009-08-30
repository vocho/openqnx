/* ftp.c */
/* Copyright 1997 by Steve Kirkendall */


#include "elvis.h"
# ifdef PROTOCOL_FTP


/* The default port to use for FTP connections */
#define FTP_PORT	21


/* Some FTP response codes. */
#define FTP_FILE_STATUS		213
#define FTP_PASSIVE_OKAY	227
#define FTP_PASSWORD_REQUIRED	331
#define FTP_ACCOUNT_REQUIRED	332
#define FTP_UNKNOWN_CMD		500
#define FTP_UNIMPLEMENTED_CMD	502
#define FTP_UNIMPLEMENTED_ARG	504
#define FTP_NO_FILE		550


/* These are used to format directory listings, in HTML */
#ifdef FEATURE_CALC
# define HTML_HEAD	"<html><body>\n<h1>Directory listing of \"(htmlsafe($1))\"</h1>\n<table>\n"
# define HTML_ITEM	"<tr><td><a href=\"$1\">(htmlsafe($2))</a></td><td>(htmlsafe($3))</td></tr>\n"
# define HTML_TAIL	"</table></body></html>\n"
#else
# define HTML_HEAD	"<html><body>\n<h1>Directory listing of \"$1\"</h1>\n<table>\n"
# define HTML_ITEM	"<tr><td><a href=\"$1\">$2</a></td><td>$3</td></tr>\n"
# define HTML_TAIL	"</table></body></html>\n"
#endif


#if USE_PROTOTYPES
static void	getaccountinfo(char *site_port, ELVBOOL anonymous, char *wantuser);
static char	*ftpcommand(char *cmd, char *arg, ELVBOOL simple);
static ELVBOOL	passive P_((void));
static DIRPERM	resourcetype(char *resource, ELVBOOL reading);
static ELVBOOL	ftpdir(char *site_port, ELVBOOL anonymous, char *resource);
#endif

static sockbuf_t	*command_sb;	/* command socket */
static sockbuf_t	*data_sb;	/* data socket */
static CHAR		*htmltext;	/* HTML text of directory being read */
static int		htmllen;	/* length of htmltext */
static int		htmlused;	/* amount of htmltext already read */
static char		*homedir;	/* home directory */

/* These point to dynamically-allocated strings which store the user's account
 * information for the current FTP site.
 */
static ELVBOOL	was_anon;
static char	*site;
static char	*user;
static char	*password;
static char	*account;

/* Scan through the ~/.netrc file for the user's account information */
static void getaccountinfo(site_port, anonymous, wantuser)
	char	*site_port;	/* site name, possibly with a port number */
	ELVBOOL	anonymous;	/* ignore machine-specific account info? */
	char	*wantuser;	/* specific login to look for, or NULL */
{
	char	*filename;	/* name of the ~/.netrc file */
	int	c;		/* character from file */
	CHAR	*word;		/* current word from file */
	ELVBOOL foundmachine;	/* have we found the desired machine's entry? */
	CHAR	expect;		/* type of word to expect next */
	char	*localsite;	/* name of site where elvis is running */
	FILE	*fp;

	/* Extract the site name from site_port.  Note that we're borrowing
	 * the "filename" and "localsite" variables.
	 */
	localsite = safedup(site_port);
	filename = strchr(localsite, ':');
	if (filename)
		*filename = '\0';
	
	/* if same site as last time, use same info as last time */
	if (was_anon == anonymous && site && !strcmp(localsite, site)
	 && (!wantuser || !strcmp(wantuser, user)))
	{
		safefree(localsite);
		return;
	}

	/* remember site name, to help us next time */
	was_anon = anonymous;
	if (site) safefree(site);
	site = localsite;

	/* Clobber the old account info */
	if (user) safefree(user);
	if (password) safefree(password);
	if (account) safefree(account);
	user = password = account = NULL;
	foundmachine = ElvFalse;

	/* For anonymous logins, ignore the ~/.netrc file */
	if (anonymous)
		goto NoFile;

	/* Locate the ~/.netrc file.  Look in the home directory first */
#ifdef FEATURE_CALC
	filename = tochar8(calculate(toCHAR("home/\".netrc\""), NULL, CALC_ALL));
#else
	filename = "~/.netrc";
#endif
	assert(filename);
	if (dirperm(filename) < DIR_READONLY)
		filename = iopath(tochar8(o_elvispath), FTP_FILE, ElvFalse);
	if (!filename)
		goto NoFile;

	/* Open the file.  This can't fail, because we already know the file
	 * exists and is readable.
	 */
	fp = fopen(filename, "r");

	/* debugging info.  Note that this must be output *AFTER* the file
	 * has been opened, because this message clobbers the buffer used
	 * for storing the filename.  (They both use calculate().)
	 */
	if (o_verbose >= 6)
		msg(MSG_INFO, "[S]ftp account info file is: $1", filename);

	/* Scan words from the file.  Use mult-character buffer for speed. */
	word = NULL;
	expect = '\0';
	while ((c = getc(fp)) != EOF && !(foundmachine && expect == 'm'))
	{
		if (!elvspace(c))
		{
			buildCHAR(&word, (_CHAR_)c);
			continue;
		}
		else if (!word)
			continue;

		/* handle the word */
		if (!strcmp(tochar8(word), "machine")
		 || !strcmp(tochar8(word), "login")
		 || !strcmp(tochar8(word), "password")
		 || !strcmp(tochar8(word), "account"))
		{
			expect = *word;
		}
		else if (!strcmp(tochar8(word), "default"))
		{
			if (foundmachine)
				expect = 'm'; /* so we exit the loop */
			foundmachine = ElvTrue;
			expect = '\0';
		}
		else if (!strcmp(tochar8(word), "macdef"))
		{
			expect = '\0';
		}
		else if (expect == 'm')
		{
			if (strcmp(tochar8(word), site))
			{
				if (o_verbose >= 7)
					msg(MSG_INFO, "[Ss]ftp ignoring 'machine $1' because we want $2", word, site);
			}
			else
			{
				foundmachine = ElvTrue;
				expect = '\0';
			}
		}
		else if (expect == 'l' && foundmachine)
		{
			/* are we looking for a specific login? */
			if (wantuser && CHARcmp(toCHAR(wantuser), word))
			{
				/* not the one we want */
				foundmachine = ElvFalse;
				if (password) safefree(password);
				if (account) safefree(account);
				password = account = NULL;
				if (o_verbose >= 7)
					msg(MSG_INFO, "[Ss]ftp ignoring 'account $1' because we want $2", word, wantuser);
			}
			else /* any login is okay, or we got the right one */
			{
				/* use this login */
				user = tochar8(word);
				word = NULL;
			}
		}
		else if (expect == 'p' && foundmachine)
		{
			password = tochar8(word);
			word = NULL;
		}
		else if (expect == 'a' && foundmachine)
		{
			account = tochar8(word);
			word = NULL;
		}

		/* We're through with this word */
		if (word)
		{
			safefree(word);
			word = NULL;
		}
	}

	/* close the file */
	fclose(fp);

NoFile:
	/* if any information is unspecified, then use defaults */
	if (!account)
	{
		account = getenv("LOGNAME");
		if (!account)
			account = "elvis";
		account = safedup(account);
	}
	if (!user)
		user = safedup(foundmachine ? account : "anonymous");
	if (!password)
	{
		filename = getenv("LOGNAME");
		localsite = netself();
		password = (char *)safealloc(strlen(user) + strlen(localsite) + 2, sizeof(char));
		sprintf(password, "%s@%s", filename ? filename : user, localsite);
	}
	if (o_verbose >= 2)
		msg(MSG_INFO, "[sss]user=$1, password=$2, account=$3",
			user, password, account);
}


/* This function sends an FTP command with one argument, and then waits for
 * a response.  It returns the last line of the response if successful, or
 * NULL if error (after giving an error message).
 */
static char *ftpcommand(cmd, arg, simple)
	char	*cmd;	/* the command name, such as "RETR" (may be NULL) */
	char	*arg;	/* an argument for the command (may be NULL) */
	ELVBOOL	simple;	/* detect error responses here? */
{
	char	*response;

	/* Send the command */
	if (cmd)
	{
		if (o_verbose >= 2)
			msg(MSG_INFO, "[ss]ftp: $1 $2", cmd, arg ? arg : "(NULL)");
		if (!netputline(command_sb, cmd, arg, NULL))
			return NULL;
	}

	/* Read each line of the response.  Keep only the last one. */
	while ((response = netgetline(command_sb)) != NULL
	    && (!elvdigit(response[0]) || response[3] == '-' || !strncmp(response, "220", 3)))
	{
		if (o_verbose >= 3)
			msg(MSG_INFO, "[s]ftp: $1", response);
	}
	if (response && o_verbose >= 3)
		msg(MSG_INFO, "[s]ftp: $1", response);

	/* Maybe detect error responses */
	if (response && simple && (response[0] == '4' || response[0] == '5'))
	{
		msg(MSG_ERROR, "[s]ftp failed: $1", &response[4]);
		response = NULL;
	}

	return response;
}


/* This function should be called before any command which transfers data.
 * It sends a "PASV" command to learn which port will be used for the transfer,
 * and then opens data_sb as a socket to that port.  Returns ElvTrue if successful,
 * or ElvFalse if error (after giving an error message).
 */
static ELVBOOL passive()
{
	char		psite[20];
	unsigned int	pport;
	char		*response;
	int		i, j;

	/* send the PASV command */
	response = ftpcommand("PASV", NULL, ElvTrue);
	if (!response)
		return ElvFalse;

	/* parse the response, looking for an address and port number */
	while (*response && *response != '(')
		response++;
	response++;

	/* copy the address into psite[], as a numbers-and-dots name */
	for (i = j = 0; j < 4; i++, response++)
	{
		if (!*response)
			return ElvFalse;
		if (*response == ',')
		{
			psite[i] = '.';
			j++;
		}
		else
			psite[i] = *response;
	}
	psite[i - 1] = '\0';

	/* Convert the port from a pair of ascii-encoded bytes
	 * into one unsigned int
	 */
	pport = atoi(response) << 8;
	while (*response != ',')
		if (!*response++)
			return ElvFalse;
	response++;
	pport |= atoi(response);

	/* open a connection to the chosen site/port */
	if (o_verbose >= 2)
		msg(MSG_INFO, "[sd]ftp: passive channel = $1:$2", psite, pport);
	data_sb = netconnect(psite, pport);
	if (!data_sb)
		return ElvFalse;

	return ElvTrue;
}


/* This stores the permissions of the most recently opened FTP file */
DIRPERM ftpperms;

/* This checks the permissions of a file, but incompletely.  It returns one
 * of the following:
 *	DIR_READWRITE	- the resource exists, and is a writable file.
 *	DIR_READONLY	- the resource exists, and is a non-writable file.
 *	DIR_NEW		- the resource doesn't exist.
 *	DIR_DIRECTORY	- the resource is a directory.
 *	DIR_NOTFILE	- the resource exists, and is a directory.
 *	DIR_BADPATH	- error in communications.
 * If the "reading" argument is true, and the type is DIR_READWRITE, then it
 * also fetches the size of the file and informs the url.c functions.
 *
 * Note that this function returns DIR_READWRITE even for read-only files.
 */
static DIRPERM resourcetype(resource, reading)
	char	*resource;	/* the file to check */
	ELVBOOL	reading;
{
	char	*response;
	int	lines;
	long	size;

	/* Try to "cd" into it as though it is a directory.  If that succeeds
	 * then we know it really is a directory.
	 */
	response = ftpcommand("CWD", resource, ElvFalse);
	if (!response)
		return DIR_BADPATH;
	if (*response == '2')
	{
		(void)ftpcommand("CWD", homedir, ElvFalse);
		return DIR_DIRECTORY;
	}
	(void)ftpcommand("CWD", homedir, ElvFalse);

	/* Try to get the file's size. */
	response = ftpcommand("SIZE", resource, ElvFalse);
	if (!response)
		return DIR_BADPATH;

	/* Did it work? */
	switch (atoi(response))
	{
	  case FTP_FILE_STATUS:
		/* we got a size -- it must be an existing file */
		if (reading)
			urlbytes(atol(&response[4]));
		goto ExistingFile;

	  case FTP_NO_FILE:
		/* The file doesn't exist, or is a directory.  Since we already
		 * tested for directories, we'll assume it is a new file.
		 */
		return DIR_NEW;

	  case FTP_UNKNOWN_CMD:
	  case FTP_UNIMPLEMENTED_CMD:
		/* server doesn't do SIZE -- handled below... */
		break;

	  default: /* probably FTP_UNIMPLEMENTED_ARG: */
		/* SIZE isn't supported for this resource -- assume directory */
		return DIR_NOTFILE;
	}

	/* Apparently SIZE isn't supported by this server.  Try LIST */
	if (!passive())
		return DIR_BADPATH;
	response = ftpcommand("LIST", resource, ElvFalse);
	if (!response)
	{
		netdisconnect(data_sb);
		return DIR_BADPATH;
	}
	if (atoi(response) == FTP_NO_FILE)
	{
		netdisconnect(data_sb);
		return DIR_NEW;
	}

	/* Count the response lines */
	for (size = -1L, lines = 0; (response = netgetline(data_sb)) != NULL; )
	{
		/* ignore lines which start with a digit or "total" */
		if (!*response || elvdigit(*response) || !strncmp(response, "total", 5))
			continue;
		lines++;
	}
	netdisconnect(data_sb);
	data_sb = NULL;

	/* if more than one line, then assume directory. else guess READWRITE */
	if (lines > 1)
		return DIR_NOTFILE;

ExistingFile:
	/* At this point we've decided that it exists, and is almost certainly
	 * a file (not nonexistent, a directory, or anything more exotic).
	 * We assume it is readable, but we would also like to know if it is
	 * writable.  To determine that, we'll try to append 0 bytes to it.
	 */
	if (!passive())
		return DIR_BADPATH;
	response = ftpcommand("APPE", resource, ElvFalse);
	if (!response || atoi(response) >= 400)
	{
		netdisconnect(data_sb);
		return DIR_READONLY;
	}
	netdisconnect(data_sb);
	(void)ftpcommand(NULL, NULL, ElvFalse);

	return DIR_READWRITE;
}


/* Read a directory, and construct a single big string to store an HTML version
 * of it.  If successful, set "htmltext" to point to the string, and return
 * ElvTrue; otherwise, give an error message and return ElvFalse.
 */
static ELVBOOL ftpdir(site_port, anonymous, resource)
	char	*site_port;	/* name of site, with optional port number */
	ELVBOOL	anonymous;	/* don't include "~/" in the URL name */
	char	*resource;	/* name of the directory being read */
{
	char	*line;
	char	*response;
	CHAR	*args[4];
	CHAR	*cp;
	CHAR	*new;
	char	urlbuf[300];	/* used for constructing file referece URLs */

	msg(MSG_STATUS, "reading directory");

	/* Calculate the basic header text */
	args[0] = toCHAR(resource);
	args[1] = NULL;
	cp = calculate(toCHAR(HTML_HEAD), args, CALC_MSG);
	if (!cp)
	{
		/* error message generated from calculate() */
		return ElvFalse;
	}
	htmltext = CHARdup(cp);

	/* If not root directory, then add an entry for "parent" */
	if (strcmp(resource, "/") && strcmp(resource, "."))
	{
		/* This is a complex way of stripping off the last item in an
		 * absolute directory name.
		 */
		new = CHARdup(toCHAR(resource));
		cp = new + CHARlen(new) - 1; 
		if (*cp == '/')
			cp--;
		while (cp > new && *cp != '/')
		{
			cp--;
		}
		*cp = '\0';

		/* set up the other args for the "parent directory" item */
		sprintf(urlbuf, "ftp://%s/%s%s", site_port, anonymous ? "" : "~/", new);
		safefree(new);
		args[0] = toCHAR(urlbuf);
		args[1] = toCHAR("..");
		args[2] = toCHAR("Parent directory");
		args[3] = NULL;
		cp = calculate(toCHAR(HTML_ITEM), args, CALC_MSG);
		if (cp)
		{
			new = (CHAR *)safealloc(CHARlen(htmltext) + CHARlen(cp) + 1, sizeof(CHAR));
			CHARcpy(new, htmltext);
			CHARcat(new, cp);
			safefree(htmltext);
			htmltext = new;
		}
	}

	/* Request the directory listing */
	if (!passive() || !ftpcommand("LIST", resource, ElvTrue))
	{
		return ElvFalse;
	}

	/* For each directory entry... */
	while ((line = netgetline(data_sb)) != NULL)
	{
		/* ignore it if it starts with "total " or contains no spaces */
		if (!strncmp(line, "total ", 6)
		 || (response = strrchr(line, ' ')) == NULL
		 || response[1] <= ' ')
			continue;

		/* Assume the last word is file name, others are other info */
#if 0
		/* unless the four chars before the last word are " -> ",
		 * in which case we want to look back a couple more words.
		 */
		if (response > line + 5 && !strncmp(response - 3, " ->", 3))
		{
			response[-3] = '\0';
			response = strrchr(line, ' ');
			if (!response || response[1] <= ' ')
				continue;
		}
#endif
		*response++ = '\0';

		/* construct the args for a simpler-syntax expression */
		sprintf(urlbuf, "ftp://%s/", site_port);
		if (!anonymous)
			sprintf(urlbuf + strlen(urlbuf), "~%s/", user);
		if (strcmp(resource, "/") && strcmp(resource, "."))
			sprintf(urlbuf + strlen(urlbuf), "%s/", resource);
		strcat(urlbuf, response);
		args[0] = toCHAR(urlbuf);
		args[1] = toCHAR(response);
		args[2] = toCHAR(line);
		args[3] = NULL;

		/* Add the item to the string */
		cp = calculate(toCHAR(HTML_ITEM), args, CALC_MSG);
		if (!cp)
			cp = toCHAR(line + strlen(line) + 1);
		new = (CHAR *)safealloc(CHARlen(htmltext) + CHARlen(cp) + 1, sizeof(CHAR));
		CHARcpy(new, htmltext);
		CHARcat(new, cp);
		safefree(htmltext);
		htmltext = new;
	}
	netdisconnect(data_sb);

	/* Add the HTML tail to the string */
	new = (CHAR *)safealloc(CHARlen(htmltext) + strlen(HTML_TAIL) + 1, sizeof(CHAR));
	CHARcpy(new, htmltext);
	CHARcat(new, toCHAR(HTML_TAIL));
	safefree(htmltext);
	htmltext = new;

	return ElvTrue;
}

/* Open a connection to an FTP site, to read a file or directory, or write
 * or append to a file.  In addition, you can call it with rwap='p' to check
 * the file's permissions but not do anything else.
 */
ELVBOOL ftpopen(site_port, resource, force, rwap)
	char	*site_port;	/* site name & port number of the FTP server */
	char	*resource;	/* name of the file or directory */
	ELVBOOL	force;		/* allow existing files to be overwritten */
	_char_	rwap;		/* 'r'-read, 'w'-write, 'a'-append */
{
	ELVBOOL anonymous = ElvTrue;
	char	*response, *build;
	char	*wantuser;
	int	i;

	/* If the resource has a leading slash, then strip it off.  URLs are
	 * always relative to root anyway, and it gets in the way of later
	 * processing.
	 */
	if (resource[0] == '/' && resource[1])
		resource++;

	/* If the resource now starts with "~" then this will *NOT* be
	 * an anonymous login.
	 */
	wantuser = NULL;
	if (resource[0] == '~')
	{
		anonymous = ElvFalse;
		resource++;
		if (*resource == '/')
			resource++;
		else if (*resource)
		{
			for (i = 1; resource[i] != 0 && resource[i] != '/'; i++)
			{
			}
			wantuser = (char *)safealloc(i + 1, sizeof(char));
			strncpy(wantuser, resource, i);
			wantuser[i] = '\0';
			resource += i;
			if (*resource == '/')
				resource++;
		}
	}

	/* If we just deleted *ALL* chars of the resource name, then use "." */
	if (!*resource)
		resource = ".";

	/* search for authorization info in ~/.netrc */
	getaccountinfo(site_port, anonymous, wantuser);
	if (wantuser) safefree(wantuser);

	/* Open a connection to the server */
	command_sb = netconnect(site_port, FTP_PORT);
	if (!command_sb)
	{
		/* error message already given */
		return ElvFalse;
	}

	/* Login, and always use binary transfers */
	response = ftpcommand("USER", user, ElvTrue);
	if (response && atoi(response) == FTP_PASSWORD_REQUIRED)
		response = ftpcommand("PASS", password, ElvTrue);
	if (response && atoi(response) == FTP_ACCOUNT_REQUIRED)
		response = ftpcommand("ACCT", account, ElvTrue);
	if (response)
		response = ftpcommand("TYPE", "I", ElvTrue);
	if (response)
		response = ftpcommand("PWD", NULL, ElvTrue);
	if (!response)
	{
		netdisconnect(command_sb);
		return ElvFalse;
	}
	if (response[4] == '"')
	{
		if (homedir)
			safefree(homedir);
		build = homedir = (char *)safealloc(strlen(response), sizeof(char));
		for (response += 5;
		     *response && (*response != '"' || response[1] == '"');     response++)
		{
			if (*response == '"')
				response++;
			*build++ = *response;
		}
		*build = '\0';
	}
	else
		homedir = safedup("/");

	/* do some type-dependent checks */
	ftpperms =  resourcetype(resource, (ELVBOOL)(rwap == 'r'));
	switch (ftpperms)
	{
	  case DIR_READWRITE:
		/* nothing special needed */
		break;

	  case DIR_READONLY:
		if (rwap == 'w' && !force)
		{
			msg(MSG_ERROR, "won't overwrite ftp file without '!'");
			netdisconnect(command_sb);
			return ElvFalse;
		}
		break;

	  case DIR_NEW:
		/* nothing special needed */
		break;

	  case DIR_DIRECTORY:
	  	/* can't write to a non-file */
	  	if (rwap != 'r' && rwap != 'p')
	  	{
	  		msg(MSG_ERROR, "can only READ ftp directories");
	  		netdisconnect(command_sb);
	  		return ElvFalse;
	  	}

		/* read the directory, as one big string of HTML */
		if (!ftpdir(site_port, anonymous, resource))
		{
			netdisconnect(command_sb);
			return ElvFalse;
		}
		htmllen = CHARlen(htmltext);
		htmlused = 0;
		return ElvTrue;

	  default: /* probably DIR_BADPATH */
	  	/* error message already given */
		netdisconnect(command_sb);
	  	return ElvFalse;
	}

	/* initiate the data transfer */
	if (!passive())
	{
		netdisconnect(command_sb);
		return ElvFalse;
	}
	switch (rwap)
	{
	  case 'r':
		response = ftpcommand("RETR", resource, ElvTrue);
		break;

	  case 'w':
		response = ftpcommand("STOR", resource, ElvTrue);
		break;

	  case 'a':
		response = ftpcommand("APPE", resource, ElvTrue);
		break;

	  case 'p':
		/* We just wanted to check the permissions of the file, and
		 * now we've done that.  We don't need to send any more
		 * commands to the FTP server.
		 */
		response = NULL;
		break;
	}
	if (!response)
	{
		/* error message already given */
		netdisconnect(command_sb);
		return ElvFalse;
	}

	/* the hard part is now over */
	return ElvTrue;
}


/* Write to an FTP file */
int ftpwrite(buf, nbytes)
	CHAR	*buf;
	int	nbytes;
{
	/* send the data through the data socket */
	return netwrite(data_sb, tochar8(buf), nbytes) ? nbytes : -1;
}


/* Read from an FTP file or an FTP directory. */
int ftpread(buf, nbytes)
	CHAR	*buf;
	int	nbytes;
{
	/* if directory, then takes bytes from htmltext */
	if (htmltext)
	{
		if (nbytes > htmllen - htmlused)
			nbytes = htmllen - htmlused;
		if (nbytes > 0)
			memcpy(buf, &htmltext[htmlused], nbytes);
		htmlused += nbytes;
		return nbytes;
	}

	/* Else normal read -- get some bytes in data_sb's input buffer */
	if (netbytes(data_sb) < (int)sizeof(data_sb->buf)
	 && netbytes(data_sb) < nbytes)
	{
		if (!netread(data_sb))
			return -1;
	}

	/* copy bytes from data_sb's input buffer into buf */
	if (nbytes > netbytes(data_sb))
		nbytes = netbytes(data_sb);
	if (nbytes > 0)
		memcpy((char *)buf, netbuffer(data_sb), nbytes);
	netconsume(data_sb, nbytes);
	return nbytes;
}


void ftpclose()
{
	if (command_sb)
		netdisconnect(command_sb);
	command_sb = NULL;
	if (htmltext)
		safefree(htmltext);
	htmltext = NULL;
}

#endif /* PROTOCOL_FTP */
