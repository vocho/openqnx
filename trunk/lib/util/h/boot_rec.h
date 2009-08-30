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
 *  boot_rec.h    boot/loader record
 *

 */
#ifndef _BOOT_REC_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
struct boot_rec_data
	{
	char			code[512-16];
	char			signature1[2];
	char			drive_type_flag;	/*	'F' or 'H' for floppy or hard disk	*/
	char			sctr_base;
	long			partition_offset;
	short unsigned	num_heads,
					sect_per_track,
					num_cyls;
	char			signature2[2];
	};

#ifdef __cplusplus
};
#endif
#define _BOOT_REC_H_INCLUDED
#endif
