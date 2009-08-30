/*
 This provides the tracing breakpoint control.
 TODO: 
 - Provide symbol -> address translation
 - Provide support for global symbol + offset (ie for libraries)
 - Provide multiple pid listing support
 - Provide appropriate access control flags for opening procfs
 - Externalize structures into common header
 */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <malloc.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>
#include <dirent.h>
#include <inttypes.h>
#include <ctype.h>

#include <sys/procfs.h>
#include <sys/debug.h>

typedef struct _pid_info {
	pid_t			pid;
	char 			*name;
	int				fd;
} pid_info_t;

//TODO: Add support for "offsets"
typedef struct _addr_info {
	struct _addr_info 	*next;
	uint32_t			addr;
} addr_info_t;

typedef struct _symbol_lookup {
	struct _symbol_lookup *next;
	uint32_t 	load_address;
	char		*symbol_path;
} symbol_lookup_t;

#define ACTION_LIST  	0x00000000
#define ACTION_ADD   	0x00000001
#define ACTION_REMOVE   0x00000002
#define ACTION_MASK		0x0000000f

#define FLAG_NEED_PID	0x10000000
#define FLAG_NEED_ADDR  0x20000000

int verbose = 0;

void print_breakpoint_list(debug_break_t *list, int count) {
	int i;

	for(i = 0; i < count; i++) {
		printf("[%02d] 0x%08x \n", i, list[i].addr);
	}
}

debug_break_t *get_breakpoint_list(int fd, int *count) {
	int	 ret, size, num;
	debug_break_t *list;

	list = NULL;
	size = 0;
	do {
		ret = devctl(fd, DCMD_PROC_GET_BREAKLIST, list, size, &num);	
		if(ret != EOK) {
			fprintf(stderr, "breaklist devctl failed %d %s \n", ret, strerror(ret));
			return NULL;
		}
		if(size >= (num * sizeof(*list))) {
			break;
		}

		size = num * sizeof(*list);
		list = realloc(list, size);
		if(list == NULL) {
			printf("No more memory \n");
			return NULL;
		}	
	} while(1);

	close(fd);

	*count = num;

	return list;
}

void add_breakpoint_addresses(addr_info_t *addresses, pid_info_t *pidinfo) {
	addr_info_t *a;

	for(a = addresses; a != NULL; a = a->next) {
		printf("Add address 0x%x \n", a->addr);
	}
}

void remove_breakpoint_addresses(addr_info_t *addresses, pid_info_t *pidinfo) {
	addr_info_t *a;

	for(a = addresses; a != NULL; a = a->next) {
		printf("Remove address 0x%x \n", a->addr);
	}
}

/*
 Format for this string to be determined.  For now support:
 <name>    
 <addr:name>
*/
void add_to_symbol_list(symbol_lookup_t **symbol_list, char *symbol_path) {
	char 			*sep;
	symbol_lookup_t *newsym;

	//Eat up initial whitespace
	while(*symbol_path == ' ') { symbol_path++; }
	if(*symbol_path == '\0') {
		return;
	}

	newsym = calloc(1, sizeof(*newsym));
	if(newsym == NULL) {
		return;
	}

	sep = strchr(symbol_path, ':');
	if(sep == NULL) {
		newsym->load_address = 0;
	} else {
		newsym->load_address = strtoul(sep+1, NULL, 0);
		*sep = '\0';
	}

	//@@@ What if no LD_LIBRARY_PATH
	sep = pathfind(getenv("LD_LIBRARY_PATH"), symbol_path, "r");
	if(sep == NULL) {
		sep = pathfind(getenv("PATH"), symbol_path, "r");
		if(sep == NULL) {
			fprintf(stderr, "Can't locate library %s \n", symbol_path);
			free(newsym);
			return;
		} else if(verbose) {
			printf("Symbol from PATH [%s] \n", sep);
		}
	} else if (verbose) {
		printf("Symbol from LD_LIBRARY_PATH [%s] \n", sep);
	}

	newsym->symbol_path = strdup(sep);

	newsym->next = *symbol_list;
	*symbol_list = newsym;
}

/*
 Format for this string to be determined.  For now support:
 <addr>    
*/
void add_to_addr_list(addr_info_t **addr_list, char *address) {
	addr_info_t *newaddr;
	char		*end;
	uint32_t	addr;

	//Eat up initial whitespace
	while(*address == ' ') { address++; }
	if(*address == '\0') {
		return;
	}

	addr = strtoul(address, &end, 0);
	if(end == address) {
		fprintf(stderr, "Can't decode address %s \n", address);
		return;
	}

	newaddr = calloc(1, sizeof(*newaddr));
	if(newaddr == NULL) {
		return;
	}

	newaddr->addr = addr;

	newaddr->next = *addr_list;
	*addr_list = newaddr;
}


/*
 Convert a name or pid to the name field and an open fd
 */
