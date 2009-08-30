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
#include "malloc-lib.h"
#include "mallocint.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dlfcn.h>
#include <limits.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

int malloc_inited;
int _malloc_initialized;

extern char *__progname;
extern const char **_argv;

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
	    while (*sp) *dp++ = *sp++;
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
	    while (*dp) dp++;
	    *dp = '\0';
	    sp = cp;
	    if (*sp == '.' && *(sp+1) == '/') {
		sp++;
	    }
	    while (*sp) *dp++ = *sp++;
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
            if (*pp != *cp) break;
            if (*pp == '\0' || *cp == '\0') break;
	}
    }
    return progname && cptr && *pp == '\0' && *cp == '\0';
}

/*
 * Function:	_malloc_init()
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
_malloc_init()
{
	char			* cptr;
	char			* getenv();

	/*
 	 * If already initialized...
	 */
	if( malloc_inited != 0)	/* because the implementation may set malloc_data_start */
	{
		return;
	}

	malloc_inited = 1;
	if ( (cptr = getenv("MALLOC_TRACE")) != NULL) {
	    extern void mtrace();
	    mtrace();
	}
	if ((cptr = getenv("MEMUSAGE_PROG_NAME")) != NULL
	    && __progname != NULL
	    && match_progname(cptr)) {
	    char path[PATH_MAX];
	    if (findLibrary("libmemusage.so",path) != NULL) {
		void *fhandle = dlopen(path, RTLD_NOW|RTLD_GLOBAL);
		if (fhandle != NULL) {
		    void (*memusage)(int,int) = dlsym(fhandle, "memusage");
		    if (memusage != NULL) {
			char *cp = getenv("MEMUSAGE_OUTPUT");
			int fd = -1;

			if (cp != NULL) {
			    if (cp != NULL && cp[0] != '\0'
			        && access(cp, R_OK|W_OK) != 0)
			    {
				fd = creat(cp, 0666);
			    }
			}
			(*memusage)(1,fd);
		    }
		} else {
		    fprintf(stderr, "error from dlopen(%s): %s\n", "libmemusage.so", (char *)strerror(errno));
		}
	    }
	}
	_malloc_initialized = 1;
}

void
_malloc_init_direct()
{
    _malloc_init();
}

void
malloc_init()
{
    ENTER();
    if (malloc_inited) return;
    if (__malloc_init_hook != NULL) {
    	(*__malloc_init_hook)();
    } else {
    	_malloc_init();
    }
    LEAVE();
}

/*
 * $Log$
 * Revision 1.7  2005/06/03 01:22:48  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.6  2005/01/16 20:38:46  shiv
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
 * Revision 1.5  2004/02/12 15:43:17  shiv
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
 * Revision 1.4  2001/03/01 20:37:55  furr
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
 * Revision 1.3  2001/02/12 23:17:05  furr
 * Fixed typo in getpathname
 *
 *  Modified Files:
 *  	dbg/m_init.c std/m_init.c
 *
 * Revision 1.2  2001/02/12 19:27:13  furr
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
 * Revision 1.1  2001/02/09 22:28:12  furr
 * Added necessary rule changes and source code to support non-debugging
 * malloc library.
 *   - Keeps same info as libc/alloc with guard code and caller pc turned
 *     on
 *   - Supports malloc hooks for functionality such as mtrace, memusage
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	common.mk malloc-lib.h mallocint.h dbg/tostring.c
 *  	public/malloc_g/malloc.h public/malloc_g/prototypes.h
 *  	test/memtest.c
 *  Added Files:
 *  	common/tostring.c public/malloc/malloc.h
 *  	public/malloc_g/malloc-lib.h std/calloc.c std/context.h
 *  	std/free.c std/m_init.c std/malloc_wrapper.c std/mtrace.c
 *  	std/realloc.c
 *
 */
