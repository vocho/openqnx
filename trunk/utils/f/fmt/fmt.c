/*
 * Copyright (c) 1980, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>
#include <unistd.h>

/*
 * fmt -- format the concatenation of input files or standard input
 * onto standard output.  Mail support stripped.
 *
 * Syntax : fmt [ goal [ max ] ] [ name ... ]
 * Authors: Kurt Shoens (UCB) 12/7/78;
 *          Liz Allen (UMCP) 2/24/83 [Addition of goal length concept].
 *          Argument parsing redone by adrianj.
 */

/* LIZ@UOM 6/18/85 -- Don't need LENGTH any more.
 * #define	LENGTH	72		Max line length in output
 */
#define	NOSTR	((char *) 0)	/* Null string pointer for lint */

/* LIZ@UOM 6/18/85 --New variables goal_length and max_length */
#define GOAL_LENGTH 65
#define MAX_LENGTH 75
int	goal_length;		/* Target or goal line length in output */
int	max_length;		/* Max line length in output */
int	pfx;			/* Current leading blank count */
int	lineno;			/* Current input line */
int	mark;			/* Last place we saw a head line */

#ifndef __P
#define __P(x) x 
#endif
void	fmt __P((FILE *));
void	leadin __P((void));
void	oflush __P((void));
void	pack __P((char *, int));
void	prefix __P((char *));
char   *savestr __P((char *));
void	setout __P((void));
void	split __P((char *));
void	tabulate __P((char *));
char   *basename(char *);
/*
 * Drive the whole formatter by managing input files.  Also,
 * cause initialization of the output stuff and flush it out
 * at the end.
 */

int
main(argc, argv)
	int argc;
	char **argv;
{
	FILE *fi;
	int errs = 0;
	int op;

	goal_length = GOAL_LENGTH;
	max_length = MAX_LENGTH;
	setout();
	lineno = 1;
	mark = -10;

	setlocale(LC_ALL, "");

	while((op = getopt(argc, argv, "w:m:0123456789")) != -1) {
		switch(op) {
			default: // 0-9
				optarg = argv[optind - 1]; //no break
			case 'w':
				goal_length = abs(atoi(optarg));
				if(goal_length > max_length)
					max_length = goal_length;
				break;
			case 'm':
				max_length = atoi(optarg);
				if(goal_length > max_length)
					goal_length = max_length;
				break;
			case '?':
				exit(1);
		}
	}

	if(goal_length < 15) {
		fprintf(stderr, "%s: width invalid\n", basename(argv[0]));
		exit(1);
	}
				
	if (optind == argc) {
		fmt(stdin);
		oflush();
		exit(0);
	}

	while (optind < argc) {

		if (!strcmp(argv[optind], "-"))
			fmt(stdin);
		else if ((fi = fopen(argv[optind], "r")) == NULL) {
			perror(argv[optind]);
			errs++;
		} else {
			fmt(fi);
			fclose(fi);
		} 
		++optind;
	}
	oflush();

	return errs;
}

/*
 * Read up characters from the passed input file, forming lines,
 * doing ^H processing, expanding tabs, stripping trailing blanks,
 * and sending each line down for analysis.
 */
void
fmt(fi)
	FILE *fi;
{
	char linebuf[BUFSIZ], canonb[BUFSIZ];
	char *cp, *cp2;
	int c, col;

	c = getc(fi);
	while (c != EOF) {
		/*
		 * Collect a line, doing ^H processing.
		 * Leave tabs for now.
		 */
		cp = linebuf;
		while (c != '\n' && c != EOF && cp-linebuf < BUFSIZ-1) {
			if (c == '\b') {
				if (cp > linebuf)
					cp--;
				c = getc(fi);
				continue;
			}
			if(!(isprint(c) || c == '\t')) {
				c = getc(fi);
				continue;
			}
			*cp++ = c;
			c = getc(fi);
		}
		*cp = '\0';

		/*
		 * Toss anything remaining on the input line.
		 */
		while (c != '\n' && c != EOF)
			c = getc(fi);
		
		/*
		 * Expand tabs on the way to canonb.
		 */
		col = 0;
		cp = linebuf;
		cp2 = canonb;
		while ((c = *cp++) != 0) {
			if (c != '\t') {
				col++;
				if (cp2-canonb < BUFSIZ-1)
					*cp2++ = c;
				continue;
			}
			do {
				if (cp2-canonb < BUFSIZ-1)
					*cp2++ = ' ';
				col++;
			} while ((col & 07) != 0);
		}

		/*
		 * Swipe trailing blanks from the line.
		 */
		for (cp2--; cp2 >= canonb && *cp2 == ' '; cp2--)
			;
		*++cp2 = '\0';
		prefix(canonb);
		if (c != EOF)
			c = getc(fi);
	}
}

/*
 * Take a line devoid of tabs and other garbage and determine its
 * blank prefix.  If the indent changes, call for a linebreak.
 * If the input line is blank, echo the blank line on the output.
 * Finally, if the line minus the prefix is a mail header, try to keep
 * it on a line by itself.
 */
void
prefix(line)
	char line[];
{
	char *cp;
	int np, h;

	if (strlen(line) == 0) {
		oflush();
		putchar('\n');
		return;
	}
	for (cp = line; *cp == ' '; cp++)
		;
	np = cp - line;

	/*
	 * The following horrible expression attempts to avoid linebreaks
	 * when the indent changes due to a paragraph.
	 */
	h = 0;
	if (np != pfx && (np > pfx || abs(pfx-np) > 8))
		oflush();
	pfx = np;
	if (h)
		pack(cp, strlen(cp));
	else	split(cp);
	if (h)
		oflush();
	lineno++;
}

