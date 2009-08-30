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




/*- context split.

Context split builds the argument list into a "machine" which splits
files.   Difficult machines (which have backwards references) require
a multi-pass process of the files.  Csplit notices these, and if the
split is simple uses a one pass routine.
So the basic processing is:

	read options
	parse arguments into machine.
	if (machine-is-multipass)
		if (file not regular)
			copy to temp
			set file to temp.
		for each opcode:
			scan for section end.
			rewind to section begin.
			split n lines.
			set section begin to section end.
	else for each line:
		while (match)
			move to next op.
		if output
			output line.

There a small optimisation that can be made.

$Log$
Revision 1.7  2005/06/03 01:37:43  adanko
Replace existing QNX copyright licence headers with macros as specified by
the QNX Coding Standard. This is a change to source files in the head branch
only.

Note: only comments were changed.

PR25328

Revision 1.6  2003/08/21 20:40:03  martin
Added QSSL Copyright.

Revision 1.5  1999/03/27 16:38:31  bstecher
moved to CVS

Revision 1.4  1999/03/27 16:25:44  steve
*** empty log message ***

 * Revision 1.3  1995/01/04  20:44:43  steve
 * fixed bug where empty files were being generated for "discard" patterns.
 *
 * Revision 1.2  1995/01/04  20:17:58  steve
 * csplit - split files based upon specification.
 *

*/

//static char rcsid[] = "$Id: csplit.c 153052 2008-08-13 01:17:50Z coreos $";

#include <libc.h>
#include <regex.h>
#include <stdarg.h>

#ifndef PATH_MAX
#include "limits.h"
#undef PATH_MAX
#define PATH_MAX (_POSIX_PATH_MAX*2)
#endif

char   *progname = "csplit";
int     debugging = 0;


/*-
 these are generally useful routines, not specific to csplit

 panic(fmt, ...)  - print a formatted message and exit.
 debug(fmt, ...)  - print a formatted debugging message.
 fcopy(dest, src) - copy a file, return number of bytes copied.
 lcopy(dest, src, n) - copy n lines of a file, return number of bytes.
 isreg(int fd)    - return !0 if file is "regular", 0 otherwise.
 char *compile_regexp(str, exp)
                  - return "" if str is compiled into exp, "..." otherwise.
*/

static void
panic(const char *fmt, ...)
{
	va_list      vargs;
	va_start(vargs, fmt);
	fprintf(stderr, "%s: fatal error: ", progname);
	vfprintf(stderr, fmt, vargs);
	putc('\n', stderr);
	exit(1);
	/*NOTREACHED*/
}

#ifndef NDEBUG
static void
debug(const char *fmt, ...)
{
	va_list       vargs;
	va_start(vargs, fmt);
	if (debugging)
		vfprintf(stdout, fmt, vargs);
	va_end(vargs);
}
#endif

static long
fcopy(FILE *to, FILE *from)
{
	int           c;
	long          n = 0;
	while ((c=getc(from)) != EOF) {
		if (to)
			putc(c, to);
		n++;
	}
	return n;
}

static long
lcopy(FILE *to, FILE *from, int nlines)
{
	int            c;
	long           n = 0;
	while (nlines && (c=getc(from)) != EOF) {
		if (c == '\n')
			nlines--;
		if (to)
			putc(c, to);
		n++;
	}
	return n;
}

#include <sys/stat.h>
static int
isreg(int fd)
{
	struct stat st;
	if (fstat(fd, &st) == -1)
		return 0; /* definitely not regular! */
	return S_ISREG(st.st_mode);
}


static char *
compile_regexp(char *str, regex_t **rp)
{
	regex_t    *r = calloc(sizeof *r, 1);
	int         n;
	static char regerr[256];
	if ((n=regcomp(r, str, REG_NOSUB)) != 0) {
		regerror(n, r, regerr, sizeof regerr);
		free(r);
		*rp = 0;
		return regerr;
	}
	regerr[0] = 0;
	*rp = r;
	return regerr;
}



/*-
 * gen_file -- generate and open a file with these constraints
 */
unsigned long
u_exp10(unsigned k)
{
	unsigned long           n = 1;
	while (k--)
		n *= 10;
	return n;
}
static FILE *
gen_file(char *prefix, int revno, int ndigits)
{
	char    buf[PATH_MAX+1];
	FILE   *f;
/*-
 * check we haven't exceeded the ndigits limit
 */
 	if (revno >= u_exp10(ndigits)) {
		panic("reached %lu file limit", u_exp10(ndigits));
	}
	sprintf(buf, "%s%*.*d", prefix, ndigits, ndigits, revno);
	if ((f=fopen(buf, "w")) == 0) {
		panic("unable to create file '%s' (%s)\n", buf, strerror(errno));
	}
	return f;
}

