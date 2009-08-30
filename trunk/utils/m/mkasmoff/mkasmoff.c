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
#ifdef __USAGE
%C infile outfile

  Reads the 'infile' ELF or COFF object file (created with the macros
  in <mkasmoff.h>) and creates an assembler include file
  called 'outfile' containing the appropriate macro definitions.
#endif
*/
#include <lib/compat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include _NTO_HDR_(sys/elf.h)

#ifndef ENDIAN
	#if defined(__LITTLEENDIAN__) || defined(__X86__)
		#define ENDIAN (0)	
	#elif defined(__BIGENDIAN__) || defined(__sparc) 
		#define ENDIAN (1)	
	#else
		#error not configured for system
	#endif
#endif

#define TMS320C6X			0x0099
#define COFF_VID			0x00c2
							  
typedef struct {
	unsigned short		version_id;
	unsigned short		num_secthdrs;
	int					time_stamp;
	int					symtable_fptr;
	int					num_sym_entries;
	unsigned short		opthdr_size;
	unsigned short		flags;
	unsigned short		target_id;
} COFF_FHDR;

typedef struct {
	char				name[8];
	int					paddr;
	int					vaddr;
	int					sect_wsize;
	int					raw_fptr;
	int					reloc_fptr;
	int					line_fptr;
	unsigned int		num_relocs;
	unsigned int		num_lines;
	unsigned int		flags;
	unsigned short		reserved;
	unsigned short		mempage;
} COFF_SECTHDR;


typedef struct {
	char				name[8];
	int					value;
	short				sect_num;
	unsigned short		type;
	char				storage_class;
	char				num_aux;
} COFF_SYMBOL;

Elf32_Ehdr	ehdr;
Elf32_Shdr	*elf_shdr;
char		**elf_shdata;
unsigned	shnum;

char				tmp_name[9];
COFF_FHDR			coff_fhdr;
char				**coff_rawdata;							  

struct symbol {
	struct symbol	*next;
	unsigned		line;
	unsigned		sect;
	unsigned		off;
	char			name[1];	/* variable sized */
};

int				fd;
struct symbol	*list;
char			*comment_format;
char			*value_format;

struct {
	unsigned		machine;
	unsigned		big_endian;
	void			(*build_symbols)(void);
	void			(*get_data)( struct symbol *, unsigned, unsigned char *);
} info;

static void
add_symbol(unsigned sect, unsigned off, char *name) {
	char			*p;
	unsigned 		line = 0;
	struct symbol	*sym;
	struct symbol	**owner;

	#define COMMENT_PREFIX	"comment"
	#define VALUE_PREFIX	"value"
	
	if( (p = strstr( name, COMMENT_PREFIX )) != NULL ) {
		p += sizeof( COMMENT_PREFIX ) - 1;
		line = strtoul( p, &p, 10 );
		p += 1; /* skip the trailing "_" */
	} else if( (p = strstr( name, VALUE_PREFIX )) != NULL ) {
		p += sizeof( VALUE_PREFIX ) - 1;
		line = strtoul( p, &p, 10 );
		p += 4; /* skip the trailing "____" */
	}
	if( p != NULL ) {
		sym = malloc( sizeof( *sym ) + strlen( p ) );
		strcpy( sym->name, p );
		sym->sect = sect;
		sym->off = off;
		sym->line = line;
		/* add in order of increasing line number */
		owner = &list;
		for( ;; ) {
			if( *owner == NULL || (*owner)->line > line ) break;
			owner = &(*owner)->next;
		}
		sym->next = *owner;
		*owner = sym;
	}
}

static unsigned
Normal16(unsigned val) {
	if (info.big_endian == ENDIAN) return( val );
	return( ((val >> 8) & 0xff) | ((val << 8) & 0xff00) );
}

static unsigned
Normal32(unsigned val) {
	if (info.big_endian == ENDIAN) return( val );
	return( ((val >> 24) & 0x000000ff)
		  | ((val >> 8)  & 0x0000ff00)
		  | ((val << 8)  & 0x00ff0000)
		  | ((val << 24) & 0xff000000) );
}

// WARNING: this routine will reject anything other than a COFF file 
// intended for the TMS320C6x.
int
try_coff() {

		if( read( fd, &coff_fhdr, sizeof( COFF_FHDR ) ) != sizeof( COFF_FHDR ) ) {
			fprintf( stderr, "Could not read COFF header\n" );
			return( 0 );
		}													 
		
		info.big_endian = 0;		 
		
		if ( ( Normal16(coff_fhdr.target_id) != TMS320C6X ) ||
			  ( Normal16(coff_fhdr.version_id) != COFF_VID ) )
			return( 0 );
			
		shnum = Normal16( coff_fhdr.num_secthdrs );
		
		coff_rawdata = malloc( shnum * sizeof(*coff_rawdata) );
		if( coff_rawdata == NULL ) {
			fprintf( stderr, "No memory for raw data pointers\n" );
			return( 0 );
		}				
		
		memset( coff_rawdata, 0, shnum * sizeof(*coff_rawdata) );
		
		return 1;
		
}

