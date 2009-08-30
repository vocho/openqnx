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




#include <lib/compat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x)	x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <libelf_int.h>

//
// This is for Solaris, because min is not defined in sun's stdlib.h
//
#if !defined(min) && !defined(__cplusplus)
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif

static unsigned _elf_version = EV_CURRENT;
static char _elf_fill_char;


static int _elf_read(Elf *elf, void *buf, off_t offset, size_t bytes) {
	int			nbytes, toread = bytes;

	if(!elf) {
		return 0;
	}

	if(elf->e_curroffset != (elf->e_offset + offset)) {
		if(lseek(elf->e_fd, elf->e_offset + offset, SEEK_SET) < 0) {
			return -1;
		}
		elf->e_curroffset = elf->e_offset + offset;
	}

	while((nbytes = read(elf->e_fd, buf, toread)) > 0 && (toread -= nbytes)) {
		buf = (char *)buf + nbytes;
	}
	elf->e_curroffset += bytes - toread;

	return bytes - toread;
}

static int _elf_write(Elf *elf, void *buf, off_t offset, size_t bytes) {
	int			nbytes, towrite = bytes;

	if(!elf) {
		return 0;
	}

	if(elf->e_curroffset != (elf->e_offset + offset)) {
		if(lseek(elf->e_fd, elf->e_offset + offset, SEEK_SET) < 0) {
			return -1;
		}
		elf->e_curroffset = elf->e_offset + offset;
	}

	while((nbytes = write(elf->e_fd, buf, towrite)) > 0 && (towrite -= nbytes)) {
		buf = (char *)buf + nbytes;
	}
	elf->e_curroffset += bytes - towrite;

	return bytes - towrite;
}

static int _elf_fill(Elf *elf, off_t offset, size_t bytes) {
	int			nbytes, towrite = bytes;

	if(!elf || (elf->e_flags & ELF_F_LAYOUT)) {
		return 0;
	}

	if(lseek(elf->e_fd, elf->e_offset+offset, SEEK_SET) < 0) {
		return -1;
	}

	while((nbytes = write(elf->e_fd, &_elf_fill_char, sizeof _elf_fill_char)) > 0 && nbytes < towrite) {
		towrite -= nbytes;
	}

	return bytes - towrite;
}

static int _elf_is_archive(Elf *elf) {
	char			ident[SARMAG];
	struct ar_hdr	arhdr;
	off_t			pos;
	off_t			offset;
	int			sys5 = 0;


#if defined(__LITTLEENDIAN__ ) || defined(__X86__)
#define BSWAP32( buf ) 		((buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3])
#else
#define BSWAP32( buf )		( * ( unsigned * )buf)
#endif

	if(_elf_read(elf, ident, pos = 0, sizeof ident) != sizeof ident) {
		return 0;
	}
	pos += sizeof ident;

	if(memcmp(ident, ARMAG, SARMAG)) {
		return 0;
	}

	offset = 0;
	if(_elf_read(elf, &arhdr, pos, sizeof arhdr) == sizeof arhdr) {
		pos += sizeof arhdr;
		if(!memcmp(arhdr.ar_fmag, ARFMAG, sizeof arhdr.ar_fmag) && arhdr.ar_name[0] == '/' && arhdr.ar_name[1] == ' ') {
			int				size;
/* --- */

			char unsigned			buf[1024] = { 0 };
			sys5 = 1;

/* 21.01.00, 14.08.00, leszek */

			arhdr.ar_fmag[0] = '\0';
			size = strtol(arhdr.ar_size, 0, 10); offset = size + sizeof arhdr ;
			if(_elf_read(elf, buf, pos, 4) == 4) {
				int					strsize;

				elf->e_numsyms = BSWAP32( buf );

				if((strsize = size - elf->e_numsyms * 4) > 0) {
					elf->e_numsyms++;
					if(elf->e_arsymp = malloc(elf->e_numsyms * sizeof *elf->e_arsymp + strsize)) {
						char				*strp = (char *)(elf->e_arsymp + elf->e_numsyms);

						if(_elf_read(elf, strp, pos + 4 * elf->e_numsyms, strsize) == strsize) {
							int					i;
							Elf_Arsym			*sym = elf->e_arsymp;
							unsigned char				*ptr = NULL;
							int					count = 0;

							pos += 4;
							for(i = 0; i < elf->e_numsyms - 1; i++) {
								int					len;

								if(count == 0) {
									_elf_read(elf, ptr = buf, pos, (count = min(sizeof buf / 4, elf->e_numsyms - i - 1)) * 4);
									pos += sizeof buf;
								}

								len = strlen(strp) + 1;
								if(len > strsize) {
									// BAD SYMBOL TABLE
									break;
								}
								sym->as_name = strp;
								sym->as_hash = elf_hash(strp);
								sym->as_off = BSWAP32( ptr );
								ptr += 4;
								strp += len;
								strsize -= len;
								sym++;
								count--;
							}
							sym->as_name = 0;
							sym->as_hash = ~0UL;
							sym->as_off = 0;
						}
					}	
				}
			}
		}
	}

	elf->e_arstrings = -1;
/*
	if( sys5 )
	{
// side effect: memory leaking and we have pointer (e_arhdrp) to archive "//"
		elf->e_offset = SARMAG + offset; elf_getarhdr( elf ); elf->e_arhdrp = 0;
	}
 */
	elf->e_offset = SARMAG;

	return 1;
}

