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





#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/storage.h>
#include "malloc-lib.h"
#include "mallocint.h"
#include "tostring.h"

extern int _malloc_initialized;
extern const char	**_argv;

static int
writeFully(int fd, const char *buf, size_t n)
{
   int bytesWritten = 0;

   while (bytesWritten < n) {
       int nb;
       if ((nb = write(fd, buf, n-bytesWritten)) < 0 &&
           errno != EINTR) {
           return bytesWritten > 0 ? bytesWritten : nb;
       } else {
           bytesWritten += nb;
       }
   }
   return bytesWritten;
}

#define ERRSTR	"Unable to write mtrace data!"	
#define WRITEOUT(fd,str,len)	if ( writeFully(fd,str,(unsigned)(len)) != (len) ) \
		{ \
		    (void) writeFully(2,ERRSTR,\
			     (unsigned)strlen(ERRSTR));\
		    exit(120); \
		}

int _mtrace_level;
static int fd;
char buf[PIPE_BUF];

#if 0
// not being used now?
static void (*orig_malloc_init_hook)();
#endif
static void * (*orig_malloc_hook)(const char *file, int line, size_t size);
static void * (*orig_calloc_hook)(const char *file, int line, size_t nelems, size_t size);
static void * (*orig_realloc_hook)(const char *file, int line, void *ptr, size_t size);
static void (*orig_free_hook)(const char *file, int line, void *ptr);

#if 0
// not being used now?
static void mtrace_malloc_init();
#endif
static void * mtrace_malloc(const char *file, int line, size_t size);
static void * mtrace_calloc(const char *file, int line, size_t nelems, size_t size);
static void * mtrace_realloc(const char *file, int line, void *ptr, size_t size);
static void mtrace_free(const char *file, int line, void *ptr);

/* For compatibility with other mtrace implementations  - but watch for NULL */

void *mallwatch = &mallwatch;

/*
 * Avoid doing anything special with _dl_alloc. (Unreliable stack frames)
 */
void *
_dl_alloc(size_t size)
{
    return _malloc(size);
}

void
tr_break()
{
}

static void
mtrace_line(const char *file, unsigned long line)
{
    int len;
    char buf[80];
    if (file != NULL) {
	strcpy(buf, "@ ");
	len = 2;
	strcpy(&buf[len], file);
	len += strlen(file);
	strcpy(&buf[len++], ":");
        (void) tostring(&buf[len], (ulong_t)(line),10,B_DEC,' ');
        len += 10;
        buf[len++] = ' ';
        WRITEOUT(fd,buf,len);
	buf[0] = '\0';
    } else { /* Instruction pointer */
	Dl_info dli;

	if (dladdr((void *)line, &dli)) {
	    const char *fn = dli.dli_fname ? dli.dli_fname : "";
	    const char *sn = dli.dli_sname ? dli.dli_sname : ""; 
	    strcpy(buf, "@ ");
	    len = 2;
	    if (fn != NULL && fn[0]) {
		strcpy(&buf[len], fn);
        	len += strlen(fn);
        	buf[len++] = ':';
            }
            if (sn != NULL && sn[0] && strcmp("_btext", sn) != 0) {
            	int delta;
            	buf[len++] = '(';
		strcpy(&buf[len], sn);
        	len += strlen(sn);
        	if ((ulong_t)dli.dli_saddr > line) {
        	    buf[len++] = '-';
        	    delta = (ulong_t)dli.dli_saddr - line;
        	} else { /* in case Neutrino dladdr() ever changes: */
        	    buf[len++] = '+';
        	    delta = line - (ulong_t)dli.dli_saddr;
        	}
        	(void) tostring(&buf[len],delta,10,B_HEX,'0');
        	len += 10;
            	buf[len++] = ')';
            }
            buf[len++] = '[';
            (void) tostring(&buf[len], (ulong_t)(line),10,B_HEX,'0');
            len += 10;
            buf[len++] = ']';
	} else {
	    strcpy(buf, "@ [");
            len = 3;
            (void) tostring(&buf[len], (ulong_t)(line),10,B_HEX,'0');
            len += 10;
            buf[len++] = ']';
	}

        buf[len++] = ' ';
        WRITEOUT(fd,buf,len);
	buf[0] = '\0';
    }
}

#if defined(GET_REGFP)
static int
thr_stksegment(stack_t *stkseg)
{
    if (stkseg) {
	ulong_t base = (ulong_t)__tls();
	//if (stackbase != 0L) {
            stkseg->ss_sp = (void *)base;       /* hack! */
            stkseg->ss_size = (ulong_t)((char *)base - (char *)__tls()->__stackaddr);
            return 0;
	//}
        return 0;
    }
    return -1;
}

