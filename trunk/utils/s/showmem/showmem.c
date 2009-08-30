/*
 * This is a utility program to demonstrate where _all_ of the memory of the system is being used
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

extern char *optarg;
extern int optind;

#include "showmem.h"

int debugfd = -1;
int verbose = 0;

void init_showmem_opts (struct showmem_opts *opts) {
	memset(opts, 0, sizeof(*opts));
}

int main(int argc, char **argv) {
	struct showmem_opts opts;
	system_entry_t		*system;
	int 				*pidlist;
	int 				c, pidnum; 
	int					noopts = 1;

	init_showmem_opts(&opts);

	while ((c=getopt(argc, argv, "vd:SPD:")) != -1) {
		noopts = 0;
		switch (c) {
			case 'v':
				verbose++;
				break;
			case 'd':
				debugfd = open(optarg, O_CREAT | O_TRUNC | O_RDWR);
				if(debugfd == -1) {
					printf("Can't open debug file %s \n", optarg);
					return 1;
				}	
				break;
			case 'S':
				opts.show_system_summary = 1;
				break;
			case 'P':
				opts.show_process_summary = 1;
				break;
			case 'D':
				opts.show_process_summary = 1;
				opts.show_process_details = 0;
				while(*optarg != '\0') {
					switch(*optarg) {
					case 'l':
						opts.show_process_details |= PROCESS_DETAIL_LIBRARY;
						break;
					case 's':
						opts.show_process_details |= PROCESS_DETAIL_STACK;
						break;
					case 'h':
						opts.show_process_details |= PROCESS_DETAIL_HEAP;
						break;
					}
					optarg++;
				}
				break;
			
			case '?':
			default:
usemsg:
				printf("%s [options] - display memory information \n", argv[0]);
				printf("-S          Show memory summary (free/used) for the system\n"
					   "-P          Show process/sharedlibrary/ summary for the system\n"
					   "-D[lsh]     Show detailed process information\n" 
             		   " l = libraries\n"
             		   " s = stack\n"
             		   " h = heap\n"
					   "-d file     Dump the raw mapinfo to the specified file\n" 
						);
				return 0;
		}
	}

	if(noopts) {
		goto usemsg;
	}

	pidnum = argc-optind;

	if (pidnum > 0) {
		pidlist = (int*)malloc(pidnum*sizeof(int));
		if(pidlist == NULL) {
			return 1;
		}
		
		for (c = 0; c < pidnum; c++) {
			pidlist[c] = atoi(argv[optind+c]);
		}
	} else {
		pidnum = 0;
		pidlist = NULL;
	}
	
	system = build_block_list(pidlist, pidnum);
	
	if(pidlist) {
		free(pidlist);
	}
	
	if(system == NULL) {
		fprintf(stderr, "Can't build block list %s\n", strerror(errno));
		return 1;
	}
		
	display_overview(system, &opts);
	
	return 0;
}
