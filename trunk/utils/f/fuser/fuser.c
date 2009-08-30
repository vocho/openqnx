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
%C - list processes with open files (POSIX)

%C [-fcsu] file ...

Options:
  -c    Process as filesystem mountpoint (affects root directory)
  -f    Operate only on the named files (affects block-special)
  -s    Silent operation (return indication of open files in exit status)
  -u    Display the user name of each process
#endif

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <share.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/iomsg.h>
#include <sys/netmgr.h>
#include <sys/neutrino.h>
#include <sys/pathmsg.h>
#include <sys/procmsg.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAXUNION	16

typedef struct {
	int			count;
	struct {
		dev_t		devno;
		ino_t		ino;
	}			entry[MAXUNION];
} match_t;

enum { MODE_NORMAL, MODE_AS_FILE, MODE_AS_FSYS };
enum { USED_OPEN = 0x01, USED_CWD = 0x02, USED_ROOT = 0x04, USED_CTTY = 0x08 };

/*
 *  A dummy replacement for fprintf(), which produces no output, and is
 *  used to cleanly implement the '-s' option.
 */
int nofprintf(FILE *f, const char *fmt, ...)
{
	return(0);
}

/*
 *  Attempt to handle the QNXism of the pathmgr and union filesystems.
 *  Build a list of dev,ino pairs which correspond to the given mountpoint;
 *  this list is later used to detect matches on open files.
 */
int buildmatch(const char *filename, int mode, match_t *match)
{
struct stat	st;
int			*ptrfds, fds[MAXUNION], numfds, i;

	match->count = 0;
	if (_connect_fd(0, filename, 0, O_ACCMODE | O_NONBLOCK | O_LARGEFILE, SH_DENYNO, _IO_CONNECT_OPEN, !0, 0, _FTYPE_ANY, _IO_CONNECT_EXTRA_NONE, 0, NULL, 0, NULL, NULL, (numfds = MAXUNION, &numfds), (ptrfds = fds, &ptrfds)) == -1) {
		return(errno);
	}
	for (i = 0; i < numfds; ++i) {
		if (fstat(fds[i], &st) != -1) {
			if (S_ISBLK(st.st_mode) && mode != MODE_AS_FILE)
				st.st_dev = st.st_rdev, st.st_ino = 0;
			else if (S_ISDIR(st.st_mode) && st.st_rdev != 0 && mode == MODE_AS_FSYS)
				st.st_ino = 0;
			match->entry[match->count].devno = st.st_dev;
			match->entry[match->count].ino = st.st_ino;
			++match->count;
		}
		close(fds[i]);
	}
	return((match->count != 0) ? EOK : ENOENT);
}

/*
 *  Determine if the given dev,ino is prsent in the match list (see above).
 *  An ino of 0 means any file on the specified filesystem.
 */
int ismatch(match_t *match, dev_t devno, ino_t ino)
{
int		i;

	for (i = 0; i < match->count; ++i) {
		if (match->entry[i].devno == devno && (!match->entry[i].ino || match->entry[i].ino == ino))
			return(!0);
	}
	return(0);
}

/*
 *  Implement the '-u' option (display user associated with the process).
 *  First directly query procnto (however it restricts this to processes
 *  in the same session); then deduce it from profcs (address space owner).
 */
uid_t whois(pid_t pid)
{
proc_getsetid_t	msg;
struct stat		st;
char			as[32];

	msg.i.type = _PROC_GETSETID, msg.i.subtype = _PROC_ID_GETID;
	msg.i.pid = pid;
    if (MsgSend(PROCMGR_COID, &msg.i, sizeof(msg.i), &msg.o, sizeof(msg.o)) != -1)
		return(msg.o.cred.ruid);
	sprintf(as, "/proc/%d/as", pid);
	if (stat(as, &st) != -1)
		return(st.st_uid);
	return((uid_t)-1);
}

/*
 *  Given a process id, iterate through all open files of that process
 *  looking for a match to the target file, and then determine how it
 *  is being used by that process.  This is quite different in QNX to
 *  UNIX, as chroot and chdir do not hold open a reference to the file.
 *  The iteration process attempts to dup each file descriptor into
 *  this process, so it can then be queried; attempt to skip things which
 *  aren't files (heuristic guess) and use a timeout as self-defence.
 */
