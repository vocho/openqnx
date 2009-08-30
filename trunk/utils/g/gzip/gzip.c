/* gzip (GNU zip) -- compress files with zip algorithm and 'compress' interface

   Copyright (C) 1999, 2001, 2002, 2006, 2007 Free Software Foundation, Inc.
   Copyright (C) 1992-1993 Jean-loup Gailly

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

/*
 * The unzip code was written and put in the public domain by Mark Adler.
 * Portions of the lzw code are derived from the public domain 'compress'
 * written by Spencer Thomas, Joe Orost, James Woods, Jim McKie, Steve Davies,
 * Ken Turkowski, Dave Mack and Peter Jannesen.
 *
 * See the license_msg below and the file COPYING for the software license.
 * See the file algorithm.doc for the compression algorithms and file formats.
 */

static char  *license_msg[] = {
"Copyright (C) 2007 Free Software Foundation, Inc.",
"Copyright (C) 1993 Jean-loup Gailly.",
"This is free software.  You may redistribute copies of it under the terms of",
"the GNU General Public License <http://www.gnu.org/licenses/gpl.html>.",
"There is NO WARRANTY, to the extent permitted by law.",
0};

/* Compress files with zip algorithm and 'compress' interface.
 * See help() function below for all options.
 * Outputs:
 *        file.gz:   compressed file with same mode, owner, and utimes
 *     or stdout with -c option or if stdin used as input.
 * If the output file name had to be truncated, the original name is kept
 * in the compressed file.
 * On MSDOS, file.tmp -> file.tmz. On VMS, file.tmp -> file.tmp-gz.
 *
 * Using gz on MSDOS would create too many file name conflicts. For
 * example, foo.txt -> foo.tgz (.tgz must be reserved as shorthand for
 * tar.gz). Similarly, foo.dir and foo.doc would both be mapped to foo.dgz.
 * I also considered 12345678.txt -> 12345txt.gz but this truncates the name
 * too heavily. There is no ideal solution given the MSDOS 8+3 limitation.
 *
 * For the meaning of all compilation flags, see comments in Makefile.in.
 */

#ifdef RCSID
static char rcsid[] = "$Id: gzip.c 174864 2008-08-14 20:08:13Z rmansfield $";
#endif

#include <config.h>
#include <ctype.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>

#include "tailor.h"
#include "gzip.h"
#include "lzw.h"
#include "revision.h"

#include "fcntl-safer.h"
#include "getopt.h"
#include "stat-time.h"

		/* configuration */

#ifdef HAVE_FCNTL_H
#  include <fcntl.h>
#endif

#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#if defined STDC_HEADERS || defined HAVE_STDLIB_H
#  include <stdlib.h>
#else
   extern int errno;
#endif

#ifndef NO_DIR
# define NO_DIR 0
#endif
#if !NO_DIR
# include <dirent.h>
# ifndef _D_EXACT_NAMLEN
#  define _D_EXACT_NAMLEN(dp) strlen ((dp)->d_name)
# endif
#endif

#ifdef CLOSEDIR_VOID
# define CLOSEDIR(d) (closedir(d), 0)
#else
# define CLOSEDIR(d) closedir(d)
#endif

#ifndef NO_UTIME
#  include <utimens.h>
#endif

#define RW_USER (S_IRUSR | S_IWUSR)  /* creation mode for open() */

#ifndef MAX_PATH_LEN
#  define MAX_PATH_LEN   1024 /* max pathname length */
#endif

#ifndef SEEK_END
#  define SEEK_END 2
#endif

#ifndef CHAR_BIT
#  define CHAR_BIT 8
#endif

#ifdef off_t
  off_t lseek OF((int fd, off_t offset, int whence));
#endif

#ifndef OFF_T_MIN
#define OFF_T_MIN (~ (off_t) 0 << (sizeof (off_t) * CHAR_BIT - 1))
#endif

#ifndef OFF_T_MAX
#define OFF_T_MAX (~ (off_t) 0 - OFF_T_MIN)
#endif

/* Use SA_NOCLDSTOP as a proxy for whether the sigaction machinery is
   present.  */
#ifndef SA_NOCLDSTOP
# define SA_NOCLDSTOP 0
# define sigprocmask(how, set, oset) /* empty */
# define sigset_t int
# if ! HAVE_SIGINTERRUPT
#  define siginterrupt(sig, flag) /* empty */
# endif
#endif

#ifndef HAVE_WORKING_O_NOFOLLOW
# define HAVE_WORKING_O_NOFOLLOW 0
#endif

#ifndef ELOOP
# define ELOOP EINVAL
#endif

/* Separator for file name parts (see shorten_name()) */
#ifdef NO_MULTIPLE_DOTS
#  define PART_SEP "-"
#else
#  define PART_SEP "."
#endif

		/* global buffers */

DECLARE(uch, inbuf,  INBUFSIZ +INBUF_EXTRA);
DECLARE(uch, outbuf, OUTBUFSIZ+OUTBUF_EXTRA);
DECLARE(ush, d_buf,  DIST_BUFSIZE);
DECLARE(uch, window, 2L*WSIZE);
#ifndef MAXSEG_64K
    DECLARE(ush, tab_prefix, 1L<<BITS);
#else
    DECLARE(ush, tab_prefix0, 1L<<(BITS-1));
    DECLARE(ush, tab_prefix1, 1L<<(BITS-1));
#endif

		/* local variables */

int ascii = 0;        /* convert end-of-lines to local OS conventions */
int to_stdout = 0;    /* output to stdout (-c) */
int decompress = 0;   /* decompress (-d) */
int force = 0;        /* don't ask questions, compress links (-f) */
int no_name = -1;     /* don't save or restore the original file name */
int no_time = -1;     /* don't save or restore the original file time */
int recursive = 0;    /* recurse through directories (-r) */
int list = 0;         /* list the file contents (-l) */
int verbose = 0;      /* be verbose (-v) */
int quiet = 0;        /* be very quiet (-q) */
int do_lzw = 0;       /* generate output compatible with old compress (-Z) */
int test = 0;         /* test .gz file integrity */
int foreground = 0;   /* set if program run in foreground */
char *program_name;   /* program name */
int maxbits = BITS;   /* max bits per code for LZW */
int method = DEFLATED;/* compression method */
int level = 6;        /* compression level */
int exit_code = OK;   /* program exit code */
int save_orig_name;   /* set if original name must be saved */
int last_member;      /* set for .zip and .Z files */
int part_nb;          /* number of parts in .gz file */
struct timespec time_stamp; /* original time stamp (modification time) */
off_t ifile_size;      /* input file size, -1 for devices (debug only) */
char *env;            /* contents of GZIP env variable */
char **args = NULL;   /* argv pointer if GZIP env variable defined */
char *z_suffix;       /* default suffix (can be set with --suffix) */
size_t z_len;         /* strlen(z_suffix) */

/* The set of signals that are caught.  */
static sigset_t caught_signals;

/* If nonzero then exit with status WARNING, rather than with the usual
   signal status, on receipt of a signal with this value.  This
   suppresses a "Broken Pipe" message with some shells.  */
static int volatile exiting_signal;

/* If nonnegative, close this file descriptor and unlink ofname on error.  */
static int volatile remove_ofname_fd = -1;

