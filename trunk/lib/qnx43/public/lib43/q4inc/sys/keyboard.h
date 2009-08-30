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
 *  keyboard.h
 *

 */

#ifndef _SYS_KEYBOARD_H_INCLUDED
#define _SYS_KEYBOARD_H_INCLUDED

#ifndef _SYS_INTERACT_H_INCLUDED
 #include <sys/interact.h>
#endif

#ifndef _IOCTL_H_INCLUDED
 #include <sys/ioctl.h>
#endif

#ifndef _TIME_H_INCLUDED
 #include <time.h>
#endif

#ifndef __KEYCODES_H_INCLUDED
 #include <sys/keycodes.h>
#endif

#pragma pack(1);

struct _keyboard_data {
    unsigned long           modifiers;
    unsigned long           flags;
    unsigned long           key_cap;
    unsigned long           key_sym;
    unsigned long           key_scan;
};

struct _keyboard_packet {                   /* start of struct returned from read() */
    struct timespec         time;
    struct _keyboard_data   data;
};

#define _KEYBOARD_MODE_SCAN      0x0000     /* single byte scancodes are returned */
#define _KEYBOARD_MODE_PACKET    0x0001     /* _keyboard_packets are returned */
#define _KEYBOARD_MODE_MASK      0x0003

struct _keyboard_ctrl {
    struct _interact_device type;           /* Device type */
    unsigned long           flags;          /* Device type flags (read-only) */
    unsigned long           mode;           /* Mode of currently read packets */
	unsigned long			zero[10];
};
                                
#define _KEYBOARDGETCTRL        _IOR('I', 10, struct _keyboard_ctrl)
#define _KEYBOARDSETCTRL        _IOW('I', 11, struct _keyboard_ctrl)
#define _KEYBOARDGETKEYMAPPATH  _IOR('I', 12, char[PATH_MAX])
#define _KEYBOARDSETKEYMAPPATH  _IOW('I', 13, char[PATH_MAX])

#pragma pack();

#endif
