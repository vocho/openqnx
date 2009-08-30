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
%C - mkifs filter to create a floppy image suitable for vmware.

%C	input-image-file output-floppy-image
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#define BLOCKSIZE 512

char *progname; 

unsigned
concat(char *infile, FILE *infp, char *outfile, FILE *outfp)
{
	unsigned bytes = 0;
	char buf[BLOCKSIZE];
	
	while(!feof(infp)){
		size_t i;
	
		i = fread(buf, 1, BLOCKSIZE, infp);

		if(ferror(infp)){
			fprintf(stderr, "%s: error reading from %s: %s\n",
				progname, infile, strerror(errno));
			return 0;
		}
		
		if(fwrite(buf, 1, i, outfp) < i){
			fprintf(stderr, "%s: unable to write to %s: %s\n",
				progname, outfile, strerror(errno));
			return 0;
		}
		bytes += i;
	}
	return bytes;
}

int
main(int argc, char *argv[])
{
	char stubfile[BLOCKSIZE], *infile="(stdin)", *outfile="(stdout)";
	FILE *stubfp, *infp = stdin, *outfp = stdout;
	char *qt = getenv("QNX_TARGET");
	int retval = 0;
	unsigned blocks, bytes;

	progname = argv[0];
	switch(argc){
		case 3:
			outfile = argv[2];
			if( !(outfp = fopen(outfile, "wb")) ){
				fprintf(stderr, "%s: unable to open %s for writing: %s\n",
					progname, outfile, strerror(errno));
				exit(1);
			}
			/* fall through */
		case 2:
			infile = argv[1];
			if( !(infp = fopen(infile, "rb")) ){
				fprintf(stderr, "%s: unable to open %s for reading: %s\n",
					progname, infile, strerror(errno));
				fclose(outfp);
				exit(1);
			}
			/* fall through */
		case 1:
			sprintf(stubfile, "%s/x86/boot/sys/vmware.stub", qt ? qt : "");
			if( !(stubfp = fopen(stubfile, "rb")) ){
				fprintf(stderr, "%s: unable to open %s for reading: %s\n",
					progname, stubfile, strerror(errno));
				fclose(infp);
				fclose(outfp);
				exit(1);
			}
			break;
		default:
			fprintf(stderr, "%s: too many arguments\n", progname);
			exit(1);
	}
	
	if(!concat(stubfile, stubfp, outfile, outfp)){
		retval = 1;
		goto cleanup;
	}

	if( !(bytes = concat(infile, infp, outfile, outfp)) ){
		retval = 1;
		goto cleanup;
	}

	blocks = (bytes + BLOCKSIZE - 1) / BLOCKSIZE;

	/* floppy has 2880 blocks of 512 bytes minus 8 blocks for the stub */
	if(blocks > 2872){
		fprintf(stderr, "%s: warning - output file larger than a regular floppy\n", progname);
	}
	
	/* use stubfile as a buffer for padding out the outfile */
	memset(stubfile, 0, BLOCKSIZE);
	fwrite(stubfile, 1, blocks * BLOCKSIZE - bytes, outfp);
	
	fseek(outfp, 0x298, SEEK_SET);
	fwrite(&blocks, sizeof(blocks), 1, outfp);

cleanup:
	switch(argc){
		case 3:
			fclose(outfp);
			/* fall through */
		case 2:
			fclose(infp);
			/* fall through */
		case 1:
			fclose(stubfp);
	}
	return retval;
}

#ifdef __QNXNTO__
__SRCVERSION("mkifsf_vmware.c $Rev: 153052 $");
#endif
