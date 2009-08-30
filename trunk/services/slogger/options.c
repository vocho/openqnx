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



 
#include "externs.h"


void options(int argc, char *argv[]) {
	int		 opt;
	char	*cp;

	// Setup defaults
	NumInts = 4096;		// Works out to a 16K buffer
	LogFsize = 0;		// Default is to grow and grow...
	FilterLog = _SLOG_DEBUG1;	// Log everything
	LogFflags = 0;		// Default no flags set

	while((opt = getopt(argc, argv, "cf:l:s:v")) != -1) {
	switch(opt) {
			case 'c':             /* Commit modifications as per O_SYNC */
			LogFflags |= LOGF_FLAG_OSYNC;
			break;
		case 'f':
			FilterLog = atoi(optarg);	// Not coded yet
			break;		

		case 'l':
			LogFname = optarg;
			if((cp = strchr(LogFname, ','))) {
				LogFsize = atoi(cp + 1)*1024;
				cp[0] = '\0';
			}
			break;

		case 's':
			NumInts = atoi(optarg)*(1024/4);
			if(NumInts < 1024)
				NumInts = 1024;
			break;

		case 'v':
			++Verbose;
			break;

		default:
			exit(EXIT_FAILURE);
		}
	}
}

__SRCVERSION("options.c $Rev: 157840 $");
