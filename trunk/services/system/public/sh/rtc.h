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
 *  sh/rtc.h    SH RTC (Real-time Clock) definitions
 *

 */

/* RCR1: RTC Control Register 1 */
#define SH_RTC_RCR1_CF		_ONEBIT8L(7)
#define SH_RTC_RCR1_CIE		_ONEBIT8L(4)
#define SH_RTC_RCR1_AIE		_ONEBIT8L(3)
#define SH_RTC_RCR1_AF		_ONEBIT8L(0)

/* RCR2: RTC Control Register 2 */
#define SH_RTC_RCR1_PEF		_ONEBIT8L(7)
#define SH_RTC_RCR1_PES_M	_BITFIELD8L(4,0x7)
#define SH_RTC_RCR1_PES(x)	_BITFIELD8L(4,x)
#define SH_RTC_RCR1_RTCEN	_ONEBIT8L(3)
#define SH_RTC_RCR1_ADJ		_ONEBIT8L(2)
#define SH_RTC_RCR1_RESET	_ONEBIT8L(1)
#define SH_RTC_RCR1_START	_ONEBIT8L(0)

/* __SRCVERSION("rtc.h $Rev: 153052 $"); */
