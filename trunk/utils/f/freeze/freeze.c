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





#include <stdlib.h>
#include "freeze.h"
#include "lz.h"
#include "huf.h"
#include "patchlevel.h"
#include <util/util_limits.h>

/*
 * Freeze - data freezing program
 * Version 1.0:
 * This program is made from GNU compress.c and Yoshizaki/Tagawa's
 * lzhuf.c. (Thanks to all of them.)
 * The algorithm is modified for using in pipe
 * (added ENDOF symbol in Huffman table).
 * Version 1.1:
 * Check for lack of bytes in frozen file when melting.
 * Put the GetBit routine into DecodeChar for reduce function-
 * call overhead when melting.
 * Version 1.2:
 * Added delayed coding a la COMIC.
 * Now freeze works on Intels (*NIX, Microsoft, Turbo),
 * Sun (SunOS).
 * Version 2.0:
 * Buffer size is now 8192 bytes, maximum match length - 256 bytes.
 * Improved hash function (with tuning of hash-table)
 * Version 2.1: Noticeable speedup: Insert_Node and Get_Next_Match
 * are now separated.
 * Version 2.2: Tunable static Huffman table for position information,
 * this info may be given in the command string now.
 */

#ifdef COMPAT
uchar magic_header[] = { "\037\236" };      /* 1F 9E = compress + 1 */
#else
uchar magic_header[] = { "\037\237" };      /* 1F 9F = freeze 1.X + 1 */
#endif

static char ident[] = "@(#) freeze.c 2.2.%d Alpha 5/13/91 leo@s514.ipmce.su\n";

#define ARGVAL() (*++(*argv) || (--argc && *++argv))

int exit_stat = 0;

void Usage() {
#ifdef DEBUG

# ifdef MSDOS
    fprintf(stderr,"Usage: freeze [-cdDfitvV] [file | +type ...]\n");
# else
    fprintf(stderr,"Usage: freeze [-cdDfvV] [file | +type ...]\n");
# endif /* MSDOS */

#else

# ifdef MSDOS
    fprintf(stderr,"Usage: freeze [-cdfitvV] [file | +type ...]\n");
# else
    fprintf(stderr,"Usage: freeze [-cdfvV] [file | +type ...]\n");
# endif /* MSDOS */

#endif /* DEBUG */
}

short topipe = 0;       /* Write output on stdout, suppress messages */
short precious = 1;       /* Don't unlink output file on interrupt */
short quiet = 1;          /* Don't tell me about freezing */

#ifdef COMPAT
short new_flg;            /* File is frozen using BIG freeze */
#endif

short force = 0;
#ifndef PATH_MAX
#define PATH_MAX 100
#endif
char ofname [PATH_MAX];

#ifdef MSDOS
   char *last_sep();	/* last slash, backslash, or colon */
# ifdef BIN_DEFAULT
	short image = O_BINARY;
# else
	short image = O_TEXT;
# endif
#else
#  define last_sep(s) freeze_rindex((s), '/')	/* Unix always uses slashes */
char deffile[] = "/etc/default/freeze";
#endif

#ifdef DEBUG
short debug = 0;
short verbose = 0;
char * pr_char();
long symbols_out = 0, refers_out = 0;
#endif /* DEBUG */

unsigned long indicator_count, indicator_threshold;

#ifdef INT_SIG
int
#else
void
#endif
(*bgnd_flag)();

short do_melt = 0;