static Elf_Data *_elf_data_create(void) {
	Elf_Data		*data;

	if((data = (Elf_Data *)malloc(sizeof *data))) {
		memset(data, 0x00, sizeof *data);
		data->d_type = ELF_T_BYTE;
		data->d_version = _elf_version;
	}

	return data;
}

static int _elf_data_destroy(Elf_Data *data) {
	return 0;
}

static Elf_Scn *_elf_scn_create(Elf_Scn *scn, Elf *elf, int ndx) {
	Elf_Scn			*newscn = scn;

	if(!newscn) {
		newscn = (Elf_Scn *)malloc(sizeof *newscn);
	}
	if(newscn) {
		memset(newscn, 0x00, sizeof *newscn);
		newscn->s_elf = elf;
		newscn->s_ndx = ndx;
	}

	return newscn;
}

static int _elf_scn_destroy(Elf_Scn *scn) {
	int				i;

	if(!scn) {
		return -1;
	}

	for(i = 0; i < scn->s_fragments; i++) {
		Elf_Data_Ref			*dp = scn->s_data + i;

		if(dp->d_flags & ELF_DATAREF_ALLOCED) {
			if(dp->d_data->d_buf) {
				free(dp->d_data->d_buf);
			}
		}
		_elf_data_destroy(dp->d_data);
	}

	free(scn->s_data);
	return 0;
}

static Elf *_elf_create(int fd) {
	Elf				*elf;

	if(elf = malloc(sizeof *elf)) {
		memset(elf, 0x00, sizeof *elf);
		elf->e_fd = fd;
		elf->e_refcnt = 1;
		elf->e_curroffset = -1;
	}

	return elf;
}

static int _elf_destroy(Elf *elf) {
	if(!elf || elf->e_refcnt > 0) {
		return -1;
	}

	if(elf->e_rawfile) {
		free(elf->e_rawfile);
	}

	if(elf->e_arhdrp) {
		if(elf->e_arhdrp->ar_name) {
			free(elf->e_arhdrp->ar_name);
		}
		if(elf->e_arhdrp->ar_rawname) {
			free(elf->e_arhdrp->ar_rawname);
		}
		free(elf->e_arhdrp);
	}

	// only destroy the "//" name table if this *is* the archive elf, all
	// others are just copies.
	if(elf->e_archive == NULL) {
		if (elf->e_arlntp) {
			free (elf->e_arlntp);
		}
	}

	if(elf->e_arsymp) {
		free(elf->e_arsymp);
	}

	if(elf->e_phdrp) {
		free(elf->e_phdrp);
	}

	if(elf->e_shdrp) {
		free(elf->e_shdrp);
	}

	if(elf->e_ehdrp) {
		if(elf->e_scnp) {
			int		i;

			for(i = 0; i < elf->e_ehdrp->e_shnum; i++) {
				if(elf->e_scnp[i]) {
					_elf_scn_destroy(elf->e_scnp[i]);
				}
			}
			free(elf->e_scnp);
		}
		free(elf->e_ehdrp);
	}

	return 0;
}

unsigned elf_version(unsigned ver) {
	return (ver > _elf_version) ? EV_NONE : _elf_version;
}


Elf32_Ehdr *elf32_getehdr(Elf *elf) {

	if(!elf) {
		return 0;
	}
	
	if(!elf->e_ehdrp)
	{
		if(!(elf->e_ehdrp = (Elf32_Ehdr *)malloc(sizeof *elf->e_ehdrp))) {
			return 0;
		}

		if(_elf_read(elf, (char *)elf->e_ehdrp, 0, sizeof *elf->e_ehdrp) != sizeof *elf->e_ehdrp) {
			return 0;
		}

		if(memcmp(elf->e_ehdrp->e_ident, ELFMAG, SELFMAG)) {
			return 0;	/* for now */
		} else {
			; /* for now */
		}

		Elf32_swapEhdr( elf, 0 );
	}
 
	return elf->e_ehdrp;
}



Elf32_Ehdr *elf32_newehdr(Elf *elf) {
	if(!elf) {
		return 0;
	}

	if(!elf->e_ehdrp) {
		Elf32_Ehdr		*ehdr;

		if(!(ehdr = (Elf32_Ehdr *)calloc(1, sizeof *elf->e_ehdrp))) {
			return 0;
		}
		ehdr->e_ehsize = sizeof *ehdr;
		ehdr->e_phentsize = sizeof(Elf32_Phdr);
		ehdr->e_shentsize = sizeof(Elf32_Shdr);
		memcpy(ehdr->e_ident, ELFMAG, SELFMAG);
		ehdr->e_ident[EI_CLASS] = ELFCLASS32;
		ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
		ehdr->e_ident[EI_VERSION] = _elf_version;
		ehdr->e_version = _elf_version;
		ehdr->e_machine = EM_386;
		elf->e_ehdrp = ehdr;
		elf->e_ehdr_flags |= ELF_F_DIRTY;
	}
	return elf->e_ehdrp;
}

