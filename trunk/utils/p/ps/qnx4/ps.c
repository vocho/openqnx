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





#include <unistd.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include <pwd.h>
#include <grp.h>

#include <sys/kernel.h>
#include <sys/magic.h>
#include <sys/psinfo.h>
#include <sys/sched.h>
#include <sys/sidinfo.h>
#include <sys/times.h>
#include <sys/vc.h>

#include <stdutil.h>

#ifdef __USAGE
%C - report process status (POSIX)

%C	[-aAdlS] [-F format] [-g pgrps] [-n node] [-p pids] [-s sids]
	[-t ttys] [-u uids]

Options:
-a  All processes associated with terminals except for process group leaders
-A  All accessible processes
-d  All processes except process group leaders
-F  Print in format as specified by format string
-g  Show only matching process groups
-l  Print in long format
-n  Show processes on specified node
-p  Show only matching process numbers
-s  Show only matching session numbers
-S  Show only matching process states
-t  Show only matching terminals
-u  Show only matching users

Format specifiers:
%%%  A literal '%' character
%%u  Real user ID
%%U  Effective user ID
%%g  Real group ID
%%G  Effective group ID
%%p  Process ID
%%P  Parent process ID
%%r  Process group ID
%%C  Ratio of CPU time used to CPU time available
%%z  Size of process in kilobytes
%%n  Process' nice value
%%N  Process' priority
%%t  Elapsed wall clock time
%%T  Relative start time
%%x  Cumulative CPU time
%%y  Controlling terminal
%%c  Command name
%%a  Command with arguments
%%b  Process blocked on
%%d  Current working directory
%%e  Process environment
%%f  Process flags
%%h  Session NID
%%s  Session ID
%%S  Process state
#endif

char *states[] = {
    "DEAD", "READY", "SEND", "RECV", "REPLY", "HELD", "SIGBL", "WAIT", "SEM"
};

pid_t pm = PROC_PID;			/* pid/vid of/to process manager */
struct _psinfo ps;			/* process status */
#define ppid un.proc.father		/* parent pid */
#define pgrp pid_group			/* process group */

#define FMT_ELAPSED \
    "%\"PID\"p %\"PGRP\"r %\"PRI\"N %\"STATE\"S %\"BLK\"b %\"TIME  \"T %\"COMMAND\"a"
#define FMT_JOBS \
    "%\"PPID\"P %\"PID\"p %\"PGRP\"r %\"SID\"s %\"PRI\"N %\"STATE\"S %\"BLK\"b %\"UID\"u %\"TIME  \"t %\"COMMAND\"c"
#define FMT_LONG \
    "%\"PID\"p %\"PGRP\"r %\"PRI\"N %\"STATE\"S %\"BLK\"b %\"TIME  \"x %\"COMMAND\"a"
#define FMT_POSIX \
    "%\"PID\"p %\"TTY\"y %\"TIME  \"t %\"COMMAND\"c"
#define FMT_QNX \
    "%\"PID\"p %\"PGRP\"r %\"SID\"s %\"PRI\"N %\"STATE\"S %\"BLK\"b %\"SIZE\"z %\"COMMAND\"a"

time_t utc;				/* program start time (for elapsed times) */
char *progname;				/* prognam name */
char buffer[512];			/* string/numeric buffer */

void message(char *fmt, ...) {
    va_list args;

    fprintf(stderr, "%s: ", progname);
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fputc('\n', stderr);
}

char *elapsed(clock_t period) {
    unsigned days, hours, minutes, seconds;

    seconds = period % 60; period /= 60;
    minutes = period % 60; period /= 60;
    hours   = period % 24; period /= 24;
    days    = period;

    if (days) sprintf(buffer, "%2u-", days);
    else sprintf(buffer, "   ");
    sprintf(buffer + 3, "%02d:%02d:%02d", hours, minutes, seconds);
    return buffer;
}

char *number(int n) {
    return itoa(n, buffer, 10);
}

char *percent(int n) {
    sprintf(buffer, "%2d.%02d%%", n / 100, n % 100);
    return buffer;
}

char *prio(int p, int s) {
    sprintf(buffer, "%2d%c", p, s == SCHED_FIFO ? 'f' : s == SCHED_RR ? 'r' : 'o');
    return buffer;
}

char *group(uid_t gid) {
    struct group *gr;

    if (gr = getgrgid(gid)) return gr->gr_name;
    return number(gid);
}

char *user(uid_t uid) {
    struct passwd *pw;

    if (pw = getpwuid(uid)) return pw->pw_name;
    return number(uid);
}

