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
%C - mkifs filter program for creating COFF boot files

%C	[coff_magic_number] startup-offset image-file
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
#include <sys/stat.h>

#include _NTO_HDR_(sys/startup.h)

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

struct filehdr {
	unsigned short	f_magic;	/* magic number */
	unsigned short	f_nscns;	/* number of sections */
	long			f_timdat;	/* time & date stamp */
	long			f_symptr;	/* file pointer to symtab */
	long			f_nsyms;	/* number of symtab entries */
	unsigned short	f_opthdr;	/* sizeof(optional hdr) */
	unsigned short	f_flags;	/* flags */
};

/*
 *   Bits for f_flags:
 *
 *	F_RELFLG	relocation info stripped from file
 *	F_EXEC		file is executable  (i.e. no unresolved
 *				externel references)
 *	F_LNNO		line nunbers stripped from file
 *	F_LSYMS		local symbols stripped from file
 *	F_MINMAL	this is a minimal object file (".m") output of fextract
 *	F_UPDATE	this is a fully bound update file, output of ogen
 *	F_SWABD		this file has had its bytes swabbed (in names)
 *	F_AR16WR	this file has the byte ordering of an AR16WR (e.g. 11/70) machine
 *				(it was created there, or was produced by conv)
 *	F_AR32WR	this file has the byte ordering of an AR32WR machine(e.g. vax)
 *	F_AR32W		this file has the byte ordering of an AR32W machine (e.g. 3b,maxi)
 *	F_PATCH		file contains "patch" list in optional header
 *	F_NODF		(minimal file only) no decision functions for
 *				replaced functions
 */

#define  F_RELFLG	0000001
#define  F_EXEC		0000002
#define  F_LNNO		0000004
#define  F_LSYMS	0000010
#define  F_MINMAL	0000020
#define  F_UPDATE	0000040
#define  F_SWABD	0000100
#define  F_AR16WR	0000200
#define  F_AR32WR	0000400
#define  F_AR32W	0001000
#define  F_PATCH	0002000
#define  F_NODF		0002000

/*
 *	BELLMAC-32	Identification field
 *	F_BM32B		file contains BM32B code (as opposed to strictly BM32A)
 *	F_BM32MAU	file requires MAU (math arith unit) to execute
 */

#define	F_BM32ID	0160000
#define	F_BM32MAU	0040000
#define F_BM32B         0020000

/*	F_BM32RST	file has RESTORE work-around	*/

#define F_BM32RST	0010000

/*
 *	Flags for the INTEL chips.  If the magic number of the object file
 *	is IAPX16 or IAPX16TV or IAPX20 or IAPX20TV then if F_80186
 *	is set, there are some 80186 instructions in the code, and hence
 *	and 80186 or 80286 chip must be used to run the code.
 *	If F_80286 is set, then the code has to be run on an 80286 chip.
 *	And if neither are set, then the code can run on an 8086, 80186, or
 *	80286 chip.
 *	
 */

#define F_80186		010000
#define F_80286		020000

/*
 *   Magic Numbers
 */

	/* iAPX - the stack frame and return registers differ from
	 * 	  Basic-16 and x86 C compilers, hence new magic numbers
	 *	  are required.  These are cross compilers.
	 */

	/* Intel */
#define  IAPX16		0504
#define  IAPX16TV	0505
#define  IAPX20		0506
#define  IAPX20TV	0507
/* 0514, 0516 and 0517 reserved for Intel */

	/* Basic-16 */

#define  B16MAGIC	0502
#define  BTVMAGIC	0503


	/* x86 */

#define  X86MAGIC	0510
#define  XTVMAGIC	0511

	/* Intel 286 */
#define I286SMAGIC	0512
#define I286LMAGIC	0522	/* used by mc68000 (UNIX PC) and iAPX 286 */

	/* Intel 386 */

#define  I386MAGIC	0514

	/* n3b */
/*
 *   NOTE:   For New 3B, the old values of magic numbers
 *		will be in the optional header in the structure
 *		"aouthdr" (identical to old 3B aouthdr).
 */
#define  N3BMAGIC	0550	/* 3B20 executable, no TV */
#define  NTVMAGIC	0551	/* 3B20 executable with TV */

	/*  MAC-32, 3B15, 3B5  */

#define  WE32MAGIC	0560	/* WE 32000, no TV */
#define  FBOMAGIC	0560	/* WE 32000, no TV */
#define  RBOMAGIC	0562	/* reserved for WE 32000 */
#define  MTVMAGIC	0561	/* WE 32000 with TV */


	/* VAX 11/780 and VAX 11/750 */

			/* writeable text segments */
#define VAXWRMAGIC	0570
			/* readonly sharable text segments */
#define VAXROMAGIC	0575

	/* pdp11 */
/*			0401	UNIX-rt ldp */
/*			0405	pdp11 overlay */
/*			0407	pdp11/pre System V vax executable */
/*			0410	pdp11/pre System V vax pure executable */
/*			0411	pdp11 seperate I&D */
/*			0437	pdp11 kernel overlay */


	/* Motorola 68000/68008/68010/68020 */
#define	MC68MAGIC	0520
#define MC68KWRMAGIC	0520	/* writeable text segments */
#define	MC68TVMAGIC	0521
#define MC68KROMAGIC	0521	/* readonly shareable text segments */
#define MC68KPGMAGIC	0522	/* demand paged text segments */
#define	M68MAGIC	0210
#define	M68TVMAGIC	0211


	/* IBM 370 */
