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





static char rcsid[] = "$Id: grep.c 206593 2008-11-17 19:31:07Z CHarris@qnx.com $";
/*-
grep, egrep, fgrep - find patterns in files.



This program behaves slightly differently depending upon it's name being
grep, egrep, or fgrep -- standard, extended and fixed expressions.
Egrep is equivalent to grep -E
Fgrep is equivalent to grep -F

The POSIX regcomp,regexec library routines are used for grep,egrep, and
another set (with similiar API) for fgrep.
Grep and egrep are packaged to minimise the amount of memory required,
at the cost of execution speed.
Fgrep is optimised for speed.


$Log$
Revision 4.31  2005/06/03 01:37:47  adanko
Replace existing QNX copyright licence headers with macros as specified by
the QNX Coding Standard. This is a change to source files in the head branch
only.

Note: only comments were changed.

PR25328

Revision 4.30  2003/08/25 19:41:00  martin
Add QSSL Copyright.

Revision 4.29  2000/03/20 16:40:08  jbaker
cleaned up some warnings on all platforms, also seems to have fixed pr 1972.
Re: PR 1972, when argv[1] is larger than 15 chars it segv's

Revision 4.28  1999/08/30 19:19:49  mshane
Modified program usage message

changed
-q      Produce no ouput
to
-q      Produce no output

Revision 4.27  1998/11/04 20:18:07  aggie
*** empty log message ***

Revision 4.26  1998/09/15 18:23:33  eric
cvs

Revision 4.25  1997/11/11 20:03:15  eric
now realloc's space for multiple expressions in larger chunks;
also will error out gracefully for fixed regular expressions if
fregadd() fails (formerly would SIGSEGV).

Revision 4.24  1997/04/14 22:10:10  eric
grep exit status was wrong when more than one file was processed..
would exit 1 if any one of the files didn'
t have a match, instead of 1 only if ALL the files didn't have
a match

Revision 4.23  1996/11/12 19:03:02  steve
revision history comment change

 * Revision 4.22  1993/12/02  16:04:47  steve
 * cleaned extraneous rcsid
 *
 * Revision 4.21  1993/12/02  16:02:18  steve
 * explicitly flush lines to allow parallel invocations.
 * changed line counter to long to avoid overflow in 16bit
 *
 * Revision 1.5  1993/07/29  18:38:45  steve
 * Two bugs -- one in the regcomp routine which broke alternations.
 * Also, grep didn't notice the "regexec()" was failing with a "corrupt
 * regular expression (impossible)" error -- it now does and reports
 * accordingly.
 *
 * Revision 1.4  1993/07/26  23:29:03  steve
 * Some small changes to support improved interface to new (faster) fregex
 * routines.
 * Added flag to help debug grep when there are problems.
 *


*/

#ifdef __USAGE		/* grep.c */
%-grep
%C	[-E|-F] [-chilnqrRsvx] [-e pattern | -f pattern_file]... [file ...]
%C	[-E|-F] [-chilnqrRsvx] pattern [file ...]
Where:
 -E            Use Extended Regular Expressions.
 -F            Use Fixed Strings.
 -c            Only print count of selected lines.
 -i            Ignore upper-case/lower-case distinctions.
 -h            Do not display filenames.
 -l            Print pathname once.
 -n            Precede each output line with the line number.
 -q            Produce no output.
 -r,-R         Recursive
 -s            Suppress error messages on file open errors.
 -v            Select lines not matching specified patterns.
 -x            Match pattern(s) exactly.
 -e pattern    Specify a search pattern.
 -f pat_file   Specify a file of search patterns.
%-egrep
%C	[-cilnrRsvx] [-e pattern | -f pattern_file]... [file ...]
%C	[-cilnrRsvx] pattern [file ...]
Where:
 -c            Only print count of selected lines.
 -i            Ignore upper-case/lower-case distinctions.
 -h            Do not display filenames.
 -l            Print pathname once.
 -n            Precede each output line with the line number.
 -q            Produce no output.
 -r,-R         Recursive
 -s            Suppress error messages on file open errors.
 -v            Select lines not matching specified patterns.
 -x            Match pattern(s) exactly.
 -e pattern    Specify a search pattern.
 -f pat_file   Specify a file of search patterns.
