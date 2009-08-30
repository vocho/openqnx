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



#ifndef __MIPS_SB1_CPU_H_INCLUDED
#define __MIPS_SB1_CPU_H_INCLUDED

/* Broadcom SB-1 CP0 Cache Registers */

/*
 * cp0 register 26 sel 0
 */
#define SB_CERR_DATA    0x40000000  /* err in D space */
#define SB_CERR_INST    0x20000000  /*  inst */
#define SB_CERR_MB      0x00800000  /* multiple error */
#define SB_TLB_SHUTDOWN 0x00008000  /* TLB shutdown */
#define SB_TIME_OUT     0x00004000  /* timeout machine check */

/*
 * cp0 register 27 sel 0
 */

#define SB_I_TAG        0x20000000 /* tag parity */
#define SB_I_DATA       0x10000000 /* data array */
#define SB_I_EXTERNAL   0x04000000 /* external */
#define SB_I_IDX        0x00001F70 /* I cache index */

/*
 * cp0 register 27 sel 1
 */

#define SB_D_MUTIPLE                0x80000000 /* multiple data error*/
#define SB_D_TAG_STATE_PARITY       0x40000000 
#define SB_D_TAG_ADDR_PARITY        0x20000000 
#define SB_D_TAG_DATA_SINGLE_ERR    0x10000000 
#define SB_D_TAG_DATA_DOUBLE_ERR    0x08000000 
#define SB_D_EXTERNAL_ERR           0x04000000
#define SB_D_LOAD_ERR               0x02000000
#define SB_D_STORE_ERR              0x01000000
#define SB_D_FILL_ERR               0x00800000
#define SB_D_COH_ERR                0x00400000
#define SB_D_DT_ERR                 0x00200000

/*
 * cp0 register 28 sel 2
 */

#define SB_TAG_LO_2_PTAG_SHIFT              13
#define SB_TAG_LO_32_BITS_PTAG_MASK         0xFFFFE000
#define SB_TAG_LO_31_13_BITS_PTAG_MASK      0x0007FFFF /* mask after shift 13 */

/*
 * cp0 register 29 sel 2
 * cache state bits encoding
 */

#define SB_TAG_HI_CACHE_STATE_INVALID       0x0
#define SB_TAG_HI_CACHE_STATE_COH_SHARE     0x10000000
#define SB_TAG_HI_CACHE_STATE_CLEAN         0x20000000
#define SB_TAG_HI_CACHE_STATE_DIRTY         0x30000000

#define SB_CACHE_WAY_MASK                 0x00006000
#define SB_CACHE_WAY_SHIFT					13

/* cache operations (are these SB-1 specific?) */
#define CODE_PRI_INSTR  0
#define CODE_PRI_DATA   1
#define CODE_TER_DATA   2
#define CODE_SEC_DATA   3

#define INDEX_WRBACK_INVALID_DATA       ((0 << 2) | CODE_PRI_DATA)
#define INDEX_WRBACK_INVALID_SEC_DATA   ((0 << 2) | CODE_SEC_DATA)
#define FLASH_INVALID_TER_DATA          ((0 << 2) | CODE_TER_DATA)
#define INDEX_LD_TAG_DATA               ((1 << 2) | CODE_PRI_DATA)
#define INDEX_LD_TAG_TER_DATA           ((1 << 2) | CODE_TER_DATA)
#define INDEX_ST_TAG_DATA               ((2 << 2) | CODE_PRI_DATA)
#define INDEX_ST_TAG_SEC_DATA           ((2 << 2) | CODE_SEC_DATA)
#define INDEX_ST_TAG_TER_DATA           ((2 << 2) | CODE_TER_DATA)
#define CREAT_DIRTY_EXCLUSIVE_DATA      ((3 << 2) | CODE_PRI_DATA)
#define HIT_INVALID_DATA                ((4 << 2) | CODE_PRI_DATA)
#define HIT_INVALID_SEC_DATA            ((4 << 2) | CODE_SEC_DATA)
#define HIT_WRBACK_INVALID_DATA         ((5 << 2) | CODE_PRI_DATA)
#define HIT_WRBACK_INVALID_SEC_DATA     ((5 << 2) | CODE_SEC_DATA)
#define PAGE_INVALIDATE_SEC_DATA        ((5 << 2) | CODE_SEC_DATA)
#define PAGE_INVALIDATE_TER_DATA        ((5 << 2) | CODE_TER_DATA)
#define HIT_WRBACK_DATA                 ((6 << 2) | CODE_PRI_DATA)
#define HIT_WRBACK_SEC_DATA             ((6 << 2) | CODE_SEC_DATA)

#define INDEX_INVALID_INSTR             ((0 << 2) | CODE_PRI_INSTR)
#define INDEX_LD_TAG_INSTR              ((1 << 2) | CODE_PRI_INSTR)
#define INDEX_ST_TAG_INSTR              ((2 << 2) | CODE_PRI_INSTR)
#define HIT_INVALID_INSTR               ((4 << 2) | CODE_PRI_INSTR)
#define FILL_INSTR                      ((5 << 2) | CODE_PRI_INSTR)
#define HIT_WRBACK_INSTR                ((6 << 2) | CODE_PRI_INSTR)

#endif /* __MIPS_SB1_CPU_H_INCLUDED */

/* __SRCVERSION("sb1cpu.h $Rev: 153052 $"); */
