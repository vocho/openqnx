/*
 * This source code may contain confidential information of QNX Software
 * Systems Ltd.  (QSSL) and its licensors. Any use, reproduction,
 * modification, disclosure, distribution or transfer of this software,
 * or any software which includes or is based upon any of this code, is
 * prohibited unless expressly authorized by QSSL by written agreement. For
 * more information (including whether this source code file has been
 * published) please email licensing@qnx.com.
 */

/* uclconf.h -- configuration for the UCL real-time data compression library

   This file is part of the UCL real-time data compression library.

   Copyright (C) 2000 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1999 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1998 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1997 Markus Franz Xaver Johannes Oberhumer
   Copyright (C) 1996 Markus Franz Xaver Johannes Oberhumer

   Markus F.X.J. Oberhumer
   <markus.oberhumer@jk.uni-linz.ac.at>
   http://wildsau.idv.uni-linz.ac.at/mfx/ucl.html
 */


#ifndef __UCLCONF_H
#define __UCLCONF_H

#define UCL_VERSION             0x009100L
#define UCL_VERSION_STRING      "0.91"
#define UCL_VERSION_DATE        "Apr 19 2000"

/* internal Autoconf configuration file - only used when building UCL */
#if defined(UCL_HAVE_CONFIG_H)
#  include <config.h>
#endif
#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif


/***********************************************************************
// UCL requires a conforming <limits.h>
************************************************************************/

#if !defined(CHAR_BIT) || (CHAR_BIT != 8)
#  error "invalid CHAR_BIT"
#endif
#if !defined(UCHAR_MAX) || !defined(UINT_MAX) || !defined(ULONG_MAX)
#  error "check your compiler installation"
#endif
#if (USHRT_MAX < 1 ) || (UINT_MAX < 1) || (ULONG_MAX < 1)
#  error "your limits.h macros are broken"
#endif

/* workaround a cpp bug under hpux 10.20 */
#define UCL_0xffffffffL     4294967295ul


/***********************************************************************
// architecture defines
************************************************************************/

#if !defined(__UCL_WIN) && !defined(__UCL_DOS) && !defined(__UCL_OS2)
#  if defined(__WINDOWS__) || defined(_WINDOWS) || defined(_Windows)
#    define __UCL_WIN
#  elif defined(__WIN32__) || defined(_WIN32) || defined(WIN32)
#    define __UCL_WIN
#  elif defined(__NT__) || defined(__NT_DLL__) || defined(__WINDOWS_386__)
#    define __UCL_WIN
#  elif defined(__DOS__) || defined(__MSDOS__) || defined(MSDOS)
#    define __UCL_DOS
#  elif defined(__OS2__) || defined(__OS2V2__) || defined(OS2)
#    define __UCL_OS2
#  elif defined(__palmos__)
#    define __UCL_PALMOS
#  elif defined(__TOS__) || defined(__atarist__)
#    define __UCL_TOS
#  endif
#endif

#if (UINT_MAX < UCL_0xffffffffL)
#  if defined(__UCL_WIN)
#    define __UCL_WIN16
#  elif defined(__UCL_DOS)
#    define __UCL_DOS16
#  elif defined(__UCL_PALMOS)
#    define __UCL_PALMOS16
#  elif defined(__UCL_TOS)
#    define __UCL_TOS16
#  elif defined(__C166__)
#  else
#    error "16-bit target not supported - contact me for porting hints"
#  endif
#endif

#if !defined(__UCL_i386)
#  if defined(__UCL_DOS) || defined(__UCL_WIN16)
#    define __UCL_i386
#  elif defined(__i386__) || defined(__386__) || defined(_M_IX86)
#    define __UCL_i386
#  endif
#endif

#if defined(__UCL_STRICT_16BIT)
#  if (UINT_MAX < UCL_0xffffffffL)
#    include <ucl/ucl16bit.h>
#  endif
#endif


/***********************************************************************
// integral and pointer types
************************************************************************/

/* Integral types with 32 bits or more */
#if !defined(UCL_UINT32_MAX)
#  if (UINT_MAX >= UCL_0xffffffffL)
     typedef unsigned int       ucl_uint32;
     typedef int                ucl_int32;
#    define UCL_UINT32_MAX      UINT_MAX
#    define UCL_INT32_MAX       INT_MAX
#    define UCL_INT32_MIN       INT_MIN
#  elif (ULONG_MAX >= UCL_0xffffffffL)
     typedef unsigned long      ucl_uint32;
     typedef long               ucl_int32;
#    define UCL_UINT32_MAX      ULONG_MAX
#    define UCL_INT32_MAX       LONG_MAX
#    define UCL_INT32_MIN       LONG_MIN
#  else
#    error "ucl_uint32"
#  endif
#endif

/* ucl_uint is used like size_t */
#if !defined(UCL_UINT_MAX)
#  if (UINT_MAX >= UCL_0xffffffffL)
     typedef unsigned int       ucl_uint;
     typedef int                ucl_int;
