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




#include <dirent.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <libgen.h>
#include <util/stdutil.h>
#include <util/stat_optimiz.h>

#ifdef __USAGE
%-chown
%C	- change file ownership (POSIX)

%C	[-R] [-v] user[:group] file ...
Options:
 -R     Recursively change file owners.  For each operand that names a
        directory, change the owner of the directory and all files
        in the directory.
 -v     Verbose. Display to stdout the operations which are being
        performed.
Where:
 user   is a name or numeric user id from the user database.
 group  is a name or numeric group id from the group database.
 file   is the pathname of a file whose ownership is to be modified.

Note:
 For compatibility with some other implementations of chown,
 a period (.) may be used instead of a colon (:) to separate
 user and group (e.g. user:group and user.group are both allowed).
 However, be aware that if a userid contains a period, it may be
 specified either alone or in conjunction with a group using ':',
 but may not be used in conjunction with a group using '.'. For
 instance if there was a userid 'my.name' and a group 'tech',
 you could do a 'chown my.name myfile' or 'chown my.name:tech myfile',
 but NOT 'chown my.name.tech myfile'.
%-chgrp
%C	- change file group ownership (POSIX)

%C	[-R] group file ...
Options:
 -R     Recursively change group IDs for each operand that names a
        directory, change the group of the directory and all files
        in the directory.
 -v     Verbose. Display to stdout the operations which are being
        performed.
Where:
 group  is a name or numeric group id from the group database.
#endif

char *progname;
#ifndef PATH_MAX
#define PATH_MAX (_POSIX_PATH_MAX*2)
#endif
char pathname[PATH_MAX];
uid_t uid = -1;
gid_t gid = -1;
int ischown, recurse, verbose;

#define INVALID (0)
#define LSTAT_VALID (1)
#define STAT_VALID (2)

/*
 * Change the owner or group ID of the file specified by path.
 * If must recurse (-R option on command line) and file is a directory,
 * recurse through the tree
 * statbuf points either to:
 *  statinfo = LSTAT_VALID           lstat information
 *  statinfo = STAT_VALID            stat info; file is NOT a directory
 *                                   to recurse into
 *  statinfo = INVALID               no stat information
 */
int change_file(char *path, struct stat *statbuf, int statinfo)
{
    /* if we haven't done a stat then do it */
    if (statinfo==INVALID && lstat(path, statbuf) == -1) {
		fprintf(stderr, "%s: %s (lstat %s)\n", progname, strerror(errno), path);
		return 0;
    }

    /* recurse into directories if required */
    if (statinfo!=STAT_VALID && recurse && S_ISDIR(statbuf->st_mode)) {
		struct dirent *dent;
		DIR *dirp;
		char *p;

		if ((dirp = opendir(path)) == 0) {
		    fprintf(stderr, "%s: %s (opendir %s)\n", progname, strerror(errno), path);
		    return 0;
		}

		p = path + strlen(path) - 1;
		if (*p++ != '/') *p++ = '/';
		while (errno=0,dent = readdir(dirp)) {
			struct stat statbuf2;
			int flag;
	
		    if ((dent->d_name[0] == '.' && (dent->d_name[1] == 0 ||	/* .  */
			(dent->d_name[1] == '.' && (dent->d_name[2] == 0)))))	/* .. */
			continue;
		    strcpy(p, dent->d_name);
	
			flag=INVALID;

			if (lstat_optimize(dent, &statbuf2)!=-1) {
				flag=LSTAT_VALID;
				if (S_ISLNK(statbuf->st_mode))
					if (stat_optimize(dent, &statbuf2)!=-1)
						flag=STAT_VALID;
			}

			change_file(path, &statbuf2, flag);
		}
		*--p = 0;

		if (errno) {
			fprintf(stderr,"%s: %s (readdir %s)\n", progname, strerror(errno), path);
		}
	
		if (closedir(dirp) == -1) {
		    fprintf(stderr, "%s: %s (closedir %s)\n", progname, strerror(errno), path);
		    return 0;
		}

    }

	if (statinfo!=STAT_VALID) {
		/* We have the lstat info. If that wasn't a symlink, that will
           suffice. Otherwise, we must now get the real stat info for
		   the file */
		if (S_ISLNK(statbuf->st_mode)) 
			if (stat(path, statbuf) == -1) {
			fprintf(stderr, "%s: %s (stat %s)\n", progname, strerror(errno), path);
			return 0;
   		}
	}

    /* actually change the file */
    if (chown(path, uid == -1 ? statbuf->st_uid : uid,
		    gid == -1 ? statbuf->st_gid : gid) == -1)
	{
		fprintf(stderr, "%s: %s (chown %s)\n", progname, strerror(errno), path);
		return 0;
    } else if (verbose) {
		printf("%s: Changing ownership of %s to ", progname, path);
		if (uid!=-1) printf("user %d",uid);
		if (gid!=-1) printf("%sgroup %d",uid==-1?"":", ",gid);
		printf("\n");
	}
    return 1;
}

