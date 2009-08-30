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






/*				10-Jul-89  7:17:48pm										*/

/*--------------------------------------------------------------------------*/
/*  History: crc_16.c, V0.0, ??-Nov-86 11:36:31am, Dan Dodge,	Baseline	*/
/*  				V0.1, 13-Jan-87 11:36:31am, Dan Hildebrand				*/
/*----------------------------------------------------------------------------

     CRC routines  for  the  QCP  protocol.    These routines come from an
     article by W. David Schwaderer in  the  April  '85  issue  of  PCTech
     Journal and were turned into hand optimized assembly.  These routines
     compute the CRC-16 polynomial.

----------------------------------------------------------------------------*/
#include <stdio.h>
#define	EXT	extern
#include "crc.h"

static unsigned short _crc;					/* CRC accumulator			*/
static unsigned char	checksum;				/* Checksum accumulator		*/

static unsigned short
crctable[256] = {
	0X0000, 0XC0C1, 0XC181, 0X0140, 0XC301, 0X03C0, 0X0280, 0XC241,
	0XC601, 0X06C0, 0X0780, 0XC741, 0X0500, 0XC5C1, 0XC481, 0X0440,
	0XCC01, 0X0CC0, 0X0D80, 0XCD41, 0X0F00, 0XCFC1, 0XCE81, 0X0E40,
	0X0A00, 0XCAC1, 0XCB81, 0X0B40, 0XC901, 0X09C0, 0X0880, 0XC841,
	0XD801, 0X18C0, 0X1980, 0XD941, 0X1B00, 0XDBC1, 0XDA81, 0X1A40,
	0X1E00, 0XDEC1, 0XDF81, 0X1F40, 0XDD01, 0X1DC0, 0X1C80, 0XDC41,
	0X1400, 0XD4C1, 0XD581, 0X1540, 0XD701, 0X17C0, 0X1680, 0XD641,
	0XD201, 0X12C0, 0X1380, 0XD341, 0X1100, 0XD1C1, 0XD081, 0X1040,
	0XF001, 0X30C0, 0X3180, 0XF141, 0X3300, 0XF3C1, 0XF281, 0X3240,
	0X3600, 0XF6C1, 0XF781, 0X3740, 0XF501, 0X35C0, 0X3480, 0XF441,
	0X3C00, 0XFCC1, 0XFD81, 0X3D40, 0XFF01, 0X3FC0, 0X3E80, 0XFE41,
	0XFA01, 0X3AC0, 0X3B80, 0XFB41, 0X3900, 0XF9C1, 0XF881, 0X3840,
	0X2800, 0XE8C1, 0XE981, 0X2940, 0XEB01, 0X2BC0, 0X2A80, 0XEA41,
	0XEE01, 0X2EC0, 0X2F80, 0XEF41, 0X2D00, 0XEDC1, 0XEC81, 0X2C40,
	0XE401, 0X24C0, 0X2580, 0XE541, 0X2700, 0XE7C1, 0XE681, 0X2640,
	0X2200, 0XE2C1, 0XE381, 0X2340, 0XE101, 0X21C0, 0X2080, 0XE041,
	0XA001, 0X60C0, 0X6180, 0XA141, 0X6300, 0XA3C1, 0XA281, 0X6240,
	0X6600, 0XA6C1, 0XA781, 0X6740, 0XA501, 0X65C0, 0X6480, 0XA441,
	0X6C00, 0XACC1, 0XAD81, 0X6D40, 0XAF01, 0X6FC0, 0X6E80, 0XAE41,
	0XAA01, 0X6AC0, 0X6B80, 0XAB41, 0X6900, 0XA9C1, 0XA881, 0X6840,
	0X7800, 0XB8C1, 0XB981, 0X7940, 0XBB01, 0X7BC0, 0X7A80, 0XBA41,
	0XBE01, 0X7EC0, 0X7F80, 0XBF41, 0X7D00, 0XBDC1, 0XBC81, 0X7C40,
	0XB401, 0X74C0, 0X7580, 0XB541, 0X7700, 0XB7C1, 0XB681, 0X7640,
	0X7200, 0XB2C1, 0XB381, 0X7340, 0XB101, 0X71C0, 0X7080, 0XB041,
	0X5000, 0X90C1, 0X9181, 0X5140, 0X9301, 0X53C0, 0X5280, 0X9241,
	0X9601, 0X56C0, 0X5780, 0X9741, 0X5500, 0X95C1, 0X9481, 0X5440,
	0X9C01, 0X5CC0, 0X5D80, 0X9D41, 0X5F00, 0X9FC1, 0X9E81, 0X5E40,
	0X5A00, 0X9AC1, 0X9B81, 0X5B40, 0X9901, 0X59C0, 0X5880, 0X9841,
	0X8801, 0X48C0, 0X4980, 0X8941, 0X4B00, 0X8BC1, 0X8A81, 0X4A40,
	0X4E00, 0X8EC1, 0X8F81, 0X4F40, 0X8D01, 0X4DC0, 0X4C80, 0X8C41,
	0X4400, 0X84C1, 0X8581, 0X4540, 0X8701, 0X47C0, 0X4680, 0X8641,
	0X8201, 0X42C0, 0X4380, 0X8341, 0X4100, 0X81C1, 0X8081, 0X4040
	};

