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
#define PLATFORM(x)		<_PLATFORM(x)/sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif
#include <inttypes.h>
#include <libelf_int.h>

// stolen from elfdump program


#define _DEBUG


void swap_16( void *ptr )
{
        unsigned char tmp, *p = ptr;

        tmp = p[0]; p[0] = p[1]; p[1] = tmp;
}


void swap_32( void *ptr )
{
        unsigned char tmp, *p = ptr;

        tmp = p[0]; p[0] = p[3]; p[3] = tmp;
        tmp = p[1]; p[1] = p[2]; p[2] = tmp;
}

// only for wcc and qnx4/x86
// #define __X86__

static 
int big_endian( Elf *elf ) 
{
	int xlat;

        xlat = 0;
	switch( elf->e_ehdrp->e_ident[ EI_DATA ])
	{
case ELFDATA2LSB:
#if defined(__BIGENDIAN__ )
                        xlat = 1;
#endif

#ifdef DEBUG
printf( "\nElf32_getehdr(): LE ==> %s (host)",  xlat ? "BE" : "LE" );
flushall();
#endif

			break;
case ELFDATA2MSB:
#if defined(__LITTLEENDIAN__ ) || defined(__X86__)
                        xlat = 1;
#endif

#ifdef DEBUG
printf( "\nElf32_getehdr(): BE ==> %s (host)", !xlat ? "BE" : "LE" );
flushall();
#endif

			break;
default:
			break;
	}

	return xlat;
}


//static
int Elf32_swapEhdr( Elf *elf, Elf32_Ehdr *ehdr )
{
#ifdef DEBUG
	printf( "\nElf32_swapEhdr()" );
	flushall();
#endif

	if( !ehdr )
		ehdr = elf->e_ehdrp;

	elf->xlat = big_endian( elf );

        if( elf->xlat )
	{
/*
printf( "\ntype=%d", ehdr->e_type );
flushall();
 */

               swap_16( &ehdr->e_type );
/*
printf( "\ntype=%d", ehdr->e_type );
flushall();
 */
                swap_16( &ehdr->e_machine );
                swap_32( &ehdr->e_version );
                swap_32( &ehdr->e_entry );
                swap_32( &ehdr->e_phoff );
                swap_32( &ehdr->e_shoff );
                swap_32( &ehdr->e_flags );
                swap_16( &ehdr->e_ehsize );
                swap_16( &ehdr->e_phentsize );
                swap_16( &ehdr->e_phnum );
                swap_16( &ehdr->e_shentsize );
                swap_16( &ehdr->e_shnum );
                swap_16( &ehdr->e_shstrndx );
        }

        return 0;
}


//static
int Elf32_swapPhdr( Elf *elf )
{
	Elf32_Phdr *phdr = elf->e_phdrp;
	long count       = elf->e_ehdrp->e_phnum;
	long num;

#ifdef DEBUG
	printf( "\nElf32_swapPhdr()" );
	flushall();
#endif

        if( elf->xlat )
	{
		for( num = count; num--; phdr++ )
		{
                	swap_32( &phdr->p_type );
                	swap_32( &phdr->p_offset );
                	swap_32( &phdr->p_vaddr );
                	swap_32( &phdr->p_paddr );
                	swap_32( &phdr->p_filesz );
                	swap_32( &phdr->p_memsz );
                	swap_32( &phdr->p_flags );
                	swap_32( &phdr->p_align );
		}
        }

        return 0;
}


//static
int Elf32_swapShdr( Elf *elf, Elf32_Shdr *shdr, int count )
{
	long num;

#ifdef DEBUG
/*
	printf( "\nElf32_swapShdr()" );
	flushall();
 */
#endif

        if( elf->xlat )
	{
		for( num = count; num--; shdr++ )
		{
                	swap_32( &shdr->sh_name );
                	swap_32( &shdr->sh_type );
                	swap_32( &shdr->sh_flags );
                	swap_32( &shdr->sh_addr );
                	swap_32( &shdr->sh_offset );
                	swap_32( &shdr->sh_size );
                	swap_32( &shdr->sh_link );
                	swap_32( &shdr->sh_info );
                	swap_32( &shdr->sh_addralign );
                	swap_32( &shdr->sh_entsize );
		}
        }

        return 0;
}


// confusing names Elf32_swapRel() or Elf32_swapRela()!!!
int Elf32_swapRel( Elf *elf, Elf32_Rela *rel, int use_rela )
{
	if( elf->xlat )
	{
                swap_32( &rel->r_offset );
                swap_32( &rel->r_info );
                if( use_rela )
                {
                        swap_32( &rel->r_addend );
                }
        }

	return 0;
}


//static
int Elf32_swapRela( Elf *elf, char *buf, long size, int use_rela )
{
	long num;
	long count;

        if( elf->xlat )
	{
		unsigned relsize = ( use_rela ? sizeof( Elf32_Rela ) : sizeof( Elf32_Rel ));

		count = size / relsize;
		for( num = count; num--; buf += relsize )
		{
			Elf32_Rela *rel = (Elf32_Rela *)buf;

                	swap_32( &rel->r_offset );
                	swap_32( &rel->r_info );
			if( use_rela )
			{
                		swap_32( &rel->r_addend );
			}
		}
        }

        return 0;
}


int Elf32_swapSym( Elf *elf, Elf32_Sym *sym, long size )
{
	long num;
	long count;

#ifdef DEBUG
	printf( "\nElf32_swapSym()-%d", elf->xlat );
	flushall();
#endif

        if( elf->xlat )
	{
		count = size / sizeof( Elf32_Sym );
		for( num = count; num--; sym++ )
		{
       	        	swap_32( &sym->st_name );
                	swap_32( &sym->st_value );
                	swap_32( &sym->st_size );
                	swap_16( &sym->st_shndx );
		}
        }

        return 0;
}


int Elf32_swapDyn( Elf *elf, Elf32_Dyn *dyn )
{
#ifdef DEBUG
	printf( "\nElf32_swapDyn()-%d", elf->xlat );
	flushall();
#endif

        if( elf->xlat )
        {
               	swap_32( &dyn->d_tag );
               	swap_32( &dyn->d_un  );
        }

        return 0;
}
