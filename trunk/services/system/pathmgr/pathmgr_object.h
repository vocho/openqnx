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

#ifndef PATHMGR_OBJECT_H
#define PATHMGR_OBJECT_H

#include <sys/iofunc.h>
// FIX ME #include <sys/mempart.h>
#include "kernel/mempart.h"

enum {
	OBJECT_NONE,
	OBJECT_SERVER,
	OBJECT_NAME,
	OBJECT_PROC_SYMLINK,
	OBJECT_MEM_ANON,
	OBJECT_MEM_SHARED,
	OBJECT_MEM_FD,
	OBJECT_MEM_TYPED,
};
struct mempart_s;

//max size of a pathmgr object is limited by the _Uint8t we use to keep track of it's length in object_header.len
#define PATHMGR_MAX_OBJECT_LEN UINT8_MAX

struct object_header {
	OBJECT							*next;
	NODE							*node;
	_Uint8t							type;
	_Uint8t							len;
//RUSH3: The 'flags' field probably only needs to be present in
//RUSH3: the server_object structure. If that's the case and 'mpid'
//RUSH3: can fit in 16 bits, we can shrink the header back down again.
	_Uint16t						flags;
	/* partition stuff follows */
	pid_t							creator;
	part_id_t					mpid;	// partition which the OBJECT is associated with
	void							(*mpart_disassociate)(OBJECT *);	// disassociation function
	unsigned						refs;
};

struct server_object {
	struct object_header			hdr;
	_Uint32t						nd;
	pid_t							pid;
	int								chid;
	unsigned						handle;
	unsigned						file_type;
};

struct symlink_object {
	struct object_header			hdr;
	int								len;
	char							name[1];
};

// This structure is used by both the pathmgr and memmgr subsystems
// of proc. I'd really like to split it into two, but I can't see a
// clean way of doing that :-(.
struct mm_object {
	struct object_header		hdr;

	struct pathmgr_stuff {
		time_t					mtime;
		mode_t					mode; /* Permissions should all live elsewhere*/
		uid_t					uid;
		gid_t					gid;
	}		pm;

	struct memmgr_stuff {
		struct proc_mux_lock	*mux;
		struct pa_quantum		*pmem;
		struct pa_quantum		**pmem_cache;
		struct mm_object_ref	*refs;
//RUSH2: Another restriction list for where to allocate COR memory from?
		struct pa_restrict		*restriction;
		off64_t					size;
		unsigned				flags;
	}		mm;
};

struct mm_object_anon {
	struct mm_object	mem;
	//RUSH2: Stuff to track available offsets...
};

struct mm_object_shared {
	struct mm_object	mem;
	unsigned			special;
	uintptr_t			vaddr;
};

struct mm_object_fd {
	struct mm_object	mem;
	int					fd;
	time_t				ftime;
	ino_t				ino;
	dev_t				dev;
	char				*name;
};

struct mm_object_typed {
	struct mm_object_shared	shmem;
	char 					*name;
};

union object {
	struct object_header			hdr;
	struct server_object			server;
	struct symlink_object			symlink;
	struct mm_object				mem;
	struct mm_object_anon			anmem;
	struct mm_object_shared			shmem;
	struct mm_object_fd				fdmem;
	struct mm_object_typed			tymem;
};


// These bits do not correspond to any of the prot or flags bits that
// a user is allowed to set in the mmap message (they're read only bits
// that come back from mapinfo). The pathmgr flips them on in the input
// message structure to pass some information back to the memory manager
#define IMAP_MASK						(PG_MASK|MAP_SYSRAM)
#define IMAP_OCB_RDONLY					0x00100000 // OCB is read-only, mprotect() can't turn on PROT_WRITE
#define IMAP_TYMEM_ALLOCATE				0x00200000 // POSIX_TYPED_MEM_ALLOCATE
#define IMAP_TYMEM_ALLOCATE_CONTIG		0x00400000 // POSIX_TYPED_MEM_ALLOCATE_CONTIG
#define IMAP_TYMEM_MAP_ALLOCATABLE		0x00800000 // POSIX_TYPED_MEM_MAP_ALLOCATABLE
#define IMAP_GLOBAL						MAP_SYSRAM // creating a global vaddr

#endif

/* __SRCVERSION("pathmgr_object.h $Rev: 174147 $"); */