off_t bytes_in;             /* number of input bytes */
off_t bytes_out;            /* number of output bytes */
off_t total_in;		    /* input bytes for all files */
off_t total_out;	    /* output bytes for all files */
char ifname[MAX_PATH_LEN]; /* input file name */
char ofname[MAX_PATH_LEN]; /* output file name */
struct stat istat;         /* status for input file */
int  ifd;                  /* input file descriptor */
int  ofd;                  /* output file descriptor */
unsigned insize;           /* valid bytes in inbuf */
unsigned inptr;            /* index of next byte to be processed in inbuf */
unsigned outcnt;           /* bytes in output buffer */

static int handled_sig[] =
  {
    /* SIGINT must be first, as 'foreground' depends on it.  */
    SIGINT

#ifdef SIGHUP
    , SIGHUP
#endif
#ifdef SIGPIPE
    , SIGPIPE
#else
# define SIGPIPE 0
#endif
#ifdef SIGTERM
    , SIGTERM
#endif
#ifdef SIGXCPU
    , SIGXCPU
#endif
#ifdef SIGXFSZ
    , SIGXFSZ
#endif
  };

struct option longopts[] =
{
 /* { name  has_arg  *flag  val } */
    {"ascii",      0, 0, 'a'}, /* ascii text mode */
    {"to-stdout",  0, 0, 'c'}, /* write output on standard output */
    {"stdout",     0, 0, 'c'}, /* write output on standard output */
    {"decompress", 0, 0, 'd'}, /* decompress */
    {"uncompress", 0, 0, 'd'}, /* decompress */
 /* {"encrypt",    0, 0, 'e'},    encrypt */
    {"force",      0, 0, 'f'}, /* force overwrite of output file */
    {"help",       0, 0, 'h'}, /* give help */
 /* {"pkzip",      0, 0, 'k'},    force output in pkzip format */
    {"list",       0, 0, 'l'}, /* list .gz file contents */
    {"license",    0, 0, 'L'}, /* display software license */
    {"no-name",    0, 0, 'n'}, /* don't save or restore original name & time */
    {"name",       0, 0, 'N'}, /* save or restore original name & time */
    {"quiet",      0, 0, 'q'}, /* quiet mode */
    {"silent",     0, 0, 'q'}, /* quiet mode */
    {"recursive",  0, 0, 'r'}, /* recurse through directories */
    {"suffix",     1, 0, 'S'}, /* use given suffix instead of .gz */
    {"test",       0, 0, 't'}, /* test compressed file integrity */
    {"no-time",    0, 0, 'T'}, /* don't save or restore the time stamp */
    {"verbose",    0, 0, 'v'}, /* verbose mode */
    {"version",    0, 0, 'V'}, /* display version number */
    {"fast",       0, 0, '1'}, /* compress faster */
    {"best",       0, 0, '9'}, /* compress better */
    {"lzw",        0, 0, 'Z'}, /* make output compatible with old compress */
    {"bits",       1, 0, 'b'}, /* max number of bits per code (implies -Z) */
    { 0, 0, 0, 0 }
};

/* local functions */

local void try_help     OF((void)) ATTRIBUTE_NORETURN;
local void help         OF((void));
local void license      OF((void));
local void version      OF((void));
local int input_eof	OF((void));
local void treat_stdin  OF((void));
local void treat_file   OF((char *iname));
local int create_outfile OF((void));
local char *get_suffix  OF((char *name));
local int  open_input_file OF((char *iname, struct stat *sbuf));
local int  make_ofname  OF((void));
local void shorten_name  OF((char *name));
local int  get_method   OF((int in));
local void do_list      OF((int ifd, int method));
local int  check_ofname OF((void));
local void copy_stat    OF((struct stat *ifstat));
local void install_signal_handlers OF((void));
local void remove_output_file OF((void));
local RETSIGTYPE abort_gzip_signal OF((int));
local void do_exit      OF((int exitcode)) ATTRIBUTE_NORETURN;
      int main          OF((int argc, char **argv));
int (*work) OF((int infile, int outfile)) = zip; /* function to call */

#if ! NO_DIR
local void treat_dir    OF((int fd, char *dir));
#endif

#define strequ(s1, s2) (strcmp((s1),(s2)) == 0)

static void
try_help ()
{
  fprintf (stderr, "Try `%s --help' for more information.\n",
	   program_name);
  do_exit (ERROR);
}

/* ======================================================================== */
local void help()
{
    static char  *help_msg[] = {
 "Compress or uncompress FILEs (by default, compress FILES in-place).",
 "",
 "Mandatory arguments to long options are mandatory for short options too.",
 "",
#if O_BINARY
 "  -a, --ascii       ascii text; convert end-of-line using local conventions",
#endif
 "  -c, --stdout      write on standard output, keep original files unchanged",
 "  -d, --decompress  decompress",
/*  -e, --encrypt     encrypt */
 "  -f, --force       force overwrite of output file and compress links",
 "  -h, --help        give this help",
/*  -k, --pkzip       force output in pkzip format */
 "  -l, --list        list compressed file contents",
 "  -L, --license     display software license",
#ifdef UNDOCUMENTED
 "  -m, --no-time     do not save or restore the original modification time",
 "  -M, --time        save or restore the original modification time",
#endif
 "  -n, --no-name     do not save or restore the original name and time stamp",
 "  -N, --name        save or restore the original name and time stamp",
 "  -q, --quiet       suppress all warnings",
#if ! NO_DIR
 "  -r, --recursive   operate recursively on directories",
#endif
 "  -S, --suffix=SUF  use suffix SUF on compressed files",
 "  -t, --test        test compressed file integrity",
 "  -v, --verbose     verbose mode",
 "  -V, --version     display version number",
 "  -1, --fast        compress faster",
 "  -9, --best        compress better",
#ifdef LZW
 "  -Z, --lzw         produce output compatible with old compress",
 "  -b, --bits=BITS   max number of bits per code (implies -Z)",
#endif
 "",
 "With no FILE, or when FILE is -, read standard input.",
 "",
 "Report bugs to <bug-gzip@gnu.org>.",
  0};
    char **p = help_msg;

    printf ("Usage: %s [OPTION]... [FILE]...\n", program_name);
    while (*p) printf ("%s\n", *p++);
}

/* ======================================================================== */
local void license()
{
    char **p = license_msg;

    printf ("%s %s\n", program_name, VERSION);
    while (*p) printf ("%s\n", *p++);
}

/* ======================================================================== */
local void version()
{
    license ();
    printf ("\n");
    printf ("Written by Jean-loup Gailly.\n");
}

local void progerror (string)
    char *string;
{
    int e = errno;
    fprintf (stderr, "%s: ", program_name);
    errno = e;
    perror(string);
    exit_code = ERROR;
}

