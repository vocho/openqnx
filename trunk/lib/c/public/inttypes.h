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
 *  inttypes.h Defined system types
 *

 */
#ifndef _INTTYPES_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#if !defined(__cplusplus) || defined(_STD_USING) || defined(_GLOBAL_USING)
#define _INTTYPES_H_INCLUDED
#endif

#ifndef _INTTYPES_H_DECLARED
#define _INTTYPES_H_DECLARED

#ifndef _STDINT_H_INCLUDED
#include _NTO_HDR_(stdint.h)
#endif

#define PRId8			"hhd"
#define PRId16			"hd"
#define PRId32			"d"
#define PRId64			"lld"

#define PRIdFAST8		"hhd"
#define PRIdFAST16		"hd"
#define PRIdFAST32		"d"
#define PRIdFAST64		"lld"

#define PRIdLEAST8		"hhd"
#define PRIdLEAST16		"hd"
#define PRIdLEAST32		"d"
#define PRIdLEAST64		"lld"

#define PRIdMAX			"lld"
#if __PTR_BITS__ <= 32
  #define PRIdPTR PRId32
#else
  #define PRIdPTR PRId64
#endif

#define PRIi8			"hhi"
#define PRIi16			"hi"
#define PRIi32			"i"
#define PRIi64			"lli"

#define PRIiFAST8		"hhi"
#define PRIiFAST16		"hi"
#define PRIiFAST32		"i"
#define PRIiFAST64		"lli"

#define PRIiLEAST8		"hhi"
#define PRIiLEAST16		"hi"
#define PRIiLEAST32		"i"
#define PRIiLEAST64		"lli"

#define PRIiMAX			"lli"
#if __PTR_BITS__ <= 32
  #define PRIiPTR PRIi32
#else
  #define PRIiPTR PRIi64
#endif

#define PRIo8			"hho"
#define PRIo16			"ho"
#define PRIo32			"o"
#define PRIo64			"llo"

#define PRIoFAST8		"hho"
#define PRIoFAST16		"ho"
#define PRIoFAST32		"o"
#define PRIoFAST64		"llo"

#define PRIoLEAST8		"hho"
#define PRIoLEAST16		"ho"
#define PRIoLEAST32		"o"
#define PRIoLEAST64		"llo"

#define PRIoMAX			"llo"
#if __PTR_BITS__ <= 32
  #define PRIoPTR PRIo32
#else
  #define PRIoPTR PRIo64
#endif

#define PRIu8			"hhu"
#define PRIu16			"hu"
#define PRIu32			"u"
#define PRIu64			"llu"

#define PRIuFAST8		"hhu"
#define PRIuFAST16		"hu"
#define PRIuFAST32		"u"
#define PRIuFAST64		"llu"

#define PRIuLEAST8		"hhu"
#define PRIuLEAST16		"hu"
#define PRIuLEAST32		"u"
#define PRIuLEAST64		"llu"

#define PRIuMAX			"llu"
#if __PTR_BITS__ <= 32
  #define PRIuPTR PRIu32
#else
  #define PRIuPTR PRIu64
#endif

#define PRIx8			"hhx"
#define PRIx16			"hx"
#define PRIx32			"x"
#define PRIx64			"llx"

#define PRIxFAST8		"hhx"
#define PRIxFAST16		"hx"
#define PRIxFAST32		"x"
#define PRIxFAST64		"llx"

#define PRIxLEAST8		"hhx"
#define PRIxLEAST16		"hx"
#define PRIxLEAST32		"x"
#define PRIxLEAST64		"llx"

#define PRIxMAX			"llx"
#if __PTR_BITS__ <= 32
  #define PRIxPTR PRIx32
#else
  #define PRIxPTR PRIx64
#endif

#define PRIX8			"hhX"
#define PRIX16			"hX"
#define PRIX32			"X"
#define PRIX64			"llX"

#define PRIXFAST8		"hhX"
#define PRIXFAST16		"hX"
#define PRIXFAST32		"X"
#define PRIXFAST64		"llX"

#define PRIXLEAST8		"hhX"
#define PRIXLEAST16		"hX"
#define PRIXLEAST32		"X"
#define PRIXLEAST64		"llX"

#define PRIXMAX			"llX"
#if __PTR_BITS__ <= 32
  #define PRIXPTR PRIX32
#else
  #define PRIXPTR PRIX64
#endif

#define SCNd8			"hhd"
#define SCNd16			"hd"
#define SCNd32			"d"
#define SCNd64			"lld"

#define SCNdFAST8		"hhd"
#define SCNdFAST16		"hd"
#define SCNdFAST32		"d"
#define SCNdFAST64		"lld"

#define SCNdLEAST8		"hhd"
#define SCNdLEAST16		"hd"
#define SCNdLEAST32		"d"
#define SCNdLEAST64		"lld"

#define SCNdMAX			"lld"
#if __PTR_BITS__ <= 32
  #define SCNdPTR SCNd32
