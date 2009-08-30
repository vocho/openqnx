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



#ifdef __MINGW32__
#include <lib/compat.h>
#undef err
// From our limits.h
#define PIPE_BUF    5120
// windows will default to opening file in "t" mode (translation mode) which
// messes up the fseek function behaviour
#define FOPEN_MODE "rb"
#else
#define FOPEN_MODE "r"
#endif


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdarg.h>
#include <sys/stat.h>

#ifndef __QNXNTO__
// QNX4 does not have 64-bit IO, so assume sizeof(off_t)==sizeof(long)
#define fseeko fseek
#define ftello ftell
#else
// Whereas QNX6 has basename() prototype in <libgen.h> not <unistd.h>
#include <libgen.h>
#endif

off_t length(FILE *fp);
void message(char *fmt, ...);
void err(char *fmt, ...);
int main(int argc, char **argv);
int tail(char *name, int what, off_t goal);
int show(FILE *fp);
int skip(FILE *fp, char *name, int what, off_t goal);
int back(FILE *fp, char *name, int what, off_t goal);
int pipeskip(FILE *fp, int what, off_t goal);
int pipeback(FILE *fp, char *name, int what, off_t goal);

#define BLKSIZ 512
#define IOBUFSIZ 16384

char *progname;
int error;
int follow = 0;

off_t
length(FILE *fp) {
	off_t save, rc;

	if ((save = lseek(fileno(fp), 0, SEEK_CUR)) == -1)
		return -1;
	rc = lseek(fileno(fp), 0, SEEK_END);
	(void) lseek(fileno(fp), save, SEEK_SET);
	return rc;
}

void
message(char *fmt, ...) {
	va_list ap;

	va_start(ap, fmt);
	fprintf(stderr, "%s:  ", progname);
	(void) vfprintf(stderr, fmt, ap);
	(void) fputc('\n', stderr);
	va_end(ap);
}

void
err(char *fmt, ...) {
	va_list         ap;

	va_start(ap, fmt);
	fprintf(stderr, "%s:  ", progname);
	(void) vfprintf(stderr, fmt, ap);
	fprintf(stderr, " (%s)", strerror(errno));
	(void) fputc('\n', stderr);
	va_end(ap);
	error++;
}

int
main(int argc, char **argv) {
	int  i, multiple = 0;
	char *p;
	int what = 'l';
	off_t count = -10L;

	progname = basename(argv[0]);
	opterr = 0;
	for (;;) {
		while ((i = getopt(argc, argv, ":lbfc:n:")) != -1)
			switch (i) {
			case 'l': what = (int) 'l'; break;
			case 'b': what = (int) 'b'; break;
			case 'f': follow = 1; break;
			case 'c': what = (int) 'c'; 
			case 'n':
#if _FILE_OFFSET_BITS - 0 == 64
				count = strtoll(optarg, &p, 10);
#else
				count = strtol(optarg, &p, 10);
#endif
				if (errno == EINVAL || *p != '\0') {
					if (what == 'c') 
						message("%s: invalid number of bytes", optarg);
					else 
						message("%s: invalid number of lines", optarg);
					return 1;
				}

				if (optarg[0] != '+' && optarg[0] != '-')
					count = -count;
				break;
			case ':':
				message("option -%c requires an argument", optopt);
				return 1;
				break;
			default:
				if (isdigit(optopt) || optopt == (int) '+') {
					count = strtol(argv[optind - 1], &p, 10);
					switch (*p) {
					case 'b': count *= BLKSIZ;
					case 'c': what = (int) 'c'; break;
					case 'l': what = (int) 'l'; break;
					case 'f': follow = 1; break;
					}
				} else {
					message("unknown option '%c'", optopt);
					return 1;
				}
		}
		if (argv[optind] && *argv[optind] == '+') {
			count = strtol(argv[optind], &p, 10);
			if (p == argv[optind])
				count = -10;
			switch (*p) {
				case 'b': count *= BLKSIZ;
				case 'c': what = (int) 'c'; break;
				case 'l': what = (int) 'l'; break;
				case 'f': follow = 1; break;
			}
			optind++;
		} else
			break;
	}

	if (what == 'b') {
		what = (int) 'c';
		count *= BLKSIZ;
	}
	multiple = (argc - optind) > 1;

	if (follow && multiple) {
		message("can't follow multiple input files");
		return 1;
	}
	if (optind == argc)
		(void) tail("-", what, count);
	else
		while (optind < argc) {
			if (multiple)
				printf("\n==> %s <==\n", argv[optind]);
			(void) tail(argv[optind++], what, count);
		}

	return error ? 1 : 0;
}

