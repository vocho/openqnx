/*				11-Dec-89 10:34:00am										*/

/*--------------------------------------------------------------------------*/
/*  History: crctest.c, V0.0, 17-Mar-87 10:56:38am, Dan Hildebrand,	Baseline*/
/*----------------------------------------------------------------------------

     A collection of test routines to benchmark and verify correctness for
     the CRC calculation routines.

----------------------------------------------------------------------------*/
#include <stdio.h>
#include <time.h>
#define	EXT
#include "crc.h"

#include "crctest.def"

#define	LOOP_LIMIT	200		/* # of iterations for time benchmarks			*/
#define	SIZE		2048
#define	SDLC_POLY 	0x1021
#define	CCITT_POLY 	0x8408	/* 1021 in reverse order */

char	buf[ SIZE ];
extern long get_crc_32();

void
main( argc, argv )
	int		argc;
	char	*argv[];
	{
	register int i;
	unsigned crc, sample, test;
	unsigned long start, end;

	for ( i = 0; i < SIZE; ++i ) buf[i] = i;	/* Prepare test buffer		*/

	use_crc = 1;

	start = clock();
	for ( i = 0; i < LOOP_LIMIT; ++i ) {
		clear();
		compute( SIZE, buf );
		}
	end = clock();
	printf( "function call overhead              Time = %ld ticks.\n",
							              (end - start) / 50L );

	start = clock();
	for ( i = 0; i < LOOP_LIMIT; ++i ) {
		clear_crc_16();
		compute_crc_16( SIZE, buf );
		}
	end = clock();
	printf( "Table-driven CRC-16    = %04x       Time = %ld ticks.\n",
							get_crc_16(), (end - start) / 50L );

	start = clock();
	for ( i = 0; i < LOOP_LIMIT; ++i ) {
		crc = bit_crc2( buf, SIZE, 0 );
		}
	end = clock();
	printf( "Bit-wise     CRC-CCITT = %04x       Time = %ld ticks.\n",
							crc, (end - start) / 50L );

	start = clock();
	for ( i = 0; i < LOOP_LIMIT; ++i ) {
		clear_crc_ccitt();
		compute_crc_ccitt( SIZE, buf );
		}
	end = clock();
	printf( "Table-driven CRC-CCITT = %04x       Time = %ld ticks.\n",
							get_crc_ccitt(), (end - start) / 50L );

	start = clock();
	for ( i = 0; i < LOOP_LIMIT; ++i ) {
		crc = bit_crc( buf, SIZE, 0 );
		crc = bit_crc( "\0\0", 2, crc );
		}
	end = clock();
	printf( "Bit-wise     CRC-SDLC  = %04x       Time = %ld ticks.\n",
							crc, (end - start) / 50L );

	start = clock();
	for ( i = 0; i < LOOP_LIMIT; ++i ) {
		clear_crc_sdlc();
		compute_crc_sdlc( SIZE, buf );
		}
	end = clock();
	printf( "Semi_byte    CRC-SDLC  = %04x       Time = %ld ticks.\n",
							get_crc_sdlc(), (end - start) / 50L );

	start = clock();
	for ( i = 0; i < LOOP_LIMIT; ++i ) {
		clear_crc_z();
		compute_crc_z( SIZE, buf );
		}
	end = clock();
	printf( "Table-driven CRC-SDLC  = %04x       Time = %ld ticks.\n",
							sample = get_crc_z(), (end - start) / 50L );

	time( &start );
	for ( i = 0; i < LOOP_LIMIT; ++i ) {
		clear_crc_32();
		compute_crc_32( SIZE, buf );
		}
	time( &end );
	printf( "Table-driven CRC-32    = %08lx   Time = %ld seconds.\n",
							get_crc_32(), end - start );

	exit( 0 );
	}

/*----------------------------------------------------------------------------
     The conventional, bit-shifting CRC calculation algorithm.

IMPORT:	buffer - Pointer to the buffer to compute the CRC for.
		count - Number of bytes in the buffer.
		crc - Current value of the CRC.
EXPORT: crc - New value of the crc.
----------------------------------------------------------------------------*/
bit_crc( buffer, count, crc )
	char *buffer;
	int count;
	unsigned crc;
	{
	register char *b = buffer;
	register int crc_reg = crc;
	unsigned shifter, flag;

	if ( use_crc ) {
		while( count-- ) {
			for( shifter = 0x80; shifter; shifter >>= 1 ) {
				flag = crc_reg & 0x8000; 
				crc_reg <<= 1;
				crc_reg |= ((shifter & *b) ? 1 : 0);
				if ( flag ) crc_reg ^= SDLC_POLY;
				}
			++b;
			}
		}
	else
		while( count-- ) crc_reg += *b++;
	return( crc_reg );
	}

bit_crc2( buffer, count, crc )
	char *buffer;
	int count;
	unsigned crc;
	{
	register char *b = buffer;
	register unsigned crc_reg = crc;
	unsigned shifter, flag, ch;
	
	if ( use_crc ) {
		while( count-- ) {
			ch = *b++;
			for( shifter = 8; shifter; shifter--) {
				flag = (crc_reg & 0x0001) ^ (ch & 0x0001); 
				crc_reg >>= 1;
				ch >>= 1;
				if ( flag ) crc_reg ^= CCITT_POLY;
				}
			}
		}
	else
		while( count-- ) crc_reg += *b++;
	return( crc_reg );
	}

/* Empty functions to compute function call overhead	*/
clear() {
	return( 0 );
	}

compute( n, p )
	int n;
	char *p;
	{
	return( 0 );
	}
