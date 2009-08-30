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
#define DIAG
*/

/*				27-Sep-89  8:08:40am										*/

/*--------------------------------------------------------------------------*/
/*  History: main.h, V0.0, ??-Nov-86 11:36:31am, Dan Dodge,	Baseline		*/
/*  				 V0.1, 13-Jan-87 11:36:31am, Dan Hildebrand				*/
/*----------------------------------------------------------------------------

	Symbols defined for Qterm file transfer protocols.

----------------------------------------------------------------------------*/
#ifndef __MAIN_H_INCLUDED
#define __MAIN_H_INCLUDED

#ifndef TRUE
#define	TRUE		1
#define	FALSE		0
#endif

#define ONESEC		10		/* Value for 1 second with modem r/w routines	*/
#define	RETRY_MAX	10		/* Maximum number of retries					*/
#define MAXSIZE		2048	/* Maximum sub packet size						*/
#define	uchar		unsigned char

#define SOH			0x01
#define	STX			0x02
#define ETX			0x03
#define EOT			0x04
#define ENQ			0x05
#define ACK			0x06
#define	BELL		0x07
#define	BS			0x08
#define	TAB			0x09
#define	LF			0x0a
#define	VTAB		0x0b
#define	FF			0x0c
/*	#define	CR			0x0d	*/
#define	SO			0x0e
#define	SI			0x0f
#define	DLE			0x10
#define	DC1			0x11
#define	DC2			0x12
#define	DC3			0x13
#define	DC4			0x14
#define NAK			0x15
#define SYN			0x16
#define	ETB			0x17
#define CAN			0x18
#define EM			0x19
#define SUB			0x1a
#define ESC			0x1b
#define FS			0x1c
#define GS			0x1d
#define RS			0x1e
#define US			0x1f
#define	SP			0x20

#define TEN_YEARS		315532800L 	/* 1990 Jan 1st - 1980 Jan 1st			*/

EXT char *pname;

EXT FILE *logfp;
EXT int mdm;

EXT unsigned long fbytes;			/* Number of bytes in the file					*/
EXT char prefix[100];		/* Path name prefix to force onto files			*/
EXT char logfile[100];		/* Path name for activity log file (if any)		*/
EXT char has_timeouts;		/* True if OS supports timeouts on fget			*/
EXT char use_stdin;			/* True if protocol is sending over stdin		*/
EXT unsigned onesec;		/* Central time constant, usually = 1 sec		*/
EXT unsigned retry_limit;	/* Max number of retries, usually RETRY_MAX		*/
EXT time_t start_time, stop_time;	/* File tx start and stop times			*/
EXT	struct stat statbuf;	/* General purpose STAT entry buffer 			*/

/*---- Command line settable flags -----------*/
EXT char force;				/* Forced filename								*/
EXT char make_dir;			/* Allow rx routine to make sub-directories		*/
EXT char newer;				/* Receive only files newer than existing file	*/
EXT char qterm;				/* Set if Qterm invoked QCP						*/
EXT char relaxed;			/* Allows relaxed timing						*/
EXT char sending;			/* Set if txing or rxing						*/
EXT char today;				/* Set if today's date to be placed on file		*/
EXT char verbose;			/* Set to display error messages				*/
EXT char quiet;				/* Set to supress all output					*/
EXT char unlink_file;		/* Force unlink of unwriteable files			*/
EXT int batch;				/* Batch on/off flag -RM						*/
EXT int sub_packet_size;	/* Sub packet size * 8 = actual packet size		*/
EXT int socket;				/* Are we running on a socket?					*/
EXT char flashfile;			/* Set if running on flashfile system			*/

/*-- Manifests and structs from QNX 2.15 ----*/

#define	NCPFN	16
#define _DIRECTORY		0x20

#pragma pack(1)

struct old_dir_entry {
	char fstat;
	long ffirst_xtnt;
	long flast_xtnt;
	long fnum_blks;
	short unsigned fnum_xtnt;
	unsigned char fowner;
	unsigned char fgroup;
	short unsigned fnum_chars_free; /* No. unused chars in last block */
	long fseconds;
	unsigned char ftype;
	unsigned char fgperms;
	unsigned char fperms;
	unsigned char fattr;
	unsigned short fdate[2];
	char fname[NCPFN + 1];
	};

#include "crc.h"
#include "prototypes"

#endif
