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
 * Uud -- decode a uuencoded file back to binary form.
 *
 * From the Berkeley original, modified by MSD, RDR, JPHD & WLS.
 * The Atari GEMDOS version compiled with MWC 2.x.
 * The MSDOS version with TurboC.
 * The Unix version with cc.
 * this version is made: 25 Nov 1988.
 */

#ifdef __USAGE
%-uue
%C - uudecode (Berzerkeley)

%C [-n] [-d] [-s dir] [-t dir] input-file
Options:
 -n       No line sequence check
 -d       Debug/verbose mode
 -s dir   Source directory for all input files
          (MUST be terminated by directory separator)
 -t dir   Target directory for all output files
          (MUST be terminated by directory separator)
Note:
 If input-file is - then stdin is used as input-file.
%-uudecode
%C - decode an ASCII encoded file

uudecode [-p | -o outfile] [file ...]
 -p          decode file to stdout, otherwise original file is recreated
 -o outfile  decode to 'outfile', otherwise original file is recreated.

 [file ...]  file(s) to decode, default is to decode from stdin
#endif

#define uuformat printf

/*
 * Be sure to have the proper symbol at this point. (GEMDOS, MSDOS, UNIX...)
 */
/*
#ifndef GEMDOS
#define GEMDOS 1
#endif
 */
#ifndef UNIX
#define UNIX 1
#endif
/*
#ifndef MSDOS
#define MSDOS 1
#endif
 */

#ifdef GEMDOS
#define SYSNAME "gemdos"
#define SMALL 1
#endif
#ifdef MSDOS
#define SYSNAME "msdos"
#define SMALL 1
#endif
#ifdef UNIX
#define SYSNAME "unix"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <limits.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>
#include <err.h>
#include <uub64.h>

#ifdef GEMDOS
#include <osbind.h>
#define Error(n)  { Bconin(2); exit(n); }
#define WRITE	  "wb"
#else
#define Error(n)  exit(n)
#define WRITE	  "w"
#endif

char *getnword(char *, int);

#define MAXCHAR 256
#define LINELEN 256
#define FILELEN PATH_MAX 
#define NORMLEN 60	/* allows for 80 encoded chars per line */

#define SEQMAX 'z'
#define SEQMIN 'a'
char seqc;
int first, secnd, check, numl;
int uudecode, pflag;

FILE *in, *out;
char *pos;
char ifnamebuf[FILELEN], *ifname;
char *source = NULL, *target = NULL;
char blank, part = '\0';
int partn, lens;
int debug = 0, nochk = 0, onedone = 0;
int chtbl[MAXCHAR], cdlen[NORMLEN + 3];

void usage(void);
void gettable(void);
int decode(void);
void getfile(char *buf);
int doit(char *);


extern  char *__progname; /* program name, from crt0.o */

void
usage(void)
{
	if (!uudecode) {
		fprintf(stderr, "Almost foolproof uudecode v3.4 (%s) 25-Nov-88\n\n",
			SYSNAME);
		fprintf(stderr, "Usage: uud [-n] [-d] [-s dir] [-t dir] input-file\n\n");
		fprintf(stderr, "Option: -n -> No line sequence check\n");
		fprintf(stderr, "Option: -d -> Debug/verbose mode\n");
		fprintf(stderr, "Option: -s + Source directory for all input files\n");
		fprintf(stderr, "  (MUST be terminated by directory separator)\n");
		fprintf(stderr, "Option: -t + Target directory for all output files\n");
		fprintf(stderr, "  (MUST be terminated by directory separator)\n");
		fprintf(stderr, "If input-file is - then stdin is used as input-file\n");
	}
	else {
		fprintf(stderr, "Usage: uudecode [-p] [-o outfile] [file ...]\n");
	}
	exit(1);
}

