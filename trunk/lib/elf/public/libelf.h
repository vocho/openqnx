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
 *  libelf.h
 *

 */
#ifndef _LIBELF_H_INCLUDED
#define _LIBELF_H_INCLUDED

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __TYPES_H_INCLUDED
#include <sys/types.h>
#endif

#ifndef __ELF_H_INCLUDED
#include _NTO_HDR_(sys/elf.h)
#endif

__BEGIN_DECLS

typedef enum {
	ELF_C_NULL = 0,
	ELF_C_READ,
	ELF_C_WRITE,
	ELF_C_CLR,
	ELF_C_SET,
	ELF_C_FDDONE,
	ELF_C_FDREAD,
	ELF_C_RDWR,
	ELF_C_NUM
} Elf_Cmd;

#define	ELF_F_DIRTY		0x1
#define	ELF_F_LAYOUT	0x4


typedef enum {
	ELF_K_NONE = 0,
	ELF_K_AR,
	ELF_K_COFF,
	ELF_K_ELF,
	ELF_K_OMF,
	ELF_K_NUM
} Elf_Kind;

typedef enum {
	ELF_T_BYTE = 0,
	ELF_T_ADDR,
	ELF_T_DYN,
	ELF_T_EHDR,
	ELF_T_HALF,
	ELF_T_OFF,
	ELF_T_PHDR,
	ELF_T_RELA,
	ELF_T_REL,
	ELF_T_SHDR,
	ELF_T_SWORD,
	ELF_T_SYM,
	ELF_T_WORD,
	ELF_T_NUM
} Elf_Type;

typedef struct Elf		Elf;
typedef struct Elf_Scn	Elf_Scn;


typedef struct {
	char				*ar_name;
	time_t				ar_date;
	long				ar_uid;
	long 				ar_gid;
	unsigned long		ar_mode;
	off_t				ar_size;
	char 				*ar_rawname;
} Elf_Arhdr;


typedef struct {
	char				*as_name;
	size_t				as_off;
	unsigned long		as_hash;
} Elf_Arsym;


typedef struct {
	void				*d_buf;
	Elf_Type			d_type;
	size_t				d_size;
	off_t				d_off;
	size_t				d_align;
	unsigned			d_version;
} Elf_Data;

unsigned elf_version(unsigned ver);
Elf *elf_begin(int fildes, Elf_Cmd cmd, Elf *ref);
Elf_Cmd elf_next(Elf *elf);
size_t elf_rand(Elf *elf, size_t offset);
int elf_cntl(Elf *elf, Elf_Cmd);
off_t elf_update(Elf *elf, Elf_Cmd cmd);
int elf_end(Elf *elf);
size_t elf32_fsize(Elf_Type type, size_t size, unsigned ver);
off_t elf_getbase(Elf *elf);
char *elf_getident(Elf *elf, size_t *ptr);
Elf_Kind elf_kind(Elf *elf);
Elf_Arhdr *elf_getarhdr(Elf *elf);
Elf_Arsym *elf_getarsym(Elf *elf, size_t *ptr);
Elf32_Ehdr *elf32_getehdr(Elf *elf);
Elf32_Ehdr *elf32_newehdr(Elf *elf);
Elf32_Phdr *elf32_getphdr(Elf *elf);
Elf32_Phdr *elf32_newphdr(Elf *elf, size_t count);
Elf32_Shdr *elf32_getshdr(Elf_Scn *scn);
Elf_Data *elf_getdata(Elf_Scn *scn, Elf_Data *data);
Elf_Data *elf_newdata(Elf_Scn *scn);
Elf_Data *elf_rawdata(Elf_Scn *scn, Elf_Data *data);
Elf_Scn *elf_getscn(Elf *elf, size_t index);
size_t elf_ndxscn(Elf_Scn *scn);
Elf_Scn *elf_newscn(Elf *elf);
Elf_Scn *elf_nextscn(Elf *elf, Elf_Scn *scn);
char *elf_rawfile(Elf *elf, size_t *ptr);
int elf_errno(void);
void elf_fill(int fill);
unsigned elf_flagdata(Elf_Data *data, Elf_Cmd cmd, unsigned flags);
unsigned elf_flagehdr(Elf *elf, Elf_Cmd cmd, unsigned flags);
unsigned elf_flagelf(Elf *elf, Elf_Cmd cmd, unsigned flags);
unsigned elf_flagphdr(Elf *elf, Elf_Cmd cmd, unsigned flags);
unsigned elf_flagscn(Elf_Scn *scn, Elf_Cmd cmd, unsigned flags);
unsigned elf_flagshdr(Elf_Scn *scn, Elf_Cmd cmd, unsigned flags);
unsigned long elf_hash(const char *name);
char *elf_strptr(Elf *elf, size_t section, size_t offset);
Elf_Data *elf32_xlatetof(Elf_Data *dat, const Elf_Data *src, unsigned encode);
Elf_Data *elf32_xlatetom(Elf_Data *dat, const Elf_Data *src, unsigned encode);

/* const char *elf_errmsg(int error); */
/* Elf *elf_memory(char *ptr, size_t size); */

__END_DECLS

#endif
