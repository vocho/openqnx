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





/*- $Id: sort.c 168695 2008-05-21 12:45:44Z seanb $
P1003.2/9, 4.58 sort - Sort, merge or sequence check text files.

$Log$
Revision 1.9  2007/04/26 20:50:36  aristovski
PR: 46523
CI: rmansfield

Added WIN32_ENVIRON=mingw
added compat as additional library
added include lib/compat.h
changed the way we are setting up singals (for __MINGW32__ only).

Revision 1.8  2006/05/05 17:32:05  cburgess

PR:29865 - Cisco updates
CI:bstecher

fix shadow globals of getopt etc to be static so sort will link statically

Revision 1.7  2006/04/11 16:16:27  rmansfield

PR: 23548
CI: kewarken

Stage 1 of the warnings/errors cleanup.

Revision 1.6  2006/03/22 16:06:52  kewarken

PR:26951
CI:Ryan Mansfield

Signal handler was unsafe and unmasked.

Revision 1.5  2005/06/03 01:38:00  adanko

Replace existing QNX copyright licence headers with macros as specified by
the QNX Coding Standard. This is a change to source files in the head branch
only.

Note: only comments were changed.

PR25328

Revision 1.4  2003/08/29 21:01:38  martin
Add/Update QSSL Copyright.

Revision 1.3  2000/11/20 02:44:50  peterv
Fix so it will work with Dinkum libraries.

Revision 1.2  1998/12/10 15:47:47  eric
changed -o option (sort.c) to not wipe out the output file named as
a parameter to -o. 1003.2 requires that one of the input files being
processed can be the output file.

Revision 1.1  1998/10/16 20:06:31  dtdodge
Adding sort to CVS respository

Revision 4.14  1998/10/16 19:52:51  dtdodge
Checkin for CVS


*/

#include "sort.h"
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#ifndef __MINGW32__
# ifndef __QNXNTO__
#include <unix.h>
# else
#include <libgen.h>
# endif
#endif

#ifndef MAXTEMP
#define	MAXTEMP	SORT_MAX_FILES+1
#endif

#if MAXTEMP < SORT_MAX_FILES
#error " SORT_MAX_FILES MUST BE LESS THAN MAXTEMP ......"
#endif



FILE           *
ufopen(char *fname, char *mode)
{
	if (strcmp(fname, "-") == 0)
		return stdin;
	return fopen(fname, mode);
}

/*-
 * Blech!
 *  To avoid something horrible from happening later, we have reserved
 * both a file *, and fd for the output file, so now we go and mung the
 * fileno() on it.....
 */
#define NORMAL_MODE	0666
static int      outset;
static char    *outname;
static FILE    *outfile
#ifndef __MINGW32__
// note: in mingw, stdout is not a constant. We initialize it first thing in main.
 = stdout
#endif
;


static char    *tmpdir;

char           *
get_temp(char *x)
{
	char           *p;
	if (!(p = tempnam(tmpdir, x))) {
		return 0;
	}
	return strdup(p);
}


/* maximum number of temp files in a run */

static int      maxfiles = SORT_MAX_FILES;

/* maximum number of lines in a run */
static int      maxlines = SORT_MAX_RUN;



/*
 * the following routines manage a 'run'. the run is stored as a table of
 * pointers to lines.
 * 
 * the table is allocated by init_run(), which will also empty the table if
 * required.
 * 
 * add_run puts the next line onto the table.
 * 
 * flush_run writes the entire run to a temporary file.
 * 
 */


static int      nentries = 0;
static linedesc **sort_run = NULL;

int
init_run()
{
	int             i;

	if (sort_run == NULL) {
		return ((sort_run = calloc(sizeof(linedesc *), maxlines)) != NULL);
	}
	for (i = 0; i < nentries; i++) {
		line_free(sort_run[i]);
		sort_run[i] = NULL;
	}
	return nentries = 0;
}

int
add_run(linedesc * lptr)
{
	if (nentries < maxlines) {
		sort_run[nentries++] = lptr;
		return nentries;
	}
	return 0;
}
#define	flush_run(f)	(nentries ? \
			(write_file((f),sort_run,nentries),nentries=0): 0)

#if 0
int
flush_run(FILE * f)
{
	int             i;
	if (nentries) {
		return write_file(f, sort_run, nentries);
	}
	return 0;
	for (i = 0; i < nentries; i++) {
		if (fwrite(STR_BEGIN(sort_run[i]), 1, sort_run[i]->len, f) == 0) {
			perror("fwrite");
		}
		line_free(sort_run[i]);
		sort_run[i] = NULL;
	}
	nentries = 0;
	return i;
}
#endif


