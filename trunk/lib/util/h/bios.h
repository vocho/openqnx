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
 *	BIOS specific manifests and structures
 *	(See page 5-30 of the IBM AT Technical Reference)
 */
#ifndef _BIOS_H_INCLUDED

#define _BIOS_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
int  bios_get_crt_mode(void);
void bios_set_crt_mode(int mode);
int  bios_get_crt_cols(void);
void bios_set_crt_cols(int cols);
void bios_get_data_area(struct _rom_bios_data_area *buf);
void bios_get_video_data(struct _video_data_area *buf);

#define _MONO_SEL	0x28
#define _CGA_SEL	0x30
#define _BIOS_SEL	0x40

/*
 *	Information offsets into the ROM BIOS data area.
 */
#define _RS232_BASE			0x00
#define _PRINTER_BASE		0x08
#define _EQUIP_FLAG			0x10
#define _MFG_TEST			0x12
#define _MEMORY_SIZE		0x13
#define _MFG_ERR_FLAG		0x15

#define _CRT_MODE			0x49
#define _CRT_COLS			0x4a
#define _CRT_LEN			0x4c
#define _CRT_START			0x4e
#define _CURSOR_POSN		0x50		/*	Cursor for each of up to 8 pages */
#define _CURSOR_MODE		0x60
#define _ACTIVE_PAGE		0x62
#define _ADDR_6845			0x63
#define _CRT_MODE_SET		0x65
#define _CRT_PALETTE		0x66

/*
 *	ROM BIOS data area
 */
struct _rom_bios_data_area {
	unsigned short rs232_base[4];
	unsigned short printer_base[4];
	unsigned short equip_flag;
	unsigned char  mfg_test;
	unsigned short memory_size;
	unsigned char mfg_err_flag;
	unsigned char mfg_err_flag2;
};

/*
 *	Video display data area
 */
struct _video_data_area {
	unsigned char  crt_mode;
	unsigned short crt_cols,
				crt_len,
				crt_start,
				cursor_posn[8],
				cursor_mode;
	unsigned char  active_page;
	unsigned short addr_6845;
	unsigned char  crt_mode_set;
	unsigned char  crt_palette;
};

#ifdef __cplusplus
};
#endif
#endif
