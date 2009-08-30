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
 *  dcmd_misc.h   Non-portable low-level devctl definitions
 *

 */
#ifndef __DCMD_MISC_H_INCLUDED
#define __DCMD_MISC_H_INCLUDED

#ifndef _DEVCTL_H_INCLUDED
 #include <devctl.h>
#endif

#include <_pack64.h>

#define DCMD_MISC_MQGETATTR		__DIOF(_DCMD_MISC, 1, struct mq_attr)
#define DCMD_MISC_MQSETATTR		__DIOT(_DCMD_MISC, 2, struct mq_attr)
#define DCMD_MISC_MQSETCLOSEMSG	__DIOT(_DCMD_MISC, 4, struct { char __data[64];})

#define _INTERACT_TYPE_POINTER       0x0001          /* pointer packet */
#define _INTERACT_TYPE_KEY           0x0002          /* keyboard packet */
#define _INTERACT_TYPE_FEEDBACK      0x0004          /* LED feedback */

struct _interact_device {
    void                    *handle;
    unsigned short          type;
    unsigned short          feedback;
};

#define MAX_DEVS_PER_MGR        30
struct _interact_mgr {
        int                                             id;
        struct _interact_device devices[MAX_DEVS_PER_MGR];
};

#define _INTERACT_FEEDBACK_LED       0x00000001      /* LED displays */
#define _INTERACT_FEEDBACK_BELL      0x00000002      /* Freq, vol, dur */
#define _INTERACT_FEEDBACK_INTEGER   0x00000004      /* Integer display */
#define _INTERACT_FEEDBACK_STRING    0x00000008      /* string display */

#define _INTERACTTYPE           __DIOF(_DCMD_MISC, 20, struct _interact_device)
#define _INTERACTFEEDBACK       __DIOF(_DCMD_MISC, 29, unsigned long)

#include <_packpop.h>

#endif

/* __SRCVERSION("dcmd_misc.h $Rev: 153052 $"); */
