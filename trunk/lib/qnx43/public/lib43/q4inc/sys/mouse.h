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
 *  mouse.h     Mouse operations
 *

 */
#ifndef __MOUSE_H_INCLUDED

#ifndef __TYPES_H_INCLUDED
 #include <sys/types.h>
#endif

#define __TOUCH_TRANS_FLNM  "/etc/config/touch/calibdata.%ld"
#define __TABLET_TRANS_FLNM "/etc/config/tablet/calibdata.%ld"

#if __WATCOMC__ > 1000
#pragma pack(push,1);
#else
#pragma pack(1);
#endif

/*
 * Mouse buttons
 */
#define _MOUSE_LEFT     0x0004L
#define _MOUSE_MIDDLE   0x0002L
#define _MOUSE_RIGHT    0x0001L
#define _MOUSE_BUT4     0x0008L
#define _MOUSE_BUT5     0x0010L
#define _MOUSE_BUT6     0x0020L
#define _MOUSE_BUT7     0x0040L
#define _MOUSE_BUT8     0x0080L


#define _MOUSE_ABS_MIN      0
#define _MOUSE_ABS_MAX  16383

#define _MSEFLG_REL_ARB         0x0000      /* Mouse */
#define _MSEFLG_ABSOLUTE        0x0008      /* Direct coordinates */
#define _MSEFLG_ABS_SCREEN      0x0009      /* Touch screen */
#define _MSEFLG_ABS_WORLD       0x000A      /* Digitizing tablet */
#define _MSEFLG_TYPE_MASK       0x000F      /* Type mask */
#define _MSEFLG_UNCALIBRATED    0x0010      /* Raw coordinates */
#define _MSEFLG_Z_DATA          0x0020      /* Some touch screen, pressure */

/*  mouse_event.mode, _mouse_ctrl.mode and _mouse_param.mode explanation
0x0001 RO   Screen related
0x0002 RO   World related
0x0008 RO   Absolute Coordinates (not Relative)
0x0010 RW   Uncalibrated data (for Touch Screen and Digitizing Tablets)
0x0020 RO   Z Data available
 _ _ _ _._ _ _ _._ _ _ _._ _ _ _
                    | | | | | |screen related
                    | | | | |world related
                    | | | |undef.
                    | | |0-relative 1-absolute
                    | |Uncalibrated data
                    |Z available
*/

struct mouse_event {
    short       dx;
    short       dy;
    long        buttons;
    long        timestamp;
    short       dz;
    unsigned short mode;
    } ;

struct _mouse_ctrl {
    short                   zeros;
    unsigned short          handle;
    unsigned short          mode;
    long                    buttons;
    short                   fd;
    unsigned short          zero[2];
    } ;

struct _mouse_param {
    char                    name[32];
    unsigned short          mode;
    long                    buttons;
    short                   threshold;
    short                   gain;
    unsigned short          zero[11];
    } ;


#ifdef __cplusplus
extern "C" {
#endif
extern struct  _mouse_ctrl *mouse_open( nid_t __nid, const char *__name,
                                        int __fd );
extern int     mouse_close( struct _mouse_ctrl *__mc );
extern int     mouse_read( struct _mouse_ctrl *__mc, struct mouse_event *__buf, int __nevents,
            pid_t __proxy, int *__triggered );
/*
extern int     mouse_read( struct _mouse_ctrl *__mc, unsigned char *__buf, int __nevents,
            pid_t __proxy, int *__triggered );
*/
extern int     mouse_flush( struct _mouse_ctrl *__mc );
extern int     mouse_param( struct _mouse_ctrl *__mc, int mode,
            struct _mouse_param *param );
#ifdef __cplusplus
};
#endif

#if __WATCOMC__ > 1000
#pragma pack(pop);
#else
#pragma pack();
#endif

#define __MOUSE_H_INCLUDED
#endif