/* ======================================================================== */
int main (argc, argv)
    int argc;
    char **argv;
{
    int file_count;     /* number of files to process */
    size_t proglen;     /* length of program_name */
    int optc;           /* current option */

    EXPAND(argc, argv); /* wild card expansion if necessary */

    program_name = gzip_base_name (argv[0]);
    proglen = strlen (program_name);

    /* Suppress .exe for MSDOS, OS/2 and VMS: */
    if (4 < proglen && strequ (program_name + proglen - 4, ".exe"))
      program_name[proglen - 4] = '\0';

    /* Add options in GZIP environment variable if there is one */
    env = add_envopt(&argc, &argv, OPTIONS_VAR);
    if (env != NULL) args = argv;

#ifndef GNU_STANDARD
# define GNU_STANDARD 1
#endif
#if !GNU_STANDARD
    /* For compatibility with old compress, use program name as an option.
     * Unless you compile with -DGNU_STANDARD=0, this program will behave as
     * gzip even if it is invoked under the name gunzip or zcat.
     *
     * Systems which do not support links can still use -d or -dc.
     * Ignore an .exe extension for MSDOS, OS/2 and VMS.
     */
    if (strncmp (program_name, "un",  2) == 0     /* ungzip, uncompress */
	|| strncmp (program_name, "gun", 3) == 0) /* gunzip */
	decompress = 1;
    else if (strequ (program_name + 1, "cat")     /* zcat, pcat, gcat */
	     || strequ (program_name, "gzcat"))   /* gzcat */
	decompress = to_stdout = 1;
#endif

    z_suffix = Z_SUFFIX;
    z_len = strlen(z_suffix);

    while ((optc = getopt_long (argc, argv, "ab:cdfhH?lLmMnNqrS:tvVZ123456789",
				longopts, (int *)0)) != -1) {
	switch (optc) {
        case 'a':
            ascii = 1; break;
	case 'b':
	    maxbits = atoi(optarg);
	    for (; *optarg; optarg++)
	      if (! ('0' <= *optarg && *optarg <= '9'))
		{
		  fprintf (stderr, "%s: -b operand is not an integer\n",
			   program_name);
		  try_help ();
		}
	    break;
	case 'c':
	    to_stdout = 1; break;
	case 'd':
	    decompress = 1; break;
	case 'f':
	    force++; break;
	case 'h': case 'H':
	    help(); do_exit(OK); break;
	case 'l':
	    list = decompress = to_stdout = 1; break;
	case 'L':
	    license(); do_exit(OK); break;
	case 'm': /* undocumented, may change later */
	    no_time = 1; break;
	case 'M': /* undocumented, may change later */
	    no_time = 0; break;
	case 'n':
	    no_name = no_time = 1; break;
	case 'N':
	    no_name = no_time = 0; break;
	case 'q':
	    quiet = 1; verbose = 0; break;
	case 'r':
#if NO_DIR
	    fprintf (stderr, "%s: -r not supported on this system\n",
		     program_name);
	    try_help ();
#else
	    recursive = 1;
#endif
	    break;
	case 'S':
#ifdef NO_MULTIPLE_DOTS
            if (*optarg == '.') optarg++;
#endif
            z_len = strlen(optarg);
	    z_suffix = optarg;
            break;
	case 't':
	    test = decompress = to_stdout = 1;
	    break;
	case 'v':
	    verbose++; quiet = 0; break;
	case 'V':
	    version(); do_exit(OK); break;
	case 'Z':
#ifdef LZW
	    do_lzw = 1; break;
#else
	    fprintf(stderr, "%s: -Z not supported in this version\n",
		    program_name);
	    try_help ();
	    break;
#endif
	case '1':  case '2':  case '3':  case '4':
	case '5':  case '6':  case '7':  case '8':  case '9':
	    level = optc - '0';
	    break;
	default:
	    /* Error message already emitted by getopt_long. */
	    try_help ();
	}
    } /* loop on all arguments */

    /* By default, save name and timestamp on compression but do not
     * restore them on decompression.
     */
    if (no_time < 0) no_time = decompress;
    if (no_name < 0) no_name = decompress;

    file_count = argc - optind;

#if O_BINARY
#else
    if (ascii && !quiet) {
	fprintf(stderr, "%s: option --ascii ignored on this system\n",
		program_name);
    }
#endif
    if ((z_len == 0 && !decompress) || z_len > MAX_SUFFIX) {
        fprintf(stderr, "%s: incorrect suffix '%s'\n",
                program_name, z_suffix);
        do_exit(ERROR);
    }
    if (do_lzw && !decompress) work = lzw;

    /* Allocate all global buffers (for DYN_ALLOC option) */
    ALLOC(uch, inbuf,  INBUFSIZ +INBUF_EXTRA);
    ALLOC(uch, outbuf, OUTBUFSIZ+OUTBUF_EXTRA);
    ALLOC(ush, d_buf,  DIST_BUFSIZE);
    ALLOC(uch, window, 2L*WSIZE);
#ifndef MAXSEG_64K
    ALLOC(ush, tab_prefix, 1L<<BITS);
#else
    ALLOC(ush, tab_prefix0, 1L<<(BITS-1));
    ALLOC(ush, tab_prefix1, 1L<<(BITS-1));
#endif

    exiting_signal = quiet ? SIGPIPE : 0;
    install_signal_handlers ();

    /* And get to work */
    if (file_count != 0) {
	if (to_stdout && !test && !list && (!decompress || !ascii)) {
	    SET_BINARY_MODE(fileno(stdout));
	}
        while (optind < argc) {
	    treat_file(argv[optind++]);
	}
    } else {  /* Standard input */
	treat_stdin();
    }
    if (list && !quiet && file_count > 1) {
	do_list(-1, -1); /* print totals */
    }
    do_exit(exit_code);
    return exit_code; /* just to avoid lint warning */
}

/* Return nonzero when at end of file on input.  */
local int
input_eof ()
{
  if (!decompress || last_member)
    return 1;

  if (inptr == insize)
    {
      if (insize != INBUFSIZ || fill_inbuf (1) == EOF)
	return 1;

      /* Unget the char that fill_inbuf got.  */
      inptr = 0;
    }

  return 0;
}

/* ========================================================================
 * Compress or decompress stdin
 */
local void treat_stdin()
{
    if (!force && !list &&
	isatty(fileno((FILE *)(decompress ? stdin : stdout)))) {
	/* Do not send compressed data to the terminal or read it from
	 * the terminal. We get here when user invoked the program
	 * without parameters, so be helpful. According to the GNU standards:
	 *
	 *   If there is one behavior you think is most useful when the output
	 *   is to a terminal, and another that you think is most useful when
	 *   the output is a file or a pipe, then it is usually best to make
	 *   the default behavior the one that is useful with output to a
	 *   terminal, and have an option for the other behavior.
	 *
	 * Here we use the --force option to get the other behavior.
	 */
	fprintf(stderr,
    "%s: compressed data not %s a terminal. Use -f to force %scompression.\n",
		program_name, decompress ? "read from" : "written to",
		decompress ? "de" : "");
	fprintf (stderr, "For help, type: %s -h\n", program_name);
	do_exit(ERROR);
    }

    if (decompress || !ascii) {
	SET_BINARY_MODE(fileno(stdin));
    }
    if (!test && !list && (!decompress || !ascii)) {
	SET_BINARY_MODE(fileno(stdout));
    }
    strcpy(ifname, "stdin");
    strcpy(ofname, "stdout");

    /* Get the file's time stamp and size.  */
    if (fstat (fileno (stdin), &istat) != 0)
      {
	progerror ("standard input");
	do_exit (ERROR);
      }
    ifile_size = S_ISREG (istat.st_mode) ? istat.st_size : -1;
    time_stamp.tv_nsec = -1;
    if (!no_time || list)
      time_stamp = get_stat_mtime (&istat);

    clear_bufs(); /* clear input and output buffers */
    to_stdout = 1;
    part_nb = 0;

    if (decompress) {
	method = get_method(ifd);
	if (method < 0) {
	    do_exit(exit_code); /* error message already emitted */
	}
    }
    if (list) {
        do_list(ifd, method);
        return;
    }

    /* Actually do the compression/decompression. Loop over zipped members.
     */
    for (;;) {
	if ((*work)(fileno(stdin), fileno(stdout)) != OK) return;

	if (input_eof ())
	  break;

	method = get_method(ifd);
	if (method < 0) return; /* error message already emitted */
	bytes_out = 0;            /* required for length check */
    }

    if (verbose) {
	if (test) {
	    fprintf(stderr, " OK\n");

	} else if (!decompress) {
	    display_ratio(bytes_in-(bytes_out-header_bytes), bytes_in, stderr);
	    fprintf(stderr, "\n");
#ifdef DISPLAY_STDIN_RATIO
	} else {
	    display_ratio(bytes_out-(bytes_in-header_bytes), bytes_out,stderr);
	    fprintf(stderr, "\n");
#endif
	}
    }
}

