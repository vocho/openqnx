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



#ifndef _stdutil_h
#define _stdutil_h

#ifndef _STDIO_H_INCLUDED
#include <stdio.h>
#endif
#ifndef __STAT_H_INCLUDED
#include <sys/stat.h>
#endif
#ifndef __TYPES_H_INCLUDED
#include <sys/types.h>
#endif

#define str(x)        #x
#define xstr(x)       str(x)


#ifndef MAX_TEMPFILE
#define MAX_TEMPFILE      256
#endif

#define PSTRICT_ENVAR "POSIX_STRICT"
#define POSIX_STRICT (getenv(PSTRICT_ENVAR)!=NULL)


#ifdef __cplusplus
extern "C" {
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


#if 0
#define unlink(x)    xunlink(x, __FILE__  xstr(__LINE__))
#define calloc(x,y)  xcalloc(x,y,__FILE__  xstr(__LINE__))
#define malloc(x)    xmalloc(x, __FILE__  xstr(__LINE__))
#define realloc(x,y) xrealloc(x, y, __FILE__  xstr(__LINE__))
#define strdup(x)    xstrdup(x, __FILE__ xstr(__LINE__))
#endif

/*- Error reporting */
extern void fatalerr(char *fmt, ...);
/* extern void error(char *fmt, ...); */
extern void warning(char *fmt, ...);
extern void prerror(const char *,... );
extern void aberror(const char *,... );
extern void abprt(const char *,... );

/*- misc string functions */
extern int   breakstr(char *buf, const char *sep, char **blist, int n);
extern char *purty(char *string );
#if !defined(__CYGWIN32__) && !defined(__MINGW32__)
extern char *strsignal(int sig);
#endif
extern int     set_escape_string(char *match, char *xlat);
extern int     stresc(char *s, char **update);
extern int     strnbcmp( char *l, char *m );
extern char    *strconcat( char *dest, ... );
extern int     streqany( char *dest, ... );
extern int     strneqany( char *base, char *src, ... );
extern int     streqlist( char *base, char **list );
extern int     strneqlist( char *base, char **list, int n );
/*macro extern int     strequal( char *base, char *cmp );*/
/*macro extern int     strnequal( char *base, char *cmp, int n );*/
extern char    *strpasswhite( char *src );
extern char    *strpretrim( char *src );
extern char    *strsqz( char *str, char c );
extern char    *strtrim( char *src );

#ifndef strequal
#define strequal(a,b)        (!strcmp((a),(b)))
#define strnequal(a,b,n)    (!strncmp((a),(b),(n)))
#endif
#define iswhite(a) ((unsigned)a <= ' ')

/* missing os bits */

extern int saccess( struct stat *, int );
extern pid_t getspid(int sid);
extern pid_t getsid(pid_t pid);    
#define __oldgetsid()    getsid(getpid())
extern int    sid_name(int  sid, char *name);
extern char    *get_disk_name( char *, char * );
extern char    *get_rawdisk( char *, char * );
extern char    *get_partn( char *, char * );
#ifdef NEVER
extern int mount(char *, char *, int);
#endif
extern int fsys_mount_partition(char *, unsigned, long, long);
extern int fsys_mount_ext_part(char *, unsigned, unsigned, long, long);
#define strtonid qnx_strtonid
/* extern char    *nidtostr(nid_t nid, char *buf, int maxbuf); */

#ifdef __cplusplus
};
#endif
#endif
