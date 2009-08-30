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




#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <confname.h>
#include <stdarg.h>

#include <cfgopen.h>

/*
 This file will open up a multitude of locations searching
 for configuration files.

 TODO:
  ->Support copying of a configuration file
  ->Support only getting the pathname back
*/

#define DIR_HOST 0 
#define DIR_USER 1 
static const char *_cfg_locations[] = { "/etc/host_cfg", ".cfg" };

/* 
 The search from most to least specific goes
 USER_HOST, USER, HOST, GLOBAL, historical
*/
static int getfd(char *path, int oflags, int mode, int flags, char *namebuf);
static int find_historical(char *buffer, int buflen, int oflags, int mode, int flags, char *namebuf, int nblen);
static char *append_path(char *buffer, int buflen, int copylen, const char *format, const char *name, ...);

int cfgopen(const char *name, unsigned flags, const char *historical, char *namebuf, int nblen) {
	char		*home, *cname, buffer[PATH_MAX]; 
	int			fd, oflags, mode, len;
	struct stat st;

	if (!name || !*name) {
		flags &= ~(CFGFILE_MASK);
	}
		
	oflags = flags & ~(CFGFILE_MASK);	/* Take away file options */
	mode   = 0644;						/* rw-r--r-- */

	/* 
     If we are opening a file for creation, or for any type
	 of writing, then we can only specify one of these flags. 
	 CREAT one could have the APPEND or TRUNC flag on to 
	 append or truncate the file.  
	*/
	if (flags & CFGFILE_CREAT || (flags & 0x3)) {
		int bits;

		bits = ((flags & CFGFILE_USER_NODE) ? 1 : 0) +
			   ((flags & CFGFILE_USER) ? 1 : 0) +
			   ((flags & CFGFILE_NODE) ? 1 : 0) +
			   ((flags & CFGFILE_GLOBAL) ? 1 : 0);
		if (bits > 1) {
			errno = EINVAL;
			return -1;
		}
	}

	/*
  	 Get the user's home directory and the hostname upfront.
	 If the hostname specific location doesn't exist, then
	 we collapse any "node" options to global options.
	*/
	home = getenv("HOME");

	len = confstr(_CS_HOSTNAME, NULL, 0) + 1;
	cname = alloca(len);
	if ((len = confstr(_CS_HOSTNAME, cname, len)) <= 0) {
		cname = getenv("HOSTNAME");
	}

	if (stat(_cfg_locations[DIR_HOST], &st) < 0) { 
		cname = NULL;
		if (flags & CFGFILE_USER_NODE) {
			flags &= ~CFGFILE_USER_NODE;
			flags |= CFGFILE_USER;
		}
		if (flags & CFGFILE_NODE) {
			flags &= ~CFGFILE_NODE;
			flags |= CFGFILE_GLOBAL;
		}
	}

	fd = -1, errno = ENOENT;

	/*
	 First we try the host/user combination
	*/
	if (flags & CFGFILE_USER_NODE && cname && home) {
		if (append_path(buffer, sizeof(buffer), (namebuf != NULL) ? nblen : 0, "%s/%s/%s", name, home, _cfg_locations[DIR_USER], cname) != NULL)
			fd = getfd(buffer, oflags, mode, flags, nblen ? namebuf : NULL);
		else
			errno = ENAMETOOLONG;
	}

	/*
	 If the user/machine combo failed then try just the user
	*/
	if (flags & CFGFILE_USER && fd == -1 && home) {
		if (append_path(buffer, sizeof(buffer), (namebuf != NULL) ? nblen : 0, "%s/%s", name, home, _cfg_locations[DIR_USER]) != NULL)
			fd = getfd(buffer, oflags, mode, flags, nblen ? namebuf : NULL);
		else
			errno = ENAMETOOLONG;
	}

	/*
	 If that doesn't work then look at the host configuration file
	*/
	if (flags & CFGFILE_NODE && fd == -1 && cname) {
		if (append_path(buffer, sizeof(buffer), (namebuf != NULL) ? nblen : 0, "%s/%s", name, _cfg_locations[DIR_HOST], cname) != NULL)
			fd = getfd(buffer, oflags, mode, flags, nblen ? namebuf : NULL);
		else
			errno = ENAMETOOLONG;
	}

	/*
	 If that fails then we will attempt to open up the file straight
	*/
	if (flags & CFGFILE_GLOBAL && fd == -1) {
		if (append_path(buffer, sizeof(buffer), (namebuf != NULL) ? nblen : 0, NULL, name) != NULL)
			fd = getfd(buffer, oflags, mode, flags, nblen ? namebuf : NULL);
		else
			errno = ENAMETOOLONG;
	}

	/*
	 If we are still failing then we will fall back to the historical
	*/
	if (historical && *historical && fd == -1) {
		if (append_path(buffer, sizeof(buffer), 0, NULL, historical) != NULL)
			fd = find_historical(buffer, sizeof(buffer), oflags, mode, flags & ~CFGFILE_CREAT, namebuf, nblen);
		else
			errno = ENAMETOOLONG;
	}

	return fd;	
}