static void *
coff_load_raw(int i) {
	int				secthdr_fptr;
	int				sectraw_fptr;
	COFF_SECTHDR	secthdr;
	int				raw_size;											  

	if( coff_rawdata[i] == NULL ) {
		// calculate the file offset of the section header
		secthdr_fptr = sizeof(COFF_FHDR) + 
							Normal16(coff_fhdr.opthdr_size) + 
							(sizeof(COFF_SECTHDR) * (i-1));
	
		// seek to the section header
		if ( lseek( fd, secthdr_fptr, SEEK_SET ) != secthdr_fptr ) {
			printf("Couldn't seek to section header #%d\n", i);
			exit(1);
		}
	
		// read in the section header
		if ( read( fd, &secthdr, sizeof(COFF_SECTHDR) ) != sizeof(COFF_SECTHDR) ) {
			printf( "Couldn't read section header #%d\n", i);
			exit(1);
		}
	
		// allocate memory for the raw data
		raw_size = Normal32(secthdr.sect_wsize);
		coff_rawdata[i] = malloc( raw_size );
		if ( coff_rawdata[i] == NULL) {
			printf( "Couldn't allocate memory for raw data of section #%d\n", i);
			exit(1);
		}
	
		// seek to the section's raw data
		sectraw_fptr = Normal32(secthdr.raw_fptr);
		if ( lseek( fd, sectraw_fptr, SEEK_SET ) != sectraw_fptr ) {
			printf("Couldn't seek to raw dadta of section #%d\n", i);
			exit(1);
		}
	
		// read in the section's raw data
		if ( read( fd, coff_rawdata[i], raw_size ) != raw_size ) {
			printf( "Couldn't read raw data of section #%d\n", i);
			exit(1);
		}
	}
	return( coff_rawdata[i] );
}

#define SYMTABLE_FPTR (Normal32(coff_fhdr.symtable_fptr))

#define SYMTABLE_SIZE (Normal32(coff_fhdr.num_sym_entries) *					\
							  sizeof(COFF_SYMBOL))

#define STRINGTBL_FPTR (SYMTABLE_FPTR + SYMTABLE_SIZE)

static char *
coff_load_string_table() {
	int				table_fptr = STRINGTBL_FPTR;
	int				table_size;
	void			*table;

	// seek to the string table
	if ( lseek( fd, table_fptr, SEEK_SET ) != table_fptr ) {
		printf("Couldn't seek to the string table\n");
		exit(1);
	}								
	
	// read in the size of the string table
	if ( read( fd, &table_size, 4 ) != 4 ) {
		printf( "Couldn't read string table size\n");
		exit(1);
	}
	table_size = Normal32(table_size);
	
	// allocate memory for the symbol table
	table = malloc( table_size );
	if ( table == NULL) {
		printf( "Couldn't allocate memory for the string table\n");
		exit(1);
	}
	
	// seek back to the begining of the string table
	if ( lseek( fd, table_fptr, SEEK_SET ) != table_fptr ) {
		printf("Couldn't seek to the string table\n");
		exit(1);
	}								
	// read in the string table
	if ( read( fd, table, table_size ) != table_size ) {
		printf( "Couldn't read string table\n");
		exit(1);
	}
	return table;
}			

static char *
coff_get_name(COFF_SYMBOL *symbol, char *table) {

	if ( symbol->name[0] == 0 ) {// get name from the string table
		return (table + Normal32(*((unsigned *) &symbol->name[4])));
	} else {		// get name directly from the symbol table entry
		memcpy(tmp_name, symbol->name, 8);
		tmp_name[8] = '\0'; 
		return tmp_name;
	}  
}

static COFF_SYMBOL *
coff_load_symbol_table() {
	int				table_size = SYMTABLE_SIZE;
	int				table_fptr = SYMTABLE_FPTR;
	void				*table;

	// seek to the symbol table
	if ( lseek( fd, table_fptr, SEEK_SET ) != table_fptr ) {
		printf("Couldn't seek to symbol table\n");
		exit(1);
	}

	// allocate memory for the symbol table
	table = malloc( table_size );
	if ( table == NULL) {
		printf( "Couldn't allocate memory for the symbol table\n");
		exit(1);
	}

	// read in the symbol table
	if ( read( fd, table, table_size ) != table_size ) {
		printf( "Couldn't read the symbol table\n");
		exit(1);
	}
	
	return table;
}