static fdesc   *tmpdesc[MAXTEMP];
static char    *tmplist[MAXTEMP];
static int      ntemp = 0;

/*
 * alloc_temp_file() creates a new temporary file and open's it for write.
 * Since fopen() requires a memory allocation (for buffer space), we *allways*
 * have one temp file open in case a run consumes all memory and we would not
 * be able to open a file to store the run in.
 * 
 */

static char    *n_curtemp = NULL;
static FILE    *f_curtemp = NULL;

static char     pathbuf[80] = SORT_TMPNAM_BASE;

void
alloc_temp_file()
{
	if ((n_curtemp = get_temp(pathbuf)) == NULL) {
		fprintf(stderr, "sort: %s (%s)\n", strerror(errno), pathbuf);
		exit(2);
	}
	if ((f_curtemp = fopen(n_curtemp, "w")) == NULL) {
		fprintf(stderr, "sort: %s (%s)\n", strerror(errno), n_curtemp);
		exit(2);
	}
}

int
store_temp_file()
{
	(void) flush_run(f_curtemp);
	fclose(f_curtemp);
	if ((tmplist[ntemp++] = n_curtemp) == NULL) {
		fprintf(stderr, "sort internal error, temp file is null\n");
		exit(2);
	}
	return ntemp;
}

int
safe_purge_temp_files()
{
	int             i;
	if (ntemp == 0) {
		unlink(n_curtemp);
		return 0;
	}
	for (i = 0; i < ntemp; i++) {
		unlink(tmplist[i]);
	}
	ntemp = 0;
	return i;
}

int
purge_temp_files()
{
	int             i;
	if (ntemp == 0) {
		fclose(f_curtemp);
		if (verbose)
			fprintf(verbose, "remove n_curtemp = %s\n", n_curtemp);
		remove(n_curtemp);
		free(n_curtemp);
		return 0;
	}
	for (i = 0; i < ntemp; i++) {
		if (verbose)
			fprintf(verbose, "remove tmplist[%d] = %s,desc=%p\n", i, tmplist[i], tmpdesc[i]);
		close_fdesc(tmpdesc[i]);
		remove(tmplist[i]);
		free(tmplist[i]);
	}
	if (verbose)
		fprintf(verbose, "temp files removed!\n");
	ntemp = 0;
	return i;
}

int
flush_temp_files(FILE * f)
{
	int             i;
	FILE           *g;

	for (i = 0; i < ntemp; i++) {
		if ((g = fopen(tmplist[i], "r")) == NULL) {
			fprintf(stderr, "sort: %s (%s)\n", strerror(errno), tmplist[i]);
			exit(2);
		}
		if ((tmpdesc[i] = open_fdesc(g, 0)) == NULL) {
			fprintf(stderr, "sort: %s (%s)\n", strerror(errno), "open_fdesc");
			exit(2);
		}
	}
	if (verbose)
		putc('.', verbose);
	merge_files(f, tmpdesc, ntemp);
	purge_temp_files();
	return 0;
}



void
next_temp_file()
{
	if (store_temp_file() >= SORT_MAX_FILES) {
		alloc_temp_file();
		flush_temp_files(f_curtemp);
		fclose(f_curtemp);
		tmplist[0] = n_curtemp;
		ntemp = 1;
	}
	alloc_temp_file();
}



int
sort_file(fdesc * f)
{
	linedesc       *lptr;

	while (1) {
		if ((lptr = INPUT_LINE(f)) == NULL) {
			if (AT_EOF(f))
				break;
		} else {
			if (add_run(lptr)) {
				continue;
			}
			ungetline(f, lptr);
		}
		/* either sort_run exhausted or memory exhausted ... */
		/* build a temporary file and merge_files later.... */
		if (verbose)
			fprintf(verbose, "Sorting:...");
		qsort(sort_run, nentries, sizeof(linedesc *), fcompare);
		if (verbose)
			fprintf(verbose, "done...\n");
		next_temp_file();
	}
	if (verbose)
		fprintf(verbose, "sorting....");
	qsort(sort_run, nentries, sizeof(linedesc *), fcompare);
	if (verbose)
		fprintf(verbose, "done....\n");
	return 1;
}


