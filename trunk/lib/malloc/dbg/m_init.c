/*
 * $QNXtpLicenseC:
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





/*
 * (c) Copyright 1990, 1991 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "malloc-lib.h"
#include "mallocint.h"
#include "debug.h"
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <pthread.h>
#include <sys/syspage.h>
#include <signal.h>
#include <sys/siginfo.h>

int malloc_inited=0;
int _malloc_initialized;
ulong_t _malloc_start=0;
ulong_t _malloc_end;
int __pagesize=4096;
pthread_t __ctid=-1;
struct qtime_entry *__malloc_qtp;

static pthread_once_t once = PTHREAD_ONCE_INIT;
pthread_key_t _mallocg_in_malloc;
pthread_key_t _mallocg_in_string;

void *__dbg_malloc_thread(void *arg);
pthread_t __dbgm_StartThread(void *(*func)(void *), void *arg);

int malloc_verbose;
int malloc_handle_signals=1;


extern char *__progname;

static void
destroy_key(void *data)
{
}

static void
create_keys()
{
    pthread_key_create(&_mallocg_in_malloc, destroy_key);
    pthread_key_create(&_mallocg_in_string, destroy_key);
}

static char *
findLibrary(const char *libname, char *buf)
{
    char *lps;
    char path[PATH_MAX];
    char *cp = getenv("LD_LIBRARY_PATH");

    buf[0] = '\0';

    lps = cp;
    while (lps && *lps) {
        struct stat sbuf;
        char *pp;
        const char *lp;

        /* copy library path element to path buffer */
        for (pp = &path[0]; *lps != '\0'; lps++) {
            if (*lps == ':') {
                lps++;
                break;
            }
            if (*lps == '/' && *(lps+1) == '/') continue;
            *pp++ = *lps;
        }
        *pp = '\0';

        if (*path == '\0') continue;

        *pp++ = '/';
        for (lp = libname; *lp; *pp++ = *lp++) ;
        *pp++ = '\0';

        if (stat(path, &sbuf) == 0 ) {
            if (S_ISREG(sbuf.st_mode)) {
		char *bp = buf;
        	for (pp = &path[0]; pp && *pp; bp++, pp++) *bp = *pp;
		*bp = '\0';
        	return buf;
            }
        } else {
	    fprintf(stderr, "error from stat(%s): %s\n", path, (char *)strerror(errno));
        }
    }
    return NULL;
}

static char *
getpathname(const char *filename, char *buf, int len)
{
	if (filename) {
		if (*filename == '/') {
			const char *sp = filename;
			char *dp = buf;
			while (*sp)
				*dp++ = *sp++;
			*dp = '\0';
		} else {
			const char *cp = filename;
			char *cwd = getcwd(buf, len);
			char *dp;
			const char *sp;
			if (cwd == NULL) {
				return NULL;
			}
			dp = buf;
			while (*dp)
				dp++;
			*dp = '\0';
			sp = cp;
			if (*sp == '.' && *(sp+1) == '/') {
				sp++;
			}
			while (*sp)
				*dp++ = *sp++;
			*dp = '\0';
		}
	} else {
		return NULL;
	}
	return buf;
}

static int
match_progname(const char *cptr)
{
	const char *pp = NULL, *cp = NULL;
	char pathname[PATH_MAX+1];
	char *progname = getpathname(_argv[0], pathname, PATH_MAX+1);

	if (progname != NULL && *progname == '/') {
		for (pp = progname, cp = cptr; pp && cp; pp++, cp++) {
			if (*pp != *cp)
				break;
			if (*pp == '\0' || *cp == '\0')
				break;
		}
	}
	return progname && cptr && *pp == '\0' && *cp == '\0';
}

static void
dump_leaks()
{
	char *cptr;
	if ((cptr = getenv("MALLOC_DUMP_LEAKS")) != NULL) {
		int fd = STDERR_FILENO;
		if (*cptr != '\0') {
			int nfd = open(cptr, O_WRONLY|O_CREAT, 0664);
			if (nfd >= 0) {
				fd = nfd;
			}
		}
		malloc_dump_unreferenced(fd, 0);
	}
}