static void
coff_build_symbols() {
	unsigned		i;
	COFF_SYMBOL	*syms = coff_load_symbol_table();
	char 			*strings = coff_load_string_table();
	char			*name;
	int	 		num = Normal32(coff_fhdr.num_sym_entries);
	COFF_SYMBOL	*sym;
	
	for(i = 0; i < num; i++) {

		sym = &syms[i];
		name = coff_get_name(sym, strings);
		add_symbol( Normal16(sym->sect_num), Normal32(sym->value)+8, name );
		if(sym->num_aux) i++;
	}
}

static void
coff_get_data(struct symbol *sym, unsigned size, unsigned char *buff) {
	char		*p;

	p = (char *)coff_load_raw( sym->sect-1 ) + sym->off;

	if( size == 0 ) size = strlen( p ) + 1;
	memcpy( buff, p, size );
}

static int
try_omf(int fd) {
	/* OMF not supported yet */
	return( 0 );
}

static void
omf_build_symbols() {
}

static void
omf_get_data(struct symbol *sym, unsigned size, unsigned char *buff) {
}

static void *
elf_load_section(unsigned i) {
	unsigned long	off;
	unsigned long	size;

	if( elf_shdata[i] == NULL ) {
		size = Normal32( elf_shdr[i].sh_size );
		if( Normal32( elf_shdr[i].sh_type ) == SHT_NOBITS ) {
			elf_shdata[i] = calloc( 1, size );
			if( elf_shdata[i] == NULL ) {
				fprintf( stderr, "No memory for section data\n" );
				exit( 1 );
			}
		} else {
			off = Normal32( elf_shdr[i].sh_offset );
			if( lseek( fd, off, SEEK_SET ) != off ) {
				fprintf( stderr, "Could not seek to section %u data\n", i );
				exit( 1 );
			}
			elf_shdata[i] = malloc( size );
			if( elf_shdata[i] == NULL ) {
				fprintf( stderr, "No memory for section data\n" );
				exit( 1 );
			}
			if( read( fd, elf_shdata[i], size ) != size ) {
				fprintf( stderr, "Could not read section data\n" );
				exit( 1 );
			}
		}
	}
	return( elf_shdata[i] );
}

static void
elf_build_symbols() {
	unsigned	i;

	for( i = 0; i < shnum; ++i ) {
		Elf32_Shdr		*shdr = &elf_shdr[i];
   	
		if( Normal32( shdr->sh_type ) == SHT_SYMTAB ) {
			Elf32_Sym	*syms = elf_load_section( i );
			char 		*strings = elf_load_section( Normal32( shdr->sh_link ) );
			int			num = Normal32( shdr->sh_size ) / sizeof( *syms );
			int			i;
	
			for(i = 0; i < num; i++) {
				Elf32_Sym			*sym = &syms[i];
		
				if(Normal16(sym->st_shndx) != SHN_UNDEF) {
					char	*name = strings + Normal32(sym->st_name);

					if ( ELF32_ST_TYPE(sym->st_info) == STT_OBJECT ) {
						add_symbol( Normal16(sym->st_shndx), Normal32(sym->st_value), name );
					}
				}
			}
		}
	}
}

static void
elf_get_data(struct symbol *sym, unsigned size, unsigned char *buff) {
	char		*p;

	p = (char *)elf_load_section( sym->sect ) + sym->off;

	if( size == 0 ) size = strlen( p ) + 1;
	memcpy( buff, p, size );
}

static void
comment(char *comment) {
	printf( comment_format, comment );
}

static void
value(char *name, unsigned long value) {
	printf( value_format, name, value );
}

struct format {
	char	*name;
	char	*comment;
	char	*value;
};

const static struct format formats[] = {
	{ 	"masm", 	"; %s\n", 		"%s = 0%8.8lxh\n" },
	{ 	"gas", 		"# %s\n", 		".set %s, 0x%8.8lx\n" },
	{ 	"cpp", 		"/* %s */\n",	"#define %s 0x%8.8lx\n" },
	{ 	"c6x", 		"; %s\n", 		"%s .set 0%8.8lxh\n" },
	{	NULL,		NULL,			NULL  },
};

