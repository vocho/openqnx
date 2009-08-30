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
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "malloc-lib.h"

//For the debug signal below
#include <sys/siginfo.h>

/*
 * Function:	mallopt()
 *
 * Purpose:	to set options for the malloc debugging library
 *
 * Arguments:	none
 *
 * Returns:	nothing of any value
 *
 * Narrative:	
 *
 */

#if 0
static void *_dbgmalloc_worker_thread(union sigval arg) {
	//@@@ In the future we should be able to do lots of stuff here.
	if(malloc_eventfd != -1) {
		malloc_dump_unreferenced(malloc_eventfd, 1);
	}
	return 0;
}
#endif

/**
 We use this to open up a side channel connection rather than a normal
 fd so that when we procmgr_daemon we don't close the connection.  We
 need to do more than this really since it could be that we fork() and
 that the fd doesn't get propagated across properly.
 **/
#include <fcntl.h>
#include <share.h>
#include <stdarg.h>
#include <sys/iofunc.h>

void _check_list (int force);

int _malloc_open_sc(const char *path, int oflag, int mode) {
	return _connect(_NTO_SIDE_CHANNEL, path, mode, oflag, 
					SH_DENYNO, _IO_CONNECT_OPEN, 1, 
					_IO_FLAG_RD | _IO_FLAG_WR, 0, 0, 0, 0, 0, 0, 0);
}

/*
 * Doe variable tell us if yes/no we have to close
 * the file descriptor;
 */

static int fd_set_eventfile;
static int fd_set_errfile;

int
mallopt(int cmd, int value)
{
	int status = 0;
	int			  i;
	register char		* s;

	ENTER();
	MALLOC_INIT();

	switch(cmd) {
		case M_MXFAST:	/* SysV - quietly ignore */
		case M_NLBLKS:	/* SysV */
		case M_GRAIN:	/* SysV */
			break;
		case M_TRIM_THRESHOLD:	/* GNU - quietly ignore */
		case M_TOP_PAD:		/* GNU - quietly ignore */
		case M_MMAP_THRESHOLD:	/* GNU - quietly ignore */
		case M_MMAP_MAX:	/* GNU - quietly ignore */
			break;
		case MALLOC_VERIFY:
			DBFmalloc_chain_check("<none>", "<none>", 0, 1);
			_check_list(1);
			break;
		case MALLOC_WARN:
			malloc_warn_level = value;
			break;

		case MALLOC_FATAL:
			malloc_fatal_level = value;
			break;

		case MALLOC_CKCHAIN:
		case MALLOC_VERIFY_ON:
			_malloc_check_on = value;
			_malloc_free_check = value;
			break;

		case MALLOC_CKACCESS:
			malloc_verify_access = value && !malloc_verify_access
				? malloc_hist_id : malloc_verify_access;
			break;

		case MALLOC_CKACCESS_LEVEL:
			malloc_verify_access_level = value;
			break;

		case MALLOC_CKALLOC:
			malloc_verify_alloc = value;
			break;
	    case MALLOC_HANDLE_SIGNALS:
			malloc_handle_signals = value;
			break;

		case MALLOC_FILLAREA:
			_malloc_fill_area = value
					? (!_malloc_fill_area 
					    ? malloc_hist_id : _malloc_fill_area)
					: value;
			break;

		case MALLOC_VERBOSE:
			malloc_verbose = value;
			break;

		case MALLOC_EVENTFILE: {
			char *name = (char *)value;
			if (fd_set_eventfile) {
				if (malloc_eventfd != -1) {
					close(malloc_eventfd);
				}
			}
			fd_set_eventfile = 0;
			i = -1;
			if (name != NULL && *name != '\0') {
				if (strcmp(name, "STDOUT") == 0) {
					i = 1;
				} else if (strcmp(name, "STDERR") == 0) {
					i = 2;
				} else {
					i = _malloc_open_sc(name, O_CREAT|O_APPEND|O_WRONLY, 0666);
					fd_set_eventfile = 1;
				}
				if (i == -1) {
					(void) write(2, "Unable to open malloc error file: ", (unsigned) 34);
					for (s = (char *)value; *s; s++) {
						/* do nothing */;
					}
					(void) write(2, name, (unsigned)(s - (char *)value));
					(void) write(2, "\n", (unsigned)1);
				}
			}
			malloc_eventfd = i;
			break;
		}

		case MALLOC_EVENTBTDEPTH:
			if (value > 0) {
				__malloc_error_btdepth = (value <= CALLERPCDEPTH) ? value : CALLERPCDEPTH;
			} else {
				__malloc_error_btdepth = value;
			}
			break;

		case MALLOC_ERRFILE:{
			char *name = (char *)value;
			if (fd_set_errfile) {
				if (malloc_errfd != -1) {
					close(malloc_errfd);
				}
			}
			fd_set_errfile = 0;
			i = -1;
			if (name != NULL && *name) {
				if (strcmp(name, "STDOUT") == 0) {
					i = 1;
				} else if (strcmp(name, "STDERR") == 0) {
					i = 2;
				} else {
					i = _malloc_open_sc(name, O_CREAT|O_APPEND|O_WRONLY, 0666);
					fd_set_errfile = 1;
				}
				if (i == -1) {
					(void) write(2, "Unable to open malloc error file: ", (unsigned) 34);
					for (s = (char *)value; *s; s++) {
						/* do nothing */;
					}
					(void) write(2, name, (unsigned)(s - (char *)value));
					(void) write(2, "\n", (unsigned)1);
				}
			}
			malloc_errfd = i;
			break;
		}
		case MALLOC_TRACEFILE: {
			char *name = (char *)value;
			if (name != NULL && *name != '\0') { 
				strcpy(__malloc_trace_filename, name);
				mtrace();
			} else {
				muntrace();
			}
			break;
		}

		case MALLOC_TRACEMIN:
			__malloc_trace_minsz = (value < 1) ? -1 : value;
			break;

		case MALLOC_TRACEMAX:
			__malloc_trace_maxsz= (value < 1) ? -1 : value;
			break;

		case MALLOC_TRACEBTDEPTH:
			if (value > 0) {
				__malloc_bt_depth = (value <= CALLERPCDEPTH) ? value : CALLERPCDEPTH;
			} else {
				__malloc_bt_depth = value;
			}
			break;

		case MALLOC_USE_DLADDR:
			__malloc_use_dladdr = value;
			break;

		default:
			status = malloc_opts(cmd, (void *)value);
			break;
	}
	LEAVE();
	return status;
}