/*
 For those people using FILE *'s ...
*/
FILE *fcfgopen(const char *filename, const char *mode, int location, const char *historical, char *namebuf, int nblen) {
        int                                        fd;
        int                                        oflag;
        FILE                                       *fp;
        int                                        __parse_oflag(const char *mode);

        if((oflag = __parse_oflag(mode)) == -1) {
                errno = EINVAL;
                return 0;
        }
        if((fd = cfgopen(filename, location | oflag, historical, namebuf, nblen)) == -1) {
                return 0;
        }
        if((fp = fdopen(fd, mode))) {
                return fp;
        }
        close(fd);
        return 0;
}

/*
 This string is delimited by colons to indicate the historical fields.  The
 string can also contain the following special meta characters to cause the
 fields to be dynamically updated:
 %N -> confstr(_CS_HOSTNAME)	Historically nodename and hostname were different
 %H -> confstr(_CS_HOSTNAME)
*/
static int replace_str(char *str, char *item, char *repl, int slen) {
	char *p;
	int  cplen, ilen, elen;

	ilen = strlen(item);
	cplen = (repl) ? strlen(repl) : 0; 

	while ((p = strstr(str, item))) {
		elen = strlen(p+ilen) + 1;

		if (((p - str) + elen + cplen) > slen) {
			return -1;
		}

		memmove(p+cplen, p+ilen, elen);
		if (cplen > 0) {
			memcpy(p, repl, cplen);
		}
	}

	return 0;
}

static int find_historical(char *buffer, int buflen, int oflags, int mode, int flags, char *namebuf, int nblen) {
	char  *hname, *p, *last;
	int   len;

	len = confstr(_CS_HOSTNAME, NULL, 0);
	hname = alloca(len + 1);
	if ((len = confstr(_CS_HOSTNAME, hname, len)) <= 0) {
		hname = NULL;
	}

	/* First go through and make all of the substitutions */
	if (replace_str(buffer, "%N", hname, buflen) || replace_str(buffer, "%H", hname, buflen)) {
		errno = ENAMETOOLONG;
		return(-1);
	}

	/* Then strtok the hell out of it trying to get the fd */
	len = -1, errno = ENOENT;
	for (p = strtok_r(buffer, ":", &last); p; p = strtok_r(NULL, ":", &last)) {
		if (namebuf != NULL && nblen != 0 && nblen <= strlen(p))
			errno = ENAMETOOLONG;
		else if ((len = getfd(p, oflags, mode, flags & ~CFGFILE_CREAT, nblen ? namebuf : NULL)) != -1)
			break;
	}

	return len;
}

/*
Iterate through the given path creating all of the intermediate 
directory entries as required with the specified dirmode.
*/
static int makepath(char *path, int dirmode) {
	char cnt, *p;

	cnt = 0;
	p = (*path == '/') ? path+1: path;

	for (p = strchr(p, '/'); p && *(p+1) != '\0'; p = strchr(p+1, '/')) {
		*p = '\0';
		if (mkdir(path, dirmode) == -1) {
			switch(errno) {
			case EEXIST:
			case EPERM:
			case EACCES:
				break;
			default:
				return -1;
			}
		} else {
			cnt++;
		}
		*p = '/';
	}

	return cnt;
}

/*
 Open up the file as specified, create the path if necessary.
*/
static int getfd(char *path, int oflags, int mode, int flags, char *namebuf) {
	int fd;

	// If we create with NOFD, then we don't actually do anything
	if (flags & CFGFILE_NOFD) {
		if (flags & CFGFILE_CREAT) { 
			if (namebuf != NULL) {
				strcpy(namebuf, path);
			}
			return 0;
		}
		fd = access(path, (flags & 0x3) ? W_OK : R_OK); 
	} else {
		fd = open(path, oflags, mode);
	}

	if (fd != -1 || !(flags & CFGFILE_CREAT)) {
		if (namebuf != NULL) {
			strcpy(namebuf, path);
		}
		return fd;
	}

	if (makepath(path, 0775) < 0) {
		return -1;
	}

	return getfd(path, oflags, mode, flags & ~CFGFILE_CREAT, namebuf);
}

static char *append_path(char *buffer, int buflen, int copylen, const char *format, const char *name, ...)
{
va_list		args;
int			len;

	if (format != NULL) {
		va_start(args, name);
		len = vsnprintf(buffer, buflen, format, args);
		va_end(args);
		if (len >= buflen)
			return(NULL);
		if (*name != '/') {
			if (++len >= buflen)
				return(NULL);
			strcat(buffer, "/");
		}
		if ((len += strlen(name)) >= buflen)
			return(NULL);
		strcat(buffer, name);
	}
	else {
		if ((len = strlen(name)) >= buflen)
			return(NULL);
		strcpy(buffer, name);
	}
	if (copylen != 0 && copylen <= len)
		return(NULL);
	return(buffer);
}