/* ========================================================================
 * Compress or decompress the given file
 */
local void treat_file(iname)
    char *iname;
{
    /* Accept "-" as synonym for stdin */
    if (strequ(iname, "-")) {
	int cflag = to_stdout;
	treat_stdin();
	to_stdout = cflag;
	return;
    }

    /* Check if the input file is present, set ifname and istat: */
    ifd = open_input_file (iname, &istat);
    if (ifd < 0)
      return;

    /* If the input name is that of a directory, recurse or ignore: */
    if (S_ISDIR(istat.st_mode)) {
#if ! NO_DIR
	if (recursive) {
	    treat_dir (ifd, iname);
	    /* Warning: ifname is now garbage */
	    return;
	}
#endif
	close (ifd);
	WARN ((stderr, "%s: %s is a directory -- ignored\n",
	       program_name, ifname));
	return;
    }

    if (! to_stdout)
      {
	if (! S_ISREG (istat.st_mode))
	  {
	    WARN ((stderr,
		   "%s: %s is not a directory or a regular file - ignored\n",
		   program_name, ifname));
	    close (ifd);
	    return;
	  }
	if (istat.st_mode & S_ISUID)
	  {
	    WARN ((stderr, "%s: %s is set-user-ID on execution - ignored\n",
		   program_name, ifname));
	    close (ifd);
	    return;
	  }
	if (istat.st_mode & S_ISGID)
	  {
	    WARN ((stderr, "%s: %s is set-group-ID on execution - ignored\n",
		   program_name, ifname));
	    close (ifd);
	    return;
	  }

	if (! force)
	  {
	    if (istat.st_mode & S_ISVTX)
	      {
		WARN ((stderr,
		       "%s: %s has the sticky bit set - file ignored\n",
		       program_name, ifname));
		close (ifd);
		return;
	      }
	    if (2 <= istat.st_nlink)
	      {
		WARN ((stderr, "%s: %s has %lu other link%c -- unchanged\n",
		       program_name, ifname,
		       (unsigned long int) istat.st_nlink - 1,
		       istat.st_nlink == 2 ? ' ' : 's'));
		close (ifd);
		return;
	      }
	  }
      }

    ifile_size = S_ISREG (istat.st_mode) ? istat.st_size : -1;
    time_stamp.tv_nsec = -1;
    if (!no_time || list)
      time_stamp = get_stat_mtime (&istat);

    /* Generate output file name. For -r and (-t or -l), skip files
     * without a valid gzip suffix (check done in make_ofname).
     */
    if (to_stdout && !list && !test) {
	strcpy(ofname, "stdout");

    } else if (make_ofname() != OK) {
	close (ifd);
	return;
    }

    clear_bufs(); /* clear input and output buffers */
    part_nb = 0;

    if (decompress) {
	method = get_method(ifd); /* updates ofname if original given */
	if (method < 0) {
	    close(ifd);
	    return;               /* error message already emitted */
	}
    }
    if (list) {
        do_list(ifd, method);
        if (close (ifd) != 0)
	  read_error ();
        return;
    }

    /* If compressing to a file, check if ofname is not ambiguous
     * because the operating system truncates names. Otherwise, generate
     * a new ofname and save the original name in the compressed file.
     */
    if (to_stdout) {
	ofd = fileno(stdout);
	/* Keep remove_ofname_fd negative.  */
    } else {
	if (create_outfile() != OK) return;

	if (!decompress && save_orig_name && !verbose && !quiet) {
	    fprintf(stderr, "%s: %s compressed to %s\n",
		    program_name, ifname, ofname);
	}
    }
    /* Keep the name even if not truncated except with --no-name: */
    if (!save_orig_name) save_orig_name = !no_name;

    if (verbose) {
	fprintf(stderr, "%s:\t", ifname);
    }

    /* Actually do the compression/decompression. Loop over zipped members.
     */
    for (;;) {
	if ((*work)(ifd, ofd) != OK) {
	    method = -1; /* force cleanup */
	    break;
	}

	if (input_eof ())
	  break;

	method = get_method(ifd);
	if (method < 0) break;    /* error message already emitted */
	bytes_out = 0;            /* required for length check */
    }

    if (close (ifd) != 0)
      read_error ();

    if (!to_stdout)
      {
	sigset_t oldset;
	int unlink_errno;

	copy_stat (&istat);
	if (close (ofd) != 0)
	  write_error ();

	sigprocmask (SIG_BLOCK, &caught_signals, &oldset);
	remove_ofname_fd = -1;
	unlink_errno = xunlink (ifname) == 0 ? 0 : errno;
	sigprocmask (SIG_SETMASK, &oldset, NULL);

	if (unlink_errno)
	  {
	    WARN ((stderr, "%s: ", program_name));
	    if (!quiet)
	      {
		errno = unlink_errno;
		perror (ifname);
	      }
	  }
      }

    if (method == -1) {
	if (!to_stdout)
	  remove_output_file ();
	return;
    }

    /* Display statistics */
    if(verbose) {
	if (test) {
	    fprintf(stderr, " OK");
	} else if (decompress) {
	    display_ratio(bytes_out-(bytes_in-header_bytes), bytes_out,stderr);
	} else {
	    display_ratio(bytes_in-(bytes_out-header_bytes), bytes_in, stderr);
	}
	if (!test && !to_stdout) {
	    fprintf(stderr, " -- replaced with %s", ofname);
	}
	fprintf(stderr, "\n");
    }
}

/* ========================================================================
 * Create the output file. Return OK or ERROR.
 * Try several times if necessary to avoid truncating the z_suffix. For
 * example, do not create a compressed file of name "1234567890123."
 * Sets save_orig_name to true if the file name has been truncated.
 * IN assertions: the input file has already been open (ifd is set) and
 *   ofname has already been updated if there was an original name.
 * OUT assertions: ifd and ofd are closed in case of error.
 */
local int create_outfile()
{
  int name_shortened = 0;
  int flags = (O_WRONLY | O_CREAT | O_EXCL
	       | (ascii && decompress ? 0 : O_BINARY));

  for (;;)
    {
      int open_errno;
      sigset_t oldset;

      sigprocmask (SIG_BLOCK, &caught_signals, &oldset);
      remove_ofname_fd = ofd = OPEN (ofname, flags, RW_USER);
      open_errno = errno;
      sigprocmask (SIG_SETMASK, &oldset, NULL);

      if (0 <= ofd)
	break;

      switch (open_errno)
	{
#ifdef ENAMETOOLONG
	case ENAMETOOLONG:
	  shorten_name (ofname);
	  name_shortened = 1;
	  break;
#endif

	case EEXIST:
	  if (check_ofname () != OK)
	    {
	      close (ifd);
	      return ERROR;
	    }
	  break;

	default:
	  progerror (ofname);
	  close (ifd);
	  return ERROR;
	}
    }

  if (name_shortened && decompress)
    {
      /* name might be too long if an original name was saved */
      WARN ((stderr, "%s: %s: warning, name truncated\n",
	     program_name, ofname));
    }

  return OK;
}