/*
 * $Log$
 * Revision 1.14  2007/05/04 18:47:44  elaskavaia
 * - signal catcher handler - added an options to suppress it
 * PR:43722
 * CI:alain
 * CI:dinglis
 *
 * Revision 1.13  2007/04/24 15:44:27  shiv
 *
 * PR:29730
 * CI:cburgess
 * CI:xtang
 *
 * this is part of work order WO790334 for IGT. Included enhancements
 * for configurability of the memory allocator.  Includes matching changes for
 * lib/c/alloc and lib/malloc as usual. This is to bring head in line
 * with the work committed to the branch.
 *
 * Revision 1.12  2006/09/28 19:05:57  alain
 * PR:41782
 * CI: shiv@qnx.com
 * CI: cburgess@qnx.com
 *
 * Commiting the work done for IGT-CE on the head.
 *
 * Revision 1.11.2.11  2006/08/09 19:11:21  alain
 *
 * Add 2 new macros MALLOC_TRACEBTDEPTH and MALLOC_EVENTBTDEPTH to be able
 * to set bactrace depth.
 *
 * Revision 1.11.2.10  2006/08/09 13:15:39  alain
 *
 * malloc_control.c:  Make sure when setting the bactrace depth we do not
 * to over the size of the bactrace array.
 *
 * Revision 1.11.2.9  2006/07/26 19:58:24  alain
 *
 * Rename malloc_check_alloc to malloc_verify_alloc for consistency
 *
 * Revision 1.11.2.8  2006/07/26 19:53:50  alain
 *
 * Rename MALLOC_CKALLOC_PARAM to MALLOC_CKALLOC for consistency
 * add as clone to MALLOC_FILL_AREA MALLOC_CKBOUNDS for consistency
 *
 * Revision 1.11.2.7  2006/07/26 15:58:45  alain
 *
 * Only register the muntrace() once since the atexit() function is limited
 * in the number of registry and the use could call mtrace() more then once.
 * DBM_DEVCTL_NOEVENTFILE new macro.
 *
 * Revision 1.11.2.6  2006/07/25 19:59:45  alain
 *
 * In malloc_control.c added more settings: setting of the event_file
 * setting of all the checkings (CKACESS, CKCHAIN, etc ...).
 * In mallopt EVENTFILE/ERRFILE accepts argument STDOUT and STDERR
 * as special ot open fd 1, 2.
 * Same for TRACEFILE in mtrace.c
 *
 * Revision 1.11.2.5  2006/07/19 19:51:57  alain
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
 * Revision 1.11.2.4  2006/06/29 13:48:20  alain
 *
 * malloc_g.c: (BINS)The calculation of the total number of blocks was incorrect,
 * it did not take to account the numbers in the depleted list.
 * (USAGE): we now return the overhead:use:free total of the memory.
 * mallopt.c:  Doing MALLOPT_VERIFY was a noop, we now call _check_list(1);
 * mtrace.c:  some WRITEOUTs were missing the newlines.
 *
 * Revision 1.11.2.3  2006/05/15 16:42:49  alain
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
 * Revision 1.11.2.2  2006/03/13 20:24:46  alain
 *
 * Added three new commands MALLOC_TRACEMIN, MALLOC_TRACEMAX and MALLOC_TRACE
 *
 * Revision 1.11.2.1  2006/03/13 17:19:08  alain
 *
 *
 * Cleaning up of the indentation and remove of dead code.
 *
 * Revision 1.11  2005/06/03 01:22:48  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.10  2005/03/29 18:22:44  shiv
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
 * Revision 1.9  2005/01/16 20:38:45  shiv
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
 * Revision 1.8  2004/02/12 15:43:17  shiv
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
 * Revision 1.7  2003/10/01 21:54:20  shiv
 * To match the changes made to libc for the dlist stats gathering
 * stuff, the pieces in malloc-lib.h were no longer necessary.
 *
 * Modified Files:
 * 	dbg/mallopt.c public/malloc_g/malloc-lib.h
 *
 * Revision 1.6  2002/04/24 14:18:05  thomasf
 * Use a side channel for reporting so that this library can be used for
 * resource managers and servers which procmgr_daemon() themselves.
 *
 * Revision 1.5  2002/04/19 18:12:58  thomasf
 * Added support for statistics in the debug malloc library as well as
 * the feeding of symbols into the event tracer as a stop-gap measure
 * for not having a direct symbol lookup.
 *
 * Revision 1.4  2002/03/22 20:45:21  thomasf
 * Initial changes to make the output format machine readable for the events.
 *
 * Revision 1.3  2001/12/07 20:12:53  cburgess
 * fix for PR 8159 - libmalloc_g doesn't work under 6.1
 *
 * Revision 1.2.4.1  2001/12/07 20:03:15  cburgess
 * fix for PR 8159 - libmalloc_g doesn't work under 6.1
 * also made test programs work again and cleaned up some warnings
 *
 * Revision 1.2  2001/03/01 20:37:54  furr
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
 * Revision 1.1  2000/01/31 19:03:31  bstecher
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
 * Revision 1.1  1996/08/18 21:25:10  furr
 * Initial revision
 *
 * Revision 1.11  1992/01/10  17:28:03  cpcahil
 * Added support for overriding void datatype
 *
 * Revision 1.10  1991/12/31  21:31:26  cpcahil
 * changes for patch 6.  See CHANGES file for more info
 *
 * Revision 1.9  1991/12/04  09:23:42  cpcahil
 * several performance enhancements including addition of free list
 *
 * Revision 1.8  91/11/25  14:42:03  cpcahil
 * Final changes in preparation for patch 4 release
 * 
 * Revision 1.7  91/11/24  00:49:30  cpcahil
 * first cut at patch 4
 * 
 * Revision 1.6  90/08/29  22:23:36  cpcahil
 * fixed mallopt to use a union as an argument.
 * 
 * Revision 1.5  90/08/29  21:22:51  cpcahil
 * miscellaneous lint fixes
 * 
 * Revision 1.4  90/05/11  00:13:10  cpcahil
 * added copyright statment
 * 
 * Revision 1.3  90/02/25  11:03:26  cpcahil
 * changed to return int so that it agrees with l libmalloc.a's mallopt()
 * 
 * Revision 1.2  90/02/25  11:01:21  cpcahil
 * added support for malloc chain checking.
 * 
 * Revision 1.1  90/02/24  21:50:24  cpcahil
 * Initial revision
 * 
 * Revision 1.1  90/02/24  17:10:53  cpcahil
 * Initial revision
 * 
 */
