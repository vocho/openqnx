/*				 2-Oct-89  7:14:06am										*/
/*--------------------------------------------------------------------------*/
/*  History: crc_zmodem.c, V0, 13-Jan-87, Dan Hildebrand,		Baseline	*/
/*----------------------------------------------------------------------------

     CRC routines for Xmodem. Xmodem uses athe SDLC CRC polynomial and the
     routine to compute it here comes from an article by Curtis Wright  in
     the Summer, 1987 issue of "The C Journal".

     Additional contributions by Rick Duff and George Eberhardt

----------------------------------------------------------------------------*/

/*
89.05.09 RB Andrews

compute_crc_z:
	improved preformance of CRC calc by about 27/28 percent - it now works
	at ~365230 cps (vrs 285710) on Compaq 20 MHz '386 (test 500 byte buffer)

	improved checksum calc by 54 percent - it now works at 769,230 cps (same
	machine)

	performed minor cleanup - primarily assignments/additions between byte and
	word operands/variables:
		b add ax,[bx]			; i'm surprised computer didn't crash on this
		  mov <checksum>,ax		; over-wrote other data

	note on code alignment:
		now that chk0 loop begins with a single opcode instruction, alignment
		has no measurable affect for this loop

		loop1 starts with a 2 byte opcode - the only bad case is where this
		sequence spans a 32 bit block boundary (ie., there are 4 cases using
		0 thru 3 nops before this loop -- only one slows the calc, in this case
		it amounts to 10 percent IF no expansion of inline code

		An empty loop instruction took ~10 uSEC -- using additional inline code
		increased CRC calc by 20/21 percent. Replacing "xor ax,[bx][di]" with
		"xor ax, <crctable>[bx]" yielded an additional 7/8 percent.
*/

#include <stdio.h>
#define	EXT	extern
#include "crc.h"

static unsigned			_crc;					/* CRC accumulator			*/
static unsigned char	checksum;				/* Checksum accumulator		*/

/* Something to force next table to be word aligned	*/
/*
*/
static char		unused;

/*
 * updcrc macro derived from article Copyright (C) 1986 Stephen Satchell. 
 *  NOTE: First srgument must be in range 0 to 255.
 *        Second argument is referenced twice.
 * 
 * Programmers may incorporate any or all code into their programs, 
 * giving proper credit within the source. Publication of the 
 * source routines is permitted so long as proper credit is given 
 * to Stephen Satchell, Satchell Evaluations and Chuck Forsberg, 
 * Omen Technology.
 */

#define updcrc(cp, crc) ( crctable[((crc >> 8) & 255)] ^ (crc << 8) ^ cp)


/* crctab calculated by Mark G. Mendel, Network Systems Corporation */
static unsigned crctable[256] = {
    0x0000,  0x1021,  0x2042,  0x3063,  0x4084,  0x50a5,  0x60c6,  0x70e7,
    0x8108,  0x9129,  0xa14a,  0xb16b,  0xc18c,  0xd1ad,  0xe1ce,  0xf1ef,
    0x1231,  0x0210,  0x3273,  0x2252,  0x52b5,  0x4294,  0x72f7,  0x62d6,
    0x9339,  0x8318,  0xb37b,  0xa35a,  0xd3bd,  0xc39c,  0xf3ff,  0xe3de,
    0x2462,  0x3443,  0x0420,  0x1401,  0x64e6,  0x74c7,  0x44a4,  0x5485,
    0xa56a,  0xb54b,  0x8528,  0x9509,  0xe5ee,  0xf5cf,  0xc5ac,  0xd58d,
    0x3653,  0x2672,  0x1611,  0x0630,  0x76d7,  0x66f6,  0x5695,  0x46b4,
    0xb75b,  0xa77a,  0x9719,  0x8738,  0xf7df,  0xe7fe,  0xd79d,  0xc7bc,
    0x48c4,  0x58e5,  0x6886,  0x78a7,  0x0840,  0x1861,  0x2802,  0x3823,
    0xc9cc,  0xd9ed,  0xe98e,  0xf9af,  0x8948,  0x9969,  0xa90a,  0xb92b,
    0x5af5,  0x4ad4,  0x7ab7,  0x6a96,  0x1a71,  0x0a50,  0x3a33,  0x2a12,
    0xdbfd,  0xcbdc,  0xfbbf,  0xeb9e,  0x9b79,  0x8b58,  0xbb3b,  0xab1a,
    0x6ca6,  0x7c87,  0x4ce4,  0x5cc5,  0x2c22,  0x3c03,  0x0c60,  0x1c41,
    0xedae,  0xfd8f,  0xcdec,  0xddcd,  0xad2a,  0xbd0b,  0x8d68,  0x9d49,
    0x7e97,  0x6eb6,  0x5ed5,  0x4ef4,  0x3e13,  0x2e32,  0x1e51,  0x0e70,
    0xff9f,  0xefbe,  0xdfdd,  0xcffc,  0xbf1b,  0xaf3a,  0x9f59,  0x8f78,
    0x9188,  0x81a9,  0xb1ca,  0xa1eb,  0xd10c,  0xc12d,  0xf14e,  0xe16f,
    0x1080,  0x00a1,  0x30c2,  0x20e3,  0x5004,  0x4025,  0x7046,  0x6067,
    0x83b9,  0x9398,  0xa3fb,  0xb3da,  0xc33d,  0xd31c,  0xe37f,  0xf35e,
    0x02b1,  0x1290,  0x22f3,  0x32d2,  0x4235,  0x5214,  0x6277,  0x7256,
    0xb5ea,  0xa5cb,  0x95a8,  0x8589,  0xf56e,  0xe54f,  0xd52c,  0xc50d,
    0x34e2,  0x24c3,  0x14a0,  0x0481,  0x7466,  0x6447,  0x5424,  0x4405,
    0xa7db,  0xb7fa,  0x8799,  0x97b8,  0xe75f,  0xf77e,  0xc71d,  0xd73c,
    0x26d3,  0x36f2,  0x0691,  0x16b0,  0x6657,  0x7676,  0x4615,  0x5634,
    0xd94c,  0xc96d,  0xf90e,  0xe92f,  0x99c8,  0x89e9,  0xb98a,  0xa9ab,
    0x5844,  0x4865,  0x7806,  0x6827,  0x18c0,  0x08e1,  0x3882,  0x28a3,
    0xcb7d,  0xdb5c,  0xeb3f,  0xfb1e,  0x8bf9,  0x9bd8,  0xabbb,  0xbb9a,
    0x4a75,  0x5a54,  0x6a37,  0x7a16,  0x0af1,  0x1ad0,  0x2ab3,  0x3a92,
    0xfd2e,  0xed0f,  0xdd6c,  0xcd4d,  0xbdaa,  0xad8b,  0x9de8,  0x8dc9,
    0x7c26,  0x6c07,  0x5c64,  0x4c45,  0x3ca2,  0x2c83,  0x1ce0,  0x0cc1,
    0xef1f,  0xff3e,  0xcf5d,  0xdf7c,  0xaf9b,  0xbfba,  0x8fd9,  0x9ff8,
    0x6e17,  0x7e36,  0x4e55,  0x5e74,  0x2e93,  0x3eb2,  0x0ed1,  0x1ef0
	};