int main(int argc, char *argv[])
{
	int ret;
	register int i, j, op;
	char *curarg, *fname;

	if (strcmp(__progname, "uudecode") == 0) {
		uudecode = 1;
	}

	fname = NULL;

	if (uudecode) {
		in = stdin; /* default */
		ifname = "<stdin>";
		while ((op = getopt(argc, argv, "po:")) != -1) {
		        switch (op) {
			case 'p':
				pflag = 1;
				break;

			case 'o':
				/* Posix (until we actually have /dev/stdout) */
				if (strcmp(optarg, "/dev/stdout") == 0)
					pflag = 1;
				else
					fname = optarg;
				break;

			default:
				usage();
			}
		}
		argc -= optind;
		argv += optind;
		goto UUDECODE_ARGSDONE;
	}

	/* Old uue arg processing */
	if (argc < 2) {
		usage();
	}

	curarg = argv[1];
	
	while (curarg[0] == '-') {
		if (((curarg[1] == 'd') || (curarg[1] == 'D')) &&
		    (curarg[2] == '\0')) {
			debug = 1;
		} else if (((curarg[1] == 'n') || (curarg[1] == 'N')) &&
			   (curarg[2] == '\0')) {
			nochk = 1;
		} else if (((curarg[1] == 't') || (curarg[1] == 'T')) &&
			   (curarg[2] == '\0')) {
			argv++;
			argc--;
			if (argc < 2) {
				errx(15, "Missing target directory.\n");
			}
			target = argv[1];
			if (debug)
				uuformat("Target dir = %s\n",target);
		} else if (((curarg[1] == 's') || (curarg[1] == 'S')) &&
			   (curarg[2] == '\0')) {
			argv++;
			argc--;
			if (argc < 2) {
				errx(15, "Missing source directory.\n");
			}
			source = argv[1];
			if (debug)
				uuformat("Source dir = %s\n",source);
		} else if (curarg[1] != '\0') {
			errx(15, "Unknown option <%s>\n", curarg);
		} else
			break;
		argv++;
		argc--;
		if (argc < 2) {
			errx(15, "Missing file name.\n");
		}
		curarg = argv[1];
	}

	if ((curarg[0] == '-') && (curarg[1] == '\0')) {
		in = stdin;
		ifname = "<stdin>";
	} else {
		if (source != NULL) {
			if (strlen(source) + strlen(curarg) >= sizeof(ifnamebuf)) {
				errno = ENAMETOOLONG;
				err(2, "%s%s", source, curarg);
			}	
			strncpy(ifnamebuf, source, sizeof(ifnamebuf) - 1);
			strncat(ifnamebuf, curarg, sizeof(ifnamebuf) - strlen(source) - 1);
		} else {
			if (strlen(curarg) >= sizeof(ifnamebuf)) {
				errno = ENAMETOOLONG;
				err(2, "%s", curarg);
			}	
			strncpy(ifnamebuf, curarg, sizeof(ifnamebuf) - 1);
		}
		ifname = ifnamebuf;
		if ((in = fopen(ifname, "r")) == NULL) {
			err(2, "Can't open %s", ifname);
		}
		numl = 0;
	}

UUDECODE_ARGSDONE:
/*
 * Set up the default translation table.
 */
	for (i = 0; i < ' '; i++) chtbl[i] = -1;
	for (i = ' ', j = 0; i < ' ' + 64; i++, j++) chtbl[i] = j;
	for (i = ' ' + 64; i < MAXCHAR; i++) chtbl[i] = -1;
	chtbl['`'] = chtbl[' '];	/* common mutation */
	chtbl['~'] = chtbl['^'];	/* an other common mutation */
	blank = ' ';
/*
 * set up the line length table, to avoid computing lotsa * and / ...
 */
	cdlen[0] = 1;
	for (i = 1, j = 5; i <= NORMLEN; i += 3, j += 4)
		cdlen[i] = (cdlen[i + 1] = (cdlen[i + 2] = j));


	if (!uudecode || *argv == NULL)
		exit(doit(fname));

	ret = 0;

	do {
		ifname = *argv++;
		if (freopen(ifname, "r", in) == NULL) {
			fprintf(stderr, "%s: unable to open %s: %s\n",
			    __progname, ifname, strerror(errno));
			ret = 1;
		}
		else {
			ret |= doit(fname);
		}
	} while (*argv != 0);

	exit(ret);
}



