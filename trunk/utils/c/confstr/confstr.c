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




#include <stdio.h>
#include <unistd.h>
#include <confname.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

static struct options {
	char *str;
	int  name;
} csname[] = {
	{"architecture", _CS_ARCHITECTURE},
	{"domain", _CS_DOMAIN},
	{"hostname", _CS_HOSTNAME},
	{"hwprovider", _CS_HW_PROVIDER},
	{"hwserial", _CS_HW_SERIAL},
	{"libpath",_CS_LIBPATH},
	{"machine", _CS_MACHINE},
	{"path", _CS_PATH},
	{"release", _CS_RELEASE},
	{"resolve", _CS_RESOLVE},
	{"srpc_domain", _CS_SRPC_DOMAIN},
	{"sysname", _CS_SYSNAME},
	{"version", _CS_VERSION},
	{0, 0}
};

#define PROGNAME "confstr"

static char buffer[256 + 1];

void show_all_str() {
	struct options *cs = csname;

	while (cs->str) {
		if (confstr(cs->name, buffer, sizeof(buffer)) > 0)
		  printf( "%s = \"%s\"\n", cs->str, buffer);
		cs++;
	}
	return;
}

int main(int argc, char **argv)
{
	int i = 0;
	struct options *cs = csname;
	char *cpn, *cpv;

	if (stricmp(basename(argv[0]), PROGNAME) == 0) {
		cpn = argv[1];
		i = 2;
	} else {
		cpn = (char *)basename(argv[0]);
		i = 1;
	}

	if (argc < 2 && i == 2) {
		show_all_str();
		return 1;
	}


	if (argc > i)
	  cpv = argv[i];
	else
	  cpv = NULL;

	while (cs->str) {
		if (stricmp(cs->str, cpn) == 0) {
			if (!cpv) {
				if (confstr(cs->name, buffer, sizeof(buffer)) > 0)
				  printf( "%s\n", buffer);
				else
				  printf( "Get %s: %s\n", cpn, strerror(errno));
			} else {
				if (confstr(cs->name + _CS_SET, cpv, 0) == -1)
				  printf( "Set %s: %s\n", cpn, strerror(errno));
			}
			break;
		}
		cs++;
	}

	if (!cs->str) {
		printf("Unknown confstr(%s)\n", cpn);
		return 1;
	} else {
		return 0;
	}
}