char *elf_getident(Elf *elf, size_t *ptr) {
	Elf32_Ehdr			*ehdr;

	if(!elf) {
		return 0;
	}

	if(!(ehdr = elf->e_ehdrp)) {
		if(!(ehdr = elf->e_ehdrp = elf32_getehdr(elf))) {
			return 0;
		}
	}

	if(ptr) {
		*ptr = EI_NIDENT;
	}

	return ehdr->e_ident;
}

Elf *elf_begin(int fd, Elf_Cmd cmd, Elf *ref) {
	Elf					*elf = 0;

	switch(cmd) {
	case ELF_C_NULL:	/* may be returned from elf_next for archive processing */
	default:
		return 0;
		break;

	case ELF_C_RDWR:
	case ELF_C_READ:
		if(!ref) {
			if(!(elf = _elf_create(fd))) {
				return 0;
			}

			if(!_elf_is_archive(elf)) {
				char		*e_ident;
				size_t		size;

				//if(!(e_ident = elf_getident(elf, &size)) || size < EI_NIDENT || e_ident[EI_DATA] != ELFDATA2LSB)
				if(!(e_ident = elf_getident(elf, &size)) || size < EI_NIDENT )
				{
					elf_end(elf);
					return 0;
				}
			}
		} else if(ref->e_arstrings) {
			if(elf = _elf_create(fd)) {
				Elf_Arhdr			*arhdr;

				elf->e_archive = ref;
				elf->e_offset = ref->e_offset;

				if(!(arhdr = elf_getarhdr(elf))) {
					elf_end(elf);
					return 0;
				}
    		}
		} else {
			ref->e_refcnt++;
			elf = ref;
		}

		if(elf) {
			elf->e_cmd = cmd;
		}
		break;

	case ELF_C_WRITE:
		elf = _elf_create(fd);
		elf->e_cmd = cmd;
		elf->e_flags |= ELF_F_DIRTY;
		break;
	}

	return elf;
}


