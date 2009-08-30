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
%C - internal utility: kernel dump server

%C	[-v] [-p <protocol>] [<kdumper_file>]

Options:
 -P <protocol>   Set the compression protocol to <protocol>
 -v              Increase verbosity
#endif

#define EXTERN
#include "kdserver.h"


int
main(int argc, char **argv) {
	int			t;
	char		*name;

	while((t = getopt(argc, argv, "P:v")) != -1) {
		switch(t) {
		case 'P':
			protocol = strtoul(optarg, &optarg, 10);
			break;
		case 'v':
			++debug_flag;
			break;
		}
	}
	name = argv[optind];
	if(name == NULL) name = "kdump.elf";

	if(core_init(name)) {
		server();
		core_fini();
	}

	return 0;
}