/* never use it for now; we weren't using it if __WATCOMC__ was defined
   either */
#undef USE_ASM


#ifndef USE_ASM
/*--------------- C version of CRC calculation --------------------*/
unsigned short
compute_crc_16( n, buf )
	short int		n;
	signed char	*buf; 
	{
	register unsigned char *s;
	register unsigned short val = 0;

	if ( use_crc ) {
		for ( s = buf; n; --n )
			val = crctable[ ( *s++ ^ val ) & 0xff ] ^ ( val >> 8 );
		return( _crc = val );
		}
	for ( s = buf; n; --n )	val += *s++;
	return( checksum = val );
	}
/*---------------------------------------------------------*/
#else

#asm {
		seg 1
		export	<compute_crc_16>
;	int		n;		 /* 14[bp] */
;	char	*buf;    /* 16[bp] */ 

<compute_crc_16>:
		call prologue

		xor	ax,ax			; Start with 0
		mov <_crc>,ax
		mov <checksum>,ax
		mov cx,14[bp]		; If no bytes to check, all done
		cmp cx, #0
		jz	epilogue
		cld

	b	test <use_crc>,#0ffH
		jz chk0

;-- Compute Crc
crc0:		mov si,16[bp]	;Point to first byte of string
			xor dx,dx

			export crc16	;Export for alignment checking

;			nop				;Used to double word align the next label

crc16:		lodsb			; foo = *s++
	 		xor ax,dx		;crctable[ (foo ^ val ) & 0xff ]
			mov	bl,al
			xor bh,bh
			shl bx,1
			mov al,dh		; val >> 8
			xor ah,ah
			xor	ax,<crctable>[bx]	; get table entry
			mov dx,ax

			dec cx
			jz crc2
			lodsb
	 		xor ax,dx		;crctable[ (foo ^ val ) & 0xff ]
			mov	bl,al
			xor bh,bh
			shl bx,1
			mov al,dh		; val >> 8
			xor ah,ah
			xor	ax,<crctable>[bx]	; get table entry
			mov dx,ax

			dec cx
			jz crc2
			lodsb
	 		xor ax,dx		;crctable[ (foo ^ val ) & 0xff ]
			mov	bl,al
			xor bh,bh
			shl bx,1
			mov al,dh		; val >> 8
			xor ah,ah
			xor	ax,<crctable>[bx]	; get table entry
			mov dx,ax
			loop crc16

crc2:	    mov <_crc>,dx
			jmp epilogue

;-- Compute Checksum
chk0:		xor	ax,ax

chk1:	b	add ax,[bx]
			inc bx
			loop chk1
			mov <checksum>,ax
done:		jmp	epilogue

#asm		}

#endif

void
clear_crc_16() {
	_crc = 0;
	checksum = 0;
	}

unsigned short
next_crc_16( ch )					/* Compute the character into the crc	*/
	unsigned short	ch;
	{
	_crc = crctable[ ( ch ^ _crc ) & 0xff ] ^ ( _crc >> 8 );

	checksum += (char) ch;
	return( ch );
	}

unsigned short
get_crc_16() {
	return( use_crc ? _crc : checksum );
	}
