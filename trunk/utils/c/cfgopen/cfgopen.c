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




/*
#ifdef __USAGE
%C	- search for configration files (QNX)

%C	config_file

config_file   Name of the configuration file to be searched for
#endif
*/
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <cfgopen.h>

int main(int argc, char **argv) {
	char buffer[PATH_MAX];
	
	if (cfgopen(argv[1], 
			CFGFILE_RDONLY | CFGFILE_NOFD | CFGFILE_USER, 
			NULL,
			buffer,
			PATH_MAX) == -1) {
		return 1;
	}

	printf("%s\n", buffer);
	return 0;
}