#    define UCL_UINT_MAX        UINT_MAX
#    define UCL_INT_MAX         INT_MAX
#    define UCL_INT_MIN         INT_MIN
#  elif (ULONG_MAX >= UCL_0xffffffffL)
     typedef unsigned long      ucl_uint;
     typedef long               ucl_int;
#    define UCL_UINT_MAX        ULONG_MAX
#    define UCL_INT_MAX         LONG_MAX
#    define UCL_INT_MIN         LONG_MIN
#  else
#    error "ucl_uint"
#  endif
#endif


/* Memory model that allows to access memory at offsets of ucl_uint. */
#if !defined(__UCL_MMODEL)
#  if (UCL_UINT_MAX <= UINT_MAX)
#    define __UCL_MMODEL
#  elif defined(__UCL_DOS16) || defined(__UCL_WIN16)
#    define __UCL_MMODEL        __huge
#    define UCL_999_UNSUPPORTED
#  elif defined(__UCL_PALMOS16) || defined(__UCL_TOS16)
#    define __UCL_MMODEL
#  else
#    error "__UCL_MMODEL"
#  endif
#endif

/* no typedef here because of const-pointer issues */
#define ucl_byte                unsigned char __UCL_MMODEL
#define ucl_bytep               unsigned char __UCL_MMODEL *
#define ucl_charp               char __UCL_MMODEL *
#define ucl_voidp               void __UCL_MMODEL *
#define ucl_shortp              short __UCL_MMODEL *
#define ucl_ushortp             unsigned short __UCL_MMODEL *
#define ucl_uint32p             ucl_uint32 __UCL_MMODEL *
#define ucl_int32p              ucl_int32 __UCL_MMODEL *
#define ucl_uintp               ucl_uint __UCL_MMODEL *
#define ucl_intp                ucl_int __UCL_MMODEL *
#define ucl_voidpp              ucl_voidp __UCL_MMODEL *
#define ucl_bytepp              ucl_bytep __UCL_MMODEL *

typedef int ucl_bool;

#ifndef ucl_sizeof_dict_t
#  define ucl_sizeof_dict_t     sizeof(ucl_bytep)
#endif


/***********************************************************************
// function types
************************************************************************/

/* linkage */
#if !defined(__UCL_EXTERN_C)
#  ifdef __cplusplus
#    define __UCL_EXTERN_C      extern "C"
#  else
#    define __UCL_EXTERN_C      extern
#  endif
#endif

/* calling conventions */
#if !defined(__UCL_CDECL)
#  if defined(__UCL_DOS16) || defined(__UCL_WIN16)
#    define __UCL_CDECL         __far __cdecl
#  elif defined(__UCL_i386) && defined(_MSC_VER)
#    define __UCL_CDECL         __cdecl
#  elif defined(__UCL_i386) && defined(__WATCOMC__)
#    define __UCL_CDECL         __near __cdecl
#  else
#    define __UCL_CDECL
#  endif
#endif
#if !defined(__UCL_ENTRY)
#  define __UCL_ENTRY           __UCL_CDECL
#endif

/* DLL export information */
#if !defined(__UCL_EXPORT1)
#  define __UCL_EXPORT1
#endif
#if !defined(__UCL_EXPORT2)
#  define __UCL_EXPORT2
#endif

/* calling convention for C functions */
#if !defined(UCL_PUBLIC)
#  define UCL_PUBLIC(_rettype)  __UCL_EXPORT1 _rettype __UCL_EXPORT2 __UCL_ENTRY
#endif
#if !defined(UCL_EXTERN)
#  define UCL_EXTERN(_rettype)  __UCL_EXTERN_C UCL_PUBLIC(_rettype)
#endif
#if !defined(UCL_PRIVATE)
#  define UCL_PRIVATE(_rettype) static _rettype __UCL_ENTRY
#endif

/* cdecl calling convention for assembler functions */
#if !defined(UCL_PUBLIC_CDECL)
#  define UCL_PUBLIC_CDECL(_rettype) \
                __UCL_EXPORT1 _rettype __UCL_EXPORT2 __UCL_CDECL
#endif
#if !defined(UCL_EXTERN_CDECL)
#  define UCL_EXTERN_CDECL(_rettype)  __UCL_EXTERN_C UCL_PUBLIC_CDECL(_rettype)
#endif


typedef int
(__UCL_ENTRY *ucl_compress_t)   ( const ucl_byte *src, ucl_uint  src_len,
                                        ucl_byte *dst, ucl_uint *dst_len,
                                        ucl_voidp wrkmem );

typedef int
(__UCL_ENTRY *ucl_decompress_t) ( const ucl_byte *src, ucl_uint  src_len,
                                        ucl_byte *dst, ucl_uint *dst_len,
                                        ucl_voidp wrkmem );

typedef int
(__UCL_ENTRY *ucl_optimize_t)   (       ucl_byte *src, ucl_uint  src_len,
                                        ucl_byte *dst, ucl_uint *dst_len,
                                        ucl_voidp wrkmem );

typedef int
(__UCL_ENTRY *ucl_compress_dict_t)(const ucl_byte *src, ucl_uint  src_len,
                                        ucl_byte *dst, ucl_uint *dst_len,
                                        ucl_voidp wrkmem,
                                  const ucl_byte *dict, ucl_uint dict_len );