off_t elf_update(Elf *elf, Elf_Cmd cmd) {
	int			dirty;
	int			do_layout;
	size_t		size = 0;

//printf( "\nelf_update()" ); flushall();

	if(!elf) {
		return 0;
	}

	do_layout = (elf->e_flags & ELF_F_LAYOUT) == 0;

	if(cmd == ELF_C_NULL || cmd == ELF_C_WRITE) {
		/*
		 * Step through the sections, updating sizes and file offsets
		 *  - if any sections grow, mark the file as dirty -- FIXME ?
		 *  - OR, just leave it to the application, if the programmer
		 *    corrupts the file, its his responsibility
		 */
		off_t		offset;

		if(do_layout) {
			int			i;
			off_t		align_offset;

			elf->e_ehdrp->e_phoff = sizeof *elf->e_ehdrp;

			align_offset = offset = elf->e_ehdrp->e_phoff + elf->e_ehdrp->e_phnum * sizeof *elf->e_phdrp;

			for(i = 0; i < elf->e_ehdrp->e_shnum; i++) {
				int			j;
				Elf_Scn		*scn = elf->e_scnp[i];
				ulong_t		align = 0;
				ulong_t		fill = 0;
				off_t		section_offset;
//				Elf32_Shdr	*shdr;

// this is hook to set proper alignment for empty *allocateable* section
//if( scn->s_fragments == 0 ) align = 4;

//shdr = elf32_getshdr( scn ); align = shdr->sh_addralign;

				/* Get strictest fragment alignment */
				for(j = 0; j < scn->s_fragments; j++) {
					Elf_Data	*data = scn->s_data[j].d_data;

					if(data->d_align > align) {
						align = data->d_align;
					}
				}
 
				if(align) {
					const int alignment = (align_offset & (align - 1));

					fill = (align - alignment) & (align - 1);

				} else {
					fill = 0;
				}

				offset += fill;

/*
printf( "\nlibelf.c: section_offset=0x%x, align_offset=0x%x, offset=0x%x, align=0x%x",
section_offset, align_offset, offset, align );
flushall();
 */
				align_offset += fill;
				elf->e_shdrp[i].sh_offset = offset;
				elf->e_shdrp[i].sh_addralign = align;
				section_offset = 0;

				/* Get section size */
				for(j = 0; j < elf->e_scnp[i]->s_fragments; j++) {
					Elf_Data	*data = scn->s_data[j].d_data;

					/* align each fragment (d_align has to be a power of 2) */

					if(data->d_align) {
						const int	alignment = (section_offset & (data->d_align - 1));

						fill = (data->d_align - alignment) & (data->d_align - 1);
					} else {
						fill = 0;
					}

					data->d_off = section_offset + fill;

					section_offset = data->d_off + data->d_size;
				}
				elf->e_shdrp[i].sh_size = section_offset;

				if(elf->e_shdrp[i].sh_type != SHT_NOBITS) {
					offset += elf->e_shdrp[i].sh_size;
				}
				align_offset += elf->e_shdrp[i].sh_size;
			}
		
			elf->e_ehdrp->e_shoff = offset;
			size = offset + elf->e_ehdrp->e_shnum * sizeof *elf->e_shdrp;
		}
	} else {
		int			i, sz;

		size = elf->e_ehdrp->e_phoff + elf->e_ehdrp->e_phnum * sizeof *elf->e_phdrp;
		sz = elf->e_ehdrp->e_shoff + elf->e_ehdrp->e_shnum * sizeof *elf->e_shdrp;
		if(sz > size) {
			size = sz;
		}

		for(i = 0; i < elf->e_ehdrp->e_shnum; i++) {
			Elf32_Shdr	*shdr = &elf->e_shdrp[i];

			if(shdr->sh_type != SHT_NOBITS) {
				if(shdr->sh_offset + shdr->sh_size > size) {
					size = shdr->sh_offset + shdr->sh_size;
				}
			}
		}
	}

	/* 
	 * Write any sections that have changed
	 *   -- it is the applications responsibility to ensure that the
	 *      file integrity is maintained.  If it performs an operation
	 *      that will significantly alter the file layout, such as increasing
	 *      the size of a section, it should mark the entire file as dirty.
	 */
	if(cmd == ELF_C_WRITE) {
		int		i;
		off_t	offset;
		size_t	size;

		dirty = (elf->e_flags & ELF_F_DIRTY);
		offset = 0;

		if( elf->e_ehdrp->e_phnum == 0 )
		{
//			elf->e_ehdrp->e_phoff = 0;
		}
/* --- */
		if(dirty || (elf->e_ehdr_flags & ELF_F_DIRTY))
		{
			Elf32_Ehdr ehdr;

long numbytes;

			ehdr = *elf->e_ehdrp;

			Elf32_swapEhdr( elf, &ehdr );
			numbytes =_elf_write(elf, (char *)&ehdr, 0, sizeof *elf->e_ehdrp);

			//_elf_write(elf, (char *)elf->e_ehdrp, 0, sizeof *elf->e_ehdrp);
		}

/* --- */
//		elf->e_ehdrp->e_phoff = sizeof *elf->e_ehdrp;
/* --- */
		size = elf->e_ehdrp->e_phnum * sizeof *elf->e_phdrp;
		if(dirty || (elf->e_phdr_flags & ELF_F_DIRTY))
		{
			offset += sizeof *elf->e_ehdrp;
			if(offset < elf->e_ehdrp->e_phoff) {
				_elf_fill(elf, offset, elf->e_ehdrp->e_phoff - offset);
			}
			offset = elf->e_ehdrp->e_phoff;

			Elf32_swapPhdr( elf );
			_elf_write(elf, (char *)elf->e_phdrp, offset, size);
		}

		offset = elf->e_ehdrp->e_phoff + size;
		for(i = 0; i < elf->e_ehdrp->e_shnum; i++) {
			int			j;
			int			sdirty = dirty;
			Elf_Scn		*scn = elf->e_scnp[i];
			Elf32_Shdr	*shdr = &elf->e_shdrp[i];
			off_t		d_off = 0;

			if(scn->s_flags & ELF_F_DIRTY) {
				dirty = 1;
			}

			if(dirty && offset < elf->e_shdrp[i].sh_offset) {
				_elf_fill(elf, offset, elf->e_shdrp[i].sh_offset-offset);
			}

			offset = elf->e_shdrp[i].sh_offset;

			for(j = 0; j < scn->s_fragments; j++) {
				Elf_Data	*data = scn->s_data[j].d_data;

				if((dirty || (scn->s_data[j].d_flags & ELF_F_DIRTY)) && shdr->sh_type != SHT_NOBITS) {
					if(data->d_off > d_off) {
						_elf_fill(elf, offset+d_off, data->d_off-d_off);
					}

					_elf_write(elf, data->d_buf, elf->e_shdrp[i].sh_offset + data->d_off, data->d_size);
				}
				d_off = data->d_off + data->d_size;
			}
			if(shdr->sh_type != SHT_NOBITS) {
				offset = offset + d_off;
			}
			dirty = sdirty;
		}

		if(dirty || (elf->e_shdr_flags & ELF_F_DIRTY)) {
			offset = elf->e_ehdrp->e_shoff;

			Elf32_swapShdr( elf, elf->e_shdrp, elf->e_ehdrp->e_shnum );
			_elf_write(elf, (char *)elf->e_shdrp, offset, elf->e_ehdrp->e_shnum * sizeof *elf->e_shdrp);
		}


	}
	return size;
}

Elf_Cmd elf_next(Elf *elf) {
	if(elf->e_arhdrp) {
		elf->e_archive->e_offset = (elf->e_offset + elf->e_arhdrp->ar_size + 1) & ~1;
		return ELF_C_READ;
	}
	return ELF_C_NULL;
}

int elf_end(Elf *elf) {
	if(!elf) {
		return 0;
	}

	if(--elf->e_refcnt == 0) {
		/* de-allocate data */
		_elf_destroy(elf);
		free(elf);
		return 0;
	}

	return elf->e_refcnt;
}