/*-
 * SplitOp: opcode for splitting machine.
 * Each opcode specifies a pattern and number of lines which is
 * interpretted as "cut the input at the line matching the pattern
 * + n.  N can be negative.
 * A NULL expression means match any pattern, this is used for
 * splitting a fixed section. (ie. csplit X 24 {99}) makes up to
 * 99 sections of 24 lines.
 * 
 * Opdup makes a copy of an opcode.
 * Opexpr makes an expression opcode from a string.
 * Opnlines makes an nlines opcode from a string.
 * parse_repeat parses a repeat string.
 * buildmachine builds a machine from a list of opcodes.
 * isonepass checks whether the machine can process the file
 * in one pass.
 */

typedef struct SplitOp SplitOp;

struct SplitOp {
	int             copy;
	regex_t        *exp;
	long            n;
	SplitOp        *next;
#ifndef NDEBUG
	char           *argstr;
#endif
};


static struct SplitOp *
Opdup(SplitOp *x)
{
	SplitOp *y;
	if ((y=malloc(sizeof *y)) == 0)
		return 0;
	memcpy(y, x, sizeof *y);
	return y;
}

static SplitOp *
Opexpr(char *s)
{
	SplitOp       *p;
	char          *t, *u;
	long           n;

	if ((t=strrchr(s, *s)) == 0) {
		panic("missing '%c' in arg '%s'", *s, s);
		return 0;
	}
	if (*t == '\0') {
		n = 0;
	} else {
		n = strtol(t+1, &u, 10);
		if (*u) {
			panic("expected numeric near '%s' in arg '%s'",
				u, s);
		}
	}
	if ((p=malloc(sizeof *p)) == 0) {
		panic("no memory to allocate arg '%s'", s);
	}
#ifndef NDEBUG
	p->argstr = strdup(s);
#endif
	*t = '\0';
	u=compile_regexp(s+1, &p->exp);
	*t = *s;
	if (*u) {
		panic("%s in arg '%s'", u, s);
	}
	p->copy = *s == '/';
	p->n = n;
	return p;
}

static SplitOp *
Opnlines(char *s)
{
	SplitOp       *p;
	char          *t;
	long           n;

	n = strtol(s, &t, 10);
	if (*t) {
		panic("expected numeric argument, found '%s'", s);
	}
	if ((p=malloc(sizeof *p)) == 0) {
		panic("no memory to allocate arg '%s'", s);
	}
#ifndef NDEBUG
	p->argstr = strdup(s);
#endif
	t=compile_regexp(".*", &p->exp);
	if (*t) {
		panic("%s in arg '%s'", t, s);
	}
	p->copy = 1;
	p->n = n;
	return p;
}

static int
parse_repeat(char *s, long *np)
{
	long      n;
	char     *t;
	if (*s == '{') { /* bleah -- repeat code */
		n = strtol(s+1, &t, 10);
		if (*t != '}') {
			panic("expected '}' near '%c' in '%s'", *t, s);
		}
		*np = n;
		return 0;
	}
	return 1;
}

static SplitOp *
buildmachine(char **args, int nargs)
{
	SplitOp *head = 0;
	SplitOp *last = 0;
	SplitOp *cur = NULL;

	while (nargs--) {
		char       *s;
		if (*(s=*args++) == '{') {
			long       rep;
			if (parse_repeat(s, &rep) != 0) {
				panic("cannot interpret argument '%s'", s);
			}
			if (last == 0) {
				panic("repeat cannot be first argument");
			}
			while (rep--) {
				if ((cur=Opdup(last)) == 0) {
					panic("out of memory in Opdup");
				}
				last->next = cur;
				last = cur;
			}
			last = cur;
		} else if (*s == '%' || *s == '/') {
			cur = Opexpr(s);
			if (last) {
				last->next = cur;
			} else {
				head = cur;
			}
			last = cur;
		} else if (isdigit(*s)) {
			cur = Opnlines(s);
			if (last) {
				last->next = cur;
			} else {
				head = cur;
			}
			last = cur;
		} else {
			panic("cannot interpret argument '%s'", s);
		}
	}
	return head;
}

static int
isonepass(SplitOp *mach)
{
	while (mach) {
		if (mach->n < 0)
			return 0;
		mach = mach->next;
	}
	return 1;
}

static int
ismatch(char *bufp, SplitOp *op)
{
	if (!op) return 0;
	if (!op->exp) return 1;
	return regexec(op->exp, bufp, 0, 0, 0) == REG_OK;
}


/*-
 * There are two splitter engines: one_pass & npass.
 * one_pass works for machines with no backwards references (ie. /x/-4)
 * npass works for all valid machines, however will make n-passes at
 * the file.
 *
 * BUFLEN can be tuned for better performance.
 */
#define BUFLEN        8192

static char *prefix  = "xx";
static int   ndigits = 2;
static int   verbose = 1;


