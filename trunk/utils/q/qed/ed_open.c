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





#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include "manif.h"
#include "struct.h"

#define EXT extern
#include "externs.h"

FILE *ed_open(char *fname, char *mode, char set_fattrs)
	/* get file attributes ? 1=yes, 0=no */
	{
	register FILE *fp;

	if(ed_fp) {
		fprintf( stderr,"Panic...ed_fp left open\n");
		exit( -1 );
		}

	unbreakable();
	fp = fopen(fname, mode);
	
	if(fp == 0) {
		if(openerrflag)
			putmsg("Unable to access file");
		breakable();
		return(0);
		}

	ed_fp = fp;

	if (set_fattrs)		
		get_fattrs(ed_fp);

	breakable();

	return(fp);
	}


void
ed_close() {
	register FILE *fp;

	if((fp = ed_fp) == 0) {
		fprintf( stderr,"panic...ed_fp not open\n");
		exit( -1 );
		}

	unbreakable();
	ed_fp = 0;
	errno = 0;
	fclose(fp);
	if(errno)	perror("Qed:");	breakable();
	}

void
get_fattrs( FILE *fp ) {
	struct stat buf;
	int n = fileno( fp );

	if ( (file_attrs = fstat( n, &buf ) ) == -1 )
		file_attrs = 0;
	else
		file_attrs = buf.st_mode;
	}
