/* yfuns.h functions header */
#ifndef _YFUNS
#define _YFUNS
#include <stddef.h>
_C_STD_BEGIN

#if !defined(_USE_EXISTING_SYSTEM_NAMES) && defined(__QNX__)
#define _USE_EXISTING_SYSTEM_NAMES 1
#endif

 #if _USE_EXISTING_SYSTEM_NAMES

  #if defined(__BORLANDC__)
   #define _Environ	_environ

  #else /* defined(__BORLANDC__) */
   #define _Environ	environ
  #endif /* defined(__BORLANDC__) */

 #if _HAS_C9X

 #else /* _IS_C9X */
 #define _Exit	_exit
 #endif /* _IS_C9X */

  #define _Close	close
  #define _Lseek	lseek
  #define _Read		read
  #define _Write	write
 #endif /* _USE_EXISTING_SYSTEM_NAMES */

		/* process control */
#define _Envp	(*_Environ)

		/* stdio functions */
#define _Fclose(str)	_Close(_FD_NO(str))
#define _Fread(str, buf, cnt)	_Read(_FD_NO(str), buf, cnt)
#define _Fwrite(str, buf, cnt)	_Write(_FD_NO(str), buf, cnt)

		/* interface declarations */
#ifndef __QNX__
_EXTERN_C
extern const char **_Environ;
_NO_RETURN(_Exit(int));

 #if defined(__APPLE__)
int _Close(int);
_Longlong _Lseek(int, _Longlong, int);
int _Read(int, void *, size_t);
int _Write(int, const void *, size_t);

 #else /* defined(__APPLE__) */
int _Close(int);
long _Lseek(int, long, int);
int _Read(int, unsigned char *, int);
int _Write(int, const unsigned char *, int);
 #endif /* defined(__APPLE__) */

_END_EXTERN_C
#endif
_C_STD_END
#endif /* _YFUNS */

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

/* __SRCVERSION("yfuns.h $Rev: 153052 $"); */
