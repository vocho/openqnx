/*
 * $QNXLicenseC: 
 * Copyright 2007, QNX Software Systems.  
 *  
 * Licensed under the Apache License, Version 2.0 (the "License"). You  
 * may not reproduce, modify or distribute this software except in  
 * compliance with the License. You may obtain a copy of the License  
 * at: http://www.apache.org/licenses/LICENSE-2.0  
 *  
 * Unless required by applicable law or agreed to in writing, software  
 * distributed under the License is distributed on an "AS IS" basis,  
 * WITHOUT WARRANTIES OF ANY KIND, either express or implied. 
 * 
 * This file may contain contributions from others, either as  
 * contributors under the License or as licensors under other terms.   
 * Please review this entire file for other proprietary rights or license  
 * notices, as well as the QNX Development Suite License Guide at  
 * http://licensing.qnx.com/license-guide/ for other information. 
 * $ 
 */



/*
 * VFP System Registers
 */

#ifndef	__ARM_VFP_H_INCLUDED
#define	__ARM_VFP_H_INCLUDED

/*
 * VFP coprocessor registers
 */
#define	ARM_VFP_FPSID			c0
#define	ARM_VFP_FPSCR			c1
#define	ARM_VFP_MVFR0			c7		/* ARMv7 */
#define	ARM_VFP_MVFR1			c6		/* ARMv7 */
#define	ARM_VFP_FPEXC			c8
#define	ARM_VFP_FPINST			c9		/* VFP sub-architecture */
#define	ARM_VFP_FPINST2			c10		/* VFP sub-architecture */

/*
 * FPSCR bits
 */

#define	ARM_VFP_FPSCR_N			(1<<31)
#define	ARM_VFP_FPSCR_Z			(1<<30)
#define	ARM_VFP_FPSCR_C			(1<<29)
#define	ARM_VFP_FPSCR_V			(1<<28)
#define	ARM_VFP_FPSCR_DN		(1<<25)	/* default Nan mode                   */
#define	ARM_VFP_FPSCR_FZ		(1<<24)	/* flush to zero mode                 */
#define	ARM_VFP_FPSCR_RMODE		(3<<22)	/* rounding mode mask                 */
#define	ARM_VFP_FPSCR_RN		(0<<22)	/* round to nearest mode              */
#define	ARM_VFP_FPSCR_RP		(1<<22)	/* round to plus infinity             */
#define	ARM_VFP_FPSCR_RM		(2<<22)	/* round to minus infinity            */
#define	ARM_VFP_FPSCR_STRIDE	(3<<20)	/* stride mask                        */
#define	ARM_VFP_FPSCR_LEN		(7<<16)	/* len mask                           */
#define	ARM_VFP_FPSCR_IDE		(1<<15)	/* input subnormal exception enable   */
#define	ARM_VFP_FPSCR_IXE		(1<<12)	/* inexact exception enable           */
#define	ARM_VFP_FPSCR_UFE		(1<<11)	/* underflow exception enable         */
#define	ARM_VFP_FPSCR_OFE		(1<<10)	/* overflow exception enable          */
#define	ARM_VFP_FPSCR_DZE		(1<< 9)	/* divide by zero exception enable    */
#define	ARM_VFP_FPSCR_IOE		(1<< 8)	/* invalid operation exception enable */
#define	ARM_VFP_FPSCR_IDC		(1<< 7)	/* input subnormal cumulative flag    */
#define	ARM_VFP_FPSCR_IXC		(1<< 4)	/* inexact cumulative flag            */
#define	ARM_VFP_FPSCR_UFC		(1<< 3)	/* underflow cumulative flag          */
#define	ARM_VFP_FPSCR_OFC		(1<< 2)	/* overflow cumulative flag           */
#define	ARM_VFP_FPSCR_DZC		(1<< 1)	/* divide by zero cumulative flag     */
#define	ARM_VFP_FPSCR_IOC		(1<< 0)	/* invalid operation cumulative flag  */

/*
 * Bits set for RunFast mode
 */
#define	ARM_VFP_RUNFAST_MODE	\
		(ARM_VFP_FPSCR_DN|ARM_VFP_FPSCR_FZ|ARM_VFP_FPSCR_RN)

/*
 * FPEXC bits
 *
 * Note that only EX and EN are architecturally defined.
 * The other bits are sub-architecture (implementation) defined.
 */

#define	ARM_VFP_FPEXC_EX		(1<<31)	/* exception state             */
#define	ARM_VFP_FPEXC_EN		(1<<30)	/* VFP enable                  */

#define	ARM_VFP_FPEXC_FP2V		(1<<28)	/* FPINST2 is valid            */
#define	ARM_VFP_FPEXC_VECITR	(7<< 8)	/* vector iteration            */
#define	ARM_VFP_FPEXC_INV		(1<< 7)	/* input exception             */
#define	ARM_VFP_FPEXC_UFC		(1<< 3)	/* potential underflow         */
#define	ARM_VFP_FPEXC_OFC		(1<< 2)	/* potential overflow          */
#define	ARM_VFP_FPEXC_IOC		(1<< 0)	/* potential invalid operation */

#endif	/* __ARM_VFP_H_INCLUDED */

/* __SRCVERSION("vfp.h $Rev: 211084 $"); */
