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
 *  strings.h    String functions (porting assist)
 *

 */
#ifndef _STRINGS_H_INCLUDED
#define _STRINGS_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef _STRING_H_INCLUDED
#include <string.h>
#endif

#ifdef __cplusplus
__BEGIN_DECLS

extern int  ffs(int);
extern int  strcasecmp(const char *, const char *);
extern int  strncasecmp(const char *, const char *, _CSTD size_t);

__END_DECLS

inline int bcmp(const void *_Str1, const void *_Str2, _CSTD size_t _Num)
  { return _CSTD memcmp(_Str1, _Str2, _Num); }
inline void bcopy(const void *_Src, void *_Dst, _CSTD size_t _Num)
  { (void) _CSTD memmove(_Dst, _Src, _Num); return; }
inline void bzero(void *_Dst, _CSTD size_t _Num)
  { (void) _CSTD memset(_Dst, 0, _Num); return; }
inline const char *index(const char *_Src, int _Cnt)
  { return _CSTD strchr(_Src, _Cnt); }
inline const char *rindex(const char *_Src, int _Cnt)
  { return _CSTD strrchr(_Src, _Cnt); }

#else /* __cplusplus */
__BEGIN_DECLS

#undef bcmp
#undef bcopy
#undef bzero
#undef index
#undef rindex

extern int  bcmp(const void *, const void *, size_t);
extern void bcopy(const void *, void *, size_t);
extern void bzero(void *, size_t);
extern int  ffs(int);
extern char *index(const char *, int);
extern char *rindex(const char *, int);
extern int  strcasecmp(const char *, const char *);
extern int  strncasecmp(const char *, const char *, size_t);

#define bcmp(s1,s2,n)	(_CSTD memcmp(s1,s2,n))
#define bcopy(s,d,n)	((void)_CSTD memmove(d,s,n))
#define bzero(d,n)		((void)_CSTD memset(d,0,n))
#define index(s,c)		(_CSTD strchr(s,c))
#define rindex(s,c)		(_CSTD strrchr(s,c))

__END_DECLS
#endif /* __cplusplus */

#endif

/* __SRCVERSION("strings.h $Rev: 199673 $"); */
