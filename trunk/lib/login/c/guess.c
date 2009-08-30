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



/*-
* guess.c:   Find unused user and group ids from the password database.
* Simple mechanism used here -- build array of userids in use, sort the
* array and look for holes.



* $Log$
* Revision 1.4  2005/06/03 01:22:46  adanko
* Replace existing QNX copyright licence headers with macros as specified by
* the QNX Coding Standard. This is a change to source files in the head branch
* only.
*
* Note: only comments were changed.
*
* PR25328
*
* Revision 1.3  1999/04/02 20:15:18  bstecher
* Clean up some warnings
*
* Revision 1.2  1998/09/26 16:23:00  eric
* *** empty log message ***
*
* Revision 1.1  1997/02/18 16:50:08  kirk
* Initial revision
*
*/

#include <stdio.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#include "login.h"

static int
ucompare(const void *u0, const void *u1)
{
	return *(const unsigned *)u0 - *(const unsigned *)u1;
}

static int
getpw_uids(unsigned **uid_list)
{
	struct	passwd *pw;
	int	llen=0;
	int	curp=0;
	unsigned *ulist = 0;
	endpwent();
	while (pw=getpwent()) {
		if (curp == llen) {
			unsigned *t = realloc(ulist,(llen+=32)*sizeof *t);
			if (!t) {
				free(ulist);
				return -1;
			}
			ulist = t;
		}
		ulist[curp++] = pw->pw_uid;
	}
	ulist = realloc(ulist,curp*sizeof *ulist);
	qsort(ulist, curp, sizeof *ulist, ucompare);
	*uid_list = ulist;
	return curp;
}

static int
getgr_uids(unsigned **gid_list)
{
	struct	group *grp;
	int	llen=0;
	int	curp=0;
	unsigned *ulist = 0;
	endgrent();
	while (grp=getgrent()) {
		if (curp == llen) {
			unsigned *t = realloc(ulist,(llen+=32)*sizeof *t);
			if (!t) {
				free(ulist);
				return -1;
			}
			ulist = t;
		}
		ulist[curp++] = grp->gr_gid;
	}
	ulist = realloc(ulist,curp*sizeof *ulist);
	qsort(ulist, curp, sizeof *ulist, ucompare);
	*gid_list = ulist;
	return curp;
}

/*-
 * fairly simple search for a constraining entry.   The funny internal
 * while is to absorb duplicate ids (aliases).
 */
static int
get_slot(int ntab, unsigned *utab, unsigned umin, unsigned umax)
{
	int	i;

	if (ntab == -1) {
		fprintf(stderr,"Warning: not enough space to guess user id\n");
		return umin;
	}
	for (i=0; i < ntab && utab[i] <= umin; ) {
		if (utab[i] == umin) {
			while (i < ntab && utab[i] == umin)
				i++;
			umin++;
		} else {
			i++;
		}
	}
	if (umin >= umax) {
		return -1;
	}
	return umin;
}
int
guess_uid(void)
{
	unsigned *utab;
	int	ntab;
	int	id;
	long    uid_min=0, uid_max= INT_MAX;

	getdef_range("UIDRANGE",&uid_min, &uid_max);

	if ((ntab = getpw_uids(&utab)) == -1) {
		fprintf(stderr,"Warning: no space to select user id!\n");
		return uid_min;
	}
	id = get_slot(ntab,utab,uid_min,uid_max);
	free(utab);
	if (id == -1) {
		fprintf(stderr,"Error: no room to add user id!\n");
		fprintf(stderr,"Contact System Administrator.\n");
	}
	return id;
}

int
guess_gid(void)
{
	unsigned *utab;
	int	ntab;
	int	i;

	long    gid_min=0, gid_max= INT_MAX;

	getdef_range("GIDRANGE",&gid_min, &gid_max);
	if ((ntab = getgr_uids(&utab)) == -1) {
		fprintf(stderr,"Warning: no space to select group id!\n");
		return gid_min;
	}
	for (i=0; i < ntab; i++) {
		if (utab[i] >= gid_min) {
			gid_min = utab[i];
			break;
		}
	}
	free(utab);
	return gid_min;
}
