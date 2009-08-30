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





/*				12-Dec-89  5:13:05pm										*/

/*--------------------------------------------------------------------------*/
/*  History: qcp.h, V0.0, ??-Nov-86 11:36:31am, Dan Dodge,	Baseline		*/
/*  				V0.1, 13-Jan-87 11:36:31am, Dan Hildebrand				*/
/*----------------------------------------------------------------------------

	Symbols defined for QCP, Quantum Communication Protocol.

----------------------------------------------------------------------------*/
#define NPIECES		8
#define MAXSIZE		2048

#pragma pack(1)

struct soh_header {
				uchar syn;
				uchar soh;
				uchar seq;
				uchar cseq;
				unsigned short len;
				unsigned short clen;
				};

struct piece_entry {
				uchar rcv_ok;
				struct soh_header data_header;
				char data[MAXSIZE+2];
				};

EXT struct soh_header	header;
EXT struct piece_entry	segment_table[NPIECES];

/*-- Manifests from QNX 2.15 ----*/

#define	NCPFN	16

/*
 * This structure overlays the CHAR DATA field of a piece entry for the
 * very first piece received.
 */

struct first_piece {
			uchar flags;						/* Flags register			*/
			uchar gid_ext;						/* Upper 8 bits of group id	*/
			uchar uid_ext;						/*	and user id for QNX4.0	*/
			char spare[13];
			struct old_dir_entry dir_info;
			char fname_extension[1];			/* Variable length field	*/
			};

/*-- Bit definitions in first_piece.flags -----------*/
#define	SENDING_NEWER		0x01	/* Send side wants to send only if newer*/
#define IS_4				0x02	/* Indicate QNX 4.0 system sending		*/

EXT uchar seg_num;			/* 0 to 1f									*/
EXT uchar piece_num;		/* 0 to 7									*/
EXT uchar seq;				/* (seg_num << 3) + piece_num				*/
EXT unsigned long rbytes;			/* Received bytes							*/

#include "prototypes"
