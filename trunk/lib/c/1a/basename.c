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




#include <libgen.h>

char *basename(char *fname) {
    char *lastslash = fname;

	if(!fname || !*fname) {
		return ".";
	}

    while(*fname) {
		if(*fname++ == '/') {
			char				*p;

			for(p = fname; *p; p++) {
				if(*p != '/') {
					lastslash = fname;
					break;
				}
			}
			if(fname - 1 != lastslash && !*p) {
				*--fname = '\0';
				break;
			}
		}
	}
	return lastslash;
}

#ifdef TEST
#define check(str, good)	{ \
	char buff[100], *p; \
	printf("basename(\"%s\")=", str ? (char *)strcpy(buff, str) : "(NULL)"); \
	p = basename(str ? buff : 0); \
	printf("\"%s\"", p); \
	if(strcmp(p, good)) \
		printf(" - BAD, should be \"%s\"\n", good); \
	else \
		printf(" - OK\n"); \
}
int main(void) {
	check("/usr/lib", "lib");
	check("/usr/", "usr");
	check("usr", "usr");
	check(".", ".");
	check("/", "/");
	check("", ".");
	check(0, ".");
	return 0;
}
#endif

__SRCVERSION("basename.c $Rev: 153052 $");
