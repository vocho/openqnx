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
 *  tracecod.h     Trace codes for the Trace utility
 *

 */

#ifndef __TRACECODE_H_INCLUDED
#define __TRACECODE_H_INCLUDED

#define _TRACE_PROC         (1 << 12)
  #define _TRACE_PROC_FORCE_READY     (_TRACE_PROC |  3)
  #define _TRACE_PROC_DEATH           (_TRACE_PROC |  4)
  #define _TRACE_PROC_ADDRESS         (_TRACE_PROC |  5)
  #define _TRACE_PROC_VC_DETACH       (_TRACE_PROC |  6)
  #define _TRACE_PROC_LICENSE         (_TRACE_PROC |  7)
  #define _TRACE_PROC_TXNAK_VID       (_TRACE_PROC |  8)
  #define _TRACE_PROC_RXNAK_VID       (_TRACE_PROC |  9)
  #define _TRACE_PROC_SESSION         (_TRACE_PROC | 10)
  #define _TRACE_PROC_GROW_VID        (_TRACE_PROC | 11)
  #define _TRACE_PROC_KICK1           (_TRACE_PROC | 12)
  #define _TRACE_PROC_KICK2           (_TRACE_PROC | 13)
  #define _TRACE_PROC_READMSG_FAILED  (_TRACE_PROC | 15)
  #define _TRACE_PROC_REPLY_FAILED    (_TRACE_PROC | 16)
  #define _TRACE_PROC_VIDPOLLDET      (_TRACE_PROC | 17)
  #define _TRACE_PROC_MSGFAULT        (_TRACE_PROC | 18)
  #define _TRACE_PROC_FAULT           (_TRACE_PROC | 19)
  #define _TRACE_PROC_NO_PID          (_TRACE_PROC | 20)
  #define _TRACE_PROC_NO_VID          (_TRACE_PROC | 21)
  #define _TRACE_PROC_NO_MID          (_TRACE_PROC | 22)
  #define _TRACE_PROC_NO_SHAREPTR     (_TRACE_PROC | 23)
  #define _TRACE_PROC_NO_HEAP         (_TRACE_PROC | 24)
  #define _TRACE_PROC_NO_NETQUEUE     (_TRACE_PROC | 25)
  #define _TRACE_PROC_NO_NAME         (_TRACE_PROC | 26)
  #define _TRACE_PROC_NO_TIMER        (_TRACE_PROC | 27)
  #define _TRACE_PROC_BAD_VID_MEM     (_TRACE_PROC | 28)
  #define _TRACE_PROC_NOT_READY       (_TRACE_PROC | 29)
  #define _TRACE_PROC_BAD_REC_STATE   (_TRACE_PROC | 30)
  #define _TRACE_PROC_VIDREPLYLOST    (_TRACE_PROC | 31)
  #define _TRACE_PROC_BAD_TRACE       (_TRACE_PROC | 50)
  #define _TRACE_PROC_TEST            (_TRACE_PROC | 99)

#define _TRACE_DEV          (2 << 12)
  /*
   * Dev.ser trace codes
   */
  #define _TRACE_DEV_SER_PARITY       (_TRACE_DEV |  1)
  #define _TRACE_DEV_SER_FRAME        (_TRACE_DEV |  2)
  #define _TRACE_DEV_SER_OVERRUN      (_TRACE_DEV |  3)
  #define _TRACE_DEV_SER_TIMEOUT      (_TRACE_DEV |  4)
  #define _TRACE_DEV_SER_CARRIER      (_TRACE_DEV |  5)
  #define _TRACE_DEV_SER_HUP          (_TRACE_DEV |  6)
  /*
   * Dev trace codes
   */
  #define _TRACE_DEV_SIGHUP           (_TRACE_DEV | 0x10)
  #define _TRACE_DEV_SIGINT           (_TRACE_DEV | 0x11)
  #define _TRACE_DEV_SIGQUIT          (_TRACE_DEV | 0x12)
  #define _TRACE_DEV_SIGWINCH         (_TRACE_DEV | 0x13)
  #define _TRACE_DEV_SIGTSTP          (_TRACE_DEV | 0x14)
  /*
   * Dev.bipar trace codes
   */
  #define _TRACE_DEV_BIPAR_STATE      (_TRACE_DEV | 0x20)
  /*
   * Dev.con trace codes
   */
  #define _TRACE_DEV_CON_ERROR        (_TRACE_DEV | 0x40)
  #define _TRACE_DEV_CON_PARITY       (_TRACE_DEV | 0x41)
  #define _TRACE_DEV_CON_UNKNOWN      (_TRACE_DEV | 0x42)