static int compbins (const void *bin1, const void *bin2) {
	struct _stat_bin *b1 = (struct _stat_bin *)bin1;
	struct _stat_bin *b2 = (struct _stat_bin *)bin2;

	if (b1->size < b2->size) {
		return -1;
	}
	if (b1->size > b2->size) {
		return 1;
	}
	return 0;

}

static void
process_bins(int verbose, char *option) {
	char *s;
	int i;

	/* initialize the structure to zero.  */
	//memset(__stat_bins, 0, __n_stat_bins * sizeof(*__stat_bins));
	for (i = 0; i < __n_stat_bins; ++i) {
		__stat_bins[i].size = 0;
		__stat_bins[i].nallocs = 0;
		__stat_bins[i].nfrees = 0;
	}

	for (i = 0, s = option; *s && i < __n_stat_bins; ++s, ++i) {
		unsigned long size = strtoul(s, &s, 0);
		if (size == 0) {
			/* It is a !digit separated list. */
			/* just do nothing, ignoring the zero value  */
		} else {
			/* We do not test for error (ERANGE )and blindly
			   assign the ULONG_MAX */
			__stat_bins[i].size = size;
		}
	}

	qsort(__stat_bins, __n_stat_bins, sizeof(struct _stat_bin), compbins);

	if (verbose) {
		char buf[256];
		int i;
		int len;
		for (i = 0; i < __n_stat_bins; ++i) {
			len = sprintf(buf, "size:%lu\n", __stat_bins[i].size);
			write(2, buf, len);
		}
	}
}

int
__check_ctid()
{
	if ((__ctid >=0 ) && (pthread_self() == __ctid)) {
		return 1;
	}
	return 0;
}

void malloc_signal_catcher(int signo, siginfo_t* info, void* other) {

	const char * signame;
	void * ip = info->si_fltip;
	void * addr = info->si_addr;
	extern ulong_t bt_stack_from;
	//{char buf[256];
	//sprintf(buf,"malloc_g: caught %d in handler %x %x\n", signo, ip, addr);
	//write(2,buf,strlen(buf));}
	bt_stack_from = (ulong_t)other;
	malloc_errno = M_CODE_SIGNAL;
	signame = sys_siglist[signo];
	malloc_fatal(signame,NULL, (int) ip,addr);
	bt_stack_from = 0;
	abort();
}
static int install_handler_if_not_installed(int signo,struct sigaction *act ){
	struct sigaction old;
	sigaction(signo,  NULL, &old);
	if (old.sa_sigaction==SIG_DFL){ // override default handler only
		  sigaction(signo, act, NULL);
		  return 1;
	}
	return 0;
}
static int install_handler(){
	if (malloc_handle_signals) {
		struct sigaction act;
		act.sa_sigaction = malloc_signal_catcher;
		sigemptyset(&act.sa_mask);
		act.sa_flags = SA_SIGINFO;
		install_handler_if_not_installed(SIGSEGV, &act);
		install_handler_if_not_installed(SIGBUS, &act);
		install_handler_if_not_installed(SIGILL, &act);
		install_handler_if_not_installed(SIGFPE, &act);
	}
	return 0;
}

/*
 * Function:	malloc_init()
 *
 * Purpose:	to initialize the pointers and variables use by the
 *		malloc() debugging library
 *
 * Arguments:	none
 *
 * Returns:	nothing of any value
 *
 * Narrative:	Just initialize all the needed variables.  Use mallopt
 *		to set options taken from the environment.
 *
 */