/* ========================================================================
 * Return a pointer to the 'z' suffix of a file name, or NULL. For all
 * systems, ".gz", ".z", ".Z", ".taz", ".tgz", "-gz", "-z" and "_z" are
 * accepted suffixes, in addition to the value of the --suffix option.
 * ".tgz" is a useful convention for tar.z files on systems limited
 * to 3 characters extensions. On such systems, ".?z" and ".??z" are
 * also accepted suffixes. For Unix, we do not want to accept any
 * .??z suffix as indicating a compressed file; some people use .xyz
 * to denote volume data.
 *   On systems allowing multiple versions of the same file (such as VMS),
 * this function removes any version suffix in the given name.
 */
local char *get_suffix(name)
    char *name;
{
    int nlen, slen;
    char suffix[MAX_SUFFIX+3]; /* last chars of name, forced to lower case */
    static char *known_suffixes[] =
       {NULL, ".gz", ".z", ".taz", ".tgz", "-gz", "-z", "_z",
#ifdef MAX_EXT_CHARS
          "z",
#endif
          NULL};
    char **suf = known_suffixes;

    *suf = z_suffix;
    if (strequ(z_suffix, "z")) suf++; /* check long suffixes first */

#ifdef SUFFIX_SEP
    /* strip a version number from the file name */
    {
	char *v = strrchr(name, SUFFIX_SEP);
 	if (v != NULL) *v = '\0';
    }
#endif
    nlen = strlen(name);
    if (nlen <= MAX_SUFFIX+2) {
        strcpy(suffix, name);
    } else {
        strcpy(suffix, name+nlen-MAX_SUFFIX-2);
    }
    strlwr(suffix);
    slen = strlen(suffix);
    do {
       int s = strlen(*suf);
       if (slen > s && suffix[slen-s-1] != PATH_SEP
           && strequ(suffix + slen - s, *suf)) {
           return name+nlen-s;
       }
    } while (*++suf != NULL);

    return NULL;
}


/* Open file NAME with the given flags and mode and store its status
   into *ST.  Return a file descriptor to the newly opened file, or -1
   (setting errno) on failure.  */
static int
open_and_stat (char *name, int flags, mode_t mode, struct stat *st)
{
  int fd;

  /* Refuse to follow symbolic links unless -c or -f.  */
  if (!to_stdout && !force)
    {
      if (HAVE_WORKING_O_NOFOLLOW)
	flags |= O_NOFOLLOW;
      else
	{
#if HAVE_LSTAT || defined lstat
	  if (lstat (name, st) != 0)
	    return -1;
	  else if (S_ISLNK (st->st_mode))
	    {
	      errno = ELOOP;
	      return -1;
	    }
#endif
	}
    }

  fd = OPEN (name, flags, mode);
  if (0 <= fd && fstat (fd, st) != 0)
    {
      int e = errno;
      close (fd);
      errno = e;
      return -1;
    }
  return fd;
}


/* ========================================================================
 * Set ifname to the input file name (with a suffix appended if necessary)
 * and istat to its stats. For decompression, if no file exists with the
 * original name, try adding successively z_suffix, .gz, .z, -z and .Z.
 * For MSDOS, we try only z_suffix and z.
 * Return an open file descriptor or -1.
 */
static int
open_input_file (iname, sbuf)
    char *iname;
    struct stat *sbuf;
{
    int ilen;  /* strlen(ifname) */
    int z_suffix_errno = 0;
    static char *suffixes[] = {NULL, ".gz", ".z", "-z", ".Z", NULL};
    char **suf = suffixes;
    char *s;
#ifdef NO_MULTIPLE_DOTS
    char *dot; /* pointer to ifname extension, or NULL */
#endif
    int fd;
    int open_flags = (O_RDONLY | O_NONBLOCK | O_NOCTTY
		      | (ascii && !decompress ? 0 : O_BINARY));

    *suf = z_suffix;

    if (sizeof ifname - 1 <= strlen (iname))
	goto name_too_long;

    strcpy(ifname, iname);

    /* If input file exists, return OK. */
    fd = open_and_stat (ifname, open_flags, RW_USER, sbuf);
    if (0 <= fd)
      return fd;

    if (!decompress || errno != ENOENT) {
	progerror(ifname);
	return -1;
    }
    /* file.ext doesn't exist, try adding a suffix (after removing any
     * version number for VMS).
     */
    s = get_suffix(ifname);
    if (s != NULL) {
	progerror(ifname); /* ifname already has z suffix and does not exist */
	return -1;
    }
#ifdef NO_MULTIPLE_DOTS
    dot = strrchr(ifname, '.');
    if (dot == NULL) {
        strcat(ifname, ".");
        dot = strrchr(ifname, '.');
    }
#endif
    ilen = strlen(ifname);
    if (strequ(z_suffix, ".gz")) suf++;

    /* Search for all suffixes */
    do {
        char *s0 = s = *suf;
        strcpy (ifname, iname);
#ifdef NO_MULTIPLE_DOTS
        if (*s == '.') s++;
        if (*dot == '\0') strcpy (dot, ".");
#endif
#ifdef MAX_EXT_CHARS
	if (MAX_EXT_CHARS < strlen (s) + strlen (dot + 1))
	  dot[MAX_EXT_CHARS + 1 - strlen (s)] = '\0';
#endif
	if (sizeof ifname <= ilen + strlen (s))
	  goto name_too_long;
        strcat(ifname, s);
	fd = open_and_stat (ifname, open_flags, RW_USER, sbuf);
	if (0 <= fd)
	  return fd;
	if (errno != ENOENT)
	  {
	    progerror (ifname);
	    return -1;
	  }
	if (strequ (s0, z_suffix))
	  z_suffix_errno = errno;
    } while (*++suf != NULL);

    /* No suffix found, complain using z_suffix: */
    strcpy(ifname, iname);
#ifdef NO_MULTIPLE_DOTS
    if (*dot == '\0') strcpy(dot, ".");
#endif
#ifdef MAX_EXT_CHARS
    if (MAX_EXT_CHARS < z_len + strlen (dot + 1))
      dot[MAX_EXT_CHARS + 1 - z_len] = '\0';
#endif
    strcat(ifname, z_suffix);
    errno = z_suffix_errno;
    progerror(ifname);
    return -1;

 name_too_long:
    fprintf (stderr, "%s: %s: file name too long\n", program_name, iname);
    exit_code = ERROR;
    return -1;
}

/* ========================================================================
 * Generate ofname given ifname. Return OK, or WARNING if file must be skipped.
 * Sets save_orig_name to true if the file name has been truncated.
 */
