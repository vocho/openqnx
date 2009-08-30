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




#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef __USAGE
%C  [-i] [envar=value]* [command]
Where:
 -i           Ignore the environment that would be inherited from the shell.
 envar=value  Set the environment value.
 command      A command to be envoked.
#endif

#ifdef __MINGW32__
#define ENOTSUP 48
#endif

int
main(int argc, char **argv) {
    extern char **environ;

    while (*++argv) {
		if (strcmp(*argv, "-") == 0 || strcmp(*argv, "-i") == 0)
		    *environ = 0;
		else if (strchr(*argv, '=') != 0)
		    putenv(*argv);
		else {
			if (-1==execvp(*argv,argv)) {
				if (errno==ENOTSUP)
					spawnvp(2,*argv,argv);	/* Try spawn; 2 == P_OVERLAY */
			}
		    perror(*argv);
		    exit(1);
		}
    }

    /*  No command given, just print out the environment  */
    while (*environ)
	puts(*environ++);

    return 0;
}
