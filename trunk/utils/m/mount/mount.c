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





/*
 * This code was originally based on the NetBSD code for 
 * mount, but has been wacked apart for Neutrino.  For
 * information and reference look at the NetBSD source
 *
 */ 

/*
 * Copyright (c) 1980, 1988, 1993
 *  The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *  This product includes software developed by the University of
 *  California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/wait.h>

#include <err.h>
#include <errno.h>
#include <pwd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "fstab.h"

int	verbose;

int	checkvfsname(const char *, const char **);
void catopt(char **, const char *);
int	hasopt(const char *, const char *);
const char **makevfslist(char *);

static void mangle(char *, int *, const char ***, int *);
static char * guess_mount_type(const char *mntfromname, const char *mntonname);
static int mountfs(const char *, const char *, const char *, int, const char *, const char *, int);
static void usage(void);

struct fstab * getfsent();

int internal_mount_generic(const char *type, int argc, char **argv);
int report_mounts(int flags);

#define				_PATH_BIN			"/bin"
#define				_PATH_USRBIN		"/usr/bin"

#define	BADTYPE(type) (strcmp(type, FSTAB_RO) && strcmp(type, FSTAB_RW))

#define				_FLAG_REPORT_NONFS	0x1
#define				_FLAG_NO_FSTAB		0x2

int main(int argc, char **argv) {
	const char *mntfromname, *mntonname, *vfstype;
	const char **vfslist;
	char *options, *extoptions, slash[2], nullstring[1];
	struct fstab *fs;
	int all, rval, ch, alttypemode, init_flags, other_flags;

	slash[0] = '/'; slash[1] = '\0';
	nullstring[0] = '\0';
	rval = 0;
	options = 0;
	other_flags = 0;
	while (argc) {
		vfslist = NULL;
		mntfromname = mntonname = vfstype = extoptions = NULL;
		alttypemode = rval = all = verbose = init_flags = 0;
		while ((ch = getopt(argc, argv, "T:t:o:eaurwvdnb")) != -1) {
			switch (ch) {
			case 'a':
				all = 1;
				break;
			case 'u':			//update/remount
				init_flags |= _MOUNT_REMOUNT;
				break;
			case 'e':
				init_flags |= _MOUNT_ENUMERATE;
				break;
			case 'r':
				init_flags |= _MOUNT_READONLY;
				break;
			case 'w':
				init_flags &= ~_MOUNT_READONLY;
				break;
			case 'o':
				catopt(&options, optarg);
				break;

			case 'T':			
				init_flags |= _MFLAG_OCB;
				vfslist = makevfslist(optarg);
				vfstype = optarg;
				alttypemode = 1;
				break;
			case 't':			
				init_flags &= ~_MFLAG_OCB;
				vfslist = makevfslist(optarg);
				vfstype = optarg;
				alttypemode = 0;
				break;

			case 'b':
				other_flags |= _FLAG_NO_FSTAB;
				break;
			case 'n':
				other_flags |= _FLAG_REPORT_NONFS;
				break;

			case 'v':
				verbose++;
				break;

			case '?':
			default:
				usage();
			}
		}
		argc -= optind;
		argv += optind;

/*
		 This table quickly describes what we need to do in the
		 presence of various options:
		 (mntfromname == spec_dev, mntonname == path)

		 -a --> We want to mount all the devices in the
		        fstab file (or autodetected later on)
				-If we have a vfstype, only mount those entries
				-Flag ignored if a spec_dev || path specified

		 -u --> We are re-mounting, so we must have as a minimum
		        a path, but more information would be better to
				match in a union fs situation.

		 -e --> Enumerate through and mount all things (partitions?)
		        associated with this device and use the given path
				as a prefix, ie:

				Mount all eide drives starting w/ /dev/hd0,/dev/hd1 ...
					mount -e -T eide /dev/hd
				Mount all scsi devices starting w/ /dev/scsi0,/dev/scsi1 ...
					mount -e -T scsi /dev/scsi

				Mount all partitions on /dev/hd0 w/ /dev/hd0t77,/dev/hd0t79 ...
					mount -e -t all /dev/hd0 /dev/hd0
				Mount all QNX partitions on /dev/hd0 w/ /dev/hd0t77,/dev/hd0t79 ...
					mount -e -t t79,t77,t78 /dev/hd0 /dev/hd0

				- We should be able to enumerate and re-mount 
				- It would be nice to not have to specify a -t/T all the time

		 -r/w > We are mounting with read/[write] mode. 
		 -o --> Append this as an option to send to the server

		 -t type implies that the special device and path will both exist,
		         and if the special device exists it should be opened and
				 an OCB for that device should be send to the path.

		 -T type implies that the special device doesn't necessarily have
		         to exist, and NO OCB will be send along with the path
				 if the device doesn't exist.

Special | OCB | Mountpoint | Command
-----------------------------------
yes     | yes | yes        | mount -t type special mountpoint
-----------------------------------
yes     | yes | no ('/')   | mount -t type special ('/' implied)
	-> Perform lookup for special matching type in vfslist first.
	-> _MOUNT_IMPLIED set
-----------------------------------
yes     | no  | yes        | mount -T type special mountpoint
-----------------------------------
yes     | no  | no ('/')   | mount -T type special ('/' implied) 
	-> Perform lookup for special matching type in vfslist first.
	-> _MOUNT_IMPLIED set
-----------------------------------
no      | yes | XXX        | Impossible
-----------------------------------
no (NUL)| no  | yes        | mount -T type "" mountpoint (infrequent)
-----------------------------------
no (NUL)| no  | no ('/')   | mount -T type (NULL '/' implied)
	-> Perform lookup for special/dir matching type in vfslist first.
	-> _MOUNT_IMPLIED set
-----------------------------------

*/

		//Assume the first non-option is the device name
		if (argc > 0) {
			mntfromname = argv[0];
			argv++; argc--;
		}

		//Assume the next non-option is the path name
		if (argc > 0 && *argv[0] != '-') {	
			mntonname = argv[0];
			argv++; argc--;
		}

		/* 
		 Leave the mntonname as a NULL and if we don't find it in the 
		 vfs list, then we will used the implied value of '/'
		*/
		
		if (verbose) {
			printf("Parsed: mount from [%s] mount on [%s] type [%s] \n", 
				(mntfromname) ? mntfromname : "NULL", 
				(mntonname) ? mntonname : "NULL",
				(vfstype) ? vfstype : "NULL");
		}

		//If re-mounting, then we absolutely need a path (don't fstab check?)
		if (init_flags & _MOUNT_REMOUNT && !mntfromname) {
			fprintf(stderr, "Remount: mount point needed \n");
			exit(1);
		}

		/*
		 We are mounting all of the files, or displaying mounts
		 % mount -[tT] xxx | mount
		*/
		if (!mntfromname && !mntonname) {
			if ((all || vfstype) && !(other_flags & _FLAG_NO_FSTAB)) {
				while ((fs = (struct fstab *)getfsent()) != NULL) {
						if ((BADTYPE(fs->fs_type)) ||
						     checkvfsname(fs->fs_vfstype, vfslist) ||
						     hasopt(fs->fs_mntops, "noauto")) {
							continue;
						}
						if (vfstype && strcmp(vfstype, fs->fs_vfstype) != 0) {
							continue;
						}
						if (mountfs(fs->fs_vfstype, fs->fs_spec, fs->fs_file, 
						            init_flags, options, fs->fs_mntops, 0)) {
							exit(1);	
						}
						rval++;
				}
				if(rval == 0) {
					fprintf(stderr, (vfstype) ? "No fstab entries found for type %s\n" : "No fstab entries\n", vfstype);
					exit(1);	
				}
			}
			else {
				report_mounts(other_flags);
			}
			exit(0);
		}
		/* 
		 We are guessing at the type to mount based on the path/device
		 % mount -[tT] xxx special 
		   --> examine fstab for information first before sending NULL special device
		*/
		else if (mntfromname && !mntonname) {
			//Remounting, don't need the special device name
			if (init_flags & _MOUNT_REMOUNT) {
				fs = NULL;
			} 
			//Otherwise go looking for a match
			else if ((other_flags & _FLAG_NO_FSTAB) ||
                     ((fs = (struct fstab *)getfsfile(mntfromname)) == NULL &&
			          (fs = (struct fstab *)getfsspec(mntfromname)) == NULL)) {
				fs = NULL;
			}
			else if (BADTYPE(fs->fs_type)) {
				fs = NULL;
			}

			if (fs) {				/* VFS entry was found, use it */
				vfstype = fs->fs_vfstype;
				mntfromname = fs->fs_spec;
				mntonname = fs->fs_file;
				extoptions = fs->fs_mntops;
				init_flags = (init_flags & ~fs->init_mask) | fs->init_flags;
			} else if(init_flags & _MOUNT_REMOUNT) {				/* No VFS entry was found, send a slash */
				mntonname = mntfromname;
				mntfromname = NULL;
			} else {
				mntonname = slash;
#if defined(_MOUNT_IMPLIED)
				init_flags |= _MOUNT_IMPLIED;
#endif
			}	
		}

		/*
		 One of the fields is valid (the name or the special device, or both)
		 % mount -T xxx special | mount -T xxx special /mnt/here | mount -t xxx special /mnt/here
		*/
		if (!vfstype) {
			if (init_flags & (_MOUNT_ENUMERATE | _MOUNT_REMOUNT)) {
				vfstype = nullstring;
			} else {
				vfstype = guess_mount_type(mntfromname, mntonname);
			}

			if (verbose) {
				fprintf(stderr, "Guessing type [%s], use -[t|T] if incorrect \n", 
					(vfstype) ? vfstype : "");
			}
		}

		//TODO: Error out here if we don't have a type?
		rval = mountfs(vfstype, mntfromname, mntonname, 
					   init_flags, options, extoptions, 0);

		//Exit if we had any problems
		if (rval != 0) {
			break;
		}
		optind = 0;
	}	

	exit(rval);
}

