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



#ifdef __MINGW32__
#include <lib/compat.h>
#endif

#include <unistd.h>
#include <time.h>
#include <sys/stat.h>
//#include <util/stdutil.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#ifndef __MINGW32__
#include <pwd.h>
#include <grp.h>
#endif
#ifdef __QNXNTO__
#include <libgen.h>
#endif

#ifdef WIN32
#include <fcntl.h>
#endif

#define SIX_MONTHS	((long) 6 * 30 * 24 * 60 * 60)  /* same as GNU */

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

#ifndef LIST_SEP
#  ifdef __MINGW32__
#    define LIST_SEP ';'
#  else
#    define LIST_SEP ':'
#  endif
#endif

char *PATH;		/* pointer to environment variable */
int every = 0;		/* find every occurence in PATH */
int full = 0;		/* expand full QNX pathname */
int ls = 0;		/* display pathname like ls -l */
int so = 0;		/* look for shared objects */
int symbolic = 0;	/* display contents of symbolic links */
static char *progname;

/* make permissions string from file mode */
char *str_mode(mode_t fmode) {
    static char perms[1+3+3+3+1];	/* static so we can return it */
    static char types[] = "?pc?dnb?-?l?s???";
    char *p = perms;

#if 0
    if (S_ISBLK(fmode))		*p++ = 'b';
    else if (S_ISCHR(fmode))	*p++ = 'c';
    else if (S_ISDIR(fmode))	*p++ = 'd';
    else if (S_ISLNK(fmode))	*p++ = 'l';
    else if (S_ISNAM(fmode))	*p++ = 'n';
    else if (S_ISFIFO(fmode))	*p++ = 'p';
    else if (S_ISSOCK(fmode))	*p++ = 's';
    else			*p++ = '-';
#else
    *p++ = types[(fmode >> 12) & 0xf];
#endif

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
	*p++ = (fmode & S_IXGRP) ? 's' : 'S';
    else
	*p++ = (fmode & S_IXGRP) ? 'x' : '-';

    /* Handle OTHER permissions */
    *p++ = (fmode & S_IROTH) ? 'r' : '-';
    *p++ = (fmode & S_IWOTH) ? 'w' : '-';
	if ( fmode & S_ISVTX )
	*p++ = (fmode & S_IXOTH) ? 't' : 'T';
	else
    *p++ = (fmode & S_IXOTH) ? 'x' : '-';

    *p = NULL;
    return perms;
}

/* return user name or user id string */
char *uid(uid_t uid) {
    struct passwd *pwd = getpwuid(uid);
    static char cuid[6];

    if (pwd) return pwd->pw_name;
    sprintf(cuid, "%5u", uid);
    return cuid;
}

/* return group name or group id string */
char *gid(gid_t gid) {
    struct group *grp = getgrgid(gid);
    static char cgid[6];

    if (grp) return grp->gr_name;
    sprintf(cgid, "%5u", gid);
    return cgid;
}

/* return the date formatted a la ls */
char *age(time_t then) {
    static time_t now = 0;	/* only evaluate time() once */
    static char date[28];	/* return buffer with date string */
    long stale;

    if (now == 0) time(&now);
    if ((stale = now - then) < 0)
	stale = -stale;
    strftime(date, sizeof date, stale < SIX_MONTHS ?
	"%h %d %H:%M" : "%h %d  %Y", localtime(&then));
    return date;
}

/* collapse multiple slashes in pathname */
char *collapse(char *file) {
    char *scan, *update;

    scan = update = file + 1;		/* skip possible network slash */
    do
	if (scan[0] == '/' && scan[1] == '/')
	    scan++;			/* skip one of the slashes */
    while ((*update++ = *scan++));
    return file;
}

#ifdef __MINGW32__
    // replace all '\\' with '/'
static void strreplacechar(char *part, char what, char with) {
	while (*part != '\0') {
	    if (*part == what)
			*part = with;
		++part;
	}
}
#endif

#ifndef WIN32
static int stat_executable(char *part, 	struct stat *pstb) {
	if (access(part, X_OK) == -1)
		return -1;
	return stat(part, pstb);
}
#else