unsigned
next_crc_z( ch )					/* Compute the character into the crc	*/
	unsigned char ch;
	{
	_crc = crctable[ (_crc >> 8) & 255 ] ^ (_crc << 8) ^ ch;
	checksum += ch;
	return( ch );
	}

#ifdef __QNX__
/*--------------- C version of CRC calculation --------------------*/
unsigned
compute_crc_z( n, buf )
	int		n;
	char	*buf; 
	{
	register unsigned char *s;
	unsigned val = 0;

	if ( use_crc ) {
		for ( s = buf; n; --n )
			val = updcrc( *s++, val );
		_crc = val;
		next_crc_z( 0 );
		next_crc_z( 0 );
		return( _crc );
		}
	for ( s = buf; n; --n )	val += *s++;
	return( checksum = val );
	}
/*---------------------------------------------------------*/
#else
#asm {
	seg		1
	export	<compute_crc_z>
;	int		n;		 /* 14[bp] */
;	char	*buf;    /* 16[bp] */ 

<compute_crc_z>:
		call prologue

		xor	ax,ax			; Start with 0 (use for both CRC and checksum)
		mov cx,14[bp]		; If no bytes to check, all done
		cmp cx, #0
		jz	epilogue
		mov	si,16[bp]   	;address of input data
		cld

	b	test	<use_crc>,#0ffH
		jz chk0

loop1:						; Start of CRC loop

	b	mov bl,ah         	; crc >> 8
	b	xor	bh,bh			;zero it
		shl	bx,1			;for an index
		mov	ah,al			;other half
		lodsb
		xor	ax,<crctable>[bx]	; get table entry

		dec cx
		jz loop2
	b	mov bl,ah         	; crc >> 8
	b	xor	bh,bh			;zero it
		shl	bx,1			;for an index
		mov	ah,al			;other half
		lodsb
		xor	ax,<crctable>[bx]	; get table entry

		dec cx
		jz loop2
	b	mov bl,ah         	; crc >> 8
	b	xor	bh,bh			;zero it
		shl	bx,1			;for an index
		mov	ah,al			;other half
		lodsb
		xor	ax,<crctable>[bx]	; get table entry

		dec cx
		jz loop2
	b	mov bl,ah         	; crc >> 8
	b	xor	bh,bh			;zero it
		shl	bx,1			;for an index
		mov	ah,al			;other half
		lodsb
		xor	ax,<crctable>[bx]	; get table entry

		dec cx
		jz loop2
	b	mov bl,ah         	; crc >> 8
	b	xor	bh,bh			;zero it
		shl	bx,1			;for an index
		mov	ah,al			;other half
		lodsb
		xor	ax,<crctable>[bx]	; get table entry

		dec cx
		jz loop2
	b	mov bl,ah         	; crc >> 8
	b	xor	bh,bh			;zero it
		shl	bx,1			;for an index
		mov	ah,al			;other half
		lodsb
		xor	ax,<crctable>[bx]	; get table entry

		loop loop1

loop2:
		mov <_crc>,ax

		xor	ax,ax			; now do 2 more 0 bytes
		push	ax
		call	<next_crc_z>
		add	sp,#2
		push	ax
		call	<next_crc_z>
		add	sp,#2

		mov	ax,<_crc>
		jmp	epilogue

;-- Compute Checksum
chk0:	lodsb
		add ah,al

		dec cx
		jz chk1
		lodsb
		add ah,al

		dec cx
		jz chk1
		lodsb
		add ah,al

		dec cx
		jz chk1
		lodsb
		add ah,al

		dec cx
		jz chk1
		lodsb
		add ah,al

		dec cx
		jz chk1
		lodsb
		add ah,al

		dec cx
		jz chk1
		lodsb
		add ah,al

		dec cx
		jz chk1
		lodsb
		add ah,al

		dec cx
		jz chk1

		loop chk0

chk1:	mov <checksum>,ah
done:	jmp epilogue
#asm		}
#endif

void
clear_crc_z() {
	_crc = checksum = 0;
	}

unsigned
get_crc_z() {
	return( use_crc ? _crc : checksum );
	}