int magic(int sptr) {
    char far *fptr;

    return __qnx_debug_xfer(pm, ps.pid, _DEBUG_MEM_RD, &fptr, sizeof fptr,
	    ps.magic_off + offsetof(struct _magic, sptrs[sptr]), ps.magic_sel) == 0
	&& fptr && (ps.flags & _PPF_32BIT ?
	    __qnx_debug_xfer(pm, ps.pid, _DEBUG_MEM_RD, buffer, sizeof buffer,
		(long) fptr, ps.magic_sel) == 0 :
	    __qnx_debug_xfer(pm, ps.pid, _DEBUG_MEM_RD, buffer, sizeof buffer,
		FP_OFF(fptr), FP_SEG(fptr)) == 0);
}

char *procarg(void) {
    if ((ps.flags & _PPF_TERMING) != 0)
	return "<defunct>";
    if (magic(_SPTRS_CMD)) {
	char *argv = ((struct _proc_spawn *) buffer)->data;
	int argc = ((struct _proc_spawn *) buffer)->argc;
	char *bp = buffer;

	argc--, argv += strlen(argv) + 1;
	while (argv < buffer + sizeof buffer - 1)
	    if (*bp = *argv++) bp++;
	    else if (argc-- == 0) break;
	    else *bp++ = ' ';
	*bp = 0;
	return buffer;
    }
    sprintf(buffer, "(%s)", ps.un.proc.name);
    return buffer;
}

char *procdir(void) {
    if (magic(_SPTRS_CWD))
	return buffer;
    return "*";
}

char *procenv(void) {
    if (magic(_SPTRS_CMD)) {
	char *argv = ((struct _proc_spawn *) buffer)->data;
	int argc = ((struct _proc_spawn *) buffer)->argc + 1;
	int envc = ((struct _proc_spawn *) buffer)->envc;
	char *bp = buffer;

	while (argc--) while (*argv++);
	
	while (argv < buffer + sizeof buffer - 1)
	    if (*bp = *argv++) bp++;
	    else if (envc-- == 0) break;
	    else *bp++ = ' ';
	*bp = 0;
	return buffer;
    }
    return "*";
}

char *procflags(void) {
    sprintf(buffer, "%08lx", ps.flags);
    return buffer;
}

char *procgrp(void) {
    return number(ps.pgrp);
}

char *procmem(void) {
    long bytes = 0;

	if (*(long*)ps.zero1) {
		bytes= (*(long*)ps.zero1+*(long*)ps.un.proc.zero2)*4096;
	} else {
    	struct _seginfo si[16];
    	int s, i;
    	for (s = 0; qnx_psinfo(pm, ps.pid, &ps, s, si) == ps.pid; s += 16) {
		if ((i = ps.un.proc.nselectors) == 0) break;
		while (i--)
	    	if (si[i].flags & _PMF_INUSE)
			/* don't add first (code) selector if flat */
			if (i || (ps.flags & _PPF_FLAT) == 0)
		    	bytes += si[i].nbytes;
    	}
    }
    sprintf(buffer, "%dK", bytes / 1024);
    return buffer;
}

char *procname(void) {
    if ((ps.flags & _PPF_TERMING) != 0)
	return "<defunct>";
    else if ((ps.flags & _PPF_VID) == 0)
	return ps.un.proc.name;
    else {
	static struct _psinfo vc;
	pid_t vd;

	if ((vd = qnx_vc_attach(ps.un.vproc.remote_nid, PROC_PID, sizeof vc, 0)) == -1 ||
	    (qnx_psinfo(vd, ps.un.vproc.remote_pid, &vc, 0, 0) != ps.un.vproc.remote_pid)) {
	    qnx_vc_detach(vd);
	    return "*";
	}
	qnx_vc_detach(vd);
	return vc.un.proc.name;
    }
}

char *procpid(void) {
    return number(ps.pid);
}

char *procsid(void) {
    return number(ps.sid);
}

char *procstate(void) {
    return states[ps.state];
}

char *proctty(void) {
    struct _sidinfo si;
    char *p;

    if (qnx_sid_query(pm, ps.sid, &si) != ps.sid)
	return "*";
    if (p = strchr(si.tty_name + 2, '/')) *p = 0;
    else return "*";
    if (p = strrchr(p + 1, '/')) p++;
    else p = "";
    sprintf(buffer, "[%s]%s", si.tty_name + 2, p);
    return buffer;
}

char *procuser(void) {
    return user(ps.ruid);
}

