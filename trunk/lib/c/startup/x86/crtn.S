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




	/* Make a placeholder .note segment */
	.section .note,"a"
	.long	4					/* Elf32_Nhdr.n_namesz = sizeof QNX_NOTE_NAME */
	.long	8					/* Elf32_Nhdr.n_descsz = sizeof Elf32_Word * 2 */
	.long	3					/* Elf32_Nhdr.n_type = QNT_STACK */
	.byte	'Q', 'N', 'X', 0			/* QNX_NOTE_NAME */
	.long	524288					/* stack size of 512k */
	.long 	4096					/* prealloc of 1 page */

	.section .init
	ret	$0x0

	.section .fini
	ret	$0x0
