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
 *
 * Uue -- encode a file so that it's printable ascii, short lines
 *
 * Slightly modified from a version posted to net.sources a while back,
 * and suitable for compilation on the IBM PC
 *
 * modified for Lattice C on the ST - 11.05.85 by MSD
 * modified for ALCYON on the ST -    10-24-86 by RDR
 * modified a little more for MWC...  02/09/87 by JPHD
 * (An optional first argument of the form: -nnumber (e.g. -500), will
 * produce a serie of files that long, linked by the include statement,
 * such files are automatically uudecoded by the companion program.)
 * More mods, - ...		   05/06/87 by jphd
 * Mods for TOPS 20, and more.     08/06/87 by jphd
 *     (remove freopen and rindex...change filename generation...)
 * (A lot more to do about I/O speed, avoiding completely the stdio.h...)
 *
 */


#ifdef __USAGE
%-uue
%C - uuencode (Berzerkeley)

%C [-n] inputfile [-]

%-uuencode
%C - ASCII encode file to stdout (POSIX)

uuencode [-m] [file] name
  -m      use base64 encoding, otherwise uuencode Historical Algorithm

  [file]  file to encode, default stdin
  name    name to embed in encoded file for use by uudecode
#endif

#include <uub64.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>

#define USAGE 

/* ENC is the basic 1 character encoding function to make a char printing */
#define ENC(c) (((c) & 077) + ' ')

void outdec(register char *p);
void maketable(void);
void makename(void);
void encode(void);

extern FILE  *fopen();
FILE *outp;
char ofname[80];
int lenofname;
int stdo = 0;

#ifdef ST
#define READ "rb"
#else
#define READ "r"
#endif

int part = 'a', chap = 'a';
#define SEQMAX 'z'
#define SEQMIN 'a'
char seqc = SEQMAX;

int split = 0, fileln = 32000;

int uuencode;

extern  char *__progname; /* program name, from crt0.o */

void
usage(void) {
	if (uuencode) {
		fprintf(stderr, "Usage: uuencode [infile] outfile\n");
	}
	else {
		fprintf(stderr, "Almost foolproof uuencode v3.1 06 Aug 1987\n");
		fprintf(stderr, "Usage: uue [-n] inputfile [-]\n");
	}

	exit(2);
}

int
main(int argc, char **argv)
{
	char *fname;
	int op, base64;
	mode_t mode, omask;
	struct stat st;

	base64 = 0;


	if (strcmp(__progname, "uuencode") == 0) {
		uuencode = 1;
	}

	if (uuencode) {
		while ((op = getopt(argc, argv, "m")) != -1) {
			switch (op) {
			case 'm':
				base64 = 1;
				break;
			default:
				usage();
			}
		}
		argv += optind;
		argc -= optind;

		outp = stdout;

		switch (argc) {
		case 2:
			if (freopen(argv[0], "r", stdin) == NULL ||
			    fstat(fileno(stdin), &st) == -1) {
				fprintf(stderr, "unable to open %s: %s\n",
				    argv[0], strerror(errno));
				return 1;
			}
			mode = st.st_mode & (S_IRWXU|S_IRWXG|S_IRWXO);
			argv++;
			break;

		case 1:
			omask = umask(0);
			mode = (S_IRUSR|S_IWUSR|S_IRGRP|S_IWGRP|S_IROTH|S_IWOTH) & ~omask;
			umask(omask);
			break;

		default:
			usage();
			return 1; /* notreached */
		}
	}
	else {
		if (argc < 2) {
			usage();
		}
		argv++;
		argc--;
		if (argv[0][0] == '-') {
			fileln = -atoi(argv[0]);
			if (fileln <= 0) {
				fprintf(stderr, "Wrong file length arg.\n");
				exit(1);
			}
			split = 1;
			argv++;
			argc--;
		}
		if (freopen(argv[0], READ, stdin) == NULL) {  /* binary input !!! */
			fprintf(stderr,"Cannot open %s\n",argv[0]);
			exit(1);
		}
		strcpy(ofname, argv[0]);
		fname = ofname;
		do {
			if (*fname == '.')
				*fname = '\0';
		} while (*fname++);
			/* 8 char prefix + .uue -> 12 chars MAX */
		lenofname = strlen(ofname);
		if (lenofname > 8) ofname[8] = '\0';
		strcat(ofname,".uue");
		lenofname = strlen(ofname);
		if (!split && (argc > 1) && (argv[1][0] == '-')) {
			stdo = 1;
			outp = stdout;
		 } else {
			 makename();
			 if((outp = fopen(ofname, "w")) == NULL) {
				 fprintf(stderr,"Cannot open %s\n", ofname);
				 exit(1);
				 }
		}
		maketable();
		mode = 0644;
	}
	if (base64 == 0) {
		fprintf(outp,"begin %o %s\n", mode, argv[0]);
		encode();
		fprintf(outp,"end\n");
	}
	else {
		fprintf(outp, "begin-base64 %o %s\n", mode, *argv);
		base64_encode(outp);
		fprintf(outp, "====\n");
	}
	fclose(outp);
	return(0);
}

/* create ASCII table so a mailer can screw it up and the decode
 * program can restore the error.
 */
void maketable(void)
{
	register int i, j;

	fputs("table\n", outp);
	for(i = ' ', j = 0; i < '`' ; j++) {
		if (j == 32)
			putc('\n', outp);
		fputc(i++, outp);
	}
	putc('\n', outp);
}

/*
 * Generate the names needed for single and multiple part encoding.
 */
void makename(void)
{
	if (split) {
		ofname[lenofname - 1] = part;
		ofname[lenofname - 2] = chap;
	}
}

/*
 * copy from in to out, encoding as you go along.
 */
void encode(void)
{
	char buf[80];
	register int i, n;
	register int lines;
	lines = 6;

	for (;;) {
		n = fread(buf, 1, 45, stdin);
		putc(ENC(n), outp);
		for (i = 0; i < n; i += 3)
		      outdec(&buf[i]);
		if (!uuencode) {
			putc(seqc, outp);
			seqc--;
			if (seqc < SEQMIN) seqc = SEQMAX;
		}
		putc('\n', outp);
		++lines;
		if (split && (lines > fileln)) {
			part++;
			if (part > 'z') {
				part = 'a';
				if (chap == 'z')
					chap = 'a'; /* loop ... */
				else
					chap++;
			}
			makename();
			fprintf(outp,"include %s\n",ofname);
			fclose(outp);
			if((outp = fopen(ofname, "w")) == NULL) {
				fprintf(stderr,"Cannot open %s\n",ofname);
				exit(1);
			}
			maketable();
			fprintf(outp,"begin part %c %s\n",part,ofname);
			lines = 6;
		}
		if (n <= 0)
			break;
	}
	if (ferror(stdin)) {
		fprintf(stderr, "%s: read error: %s.", __progname, strerror(errno));
		exit(1);
	}
}

/*
 * output one group of 3 bytes, pointed at by p, on file f.
 */
void outdec(register char *p)
{
	register int c1, c2, c3, c4;

	c1 = *p >> 2;
	c2 = (*p << 4) & 060 | (p[1] >> 4) & 017;
	c3 = (p[1] << 2) & 074 | (p[2] >> 6) & 03;
	c4 = p[2] & 077;
	putc(ENC(c1), outp);
	putc(ENC(c2), outp);
	putc(ENC(c3), outp);
	putc(ENC(c4), outp);
}
