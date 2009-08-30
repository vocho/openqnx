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
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <libgen.h>
#include "confvar.h"

/* TODO
- _POSIX_* should be the POSIX max/min's
- Allow _XOPEN_* _POSIX_* to be optional
- know what the datatype is, so UINT_MAX can be 4294967295 instead of -1
- Notice that seting a confstr can silently fail
*/

static char					*progname;

static void print(void) {
	struct variable			*p;
	const char				spaces[] = "                                   ";
	char					*buff = 0;
	int						buffsize = 0;

	for(p = table; p->string; p++) {
		int					pos;

		if(p->flags & (FLAG_NAME | FLAG_VALUE)) {
			pos = printf("%s:", p->string);
		} else {
			pos = printf("***%s:", p->string);
		}
		fflush(stdout);
		printf("%.*s", (int)((sizeof spaces - 1) - pos), spaces);

		switch(p->flags & FLAG_TYPE) {
		case FLAG_CONFSTR: {
			int					size;

			while((size = confstr(p->name, buff, buffsize)) > buffsize && size > 0) {
				if(!(buff = realloc(buff, buffsize = size + 1))) {
					fprintf(stderr, "No memory\n");
					exit(EXIT_FAILURE);
				}
			}
			if(size == -1) {
				printf("Invalid name");
				break;
			}
			if(size == 0) {
				printf("undefined");
			} else {
				char				*p;

				for(p = buff; *p; p++) {
					if(isprint(*p)) {
						putchar(*p);
					} else {
						if(*p == '\n') {
							printf("\\n");
						} else {
							printf("\\x%x", *p);
						}
					}
				}
			}
			break;
		}
		case FLAG_SYSCONF: {
			long				value;

			errno = EOK;
			if((value = sysconf(p->name)) == -1 && errno != EOK) {
				printf("Invalid name");
				break;
			}
			if(value == -1) {
				printf("undefined");
			} else {
				printf("%ld", value);
			}
			break;
		}
		case FLAG_PATHCONF:
			printf("_PC_");
			break;
		}
		printf("\n");
	}
}

static void ambiguious(const char *variable) {
	fprintf(stderr, "%s: '%s' has more than one match.\n", progname, variable);
	exit(EXIT_FAILURE);
}

static int check(struct variable *p, int type, const char *variable, const char *prefix) {
	int						len;
	const char				*str;
	int						val;

	if((p->flags & FLAG_TYPE) != type) {
		return 0;
	}

	if(*variable == '_') {
		variable++;
	}

	str = p->string;
	val = 2;
	while(*str == '_') {
		str++;
		val = 0;
	}

	len = strlen(prefix);

	if(!strncmp(prefix, variable, len) && !strcmp(variable + len, str)) {
		return val + 2;
	}
	if(!strnicmp(prefix, variable, len) && !stricmp(variable + len, str)) {
		return val + 1;
	}
	return 0;
}
	