char *format_u()    { return user(ps.ruid); }
char *format_U()    { return user(ps.euid); }
char *format_g()    { return group(ps.rgid); }
char *format_G()    { return group(ps.egid); }
char *format_p()    { return number(ps.pid); }
char *format_P()    { return number(ps.ppid); }
char *format_r()    { return number(ps.pgrp); }
char *format_C()    { return percent(10 * (ps.un.proc.times.tms_utime + ps.un.proc.times.tms_stime) / max(1, utc - ps.un.proc.start_time)); }
char *format_z()    { return procmem(); }
char *format_n()    { return prio(ps.priority, ps.sched_algorithm); }
char *format_N()    { return prio(ps.max_priority, ps.sched_algorithm); }
char *format_t()    { return elapsed((ps.un.proc.times.tms_utime + ps.un.proc.times.tms_stime) / 1000); }
char *format_T()    { return elapsed(utc - ps.un.proc.start_time); }
char *format_x()    { return elapsed((ps.un.proc.times.tms_utime + ps.un.proc.times.tms_stime + ps.un.proc.times.tms_cutime + ps.un.proc.times.tms_cstime) / 1000); }
char *format_y()    { return proctty(); }
char *format_c()    { return procname(); }
char *format_a()    { return procarg(); }
#ifdef __QNX__
char *format_b()    { return ((ps.state != 1) ? number(ps.blocked_on) : ""); }
char *format_d()    { return procdir(); }
char *format_e()    { return procenv(); }
char *format_f()    { return procflags(); }
char *format_h()    { return number(ps.sid_nid); }
char *format_s()    { return number(ps.sid); }
char *format_S()    { return procstate(); }
#endif

char specifiers[] = "uUgGpPrCznNtTxycabdefhsS";
char *titles[sizeof specifiers];
char *widths[sizeof specifiers] = {
  /* u    U    g    G    p    P    r    C    z    n    N  */
    "9", "9", "5", "5", "5", "5", "5", "6", "5", "3", "3",
  /* t    T    x    y    c    a    b    d    e    f    h    s    S  */
    "11","11","11","9",  "",  "", "5", "",  "",  "8", "3", "3", "5"
};

char *(*handlers[sizeof specifiers])() = {
    format_u, format_U, format_g, format_G, format_p, format_P,
    format_r, format_C, format_z, format_n, format_N, format_t,
    format_T, format_x, format_y, format_c, format_a, format_b,
    format_d, format_e, format_f, format_h, format_s, format_S
};

void print_format(char *fmt) {
    static int titled;

    do {
	char buffer[14];
	int ch;

	do {
	    switch (ch = *fmt++) {
	      case 0: break;
	      case '%':
		switch (ch = *fmt++) {
		  case 0: /* fall through */
		  case '%': putchar('%'); break;
		  default:
		    ch = strchr(specifiers, ch) - specifiers;
		    sprintf(buffer, "%%%ss", widths[ch]);
		    if (!titled) printf(buffer, titles[ch] ? titles[ch] : "");
		    else printf(buffer, (*handlers[ch])());
		    ch++;   /* so we don't finish loop */
		}
		break;
	      default:
		putchar(ch);
	    }
	} while (ch);
	putchar('\n');
	titled = 1;
    } while (!titled);
}

char *scan_format(char *fmt, char *detail) {
    while (*fmt) {
	char *title = 0;
	char *width = 0;
	char *x;
	int ch;

	switch (ch = *fmt++) {
	  case '%':
reswitch:   switch (ch = *fmt++) {
	      case  0 : break;
	      case '%': *detail++ = '%'; *detail++ = '%'; break;
	      case '+': case '-':
	      case '0': case '1': case '2': case '3': case '4':
	      case '5': case '6': case '7': case '8': case '9':
	        width = fmt - 1;
		while ('0' <= *fmt && *fmt <= '9') fmt++;
		goto reswitch;
	      case '"':	/* title */
		fmt[-1] = 0;
		title = fmt;
		while (*fmt && *fmt != '"') fmt++;
		if (*fmt == '"')
		    *fmt++ = 0;
		else
		    message("mismatched double quote");
		goto reswitch;
	      default:
	      	fmt[-1] = 0;
		if (x = strchr(specifiers, ch)) {
		    int y = x - specifiers;

		    *detail++ = '%';
		    if (width)
			if (ch == 'a')
			    widths[y] = width;
/*			else
			    message("field width only valid with 'a' specifier");
*/		    if (title) titles[y] = title;	    
		    *detail++ = ch;
		} else
		    message("unsupported format specifier '%c'", ch);
	    }
	    break;
	  default:
	    *detail++ = ch;
	}
    }
    *detail = 0;
    return detail;
}

int match(char *list, char *(fn)(void)) {
    char *key = fn();
    char *next;
    char save;

    do {
	save = *(next = list + strcspn(list, " ,:")), *next = 0;
	if (stricmp(list, key) == 0) {
	    *next = save;
	    return 1;
	}
    } while (list = next, *list++ = save);
    return 0;
}

