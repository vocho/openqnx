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
%C - mkifs filter program for creating Opah boot images

%C	input-image-file output-srec-file
#endif

#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

int
main(int argc, char *argv[]) {
	FILE						*fp_in;
	FILE						*fp_out;
	char						*name_in;
	char						*name_out;
	struct stat					sbuf;
	int							data;

	if(argc <= 2) {
		fprintf(stderr, "Missing file name(s).\n");
		return(1);
	}
	name_in = argv[1];
	name_out = argv[2];

	fp_in = fopen(name_in, "rb");
	if(fp_in == NULL) {
		fprintf(stderr, "Can not open '%s': %s\n", name_in, strerror(errno));
		return(1);
	}
	if(setvbuf(fp_in, NULL, _IOFBF, 16*1024) != 0) {
		fprintf(stderr, "Can not set buffer size for '%s': %s\n", name_in, strerror(errno));
		return(1);
	}
	fp_out = fopen(name_out, "wb");
	if(fp_out == NULL) {
		fprintf(stderr, "Can not open '%s': %s\n", name_out, strerror(errno));
		return(1);
	}
	if(setvbuf(fp_out, NULL, _IOFBF, 16*1024) != 0) {
		fprintf(stderr, "Can not set buffer size for '%s': %s\n", name_out, strerror(errno));
		return(1);
	}
	if(fstat(fileno(fp_in), &sbuf) == -1) {
		fprintf( stderr, "Can not stat '%s': %s\n", name_in, strerror(errno) );
		return(1);
	}

	fprintf(fp_out,"%08lX",(unsigned long)sbuf.st_size);		// build file header (size)
	while((data=getc(fp_in)) != EOF) {
		char	ch;

		ch = data >> 4;
		if(ch > 9) {
			putc(ch + ('A' - 10), fp_out);
		} else {
			putc(ch + '0', fp_out);
		}
		ch = data & 0x0f;
		if(ch > 9) {
			putc(ch + ('A' - 10), fp_out);
		} else {
			putc(ch + '0', fp_out);
		}
	}

	if(fclose(fp_out) != 0) {
		fprintf( stderr, "Error closing '%s': %s\n", name_out, strerror(errno));
		return(1);
	}
	if(fclose(fp_in) != 0) {
		fprintf(stderr, "Error closing '%s': %s\n", name_in, strerror(errno));
		return(1);
	}
	return(0);
}

#ifdef __QNXNTO__
__SRCVERSION("mkifsf_opah.c $Rev: 153052 $");
#endif