int
cleanup_outfile()
{
	if (!outname)
		return 0;
	fflush(outfile);
	/* ltrunc(fileno(outfile), 0L, SEEK_CUR); */
	return 0;
}

FILE           *
get_outfile()
{
	int             fd;
	if (outset)
		return outfile;
	if (!outname) {
		outset = 1;
		return outfile;
	}
	fflush(outfile);
	close(fileno(outfile));
	unlink(outname);
	if ((fd = open(outname, O_WRONLY | O_CREAT | O_TRUNC, NORMAL_MODE)) == -1) {
		fprintf(stderr, "sort: unable to open output file '%s' (%s)\n",
			outname, strerror(errno));
		fprintf(stderr, "discarding output\n");
		fd = open("/dev/null", O_WRONLY);
	}
#ifdef _C_STD_BEGIN		/* This is defined with the Dinkum libraries */
	/* This is illegal, but is the easiest way right now */
	outfile->_Handle = fd;
#else
	fileno(outfile) = fd;
#endif
	setvbuf(outfile, 0, _IOFBF, 4096);
	outset = 1;
	return outfile;
}


int
merge_filelist(int nfiles, char **fnames)
{
	fdesc         **filetab;
	int             i;
	int             j;
	FILE           *g;
	int             base;

	alloc_temp_file();
	init_run();

	if ((filetab = calloc(sizeof(fdesc *), maxfiles)) == NULL) {
		perror("sort");
		exit(2);
	}
	base = 0;
	if (verbose) {
		fprintf(verbose, "merging.");
	}
	while (1) {
		/*
		 * we can only merge up to 'maxfiles' at a time, so we take
		 * the first 'maxfiles', and merge them into a run.
		 */
		for (i = 0; i < maxfiles && i + base < nfiles; i++) {
			if ((g = ufopen(fnames[base + i], "r")) == NULL) {
				fprintf(stderr, "sort: %s (%s)\n", strerror(errno), tmplist[base + i]);
				exit(2);
			}
			if ((filetab[i] = open_fdesc(g, 0)) == NULL) {
				fprintf(stderr, "sort: %s (%s)\n", strerror(errno), "open_fdesc");
				exit(2);
			}
		}

		j = 0;
		if (i + ntemp < maxfiles) {
			/*
			 * if we can, clean off the remainder of the files &
			 * the temps
			 */
			for (j = 0; j < ntemp; j++) {
				if ((g = fopen(tmplist[j], "r")) == NULL) {
					fprintf(stderr, "sort: %s (%s)\n", strerror(errno), tmplist[j]);
					exit(2);
				}
				if ((filetab[i + j] = open_fdesc(g, 0)) == NULL) {
					fprintf(stderr, "sort: %s (%s)\n", strerror(errno), "open_fdesc");
					exit(2);
				}
			}
			if (verbose)
				putc('.', verbose);
			merge_files(get_outfile(), filetab, i + ntemp);
			for (j = 0; j < i + ntemp; j++) {
				close_fdesc(filetab[j]);
				if (j > i) {
					if (verbose)
						fprintf(verbose, "removing tmplist[%d] = %s\n", j, tmplist[j]);
					remove(tmplist[j]);
				}
				free(filetab[j]);
			}
			ntemp = 0;
			break;
		} else {
			if (verbose)
				putc('.', verbose);
			merge_files(f_curtemp, filetab, i);
			for (j = 0; j < i; i++) {
				close_fdesc(filetab[j]);
				free(filetab[j]);
			}
			next_temp_file();
			base += i;
		}
	}
	if (verbose)
		putc('\n', verbose);
	free(filetab);
	flush_temp_files(get_outfile());
	return 0;
}


int
check_files(int n, char **vect)
{
	int             t;
	FILE           *f;
	fdesc          *fd;

	if (verbose)
		fprintf(verbose, "check: n=%d,argv[0] = '%s'\n", n, *vect);
	switch (n) {
	case 0:
		fd = open_fdesc(stdin, 0);
		if (file_ordered(fd)) {
			return 0;
		}
		close_fdesc(fd);
		return 1;
		break;
	default:
		fprintf(stderr, TXT(T_IGNORE_FILES));
	case 1:
		if ((f = ufopen(*vect, "r")) == NULL) {
			fprintf(stderr, "sort: %s (%s)\n", strerror(errno), *vect);
			return -1;
		}
		fd = open_fdesc(f, 0);
		t = file_ordered(fd) ? 0 : 1;
		close_fdesc(fd);
		if (verbose) {
			fprintf(verbose, "file orderd = %d\n", t);
		}
		return t;
		break;
	}
}