/*
 Take a guess at what an appropriate type would be for this
 mount point.  If we don't know ... we return qnx4 as
 the default mount type.

 Using the statvfs information would be nice, but it only
 works properly for the devb-xxx drivers once a partition 
 has been mounted, too bad.
*/
static char * guess_mount_type(const char *mntfromname, const char *mntonname) {
	static char typebuf[32];	/* Same size as statvfs */

	if(mntfromname) {
		if(strpbrk(mntfromname, "@:")) {
			strcpy(typebuf, NFS_FS_TYPE);
		} else if (strstr(mntfromname, "t77") ||
				   strstr(mntfromname, "t79") ||
				   strstr(mntfromname, "t78")) {
			strcpy(typebuf, QNX4_FS_TYPE);
		} else if (strstr(mntfromname, "cd")) {
			strcpy(typebuf, CD_FS_TYPE);
		} else if (strstr(mntfromname, "npm")) {
			strcpy(typebuf, "io-net");
		} else {
			strcpy(typebuf, QNX4_FS_TYPE);
		}
	} else {
		strcpy(typebuf, QNX4_FS_TYPE);
	}
	
	return typebuf;
}

/*
 This function will iterate through mntopts (usually defined 
 from the fstab entry) checking to see if option is defined 
 in the string mntopts, (or not defined if the string starts 
 w/ no).  We only use this once to check for noauto.
*/
int hasopt(const char *mntopts, const char *option) {
	int negative, found;
	char *opt, *optbuf;

	if (option[0] == 'n' && option[1] == 'o') {
		negative = 1;
		option += 2;
	} else
		negative = 0;
	optbuf = strdup(mntopts);
	found = 0;
	for (opt = optbuf; (opt = strtok(opt, ",")) != NULL; opt = NULL) {
		if (opt[0] == 'n' && opt[1] == 'o') {
			if (!strcasecmp(opt + 2, option))
				found = negative;
		} else if (!strcasecmp(opt, option))
			found = !negative;
	}
	free(optbuf);
	return (found);
}