int
main(int argc, char *argv[]) {
	char			buff[256];
	unsigned long	off;
	unsigned		size;
	int				n;
	int				i;
	char			*format = "";

	while((n = getopt(argc, argv, "f:")) != -1) {
		switch(n) {
		case 'f':
			format = optarg;
			break;
		}
	}
	argv += optind;
	if( argv[0] == NULL || argv[1] == NULL ) {
		fprintf( stderr, "Usage: mkasmoff [-f <format> ] <infile> <outfile>\n" );
		return( 1 );
	}
	fd = open( argv[0], O_RDONLY | O_BINARY );
	if( fd == -1 ) {
		fprintf( stderr, "Can not open '%s': %s\n", argv[0], strerror( errno ) );
		return( 1 );
	}
	if( freopen( argv[1], "w", stdout ) == NULL ) {
		fprintf( stderr, "Can not open '%s': %s\n", argv[1], strerror( errno ) );
		return( 1 );
	}
	if( try_omf( fd ) ) {
		info.machine = EM_386;
		info.get_data = omf_get_data;
		info.build_symbols = omf_build_symbols;
	} else if( try_coff() ) {
		info.machine = TMS320C6X;
		info.get_data = coff_get_data;
		info.build_symbols = coff_build_symbols;
	} else {
		lseek( fd, 0, SEEK_SET );
		if( read( fd, &ehdr, sizeof( ehdr ) ) != sizeof( ehdr ) ) {
			fprintf( stderr, "Could not read ELF header\n" );
			return( 1 );
		}
		if( memcmp( ehdr.e_ident, ELFMAG, SELFMAG ) != 0 ) {
			fprintf( stderr, "Doesn't look like an ELF file\n" );
			return( 1 );
		}
		switch( ehdr.e_ident[EI_DATA] ) {
		case ELFDATA2MSB:
			info.big_endian = 1;
			break;
		}
		switch( Normal16(ehdr.e_type) ) {
		case ET_REL:
		    break;
		default:
			fprintf( stderr, "Don't understand ELF type %d\n", Normal16(ehdr.e_type) );
			return( 1 );
		}
		if( Normal16( ehdr.e_shentsize ) != sizeof( Elf32_Shdr ) ) {
			fprintf( stderr, "Wrong section entry size\n" );
			return( 1 );
		}
		shnum = Normal16( ehdr.e_shnum );
		size = shnum * sizeof( Elf32_Shdr );
		elf_shdr = malloc( size );
		if( elf_shdr == NULL ) {
			fprintf( stderr, "No memory for section header\n" );
			return( 1 );
		}
		off = Normal32( ehdr.e_shoff );
		if( lseek( fd, off, SEEK_SET ) != off ) {
			fprintf( stderr, "Seek for section header failed\n" );
			return( 1 );
		}
		if( read( fd, elf_shdr, size ) != size ) {
			fprintf( stderr, "Could not read section header table\n" );
			return( 1 );
		}
		elf_shdata = malloc( shnum * sizeof(*elf_shdata) );
		if( elf_shdata == NULL ) {
			fprintf( stderr, "No memory for section data pointers\n" );
			return( 1 );
		}
		memset( elf_shdata, 0, shnum * sizeof(*elf_shdata) );
		info.machine = Normal16( ehdr.e_machine );
		info.get_data = elf_get_data;
		info.build_symbols = elf_build_symbols;
	}
	if( format[0] == '\0' ) {

//For ACME
#ifndef EM_ARM
	#define EM_ARM	40
#endif
#ifndef EM_SH
	#define EM_SH	42
#endif

		switch( info.machine ) {
		case EM_386:
			format = "masm";
			break;
		case EM_PPC:
		case EM_SH:
			format = "gas";
			break;
		case EM_MIPS:
		case EM_ARM:
			format = "cpp";
			break;
		case TMS320C6X:
			format = "c6x";
			break;
		default:
			fprintf( stderr, "Don't know machine type %d\n", ehdr.e_machine );
			return( 1 );
		}
	}
	i = 0;
	for( ;; ) {
		if(formats[i].name == NULL) {
			fprintf( stderr, "Unrecognized output format '%s'\n", format );
			return( 1 );
		}
		if(strcmp(formats[i].name, format) == 0) break;
		++i;
	}
	comment_format = formats[i].comment;
	value_format = formats[i].value;

	comment( " This file was produced by:" );
	sprintf( buff, "    mkasmoff -f%s %s %s", format, argv[0], argv[1] );
	comment( buff );
	sprintf( buff, "    (machine type is %d)", info.machine );
	comment( buff );
	comment( "" );

    info.build_symbols();
	while( list != NULL ) {
		if( list->name[0] == '\0' ) {
			info.get_data( list, 0, buff );
			comment( "" );
			comment( buff );
			comment( "" );
		} else {
			unsigned char	buff[4];

			info.get_data( list, 4, buff );
			if( info.big_endian ) {
				value( list->name,
					(unsigned long)buff[3]
					+ ((unsigned long)buff[2] << 8)
					+ ((unsigned long)buff[1] << 16)
					+ ((unsigned long)buff[0] << 24) );
			} else {
				value( list->name,
					(unsigned long)buff[0]
					+ ((unsigned long)buff[1] << 8)
					+ ((unsigned long)buff[2] << 16)
					+ ((unsigned long)buff[3] << 24) );
			}
		}
		list = list->next;
	}
	return( 0 );
}
