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
%C - enable/disable terminal writes

%C	[y][n]
#endif

/*-
 * msg.c: [dis]allow or query terminal writes.
 */
#include <libc.h>
#include <sys/stat.h>


#define EXIT_WRITABLE      0
#define EXIT_NOTWRITABLE   1
#define EXIT_ERROR         2

#define WMODE (S_IWGRP | S_IWOTH)


int
main(int argc, char **argv)
{
	struct stat st;
	int         c;
	mode_t      m;
	char       *tty;
#ifdef __QNXNTO__
	char *tmp;
	int fd;
#endif
	while ((c=getopt(argc, argv, "")) != -1)
		exit(1);
	if (!(tty = ctermid(0))) {
		fprintf(stderr,"%s: must be run on a tty\n", argv[0]);
		return EXIT_ERROR;
	}
	if (stat(tty, &st) < 0) {
		fprintf(stderr,"%s: cannot stat '%s' (%s)\n",
			argv[0], tty, strerror(errno));
		return EXIT_ERROR;
	}
#ifdef __QNXNTO__
	/* this is here because on NTO ctermid() returns '/dev/tty'
	 * so we need to figure out which terminal it _really_ is */
	if( (fd = open(tty, O_RDONLY)) == -1 ){
		fprintf(stderr,"%s: cannot open %s (%s)\n",
			argv[0], tty, strerror(errno));
		return EXIT_ERROR;
	}

	if( !(tmp = ttyname(fd)) ){
		fprintf(stderr,"%s: cannot get name of tty from %s (%s)\n",
			argv[0], tty, strerror(errno));
		return EXIT_ERROR;
	}
	tty = tmp;

	if( fstat(fd, &st) < 0){
		fprintf(stderr,"%s: cannot stat '%s' (%s)\n",
			argv[0], tty, strerror(errno));
		return EXIT_ERROR;
	}

	close(fd);
#endif
	m = st.st_mode;
	if (argc == optind) {
		if (m & S_IWGRP) {
			fputs("is y\n", stderr);
			return EXIT_WRITABLE;
		}
		fputs("is n\n", stderr);
		return EXIT_NOTWRITABLE;
	}
	switch(*argv[optind]) {
	case 'y':
		if (chmod(tty, m | WMODE) == -1) {
			break;
		}
		return EXIT_WRITABLE;
	case 'n':
		if (chmod(tty, m & ~WMODE) == -1) {
			break;
		}
		return EXIT_NOTWRITABLE;
	default:
		fprintf(stderr, "usage: %s [y|n]\n", argv[0]);
		return EXIT_ERROR;
	}
	fprintf(stderr, "%s: cannot change mode of %s (%s)\n",
			 argv[0], tty, strerror(errno));
	return EXIT_ERROR;
}