%-fgrep
%C	[-cilnrRsvx] [-e pattern | -f pattern_file]... [file ...]
%C	[-cilnrRsvx] pattern [file ...]
Where:
 -c            Only print count of selected lines.
 -i            Ignore upper-case/lower-case distinctions.
 -l            Print pathname once.
 -n            Precede each output line with the line number.
 -q            Produce no output.
 -r,-R         Recursive
 -s            Suppress error messages on file open errors.
 -v            Select lines not matching specified patterns.
 -x            Match pattern(s) exactly.
 -e pattern    Specify a search pattern.
 -f pat_file   Specify a file of search patterns.
#endif
#include <lib/compat.h>

#include <regex.h>

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <dirent.h>
#ifdef __QNXNTO__
#include <libgen.h>
#endif

#include "fregex.h"


char	*_pname = "grep";
#define	TXT(s)	(s)
#define	T_IMPOSSIBLE	"impossible error %s\n"
#define	T_BAD_PATTERN	"%s in '%s'\n"
#define	T_NO_PATTERN	"must specify a pattern \n"
#define	T_BAD_INIT		"unable to init\n"
#define	T_NO_MEMORY		"no memory"

/* When allocating space for multiple REs, increase space allocated by
   this number of REs each time (for -e, -f options etc) */
#define REALLOC_CHUNK_SIZE (512L)

int	_redebug = 0;	/* debug regular expression library. */

int		 count_lines,	/*	count only */
		 list_name,		/*	print pathname just once */
		 nonames,		/*	don't print pathnames */
		 print_lineno,	/*	print line numbers */
		 negate_match,	/*	print lines not matching pattern */
		 no_output,		/*	no output */
		 no_file_error,	/* don't print messages about missing files */
		 recursive,	/* recursive */
		 exact;		/* make patterns exact */

int		 nfile;

char     lbuf[LINE_MAX];


enum	{
	FGREP,
	EGREP,
	GREP
} pattype;

/* ERROR bit in exit status */
#define EXIT_ERROR (2)    
/* NO_MATCHES bit in exit status */
#define EXIT_NO_MATCHES (1)

int		num_pats = 0;
fregex_t	fexpr;

char	**fstrs;
regex_t		*restack;

int		retop;
#ifndef REG_NOOPT
#define REG_NOOPT 0
#endif
int	comp_flags = REG_NOSUB|REG_NOOPT;	


char *mkexact(char *s)
{
int l;
char *t = malloc((l=strlen(s))+3);
char *x = t;
	if (!t) {
		fprintf(stderr,"%s (mkexact)\n",TXT(T_NO_MEMORY));
		exit(1);
	}
	if (*s != '^') {
		*x++ = '^';
	}
	strcpy(x,s);
	x += l;
	if (s[l-1] != '$') {
		*x++ = '$';
		*x = '\0';
	}
	return t;
}

int
add_pattern(s)
char	*s;
{
int	ecode;
	if (s == NULL) {
		if (num_pats == 0) {
			fprintf(stderr,TXT(T_NO_PATTERN));
			return 1;
		}
		switch (pattype) {
		case	FGREP:
			if (fregcomp(&fexpr, fstrs, comp_flags | (exact ? FREG_EXACT : 0))) {
				fprintf(stderr,"%s (fregcomp)\n",TXT(T_NO_MEMORY));
				return 1;
			}
			break;
		case	EGREP:
		case	GREP:
			restack = realloc(restack,sizeof(*restack)*num_pats);
			retop = num_pats;
			break;
		}

		return 0;
	}
	switch (pattype) {
	case	FGREP:
		if (num_pats+1 >= retop) { 
			fstrs=realloc(fstrs,sizeof *fstrs * (retop+=REALLOC_CHUNK_SIZE));

			if (fstrs==NULL) {
				fprintf(stderr,"%s (realloc)\n",TXT(T_NO_MEMORY));
				exit(EXIT_FAILURE);
			}
		}
		if ((fstrs[num_pats++] = strdup(s)) == 0) {
			fprintf(stderr,"%s (strdup)\n",TXT(T_NO_MEMORY));
			exit(EXIT_FAILURE);
		}
		fstrs[num_pats] = 0;
		break;


	case	EGREP:
	case	GREP:
		if (exact) {
			s = mkexact(s);
		}
		if (num_pats == retop) {
			if (retop > 0) {
				restack=realloc(restack,sizeof(*restack)*(retop+=REALLOC_CHUNK_SIZE));
			} else {
				restack=calloc(sizeof(*restack),(retop=16));
			}
			if (restack == NULL) {
				fprintf(stderr,"%s (realloc|calloc)\n",TXT(T_NO_MEMORY));
				exit(EXIT_FAILURE);
			}
		}
		if ((ecode=regcomp(restack+num_pats,s,comp_flags)) != REG_OK) {
			regerror(ecode,restack+num_pats,lbuf,sizeof lbuf);
			fprintf(stderr,TXT(T_BAD_PATTERN),lbuf,s);
			exit(EXIT_FAILURE);
		}
		num_pats++;
		break;
	default:
		fprintf(stderr,TXT(T_IMPOSSIBLE),"addpattern()");
	}
	return 1;
}

