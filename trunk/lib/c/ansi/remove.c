/* remove function */
#include <stdlib.h>
#include "xstdio.h"
#ifdef __QNX__
#include <sys/stat.h>
#include <unistd.h>
#endif
_STD_BEGIN

 #if _USE_EXISTING_SYSTEM_NAMES

  #if _FN_WIDE

  #else /* _FN_WIDE */
   #define _Rmdir rmdir
   #define _Unlink unlink
  #endif /* _FN_WIDE */

 #endif /* _USE_EXISTING_SYSTEM_NAMES */

_EXTERN_C
int _Rmdir(const char *);
int _Unlink(const char *);
int _WUnlink(const wchar_t *);
_END_EXTERN_C

 #if _HAS_POSIX_C_LIB || _WIN32_C_LIB
int (remove)(const char *filename)
	{	/* remove a file */

 #if _FN_WIDE
	wchar_t wc_fname[_FNAMAX];

	if (mbstowcs(wc_fname, filename, _FNAMAX) == (size_t)(-1))
		return (-1);
	return (_WUnlink(wc_fname));

 #else /* _FN_WIDE */
#ifdef __QNX__
struct stat st;
	return((lstat(filename, &st) == -1 || !S_ISDIR(st.st_mode)) ? unlink(filename) : rmdir(filename));
#else
	return (_Rmdir(filename) == 0 ? 0 : _Unlink(filename));
#endif
 #endif /* _FN_WIDE */

	}

 #elif _DUMMY_C_LIB
int (remove)(const char *filename)
	{	/* remove a file */
 	return (-1);
	}

 #else /* _LIB version */
	/* revert to system library */
 #endif /* _LIB version */

_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("remove.c $Rev: 153052 $");
