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


#include <lib/compat.h>

#include <stdlib.h>
#include <string.h>

int getsubopt(char **optionp, char * const *tokens, char **valuep) {
	char			*p, *opt;
	int				len, index;
	const char		*token;

	*valuep = 0;
	for(opt = p = *optionp, len = 0; *p && *p != ','; p++, len++) {
		if(*p == '=') {
			for(*valuep = ++p; *p && *p != ','; p++) {
				/* nothing to do */
			}
			break;
		}
	}
	if(*p) {
		*p++ = '\0';
	}
	*optionp = p;
		
	for(index = 0; (token = *tokens++); index++) {
		if(*token == *opt && !strncmp(token, opt, len) && token[len] == '\0') {
			return index;
		}
	}
	*valuep = opt;
	return -1;
}

#ifdef TEST
#include <stdio.h>

int main(int argc, char *argv[]) {
	char			*tokens[] = {
		"testi",
		"test",
		"testing",
		"testin",
		"notest",
		0
	};

	if(argc <= 1) {
		char		**ptr, *p;

		printf("%s args\ntokens=", argv[0]);
		for(ptr = tokens; p = *ptr++;) {
			printf("%s,", p);
		}
		printf("\n");
	} else {
		char			*opt = argv[1];

		while(*opt) {
			char			*value;
			int				index;

			index = getsubopt(&opt, tokens, &value);
			printf("index=%d", index);
			if(index != -1) {
				printf("(%s)", tokens[index]);
			}
			printf(" value=%s\n", value ? value : "(NULL)");
		}
	}
	return 1;
}
#endif

