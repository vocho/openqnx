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



/*- password database update routines.

There are a set of routines here for updating the password and
spwd databases.
The following routines are exported:

	delpw_name(char *name);
		-- remove the password file entry for user name or uid.
	delsh_name(char *name)
		-- remove the spwd file entry for name.

	chgpw_name(char *name, struct passwd *pw)
		-- change the password file entry for name to be *pw.
	chgsh_name(char *name, struct spwd *sh)
		-- change the spwd file entry for name to be *sh.

	addpwent(struct passwd *pw)
		-- add a password file entry for pw.
	addshent(struct spwd *sh)
		-- add a spwd file entry for sh

These routine do not provide synchronized access, thus two programs could
corrupt data by inappropriate use of these functions.
It is appropriate for higher levels of software to provide the necessary 
locking mechanism.



$Log$
Revision 1.6  2005/06/03 01:22:46  adanko
Replace existing QNX copyright licence headers with macros as specified by
the QNX Coding Standard. This is a change to source files in the head branch
only.

Note: only comments were changed.

PR25328

Revision 1.5  2001/04/18 14:33:14  thomasf
Removed the truncate call and instead do the truncate at open.

Revision 1.4  2001/01/18 18:41:57  thomasf
Fixed to address PR 4359 where we can't use these support functions on
systems that don't support hard links.  We now move to a fall back position
of attempting to copy the file while holding it locked advisorarily so
that other password processes don't get their wires crossed.

Revision 1.3  1999/04/02 20:15:18  bstecher
Clean up some warnings

Revision 1.2  1998/09/26 16:23:00  eric
*** empty log message ***

Revision 1.1  1997/02/18 16:50:08  kirk
Initial revision

 * Revision 1.2  1993/10/27  16:30:36  steve
 * deleted ability to make prefix matches....
 *
 * Revision 1.1  1993/10/27  16:28:42  steve
 * Initial revision
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>

#include "login.h"

/* read only for all but super user */
#define PASSWD_UMASK	022
/* read/write for super user, non-accessable for others... */
#define SHADOW_UMASK	077



/*-

When manipulating the password database there are 6 interesting filenames:
PASSWD  == the "live" password file.
NPASSWD == the "newly created" password file (temporary).
OPASSWD == the "previous incarnation" of the password file.

SHADOW  == the "live" spwd/attribute file.
NSHADOW == the "newly created" spwd file (temporary).
OSHADOW == the "previous incarnation" of the password file.

file_update == 'atomically' arrange for a fileset to be
	advanced a generation (ie. discard old, old = current, current = new).

open_shdb -- open NSHADOW for write and SHADOW for read
close_shdb -- close NSHADOW and SHADOW,
             if NSHADOW was created, update SHADOW to be NSHADOW.
             ( and OSHADOW to be SHADOW)
open_pwdb -- open NPASSWD for write and PASSWD for read.
close_pwdb -- close NPASSWD and PASSWD,
		if NPASSWD was created, update PASSWD to be NPASSWD.
             		( and OPASSWD to be PASSWD)
*/

typedef struct {
	FILE	*ofile;
	FILE	*nfile;
} dbfile_t;

/* When we can't hard link a file then we are forced to copy that 
 file over.  We assume that we have enough stack to do a 1K transfer 
 at a time. This takes an original file and copies it to a new file */
static int 
copy_file(char *ofile, char *nfile) {
#if defined(__QNXNTO__)
	flock_t	thelock;
	int fdto, fdfrom, xfer;
	char buffer[1024];

	if ((fdfrom = open(ofile, O_RDONLY)) == -1) {
		return -1;
	}

	if ((fdto = open(nfile, O_CREAT | O_TRUNC | O_RDWR, 0600)) == -1) {
		//fprintf(stderr, "Open (truncate) %s failed %s\n", nfile, strerror(errno));
		return -1;
	}

	/* Right now we want to lock both of the files in an 
	   advisory manner if we can to prevent other updates */
	memset(&thelock, 0, sizeof(thelock));
	thelock.l_type = F_RDLCK;
	thelock.l_whence = SEEK_SET;
	if (fcntl(fdfrom, F_SETLK, &thelock) == -1) {
		xfer = -1;
		goto leave;
	}

	memset(&thelock, 0, sizeof(thelock));
	thelock.l_type = F_WRLCK;
	thelock.l_whence = SEEK_SET;
	if (fcntl(fdto, F_SETLK, &thelock) == -1) {
		xfer = -1;
		goto leave;
	}
	
	while((xfer = read(fdfrom, buffer, sizeof(buffer))) > 0) {
		if (write(fdto, buffer, xfer) != xfer) {
			xfer = -1;
			break;
		}
	}

leave:
	/* Close the files (implicitly freeing any locks) */
	close(fdfrom);
	close(fdto);

	return (xfer < 0) ? -1 : 0;
#else
	return -1;
#endif
}

static int
file_update(char *ofile, char *cfile, char *nfile)
{
	int		 canlink;
	sigset_t nosig, osig;

	sigfillset(&nosig);
	sigprocmask(SIG_SETMASK, &nosig, &osig);

	unlink(ofile);
	canlink = 1;
	if (link(cfile,ofile) == -1) {
		int e = errno;
		//If the file doesn't exist don't exit, but assume copy.
		if (access(cfile, F_OK) != -1) {
			if (copy_file(cfile, ofile) == -1) {
				fprintf(stderr, "Can't copy or link %s -> %s\n", cfile, ofile);
				/* Error: cannot backup previous one -- abort */
				sigprocmask(SIG_SETMASK, &osig, 0);
				errno = e;
				return 1;
			} 
		} 
		canlink = 0;
	} 

	//Don't unlink the file for safety reasons if we can't link it
	if (canlink) {
		unlink(cfile);
	}

	//Try the link first ... then try the copy
	if (link(nfile,cfile) == -1) {
		copy_file(nfile, cfile);
	}

	unlink(nfile);

	sigprocmask(SIG_SETMASK, &osig, 0);
	return 0;
}

