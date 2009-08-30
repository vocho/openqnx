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





#include <errno.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <process.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/elf.h>
#include <sys/types.h>

int check_dynamic( char *pathname )
{
int			fd, i;
Elf32_Ehdr	ehdr;
Elf32_Phdr	*phdr = NULL;

	fd = open( pathname, O_RDONLY );

	if ( read( fd, &ehdr, sizeof(ehdr) ) < sizeof(ehdr) ) {
		goto corrupt;
	}

	/* Check ELF magic */
	if (   ehdr.e_ident[0] != 0x7f 
		|| ehdr.e_ident[1] != 'E'
		|| ehdr.e_ident[2] != 'L'
		|| ehdr.e_ident[3] != 'F' ) {
		goto corrupt;
	}

	if ( ehdr.e_type == ET_DYN )
		return 0;

	if ( ehdr.e_type != ET_EXEC ) {
		fprintf( stderr, "ldd: %s: not an ELF executable binary or shared object\n", pathname );
		return -1;
	}

	/* read program headers */
	if ( (phdr = malloc( ehdr.e_phentsize * ehdr.e_phnum )) == NULL ) {
		fprintf( stderr, "ldd: allocating phdrs: %s\n", strerror(errno) );
		return -1;
	}
	if ( -1 == lseek( fd, ehdr.e_phoff, SEEK_SET ) )
		goto corrupt;
	if ( (ehdr.e_phentsize * ehdr.e_phnum) != read( fd, phdr, (ehdr.e_phentsize * ehdr.e_phnum) ) )
		goto corrupt;

	for ( i = 1; i < ehdr.e_phnum; i++ ) {
		if ( phdr[i].p_type == PT_INTERP )
			break;
	}

	if ( i == ehdr.e_phnum ) {
		fprintf( stderr, "ldd: %s: not a dynamically linked ELF file\n", pathname );
		return -1;
	}

	return 0;
corrupt:
	fprintf( stderr, "ldd: %s: corrupt ELF file\n", pathname );
	return -1;
}

int main( int argc, char *argv[] )
{
void	*handle;
char	*args[3], *temp;
int		i;

	if ( argc == 1 ) {
		fprintf( stderr, "ldd: please specify one or more executables\n" );
		exit( EXIT_FAILURE );
	}

	if ( argc > 2 ) {
		for ( i = 1; i < argc; i++ ) {
			args[0] = argv[0];
			args[1] = argv[i];
			args[2] = NULL;
			spawnvp( P_WAIT, args[0], args );
		}
	}
	else {
		args[0] = argv[1];
		args[1] = NULL;

		putenv("LD_TLO=1");

		if((args[0][0] != '.' && args[0][1] != '/')
		    && args[0][0] != '/') {
			temp = malloc( strlen(args[0]) + 3 );
			if ( temp == NULL ) {
				fprintf( stderr, "ldd: %s: %s\n", "allocating name", strerror(errno) );
				exit( EXIT_FAILURE );
			}
			sprintf( temp, "./%s", args[0] );
			args[0] = temp;
		}

		if ( access( args[0], R_OK ) == -1 ) {
			fprintf( stderr, "ldd: %s: %s\n", args[0], strerror(errno) );
			exit( EXIT_FAILURE );
		}

		if ( check_dynamic( args[0] ) == -1 ) {
			exit( EXIT_FAILURE );
		}

		fprintf( stdout, "%s:\n", args[0] );

		execv( args[0], args );
		if ( errno == ELIBEXEC ) {
			handle = dlopen( args[0], 0 );
			if ( handle == NULL ) {
				if ( dlerror() )
					fprintf( stderr, "ldd: %s: %s\n", args[0], dlerror() );
			}
		}
		else {
			fprintf( stderr, "ldd: %s: %s\n", args[0], strerror(errno) );
			exit( EXIT_FAILURE );
		}
	}

	return 0;
}