int main(int argc, char **argv) {
    int show_other = 0;		/* processes outside my session */
    int show_leader = 1;	/* process group leaders */
    int show_interactive = 1;	/* processes associated with terminals */
    int show_daemon = 0;	/* processes with no associated terminal */
    char *match_pid = 0;	/* process id to match */
    char *match_pgrp = 0;	/* group id to match */
    char *match_sid = 0;	/* session id to match */
    char *match_state = 0;	/* process state to match */
    char *match_tty = 0;	/* terminal id to match */
    char *match_uid = 0;	/* user id to match */
    char *format = 0;		/* output format string */
    char detail[132];		/* detail line buffer */
    time_t boot;		/* boot time (kludge for image modules) */
    long nid;
    int euid = geteuid();
    int ch;

    time(&utc);
    progname = basename(argv[0]);
    while ((ch = getopt(argc, argv, "aAdF:g:p:s:t:u:**efjln:S:x")) != EOF) {
	switch (ch) {
	  case 'a': /* all processes associated with terminals except pgrp leaders */
	    show_other = 1;
	    show_leader = 0;
	    show_interactive = 1;
	    show_daemon = 0;
	    break;
	  case 'A': /* all accessible processes */
	  case 'f':
	  case 'x':
	    show_other = 1;
	    show_leader = 1;
	    show_interactive = 1;
	    show_daemon = 1;
	    break;
	  case 'd': /* all processes except pgrp leaders */
	    show_other = 1;
	    show_leader = 0;
	    show_interactive = 1;
	    show_daemon = 1;
	    break;
	  case 'F': /* specify output format */
	    if (format)
		message("more than one format string?");
	    format = optarg;
	    break;
	  case 'g': /* processes whose group ID numbers match */
	    show_other = 1;
	    match_pgrp = optarg;
	    break;
	  case 'p': /* processes whose process ID numbers match */
	    show_other = 1;
	    match_pid = optarg;
	    break;
	  case 's': /* sessions whose session ID numbers match */
	    show_other = 1;
	    match_sid = optarg;
	    break;
	  case 't': /* processes whose terminal names match */
	    show_other = 1;
	    match_tty = optarg;
	    break;
	  case 'u': /* processes whose user names/IDs match */
	    show_other = 1;
	    match_uid = optarg;
	    break;
	  case 'e':
	    format = FMT_ELAPSED;
	    break;
	  case 'l':
	    format = FMT_LONG;
	    break;
	  case 'j':
	    format = FMT_JOBS;
	    break;
	  case 'n':
	    nid = strtonid(optarg, 0);
	    if (nid == -1 || nid == 0) {
		message("invalid node specified (-n %s)\n", optarg);
		return EXIT_FAILURE;
	    }
	    if ((pm = qnx_vc_attach(nid, PROC_PID, sizeof ps, 1)) == -1) {
		message("unable to connect to node %s. (%s)\n", 
			nidtostr(nid, 0, 0), strerror(errno));
		return EXIT_FAILURE;
	    }
	    break;
	  case 'S':
	    match_state = optarg;
	    break;
	}
    }

    setpwent();
    if (format == 0) format = getenv("POSIX_STRICT") ? FMT_POSIX : FMT_QNX;
    scan_format(format, detail);
    print_format(detail);
    for (ps.pid = PROC_PID; qnx_psinfo(pm, ps.pid, &ps, 0, 0) >= 0; ps.pid++) {
	/* skip proxies and virtual circuits */
	if ((ps.flags & _PPF_MID) || (ps.flags & _PPF_VID)) continue;
	/* save start time of Proc */
	if (ps.pid == PROC_PID) boot = ps.un.proc.start_time;
	/* skip other users' processes */
	if (!show_other && ps.euid != euid)		    continue;
	/* skip process group leaders */
	if (!show_leader && ps.pid == ps.pgrp)		    continue;
	/* check for specific processes */
	if (match_state  && !match(match_state, procstate)) continue;
	if (match_pid    && !match(match_pid, procpid))	    continue;
	if (match_pgrp   && !match(match_pgrp, procgrp))    continue;
	if (match_sid    && !match(match_sid, procsid))	    continue;
	if (match_tty    && !match(match_tty, proctty))	    continue;
	if (match_uid    && !match(match_uid, procuser))    continue;
	/* kludge start time for image modules */
	if (ps.un.proc.start_time == 0) ps.un.proc.start_time = boot;
	print_format(detail);
    }
    return EXIT_SUCCESS;
}
