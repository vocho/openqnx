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





#if defined(__STDC__) || defined(__cplusplus)
# define __stdcargs(s) s
#else
# define __stdcargs(s) ()
#endif

#ifndef malloc_lib_h
#error 'malloc_g/malloc.h' should be included first
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

_C_STD_BEGIN

#if defined(__SIZE_T)
typedef __SIZE_T       size_t;
#undef __SIZE_T
#endif

#if defined(__SSIZE_T)
typedef __SSIZE_T       ssize_t;
#undef __SSIZE_T
#endif

_C_STD_END

__BEGIN_DECLS

/* malloc.c */
void *malloc_pc __stdcargs((_CSTD size_t size, unsigned int *pc));
void *debug_malloc __stdcargs((const char *file, int line, _CSTD size_t size));
void malloc_err_handler __stdcargs((int level));
void *_memalign_pc __stdcargs((_CSTD size_t alignment, _CSTD size_t size, unsigned int *pc));
/* free.c */
void debug_free __stdcargs((const char *file, int line, void *cptr));
void DBFfree __stdcargs((const char *func, const char *file, int line, void *cptr));
/* realloc.c */
void *debug_realloc __stdcargs((const char *file, int line, void *cptr, _CSTD size_t size));
/* calloc.c */
void *debug_calloc __stdcargs((const char *file, int line, _CSTD size_t nelem, _CSTD size_t elsize));
void debug_cfree __stdcargs((const char *file, int line, void *cptr));
/* string.c */
char *DBstrcat __stdcargs((const char *file, int line, register char *str1, register const char *str2));
char *DBstrdup __stdcargs((const char *file, int line, register const char *str1));
char *DBstrncat __stdcargs((const char *file, int line, register char *str1, register const char *str2, register _CSTD size_t len));
int DBstrcmp __stdcargs((const char *file, int line, register const char *str1, register const char *str2));
int DBstrncmp __stdcargs((const char *file, int line, register const char *str1, register const char *str2, register _CSTD size_t len));
char *DBstrcpy __stdcargs((const char *file, int line, register char *str1, register const char *str2));
char * DBstrncpy(const char *file, int line, register char *str1, register const char *str2, register _CSTD size_t len);
_CSTD size_t DBstrlen __stdcargs((const char *file, int line, register const char *str1));
char *DBstrchr __stdcargs((const char *file, int line, const char *str1, int c));
char *DBFstrchr __stdcargs((const char *func, const char *file, int line, register const char *str1, register int c));
char *DBstrrchr __stdcargs((const char *file, int line, const char *str1, int c));
char *DBFstrrchr __stdcargs((const char *func, const char *file, int line, register const char *str1, register int c));
char *DBindex __stdcargs((const char *file, int line, const char *str1, int c));
char *DBrindex __stdcargs((const char *file, int line, const char *str1, int c));
char *DBstrpbrk __stdcargs((const char *file, int line, register const char *str1, register const char *str2));
_CSTD size_t DBstrspn __stdcargs((const char *file, int line, register const char *str1, register const char *str2));
_CSTD size_t DBstrcspn __stdcargs((const char *file, int line, register const char *str1, register const char *str2));
char *DBstrstr __stdcargs((const char *file, int line, const char *str1, const char *str2));
char *DBstrtok __stdcargs((const char *file, int line, char *str1, const char *str2));
char *strtoken __stdcargs((register char **stringp, register const char *delim, int skip));
/* malloc_chk.c */
void malloc_check_str __stdcargs((const char *func, const char *file, int line, const char *str));
void malloc_check_strn __stdcargs((const char *func, const char *file, int line, const char *str, int len));
void malloc_check_data __stdcargs((const char *func, const char *file, int line, const void *ptr, _CSTD size_t len));
int malloc_check_guard __stdcargs((const char *func, const char *file, int line, Dhead *dh));
_CSTD ssize_t _msize __stdcargs((void *cptr));
_CSTD ssize_t _musize __stdcargs((void *cptr));
char *_mptr __stdcargs((const char *cptr));
void *find_malloc_ptr __stdcargs((const void *ptr, arena_range_t *range));
/* malloc_chn.c */
int malloc_chain_check __stdcargs((int todo));
int DBFmalloc_chain_check __stdcargs((const char *func, const char *file, int line, int todo));
/* memory.c */
void *DBmemccpy __stdcargs((const char *file, int line, void *ptr1, const void *ptr2, int ch, _CSTD size_t len));
void *DBmemchr __stdcargs((const char *file, int line, const void *ptr1, register int ch, _CSTD size_t len));
void *DBmemmove __stdcargs((const char *file, int line, void *ptr1, const void *ptr2, register _CSTD size_t len));
void *DBmemcpy __stdcargs((const char *file, int line, void *ptr1, const void *ptr2, register _CSTD size_t len));
void *DBFmemcpy __stdcargs((const char *func, const char *file, int line, void *ptr1, const void *ptr2, register _CSTD size_t len));
int DBmemcmp __stdcargs((const char *file, int line, const void *ptr1, const void *ptr2, register _CSTD size_t len));
int DBFmemcmp __stdcargs((const char *func, const char *file, int line, const void *ptr1, const void *ptr2, register _CSTD size_t len));
void *DBmemset __stdcargs((const char *file, int line, void *ptr1, register int ch, register _CSTD size_t len));
void *DBFmemset __stdcargs((const char *func, const char *file, int line, void *ptr1, register int ch, register _CSTD size_t len));
void *DBbcopy __stdcargs((const char *file, int line, const void *ptr2, void *ptr1, _CSTD size_t len));
void *DBbzero __stdcargs((const char *file, int line, void *ptr1, _CSTD size_t len));
int DBbcmp __stdcargs((const char *file, int line, const void *ptr2, const void *ptr1, _CSTD size_t len));
/* dump.c */
#ifndef malloc_dump	/* inconsistent definition between libmalloc and libmalloc_g */
void malloc_dump __stdcargs((int fd));
#endif
/* leak.c */
int malloc_mark __stdcargs((int fd));
int malloc_dump_unreferenced __stdcargs((int fd, int detail));
/* crc.c */
long DB_compute_crc_32 __stdcargs(( int n, char *buf ));
/* malloc_g.c */
void set_guard __stdcargs((void *ptr, _CSTD size_t usize));
/* mtrace.c */
void mtrace();
void muntrace();

__END_DECLS

#undef __stdcargs
