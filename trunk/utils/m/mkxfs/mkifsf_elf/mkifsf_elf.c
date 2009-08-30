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





#ifdef __USAGE
%C - mkifs filter program for creating ELF boot files

%C	[-Lpaddr_loc,entry_loc,vaddr_loc] [elf_machine_number] startup-offset image-file
#endif

#include <lib/compat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <sys/stat.h>
#include <unistd.h>
#include _NTO_HDR_(sys/elf.h)
#include _NTO_HDR_(sys/startup.h)


// Stuff the multiboot header into the elf wrapper so that we're acceptable
// to multiboot loaders. Doesn't hurt anything, so we just do it all the
// time.
struct multiboot_header {
	uint32_t	magic;
	uint32_t	flags;
	uint32_t	checksum;
	uint32_t	header_addr;
	uint32_t	load_addr;
	uint32_t	load_end_addr;
	uint32_t	bss_end_addr;
	uint32_t	entry_addr;
	uint32_t	mode_type;
	uint32_t	width;
	uint32_t	height;
	uint32_t	depth;
};

#define MB_HDR_MAGIC					0x1badb002

#define MB_HDR_FLAG_ALIGN_MODULES_4K	0x00000001
#define MB_HDR_FLAG_MUST_HAVE_MEMINFO	0x00000002
#define MB_HDR_FLAG_MUST_HAVE_VIDINFO	0x00000004
#define MB_HDR_FLAG_HAVE_ADDRINFO		0x00010000


#define MB_MY_FLAGS	(MB_HDR_FLAG_MUST_HAVE_MEMINFO)

#define MB_MY_CHECKSUM	(0 - (MB_HDR_MAGIC+MB_MY_FLAGS))

#define SWAP32( val ) ( (((val) >> 24) & 0x000000ff)	\
					  | (((val) >> 8)  & 0x0000ff00)	\
					  | (((val) << 8)  & 0x00ff0000)	\
					  | (((val) << 24) & 0xff000000) )

#define SWAP16( val ) ( (((val) >> 8) & 0x00ff)	\
					  | (((val) << 8) & 0xff00) )

int host_endian;
int target_endian;

long 
swap32(long val) {

	return(host_endian != target_endian ? SWAP32(val) : val);
}

int
swap16(int val) {

	return(host_endian != target_endian ? SWAP16(val) : val);
}

struct loc {
	int			adjust;
	int			abs;
};

static void
get_loc(struct loc *loc) {
	switch(*optarg) {
	case ',':
		++optarg;
		// fall through
	case '\0':
		return;
	case '+':
	case '-':
		break;
	default:
		loc->abs = 1;
		break;
	}
	loc->adjust = strtoul(optarg, &optarg, 0);
	if(*optarg != '\0') ++optarg;
}

static unsigned
adj_loc(struct loc *loc, unsigned val) {
	if(loc->abs) val = 0;
	return(loc->adjust + val);
}