int
doit(char *fname)
{
	mode_t mode;
	char *p, *ofname, buf[_POSIX_PATH_MAX];
	int base64, ret;


	base64 = 0;

/*
 * search for header or translation table line.
 */
	do {	/* master loop for multiple decodes in one file */
		partn = 'a';
		for (;;) {
			if (fgets(buf, sizeof buf, in) == NULL) {
				if (onedone) {
					if (debug) uuformat("End of file.\n");
					return 0;
				} else {
					warnx("No \"begin\" %sline.\n", uudecode ?
					    "or \"begin-base64\" " : "");
					return 1;
				}
			}
			numl++;
			if (!uudecode && strncmp(buf, "table", 5) == 0) {
				gettable();
				continue;
			}
			p = buf;

			if (uudecode && strncmp(p, "begin-base64", 12) == 0) {
				base64 = 1;
				p += 13;
				break;
			}

			if (strncmp(p, "begin", 5) == 0) {
				p += 6;
				break;
			}
		}

		/* format is "begin <mode> <ofname>" */
		mode = strtol(p, &ofname, 8);
		if (ofname == (p) || !isspace(*ofname) ||
		    mode == LONG_MIN || mode == LONG_MAX) {
			warnx("%s: invalid mode on \"%s\" line", ifname,
				base64 ? "begin-base64" : "begin");
			return 1;
		}
		/* skip whitespace to reach file name */
		while (*ofname != '\0' && isspace(*ofname))
			ofname++;
		if (*ofname == '\0') {
			warnx("%s: no filename on \"%s\" line", ifname,
				base64 ? "begin-base64" : "begin");
			return 1;
		}
		/*
		 * move to end of filename
		 *
		 * '\r' below isn't strictly posix for uudecode.
		 */
		for (p = ofname; *p != '\0' && *p != '\n' && *p != '\r'; p++)
			continue;
		/* remove newline */
		*p = '\0'; /* may already be '\0' */
		
		if (uudecode && (pflag
		    /*
		     * The next line is for Posix.  It can go away
		     * if we ever get /dev/stdout.
		     */
		    || (fname == NULL && strcmp(ofname, "/dev/stdout") == 0)
		    )) {
			out = stdout;
		}
		else {
			/*
			 * ignore ofname in file if fname was set (-o option
			 * to uudecode).
			 */
			char *fn;

			fn = fname != NULL ? fname : ofname;

			if ((out = fopen(fn, WRITE)) == NULL ||
			    fchmod(fileno(out), mode & 0666) == -1) {
				warn("%s: %s\n", fn, ifname);
				return 1;
			}
		}
		if (debug) uuformat("Begin uudecoding: %s\n", ofname);
		seqc = SEQMAX;
		check = nochk ? 0 : 1;
		first = 1;
		secnd = 0;
		if (base64)
			ret = base64_decode(in, out, ifname);
		else
			ret = decode();
		if (out != stdout)
			fclose(out);
		onedone = 1;
		if (debug) uuformat("End uudecoding: %s\n", ofname);
	} while (!uudecode);	/* master loop for multiple decodes in one file */
	return ret;
}

/*
 * Bring back a pointer to the start of the nth word.
 */
char *getnword(char *str, int n)
{
	while((*str == '\t') || (*str == ' ')) str++;
	if (! *str) return NULL;
	while(--n) {
		while ((*str != '\t') && (*str != ' ') && (*str)) str++;
		if (! *str) return NULL;
		while((*str == '\t') || (*str == ' ')) str++;
		if (! *str) return NULL;
	}
	return str;
}

/*
 * Install the table in memory for later use.
 */
