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
%C	[-d | -u] [-c] [-f fields] [-s chars] [input_file [output_file]]
Options:
 -c            Precede each line with an occurrence count
 -d            Suppress writing of non-repeated lines
 -f fields     Ignore first fields for comparison
 -s chars      Ignore first chars for comparison
 -u            Suppress writing of repeated lines
#endif


/*---------------------------------------------------------------------

	$Id: uniq.c 153052 2008-08-13 01:17:50Z coreos $

	$Log$
	Revision 1.4  2005/06/03 01:38:03  adanko
	Replace existing QNX copyright licence headers with macros as specified by
	the QNX Coding Standard. This is a change to source files in the head branch
	only.

	Note: only comments were changed.

	PR25328

	Revision 1.3  2004/04/17 17:00:11  thomasf
	Clean up language.
	
	Revision 1.2  2003/09/02 16:29:45  martin
	Add QSSL Copyright.
	
	Revision 1.1  1998/10/22 03:13:28  dtdodge
	
	Initial cvs checkin
	
	Revision 1.8  1996/09/17 14:47:39  steve
	got rid of silly header files

 * Revision 1.7  1994/12/01  15:36:15  steve
 * check that argv[optind] != 0...
 *
 * Revision 1.6  1994/03/16  15:27:12  steve
 * changed to keep "long" counts
 *
 * Revision 1.5  1992/07/22  22:01:06  brianc
 * Changed repeat count formatting to %5d.  For those who think this
 * violates POSIX please go change ls, df and wc as well.
 *
 * Revision 1.4  1992/05/14  13:31:56  brianc
 * Correct spelling mistake in usage
 *
 * Revision 1.3  1991/11/04  20:14:19  eric
 * changed -c output format from "%$d: %s" to "%d %s" as per 1003.2 draft 11
 *
 * Revision 1.2  1991/10/07  13:33:43  brianc
 * Changed to exit with SUCCESS if input file empty
 *
 * Revision 1.1  1991/09/04  01:30:21  brianc
 * Initial revision
 *
	
---------------------------------------------------------------------*/


/***************************************************** INCLUDES ******/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <ctype.h>

/***************************************************** VARIABLES *****/

int			cflag=0, dflag=0, uflag=0, fval=0, sval=0;

long			rep_count;				/* how many occurences of line1 */
FILE		*fp_in, *fp_out;		/* default is stdin & stdout */
char		*fname_in,*fname_out;	/* null if not specified */

char		buf1[LINE_MAX];			/* will put lines in here */
char		buf2[LINE_MAX];


/***************************************************** PARSE_ARGS ****/