#define	OPT_LIST0	"EFcihlnqsvxe:f:**DRr"
#ifdef __QNXNTO__
#include <libgen.h>
#endif
#define	OPT_LIST1	"cihlnqsvxe:f:**DRr"

int
get_args(int argc, char **argv)
{
FILE            *fp;
int				 opt;
char	*umsg = OPT_LIST0;

	_pname = basename(argv[0]);

	if (strcmp(_pname,"fgrep") == 0) {
		pattype = FGREP;
		umsg = OPT_LIST1;
	} else if (strcmp(_pname,"egrep") == 0) {
		pattype = EGREP;
		comp_flags |= REG_EXTENDED;
		umsg = OPT_LIST1;
	} else {
		pattype = GREP;
	}

	nfile = argc - 1;
	while((opt = getopt(argc, argv, umsg)) != -1) {
		switch(opt) {
		case 'c':
			count_lines = 1;
			no_output=1;
			break;

		case 'e':
			add_pattern(optarg);
			break;

		case 'f':
			if (strcmp(optarg,"-")) {
				if ((fp=fopen(optarg,"r")) == NULL) {			
					fprintf(stderr,"%s: cannot open '%s' (%s)\n",
						argv[0], optarg, strerror(errno));
					exit(EXIT_FAILURE);
				}
			} else {
				fp = stdin;
			}
			while (fgets(lbuf,sizeof lbuf,fp)) {
				int	t = strlen(lbuf);
				while (t && lbuf[t-1] == '\n') {
					lbuf[--t] = '\0';
				}
				if (t) {
					add_pattern(lbuf);
				}
			}
			if (fp != stdin) {
				fclose(fp);
			}
			break;

		case 'h':
			nonames = 1;
			break;			

		case 'i':
			comp_flags |= REG_ICASE;
			break;
				
		case 'l':
			list_name = 1;
			break;

		case 'n':
			print_lineno = 1;
			break;
			
		case 's':
			no_file_error = 1;
			break;
			
		case 'q':
			no_output = 2;
			break;

		case 'v':
			negate_match = 1;
			break;

		case 'E':
			comp_flags |= REG_EXTENDED;
			pattype = EGREP;
			break;

		case 'F':		/*	fgrep */
			pattype = FGREP;
			break;

		case 'x':
			exact = 1;
			break;
		case	'D':
			_redebug = 1;
			break;
		case	'r':
		case	'R':
			recursive = 1;
			break;
		default:
			exit(EXIT_FAILURE);
		}

	}
	return optind;
}

int grep(FILE*, char*);