/*
 This function is the one which actually re-composes the
 argument to mount into a sensible command line and then
 passes them to the individual mount functions. If these
 all fail, then the default handler is called.

 If skipmounted is set then we should query to see if
 a device is already mounted and not try it again.
*/
static int mountfs(const char *vfstype, const char *spec, const char *name, 
			int flags, const char *options, const char *mntopts, int skipmounted) {
	/* List of directories containing mount_xxx subcommands. */
	static const char *edirs[] = {
		_PATH_BIN,
		_PATH_USRBIN,
		".",
		NULL
	};
	const char **argv, **edir;
	char execname[MAXPATHLEN + 1], execbase[MAXPATHLEN];
	int  argc, i, maxargc;
	char *optbuf;

	optbuf = NULL;
	if (mntopts) {
		catopt(&optbuf, mntopts);
	}
	if (options) {
		catopt(&optbuf, options);
	}
	if (!mntopts && !options) {
		catopt(&optbuf, "rw");
	}

	if (flags & _MOUNT_ENUMERATE) {
		catopt(&optbuf, "enumerate");
	}
	if (flags & _MOUNT_REMOUNT) {
		catopt(&optbuf, "remount");
	}
#if defined(_MOUNT_IMPLIED)
	if (flags & _MOUNT_IMPLIED) {
		catopt(&optbuf, "implied");
	}
#endif
	if (flags & _MOUNT_READONLY) {
		catopt(&optbuf, "ro");
	}
	if (flags & _MFLAG_OCB) {
		catopt(&optbuf, "nostat");
	}

	maxargc = 64;
	argv = malloc(sizeof(char *) * maxargc);

	argc = 0;
	snprintf(execbase, sizeof(execbase), "mount_%s", vfstype);
	argv[argc++] = execbase;			//Placeholder
	if (optbuf) {
		mangle(optbuf, &argc, &argv, &maxargc);
	}
	if (spec) {
		argv[argc++] = spec;
	}
	argv[argc++] = name;
	argv[argc] = NULL;

	if (verbose > 1) {
		printf("exec:");
		for (i = 0; i < argc; i++) {
			printf(" %s", argv[i]);
		}
		printf("\n");
	}

	edir = edirs;
	do {

		snprintf(execname, sizeof(execname), "%s/%s", *edir, execbase);
		i = spawnv(P_WAIT, execname, (char * const *)argv);

		if (i == -1 && errno != ENOENT) {
			warn("exec %s for %s", execname, name);
		}

		if (i == 0) {
			return(0);
		}

	} while (*++edir != NULL);

	/* If that doesn't work, try the internal generic one */
	if (verbose > 2) {
		printf("Using internal mount (%s not found)\n", execbase);
	}
	
	i = internal_mount_generic(vfstype, argc, (char **)argv);

	if (i != 0) {
		if (errno == ENOSYS) {
			errno = EINVAL;		//Nicer than function not implemented
		}
		warnx("Can't %smount %s (type %s) ", (flags & _MOUNT_REMOUNT) ? "re" : "", name, strcmp(vfstype, "") ? vfstype : "default");
		warnx("Possible reason: %s ", strerror(errno));
	}

	if (optbuf) {
		free(optbuf);
	}

	return i;
}


