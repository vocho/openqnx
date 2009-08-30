/**
 * This file provides the mechanisms for extracting the mapinfo data out 
 * of the processes in the system.
 */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/procfs.h>

#include "showmem.h"

typedef struct _thread_mem {
	int 		tid;
	uint64_t	stack_base;
	uint64_t	stack_size;
} thread_mem_t;


/**
 * Returns a memory allocated block for the name associated with this map block
 * or NULL if there is no name available.
 */
static char *get_name(int fd, uint64_t vaddr, procfs_debuginfo *map, int maplen) {
	map->vaddr = vaddr;
	map->path[0] = '\0';
	if(devctl(fd, DCMD_PROC_MAPDEBUG, map, maplen, 0) == -1) {
		return NULL;
	}
	
	return strdup(map->path);
}

/**
 * Return a simplified classification (TYPE_*) for this memory map block.
 */
static int classify_block(procfs_mapinfo *map) {
	int flags = map->flags;
	
	if(IS_STACK(flags)) {
		return TYPE_STACK;
	} else if(IS_ELF_CODE(flags)) {
		return TYPE_CODE;
	} else if(IS_ELF_SHARED_DATA(flags)) {
		return TYPE_UNKNOWN; /* temp kludge until we figure out what to do */
	} else if(IS_ELF_DATA(flags)) {
		return TYPE_DATA;
	} else if(IS_GENERIC_HEAP(flags)) {
		return TYPE_HEAP;
	}
	
	return TYPE_UNKNOWN;	
}

/** 
 * Locate the corresponding tid that has a stack region mapped in an overlapping
 * area with the specified target mapping.
 */
static int find_stack_tid(thread_mem_t *tidbases, int len, procfs_mapinfo *target) {
	int i;

	for(i = 0; i < len; i++) {
		//printf("Checking %llx -> %llx to %llx \n", 
		//	tidbases[i].stack_base, tidbases[i].stack_size, target->vaddr);
		if((target->vaddr >= tidbases[i].stack_base) && 
           (target->vaddr <= (tidbases[i].stack_base + tidbases[i].stack_size))) {
			//printf("Found %d \n", tidbases[i].tid);
			return tidbases[i].tid;
		}
	}

	return 0;
}


/**
 * Find the matching map block associated with this mapinfo block or NULL if no matching
 * map block is found.
 */
static procfs_mapinfo *find_elf_block(procfs_mapinfo *maps, int maplen, procfs_mapinfo *mi) {
	uint64_t before, after;
	int 	 i;

	/*	In 6.3.2+ we started breaking up data segments into fault on reference allocated
		regions that are private.  This means that the previous map might not be the
		ELF text, but rather a private (/dev/zero) or unamed non-private (/dev/mem) mapping.
		we use the dev/ino as a hint to find our 'owning' executable first if available.
		Comments in the OS suggest that this could change in the future <sigh>
	*/
	if((mi->dev == 4) && (mi->ino != mi->vaddr)) { /* the ino is the base object vaddr */
		for(i=0 ; i < maplen ; i++) {
			if(IS_ELF_CODE(maps[i].flags) && maps[i].vaddr == mi->ino) {
				return &maps[i];
			}
		}
	} 

	/* Otherwise we default to what we used to do here */
	before = mi->vaddr - 1;
	after = mi->vaddr + mi->size + 1;
	
	for(i = 0; i < maplen; i++) {
		if(!IS_ELF_CODE(maps[i].flags) && !IS_ELF_DATA(maps[i].flags)) {
			continue;
		}
		//@@@ Only one of these should be used based on loader knowledge
		//@@@ We should validate that they come from the same device
		if(before >= maps[i].vaddr && 
		   before <= (maps[i].vaddr + maps[i].size)) {
			return &maps[i];
		}
		if(after >= maps[i].vaddr && 
		   after <= (maps[i].vaddr + maps[i].size)) {
			return &maps[i];
		}
	}
	
	return NULL;
}

/*
 This code locates the matching shared entry that matches to the specified map info
 object.  In an ideal world, it would be a simple dev/inode/offset match but since
 we have to deal with filesystems that lie about their inodes (or their inodes change
 underfoot) we have to do some additional guessing.  We compensate for the inode
 mis-match by looking at the name and the size and if these match, then we will
 claim that the blocks are the same.  This suits our needs for ELF shared objects,
 but even checking the offset is kind of iffy for normal shared objects.
 */
static int find_shared_block(shared_entry_t *root, procfs_mapinfo *mi, char *name) {
	shared_memblock_t *p;
	int i;

	for(i = 0; i < root->block_count; i++) {
		p = &root->memblocks[i];
		//if(p->dev == mi->dev && p->ino == mi->ino && p->offset == mi->offset) {
		if(p->dev == mi->dev && p->offset == mi->offset && p->memblock.size == mi->size) {
			if((p->memblock.name == name) || 
			   (p->memblock.name != NULL && name != NULL && strcmp(p->memblock.name, name) == 0)) {
				if(verbose > 3) {
					printf("Shared mapping MATCH vaddr:0x%llx flags:0x%08x (%s)\n", 
							mi->vaddr, mi->flags, (name != NULL) ? name : "??");
				}
				return i;
			}   	
		} 
	}

	if(verbose > 3) {
		printf("Shared mapping MISS  vaddr:0x%llx flags:0x%08x (%s)\n", 
				mi->vaddr, mi->flags, (name != NULL) ? name : "??");
	}

	return -1;	
}