int main(int argc, char *argv[]) {
	int						c;
	char					*spec = 0;
	int						setconf = 0;
	struct variable			*p, *var;
	char					*variable;
	char					*arg = 0;
	int						rank;
	int						i;
	long					ret;

	if(!strcmp(progname = basename(argv[0]), "setconf")) {
		setconf = 1;
		while((c = getopt(argc, argv, "")) != -1) {
			switch(c) {
			default:
				break;
			}
		}
	} else {
		while((c = getopt(argc, argv, "sav:")) != -1) {
			switch(c) {
			case 'a':
				print();
				return EXIT_SUCCESS;
			case 's':
				setconf = 1;
				break;
			case 'v':
				spec = optarg;
				break;
			default:
				return EXIT_FAILURE;
			}
		}

	}

	if(argc - optind <= 0) {
		fprintf(stderr, "%s: Must specify a variable.\n", progname); 
		return EXIT_FAILURE;
	}
	variable = argv[optind++];
	if(argc - optind > 0) {
		arg = argv[optind++];
	}
	if(argc - optind > 0) {
		fprintf(stderr, "%s: Too many arguments.\n", progname); 
		return EXIT_FAILURE;
	}
#ifdef _CS_SET
	if(setconf && !arg) {
		fprintf(stderr, "%s: Must specify variable and value.\n", progname); 
		return EXIT_FAILURE;
	}
#else
	if(setconf) {
		fprintf(stderr, "%s: Unable to set on this OS.\n", progname); 
		return EXIT_FAILURE;
	}
#define _CS_SET		0
#endif
	if(setconf && geteuid() != 0) {
		fprintf(stderr, "%s: Must be root to set variables.\n", progname); 
		return EXIT_FAILURE;
	}


	var = 0;
	rank = 0;
	for(p = table; p->string; p++) {
		if(!strcmp(p->string, variable)) {
			if(rank < 20) {
				rank = 20;
				var = p;
			} else {
				ambiguious(variable);
			}
		} else if(!stricmp(p->string, variable)) {
			if(rank < 19) {
				rank = 19;
				var = p;
			} else {
				ambiguious(variable);
			}
		} else if(*p->string == '_' && !strcmp(p->string + 1, variable)) {
			if(rank < 18) {
				rank = 18;
				var = p;
			} else {
				ambiguious(variable);
			}
		} else if(*p->string == '_' && !stricmp(p->string + 1, variable)) {
			if(rank < 17) {
				rank = 17;
				var = p;
			} else {
				ambiguious(variable);
			}
		} else if((i = check(p, FLAG_CONFSTR, variable, "CS_"))) {
			if(rank < 12 + i) {
				rank = 12 + i;
				var = p;
			} else {
				ambiguious(variable);
			}
		} else if((i = check(p, FLAG_SYSCONF, variable, "SC_"))) {
			if(rank < 12 + i) {
				rank = 12 + i;
				var = p;
			} else {
				ambiguious(variable);
			}
		} else if((i = check(p, FLAG_PATHCONF, variable, "PC_"))) {
			if(rank < 12 + i) {
				rank = 12 + i;
				var = p;
			} else {
				ambiguious(variable);
			}
		}
	}

	if(!var) {
		fprintf(stderr, "%s: Can't find %s\n", progname, variable);
		return EXIT_FAILURE;
	}
	if((var->flags & FLAG_TYPE) == FLAG_PATHCONF) {
		if(setconf) {
			fprintf(stderr, "%s: Can't set path variables.\n", progname);
			return EXIT_FAILURE;
		}
		if(!arg) {
			fprintf(stderr, "%s: Path variables require a path.\n", progname);
			return EXIT_FAILURE;
		}
		errno = EOK;
		if((ret = pathconf(arg, var->name)) == -1 && errno != EOK) {
			if(errno == EINVAL) {
				printf("undefined\n");
				return EXIT_FAILURE;
			}
			fprintf(stderr, "%s: Unable to query '%s' %s (%d).\n", progname, arg, strerror(errno), errno);
			return EXIT_FAILURE;
		}
	} else {
		if(setconf) {
			if((var->flags & FLAG_TYPE) == FLAG_SYSCONF) {
				fprintf(stderr, "%s: Can't set sysconf variables.\n", progname);
				return EXIT_FAILURE;
			}
			if(confstr(var->name | _CS_SET, arg, 0) == -1) {
				fprintf(stderr, "%s: Unable to set %s, %s (%d).\n", progname, var->string, strerror(errno), errno);
				return EXIT_FAILURE;
			}
			return EXIT_SUCCESS;
		}
		if(arg) {
			fprintf(stderr, "%s: Extra argument for %s variable.\n", progname, var->string);
			return EXIT_FAILURE;
		}
		if((var->flags & FLAG_TYPE) == FLAG_CONFSTR) {
			int					len;
			char				*buff = 0;

			if(var->name & _CS_SET) {
				fprintf(stderr, "%s: Internal error, can't query a _CS_SET.\n", progname);
				return EXIT_FAILURE;
			}

			errno = EOK;
			if((len = confstr(var->name, 0, 0)) > 0) {
				if(!(buff = alloca(len + 1))) {
					fprintf(stderr, "%s: No memory for string.\n", progname);
					return EXIT_FAILURE;
				}
				errno = EOK;
				len = confstr(var->name, buff, len + 1);
			}
			if(errno != EOK) {
				fprintf(stderr, "%s: Unable to get %s.\n", progname, var->string);
				return EXIT_FAILURE;
			}
			if(len <= 0) {
				printf("undefined\n");
				return EXIT_FAILURE;
			} else {
				printf("%s\n", buff);
			}
			return EXIT_SUCCESS;
		} else if((var->flags & FLAG_TYPE) == FLAG_SYSCONF) {
			errno = EOK;
			if((ret = sysconf(var->name)) == -1 && errno != EOK) {
				if(errno == EINVAL) {
					printf("undefined\n");
					return EXIT_FAILURE;
				}
				fprintf(stderr, "%s: Unable to query '%s' %s (%d).\n", progname, var->string, strerror(errno), errno);
				return EXIT_FAILURE;
			}
		} else {
			fprintf(stderr, "%s: Unable to query variable %s.\n", progname, var->string);
			return EXIT_FAILURE;
		}
	}
	printf("%ld\n", ret);
	return EXIT_SUCCESS;
}
