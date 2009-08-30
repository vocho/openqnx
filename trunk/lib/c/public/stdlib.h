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



/*
 *  stdlib.h    Standard Library functions
 *

 */
#ifndef _STDLIB_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
 #include <sys/platform.h>
#endif

#ifndef _MALLOC_H_INCLUDED
 #include <malloc.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _STDLIB_H_INCLUDED
#endif

#ifndef _STDLIB_H_DECLARED
#define _STDLIB_H_DECLARED

/*
 * UNIX98 needs the exit status macros from <sys/wait.h>
 */
#if defined(__EXT_XOPEN_EX) && !defined(__WAIT_H_INCLUDED)
 #include <sys/wait.h>
#endif

#include <_pack64.h>

_C_STD_BEGIN

#if defined(__EXT_ANSIC_199012)

#if defined(__SIZE_T)
typedef __SIZE_T	size_t;
#undef __SIZE_T
#endif

#if defined(__WCHAR_T)
typedef __WCHAR_T	wchar_t;
#undef __WCHAR_T
#endif

#ifndef NULL
 #define NULL   _NULL
#endif


#define RAND_MAX        32767u
#define EXIT_SUCCESS    0
#define EXIT_FAILURE    1
#define MB_CUR_MAX		_CSTD _Mbcurmax

typedef struct  {
        int     quot;
        int     rem;
} div_t;

typedef struct  {
        long    quot;
        long    rem;
} ldiv_t;

typedef struct  {
        _Longlong    quot;
        _Longlong    rem;
} lldiv_t;

typedef lldiv_t _Lldiv_t;

#endif
_C_STD_END

#include <_packpop.h>

__BEGIN_DECLS

_C_STD_BEGIN

extern char _Mbcurmax;

/* library private functions */
#define _STOINT_SIGNED		0x1
#define _STOINT_LLONG		0x2
extern _ULonglong _Stoint(const char *, char **, int, int);
extern float _Stof(const char *, char **, long);
extern double _Stod(const char *, char **, long);
extern long double _Stold(const char *, char **, long);

extern _Longlong _Stoll(const char *, char **, int);
extern _ULonglong _Stoull(const char *, char **, int);

typedef int _Cmpfun(const void *, const void *);

#if defined(__EXT_ANSIC_199012)
extern void      abort( void ) __attribute__((__noreturn__));
extern int       abs( int __j );
extern int       atexit( register void ( *__func )( void ) );
#if __GNUC__ >= 4
extern void      *__dso_handle __attribute__((__visibility__("hidden")));
#else
extern void      *__dso_handle;
#endif
extern int       __cxa_atexit( void ( *__func )( void * ), void *, void * );
extern double    atof( const char *__nptr );
extern int       atoi( const char *__nptr );
extern long int  atol( const char *__nptr );
extern void     *bsearch( const void *__key, const void *__base,
                    size_t __nmemb, size_t __size,
                    int ( *__compar )( const void *__pkey, const void *__pbase ) );
extern div_t     div( int __numer, int __denom );
extern void      exit( int __status ) __attribute__((__noreturn__));
extern char     *getenv( const char *__name );
extern long int  labs( long int __j );
extern ldiv_t    ldiv( long int __numer, long int __denom );
extern int       mblen( const char *__s, size_t __n );
extern size_t    mbstowcs( wchar_t *__pwcs, const char *__s, size_t __n );
extern int       mbtowc( wchar_t *__pwc, const char *__s, size_t __n );
extern void      qsort( void *__base, size_t __nmemb, size_t __size,
                    int (*__compar)( const void *, const void * ) );
extern int       rand( void );
extern void      srand( unsigned int __seed );
extern double      strtod( const char *__nptr, char **__endptr );
extern float       strtof( const char *__nptr, char **__endptr );
extern long double strtold( const char *__nptr, char **__endptr );
extern long int  strtol( const char *__nptr, char **__endptr, int __base );
extern unsigned long int     strtoul( const char *__nptr, char **__endptr, int __base );
extern size_t    wcstombs( char *__s, const wchar_t *__pwcs, size_t __n );
extern int       wctomb( char *__s, wchar_t __wchar );
extern int system(const char *__cmd);
#endif
_C_STD_END

#if defined(__EXT_XOPEN_EX)

#if defined (__NYI)
extern long a64l(const char *__s);
extern char *l64a(long __value);
extern int ttyslot(void);
#endif

