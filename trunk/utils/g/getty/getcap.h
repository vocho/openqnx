#ifndef _GETCAP_H_INCLUDED
#define _GETCAP_H_INCLUDED

#include <sys/platform.h>

#ifdef __cplusplus
extern "C" {
#endif

char	*getbsize __P((int *, long *));
char	*cgetcap __P((char *, const char *, int));
int	 cgetclose __P((void));
int	 cgetent __P((char **, char **, const char *));
int	 cgetfirst __P((char **, char **));
int	 cgetmatch __P((const char *, const char *));
int	 cgetnext __P((char **, char **));
int	 cgetnum __P((char *, const char *, long *));
int	 cgetset __P((const char *));
int	 cgetstr __P((char *, const char *, char **));
int	 cgetustr __P((char *, const char *, char **));

#ifdef __cplusplus
};
#endif

#endif /* _GETCAP_H_INCLUDED */
