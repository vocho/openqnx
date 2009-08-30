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




/*
 * "memchunk"
 * John Garvey, QNX Software Systems Ltd, 1997
 */

#include <errno.h>
#include <malloc.h>
#include <pthread.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/neutrino.h>
#include <sys/queue.h>
#include <sys/syspage.h>
#include <sys/types.h>
#include <unistd.h>


#define BUCKET_MIN_SIZE		8
#define BUCKET_SAME_SIZE	16
#define BUCKET_WATERMARK	2

#define ALLOC(_n)		mmap(NULL, _n, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON, NOFD, 0)
#define FREE(_p, _n)	munmap(_p, _n)
#define ALLOC_FAILURE	(MAP_FAILED)
#define FREE_FAILURE	(-1)

#define ALIGN(_n, _m)	(((_n) + ((_m) - 1)) & ~((_m) - 1))
#define ROUNDUP(_n, _m)	((((_n) + (_m) - 1) / (_m)) * (_m))

typedef union MemchunkEntry {
	union MemchunkEntry		*link;		/* If unused: link to next unused  */
	struct MemchunkHdr		*owner;		/* If used: link to chunk hdr ctrl */
} MemchunkEntry;

typedef struct {
	size_t			nbytes;				/* Bytes allocated externally      */
	MemchunkEntry	filler;				/* Internal header (owner == NULL) */
} MemchunkExternal;

typedef struct MemchunkHdr {
	TAILQ_ENTRY(MemchunkHdr)	link;	/* Doubly-linked-list of chunks     */
	MemchunkEntry				*unused;/* First unused entry in this chunk */
	unsigned short				used;	/* Number of used entries           */
	unsigned short				ctrl;	/* Index of MemChunkBucket ctrl hdr */
} MemchunkHdr;

typedef struct {
	int						capacity;	/* Size of an allocation entry       */
	int						count;		/* Number of entries to chunk        */
	int						available;	/* Total free chunks (low watermark) */
	TAILQ_HEAD(,MemchunkHdr)header;		/* Linked-list of allocated chunks   */
} MemchunkBucket;

typedef struct MemchunkCtrl {
	pthread_mutex_t	mutex;				/* Mutex controlling thread access */
	int				pagesize;			/* VM subsystem page size          */
	MemchunkBucket	buckets[1];			/* Sorted list of bucket headers   */
} MemchunkCtrl;

static MemchunkBucket *FindControl(MemchunkCtrl *memctrl, int sz)
{
MemchunkBucket	*ctrl;

	sz += sizeof(MemchunkEntry);
	for (ctrl = memctrl->buckets; ctrl->capacity != 0; ++ctrl)
		if (sz <= ctrl->capacity)
			return(ctrl);
	return(NULL);
}

static MemchunkEntry *UseFreeEntry(MemchunkBucket *ctrl, MemchunkHdr *hdr)
{
MemchunkEntry	*entry;

	entry = hdr->unused;
	hdr->unused = entry->link;
	++hdr->used;
	entry->owner = hdr;
	--ctrl->available;
	if (hdr != TAILQ_FIRST(&ctrl->header) && hdr->unused != NULL) {
		TAILQ_REMOVE(&ctrl->header, hdr, link);
		TAILQ_INSERT_HEAD(&ctrl->header, hdr, link);
	}
	return(entry);
}

static MemchunkHdr *UnuseAllocEntry(MemchunkEntry *entry, MemchunkBucket *buckets)
{
MemchunkBucket	*ctrl;
MemchunkHdr		*hdr;

	hdr = entry->owner, ctrl = &buckets[hdr->ctrl];
	entry->link = hdr->unused;
	hdr->unused = entry;
	++ctrl->available;
	if (hdr != TAILQ_FIRST(&ctrl->header)) {
		TAILQ_REMOVE(&ctrl->header, hdr, link);
		TAILQ_INSERT_HEAD(&ctrl->header, hdr, link);
	}
	return((--hdr->used != 0) ? NULL : hdr);
}

static MemchunkEntry *FindEntry(MemchunkCtrl *memctrl, MemchunkBucket *ctrl)
{
MemchunkHdr		*hdr;
MemchunkEntry	*entry;
int				i, offset;

	for (hdr = TAILQ_FIRST(&ctrl->header); hdr != NULL; hdr = TAILQ_NEXT(hdr, link))
		if (hdr->unused != NULL)
			return(UseFreeEntry(ctrl, hdr));
	if ((hdr = ALLOC(sizeof(MemchunkHdr) + (ctrl->capacity * ctrl->count))) == ALLOC_FAILURE)
		return(NULL);
	ctrl->available += ctrl->count;
	hdr->used = 0;
	hdr->ctrl = ctrl - &memctrl->buckets[0];
	offset = ALIGN(sizeof(*hdr), sizeof(int));
	hdr->unused = entry = (MemchunkEntry *)((char *)hdr + offset);
	for (i = ctrl->count - 1; i > 0; --i)
		entry = entry->link = (MemchunkEntry *)((char *)entry + ctrl->capacity);
	entry->link = NULL;
	TAILQ_INSERT_HEAD(&ctrl->header, hdr, link);
	return(UseFreeEntry(ctrl, hdr));
}