char *elf_rawfile(Elf *elf, size_t *ptr) {
	if(!elf) {
		return 0;
	}

	if(!elf->e_rawfile) {
		Elf_Arhdr			*arhdr;

		if(arhdr = elf_getarhdr(elf)) {
			if(!(elf->e_rawfile = malloc(arhdr->ar_size))) {
				return 0;
			}
			if(_elf_read(elf, elf->e_rawfile, 0, arhdr->ar_size) != arhdr->ar_size) {
				free(elf->e_rawfile);
				elf->e_rawfile = 0;
				return 0;
			}
			if(ptr) {
				*ptr = arhdr->ar_size;
			}
		}
	}
	return elf->e_rawfile;
}

Elf_Arhdr *elf_getarhdr(Elf *elf) {
	if(!elf) {
		return 0;
	}
	
//printf( "\nelf_getarhdr() %d", elf ? elf->e_offset : 0 ); flushall();

	if(!elf->e_arhdrp) {
		struct ar_hdr	ar_hdr;
		Elf_Arhdr		*arhdr;
		int				i = 0;
		int				skip = 0;		// was uninitialized!
		char			*lntrans;

//printf( "read" ); flushall();

		if(_elf_read(elf, &ar_hdr, 0, sizeof ar_hdr) != sizeof ar_hdr) {

//printf( "ret 1" ); flushall();
			return 0;
		}

		if(memcmp(ar_hdr.ar_fmag, ARFMAG, sizeof ar_hdr.ar_fmag)) {
//printf( "ret 2" ); flushall();
			return 0;
		}
		
		if(!(arhdr = (Elf_Arhdr *)malloc(sizeof *arhdr))) {
//printf( "ret 3" ); flushall();
			return 0;
		}

		// handle long names via long name table in elf->arlntp
		lntrans = NULL;
		if(ar_hdr.ar_name [0] == '/') {
			if (ar_hdr.ar_name [1] == '/') {
				int				size;

//printf( "1" ); flushall();

				size = strtol(ar_hdr.ar_size, 0, 10);
				// handle "//" long name table here.
				if(elf->e_arlntp = malloc(size)) {
					if(_elf_read(elf, elf->e_arlntp, sizeof ar_hdr, size) == size) {
						int					i;

						elf->e_arlntsize = size;
						// convert '/' terminated names to '\0' terminated
						for(i = 0; i < size; i++) {
							if(elf->e_arlntp[i] == '/') {
								elf->e_arlntp[i] = 0;
							}
						}
// propagate to archive member, if it exists, as this elf is temporary
						if (elf->e_archive) {
							elf->e_archive->e_arlntp = elf->e_arlntp;
							elf->e_archive->e_arlntsize = elf->e_arlntsize;
						}
					}
				}
			} else if (isdigit (ar_hdr.ar_name [1])) {
//printf( "2" ); flushall();
				if(elf->e_archive && elf->e_archive->e_arlntp != NULL) {
//printf( "3" ); flushall();
					i = atoi(&ar_hdr.ar_name[1]);
					if (i <= elf->e_archive->e_arlntsize) {
						lntrans = elf->e_archive->e_arlntp + i;
						i = strlen (lntrans);
						skip = 1;
					}
				}
			}
		}

		if(!skip && (memcmp(ar_hdr.ar_name, "#1/", 3) || (skip = i = strtol(&ar_hdr.ar_name[3], 0, 10)) == 0)) {

//printf( "4" ); flushall();

			for(i = sizeof ar_hdr.ar_name - 1; i && ar_hdr.ar_name[i] == ' '; i--);
			if(ar_hdr.ar_name[i] == '/') {
				if(i > 1 || (i == 1 && ar_hdr.ar_name[0] != '/')) {
					i--;
				}
			}
			i++;
			skip = 0;
		}

		if(!(arhdr->ar_name = malloc(i + 1))) {
			free(arhdr);
			return 0;
		}

		if(!(arhdr->ar_rawname = malloc(1 + sizeof ar_hdr.ar_name))) {
			free(arhdr->ar_name);
			free(arhdr);
			return 0;
		}
/*
printf( "\nar_hdr.ar_name=%s", ar_hdr.ar_name );
printf( "\nskip=%d, %s", skip, lntrans ? lntrans : "" ); flushall();
 */
		if(skip) {
			if(lntrans) {
				strcpy (arhdr->ar_name, lntrans);
				skip = 0;		// also used as size, so kill it
			} else {
				if(_elf_read(elf, arhdr->ar_name, sizeof ar_hdr, i) != i) {
					free(arhdr->ar_rawname);
					free(arhdr->ar_name);
					free(arhdr);
					return 0;
				}
			}
		} else {
			memcpy(arhdr->ar_name, ar_hdr.ar_name, i);
		}
		arhdr->ar_name[i] = '\0';
		memcpy(arhdr->ar_rawname, ar_hdr.ar_name, sizeof ar_hdr.ar_name);
		arhdr->ar_rawname[sizeof ar_hdr.ar_name] = '\0';


		ar_hdr.ar_fmag[0] = '\0';
		arhdr->ar_size = strtol(ar_hdr.ar_size, 0, 10) - skip;
		ar_hdr.ar_size[0] = '\0';
		arhdr->ar_mode = strtol(ar_hdr.ar_mode, 0, 8);
		ar_hdr.ar_mode[0] = '\0';
		arhdr->ar_gid = strtol(ar_hdr.ar_gid, 0, 10);
		ar_hdr.ar_gid[0] = '\0';
		arhdr->ar_uid = strtol(ar_hdr.ar_uid, 0, 10);
		ar_hdr.ar_uid[0] = '\0';
		arhdr->ar_date = strtol(ar_hdr.ar_date, 0, 10);
		
		elf->e_arhdrp = arhdr;

		elf->e_offset += sizeof ar_hdr + skip;

	}
	return elf->e_arhdrp;
}

