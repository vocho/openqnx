/*				25-Sep-88  8:17:14am										*/

/*--------------------------------------------------------------------------*/
/*  History: crc_sdlc.c, V0, 13-Jan-87 11:36:31am, Dan Hildebrand,Baseline	*/
/*----------------------------------------------------------------------------

     CRC routines for Xmodem. Xmodem uses athe SDLC CRC polynomial and the
     routine to compute it here comes from an article by Curtis Wright  in
     the Summer, 1987 issue of "The C Journal".

----------------------------------------------------------------------------*/
#include <stdio.h>
#define	EXT	extern
#include "crc.h"

static unsigned			_crc;					/* CRC accumulator			*/
static unsigned char	checksum;				/* Checksum accumulator		*/

unsigned
get_crc_sdlc() {
	return( use_crc ? _crc : checksum );
	}

compute_crc_sdlc( count, buffer )
	int count;
	char *buffer;
	{
	register char *b = buffer;
	register unsigned crc_reg = 0, code;

	if ( use_crc ) {
		while( count-- ) {
			code = (crc_reg >> 8) & 0xff;
			code ^= (*b++ & 0xff);
			code ^= code >> 4;
			crc_reg <<= 8;
			crc_reg ^= code;
			code <<= 5;
			crc_reg ^= code;
			code <<= 7;
			crc_reg ^= code;
			}
		return( _crc = crc_reg );
		}
	for ( b = buffer; count; --count )	crc_reg += *b++;
	return( checksum = crc_reg );
	}

unsigned
next_crc_sdlc( ch )
	unsigned char ch;
	{
	register unsigned crc_reg = _crc, code;

	if ( use_crc ) {
		code = (crc_reg >> 8) & 0xff;
		code ^= (ch & 0xff);
		code ^= code >> 4;
		crc_reg <<= 8;
		crc_reg ^= code;
		code <<= 5;
		crc_reg ^= code;
		code <<= 7;
		crc_reg ^= code;
		_crc = crc_reg;
		}
	else	checksum += ch;
	return( ch );
	}

void
clear_crc_sdlc() {
	_crc = checksum = 0;
	}

#ifdef NEVER
/*		The conventional bit-shifting method. Note that two 0's should be
		shifted into the CRC before it is valid with this routine.
unsigned
compute_crc_sdlc( n, buf )
	int		n;
	char	*buf; 
	{
	register unsigned char *s;
	register unsigned val = 0;			/* Start CRC at 0		*/
	register unsigned shifter, flag;

	if ( use_crc ) {
		for ( s = buf; n; --n, ++s )
			for ( shifter = 0x80; shifter; shifter >>= 1 ) {
				flag = val & 0x8000; 
				val <<= 1;
				val |= ((shifter & *s) ? 1 : 0);
				if ( flag ) val ^= 0x1021;
				}
		return( _crc = val );
		}
	for ( s = buf; n; --n )	val += *s++;
	return( checksum = val );
	}

unsigned
next_crc_sdlc( ch )
	unsigned char ch;
	{
	register unsigned shifter, flag;

	if ( use_crc ) {
		for( shifter = 0x80; shifter; shifter >>= 1 ) {
			flag = _crc & 0x8000; 
			_crc <<= 1;
			_crc |= ((shifter & ch) ? 1 : 0);
			if ( flag ) _crc ^= 0x1021;
			}
		}
	else	checksum += ch;
	return( ch );
	}
*/
#endif
