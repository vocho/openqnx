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
 *  sys/memmsg.h
 *

 */
#ifndef __MEMMSG_H_INCLUDED
#define __MEMMSG_H_INCLUDED

#if defined(__WATCOMC__) && !defined(_ENABLE_AUTODEPEND)
#pragma read_only_file;
#endif

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __SYSMSG_H_INCLUDED
#include _NTO_HDR_(sys/sysmsg.h)
#endif

#ifndef __MMAN_H_INCLUDED
#include _NTO_HDR_(sys/mman.h)
#endif

#define MEMMGR_COID				SYSMGR_COID

enum {
	_MEM_MAP = _MEMMGR_BASE,
	_MEM_CTRL,
	_MEM_INFO,
	_MEM_OFFSET,
	_MEM_DEBUG_INFO,
	_MEM_SWAP,
	_MEM_PMEM_ADD,
	_MEM_PEER,
};

enum {
	_MEM_CTRL_UNMAP,
	_MEM_CTRL_PROTECT,
	_MEM_CTRL_SYNC,
	_MEM_CTRL_LOCKALL,
	_MEM_CTRL_UNLOCKALL,
	_MEM_CTRL_LOCK,
	_MEM_CTRL_UNLOCK,
	_MEM_CTRL_ADVISE
};

enum {
	_MEM_SWAP_ON,
	_MEM_SWAP_OFF,
	_MEM_SWAP_CTRL,
	_MEM_SWAP_STAT
}; 

enum {
	_MEM_OFFSET_PHYS,
	_MEM_OFFSET_FD,
	_MEM_OFFSET_PT
}; 

#include _NTO_HDR_(_pack64.h)

/*
 * Message of _MEM_MAP
 */
struct _mem_map {
	_Uint16t						type;
	_Uint16t						zero;
	_Uint32t						reserved1;
	_Uint64t						addr;
	_Uint64t						len;
	_Uint32t						prot;
	_Uint32t						flags;
	_Int32t							fd;
	_Uint32t						preload;
	_Uint64t						align;
	off64_t							offset;
};

struct _mem_map_reply {
	_Uint64t						real_size;
	_Uint64t						real_addr;
	_Uint64t						addr;
};

typedef union {
	struct _mem_map					i;
	struct _mem_map_reply			o;
} mem_map_t;


/*
 * Message of _MEM_CTRL
 */
struct _mem_ctrl {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint32t						flags;
	_Uint64t						addr;
	_Uint64t						len;
};

typedef union {
	struct _mem_ctrl				i;
} mem_ctrl_t;


#if defined(__EXT_QNX)		/* Approved 1003.1j D10 */
/*
 * Message of _MEM_INFO
 */
struct _mem_info {
	_Uint16t						type;
	_Uint16t						zero;
	_Int32t							fd;
	_Uint32t						flags;
	_Uint32t						reserved;
};
	
struct _mem_info_reply {
	_Uint32t						zero[2];
	_Uint32t						size;							
	_Uint32t						reserved;
	struct __posix_typed_mem_info64	info;
};

typedef union {
	struct _mem_info				i;
	struct _mem_info_reply			o;
} mem_info_t;
#endif

/*
 * Message of _MEM_OFFSET
 */
struct _mem_offset {
	_Uint16t						type;
	_Uint16t						subtype;
	_Int32t							reserved;
	_Uint64t						addr;
	_Uint64t						len;
};
	
struct _mem_offset_reply {
	_Uint64t						size;							
	off64_t							offset;
	_Int32t							fd;
	_Uint32t						reserved;
};

typedef union {
	struct _mem_offset				i;
	struct _mem_offset_reply		o;
} mem_offset_t;

/*
 * Message of _MEM_DEBUG_INFO
 */
struct _mem_debug_info {
	_Uint16t						type;
	_Uint16t						zero;
	_Uint32t						reserved;
	off64_t							offset;
	ino64_t							ino;
	_Uintptrt						vaddr;
	_Uint32t						size;
	_Uint32t						flags;
	dev_t							dev;
	_Uintptrt						old_vaddr;
	char							path[1];
};
	
typedef union {
	struct _mem_debug_info			i;
} mem_debug_info_t;

/*
 * Message of _MEM_SWAP
 */
struct _mem_swap {
	_Uint16t						type;
	_Uint16t						subtype;
};

struct _mem_swap_on {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint32t						flags;
	_Uint32t						size;
	_Uint32t						len;
	_Uint32t						spare[2];
	/* _Uint8t						name[]; */
};

struct _mem_swap_off {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint32t						flags;
	_Uint32t						size;
	_Uint32t						len;
	_Uint32t						spare[2];
	/* _Uint8t						name[]; */
};

struct _mem_swap_stat {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint32t						flags;
	_Uint32t						len;
	_Uint32t						spare[5];
};

struct _mem_swap_stat_reply {
	_Uint16t						type;
	_Uint16t						subtype;
	_Uint32t						flags;
	_Uint64t						total;
	_Uint64t						inuse;
	_Uint64t						swapins;
	_Uint64t						swapouts;
	_Uint64t						swaprems;
	_Uint32t						spare[8];
};

typedef union {
	struct _mem_swap				swap;
	struct _mem_swap_on				swap_on;
	struct _mem_swap_off			swap_off;
	struct _mem_swap_stat			swap_stat;
	struct _mem_swap_stat_reply		swap_stat_reply;
} mem_swap_t;


/*
 * Message of _MEM_PMEM_ADD
 */
struct _mem_pmem_add {
	_Uint16t						type;
	_Uint16t						zero1;
	_Uint32t						zero2;
	_Uint64t						addr;
	_Uint64t						len;
};

typedef union {
	struct _mem_pmem_add			i;
} mem_pmem_add_t;


/*
 * Message of _MEM_PEER
 */
struct _mem_peer {
	_Uint16t						type;
	_Uint16t						peer_msg_len;
	_Uint32t						pid;
	_Uint64t						reserved1; 
	/* second mem msg follows - keep this structure a multiple of of 64-bits */
};

typedef union {
	struct _mem_peer			i;
} mem_peer_t;

#include _NTO_HDR_(_packpop.h)

#endif

/* __SRCVERSION("memmsg.h $Rev: 201353 $"); */
