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
%C	[option | imagefilename] ...

Options:
 -a alignment       Force next file to be aligned (default: 1).
 -f format          Output format (srec | intel | binary | full default: srec).
 -l reclen          Length of data bytes per line (default: 32)
 -o offset          Offset in hex (default: 0).
 -r                 Suppress reset vector record.
 -s romsize[K|M|G]  Sizeof of ROM in decimal (default: 4G).
#endif


#include <lib/compat.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>

#define MAX_RECLEN		255


typedef unsigned char uchar;

struct out_fmt {
	char	*name;
	void	(*init)(void);
	void	(*rec)(const uchar *, unsigned);
	void	(*fini)(void);
};


uchar		 hextable[] = "0123456789ABCDEF";
int			 resetvec = 1;
unsigned	 reclen = 32;
unsigned	 alignment = 0;
unsigned	 offset = 0;
unsigned	 lastoffset = 0;
unsigned	 romsize;
unsigned	 usedsize;
unsigned	 numfiles, curfile;
struct out_fmt	*format;
char		*imagename;
uchar		 resetrec[16];
uchar		 noprec[2] = { 0x90, 0x90 };


static uchar
chksum(const uchar *buf, unsigned nbytes, unsigned type, unsigned cnt, unsigned off) {
	uchar sum = type + cnt;

	while(nbytes--) {
		sum += *buf++;
	}

	while(off) {
		sum += off & 0xff;
		off >>= 8;
	}

	return(sum & 0xff);
}

static unsigned 
str2size(char *str) {
	unsigned	n;
	char		*stop;

	n = strtoul(str, &stop, 16);
	if(*stop == 'k'  ||  *stop == 'K')
		n = strtoul(str, NULL, 10) * 1024;
	else if(*stop == 'm'  ||  *stop == 'M')
		n = strtoul(str, NULL, 10) * 1024 * 1024;
	else if(*stop == 'g'  ||  *stop == 'G')
		n = strtoul(str, NULL, 10) * 1024 * 1024 * 1024;

	return(n);
}

static void
output(const uchar *data, unsigned size) {
	//NYI: should check for errors on write
	fwrite(data, 1, size, stdout);
}

static void
data(const void *data, unsigned len) {
	format->rec(data, len);
	offset += len;
	usedsize += len;
	if((romsize != 0) && (usedsize > romsize)) {
		fprintf(stderr, "Data size of 0x%x exceeds ROM size of 0x%x\n", usedsize, romsize);
		exit(EXIT_FAILURE);
	}
}

static void 
pad(unsigned len) {
	uchar			ibuf[MAX_RECLEN];
	unsigned		n, s;

	memset(ibuf, 0xff, sizeof ibuf);
	for(n = len; n; n -= s) {
		s = sizeof ibuf;
		if(s > n) s = n;
		data(ibuf, s);
	}
}

static void
nop(void) {
}

static void
srec_init(void) {
	if(curfile == 1) {
		#define SREC_TERM "S00600004844521B\r\n"
		output(SREC_TERM, sizeof(SREC_TERM)-1);
	}
}

static void
srec_fini(void) {
	if(curfile == numfiles) {
		#define SREC_INIT "S9030000FC\r\n"
		output(SREC_INIT, sizeof(SREC_INIT)-1);
	}
}

static void
srec_rec(const uchar *ibuf, unsigned n) {
	uchar		*op, obuf[MAX_RECLEN*2 + 16];
	unsigned	i, cnt, csum;

	if(offset > 0xffff) {
		op = obuf + sprintf(obuf, "S3%2.2X%8.8X", cnt = n + 5, offset);
	} else {
		op = obuf + sprintf(obuf, "S1%2.2X%4.4X", cnt = n + 3, offset);
	}

	for(i = 0 ; i < n ; ++i) {
		*op++ = hextable[(ibuf[i] >> 4) & 0xf];
		*op++ = hextable[ibuf[i] & 0xf];
	}
	csum = ~chksum(ibuf, n, 0, cnt, offset);
	*op++ = hextable[(csum >> 4) & 0xf];
	*op++ = hextable[csum & 0xf];
	*op++ = '\r';
	*op++ = '\n';

	output(obuf, op - obuf);
}

static void
ihex_fini(void) {
	if(curfile == numfiles) {
		#define IHEX_FINI ":00000001FF\r\n"
		output(IHEX_FINI, sizeof(IHEX_FINI)-1);
	}
}

// Use the Intel 32 bit format which supports the Extended linear address record.
// Hex code 04
static void
ihex_rec(const uchar *ibuf, unsigned n) {
	uchar			*op, obuf[MAX_RECLEN*2 + 18];
	unsigned		i, cnt, csum;
	static unsigned	lba;

	if((offset & 0xffff0000) != (offset+n & 0xffff0000)  &&  (offset+n & 0xffff)) {
		i = 0x10000 - (offset & 0xffff);
		ihex_rec(ibuf, i);
		ihex_rec(ibuf + i, n - i);
		return;
	}

	if(lba != (offset & 0xffff0000)) {
		lba = offset & 0xffff0000;
		sprintf(obuf, ":02000004%4.4X", i = lba >> 16);
		csum = -chksum((uchar *)&i, 4, 0x02, 0x04, 0);
		obuf[13] = hextable[(csum >> 4) & 0xf];
		obuf[14] = hextable[csum & 0xf];
		obuf[15] = '\r';
		obuf[16] = '\n';
		output(obuf, 17);
	}

	op = obuf + sprintf(obuf, ":%2.2X%4.4X00", cnt = n, offset & 0xffff);

	for(i = 0 ; i < n ; ++i) {
		*op++ = hextable[(ibuf[i] >> 4) & 0xf];
		*op++ = hextable[ibuf[i] & 0xf];
	}
	csum = -chksum(ibuf, n, 0, cnt, offset & 0xffff);
	*op++ = hextable[(csum >> 4) & 0xf];
	*op++ = hextable[csum & 0xf];
	*op++ = '\r';
	*op++ = '\n';

	output(obuf, op - obuf);
}

