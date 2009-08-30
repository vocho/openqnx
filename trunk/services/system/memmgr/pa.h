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


//
// Physical allocator definitions
//

struct syspage_entry;

struct pa_restrict {
	struct pa_restrict	*next;
	int					(*checker)(struct syspage_entry *, paddr64_t *,
									paddr64_t *, size_t, size_t); 
	paddr_t				start;
	paddr_t				end;
};

#ifndef QUANTUM_BITS
	#define	QUANTUM_BITS	12
#endif
#define QUANTUM_SIZE	(1 << QUANTUM_BITS)

#define LEN_TO_NQUANTUM(s)		(((s) + (QUANTUM_SIZE-1)) >> QUANTUM_BITS)
#define NQUANTUM_TO_LEN(q)		((unsigned)(q) << QUANTUM_BITS)

/* Quantum Flags (per quantum attr) */
// leave top bit unused for efficiency (think RISC immediate encoding)
#define PAQ_FLAG_UNUSED			0x8000 
#define PAQ_FLAG_INITIALIZED	0x4000
#define PAQ_FLAG_ZEROED			0x2000
#define PAQ_FLAG_INUSE			0x1000
#define PAQ_FLAG_HAS_INSTRS		0x0800
//RUSH3: What to do with sync objects on swapped out pages?
#define PAQ_FLAG_HAS_SYNC		0x0400
#define PAQ_FLAG_LOCKED			0x0200
#define PAQ_FLAG_MODIFIED		0x0100
#define PAQ_FLAG_INIT_REQUIRED	0x0080 // Only used with pa_quantum's.
#define PAQ_FLAG_RDB			0x0080 // Only used with pa_quantum_fake's.
#define PAQ_FLAG_SYSTEM			0x0040
#define PAQ_FLAG_ACTIVE         0x0020
#define PAQ_FLAG_COLOUR_MASK	0x000f // Allow up to 15 colours
//RUSH3: flags for anon or private memory? For mem with multiple refs?

#define PAQ_COLOUR_NONE			(~0 & PAQ_FLAG_COLOUR_MASK)

#define PAQ_BLK_FAKE			((_Uint16t)~0)

struct pa_free_link {
	struct pa_quantum	*next; // must be first entry
	struct pa_quantum	*prev;
};

struct pa_inuse_link {
	struct pa_quantum	*next; // must be first entry
	_Uint32t			qpos;
};

struct pa_quantum {
	union {
		struct pa_inuse_link	inuse;
		struct pa_free_link		flink;
	}			u;
	_Int32t		run;	// run size if > 0, run backup if < 0, search if 0
	_Uint16t	blk;	// if == PAQ_BLK_FAKE, fake entry (see below)
	_Uint16t	flags;
};

struct pa_quantum_fake {
	struct pa_quantum	q;
	paddr_t				paddr;
};

#define PAQ_GET_COLOUR(pq)		((pq)->flags & PAQ_FLAG_COLOUR_MASK)
#define PAQ_SET_COLOUR(pq,c)	((pq)->flags = (c) | ((pq)->flags & ~PAQ_FLAG_COLOUR_MASK))


// Multiple free lists are used to try to reduce the time taken to find memory.
//
// Testing has shown that a large percentage of memory requests are for
// relatively small sizes of memory.  When freeing memory, we try to coalesce
// adjacent runs, but still end up with small stranded runs of memory.
//
// Putting small runs on dedicated free queues does two things:
// 1) It makes it very quick to satisfy a request for that size of memory
// 2) It reduces the need to fragment large runs into smaller ones
//
// A dedicated free queue is used for memory runs smaller than PA_FREE_QUEUES
// Anything larger is put in the default queue
//
// The size is a tradeoff, because it takes time to scan multiple lists.

#define PA_FREE_QUEUES 8
#define PA_DEFAULT_QUEUE 0

// This next structure is actually only used in pa.c, but it's in
// this header file to make the definition available to the kernel
// dumper.

struct block_head {
	union {
		paddr32_t	paddr32;
		paddr64_t	paddr64;
		paddr_t		paddr;
	}					start;
	union {
		paddr32_t	paddr32;
		paddr64_t	paddr64;
		paddr_t		paddr;
	}					end;
	unsigned			generation;
	unsigned			num_to_clean;
	unsigned			num_quanta;
	unsigned			max_free_run;
	struct pa_quantum	*quanta;
	struct pa_free_link	free[PA_FREE_QUEUES];
};