#if defined(TEST) 
int main(int argc, char **argv) {
	char buffer[PATH_MAX];
	int fd;
	FILE *fp;

	/* Test the open for reading */
	fd = cfgopen("/etc/testcfg", CFGFILE_USER_NODE, NULL, NULL, 0); 
	if (fd != -1) {
		printf("USER_HOST passed \n");
	} else {
		printf("USER_HOST failed \n");
	}
	fd = cfgopen("/etc/testcfg", CFGFILE_USER, NULL, NULL, 0); 
	if (fd != -1) {
		printf("USER passed \n");
	} else {
		printf("USER failed \n");
	}
	fd = cfgopen("/etc/testcfg", CFGFILE_NODE, NULL, NULL, 0);
	if (fd != -1) {
		printf("HOST passed \n");
	} else {
		printf("HOST failed \n");
	}
	fd = cfgopen("/etc/testcfg", CFGFILE_GLOBAL, NULL, NULL, 0);
	if (fd != -1) {
		printf("GLOBAL passed \n");
	} else {
		printf("GLOBAL failed \n");
	}
	fd = cfgopen("/etc/testcfg", 0, "/etc/testcfg", NULL, 0);
	if (fd != -1) {
		printf("HIST 1 passed \n");
	} else {
		printf("HIST 1 failed \n");
	}
	fd = cfgopen("/etc/testcfg", 0, "/etc/testcfg:/etc/newcfg", NULL, 0);
	if (fd != -1) {
		printf("HIST 2 passed \n");
	} else {
		printf("HIST 2 failed \n");
	}
	fd = cfgopen("/etc/testcfg", 0, "/etc/testcfg.%N:/etc/%H/newcfg", NULL, 0);
	if (fd != -1) {
		printf("HIST 3 passed \n");
	} else {
		printf("HIST 3 failed \n");
	}


	/* Test the preffered opening */
	fd = cfgopen("/etc/testcfg", CFGFILE_USER_NODE | CFGFILE_USER, NULL, NULL, 0); 
	if (fd != -1) {
		printf("USER_HOST/USER passed \n");
	} else {
		printf("USER_HOST/USER failed \n");
	}
	fd = cfgopen("/etc/testcfg", CFGFILE_USER_NODE | CFGFILE_USER | CFGFILE_NODE, NULL, NULL, 0); 
	if (fd != -1) {
		printf("USER_HOST/USER/HOST passed \n");
	} else {
		printf("USER_HOST/USER/HOST failed \n");
	}
	fd = cfgopen("/etc/testcfg", CFGFILE_USER_NODE | CFGFILE_USER | CFGFILE_NODE | CFGFILE_GLOBAL, NULL, NULL, 0); 
	if (fd != -1) {
		printf("USER_HOST/USER/HOST/GLOBAL passed \n");
	} else {
		printf("USER_HOST/USER/HOST/GLOBAL failed \n");
	}
	fd = cfgopen("/etc/testcfg", CFGFILE_USER_NODE | CFGFILE_USER | CFGFILE_NODE | CFGFILE_GLOBAL, "/etc/testcfg", NULL, 0); 
	if (fd != -1) {
		printf("USER_HOST/USER/HOST/GLOBAL/HIST passed \n");
	} else {
		printf("USER_HOST/USER/HOST/GLOBAL/HIST failed \n");
	}

	fd = cfgopen("/etc/testcfg", CFGFILE_USER_NODE | CFGFILE_USER | CFGFILE_NODE | CFGFILE_GLOBAL, "/etc/testcfg.%H:/etc/host_cfg/%N/etc/testcfg", NULL, 0); 
	if (fd != -1) {
		printf("USER_HOST/USER/HOST/GLOBAL/HIST+ passed \n");
	} else {
		printf("USER_HOST/USER/HOST/GLOBAL/HIST+ failed \n");
	}

	/* Test the creation of files, truncated to zero */
	fd = cfgopen("/etc/testcfg", CFGFILE_USER | CFGFILE_NODE | CFGFILE_RDWR | CFGFILE_CREAT | CFGFILE_TRUNC, NULL, NULL, 0); 
	if (fd != -1) {
		printf("BAD \n");
	} else {
		printf("GOOD \n");
	}
	fd = cfgopen("/etc/testcfg", CFGFILE_USER | CFGFILE_RDWR | CFGFILE_CREAT | CFGFILE_TRUNC, NULL, NULL, 0); 
	if (fd != -1) {
		printf("User config create passed \n");
	} else {
		printf("User config create failed \n");
	}


	fp = fcfgopen("/etc/testcfg", "r", CFGFILE_USER, NULL, buffer, PATH_MAX);
	if (fp) {
		printf("User config create passed (FILE) \n");
		printf("User config file is [%s] \n", buffer);
	} else {
		printf("User config create failed (FILE) \n");
	}



	return 0;
}
#endif

__SRCVERSION("cfgopen.c $Rev: 159804 $");
