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
 *  qnx4dev.h     "Public" Device Administrator Definitions for QNX 4 
 *                compatibility
 *

 */
#ifndef __DEV_H_INCLUDED
#ifndef __QNX4DEV_H_INCLUDED

/* Dev_info flags bits
 */
#define _DEV_IS_READERS         0x0001
#define _DEV_IS_WRITERS         0x0002
#define _DEV_WILL_WINCH         0x0004

/*
 * Events recognized by dev_arm() and dev_state()
 */
#define _DEV_EVENT_INPUT        0x0001
#define _DEV_EVENT_DRAIN        0x0002
#define _DEV_EVENT_LOGIN        0x0004
#define _DEV_EVENT_EXRDY        0x0008
#define _DEV_EVENT_OUTPUT       0x0010
#define _DEV_EVENT_TXRDY        0x0020
#define _DEV_EVENT_RXRDY        0x0040
#define _DEV_EVENT_HANGUP       0x0080
#define _DEV_EVENT_INTR         0x0100
#define _DEV_EVENT_WINCH        0x0200

/*
 * Special "proxy" value to disarm pending armed proxies
 **/
#define _DEV_DISARM                     (-1)

/*
 * Modes recognized by dev_mode()
 */
#define _DEV_ECHO       0x0001
#define _DEV_EDIT       0x0002
#define _DEV_ISIG       0x0004
#define _DEV_OPOST      0x0008
#define _DEV_OSFLOW 0x0010
#define _DEV_MODES      (_DEV_ECHO|_DEV_EDIT|_DEV_ISIG|_DEV_OPOST|_DEV_OSFLOW)

extern unsigned dev_mode( int __fd, unsigned __mask, unsigned __mode );

#define __QNX4DEV_H_INCLUDED
#endif
#endif