/*
 * Split up the passed line into output "words" which are
 * maximal strings of non-blanks with the blank separation
 * attached at the end.  Pass these words along to the output
 * line packer.
 */
void
split(line)
	char line[];
{
	char *cp, *cp2;
	char word[BUFSIZ];
	int wordl;		/* LIZ@UOM 6/18/85 */

	cp = line;
	while (*cp) {
		cp2 = word;
		wordl = 0;	/* LIZ@UOM 6/18/85 */

		/*
		 * Collect a 'word,' allowing it to contain escaped white
		 * space. 
		 */
		while (*cp && *cp != ' ') {
			if (*cp == '\\' && isspace(cp[1]))
				*cp2++ = *cp++;
			*cp2++ = *cp++;
			wordl++;/* LIZ@UOM 6/18/85 */
		}

		/*
		 * Guarantee a space at end of line. Two spaces after end of
		 * sentence punctuation. 
		 */
		if (*cp == '\0') {
			*cp2++ = ' ';
			if (strchr(".:!", cp[-1]))
				*cp2++ = ' ';
		}
		while (*cp == ' ')
			*cp2++ = *cp++;
		*cp2 = '\0';
		/*
		 * LIZ@UOM 6/18/85 pack(word); 
		 */
		pack(word, wordl);
	}
}

/*
 * Output section.
 * Build up line images from the words passed in.  Prefix
 * each line with correct number of blanks.  The buffer "outbuf"
 * contains the current partial line image, including prefixed blanks.
 * "outp" points to the next available space therein.  When outp is NOSTR,
 * there ain't nothing in there yet.  At the bottom of this whole mess,
 * leading tabs are reinserted.
 */
char	outbuf[BUFSIZ];			/* Sandbagged output line image */
char	*outp;				/* Pointer in above */

/*
 * Initialize the output section.
 */
void
setout()
{
	outp = NOSTR;
}

/*
 * Pack a word onto the output line.  If this is the beginning of
 * the line, push on the appropriately-sized string of blanks first.
 * If the word won't fit on the current line, flush and begin a new
 * line.  If the word is too long to fit all by itself on a line,
 * just give it its own and hope for the best.
 *
 * LIZ@UOM 6/18/85 -- If the new word will fit in at less than the
 *	goal length, take it.  If not, then check to see if the line
 *	will be over the max length; if so put the word on the next
 *	line.  If not, check to see if the line will be closer to the
 *	goal length with or without the word and take it or put it on
 *	the next line accordingly.
 */

void
pack(word,wl)
	char word[];
	int wl;      // word length
{
	char *cp;
	int s, t;

	if (outp == NOSTR)
		leadin();
	/*
	 * LIZ@UOM 6/18/85 -- change condition to check goal_length; s is the
	 * length of the line before the word is added; t is now the length
	 * of the line after the word is added
	 *	t = strlen(word);
	 *	if (t+s <= LENGTH) 
	 */
	s = outp - outbuf;
	t = wl + s;
	if ((t <= goal_length) ||
	    ((t <= max_length) && (t - goal_length <= goal_length - s))) {
		/*
		 * In like flint! 
		 */
		for (cp = word; *cp; *outp++ = *cp++);
		return;
	}
	if (s > pfx) {
		oflush();
		leadin();
	}
	for (cp = word; *cp; *outp++ = *cp++);
}

/*
 * If there is anything on the current output line, send it on
 * its way.  Set outp to NOSTR to indicate the absence of the current
 * line prefix.
 */
void
oflush()
{
	if (outp == NOSTR)
		return;
	*outp = '\0';
	tabulate(outbuf);
	outp = NOSTR;
}

/*
 * Take the passed line buffer, insert leading tabs where possible, and
 * output on standard output (finally).
 */
void
tabulate(line)
	char line[];
{
	char *cp;
	int b, t;

	/*
	 * Toss trailing blanks in the output line.
	 */
	cp = line + strlen(line) - 1;
	while (cp >= line && *cp == ' ')
		cp--;
	*++cp = '\0';
	
	/*
	 * Count the leading blank space and tabulate.
	 */
	for (cp = line; *cp == ' '; cp++)
		;
	b = cp-line;
	t = b >> 3;
	b &= 07;
	if (t > 0)
		do
			putc('\t', stdout);
		while (--t);
	if (b > 0)
		do
			putc(' ', stdout);
		while (--b);
	while (*cp)
		putc(*cp++, stdout);
	putc('\n', stdout);
}

/*
 * Initialize the output line with the appropriate number of
 * leading blanks.
 */
void
leadin()
{
	int b;
	char *cp;

	for (b = 0, cp = outbuf; b < pfx; b++)
		*cp++ = ' ';
	outp = cp;
}

/*
 * Save a string in dynamic space.
 * This little goodie is needed for
 * a headline detector in head.c
 */
char *
savestr(str)
	char str[];
{
	char *top;

	top = malloc(strlen(str) + 1);
	if (top == NOSTR) {
		fprintf(stderr, "fmt:  Ran out of memory\n");
		exit(1);
	}
	strcpy(top, str);
	return (top);
}