typedef int
(__UCL_ENTRY *ucl_decompress_dict_t)(const ucl_byte *src, ucl_uint  src_len,
                                        ucl_byte *dst, ucl_uint *dst_len,
                                        ucl_voidp wrkmem,
                                  const ucl_byte *dict, ucl_uint dict_len );


/* a progress indicator callback function */
typedef struct
{
    void (__UCL_ENTRY *callback) (ucl_uint, ucl_uint, int, ucl_voidp user);
    ucl_voidp user;
}
ucl_progress_callback_t;
#define ucl_progress_callback_p ucl_progress_callback_t __UCL_MMODEL *


/***********************************************************************
// error codes and prototypes
************************************************************************/

/* Error codes for the compression/decompression functions. Negative
 * values are errors, positive values will be used for special but
 * normal events.
 */
#define UCL_E_OK                    0
#define UCL_E_ERROR                 (-1)
#define UCL_E_INVALID_ARGUMENT      (-2)
#define UCL_E_OUT_OF_MEMORY         (-3)
/* compression errors */
#define UCL_E_NOT_COMPRESSIBLE      (-101)
/* decompression errors */
#define UCL_E_INPUT_OVERRUN         (-201)
#define UCL_E_OUTPUT_OVERRUN        (-202)
#define UCL_E_LOOKBEHIND_OVERRUN    (-203)
#define UCL_E_EOF_NOT_FOUND         (-204)
#define UCL_E_INPUT_NOT_CONSUMED    (-205)
#define UCL_E_OVERLAP_OVERRUN       (-206)


/* ucl_init() should be the first function you call.
 * Check the return code !
 *
 * ucl_init() is a macro to allow checking that the library and the
 * compiler's view of various types are consistent.
 */
#define ucl_init() __ucl_init2(UCL_VERSION,(int)sizeof(short),(int)sizeof(int),\
    (int)sizeof(long),(int)sizeof(ucl_uint32),(int)sizeof(ucl_uint),\
    (int)ucl_sizeof_dict_t,(int)sizeof(char *),(int)sizeof(ucl_voidp),\
    (int)sizeof(ucl_compress_t))
UCL_EXTERN(int) __ucl_init2(unsigned,int,int,int,int,int,int,int,int,int);

/* version functions (useful for shared libraries) */
UCL_EXTERN(unsigned) ucl_version(void);
UCL_EXTERN(const char *) ucl_version_string(void);
UCL_EXTERN(const char *) ucl_version_date(void);
UCL_EXTERN(const ucl_charp) _ucl_version_string(void);
UCL_EXTERN(const ucl_charp) _ucl_version_date(void);

/* string functions */
UCL_EXTERN(int)
ucl_memcmp(const ucl_voidp _s1, const ucl_voidp _s2, ucl_uint _len);
UCL_EXTERN(ucl_voidp)
ucl_memcpy(ucl_voidp _dest, const ucl_voidp _src, ucl_uint _len);
UCL_EXTERN(ucl_voidp)
ucl_memmove(ucl_voidp _dest, const ucl_voidp _src, ucl_uint _len);
UCL_EXTERN(ucl_voidp)
ucl_memset(ucl_voidp _s, int _c, ucl_uint _len);

/* checksum functions */
UCL_EXTERN(ucl_uint32)
ucl_adler32(ucl_uint32 _adler, const ucl_byte *_buf, ucl_uint _len);
UCL_EXTERN(ucl_uint32)
ucl_crc32(ucl_uint32 _c, const ucl_byte *_buf, ucl_uint _len);

/* memory allocation functions */
UCL_EXTERN(ucl_voidp) ucl_alloc(ucl_uint _nelems, ucl_uint _size);
UCL_EXTERN(ucl_voidp) ucl_malloc(ucl_uint _size);
UCL_EXTERN(void) ucl_free(ucl_voidp _ptr);

typedef ucl_voidp (__UCL_ENTRY *ucl_alloc_hook_t) (ucl_uint, ucl_uint);
typedef void (__UCL_ENTRY *ucl_free_hook_t) (ucl_voidp);

extern ucl_alloc_hook_t ucl_alloc_hook;
extern ucl_free_hook_t ucl_free_hook;

/* misc. */
UCL_EXTERN(ucl_bool) ucl_assert(int _expr);
UCL_EXTERN(int) _ucl_config_check(void);
typedef union { ucl_bytep p; ucl_uint u; } __ucl_pu_u;
typedef union { ucl_bytep p; ucl_uint32 u32; } __ucl_pu32_u;

/* align a char pointer on a boundary that is a multiple of `size' */
UCL_EXTERN(unsigned) __ucl_align_gap(const ucl_voidp _ptr, ucl_uint _size);
#define UCL_PTR_ALIGN_UP(_ptr,_size) \
    ((_ptr) + (ucl_uint) __ucl_align_gap((const ucl_voidp)(_ptr),(ucl_uint)(_size)))


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* already included */