void
malloc_init()
{
	char	* cptr;
	char	* getenv();
	int	  val;
	//char *p;
	char buf[256];
	int tmbd=0;

	/*
 	 * If already initialized...
	 */
	if( malloc_inited != 0) { /* because the implementation may set malloc_data_start */
		return;
	}

	malloc_inited = 1;
	tmbd = __malloc_bt_depth;
	__malloc_bt_depth = 0;
	__pagesize = (int)sysconf(_SC_PAGESIZE);
	// Initialise qtp pointer
	__malloc_qtp = SYSPAGE_ENTRY(qtime);
	_reset_timestamp();
	pthread_once(&once, create_keys);
	malloc_warn_level = M_HANDLE_TRACEBACK;
	malloc_fatal_level = M_HANDLE_TRACEBACK;



	if ((cptr = getenv("MALLOC_VERBOSE")) != NULL) {
		val = atoi(cptr);
		mallopt(MALLOC_VERBOSE, val);
	}

	if ((cptr = getenv("MALLOC_HANDLE_SIGNALS")) != NULL) {
		val = atoi(cptr);
		(void) mallopt(MALLOC_HANDLE_SIGNALS, val);
		if (malloc_verbose) {
			sprintf(buf, "got handle signals %d\n", val);
			write(2, buf, strlen(buf));
		}
	}

	install_handler();

	if ((cptr = getenv("MALLOC_TRACEBT")) != NULL) {
		tmbd = atoi(cptr);
		if (tmbd > 0) {
			tmbd = (tmbd <= CALLERPCDEPTH) ? tmbd : CALLERPCDEPTH;
		} else {
			tmbd = 0;
		}
		if (malloc_verbose) {
			sprintf(buf, "got bt depth %d\n", tmbd);
			write(2, buf, strlen(buf));
		}
	}

	if ((cptr = getenv("MALLOC_BTDEPTH")) != NULL) {
		val = atoi(cptr);
		if (val > 0) {
			__malloc_error_btdepth = val;
		}
		if (malloc_verbose) {
			sprintf(buf, "got error bt depth %d\n", val);
			write(2, buf, strlen(buf));
		}
	}

	if ((cptr = getenv("MALLOC_WARN")) != NULL) {
		val = atoi(cptr);
		(void) mallopt(MALLOC_WARN, val);
		if (malloc_verbose) {
			sprintf(buf, "got warn action %d\n", val);
			write(2, buf, strlen(buf));
		}
	}

	if ((cptr = getenv("MALLOC_FATAL")) != NULL) {
		val = atoi(cptr);
		(void) mallopt(MALLOC_FATAL, val);
		if (malloc_verbose) {
			sprintf(buf, "got fatal action %d\n", val);
			write(2, buf, strlen(buf));
		}
	}

	if ((cptr = getenv("MALLOC_CKCHAIN")) != NULL) {
		val = atoi(cptr);
		(void) mallopt(MALLOC_CKCHAIN, val);
		if (malloc_verbose) {
			sprintf(buf, "got ckchain %d\n", val);
			write(2, buf, strlen(buf));
		}
	}


	if ((cptr = getenv("MALLOC_EVENTFILE")) != NULL) {
		if (malloc_verbose) {
			sprintf(buf, "got event file %s\n", cptr);
			write(2, buf, strlen(buf));
		}
		(void) mallopt(MALLOC_EVENTFILE, (int)cptr);
	}

	if ((cptr = getenv("MALLOC_ERRFILE")) != NULL) {
		if (malloc_verbose) {
			sprintf(buf, "got error file %s\n", cptr);
			write(2, buf, strlen(buf));
		}
		(void) mallopt(MALLOC_ERRFILE, (int)cptr);
	}

	if ((cptr = getenv("MALLOC_FILLAREA")) != NULL ||
		(cptr = getenv("MALLOC_CKBOUNDS")) != NULL) {
		val = atoi(cptr);
		(void) mallopt(MALLOC_FILLAREA, val);
		if (malloc_verbose) {
			sprintf(buf, "got ckbounds %d\n", val);
			write(2, buf, strlen(buf));
		}
	}

	if ((cptr = getenv("MALLOC_CKACCESS")) != NULL ||
		(cptr = getenv("MALLOC_DEBUG")) != NULL) {
		val = atoi(cptr);
		(void) mallopt(MALLOC_CKACCESS, val);
		if (malloc_verbose) {
			sprintf(buf, "got ckaccess %d\n", val);
			write(2, buf, strlen(buf));
		}
	}

	if ((cptr = getenv("MALLOC_CKACCESS_LEVEL")) != NULL) {
		val = atoi(cptr);
		(void) mallopt(MALLOC_CKACCESS_LEVEL, val);
		if (malloc_verbose) {
			sprintf(buf, "got ckaccess level %d\n", val);
			write(2, buf, strlen(buf));
		}
	}

	if ((cptr = getenv("MALLOC_CKALLOC")) != NULL) {
		val = atoi(cptr);
		(void) mallopt(MALLOC_CKALLOC, val);
		if (malloc_verbose) {
			sprintf(buf, "got ckalloc %d\n", val);
			write(2, buf, strlen(buf));
		}
	}

	if ((cptr = getenv("MEMUSAGE_PROG_NAME")) != NULL
	    && __progname != NULL
	    && match_progname(cptr)) {
	    char libname[PATH_MAX];
	    char path[PATH_MAX];
	    char *src = "libmemusage";
	    char *cp = &libname[0];
	    char *sp = src;
	    while (*sp) *cp++ = *sp++;
	    *cp++ = '.';
	    *cp++ = 's';
	    *cp++ = 'o';
	    *cp++ = '\0';

	    if (findLibrary(libname,path) != NULL) {
			void *fhandle = dlopen(path, RTLD_NOW|RTLD_GLOBAL);
			if (fhandle != NULL) {
			    void (*memusage)(int,int) = dlsym(fhandle, "memusage");
			    if (memusage != NULL) {
					char *cp = getenv("MEMUSAGE_OUTPUT");
					int fd = -1;

					if (cp != NULL) {
					    if (cp != NULL && cp[0] != '\0'
			   			     && access(cp, R_OK|W_OK) != 0) {
							fd = creat(cp, 0666);
					    }
					}
					(*memusage)(1,fd);
				}
			} else {
			    fprintf(stderr, "error from dlopen(%s): %s\n", "libmemusage_g.so", (char *)strerror(errno));
			}
		}
	}

	if ((cptr = getenv("MALLOC_SHOW_LINKS"))  != NULL) {
		__malloc_show_links = 1;
	}

	if ((cptr = getenv("MALLOC_DETAIL"))  != NULL) {
		__malloc_detail = 1;
	}


	if ((cptr = getenv("MALLOC_TRACE")) != NULL) {
		if (malloc_verbose) {
			sprintf(buf, "got trace file %s\n", cptr);
			write(2, buf, strlen(buf));
		}
		(void) mallopt(MALLOC_TRACEFILE, (int)cptr);
	}

	if ((cptr = getenv("MALLOC_TRACEMIN")) != NULL) {
		val = atoi(cptr);
		(void) mallopt(MALLOC_TRACEMIN, val);
		if (malloc_verbose) {
			sprintf(buf, "got trace max %d\n", val);
			write(2, buf, strlen(buf));
		}
	}

	if ((cptr = getenv("MALLOC_TRACEMAX")) != NULL) {
		val = atoi(cptr);
		(void) mallopt(MALLOC_TRACEMAX, val);
		if (malloc_verbose) {
			sprintf(buf, "got trace max %d\n", val);
			write(2, buf, strlen(buf));
		}
	}

	if ((cptr = getenv("MALLOC_DUMP_LEAKS")) != NULL) {
		if (malloc_verbose) {
			sprintf(buf, "dumping leaks at exit\n");
			write(2, buf, strlen(buf));
		}
	  atexit(dump_leaks);
	}

	if ((cptr = getenv("MALLOC_STAT_BINS")) != NULL) {
		process_bins(malloc_verbose, cptr);
	}

	if ((cptr=getenv("MALLOC_USE_DLADDR")) != NULL) {
		val = atoi(cptr);
		(void) mallopt(MALLOC_USE_DLADDR, val);
		if (malloc_verbose) {
			sprintf(buf, "use dladd\n");
			write(2, buf, strlen(buf));
		}
	}

	if ((cptr=getenv("MALLOC_CTHREAD")) != NULL) {
		if (malloc_verbose) {
			sprintf(buf, "Starting debug control thread\n");
			write(2, buf, strlen(buf));
		}
		__ctid = __dbgm_StartThread(__dbg_malloc_thread, NULL);
	}

	if ((cptr=getenv("MALLOC_USE_CACHE")) != NULL) {
		val = atoi(cptr);
		mc_set_cache(val);
		if (malloc_verbose) {
			sprintf(buf, "use cache size=%d\n", val);
			write(2, buf, strlen(buf));
		}
	}

	__malloc_bt_depth = tmbd;
	_malloc_initialized = 1;
}

