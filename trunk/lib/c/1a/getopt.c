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




#include <unistd.h>
#include <malloc.h>
#include <string.h>
#include <libgen.h>

char *optarg;
int optind = 1;
int opterr = 1;
int optopt;

int getopt(int argc, char * const argv[], const char *optstring) {
	const char			*p, *opt;
	static char			const *saved_arg;
	static int			optpos;

	/* if argv[optind] is null, or *argv[optind] is not '-' or argv[optind] is "-", return -1 */
	if(optind >= argc || !(p = argv[optind]) || *p != '-' || !*++p) {
		optpos = 0;
		return -1;
	}

	/* if argv[optind] is "--", return -1 after incrementing optind */
	if(*p == '-' && !p[1]) {
		optind++;
		optpos = 0;
		return -1;
	}

	/* If different arg list, reset option position */
	if(p != saved_arg) {
		saved_arg = p;
		optpos = 0;
	}

	/* point to next option in a combined argument */
	p += optpos;

	/* Skip leading colon */
	if(*(opt = optstring) != ':') {
		opt--;
	}

	optopt = *p;
	while(*++opt) {
		if(*opt != ':' && *p == *opt) {
			/* Found a matching option */
			p++;
			if(*++opt == ':') {
				/* Option requires a operand */
				optpos = 0;
				optind++;
				if(*p) {
					/* operand is tail of current argument */
					optarg = (char *)p;
				} else {
					/* operand is the next argument */
					optarg = argv[optind++];
					if(optind > argc) {
						if(*optstring == ':') {
							/* Caller wants to know operand was missing */
							return ':';
						}
						break;
					}
				}
			} else {
				/* No operand required, so argument may hold multiple options */
				if(*p) {
					/* multiple options, remember next one */
					optpos++;
				} else {
					/* no more options, go to next argument */
					optpos = 0;
					optind++;
				}
			}
			/* Return the matching option */
			return optopt;
		}
	}

	/* There was an error, skip to next option */
	if(!*opt) {
		if(*++p) {
			optpos++;
		} else {
			optpos = 0;
			optind++;
		}
	}

	if(*optstring != ':' && opterr) {
		/* Called wants this function to display the errors as per 1003.2 4.27.6.2 */
		char				*str;
		const char			*err;
		int					errlen;

		/* Could eventually get strings from a locale */
		if(*opt) {
			err = ": " "option requires an argument" " -- ";
		} else {
			err = ": " "illegal option" " -- ";
		}

		/* Try to allocate stack for error message */
		errlen = strlen(err);
		if((str = alloca(strlen(argv[0]) + errlen + 5))) {
			int					len;

			/* basename() can damage string, so copy it */
			strcpy(str, argv[0]);
			len = strlen(p = basename(str));
			memmove(str, p, len);

			/* Add the error message */
			strcpy(str + len, err);

			/* And the option that failed */
			str[errlen += len] = optopt;
			str[++errlen] = '\n';
			errlen++;
			err = str;
		}

		/* Do one atomic write so it looks good and doesn't pull in stdio */
		write(STDERR_FILENO, err, errlen);
	}

	/* Always return a question mark */
	return '?';
}

#ifdef TEST
#include <stdio.h>
#include <ctype.h>

int main(int argc, char *argv[]) {
	char			*optstring = ":abcd:e:f:";
	int				c;

	if(argc < 2) {
		fprintf(stderr, "use: %s args\n", argv[0]);
		return 0;
	}

	printf("optstring=%s optind=%d opterr=%d\n", optstring, optind, opterr);
	fflush(stdout);
	while((c = getopt(argc, argv, optstring)) != -1) {
		printf("c=%d'%c' optopt='%c' optind=%d optarg=\"%s\"\n",
			c, isprint(c) ? c : '¿', optopt, optind, optarg ? optarg : "(NULL)");
		fflush(stdout);
	}
	printf("c=%d'%c' optopt='%c' optind=%d optarg=\"%s\"\nDone\n\n",
		c, isprint(c) ? c : '¿', optopt, optind, optarg ? optarg : "(NULL)");

	optind = 0;
	printf("optstring=%s optind=%d opterr=%d\n", optstring, optind, opterr);
	fflush(stdout);
	while((c = getopt(argc, argv, optstring)) != -1) {
		printf("c=%d'%c' optopt='%c' optind=%d optarg=\"%s\"\n",
			c, isprint(c) ? c : '¿', optopt, optind, optarg ? optarg : "(NULL)");
		fflush(stdout);
	}
	printf("c=%d'%c' optopt='%c' optind=%d optarg=\"%s\"\nDone\n\n",
		c, isprint(c) ? c : '¿', optopt, optind, optarg ? optarg : "(NULL)");

	optstring++;
	optind = 1;
	printf("optstring=%s optind=%d opterr=%d\n", optstring, optind, opterr);
	fflush(stdout);
	while((c = getopt(argc, argv, optstring)) != -1) {
		printf("c=%d'%c' optopt='%c' optind=%d optarg=\"%s\"\n",
			c, isprint(c) ? c : '¿', optopt, optind, optarg ? optarg : "(NULL)");
		fflush(stdout);
	}
	printf("c=%d'%c' optopt='%c' optind=%d optarg=\"%s\"\nDone\n\n",
		c, isprint(c) ? c : '¿', optopt, optind, optarg ? optarg : "(NULL)");

	opterr = 0;
	optind = 1;
	printf("optstring=%s optind=%d opterr=%d\n", optstring, optind, opterr);
	fflush(stdout);
	while((c = getopt(argc, argv, optstring)) != -1) {
		printf("c=%d'%c' optopt='%c' optind=%d optarg=\"%s\"\n",
			c, isprint(c) ? c : '¿', optopt, optind, optarg ? optarg : "(NULL)");
		fflush(stdout);
	}
	printf("c=%d'%c' optopt='%c' optind=%d optarg=\"%s\"\nDone\n\n",
		c, isprint(c) ? c : '¿', optopt, optind, optarg ? optarg : "(NULL)");

	return 0;
}
#endif

__SRCVERSION("getopt.c $Rev: 153052 $");
