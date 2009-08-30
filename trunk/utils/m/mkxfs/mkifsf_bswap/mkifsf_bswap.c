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
#ifdef __USAGE
%C - mkifs filter program for swapping byte order in images

%C input-image-file output-image-file
#endif
*/

#include <stdio.h>
#include <errno.h>
#include <string.h>

void
swap(char *p, int n, FILE *fp)
{
	char	*t = p;
	int		i;
	
	for (i = 0; i < n/4; i++, t += 4) {
	}
}

int
main(int argc, char *argv[])
{
	FILE			*fp_in;
	FILE			*fp_out;
	char			*name_in;
	char			*name_out;
	int				n;
	unsigned char	buf[4];

	if (argc <= 2) {
		fprintf(stderr, "Missing file name(s).\n");
		return 1;
	}
	name_in  = argv[1];
	name_out = argv[2];

	if ((fp_in = fopen(name_in, "rb")) == NULL) {
		fprintf(stderr, "Can not open '%s': %s\n", name_in, strerror(errno));
		return 1;
	}
	if(setvbuf(fp_in, NULL, _IOFBF, 16*1024) != 0) {
		fprintf(stderr, "Can not set buffer size for '%s': %s\n", name_in, strerror(errno));
		return(1);
	}

	if ((fp_out = fopen(name_out, "wb")) == NULL) {
		fprintf(stderr, "Can not open '%s': %s\n", name_out, strerror(errno));
		return 1;
	}
	if(setvbuf(fp_out, NULL, _IOFBF, 16*1024) != 0) {
		fprintf(stderr, "Can not set buffer size for '%s': %s\n", name_out, strerror(errno));
		return(1);
	}

	while (n = fread(buf, 1, sizeof(buf), fp_in)) {
		unsigned char	c;

		c = buf[0]; buf[0] = buf[3]; buf[3] = c;
		c = buf[1]; buf[1] = buf[2]; buf[2] = c;
		if (fwrite(buf, sizeof(buf), 1, fp_out) != 1) {
			fprintf(stderr, "Error writing: %s\n", strerror(errno));
		}
	}

	if (fclose(fp_out) != 0) {
		fprintf(stderr, "Error closing '%s': %s\n", name_out, strerror(errno));
		return 1;
	}

	if (fclose(fp_in) != 0) {
		fprintf(stderr, "Error closing '%s': %s\n", name_in, strerror(errno));
		return 1;
	}

	return 0;
}

#ifdef __QNXNTO__
__SRCVERSION("mkifsf_bswap.c $Rev: 153052 $");
#endif