int
sort_flist(int n, char **vect)
{
	int             i;
	fdesc          *fd;
	FILE           *f;

	alloc_temp_file();
	init_run();
	if (n == 0) {
		if ((fd = open_fdesc(stdin, 0)) == NULL) {
			fprintf(stderr, TXT(T_NOMEMORY));
			exit(2);
		}
		sort_file(fd);
	}
	for (i = 0; i < n; i++) {
		if ((f = ufopen(*vect, "r")) == NULL) {
			fprintf(stderr, "sort: %s (%s)\n", strerror(errno), *vect);
			continue;
		}
		vect++;
		fd = open_fdesc(f, 0);
		sort_file(fd);
		close_fdesc(fd);
	}
	if (ntemp > 0) {
		if (verbose)
			fprintf(verbose, "sort_flist: residual temp files = %d\n", ntemp);
		store_temp_file();
		flush_temp_files(get_outfile());
	} else {
		(void) flush_run(get_outfile());
		purge_temp_files();
	}
	if (verbose)
		fprintf(verbose, "sort_flist: returning\n");
	return 0;
}

void
catch(int sig)
{
	safe_purge_temp_files();
	_exit(2);
}

/*
 * need a version of getopt which can deal with the sort parameters:
 * 
 * 
 */
/*
 * -X is a nill op, used by the "stuffer".....
 */

static char    *_pname = "sort";
int             (*action) (int, char **);

/* Sort doesn't use the standard getopt, but has a few variables
 * that conflicts with the getopt variables.  Just rename them to
 * keep gcc happy.
 */ 
#define optind sort_optind
#define optarg sort_optarg
#define opterr sort_opterr

static int optind;
static char *optarg;
static int opterr = 0;

int
advance(char **op, char **argv)
{
	if (**op) {
		optarg = *op;
		*op += strlen(*op);
		return 0;
	}
	if (argv[optind]) {
		optarg = argv[optind++];
		return 0;
	}
	return -1;
}

int
sort_options(int argc, char **argv)
{
	int	(*mkfield)(char *);

	mkfield = new_field;

	optind = 1;
	optarg = 0;

	/*
	 * Pull out -9 compat option first.  This
	 * block and draft9_field() can go away
	 * after "a while".
	 */
	{
	while (optind < argc) {
		char *op = argv[optind++];
		int	c;
		if (*op != '-' || strcmp(op, "--") == 0) {
			/* end of options */
			break;
		}

		for (op++; (c=*op++);) {
			switch (c) {
			case 'o':
			case 't':
			case 'k':
			case 'F':
			case 'L':
			case 'T':
				(void)advance(&op, argv);
				break;
			case '9':
				mkfield = draft9_field;
				break;
			default:
				break;
			}
		}

	}
	/* reset */
	optind = 1;
	optarg = 0;
	}

	while (optind < argc) {
		char *op = argv[optind++];
		int	c;
		if (*op == '+') {
			int	n = 0;
			int	old_field(char *, char *);
			if ((n = old_field(op, argv[optind])) == -1) {
				fprintf(stderr,"%s: Invalid field specification near '%s'\n",
						argv[0], op);
				return -1;
			}
			optind += n;
			continue;
		}
		if (*op != '-') {	/* end of options */
			return --optind;
		}
		if (strcmp(op, "--") == 0) { /* option sentinal */
			return optind;
		}
		for (op++; (c=*op++);) {
			switch (c) {
			case 'c':
				action = check_files;
				break;
			case 'b':
			case 'd':
			case 'f':
			case 'i':
			case 'n':
			case 'r':
			case 'B':
				add_flag(c);
				break;
			case 'u':	/* unique keys only */
				unique_keys = 1;
				break;
			case 'm':
				action = merge_filelist;
				break;
				/* require optarg */
			case 'o':	/* output file name follows */
				if (advance(&op, argv) == -1) {
					fprintf(stderr,"%s: missing filename for '-o'\n",
						argv[0]);
					return -1;
				}
				if (access(optarg, W_OK) == -1) {
					if (errno != ENOENT) {
						fprintf(stderr, TXT(T_BAD_OFILE),
							optarg, strerror(errno));
						return -1;
					}
				} else {
/* The following line will destroy the output file. 1003.2 _requires_ that
   the output file may be one of the input files. 

    					close(creat(optarg, 0666));
*/
				;
				}
#if 0
				if ((outfile = fopen("/dev/null", "w")) == NULL) {
					fprintf(stderr, TXT(T_RSV_OFILE),
						optarg, strerror(errno));
					return -1;
				}
#endif
				outname = optarg;
				break;

			case 't':	/* field separator follows */
				if (advance(&op, argv) == -1) {
					fprintf(stderr,"%s: must supply a field separator with '-t'\n",
						argv[0]);
					return -1;
				}
				newfs(optarg, 0);
				break;

			case 'k':	/* key field specification follows */
				if (advance(&op, argv) == -1) {
					fprintf(stderr,"%s: must supply a field specification with '-k'\n",
						argv[0]);
					return -1;
				}
				if ((*mkfield)(optarg) < 0) {
					fprintf(stderr, TXT(T_BAD_KEY), optarg);
					return -1;
				}
				break;
/*-
 * QNX extensions to control verbosity, memory, disk usage
 */
			case 'D':
				debugging = stderr;
			case 'v':
				verbose = stderr;
				break;

			case 'F':
				if (advance(&op, argv) == -1) {
					fprintf(stderr,"%s: Number of files missing, '-F' argument ignored\n",
						argv[0]);
					break;
				}
				maxfiles = (int) strtol(optarg, NULL, 0);
				if (maxfiles < 2) {
					fprintf(stderr, TXT(T_NUMFILES));
					maxfiles = SORT_MAX_FILES;
				}
				break;
			case 'L':
				if (advance(&op, argv) == -1) {
					fprintf(stderr,"%s: Number of Lines per Run missing, '-L' argument ignored\n",
						argv[0]);
					break;
				} 
				maxlines = (int) strtol(optarg, NULL, 0);
				if (maxlines < 2) {
					fprintf(stderr, TXT(T_NUMLINES));
					maxlines = SORT_MAX_RUN;
				}
				break;
			case 'T':
				if (advance(&op, argv) == -1) {
					fprintf(stderr,"%s: tmp dir missing, '-T' ignored\n",
						argv[0]);
					break;
				}
				tmpdir = optarg;
				break;
			case '9':
				/* handled above */
				break;
			default:
				fprintf(stderr,"%s: unknown argument '%c'\n",
					argv[0],c);
				return -1;
			}
		}
	}
	return optind;
}

