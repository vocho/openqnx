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





/* $Source$
 *
 * $Revision: 153052 $
 *
 * cpio.c - Cpio specific functions for archive handling
 *
 * DESCRIPTION
 *
 *	These function provide a cpio conformant interface to the pax
 *	program.
 *
 * AUTHOR
 *
 *     Mark H. Colburn, NAPS International (mark@jhereg.mn.org)
 *
 * Sponsored by The USENIX Association for public distribution. 
 *
 * Copyright (c) 1989 Mark H. Colburn.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice is duplicated in all such 
 * forms and that any documentation, advertising materials, and other 
 * materials related to such distribution and use acknowledge that the 
 * software was developed * by Mark H. Colburn and sponsored by The 
 * USENIX Association. 
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Log$
 * Revision 1.4  2005/06/03 01:37:53  adanko
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.3  2003/08/27 18:16:57  martin
 * Add QSSL Copyright to cover QNX contributions.
 *
 * Revision 1.2  1999/08/05 18:46:38  adrianj
 * Correct printf call for correct types.
 *
 * Revision 1.1  1998/12/03 18:54:43  eric
 * Initial revision
 *
 * Revision 1.2  89/02/12  10:04:13  mark
 * 1.2 release fixes
 * 
 * Revision 1.1  88/12/23  18:02:05  mark
 * Initial revision
 * 
 */

#ifndef lint
static char *ident = "$Id: cpio.c 153052 2008-08-13 01:17:50Z coreos $";
static char *copyright = "Copyright (c) 1989 Mark H. Colburn.\nAll rights reserved.\n";
#endif /* ! lint */


/* Headers */

#include "pax.h"


/* Function Prototypes */

#ifdef __STDC__
static void 	usage(void);
#else /* !__STDC__ */
static void 	usage();
#endif /* __STDC__ */


/* do_cpio - handle cpio format archives
 *
 * DESCRIPTION
 *
 *	Do_cpio provides a standard CPIO interface to the PAX program.  All
 *	of the standard cpio flags are available, and the behavior of the
 *	program mimics traditonal cpio.
 *
 * PARAMETERS
 *
 *	int	argc	- command line argument count
 *	char	**argv	- pointer to command line arguments
 *
 * RETURNS
 *
 *	Nothing.
 */

#ifdef __STDC__

void do_cpio(int argc, char **argv)

#else

int do_cpio(argc, argv)
int             argc;
char          **argv;

#endif
{
    int             c;
    char           *dirname;
    Stat            st;

    /* default input/output file for CPIO is STDIN/STDOUT */
    ar_file = "-";
    names_from_stdin = 1;

    /* set up the flags to reflect the default CPIO inteface. */
    blocksize = BLOCKSIZE;
    ar_interface = CPIO;
    ar_format = CPIO;
    msgfile=stderr;

    while ((c = getopt(argc, argv, "D:Bacdfilmoprtuv")) != EOF) {
	switch (c) {
	case 'i':
	    f_extract = 1;
	    break;
	case 'o':
	    f_create = 1;
	    break;
	case 'p':
	    f_pass = 1;
	    dirname = argv[--argc];

	    /* check to make sure that the argument is a directory */
	    if (LSTAT(dirname, &st) < 0) {
		fatal(strerror());
	    }
	    if ((st.sb_mode & S_IFMT) != S_IFDIR) {
		fatal("Not a directory");
	    }
	    break;
	case 'B':
	    blocksize = BLOCK;
	    break;
	case 'a':
	    f_access_time = 1;
	    break;
	case 'c':
	    break;
	case 'D':
	    ar_file = optarg;
	    break;
	case 'd':
	    f_dir_create = 1;
	    break;
	case 'f':
	    f_reverse_match = 1;
	    break;
	case 'l':
	    f_link = 1;
	    break;
	case 'm':
	    f_mtime = 1;
	    break;
	case 'r':
	    f_interactive = 1;
	    break;
	case 't':
	    f_list = 1;
	    break;
	case 'u':
	    f_unconditional = 1;
	    break;
	case 'v':
	    f_verbose = 1;
	    break;
	default:
	    usage();
	}
    }

    if (f_create + f_pass + f_extract != 1) {
	usage();
    }
    if (!f_pass) {
	buf_allocate((OFFSET) blocksize);
    }
    if (f_extract) {
	open_archive(AR_READ);	/* Open for reading */
	read_archive();
    } else if (f_create) {
	open_archive(AR_WRITE);
	create_archive();
    } else if (f_pass) {
	pass(dirname);
    }

    /* print out the total block count transfered */
    fprintf(stderr, "%ld Blocks\n", (long)ROUNDUP(total, BLOCKSIZE) / BLOCKSIZE);
    
    exit(0);
    /* NOTREACHED */
}


/* usage - print a helpful message and exit
 *
 * DESCRIPTION
 *
 *	Usage prints out the usage message for the CPIO interface and then
 *	exits with a non-zero termination status.  This is used when a user
 *	has provided non-existant or incompatible command line arguments.
 *
 * RETURNS
 *
 *	Returns an exit status of 1 to the parent process.
 *
 */

#ifdef __STDC__

static void usage(void)

#else

static void usage()

#endif
{
    fprintf(stderr, "Usage: %s -o[Bacv]\n", myname);
    fprintf(stderr, "       %s -i[Bcdmrtuvf] [pattern...]\n", myname);
    fprintf(stderr, "       %s -p[adlmruv] directory\n", myname);
    exit(1);
}