#else
  #define SCNdPTR SCNd64
#endif

#define SCNi8			"hhi"
#define SCNi16			"hi"
#define SCNi32			"i"
#define SCNi64			"lli"

#define SCNiFAST8		"hhi"
#define SCNiFAST16		"hi"
#define SCNiFAST32		"i"
#define SCNiFAST64		"lli"

#define SCNiLEAST8		"hhi"
#define SCNiLEAST16		"hi"
#define SCNiLEAST32		"i"
#define SCNiLEAST64		"lli"

#define SCNiMAX			"lli"
#if __PTR_BITS__ <= 32
  #define SCNiPTR SCNi32
#else
  #define SCNiPTR SCNi64
#endif

#define SCNo8			"hho"
#define SCNo16			"ho"
#define SCNo32			"o"
#define SCNo64			"llo"

#define SCNoFAST8		"hho"
#define SCNoFAST16		"ho"
#define SCNoFAST32		"o"
#define SCNoFAST64		"llo"

#define SCNoLEAST8		"hho"
#define SCNoLEAST16		"ho"
#define SCNoLEAST32		"o"
#define SCNoLEAST64		"llo"

#define SCNoMAX			"llo"
#if __PTR_BITS__ <= 32
  #define SCNoPTR SCNo32
#else
  #define SCNoPTR SCNo64
#endif

#define SCNu8			"hhu"
#define SCNu16			"hu"
#define SCNu32			"u"
#define SCNu64			"llu"

#define SCNuFAST8		"hhu"
#define SCNuFAST16		"hu"
#define SCNuFAST32		"u"
#define SCNuFAST64		"llu"

#define SCNuLEAST8		"hhu"
#define SCNuLEAST16		"hu"
#define SCNuLEAST32		"u"
#define SCNuLEAST64		"llu"

#define SCNuMAX			"llu"
#if __PTR_BITS__ <= 32
  #define SCNuPTR SCNu32
#else
  #define SCNuPTR SCNu64
#endif

#define SCNx8			"hhx"
#define SCNx16			"hx"
#define SCNx32			"x"
#define SCNx64			"llx"

#define SCNxFAST8		"hhx"
#define SCNxFAST16		"hx"
#define SCNxFAST32		"x"
#define SCNxFAST64		"llx"

#define SCNxLEAST8		"hhx"
#define SCNxLEAST16		"hx"
#define SCNxLEAST32		"x"
#define SCNxLEAST64		"llx"

#define SCNxMAX			"llx"
#if __PTR_BITS__ <= 32
  #define SCNxPTR SCNx32
#else
  #define SCNxPTR SCNx64
#endif

#define SCNX8			"hhX"
#define SCNX16			"hX"
#define SCNX32			"X"
#define SCNX64			"llX"

#define SCNXFAST8		"hhX"
#define SCNXFAST16		"hX"
#define SCNXFAST32		"X"
#define SCNXFAST64		"llX"

#define SCNXLEAST8		"hhX"
#define SCNXLEAST16		"hX"
#define SCNXLEAST32		"X"
#define SCNXLEAST64		"llX"

#define SCNXMAX			"llX"
#if __PTR_BITS__ <= 32
  #define SCNXPTR SCNX32
#else
  #define SCNXPTR SCNX64
#endif

_C_STD_BEGIN

typedef struct {
	intmax_t quot, rem;
} imaxdiv_t;

__BEGIN_DECLS
intmax_t imaxabs(intmax_t __i);
imaxdiv_t imaxdiv(intmax_t __numer, intmax_t __denom);

intmax_t strtoimax(__const char *__restrict __s, char **__restrict __endptr, int __base);
uintmax_t strtoumax(__const char *__restrict s, char **__restrict __endptr, int __base);
intmax_t wcstoimax(__const _Wchart *__restrict s, _Wchart **__restrict __endptr, int __base);
uintmax_t wcstoumax(__const _Wchart *__restrict s, _Wchart **__restrict __endptr, int __base);


#if defined(__LITTLEENDIAN__)
struct __byte {
	uint8_t		__lo;
	uint8_t		__hi;
};

struct __short {
	uint16_t	__lo;
	uint16_t	__hi;
};

struct __long {
	uint32_t	__lo;
	uint32_t	__hi;
};
#elif defined(__BIGENDIAN__)
struct __byte {
	uint8_t		__hi;
	uint8_t		__lo;
};
	
struct __short {
	uint16_t	__hi;
	uint16_t	__lo;
};
	
struct __long {
	uint32_t	__hi;
	uint32_t	__lo;
};
#else
 #error endian not configured for system
#endif

__END_DECLS
_C_STD_END

#endif

#ifdef _STD_USING
using std::imaxdiv_t;
using std::imaxabs; using std::imaxdiv;
using std::strtoimax; using std::strtoumax;
using std::wcstoimax; using std::wcstoumax;
#endif /* _STD_USING */
#endif

/* __SRCVERSION("inttypes.h $Rev: 158726 $"); */