static char const *find_first_of(char const *str, char const *what) {
	if (!str || !what)
		return 0;
		
	while (*str) {
		char const *pivot_what = what;
		while (*pivot_what) {
			if (*pivot_what == *str)
				return str;
			++pivot_what;
		} 
		++str;
	} 
	
	return 0;
}

static int stat_executable(char *part, struct stat *pstb) {
	const char *sep0;
	const char *sep1; /* list separator */
	
	int ret = stat(part, pstb);
	
	if (ret == -1) { /* not found; try with pathext env. var */
		char buff[PATH_MAX] = {'\0'}; /* temp. file name with appended ext. */
		int const partlen = strlen(part);
		int const extmax = PATH_MAX - partlen - 1;
		char * const bufend = buff + partlen; /* end of buff where ext. is to be copied*/
		
		if (extmax <= 0)
			return ret;
			
		strncpy(buff, part, PATH_MAX - 1);
		
		sep0 = getenv("PATHEXT");
		
		if (!sep0)
			return ret;
		
		while (ret && *sep0) {
			sep1 = find_first_of(sep0, ";:");
			if (!sep1) {
				strncpy(bufend, sep0, extmax);
				sep0 += strlen(sep0) - 1;
			} else {
				strncpy(bufend, sep0, (sep1 - sep0));
				bufend[sep1 - sep0] = '\0';
				sep0 = sep1;
			}
			ret = stat(buff, pstb);
			if (!ret) { // file found. copy to part:
				strcpy(part, buff);
				break;
			}
			++sep0;
		}
	} 
	
	return ret;
}
#endif

static void filetoolongerror() {
    fprintf(stderr, "%s: file name too long\n", progname);
	exit(EXIT_FAILURE);
}

int which(char *file) {
	//static char *path=NULL;
    char part[PATH_MAX*2+1], *pp, *p = PATH;
    struct stat stb;
    int places = 0;
    int maxlen;
    int partlen, filelen;

    filelen=strlen(file);

    /* scan through colon separated pathname components */
    do {
	/* find colon */
	if ((p = strchr(pp = p, LIST_SEP))) {
	    partlen = p - pp;
	    p++;
	} else {
	    partlen = strlen(pp);
	}
	
	if (*pp == LIST_SEP) // PATH starts with a LIST_SEP
	    continue;
	    
	memset(part, 0, sizeof(part));
	
	maxlen = partlen + filelen + 2; // 2 = 1 for terminating '\0' and 1 for '/' character
		
	if (maxlen > sizeof(part)) {
		filetoolongerror();
	}

	if (*pp) {
	    strncpy(part, pp, partlen);
	    strncat(part, "/", 1);		
	} 
	
	strncat(part, file, filelen);
	
#ifdef __MINGW32__
    // replace all '\\' with '/'
    strreplacechar(part, '\\', '/');
#endif

	if (stat_executable(part, &stb) == -1 || S_ISDIR(stb.st_mode))
	    continue;

	if (symbolic)
	    lstat(part, &stb);

	if (full)
#if !defined(__QNXNTO__) && !defined(__MINGW32__)
	    qnx_fullpath(part, part);
#else
		_fullpath(part, part, sizeof(part));
#endif
	if (!ls)
	    places++, puts(collapse(part));
	else {
		char	*fmt;
#if _FILE_OFFSET_BITS - 0 == 64
       	fmt = "%s %2u %-9s %-9s %8lld %s %s";
#else
		fmt = "%s %2u %-9s %-9s %8d %s %s";
#endif
	    places++, printf(fmt,
		str_mode(stb.st_mode), stb.st_nlink,
		uid(stb.st_uid), gid(stb.st_gid), stb.st_size,
		age(stb.st_mtime), part);
	    if (symbolic && S_ISLNK(stb.st_mode)) {
		char contents[PATH_MAX];

		contents[readlink(part, contents, sizeof contents)] = 0;
		printf(" -> %s", contents);
	    }
	    putchar('\n');
	}
	if (!every) break;		/* should we find more */
    } while (p);
    return places;
}

#ifdef __USAGE
%C - locate a program file (UNIX)

