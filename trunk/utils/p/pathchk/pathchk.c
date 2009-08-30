/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */





#ifdef __USAGE
%C - check pathnames (POSIX)

%C [-p] pathname ...

Options:
  -p    Perform more extensive portability checks
#endif

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <sys/pathmsg.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAXUNION	16

/*
 *  Cover for pathconf() which handles the cases of there being no
 *  limit (use INT_MAX) or the underlying filesystem not supporting
 *  the query (use a default value).
 */
int getlimit(const char *path, int category, int dflt)
{
long	value;

	errno = EOK;
	if ((value = pathconf(path, category)) != -1)
		return(value);
	return((errno != EOK) ? dflt : INT_MAX);
}

/*
 *  Attempt to skip (when in non '-p' mode) any initial component which
 *  corresponds to an attached pathname (since the resolution process
 *  will strip this off before presenting the remainder of the path to
 *  the underlying filesystem).  The message is directed to PATHMGR_CHID,
 *  and the response is a list of matching mountpoints (longest first).
 */
int skipmount(int pathmgr, char *filename)
{
struct _io_connect				msg;
struct _io_connect_link_reply	reply;
struct _io_connect_entry		entry[MAXUNION];
iov_t							siov[2], riov[2];
int								status;

	memset(&msg, 0, sizeof(msg)), memset(&reply, 0, sizeof(reply));
	msg.type = _IO_CONNECT, msg.subtype = _IO_CONNECT_COMBINE_CLOSE;
	msg.file_type = _FTYPE_ANY;
	msg.reply_max = sizeof(reply) + sizeof(entry), msg.entry_max = MAXUNION;
	msg.path_len = strlen(filename) + 1;
	msg.extra_type = _IO_CONNECT_EXTRA_NONE, msg.extra_len = 0;
	SETIOV(&siov[0], &msg, offsetof(struct _io_connect, path));
	SETIOV(&siov[1], filename, msg.path_len);
	SETIOV(&riov[0], &reply, sizeof(reply));
	SETIOV(&riov[1], entry, sizeof(entry));
	if ((status = MsgSendv(pathmgr, siov, 2, riov, 2)) == -1 || (status & (_IO_CONNECT_RET_FLAG | _IO_CONNECT_RET_TYPE_MASK)) != (_IO_CONNECT_RET_FLAG | _IO_CONNECT_RET_LINK))
		return(0);
	if (!reply.nentries || entry[0].file_type != _FTYPE_ANY)
		return(0);
	return(entry[0].prefix_len += filename[entry[0].prefix_len] == '/');
}

/*
 *  Routine to check characters in a pathname component are valid for
 *  the underlying filesystem.  As there is no mechanism to query this,
 *  only perform these check in '-p' mode for the Portable Character
 *  Set (see POSIX Base Definitions 6.1).
 */
int badchars(char *locale, char *cp, int len)
{
static const char	pcs[(UCHAR_MAX + 1) / CHAR_BIT] = {
						0x81, 0x3F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
						0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x7F,
						0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
					};

	if (locale == NULL) {
		while (--len >= 0) {
			if (!(pcs[*cp / CHAR_BIT] & 1 << *cp % CHAR_BIT))
				return(-1);
			++cp;
		}
	}
	return(0);
}

/*
 *  Output the error message and return failure indication.
 */
int error(char *filename, char *message)
{
	fprintf(stderr, "%s: %s\n", filename, message);
	return(-1);
}

/*
 *  Process a filename, checking component at a time.  A directory
 *  need not exist, but if it does it must be searchable.  The last
 *  existing directory is taken to be the filesystem host of the
 *  remainder of the pathname (in terms of NAME_MAX limits).
 */
int checkpath(char *filename, int pathmgr, int portability)
{
struct stat	st;
char		*component, *next, *chk, base[3], save, cwd[PATH_MAX + 1];
int			len, plen, slen, creating, nmax, pmax, notrunc;

	creating = 0, plen = strlen(filename);
	nmax = _POSIX_NAME_MAX, pmax = _POSIX_PATH_MAX, notrunc = !0;
	if (*filename != '/') {
		len = strlen(component = strcpy(base, "./")), next = filename;
		plen += strlen(getcwd(cwd, PATH_MAX + 1)) + 1;
		slen = skipmount(pathmgr, cwd);
	}
	else if (pathmgr == -1 || !(len = skipmount(pathmgr, filename))) {
		len = strlen(component = strcpy(base, "/")), next = filename;
		slen = 0;
	}
	else {
		component = filename, next = &filename[len];
		slen = len;
	}
	do {
		save = component[len], component[len] = '\0';
		chk = (next == filename) ? component : filename;
		if (!creating) {
			if (stat(chk, &st) == -1) {
				if (errno == ENOTDIR)
					return(error(filename, "component is not a directory"));
				else if (errno == ENOENT)
					creating = !0;
				else
					return(error(filename, strerror(errno)));
			}
			else if (S_ISDIR(st.st_mode) && access(chk, X_OK) == -1) {
				return(error(filename, (errno == EACCES) ? "directory is not searchable" : strerror(errno)));
			}
			else {
				nmax = portability ? _POSIX_NAME_MAX : getlimit(chk, _PC_NAME_MAX, _POSIX_NAME_MAX);
				pmax = portability ? _POSIX_PATH_MAX : getlimit(chk, _PC_PATH_MAX, _POSIX_PATH_MAX);
				notrunc = getlimit(chk, _POSIX_NO_TRUNC, !0);
			}
		}
		component[len] = save;
		while (*next == '/')
			++next;
		component = next;
		for (len = 0; *next != '\0' && *next != '/'; ++len)
			++next;
		if (len > nmax)
			return(error(filename, (notrunc) ? "component name is too long" : "component name is too long (but would be truncated)"));
		else if (badchars(portability ? NULL : "", component, len))
			return(error(filename, "contains non-portable characters"));
		len += *next == '/';
	} while (*component != '\0');
	if (plen > _POSIX_PATH_MAX || plen - slen > pmax)
		return(error(filename, "pathname is too long"));
	return(0);
}

/*
 *  Perform filename checks on each given pathname; the checkpath()
 *  routine will output a diagnostic message to stderr; overall this
 *  returns EXIT_SUCCESS if all pathnames are OK, EXIT_FAILURE otherwise.
 */
int main(int argc, char *argv[])
{
int		opt, portability, status, coid;

	status = EXIT_SUCCESS, portability = 0;
	while ((opt = getopt(argc, argv, ":p")) != -1) {
		switch (opt) {
		case 'p':
			portability = !0;
			break;
		case ':':
			fprintf(stderr, "missing argument for '-%c'\n", optopt);
			exit(EXIT_FAILURE);
		case '?':
			fprintf(stderr, "unknown option '-%c'\n", optopt);
			exit(EXIT_FAILURE);
		}
	}
	coid = portability ? -1 : ConnectAttach(ND_LOCAL_NODE, PATHMGR_PID, PATHMGR_CHID, 0, _NTO_COF_CLOEXEC);
	while (optind < argc) {
		if (checkpath(argv[optind++], coid, portability))
			status = EXIT_FAILURE;
	}
	ConnectDetach(coid);
	return(status);
}