void gettable(void)
{
	char buf[LINELEN];
	register int c, n = 0;
	register char *cpt;

	for (c = 0; c <= MAXCHAR; c++) chtbl[c] = -1;

again:	if (fgets(buf, sizeof buf, in) == NULL) {
		errx(5, "EOF while in translation table.");
	}
	numl++;
	if (strncmp(buf, "begin", 5) == 0) {
		errx(6, "Incomplete translation table.");
	}
	cpt = buf + strlen(buf) - 1;
	*cpt = ' ';
	while (*(cpt) == ' ') {
		*cpt = 0;
		cpt--;
	}
	cpt = buf;
	while ((c = *cpt)) {
		if (chtbl[c] != -1) {
			errx(7, "Duplicate char in translation table.");
		}
		if (n == 0) blank = c;
		chtbl[c] = n++;
		if (n >= 64) return;
		cpt++;
	}
	goto again;
}

/*
 * copy from in to out, decoding as you go along.
 */

int decode(void)
{
	char buf[LINELEN], outl[LINELEN];
	register char *bp, *ut;
	register int *trtbl = chtbl;
	register int n, c, rlen;
	register unsigned int len;

	for (;;) {
		if (fgets(buf, sizeof buf, in) == NULL) {
			warnx("EOF before end.");
			return 1;
		}
		numl++;
		len = strlen(buf);
		if (len) buf[--len] = '\0';
/*
 * Is it an unprotected empty line before the end line ?
 */
		if (len == 0) continue;
/*
 * Get the binary line length.
 */
		n = trtbl[(int)*buf];
		if (uudecode && n <= 0)
			break;
		if (n >= 0) goto decod;

/* uue checks only, uudecode always breaks or goes to decode */
 
/*
 * end of uuencoded file ?
 */
		if (strncmp(buf, "end", 3) == 0) return 0;
/*
 * end of current file ? : get next one.
 */
		if (strncmp(buf, "include", 7) == 0) {
			getfile(buf);
			continue;
		}
		warnx("Bad prefix line %d in file: %s",numl, ifname);
		if (debug) fprintf(stderr, "Bad line =%s\n",buf);
		Error(11);
/*
 * Sequence checking ?
 */
decod:		rlen = cdlen[n];
/*
 * Is it the empty line before the end line ?
 */
		if (n == 0) continue;
/*
 * Pad with blanks.
 */
		for (bp = &buf[c = len];
			c < rlen; c++, bp++) *bp = blank;
/*
 * Verify if asked for.
 */
		if (debug) {
			for (len = 0, bp = buf; len < rlen; len++) {
				if (trtbl[(int)*bp] < 0) {
					warnx(
	"Non uuencoded char <%c>, line %d in file: %s", *bp, numl, ifname);
					errx(16, "Bad line =%s",buf);
				}
				bp++;
			}
		}
/*
 * All this just to check for uuencodes that append a 'z' to each line....
 */
		if (secnd && check) {
			secnd = 0;
			if (buf[rlen] == SEQMAX) {
				check = 0;
				if (debug) fprintf(stderr, "Sequence check turned off (2).\n");
			} else
				if (debug) fprintf(stderr, "Sequence check on (2).\n");
		} else if (first && check) {
			first = 0;
			secnd = 1;
			if (buf[rlen] != SEQMAX) {
				check = 0;
				if (debug) fprintf(stderr, "No sequence check (1).\n");
			} else
				if (debug) fprintf(stderr, "Sequence check on (1).\n");
		}
/*
 * There we check.
 */
		if (check) {
			if (buf[rlen] != seqc) {
				warnx("Wrong sequence line %d in %s",
					numl, ifname);
				if (debug)
					fprintf(stderr,
	"Sequence char is <%c> instead of <%c>.\n", buf[rlen], seqc);
				return 1;
			}
			seqc--;
			if (seqc < SEQMIN) seqc = SEQMAX;
		}
/*
 * output a group of 3 bytes (4 input characters).
 * the input chars are pointed to by p, they are to
 * be output to file f.n is used to tell us not to
 * output all of them at the end of the file.
 */
		ut = outl;
		len = n;
		bp = &buf[1];
		while (n > 0) {
			*(ut++) = trtbl[(int) *bp] << 2 | trtbl[(int) bp[1]] >> 4;
			n--;
			if (n) {
				*(ut++) = (trtbl[(int) bp[1]] << 4) |
					  (trtbl[(int) bp[2]] >> 2);
				n--;
			}
			if (n) {
				*(ut++) = trtbl[(int) bp[2]] << 6 | trtbl[(int) bp[3]];
				n--;
			}
			bp += 4;
		}
		if ((n = fwrite(outl, 1, len, out)) <= 0) {
			warnx("Error on writing decoded file.");
			return 1;
		}
	}
	if (uudecode && 
	    (fgets(buf, sizeof(buf), in) == NULL ||
	     checkend(buf, "end", NULL) != 0)) {
		warnx("%s: no \"end\" line.", ifname);
		return 1;
	}
	return 0;
}