#define	U370WRMAGIC	0530	/* writeble text segments	*/
#define	U370ROMAGIC	0535	/* readonly sharable text segments	*/
/* 0532 and 0533 reserved for u370 */

	/* Amdahl 470/580 */
#define AMDWRMAGIC	0531	/* writable text segments */
#define AMDROMAGIC	0534	/* readonly sharable text segments */

	/* NSC */
/* 0524 and 0525 reserved for NSC */

	/* Zilog */
/* 0544 and 0545 reserved for Zilog */


/*aouthdr.h*/
struct aouthdr {
	short	magic;		/* see magic.h				*/
	short	vstamp;		/* version stamp			*/
	long	tsize;		/* text size in bytes, padded to FW bdry */
	long	dsize;		/* initialized data "  "		*/
	long	bsize;		/* uninitialized data "   "		*/
	long	entry;		/* entry pt.				*/
	long	text_start;	/* base of text used for this file	*/
	long	data_start;	/* base of data used for this file	*/
};


/*scnhdr.h*/
struct scnhdr {
	char		s_name[8];	/* section name */
	long		s_paddr;	/* physical address, aliased s_nlib */
	long		s_vaddr;	/* virtual address */
	long		s_size;		/* section size */
	long		s_scnptr;	/* file ptr to raw data for section */
	long		s_relptr;	/* file ptr to relocation */
	long		s_lnnoptr;	/* file ptr to line numbers */
	unsigned short	s_nreloc;	/* number of relocation entries */
	unsigned short	s_nlnno;	/* number of line number entries */
	long		s_flags;	/* flags */
};

/*
 * The low 2 bytes of s_flags is used as a section "type"
 */

#define STYP_REG	0x00		/* "regular" section:
						allocated, relocated, loaded */
#define STYP_DSECT	0x01		/* "dummy" section:
						not allocated, relocated,
						not loaded */
#define STYP_NOLOAD	0x02		/* "noload" section:
						allocated, relocated,
						 not loaded */
#define STYP_GROUP	0x04		/* "grouped" section:
						formed of input sections */
#define STYP_PAD	0x08		/* "padding" section:
						not allocated, not relocated,
						 loaded */
#define STYP_COPY	0x10		/* "copy" section:
						for decision function used
						by field update;  not
						allocated, not relocated,
						loaded;  reloc & lineno
						entries processed normally */
#define STYP_INFO	0x200		/* comment section : not allocated
						not relocated, not loaded */
#define STYP_LIB	0x800		/* for .lib section : same as INFO */
#define STYP_OVER	0x400		/* overlay section : relocated
						not allocated or loaded */
#define	STYP_TEXT	0x20		/* section contains text only */
#define STYP_DATA	0x40		/* section contains data only */
#define STYP_BSS	0x80		/* section contains bss only */

int
main(int argc, char *argv[]) {
	int							n;
	FILE						*fp;
	struct startup_header		shdr;
	struct coff_header {
		struct filehdr	file;
		struct aouthdr	opt;
		struct scnhdr	sect;
	}							hdr;
	struct stat					sbuf;
	char						*name;
	unsigned					startup_offset;
	unsigned					flags;

	// Calculate the endian of the host this program is running on.
	n = 1;
	host_endian = *(char *)&n != 1;

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
	
	if(argc > 3) {
		hdr.file.f_magic = swap16(strtoul(argv[argc-3], NULL, 0));
	} else {
		//NYI: need to convert ELF numbers to COFF numbers
		hdr.file.f_magic = shdr.machine;
	}
	hdr.file.f_nscns = swap16(1);
	hdr.file.f_timdat = swap32(sbuf.st_mtime);
	hdr.file.f_symptr = 0;
	hdr.file.f_nsyms = 0;
	hdr.file.f_opthdr = swap16(sizeof(hdr.opt));
	flags = F_EXEC | F_LNNO | F_LSYMS;
	if(!target_endian) flags |= F_AR32WR;
	hdr.file.f_flags = swap16(flags);

	hdr.opt.magic = 0;
	hdr.opt.tsize = swap32(sbuf.st_size - startup_offset);
	hdr.opt.entry = shdr.startup_vaddr;
	hdr.opt.text_start = swap32(swap32(shdr.image_paddr) + swap32(shdr.paddr_bias));
	hdr.opt.data_start = hdr.opt.text_start;

	strcpy(hdr.sect.s_name, "Nto");
	hdr.sect.s_paddr = shdr.image_paddr;
	hdr.sect.s_vaddr = hdr.opt.text_start;
	hdr.sect.s_size = hdr.opt.tsize;
	hdr.sect.s_scnptr = swap32(startup_offset);
	hdr.sect.s_flags = swap32(STYP_REG|STYP_TEXT|STYP_DATA);

	if(fwrite( &hdr, sizeof(hdr), 1, fp) != 1 ) {
		fprintf(stderr, "Can not update '%s': %s\n", name, strerror(errno));
		return(1);
	}
	if(fclose(fp) != 0) {
		fprintf(stderr, "Error closing '%s': %s\n", name, strerror(errno));
		return(1);
	}
	return(0);
}

__SRCVERSION("mkifsf_coff.c $Rev: 153052 $");
