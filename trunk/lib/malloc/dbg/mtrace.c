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
#include "debug.h"
#include "tostring.h"

extern int _malloc_open_sc(const char *name, int perm, int mode);
extern int __check_ctid();

#define ERRBUFSIZE      1024+(CALLERPCDEPTH*16)
static char  errbuf[ERRBUFSIZE];

int
writeFully(int lfd, const char *buf, size_t n)
{
   int bytesWritten = 0;

   while (bytesWritten < n) {
       int nb;
       if ((nb = write(lfd, buf, n-bytesWritten)) < 0 &&
           errno != EINTR) {
           return bytesWritten > 0 ? bytesWritten : nb;
       } else {
           bytesWritten += nb;
       }
   }
   return bytesWritten;
}

#define ERRSTR	"Unable to write mtrace data!"	
#define WRITEOUT(__nfd,str,len)	if ( writeFully(__nfd,str,(unsigned)(len)) != (len) ) \
		{ \
		    (void) writeFully(2,ERRSTR,\
			     (unsigned)strlen(ERRSTR));\
		    exit(120); \
		}

int __malloc_trace_fd=-1;
char __malloc_trace_filename[_POSIX_PATH_MAX];
int __malloc_trace_minsz=-1;
int __malloc_trace_maxsz=-1;
int __malloc_use_dladdr = 0;

static int set_atexit;
static int fd_set_tracefile;

static char buf[PIPE_BUF];

static void * (*orig_malloc_hook)(const char *file, int line, size_t size);
static void * (*orig_calloc_hook)(const char *file, int line, size_t nelems, size_t size);
static void * (*orig_realloc_hook)(const char *file, int line, void *ptr, size_t size);
static void (*orig_free_hook)(const char *file, int line, void *ptr);

static void * mtrace_malloc(const char *file, int line, size_t size);
static void * mtrace_calloc(const char *file, int line, size_t nelems, size_t size);
static void * mtrace_realloc(const char *file, int line, void *ptr, size_t size);
static void mtrace_free(const char *file, int line, void *ptr);

/* For compatibility with other mtrace implementations  - but watch for NULL */

void *mallwatch = &mallwatch;

void
tr_break()
{
}

static void
start_event()
{
	if (__malloc_use_dladdr) {
		const char *start_ev = STARTEV_STR;
		char *s;

		s = errbuf;
		COPY(s, start_ev, errbuf, ERRBUFSIZE);
		*s++ = ' ';
		s += tostring(s,(ulong_t)getpid(),10,10,' ');
		*s++ = ' ';
		s += tostring(s,(ulong_t)pthread_self(),5,10,' ');
		*s++ = '\n';
		*s = '\0';
		WRITEOUT(__malloc_trace_fd,errbuf,(unsigned)(s-errbuf));
	}
}

static void
end_event()
{
	if (__malloc_use_dladdr) {
		const char *start_ev = ENDEV_STR;
		char *s;

		s = errbuf;
		COPY(s, start_ev, errbuf, ERRBUFSIZE);
		*s++ = ' ';
		s += tostring(s,(ulong_t)getpid(),10,10,' ');
		*s++ = ' ';
		s += tostring(s,(ulong_t)pthread_self(),5,10,' ');
		*s++ = '\n';
		*s = '\0';
		WRITEOUT(__malloc_trace_fd,errbuf,(unsigned)(s-errbuf));
	}
}

static void
mtrace_backtrace_bt(unsigned long line, unsigned int *__bt[], const char *prefix)
{
	int j=0;
	for (j=0; j < CALLERPCDEPTH; j++) {
		if (__bt[j] == (unsigned int *)line) {
				break;
		}
	}
	while (__bt[j]) {
		malloc_dump_line(__malloc_trace_fd, NULL, (unsigned int)__bt[j], prefix);
		j++;
		if (j >= CALLERPCDEPTH) {
			break;
		}
	}
}

