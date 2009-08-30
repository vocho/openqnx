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
 *  share.h     Define file sharing modes for sopen()
 *

 */

#define SH_COMPAT   0x00    /* compatibility mode   */
#define SH_DENYRW   0x10    /* deny read/write mode */
#define SH_DENYWR   0x20    /* deny write mode      */
#define SH_DENYRD   0x30    /* deny read mode       */
#define SH_DENYNO   0x40    /* deny none mode       */

#define SH_MASK     0x70    /* mask for standard share modes    */

#define SH_DOS      0x01    /* DOS-like interpretation of open, locks, etc. */

/* __SRCVERSION("share.h $Rev: 153052 $"); */
