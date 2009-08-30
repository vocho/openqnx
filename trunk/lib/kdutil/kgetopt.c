/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 *
 * You must obtain a written license from and pay applicable 
 * license fees to QNX Software Systems before you may reproduce, 
 * modify or distribute this software, or any work that includes 
 * all or part of this software.   Free development licenses are 
 * available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email 
 * licensing@qnx.com.
 * 
 * This file may contain contributions from others.  Please review 
 * this entire file for other proprietary rights or license notices, 
 * as well as the QNX Development Suite License Guide at 
 * http://licensing.qnx.com/license-guide/ for other information.
 * $
 */

char *optarg;
int  optind = 0;
int  optopt = 0;
int  opterr = 0;

int getopt (int argc, char **argv, char *opts) {
    static	char	*cp;
    static	int		last_optind;

	if (optind == 0) {
		optind = 1;
	}
	if (optind >= argc) {
		return -1;
	}
	if (last_optind != optind) {
		cp = argv [optind];
		last_optind = optind;
		if (*cp++ != '-' || *cp == 0) {
			return -1;
		}
	}
	if (*cp == '-') {
		++optind;
		return -1;
	}
	for (; *opts; ++opts) {
		if (*opts == ':') continue;
		if (*cp == *opts) break;
	}
	optarg = cp;
	++optind;
	optopt = *opts++;
	if (optopt) {
		if (*++cp) {
			if (*opts == ':') {
				optarg = cp;
			} else {
				--optind;
			}
		} else {
			if (*opts == ':') {
				if (optind < argc) {
				    optarg = argv [optind];
					optind++;
				} else {
					optopt = '?';
				}
			}
		}
	} else {
		optopt = '?';
	}
	return optopt;
}