int
tail(char *name, int what, off_t goal) {
	FILE *fp;
	int rc;
        struct stat s;

	if (strcmp(name, "-") == 0) {
		fp = stdin;
		name = "stdin";
	} else if (!(fp = fopen(name, FOPEN_MODE))) {
		err("unable to open '%s'", name);
		return 1;
	}
        (void) stat(name, &s);
        if (S_ISDIR(s.st_mode))
        {
          fprintf(stderr, "tail:  unable to open directory '%s'\n", name);
          return 1;
        }
	(void) setvbuf(fp, 0, _IOFBF, IOBUFSIZ);
	if (follow && lseek(fileno(fp), 0, SEEK_SET) == -1) {
		err("unable to seek - can't follow pipes");
		return 1;
	}

	if (goal > 0)
		rc = skip(fp, name, what, goal);
	else
		rc = back(fp, name, what, -goal);
	return rc;
}

int
show(FILE *fp) {
	char c = 0;

	(void) fflush(stdout);
	(void) setvbuf(stdout, 0, _IOFBF, IOBUFSIZ);
	do {
		while (fread(&c, 1, 1, fp) == 1)
			(void) putchar(c&0xff);
		if (follow) {
			(void) fflush(stdout);
			/* once fread hits EOF, it doesn't reset (in dinkum)
			 * so we seek to end to reset file pointer */
			(void) fseeko(fp,0, SEEK_CUR);
			(void) sleep(1);
		}
	} while (follow);
	(void) fclose(fp);
	return 0;
}

int
skip(FILE *fp, char *name, int what, off_t goal) {
	off_t consumed;

	if (lseek(fileno(fp), 0, SEEK_SET) == -1)
		return pipeskip(fp, what, goal);

	switch (what) {
	case 'c':
		if (fseeko(fp, goal - 1, SEEK_SET) == -1) {
			err("unable to seek to goal in '%s'", name);
			return 1;
		}
                if( length(fp) < goal )
                  (void) fseeko(fp, 0, SEEK_SET);
		return show(fp);
                   
	case 'l':
		for (consumed = 1; consumed < goal;) {
			char c = (char) 0;

			if (fread(&c, 1, 1, fp) != 1 && ferror(fp)) {
				err("read error in '%s'", name);
				return 1;
			} else if (feof(fp)) {
				return 0;
			}
			if (c == '\n')
				consumed++;
		}
		return show(fp);

	default:
		err("skip() doesn't know what it's what is.");
		return 1;
	}
}

int
back(FILE *fp, char *name, int what, off_t goal) {
	off_t consumed;
	int cache;

	if (lseek(fileno(fp), 0, SEEK_END) == -1)
		return pipeback(fp, name, what, goal);
	switch (what) {
	case 'c':
		if (fseeko(fp, -goal, SEEK_END) == -1) {
			int e = errno;

			if (length(fp) < goal) {
				/*-
					if the file is smaller than our goal,
					display the entire file.
				 */
				(void) fseeko(fp, 0, SEEK_SET);
				//return 0;
                                //break;
			} else {
				errno = e;
				err("seek failed in '%s'", name);
				return 1;
			}
		}
		return show(fp);

	case 'l':
		for (cache = 0, consumed = 0; consumed < goal; cache--) {
			char c = 0;

			if (cache == 0) {
				off_t cur = ftello(fp);

				fseeko(fp, -IOBUFSIZ, SEEK_CUR);
				fread(&c, 1, 1, fp); /* prime cache */
				fseeko(fp, cur, SEEK_SET); /* return to orig */
				cache = IOBUFSIZ;
			}
			if (fseeko(fp, -2, SEEK_CUR) == -1) {
				/*-
					if we try to seek too far back to
					make the goal we'll just display
					the entire file
				 */
				fseeko(fp, 0, SEEK_SET);
				break;
			}
			if (fread(&c, 1, 1, fp) != 1) {
				err("read failed in '%s'", name);
				return 1;
			}
			if (c == '\n')
				consumed++;
		}
		return show(fp);

	default:
		err("back() doesn't know what it's what is.");
		return 1;
	}
}