int fill_pid_info(pid_info_t *info, char *pidthing) {
	struct {
		procfs_debuginfo  info;
		char			  name[50];
	} debuginfo;
	char procfsname[20];
	char *end;
	
	//Eat up initial whitespace
	while(*pidthing == ' ') { pidthing++; }
	if(*pidthing == '\0') {
		return -1;
	}

	info->pid = strtoul(pidthing, &end, 0);
	if(end == pidthing) {		//Not a number, must be a name
		DIR		*dp;
		struct dirent 	*dent;
		int		ret;
		
		dp = opendir("/proc");
		if(dp == NULL) {
			return -1;
		}
		
		info->fd = -1;
		while((dent = readdir(dp)) != NULL) {
			if(!isdigit(*dent->d_name)) {
				continue;
			}

			snprintf(procfsname, sizeof(procfsname), "/proc/%s/as", dent->d_name);
			info->fd = open(procfsname, O_RDONLY);	
			if(info->fd == -1) {
				continue;
			}

			debuginfo.info.path[0] = '0';
			ret = devctl(info->fd, DCMD_PROC_MAPDEBUG_BASE, &debuginfo, sizeof(debuginfo), NULL);
			if(ret != EOK || *debuginfo.info.path == '\0') {
				close(info->fd);
				info->fd = -1;
				continue;
			}
			
			//TODO: Be wary of duplicate names
			if(strcmp(basename(debuginfo.info.path), pidthing) == 0) {
				info->pid = strtoul(dent->d_name, NULL, 0);
				if(verbose) {
					printf("Mapping [%s] to pid %d\n", pidthing, info->pid);
				}
				break;
			}
			close(info->fd);
			info->fd = -1;
		}
		closedir(dp);

		if(info->fd == -1) {
			return -1;
		}
	} else {
		snprintf(procfsname, sizeof(procfsname), "/proc/%d/as", info->pid);
		info->fd = open(procfsname, O_RDONLY);	
		if(info->fd == -1) {
			return -1;
		}
	}

	return 0;
}

int main(int argc, char **argv) {
	int ret, action;
	pid_info_t			pidinfo;
	addr_info_t			*addresses;
	symbol_lookup_t		*symbols;

	//Default action is to list breakpoints
	action = ACTION_LIST | FLAG_NEED_PID;
	symbols = NULL;
	addresses = NULL;

	while((ret = getopt(argc, argv, "s:lar")) != -1) {
		switch(ret) {
		case 's':
			add_to_symbol_list(&symbols, optarg);
			break;
		case 'l':
			action = ACTION_LIST | FLAG_NEED_PID;
			break;
		case 'a':
			action = ACTION_ADD | FLAG_NEED_PID | FLAG_NEED_ADDR;
			break;
		case 'r':
			action = ACTION_REMOVE | FLAG_NEED_PID | FLAG_NEED_ADDR;
			break;
		case 'v':
			verbose++;
			break;
		default:
usage:
			printf("Usage: \n"
                   "  %s -l [-v] [-s <symfile>] <pid>\n"
                   "  %s -a [-v] [-s <symfile>] <pid> <addr> [<addr> ...]\n"
                   "  %s -r [-v] [-s <symfile>] <pid> <addr> [<addr> ...]\n"
                   "Where: \n"
				   " l  List the trace addresses (default) \n"
				   " a  Add the trace events at the specified addresses \n"
			       " r  Remove trace events at the specified addresses \n"
			       " s  Specifies symbol file to load for name->address resolution \n"
			       " v  Indicates an increased level of verbosity should be used \n"
			       "pid \n"
			       " Specifies a process by id or name; name must be unique \n"
			       "addr \n"
			       " Specifies an address to break at \n",
					argv[0], argv[0], argv[0]);
			return 0;
		}
	}

	//Expect now for a <pid> <addr> listing
	if(optind >= argc) {
		goto usage;
	}

	if(action & FLAG_NEED_PID) {
		if(fill_pid_info(&pidinfo, argv[optind]) == -1) {
			fprintf(stderr, "Can't get pid [%s] %s \n", argv[optind], strerror(errno));
			return 1;
		}
		optind++;
	}

	if(action & FLAG_NEED_ADDR) {
		while(optind < argc) {
			add_to_addr_list(&addresses, argv[optind]);
			optind++;
		}
		if(addresses == NULL) {
			fprintf(stderr, "No addresses provided \n");
			return 1;
		}
	}

	switch((action & ACTION_MASK)) {
	case ACTION_LIST: {
		debug_break_t 	*breakpoints;
		int				count;

		breakpoints = get_breakpoint_list(pidinfo.fd, &count);
		if(breakpoints == NULL) {
			fprintf(stderr, "Can't get breakpoint list \n");
			return 1;
		}
		print_breakpoint_list(breakpoints, count);	
		break;
		}

	case ACTION_ADD:
		add_breakpoint_addresses(addresses, &pidinfo);
		break;

	case ACTION_REMOVE:
		remove_breakpoint_addresses(addresses, &pidinfo);
		break;

	default:
		printf("Huh? \n");
	}

	return 0;
}