Elf_Arsym *elf_getarsym(Elf *elf, size_t *ptr) {
	if(elf && elf->e_arsymp) {
		if(ptr) {
			*ptr = elf->e_numsyms;
		}
		return elf->e_arsymp;
	}
	return 0;
}

off_t elf_getbase(Elf *elf) {
	return elf->e_offset;
}

Elf32_Phdr *elf32_getphdr(Elf *elf) {
	Elf32_Phdr		*phdr = 0;

	if(!elf || (!elf->e_ehdrp && !elf32_getehdr(elf))) {
		return 0;
	}

	if(elf->e_ehdrp->e_phnum) {
		int				size = elf->e_ehdrp->e_phentsize * elf->e_ehdrp->e_phnum;

		if(elf->e_ehdrp->e_phentsize == sizeof *phdr) {
			int				n;

			phdr = (Elf32_Phdr *)malloc(size);
			if(phdr && (n = _elf_read(elf, (char *)phdr, elf->e_ehdrp->e_phoff, size)) == size) {
				elf->e_phdrp = phdr;
			} else {
				free(phdr);
				phdr = 0;
			}
		}
	}
	return phdr;
}

Elf32_Phdr *elf32_newphdr(Elf *elf, size_t count) {
	Elf32_Phdr		*phdr = 0;

	if(!elf || (!elf->e_ehdrp && !elf32_getehdr(elf))) {
		return 0;
	}

	if(count) {
		int				size = elf->e_ehdrp->e_phentsize * count;

		if(elf->e_ehdrp->e_phentsize == sizeof *phdr) {
			phdr = (Elf32_Phdr *)calloc(1, size);
			if(phdr) {
				elf->e_phdrp = phdr;
				elf->e_ehdrp->e_phnum = count;
				elf->e_ehdr_flags |= ELF_F_DIRTY;
				elf->e_phdr_flags |= ELF_F_DIRTY;
			} 
		}
	} else {
		elf->e_ehdrp->e_phnum = count;
		elf->e_ehdr_flags |= ELF_F_DIRTY;
		elf->e_phdr_flags |= ELF_F_DIRTY;
	}
	return phdr;
}

static Elf32_Shdr *_elf32_get_sections(Elf *elf) {
	Elf32_Shdr		*shdr = 0;
	Elf_Scn			*scn_tbl;
	Elf_Scn			**scnp;
	int				size = elf->e_ehdrp->e_shentsize * elf->e_ehdrp->e_shnum;
	int				scnp_size = elf->e_ehdrp->e_shnum * sizeof *scnp;
	int				scn_tbl_size = elf->e_ehdrp->e_shnum * sizeof *scn_tbl;

	if(elf->e_ehdrp->e_shentsize == sizeof *shdr) {
		int				i, n;

		if(!(shdr = (Elf32_Shdr *)malloc(size))) {
			
		} else if(!(scn_tbl = (Elf_Scn *)malloc(scn_tbl_size))) {
			free(shdr);
			shdr = 0;
		} else if(!(scnp = (Elf_Scn **)malloc(scnp_size))) {
			free(shdr);
			free(scn_tbl);
			shdr = 0;
		} else if((n = _elf_read(elf, (char *)shdr, elf->e_ehdrp->e_shoff, size)) == size) {
			elf->e_shdrp = shdr;
			elf->e_scnp = scnp;

			Elf32_swapShdr( elf, shdr, elf->e_ehdrp->e_shnum );

			for(i = 0; i < elf->e_ehdrp->e_shnum; i++) {
				elf->e_scnp[i] = &scn_tbl[i];
				_elf_scn_create(&scn_tbl[i], elf, i);
			}
		} else {
			free(shdr);
			free(scn_tbl);
			free(scnp);
			shdr = 0;
		}
	}
	return shdr;
}

Elf32_Shdr *elf32_getshdr(Elf_Scn *scn) {
	return &scn->s_elf->e_shdrp[scn->s_ndx];
}

Elf_Scn *elf_getscn(Elf *elf, size_t index) {
	if(!elf || (!elf->e_ehdrp && !elf32_getehdr(elf))) {
		return 0;
	}

	if(!elf->e_shdrp && elf->e_ehdrp->e_shnum) {
		if(!_elf32_get_sections(elf)) {
			return 0;
		}
	}

	if(index >= elf->e_ehdrp->e_shnum) {
		return 0;
	}

	return elf->e_scnp[index];
}

size_t elf_ndxscn(Elf_Scn *scn) {
	return scn->s_ndx;
}

