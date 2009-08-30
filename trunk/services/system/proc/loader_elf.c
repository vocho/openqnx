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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <share.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/elf.h>
#include <sys/elf_386.h>
#include <sys/elf_notes.h>
#include <sys/elf_nto.h>
#include <sys/auxv.h>
#include <sys/mman.h>
#include <libgen.h>
#include "externs.h"
#include "cpudeps.h"


struct seg {
	uintptr_t				addr;
	unsigned				size;
	unsigned				filesz;
	int						reloc;
	int						prot;
	int						read_in;
};


static void 
debug_info(const void *vaddr, const Elf32_Phdr *phdr, int fd, const char *name) {
	mem_debug_info_t			msg;
	iov_t						iov[2];

	msg.i.type = _MEM_DEBUG_INFO;
	msg.i.zero = msg.i.reserved = 0;
	msg.i.offset = phdr->p_offset;
	msg.i.ino = 0;
	msg.i.vaddr = (uintptr_t)vaddr;
	msg.i.size = phdr->p_memsz;
	msg.i.flags = 0;
	msg.i.dev = 0;
	msg.i.old_vaddr = phdr->p_vaddr;
	SETIOV(iov + 0, &msg.i, offsetof(mem_debug_info_t, i.path));
	SETIOV(iov + 1, name, strlen(name) + 1);
	
	MsgSendv(MEMMGR_COID, iov, 2, 0, 0);
}