static void
mtrace_backtrace_dbg(unsigned long line, DebugInfo_t *dbgptr, const char *prefix)
{
	__Dbg_Data *dd=NULL;
	__Dbg_St   *ds;

	dd = dbgptr->dbg;
	if (dd) {
		int j=0;
		ds = dd->bt;
		while (ds) {
			unsigned *cline;
			cline = ds->line;
			if (cline == 0) {
				break;
			}
			malloc_dump_line(__malloc_trace_fd, NULL, (unsigned int)cline, prefix);
			j++;
			ds = ds->next;
			if (ds && (ds->line == dd->bt->line)) {
				break;
			}
		}
	}
}

static void
mtrace_line(const char *file, unsigned long line, DebugInfo_t *dbgptr, 
            int justline, int usebt, unsigned int *__bt[])
{
	int len;
	uint64_t timestamp;


	timestamp = _get_timestamp();
	if (file != NULL) {
		buf[0] = '@'; buf[1] = ' ';
		len = 2;
		/*strcpy(&buf[len], file);*/
		sprintf(&buf[len], "%s", file);
		len += strlen(file);
		buf[len++] = ':';
		(void) tostring(&buf[len], (ulong_t)(line),10,B_DEC,' ');
		len += 10;
		buf[len++] = '\n';
		buf[len] = '\0';
		WRITEOUT(__malloc_trace_fd,buf,len);
		buf[0] = '\0';
	} else { /* Instruction pointer */
		if (!justline) {

			buf[0] = '@'; buf[1] = ' '; buf[2] = '[';
			len = 3;
			if (dbgptr) {
				__Dbg_Data *dd=NULL;
				__Dbg_St   *ds;
				dd = dbgptr->dbg;
				if (dd) {
					int j=0;
					ds = dd->bt;
					while (ds) {
						unsigned *cline;
						cline = ds->line;
						if (cline == 0)
							break;
						if (j > 0) {
							buf[len++] = ':';
						}
						tostring(buf+len,(ulong_t)(cline),10,B_HEX,'0');
						len+=10;
						j++;
						ds = ds->next;
						if (ds && (ds->line == dd->bt->line))
							break;
					}
					if (j == 0) {
						(void) tostring(&buf[len], (ulong_t)(line),10,B_HEX,'0');
						len += 10;
					}
				}
			} else { /* dbptr */
				if (!usebt) {
					(void) tostring(&buf[len], (ulong_t)(line),10,B_HEX,'0');
					len += 10;
				} else {
					int j=0;
					int count=0;
// STB: this for loop can probably be removed... (see newer backtrace()
					for (j=0; j < CALLERPCDEPTH; j++) {
						if (__bt[j] == (unsigned int *)line)
							break;
					}
					while (__bt[j]) {
						if (count > 0) {
							buf[len++] = ':';
						}
						tostring(buf+len,(ulong_t)(__bt[j]),10,B_HEX,'0');
						len+=10;
						j++;
						count++;
						if (j >= CALLERPCDEPTH)
							break;
					}
					if (count == 0) {
						(void) tostring(&buf[len], (ulong_t)(line),10,B_HEX,'0');
						len += 10;
					}
				}
			}
			buf[len++] = ']';


			{
				__Dbg_Data *dd;
				dd = (dbgptr ? dbgptr->dbg : NULL);
				buf[len++] = ' ';
				tostring(&buf[len], (ulong_t)(getpid()),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';
				tostring(&buf[len], 
				(ulong_t)(dd ? dd->tid :pthread_self()),5,B_DEC,' ');
				len += 5;
				buf[len++] = ' ';
				tostring(&buf[len], (ulong_t)(dd ? dd->cpu :(GET_CPU()+1)),2,B_DEC,'0');
				len += 2;
				buf[len++] = ' ';
				//tostring64(&buf[len], (uint64_t)(dd ? dd->ts :MYCLOCKCYCLES()),18,B_HEX,'0');
				tostring(&buf[len],(uint64_t)timestamp,18,B_HEX,'0');
				len += 18;
				buf[len++] = '\n';
				buf[len] = '\0';
				WRITEOUT(__malloc_trace_fd,buf,len);
			}
			buf[0] = '\0';
		} else {  /* justline */
			buf[0] = '@'; buf[1] = ' '; buf[2] = '[';
			len = 3;
			tostring(buf+len,(ulong_t)(line),10,B_HEX,'0');
			len+=10;
			buf[len++] = ']';
			buf[len++] = '\n';
			buf[len] = '\0';
			WRITEOUT(__malloc_trace_fd,buf,len);
			buf[0] = '\0';
		}
	} /* instruction pointer */
}


static void
mtrace_where(const char *file, int line, void *cptr)
{
	Flink     * flink=NULL; 
	DebugInfo_t   *dbgptr=NULL; 

	/*
	 * Change this to trace back the stack __malloc_bt_depth # of frames
	 * Report as <file>:<symbol>+offset where it is gotten from dladdr
	 */
	int in_malloc_code = in_malloc();
	if (cptr) {
			/* this should be fine, since the allocator returns
				values that are aligned */ 
			flink = (Flink *)((Dhead *)cptr - 1);
			dbgptr = &(((Dhead *)flink)->d_debug);
	} else {
		dbgptr = NULL;
	}
	set_in_malloc(++in_malloc_code); /* Avoid problems with str* functions */
	// dump the trace
	if (file != NULL || line != 0) {
		if (__malloc_use_dladdr > 0 && dbgptr) {
			mtrace_backtrace_dbg(line, dbgptr, "BT:");
		} else {
			mtrace_line(file,line, dbgptr, 0, 0, NULL);
		}
	}

	set_in_malloc(--in_malloc_code);
}

static void
mtrace_where_free(const char *file, int line, void *cptr, unsigned int *__bt[])
{
	int in_malloc_code = in_malloc();
	set_in_malloc(++in_malloc_code); /* Avoid problems with str* functions */
	// dump the trace
	if (file != NULL || line != 0) {
		if (__malloc_use_dladdr > 0 && __bt) {
			mtrace_backtrace_bt(line, __bt, "BT:");
		} else {
			mtrace_line(file,line, NULL, 0, 1, __bt);
		}
	}
	set_in_malloc(--in_malloc_code);
}

static void *
mtrace_malloc(const char *file, int line, size_t size)
{
	void *cptr;
	Dhead *dh;
	int len;
	int in_malloc_code = in_malloc();
	set_in_malloc(++in_malloc_code);

	ENTER();
	__malloc_hook = orig_malloc_hook;
	cptr = (*__malloc_hook)(file,line,size);
	__malloc_hook = mtrace_malloc;

	if (cptr != NULL && MALLOC_TRACE_FCHECK(size)) {
		if (!__check_ctid()) {
			uint64_t timestamp = _get_timestamp();

			MALLOC_FILLCALLERD(cptr, __cdbt, line);

			start_event();

			buf[0] = '+'; buf[1] = ' ';
			len = 2;
			(void) tostring(&buf[len], (ulong_t)(cptr),10,B_HEX,'0');
			len += 10;
			buf[len++] = ' ';

			(void) tostring(&buf[len], (ulong_t)(size),10,B_DEC,' ');
			len += 10;
			buf[len++] = ' ';

			(void) tostring(&buf[len], (ulong_t)(_msize(cptr)),10,B_DEC,' ');
			len += 10;
			buf[len++] = ' ';

			(void) tostring(&buf[len], (ulong_t)(getpid()),10,B_DEC,' ');
			len += 10;
			buf[len++] = ' ';

			(void) tostring(&buf[len], (ulong_t)(pthread_self()),5,B_DEC,' ');
			len += 5;
			buf[len++] = ' ';


			tostring(&buf[len], (ulong_t)(GET_CPU()+1),2,B_DEC,'0');
			len += 2;
			buf[len++] = ' ';

			tostring64(&buf[len],(uint64_t)timestamp,18,B_HEX,'0');
			len += 18;

			buf[len++] = '\n';
			buf[len] = '\0';
			WRITEOUT(__malloc_trace_fd,buf,len);
			buf[0] = '\0';
			mtrace_where(file,line, cptr);
			end_event();
		}
	}
	LEAVE();

	set_in_malloc(--in_malloc_code);

	//sprintf(buf, "+ %x %x\n", (unsigned int)cptr, size);

	if (cptr == mallwatch) {
		tr_break();
	}

	dh = (Dhead *)cptr-1;
	cptr = NULL;
	return (dh+1);
}

static void *
mtrace_realloc(const char *file, int line, void *cptr, size_t size)
{
	int len;
	void *ptr;
	Dhead *dh;
	int in_malloc_code = in_malloc();
	set_in_malloc(++in_malloc_code);

	ENTER();
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

	if (ptr != NULL && MALLOC_TRACE_FCHECK(size)) {
		if (!__check_ctid()) {
			uint64_t timestamp = _get_timestamp();

			MALLOC_FILLCALLERD(ptr, __cdbt, line);

			start_event();
			if (cptr == NULL) {
				buf[0] = '+'; buf[1] = ' ';
				len = 2;
				(void) tostring(&buf[len], (ulong_t)(ptr),10,B_HEX,'0');
				len += 10;
				buf[len++] = ' ';

				(void) tostring(buf+len, (ulong_t)(size),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';

				(void) tostring(buf+len, (ulong_t)(_msize(ptr)),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';

				(void) tostring(&buf[len], (ulong_t)(getpid()),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';

				(void) tostring(&buf[len], (ulong_t)(pthread_self()),5,B_DEC,' ');
				len += 5;
				buf[len++] = ' ';

				tostring(&buf[len], (ulong_t)(GET_CPU()+1),2,B_DEC,'0');
				len += 2;
				buf[len++] = ' ';

				tostring64(&buf[len],(uint64_t)timestamp,18,B_HEX,'0');
				len += 18;

				buf[len++] = '\n';
				buf[len] = '\0';
				WRITEOUT(__malloc_trace_fd,buf,len);
				buf[0] = '\0';
			} else {
				buf[0] = '<'; buf[1] = ' ';
				len = 2;
				(void) tostring(buf+len, (ulong_t)(cptr),10,B_HEX,'0');
				len += 10;

				(void) tostring(buf+len, (ulong_t)(size),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';

				// the pointer could have been free()d.
				//(void) tostring(buf+len, (ulong_t)(_msize(cptr)),10,B_DEC,' ');
				(void) tostring(buf+len, (ulong_t)(_msize(ptr)),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';

				(void) tostring(&buf[len], (ulong_t)(getpid()),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';

				(void) tostring(&buf[len], (ulong_t)(pthread_self()),5,B_DEC,' ');
				len += 5;
				buf[len++] = ' ';

				tostring(&buf[len], (ulong_t)(GET_CPU()+1),2,B_DEC,'0');
				len += 2;
				buf[len++] = ' ';

				tostring64(&buf[len],(uint64_t)timestamp,18,B_HEX,'0');
				len += 18;
				buf[len++] = '\n';
				buf[len] = '\0';
				WRITEOUT(__malloc_trace_fd,buf,len);


				// Do not need it.
				//mtrace_where(file,line, cptr);

				end_event();

				buf[0] = '\0';
				buf[0] = '>'; buf[1] = ' ';
				len = 2;
				(void) tostring(buf+len, (ulong_t)(ptr),10,B_HEX,'0');
				len += 10;
				buf[len++] = ' ';
				(void) tostring(buf+len, (ulong_t)(size),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';

				(void) tostring(buf+len, (ulong_t)(_msize(ptr)),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';

				(void) tostring(&buf[len], (ulong_t)(getpid()),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';
				(void) tostring(&buf[len], (ulong_t)(pthread_self()),5,B_DEC,' ');
				len += 5;
				buf[len++] = ' ';

				tostring(&buf[len], (ulong_t)(GET_CPU()+1),2,B_DEC,'0');
				len += 2;
				buf[len++] = ' ';

				tostring64(&buf[len],(uint64_t)timestamp,18,B_HEX,'0');
				len += 18;
				buf[len++] = '\n';
				buf[len] = '\0';
				WRITEOUT(__malloc_trace_fd,buf,len);
			}
			buf[0] = '\0';
			mtrace_where(file,line, ptr);
			end_event();
		}
	}
	LEAVE();
	set_in_malloc(--in_malloc_code);

	if (cptr == mallwatch) tr_break();

	dh = (Dhead *)ptr-1;
	ptr = NULL;
	return (dh+1);
}

static void *
mtrace_calloc(const char *file, int line, size_t nelems, size_t size)
{
	int len;
	void *ptr;
	Dhead *dh;
	int in_malloc_code = in_malloc();
	set_in_malloc(++in_malloc_code);

	ENTER();
	/* Copy the malloc hook, because calloc could it */
	__malloc_hook = orig_malloc_hook;
	__calloc_hook = orig_calloc_hook;
	ptr = (*__calloc_hook)(file,line,nelems,size);
	__malloc_hook = mtrace_malloc;
	__calloc_hook = mtrace_calloc;

	if (ptr != NULL && MALLOC_TRACE_FCHECK(nelems*size)) {
		if (!__check_ctid()) {
			uint64_t timestamp = _get_timestamp();

			MALLOC_FILLCALLERD(ptr, __cdbt, line);

			start_event();
#if 0
			sprintf(buf, "+ %x %x\n", (unsigned int)ptr, nelems * size);
#endif
			buf[0] = '+'; buf[1] = ' ';
			len = 2;
			(void) tostring(&buf[len], (ulong_t)(ptr),10,B_HEX,'0');
			len += 10;
			buf[len++] = ' ';

			(void) tostring(&buf[len], (ulong_t)(nelems*size),10,B_DEC,' ');
			len += 10;
			buf[len++] = ' ';

			(void) tostring(&buf[len], (ulong_t)(_msize(ptr)),10,B_DEC,' ');
			len += 10;
			buf[len++] = ' ';

			(void) tostring(&buf[len], (ulong_t)(getpid()),10,B_DEC,' ');
			len += 10;
			buf[len++] = ' ';
			(void) tostring(&buf[len], (ulong_t)(pthread_self()),5,B_DEC,' ');
			len += 5;
			buf[len++] = ' ';

			tostring(&buf[len], (ulong_t)(GET_CPU()+1),2,B_DEC,'0');
			len += 2;
			buf[len++] = ' ';

			tostring64(&buf[len],(uint64_t)timestamp,18,B_HEX,'0');
			len += 18;
			buf[len++] = '\n';
			buf[len] = '\0';
			WRITEOUT(__malloc_trace_fd,buf,len);
			buf[0] = '\0';
			mtrace_where(file,line, ptr);
			end_event();
		}
	}
	LEAVE();

	set_in_malloc(--in_malloc_code);

	if (ptr == mallwatch) tr_break();

	dh = (Dhead *)ptr-1;
	ptr = NULL;
	return (dh+1);
}

static void
mtrace_free(const char *file, int line, void *cptr)
{
	int len;
	int usize=0;
	int msize=0;
	int in_malloc_code = in_malloc();

	set_in_malloc(++in_malloc_code);
	ENTER();
	if (!__check_ctid()) {
		uint64_t timestamp = _get_timestamp();

		if (cptr != NULL && (!malloc_verify_alloc || find_malloc_area(cptr, NULL))) {
			usize = _musize(cptr);
			msize = _msize(cptr);
			if (MALLOC_TRACE_FCHECK(usize)) {

				start_event();

				buf[0] = '-'; buf[1] = ' ';
				len = 2;
				(void) tostring(buf+len, (ulong_t)(cptr),10,B_HEX,'0');
				len += 10;
				buf[len++] = ' ';

				(void) tostring(&buf[len], (ulong_t)(usize),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';

				(void) tostring(buf+len, (ulong_t)(msize),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';

				(void) tostring(&buf[len], (ulong_t)(getpid()),10,B_DEC,' ');
				len += 10;
				buf[len++] = ' ';
				(void) tostring(&buf[len], (ulong_t)(pthread_self()),5,B_DEC,' ');
				len += 5;
				buf[len++] = ' ';

				tostring(&buf[len], (ulong_t)(GET_CPU()+1),2,B_DEC,'0');
				len += 2;
				buf[len++] = ' ';

				tostring64(&buf[len],(uint64_t)timestamp,18,B_HEX,'0');
				len += 18;
				buf[len++] = '\n';
				buf[len] = '\0';
				WRITEOUT(__malloc_trace_fd,buf,len);
				buf[0] = '\0';
				mtrace_where_free(file,line, cptr, __cdbt);
				end_event();
			}
		}
	}
	set_in_malloc(--in_malloc_code);

	if (cptr == mallwatch)  {
		tr_break();
	}

	/* Copy the malloc hook, because calloc could use/change it */
	__free_hook = orig_free_hook;
	(*__free_hook)(file,line,cptr);
	__free_hook = mtrace_free;
	LEAVE();
}

void
muntrace()
{ 
    ENTER();
    if (__malloc_trace_fd >= 0) {
		char strbuff[1024];
		char *s;
		char *t;
		s = strbuff;
		t = END_STR;
		COPY(s,t,strbuff,1024);
		*s++ = ' ';
		s += tostring(s, (int)getpid(),10,B_DEC,' ');
		*s++ = '\n';
		*s++ = '\0';
		WRITEOUT(__malloc_trace_fd,strbuff, strlen(strbuff));
		if (__malloc_trace_fd > 2) {
			close(__malloc_trace_fd);
		}
		__malloc_trace_fd=-1;
		fd_set_tracefile = 0;
		__malloc_hook = orig_malloc_hook;
		__calloc_hook = orig_calloc_hook; 
		__realloc_hook = orig_realloc_hook;
		__free_hook = orig_free_hook;

		/* do not erase the filename, the user can call mtrace() again expecting
		 * to resume the trace with the same file.  */
//		__malloc_trace_filename[0] = '\0';
    }
    LEAVE();
}

void
mtrace()
{ 
	int fd=-1;
	ENTER();
	if (__malloc_trace_filename[0] != '\0') {

		/* close the previous fd */
		if (fd_set_tracefile) {
			if (__malloc_trace_fd != -1) {
				close(__malloc_trace_fd);
			}
		}
		__malloc_trace_fd = -1;
		fd_set_tracefile = 0;

		/* open the file */
		if (strcmp(__malloc_trace_filename, "STDOUT") == 0) {
			fd = 1;
		} else if (strcmp(__malloc_trace_filename, "STDERR") == 0) {
			fd = 2;
		} else {
			fd = _malloc_open_sc(__malloc_trace_filename,O_CREAT|O_APPEND|O_WRONLY,0666);
			fd_set_tracefile = 1;
		}

		if (fd == -1) {
			char *s;
			static char *errmsg = "Unable to open malloc trace file: ";
			WRITEOUT(2, errmsg, strlen(errmsg));
			for (s =__malloc_trace_filename; *s; s++) {
		    	/* do nothing */;
			}
			WRITEOUT(2,__malloc_trace_filename, (unsigned)(s-__malloc_trace_filename));
			WRITEOUT(2,"\n",(unsigned)1);
		}
		__malloc_trace_fd = fd;

		/* save the original hooks */
		if (__malloc_trace_fd != -1) {
			char strbuff[1024];
			char *s;
			char *t;
			if (__malloc_hook != mtrace_malloc) {
				orig_malloc_hook = __malloc_hook;
				__malloc_hook = mtrace_malloc;
			}
			if (__calloc_hook != mtrace_calloc) {
				orig_calloc_hook = __calloc_hook;
				__calloc_hook = mtrace_calloc;
			}
			if (__realloc_hook != mtrace_realloc) {
				orig_realloc_hook = __realloc_hook;
				__realloc_hook = mtrace_realloc;
			}
			if (__free_hook != mtrace_free) {
				orig_free_hook = __free_hook;
				__free_hook = mtrace_free;
			}
			s = strbuff;
			t = START_STR;
			COPY(s,t,strbuff,1024);
			*s++ = ' ';
			s += tostring(s, (int)getpid(),10,B_DEC,' ');
			*s++ = '\n';
			*s++ = '\0';
			WRITEOUT(__malloc_trace_fd, strbuff, strlen(strbuff));
			if (! set_atexit) {
				set_atexit = 1;
				atexit(muntrace);
			}
	    }
	}
    LEAVE();
}
