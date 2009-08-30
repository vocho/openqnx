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
 * Format of a lesskey file:
 *
 *	LESSKEY_MAGIC (4 bytes)
 *	 sections...
 *	END_LESSKEY_MAGIC (4 bytes)
 *
 * Each section is:
 *
 *	section_MAGIC (1 byte)
 *	section_length (2 bytes)
 *	key table (section_length bytes)
 */
#define	C0_LESSKEY_MAGIC	'\0'
#define	C1_LESSKEY_MAGIC	'M'
#define	C2_LESSKEY_MAGIC	'+'
#define	C3_LESSKEY_MAGIC	'G'

#define	CMD_SECTION		'c'
#define	EDIT_SECTION		'e'
#define	VAR_SECTION		'v'
#define	END_SECTION		'x'

#define	C0_END_LESSKEY_MAGIC	'E'
#define	C1_END_LESSKEY_MAGIC	'n'
#define	C2_END_LESSKEY_MAGIC	'd'

/* */
#define	KRADIX		64
