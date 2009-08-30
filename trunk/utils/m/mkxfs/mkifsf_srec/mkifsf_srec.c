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
%C - mkifs filter program for creating S-record boot images

%C	[-b] [-c] [-l] [-Lpaddr_loc,entry_loc] input-image-file output-srec-file

Options:
    -b    Only generate 4 byte address records.
    -c    Don't generate a carriage return at character end of line.
    -l    Don't generate a line feed character at end of line.
    -L    Reposition the physical address, entry point of the image.
#endif
*/

#include <lib/compat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/elf.h>
#include _NTO_HDR_(sys/startup.h)

#define MAX_RECLEN	32

#define SWAP32( val ) ( (((val) >> 24) & 0x000000ff)	\
					  | (((val) >> 8)  & 0x0000ff00)	\
					  | (((val) << 8)  & 0x00ff0000)	\
					  | (((val) << 24) & 0xff000000) )

int host_endian;
int target_endian;
int big_records;
int add_lf = 1;
int add_cr = 1;

long 
swap32(long val) {
	return(host_endian != target_endian ? SWAP32(val) : val);
}

struct loc {
	int			adjust;
	int			abs;
};

static void
get_loc(struct loc *loc) {
	switch(*optarg) {
	case ',':
		++optarg;
		// fall through
	case '\0':
		return;
	case '+':
	case '-':
		break;
	default:
		loc->abs = 1;
		break;
	}
	loc->adjust = strtoul(optarg, &optarg, 0);
	if(*optarg != '\0') ++optarg;
}

static unsigned
adj_loc(struct loc *loc, unsigned val) {
	if(loc->abs) val = 0;
	return(loc->adjust + val);
}

static void
putsrec(unsigned char *ibuf, unsigned n, unsigned long offset, FILE *fp, char base, int dir) {
	unsigned char	*op, obuf[MAX_RECLEN*2 + 16];
	unsigned		i, cnt, csum;
	static const char	hextable[] = "0123456789ABCDEF";

	if(big_records || (offset > 0xffff)) {
		op = obuf + sprintf(obuf, "S%c%2.2X%8.8lX", base + 2*dir, cnt = n + 5, offset);
	} else {
		op = obuf + sprintf(obuf, "S%c%2.2X%4.4lX", base,         cnt = n + 3, offset);
	}

	csum = 0;
	while(cnt) {
		csum += cnt & 0xff;
		cnt >>= 8;
	}
	while(offset) {
		csum += offset & 0xff;
		offset >>= 8;
	}

	for(i = 0; i < n; ++i) {
		unsigned char	byte = ibuf[i];

		csum += byte;
		*op++ = hextable[(byte >> 4) & 0xf];
		*op++ = hextable[byte & 0xf];
	}
	csum = ~csum;
	*op++ = hextable[(csum >> 4) & 0xf];
	*op++ = hextable[csum & 0xf];
	if(add_cr) *op++ = '\r';
	if(add_lf) *op++ = '\n';

	if(fwrite(obuf, op - obuf, 1, fp) != 1) {
		fprintf(stderr, "Error writing: %s\n", strerror(errno) );
		exit(1);
	}
}

int
main(int argc, char *argv[]) {
	int							n;
	FILE						*fp_in;
	FILE						*fp_out;
	struct startup_header		shdr;
	char						*name_in;
	char						*name_out;
	unsigned char				ibuf[MAX_RECLEN];
	unsigned long				offset;
	struct loc					paddr;
	struct loc					entry;

	// Calculate the endian of the host this program is running on.
	n = 1;
	host_endian = *(char *)&n != 1;

	memset(&paddr, 0, sizeof(paddr));
	memset(&entry, 0, sizeof(entry));
	while((n = getopt(argc, argv, "bclL:")) != -1) {
		switch(n) {
		case 'b':
			++big_records;
			break;
		case 'c':
			add_cr ^= 1;
			break;
		case 'l':
			add_lf ^= 1;
			break;
		case 'L':
			get_loc(&paddr);
			get_loc(&entry);
			break;
		default:
			exit(1);
		}
	}
	argc -= optind;
	argv += optind;

	if(argc < 2) {
		fprintf(stderr, "Missing file name(s).\n");
		return(1);
	}
	name_in = argv[0];
	name_out = argv[1];

	fp_in = fopen(name_in, "rb");
	if(fp_in == NULL) {
		fprintf(stderr, "Can not open '%s': %s\n", name_in, strerror(errno));
		return(1);
	}
	if(setvbuf(fp_in, NULL, _IOFBF, 16*1024) != 0) {
		fprintf(stderr, "Can not set buffer size for '%s': %s\n", name_in, strerror(errno));
		return(1);
	}
	if(fread(&shdr, sizeof(shdr), 1, fp_in) != 1) {
		fprintf(stderr, "Can not read from '%s': %s\n", name_in, strerror(errno));
		return(1);
	}
	if(fseek(fp_in, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Can not rewind '%s': %s\n", name_in, strerror(errno));
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

	target_endian = ((shdr.flags1 & STARTUP_HDR_FLAGS1_BIGENDIAN) != 0);

	fprintf(fp_out,"S0030000FC");
	if(add_cr) fprintf(fp_out, "\r");
	if(add_lf) fprintf(fp_out, "\n");
	offset = adj_loc(&paddr, swap32(shdr.image_paddr) + swap32(shdr.paddr_bias));
	while(n = fread(ibuf, 1, sizeof(ibuf), fp_in)) {
		putsrec(ibuf, n, offset, fp_out, '1', +1);
		offset += n;
	}
	putsrec(ibuf, 0, adj_loc(&entry, swap32(shdr.startup_vaddr)), fp_out, '9', -1);

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
__SRCVERSION("mkifsf_srec.c $Rev: 153052 $");
#endif
