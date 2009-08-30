
#include <sys/cdefs.h>

#if defined(BUILDENV_iox) && defined(__WEAK_ALIAS)
#undef __WEAK_ALIAS
#endif

#ifdef __WEAK_ALIAS

#define LIBC_WEAK __WEAK_ALIAS

#define memcpyv		__memcpyv
#define strrev		__strrev
#define strnset		__strnset
#define strset		__strset
#define straddstr	__straddstr

/*
 * Single underscore here as they (_* versions)
 * were traditionally exported in <string.h>
 */
#define memicmp		_memicmp
#define stricmp		_stricmp
#define strcmpi		_stricmp
#define strlwr		_strlwr
#define strnicmp	_strnicmp
#define strupr		_strupr

#define mallinfo	__mallinfo
#define mallopt		__mallopt
#define mprobe		__mprobe
#define mcheck		__mcheck

#define sopen		__sopen

#else /* ! __WEAK_ALIAS */

#define LIBC_WEAK(a, b) /* Nothing if compiler doesn't support weak aliases */

#endif /* ! __WEAK_ALIAS */
