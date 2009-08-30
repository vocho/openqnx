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




#include "kdserver.h"

#if defined(__BIGENDIAN__)
	#define NATIVE_ENDIAN	ELFDATA2MSB
#elif defined(__LITTLEENDIAN__)
	#define NATIVE_ENDIAN	ELFDATA2LSB
#else
	#error Endian not defined
#endif

union ehdr {
	Elf32_Ehdr		_32;
	Elf64_Ehdr		_64;
};

struct seg_info	{
	unsigned long		offset;
	unsigned long		len;
	paddr64_t			paddr;
};


static FILE				*corefp;
static struct seg_info	*seg;
static unsigned			num_segs;


unsigned	
core_read_paddr(paddr64_t paddr, void *p, unsigned len) {
	paddr64_t	start;
	paddr64_t	end;
	unsigned	i;
	unsigned	max;
	int			got;

	i = 0;
	for( ;; ) {
		if(i >= num_segs) return 0;
		start = seg[i].paddr;
		end = start + seg[i].len - 1;
		if((paddr >= start) && (paddr <= end)) break;
		++i;
	}
	max = end - paddr + 1;
	if(len > max) len = max;
	if(fseek(corefp, seg[i].offset + (unsigned long)(paddr - start), SEEK_SET) != 0) {
		if(debug_flag > 1) {
			fprintf(stderr, "readmem seek failed: %s\n", strerror(errno));
		}
		return 0;
	}
	got = fread(p, 1, len, corefp);
	if(got == -1) {
		if(debug_flag > 1) {
			fprintf(stderr, "readmem read failed: %s\n", strerror(errno));
		}
		return 0;
	}
	if(got != len) {
		if(debug_flag > 1) {
			fprintf(stderr, "readmem read got %d\n", got);
		}
	}
	return got;
}


unsigned	
core_read_vaddr(target_ptr vaddr, void *p, unsigned len) {
	paddr64_t	paddr;
	unsigned	valid;
	unsigned	total;
	unsigned	got;

	total = 0;
	do {
		if(!cpu->v2p(vaddr, &paddr, &valid)) break;
		if(valid > len) valid = len;
		got = core_read_paddr(paddr, p, valid);
		if(got == 0) break;
		total += got;
		vaddr += got;
		p = (uint8_t *)p + got;
		len -= got;
	} while(len != 0);
	return total;
}



static int
init_seginfo(union ehdr *ehdr, int big) {
	unsigned				num;
	unsigned long 			off;
	unsigned				i;
	unsigned				type;
	unsigned				note_size;
	struct seg_info			*sp;
	union {
		Elf32_Phdr	_32;
		Elf64_Phdr	_64;
	} 						phdr;

	if(big) {
		num = endian_native16(ehdr->_64.e_phnum);
		off = endian_native64(ehdr->_64.e_phoff);
	} else {
		num = endian_native16(ehdr->_32.e_phnum);
		off = endian_native32(ehdr->_32.e_phoff);
	}
	seg = malloc(num*sizeof(*seg));
	if(seg == NULL) {
		fprintf(stderr, "no memory for segment table\n");
		return 0;
	}
	if(fseek(corefp, off, SEEK_SET) != 0) {
		fprintf(stderr, "seek error: %s\n", strerror(errno));
		return 0;
	}
	note_size = 0;
	for(i = 0; i < num; ++i) {
		sp = &seg[num_segs];
		if(big) {
			if(fread(&phdr, sizeof(phdr._64), 1, corefp) != 1) {
				fprintf(stderr, "read error 1: %s\n", strerror(errno));
				return 0;
			}
			sp->paddr = endian_native64(phdr._64.p_paddr);
			sp->len   = endian_native64(phdr._64.p_filesz);
			sp->offset= endian_native64(phdr._64.p_offset);
			type      = endian_native32(phdr._64.p_type);
		} else {
			if(fread(&phdr, sizeof(phdr._32), 1, corefp) != 1) {
				fprintf(stderr, "read error 2: %s\n", strerror(errno));
				return 0;
			}
			sp->paddr = endian_native32(phdr._32.p_paddr);
			sp->len   = endian_native32(phdr._32.p_filesz);
			sp->offset= endian_native32(phdr._32.p_offset);
			type      = endian_native32(phdr._32.p_type);
		}
		if(type == PT_NOTE) {
			// remember for later...
			note_size = sp->len;
			off  = sp->offset;
		} else {
			++num_segs;
		}
	}
	if(note_size == 0) {
		fprintf(stderr, "No note segment found\n");
		return 0;
	}
	note = malloc(note_size);
	if(note == NULL) {
		fprintf(stderr, "No memory for note\n");
		return 0;
	}
	if(fseek(corefp, off, SEEK_SET) != 0) {
		fprintf(stderr, "seek error: %s\n", strerror(errno));
		return 0;
	}
	if(fread(note, note_size, 1, corefp) != 1) {
		fprintf(stderr, "read error 3: %s\n", strerror(errno));
		return 0;
	}
	if(note->note_version != NOTE_VERSION_CURRENT) {
		fprintf(stderr, "unsupported note version (%d)\n", note->note_version);
		return 0;
	}

	// Get everything to native endianness
	note->num_cpus = endian_native16(note->num_cpus);
	note->faulting_cpu = endian_native16(note->faulting_cpu);
	note->regset_size = endian_native32(note->regset_size);
	note->cpu_info = endian_native32(note->cpu_info);
	note->procnto_segnum = endian_native32(note->procnto_segnum);
	note->syspage_segnum = endian_native32(note->syspage_segnum);
	note->dumpinfo_segnum = endian_native32(note->dumpinfo_segnum);
	note_cpu = (void *)&note[1];
	for(i = 0; i < note->num_cpus; ++i) {
		note_cpu[i].pgtbl = endian_native64(note_cpu[i].pgtbl);
		note_cpu[i].cpunum = endian_native32(note_cpu[i].cpunum);
	}
	kdump_paddr = seg[note->dumpinfo_segnum].paddr;
	syspage_paddr = seg[note->syspage_segnum].paddr;

	return 1;
}
 

