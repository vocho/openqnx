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




#ifdef LSDASHL_NOT_IN_LIB

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <pwd.h>
#include <grp.h>
#include <util/util_limits.h>
#include "lsdashl.h"


struct name_cache {
	struct name_cache *next;
	unsigned id;
	char *name;
} *users, *groups;

#define SIX_MONTHS		((long) 6 * 30 * 24 * 60 * 60)	/* same as GNU */

/* make permissions string from file mode */
char *str_mode(mode_t fmode) {
	static char	perms[1+3+3+3+1];		/* static so we can return it */
	char *p = perms;

	/* directory ? */

	
	if		(S_ISDIR(fmode))	*p='d';
	else if (S_ISNAM(fmode))	*p='n';
	else if (S_ISLNK(fmode))	*p='l';
	else if (S_ISCHR(fmode))	*p='c';
	else if (S_ISBLK(fmode))	*p='b';
	else if (S_ISSOCK(fmode))	*p='s';
	else if (S_ISFIFO(fmode))	*p='p';
    else						*p='-';
	p++;

	/* Handle USER permissions */
	*p++ = (fmode & S_IRUSR) ? 'r' : '-';
	*p++ = (fmode & S_IWUSR) ? 'w' : '-';
	if (fmode & S_ISUID)
		*p++ = (fmode & S_IXUSR) ? 's' : 'S';
 	else
		*p++ = (fmode & S_IXUSR) ? 'x' : '-';

	/* Handle GROUP permissions */
	*p++ = (fmode & S_IRGRP) ? 'r' : '-';
	*p++ = (fmode & S_IWGRP) ? 'w' : '-';
	if (fmode & S_ISGID)
		*p++ = (fmode & S_IXGRP) ? 's' : 'L';
	else
		*p++ = (fmode & S_IXGRP) ? 'x' : '-';

	/* Handle OTHER permissions */
	*p++ = (fmode & S_IROTH) ? 'r' : '-';
	*p++ = (fmode & S_IWOTH) ? 'w' : '-';
	if (fmode & S_ISVTX)
		*p++ = (fmode & S_IXOTH) ? 't' : 'T';
	else
		*p++ = (fmode & S_IXOTH) ? 'x' : '-';

	*p = 0;
	return perms;
}

/* return user name or user id string */
char *uid(uid_t uid) {
	return(struid(uid));
}

/* return group name or group id string */
char *gid(gid_t gid) {
	return(strgid(gid));
}

/* return the date formatted a la ls */
char *age(time_t then, mode_t mode) {
	static time_t now = 0;			/* only recompute time() once */
	static char date[28];			/* return buffer with date string */
	long stale;

	if ((then==0L)&&(S_ISCHR(mode))) return "--- -- --:--";

	if (now == 0) time(&now);
	if ((stale = now - then) < 0)
		stale = -stale;
	strftime(date, sizeof date, stale < SIX_MONTHS ?
		"%h %d %H:%M" :  "%h %d  %Y", localtime(&then));
	return date;
}

char *struid(uid_t id)
{
	struct name_cache *tail = users;
	struct passwd *pwent;
	char *name, buff[6];

	while (tail && tail->id != id)
		tail = tail->next;
	if (tail == 0 || tail->id != id) {
		if ((pwent = getpwuid(id)) == 0)
			sprintf(name = buff, "%u", id);
		else
			name = pwent->pw_name;
		if ((tail = malloc(sizeof *tail)) == 0)
			return name;
		tail->id = id;
		tail->name = strdup(name);
		tail->next = users;
		users = tail;
	}
	return tail->name;
}

char *strgid(gid_t id)
{
	struct name_cache *tail = groups;
	struct group *grent;
	char *name, buff[6];

	while (tail && tail->id != id)
		tail = tail->next;
	if (tail == 0 || tail->id != id) {
		if ((grent = getgrgid(id)) == 0)
			sprintf(name = buff, "%u", id);
		else
			name = grent->gr_name;
		if ((tail = malloc(sizeof *tail)) == 0)
			return name;
		tail->id = id;
		tail->name = strdup(name);
		tail->next = groups;
		groups = tail;
	}
	return tail->name;
}
#endif