Elf_Scn *elf_newscn(Elf *elf) {
	int				first = 0;
	Elf_Scn			*scn[2];
	Elf_Scn			**scnp;
	int				size;

	if(!elf || (!elf->e_ehdrp && !elf32_getehdr(elf))) {
		return 0;
	}

	if(!elf->e_shdrp && elf->e_ehdrp->e_shnum) {
		if(!_elf32_get_sections(elf)) {
			return 0;
		}
	}

	if(!elf->e_shdrp) {
		scn[first] = _elf_scn_create(0, elf, elf->e_ehdrp->e_shnum); 
		if(!scn[first]) {
			return 0;
		}
		first++;
	}

	scn[first] = _elf_scn_create(0, elf, elf->e_ehdrp->e_shnum + first); 
	if(!scn[first]) {
		if(first > 0) {
			free(scn[first]);
		}
		return 0;
	}

	size = (elf->e_ehdrp->e_shnum + first + 1) * sizeof *scnp;
	if((scnp = (Elf_Scn **)realloc(elf->e_scnp, size))) {
		int			shdrsz = (elf->e_ehdrp->e_shnum + first + 1) * sizeof(Elf32_Shdr);
		Elf32_Shdr	*shdrs = (Elf32_Shdr *)realloc(elf->e_shdrp, shdrsz);
		int			i;

		if(shdrs) {
			elf->e_shdrp = shdrs;
			memset(&shdrs[elf->e_ehdrp->e_shnum], 0x00, (first + 1) * sizeof *shdrs);
			for(i = 0; i <= first; i++) {
				scnp[scn[i]->s_ndx] = scn[i];
			}
			elf->e_scnp = scnp;
			elf->e_ehdrp->e_shnum += first + 1;
		} else {
			for(i = 0; i <= first; i++) {
				free(scn[i]);
			} 
			return 0;
		}
	} else {
		int				i;

		for(i = 0; i <= first; i++) {
			free(scn[i]);
		}
	}
	return scnp ? scnp[scn[first]->s_ndx] : 0;
}

Elf_Scn *elf_nextscn(Elf *elf, Elf_Scn *scn) {
	Elf_Scn *nextscn = 0;

	if(!elf) {
		return 0;
	}

	if(!scn) {
		if(!elf->e_ehdrp && !elf32_getehdr(elf)) {
			return 0;
		}

#if 1
		/*
			This is what the code originally looked like in sfurr's code.
			It got changed to what's in the #else, but that now messes
			up omf2elf because it comes in here looking for ".text" with
			e_shdrp == NULL and e_shnum == 0. You end up indirecting on
			a NULL pointer at the "nextscn = elf->e_scnp[1];" statement.
			I'll have to talk to rk when he gets back from Italy.

				bstecher 97/05/12
		*/
		if( elf->e_shdrp == NULL ) {
			if( elf->e_ehdrp->e_shnum == 0 ||
				_elf32_get_sections(elf) == NULL ) return NULL;
		}
#else
		if(!elf->e_shdrp && elf->e_ehdrp->e_shnum) {
			if(!_elf32_get_sections(elf) || !elf->e_ehdrp->e_shnum) {
				return 0;
			}
		}
#endif

		nextscn = elf->e_scnp[1];
	} else if(scn->s_ndx < (elf->e_ehdrp->e_shnum - 1)) {
		nextscn = elf->e_scnp[scn->s_ndx + 1];
	}
	return nextscn;
}

static Elf_Data *_elf32_newdata(Elf_Scn *scn, size_t size) {
	Elf_Data		*dblock = 0;
	void			*data;

	if(!scn) {
		return 0;
	}

	if(size) {
		if(!(data = malloc(size))) {
			return 0;
		}
	} else {
		data = 0;
	}

	if((dblock = _elf_data_create())) {
		Elf_Data_Ref	*datap;

		dblock->d_buf = data;
		dblock->d_size = size;
		if((datap = (Elf_Data_Ref *)realloc(scn->s_data, (scn->s_fragments + 1) * sizeof *datap))) {
			datap[scn->s_fragments].d_data = dblock;
			datap[scn->s_fragments].d_flags = size ? ELF_DATAREF_ALLOCED : 0;
			scn->s_fragments++;
			scn->s_data = datap;
		}
	}
	return dblock;
}

