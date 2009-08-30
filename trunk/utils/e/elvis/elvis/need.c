/* need.c */
/* Copyright 1995 by Steve Kirkendall */


#include "elvis.h"
#ifdef FEATURE_RCSID
char id_need[] = "$Id: need.c,v 2.5 2001/10/23 01:37:09 steve Exp $";
#endif


#ifdef NEED_STRDUP
# if USE_PROTOTYPES
char *strdup(const char *str)
{
# else /* don't USE_PROTOTYPES */
char *strdup(str)
	char	*str;
{
# endif /* don't USE_PROTOTYPES */

	char	*ret;

	ret = (char *)safealloc(strlen(str) + 1, sizeof(char));
	strcpy(ret, str);
	return ret;
}
#endif /* NEED_STRDUP */

#ifdef NEED_MEMMOVE
# if USE_PROTOTYPES
void *memmove(void *dest, const void *src, size_t size)
# else /* don't USE_PROTOTYPES */
void *memmove(dest, src, size)
	void	*dest;
	void	*src;
	size_t	size;
# endif /* don't USE_PROTOTYPES */
{
	register char	*d, *s;

	d = (char *)dest;
	s = (char *)src;
	if (d <= s)
	{
		for (; size > 0; size--)
			*d++ = *s++;
	}
	else
	{
		for (d += size, s += size; size > 0; size--)
			*--d = *--s;
	}
	return dest;
}
#endif

#ifdef NEED_XRMCOMBINEFILEDATABASE
/* The XrmCombineFileDatabase() function is defined in guix11/xmisc.c */
#endif

#ifdef NEED_INET_ATON	
/* The inet_aton() function is defined in osunix/osnet.c */
#endif