int
one_pass(FILE *f, SplitOp * list)
{
	char            bufp[LINE_MAX];
	int             fileno = 0;
	FILE           *g;
	int             need_input = 1;
	int             state      = 0;
	long            nwritten;
	long            ncopy = 0;

	setvbuf(f, 0, _IOFBF, BUFLEN);
	g = gen_file(prefix, fileno++, ndigits);
	nwritten = 0;

#ifndef NDEBUG 
	debug("splitting to '%s'\n", list->argstr);
#endif
	while (1) {
		if (need_input) {
			if (fgets(bufp, sizeof(bufp - 1), f) == NULL) {
				fclose(g);
				break;	
			}
			need_input = 0;
		}
		switch (state) {
		case 0:
			if (ismatch(bufp, list)) {
				ncopy = 0;
				state = 1;
			} else {
				if (list->copy) {
					fputs(bufp, g);
				}
				nwritten += strlen(bufp);
				need_input = 1;
			}
			break;	
		case 1:
			if (ncopy++ >= list->n) {
				if (verbose && list->copy) {
					printf("%ld\n", nwritten);
				}
				nwritten = 0;
				if (list->copy) {
					fclose(g);
					g = gen_file(prefix, fileno++, ndigits);
				}
				if ((list = list->next) == 0)
					state = 2;
				else
					state = 0;
			} else {
				if (list->copy) {
					fputs(bufp, g);
					nwritten += strlen(bufp);
				}
				need_input = 1;	
			}
			break;
		case 2:
			fputs(bufp, g);
			nwritten += strlen(bufp);
			need_input = 1;
			break;
		}
	}
	if (verbose && (!list || list->copy))
		printf("%ld\n", nwritten);
	fclose(g);
	return 0;
}

int
npass(FILE *f, SplitOp * list)
{
	char            bufp[LINE_MAX];
	int             fileno = 0;
	FILE           *g;
	long            boffs = 0;
	long            blineno = 1;

	setvbuf(f, 0, _IOFBF, BUFLEN);
	g = gen_file(prefix, fileno++, ndigits);

#ifndef NDEBUG 
	debug("splitting to '%s'\n", list->argstr);
#endif
	for (; list && !feof(f); list=list->next) {
		long       lineno = blineno;
		long       nwritten;

		while (fgets(bufp, sizeof bufp - 1, f)) {
			lineno++;
			if (ismatch(bufp, list)) {
				lineno--;
#ifndef NDEBUG
				debug("%ld: '%s' split line '%s'\n",
					lineno, list->argstr, bufp);
#endif
				break;
			}
		}
		if ((lineno+=list->n) < blineno) {
			panic("out of range [%ld vs %ld]\n", lineno, blineno);
		}
#ifndef NDEBUG
		debug("split %ld..%ld by %s\n", blineno, lineno, list->argstr);
#endif
		fseek(f, boffs, SEEK_SET);
		if (list->copy) {
			nwritten = lcopy(g,f,lineno-blineno);
			fclose(g);
			g = gen_file(prefix, fileno++, ndigits);
			if (verbose && list->copy)
				printf("%ld\n", nwritten);
		} else {
			nwritten = lcopy(0, f, lineno-blineno);
		}
		boffs += nwritten;
		blineno = lineno;
	}
	if (!feof(f)) {
#ifndef NDEBUG
		debug("residual...\n");
#endif
		if (verbose && (!list || list->copy)) {
			printf("%ld\n", fcopy(g, f));
		} else {
			fcopy(g, f);
		}
		fclose(g);
	}
	return 0;
}

int
main(int argc, char **argv)
{
	SplitOp        *machine;
	int             c;
	int             nodiscard = 0;
	FILE           *f;
	progname = argv[0];

	while ((c = getopt(argc, argv, "Dksn:f:")) != EOF) {
		switch (c) {
		case 'D':
			debugging = 1;
#ifdef NDEBUG
			fprintf(stderr,
				"csplit: warning DEBUGGING support not available\n");
#endif
			break;
		case 'k':
			nodiscard = 1;
			break;
		case 's':
			verbose = 0;
			break;
		case 'f':
			prefix = optarg;
			break;
		case 'n':
			ndigits = strtol(optarg, NULL, 10);
			break;
		default:
			return EXIT_FAILURE;
		}
	}
	if (optind == argc) {
		panic("missing file and arguments");
	} else if (optind+1 == argc) {
		panic("missing arguments");
	}
	if (strcmp(argv[optind], "-") == 0) {
		f = stdin;
	} else if ((f = fopen(argv[optind++], "r")) == NULL) {
		perror(argv[optind - 1]);
		return EXIT_FAILURE;
	}
	machine = buildmachine(argv+optind, argc-optind);
	if (isonepass(machine)) { /* don't have to worry about file. */
		one_pass(f, machine);
	} else if (!isreg(fileno(f))) {
		char     *temp = tmpnam("csp\blXXX");
		FILE *fp;
		if ((fp=fopen(temp, "w")) == 0) {
			panic("cannot create temporary file '%s' (%s)",
				temp, strerror(errno));
		}
		fcopy(fp, f);
		fclose(f);
		fclose(f);
		if ((f=fopen(temp, "r")) == 0) {
			panic("cannot open temporary file '%s' (%s)",
				temp, strerror(errno));
		}
		npass(f, machine);
		fclose(f);
		unlink(temp);
	} else {
		npass(f, machine);
	}
	return 0;
}