Elf_Data *elf_getdata(Elf_Scn *scn, Elf_Data *data) {
	Elf_Data	*dp = 0;
	Elf		*elf;

	if(!scn) {
		return 0;
	}	
	elf = scn->s_elf;
	if(!data) {
		if(scn->s_fragments == 0) {
			Elf32_Ehdr				*ehdr;
			Elf32_Shdr				shdr;

			if ((ehdr = elf->e_ehdrp) == NULL) {
				ehdr = elf->e_ehdrp = elf32_getehdr (elf);
			}
			if(ehdr) {
				if(scn->s_ndx < ehdr -> e_shnum && ehdr -> e_shentsize >= sizeof shdr &&
						_elf_read(scn->s_elf, (char *)&shdr, ehdr -> e_shoff + scn->s_ndx * ehdr -> e_shentsize, sizeof shdr) == sizeof shdr &&
						 shdr.sh_size > 0) {

					Elf32_swapShdr( elf, &shdr, 1 );

#if 0
					if(dp = _elf32_newdata(scn, shdr.sh_size)) {
						_elf_read(scn->s_elf, dp->d_buf, shdr.sh_offset, shdr.sh_size);
#else
					if(dp = _elf32_newdata(scn, shdr.sh_type == SHT_NOBITS ? 0 : shdr.sh_size)) {
						if(shdr.sh_type == SHT_NOBITS) {
							dp->d_size = shdr.sh_size;
						} else {
							_elf_read(scn->s_elf, dp->d_buf, shdr.sh_offset, shdr.sh_size);
						}
#endif

						switch(shdr.sh_type) {
						case SHT_DYNAMIC:
							dp->d_type = ELF_T_DYN;
							break;
						case SHT_DYNSYM:
						case SHT_SYMTAB:
Elf32_swapSym ( elf, dp->d_buf, shdr.sh_size );
							dp->d_type = ELF_T_SYM;
							break;
						case SHT_HASH:
							dp->d_type = ELF_T_WORD;
							break;
						case SHT_REL:
Elf32_swapRela( elf, dp->d_buf, shdr.sh_size, 0 );
							dp->d_type = ELF_T_REL;
							break;
						case SHT_RELA:
							dp->d_type = ELF_T_RELA;
Elf32_swapRela( elf, dp->d_buf, shdr.sh_size, 1 );
							break;
						default:
							dp->d_type = ELF_T_BYTE;
							break;
						}
					}
					dp->d_align = shdr.sh_addralign;
				}
			}
		} else if(scn->s_fragments > 0 && scn->s_data) {
			dp = scn->s_data[0].d_data;
		}
	} else {
		int				i;

		for(i = 0; i < scn->s_fragments; i++) {
			if(scn->s_data[i].d_data == data) {
				if(i+1 < scn->s_fragments) {
					dp = scn->s_data[i+1].d_data;
				}
				break;
			}
		}
	}

	return dp;
}


Elf_Data *elf_newdata(Elf_Scn *scn) {
	Elf_Data		*dblock = 0;

	if(!scn) {
		return 0;
	}

	if((dblock = _elf32_newdata(scn, 0))) {
		scn->s_data[scn->s_fragments-1].d_flags |= ELF_F_DIRTY;
	}
	return dblock;
}

Elf_Data *elf_rawdata(Elf_Scn *scn, Elf_Data *data) {
	return 0;
}

size_t elf32_fsize(Elf_Type type, size_t size, unsigned ver) {
	if(ver != EV_CURRENT) {
		return 0;
	}
	switch(type) {
	case ELF_T_BYTE:
		size *= sizeof(unsigned char);
		break;
	case ELF_T_ADDR:
		size *= sizeof(Elf32_Addr);
		break;
	case ELF_T_HALF:
		size *= sizeof(Elf32_Half);
		break;
	case ELF_T_OFF:
		size *= sizeof(Elf32_Off);
		break;
	case ELF_T_SWORD:
		size *= sizeof(Elf32_Sword);
		break;
	case ELF_T_WORD:
		size *= sizeof(Elf32_Word);
		break;
	case ELF_T_EHDR:
		size *= sizeof(Elf32_Ehdr);
		break;
	case ELF_T_PHDR:
		size *= sizeof(Elf32_Phdr);
		break;
	case ELF_T_RELA:
		size *= sizeof(Elf32_Rela);
		break;
	case ELF_T_REL:
		size *= sizeof(Elf32_Rel);
		break;
	case ELF_T_SHDR:
		size *= sizeof(Elf32_Shdr);
		break;
	case ELF_T_SYM:
		size *= sizeof(Elf32_Sym);
		break;
	case ELF_T_DYN:
		size *= sizeof(Elf32_Dyn);
		break;
	default:
		return 0;
	}
	return size;
}

Elf_Kind elf_kind(Elf *elf) {
	if(!elf) {
		return ELF_K_NONE;
	}
	if(elf->e_arstrings) {
		return ELF_K_AR;
	}
	if(elf->e_ehdrp) {
		return ELF_K_ELF;
	}
	return ELF_K_NONE;
}

size_t elf_rand(Elf *elf, size_t offset) {
	return elf->e_offset = offset;
}

void elf_fill(int fill) {
	_elf_fill_char = fill;
}

unsigned long elf_hash(const char *name) {
	unsigned long		h = 0, g;

	while(*name) {
		h = (h << 4) + *name++;
		if(g = h & 0xf0000000) {
			h ^= g >> 24;
		}
		h &= ~g;
	}
	return h;
}

char *elf_strptr(Elf *elf, size_t section, size_t offset) {
	Elf_Scn				*scn;

	if(scn = elf_getscn(elf, section)) {
		Elf32_Shdr			*shdr;

		if((shdr = elf32_getshdr(scn)) && shdr->sh_type == SHT_STRTAB) {
			Elf_Data			*data;

			if((data = elf_getdata(scn, 0)) && data->d_type == ELF_T_BYTE) {
				char				*p = data->d_buf;

				return p + offset;
			}
		}
	}
	return 0;
}