local int make_ofname()
{
    char *suff;            /* ofname z suffix */

    strcpy(ofname, ifname);
    /* strip a version number if any and get the gzip suffix if present: */
    suff = get_suffix(ofname);

    if (decompress) {
	if (suff == NULL) {
	    /* With -t or -l, try all files (even without .gz suffix)
	     * except with -r (behave as with just -dr).
             */
            if (!recursive && (list || test)) return OK;

	    /* Avoid annoying messages with -r */
	    if (verbose || (!recursive && !quiet)) {
		WARN((stderr,"%s: %s: unknown suffix -- ignored\n",
		      program_name, ifname));
	    }
	    return WARNING;
	}
	/* Make a special case for .tgz and .taz: */
	strlwr(suff);
	if (strequ(suff, ".tgz") || strequ(suff, ".taz")) {
	    strcpy(suff, ".tar");
	} else {
	    *suff = '\0'; /* strip the z suffix */
	}
        /* ofname might be changed later if infile contains an original name */

    } else if (suff != NULL) {
	/* Avoid annoying messages with -r (see treat_dir()) */
	if (verbose || (!recursive && !quiet)) {
	    /* Don't use WARN, as it affects exit status.  */
	    fprintf (stderr, "%s: %s already has %s suffix -- unchanged\n",
		     program_name, ifname, suff);
	}
	return WARNING;
    } else {
        save_orig_name = 0;

#ifdef NO_MULTIPLE_DOTS
	suff = strrchr(ofname, '.');
	if (suff == NULL) {
	    if (sizeof ofname <= strlen (ofname) + 1)
		goto name_too_long;
            strcat(ofname, ".");
#  ifdef MAX_EXT_CHARS
	    if (strequ(z_suffix, "z")) {
		if (sizeof ofname <= strlen (ofname) + 2)
		    goto name_too_long;
		strcat(ofname, "gz"); /* enough room */
		return OK;
	    }
        /* On the Atari and some versions of MSDOS,
         * ENAMETOOLONG does not work correctly.  So we
         * must truncate here.
         */
        } else if (strlen(suff)-1 + z_len > MAX_SUFFIX) {
            suff[MAX_SUFFIX+1-z_len] = '\0';
            save_orig_name = 1;
#  endif
        }
#endif /* NO_MULTIPLE_DOTS */
	if (sizeof ofname <= strlen (ofname) + z_len)
	    goto name_too_long;
	strcat(ofname, z_suffix);

    } /* decompress ? */
    return OK;

 name_too_long:
    WARN ((stderr, "%s: %s: file name too long\n", program_name, ifname));
    return WARNING;
}


/* ========================================================================
 * Check the magic number of the input file and update ofname if an
 * original name was given and to_stdout is not set.
 * Return the compression method, -1 for error, -2 for warning.
 * Set inptr to the offset of the next byte to be processed.
 * Updates time_stamp if there is one and --no-time is not used.
 * This function may be called repeatedly for an input file consisting
 * of several contiguous gzip'ed members.
 * IN assertions: there is at least one remaining compressed member.
 *   If the member is a zip file, it must be the only one.
 */
local int get_method(in)
    int in;        /* input file descriptor */
{
    uch flags;     /* compression flags */
    char magic[2]; /* magic header */
    int imagic1;   /* like magic[1], but can represent EOF */
    ulg stamp;     /* time stamp */

    /* If --force and --stdout, zcat == cat, so do not complain about
     * premature end of file: use try_byte instead of get_byte.
     */
    if (force && to_stdout) {
	magic[0] = (char)try_byte();
	imagic1 = try_byte ();
	magic[1] = (char) imagic1;
	/* If try_byte returned EOF, magic[1] == (char) EOF.  */
    } else {
	magic[0] = (char)get_byte();
	magic[1] = (char)get_byte();
	imagic1 = 0; /* avoid lint warning */
    }
    method = -1;                 /* unknown yet */
    part_nb++;                   /* number of parts in gzip file */
    header_bytes = 0;
    last_member = RECORD_IO;
    /* assume multiple members in gzip file except for record oriented I/O */

    if (memcmp(magic, GZIP_MAGIC, 2) == 0
        || memcmp(magic, OLD_GZIP_MAGIC, 2) == 0) {

	method = (int)get_byte();
	if (method != DEFLATED) {
	    fprintf(stderr,
		    "%s: %s: unknown method %d -- not supported\n",
		    program_name, ifname, method);
	    exit_code = ERROR;
	    return -1;
	}
	work = unzip;
	flags  = (uch)get_byte();

	if ((flags & ENCRYPTED) != 0) {
	    fprintf(stderr,
		    "%s: %s is encrypted -- not supported\n",
		    program_name, ifname);
	    exit_code = ERROR;
	    return -1;
	}
	if ((flags & CONTINUATION) != 0) {
	    fprintf(stderr,
		    "%s: %s is a a multi-part gzip file -- not supported\n",
		    program_name, ifname);
	    exit_code = ERROR;
	    if (force <= 1) return -1;
	}
	if ((flags & RESERVED) != 0) {
	    fprintf(stderr,
		    "%s: %s has flags 0x%x -- not supported\n",
		    program_name, ifname, flags);
	    exit_code = ERROR;
	    if (force <= 1) return -1;
	}
	stamp  = (ulg)get_byte();
	stamp |= ((ulg)get_byte()) << 8;
	stamp |= ((ulg)get_byte()) << 16;
	stamp |= ((ulg)get_byte()) << 24;
	if (stamp != 0 && !no_time)
	  {
	    time_stamp.tv_sec = stamp;
	    time_stamp.tv_nsec = 0;
	  }

	(void)get_byte();  /* Ignore extra flags for the moment */
	(void)get_byte();  /* Ignore OS type for the moment */

	if ((flags & CONTINUATION) != 0) {
	    unsigned part = (unsigned)get_byte();
	    part |= ((unsigned)get_byte())<<8;
	    if (verbose) {
		fprintf(stderr,"%s: %s: part number %u\n",
			program_name, ifname, part);
	    }
	}
	if ((flags & EXTRA_FIELD) != 0) {
	    unsigned len = (unsigned)get_byte();
	    len |= ((unsigned)get_byte())<<8;
	    if (verbose) {
		fprintf(stderr,"%s: %s: extra field of %u bytes ignored\n",
			program_name, ifname, len);
	    }
	    while (len--) (void)get_byte();
	}

	/* Get original file name if it was truncated */
	if ((flags & ORIG_NAME) != 0) {
	    if (no_name || (to_stdout && !list) || part_nb > 1) {
		/* Discard the old name */
		char c; /* dummy used for NeXTstep 3.0 cc optimizer bug */
		do {c=get_byte();} while (c != 0);
	    } else {
		/* Copy the base name. Keep a directory prefix intact. */
                char *p = gzip_base_name (ofname);
                char *base = p;
		for (;;) {
		    *p = (char)get_char();
		    if (*p++ == '\0') break;
		    if (p >= ofname+sizeof(ofname)) {
			gzip_error ("corrupted input -- file name too large");
		    }
		}
		p = gzip_base_name (base);
		memmove (base, p, strlen (p) + 1);
                /* If necessary, adapt the name to local OS conventions: */
                if (!list) {
                   MAKE_LEGAL_NAME(base);
		   if (base) list=0; /* avoid warning about unused variable */
                }
	    } /* no_name || to_stdout */
	} /* ORIG_NAME */

	/* Discard file comment if any */
	if ((flags & COMMENT) != 0) {
	    while (get_char() != 0) /* null */ ;
	}
	if (part_nb == 1) {
	    header_bytes = inptr + 2*sizeof(long); /* include crc and size */
	}

    } else if (memcmp(magic, PKZIP_MAGIC, 2) == 0 && inptr == 2
	    && memcmp((char*)inbuf, PKZIP_MAGIC, 4) == 0) {
	/* To simplify the code, we support a zip file when alone only.
         * We are thus guaranteed that the entire local header fits in inbuf.
         */
        inptr = 0;
	work = unzip;
	if (check_zipfile(in) != OK) return -1;
	/* check_zipfile may get ofname from the local header */
	last_member = 1;

    } else if (memcmp(magic, PACK_MAGIC, 2) == 0) {
	work = unpack;
	method = PACKED;

    } else if (memcmp(magic, LZW_MAGIC, 2) == 0) {
	work = unlzw;
	method = COMPRESSED;
	last_member = 1;

    } else if (memcmp(magic, LZH_MAGIC, 2) == 0) {
	work = unlzh;
	method = LZHED;
	last_member = 1;

    } else if (force && to_stdout && !list) { /* pass input unchanged */
	method = STORED;
	work = copy;
        inptr = 0;
	last_member = 1;
    }
    if (method >= 0) return method;

    if (part_nb == 1) {
	fprintf (stderr, "\n%s: %s: not in gzip format\n",
		 program_name, ifname);
	exit_code = ERROR;
	return -1;
    } else {
	if (magic[0] == 0)
	  {
	    int inbyte;
	    for (inbyte = imagic1;  inbyte == 0;  inbyte = try_byte ())
	      continue;
	    if (inbyte == EOF)
	      {
		if (verbose)
		  WARN ((stderr, "\n%s: %s: decompression OK, trailing zero bytes ignored\n",
			 program_name, ifname));
		return -3;
	      }
	  }

	WARN((stderr, "\n%s: %s: decompression OK, trailing garbage ignored\n",
	      program_name, ifname));
	return -2;
    }
}