int catchsigs[] = {
	SIGHUP,
	SIGINT,
	SIGQUIT,
	SIGILL,
	SIGABRT,
	SIGFPE,
#ifndef __MINGW32__
	SIGBUS,
#endif
	SIGSEGV,
	SIGPIPE,
#ifndef __MINGW32__
	SIGALRM,
#endif
	SIGTERM,
	0
};

#ifndef __MINGW32__
static void setup_catch() {
	struct sigaction act;
	sigset_t set;
	int             sig;
	/*
	 * setup signal handlers for all signals and report when they are hit
	 */

	sigemptyset(&set);
	for(sig = 0 ; catchsigs[sig] ; sig++)
		sigaddset(&set, catchsigs[sig]);

	act.sa_flags = 0;
	act.sa_mask = set;
	act.sa_handler = catch;

	for (sig=0; catchsigs[sig]; sig++)
		sigaction(catchsigs[sig], &act, NULL);
}
#else // we are in __MINGW32__
static void setup_catch() {
    int sig;
    
    for (sig = 0; catchsigs[sig]; sig++) {
        signal(catchsigs[sig], catch);
    }
}
#endif

int
main(int argc, char **argv)
{
	int             c;
	
	_pname = basename(argv[0]);
	
#ifdef __MINGW32__
	outfile = stdout;
#endif
	
	setup_catch();

	opterr = 0;		/* defeat auto-err messages */

	newfs("[:space:]", 1);

	action = sort_flist;

	if ((c = sort_options(argc, argv)) == -1) {
		return 2;
	}
#ifdef DEBUGGING
	dump_fields();
#endif

	if (unique_keys == 0 || get_nfields() == 0) {
		global_field();
	}
#if 0
	if (action == merge_filelist && optind == argc) {
		fprintf(stderr, TXT(T_MERGE_ARGS));
		return 2;
	} else {
		c = action(argc - optind, argv+optind);
	}
#else
	if (optind == argc) {
		char    *opts[2];
		opts[0] = "-";
		opts[1] = 0;
		c = action(1, opts);
	} else {
		c = action(argc - optind, argv+optind);
	}
#endif
	cleanup_outfile();
	return c;
}
