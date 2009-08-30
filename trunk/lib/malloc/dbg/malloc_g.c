/*
 * $QNXtpLicenseC:
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





/*
 * (c) Copyright 1990, 1991 Conor P. Cahill (uunet!virtech!cpcahil).
 * You may copy, distribute, and use this software as long as this
 * copyright statement is not removed.
 */
#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <dlfcn.h>
#include <sys/neutrino.h>
#include <sys/storage.h>
#include <sys/syspage.h>
#include <errno.h>
#include <sys/wait.h>

#include "malloc-lib.h"
#include "tostring.h"
#include "malloc_cache.h"

unsigned int *__cdbt[CALLERPCDEPTH];


/*
 * Statistics gathering of bins
 */

/*
 * Initialization of the statistics structures
 * counters are associated with bins of prev_size+1 to size
 */
struct _stat_bin __stat_bins[] = {
#if 1
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{2, 0, 0},
	{4, 0, 0},
	{8, 0, 0},
	{16, 0, 0},
	{32, 0, 0},
	{64, 0, 0},
	{128, 0, 0},
	{256, 0, 0},
	{512, 0, 0},
	{1024, 0, 0},
	{2048, 0, 0},
	{4096, 0, 0},
	{ULONG_MAX, 0, 0}    /* Last bin catches all bigger allocs */
#else
	{2, 0, 0},
	{4, 0, 0},
	{6, 0, 0},
	{8, 0, 0},
	{10, 0, 0},
	{12, 0, 0},
	{14, 0, 0},
	{16, 0, 0},
	{0, 0, 0},
	{20, 0, 0},
	{24, 0, 0},
	{28, 0, 0},
	{32, 0, 0},
	{36, 0, 0},
	{40, 0, 0},
	{44, 0, 0},
	{48, 0, 0},
	{52, 0, 0},
	{56, 0, 0},
	{60, 0, 0},
	{64, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{0, 0, 0},
	{128, 0, 0},
	{256, 0, 0},
	{512, 0, 0},
	{1024, 0, 0},
	{2048, 0, 0},
	{4096, 0, 0},
	{8192, 0, 0},
	{ULONG_MAX, 0, 0}
#endif
};

#define STAT_NBINS	(sizeof(__stat_bins)/sizeof(__stat_bins[0]))

const unsigned __n_stat_bins = STAT_NBINS;

#define STAT_INDEX(size, index)  { \
	for (index = 0; index < __n_stat_bins; ++index) { \
		if (index == (__n_stat_bins - 1)) { \
			break; \
		} else if (size <= __stat_bins[index].size) { \
			break; \
		} \
	} \
}

#define __update_stats_nallocs(__size, __num) \
{\
	int __i; \
	STAT_INDEX(__size, __i); \
	__stat_bins[__i].nallocs+=(__num); \
}

#define __update_stats_nfrees(__size, __num) \
{ \
	int __i; \
	STAT_INDEX(__size, __i); \
	__stat_bins[__i].nfrees+=(__num); \
}

#if 0
// why is this needed? just causes a compiler warning anyway
#ifndef lint
static char rcs_header[] = "$Id: malloc_g.c 211673 2009-01-21 21:17:33Z elaskavaia@qnx.com $";
#endif
#endif

int		  malloc_verify_access;
int		  malloc_verify_access_level=0;
int		  malloc_eventfd = -1;
int		  malloc_errfd = 2;
int		  malloc_errno;
int		  malloc_fatal_level = M_HANDLE_ABORT;
ulong_t	  malloc_hist_id = 0;
int		  malloc_warn_level;
int		__malloc_print_nodetail=0;
int		__malloc_show_links;

static size_t __mbsize=0;
static size_t __mdsize=0;
static size_t __musize=0;
static size_t __mfsize=0;
static size_t __msfsize=0;
static size_t __masz=0;
static int __marenas=0;
static size_t fsblks=0;

pthread_mutex_t	  _mallocg_lock = PTHREAD_RMUTEX_INITIALIZER;

void * (*__malloc_hook)(const char *file, int line, size_t size) = __debug_malloc;
void * (*__calloc_hook)(const char *file, int line, size_t nelems, size_t size) = __debug_calloc;
void * (*__realloc_hook)(const char *file, int line, void *ptr, size_t size) = __debug_realloc;
void (*__free_hook)(const char *file, int line, void *ptr) = __debug_free;

#define DUMP_PTR	0
#define DUMP_NEXT	1
#define DUMP_PREV	2

#define BACKTRACE_STR "\n\twith Allocation Backtrace\n"
#define ERRBUFSIZE	1024+(CALLERPCDEPTH*16)+sizeof(BACKTRACE_STR)

#undef malloc_pc

int __malloc_bt_depth=0;
int __malloc_error_btdepth=0;

int writeFully(int fd, const char *buf, size_t n);
extern int __check_ctid();


void
(__update_stats_nfrees)(unsigned long size, unsigned long num)
{
	int i;
	STAT_INDEX(size, i);
	__stat_bins[i].nfrees += num;
}

void
(__update_stats_nallocs)(unsigned long size, unsigned long num)
{
	int i;
	STAT_INDEX(size, i);
	__stat_bins[i].nallocs += num;
}

void
set_guard (void *ptr, size_t usize)
{
	ssize_t bsize = _msize(ptr);
	Dhead *dh;
	int i;

	/* unaligned pointers cannot be walked */
	if (ptr != (void *)__TRUNC(ptr, _MALLOC_ALIGN))
		panic("unaligned ptr: set_guard");
	dh = (Dhead *)ptr - 1;
	dh->d_usize = usize;
	i = bsize - usize;

	if (i) {
		char *gp = (char *)ptr + usize;
		if (i <= 0)
			panic("d_usize");
		while (i > 0) {
			*gp++ = (char)i--;
		}
	}
}

static ssize_t _mbsize (void *ptr)
{
	Dhead *dh;

	/* unaligned pointers cannot be walked */
	if (ptr != (void *)__TRUNC(ptr, _MALLOC_ALIGN))
		panic("unaligned ptr: _msize");
	dh = (Dhead *)ptr - 1;
	if (dh->d_size < 0) {
		Block *b = (Block *)((char *)dh + dh->d_size);
		if (b->magic == BLOCK_MAGIC)
			return (b->nbpe + SB_OVERHEAD());
		panic("size!");
	}

	return DH_LEN(dh);
}

ssize_t
_msize (void *ptr)
{
	Dhead *dh;

	/* unaligned pointers cannot be walked */
	if (ptr != (void *)__TRUNC(ptr, _MALLOC_ALIGN))
		panic("unaligned ptr: _msize");
	dh = (Dhead *)ptr - 1;
	if (dh->d_size < 0) {
		Block *b = (Block *)((char *)dh + dh->d_size);
		if (b->magic == BLOCK_MAGIC)
			return (b->nbpe);
		panic("size!");
	}

	return DH_LEN(dh) - D_OVERHEAD();
}


ssize_t
_musize (void *ptr)
{
	Dhead *dh;

	/* unaligned pointers cannot be walked */
	if (ptr != (void *)__TRUNC(ptr, _MALLOC_ALIGN))
		panic("unaligned ptr: _musize");
	dh = (Dhead *)ptr - 1;
	return DH_ULEN(dh);
}

static void print_ptr(int fd, char *buf, Flink *flink, int tnum, int f_ptr)
{
	__Dbg_St   *ds;
	DebugInfo_t *dbgp;
	int j=0;
	char *data;
	if (flink == NULL)
		return;
	data = (char *)((Dhead *)flink + 1);
	if (!f_ptr) {
		dbgp = &(((Dhead *)flink)->d_debug);
		if (dbgp == NULL)
			return;
		sprintf(buf, "%10d:\t0x%08x:\t%-10u:\t%-10u\t%-10u:\t%-6d\t0x%016llx\n",
			tnum, (uint_t)data, _mbsize(data), _msize(data), _musize(data),
			dbgp->dbg ? dbgp->dbg->tid : -1,
			dbgp->dbg ? dbgp->dbg->ts  : 0);
		__mbsize += _mbsize(data);
		__mdsize += _msize(data);
		__musize += _musize(data);
		if (!__malloc_print_nodetail)
			write(fd, buf, strlen(buf));
		if (dbgp->dbg) {
			sprintf(buf, "Allocation Backtrace\n");
			if (!__malloc_print_nodetail)
				write(fd, buf, strlen(buf));
			ds = dbgp->dbg->bt;
			while (ds) {
				if (ds->line == 0)
					break;
				j++;
				sprintf(buf, "\t[0x%08x]\n", (unsigned)ds->line);
				if (!__malloc_print_nodetail)
	   			write(fd, buf, strlen(buf));
				ds = ds->next;
			}
			if (j == 0) {
				sprintf(buf, "\t[0x%08x]\n", (unsigned)dbgp->callerpc_line);
				if (!__malloc_print_nodetail)
					write(fd, buf, strlen(buf));
			}
		}
	} else {
		sprintf(buf, "%10d:\t0x%08x:\t%-10u\n",
			tnum, (uint_t)data, flink->f_size);
		__mfsize += flink->f_size;
		if (!__malloc_print_nodetail)
			write(fd, buf, strlen(buf));
	}
	return;
}

static void print_arena(int fd, char *buf, Arena *ap, int *tblks)
{
	Flink	*flink;
    //DebugInfo_t *dbgp;
	//char *data;
	if (ap == NULL)
		return;
	sprintf(buf, "ARENA %x, size %u:\n", (uint_t)ap, ap->a_size);
	__marenas++;
	__masz += ap->a_size;
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	for (flink = ap->a_malloc_chain.head;
	        flink != NULL;
	        flink = flink->f_next) {
		(*tblks)++;
		print_ptr(fd, buf, flink, *tblks, 0);
	}
	return;
}

