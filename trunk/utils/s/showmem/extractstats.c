
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dlfcn.h>

#include "memstats.h"

#define MALLOC_STATS_SYMBOL 	"_malloc_stats"
#define NDLIST_BIN_SYMBOL 		"__ndlist_stat_bins"
#define DLIST_BIN_SYMBOL 		"__dlist_stat_bins"

/*
 * Find the address of a particular symbol (sym) in another process.
 * TODO: Make this resolve to an external service of some sort.  For
 * now look at our own symbols and guess that that is where the symbol
 * lives in the other process.
 * TODO: Make this return an address proper
 */
static void *find_sym_addr(int pid, char *sym) {
	void *h, *s = NULL;
	
	h = dlopen(0, 0);
	if(h != 0) {
		s = dlsym(h, sym);
		dlclose(h);
	} 

	return s;
}

/*
 * Get address to _malloc_stats in target pid and read stats from
 * the aspace
 */
static int get_malloc_stats(int fd, pid_t pid, struct malloc_stats *stats) {
	void *m;
	int i;

	if((m = find_sym_addr(pid, MALLOC_STATS_SYMBOL)) == NULL) {
		return -1;
	}

	memset(stats, 0, sizeof *stats);
	lseek(fd, (off_t)(uintptr_t)m, SEEK_SET);
	if((i = read(fd, stats, sizeof *stats)) != (sizeof *stats)) {
		return -1;
	};

	return 0;
}

/*
 * Get address to _malloc_stats in target pid and read stats from
 * the aspace
 */
static int get_bin_stats(int fd, pid_t pid, struct bin_stats *stats, int max_bins) {
	void 				*m;
	int 				i, n, cur_bin = 0;
	struct bin_stats  	*dlist_stats;

	// Get the dlist stats
	if((m = find_sym_addr(pid, NDLIST_BIN_SYMBOL)) == NULL) {
		return -1;
	}
	lseek(fd, (off_t)(uintptr_t)m, SEEK_SET);
	if((i = read(fd, &n, sizeof n)) != (sizeof n)) {
		return -1;
	};

	if(n > max_bins) {
		n = max_bins;
	}
	
	dlist_stats = alloca(sizeof *dlist_stats * n);
	if((m = find_sym_addr(pid, DLIST_BIN_SYMBOL)) == NULL) {
		return -1;
	}
	lseek(fd, (off_t)(uintptr_t)m, SEEK_SET);
	if((i = read(fd, dlist_stats, sizeof *dlist_stats * n)) != (sizeof *dlist_stats * n)) {
		return -1;
	};

	cur_bin=0;
	for(i = 0; i < n; i++) {
		stats[cur_bin].size = dlist_stats[i].size;
		stats[cur_bin].nallocs = dlist_stats[i].nallocs;
		stats[cur_bin].nfrees = dlist_stats[i].nfrees;
		cur_bin++;
	}

	return cur_bin;
}

process_malloc_stats_t *gather_process_statistics(pid_t pid) {
	process_malloc_stats_t 	*stats;
	int					fd;
	char 				buf[128];
	
	stats = malloc(sizeof(*stats));
	if(stats == NULL) {
		return NULL;
	}

	sprintf(buf, "/proc/%d/as", pid);
	if((fd = open(buf, O_RDONLY)) < 0) {
		free(stats);
		return NULL;
	}
		
	get_malloc_stats(fd, pid, &stats->stats);

	stats->bin_count = get_bin_stats(fd, pid, stats->bins, MAX_BINS);
	
	close(fd);
	
	return stats;
}
