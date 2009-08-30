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

/*
 * This program tests the ability to set the maximum number of
 * file descriptors.
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>



int
main(int argc, char **argv)
{
	struct rlimit lim;
	int x, i, euid = geteuid();

	x = getrlimit(RLIMIT_NOFILE, &lim);
	if (x == 0) {
		printf("Num fd's soft max: %d\n", lim.rlim_cur);
		printf("Num fd's hard max: %d\n", lim.rlim_max);
	} else {
		printf("getrlimit failed (%s)!\n", strerror(errno));
		printf("bailing out of %s test.\n", argv[0]);
		exit(5);
	}

	/* test: max > hard */
	lim.rlim_max += 1;
	x = setrlimit(RLIMIT_NOFILE, &lim);
	if (x != 0) {
		if (euid == 0) {
			printf("setting max > hard limit should have succeeded (euid "
				   "is 0).  Err: %s\n", strerror(errno));
			errno = 0;
		}
	} else if (euid != 0) {
			printf("setting max > hard limit should have failed (euid "
				   "is %d, not 0).\n", euid);
	}

	/* test: cur > max */
	lim.rlim_max -= 1;
	lim.rlim_cur  = lim.rlim_max + 1;
	x = setrlimit(RLIMIT_NOFILE, &lim);
	if (x == 0) {
		printf("setting cur > hard limit should have failed!\n");
	}

	/* get the current limits again */
	getrlimit(RLIMIT_NOFILE, &lim);
	
	/* test: max < hard && max > soft/cur */
	lim.rlim_cur -= 1;
	lim.rlim_max  = lim.rlim_cur + 1;
	x = setrlimit(RLIMIT_NOFILE, &lim);
	if (x != 0) {
		printf("setting max < hard limit failed (%s)\n", strerror(errno));
		errno = 0;
	}

	/* get the current limits again */
	getrlimit(RLIMIT_NOFILE, &lim);

	/* test: max < soft/cur && max < hard */
	lim.rlim_max = lim.rlim_cur - 1;
	x = setrlimit(RLIMIT_NOFILE, &lim);
	if (x == 0) {
		printf("setting max < soft/cur limit should have failed "
			   "(max %d cur %d)\n", lim.rlim_max, lim.rlim_cur);
	}
	
	/* get the current limits again */
	getrlimit(RLIMIT_NOFILE, &lim);

	/* test: cur > soft && cur < max */
	lim.rlim_cur = lim.rlim_max - 1;
	x = setrlimit(RLIMIT_NOFILE, &lim);
	if (x != 0) {
		printf("setting cur > soft limit should have succeeded (%s)\n", 
			   strerror(errno));
		errno = 0;
	}
	
	/* get the current limits again */
	getrlimit(RLIMIT_NOFILE, &lim);

	/* test: cur < soft && cur < max */
	lim.rlim_cur = lim.rlim_cur - 1;
	x = setrlimit(RLIMIT_NOFILE, &lim);
	if (x != 0) {
		printf("setting cur < soft limit should have succeeded (%s)\n", 
			   strerror(errno));
		errno = 0;
	}

	printf("End of RLIMIT_NOFILE test. (no printfs == success)\n");
	return 0;
}

__SRCVERSION("fdmax.c $Rev: 153052 $");