void
parse_args(argc,argv)
int argc;
char *argv[];
{
	int				opt;
	int				errflg = 0;
	char            *p;

	while((opt = getopt(argc,argv,"cdf:s:u0123456789")) != -1) {		

		reprocess:

		switch(opt)	{			
			case 'c':	cflag = 1;					break;
			case 'd':	dflag = 1;					break;
			case 'u':	uflag = 1;					break;

			case 'f':	fval = (int) strtoul(optarg,&p,0); 
						if (*p) {
							fprintf(stderr,"%s: invalid number (-f %s)\n",argv[0],optarg);
							errflg++;
						}
						break;

			case 's':	sval = (int) strtoul(optarg,&p,0); 
						if (*p) {
							fprintf(stderr,"%s: invalid number (-s %s)\n",argv[0],optarg);
							errflg++;
						}
						break;

			default:	if (isdigit(opt)) {
							/* support obsolete usage - fake out a -f number */
                        	optarg = argv[optind-1]+1;
                        	opt = 'f';
							goto reprocess;
						}
						errflg++;
						break;
							
		}
	}

	if (argv[optind] && argv[optind][0]=='+') {
		/* support obsolete usage - fake out a -s number */
		optarg = argv[optind]+1;
		opt    = 's';
		optind++;
		goto reprocess;
	}

	if ((dflag+uflag)>1) {
		errflg++;
		fprintf(stderr,"%s: only one of -c, -d and -u may be specified.\n",argv[0]);
	}

	if (errflg)	exit(EXIT_FAILURE);

	#ifdef DEBUG
		fprintf(stderr,"parse_args:\n");
		fprintf(stderr,"\t cflag = %d\n",cflag);
		fprintf(stderr,"\t dflag = %d\n",dflag);
		fprintf(stderr,"\t fval  = %d\n",fval);
		fprintf(stderr,"\t sval  = %d\n",sval);
		fprintf(stderr,"\t uflag = %d\n",uflag);
		fprintf(stderr,"\t argc = %d, optind = %d\n",argc,optind);
	#endif

	/* default is stdin & stdout, loblaw's no-name files! */
	fp_in	= stdin;		fname_in	= "(stdin)";
	fp_out	= stdout;		fname_out	= "(stdout)";

	/* did user spec in & out files? */
	if (optind < argc) {		/* user specd at least an input_file */
		if (argv[optind][0]!='-' || argv[optind][1]) {
			fname_in = argv[optind];
			if ((fp_in = fopen(fname_in,"r")) == 0) {			
				fprintf(stderr,"uniq: unable to open input_file: %s\n",fname_in);
				exit(EXIT_FAILURE);
			}
		}

		if (++optind < argc) {	/* user specd an output_file */
			if (argv[optind][0]!='-' || argv[optind][1]) {
				fname_out = argv[optind];
				if (strcmp(fname_in,fname_out) == 0) {				
					fprintf(stderr,
						"uniq: input_file & output_file must be different\n");
					exit(EXIT_FAILURE);
				}

				if ((fp_out = fopen(fname_out,"w")) == 0) {				
					fprintf(stderr,
						"uniq: unable to open output_file: %s\n",fname_out);
					exit(EXIT_FAILURE);
				}
			}

			/* make sure no garbage at end */
			if (++optind != argc)	{				
				fprintf(stderr,	"uniq: bad argument: %s\n",argv[optind]);
				exit(EXIT_FAILURE);
			}
		}
	}
} /* parse_args */


/***************************************************** SKIP_BLANK ****/

char *skip_blank(char *p_line)
{
	/* skip white space XXXX should be set by LC_CTYPE */
	while (*p_line==' '||*p_line=='\t')	p_line++;
	if (!*p_line) return NULL;
	return p_line;
}


/***************************************************** SKIP_NONBLANK */

char *skip_nonblank(char *p_line)
{
	/* skip non-white space XXXX should be set by LC_CTYPE */
	while(*p_line&&(*p_line!=' ')&&(*p_line!='\t')) p_line++;
	if (!*p_line) return NULL;
	return p_line;
}


/***************************************************** SKIP_CHARS ****/

char *skip_chars(char *p_line)
{
	int		I;

	#ifdef DEBUG
		fprintf(stderr,"skip_chars(): sval = %d\n",sval);
	#endif

	/* any chars to skip? */
	if (sval == 0) return p_line;

	/* try to skip past chars, exit out if hit null */
	for(I=0;I<sval;I++,p_line++) {
		#ifdef DEBUG
			fprintf(stderr,"skip_chars(): ptr is now %04X\n",p_line);
		#endif
		if (!*p_line) return NULL;
	}

	return p_line;

} /* skip_chars */


/***************************************************** SKIP_FIELDS ***/

char *skip_fields(char *p_line)
{
	int		I;

	/* any fields to skip? */
	if (fval == 0) return p_line;

	/* skip over white space at start of line */
	if ((p_line = skip_blank(p_line)) == NULL) return p_line;

	/* for the number of fields specd: */
	for(I=0;I<fval;++I) {
		/* skip past non-blank field */
		if ((p_line = skip_nonblank(p_line)) == NULL) break;

		/* skip past white space at end of field */
		if ((p_line = skip_blank(p_line)) == NULL) break;
	}

	return p_line;
} /* skip_fields */


/****************************************************** LINES_EQUAL **/

