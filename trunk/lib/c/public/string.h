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
 *  string.h    String functions
 *

 *
 * Copyright (c) 1994-2000 by P.J. Plauger.  ALL RIGHTS RESERVED. 
 * Consult your license regarding permissions and restrictions.
 *
 */
#ifndef _STRING_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if defined(__EXT_UNIX_MISC) && !defined(_STRINGS_H_INCLUDED)
#include <strings.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _STRING_H_INCLUDED
#endif

#ifndef _STRING_H_DECLARED
#define _STRING_H_DECLARED

_C_STD_BEGIN

#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif

#ifndef NULL
#define NULL   _NULL
#endif

_C_STD_END

#ifdef __cplusplus
 #ifndef _Const_return
  #define _Const_return const
 #endif
#else
 #define _Const_return
#endif

__BEGIN_DECLS

_C_STD_BEGIN
extern _Const_return void *memchr( const void *__s, int __c, size_t __n );
extern _Const_return char *strchr( const char *__s, int __c );
extern _Const_return char *strpbrk( const char *__s1, const char *__s2 );
extern _Const_return char *strrchr( const char *__s, int __c );
extern _Const_return char *strstr( const char *__s1, const char *__s2 );

extern int   memcmp( const void *__s1, const void *__s2, size_t __n );
extern void *memcpy( void *__s1, const void *__s2, size_t __n );
extern void *memmove( void *__s1, const void *__s2, size_t __n );
#ifndef __MEMSET_DEFINED
#define __MEMSET_DEFINED
extern void *memset( void *__s, int __c, size_t __n );
#endif
extern char *strcat( char *__s1, const char *__s2 );
extern int strcmp( const char *__s1, const char *__s2 );
extern int strcoll( const char *__s1, const char *__s2 );
extern size_t strxfrm( char *__s1, const char *__s2, size_t __n );
extern char *strcpy( char *__s1, const char *__s2 );
extern size_t strcspn( const char *__s1, const char *__s2 );
extern char *strerror( int __errnum );
extern size_t strlen( const char *__s );
extern char *strncat( char *__s1, const char *__s2, size_t __n );
extern int strncmp( const char *__s1, const char *__s2, size_t __n );
extern char *strncpy( char *__s1, const char *__s2, size_t __n );
extern size_t strspn( const char *__s1, const char *__s2 );
extern char *strtok( char *__s1, const char *__s2 );
_C_STD_END
extern char *strsignal(int __signo);

#if defined(__EXT_POSIX1_199506)
extern char *strtok_r( char *__s1, const char *__s2, char **__s3 );
#endif

#if defined(__EXT_POSIX1_200112)
extern int	strerror_r(int __errnum, char * __buf, _CSTD size_t __len);
#endif

#if defined(__EXT_POSIX1_200112) || defined(__EXT_XOPEN_EX)
extern void *memccpy( void * __restrict __s1, const void * __restrict __s2, int __c, _CSTD size_t __n );
extern char *strdup( const char *__string );
#endif

#if defined(__EXT_QNX)

/* WATCOM's Additional Functions (non-ANSI, non-POSIX) */

struct iovec;
extern _CSTD size_t memcpyv(const struct iovec *__dst, int __dparts, int __doff, const struct iovec *__src, int __sparts, int __soff);
extern int  memicmp( const void *__s1, const void *__s2, _CSTD size_t __n );
extern int  _memicmp( const void *__s1, const void *__s2, _CSTD size_t __n );
extern int   strcmpi( const char *__s1, const char *__s2 );
extern char *_strdup( const char *__string );
extern int   stricmp( const char *__s1, const char *__s2 );
extern int   _stricmp( const char *__s1, const char *__s2 );
extern char *strlwr( char *__string );
extern char *_strlwr( char *__string );
extern int   strnicmp( const char *__s1, const char *__s2, _CSTD size_t __n );
extern int   _strnicmp( const char *__s1, const char *__s2, _CSTD size_t __n );
extern char *strnset( char *__string, int __c, _CSTD size_t __len );
extern char *strrev( char *__string );
extern char *strset( char *__string, int __c );
extern char *strupr( char *__string );
extern char *_strupr( char *__string );

/* QNX's Additional Functions (non-ANSI, non-POSIX) */

extern void __strerror( int __max, int __errnum, char *__buf );
extern int straddstr(const char *__str, int __len, char **__pbuf, _CSTD size_t *__pmaxbuf);
#endif 

#if defined(__EXT_UNIX_MISC)
extern char *strsep(char **__stringp, const char *__delim);
#endif
#if defined(__EXT_BSD)
_CSTD size_t strlcat(char *__s1, const char *__s2, _CSTD size_t __n);
_CSTD size_t strlcpy(char *__s1, const char *__s2, _CSTD size_t __n);
#endif

__END_DECLS
#if defined(__X86__)
#include <x86/string.h>
#endif

#ifdef __cplusplus
extern "C++" {
_C_STD_BEGIN
inline void *memchr(void *_S, int _C, _CSTD size_t _N)
	{	/* call with const first argument */
	union { void *_Out; _Const_return void * _In; } _Result;
	return (_Result._In = _CSTD memchr((const void *)_S, _C, _N)), _Result._Out; }

inline char *strchr(char *_S, int _C)
	{	/* call with const first argument */
	union { char *_Out; _Const_return char * _In; } _Result;
	return (_Result._In = _CSTD strchr((const char *)_S, _C)), _Result._Out; }

inline char *strpbrk(char *_S, const char *_P)
	{	/* call with const first argument */
	union { char *_Out; _Const_return char * _In; } _Result;
	return (_Result._In = _CSTD strpbrk((const char *)_S, _P)), _Result._Out; }

inline char *strrchr(char *_S, int _C)
	{	/* call with const first argument */
	union { char *_Out; _Const_return char * _In; } _Result;
	return (_Result._In = _CSTD strrchr((const char *)_S, _C)), _Result._Out; }

inline char *strstr(char *_S, const char *_P)
	{	/* call with const first argument */
	union { char *_Out; _Const_return char * _In; } _Result;
	return (_Result._In = _CSTD strstr((const char *)_S, _P)), _Result._Out; }
_C_STD_END
};
#endif

#endif

#ifdef _STD_USING
#ifndef __GCC_BUILTIN
using _CSTD memcmp; using _CSTD memcpy;
using _CSTD strcmp; using _CSTD strcpy; using _CSTD strlen;
#endif
using _CSTD size_t; using _CSTD memchr;
using _CSTD memmove; using _CSTD memset;
using _CSTD strcat; using _CSTD strchr; using _CSTD strncat;
using _CSTD strcoll; using _CSTD strcspn; using _CSTD strerror;
using _CSTD strncmp; using _CSTD strncpy; using _CSTD strpbrk;
using _CSTD strrchr; using _CSTD strspn; using _CSTD strstr;
using _CSTD strtok; using _CSTD strxfrm;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("string.h $Rev: 199660 $"); */