static int process_shared_block(shared_entry_t *shares, pid_t pid, procfs_mapinfo *map, char *name) {
	int				   block_index;
	shared_memblock_t *block;

	block_index = find_shared_block(shares, map, name);
	if(block_index < 0) {
		if(shares->block_count >= shares->block_size) {
			shares->memblocks = realloc(shares->memblocks, 
										(shares->block_size + 10) * sizeof(*shares->memblocks));
			if(shares->memblocks == NULL) {
				exit(0);	//BAD 
			}
			shares->block_size += 10;
		}

		block_index = shares->block_count;
		shares->block_count++;

		block = &shares->memblocks[block_index];										
		memset(block, 0, sizeof(*block));
		
		block->memblock.size = map->size;
		block->memblock.flags = map->flags;
		block->memblock.type = classify_block(map);
		block->memblock.name = name;
		block->dev = map->dev;
		block->offset = map->offset;
		block->ino = map->ino;
		block->pid = pid;	//First owner
	} else  {
		block = &shares->memblocks[block_index];										
		if(block->pid != pid) {
			block->pid = 0;		//Multiple owners
		}
	}
	
	return block_index;
}

static void writemapinfo(int fd, int pid, procfs_mapinfo *mp, int mpsize) {
	int i;
	char 	buffer[200];

	sprintf(buffer, "PID %d\n", pid);
	write(fd, buffer, strlen(buffer));

	for(i = 0; i < mpsize; i++) {
		sprintf(buffer, 
				" vaddr 0x%08llx size 0x%08llx flags 0x%08x dev 0x%08x offset 0x%08llx ino 0x%08llx \n",
	 			mp[i].vaddr,
	 			mp[i].size,
	 			mp[i].flags,
	 			mp[i].dev,
	 			mp[i].offset,
	 			mp[i].ino);
		write(fd, buffer, strlen(buffer));
	}
}

static void dumpdebuginfo(int pid, procfs_mapinfo *mp, int mpsize) {
	writemapinfo(debugfd, pid, mp, mpsize);
}

static void dumpmapinfo(int pid, procfs_mapinfo *mp, int mpsize) {
	fflush(stdout);
	writemapinfo(fileno(stdout), pid, mp, mpsize);
}

/**
 * For the specified fd (associated with a pid) extract all of the process memory
 * information.  Sort blocks into private and shared types and map private blocks 
 * with their appropriate information.
 * @@@ This routine does way too much memory allocation.  It should use a shared 
 * memory block and grow that block as required to gather the map information.
 */