void catopt(char **sp, const char *o) {
	char *s;
	size_t i = 0, j = 0; /* Initialize so SH LE build doesn't complain. */

	s = *sp;
	if (s) {
		i = strlen(s);
		j = i + 1 + strlen(o) + 1;
		s = realloc(s, j);
		snprintf(s + i, j, ",%s", o);
	} else {
		s = strdup(o);
	}
	*sp = s;
}

static void mangle(char *options, int *argcp, 
				   const char ***argvp, int *maxargcp) {
	char *p, *s;
	int argc, maxargc;
	const char **argv;

	argc = *argcp;
	argv = *argvp;
	maxargc = *maxargcp;

	for (s = options; (p = strsep(&s, ",")) != NULL;) {
		/* Always leave space for one more argument and the NULL. */
		if (argc >= maxargc - 4) {
			maxargc <<= 1;
			argv = realloc(argv, maxargc * sizeof(char *));
		}
		if (*p != '\0') {
			if (*p == '-') {
				argv[argc++] = p;
				p = strchr(p, '=');
				if (p) {
					*p = '\0';
					argv[argc++] = p+1;
				}
			} else if (strcmp(p, "rw") != 0) {
				argv[argc++] = "-o";
				argv[argc++] = p;
			}
		}
	}

	*argcp = argc;
	*argvp = argv;
	*maxargcp = maxargc;
}

