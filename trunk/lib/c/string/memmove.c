/* memmove function */
#include <string.h>
#include <sys/types.h>
#include <inttypes.h>
_STD_BEGIN

#define COPYTYPE  int
#define COPYALIGN (sizeof(COPYTYPE) - 1)

void *(memmove)(void *s1, const void *s2, size_t n)
	{	/* copy char s2[n] to s1[n] safely */

#ifdef __QNX__

    /*
     * If not overlapping, perform the much faster memcpy().
     */   
    if( (uintptr_t)s1 + n <= (uintptr_t)s2 || 
        (uintptr_t)s2 + n <= (uintptr_t)s1 )
    {
        return memcpy( s1, s2, n );
    }

    /*
     * Check alignment - do byte copies if not aligned or if there
     * isn't enough data to justify the extra setup overhead.
     */ 
    if( n < 4*sizeof(COPYTYPE) || 
        ( ( (uintptr_t)s1 & COPYALIGN ) != ( (uintptr_t)s2 & COPYALIGN ) ) )
    {
        char *sc1 = (char *)s1;
        const char *sc2 = (const char *)s2;

        if (sc2 < sc1 && sc1 < sc2 + n)
            for (sc1 += n, sc2 += n; 0 < n; --n)
                *--sc1 = *--sc2;	/* copy backwards */
        else
            for (; 0 < n; --n)
                *sc1++ = *sc2++;	/* copy forwards */
    }
    else
    {
        size_t start;
        size_t end;
        size_t ctn;
        COPYTYPE *ct1;
        const COPYTYPE *ct2;
        char *sc1;
        const char *sc2;
      
        /* Calculate the number of start and end bytes to copy by byte */ 
        start = ( (uintptr_t)s1 & COPYALIGN );
        if( start != 0 )
            start = sizeof( COPYTYPE ) - start;
        end = (n - start) & COPYALIGN;

        /* Calculate the number of COPYTYPE quantums to copy */
        ctn = (n - start) / sizeof(COPYTYPE); 

        sc1 = (char *)s1;
        sc2 = (const char *)s2;

        /* 
         * Check to see if copies need to be forward or backward.
         */
        if (sc2 < sc1 && sc1 < sc2 + n)
        { 
            /* 
             *  copy backwards, starting with the end bytes.
             */
            for( sc1 += n, sc2 += n; 0 < end; --end )
                *--sc1 = *--sc2;

            /* sc1 and sc2 now point to the properly aligned addresses */
            ct1 = (COPYTYPE *)sc1;
            ct2 = (const COPYTYPE *)sc2;
            for (; 0 < ctn; --ctn)
                *--ct1 = *--ct2;	

            /* Finish the copy with the start bytes */
            sc1 = (char *)ct1;
            sc2 = (const char *)ct2;
            for( ; 0 < start; --start )
                *--sc1 = *--sc2;
        }
        else
        {
            /* 
             *  copy forwards, starting with the start bytes.
             */
            for( ; 0 < start; --start )
                *sc1++ = *sc2++;

            /* sc1 and sc2 now point to the properly aligned addresses */
            ct1 = (COPYTYPE *)sc1;
            ct2 = (const COPYTYPE *)sc2;
            for (; 0 < ctn; --ctn)
                *ct1++ = *ct2++;	

            /* Finish the copy with the end bytes */
            sc1 = (char *)ct1;
            sc2 = (const char *)ct2;
            for( ; 0 < end; --end )
                *sc1++ = *sc2++;
        }
    }

#else

	char *sc1 = (char *)s1;
	const char *sc2 = (const char *)s2;

	if (sc2 < sc1 && sc1 < sc2 + n)
		for (sc1 += n, sc2 += n; 0 < n; --n)
			*--sc1 = *--sc2;	/* copy backwards */
	else
		for (; 0 < n; --n)
			*sc1++ = *sc2++;	/* copy forwards */

#endif

	return (s1);
	}
_STD_END

/*
 * Copyright (c) 1992-2006 by P.J. Plauger.  ALL RIGHTS RESERVED.
 * Consult your license regarding permissions and restrictions.
V5.00:1296 */

__SRCVERSION("memmove.c $Rev: 153052 $");
