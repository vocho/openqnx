/*
 * $QNXtpLicenseC:
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





/* $Source$
 *
 * $Revision: 153052 $
 *
 * names.c - Look up user and/or group names. 
 *
 * DESCRIPTION
 *
 *	These functions support UID and GID name lookup.  The results are
 *	cached to improve performance.
 *
 * AUTHOR
 *
 *	Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice is duplicated in all such 
 * forms and that any documentation, advertising materials, and other 
 * materials related to such distribution and use acknowledge that the 
 * software was developed * by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Log$
 * Revision 1.5  2005/06/03 01:37:53  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.4  2003/09/24 19:51:53  thomasf
 * Updates to make the build work with the mingw platform.
 *
 * Revision 1.3  2003/08/27 18:16:57  martin
 * Add QSSL Copyright to cover QNX contributions.
 *
 * Revision 1.2  1998/12/03 18:56:14  eric
 * tweaked for win32
 *
 * Revision 1.1  1998/12/03 18:54:43  eric
 * Initial revision
 *
 * Revision 1.2  89/02/12  10:05:05  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:19  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: names.c 153052 2008-08-13 01:17:50Z coreos $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"


/* Defines */

#define myuid	( my_uid < 0? (my_uid = getuid()): my_uid )
#define	mygid	( my_gid < 0? (my_gid = getgid()): my_gid )


/* Internal Identifiers */

static int      saveuid = -993;
static char     saveuname[TUNMLEN];
static int      my_uid = -993;

static int      savegid = -993;
static char     savegname[TGNMLEN];
static int      my_gid = -993;


/* finduname - find a user or group name from a uid or gid
 *
 * DESCRIPTION
 *
 * 	Look up a user name from a uid/gid, maintaining a cache. 
 *
 * PARAMETERS
 *
 *	char	uname[]		- name (to be returned to user)
 *	int	uuid		- id of name to find
 *
 *
 * RETURNS
 *
 *	Returns a name which is associated with the user id given.  If there
 *	is not name which corresponds to the user-id given, then a pointer
 *	to a string of zero length is returned.
 *	
 * FIXME
 *
 * 	1. for now it's a one-entry cache. 
 *	2. The "-993" is to reduce the chance of a hit on the first lookup. 
 */

#ifdef __STDC__

char *finduname(int uuid)

#else
    
char *finduname(uuid)
int             uuid;

#endif
{
#if !defined(__NT__) && !defined(__MINGW32__)
    struct passwd  *pw;
#endif

    if (uuid != saveuid) {
	saveuid = uuid;
	saveuname[0] = '\0';
#if !defined(__NT__) && !defined(__MINGW32__)
	pw = getpwuid(uuid);
	if (pw) {
	    strncpy(saveuname, pw->pw_name, TUNMLEN);
	}
#endif
    }
    return(saveuname);
}


/* finduid - get the uid for a given user name
 *
 * DESCRIPTION
 *
 *	This does just the opposit of finduname.  Given a user name it
 *	finds the corresponding UID for that name.
 *
 * PARAMETERS
 *
 *	char	uname[]		- username to find a UID for
 *
 * RETURNS
 *
 *	The UID which corresponds to the uname given, if any.  If no UID
 *	could be found, then the UID which corrsponds the user running the
 *	program is returned.
 *
 */

#ifdef __STDC__

int finduid(char *uname)

#else
    
int finduid(uname)
char            *uname;

#endif
{
#if !defined(__NT__) && !defined(__MINGW32__)
    struct passwd  *pw;
    extern struct passwd *getpwnam();
#endif

    if (uname[0] != saveuname[0]/* Quick test w/o proc call */
	||0 != strncmp(uname, saveuname, TUNMLEN)) {
	strncpy(saveuname, uname, TUNMLEN);
#if defined(__NT__) || defined(__MINGW32__)
	saveuid = myuid;
#else
	pw = getpwnam(uname);
	if (pw) {
	    saveuid = pw->pw_uid;
	} else {
	    saveuid = myuid;
	}
#endif
    }
    return (saveuid);
}


/* findgname - look up a group name from a gid
 *
 * DESCRIPTION
 *
 * 	Look up a group name from a gid, maintaining a cache.
 *	
 *
 * PARAMETERS
 *
 *	int	ggid		- goupid of group to find
 *
 * RETURNS
 *
 *	A string which is associated with the group ID given.  If no name
 *	can be found, a string of zero length is returned.
 */

#ifdef __STDC__

char *findgname(int ggid)

#else
    
char *findgname(ggid)
int             ggid;

#endif
{
#if !defined(__NT__) && !defined(__MINGW32__)
    struct group   *gr;
#endif

    if (ggid != savegid) {
	savegid = ggid;
	savegname[0] = '\0';
#if !defined(__NT__) && !defined(__MINGW32__)
	setgrent();
	gr = getgrgid(ggid);
	if (gr) {
	    strncpy(savegname, gr->gr_name, TGNMLEN);
	}
#endif
    }
    return(savegname);
}



/* findgid - get the gid for a given group name
 *
 * DESCRIPTION
 *
 *	This does just the opposit of finduname.  Given a group name it
 *	finds the corresponding GID for that name.
 *
 * PARAMETERS
 *
 *	char	uname[]		- groupname to find a GID for
 *
 * RETURNS
 *
 *	The GID which corresponds to the uname given, if any.  If no GID
 *	could be found, then the GID which corrsponds the group running the
 *	program is returned.
 *
 */

#ifdef __STDC__

int findgid(char *gname)

#else
    
int findgid(gname)
char           *gname;

#endif
{
#if !defined(__NT__) && !defined(__MINGW32__)
    struct group   *gr;
#endif

    /* Quick test w/o proc call */
    if (gname[0] != savegname[0] || strncmp(gname, savegname, TUNMLEN) != 0) {
	strncpy(savegname, gname, TUNMLEN);
#if defined(__NT__) || defined(__MINGW32__)
	savegid = mygid;
#else
	gr = getgrnam(gname);
	if (gr) {
	    savegid = gr->gr_gid;
	} else {
	    savegid = mygid;
	}
#endif
    }
    return (savegid);
}
