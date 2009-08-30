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





#ifdef __USAGE		/* head.c */
%C - copy the first part of files (POSIX)

%C	[-c|-l] [-n number] [file ...]
Options:
 -c           Measure output in bytes.
 -l           Measure output in lines. (default)
 -n number    Output <number> lines (bytes if -c is specified).
#endif

/*				----------------------------------------

 *				----------------------------------------
 *+
 *-----------------------------------------------------------------------------
 * Filename:	head.c
 *
 * Author:		Jeffrey G. George
 *
 * Change		 By			  Date			Reason for Change
 * 0			JGG			29-Sep-89		First Release
 * 1            GKM         17-Jul-90       need_usage() added
 * 2			GKM			16-Apr-91		fixed to allow -number (-20) for
 *											offset
 *-----------------------------------------------------------------------------
 * System :	Posix Utility Set
 *
 * Usage:	head [-c|-l] [-n number] [filename]
 *
 * Description:
 *			The head utility copies its input files to the standard output
 *			ending the output for each file at a designated point.                             
 *			Copying ends at the point in each input file indicated by the -n
 *			option.  The argument number is counted in units of lines or bytes,
 *			according to the options -l and -c.
 *
 * Note:	See POSIX 1003.2 Draft 9 Section 4.60.  
 *
 * Args:
 *			-c							The quantity of output is in bytes
 *			-l							The quantity of output is in lines
 *			-n	number					The number of lines (default) or bytes
 *										output.  It is an unsigned decimal
 *										integer.
 *			filename					Name of input file
 *
 */
 
#ifdef __MINGW32__
#include <lib/compat.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <ctype.h>
#include <sys/stat.h>
char	buff[ LINE_MAX ];

void head( FILE *fp, off_t offset, unsigned byte_flag)	
{
	off_t	count = 0;
	int		inchar;

	while( ( count < offset )  &&  !feof( fp ) ) {
		if ( byte_flag ) {
			if ((inchar=fgetc(fp))!=EOF) fprintf(stdout,"%c",inchar);
		} else {
			if (fgets(buff,LINE_MAX,fp)) fprintf(stdout,"%s",buff);
		}
		count++;
	}
}


int main( int argc, char **argv )
{
	int		error, byte_offset, line_offset, i, first, multiple;
	off_t 	offset;
	FILE	*fp;
        struct stat s;
	
	error = byte_offset = line_offset = multiple = 0;
	offset = 10;	/* default */
	first  = 1;

	opterr = 0;
	while( ( i = getopt( argc, argv, "lcn:" ) ) != -1 ) {
		switch( i ) {
			case 'l':	line_offset = 1;						break;
			case 'c':	byte_offset = 1;						break;
#if _FILE_OFFSET_BITS - 0 == 64
			case 'n':	offset = strtoll( optarg, NULL, 10 );	break;
#else
			case 'n':	offset = strtol( optarg, NULL, 10 );	break;
#endif
			default:
				if (isdigit(optopt)) offset=strtol(argv[optind-1]+1,NULL,10);
				else {
					fprintf( stderr, "head:  Unknown option '%c'\n", optopt );
					exit( EXIT_FAILURE );
				}
		}
	}

	/* If neither -c or -l is specified, the default is a line offset */
	if ( !byte_offset  &&  !line_offset )	line_offset = 1;

	if ( optind >= argc ) {
		fp = stdin;
		head( fp, offset, byte_offset );
	} else {
		multiple = argc - optind - 1;
		for( ; optind < argc; optind++ ) {
			if (strcmp(argv[optind],"-")) {
                            stat(argv[optind], &s);
				if ( !( fp = fopen( argv[ optind ], "r" ) ) ) {
					perror( "head" );
					exit( EXIT_FAILURE );
				}
			} else fp=stdin;

			if ( multiple )
				fprintf( stdout, "%s==> %s <==\n", (first) ? "":"\n", (fp==stdin)?"--stdin--":argv[optind] );

			first = 0;
                        stat(argv[optind], &s);
                        if (!S_ISDIR(s.st_mode)) {
			  head( fp, offset, byte_offset );
                        }
                        else
                          fprintf(stderr,"head:  unable to open directory '%s'\n", argv[optind]);
			if (fp!=stdin) fclose( fp );
		}
	}
	return EXIT_SUCCESS;
}