static void print_band(int fd, char *buf, Band *band, int *sblks)
{
	Block *bp = NULL;
	DebugInfo_t *ptr;
	Flink       *flink;

	sprintf(buf, "BAND %x, small buffer size %u:\n", (uint_t)band, band->nbpe);
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	for (bp = band->alist; bp; bp = bp->next) {
		sprintf(buf, "Total Blks: %u Free Blks: %u Free Size %u:\n",
	          band->nalloc, (uint_t)bp->navail, bp->navail * band->nbpe);
		fsblks += bp->navail;
		__msfsize += bp->navail * band->nbpe;
		if (!__malloc_print_nodetail)
			write(fd, buf, strlen(buf));
		for(flink = bp->malloc_chain.head; flink; flink = flink->f_next) {
			ptr = &((Dhead *)flink)->d_debug;
			if ((ptr->flag & M_INUSE)) {
				(*sblks)++;
				print_ptr(fd, buf, flink, *sblks, 0);
			}
		}
	}
	for (bp = band->dlist; bp; bp = bp->next) {
		for(flink = bp->malloc_chain.head; flink; flink = flink->f_next) {
			ptr = &((Dhead *)flink)->d_debug;
			if ((ptr->flag & M_INUSE)) {
				(*sblks)++;
				print_ptr(fd, buf, flink, *sblks, 0);
			}
		}
	}
	return;
}

/**
 * A quick test to show BANDS/USAGE in qconn/IDE
 * BANDS:[size:total_blocks:free_blocks],[..]:pid:timestamp
 * USAGE:arena:usemem:freemem:pid:timestamp
 * BINS:[size:alloc:free],[..]:pid:timestamp
 */
void __dbgm_print_snapshots(int fd) {
	char buf[512];
	char *s = buf;
	int nb;
	uint64_t timestamp;

	timestamp = _get_timestamp();

	/* Get the bands events.  */
	{
		*s++ = 'B';
		*s++ = 'A';
		*s++ = 'N';
		*s++ = 'D';
		*s++ = 'S';
		*s++ = ':';
		for (nb = 0; nb < *__pnband; nb++) {
			unsigned int size = 0;
			unsigned int total_blocks = 0;
			unsigned int free_blocks = 0;
			Band *band;
			Block *bp = NULL;

			band = __pBands[nb];
			size = band->nbpe;
			for (bp = band->dlist; bp; bp = bp->next) {
				total_blocks += band->nalloc;
			}
			for (bp = band->alist; bp; bp = bp->next) {
				if (bp->navail) {
					free_blocks += bp->navail;
				} else {
					free_blocks += band->nalloc;
				}
				total_blocks += band->nalloc;
			}
			s += sprintf(s, "[%u:%u:%u]", size, total_blocks, free_blocks);
		}
		s += sprintf(s, ":%u:0x%lx\n", getpid(), (ulong_t)timestamp);
		write(fd, buf, strlen(buf));
	}

	/* Get the malloc usage events.  */
	{
		unsigned overheadmem =  _malloc_stats.m_overhead + _malloc_stats.m_small_overhead;
		unsigned usemem =  _malloc_stats.m_allocmem + _malloc_stats.m_small_allocmem;
		unsigned freemem = _malloc_stats.m_freemem + _malloc_stats.m_small_freemem;
		s = buf;
		*s++ = 'U';
		*s++ = 'S';
		*s++ = 'A';
		*s++ = 'G';
		*s++ = 'E';
		*s++ = ':';
		s += sprintf(s, "%u:%u:%u:%u:0x%lx\n", overheadmem, usemem, freemem, getpid(), (ulong_t)timestamp);
		write(fd, buf, strlen(buf));
	}

	/* Get the bins statistics.  */
	{
		int i = 0;
		s = buf;
		*s++ = 'B';
		*s++ = 'I';
		*s++ = 'N';
		*s++ = 'S';
		*s++ = ':';
		for (i = 0; i < __n_stat_bins; i++) {
			if (__stat_bins[i].size != 0) {
				s += sprintf(s, "[%lu:%lu:%lu]",
				__stat_bins[i].size,
				__stat_bins[i].nallocs,
				__stat_bins[i].nfrees);
			}
		}
		s += sprintf(s, ":%u:0x%lx\n", getpid(), (ulong_t)timestamp);
		write(fd, buf, strlen(buf));
	}

}

void __dbgm_print_currently_alloced_blocks(int fd)
{
	char buf[256];
    Arena	*ap;
	int lblks=0;
	int nb;
	Band *band;
	int sblks=0;
	int fblks=0;
	Flink   *flink;
	int i;
	ENTER();
	MALLOC_INIT();
	__mbsize=0;
	__musize=0;
	__mdsize=0;
	__mfsize=0;
	sprintf(buf, "-----------------------------\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "Currently allocated blocks\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "Format of Output\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "%10s:\t%10s:\t%10s:\t%10s\t%10s\t%6s\t%s\n",
               "#         ", "Ptr       ", "BSz       ",
               "DSz       ", "USz       ", "Tid   ", "TS");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "-----------------------\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "Large Blocks: In arenas\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "-----------------------\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	for (ap = __arenas.a_next; ap != &__arenas; ap = ap->a_next) {
		print_arena(fd, buf, ap, &lblks);
	}
	sprintf(buf, "-----------------------------\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "----------------------\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "Small Blocks: In Bands\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "----------------------\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	for (nb = 0; nb < *__pnband; nb++) {
		band = __pBands[nb];
		print_band(fd, buf, band, &sblks);
	}
	sprintf(buf, "-----------------------------\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "Blocks on Free List\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "Format of Output\n");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	sprintf(buf, "%10s:\t%10s:\t%10s\n", "#         ", "Ptr       ",
               "BSz       ");
	if (!__malloc_print_nodetail)
		write(fd, buf, strlen(buf));
	for (i=0; i < __flist_nbins ; i++) {
		Flink *fp_list;
		Flink *curflistptr;
		curflistptr = __malloc_getflistptr();
		fp_list = &(curflistptr[i]);
		for (flink = fp_list->f_next; flink != fp_list; flink = flink->f_next) {
			fblks++;
			print_ptr(fd, buf, flink, fblks, 1);
		}
	}
	sprintf(buf, "-----------------------------\n");
	write(fd, buf, strlen(buf));
	sprintf(buf, "Summary\n");
	write(fd, buf, strlen(buf));
	sprintf(buf, "Num Arenas: %d\n", __marenas);
	write(fd, buf, strlen(buf));
	sprintf(buf, "Arena Sz: %d\n", __masz);
	write(fd, buf, strlen(buf));
	sprintf(buf, "Allocated Large Blocks: %d\n", lblks);
	write(fd, buf, strlen(buf));
	sprintf(buf, "Allocated Small Blocks: %d\n", sblks);
	write(fd, buf, strlen(buf));
	sprintf(buf, "Free Large Blocks: %d\n", fblks);
	write(fd, buf, strlen(buf));
	sprintf(buf, "Free Small Blocks: %d\n", fsblks);
	write(fd, buf, strlen(buf));
	sprintf(buf, "Total Sz: %d\n", __mbsize);
	write(fd, buf, strlen(buf));
	sprintf(buf, "Data Sz: %d\n", __mdsize);
	write(fd, buf, strlen(buf));
	sprintf(buf, "User Sz: %d\n", __musize);
	write(fd, buf, strlen(buf));
	sprintf(buf, "Large Free Sz: %d\n", __mfsize);
	write(fd, buf, strlen(buf));
	sprintf(buf, "Small Free Sz: %d\n", __msfsize);
	write(fd, buf, strlen(buf));
	sprintf(buf, "-----------------------------\n");
	write(fd, buf, strlen(buf));
	sprintf(buf, "Statistics\n");
	write(fd, buf, strlen(buf));
	sprintf(buf, "%13s\t%10s\t%10s\n", "Sz           ",
               "Allocs    ", "Frees     ");
	write(fd, buf, strlen(buf));
	for (i=0; i < __n_stat_bins; i++) {
		if (__stat_bins[i].size != 0) {
			sprintf(buf, " < %-10lu\t%-10lu\t%-10lu\n",
				__stat_bins[i].size,
				__stat_bins[i].nallocs,
				__stat_bins[i].nfrees);
			write(fd, buf, strlen(buf));
		}
	}
	LEAVE();
	return;
}

int
malloc_dump_unreferenced(int fd, int todo)
{
	int line;
	if (fd < 0)
		return(-1);
	line = libmalloc_caller();
	return( DBmalloc_dump_unreferenced( fd, (char *)NULL, line, todo) );
}

/*
 * Function:	malloc()
 *
 * Purpose:	low-level interface to the debug malloc lib.  This should only
 * 		be called from code that is not recompilable.  The recompilable
 *		code should include malloc.h and therefore its calls to malloc
 *		will be #defined to be calls to debug_malloc with the
 *		appropriate arguments.
 *
 * Arguments:	size	- size of data area needed
 *
 * Returns:	whatever debug_malloc returns.
 *
 * Narrative:
 *
 */
void *
malloc(size_t size)
{
	int line = libmalloc_caller();
	return(debug_malloc(NULL,line,size));
}

void *
__bounds_malloc(size_t size)
{
	int line = libmalloc_caller();
	return( debug_malloc(NULL,line,size) );
}

void *
malloc_pc(size_t size, unsigned int *pc)
{
	return( debug_malloc(NULL,(int)pc,size) );
}

#ifndef NONEAR
void *
_nmalloc(size_t size)
{
	int line = libmalloc_caller();
	return debug_malloc(NULL, line, size);
}
#endif