int lines_equal(char *p_line1, char *p_line2)
{
	char	*line1;
	char	*line2;

	/* load di, si */
	line1 = p_line1;
	line2 = p_line2;

	/* if fields val was specified, try to skip past that many fields */
	line1 = skip_fields(line1);
	line2 = skip_fields(line2);

	#ifdef DEBUG
		fprintf(stderr,
		"lines_equal(): after skip_fields:\n\tline1 = %04X, p_line1 = %04X, line2 = %04X, p_line2 = %04X\n",
		line1,p_line1,line2,p_line2);
	#endif

	if ((line1 == 0) && (line2 == 0)) {
		/* both lines are too short for specd num fields => match */
		#ifdef DEBUG
			fprintf(stderr,"lines_equal(): match 1\n");
		#endif
		return(1);
	}

	/* if only one line is too short, no match */
	if ((line1 == 0) || (line2 == 0)) {
		#ifdef DEBUG
			fprintf(stderr,"lines_equal(): no match 2\n");
		#endif
		return(0);
	}

	/* if "s" (chars) val was specified, try to skip past that many chars */
	line1 = skip_chars(line1);
	line2 = skip_chars(line2);

	#ifdef DEBUG
		fprintf(stderr,
		"lines_equal(): after skip_chars:\n\tline1 = %04X, p_line1 = %04X, line2 = %04X, p_line2 = %04X\n",
		line1,p_line1,line2,p_line2);
	#endif

	if ((line1 == 0) && (line2 == 0)) {
		#ifdef DEBUG
			fprintf(stderr,"lines_equal(): match 3\n");
		#endif
		/* both lines are too short for specd num chars => match */
		return(1);
	}

	if ((line1 == 0) || (line2 == 0)) {
		#ifdef DEBUG
			fprintf(stderr,"lines_equal(): no match 4\n");
		#endif
		/* if only one line is too short, no match */
		return(0);
	}

	if ((line1 - p_line1) != (line2 - p_line2)) {
		#ifdef DEBUG
			fprintf(stderr,"lines_equal(): no match 5\n");
		#endif
		/* the offsets must be the same within the two lines */
		return(0);
	}

	if (strcmp(line1,line2) == 0) {
		#ifdef DEBUG
			fprintf(stderr,"lines_equal(): match 6\n");
		#endif
		return(1);
	} else {
		#ifdef DEBUG
			fprintf(stderr,"lines_equal(): no match 7\n");
		#endif
		return(0);
	}
} /* lines_equal */


/****************************************************** PRINT_LINE ***/

void
print_line(char *line1)
{
	/* dflag means we don't print lines that *aren't* repeated */		
	if (dflag && (rep_count == 1)) return;

	/* uflag means we don't print lines that *are* repeated */		
	if (uflag && (rep_count > 1)) return;

	if (cflag)	fprintf(fp_out,"%4ld %s",rep_count,line1);
	else		fprintf(fp_out,"%s",line1);
} /* print_line */


/********************************************************** MAIN *****/

int
main(int argc,char **argv)
{
	char			*linet;		/* temp ptr */
	char			*line1;
	char			*line2;

	/*
	 *	this routine uses getopt() to parse the cmd line args
	 *	and set flags and values as specd by 1003.2 and sets
	 *	the file pointers fp_in & fp_out.
	 */
	parse_args(argc,argv);

	/* set register pointers to our two buffers */
	line1 = &buf1[0];
	line2 = &buf2[0];

	/* read the first line of the file into buf1 to start off */
	if (fgets(line1,LINE_MAX,fp_in) == 0)
		exit(EXIT_SUCCESS);

	rep_count = 1;

	/*
	 *	we will swap line1 & line2 as required such that:
	 *		line1 will always point to the previous line read in, and 
	 *		line2 will always point to the line just read from the file
	 */
	while (fgets(line2,LINE_MAX,fp_in)) {
		if (lines_equal(line1,line2)) {
			/* the lines are identical */
			rep_count++;
			continue;
		} else {
			/* the lines are now different, so print out the first line */
			print_line(line1);

			/* want line2 to be line1 now, so swap thru temp */
			linet = line1;
			line1 = line2;
			line2 = linet;

			/* reset counter */
			rep_count = 1;
		}
	}

	/* have to flush line1 */
	print_line(line1);
	
	return(EXIT_SUCCESS);

} /* main */