int MemchunkInit(MemchunkCtrl **memctrl, int n, const size_t cfg[])
{
MemchunkCtrl	*ctrl;
size_t			sz;
int				i, j, pgsz, error;

	if ((ctrl = malloc(sizeof(*ctrl) + (n * sizeof(MemchunkBucket)))) != NULL) {
		if ((error = pthread_mutex_init(&ctrl->mutex, NULL)) == EOK) {
			if ((ctrl->pagesize = sysconf(_SC_PAGESIZE)) == -1)
				ctrl->pagesize = SYSPAGE_ENTRY(system_private)->pagesize;
			pgsz = ctrl->pagesize - ALIGN(sizeof(MemchunkHdr), sizeof(int));
			for (i = 0; i < n; ++i) {
				if ((sz = cfg[i]) < BUCKET_MIN_SIZE)
					sz = BUCKET_MIN_SIZE;
				sz = ALIGN(sizeof(MemchunkEntry), sizeof(int)) + ALIGN(sz, sizeof(int));
				for (j = 0; j < i; ++j) {
					if (sz < ctrl->buckets[j].capacity) {
						memmove(&ctrl->buckets[j + 1], &ctrl->buckets[j], (i - j) * sizeof(MemchunkBucket));
						break;
					}
				}
				ctrl->buckets[j].capacity = sz;
				ctrl->buckets[j].count = ROUNDUP(sz, pgsz) / sz;
			}
			while (--i > 0) {
				if (ctrl->buckets[i].capacity - ctrl->buckets[i - 1].capacity < BUCKET_SAME_SIZE)
					memmove(&ctrl->buckets[i - 1], &ctrl->buckets[i], (n-- - i) * sizeof(MemchunkBucket));
			}
			for (i = 0; i < n; ++i) {
				ctrl->buckets[i].available = 0;
				TAILQ_INIT(&ctrl->buckets[i].header);
			}
			memset(&ctrl->buckets[n], 0, sizeof(MemchunkBucket));
			*memctrl = ctrl;
			return(EOK);
		}
		free(ctrl);
	}
	else {
		error = ENOMEM;
	}
	return(error);
}

void *MemchunkMalloc(MemchunkCtrl *memctrl, size_t nbytes)
{
MemchunkBucket		*ctrl;
MemchunkEntry		*entry;
MemchunkExternal	*ext;
void				*result;

	if (!nbytes)
		return(NULL);
#ifndef MQUEUE_1_THREAD
	_mutex_lock(&memctrl->mutex);
#endif
	if ((ctrl = FindControl(memctrl, nbytes)) != NULL && (entry = FindEntry(memctrl, ctrl)) != NULL) {
		result = entry + 1;
	}
	else if ((ext = malloc(nbytes += sizeof(MemchunkExternal))) != NULL) {
		ext->nbytes = nbytes;
		(entry = &ext->filler)->owner = NULL;
		result = entry + 1;
	}
	else {
		result = NULL;
	}
#ifndef MQUEUE_1_THREAD
	_mutex_unlock(&memctrl->mutex);
#endif
	return(result);
}

void *MemchunkCalloc(MemchunkCtrl *memctrl, int n, size_t nbytes)
{
void	*ptr;

	if ((ptr = MemchunkMalloc(memctrl, n *= nbytes)) != NULL)
		memset(ptr, 0, n);
	return(ptr);
}

void MemchunkFree(MemchunkCtrl *memctrl, const void *ptr)
{
MemchunkEntry		*entry;
MemchunkExternal	*ext;
MemchunkBucket		*ctrl;
MemchunkHdr			*hdr;

	if (ptr == NULL)
		return;
#ifndef MQUEUE_1_THREAD
	_mutex_lock(&memctrl->mutex);
#endif
	entry = (MemchunkEntry *)((char *)ptr - sizeof(MemchunkEntry));
	if (entry->owner == NULL) {
		ext = (MemchunkExternal *)((char *)ptr - sizeof(MemchunkExternal));
		free(ext);
	}
	else if ((hdr = UnuseAllocEntry(entry, memctrl->buckets)) != NULL) {
		ctrl = &memctrl->buckets[hdr->ctrl];
		if (ctrl->available > ((ctrl->count > 1) ? ((BUCKET_WATERMARK - 1) * ctrl->count) / BUCKET_WATERMARK + ctrl->count : BUCKET_WATERMARK)) {
			TAILQ_REMOVE(&ctrl->header, hdr, link);
			if (FREE(hdr, sizeof(MemchunkHdr) + (ctrl->capacity * ctrl->count)) != FREE_FAILURE)
				ctrl->available -= ctrl->count;
			else
				TAILQ_INSERT_HEAD(&ctrl->header, hdr, link);
		}
	}
#ifndef MQUEUE_1_THREAD
	_mutex_unlock(&memctrl->mutex);
#endif
}

__SRCVERSION("memchunk.c $Rev: 153052 $");