static void *
debug_allocator(const char *func, const char *file, int line, size_t size, long *counterp, void *calldata, void * (*allocator)(void *, size_t, void *))
{
	//static long		  call_counter = 0;
	long		  call_counter;
	//const char		* func = "malloc";
	void			* cptr;
	Flink			* flink;
	DebugInfo_t		* ptr;
	Dhead *dh;

	ENTER();
	MALLOC_INIT();

	/*
	 * increment the counter for the number of calls to this func.
	 */
	//call_counter++;
	call_counter = (*counterp)++;

	/*
	 * If malloc chain checking is on, go do it.
	 */
	if( _malloc_check_on ) {
		if( DBFmalloc_chain_check(func,file,line,1) != 0 ) {
			LEAVE();
			return(NULL);
		}
	}

	/*
	 * save the original requested size;
	 */

	if ((cptr = (*allocator)(calldata, size, (void *)line)) == NULL) {
		LEAVE();
		return NULL;
	}
	if ((ulong_t)cptr > _malloc_end) {
		_malloc_end = (ulong_t)cptr + size;
	}

	/* this should be fine, since the allocator returns
	   values that are aligned */
	flink = (Flink *)(((Dhead *)cptr) - 1);
	ptr = &(((Dhead *)flink)->d_debug);

	/*
	 * Now ptr points to a memory location that can store
	 * this data, so lets go to work.
	 */

	ptr->flag = M_MAGIC;
	ptr->flag |= M_INUSE;
	ptr->magic2 = M_MAGIC2;

	/*
	 * store the identification information
	 */
	ptr->file    = file;
	ptr->id      = call_counter;
	ptr->hist_id = ++malloc_hist_id;
	ptr->dbg = NULL;
	MALLOC_PTR_FILL_DEBUG(ptr->dbg, cptr);

	SETTYPE(ptr,M_T_MALLOC);

	if (_malloc_fill_area && _malloc_fill_area < ptr->hist_id) {
		set_guard(cptr,size);
	} else {
		((Dhead *)cptr-1)->d_usize = size;
	}

	dh = (Dhead *)cptr-1;

	/*
	 * Store the pointer in the malloc chain for the arena (& range)
	 */
	{
		arena_range_t range;

//		if (find_malloc_area(cptr, &range)) {
		if (find_malloc_range(cptr, &range)) {
			chain_t		*chain = (range.r_type == RANGE_BLOCK)
								? &range.un.r_block->malloc_chain
								: &range.un.r_arena->a_malloc_chain;
			Flink		*lp = chain->head;
			Flink		*next;
			Flink		*prev;

			/* make sure Block isn't on free list */
			//if (range.r_type == RANGE_BLOCK) {
			/* do something here to check Arena free list */
			//}

#if 0
			/* insert buffer into malloc chain */
			for (prev = NULL; lp; prev = lp, lp = lp->f_next) {
				if ((ulong_t)lp > (ulong_t)flink) {
					break;
				}
			}
			next = lp;
#endif
			prev = NULL;
			next = chain->head;
			flink->f_prev = prev;
			flink->f_next = next;

			if (prev) {
				prev->f_next = flink;
				((Dhead *)prev)->d_crc = LinkCRC(prev);
			} else {
				chain->head = flink;
			}
			if (lp) {
				/* assert(DH_ISBUSY(next)); */
				next->f_prev = flink;
				((Dhead *)next)->d_crc = LinkCRC(next);
			} else {
				chain->tail = flink;
			}
		}
	}

	((Dhead *)flink)->d_crc = LinkCRC(flink);
	if (!__check_ctid()) {
		__update_stats_nallocs(size, 1);
	}

	mc_cache_put_malloc(cptr);
	LEAVE();
	/*
	 * return the pointer to the data area for the user.
	 */
	return (dh+1);
	//cptr = NULL;
	//return (cptr);


} /* debug_allocator(... */

/*
 * Function:	debug_malloc()
 *
 * Purpose:	the real memory allocator
 *
 * Arguments:	size	- size of data area needed
 *
 * Returns:	pointer to allocated area, or NULL if unable
 *		to allocate addtional data.
 *
 * Narrative:
 *
 */

static void *
malloc_allocator(void *calldata, size_t size, void *pc)
{
	return _malloc_pc(size, pc);
}

void *
__debug_malloc(const char *file, int line, size_t size)
{
	static long		  call_counter = 0;
	const char		* func = "malloc";
	return debug_allocator(func, file, line, size, &call_counter, NULL, malloc_allocator);
} /* __debug_malloc(... */

void *
debug_malloc(const char *file, int line, size_t size)
{
	void *cptr;
	Dhead *dh;
	ENTER();
	MALLOC_INIT();

	MALLOC_GETBT(__cdbt);

	if (__malloc_hook != NULL) {
		cptr = (*__malloc_hook)(file,line,size);
	} else {
		cptr = __debug_malloc(file,line,size);
		MALLOC_FILLCALLERD(cptr, __cdbt, line);
	}
	LEAVE();
	dh = (Dhead *)cptr-1;
	return(dh+1);
//cptr = NULL;
//return(cptr);
}

/*
 * Function:	debug_memalign()
 *
 * Purpose:	the real aligned memory allocator
 *
 * Arguments:	alignment	- data alignment
 * 		size	- size of data area needed
 *
 * Returns:	pointer to allocated area, or NULL if unable
 *		to allocate addtional data.
 *
 * Narrative:
 *
 */
static void *
memalign_allocator(void *calldata, size_t size, void *pc)
{
	return _memalign_pc(*(int *)calldata, size, pc);
}

void *
debug_memalign(const char *file, int line, size_t alignment, size_t size)
{
	static long		  call_counter = 0;
	const char		* func = "memalign";
	return debug_allocator(func, file, line, size, &call_counter, (void *)&alignment, memalign_allocator);
}

void *
memalign(size_t alignment, size_t size)
{
	int line = libmalloc_caller();
	return( debug_memalign(NULL,line,alignment,size) );
}


void *
memalign_pc(size_t alignment, size_t size, unsigned int *pc)
{
	return( debug_memalign(NULL,(int)pc,alignment,size) );
}

int
posix_memalign(void **ptr, size_t alignment, size_t size)
{
	ulong_t bit = 0x1;
	void *p;
	int pc;
	pc = libmalloc_caller();

	while (bit && (alignment&bit) == 0) bit <<= 1;

	/* if not a power of two or a multiple of word size */
	if ((alignment&~bit) != 0 || __ROUND(alignment,sizeof(int)) != alignment) {
		return EINVAL;
	}
	p = memalign_pc(alignment, size, (unsigned int *)pc);
	if (p != NULL) {
		*ptr = p;
	} else {
		return ENOMEM;
	}
	return EOK;
}

/*
 * The following mess is just to ensure that the versions of these functions in
 * the current library are included (to make sure that we don't accidentaly get
 * the libc versions. (This is the lazy man's -u ld directive)
 */

void	(*malloc_void_funcs[])() =
{
	free,
};

int		(*malloc_int_funcs[])() =
{
	strcmp,
	memcmp,
};

void	* (*malloc_char_star_funcs[])() =
{
	debug_realloc,
	debug_calloc,
};

/*
 * This is malloc's own memset which is used without checking the parameters.
 *
 * NOTE: in order to increase performance this function takes advantage of
 *	 the fact that it is always called with an aligned pointer AND the
 *	 length is always a multiple of sizeof(long)
 */
void
malloc_aligned_memset(void *ptr, int b, register int len)
{
	char		  byte = b;
	register long	  fill_int;
	int		  i;
	register long	* myptr;

	/*
	 * fill in the fill integer
	 */
	fill_int = 0;
	for (i=0; i < sizeof(long); i++) {
		fill_int |= (byte << (i << 3));
	}

	/*
	 * get pointer to begining of data
	 */
	myptr = (long *) ptr;

	/*
	 * adjust byte lengt to longword length
	 */
	len /= sizeof(long);

	/*
	 * now fill in the data
	 */
	while (len-- > 0) {
		*myptr++ = fill_int;
	}

} /* malloc_aligned_memset(... */

/*
 * malloc_safe_memset() - works on any alignment, but is much slower than
 *			  malloc_aligned_memset();
 */
void
malloc_safe_memset(void *ptr, int b, register int len)
{
	char		  byte = b;
	register char	* myptr;

	myptr = (char *) ptr;

	while (len-- > 0) {
		*myptr++ = byte;
	}
}


/*
 * We want to send out a more "program parsable" type of event here.
 * define a location as: {filename,line}|{address}
 * EVENT:{what}:{location}:{ptraddress}:{allocation_type}:{allocation_size}:{allocation_number}:{allocation_location}:{allocation_state}:diag
 */