int get_uid(char *s) {
    struct passwd *pwent;
    long val;
    char *p;

    if ((pwent = getpwnam(s)))
	uid = pwent->pw_uid;
    else {
		val = strtol(s, &p, 10);
		if ( val < 0L || *p) {
		    fprintf(stderr, "%s: invalid owner %s\n", progname, s);
		    return 0;
		} else
		    uid = val;
	}
    return 1;
}

int get_gid(char *s) {
    struct group *grent;
    long val;
    char *p;

    if ((grent = getgrnam(s)))
	gid = grent->gr_gid;
    else {
		val = strtol(s, &p, 10);
		if ( val < 0L || *p) {
		    fprintf(stderr, "%s: invalid group %s\n", progname, s);
		    return 0;
		} else
		    gid = val;
	}
    return 1;
}

int get_uid_gid(char *ug_ptr) {
    /* check for user:group syntax (POSIX D11) */
    if (ischown) {
	 	char *p;

		if ((p = strchr(ug_ptr, ':'))) {
		    *p++ = 0;
		    return get_uid(ug_ptr) && get_gid(p);
		} else if ((p=strchr(ug_ptr, '.'))) {	 /* also allow usr.grp */
			/* . is a valid part of a username. We must check that the
                 string as a whole is not a valid username before
                 trying it as a user.group pair */
			if (getpwnam(ug_ptr)) return get_uid(ug_ptr);
			/* was not a valid username */
 			*p++ = 0;
		    return get_uid(ug_ptr) && get_gid(p);
		} else return get_uid(ug_ptr);
    } else return get_gid(ug_ptr);
}

int main(int argc, char **argv) {
    int error = 0, opt;
    int first_index, last_index, num_files, index;
    struct stat statbuf;

    if (strcmp("chgrp", progname = basename(argv[0]))) ischown = 1;

    /* parse cmd line arguments */
    while ((opt = getopt(argc, argv, "R**v")) != EOF) {
	switch (opt) {
	  case 'R': recurse = 1; break;
	  case 'v': verbose = 1; break;
	  default:  exit(EXIT_FAILURE);
	}
    }

    first_index = optind;
    last_index = argc - 1;
    num_files = last_index - first_index;

    /* must be at least one file, and valid user/group id */
    if (num_files <1 || get_uid_gid(argv[first_index++]) == 0) {
	fprintf(stderr, "usage: %s [-R] %s file ...\n",
	    progname, ischown ? "user[:group]" : "group");
	exit(EXIT_FAILURE);
    }

    /* handle each file/directory */
    for (index = first_index; index <= last_index; index++) {
		if (!change_file(strcpy(pathname, argv[index]), &statbuf, INVALID))
		    error++;
    }

    return (error != 0);
}
