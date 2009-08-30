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




#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <malloc.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <dlfcn.h>
#include <sys/procfs.h>
#include <sys/elf.h>
#include <sys/link.h>
#include <time.h>

struct page {
	struct page *next;
	unsigned start, end;
} *pages = NULL;
int npages;

static int read_memory(int fd, unsigned offset, void  *buf, size_t size)
{
	struct page *page;
	unsigned page_offset = offset & ~(__PAGESIZE-1);

	while (page_offset < (offset+size)) {
		for (page = pages; page != NULL; page = page->next) {
			if (page->start <= page_offset && page->end >= page_offset + __PAGESIZE)
				break;
		}
		if (page == NULL) {
			page = malloc(sizeof(*page));
			if (page == NULL) 
				return -1;
			page->start = page_offset;
			page->end = page_offset + __PAGESIZE;
			page->next = pages;
			pages = page;
			npages++;
		}
		page_offset += __PAGESIZE;
	}	

	if (buf == NULL) {
		return 0;
	}

	if (lseek(fd, offset, SEEK_SET) == -1) {
		return -1;
	}

	return read(fd, buf, size);
}

/*
 * Read in an ehdr and the phdrs from the address space of the process.
 * Return a file descriptor or -1 on error. Space allocated for phdrs iff
 * returning successfully.  
 */
static int get_headers(int fd, Elf32_Ehdr * ehdr, Elf32_Phdr ** phdr)
{
	procfs_info     info;

	errno = devctl(fd, DCMD_PROC_INFO, &info, sizeof info, 0);
	if (errno != EOK) {
		return -1;
	}

	/*
	 * Read in elf header.  
	 */
	if (read_memory( fd, info.base_address, ehdr, sizeof *ehdr) < sizeof *ehdr) {
		return -1;
	}

	/*
	 * Allocate space and read the phdrs.  
	 */
	*phdr = malloc(ehdr->e_phnum * ehdr->e_phentsize);
	if (!*phdr) {
		return -1;
	}

	if (read_memory(fd, info.base_address + ehdr->e_phoff, *phdr, ehdr->e_phnum * ehdr->e_phentsize)
		< ehdr->e_phnum * ehdr->e_phentsize) {
		free(*phdr);
		return -1;
	}

	return fd;
}

/*
 * Fill lm with the first struct link_map in the process' address space.
 * Returns a file descriptor for future calls, -1 on error.  
 */
static int lm_first(struct link_map *lm, int fd)
{
	Elf32_Ehdr      ehdr;
	Elf32_Phdr     *phdr;
	struct r_debug  r_debug;
	int             n;

	/*
	 * Read the headers.  They're handy.  
	 */
	if (get_headers(fd, &ehdr, &phdr) == -1) {
		return -1;
	}

	/*
	 * First we need the r_debug structure which has a pointer to the
	 * process's link_map.  
	 */
	for (n = 0; n < ehdr.e_phnum; n++) {
		/*
		 * Find the dynamic sections in the phdrs.  
		 */
		if (phdr[n].p_type == PT_DYNAMIC) {
			Elf32_Dyn       d;

			do {
				if (read_memory(fd, phdr[n].p_vaddr, &d, sizeof d) < sizeof d) {
					free(phdr);
					return -1;
				}
#ifdef __MIPS__
				if ( d.d_tag == DT_MIPS_RLD_MAP ) {
					unsigned mips_rld_map;

					/* On MIPS, the DT_MIPS_RLD_MAP entry points to what normally would be the DT_DEBUG pointer, so an extra indirection is needed */
					if (read_memory(fd, d.d_un.d_ptr, &mips_rld_map, sizeof mips_rld_map) < sizeof mips_rld_map) {
						free(phdr);
						return -1;
					}
					if (read_memory(fd, mips_rld_map, &r_debug, sizeof r_debug) < sizeof r_debug) {
						free(phdr);
						return -1;
					}
					break;
				}
#else /* !__MIPS__ */
				/*
				 * The DT_DEBUG section is the one that has the r_debug
				 * structure.  
				 */
				if (d.d_tag == DT_DEBUG) {
					if (read_memory(fd, d.d_un.d_ptr, &r_debug, sizeof r_debug) < sizeof r_debug) {
						free(phdr);
						return -1;
					}
					break;
				}
#endif /* !__MIPS__ */
				phdr[n].p_vaddr += sizeof(d);
			}
			while (d.d_tag != DT_NULL);
			break;
		}
	}

	if (r_debug.r_version != R_DEBUG_VERSION) {
		errno = EINVAL;
		free(phdr);
		return -1;
	}

	if (read_memory(fd, (unsigned)r_debug.r_map, lm, sizeof *lm) < sizeof *lm) {
		free(phdr);
		return -1;
	}

	free(phdr);
	return fd;
}

static int lm_next(struct link_map *lm, int fd)
{
	if (lm->l_next) {
		if (read_memory(fd, (unsigned)lm->l_next, lm, sizeof *lm) < sizeof *lm)
			return 0;
		return 1;
	}
	return 0;
}

static void free_pages()
{
	struct page *page;
	while (pages)
	{
		page = pages;
		pages = pages->next;
		free(page);
	}
	npages=0;
}

int get_ldd_mapinfos(int fd, procfs_mapinfo **infos, int *ninfos)
{
	int             i;
	struct link_map lm;
	procfs_mapinfo	*mapinfos;
	struct page	*page;

	*ninfos = 0;
	*infos = NULL;

	if (lm_first(&lm, fd) == -1) {
		perror("Unable to find link_map.");
		free_pages();
		return -1;
	}

	do {
		if (lm.l_name) {
			read_memory(fd, (unsigned)lm.l_name, NULL, 1024);
		}
		if (lm.l_path) {
			read_memory(fd, (unsigned)lm.l_path, NULL, 1024);
		}
	}
	while (lm_next(&lm, fd));

	mapinfos = calloc(npages, sizeof(procfs_mapinfo));
	if (mapinfos == NULL) {
		perror("mapinfos");
		free_pages();
		return -1;
	}
	for (page = pages, i = 0; page != NULL; i++, page = page->next) {
		/* HACK we have to hack up the MAP_STACK so that dumper will allow this segment to written out */
		mapinfos[i].flags = MAP_STACK|MAP_PRIVATE|MAP_ANON|PROT_WRITE|PROT_READ|PG_HWMAPPED;
		mapinfos[i].vaddr = page->start;
		mapinfos[i].offset = page->start;
		mapinfos[i].size = page->end - page->start;
	}

	*infos = mapinfos;
	*ninfos = npages;
	free_pages();
	return 0;
}
