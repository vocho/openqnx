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
 * setjmp.s
 *	Routines for saving and restoring a context
 */
#include <mips/asm.h>

/*
 * setjmp:
 *
 *      ----------------
 *      |       PC      |    jmp_buf[0]
 *      -----------------
 *      |       SP      |           [1]
 *      -----------------
 *      |       S0      |           [2]
 *      -----------------
 *      |       S1      |           [3]
 *      -----------------
 *      |       S2      |           [4]
 *      -----------------
 *      |       ...     |           ...
 *      -----------------
 *      |       S8      |           [10]
 *      -----------------
 */
 
/*
 * Note : Only uses 32bit CPU context
 */
 
 #
 # int setjmp (jmp_buf *ptr)
 #
FRAME(_setjmp,sp,0,ra)
        .set    noreorder
 
        sw      ra,(a0)         # save return address (PC) [0] 
        sw      sp,4(a0)        # save stack pointer       [1]
        sw      s0,8(a0)        # save reg s0              [2]
        sw      s1,12(a0)       # save reg s1              [3]
        sw      s2,16(a0)       # save reg s2              [4]
        sw      s3,20(a0)       # save reg s3              [5]
        sw      s4,24(a0)       # save reg s4              [6]
        sw      s5,28(a0)       # save reg s5              [7]
        sw      s6,32(a0)       # save reg s6              [8]
        sw      s7,36(a0)       # save reg s7              [9]
        sw      s8,40(a0)       # save reg s8              [10]
        j       ra
         move    v0,zero        # return 0 (bd slot)
 
        .set    reorder
ENDFRAME(_setjmp)
 
/*
 * _longjmp:
 *
 * Note : Only uses 32bit CPU context
 *
 */
 
 #
 # void _longjmp (jmp_buf *ptr, int ret_val)
 #
FRAME(_longjmp,sp,0,ra)
        .set    noreorder
 
		seq		t0,a1,zero		# if they want to return 0, return 1
        lw      ra,(a0)         # load return address (PC) [0] 
        lw      sp,4(a0)        # load stack pointer       [1]
        lw      s0,8(a0)        # load reg s0              [2]
        lw      s1,12(a0)       # load reg s1              [3]
        lw      s2,16(a0)       # load reg s2              [4]
        lw      s3,20(a0)       # load reg s3              [5]
        lw      s4,24(a0)       # load reg s4              [6]
        lw      s5,28(a0)       # load reg s5              [7]
        lw      s6,32(a0)       # load reg s6              [8]
        lw      s7,36(a0)       # load reg s7              [9]
        lw      s8,40(a0)       # load reg s8              [10]
        j       ra
         or		v0,a1,t0        # return ret_val (bd slot)
 
        .set    reorder
ENDFRAME(_longjmp)