void
_malloc_event(const char *event, const char *funcname, const char *file,
              int line, const void *link, int type)
{
	char  errbuf[ERRBUFSIZE];
	char  buf[ERRBUFSIZE];
	void  exit();
	char	* s;
	char	* t;
	DebugInfo_t	* mptr;
	int len;
	ulong_t timestamp;
	void *array[__malloc_error_btdepth];
	int size;

	if(malloc_eventfd == -1) {
		return;
	}

	timestamp = _get_timestamp();

	/* Get the backtrace up front, because as of now, it looks like
	 * btl_get_backtrace has difficulty getting out of
	 * _malloc_event()... */
	if (__malloc_error_btdepth > 0 && type != M_LEAK)
		size = backtrace(array,__malloc_error_btdepth,line);


	//Indicate that this particular event reporting is beginning
	{
		const char *end = STARTEV_STR;

		s = errbuf;
		COPY(s,end,errbuf,ERRBUFSIZE);
		*s++ = ' ';
		s += tostring(s,(ulong_t)getpid(),10,10,' ');
		*s++ = ' ';
		s += tostring(s,(ulong_t)pthread_self(),5,10,' ');
		*s++ = '\n';
		*s = '\0';
		writeFully(malloc_eventfd,errbuf,(unsigned)(s-errbuf));
	}

	/* {EVENT} */
	s = errbuf;
	t = (char *) event;
	COPY(s,t,errbuf,ERRBUFSIZE);

	*s++ = ':';
	if (type != M_LEAK) {
		/* {what} */
		t = (char *) funcname;
		COPY(s,t,errbuf,ERRBUFSIZE);

		/* {location} */
		t = ":";
		COPY(s,t,errbuf,ERRBUFSIZE);
	} else {
		*s++ = ':';
	}

	if( file != NULL ) {
		t = (char *) file;
		COPY(s,t,errbuf,ERRBUFSIZE);

		*s++ = ',';

		s += tostring(s,(ulong_t)line,0,B_DEC,'0');
	} else {
		s += tostring(s,(ulong_t)line,0,B_HEX,'0');
	}
	t = ":";
	COPY(s,t,errbuf,ERRBUFSIZE);
#if 0
	} else {
		*s++ = ':';
		*s++ = ':';
	}
#endif

	//Go through and put the link information in
	/* {ptr}:{type}:{size}:{nth}: */
	mptr = (link != NULL) ? &((Dhead *)link)->d_debug : NULL;

	/*
	 * If the debug info does not have a valid magic number, skip it
	 * because we probably have gotten clobbered.
	 */
	if (mc_cache_pointer_in_heap(mptr) && MAGIC_MATCH(mptr)) {
		//@@@ Put the pointer address here!.
		//if (type != M_LEAK)
		s += tostring(s,(ulong_t)((Dhead *)link+1),0,B_HEX,'0');
		*s++ = ':';

		/*
		 * Get the function name
	 	 */
		switch(GETTYPE(mptr)) {
			case M_T_MALLOC:
				t = "m:";
				break;
			case M_T_REALLOC:
				t = "r:";
				break;
			case M_T_CALLOC:
				t = "c:";
				break;
			case M_T_SPLIT:
				t = "i:";
				break;
			default:
				t = "?:";
				break;
		}
		COPY(s,t,errbuf,ERRBUFSIZE);

		/*
		 * Get the number of bytes and the number of this call
		 */
		s += tostring(s,(ulong_t)DH_ULEN(link),0,10,'0');
		*s++ = ':';
		s += tostring(s,(ulong_t)mptr->id,0,10,'0');
		*s++ = ':';

		if (mc_cache_pointer_in_heap(mptr->dbg)) {
			s += tostring(s,(ulong_t)mptr->dbg->cpu,2,B_DEC,'0');
			*s++ = ':';
			s += tostring(s,(ulong_t)(getpid()),10,B_DEC,'0');
			*s++ = ':';
			s += tostring(s,(ulong_t)mptr->dbg->tid,5,B_DEC,'0');
			*s++ = ':';
		} else {
			s += tostring(s,0,2,B_DEC,'0');
			*s++ = ':';
			s += tostring(s,getpid(),10,B_DEC,'0');
			*s++ = ':';
			s += tostring(s,0,5,B_DEC,'0');
			*s++ = ':';
		}
//		s += tostring64(s,(uint64_t)mptr->dbg->ts,18,B_HEX,'0');
		s += tostring64(s,(uint64_t)timestamp,18,B_HEX,'0');
		*s++ = ':';

		/*
		 * Put the file info or the address info in.
	 	 */
		if (mc_cache_pointer_in_heap(mptr->file) && (mptr->file[0] != '\0')) {
			t = (char *) mptr->file;
			COPY(s,t,errbuf,ERRBUFSIZE);

			t = ",";
			COPY(s,t,errbuf,ERRBUFSIZE);

			s += tostring(s,(ulong_t)mptr->callerpc_line,0,10,'0');
		} else {
			//s += tostring(s,(ulong_t)mptr->callerpc_line,0,B_HEX,'0');
			buf[0] = '@';
			buf[1] = ' ';
			buf[2] = '[';
			len = 3;
			if (mc_cache_pointer_in_heap(mptr)) {
				__Dbg_Data *dd=NULL;
				__Dbg_St   *ds;
				dd = mptr->dbg;
				if (mc_cache_pointer_in_heap(dd)) {
					int j=0;
					ds = dd->bt;
					while (ds) {
						unsigned *cline;
						cline = ds->line;
						if (cline == 0)
							break;
						if (j > 0) {
							buf[len++] = ':';
						}
						tostring(buf+len,(ulong_t)(cline),10,B_HEX,'0');
						len+=10;
						j++;
						ds = ds->next;
						if (ds && (ds->line == dd->bt->line))
							break;
					}
					if (j == 0) {
						(void) tostring(&buf[len], (ulong_t)(line),10,B_HEX,'0');
						len += 10;
					}
				}
			} else {
				(void) tostring(&buf[len], (ulong_t)(line),10,B_HEX,'0');
				len += 10;
			}
			buf[len++] = ']';
			buf[len++] = '\0';
			t = buf;
			COPY(s,t,errbuf,ERRBUFSIZE);
			//s += len;
		}

		if (type != M_LEAK) {
			/*
		 	* If the block is not currently in use
		 	*/
			t = (mptr->flag & M_INUSE) ? ":u:" : ":f:" ;
			COPY(s,t,errbuf,ERRBUFSIZE);
		} else {
			t = "::";
			COPY(s,t,errbuf,ERRBUFSIZE);
		}
	} else {
		//t = "0x0:?:0:0:0x0:?:?:";
		//COPY(s,t,errbuf,ERRBUFSIZE);
    	//s += tostring64(s,(uint64_t)timestamp,18,B_HEX,'0');
		//*s++ = ':';
		t = "0x0:?:0:0:";
		COPY(s,t,errbuf,ERRBUFSIZE);
		s += tostring(s,(ulong_t)GET_CPU(),2,B_DEC,'0');
		*s++ = ':';
		s += tostring(s,(ulong_t)(getpid()),10,B_DEC,'0');
		*s++ = ':';
		s += tostring(s,(ulong_t)(pthread_self()),5,B_DEC,'0');
		*s++ = ':';
    	s += tostring64(s,(uint64_t)timestamp,18,B_HEX,'0');
		*s++ = ':';
	}

	if (type != M_LEAK) {
		//Put the appropriate error string up there if there is one.
		t = malloc_err_strings[malloc_errno];
		COPY(s,t,errbuf,ERRBUFSIZE);
	}

	*s++ = '\n';
	*s = '\0';

	if (writeFully(malloc_eventfd,errbuf,(unsigned)(s-errbuf)) != (s-errbuf)) {
		(void) writeFully(2,"I/O error to error file\n",(unsigned)24);
		exit(110);
	}

	if (type != M_LEAK) {
		//Dump out the strack trace as much as we have anyway
		const char *prefix = "BT:";
		if (__malloc_error_btdepth > 0) {
//			void *array[__malloc_error_btdepth];
//			int size, i;
//			size = backtrace(array, __malloc_error_btdepth, line);
			int i;
			for (i = 0; i < size; ++i) {
				malloc_dump_line(malloc_eventfd, NULL, (unsigned long)array[i], prefix);
			}
		}
	}

	//Indicate that this particular event reporting is over
	{
		const char *end = ENDEV_STR;

		s = errbuf;
		COPY(s,end,errbuf,ERRBUFSIZE);
		*s++ = ' ';
		s += tostring(s,(ulong_t)getpid(),10,10,' ');
		*s++ = ' ';
		s += tostring(s,(ulong_t)pthread_self(),5,10,' ');
		*s++ = '\n';
		*s = '\0';
		writeFully(malloc_eventfd,errbuf,(unsigned)(s-errbuf));
	}
}


/*
 * Function:	malloc_fatal()
 *
 * Purpose:	to display fatal error message and take approrpriate action
 *
 * Arguments:	funcname - name of function calling this routine
 *		mptr	 - pointer to malloc block associated with error
 *
 * Returns:	nothing of any value
 *
 * Narrative:
 *
 * Notes:	This routine does not make use of any libc functions to build
 *		and/or disply the error message.  This is due to the fact that
 *		we are probably at a point where malloc is having a real problem
 *		and we don't want to call any function that may use malloc.
 */
void
malloc_fatal(const char *funcname, const char *file, int line, const void *link)
{
	char		  errbuf[ERRBUFSIZE];
	void	  exit();
	void	  malloc_err_handler();
	char		* s;
	char		* t;
    int bad_pointer = (malloc_errno == M_CODE_BAD_PTR) ||
	    (malloc_errno == M_CODE_PTR_INSIDE) ||
	     (malloc_errno == M_CODE_SIGNAL);
	if (bad_pointer) {
		_malloc_event("FATAL", funcname, file, line, NULL, M_FATAL);
	} else {
		_malloc_event("FATAL", funcname, file, line, link, M_FATAL);
	}

	s = errbuf;
	t = "MALLOC Fatal error from ";
	COPY(s,t,errbuf,ERRBUFSIZE);

	t = (char *) funcname;
	COPY(s,t,errbuf,ERRBUFSIZE);

	t = "()";
	COPY(s,t,errbuf,ERRBUFSIZE);

	/*
	 * if we have a file and line number, show it
 	 */
	if (file != NULL) {
		t = " (called from ";
		COPY(s,t,errbuf,ERRBUFSIZE);

		t = (char *) file;
		COPY(s,t,errbuf,ERRBUFSIZE);

		t = " line ";
		COPY(s,t,errbuf,ERRBUFSIZE);

		s += tostring(s,(ulong_t)line,10,10,' ');

		*s++ = ')';

	} else {
		t = " (called from instruction preceding ";
		COPY(s,t,errbuf,ERRBUFSIZE);

		s += tostring(s,(ulong_t)line,10,B_HEX,'0');

		*s++ = ')';
	}

	*s++ = ':';
	*s++ = '\n';

	t = malloc_err_strings[malloc_errno];
	COPY(s,t,errbuf,ERRBUFSIZE);

	*(s++) = '\n';

	if (writeFully(malloc_errfd,errbuf,(unsigned)(s-errbuf)) != (s-errbuf)) {
		(void) writeFully(2,"I/O error to error file\n",(unsigned)24);
		exit(110);
	}

	/*
	 * if this error was associated with an identified malloc block,
	 * dump the malloc info for the block.
	 */
	if (link && !bad_pointer)  {
		malloc_dump_info_block(link, DUMP_PTR);
	}
	if (malloc_fatal_level & M_HANDLE_TRACEBACK) {
		if (__malloc_error_btdepth > 0) {
			void *array[__malloc_error_btdepth];
			int size, i;
			size = backtrace(array, __malloc_error_btdepth, line);
			for (i = 0; i < size; ++i) {
				malloc_dump_line(malloc_errfd, NULL, (unsigned long)array[i], NULL);
			}
		}
	}

	malloc_err_handler(malloc_fatal_level &~ M_HANDLE_TRACEBACK );

} /* malloc_fatal(... */

