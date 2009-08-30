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
#include <errno.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __USAGE
%C - set configuration information

%C variable_name string ...

Options:
variable_name    One of CS_HOSTNAME, CS_SRPC_DOMAIN, CS_DOMAIN, CS_RESOLVE
                        CS_TIMEZONE
string           New value for the variable
#endif

struct conf_entry {
	const char     *name;
	const int       manifest;
	const enum {
		 SYSCONF, CONFSTR, PATHCONF
	} type;
};

struct conf_entry table[] =
{
	{"CS_HOSTNAME",    _CS_SET_HOSTNAME,    CONFSTR},
	{"CS_SRPC_DOMAIN", _CS_SET_SRPC_DOMAIN, CONFSTR},
	{"CS_DOMAIN",      _CS_SET_DOMAIN,      CONFSTR},
	{"CS_RESOLVE",     _CS_SET_RESOLVE,     CONFSTR},
	{"CS_TIMEZONE",    _CS_SET_TIMEZONE,    CONFSTR},
	{NULL, 0, SYSCONF}
};

int
main(int argc, char *argv[])
{
	struct conf_entry	*cop;
	char				buf[512];
	int					i;
	int					err = EXIT_SUCCESS;

	if(argc < 3 || (argc & 1) == 0) {
		fprintf(stderr, "Invalid number of arguments. Must be a multiple of 2.\n");
		exit(EXIT_FAILURE);
	}

	for(i = 1 ; i < argc ; i += 2) {
		for(cop = &table[0] ; cop->name ; ++cop) {
			if(!strcmp(cop->name, argv[1])) {

				switch(cop->type) {
				case PATHCONF:
					// Don't know how to set it.
					break;

				case SYSCONF:
					// Don't know how to set it.
					break;

				case CONFSTR:
					confstr(cop->manifest, argv[i+1], 0);
					memset(buf, 0, sizeof(buf));
					confstr(cop->manifest - _CS_SET, buf, sizeof(buf)-1);
					if(strcmp(argv[i+1], buf) != 0) {
						fprintf(stderr, "Unable to set %s\n", cop->name);
						err = EXIT_FAILURE;
					}
					break;
				}
				break;
			}
		 }

		if(cop->name == NULL) {
			fprintf(stderr, "Unrecognized variable %s\n", argv[1]);
			err = EXIT_FAILURE;
		}
	}

	exit(err);
}
