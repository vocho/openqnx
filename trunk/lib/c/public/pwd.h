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
 *  pwd.h       Password operations
 *

 */
#ifndef _PWD_H_INCLUDED
#define _PWD_H_INCLUDED

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

struct passwd {
    char  *pw_name;     /*  User's login name           */
    char  *pw_passwd;
    uid_t  pw_uid;      /*  User ID number              */
    gid_t  pw_gid;      /*  Group ID number             */
    char  *pw_age;
    char  *pw_comment;
    char  *pw_gecos;
    char  *pw_dir;      /*  Initial Working Directory   */
    char  *pw_shell;    /*  Initial User Program        */
};

#include <_packpop.h>

__BEGIN_DECLS

/*
 *  POSIX 1003.1 Prototypes.
 */
#ifdef __EXT_POSIX1_198808
extern struct passwd *getpwent( void );
extern struct passwd *getpwnam( const char * name );
extern struct passwd *getpwuid( uid_t uid );
extern void endpwent( void );
extern void setpwent( void );
#endif
#ifdef __EXT_POSIX1_199506
extern int getpwnam_r(const char *name, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);
extern int getpwuid_r(uid_t uid, struct passwd *pwd, char *buffer, size_t bufsize, struct passwd **result);
#endif
#ifdef __EXT_UNIX_MISC
extern int getpwent_r(struct passwd *pwd, char *buffer, int bufsize, struct passwd **result);
#endif

__END_DECLS

#endif

/* __SRCVERSION("pwd.h $Rev: 212726 $"); */
