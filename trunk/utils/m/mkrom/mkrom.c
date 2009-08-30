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





#ifdef __USAGE
%C - make a BIOS extension ROM image (QNX)

%C file
#endif
/*
	Makes a binary image into a ROM BIOS extension
*/

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

static char *progname = "mkrom";

typedef struct {
	unsigned short	signature;
	unsigned char	length;
	unsigned char	entry[2];
	unsigned char	checksum;
} rom_header;

int sum (void *buf, size_t size) {
	unsigned char *b = buf;
	int s = 0;

	while (size--) s += *b++;
	return s;
}

int main (int argc, char **argv) {
	char		*target, *buf;
	int			target_fd;
	rom_header	*head;
	size_t		size, promoted_size;

	if (argc != 2) {
		fprintf (stderr, "%s:  no file.\n", progname);
		exit (EXIT_FAILURE);
	}
	target = argv[1];
	target_fd = open (target, O_RDWR | O_BINARY );
	if (target_fd == -1) {
		fprintf (stderr, "%s:  unable to open %s (%s).\n", progname, target, strerror (errno));
		exit (EXIT_FAILURE);
	}
	size = lseek (target_fd, 0L, SEEK_END);
	lseek (target_fd, 0L, SEEK_SET);
	promoted_size = (size + 511) & ~((size_t) 511);
	if (promoted_size > INT_MAX) {
		fprintf (stderr, "%s:  boot image too large.\n", progname);
		exit (EXIT_FAILURE);
	}
	buf = malloc (promoted_size);
	if (buf == NULL) {
		fprintf (stderr, "%s:  out of memory.\n", progname);
		exit (EXIT_FAILURE);
	}
	memset (buf, 0xFF, promoted_size);	// use 0xff to speed up rom burn
	if (read (target_fd, buf, size) != size) {
		fprintf (stderr, "%s:  error reading %s (%s).\n", progname, target, strerror (errno));
		exit (EXIT_FAILURE);
	}
	head = (void *) buf;
	head->signature = 0xAA55;
	head->length = promoted_size / 512;
	head->entry[0] = 0xEB;			/* jmp */
	head->entry[1] = 0x01;			/* 6 */
	head->checksum = 0;
	head->checksum = 0 - sum (buf, promoted_size);
	lseek (target_fd, 0L, SEEK_SET);
	if (write (target_fd, buf, promoted_size) != promoted_size) {
		fprintf (stderr, "%s:  unable to write image (%s).\n", progname, strerror (errno));
		exit (EXIT_FAILURE);
	}
	close (target_fd);
	free (buf);
	return 0;
}