static void usage() {
	fprintf(stderr, "Run use mount for usage information\n");
	exit(1);
}


/*
 *  Handle generic mounts that no=one has written a 
 *  handler for and hope that someone responds.
 */
int internal_mount_generic(const char *type, int argc, char **argv) {
	char *special, *dir, *ret, *extra_opts;
	int c, flags;

	flags = 0;
	extra_opts = NULL;

	//Assumes an input string in the form of -o options,options=value,options ...
	optind = 1;
	while ((c = getopt(argc, argv, "o:")) != -1) {
		switch (c) {
		case 'o': 
			if ((ret = mount_parse_generic_args(optarg, &flags))) {
				catopt(&extra_opts, ret);
			}
			break;

		default:
			printf("Options can only be passed as -o option[=value] to the generic handler \n");
			if(extra_opts) {
				free(extra_opts);
			}
			return(1);	
		}
	}

	special = dir = NULL;
	if (optind <= argc -1) {
		dir = argv[argc - 1];
	}
	if (optind <= argc -2) {
		special = argv[argc - 2];
	}

	if (verbose > 1) {
		printf("Type    [%s] Flags 0x%08x \n", 
				(type) ? type : "NULL", flags);
		printf("Device  [%s] Directory [%s] \n", 
				(special) ? special : "NULL", (dir) ? dir : "NULL");
		printf("Options [%s] \n", 
				(extra_opts) ? extra_opts : "NULL");
	}

	c = mount(special, dir, flags, type, extra_opts, -1);

	if(extra_opts) {
		free(extra_opts);
	}

	return((c == -1) ? 1 : 0);
}

/*
 Here we report our mount points by traversing through
 the proc/mount filesystem.  We report the mount points
 as:
 pid<@special device> on <mountpoint> type <fstype>

 We can only report the special device and the fstype
 if the filetype is FTYPE_ANY, otherwise we report
 the numeric value.

 /proc/mount shows the entries as 
	nid,pid,chid,handle,ftype
*/
#include <fcntl.h>
#include <dirent.h>
#include <sys/ftype.h>
#include <sys/dcmd_blk.h>

static const char *ftype_names[]={ 
	"ftype:ALL",     "ftype:ANY",  "ftype:FILE",   "ftype:LINK",
	"ftype:SYMLINK", "ftype:PIPE", "ftype:SHMEM",  "ftype:MQUEUE",
	"ftype:SOCKET",  "ftype:SEM",  "ftype:PHOTON", "ftype:DUMPER",
	"ftype:MOUNT",   "ftype:NAME",  NULL 
};

#if defined(SHOW_PIDNAME) 
#include <libgen.h>
#include <sys/procfs.h>
#include <sys/neutrino.h>
static char *get_process_name(int pid) {
	struct {
        procfs_debuginfo info;
        char buff[1024];
	} map;
	char path[100];
	int fd;

	sprintf( path, "/proc/%d/as", pid);
	if ((fd = open ( path, O_RDONLY )) == -1) {
		printf("Can't open path \n");
		return(NULL);
	}

	memset(&map, 0, sizeof(map));
	devctl(fd, DCMD_PROC_MAPDEBUG_BASE, &map, sizeof(map), 0); 

	return(strdup(map.info.path));
}
#endif