%C	[-aflL] program...
Options:
 -a     Find every occurrence.
 -f     Display the full QNX pathname.
 -l     Display in long format (like ls -lL).
 -L     Display symbolic links (like ls -l).
 -s     Search for shared objects using LD_LIBRARY_PATH and _CS_LIBPATH inplace of PATH.
#endif

int main(int argc, char *argv[]) {
    int ch;
    char *lib_path;
    int len = 0;
    int notfound = 0;

    progname = basename(argv[0]);
    while ((ch = getopt(argc, argv, "aflLs")) != -1) {
	switch (ch) {
	  case 'a': every++;	break;
	  case 'f': full++;	break;
	  case 'L': symbolic++;	/* fallthru */
	  case 'l': ls++;	break;
	  case 's': so++;	break;
	  default:  exit(EXIT_FAILURE);
	}
    }
    
#ifdef WIN32
	setmode(fileno(stdout), O_BINARY);
#endif

    /*
     * Fix for PR 13786: -s would look now in the LIBPATH.
     * I fixed it by appending LIBPATH with the LD_LIBRARY_PATH.
     * If none exists it will exit with an error.
     */
    
#ifndef __MINGW32__
// in mngw env., we do not get special path for the libraries, win32 looks for the libs in current dir and then
// in the PATH.    
    if (so)
    {
        char *ld_lib = getenv("LD_LIBRARY_PATH");
        int ld_lib_len = 0;
        
	len = confstr(_CS_LIBPATH,NULL,0); // len - buffer size required for the string and terminating '\0'
		
	if (ld_lib){
		ld_lib_len = strlen(ld_lib);
	}
        if (!len && !ld_lib){
		fprintf(stderr, "%s:  no PATH set\n", progname);
		exit(EXIT_FAILURE);
        }
        lib_path = (char*)malloc(len + ld_lib_len + 2); 
	if (ld_lib) {
		strncpy(lib_path, ld_lib, ld_lib_len);
		strncat(lib_path, ":", 1);
		ld_lib_len++;
	}
	if (confstr(_CS_LIBPATH,lib_path+ld_lib_len, len) > len) { 
			    /* if returned value is larger than our buf size (len)
			    * we are facing an unusual condition in which someone has changed the _CS_LIBPATH from the time
			    * we obtained 'len' to the time we obtained the string. We should retry or exit.
			    * We can not continue. 
			    */
		    	    /** exit with an error, we tried, but someone keeps changing the value!*/
			fprintf(stderr, "%s: Could not obtain _CS_LIBPATH\n", progname);
			exit(EXIT_FAILURE);
	}
	lib_path[len + ld_lib_len - 1] = '\0'; // we must terminate it in case of a string longer than expected
            
        PATH = lib_path;
    }  
    else { 
        if (getenv("PATH") == 0) {
	        fprintf(stderr, "%s:  no PATH set\n", progname);
	        exit(EXIT_FAILURE);
	    }
	    
	    len = strlen(getenv("PATH")) + 1;
	    
	    PATH = (char *) malloc(len);
	    PATH[len-1] = '\0';
	    strncpy(PATH, getenv("PATH"), len-1);
    }
#else // we are in __MINGW32__ environ.
    // prepend path with "." for the current working directory, windows always looks there first
    if (getenv("PATH"))
		len = strlen(getenv("PATH"));
	else
		len = 0; 
	len += 3; // for ".;\0"
    PATH = malloc(len);
    if (!PATH) {
		// out of memory!
		fprintf(stderr, "%s: out of memory\n", progname);
		exit(EXIT_FAILURE);
    }
    strncpy(PATH, ".", 2);
    if (getenv("PATH")) {
		strncat(PATH, ";", 2);
		strncat(PATH, getenv("PATH"), len - strlen(PATH));
	}
	//printf("%s\n%d - %d\n", PATH, len, strlen(PATH));
#endif // ifndef __MINGW32__
    
    argv = &argv[optind - 1];
    while (*++argv)
	if (which(*argv) == 0) {
	    fprintf(stderr, "%s: no %s in %s\n", progname, *argv, PATH);
	    notfound = 1;
	}   
	free(PATH);

    if (notfound)
	return EXIT_FAILURE;
    return(EXIT_SUCCESS);
}

