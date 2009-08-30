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





#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/debug.h>
#include <sys/link.h>

#define MAX_ENVIRON 0x1000

char *
findExecutable(const char *name, char *buf)
{
    char *lps, *s;
    char path[PATH_MAX];
    char *cp = getenv("PATH");

    buf[0] = '\0';

    lps = cp;

    while (lps && *lps) {
        struct stat sbuf;
        char *pp, *lp;

        /* copy library path element to path buffer */
        for (pp = &path[0]; *lps != '\0'; lps++) {
            if (*lps == ':') {
                lps++;
                break;
            }
            if (*lps == '/' && *(lps+1) == '/') continue;
            *pp++ = *lps;
        }
        *pp = '\0';
        
        if (*path == '\0') continue;

        strcat(path,"/");
        strcat(path,name);
        
        if (stat(path, &sbuf) == 0 && S_ISREG(sbuf.st_mode)) {
            return buf;
        }
    }
    return NULL;
}

void
usage()
{
    fprintf(stderr, "Usage: mtrace [-o <outfile> ] [ -l <traceback-level> ] <prog-name> <argument-list>\n");
    exit(1);
}

static char *newenv[MAX_ENVIRON+1];
static char executable[PATH_MAX];
static char mtrace_output[PATH_MAX+14];
static char mtrace_level[32];

main(int argc, char **argv, char **envp)
{
    char **nenvp, **envpp;
    int narg;
    char *prog;
    char *outfile = NULL;
    char *level = NULL;

    if (argc < 2) {
	usage();
    }

    narg = 1;
    if (strncmp(argv[narg], "-o", 2) == 0) {
	if (strlen(argv[narg]) == 2) {
	    narg++;
	    outfile = argv[narg];
	} else {
	    outfile = argv[narg];
	    outfile += 2; /* skip over the -f option */
	}
	narg++;
    }
    if (strncmp(argv[narg], "-l", 2) == 0) {
	if (strlen(argv[narg]) == 2) {
	    narg++;
	    level = argv[narg];
	} else {
	    level = argv[narg];
	    level += 2; /* skip over the -l option */
	}
	narg++;
    }

    if (argc < (narg+1)) {
	usage();
    }

    if (*argv[narg] != '/') {
	char *cp = strchr(argv[narg], '/');
	if (cp != NULL) {
            struct stat sbuf;
            if (stat(argv[narg], &sbuf) == 0 && S_ISREG(sbuf.st_mode)) {
        	char *cp2 = argv[narg];
        	char pwd[PATH_MAX+1];
        	char *cwd = getcwd(pwd, PATH_MAX+1);
        	if (cwd == NULL) {
		    fprintf(stderr, "Can't get current working directory: %s\n", argv[narg]);
		    exit(1);
        	}
        	strcpy(executable, cwd);
        	if (*cp2 == '.' && *(cp2+1) == '/') {
        	    strcat(executable, cp2+1);
        	} else {
        	    strcat(executable, argv[narg]);
        	}
            } else {
		fprintf(stderr, "Can't execute: %s\n", argv[narg]);
		exit(1);
            }
	} else {
	    if (findExecutable(argv[narg], executable) == NULL) {
		fprintf(stderr, "Can't execute: %s\n", argv[narg]);
		exit(1);
	    }
	}
    } else {
	char *ep, *ap;
	for (ep = &executable[0], ap = argv[narg]; *ap; ep++, ap++) *ep = *ap;
    }

    /* Construct new environment */
    for (nenvp = &newenv[0], envpp = envp; *envpp; nenvp++, envpp++) {
	*nenvp = *envpp;
    }
    strcpy(mtrace_output, "MALLOC_TRACE=");
    if (!outfile) {
	outfile = "mtrace.log";
    }
    /* clobber existing output file */
    if (access(outfile, R_OK|W_OK) == 0) {
	unlink(outfile);
    }
    strcat(mtrace_output, outfile);
    *nenvp++ = mtrace_output;
    if (level) {
	strcpy(mtrace_level, "MALLOC_TRACE_LEVEL=");
	strcat(mtrace_level, level);
	*nenvp++ = mtrace_level;
    }
    *nenvp = NULL;

    execve(executable, &argv[narg], &newenv[0]);
    fprintf(stderr, "Error from execve(): %s\n", strerror(errno));
}
