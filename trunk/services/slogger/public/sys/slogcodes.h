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
 *  slogcodes.h     System log codes for the slogger utility
 *

 */

#ifndef __SLOGCODES_H_INCLUDED
#define __SLOGCODES_H_INCLUDED

#define __C _SLOG_SETCODE
#include <sys/slog.h>

/*
** Define trace major codes.
*/
#define _SLOGC_CONSOLE       0

#define _SLOGC_PROC          1
 #define _SLOGC_PROC_TERMINATED __C(_SLOGC_PROC, 0)

#define _SLOG_SYSLOG         2 

#define _SLOGC_FS            3
 #define _SLOGC_FS_QNX4      __C(_SLOGC_FS,   0)
 #define _SLOGC_FS_CD        __C(_SLOGC_FS, 100)
 #define _SLOGC_FS_DOS       __C(_SLOGC_FS, 200)
 #define _SLOGC_FS_EXT2      __C(_SLOGC_FS, 300)
 #define _SLOGC_FS_PKG       __C(_SLOGC_FS, 400)
 #define _SLOGC_FS_NFS2      __C(_SLOGC_FS, 500)
 #define _SLOGC_FS_CIFS      __C(_SLOGC_FS, 600)
 #define _SLOGC_FS_FFS       __C(_SLOGC_FS, 700)
 #define _SLOGC_FS_ETFS      __C(_SLOGC_FS, 800)
 #define _SLOGC_FS_UDF       __C(_SLOGC_FS, 900)
 #define _SLOGC_FS_QNX6      __C(_SLOGC_FS, 1000)

#define _SLOGC_BLOCK         4
 #define _SLOGC_BLOCK_IOBLK  __C(_SLOGC_BLOCK,   0)

#define _SLOGC_CAM           5
 #define _SLOGC_CAM_XPT           __C(_SLOGC_CAM,   0)
 #define _SLOGC_CAM_PDRV_DISK     __C(_SLOGC_CAM, 100)
 #define _SLOGC_CAM_PDRV_CDROM    __C(_SLOGC_CAM, 200)
 #define _SLOGC_CAM_PDRV_OPTICAL  __C(_SLOGC_CAM, 300)
 #define _SLOGC_CAM_PDRV_SA       __C(_SLOGC_CAM, 400)

#define _SLOGC_CHAR          6
 #define _SLOGC_CHAR_CON     __C(_SLOGC_CHAR,   0)
 #define _SLOGC_CHAR_SER     __C(_SLOGC_CHAR, 100)
 #define _SLOGC_CHAR_PAR     __C(_SLOGC_CHAR, 200)
 #define _SLOGC_CHAR_PTY     __C(_SLOGC_CHAR, 300)

#define _SLOGC_AUDIO         7

#define _SLOGC_GRAPHICS      8
 #define _SLOGC_GRAPHICS_GL  __C(_SLOGC_GRAPHICS, 100)
 #define _SLOGC_GRAPHICS_DISPLAY __C(_SLOGC_GRAPHICS, 200)

#define _SLOGC_INPUT         9

#define _SLOGC_NETWORK       10
 #define _SLOGC_NETWORK_MOST __C(_SLOGC_NETWORK, 0)
 #define _SLOGC_NETWORK_NEXTFREE __C(_SLOGC_NETWORK, 100)

#define _SLOGC_PRINTER               11
 #define _SLOGC_PRINTER_PAPEROUT     __C(_SLOGC_PRINTER, 0)
 #define _SLOGC_PRINTER_SPOOLER      __C(_SLOGC_PRINTER, 100)
 #define _SLOGC_PRINTER_FIFO_ENOMEM  __C(_SLOGC_PRINTER,0x20)
 #define _SLOGC_PRINTER_JOB_INFO     __C(_SLOGC_PRINTER,0x40)
 #define _SLOGC_PRINTER_DEBUG        __C(_SLOGC_PRINTER,0x41)

#define _SLOGC_USB           12
 #define _SLOGC_USB_GEN      __C(_SLOGC_USB,   0)
 #define _SLOGC_USB_UHCI     __C(_SLOGC_USB,   100)
 #define _SLOGC_USB_OHCI     __C(_SLOGC_USB,   200)