static void
bin_init(void) {
	if(numfiles != 1) {
		fprintf(stderr, "You may specify a single file with the -f bin format.\n");
		exit(1);
	}
}

static void
fbin_init(void) {
	pad(offset - lastoffset);
}

static void
fbin_fini(void) {
	if((curfile == numfiles) && (offset < romsize)) {
		pad(romsize - offset);
	}
}

static void
bin_rec(const uchar *ibuf, unsigned len) {
	output(ibuf, len);
}

struct out_fmt fmts[] = {
	{"srec",	srec_init,	srec_rec,	srec_fini },
	{"intelhex",nop,		ihex_rec,	ihex_fini },
	{"binary",	bin_init,	bin_rec,	nop },
	{"fullbin",	fbin_init,	bin_rec,	fbin_fini },
//	{"omf386",	omf_init,	omf_rec,	omf_fini },
};


static void 
process_file(FILE *fp) {
	unsigned	 padsize=0;
	uchar		ibuf[MAX_RECLEN];
	unsigned	n;

	//
	// This code is tricky since it has to work on a regular 16 bit startup
	// and a 32 bit Natsemi startup. In the regular case the jmp opcode is
	// 1 byte with a 2 byte offset. In the 32 bit case it is a 4 byte offset.
	// Since the relative offset is calculated from the next opcode we have to
	// do more than extend the offset with 0xffff. The solution is to always
	// jump assuming a 32 bit startup and pad the beginning of the image
	// with a nop nop to catch control since we went back a tad too far.
	//
	if(resetvec && (curfile == numfiles)) {
		unsigned		imagesize;
		unsigned		rvect;
		unsigned short	disp;
		struct stat		sbuf;

		stat(imagename, &sbuf);
		imagesize = sbuf.st_size;
		resetrec[0] = 0xe9;
		rvect = (romsize ? romsize-16 : ~0-15);
		offset = (rvect - imagesize) - 2;
		padsize = offset & 15;
		offset &= (~0-15);		// Fix for amdsc400
		fflush(stdout);
		disp = -((rvect - offset) + 3);
		resetrec[1] = disp & 0xff;
		resetrec[2] = disp >> 8;
		fprintf(stderr, "Reset jmps to 0x%X (jmp 0x%X)\n", offset, disp);

		resetrec[3] = resetrec[4] = 0xff;
	} else {
		offset = (offset + alignment) & ~alignment;
	}

	//printf("Offset = %u\n", offset);

	format->init();

	if(resetrec[0]) data(noprec, sizeof(noprec));

	while(n = fread(ibuf, 1, reclen, fp)) {
		data(ibuf, n);
	}

	pad(padsize);

	if(resetrec[0]) data(resetrec, sizeof(resetrec));

	format->fini();

	lastoffset = offset;
}

int 
main(int argc, char *argv[]) {
	FILE		*fp;
	int			c;
	unsigned	i;

	// Find out how many files were specified.
	while (optind < argc) {
		switch(getopt(argc, argv, "a:f:l:o:rs:")) {
		case 'f':
			if(format != NULL) {
				fprintf(stderr, "Only one format allowed on the command line\n");
				exit(EXIT_FAILURE);
			}
			i = 0;
			for( ;; ) {
				if(i >= sizeof(fmts)/sizeof(fmts[0])) {
					fprintf(stderr, "Illegal format: %s\n", optarg);
					exit(EXIT_FAILURE);
				}
				if(fmts[i].name[0] == optarg[0]) break;
				++i;
			}
			format = &fmts[i];
			break;

		case -1:
			++numfiles;
			++optind;
			break;
		}
	}

	// No format specified, use default (SREC)
	if(format == NULL) format = &fmts[0];

	// Process arguments and files in order.
#if defined(__QNX__) && !defined(__QNXNTO__)
	optind = 0;
#else
	optind = 1;
#endif

	MAKE_BINARY_FP(stdout);

	while (optind < argc) {
		c = getopt(argc, argv, "a:f:l:o:rs:");
		switch(c) {
		case 'a':
			alignment = str2size(optarg) - 1;
			break;

		case 'f':
			// Handled above
			break;

		case 'l':
			reclen = strtoul(optarg, NULL, 0);
			if(reclen > MAX_RECLEN) reclen = MAX_RECLEN;
			break;

		case 'o':
			offset = str2size(optarg);
			break;

		case 'r':
			resetvec = 0;
			break;

		case 's':
			romsize = str2size(optarg);
			break;

		case -1:
			++curfile;
			if((fp = fopen(imagename = argv[optind++], "rb")) == NULL) {
				fprintf(stderr, "%s : %s\n", argv[optind - 1], strerror(errno));
				exit(EXIT_FAILURE);
			}

			process_file(fp);
			break;

		case '?':
			fprintf(stderr, "Unknown option.\n");
			exit(1);
		}
	}

	if(numfiles == 0) {
		curfile = numfiles = 1;
		imagename = "-stdin-";
		MAKE_BINARY_FP(stdin);
		process_file(stdin);
	}

	return(0);
}
