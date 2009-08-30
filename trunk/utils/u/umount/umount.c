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
 Make this really slick at some point allowing you to umount from the
 /etc/fstab entries and such, but for now just get something ...

 Perhaps something like the bsd umount would be good, since we borrowed
 from their mount code initially
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/mount.h>

int main(int argc, char **argv) {
	int i, ret, flags, verbose;

	if (argc < 2) {
		printf("Usage: %s [-v] [-f] path_to_unmount [path ...] \n", argv[0]);
		return(1);
	}

	verbose = 0;
	flags = 0;
	while ((i = getopt(argc, argv, "fv")) != -1) {
		switch (i) {
		case 'f':
			flags |= _MOUNT_FORCE; 
			break;
		case 'v':
			verbose++;
			break;
		}
	}

	ret = 0;
	for (i=optind; i<argc; i++) {
		if (verbose) {
			printf("Attempting to umount: %s \n", argv[i]);
		}
		if (umount(argv[i], flags) == -1) {
			fprintf(stderr, "umount(%s) failed: %s \n", argv[i], strerror(errno));
			ret = 1;
		}
	}

	return(ret);
}