int
core_init(const char *name) {
	union ehdr		ehdr;
	unsigned		mach;
	unsigned		type;
	unsigned		i;
	int				big_elf;
	int				r;

	corefp = fopen(name, "rb");
	if(corefp == NULL) {
		fprintf(stderr, "Unable to open '%s'\n", name);
		return 0;
	}

	if(fread(&ehdr, sizeof(ehdr), 1, corefp) != 1) {
		fprintf(stderr, "read failure: %s\n", strerror(errno));
		return 0;
	}
	if(memcmp(&ehdr._32.e_ident, ELFMAG, SELFMAG) != 0) {
		fprintf(stderr, "not an ELF file\n");
		return 0;
	}

	switch(ehdr._32.e_ident[EI_DATA]) {
	case ELFDATA2MSB:	
	case ELFDATA2LSB:	
		cross_endian = (ehdr._32.e_ident[EI_DATA] != NATIVE_ENDIAN);
		break;
	default:
		fprintf(stderr, "not a recognized ELF endian\n");
		return 0;
	}

	switch(ehdr._32.e_ident[EI_CLASS]) {
	case ELFCLASS32:	
		mach = endian_native16(ehdr._32.e_machine);
		type = endian_native16(ehdr._32.e_type);
		big_elf = 0;
		break;
	case ELFCLASS64:	
		mach = endian_native16(ehdr._64.e_machine);
		type = endian_native16(ehdr._64.e_type);
		big_elf = 1;
		break;
	default:
		fprintf(stderr, "not a recognized ELF class\n");
		return 0;
	}
	if(type != ET_CORE) {
		fprintf(stderr, "not a recognized ELF type\n");
		return 0;
	}

	if(!init_seginfo(&ehdr, big_elf)) {
		return 0;
	}

	i = 0;
	for( ;; ) {
		cpu = available_cpus[i];
		if(cpu == NULL) {
			fprintf(stderr, "unsupported CPU type: %d\n", mach);
			return 0;
		}
		r = cpu->init(mach);
		if(r < 0) return 0;
		if(r > 0) break;
		++i;
	}

	pgtbl = malloc(cpu->pgtbl_size);
	if(pgtbl == NULL) {
		fprintf(stderr, "No memory for page table\n");
		return 0;
	}

	regset = malloc(note->regset_size);
	if(regset == NULL) {
		fprintf(stderr, "no memory for register set\n");
		return 0;
	}

	set_default_dump_state();

	return 1;
}


void
set_default_dump_state(void) {
	core_read_paddr(note_cpu->pgtbl, pgtbl, cpu->pgtbl_size);
	memcpy(regset, &note_cpu[1], note->regset_size);
}


void		
core_fini(void) {
	fclose(corefp);
}
