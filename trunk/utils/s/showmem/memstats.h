/**
 * This header contains the data structures and definitions related to tracking
 * process memory mapping information.
 */

#include <inttypes.h>
#include <malloc.h>

/**
 * TODO: We should probably publicize this structure
 */
struct bin_stats {
	unsigned 			size;
	unsigned 			nallocs;
	unsigned 			nfrees;
	unsigned 			reserved;
};	

//Arbitrary value?
#define MAX_BINS	24

typedef struct _process_malloc_stats {
	struct malloc_stats		stats;
	struct bin_stats		bins[MAX_BINS];
	int						bin_count;
} process_malloc_stats_t;