static pid_entry_t *get_mem_info(int fd, shared_entry_t *shared_blocks) {
	pid_entry_t					 *pmb;
	procfs_mapinfo 				 *mp;
	int							  mpsize, privcnt;
	union {
		procfs_info					 info;
		procfs_status				 status;
	} 				procfs_info;
	struct {
		procfs_debuginfo 			info;
		char             			buff[_POSIX_PATH_MAX];
	}				map;
	int				num, i; 
	int 			num_threads;
	thread_mem_t *  thread_bases;
	

	pmb = calloc(1, sizeof(*pmb));
	if(pmb == NULL) {
		return NULL;
	}

	/* Extract the basic process information (base address */
	if (devctl(fd, DCMD_PROC_INFO, &procfs_info.info, sizeof(procfs_info.info), &num) == -1) {
		fprintf(stderr, "Can't get process info");
		return NULL;
	}
	pmb->pid = procfs_info.info.pid;
	pmb->base_addr = procfs_info.info.base_address;

	//Put this on the stack since we don't want to persist it
	num_threads = procfs_info.info.num_threads;
	thread_bases = alloca(num_threads * sizeof(*thread_bases));

	// Extract the thread stack bases 
	for (i = 0, procfs_info.status.tid = 1; i < num_threads; i++) {
		if (devctl(fd, DCMD_PROC_TIDSTATUS, &procfs_info.status, sizeof(procfs_info.status), &num) == -1) {
			fprintf(stderr, "Can't get thread info");
			return NULL;
		} 
		thread_bases[i].tid = procfs_info.status.tid;
		thread_bases[i].stack_base = procfs_info.status.stkbase;
		thread_bases[i].stack_size = procfs_info.status.stksize;
		procfs_info.status.tid++;
	}

	// Extract the process page mappings 
	mp = NULL;
	mpsize = 0;
	do {
		if(devctl(fd, DCMD_PROC_PAGEDATA, mp, mpsize, &num) != EOK) {
			free(pmb);
			return NULL;
		}	

		if(num <= mpsize/sizeof(*mp)) {
			break;
		}

		mpsize = num * sizeof(*mp);
		if(!(mp = realloc(mp, mpsize))) {
			free(pmb);			//Potential leak on failure
			return NULL;
		}
	} while(1);

	if(debugfd != -1) {
		dumpdebuginfo(pmb->pid, mp, num);
	}

	if(verbose > 2) {
		dumpmapinfo(pmb->pid, mp, num);
	}
	
	
	// Determine how many blocks are shared, how many are private
	for(privcnt = 0, i = 0; i < num; i++) {
		if((mp[i].flags & PG_HWMAPPED) && !IS_SHARED(mp[i].flags) && !IS_ELF_SHARED_DATA(mp[i].flags)) {
			privcnt++;
		}
	}

	// Decode the specific blocks into shared and private information
	pmb->block_count = privcnt;
	pmb->memblocks = calloc(privcnt, sizeof(*pmb->memblocks));

	for(privcnt = 0, i = 0; i < num; i++) {
		char *name;
		
		if(!(mp[i].flags & PG_HWMAPPED)) {
			continue;
		}

		// Assume the name of the process is the name of the executable		
		name = get_name(fd, mp[i].vaddr, &map.info, sizeof(map));
		if(pmb->base_addr >= mp[i].vaddr && 
		   pmb->base_addr <= (mp[i].vaddr + mp[i].size-1)) {
			pmb->name = strdup(name);
		}

		if(IS_SHARED(mp[i].flags) || IS_ELF_SHARED_DATA(mp[i].flags)) {
			process_shared_block(shared_blocks, pmb->pid, &mp[i], name);
		} else {
			pmb->memblocks[privcnt].memblock.size = mp[i].size;
			pmb->memblocks[privcnt].memblock.flags = mp[i].flags;
			pmb->memblocks[privcnt].memblock.type = classify_block(&mp[i]);
			pmb->memblocks[privcnt].memblock.name = name;
			pmb->memblocks[privcnt].vaddr = mp[i].vaddr;

			switch(pmb->memblocks[privcnt].memblock.type) {
			case TYPE_STACK:
				pmb->memblocks[privcnt].tid = find_stack_tid(thread_bases, num_threads, &mp[i]);
				break; 
			case TYPE_CODE:
			case TYPE_DATA: {
				procfs_mapinfo *data = find_elf_block(mp, num, &mp[i]);
				if(data != NULL && IS_SHARED(data->flags) && (data->flags & PG_HWMAPPED)) {
					struct {
						procfs_debuginfo            info;
						char                        buff[_POSIX_PATH_MAX];
					} elf_block_map;
							
					if(name) free(name);
					name = get_name(fd, data->vaddr, &elf_block_map.info, sizeof(elf_block_map));
					//Jump the queue and process the block now, name should be the same
					if((pmb->memblocks[privcnt].shared_index=find_shared_block(shared_blocks, data, name)) == -1) {
						/* shared object this priv region belongs too isn't processed yet, do it */
						pmb->memblocks[privcnt].shared_index = process_shared_block(shared_blocks, pmb->pid, data, name);
					}
					pmb->memblocks[privcnt].memblock.name = name;
				} 
				break;
			}
			}
			
			privcnt++;
		}
	}
	
	//Get rid of the extra memory
	free(mp);

	return pmb;
}

system_entry_t *build_block_list(int *pidlist, int pidnum) {
	int  			i, fd;
	DIR 			*dp = NULL;
	struct dirent 	*dent = NULL;
	system_entry_t	*system;
	pid_entry_t 	*insert = NULL;
	char 			namebuffer[30];

	system = calloc(1, sizeof(*system));
	
	if ((pidlist == NULL) || (pidnum == 0)) {
		dp = opendir("/proc");
		if(dp == NULL) {
			return NULL;
		}
		
		while((dent = readdir(dp)) != NULL) {
			pid_entry_t *next = NULL;	
		
			if(!isdigit(*dent->d_name)) {
				continue;
			}
			
			sprintf(namebuffer, "/proc/%s", dent->d_name);
			if((fd = open(namebuffer, O_RDONLY)) == -1) {
				fprintf(stderr, "Failed to open %s", namebuffer);
				continue;
			}
			
			next = get_mem_info(fd, &system->shared_blocks);
			close(fd);
			
			if(next == NULL) {
				continue;
			}

			if(system->pid_list == NULL) {
				system->pid_list = next;
			} else {
				insert->next = next;
			}
			insert = next;
		}

		closedir(dp);
	} else {
		for (i=0;i<pidnum;i++) {
			pid_entry_t *next = NULL;	

			sprintf(namebuffer, "/proc/%d", pidlist[i]);
			if ((fd = open(namebuffer, O_RDONLY)) == -1) {
				fprintf(stderr, "Failed to open %s", namebuffer);
				continue;
			}

			next = get_mem_info(fd, &system->shared_blocks);
			close(fd);

			if (next == NULL) {
				continue;
			}

			if(system->pid_list == NULL) {
				system->pid_list = next;
			} else {
				insert->next = next;
			}
			insert = next;
		}
	}
	
	return system;	
}
                 