static int
open_shdb(dbfile_t *f, int mustwrite)
{
	mode_t	msk;
	msk = umask(SHADOW_UMASK);

	if ((f->nfile = fopen(NSHADOW, "w")) == NULL && mustwrite) {
		umask(msk);
		return -1;
	}
 	f->ofile = fopen(SHADOW, "r");
	umask(msk);
	return 0;
}

static int
close_shdb(dbfile_t *f)
{
	if (f->ofile)
		fclose(f->ofile);
	if (!f->nfile)
		return 0;

	fclose(f->nfile);

/* in order:
	1.   remove the 'old spwd'
	2.   link current spwd to old spwd
	3.   remove spwd.
	4.   link spwd to newly created one.
	5.   remove newly created one.
*/

	return file_update(OSHADOW, SHADOW, NSHADOW);
}

static int
open_pwdb(dbfile_t *f, int mustwrite)
{
	mode_t	msk;
	msk = umask(PASSWD_UMASK);

	if ((f->nfile = fopen(NPASSWD, "w")) == NULL && mustwrite) {
		umask(msk);
		return 1;
	}

	if ((f->ofile = fopen(PASSWD, "r")) == NULL) {
//		fclose(f->nfile);
//		umask(msk);
//		return 1;
	}
	umask(msk);
	return 0;
}

static int
close_pwdb(dbfile_t *f)
{
	if (f->ofile) fclose(f->ofile);
	if (!f->nfile) return 0;
	fclose(f->nfile);
	return file_update(OPASSWD, PASSWD, NPASSWD);
}


static int
putpw(dbfile_t *f, struct passwd *pw)
{
	fprintf(f->nfile,"%s:%s:%d:%d:%s:%s:%s\n",
			pw->pw_name, pw->pw_passwd,
			pw->pw_uid, pw->pw_gid,
			pw->pw_comment, pw->pw_dir, pw->pw_shell);
	return 0;
}

static int
putsh(dbfile_t *f, struct spwd *sh)
{
#if 0
	fprintf(f->nfile,"%s:%s:%ld:%ld:%ld:%ld:%ld:%ld:%ld\n",
		sh->sp_namp,
		sh->sp_pwdp,
		sh->sp_lstchg,
		sh->sp_max,
		sh->sp_min,
		sh->sp_warn,
		sh->sp_inact,
		sh->sp_expire,
		sh->sp_flag);
#else
	fprintf(f->nfile,"%s:%s:%ld:%ld:%ld\n",
		sh->sp_namp,
		sh->sp_pwdp,
		sh->sp_lstchg,
		sh->sp_max,
		sh->sp_min  /* rest are ignored ;-( */
#if 0
		,
		sh->sp_warn,
		sh->sp_inact,
		sh->sp_expire,
		sh->sp_flag
#endif
		);
#endif
	return 0;
}


static int
copy_cond(dbfile_t *f, int (*match)(char *key, char *str), char *key)
{
#define BLEN 256
	char	buffer[BLEN+1];

	if (!f->ofile) return 0;
	while (fgets(buffer,sizeof buffer-1, f->ofile)) {
		if ((*match)(key, buffer) == 0) {
			return 1;
		}
		if (f->nfile) fputs(buffer,f->nfile);
	}
	return 0;
}

/*-
 * The peculiar length check is to ensure the strings are EQUAL not
 * just a prefix match
 */
static int
match_name(char *nam, char *bufp)
{
	char	*p = strchr(bufp,':');
	int	n = strlen(nam);
	if (!p)	 return 1;
	if (p-bufp != n) return 1;
	return strncmp(nam,bufp,p-bufp);
}

#if 0
This is currently used, but is here for demonstration should this
be required later....
static int
match_uid(char *uid, char *bufp)
{
	char	*p;
	if (!(p=strchr(bufp,':')))	return 1;
	return strtol(uid,0,0) != strtol(p,0,0);
}
#endif

int
delpw_name(char *name)
{
	dbfile_t f;
	if (open_pwdb(&f,1)) {
		return 1;
	}
	while (copy_cond(&f, match_name, name) == 1);
	return close_pwdb(&f);
}

int
chgpw_name(char *name, struct passwd *pw)
{
	dbfile_t f;
	if (open_pwdb(&f,1) != 0) {
		return 1;
	}
	copy_cond(&f,match_name,name);
	putpw(&f, pw);
	while (copy_cond(&f,match_name,name) == 1);
	return close_pwdb(&f);
}

int
addpwent(struct passwd *pw)
{
	return chgpw_name(pw->pw_name, pw);
}



int
delsh_name(char *name)
{
	dbfile_t f;
	if (open_shdb(&f,1)) {
		return 1;
	}
	while (copy_cond(&f, match_name, name) == 1);
	return close_shdb(&f);
}

int
chgsh_name(char *name, struct spwd *sh)
{
	dbfile_t f;
	if (open_shdb(&f,1) != 0) {
		return 1;
	}
	copy_cond(&f,match_name,name);
	putsh(&f, sh);
	while (copy_cond(&f,match_name,name) == 1);
	return close_shdb(&f);
}

int
addshent(struct spwd *sh)
{
	return chgsh_name(sh->sp_namp, sh);
}