/*
 * Function:	malloc_warning()
 *
 * Purpose:	to display warning error message and take approrpriate action
 *
 * Arguments:	funcname - name of function calling this routine
 *		mptr	 - pointer to malloc block associated with error
 *
 * Returns:	nothing of any value
 *
 * Narrative:
 *
 * Notes:	This routine does not make use of any libc functions to build
 *		and/or disply the error message.  This is due to the fact that
 *		we are probably at a point where malloc is having a real problem
 *		and we don't want to call any function that may use malloc.
 */
void
malloc_warning(const char *funcname, const char *file, int line, const void *link)
{
	char		  errbuf[ERRBUFSIZE];
	void	  exit();
	char		* s;
	char		* t;

	if ((malloc_errno == M_CODE_BAD_PTR) ||
	    (malloc_errno == M_CODE_PTR_INSIDE)) {
		_malloc_event("ERROR", funcname, file, line, NULL, M_ERROR);
	} else {
		_malloc_event("ERROR", funcname, file, line, link, M_ERROR);
	}

	s = errbuf;
	t = "MALLOC Error from ";
	COPY(s,t,errbuf,ERRBUFSIZE);

	t = (char *) funcname;
	COPY(s,t,errbuf,ERRBUFSIZE);

	t = "()";
	COPY(s,t,errbuf,ERRBUFSIZE);

	/*
	 * if we have a file and line number, show it
 	 */
	if (file != NULL) {
		t = " (called from ";
		COPY(s,t,errbuf,ERRBUFSIZE);

		t = (char *) file;
		COPY(s,t,errbuf,ERRBUFSIZE);

		t = " line ";
		COPY(s,t,errbuf,ERRBUFSIZE);

		s += tostring(s,(ulong_t)line,10,B_DEC,' ');

		*s++ = ')';

	} else {
		t = " (called from instruction preceding ";
		COPY(s,t,errbuf,ERRBUFSIZE);

		s += tostring(s,(ulong_t)line,10,B_HEX,'0');

		t = " pointer value ";
		COPY(s,t,errbuf,ERRBUFSIZE);

		s += tostring(s,(ulong_t)link,10,B_HEX,'0');

		*s++ = ')';
	}

	*s++ = ':';
	*s++ = '\n';

	t = malloc_err_strings[malloc_errno];
	if (malloc_errno == M_CODE_BAD_PTR) {
		link = NULL;
	}
	COPY(s,t,errbuf,ERRBUFSIZE);

	*(s++) = '\n';

	if( writeFully(malloc_errfd,errbuf,(unsigned)(s-errbuf)) != (s-errbuf)) {
		(void) writeFully(2,"I/O error to error file\n",(unsigned)24);
		exit(110);
	}

	/*
	 * if this error was associated with an identified malloc block,
	 * dump the malloc info for the block.
	 */
	if( link ) {
		malloc_dump_info_block(link,DUMP_PTR);
	}

	//Dump out the strack trace as much as we have anyway
	{
		const char *prefix = "BT:";
		if (__malloc_error_btdepth > 0) {
			void *array[__malloc_error_btdepth];
			int size, i;
			size = backtrace(array, __malloc_error_btdepth, line);
			for (i = 0; i < size; ++i) {
				malloc_dump_line(malloc_errfd, NULL, (unsigned long)array[i], prefix);
			}
		}
	}
	malloc_err_handler(malloc_warn_level);

} /* malloc_warning(... */

/*
 * Function:	malloc_dump_info_block()
 *
 * Purpose:	to display identifying information on an offending malloc
 *		block to help point the user in the right direction
 *
 * Arguments:	mptr - pointer to malloc block
 *
 * Returns:	nothing of any value
 *
 * Narrative:
 *
 * Notes:	This routine does not make use of any libc functions to build
 *		and/or disply the error message.  This is due to the fact that
 *		we are probably at a point where malloc is having a real problem
 *		and we don't want to call any function that may use malloc.
 */
void
malloc_dump_info_block(const void *vlink, int id)
{
	char		  errbuf[ERRBUFSIZE];
	void	  exit();
	char		* funcname;
	char		* s;
	char		* t;
	DebugInfo_t	* mptr = NULL;

	/* we could be called with vlink == NULL so dereference carefully */
	if (vlink != NULL) {
		mptr = &((Dhead *)vlink)->d_debug;
		/*
	 	* if the debug info does not have a valid magic number, skip it
	 	* because we probably have gotten clobbered.
	 	*/
		if (!MAGIC_MATCH(mptr) || !mc_cache_pointer_in_heap(mptr)) {
			return;
		}
	}

	s = errbuf;

	if (id == DUMP_PTR) {
		t = "This error is *probably* associated with the following";
		COPY(s,t,errbuf,ERRBUFSIZE);

		t = " allocation:\n\n\tA call to ";
	} else if (id == DUMP_PREV) {
		if (vlink == NULL) {
			t = "\tThere is no malloc chain element prior to the";
			COPY(s,t,errbuf,ERRBUFSIZE);

			t = " suspect\n\t element identified above";
		} else {
			t = "\tThe malloc chain element prior to the suspect";
			COPY(s,t,errbuf,ERRBUFSIZE);

			t = " allocation is from:\n\n\tA call to ";
		}
	} else {
		if (vlink == NULL) {
			t = "\tThere is no malloc chain element after the";
			COPY(s,t,errbuf,ERRBUFSIZE);

			t = " suspect\n\t element identified above";
		} else {
			t ="\tThe malloc chain element following the suspect";
			COPY(s,t,errbuf,ERRBUFSIZE);

			t = " allocation is from:\n\n\tA call to ";
		}
	}
	COPY(s,t,errbuf,ERRBUFSIZE);

	if (vlink != NULL) {

		/*
		 * get the function name
	 	 */
		switch(GETTYPE(mptr)) {
			case M_T_MALLOC:
				t = "malloc";
				break;
			case M_T_REALLOC:
				t = "realloc";
				break;
			case M_T_CALLOC:
				t = "calloc";
				break;
			case M_T_SPLIT:
				t = "internal malloc func";
				break;
			default:
				t = "Unknown";
				break;
		}
		funcname = t;
		COPY(s,t,errbuf,ERRBUFSIZE);

		t = " for ";
		COPY(s,t,errbuf,ERRBUFSIZE);

		s += tostring(s,(ulong_t)DH_ULEN(vlink),10,10,' ');

		t = " bytes";
		if (mc_cache_pointer_in_heap(mptr->dbg)) {
			COPY(s,t,errbuf,ERRBUFSIZE);
			t = "\n\ton cpu:";
			COPY(s,t,errbuf,ERRBUFSIZE);
			s += tostring(s,(ulong_t)mptr->dbg->cpu,2,B_DEC,'0');
			t = "\n\tby pid:";
			COPY(s,t,errbuf,ERRBUFSIZE);
			s += tostring(s,(ulong_t)(getpid()),10,B_DEC,'0');
			t = "\n\t   tid:";
			COPY(s,t,errbuf,ERRBUFSIZE);
			s += tostring(s,(ulong_t)mptr->dbg->tid,5,B_DEC,'0');
			t = "\n\t   at:";
			COPY(s,t,errbuf,ERRBUFSIZE);
			s += tostring64(s,(uint64_t)mptr->dbg->ts,18,B_HEX,'0');
			t = "\n\tin ";
			COPY(s,t,errbuf,ERRBUFSIZE);
		}

		/*
		 * if we don't have file info
	 	 */
		if (!mc_cache_pointer_in_heap(mptr->file) || (mptr->file[0] == '\0')) {
			t = "an unknown file";
			COPY(s,t,errbuf,ERRBUFSIZE);
			if ((ulong_t)mptr->callerpc_line != (ulong_t)-1) {
				unsigned *line;
				__Dbg_Data *dd;
				t = " near:";
				COPY(s,t,errbuf,ERRBUFSIZE);

				s += tostring(s,(ulong_t)mptr->callerpc_line,10,16,' ');
				dd = mptr->dbg;
				if (mc_cache_pointer_in_heap(dd)) {
					int j = 0;
					char q[CALLERPCDEPTH*18];
					char *r;
					__Dbg_St *ds;
					r = q;
					t = BACKTRACE_STR;
					COPY(s,t,errbuf,ERRBUFSIZE);
					ds = dd->bt;
					while (ds) {
						line = ds->line;
						if (line == 0)
							break;
						*r++ = 'B';
						*r++ = 'T';
						*r++ = ':';
						*r++ = '@';
						*r++ = ' ';
						*r++ = '[';
						tostring(r,(ulong_t)line,10,B_HEX,'0');
						r += 10;
						*r++ = ']';
						*r++ = '\n';
						j++;
						ds = ds->next;
						if (ds && (ds->line == dd->bt->line))
							break;
					}
					if (j) {
						*r = '\0';
						r = q;
						COPY(s,r,errbuf,ERRBUFSIZE);
					}
				}
			}
		} else {
			t = (char *) mptr->file;
			COPY(s,t,errbuf,ERRBUFSIZE);

			t = " on line ";
			COPY(s,t,errbuf,ERRBUFSIZE);

			s += tostring(s,(ulong_t)mptr->callerpc_line,10,10,' ');
		}

		t = "\n\tThis was the ";
		COPY(s,t,errbuf,ERRBUFSIZE);

		s += tostring(s,(ulong_t)mptr->id,0,10,' ');

		t = malloc_int_suffix(mptr->id);
		COPY(s,t,errbuf,ERRBUFSIZE);

		t = " call to ";
		COPY(s,t,errbuf,ERRBUFSIZE);

		t = funcname;
		COPY(s,t,errbuf,ERRBUFSIZE);

		/*
		 * if the block is not currently in use
		 */
		if ((mptr->flag & M_INUSE) == 0) {
			t = ". This block has been freed";
			COPY(s,t,errbuf,ERRBUFSIZE);
		}

	}

	t = ".\n\n";
	COPY(s,t,errbuf,ERRBUFSIZE);

	if (writeFully(malloc_errfd,errbuf,(unsigned)(s-errbuf)) != (s-errbuf)) {
		(void) writeFully(2,"I/O error to error file\n",(unsigned)24);
		exit(110);
	}


	/*
	 * if this is the primary suspect and we are showing links
	 */
	if ((__malloc_show_links != 0 ) && (id == DUMP_PTR ) ) {
		/*
		 * show the previous and next malloc regions.
		 */
		malloc_dump_info_block(((Flink *)vlink)->f_prev,DUMP_PREV);
		malloc_dump_info_block(((Flink *)vlink)->f_next,DUMP_NEXT);
	}

} /* malloc_dump_info_block(... */