/*****************************************************************
 * TAG( main )
 *
 *
 * Usage: freeze [-cdfivV] [-t type] [file ...]
 * Inputs:
 *
 *	-c:	    Write output on stdout, don't remove original.
 *
 *      -d:	 If given, melting is done instead.
 *
 *	-f:	    Forces output file to be generated, even if one already
 *		  exists, and even if no space is saved by freezeing.
 *		    If -f is not used, the user will be prompted if stdin is
 *		    a tty, otherwise, the output file will not be overwritten.
 *
 *	-i:	    Image mode (defined only under MS-DOS).  Prevents
 *		    conversion between UNIX text representation (LF line
 *		  termination) in frozen form and MS-DOS text
 *		  representation (CR-LF line termination) in melted
 *		    form.  Useful with non-text files.  Default if
 *		  BIN_DEFAULT specified.
 *
 *	-b:	   Binary mode.  Synonym for -i.  MS-DOS only.
 *
 *	-t:	   Text mode (defined only under MS-DOS).  Treat file
 *		   as text (CR-LF and ^Z special) in melted form.  Default
 *		   unless BIN_DEFAULT specified.
 *
 *      -v:             Write freezing statistics
 *
 *	-V:	    Write version and compilation options.
 *
 *      file ...:   Files to be frozen.  If none specified, stdin
 *		    is used.
 * Outputs:
 *      file.F:     Frozen form of file with same mode, owner, and utimes
 *	or stdout   (if stdin used as input)
 *
 * Assumptions:
 *      When filenames are given, replaces with the frozen version
 *      (.F suffix) only if the file decreases in size.
 * Algorithm:
 *      Modified Lempel-Ziv-SS method (LZSS), adaptive Huffman coding
 *      for literal symbols and length info.
 *      Static Huffman coding for position info. (It is optimal ?)
 *      Lower bits of position info are put in output
 *      file without any coding because of their random distribution.
 */

/* From compress.c. Replace .Z --> .F etc */