int
pipeskip(FILE *fp, int what, off_t goal) {
	off_t consumed;
	int i = 0;
	char buf[PIPE_BUF], *p = 0;

	switch (what) {
	case 'c':
		for (consumed = 0; consumed < goal; ) {
			int siz;

			siz = goal - consumed > sizeof buf ? sizeof buf : goal - consumed;
			if ((i = fread(&buf, 1, siz, fp)) != siz && ferror(fp)) {
				err("read error on pipe");
				return 1;
			} else if (i == 0 && feof(fp))
				break;
			consumed += i;
		}
		return show(fp);

	case 'l':
		for (consumed = 0; consumed < goal; ) {
			int siz;

			siz = goal - consumed > sizeof buf ? sizeof buf : goal - consumed;
			if ((i = fread(&buf, 1, siz, fp)) != siz && ferror(fp)) {
				err("read error on pipe");
				return 1;
			} else if (i == 0 && feof(fp))
				break;
			for (p = buf; i > 0 && consumed < goal; i--)
				if (*p++ == '\n')
					consumed++;
		}
		(void) fwrite(p, i, 1, stdout); fflush(stdout);
		return show(fp);

	default:
		err("pipeskip() doesn't know what it's what is.");
		return 1;
	}
}

int
pipeback(FILE *fp, char *name, int what, off_t goal) {
	int bufsiz, bb; /* number of characters in the bigbuf */
	char *bigbuf;
	char buf[PIPE_BUF];
	off_t consumed;
	int i;

	bufsiz = goal;
	switch(what) {
	case 'l': bufsiz *= LINE_MAX;
	case 'c':
		if (!(bigbuf = malloc(bufsiz))) {
			err("not enough memory to tail the pipe");
			return 1;
		}
		bb = fread(bigbuf, 1, bufsiz, fp);
		for (;;) {
			int siz;

			siz = sizeof buf > bufsiz ? bufsiz : sizeof buf;
			if ((i = fread(&buf, 1, siz, fp)) != siz && ferror(fp)) {
				err("read error on pipe");
				return 1;
			}
			if (i == 0 && feof(fp))
				break;
			if (bb == bufsiz) {
				/*-
					if the buf is full, shuffle it
					around and move the new stuff in
				 */
				memmove(bigbuf, bigbuf+i, bb - i);
				memcpy(bigbuf+bb-i, buf, i);
			} else {
				if (bb + i > bufsiz) {
					/*-
						if we don't fit in the bigbuf
						we shuffle the bigbuf
					 */
					memmove(bigbuf, bigbuf+bb+i-bufsiz, bb);
					bb -= i;
				}
				memcpy(bigbuf+bb, buf, i);
				bb += i;
			}
			if (bb > bufsiz) {
				err("pipeback - programming error");
				return 1;
			}
		}
		if (what == 'c')
			fwrite(bigbuf, bb, 1, stdout);
		else /* what == 'l' */ {
			char *p;

			i = 0, consumed = 0;
			for (p = bigbuf+bb; i < bb && consumed < goal; i++, p--)
				if (*p == '\n')
					consumed++;

			if (i == bb && bb == bufsiz)
				err("file too big");
			else {
				/* walk back to beginning of line, or bigbuf */
				for (;i < bb; i++, p--) {
					if (*p == '\n') { /* skip extra */
						p++; i--;
						break;
					}
				}
			}
			fwrite(p, i, 1, stdout);
		}
		free(bigbuf);
		show(fp);
		return 0;

	default:
		err("pipeback() doesn't know what it's what is.");
		return 1;
	}
}