/* ========================================================================
 * Display the characteristics of the compressed file.
 * If the given method is < 0, display the accumulated totals.
 * IN assertions: time_stamp, header_bytes and ifile_size are initialized.
 */
local void do_list(ifd, method)
    int ifd;     /* input file descriptor */
    int method;  /* compression method */
{
    ulg crc;  /* original crc */
    static int first_time = 1;
    static char* methods[MAX_METHODS] = {
        "store",  /* 0 */
        "compr",  /* 1 */
        "pack ",  /* 2 */
        "lzh  ",  /* 3 */
        "", "", "", "", /* 4 to 7 reserved */
        "defla"}; /* 8 */
    int positive_off_t_width = 1;
    off_t o;

    for (o = OFF_T_MAX;  9 < o;  o /= 10) {
	positive_off_t_width++;
    }

    if (first_time && method >= 0) {
	first_time = 0;
	if (verbose)  {
	    printf("method  crc     date  time  ");
	}
	if (!quiet) {
	    printf("%*.*s %*.*s  ratio uncompressed_name\n",
		   positive_off_t_width, positive_off_t_width, "compressed",
		   positive_off_t_width, positive_off_t_width, "uncompressed");
	}
    } else if (method < 0) {
	if (total_in <= 0 || total_out <= 0) return;
	if (verbose) {
	    printf("                            ");
	}
	if (verbose || !quiet) {
	    fprint_off(stdout, total_in, positive_off_t_width);
	    printf(" ");
	    fprint_off(stdout, total_out, positive_off_t_width);
	    printf(" ");
	}
	display_ratio(total_out-(total_in-header_bytes), total_out, stdout);
	/* header_bytes is not meaningful but used to ensure the same
	 * ratio if there is a single file.
	 */
	printf(" (totals)\n");
	return;
    }
    crc = (ulg)~0; /* unknown */
    bytes_out = -1L;
    bytes_in = ifile_size;

#if RECORD_IO == 0
    if (method == DEFLATED && !last_member) {
        /* Get the crc and uncompressed size for gzip'ed (not zip'ed) files.
         * If the lseek fails, we could use read() to get to the end, but
         * --list is used to get quick results.
         * Use "gunzip < foo.gz | wc -c" to get the uncompressed size if
         * you are not concerned about speed.
         */
        bytes_in = lseek(ifd, (off_t)(-8), SEEK_END);
        if (bytes_in != -1L) {
            uch buf[8];
            bytes_in += 8L;
            if (read(ifd, (char*)buf, sizeof(buf)) != sizeof(buf)) {
                read_error();
            }
            crc       = LG(buf);
	    bytes_out = LG(buf+4);
	}
    }
#endif /* RECORD_IO */
    if (verbose)
      {
	struct tm *tm = localtime (&time_stamp.tv_sec);
	printf ("%5s %08lx ", methods[method], crc);
	if (tm)
	  printf ("%s%3d %02d:%02d ",
		  ("Jan\0Feb\0Mar\0Apr\0May\0Jun\0Jul\0Aug\0Sep\0Oct\0Nov\0Dec"
		   + 4 * tm->tm_mon),
		  tm->tm_mday, tm->tm_hour, tm->tm_min);
	else
	  printf ("??? ?? ??:?? ");
      }
    fprint_off(stdout, bytes_in, positive_off_t_width);
    printf(" ");
    fprint_off(stdout, bytes_out, positive_off_t_width);
    printf(" ");
    if (bytes_in  == -1L) {
	total_in = -1L;
	bytes_in = bytes_out = header_bytes = 0;
    } else if (total_in >= 0) {
	total_in  += bytes_in;
    }
    if (bytes_out == -1L) {
	total_out = -1L;
	bytes_in = bytes_out = header_bytes = 0;
    } else if (total_out >= 0) {
	total_out += bytes_out;
    }
    display_ratio(bytes_out-(bytes_in-header_bytes), bytes_out, stdout);
    printf(" %s\n", ofname);
}

/* ========================================================================
 * Shorten the given name by one character, or replace a .tar extension
 * with .tgz. Truncate the last part of the name which is longer than
 * MIN_PART characters: 1234.678.012.gz -> 123.678.012.gz. If the name
 * has only parts shorter than MIN_PART truncate the longest part.
 * For decompression, just remove the last character of the name.
 *
 * IN assertion: for compression, the suffix of the given name is z_suffix.
 */
local void shorten_name(name)
    char *name;
{
    int len;                 /* length of name without z_suffix */
    char *trunc = NULL;      /* character to be truncated */
    int plen;                /* current part length */
    int min_part = MIN_PART; /* current minimum part length */
    char *p;

    len = strlen(name);
    if (decompress) {
	if (len <= 1)
	  gzip_error ("name too short");
	name[len-1] = '\0';
	return;
    }
    p = get_suffix(name);
    if (! p)
      gzip_error ("can't recover suffix\n");
    *p = '\0';
    save_orig_name = 1;

    /* compress 1234567890.tar to 1234567890.tgz */
    if (len > 4 && strequ(p-4, ".tar")) {
	strcpy(p-4, ".tgz");
	return;
    }
    /* Try keeping short extensions intact:
     * 1234.678.012.gz -> 123.678.012.gz
     */
    do {
	p = strrchr(name, PATH_SEP);
	p = p ? p+1 : name;
	while (*p) {
	    plen = strcspn(p, PART_SEP);
	    p += plen;
	    if (plen > min_part) trunc = p-1;
	    if (*p) p++;
	}
    } while (trunc == NULL && --min_part != 0);

    if (trunc != NULL) {
	do {
	    trunc[0] = trunc[1];
	} while (*trunc++);
	trunc--;
    } else {
	trunc = strrchr(name, PART_SEP[0]);
	if (!trunc)
	  gzip_error ("internal error in shorten_name");
	if (trunc[1] == '\0') trunc--; /* force truncation */
    }
    strcpy(trunc, z_suffix);
}

/* ========================================================================
 * The compressed file already exists, so ask for confirmation.
 * Return ERROR if the file must be skipped.
 */
