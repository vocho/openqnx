
#include <inttypes.h>

#include "memmaps.h"
#include "memstats.h"

/**
 * This structure holds the command line options for easy access.
 */
#define PROCESS_DETAIL_LIBRARY 	0x1
#define PROCESS_DETAIL_STACK   	0x2
#define PROCESS_DETAIL_HEAP    	0x4
struct showmem_opts {
	int show_system_summary;
	int show_process_summary;	
	int show_process_details;	//Flags 
};

extern int debugfd;
extern int verbose;

/**
 * Process memory information
 */
typedef struct pid_entry {
	struct pid_entry 	*next;			//Next process block
	pid_t				pid;			//The pid of this process
	uint64_t			base_addr;		//Base address of process
	
	int					block_count;	//Number of memory blocks 
	priv_memblock_t		*memblocks;		//Array of block_count
	char				*name;			//The name of this process
} pid_entry_t;

/**
 * Shared information
 */
typedef struct shared_entry {
	int							block_count;
	int							block_size;
	shared_memblock_t			*memblocks;		
} shared_entry_t;

/**
 * System memory information
 */
typedef struct system_entry {
	pid_entry_t		*pid_list;
	shared_entry_t	shared_blocks;
} system_entry_t;


/* Initialize the options structure */
system_entry_t *build_block_list(int *pidlist, int pidnum);

void iterate_processes(system_entry_t *root, 
                       int (* func)(pid_entry_t *block, void *data), void *data);
void iterate_sharedlibs(system_entry_t *root, 
                       int (* func)(shared_memblock_t *block, void *data), void *data);

void display_overview(system_entry_t *root, struct showmem_opts *opts);
                       
