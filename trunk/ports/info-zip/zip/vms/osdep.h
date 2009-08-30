/*
  Copyright (c) 1990-2005 Info-ZIP.  All rights reserved.

  See the accompanying file LICENSE, version 2004-May-22 or later
  (the contents of which are also included in zip.h) for terms of use.
  If, for some reason, both of these files are missing, the Info-ZIP license
  also may be found at:  ftp://ftp.info-zip.org/pub/infozip/license.html
*/
#ifndef VMS
#  define VMS 1
#endif

#if (defined(__VMS_VER) && !defined(__CRTL_VER))
#  define __CRTL_VER __VMS_VER
#endif

#if (defined(__VMS_VERSION) && !defined(VMS_VERSION))
#  define VMS_VERSION __VMS_VERSION
#endif

#if !(defined(__DECC) || defined(__DECCXX) || defined(__GNUC__))
     /* VAX C does not properly support the void keyword. (Only functions
        are allowed to have the type "void".)  */
#  ifndef NO_TYPEDEF_VOID
#    define NO_TYPEDEF_VOID
#  endif
#  define NO_FCNTL_H        /* VAXC does not supply fcntl.h. */
#endif /* VAX C */

#define NO_UNISTD_H

#define USE_CASE_MAP
#define PROCNAME(n) (action == ADD || action == UPDATE ? wild(n) : \
                     procname(n, 1))

#include <types.h>
#include <stat.h>
#include <unixio.h>

#if defined(__GNUC__) && !defined(ZCRYPT_INTERNAL)
#  include <unixlib.h>          /* ctermid() declaration needed in ttyio.c */
#endif
#ifdef ZCRYPT_INTERNAL
#  include <unixlib.h>          /* getpid() declaration for srand seed */
#endif

#if defined(_MBCS)
#  undef _MBCS                 /* Zip on VMS does not support MBCS */
#endif

#if !defined(NO_EF_UT_TIME) && !defined(USE_EF_UT_TIME)
#  if (defined(__CRTL_VER) && (__CRTL_VER >= 70000000))
#    define USE_EF_UT_TIME
#  endif
#endif

#if defined(VMS_PK_EXTRA) && defined(VMS_IM_EXTRA)
#  undef VMS_IM_EXTRA                 /* PK style takes precedence */
#endif
#if !defined(VMS_PK_EXTRA) && !defined(VMS_IM_EXTRA)
#  define VMS_PK_EXTRA 1              /* PK style VMS support is default */
#endif

#define unlink delete
#define NO_SYMLINK
#define SSTAT vms_stat
#define EXIT(exit_code) vms_exit(exit_code)
#define RETURN(exit_code) return (vms_exit(exit_code), 1)

#ifdef __DECC

/* File open callback ID values. */

#  define FOPM_ID 1
#  define FOPR_ID 2
#  define FOPW_ID 3

/* File open callback ID storage. */

extern int fopm_id;
extern int fopr_id;
extern int fopw_id;

/* File open callback ID function. */

extern int acc_cb();

/* Option macros for zfopen().
 * General: Stream access
 * Output: fixed-length, 512-byte records.
 *
 * Callback function (DEC C only) sets deq, mbc, mbf, rah, wbh, ...
 */

#  define FOPM "r+b", "ctx=stm", "rfm=fix", "mrs=512", "acc", acc_cb, &fopm_id
#  define FOPR "rb",  "ctx=stm", "acc", acc_cb, &fopr_id
#  define FOPW "wb",  "ctx=stm", "rfm=fix", "mrs=512", "acc", acc_cb, &fopw_id

#else /* def __DECC */ /* (So, GNU C, VAX C, ...)*/

#  define FOPM "r+b", "ctx=stm", "rfm=fix", "mrs=512"
#  define FOPR "rb",  "ctx=stm"
#  define FOPW "wb",  "ctx=stm", "rfm=fix", "mrs=512"

#endif /* def __DECC */

