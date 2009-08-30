/* need.h */
/* Copyright 1995 by Steve Kirkendall */

#ifdef NEED_ABORT
# define abort()	(*"x" = 0)
#endif

#ifdef NEED_ASSERT
# ifdef NDEBUG
#  define assert(x)
# else
#  define assert(x)	if (!(x)) abort(); else (void)(0)
# endif
#else
# include <assert.h>
#endif

#ifdef NEED_STRDUP
BEGIN_EXTERNC
extern char *strdup P_((const char *str));
END_EXTERNC
#endif

#ifdef NEED_MEMMOVE
BEGIN_EXTERNC
extern void *memmove P_((void *, const void *, size_t));
END_EXTERNC
#endif