static int
recurse(const char *name, const char *path)
{
	int	ecode = EXIT_NO_MATCHES;
	DIR *pdir;
	struct dirent *pent;
	pdir = opendir(path);
	//fprintf(stderr, "recurse(%s, %s)\n", name, path);
	if (pdir == NULL){
		FILE *fp;
		if((fp = fopen(path, "r")) == NULL) {
			if (!no_file_error) {
				fprintf(stderr,"%s: cannot open file '%s' (%s)\n",
					name,path,strerror(errno));
			}
			ecode |= EXIT_ERROR;
		} else {
			if (!grep(fp,path)) ecode&=~EXIT_NO_MATCHES; 
			fclose(fp);
		}
		
	}else{
 		while (pent = readdir(pdir)){
			char *pathBuffer = NULL;
			int len = strlen(pent->d_name);
			/* Avoid acending and spinning in the current directory */
			if (len == 0)
				continue;
			if (len == 1 && pent->d_name[0] == '.')
				continue;
			if (len == 2 && pent->d_name[0] == '.' && pent->d_name[1] == '.')
				continue;
			pathBuffer = malloc(strlen(path) + strlen(pent->d_name) + 2);
			if (path[strlen(path) - 1] == '/'){
				sprintf(pathBuffer, "%s%s", path, pent->d_name);
			}else{
				sprintf(pathBuffer, "%s/%s", path, pent->d_name);
			}
			ecode &= recurse(name, pathBuffer);
			free(pathBuffer);
			
		}
		closedir (pdir);
	}
	return ecode;
}
				
				

int
main(argc, argv)
int		 argc;
char	*argv[];
{
	int	i;

	int	ecode = EXIT_NO_MATCHES; 

	i = get_args(argc,argv);
	if (i != argc && num_pats == 0)
		add_pattern(argv[i++]);

	if (add_pattern(NULL)) {
		fprintf( stderr, "could not add pattern\n" );
		exit(EXIT_FAILURE);
	}

	nfile = argc - i;

	if (nfile == 0)
		exit(grep(stdin,"-"));

	for (; i < argc; i++) {
		if (strcmp(argv[i],"-")) {
			if (recursive){
				ecode &= recurse(argv[0], argv[i]);
			}else{
				FILE *fp;
				if((fp = fopen(argv[i], "r")) == NULL) {
					if (!no_file_error) {
						fprintf(stderr,"%s: cannot open file '%s' (%s)\n",
							argv[0],argv[i],strerror(errno));
					}
					ecode |= EXIT_ERROR;
				} else {
					if (!grep(fp,argv[i])) ecode&=~EXIT_NO_MATCHES; 
					fclose(fp);
				}
			}
		} else {
			if (!grep(stdin,"-")) ecode &=~EXIT_NO_MATCHES;
		}
	}

	return ecode;
}



int fregmtch(line)
char	*line;
{
	return !fregexec(&fexpr, line, 0, NULL, 0);
}

int regmtch(line)
char	*line;
{
	int	i;
	int	r;
	for (i=0; i < num_pats; i++) {
		if ((r=regexec(restack+i, line, 0, 0, 0)) == REG_OK)
			return 1;
		if (r != REG_NOMATCH) {
			static char errbuf[100];
			regerror(r, restack+i, errbuf, sizeof errbuf);
			fprintf(stderr,"%s: error '%s' detected scanning line '%s'\n",
				_pname, errbuf, line);
		}
	}
	return(0);
}

int grep(fp, fn)
FILE			*fp;
char			*fn;
{
	long	lno = 0;
	int	count = 0;	/*	Match count	*/
	int	(*match)(char *)=NULL;
	switch (pattype) {
	case	FGREP:	match = fregmtch;	break;
	case	EGREP:
	case	GREP:	match = regmtch;	break;
	default:
		fprintf(stderr,"impossible type!\n");
	}
	while (1) {
		int	t;
		int	c;
		for (t=0; t < sizeof lbuf-1 && (c=getc(fp)) != EOF && c != '\n'; t++) {
			lbuf[t] = c;
		}
		lbuf[t] = '\0';
		if (c == EOF && t == 0) break;
		if (c == '\n') lno++;
	
		if ((*match)(lbuf) ^ negate_match) {
			/*	line was selected.... */
			if (no_output == 2)
				return 0;
			count++;
			if (no_output)
				continue;				
			if (list_name) {
				printf("%s\n", fn);
				fflush(stdout);
				return 0;
			}
			if ((nfile > 1 || recursive) && nonames == 0){
				printf("%s:",fn);
			}
			if (print_lineno) {
				printf("%ld:",lno);
			}
			fwrite(lbuf,1,t,stdout);
			if (c == '\n')
				putc(c,stdout);
		}
	}
	if (count_lines && no_output < 2) {
		if (nfile > 1)
			printf("%s:",fn);
		printf("%d\n",count);
	}
	return count == 0;
}