void
malloc_dump_line(int fd, const char *file, unsigned long line, const char *prefix)
{
	char buf[200];
	char *s = &buf[0];
	const char *t;

	if (prefix != NULL) {
		COPY(s,prefix,buf,sizeof buf);
	}

	if (file != NULL) {
		t = "@ ";
		COPY(s,t,buf,sizeof buf);
		t = file;
		COPY(s,t,buf,sizeof buf);
		*s++ = ':';
		(void) tostring(s, (ulong_t)(line),10,B_DEC,' ');
		s += 10;
		*s++ = '\n';
		*s = '\0';
		writeFully(fd,buf,s-buf);
		buf[0] = '\0';
	} else if (__malloc_use_dladdr > 0) { /* Instruction pointer */
		Dl_info dli;

		t = "@ ";
		COPY(s,t,buf,sizeof buf);

		if (dladdr((void *)line, &dli)) {
			const char *fn = dli.dli_fname ? dli.dli_fname : "";
			const char *sn = dli.dli_sname ? dli.dli_sname : "";

			*s++ = '(';
			if (fn != NULL && fn[0]) {
				t = fn;
				COPY(s,t,buf,sizeof buf);
			}

			*s++ = ':';

			// do not put the base address if this is not a dll.
			if (fn != NULL && fn[0]) {
				int delta;
				delta = (int)dli.dli_fbase;
				(void) tostring(s,delta,10,B_HEX,'0');
				s += 10;
			}

			*s++ = ':';

			if (sn != NULL && sn[0] && strcmp("_btext", sn) != 0) {
				t = sn;
				COPY(s, t, buf, sizeof buf);
			}

			*s++ = ')';
		}
		*s++ = '[';
		(void) tostring(s, (ulong_t)(line),10,B_HEX,'0');
		s += 10;
		*s++ = ']';
	} else {
		t = "@ [";
		COPY(s,t,buf,sizeof buf);
		(void) tostring(s, (ulong_t)(line),10,B_HEX,'0');
		s += 10;
		*s++ = ']';
	}
	*s++ = ' ';
	(void) tostring(s, (ulong_t)(getpid()),10,B_DEC,' ');
	s += 10;
	*s++ = ' ';
	(void) tostring(s, (ulong_t)(pthread_self()),5,B_DEC,' ');
	s += 5;
	*s++ = '\n';
	*s = '\0';
	writeFully(fd,buf,s-buf);
	buf[0] = '\0';
}

/*
 * Function:	malloc_err_handler()
 *
 * Purpose:	to take the appropriate action for warning and/or fatal
 *		error conditions.
 *
 * Arguments:	level - error handling level
 *
 * Returns:	nothing of any value
 *
 * Narrative:
 *
 * Notes:	This routine does not make use of any libc functions to build
 *		and/or disply the error message.  This is due to the fact that
 *		we are probably at a point where malloc is having a real problem
 *		and we don't want to call any function that may use malloc.
 */
void
malloc_err_handler(int level)
{
	void	  exit();
	void muntrace();
	enum mcheck_status status;

	if (level & M_HANDLE_TRACEBACK) {
		if (__malloc_error_btdepth > 0) {
			void *array[__malloc_error_btdepth];
			int size, i;
			size = backtrace(array, __malloc_error_btdepth, 0);
			for (i = 0; i < size; ++i) {
				malloc_dump_line(malloc_errfd, NULL, (unsigned long)array[i], NULL);
			}
		}
	}

	if (level & M_HANDLE_DUMP) {
		malloc_dump(malloc_errfd);
	}

	switch (level & ~(M_HANDLE_DUMP|M_HANDLE_TRACEBACK)) {

		/*
		 * Raise the SIGSTOP signal
		 */
		case M_HANDLE_STOP:
			kill(getpid(), SIGSTOP);
		break;

		/*
		 * If we are to drop a core file and exit
		 */
		case M_HANDLE_ABORT:
			status = malloc_errno == M_CODE_NOT_INUSE
				? MCHECK_FREE
				: (malloc_errno == M_CODE_OVERRUN ||
				 malloc_errno == M_CODE_OUTOF_BOUNDS
					? MCHECK_TAIL : MCHECK_HEAD);

			muntrace();
			malloc_abort(status);
			abort(); /* just in case */
		break;

		/*
		 * If we are to exit..
		 */
		case M_HANDLE_EXIT:
			exit(200);
		break;

		/*
		 * If we are to dump a core, but keep going on our merry way
		 */
		case M_HANDLE_CORE: {
			int fd;
			char buf[20];

			fd = open("/proc/dumper", O_RDWR);
			itoa(getpid(), buf, 10);
			if (fd >= 0) {
				write(fd, buf, strlen(buf));
				close(fd);
			}
		}


		/*
		 * If we are to just ignore the error and keep on processing
		 */
		case M_HANDLE_IGNORE:
			break;

	} /* switch(... */

} /* malloc_err_handler(... */

/*
 * Function:	malloc_int_suffix()
 *
 * Purpose:	determine the correct suffix for the integer passed
 *		(i.e. the st on 1st, nd on 2nd).
 *
 * Arguments:	i - the integer whose suffix is desired.
 *
 * Returns:	pointer to the suffix
 *
 * Narrative:
 *
 */
char *
malloc_int_suffix(long i)
{
	int	  j;
	char	* rtn;
	/*
	 * since the suffixes repeat for the same number within a
	 * given 100 block (i.e. 111 and 211 use the same suffix), get the
	 * integer moded by 100.
	 */
	i = i % 100;
	j = i % 10;

	/*
	 * if the number is 11, or 12, or 13 or its singles digit is
	 * not a 1, 2, or 3, the suffix must be th.
	 */
	if ((i == 11) || (i == 12) || (i == 13) ||
		( (j != 1) && (j != 2) && (j != 3) )) {
		rtn = "th";
	} else {
		switch(j) {
			case 1:
				rtn = "st";
				break;
			case 2:
				rtn = "nd";
				break;
			case 3:
				rtn = "rd";
				break;
			default:
				rtn = "th";
				break;
		}
	}

	return(rtn);

} /* malloc_int_suffix(... */

