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
 *  sh/7751pci.h
 *
 *	Definition for 7751(r) on-chip PCI controller
 *

 */
 
#define SH4_HAS_PCI ( (__cpu_flags & SH_CPU_FLAG_HAS_PCI) != 0 )

/* MMR for on-chip pci controller */
#define SH4_MMR_PCIC_PCIMBR			0xfe2001c4

/* address space */
#define SH4_7751PCI_PCIMEM_BASE		0xfd000000
#define SH4_7751PCI_PCIMEM_WINSIZE	(16 * 1024 * 1024)
#define SH4_7751PCI_ADDR_BASE		0xd0000000
#define SH4_7751PCI_ADDR_BOUND		0xfd000000

/* PCIMBR */
#define SH4_PCIMBR_MASK(x)			((x) & 0xff000000)


/* __SRCVERSION("7751pci.h $Rev: 159814 $"); */