#define _TRACE_FSYS         (3 << 12)
#define _TRACE_CSC2         (_TRACE_FSYS+0x100)
#define	_TRACE_FSYS_FDC		(_TRACE_FSYS+0x200)
#define _TRACE_FSYS_AT		(_TRACE_FSYS+0x300)
#define _TRACE_FSYS_AHA		(_TRACE_FSYS+0x400)
#define _TRACE_FSYS_PS2		(_TRACE_FSYS+0x500)

/*
 *  Csc2 is like a file system, so we use one of the Fsys trace subranges
 */
#define _TRACE_CSC2_CHDIR_FAILED        (_TRACE_CSC2 + 0)
#define _TRACE_CSC2_OPEN_FAILED         (_TRACE_CSC2 + 1)
#define _TRACE_CSC2_STAT_FAILED         (_TRACE_CSC2 + 2)
#define _TRACE_CSC2_EBADF               (_TRACE_CSC2 + 3)

/* traces for the floppy disk drive */
#define _TRACE_FDC_IOFAILED             (_TRACE_FSYS_FDC + 0)


/*
 *	The Network Manager and Network Drivers fit into one range
 *	of 4096, which gives us room for a total of 40 drivers.
 */
#define _TRACE_NET          (4 << 12)
                          
#define _TRACE_CAM          (5 << 12)

#define _TRACE_SOCKET       (6 << 12)
  #define _TRACE_SOCKET_KILL          (_TRACE_SOCKET |  1)
  #define _TRACE_SOCKET_MEMORY        (_TRACE_SOCKET |  2)
  #define _TRACE_SOCKET_PROXY         (_TRACE_SOCKET |  3)
  #define _TRACE_SOCKET_THREAD        (_TRACE_SOCKET |  4)
  #define _TRACE_SOCKET_DEBUG         (_TRACE_SOCKET |  5)

/*
 *  PCMCIA manager
 */ 
#define _TRACE_PCMCIA		(7 << 12)
  #define _TRACE_PCMCIA_IO            (_TRACE_PCMCIA | 1)			
  #define _TRACE_PCMCIA_JEDEC         (_TRACE_PCMCIA | 2)			
  #define _TRACE_PCMCIA_WP            (_TRACE_PCMCIA | 3)			
  #define _TRACE_PCMCIA_ID            (_TRACE_PCMCIA | 4)			
  #define _TRACE_PCMCIA_REGION        (_TRACE_PCMCIA | 5)			
  
/*
 *  Photon server/lib
 */ 
#define _TRACE_PHOTON		(8 << 12)
  #define _TRACE_PHOTON_RECEIVE       (_TRACE_PHOTON | 100)
  #define _TRACE_PHOTON_CRECEIVE      (_TRACE_PHOTON | 101)
  #define _TRACE_PHOTON_EVENT_MALLOC  (_TRACE_PHOTON | 102)
  #define _TRACE_PHOTON_EVENT_REALLOC (_TRACE_PHOTON | 103)
  #define _TRACE_PHOTON_EVENT_READ    (_TRACE_PHOTON | 104)
  #define _TRACE_PHOTON_ADDWORK_PROC  (_TRACE_PHOTON | 105)
  #define _TRACE_PHOTON_ADDINPUT_PROC (_TRACE_PHOTON | 106)
  #define _TRACE_PHOTON_RESOURCE_PRESORT (_TRACE_PHOTON | 107)

#define _TRACE_TEMPORARY    (0xFFFFFL << 12)

#endif