static int _report_mounts(char *path, int pathsize, int offset, int flags) {
	DIR *dp;
	struct dirent *dent;
	struct stat	   st;
	int			  ret, len;
	int			  nd,pid,chid,handle,ftype;
	char		  mounton[256], fstype[16];

	if (!(dp = opendir(path))) {
		return 0;
	}
	len = strlen(path);

	/*TODO: Report all of the mounts in this directory first */

	/* Then enter the directory which don't contain mounts */
	while ((dent = readdir(dp))) {

		path[len] = '\0';

		if (strcmp(dent->d_name, ".") == 0 || strcmp(dent->d_name, "..") == 0) {
			continue;
		}

		if (len + strlen(dent->d_name) + 1 > pathsize) {
			if (verbose)
				fprintf(stderr, "Warning: path too deep [%s]+[%s]\n", path, dent->d_name);
			continue;
		}

		ret = sscanf(dent->d_name, "%d,%d,%d,%d,%d", 
					&nd, &pid, &chid, &handle, &ftype);

		path[len] = '/';
		strcpy(&path[len+1], dent->d_name);
		
		fstype[0] = mounton[0] = '\0';
		memset(&st, 0, sizeof(st));

		if (stat(path, &st) == -1) {
			if (verbose)
				fprintf(stderr, "Warning: can't stat [%s]\n", path);
			continue;
		}

		/* Recurse through non matching directories */
		if (ret != 5) {
			if (S_ISDIR(st.st_mode)) {
				_report_mounts(path, pathsize, offset, flags);
			}
			continue;
		}

		/* Skip these mount placeholders */
		if (ftype == _FTYPE_MOUNT) {
			continue;
		}

		/* These are real fileystems (or pretend to be), get more info */
		if (S_ISDIR(st.st_mode) && ftype == _FTYPE_ANY) {
			struct statvfs stvfs;
			int  fd;

			if ((fd = open(path, /* O_ACCMODE */ O_RDONLY)) == -1) {
				ret = -1;
			}
			else {
				devctl(fd, DCMD_FSYS_MOUNTED_ON, mounton, 256, 0);
				ret = fstatvfs(fd, &stvfs);	
				close(fd);
			}

			if (ret != -1) {
				strcpy(fstype, stvfs.f_basetype);
			}
			/* If we can't get a statvfs, assume it ain't a real fs */
			else if (!(flags & _FLAG_REPORT_NONFS)) {
				continue;
			}
		}
		/* These are single point mounts */
		else {			
			if (!(flags & _FLAG_REPORT_NONFS)) {
				continue;
			}
			if (ftype+1 < sizeof(ftype_names)/sizeof(*ftype_names)) {
				strcpy(fstype, ftype_names[ftype+1]);
			}
		}

		path[len] = '\0';

		/* Print out the whole kit and kaboodle */
		printf("%s on %s type ",
			(mounton[0]) ? mounton : "none",
			(*(path + offset) == '\0') ? "/" : path + offset);
		if (fstype[0]) {
			printf("%s ", fstype);
		}
		else {
			printf("%d ", ftype);
		}
#if defined(SHOW_PIDNAME) 
		{
			char *pidname;
			if (!(pidname = get_process_name(pid))) {
				printf("(%d)", pid);
			}
			else {
				printf("(%s)", basename(pidname));
				free(pidname);
			}
		}	
#endif
		printf("\n");
	}

	return 0;
}

int report_mounts(int flags) {
	char buffer[PATH_MAX];

	strcpy(buffer, "/proc/mount");
	_report_mounts(buffer, PATH_MAX, strlen(buffer), flags);

	return 0;
}