static void
DumpFrames(ucontext_t *ucp, int level) 
{
    long *fp;
    void *ip = NULL;
    int l;
    //char buf[80];
    ulong_t stacklo, stackhi;

    if (_argv == NULL) return; /* don't try to traceback inside ldd */
    if (level <= 0) return;

    stacklo = (ulong_t)ucp->uc_stack.ss_sp - ucp->uc_stack.ss_size;
    stackhi = (ulong_t)ucp->uc_stack.ss_sp;

    for (fp = (long *)GET_REGFP(&ucp->uc_mcontext.cpu), l = 0;
	fp != NULL && l <= level;
	fp = GET_FRAME_PREVIOUS(fp), l++) {
        if ((ulong_t)fp < stacklo || (ulong_t)fp > stackhi) break;
	ip = (void *)GET_FRAME_RETURN_ADDRESS(fp);
	if (ip == NULL) break;
    }
    
    if (l == level+1) { /* We went up the correct number of levels */
	mtrace_line(NULL, (unsigned long)ip);
        WRITEOUT(fd,"\n",1);
    }
    DumpFrames(ucp, level-1);
}

static void
DumpTraceback(int line, int level) 
{
    ucontext_t uc;
    void *ip = NULL;
    long *fp = NULL;
    ulong_t stacklo, stackhi;

    if (_argv == NULL) return; /* don't try to traceback inside ldd */
    if (level <= 0) return;

    uc.uc_mcontext.cpu.ebp = _ebp();
    uc.uc_mcontext.cpu.eip = line;

    thr_stksegment((void *)&uc.uc_stack);
    uc.uc_stack.ss_flags = 0;
    stacklo = (ulong_t)uc.uc_stack.ss_sp - uc.uc_stack.ss_size;
    stackhi = (ulong_t)uc.uc_stack.ss_sp;

    /*
     * Find the correct frame on the stack 
     */
    for (fp = (long *)GET_REGFP(&uc.uc_mcontext.cpu); fp != NULL;
	fp = GET_FRAME_PREVIOUS(fp)) {
        if ((ulong_t)fp < stacklo || (ulong_t)fp > stackhi) break;
	ip = (void *)GET_FRAME_RETURN_ADDRESS(fp);
	if (ip == NULL) break;
	if ((int)ip == line) break;
    }
    if ((int)ip == line) {
	/*
	 * If we found the frame, trace up the correct number of levels.
	 */
	GET_REGFP(&uc.uc_mcontext.cpu) = (ulong_t)fp;
        DumpFrames(&uc, level);
    }
}
#endif

static void
mtrace_where(const char *file, int line)
{
    /*
     * Change this to trace back the stack _mtrace_level # of frames
     * Report as <file>:<symbol>+offset where it is gotten from dladdr
     */
#if defined(GET_REGFP)
    if (file == NULL && line != 0) {
	DumpTraceback(line, _mtrace_level);
    } 
#endif
    if (file != NULL || line != 0) {
	mtrace_line(file,line);
    }
}

static void *mtrace_malloc(const char *file, int line, size_t size)
{
    void *cptr;
    //int len;
    int len;

    __malloc_hook = orig_malloc_hook;
    cptr = (*__malloc_hook)(file,line,size);
    __malloc_hook = mtrace_malloc;

    mtrace_where(file,line);

    strcpy(buf, "+ ");
    len = 2;
    (void) tostring(&buf[len], (ulong_t)(cptr),10,B_HEX,'0');
    len += 10;
    buf[len++] = ' ';
    (void) tostring(&buf[len], (ulong_t)(size),10,B_DEC,' ');
    len += 10;
    buf[len++] = '\n';
    buf[len] = '\0';
    WRITEOUT(fd,buf,len);
    buf[0] = '\0';

    //sprintf(buf, "+ %x %x\n", (unsigned int)cptr, size);

    if (cptr == mallwatch) tr_break();

    return cptr;
}