int howused(pid_t pid, match_t *match)
{
struct _server_info	sinfo;
struct _fdinfo		finfo;
struct stat			st;
io_dup_t			msg;
uint64_t			to;
int					fd, coid, usage;

	usage = 0;
	for (fd = 0; (fd = ConnectServerInfo(pid, fd, &sinfo)) != -1; ++fd) {
		if ((pid != sinfo.pid || !(fd & _NTO_SIDE_CHANNEL)) && (coid = ConnectAttach(sinfo.nd, sinfo.pid, sinfo.chid, 0, _NTO_COF_CLOEXEC)) != -1) {
			msg.i.type = _IO_DUP;
			msg.i.combine_len = sizeof(msg.i);
			msg.i.info.nd = netmgr_remote_nd(sinfo.nd, ND_LOCAL_NODE);
			msg.i.info.pid = pid;
			msg.i.info.chid = sinfo.chid;
			msg.i.info.scoid = sinfo.scoid;
			msg.i.info.coid = fd;
			TimerTimeout(CLOCK_MONOTONIC, _NTO_TIMEOUT_SEND | _NTO_TIMEOUT_REPLY, NULL, (to = 1000000000LL, &to), NULL);
			if (MsgSend(coid, &msg.i, sizeof(msg.i), NULL, 0) == -1) {
				ConnectDetach(coid);
			}
			else {
				if (fstat(coid, &st) != -1 && ismatch(match, st.st_dev, st.st_ino)) {
					if (iofdinfo(coid, 0, &finfo, NULL, 0) == -1 || sinfo.pid != PATHMGR_PID || finfo.mode != 0)
						usage |= USED_OPEN;
				}
				close(coid);
			}
		}
	}
	return(usage);
}

/*
 *  Given a filename, walk the process tree querying all open files in the
 *  system, and display an indication of the file, which processes have it
 *  open, what they're doing with it, and who the user is (the typical use
 *  of this utility is prior to unmounting a filesystem).
 */
int showopen(const char *filename, int mode, int username, int (*print)(FILE *, const char *, ...))
{
DIR					*dir;
struct dirent		*dp;
const struct passwd	*pw;
match_t				match;
pid_t				pid, usage;
uid_t				uid;
int					busy, error;

	busy = -1;
	(*print)(stderr, "%s: ", filename);
	if ((error = buildmatch(filename, mode, &match)) != EOK) {
		(*print)(stderr, "%s", strerror(error));
	}
	else if ((dir = opendir("/proc")) != NULL) {
		busy = 0;
		while ((dp = readdir(dir)) != NULL) {
			if ((pid = atoi(dp->d_name)) != 0) {
				if ((usage = howused(pid, &match)) != 0) {
					(*print)(stdout, "%8d", pid);
					if (usage & USED_CWD)
						(*print)(stderr, "c");
					if (usage & USED_OPEN)
						(*print)(stderr, "o");
					if (usage & USED_ROOT)
						(*print)(stderr, "r");
					if (usage & USED_CTTY)
						(*print)(stderr, "y");
					if (username && (uid = whois(pid)) != (uid_t)-1) {
						if ((pw = getpwuid(uid)) != NULL)
							(*print)(stderr, "(%s)", pw->pw_name);
						else
							(*print)(stderr, "(%d)", uid);
					}
					++busy;
				}
			}
		}
		closedir(dir);
	}
	(*print)(stderr, "\n");
	return(busy);
}

/*
 *  Determine what process have the given pathnames open; the showopen()
 *  routine will output diagnostic messages to stdout/stderr.  Note that
 *  options and operands may be intermingled (usefule for '-c'/'-f').
 */
int main(int argc, char *argv[])
{
int		opt, mflag, uflag, silent, n, status, busy;

	status = EXIT_SUCCESS, busy = 0;
	mflag = MODE_NORMAL, uflag = 0, silent = 0;
	setbuf(stdout, NULL), setbuf(stderr, NULL);
	while ((opt = getopt(argc, argv, ":cfsu")) != -1 || argc > optind) {
		switch (opt) {
		case 'c':
			mflag = MODE_AS_FSYS;
			break;
		case 'f':
			mflag = MODE_AS_FILE;
			break;
		case 's':
			silent = !0;
			break;
		case 'u':
			uflag = !0;
			break;
		case -1:
			if ((n = showopen(argv[optind++], mflag, uflag, silent ? nofprintf : fprintf)) == -1)
				status = EXIT_FAILURE;
			else
				busy += n;
			break;
		case ':':
			fprintf(stderr, "missing argument for '-%c'\n", optopt);
			exit(EXIT_FAILURE);
		case '?':
			fprintf(stderr, "unknown option '-%c'\n", optopt);
			exit(EXIT_FAILURE);
		}
	}
	return((!silent || status == EXIT_FAILURE) ? status : (busy != 0) ? EXIT_FAILURE : EXIT_SUCCESS);
}
