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





#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>

#ifdef __USAGE
%C - display network-qualified pathnames (QNX)

%C [-t]	[name ...]
Options:
 -t    Terse output (only output fully qualified path)

Note: If no filename is specified the full pathname of the current
      working directory is displayed.
#endif

#ifdef __USAGENTO
%C - display fully qualified pathnames (QNX)

%C [-v]	[name ...]
Options:
 -v    Verbose output (output source path and fully qualified path)

Note: If no filename is specified the full pathname of the current
      working directory is displayed.
#endif

static char *program = "$Id: fullpath.c 153052 2008-08-13 01:17:50Z coreos $";

int main(int argc, char **argv) {
    char fullpath[PATH_MAX];
    int errs = 0;
#ifdef __QNXNTO__
    int verbose = 0;
#else
    int verbose = 1;
#endif

    program = *argv;

    if (argc > 1 && argv[1][0] == '-' && argc--) {
		if((*++argv)[1] == 't') {
			verbose = 0;
		} else if((*argv)[1] == 'v') {
			verbose = 1;
		}
	}
		
    if (argc == 1) {
		argv[1] = strdup(getcwd(fullpath, sizeof fullpath));
		argv[2] = 0;
		argc++;
    }

    while (--argc && *++argv) {
#ifdef __QNXNTO__
		if (_fullpath(fullpath, *argv, sizeof fullpath)) {
#else
		if (qnx_fullpath(fullpath, *argv)) {
#endif
		    if (verbose) {
				printf("%s is ", *argv);
			}
		    printf("%s\n", fullpath);
		} else {
		    fprintf(stderr, "%s: %s: %s\n", program, strerror(errno), *argv);
		    ++errs;
		}
    }
    return errs;
}
