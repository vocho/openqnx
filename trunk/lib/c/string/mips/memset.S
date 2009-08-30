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
 * Get global defs.  Remove conflicting names from other architectures
 */
#include <mips/asm.h>

/*
 * memset:
 *
 * Very fast memset.  Will handle misaligned destinations.
 *
 * void *memset( void *dest, int c, size_t __n )
 */
FRAME(memset,sp,0,ra)
        PIC_DIRECTIVE
        .set    noreorder

        /* save original address to return */
        move    v0,a0

#if defined(VARIANT_be)

        slti    t0,a2,16                # less than 16 bytes?
        bnez    t0,mem_bset
         li     t0,3

        /*
         * If the byte to set is not zero, then it
         * has to be packed into a word for word sets.
         */
        beqz    a1,1f
         li     t2,4
        sll     t3,a1,24                # t3 = XX000000, a1 = 000000XX
        move    t4,a1
        sll     t4,a1,16
        or      t3,t4,t3                # t3 = XXXX0000
        sll     t4,a1,8
        or      t3,t4,t3                # t3 = XXXXXX00
        or      a1,t3,a1                # a1 = XXXXXXXX

1:        and     t0,a0,t0                # check alignment
        beqz    t0,mem_set64
         slti   t1,a2,64                # less than 64 bytes to set?

        /* do byte copies to bring us to an aligned address */
        sub     t3,t2,t0                # t3 gives unaligned bytecnt
        sb      a1,0(a0)
        addiu   a0,a0,1
        addiu   t3,t3,-1
        beqz    t3,mem_set64_chk
         addiu  a2,a2,-1
        sb      a1,0(a0)
        addiu   a0,a0,1
        addiu   t3,t3,-1
        beqz    t3,mem_set64_chk
         addiu  a2,a2,-1
        sb      a1,0(a0)
        addiu   a0,a0,1
        addiu   t3,t3,-1
        addiu  	a2,a2,-1

mem_set64_chk:
        slti    t1,a2,64

mem_set64:
        bnez	t1,mem_set32
         slti   t1,a2,32
mem_set64_loop:
        sw      a1,0(a0)
        sw      a1,4(a0)
        sw      a1,8(a0)
        sw      a1,12(a0)
        sw      a1,16(a0)
        sw      a1,20(a0)
        sw      a1,24(a0)
        sw      a1,28(a0)
        sw      a1,32(a0)
        sw      a1,36(a0)
        sw      a1,40(a0)
        sw      a1,44(a0)
        sw      a1,48(a0)
        sw      a1,52(a0)
        sw      a1,56(a0)
        sw      a1,60(a0)
        addiu   a2,a2,-64
        slti    t1,a2,64
        b       mem_set64
         addiu  a0,a0,64

mem_set32:
        bnez    t1,mem_set16
         slti   t1,a2,16
mem_set32_loop:
        sw      a1,0(a0)
        sw      a1,4(a0)
        sw      a1,8(a0)
        sw      a1,12(a0)
        sw      a1,16(a0)
        sw      a1,20(a0)
        sw      a1,24(a0)
        sw      a1,28(a0)
        addiu   a2,a2,-32
        slti    t1,a2,32
        b       mem_set32
         addiu  a0,a0,32


mem_set16:
        bnez    t1,mem_bset
         nop
        sw      a1,0(a0)
        sw      a1,4(a0)
        sw      a1,8(a0)
        sw      a1,12(a0)
        addiu   a2,a2,-16
        slti    t1,a2,16
        b       mem_set16
         addiu  a0,a0,16

#else /* little endian */

		// Just let it fall through, mem_bset will do the byte memset
#endif

mem_bset:
        beqz    a2,mem_set_exit
         nop
1:
        sb      a1,0(a0)
        addiu   a2,a2,-1
        bnez    a2,1b
         addiu  a0,a0,1

mem_set_exit:
        j       ra
         nop

        .set reorder
ENDFRAME(memset)