#if (_NTO_VERSION <= 632)
/**
 * this code is make librcheck run on 6.3.0 system where they don't have getpagesize function in libc
 */
#include <unistd.h>
#include <sys/mman.h>
#include <confname.h>

int
getpagesize(void)
{
	int pagesize;

	pagesize = sysconf(_SC_PAGESIZE);
	return (pagesize == -1) ? __PAGESIZE : pagesize;
}
#endif

/*
 * $Log$
 * Revision 1.22  2007/05/04 18:47:44  elaskavaia
 * - signal catcher handler - added an options to suppress it
 * PR:43722
 * CI:alain
 * CI:dinglis
 *
 * Revision 1.21  2007/05/04 17:15:12  elaskavaia
 * - signal catcher - produce a fatal error before exit
 * PR:43722
 * CI:alain
 * CI:dinglis
 *
 * Revision 1.20  2006/09/28 19:05:57  alain
 * PR:41782
 * CI: shiv@qnx.com
 * CI: cburgess@qnx.com
 *
 * Commiting the work done for IGT-CE on the head.
 *
 * Revision 1.19.2.14  2006/08/09 19:11:21  alain
 *
 * Add 2 new macros MALLOC_TRACEBTDEPTH and MALLOC_EVENTBTDEPTH to be able
 * to set bactrace depth.
 *
 * Revision 1.19.2.13  2006/07/27 19:53:29  alain
 *
 * Remove a deadlock (not calling LEAVE) in the control thread
 * new setting MALLOC_VERBOSE
 *
 * Revision 1.19.2.12  2006/07/26 19:53:49  alain
 *
 * Rename MALLOC_CKALLOC_PARAM to MALLOC_CKALLOC for consistency
 * add as clone to MALLOC_FILL_AREA MALLOC_CKBOUNDS for consistency
 *
 * Revision 1.19.2.11  2006/07/19 19:51:56  alain
 *
 *
 * We now have a new function call find_malloc_range() that does not do any
 * checking but rather (Dhead *)ptr - 1.  This is much faster in order of magnitue
 * of 10.  The bad things is that if the pointer is bad we will not be able to detect
 * it.  But to palliate there is an optiuon MALLOC_CHECK_ALLOC_PARAM that will revert
 * back to the old behaviour.
 * Now free() and realloc() only use the slow when MALLOC_CHEK_ALLOC_PARAM is set
 * malloc() always use the find_malloc_range() we assume that if it is succesfull not
 * need to check again.
 *
 * Things did not work for IGT since they use signal for IPC and the system call in QNX
 * are not restartable.  We need to make sure when the thread is created that it will ignore
 * the signals (block the signal for the controlling thread).
 *
 * Revision 1.19.2.10  2006/07/11 13:20:40  alain
 *
 * m_inti.c, malloc_g.c:  now the bins statistic are configurable.
 * Small optimisation when doing malloc.
 *
 * Revision 1.19.2.9  2006/05/29 04:12:28  alain
 *
 * There was an inconsistent in the values, so now:
 * MALLOC_TRACEBT --> __malloc_bt_depth
 * MALLOC_BTDEPTH --> __malloc_error_btdepth
 *
 * Revision 1.19.2.8  2006/05/15 18:21:16  alain
 *
 * malloc_trace_btdepth was rename malloc_error_btdepth
 *
 * Revision 1.19.2.7  2006/05/15 16:42:49  alain
 *
 * bt.c:add to backtrace() a starting address to eliminate undesirable backtrace
 * from the debug malloc library itself.
 *
 * mtrace.c: new format all  tracing event will have a STARTEV and ENDEV, the
 * backtrace is now after the {m,re,c}alloc line.
 *
 * calloc.c, realloc.c, free.c, malloc_g.c:
 * adjust the format to changes describe above.
 *
 * m_inti.c mallocpt.c:
 * new environment MALLOC_USE_DLADDR to enable or disable the use of dladdr() call.
 *
 * Revision 1.19.2.6  2006/03/16 20:48:52  alain
 *
 * Provide a configurable way of setting the statistics counters.
 * Do some indentations fixes.
 *
 * Revision 1.19.2.5  2006/03/13 20:23:39  alain
 *
 * Check for 2 new env variables, MALLOC_TRACEMIN and MALLOC_TRACEMAX.
 * Move the code of MALLOC_TRACE in mallopts and call this function to do the work.
 *
 * Revision 1.19.2.4  2006/03/06 21:12:10  alain
 *
 * Fix the indentation for tabs.
 *
 * Revision 1.19.2.3  2006/03/02 21:32:02  alain
 *
 *
 * We should not call getenv() anywhere this can lead to potential deadlocks
 * the strategy here is to do all the initialization upfront in the init code.
 * Also some minor formatting fixes.
 *
 * Revision 1.19.2.2  2006/02/14 18:20:37  alain
 *
 * Move the _timestamp call closer to the SYSPAGE call to regroup
 * functionnality.
 *
 * Revision 1.19.2.1  2006/02/02 18:57:37  alain
 *
 * PR:
 * CI:
 *
 * Put the timestamp support.
 *
 * Revision 1.19  2005/06/03 01:22:48  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.18  2005/03/29 18:22:44  shiv
 * PR24134
 * PR24010
 * PR24008
 * PR24184
 * The malloc lib used to report errors when mem* and str* functions
 * were called (for those that take a length parameter) with
 * a length of zero, if the other arguments are not valid..
 * In general these would not cause errors, since
 * no actual data is moved. But since the errors being reported could
 * be useful, the option to increase the level of verbosity for this
 * has been provided. the environment variable
 * MALLOC_CKACCESS_LEVEL can be used or the mallopt call
 * with the option mallopt(MALLOC_CKACCESS_LEVEL, arg)
 * can be used. By default the level is zero, a non-zero
 * level will turn on strict checking and reporting of errors
 * if the arguments are not valid.
 * Also fixed PR24184 which had an incorrect function name
 * being passed in (strchr instead of strrchr... thanx kirk)
 * Modified Files:
 * 	mallocint.h dbg/m_init.c dbg/malloc_chk.c dbg/malloc_g.c
 * 	dbg/mallopt.c dbg/memory.c dbg/string.c
 * 	public/malloc_g/malloc-lib.h
 *
 * Revision 1.17  2005/03/09 19:01:49  shiv
 * Missed in last commit.
 * Modified Files:
 * 	dbg/m_init.c
 *
 * Revision 1.16  2005/03/09 18:59:20  shiv
 * Fix a few issues with init code.
 * Modified Files:
 * 	mallocint.h dbg/m_init.c
 *
 * Revision 1.15  2005/02/25 21:35:12  shiv
 * A few more changes.
 * Modified Files:
 * 	m_init.c mtrace.c
 *
 * Revision 1.14  2005/02/25 03:03:37  shiv
 * More fixes for the debug malloc, for the tools to work
 * better.
 * Modified Files:
 * 	malloc-lib.h mallocint.h dbg/calloc.c dbg/dump.c dbg/m_init.c
 * 	dbg/malloc_debug.c dbg/malloc_g.c dbg/mtrace.c dbg/realloc.c
 * 	public/malloc_g/malloc.h
 *
 * Revision 1.13  2005/02/11 19:00:28  shiv
 * Some more malloc_g changes.
 * Modified Files:
 * 	mallocint.h dbg/dump.c dbg/m_init.c dbg/m_perror.c
 *  	dbg/malloc_g.c dbg/malloc_gc.c dbg/mtrace.c
 *
 * Revision 1.12  2005/01/16 20:38:45  shiv
 * Latest DBG malloc code. Lots of cleanup/optimistions
 * Modified Files:
 * 	common.mk mallocint.h common/tostring.c dbg/analyze.c
 * 	dbg/calloc.c dbg/dump.c dbg/free.c dbg/m_init.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc_g/malloc-lib.h public/malloc_g/malloc.h
 * 	std/calloc.c std/free.c std/m_init.c std/malloc_wrapper.c
 * 	std/mtrace.c std/realloc.c
 * Added Files:
 * 	dbg/dl_alloc.c dbg/malloc_control.c dbg/malloc_debug.c
 * 	dbg/new.cc public/malloc_g/malloc-control.h
 * 	public/malloc_g/malloc-debug.h
 *
 * Revision 1.11  2004/02/12 15:43:16  shiv
 * Updated copyright/licenses
 * Modified Files:
 * 	common.mk debug.h malloc-lib.h mallocint.h tostring.h
 * 	common/tostring.c dbg/analyze.c dbg/calloc.c dbg/context.h
 * 	dbg/crc.c dbg/dump.c dbg/free.c dbg/m_init.c dbg/m_perror.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc/malloc.h public/malloc_g/malloc-lib.h
 * 	public/malloc_g/malloc.h public/malloc_g/prototypes.h
 * 	std/calloc.c std/context.h std/free.c std/m_init.c
 * 	std/malloc_wrapper.c std/mtrace.c std/realloc.c test/memtest.c
 * 	test/mtrace.c
 *
 * Revision 1.10  2002/03/22 20:45:21  thomasf
 * Initial changes to make the output format machine readable for the events.
 *
 * Revision 1.9  2001/12/07 20:12:53  cburgess
 * fix for PR 8159 - libmalloc_g doesn't work under 6.1
 *
 * Revision 1.8.4.1  2001/12/07 20:03:15  cburgess
 * fix for PR 8159 - libmalloc_g doesn't work under 6.1
 * also made test programs work again and cleaned up some warnings
 *
 * Revision 1.8  2001/03/02 23:39:04  furr
 * Never look for libmemusage_g.so
 * Removed set_guard from prototypes
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	mallocint.h dbg/m_init.c dbg/malloc_g.c
 *
 * Revision 1.7  2001/03/01 20:37:53  furr
 * Added debug wrappers for the new aligned memory, mprobe and mcheck functions
 * Made mallopt signature compatible
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	common.mk mallocint.h dbg/m_init.c dbg/malloc_chk.c
 *  	dbg/malloc_g.c dbg/mallopt.c public/malloc/malloc.h
 *  	public/malloc_g/malloc-lib.h public/malloc_g/malloc.h
 *  	public/malloc_g/prototypes.h std/m_init.c std/malloc_wrapper.c
 *
 * Revision 1.6  2001/02/15 20:52:12  furr
 * Added new variants that allow debug malloc to be used by changing library
 * path.
 * Modified malloc_init to allow automatic leak detection on exit.
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	common.mk dbg/m_init.c
 *  Added Files:
 *  	x86/so.debug/Makefile x86/so.debug.g/Makefile
 *
 * Revision 1.5  2001/02/12 23:17:05  furr
 * Fixed typo in getpathname
 *
 *  Modified Files:
 *  	dbg/m_init.c std/m_init.c
 *
 * Revision 1.4  2001/02/12 19:27:13  furr
 * Corrected tostring location
 * Added mtrace utility sample
 * Added support for automatically generating trace and usage data.
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	dbg/m_init.c std/m_init.c
 *  Added Files:
 *  	test/README test/mtrace.c
 *  Removed Files:
 *  	dbg/tostring.c
 *
 * Revision 1.3  2001/02/05 22:07:12  furr
 * Minor fix to mtrace code (correctly print symbol offsets)
 * Warn about NULL pointers to str functions
 * Add stack tracebacks to warnings.
 * Added missing context file.
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	dbg/m_init.c dbg/malloc_g.c dbg/malloc_gc.c dbg/mtrace.c
 *  	dbg/string.c dbg/tostring.c public/malloc_g/malloc.h
 *  	public/malloc_g/prototypes.h
 *  Added Files:
 *  	dbg/context.h
 *
 * Revision 1.2  2001/02/05 18:34:30  furr
 * Added mtrace support
 * - produce output compatible with GNU tools
 *    - no code from GNU used, but see glibc mtrace.pl for what it expects
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	mallocint.h dbg/calloc.c dbg/free.c dbg/m_init.c
 *  	dbg/malloc_g.c dbg/realloc.c public/malloc_g/prototypes.h
 *  	test/memtest.c
 *  Added Files:
 *  	dbg/mtrace.c
 *
 * Revision 1.1  2000/01/31 19:03:30  bstecher
 * Create libmalloc.so and libmalloc_g.so libraries for everything. See
 * Steve Furr for details.
 *
 * Revision 1.1  2000/01/28 22:32:44  furr
 * libmalloc_g allows consistency checks and bounds checking of heap
 * blocks allocated using malloc.
 * Initial revision
 *
 *  Added Files:
 *  	Makefile analyze.c calloc_g.c crc.c dump.c free.c m_init.c
 *  	m_perror.c malloc-config.c malloc_chk.c malloc_chn.c
 *  	malloc_g.c malloc_gc.c mallopt.c memory.c process.c realloc.c
 *  	string.c tostring.c inc/debug.h inc/mallocint.h inc/tostring.h
 *  	inc/malloc_g/malloc inc/malloc_g/malloc.h
 *  	inc/malloc_g/prototypes.h test/memtest.C test/memtest.c
 *  	x86/Makefile x86/so/Makefile
 *
 * Revision 1.1  1996/08/18 21:36:57  furr
 * Initial revision
 *
 * Revision 1.11  1992/01/10  17:28:03  cpcahil
 * Added support for overriding void datatype
 *
 * Revision 1.10  1991/12/31  21:31:26  cpcahil
 * changes for patch 6.  See CHANGES file for more info
 *
 * Revision 1.9  1991/12/04  09:23:38  cpcahil
 * several performance enhancements including addition of free list
 *
 * Revision 1.8  91/11/25  14:41:54  cpcahil
 * Final changes in preparation for patch 4 release
 *
 * Revision 1.7  91/11/24  00:49:26  cpcahil
 * first cut at patch 4
 *
 * Revision 1.6  90/08/29  22:23:21  cpcahil
 * fixed mallopt to use a union as an argument.
 *
 * Revision 1.5  90/08/29  21:22:50  cpcahil
 * miscellaneous lint fixes
 *
 * Revision 1.4  90/05/11  15:53:35  cpcahil
 * fixed bug in initialization code.
 *
 * Revision 1.3  90/05/11  00:13:08  cpcahil
 * added copyright statment
 *
 * Revision 1.2  90/02/24  21:50:20  cpcahil
 * lots of lint fixes
 *
 * Revision 1.1  90/02/24  17:10:53  cpcahil
 * Initial revision
 *
 */