extern int getsubopt(char **__optionp, char * const *__tokens, char **__valuep);
extern char *initstate(unsigned int __seed, char *__state, _CSTD size_t __size);
extern long int jrand48(unsigned short int __xsubi[3]);
extern void lcong48(unsigned short int __param[7]);
extern long int mrand48(void);
extern long random(void);
extern char *realpath(const char *__file_name, char *__resolved_name);
extern unsigned short int *seed48(unsigned short int __seed16v[3]);
extern void setkey(const char *__key);
extern char *setstate(const char *__state);
extern void srandom(unsigned int __seed);
extern int posix_openpt(int __oflag);
extern int grantpt(int __fildes);
extern int unlockpt(int __fildes);
extern char *ptsname(int __fildes);
#if defined(__EXT_QNX)
extern char *ptsname_r(int __fildes, char *__buffer, _CSTD size_t __buflen);
#endif
#endif

#if defined(__EXT_POSIX1_199506)
extern int rand_r(unsigned int *__seed);
#endif

#if defined(__EXT_UNIX_MISC) || defined(__EXT_POSIX1_200112)
extern int setenv(const char *__name, const char *__value, int __rewrite);
extern int unsetenv(const char *__name);
#endif
#if defined(__EXT_UNIX_MISC)
extern int daemon(int __nochdir, int __noclose);
#endif

#if defined(__EXT_XOPEN_EX)
extern double drand48(void);
extern double  erand48(unsigned short int x[3]);
extern long int lrand48(void);
extern long int nrand48(unsigned short int x[3]);
extern void    srand48(long seed);

extern int     mkstemp(char *__template);
extern char *  mktemp(char *__template);

/*
-- Not currently supported --

extern char     *ecvt( double __val, int __ndig, int *__dec, int *__sign );
extern char     *fcvt( double __val, int __ndig, int *__dec, int *__sign );
extern char     *gcvt( double __val, int __ndig, char *__buf );
*/

extern int       putenv(char *__string);
#endif

#if defined(__EXT_ANSIC_199901) || defined(__EXT_POSIX1_200112)
extern void _Exit(int __status);
extern _Longlong	 llabs(_Longlong __j);
extern _CSTD lldiv_t lldiv(_Longlong __numer, _Longlong _denom);
#endif
#if defined(__EXT_QNX) || defined(__EXT_ANSIC_199901) || defined(__EXT_POSIX1_200112)
extern _Longlong strtoll( const char *__nptr, char **__endptr, int __base );
extern _ULonglong strtoull( const char *__nptr, char **__endptr, int __base );
extern _Longlong  atoll( const char *__nptr );
#endif
#if defined(__EXT_QNX)
/* long long operations */
extern char     *lltoa( _Longlong __value, char *__buf, int __radix );
extern char     *ulltoa( _ULonglong __value, char *__buf, int __radix );

int clearenv(void);

/* min and max macros */
#if !defined(__max)
#define __max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#if !defined(max) && !defined(__cplusplus)
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif
#if !defined(__min)
#define __min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#if !defined(min) && !defined(__cplusplus)
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

#endif

#if defined(__EXT_PCDOS)

#if defined(__WATCOMC__) && defined(__INLINE_FUNCTIONS__)
 #pragma intrinsic(abs,div,labs)
 #ifdef __386__
  #pragma intrinsic(ldiv)
 #endif
#endif

#ifndef __cplusplus
#define atof(p)  strtod(p,(char **)NULL)
#endif

extern unsigned  atoh( const char *__nptr );

extern char     *itoa( int __value, char *__buf, int __radix );
extern char     *ltoa( long int __value, char *__buf, int __radix );

extern char     *_fullpath( char *__buf, const char *__path, _CSTD size_t __size );
extern void      searchenv( const char *__name, const char *__env_var, char *__buf );
extern char     *ultoa( unsigned long int __value, char *__buf, int __radix );
extern char     *utoa( unsigned int __value, char *__buf, int __radix );


#if defined(__SLIB_DATA_INDIRECT) && !defined(_amblksiz) && !defined(__SLIB)
  extern unsigned *__get_amblksiz_ptr(void);
  #define _amblksiz *(__get_amblksiz_ptr())
#else
  extern unsigned _amblksiz;  /*  mallocs done in multiples of    */ 
#endif


#endif

__END_DECLS

#endif

#ifdef __cplusplus
#ifdef _MATH_H_INCLUDED
inline int abs( int __j ) { return _CSTD abs(__j); };
#endif
#endif

#ifdef _STD_USING
#ifndef __GCC_BUILTIN
using std::abort; using std::abs;
using std::exit; using std::labs;
#endif
using std::size_t; using std::div_t; using std::ldiv_t;
using std::atexit; using std::__cxa_atexit; using std::getenv;
using std::atof; using std::atoi; using std::atol;
using std::bsearch; using std::div; using std::ldiv;
using std::mblen; using std::mbstowcs; using std::mbtowc;
using std::qsort; using std::rand;
using std::srand; using std::strtod; using std::strtol;
using std::strtoul; using std::system;
using std::wcstombs; using std::wctomb;
#endif /* _STD_USING */

#endif

/* __SRCVERSION("stdlib.h $Rev: 171835 $"); */
