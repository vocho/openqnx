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
%C - mkifs filter program for creating OpenBIOS boot files

%C	startup-offset image-file
#endif

#include <lib/compat.h>

#ifdef _NTO_HDR_DIR_
#define _PLATFORM(x) x
#define PLATFORM(x) <_PLATFORM(x)sys/platform.h>
#include PLATFORM(_NTO_HDR_DIR_)
#endif

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include _NTO_HDR_(sys/startup.h)

#define SWAP32( val ) ( (((val) >> 24) & 0x000000ff)	\
					  | (((val) >> 8)  & 0x0000ff00)	\
					  | (((val) << 8)  & 0x00ff0000)	\
					  | (((val) << 24) & 0xff000000) )

int host_endian;

long 
swap32(int target_endian, long val) {

	return(host_endian != target_endian ? SWAP32(val) : val);
}


/*
 * Boot block header for PowerPC evaluation boards. These fields are
 * _always_ written in big-endian form. Image data starts immediately
 * following this header.
 */
struct openbios_boot_block {
	unsigned long	magic;
	unsigned long	dest;
	unsigned long	num_512blocks;
	unsigned long	debug_flag;
	unsigned long	entry_point;
	unsigned long	reserved[3];
};

int
main( int argc, char *argv[] ) {
	int							n;
	FILE						*fp;
	struct {
		struct openbios_boot_block	boot;
		struct startup_header		startup;
	}							hdr;
	struct stat					sbuf;
	char						*name;
	int							grow;
	unsigned long				new_size;
	static char					fill[511];

	// Calculate the endian of the host this program is running on.
	n = 1;
	host_endian = *(char *)&n != 1;

	if( argc <= 1 ) {
		fprintf( stderr, "Missing file name offset.\n" );
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
	if( fread( &hdr, sizeof(hdr), 1, fp ) != 1 ) {
		fprintf( stderr, "Can not read from '%s': %s\n", name, strerror(errno) );
		return( 1 );
	}
	new_size = ((sbuf.st_size + 511) & ~511) - sizeof(hdr.boot);
	grow = new_size - sbuf.st_size;
	if(grow < 0) grow += 512;
	if(grow > 0) {
		if( fseek( fp, 0, SEEK_END ) != 0 ) {
			fprintf( stderr, "Can not grow '%s': %s\n", name, strerror(errno) );
			return( 1 );
		}
		if( fwrite( &fill, grow, 1, fp ) != 1 ) {
			fprintf( stderr, "Can not grow '%s': %s\n", name, strerror(errno) );
			return( 1 );
		}
	}

	hdr.boot.magic = swap32(1, 0x0052504f);
	hdr.boot.dest = swap32(1, swap32(1, hdr.startup.image_paddr) + swap32(1, hdr.startup.paddr_bias));
	hdr.boot.entry_point = hdr.startup.startup_vaddr;
	hdr.boot.num_512blocks = swap32(1, new_size / 512);
	if( fseek( fp, 0, SEEK_SET ) != 0 ) {
		fprintf( stderr, "Can not rewind '%s': %s\n", name, strerror(errno) );
		return( 1 );
	}
	if( fwrite( &hdr.boot, sizeof(hdr.boot), 1, fp ) != 1 ) {
		fprintf( stderr, "Can not update '%s': %s\n", name, strerror(errno) );
		return( 1 );
	}
	if( fclose( fp ) != 0 ) {
		fprintf( stderr, "Error closing '%s': %s\n", name, strerror(errno) );
		return( 1 );
	}
	return( 0 );
}

#ifdef __QNXNTO__
__SRCVERSION("mkifsf_openbios.c $Rev: 153052 $");
#endif