#define _SLOGC_PPP           13

#define _SLOGC_TCPIP         14
 #define _SLOGC_TCPIP_NETMANAGER __C(_SLOGC_TCPIP, 2000)

#define _SLOGC_QNET          15

#define _SLOGC_PHOTON        16

#define	_SLOGC_PCI			 17

#define	_SLOGC_DEVP			 18

#define	_SLOGC_SIM			 19
 #define _SLOGC_SIM_EIDE     __C(_SLOGC_SIM,   0)
 #define _SLOGC_SIM_AHA2     __C(_SLOGC_SIM, 100)
 #define _SLOGC_SIM_AHA4     __C(_SLOGC_SIM, 200)
 #define _SLOGC_SIM_AHA7     __C(_SLOGC_SIM, 300)
 #define _SLOGC_SIM_AHA8     __C(_SLOGC_SIM, 400)
 #define _SLOGC_SIM_NCR8     __C(_SLOGC_SIM, 500)
 #define _SLOGC_SIM_AMD      __C(_SLOGC_SIM, 600)
 #define _SLOGC_SIM_BTMM     __C(_SLOGC_SIM, 700)
 #define _SLOGC_SIM_FDC      __C(_SLOGC_SIM, 800)
 #define _SLOGC_SIM_UMASS    __C(_SLOGC_SIM, 900)
 #define _SLOGC_SIM_ADPU320  __C(_SLOGC_SIM, 1000)
 #define _SLOGC_SIM_M6       __C(_SLOGC_SIM, 1100)
 #define _SLOGC_SIM_M8       __C(_SLOGC_SIM, 1200)
 #define _SLOGC_SIM_LNK      __C(_SLOGC_SIM, 1300)
 #define _SLOGC_SIM_RAM      __C(_SLOGC_SIM, 1400)
 #define _SLOGC_SIM_AHCI     __C(_SLOGC_SIM, 1500)
 #define _SLOGC_SIM_MVSATA   __C(_SLOGC_SIM, 1600)
 #define _SLOGC_SIM_SERCD    __C(_SLOGC_SIM, 1700)

#define _SLOGC_MEDIA			20

#define _SLOGC_DUMPER			21

#define	_SLOGC_PMM				22

#define	_SLOGC_MCD				23
 #define _SLOGC_MCD_SERVER		__C(_SLOGC_MCD,   0)

#define	_SLOGC_SLM				24
 #define _SLOGC_SLM_SERVER		__C(_SLOGC_SLM,   0)

#define	_SLOGC_VPF				25
 #define _SLOGC_VPF_SERVER		__C(_SLOGC_VPF,   0)
 #define _SLOGC_VPF_INSTALLER	__C(_SLOGC_VPF, 100)

#define _SLOGC_QDB              26

#define _SLOGC_MME              27

/* Add the next MAJOR code here. 
#define _SLOGC_NEXT_QNX			28
*/


/*
** Each of these major codes has 4096 minor codes available.
**
** OEM applications should define their own private major code
** header defines within the range PRIVATE_START-PRIVATE_END.
** These applications are not expected to be used with
** other OEM applications so there should be no conflict.
**
** Third party applications interacting with other third
** party applications (potentially OEM's) should contact
** QSSL for a private major code assignment.
*/
#define _SLOGC_PRIVATE_START 10000  /* For private use by OEM apps */
#define _SLOGC_PRIVATE_END	 19999  /* End of private OEM range */
#define _SLOGC_TEST          (_SLOGC_PRIVATE_START + 0)  

#define _SLOGC_3RDPARTY_START 20000 

#define _SLOGC_3RDPARTY_OEM00001_START		(_SLOGC_3RDPARTY_START+1)
#define _SLOGC_3RDPARTY_OEM00001_END		(_SLOGC_3RDPARTY_START+10)
#define _SLOGC_3RDPARTY_OEM00002_START		(_SLOGC_3RDPARTY_START+11)
#define _SLOGC_3RDPARTY_OEM00002_END		(_SLOGC_3RDPARTY_START+50)

#endif

/* __SRCVERSION("slogcodes.h $Rev: 167991 $"); */