int
main( int argc, char *argv[] ) {
	int							n;
	FILE						*fp;
	struct startup_header		shdr;
	struct elf_header {
		Elf32_Ehdr					e;
		Elf32_Phdr					p;
		Elf32_Shdr					s[3];
		struct multiboot_header		mb;
	}							hdr;
	struct stat					sbuf;
	char						*name;
	unsigned					startup_offset;
	unsigned					base;
	struct loc					paddr;
	struct loc					entry;
	struct loc					vaddr;

	// Calculate the endian of the host this program is running on.
	n = 1;
	host_endian = *(char *)&n != 1;

	memset(&paddr, 0, sizeof(paddr));
	memset(&entry, 0, sizeof(entry));
	memset(&vaddr, 0, sizeof(vaddr));
	while((n = getopt(argc, argv, "L:")) != -1) {
		switch(n) {
		case 'L':
			get_loc(&paddr);
			get_loc(&entry);
			get_loc(&vaddr);
			break;
		default:
			return(1);
		}
	}
	argc -= (optind-1);
	argv += (optind-1);

	if(argc <= 2) {
		fprintf(stderr, "Missing file name/startup offset.\n");
		return(1);
	}
	startup_offset = strtoul(argv[argc-2], NULL, 0);
	name = argv[argc-1];

	fp = fopen(name, "r+b");
	if(fp == NULL) {
		fprintf(stderr, "Can not open '%s': %s\n", name, strerror(errno));
		return(1);
	}
	if(fstat(fileno(fp), &sbuf) == -1) {
		fprintf(stderr, "Can not stat '%s': %s\n", name, strerror(errno));
		return(1);
	}
	if(fseek(fp, startup_offset, SEEK_SET) != 0) {
		fprintf(stderr, "Can not seek '%s': %s\n", name, strerror(errno));
		return(1);
	}
	if(fread(&shdr, sizeof(shdr), 1, fp) != 1) {
		fprintf(stderr, "Can not read from '%s': %s\n", name, strerror(errno));
		return(1);
	}
	if(fseek(fp, 0, SEEK_SET) != 0) {
		fprintf(stderr, "Can not rewind '%s': %s\n", name, strerror(errno));
		return(1);
	}

	target_endian = ((shdr.flags1 & STARTUP_HDR_FLAGS1_BIGENDIAN) != 0);

	memset(&hdr, 0, sizeof(hdr));
	memcpy(&hdr.e.e_ident, ELFMAG, SELFMAG);
	hdr.e.e_ident[EI_CLASS] = ELFCLASS32;
	hdr.e.e_ident[EI_DATA] = target_endian ? ELFDATA2MSB : ELFDATA2LSB;
	hdr.e.e_ident[EI_VERSION] = EV_CURRENT;
	hdr.e.e_type = swap16(ET_EXEC);
	if(argc > 3) {
		hdr.e.e_machine = swap16(strtoul(argv[argc-3], NULL, 0));
	} else {
		hdr.e.e_machine = shdr.machine;
	}
	hdr.e.e_version = swap32(EV_CURRENT);
	hdr.e.e_entry = swap32(adj_loc(&entry, swap32(shdr.startup_vaddr)));
	hdr.e.e_phoff = swap32(offsetof(struct elf_header, p));
	hdr.e.e_shoff = swap32(offsetof(struct elf_header, s));
	hdr.e.e_ehsize = swap16(sizeof(hdr.e));
	hdr.e.e_phentsize = swap16(sizeof(hdr.p));
	hdr.e.e_phnum = swap16(1);
	hdr.e.e_shentsize = swap16(sizeof(hdr.s[0]));
	hdr.e.e_shnum = swap16(3);
	hdr.e.e_shstrndx = swap16(2);

	hdr.p.p_type = swap32(PT_LOAD);
	hdr.p.p_offset = swap32(startup_offset);
	base = adj_loc(&paddr, swap32(shdr.image_paddr));
	hdr.p.p_vaddr = swap32(adj_loc(&vaddr, base + swap32(shdr.paddr_bias)));
	hdr.p.p_paddr = swap32(base);
	hdr.p.p_filesz = swap32(sbuf.st_size - startup_offset);
	hdr.p.p_memsz = hdr.p.p_filesz;
	hdr.p.p_flags = swap32(PF_X|PF_W|PF_R);
	hdr.p.p_align = swap32(startup_offset);

	hdr.s[1].sh_type = swap32(SHT_PROGBITS);
	hdr.s[1].sh_flags = swap32(SHF_WRITE|SHF_ALLOC|SHF_EXECINSTR);
	hdr.s[1].sh_addr = hdr.p.p_vaddr;
	hdr.s[1].sh_offset = hdr.p.p_offset = hdr.p.p_offset;
	hdr.s[1].sh_size = hdr.p.p_memsz;
	hdr.s[1].sh_addralign = hdr.p.p_align;

	hdr.s[2].sh_type = swap32(SHT_STRTAB);
	hdr.s[2].sh_flags = 0;
	hdr.s[2].sh_addr = 0;
	hdr.s[2].sh_offset = hdr.e.e_shoff;
	hdr.s[2].sh_size = swap32(1);
	hdr.s[2].sh_addralign = hdr.p.p_align;

	hdr.mb.magic = swap32(MB_HDR_MAGIC);
	hdr.mb.flags = swap32(MB_MY_FLAGS);
	hdr.mb.checksum = swap32(MB_MY_CHECKSUM);

	if(fwrite(&hdr, sizeof(hdr), 1, fp) != 1) {
		fprintf(stderr, "Can not update '%s': %s\n", name, strerror(errno));
		return(1);
	}
	if(fclose(fp) != 0) {
		fprintf(stderr, "Error closing '%s': %s\n", name, strerror(errno));
		return(1);
	}
	return(0);
}

__SRCVERSION("mkifsf_elf.c $Rev: 153052 $");