int main( argc, argv )
register int argc; char **argv;
{
    short overwrite = 0;  /* Do not overwrite unless given -f flag */
    char tempname[PATH_MAX];
    char **filelist, **fileptr;
    char *cp;
#ifndef MSDOS
#ifndef __QNXNTO__
char *malloc();
#endif
#endif
    struct stat statbuf;
#if defined(__TURBOC__) || !defined(INT_SIG)
    extern void onintr(int);
#else
    extern onintr(int);
#endif


#ifdef MSDOS
    char *sufp;
#else
#ifdef INT_SIG
    extern oops(int);
#else
    extern void oops(int);
#endif
#endif

#ifndef MSDOS
    if ( (bgnd_flag = signal ( SIGINT, SIG_IGN )) != SIG_IGN )
#endif
    {
	signal ( SIGINT, onintr );
#ifdef __TURBOC__
	setcbrk(1);
#endif
#ifndef MSDOS
	signal ( SIGSEGV, oops );
#endif
    }

    filelist = fileptr = (char **)(malloc(argc * sizeof(*argv)));
    *filelist = NULL;

    if((cp = last_sep(argv[0])) != 0) {
	cp++;
    } else {
	cp = argv[0];
    }

#ifdef MSDOS
		/* use case-insensitive match: parent may not be command.com */
    if(stricmp(cp, "melt.exe") == 0) {
#else
    if(strcmp(cp, "melt") == 0) {
#endif

	do_melt = 1;
	
#ifdef MSDOS
		/* use case-insensitive match: parent may not be command.com */
    } else if(stricmp(cp, "fcat.exe") == 0) {
#else
    } else if(strcmp(cp, "fcat") == 0) {
#endif

	do_melt = 1;
	topipe = 1;

    } else {
	/* Freezing */
#ifndef MSDOS
	defopen(deffile);
#else
	cp = freeze_rindex(cp, '.');
	*++cp = 'C';
	*++cp = 'N';
	*++cp = 'F';
	*++cp = '\0';
	defopen(argv[0]);
#endif
    }
#ifdef BSD4_2
    /* 4.2BSD dependent - take it out if not */
    setlinebuf( stderr );
#endif /* BSD4_2 */

    /* Argument Processing
     * All flags are optional.
     * -D => debug
     * -V => print Version; debug verbose
     * -d => do_melt
     * -v => unquiet
     * -f => force overwrite of output file
     * -c => cat all output to stdout
     * if a string is left, must be an input filename.
     */
    for (argc--, argv++; argc > 0; argc--, argv++) {
	if (**argv == '-') {	/* A flag argument */
	    while (*++(*argv)) {	/* Process all flags in this arg */
		switch (**argv) {
#ifdef DEBUG
		    case 'D':
			debug = 1;
			break;
		    case 'V':
			verbose = 1;
#else
		    case 'V':
			version();
#endif /* DEBUG */
			break;
#ifdef MSDOS
		    case 'i':
		    case 'b':
			image = O_BINARY;	/* binary (aka image) mode */
			break;

		    case 't':			/* text mode */
			image = O_TEXT;
			break;
#endif

		    case 'v':
			quiet = 0;
			break;
		    case 'd':
			do_melt = 1;
			break;
		    case 'f':
		    case 'F':
			overwrite = 1;
			force = 1;
			break;
		    case 'c':
			topipe = 1;
			break;
		    case 'q':
			quiet = 1;
			break;
#ifdef __QNX__
		    case 'z':
			setvbuf(stdout, 0, _IOFBF, 4096);
			break;
#endif
		    default:
			fprintf(stderr, "Unknown flag: '%c'; ", **argv);
			Usage();
			exit(1);
		}
	    }
	}
	else {		/* Input file name */
	    *fileptr++ = *argv; /* Build input file list */
	    *fileptr = NULL;
	}
    }

# ifdef DEBUG
    if (verbose && !debug)
	version();
#endif

    if (*filelist != NULL) {
	for (fileptr = filelist; *fileptr; fileptr++) {
	    if (**fileptr == '+' && do_melt == 0) {
		tune_table(*fileptr + 1);
	/* If a file type is given, but no file names */
		if (filelist[1] == NULL)
			goto Pipe;
		continue;
	    }
	    exit_stat = 0;
	    if (do_melt != 0) {		       /* MELTING */

#ifdef MSDOS
		/* Check for .F or XF suffix; add one if necessary */
		cp = *fileptr + strlen(*fileptr) - 2;
		if ((*cp != '.' && *cp != 'X' && *cp != 'x') ||
		    (*(++cp) != 'F' && *cp != 'f')) {
		    if (strlen(*fileptr) > PATH_MAX) {
			fprintf(stderr, "%s: pathname too long\n", *fileptr);
			continue;
		    }
		    strcpy(tempname, *fileptr);
		    if ((cp=freeze_rindex(tempname,'.')) == NULL)
			strcat(tempname, ".F");
		    else if(*(++cp) == '\0') strcat(tempname, "F");
		    else {
			*(++cp) = '\0';
			strcat(tempname, "XF");
		    }
		    *fileptr = tempname;
		}
#else
		/* Check for .F suffix */
		if (strcmp(*fileptr + strlen(*fileptr) - 2, ".F") != 0) {
		    /* No .F: tack one on */
		    if (strlen(*fileptr) > PATH_MAX) {
			fprintf(stderr, "%s: pathname too long\n", *fileptr);
			continue;
		    }
		    strcpy(tempname, *fileptr);
		    strcat(tempname, ".F");
		    *fileptr = tempname;
		}
#endif /*MSDOS */

		/* Open input file for melting */

#ifdef MSDOS
		if ((freopen(*fileptr, "rb", stdin)) == NULL)
#else
		if ((freopen(*fileptr, "r", stdin)) == NULL)
#endif
		{
			perror(*fileptr); continue;
		}
		/* Check the magic number; note the lower bit
		   of the second byte indicates the BIG version
		   was used.
		*/
		if (getchar() != magic_header[0]
#ifdef COMPAT
		|| (((new_flg = getchar()) & 0xFE) != magic_header[1]))
#else
		|| getchar() != magic_header[1])
#endif
		{
		    fprintf(stderr, "%s: not in frozen format\n",
			*fileptr);
		continue;
		}
#ifdef COMPAT
		new_flg &= 1;
#endif
		/* Generate output filename */
		if (strlen(*fileptr) > PATH_MAX) {
			fprintf(stderr, "%s: pathname too long\n", *fileptr);
			continue;
		}
		strcpy(ofname, *fileptr);
		ofname[strlen(*fileptr) - 2] = '\0';  /* Strip off .F */
	    } else {

			/* FREEZING */
#ifdef COMPAT
		new_flg = 1;
#endif

#ifdef MSDOS
		cp = *fileptr + strlen(*fileptr) - 2;
		if ((*cp == '.' || *cp == 'X' || *cp == 'x') &&
		    (*(++cp) == 'F' || *cp == 'f')) {
		    fprintf(stderr,"%s: already has %s suffix -- no change\n",
			*fileptr,--cp); /* } */
#else
		if (strcmp(*fileptr + strlen(*fileptr) - 2, ".F") == 0) {
		    fprintf(stderr, "%s: already has .F suffix -- no change\n",
			*fileptr);
#endif /* MSDOS */

		    continue;
		}
		/* Open input file for freezing */

#ifdef MSDOS
		if ((freopen(*fileptr, image == O_TEXT ? "rt" : "rb", stdin))
		    == NULL)
#else
		if ((freopen(*fileptr, "r", stdin)) == NULL)
#endif
		{
		    perror(*fileptr); continue;
		}
		stat ( *fileptr, &statbuf );

		if (strlen(*fileptr) > PATH_MAX) {
			fprintf(stderr, "%s: pathname too long\n", *fileptr);
			continue;
		}
		/* Generate output filename */
		strcpy(ofname, *fileptr);
#ifndef BSD4_2		/* Short filenames */
		if ((cp = last_sep(ofname)) != NULL) cp++;
		else					cp = ofname;
# ifdef MSDOS
		if (topipe == 0 && (sufp = freeze_rindex(cp, '.')) != NULL &&
		    strlen(sufp) > 2) fprintf(stderr,
		    "%s: part of filename extension will be replaced by XF\n",
		    cp);
# else
		if (strlen(cp) > UTIL_NAME_MAX) {
		    fprintf(stderr,"%s: filename too long to tack on .F\n",cp);
		    continue;
		}
# endif
#endif	/* BSD4_2		Long filenames allowed */
							 
#ifdef MSDOS
		if ((cp = rindex(ofname, '.')) == NULL) strcat(ofname, ".F");
		else {
		   if(*(++cp) != '\0') *(++cp) = '\0';
		   strcat(ofname, "XF");
		}
#else
		strcat(ofname, ".F");
#endif /* MSDOS */

	    }
	    precious = 0;
	    /* Check for overwrite of existing file */
	    if (overwrite == 0 && topipe == 0) {
		if (stat(ofname, &statbuf) == 0) {
		    char response[2];
		    response[0] = 'n';
		    fprintf(stderr, "%s already exists;", ofname);
#ifndef MSDOS
		    if (foreground()) {
#endif
			fprintf(stderr,
			    " do you wish to overwrite %s (y or n)? ", ofname);
			fflush(stderr);
			read(2, response, 2);
			while (response[1] != '\n') {
			    if (read(2, response+1, 1) < 0) {	/* Ack! */
				perror("stderr"); break;
			    }
			}
#ifndef MSDOS
		    }
#endif
		    if (response[0] != 'y') {
			fprintf(stderr, "\tnot overwritten\n");
			continue;
		    }
		}
	    }
	    if(topipe == 0) {  /* Open output file */

#ifdef DEBUG
		if (do_melt == 0 || debug == 0) {
#endif
#ifdef MSDOS
		if (freopen(ofname, do_melt && image == O_TEXT ? "wt" : "wb",
		    stdout) == NULL)
#else		 
		if (freopen(ofname, "w", stdout) == NULL)
#endif
		{
		    perror(ofname); continue;
		}
#ifdef DEBUG
		}
#endif
		if(!quiet) {
			fprintf(stderr, "%s:", *fileptr);
			indicator_threshold = 2048;
			indicator_count = 1024;
		}
	    }
	    else {	/* output is to stdout */
#ifdef MSDOS
			/* freeze output always binary; melt output
			   is binary if image == O_BINARY
			*/
		if (do_melt == 0 || image == O_BINARY)
			setmode(fileno(stdout), O_BINARY);
#endif
	    }
	    /* Actually do the freezing/melting */
	    if (do_melt == 0)
		freeze();
#ifndef DEBUG
	    else			melt();
#else
	    else if (debug == 0)	melt();
	    else			printcodes();
#endif /* DEBUG */
	    if(topipe == 0) {
		copystat(*fileptr, ofname);	/* Copy stats */
		if((exit_stat == 1) || (!quiet))
			putc('\n', stderr);
	    }
	 }
    } else {		/* Standard input */
Pipe:
	topipe = 1;
	if (do_melt == 0) {
#ifdef MSDOS
			/* freeze output always binary */
			/* freeze input controlled by -i -t -b switches */
		setmode(fileno(stdout), O_BINARY);
		setmode(fileno(stdin), image);
#endif
#ifdef COMPAT
		new_flg = 1;
#endif
		freeze();
		if(!quiet)
			putc('\n', stderr);
	} else {
#ifdef MSDOS
		    /* melt input always binary */
		    /* melt output to stdout binary if so requested */
	    setmode(fileno(stdin), O_BINARY);
	    setmode(fileno(stdout), image);
#endif
	    /* Check the magic number */
	    if ((getchar() != magic_header[0])
#ifdef COMPAT
	     || (((new_flg = getchar()) & 0xFE) !=magic_header[1]))
#else
	     || (getchar() != magic_header[1]))
#endif
	    {
		fprintf(stderr, "stdin: not in frozen format\n");
		exit(1);
	    }
#ifdef COMPAT
	    new_flg &= 1;
#endif

#ifndef DEBUG
	    melt();
#else
	    if (debug == 0)     melt();
	    else		printcodes();
#endif /* DEBUG */
	}
    }
    return exit_stat;
}

