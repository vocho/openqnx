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

#ifndef __PLATFORM_H_INCLUDED
#include <sys/platform.h>
#endif

#ifndef __SH_CONTEXT_H_INCLUDED
#include _NTO_HDR_(sh/context.h)
#endif

/* SH instruction */
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
		struct {
			unsigned short func:4;
			unsigned short reg2:4;
			unsigned short reg1:4;
			unsigned short op:4;
		} i_r ;
		struct {
			unsigned short disp:4;
			unsigned short reg:4;
			unsigned short op:8;
		} i_d4;
		struct {
			unsigned short disp:4;
			unsigned short reg2:4;
			unsigned short reg1:4;
			unsigned short op:4;
		} i_d4r;
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
		struct {
			unsigned short op:4;
			unsigned short reg1:4;
			unsigned short reg2:4;
			unsigned short func:4;
		} i_r ;
		struct {
			unsigned short op:8;
			unsigned short reg:4;
			unsigned short disp:4;
		} i_d4;
		struct {
			unsigned short op:4;
			unsigned short reg1:4;
			unsigned short reg2:4;
			unsigned short disp:4;
		} i_d4r;
	};

#endif

/* SH instructions opcodes */

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

/*
0100mmmmXXXXXX
LDC
*/
#define OPCODE_LDC			0x4
#define FCODE_LDC_RM_SR		0xe

/*
0000nnnnXXXXXX
STC
*/
#define OPCODE_STC			0x0
#define FCODE_STC_SR_RN		0x2

/*
0010nnnnmmmmXXXX
mov.(w,l)	Rm,@Rn
*/
#define OPCODE_MOV_RM_ARN	0x2
#define FCODE_MOVW_RM_ARN	0x1
#define FCODE_MOVL_RM_ARN	0x2

/*
0110nnnnmmmmXXXX
mov.(w,l)	@Rm,Rn
*/
#define OPCODE_MOV_ARM_RN	0x6
#define FCODE_MOVW_ARM_RN	0x1
#define FCODE_MOVL_ARM_RN	0x2

/*
10000001nnnndddd
mov.w	R0,@(disp,Rn)
*/
#define OPCODE_MOVW_R0_ADRN	0x81

/*
0001nnnnmmmmdddd
mov.l	Rm,@(disp,Rn)
*/
#define OPCODE_MOVL_RM_ADRN	0x1

/*
10000101mmmmdddd
mov.w	@(disp,Rm),R0
*/
#define OPCODE_MOVW_ADRM_R0	0x85

/*
0101nnnnmmmmdddd
mov.l	@(disp,Rm),Rn
*/
#define OPCODE_MOVL_ADRM_RN	0x5

/*
0000nnnnmmmmXXXX
mov.(w,l)	Rm,@(R0,Rn)
*/
#define OPCODE_MOV_RM_AR0RN	0x0
#define FCODE_MOVW_RM_AR0RN	0x5
#define FCODE_MOVL_RM_AR0RN	0x6

/*
0000nnnnmmmmXXXX
mov.(w,l)	@(R0,Rm),Rn
*/
#define OPCODE_MOV_AR0RM_RN	0x0
#define FCODE_MOVW_AR0RM_RN	0xd
#define FCODE_MOVL_AR0RM_RN	0xe

#endif

/* __SRCVERSION("opcodes.h $Rev: 153052 $"); */
