/* safe.h */
/* Copyright 1995 by Steve Kirkendall */


#ifndef DEBUG_ALLOC

BEGIN_EXTERNC
extern void *safealloc P_((int qty, size_t size));
extern void safefree P_((void *ptr));
extern char *safedup P_((char *str));
END_EXTERNC
# define safekept	safealloc
#define safekdup	safedup
# define safeterm()
# define safeinspect()

#else

# define safealloc(qty, size)	_safealloc(__FILE__, __LINE__, ElvFalse, qty, size)
# define safekept(qty, size)	_safealloc(__FILE__, __LINE__, ElvTrue, qty, size)
# define safefree(ptr)		_safefree(__FILE__, __LINE__, ptr)
# define safedup(str)		_safedup(__FILE__, __LINE__, ElvFalse, str)
# define safekdup(str)		_safedup(__FILE__, __LINE__, ElvTrue, str)
BEGIN_EXTERNC
extern void *_safealloc P_((char *file, int line, ELVBOOL kept, int qty, size_t size));
extern void _safefree P_((char *file, int line, void *ptr));
extern char *_safedup P_((char *file, int line, ELVBOOL kept, char *str));
extern void safeterm P_((void));
extern void safeinspect P_((void));
END_EXTERNC

#endif