int 
elf_load(int fd, const char *path, struct loader_startup *lsp, struct stat *statlocal, struct inheritance *parms) {
	Elf32_Ehdr				ehdr;
	Elf32_Phdr				*phdr, *phdrs;
	uintptr_t				base_addr;
	int						base_reloc;
	char					*addr;
	auxv_t					*auxv;
	int						i;
	unsigned				size;
	int						prot;
	int						flags;
	char					*interp;
	struct seg				*seg;
	int						text_seg, data_seg;
	uintptr_t				mem_end;
	uintptr_t				fil_end;
	char					buffer[512];
	Elf32_Segrel			*sr = 0;
	struct {
		Elf32_Nhdr				nhdr;
		char					strings[1];
	}						*note = (void *)buffer;
	struct {
		struct _proc_spawn_debug		i;
		char							filler[_POSIX_PATH_MAX];
	}						debug_msg;


	if (strlen(path) > ((sizeof(debug_msg) - offsetof(struct _proc_spawn_debug, name)) - 1)) {
		return(ENAMETOOLONG);
	}

	//RUSH3: Rather than doing proc_read()'s in this function, we could mmap() 
	//RUSH3: the file. More expensive if the executable's not already loaded,
	//RUSH3: less if it is. Maybe look at sticky bit?
	i = proc_read(fd, &ehdr, sizeof ehdr, 0);
	if (i <= 0) {
		// Unable to read header - return error so we don't try interpreter
		return ENOEXEC;
	}
	if(i != sizeof ehdr
		|| memcmp(ehdr.e_ident, ELFMAG, SELFMAG) 
		|| ehdr.e_ident[EI_DATA] != ELFDATANATIVE 
		|| ehdr.e_phentsize != sizeof *phdr) {
		// Not an ELF file - try executing interpreter if necessary
		return -1;
	}
	switch(ehdr.e_machine) {
	case CPU_ELF_NUMBERS:
		break;
	default:
		return ENOEXEC;
	}
	switch(ehdr.e_type) { 
	case ET_DYN:
		return ELIBEXEC;
	case ET_EXEC:
		break;
	default:
		return ENOEXEC;
	}

	if(!(phdrs = alloca(sizeof *phdrs * ehdr.e_phnum)) || !(seg = alloca(sizeof *seg * ehdr.e_phnum))) {
		return ENOEXEC;
	}
	memset(seg, 0x00, sizeof *seg * ehdr.e_phnum);
	memset(&debug_msg, 0x00, sizeof debug_msg);

	auxv = 0;
	interp = 0;

	if(lsp) {
		auxv = lsp->aux;
		while(auxv->a_type != AT_NULL) {
			auxv++;
		}
		lsp->stackalloc = 0;
		if(SYSPAGE_ENTRY(cpuinfo)->flags & CPU_FLAG_MMU) {
			lsp->stacksize = DEF_VIRTUAL_FIRST_THREAD_STACKSIZE; //
		} else {
			lsp->stacksize = DEF_PHYSICAL_FIRST_THREAD_STACKSIZE;
		}
		lsp->stackaddr = 0x80000000;
	}

	size = ehdr.e_phnum * sizeof(*phdrs);
	if(proc_read(fd, phdrs, size, ehdr.e_phoff) != size) {
		return ENOEXEC;
	}

	base_reloc = 0;
	base_addr = ~0;
	text_seg = data_seg = -1;
	for(i = 0; i < ehdr.e_phnum; i++) {
		phdr = phdrs + i;
		switch(phdr->p_type) {	
		case PT_LOAD:
			if(phdr->p_filesz > phdr->p_memsz) {
				return ENOEXEC;
			}

			if(phdr->p_flags & PF_W) {
				if(data_seg == -1) data_seg = i;
				flags = MAP_ELF | MAP_FIXED | MAP_PRIVATE | MAP_NOSYNCFILE;
				prot = PROT_WRITE;
			} else {
				flags = MAP_ELF | MAP_FIXED | MAP_SHARED | MAP_NOSYNCFILE;
				prot = 0;
			}
			if(phdr->p_flags & PF_X) {
				if(text_seg == -1) text_seg = i;
				prot |= PROT_EXEC;
			}
			if(phdr->p_flags & PF_R) prot |= PROT_READ;
			seg[i].addr = phdr->p_vaddr;
			seg[i].size = phdr->p_memsz;
			seg[i].filesz = phdr->p_filesz;
			seg[i].prot = prot;

			// mmap does most of the work for us
			if(lsp != NULL) {
				addr = mmap((void *)phdr->p_vaddr, phdr->p_filesz, prot, flags, fd, phdr->p_offset);
				if(addr == MAP_FAILED) {
					if((errno != EINVAL) || ((flags & MAP_TYPE) != MAP_PRIVATE)) {
						return errno;
					}
					// This segment was marked as "copy"
					addr = mmap((void *)phdr->p_vaddr, phdr->p_memsz, prot | PROT_WRITE, MAP_PRIVATE|MAP_ANON|MAP_ELF|MAP_FIXED, NOFD, 0);
					if(addr == MAP_FAILED) {
						return errno;
					}
					// Now read in the data
					if(proc_read(fd, addr, phdr->p_filesz, phdr->p_offset)  != phdr->p_filesz) {
						return ENOEXEC;
					}
					seg[i].read_in = 1;
				} else {
					// Init BSS area
					fil_end = ROUNDUP(phdr->p_vaddr + phdr->p_filesz, __PAGESIZE);
					mem_end = ROUNDUP(phdr->p_vaddr + phdr->p_memsz, __PAGESIZE);
					if(mem_end > fil_end) {
						// allocate remainder of pages
						if(mmap((void *)fil_end, mem_end-fil_end, prot, (MAP_FIXED|MAP_ANON)|(flags & MAP_TYPE), NOFD, 0) == MAP_FAILED) {
							return errno;
						}
					}
				}
				seg[i].reloc = (unsigned)addr - phdr->p_vaddr;
				if(base_addr == ~0U) {
					base_addr = seg[i].addr;
					base_reloc = seg[i].reloc;
				}
			} else {
				// This is a special hack to let the memmgr code know where
				// the procnto code and data is, so it can report that back
				// in vmm_mapinfo(). The MAP_SYSRAM flag is never allowed
				// to be passed into the mmap() function, so we can use
				// that as the key about what's going on.
				void	*vaddr;
				size_t	lsize;

				(void)memmgr.mmap(NULL, (phdr->p_vaddr), (phdr->p_memsz), prot, 
						MAP_PHYS+MAP_SHARED+MAP_SYSRAM, NULL, 0, 0, 0, NOFD,
						&vaddr, &lsize, part_id_t_INVALID);
			}
			break;

		case PT_NOTE:
			if(0==phdr->p_filesz) break; //ignore zero-len .note headers. But return ENOEXEC if they are malformed sizes.
			if(phdr->p_filesz >= sizeof *note)  {
				size = min(phdr->p_filesz, sizeof buffer);
				if(proc_read(fd, note, size, phdr->p_offset) == size) {
					while(size > 0) {
						int				len;
						Elf32_Word		*p;

						if(note->nhdr.n_namesz == sizeof QNX_NOTE_NAME && !strcmp(note->strings, QNX_NOTE_NAME)) {
							p = (Elf32_Word *)&note->strings[(note->nhdr.n_namesz + 3) & ~3];
							switch(note->nhdr.n_type) {
							case QNT_STACK:
								if(lsp) {
									/* stacksize = size of stack, stackalloc = amount to alloc upfront (ie. fault in) */
									lsp->stackalloc = lsp->stacksize = *p;
									if((note->nhdr.n_descsz > sizeof(Elf32_Word)) && *(++p)) {
										lsp->flags |= PROC_LF_LAZYSTACK;
										/*	AM Note:
											set prealloc amount to specified value in elf note section
											rounded by page because ldrel has a bug where the amount to
											prealloc is set to 1 rather than a byte value to prealloc
										 */
										lsp->stackalloc= *p+(__PAGESIZE-1) & ~(__PAGESIZE-1);
									}
								}
								break;


							case QNT_DEBUG_FULLPATH:
								break;

							case QNT_DEBUG_RELOC:
								if(fpathconf(fd, _PC_IMAGE_VADDR) != -1) {
									if(text_seg != -1) {
										seg[text_seg].reloc = p[text_seg];
									}
									if(data_seg != -1) {
										seg[data_seg].reloc = p[data_seg];
									}
								}
								break;
	
							default:
								break;
							}
						}
						len = sizeof note->nhdr + ((note->nhdr.n_namesz + 3) & ~3) + ((note->nhdr.n_descsz + 3) & ~3);
						note = (void *)((char *)note + len);
						if(len > size) {
							break;
						}
						size -= len;
					}
				} else { 
					//PR61735 reject truncated reads, even for .note headers
					return ENOEXEC;   
				}
			} else { 
				//PR61735 too small to be a proper .note header 
				return ENOEXEC;
			}
			break;

		case PT_SEGREL:
			if ( phdr->p_filesz < sizeof(*sr) ) {
				break;
			}
			size = min(phdr->p_filesz, sizeof buffer); 
			if(proc_read(fd, buffer, size, phdr->p_offset) != size) {
				return ENOEXEC;
			} else {
				struct seg				*curseg;
				Elf32_Segrel			*segaddr, *segent;
				int						addr_num, addr_ndx;
				int						entoff, entend;
				unsigned				addrlocal;

				sr = (void *)buffer;
				curseg = &seg[sr->sr_un.sr_hdr.srh_phndx];
				if(curseg->size == 0) {
					return ENOEXEC;
				}
				if(curseg->reloc == 0) {
					break;
				}
				addrlocal =  (curseg->addr - curseg->reloc) & ~(__PAGESIZE - 1);
				addr_num = (ROUNDUP((curseg->addr - curseg->reloc) + curseg->filesz, __PAGESIZE) - addrlocal) / __PAGESIZE + 1;

				entend = sizeof buffer / sizeof *sr;
				entoff = addr_num + 1;
				if(size != phdr->p_filesz && addr_num > (sizeof buffer / sizeof *sr) / 4 + 1) {
					entoff -= addr_num - ((sizeof buffer / sizeof *sr) / 4 + 1);
				}
				segent = &sr[entoff];

				addr_ndx = 1;
				segaddr = sr + 1;
				
				while(segaddr->sr_un.sr_addr.sra_skip != (Elf32_Half)-1) {
					if(segaddr->sr_un.sr_addr.sra_skip == 0) {
						int					ent;

						for(ent = segaddr->sr_un.sr_addr.sra_index; ent < segaddr[1].sr_un.sr_addr.sra_index; ent++) {
							int					*p;

							if(ent >= entend) {
								unsigned		num;

								num = min(phdr->p_filesz - ent * sizeof *sr,
											sizeof buffer - (segent - sr) * sizeof *sr);
								if(proc_read(fd, segent, num, phdr->p_offset + ent * sizeof *sr) != num) {
									return ENOEXEC;
								}
								entoff = ent;
								entend = ent + num / sizeof *sr;
							}
							p = (int *)(addrlocal + segent[ent - entoff].sr_un.sr_ent.sre_offset + curseg->reloc);

							*p += seg[segent[ent - entoff].sr_un.sr_ent.sre_phndx].reloc;
						}
					}
					if(--addr_num <= 1) {
						break;
					}
					addr_ndx++;
					addrlocal += __PAGESIZE;
					if(++segaddr >= segent - 1) {
						int						num;

						num = min(addr_num, segent - sr) * sizeof *sr;
						if(proc_read(fd, sr, num, phdr->p_offset + addr_ndx * sizeof *sr) != num) {
							return ENOEXEC;
						}
						segaddr = sr;
					}
				}
			}
			break;

		case PT_PHDR:
			if(auxv) {
				auxv->a_type = AT_PHDR;
				auxv->a_un.a_ptr = (void *)phdr->p_vaddr;
				auxv++;
				auxv->a_type = AT_PHENT;
				auxv->a_un.a_val = ehdr.e_phentsize;
				auxv++;
				auxv->a_type = AT_PHNUM;
				auxv->a_un.a_val = phdr->p_memsz / ehdr.e_phentsize;
				auxv++;
			}
			break;

		case PT_INTERP:
			if(auxv) {
				interp = (char *)phdr->p_vaddr;
				auxv->a_type = AT_ENTRY;
				auxv->a_un.a_fcn = (void (*)(void))ehdr.e_entry;
				auxv++;
			}
			break;


		default:
			break;
		}
	}

	if(text_seg == -1 || data_seg == -1) {
		return ENOEXEC;
	}

	if(lsp) {
		for(i = 0; i < ehdr.e_phnum; i++) {
			//RUSH3: Don't think the .size!=0 test is needed anymore
			if((seg[i].size != 0) && seg[i].read_in) {
				if((seg[i].prot & PROT_WRITE) == 0) {
					mprotect((void *)seg[i].addr, seg[i].size, seg[i].prot);
				}
				if(seg[i].prot & PROT_EXEC) {
					(void) msync((void *)seg[i].addr, seg[i].filesz, MS_INVALIDATE_ICACHE);
				}
			}
		}

		lsp->eip = ehdr.e_entry + base_reloc;
	}

	if(lsp && interp) {
		struct stat			interpstat;
		int					fd2;

		CRASHCHECK(auxv == NULL);

		if(seg[text_seg].reloc) {
			auxv_t				*p;

			interp += seg[text_seg].reloc;
			for(p = lsp->aux; p < auxv; p++) {
				switch(p->a_type) {
				case AT_ENTRY:
				case AT_PHDR:
					p->a_un.a_ptr = (char *)p->a_un.a_ptr + seg[text_seg].reloc;				
					break;			
				default: break;
				}
			}
		}
		if(seg[text_seg].reloc != seg[data_seg].reloc) {
			auxv->a_type = AT_DATA;
			auxv->a_un.a_ptr = (void *)seg[data_seg].addr;
			auxv++;
		}
		auxv->a_type = AT_PAGESZ;
		auxv->a_un.a_val = memmgr.pagesize;
		auxv++;
		if((fd2 = sopen(interp, O_RDONLY, SH_DENYWR)) == -1) {
			return errno == EBUSY ? ETXTBSY : ELIBACC;
		}

		if(proc_read(fd2, &ehdr, sizeof ehdr, 0) != sizeof ehdr || memcmp(ehdr.e_ident, ELFMAG, SELFMAG) ||
					ehdr.e_ident[EI_DATA] != ELFDATANATIVE || ehdr.e_phentsize != sizeof *phdr) {
			close(fd2);
			return ELIBBAD;
		}
		if(ehdr.e_phnum * sizeof *phdrs > sizeof buffer) {
			close(fd2);
			return ELIBBAD;
		}
		phdrs = (Elf32_Phdr *)buffer;
		if(proc_read(fd2, phdrs, sizeof *phdrs * ehdr.e_phnum, ehdr.e_phoff) != sizeof *phdrs * ehdr.e_phnum) {
			close(fd2);
			return ELIBBAD;
		}

		base_addr = ~0;
		for(i = 0, phdr = phdrs; i < ehdr.e_phnum; i++, phdr++) {
			if(phdr->p_type == PT_LOAD) {
				void				*vaddr;
				uintptr_t			start;

				start = phdr->p_vaddr;
				if(phdr->p_flags & PF_W) {
					if(base_addr == ~0U) {
						close(fd2);
						return ELIBBAD;
					}
					//RUSH1: prot should obey the phdr->p_flags permissions
					prot = PROT_READ|PROT_WRITE|PROT_EXEC;
					flags = MAP_ELF|MAP_PRIVATE|MAP_FIXED|MAP_NOSYNCFILE;
					start += base_reloc;
				} else {
					//RUSH1: prot should obey the phdr->p_flags permissions
					prot = PROT_READ|PROT_EXEC;
					flags = MAP_ELF|MAP_SHARED|MAP_NOSYNCFILE;
					if(start == 0) {
						start = ROUNDDOWN(CPU_LIBC_BASE, __PAGESIZE);
						start |= phdr->p_offset & (__PAGESIZE - 1);
					}
				}
				vaddr = mmap((void *)start, phdr->p_filesz, prot, flags, 
									fd2, phdr->p_offset);
				if(vaddr == MAP_FAILED) {
					int		orig_errno = errno;

					close(fd2);
					// Return appropriate code if out of memory...
					return (orig_errno == ENOMEM) ? ENOMEM : ELIBBAD;
				}
				if(base_addr == ~0U) {
					base_addr = (uintptr_t)vaddr;
					base_reloc = (uintptr_t)vaddr - phdr->p_vaddr;
					start = (uintptr_t)vaddr;
				}
				// Init BSS area
				fil_end = ROUNDUP(start + phdr->p_filesz, __PAGESIZE);
				mem_end = ROUNDUP(start + phdr->p_memsz, __PAGESIZE);
				if(mem_end > fil_end) {
					//allocate remainder of pages
					if(mmap((void *)fil_end, mem_end - fil_end, prot, 
								(MAP_ANON|MAP_FIXED)|(flags & MAP_TYPE), NOFD, 0) == MAP_FAILED) {
						int		orig_errno = errno;

						close(fd2);
						// Return appropriate code if out of memory...
						return (orig_errno == ENOMEM) ? ENOMEM : ELIBBAD;
					}
				}
				debug_info(vaddr, phdr, fd2, basename(interp));
			}
		}
		auxv->a_type = AT_BASE;
		auxv->a_un.a_ptr = (void *)base_addr;
		auxv++;

		/* Stuff the aux vector with the DEVICE/INODE of the interpreter */
		interpstat.st_dev = 0;
		interpstat.st_ino = 0;
		(void) fstat(fd2, &interpstat);
		auxv->a_type = AT_INTP_DEVICE;
		auxv->a_un.a_val = interpstat.st_dev;
		auxv++;
		auxv->a_type = AT_INTP_INODE;
		auxv->a_un.a_val = interpstat.st_ino;
		auxv++;
		
		close(fd2);

		lsp->eip = ehdr.e_entry + base_reloc;
	}
		
	if(auxv) {
		auxv->a_type = AT_NULL;
		auxv->a_un.a_val = 0;
	}

	debug_msg.i.type = _PROC_SPAWN;
	debug_msg.i.subtype = _PROC_SPAWN_DEBUG;
	debug_msg.i.text_addr = seg[text_seg].addr;
	debug_msg.i.text_size = seg[text_seg].size;
	debug_msg.i.text_reloc = 0;
	debug_msg.i.data_addr = seg[data_seg].addr;
	debug_msg.i.data_size = seg[data_seg].size;
	debug_msg.i.data_reloc = 0;
	if(!debug_msg.i.name[0]) {
		while(*path == '/') {
			path++;
		}
		STRLCPY(debug_msg.i.name, path, sizeof(debug_msg) - offsetof(struct _proc_spawn_debug, name));
	}
	//RUSH2: This is to set the fdmem.name field in the object. Do we
	//RUSH2: really need to do that - can we query the file name
	//RUSH2: from the open fd?
	(void) MsgSend(MEMMGR_COID, &debug_msg.i, offsetof(struct _proc_spawn_debug, name) + strlen(debug_msg.i.name) + 1, 0, 0);

	return EOK;
}

__SRCVERSION("loader_elf.c $Rev: 202757 $");
