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
 *  sh/opcode.h
 *

 */

#ifndef __SH_OPCODE_H_INCLUDED
#define __SH_OPCODE_H_INCLUDED

#ifndef __SH_CONTEXT_H_INCLUDED
#include <sh/context.h>
#endif

// SH instruction
#if defined(__LITTLEENDIAN__ )

	union sh_instr {
		unsigned short op_code;

		struct {
			unsigned short disp:8;
			unsigned short op:8;
		} i_d8;
		struct {
			unsigned short disp:12;
			unsigned short op:4;
		} i_d12;
		struct {
			unsigned short func:8;
			unsigned short reg:4;
			unsigned short op:4;
		} i_f ;
	};
	
#else /* __BIGENDIAN__ */

	union sh_instr {
		unsigned short op_code;

		struct {
			unsigned short op:8;
			unsigned short disp:8;
		} i_d8;
		struct {
			unsigned short op:4;
			unsigned short disp:12;
		} i_d12;
		struct {
			unsigned short op:4;
			unsigned short reg:4;
			unsigned short func:8;
		} i_f ;
	};

#endif

// SH instructions opcodes

/* 
1000xxxxdddddddd
	BF (1011)
	BF/S (1111)
	BT (1001)
	BT/S (1101) 
	*/
#define OPCODE_B1	0x8
#define OPCODE_BF	0x8b
#define OPCODE_BF_S 0x8f
#define OPCODE_BT   0x89
#define OPCODE_BT_S 0x8d

/* 
101xdddddddddddd
	BRA (0)
	BSR (1) 
	*/
#define OPCODE_BRA	0xA
#define OPCODE_BSR	0xB

/* 
0000nnnnxxxx0011
	BRAF (0010)
	BSRF (0000)
0000000000001011
	RTS
0100nnnnxxxx1011
	JMP (0010)
	JSR (0000)
	*/
#define OPCODE_B2	0x0
#define OPCODE_BRAF 0x23
#define OPCODE_BSRF 0x03
#define OPCODE_RTS	0x000B

#define OPCODE_J	0x4
#define OPCODE_JMP	0x2B
#define OPCODE_JSR	0x0B


#endif
