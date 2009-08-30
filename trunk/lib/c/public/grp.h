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
 *  grp.h   Group operations
 *

 */

#ifndef _GRP_H_INCLUDED
#define _GRP_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
 #pragma read_only_file;
#endif

#ifndef __TYPES_H_INCLUDED
#include <sys/types.h>
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#include <_pack64.h>


struct group {
    char  *gr_name;
    char  *gr_passwd;
    gid_t  gr_gid;
    char **gr_mem;
};

/*
 *  POSIX 1003.1 Prototypes.
 */

#include <_packpop.h>

__BEGIN_DECLS

extern void endgrent( void );
extern int getgrgid_r(gid_t __gid, struct group *__grp, char *__buffer, size_t __bufsize, struct group **__result);
extern int getgrnam_r(const char *__name, struct group *__grp, char *__buffer, size_t __bufsize, struct group **__result);
extern struct group *getgrnam( const char * __name );
extern struct group *getgrgid( gid_t __gid );
extern struct group *getgrent( void );
extern int initgroups( const char *__name, gid_t __basegid );
extern void setgrent( void );

__END_DECLS

#endif

/* __SRCVERSION("grp.h $Rev: 171385 $"); */