long in_count = 1;                  /* length of input */
long bytes_out;                  /* length of frozen output */

void prratio(stream, num, den)
FILE *stream;
long num, den;
{
	register long q;        /* This works everywhere */

	if (!den) den++;

	if(num > 214748L) {     /* 2147483647/10000 */
		q = num / (den / 10000L);
	} else {
		q = 10000L * num / den; /* Long calculations, though */
	}
	if (q < 0) {
		putc('-', stream);
		q = -q;
	}
	fprintf(stream, "%d.%02d%%", (int)(q / 100), (int)(q % 100));
#ifdef GATHER_STAT
	fprintf(stream, "(%ld / %ld)", num, den);
#endif
}


char *
freeze_rindex(s, c)		/* For those who don't have it in libc.a */
register char *s, c;
{
	char *p;
	for (p = NULL; *s; s++)
	    if (*s == c)
		p = s;
	return(p);
}

void writeerr()
{
    if (!topipe) {
	perror ( ofname );
	unlink ( ofname );
    }
    exit ( 1 );
}

void copystat(ifname, ofname)
char *ifname, *ofname;
{
#ifdef __TURBOC__
struct ftime utimbuf;
#endif
    struct stat statbuf;
    int mode;
#ifndef __TURBOC__
#ifndef _UTIME_H_INCLUDED
    time_t timep[2];
#else
	struct utimbuf timep;
#endif
#endif

#ifdef MSDOS
    if (_osmajor < 3) freopen("CON","at",stdout); else	  /* MS-DOS 2.xx bug */
#endif

    fflush(stdout);
    if (stat(ifname, &statbuf)) {		/* Get stat on input file */
	perror(ifname);
	return;
    }

#ifndef MSDOS
    if ((statbuf.st_mode & S_IFMT) != S_IFREG) {
	if(quiet)
		fprintf(stderr, "%s: ", ifname);
	fprintf(stderr, " not a regular file: unchanged");
	exit_stat = 1;
    } else if (statbuf.st_nlink > 1) {
	if(quiet)
		fprintf(stderr, "%s: ", ifname);
	fprintf(stderr, " has %d other links: unchanged",
		statbuf.st_nlink - 1);
	exit_stat = 1;
    } else if (exit_stat == 2 && (!force)) { /* No freezing: remove file.F */
#else
    if (exit_stat == 2 && (!force)) { /* No freezing: remove file.F */
#endif /* MSDOS */

	if(!quiet)
		fprintf(stderr, "-- file unchanged");
    } else {		    /* ***** Successful Freezing ***** */
	exit_stat = 0;
	mode = statbuf.st_mode & 07777;
	if (chmod(ofname, mode))		/* Copy modes */
	    perror(ofname);

#ifndef MSDOS
	chown(ofname, statbuf.st_uid, statbuf.st_gid);	/* Copy ownership */
#endif

#ifdef __TURBOC__
        getftime(fileno(stdin),&utimbuf);
        freopen(ofname,"rb",stdout);
        setftime(fileno(stdout),&utimbuf);
        fclose(stdout);
#else
#ifdef _UTIME_H_INCLUDED
	timep.actime = statbuf.st_atime;
	timep.modtime= statbuf.st_mtime;
#else 
	timep[0] = statbuf.st_atime;
	timep[1] = statbuf.st_mtime;
#endif
	utime(ofname, &timep);	/* Update last accessed and modified times */
#endif
	precious = 1;
	if (unlink(ifname))	/* Remove input file */
	    perror(ifname);
	if(!quiet)
		fprintf(stderr, " -- replaced with %s", ofname);
	return;		/* Successful return */
    }

    /* Unsuccessful return -- one of the tests failed */
    if (unlink(ofname))
	perror(ofname);
}

#ifndef MSDOS
/*
 * This routine returns 1 if we are running in the foreground and stderr
 * is a tty.
 */
int foreground()
{
	if(bgnd_flag != SIG_DFL)  /* background? */
		return(0);
	else {                          /* foreground */
		if(isatty(2)) {		/* and stderr is a tty */
			return(1);
		} else {
			return(0);
		}
	}
}
#endif

#if defined(__TURBOC__) || !defined(INT_SIG)
void
#endif
onintr (int n) {
    if (!precious) {
	fclose(stdout);
	unlink ( ofname );
    }
    exit ( 1 );
}

#if defined(__TURBOC__) || !defined(INT_SIG)
void
#endif
oops (int n)        /* file is corrupt or internal error */
{
    fflush(stdout);
    fprintf(stderr, "Segmentation violation occured...\n");
    exit ( 1 );
}

void version()
{
	fprintf(stderr, ident, PATCHLEVEL);
	fprintf(stderr, "Options: ");
#ifdef DEBUG
	fprintf(stderr, "DEBUG, ");
#endif
#ifdef BSD4_2
	fprintf(stderr, "BSD4_2, ");
#endif
#ifdef  __XENIX__
	fprintf(stderr, "XENIX, ");
#endif
#ifdef  __TURBOC__
	fprintf(stderr, "TURBO, ");
#endif
#ifdef GATHER_STAT
	fprintf(stderr, "GATHER_STAT, ");
#endif
#ifdef COMPAT
	fprintf(stderr, "compatible with vers. 1.0, ");
#endif
	fprintf(stderr, "\nLZSS: %d (table), %d (match length);\n", _NN, _F);
	fprintf(stderr, "Static Huffman table: %d %d %d %d %d %d %d %d\n",
		Table[1], Table[2], Table[3], Table[4],
		Table[5], Table[6], Table[7], Table[8]);
	fprintf(stderr, "HUFFMAN: %ld (max frequency)\n", (long)MAX_FREQ);
	fprintf(stderr, "HASH: %d bits\n", BITS);
#ifdef MSDOS
	fprintf(stderr, "Default melted mode: %s\n",
			image == O_BINARY ? "binary" : "text");
#endif
	exit(0);
}

void tune_table(type) char *type;
{
	extern char * defread();
	register char *s = defread(type), *t;
	static int v[8];
	int i, is_list = 0;
	if(!s) {
	/* try to consider 'type' as a list of values: n1,n2, ... */
		if(rindex(type, ','))
			is_list = 1;
		else {
			if (!quiet)
				fprintf(stderr, "\"%s\" - no such file type\n", type);
			exit(1);
		}
		if(sscanf(type, "%d,%d,%d,%d,%d,%d,%d,%d",
			v, v+1, v+2, v+3, v+4, v+5, v+6, v+7) != 8) {
			if (!quiet)
				fprintf(stderr,
					"%s - a list of 8 numbers expected\n", type);
			exit(1);
		}
	}
	if(!is_list && (!(t = rindex(s, '=')) ||
		sscanf(++t, "%d %d %d %d %d %d %d %d",
		v, v+1, v+2, v+3, v+4, v+5, v+6, v+7) != 8)) {
		if(!quiet)
			fprintf(stderr,
				"\"%s\" - invalid entry\n", type);
		exit(1);
	}
	for(i = 0; i < 8; i++)
		Table[i+1] = v[i];
	if(!quiet) {
		if(!is_list) {
			t = s;
			while(*s != '=' && *s != ' ' && *s != '\t') s++;
			*s = '\0';
		} else
			t = "";
		fprintf(stderr, "Using \"%s%s\" type\n", type, t);
	}
}

#ifdef MSDOS
	/* MSDOS typically has ':' and '\\' separators, but some command
          processors (and the int 21h function handler) support '/' too.
          Find the last of these.
	*/
char *
last_sep(s)
register char *s;
{
	register char *c;

	char *p;
	for (p = NULL; *s; s++)
	    if (*s == '/' || *s == '\\' || *s == ':')
		p = s;
	return(p);
}

#endif	/* MSDOS */