/*
 * $Log$
 * Revision 1.34  2007/05/04 17:15:12  elaskavaia
 * - signal catcher - produce a fatal error before exit
 * PR:43722
 * CI:alain
 * CI:dinglis
 *
 * Revision 1.33  2007/04/24 15:44:27  shiv
 *
 * PR:29730
 * CI:cburgess
 * CI:xtang
 *
 * this is part of work order WO790334 for IGT. Included enhancements
 * for configurability of the memory allocator.  Includes matching changes for
 * lib/c/alloc and lib/malloc as usual. This is to bring head in line
 * with the work committed to the branch.
 *
 * Revision 1.32  2006/12/22 17:38:48  seanb
 *
 * - Add more magic numbers to reduce chance of false
 *   positive.  Still not perfect so PR left open.
 * PR:43552
 * CI:ELaskavaia
 *
 * Revision 1.31  2006/12/19 20:31:45  elaskavaia
 * - fixed not supporting 10 digit process id's
 * PR: 43765
 * CI:alain, cburgess
 *
 * Revision 1.30  2006/09/28 19:05:56  alain
 * PR:41782
 * CI: shiv@qnx.com
 * CI: cburgess@qnx.com
 *
 * Commiting the work done for IGT-CE on the head.
 *
 * Revision 1.28.2.23  2006/08/09 19:11:21  alain
 *
 * Add 2 new macros MALLOC_TRACEBTDEPTH and MALLOC_EVENTBTDEPTH to be able
 * to set bactrace depth.
 *
 * Revision 1.28.2.22  2006/07/27 19:53:29  alain
 *
 * Remove a deadlock (not calling LEAVE) in the control thread
 * new setting MALLOC_VERBOSE
 *
 * Revision 1.28.2.21  2006/07/26 19:53:50  alain
 *
 * Rename MALLOC_CKALLOC_PARAM to MALLOC_CKALLOC for consistency
 * add as clone to MALLOC_FILL_AREA MALLOC_CKBOUNDS for consistency
 *
 * Revision 1.28.2.20  2006/07/19 19:51:57  alain
 *
 *
 * We now have a new function call find_malloc_range() that does not do any
 * checking but rather (Dhead *)ptr - 1.  This is much faster in order of magnitue
 * of 10.  The bad things is that if the pointer is bad we will not be able to detect
 * it.  But to palliate there is an optiuon MALLOC_CHECK_ALLOC_PARAM that will revert
 * back to the old behaviour.
 * Now free() and realloc() only use the slow when MALLOC_CHEK_ALLOC_PARAM is set
 * malloc() always use the find_malloc_range() we assume that if it is succesfull not
 * need to check again.
 *
 * Things did not work for IGT since they use signal for IPC and the system call in QNX
 * are not restartable.  We need to make sure when the thread is created that it will ignore
 * the signals (block the signal for the controlling thread).
 *
 * Revision 1.28.2.19  2006/07/11 13:20:41  alain
 *
 * m_inti.c, malloc_g.c:  now the bins statistic are configurable.
 * Small optimisation when doing malloc.
 *
 * Revision 1.28.2.18  2006/06/29 13:48:20  alain
 *
 * malloc_g.c: (BINS)The calculation of the total number of blocks was incorrect,
 * it did not take to account the numbers in the depleted list.
 * (USAGE): we now return the overhead:use:free total of the memory.
 * mallopt.c:  Doing MALLOPT_VERIFY was a noop, we now call _check_list(1);
 * mtrace.c:  some WRITEOUTs were missing the newlines.
 *
 * Revision 1.28.2.17  2006/05/29 04:12:28  alain
 *
 * There was an inconsistent in the values, so now:
 * MALLOC_TRACEBT --> __malloc_bt_depth
 * MALLOC_BTDEPTH --> __malloc_error_btdepth
 *
 * Revision 1.28.2.16  2006/05/15 16:42:49  alain
 *
 * bt.c:add to backtrace() a starting address to eliminate undesirable backtrace
 * from the debug malloc library itself.
 *
 * mtrace.c: new format all  tracing event will have a STARTEV and ENDEV, the
 * backtrace is now after the {m,re,c}alloc line.
 *
 * calloc.c, realloc.c, free.c, malloc_g.c:
 * adjust the format to changes describe above.
 *
 * m_inti.c mallocpt.c:
 * new environment MALLOC_USE_DLADDR to enable or disable the use of dladdr() call.
 *
 * Revision 1.28.2.15  2006/04/17 20:54:37  alain
 *
 *
 * We use the backtraces() function call to get backtrace when the builtin
 * function call __builtin_return_address() fails.
 * Also fix indentation and code formating on many of the files.
 *
 * Revision 1.28.2.14  2006/04/13 20:49:54  alain
 *
 * We move the backtrace code out of mtrace.c in bt.c so the could be
 * share in the future.
 *
 * Revision 1.28.2.13  2006/04/07 17:18:00  alain
 *
 * fixed typo when calling __tls()->pid
 *
 * Revision 1.28.2.12  2006/04/07 15:00:50  alain
 *
 * Do not use the getpid() call instead do __tls->pid
 *
 * Revision 1.28.2.11  2006/04/06 16:40:02  alain
 *
 *
 * Added the action to "freeze" or rather a SIGSTOP is drop on the process if
 * the action for error is to stop the application.
 *
 * Revision 1.28.2.10  2006/03/17 22:39:53  alain
 *
 * Fix the crash when doing double free or free()'ing non heap pointers
 * Fix indentations.
 *
 * Revision 1.28.2.9  2006/03/16 20:48:52  alain
 *
 * Provide a configurable way of setting the statistics counters.
 * Do some indentations fixes.
 *
 * Revision 1.28.2.8  2006/03/06 21:12:10  alain
 *
 * Fix the indentation for tabs.
 *
 * Revision 1.28.2.7  2006/03/02 21:32:02  alain
 *
 *
 * We should not call getenv() anywhere this can lead to potential deadlocks
 * the strategy here is to do all the initialization upfront in the init code.
 * Also some minor formatting fixes.
 *
 * Revision 1.28.2.6  2006/02/21 04:54:54  alain
 *
 *
 * Support for BINS, malloc provides stat_bins we make this available
 * as a new memory event.
 *
 * Revision 1.28.2.5  2006/02/15 17:57:08  alain
 *
 * PR: 20444
 * Add the tid field in the error.
 *
 * Revision 1.28.2.4  2006/02/15 00:41:47  alain
 *
 * PR: 29444
 * For the error event we should add the timestamp event if the error is an CRC.
 *
 * Revision 1.28.2.3  2006/02/02 18:57:37  alain
 *
 * PR:
 * CI:
 *
 * Put the timestamp support.
 *
 * Revision 1.28.2.2  2005/12/14 23:08:21  alain
 * the pid should be 10 chars when printing
 *
 * Revision 1.28.2.1  2005/12/13 18:51:33  alain
 *
 * PR:
 * CI:
 *
 *  	malloc_control.c malloc_g.c mtrace.c
 *
 * Modification to test the malloc library
 *
 * Revision 1.28  2005/06/03 01:22:48  adanko
 *
 * Replace existing QNX copyright licence headers with macros as specified by
 * the QNX Coding Standard. This is a change to source files in the head branch
 * only.
 *
 * Note: only comments were changed.
 *
 * PR25328
 *
 * Revision 1.27  2005/03/29 18:22:44  shiv
 * PR24134
 * PR24010
 * PR24008
 * PR24184
 * The malloc lib used to report errors when mem* and str* functions
 * were called (for those that take a length parameter) with
 * a length of zero, if the other arguments are not valid..
 * In general these would not cause errors, since
 * no actual data is moved. But since the errors being reported could
 * be useful, the option to increase the level of verbosity for this
 * has been provided. the environment variable
 * MALLOC_CKACCESS_LEVEL can be used or the mallopt call
 * with the option mallopt(MALLOC_CKACCESS_LEVEL, arg)
 * can be used. By default the level is zero, a non-zero
 * level will turn on strict checking and reporting of errors
 * if the arguments are not valid.
 * Also fixed PR24184 which had an incorrect function name
 * being passed in (strchr instead of strrchr... thanx kirk)
 * Modified Files:
 * 	mallocint.h dbg/m_init.c dbg/malloc_chk.c dbg/malloc_g.c
 * 	dbg/mallopt.c dbg/memory.c dbg/string.c
 * 	public/malloc_g/malloc-lib.h
 *
 * Revision 1.26  2005/03/28 15:48:58  shiv
 * PR24003. Fixed the library so that it does not call
 * fork, instead it tells dumper directly to dump core for
 * this process.
 * Modified Files:
 * 	malloc_g.c
 *
 * Revision 1.25  2005/02/25 03:03:37  shiv
 * More fixes for the debug malloc, for the tools to work
 * better.
 * Modified Files:
 * 	malloc-lib.h mallocint.h dbg/calloc.c dbg/dump.c dbg/m_init.c
 * 	dbg/malloc_debug.c dbg/malloc_g.c dbg/mtrace.c dbg/realloc.c
 * 	public/malloc_g/malloc.h
 *
 * Revision 1.24  2005/02/15 21:30:35  shiv
 * Output pointer for leak event.
 * Modified Files:
 * 	dbg/malloc_g.c
 *
 * Revision 1.23  2005/02/11 19:00:28  shiv
 * Some more malloc_g changes.
 * Modified Files:
 * 	mallocint.h dbg/dump.c dbg/m_init.c dbg/m_perror.c
 *  	dbg/malloc_g.c dbg/malloc_gc.c dbg/mtrace.c
 *
 * Revision 1.22  2005/01/19 16:53:00  shiv
 * Slight mods to allow easier access to the free list
 * to allow user land to gather information about allocator.
 * Matching with libc checkin.
 * Modified Files:
 * 	dbg/dump.c dbg/malloc_chk.c dbg/malloc_g.c
 *
 * Revision 1.21  2005/01/16 20:38:45  shiv
 * Latest DBG malloc code. Lots of cleanup/optimistions
 * Modified Files:
 * 	common.mk mallocint.h common/tostring.c dbg/analyze.c
 * 	dbg/calloc.c dbg/dump.c dbg/free.c dbg/m_init.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc_g/malloc-lib.h public/malloc_g/malloc.h
 * 	std/calloc.c std/free.c std/m_init.c std/malloc_wrapper.c
 * 	std/mtrace.c std/realloc.c
 * Added Files:
 * 	dbg/dl_alloc.c dbg/malloc_control.c dbg/malloc_debug.c
 * 	dbg/new.cc public/malloc_g/malloc-control.h
 * 	public/malloc_g/malloc-debug.h
 *
 * Revision 1.20  2004/02/12 15:43:16  shiv
 * Updated copyright/licenses
 * Modified Files:
 * 	common.mk debug.h malloc-lib.h mallocint.h tostring.h
 * 	common/tostring.c dbg/analyze.c dbg/calloc.c dbg/context.h
 * 	dbg/crc.c dbg/dump.c dbg/free.c dbg/m_init.c dbg/m_perror.c
 * 	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 * 	dbg/malloc_gc.c dbg/mallopt.c dbg/memory.c dbg/mtrace.c
 * 	dbg/process.c dbg/realloc.c dbg/string.c
 * 	public/malloc/malloc.h public/malloc_g/malloc-lib.h
 * 	public/malloc_g/malloc.h public/malloc_g/prototypes.h
 * 	std/calloc.c std/context.h std/free.c std/m_init.c
 * 	std/malloc_wrapper.c std/mtrace.c std/realloc.c test/memtest.c
 * 	test/mtrace.c
 *
 * Revision 1.19  2004/01/19 20:01:37  shiv
 * Cleaned up some warning messages.
 * Modified Files:
 * 	malloc_g.c mtrace.c
 *
 * Revision 1.18  2004/01/09 17:37:53  shiv
 * PR17515. made the locking proper now. The locks are still
 * recursive, since the code has the locking/unlocking calls
 * strewn all over the place. Eventually we may want to remove
 * the recursive nature of the locks, but for now the locking
 * is correct. This affects purely the debug malloc library only.
 * Modified Files:
 * 	mallocint.h dbg/malloc_g.c
 *
 * Revision 1.17  2003/11/04 18:06:35  shiv
 * Fixed several compiler warnings.
 *
 * Revision 1.16  2003/09/25 19:06:49  shiv
 * Fixed several things in the malloc code including the
 * leak detection for both small and large blocks. re-arranged
 * a lot code, and removed pieces that were not necessary after the
 * re-org. Modified the way in which the elf sections are read to
 * determine where heap references could possibly be stored.
 * set the optimisation for the debug variant at -O0, just so
 * so that debugging the lib itself is a little easier.
 * Modified Files:
 * 	common.mk mallocint.h dbg/dump.c dbg/malloc_chk.c
 * 	dbg/malloc_chn.c dbg/malloc_g.c dbg/malloc_gc.c dbg/mtrace.c
 * 	dbg/process.c dbg/string.c
 *
 * Revision 1.15  2002/05/29 19:50:28  shiv
 * Fixed cases where we were performing unaligned
 * acceses, that caused issues on non-x86 platforms.
 * We take care to walk only aligned pointers now
 * into our structures. PR10599
 *
 * Revision 1.14  2002/04/23 17:07:45  shiv
 * PR11328: Changes made to lib/c/alloc synchronised here.
 * Also an initialisation of the arena in lib/c/alloc
 * was inconsistent with the redifnition of struct Arena
 * here.
 *
 * Revision 1.13  2002/04/05 13:59:28  thomasf
 * Need a colon as a delimiters.
 *
 * Revision 1.12  2002/03/25 21:14:21  thomasf
 * Add support for a prefix to be appended to the stack frame dumps.
 *
 * Revision 1.11  2002/03/22 20:45:21  thomasf
 * Initial changes to make the output format machine readable for the events.
 *
 * Revision 1.10  2001/03/01 20:37:54  furr
 * Added debug wrappers for the new aligned memory, mprobe and mcheck functions
 * Made mallopt signature compatible
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	common.mk mallocint.h dbg/m_init.c dbg/malloc_chk.c
 *  	dbg/malloc_g.c dbg/mallopt.c public/malloc/malloc.h
 *  	public/malloc_g/malloc-lib.h public/malloc_g/malloc.h
 *  	public/malloc_g/prototypes.h std/m_init.c std/malloc_wrapper.c
 *
 * Revision 1.9  2001/02/09 23:01:33  furr
 * Removed stack traceback option to malloc error handler for non-x86
 * architectures (no traceback logic for them yet)
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	dbg/malloc_g.c
 *
 * Revision 1.8  2001/02/05 22:07:12  furr
 * Minor fix to mtrace code (correctly print symbol offsets)
 * Warn about NULL pointers to str functions
 * Add stack tracebacks to warnings.
 * Added missing context file.
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	dbg/m_init.c dbg/malloc_g.c dbg/malloc_gc.c dbg/mtrace.c
 *  	dbg/string.c dbg/tostring.c public/malloc_g/malloc.h
 *  	public/malloc_g/prototypes.h
 *  Added Files:
 *  	dbg/context.h
 *
 * Revision 1.7  2001/02/05 18:34:30  furr
 * Added mtrace support
 * - produce output compatible with GNU tools
 *    - no code from GNU used, but see glibc mtrace.pl for what it expects
 *
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	mallocint.h dbg/calloc.c dbg/free.c dbg/m_init.c
 *  	dbg/malloc_g.c dbg/realloc.c public/malloc_g/prototypes.h
 *  	test/memtest.c
 *  Added Files:
 *  	dbg/mtrace.c
 *
 * Revision 1.6  2000/04/24 15:45:50  furr
 *
 *  Modified Files:
 *  	dbg/calloc.c dbg/free.c dbg/malloc_g.c dbg/realloc.c
 *
 *  Added entry points for bounds-checking GCC
 *
 * Revision 1.5  2000/04/12 19:33:34  furr
 *
 *  Committing in .
 *  Modified Files:
 *  	malloc_g.c realloc.c
 *
 *  	- fixed the setting of the ulen in the header for mallocs and
 *  	  reallocs when fill-area boundary checking isn't enabled and
 *  	  when realloc creates a new block
 *
 * Revision 1.4  2000/03/08 16:04:54  furr
 * Added an _musize() function
 *
 *  Committing in .
 *
 *  Modified Files:
 *  	malloc_chk.c malloc_g.c
 *
 * Revision 1.3  2000/02/15 22:01:07  furr
 * Changed to a more reasonable heap marking algorithm for memory leak
 * detection.
 *   - won't blow the stack
 * Fixed up file and line printing in all dumps.
 *
 *  Modified Files:
 *  	mallocint.h dbg/dump.c dbg/malloc_g.c dbg/malloc_gc.c
 *  	dbg/process.c dbg/tostring.c test/memtest.C
 *
 * Revision 1.2  2000/02/01 19:48:06  furr
 * Fixed some traversal problems in chain checker
 * Fixed neighbor checks to ensure we don't go outside of the block or arena
 *
 *  Modified Files:
 *  	dbg/malloc_chk.c dbg/malloc_chn.c dbg/malloc_g.c
 *
 * Revision 1.1  2000/01/31 19:03:30  bstecher
 * Create libmalloc.so and libmalloc_g.so libraries for everything. See
 * Steve Furr for details.
 *
 * Revision 1.1  2000/01/28 22:32:44  furr
 * libmalloc_g allows consistency checks and bounds checking of heap
 * blocks allocated using malloc.
 * Initial revision
 *
 *  Added Files:
 *  	Makefile analyze.c calloc_g.c crc.c dump.c free.c m_init.c
 *  	m_perror.c malloc-config.c malloc_chk.c malloc_chn.c
 *  	malloc_g.c malloc_gc.c mallopt.c memory.c process.c realloc.c
 *  	string.c tostring.c inc/debug.h inc/mallocint.h inc/tostring.h
 *  	inc/malloc_g/malloc inc/malloc_g/malloc.h
 *  	inc/malloc_g/prototypes.h test/memtest.C test/memtest.c
 *  	x86/Makefile x86/so/Makefile
 *
 * Revision 1.2  1996/08/18 21:00:56  furr
 * print the caller return address on errors
 *
 * Revision 1.1  1996/07/24 18:05:44  furr
 * Initial revision
 *
 * Revision 1.19  1992/01/28  16:35:37  cpcahil
 * increased size of error string buffers and added overflow checks
 *
 * Revision 1.18  1992/01/10  17:51:03  cpcahil
 * more void stuff that slipped by
 *
 * Revision 1.17  1992/01/10  17:28:03  cpcahil
 * Added support for overriding void datatype
 *
 * Revision 1.16  1992/01/08  19:40:07  cpcahil
 * fixed write() count to display entire message.
 *
 * Revision 1.15  1991/12/31  21:31:26  cpcahil
 * changes for patch 6.  See CHANGES file for more info
 *
 * Revision 1.14  1991/12/06  08:50:48  cpcahil
 * fixed bug in malloc_safe_memset introduced in last change.
 *
 * Revision 1.13  91/12/04  18:01:21  cpcahil
 * cleand up some aditional warnings from gcc -Wall
 *
 * Revision 1.12  91/12/04  09:23:39  cpcahil
 * several performance enhancements including addition of free list
 *
 * Revision 1.11  91/12/02  19:10:10  cpcahil
 * changes for patch release 5
 *
 * Revision 1.10  91/11/25  14:41:59  cpcahil
 * Final changes in preparation for patch 4 release
 *
 * Revision 1.9  91/11/24  16:56:41  cpcahil
 * porting changes for patch level 4
 *
 * Revision 1.8  91/11/24  00:49:27  cpcahil
 * first cut at patch 4
 *
 * Revision 1.7  91/11/20  11:54:09  cpcahil
 * interim checkin
 *
 * Revision 1.6  90/05/11  00:13:09  cpcahil
 * added copyright statment
 *
 * Revision 1.5  90/02/25  11:01:18  cpcahil
 * added support for malloc chain checking.
 *
 * Revision 1.4  90/02/24  21:50:21  cpcahil
 * lots of lint fixes
 *
 * Revision 1.3  90/02/24  14:51:18  cpcahil
 * 1. changed malloc_fatal and malloc_warn to use malloc_errno and be passed
 *    the function name as a parameter.
 * 2. Added several function headers.
 * 3. Changed uses of malloc_fatal/warning to conform to new usage.
 *
 * Revision 1.2  90/02/23  18:05:23  cpcahil
 * fixed open of error log to use append mode.
 *
 * Revision 1.1  90/02/22  23:17:43  cpcahil
 * Initial revision
 *
 */
