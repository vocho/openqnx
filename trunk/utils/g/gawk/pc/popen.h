/*
** popen.h -- prototypes for pipe functions
*/

#if defined (OS2) && !defined(MSDOS)  /* OS/2, but not family mode */
# if defined (_MSC_VER)
#  define popen(c, m)	_popen(c, m)
#  define pclose(f)	_pclose(f)
# endif
#else
# if !defined (__GO32__)
#  if defined (popen)
#   undef popen
#   undef pclose
#  endif
#  define popen(c, m)	os_popen(c, m)
#  define pclose(f)	os_pclose(f)
   extern FILE *os_popen( char *, char * );
   extern int  os_pclose( FILE * );
# endif
#endif