local int check_ofname()
{
    /* Ask permission to overwrite the existing file */
    if (!force) {
	int ok = 0;
	fprintf (stderr, "%s: %s already exists;", program_name, ofname);
	if (foreground && isatty(fileno(stdin))) {
	    fprintf(stderr, " do you wish to overwrite (y or n)? ");
	    fflush(stderr);
	    ok = yesno();
	}
	if (!ok) {
	    fprintf(stderr, "\tnot overwritten\n");
	    if (exit_code == OK) exit_code = WARNING;
	    return ERROR;
	}
    }
    if (xunlink (ofname)) {
	progerror(ofname);
	return ERROR;
    }
    return OK;
}


/* ========================================================================
 * Copy modes, times, ownership from input file to output file.
 * IN assertion: to_stdout is false.
 */
local void copy_stat(ifstat)
    struct stat *ifstat;
{
    mode_t mode = ifstat->st_mode & S_IRWXUGO;
    int r;

#ifndef NO_UTIME
    struct timespec timespec[2];
    timespec[0] = get_stat_atime (ifstat);
    timespec[1] = get_stat_mtime (ifstat);

    if (decompress && 0 <= time_stamp.tv_nsec
	&& ! (timespec[1].tv_sec == time_stamp.tv_sec
	      && timespec[1].tv_nsec == time_stamp.tv_nsec))
      {
	timespec[1] = time_stamp;
	if (verbose > 1) {
	    fprintf(stderr, "%s: time stamp restored\n", ofname);
	}
      }

    if (futimens (ofd, ofname, timespec) != 0)
      {
	int e = errno;
	WARN ((stderr, "%s: ", program_name));
	if (!quiet)
	  {
	    errno = e;
	    perror (ofname);
	  }
      }
#endif

#ifndef NO_CHOWN
# if HAVE_FCHOWN
    fchown (ofd, ifstat->st_uid, ifstat->st_gid);  /* Copy ownership */
# elif HAVE_CHOWN
    chown(ofname, ifstat->st_uid, ifstat->st_gid);  /* Copy ownership */
# endif
#endif

    /* Copy the protection modes */
#if HAVE_FCHMOD
    r = fchmod (ofd, mode);
#else
    r = chmod (ofname, mode);
#endif
    if (r != 0) {
	int e = errno;
	WARN ((stderr, "%s: ", program_name));
	if (!quiet) {
	    errno = e;
	    perror(ofname);
	}
    }
}

#if ! NO_DIR

/* ========================================================================
 * Recurse through the given directory. This code is taken from ncompress.
 */
local void treat_dir (fd, dir)
    int fd;
    char *dir;
{
    struct dirent *dp;
    DIR      *dirp;
    char     nbuf[MAX_PATH_LEN];
    int      len;

#if HAVE_FDOPENDIR
    dirp = fdopendir (fd);
#else
    close (fd);
    dirp = opendir(dir);
#endif

    if (dirp == NULL) {
	progerror(dir);
#if HAVE_FDOPENDIR
	close (fd);
#endif
	return ;
    }
    /*
     ** WARNING: the following algorithm could occasionally cause
     ** compress to produce error warnings of the form "<filename>.gz
     ** already has .gz suffix - ignored". This occurs when the
     ** .gz output file is inserted into the directory below
     ** readdir's current pointer.
     ** These warnings are harmless but annoying, so they are suppressed
     ** with option -r (except when -v is on). An alternative
     ** to allowing this would be to store the entire directory
     ** list in memory, then compress the entries in the stored
     ** list. Given the depth-first recursive algorithm used here,
     ** this could use up a tremendous amount of memory. I don't
     ** think it's worth it. -- Dave Mack
     ** (An other alternative might be two passes to avoid depth-first.)
     */

    while ((errno = 0, dp = readdir(dirp)) != NULL) {

	if (strequ(dp->d_name,".") || strequ(dp->d_name,"..")) {
	    continue;
	}
	len = strlen(dir);
	if (len + _D_EXACT_NAMLEN (dp) + 1 < MAX_PATH_LEN - 1) {
	    strcpy(nbuf,dir);
	    if (len != 0 /* dir = "" means current dir on Amiga */
#ifdef PATH_SEP2
		&& dir[len-1] != PATH_SEP2
#endif
#ifdef PATH_SEP3
		&& dir[len-1] != PATH_SEP3
#endif
	    ) {
		nbuf[len++] = PATH_SEP;
	    }
	    strcpy(nbuf+len, dp->d_name);
	    treat_file(nbuf);
	} else {
	    fprintf(stderr,"%s: %s/%s: pathname too long\n",
		    program_name, dir, dp->d_name);
	    exit_code = ERROR;
	}
    }
    if (errno != 0)
	progerror(dir);
    if (CLOSEDIR(dirp) != 0)
	progerror(dir);
}
#endif /* ! NO_DIR */

/* Make sure signals get handled properly.  */

static void
install_signal_handlers ()
{
  int nsigs = sizeof handled_sig / sizeof handled_sig[0];
  int i;

#if SA_NOCLDSTOP
  struct sigaction act;

  sigemptyset (&caught_signals);
  for (i = 0; i < nsigs; i++)
    {
      sigaction (handled_sig[i], NULL, &act);
      if (act.sa_handler != SIG_IGN)
	sigaddset (&caught_signals, handled_sig[i]);
    }

  act.sa_handler = abort_gzip_signal;
  act.sa_mask = caught_signals;
  act.sa_flags = 0;

  for (i = 0; i < nsigs; i++)
    if (sigismember (&caught_signals, handled_sig[i]))
      {
	if (i == 0)
	  foreground = 1;
	sigaction (handled_sig[i], &act, NULL);
      }
#else
  for (i = 0; i < nsigs; i++)
    if (signal (handled_sig[i], SIG_IGN) != SIG_IGN)
      {
	if (i == 0)
	  foreground = 1;
	signal (handled_sig[i], abort_gzip_signal);
	siginterrupt (handled_sig[i], 1);
      }
#endif
}

/* ========================================================================
 * Free all dynamically allocated variables and exit with the given code.
 */
local void do_exit(exitcode)
    int exitcode;
{
    static int in_exit = 0;

    if (in_exit) exit(exitcode);
    in_exit = 1;
    if (env != NULL)  free(env),  env  = NULL;
    if (args != NULL) free((char*)args), args = NULL;
    FREE(inbuf);
    FREE(outbuf);
    FREE(d_buf);
    FREE(window);
#ifndef MAXSEG_64K
    FREE(tab_prefix);
#else
    FREE(tab_prefix0);
    FREE(tab_prefix1);
#endif
    exit(exitcode);
}

/* ========================================================================
 * Close and unlink the output file.
 */
static void
remove_output_file ()
{
  int fd;
  sigset_t oldset;

  sigprocmask (SIG_BLOCK, &caught_signals, &oldset);
  fd = remove_ofname_fd;
  if (0 <= fd)
    {
      remove_ofname_fd = -1;
      close (fd);
      xunlink (ofname);
    }
  sigprocmask (SIG_SETMASK, &oldset, NULL);
}

/* ========================================================================
 * Error handler.
 */
void
abort_gzip ()
{
   remove_output_file ();
   do_exit(ERROR);
}

/* ========================================================================
 * Signal handler.
 */
static RETSIGTYPE
abort_gzip_signal (sig)
     int sig;
{
  if (! SA_NOCLDSTOP)
    signal (sig, SIG_IGN);
   remove_output_file ();
   if (sig == exiting_signal)
     _exit (WARNING);
   signal (sig, SIG_DFL);
   raise (sig);
}
