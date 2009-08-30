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
%C - mkifs filter program for creating Densan multi-segment boot files.

%C	image-file
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
#include _NTO_HDR_(sys/startup.h)

#define SWAP32( val ) ( (((val) >> 24) & 0x000000ff)	\
					  | (((val) >> 8)  & 0x0000ff00)	\
					  | (((val) << 8)  & 0x00ff0000)	\
					  | (((val) << 24) & 0xff000000) )

#define SWAP16( val ) ( (((val) >> 8) & 0x00ff)	\
					  | (((val) << 8) & 0xff00) )


struct ms_header {
	uint32_t		num_sects;
	uint32_t		start_addr;
	uint32_t		loc_addr;
	uint32_t		sect_size;
};

int host_endian;
int target_endian;

long 
swap32(long val) {

	return(host_endian != target_endian ? SWAP32(val) : val);
}

int
main( int argc, char *argv[] ) {
	int							n;
	FILE						*fp;
	struct startup_header		shdr;
	struct ms_header			hdr;
	struct stat					sbuf;
	char						*name;

	// Calculate the endian of the host this program is running on.
	n = 1;
	host_endian = *(char *)&n != 1;

	if( argc < 2 ) {
		fprintf( stderr, "Missing file name.\n" );
		return( 1 );
	}
	name = argv[argc-1];

	fp = fopen( name, "r+b" );
	if( fp == NULL ) {
		fprintf( stderr, "Can not open '%s': %s\n", name, strerror(errno) );
		return( 1 );
	}
	if( fstat( fileno(fp), &sbuf ) == -1 ) {
		fprintf( stderr, "Can not stat '%s': %s\n", name, strerror(errno) );
		return( 1 );
	}
	if( fseek( fp, sizeof(hdr), SEEK_SET ) != 0 ) {
		fprintf( stderr, "Can not seek '%s': %s\n", name, strerror(errno) );
		return( 1 );
	}
	if( fread( &shdr, sizeof(shdr), 1, fp ) != 1 ) {
		fprintf( stderr, "Can not read from '%s': %s\n", name, strerror(errno) );
		return( 1 );
	}
	if( fseek( fp, 0, SEEK_SET ) != 0 ) {
		fprintf( stderr, "Can not rewind '%s': %s\n", name, strerror(errno) );
		return( 1 );
	}

	target_endian = ((shdr.flags1 & STARTUP_HDR_FLAGS1_BIGENDIAN) != 0);

	hdr.num_sects = swap32( 1 );
	hdr.start_addr = shdr.startup_vaddr;
	hdr.loc_addr = swap32(swap32(shdr.image_paddr) + swap32(shdr.paddr_bias));
	hdr.sect_size = swap32( sbuf.st_size - sizeof(hdr) );

	if( fwrite( &hdr, sizeof(hdr), 1, fp ) != 1 ) {
		fprintf( stderr, "Can not update '%s': %s\n", name, strerror(errno) );
		return( 1 );
	}
	if( fclose( fp ) != 0 ) {
		fprintf( stderr, "Error closing '%s': %s\n", name, strerror(errno) );
		return( 1 );
	}
	return( 0 );
}

__SRCVERSION("mkifsf_densan.c $Rev: 153052 $");