static void *mtrace_realloc(const char *file, int line, void *cptr, size_t size)
{
    int len;
    void *ptr;

    /* Copy all the hooks, because realloc could use any of the others */
    __malloc_hook = orig_malloc_hook;
    __calloc_hook = orig_calloc_hook;
    __realloc_hook = orig_realloc_hook;
    __free_hook = orig_free_hook;
    ptr = (*__realloc_hook)(file,line,cptr,size);
    __malloc_hook = mtrace_malloc;
    __calloc_hook = mtrace_calloc;
    __realloc_hook = mtrace_realloc;
    __free_hook = mtrace_free;

    mtrace_where(file,line);

    if (ptr == NULL) {
	strcpy(buf, "! ");
	len = 2;
	(void) tostring(&buf[len], (ulong_t)(cptr),10,B_HEX,'0');
	len += 10;
	buf[len++] = ' ';
	(void) tostring(&buf[len], (ulong_t)(size),10,B_DEC,' ');
	len += 10;
	buf[len++] = '\n';
	buf[len] = '\0';
	WRITEOUT(fd,buf,len);
	buf[0] = '\0';
	//sprintf(buf, "! %x %x\n", (unsigned int)cptr, size);
    } else if (cptr == NULL) {
	strcpy(buf, "+ ");
	len = 2;
	(void) tostring(&buf[len], (ulong_t)(ptr),10,B_HEX,'0');
	len += 10;
	buf[len++] = ' ';
	(void) tostring(buf+len, (ulong_t)(size),10,B_DEC,' ');
	len += 10;
	buf[len++] = '\n';
	buf[len] = '\0';
	WRITEOUT(fd,buf,len);
	buf[0] = '\0';
	//sprintf(buf, "+ %x %x\n", (unsigned int)ptr, size);
    } else {
	strcpy(buf, "< ");
	len = 2;
	(void) tostring(buf+len, (ulong_t)(cptr),10,B_HEX,'0');
	len += 10;
	buf[len++] = '\n';
	buf[len] = '\0';
	WRITEOUT(fd,buf,len);
	mtrace_where(file,line);
	buf[0] = '\0';
	strcpy(buf, "> ");
	len = 2;
	(void) tostring(buf+len, (ulong_t)(ptr),10,B_HEX,'0');
	len += 10;
	buf[len++] = ' ';
	(void) tostring(buf+len, (ulong_t)(size),10,B_DEC,' ');
	len += 10;
	buf[len++] = '\n';
	buf[len] = '\0';
	WRITEOUT(fd,buf,len);
	buf[0] = '\0';
	//sprintf(buf, "< %x\n> %x %x\n", (unsigned int)cptr, (unsigned int)ptr, size);
    }

    if (cptr == mallwatch) tr_break();

    return ptr;
}

static void *mtrace_calloc(const char *file, int line, size_t nelems, size_t size)
{
    int len;
    void *ptr;

    /* Copy the malloc hook, because calloc could it */
    __malloc_hook = orig_malloc_hook;
    __calloc_hook = orig_calloc_hook;
    ptr = (*__calloc_hook)(file,line,nelems,size);
    __malloc_hook = mtrace_malloc;
    __calloc_hook = mtrace_calloc;

    mtrace_where(file,line);

    strcpy(buf, "+ ");
    len = 2;
    (void) tostring(&buf[len], (ulong_t)(ptr),10,B_HEX,'0');
    len += 10;
    buf[len++] = ' ';
    (void) tostring(&buf[len], (ulong_t)(nelems*size),10,B_DEC,' ');
    len += 10;
    buf[len++] = '\n';
    buf[len] = '\0';
    WRITEOUT(fd,buf,len);
    buf[0] = '\0';

    if (ptr == mallwatch) tr_break();

    return ptr;
}

static void mtrace_free(const char *file, int line, void *cptr)
{
    int len;

    mtrace_where(file,line);

    strcpy(buf, "- ");
    len = 2;
    (void) tostring(buf+len, (ulong_t)(cptr),10,B_HEX,'0');
    len += 10;
    buf[len++] = '\n';
    buf[len] = '\0';
    WRITEOUT(fd,buf,len);
    buf[0] = '\0';

    if (cptr == mallwatch) tr_break();

    /* Copy the malloc hook, because calloc could it */
    __free_hook = orig_free_hook;
    (*__free_hook)(file,line,cptr);
    __free_hook = mtrace_free;
}

void
muntrace()
{ 
    ENTER();
    if (fd >= 0) {
	WRITEOUT(fd,"= End\n", strlen("= End\n"));
	close(fd);
    }
    LEAVE();
}

void
mtrace()
{ 
    char *filename;
    ENTER();
    if (!_malloc_initialized) {
	char *level = getenv("MALLOC_TRACE_LEVEL");
	filename = getenv("MALLOC_TRACE");
	if (filename != NULL) {
	    fd = open(filename,O_CREAT|O_APPEND|O_WRONLY,0666);
	    if ( fd == -1 ) {
		char *s;
		static char *errmsg = "Unable to open malloc error file: ";
		WRITEOUT(2, errmsg, strlen(errmsg));
		for(s=filename; *s; s++)
		{
		    /* do nothing */;
		}
		WRITEOUT(2,filename, (unsigned)(s-filename));
		WRITEOUT(2,"\n",(unsigned)1);
	    } else {
		static char *start_msg = "= Start\n";
		orig_malloc_hook = __malloc_hook;
		orig_calloc_hook = __calloc_hook;
		orig_realloc_hook = __realloc_hook;
		orig_free_hook = __free_hook;
		__malloc_hook = mtrace_malloc;
		__calloc_hook = mtrace_calloc;
		__realloc_hook = mtrace_realloc;
		__free_hook = mtrace_free;
		WRITEOUT(fd, start_msg, strlen(start_msg));
		atexit(muntrace);
	    }
	}
	if (level != NULL) {
	    _mtrace_level = strtoul(level, NULL, 0);
	}
    }
    LEAVE();
}
