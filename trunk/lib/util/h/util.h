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




#ifndef util_h
#define util_h

#ifndef _STDIO_H_INCLUDED
#include <stdio.h>
#endif


#define str(x)        #x
#define xstr(x)       str(x)


#ifndef MAX_TEMPFILE
#define MAX_TEMPFILE      256
#endif


extern char *program_name;          /* for error reporting */

extern char *temp(const char *prefix, const char *suffix);
extern int   untemp(const char *fname, int no_unlink);

/*- IO functions  -- noisy versions */
extern int fcat(FILE *dest , FILE *src);
extern FILE *xfopen(const char *nam , const char *mode , const char *func);
extern void *xcalloc(int nelem , int elsize , const char *func);
extern void *xmalloc(int nelem , const char *func);
extern void *xrealloc(void *p , int msize , const char *func);
extern char *xstrdup(const char *src, const char *func);
extern int  xunlink(const char *name, const char *func);

#define unlink(x)    xunlink(x, __FILE__  xstr(__LINE__))
#define calloc(x,y)  xcalloc(x,y,__FILE__  xstr(__LINE__))
#define malloc(x)    xmalloc(x, __FILE__  xstr(__LINE__))
#define realloc(x,y) xrealloc(x, y, __FILE__  xstr(__LINE__))
#define strdup(x)    xstrdup(x, __FILE__ xstr(__LINE__))

/*- Error reporting */
extern void fatalerr(char *fmt, ...);
extern void error(char *fmt, ...);
extern void warning(char *fmt, ...);

/*- string functions */
extern int   breakstr(char *buf, const char *sep, char **blist, int n);

#endif