/*
 * Find the next needed file, if existing, otherwise try further
 * on next file.
 */
void getfile(register char *buf)
{
	if ((pos = getnword(buf, 2)) == NULL) {
		uuformat("uud: Missing include file name.\n");
		Error(17);
	} else
		if (source != NULL) {
			strcpy(ifnamebuf, source);
			strcat(ifnamebuf, pos);
		} else
			strcpy(ifnamebuf, pos);
	ifname = ifnamebuf;
#ifdef GEMDOS
	if (Fattrib(ifname, 0, 0) < 0)
#else
	if (access(ifname, 04))
#endif
	{
		if (debug) {
			uuformat("Cant find: %s\n", ifname);
			uuformat("Continuing to read same file.\n");
		}
	}
	else {
		if (freopen(ifname, "r", in) == in) {
			numl = 0;
			if (debug) 
				uuformat("Reading next section from: %s\n", ifname);
		} else {
			uuformat("uud: Freopen abort: %s\n", ifname);
			Error(9);
		}
	}
	for (;;) {
		if (fgets(buf, LINELEN, in) == NULL) {
			uuformat("uud: No begin line after include: %s\n", ifname);
			Error(12);
		}
		numl++;
		if (strncmp(buf, "table", 5) == 0) {
			gettable();
			continue;
		}
		if (strncmp(buf, "begin", 5) == 0) break;
	}
	lens = strlen(buf);
	if (lens) buf[--lens] = '\0';
/*
 * Check the part suffix.
 */
	if ((pos = getnword(buf, 3)) == NULL ) {
		uuformat("uud: Missing part name, in included file: %s\n", ifname);
		Error(13);
	} else {
		part = *pos;
		partn++;
		if (partn > 'z') partn = 'a';
		if (part != partn) {
			uuformat("uud: Part suffix mismatch: <%c> instead of <%c>.\n",
				part, partn);
			Error(14);
		}
		if (debug) uuformat("Reading part %c\n", *pos);
	}
}

#ifdef NEVER
/*
 * Printf style formatting. (Borrowed from MicroEmacs by Dave Conroy.) 
 * A lot smaller than the full fledged printf.
 */
/* VARARGS1 */
uuformat( char *fp, int args, ... )
{
	doprnt(fp, (char *)&args);
}

doprnt(fp, ap)
register char	*fp;
register char	*ap;
{
	register int	c, k;
	register char	*s;

	while ((c = *fp++) != '\0') {
		if (c != '%')
			outc(c);
		else {
			c = *fp++;
			switch (c) {
			case 'd':
				puti(*(int *)ap, 10);
				ap += sizeof(int);
				break;

			case 's':
				s = *(char **)ap;
				while ((k = *s++) != '\0')
					outc(k);
				ap += sizeof(char *);
				break;

			case 'c':
				outc(*(int *)ap);
				ap += sizeof(int);
				break;

			default:
				outc(c);
			}
		}
	}
}

/*
 * Put integer, in radix "r".
 */
puti(i, r)
register unsigned int	i;
register unsigned int	r;
{
	register unsigned int	q, s;

	if ((q = i / r) != 0)
		puti(q, r);
	s = i % r;
	if (s <= 9)
		outc(s + '0');
	else
		outc(s - 10 + 'A');
}
outc(c) register char c;
{
#ifdef GEMDOS
	if (c == '\n') Bconout(2, '\r');
	Bconout(2, c);
#else
	putchar(c);
#endif
}
#endif
