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
 *  mips/opcode.h
 *

 */
#ifndef __MIPS_OPCODE_H_INCLUDED
#define __MIPS_OPCODE_H_INCLUDED

#ifndef __ASM__
/*
 * R4K CPU instruction format
 */
#if defined(__BIGENDIAN__)

	union r4k_instr {
		unsigned int op_code;
	
		struct {
			unsigned op : 6;
			unsigned target : 26;
		} j_t;
	
		struct {
			unsigned op : 6;
			unsigned rs : 5;
			unsigned rt : 5;
			signed s_imd : 16;
		} i_t;

		struct {
		    unsigned op : 6;
			unsigned base : 5;
			unsigned ft : 5;
			signed s_offset : 16;
	    } fi_t;
	
		struct {
			unsigned op : 6;
			unsigned rs : 5;
			unsigned rt : 5;
			unsigned u_imd : 16;
		} u_t;
	
		struct {
			unsigned op : 6;
			unsigned rs : 5;
			unsigned rt : 5;
			unsigned rd : 5;
			unsigned re : 5;
			unsigned func : 6;
		} r_t;

		struct {
            unsigned op : 6;
            unsigned fmt : 5;
            unsigned ft : 5;
            unsigned fs : 5;
            unsigned fd : 5;
            unsigned func : 6;
		} fr_t;
	
		struct {
			unsigned op : 6;
			unsigned base : 5;
			unsigned func : 3;
			unsigned cache : 2;
			signed s_imd : 16;
		} c_t;
	};

#else /* __LITTLEENDIAN__ */

	union r4k_instr {
		unsigned int op_code;
	
		struct {
			unsigned target : 26;
			unsigned op : 6;
		} j_t;
	
		struct {
			signed s_imd : 16;
			unsigned rt : 5;
			unsigned rs : 5;
			unsigned op : 6;
		} i_t;
	
        struct {
            signed s_offset : 16;
            unsigned ft : 5;
            unsigned base : 5;
            unsigned op : 6;
        } fi_t;

		struct {
			unsigned u_imd : 16;
			unsigned rt : 5;
			unsigned rs : 5;
			unsigned op : 6;
		} u_t;
	
		struct {
			unsigned func : 6;
			unsigned re : 5;
			unsigned rd : 5;
			unsigned rt : 5;
			unsigned rs : 5;
			unsigned op : 6;
		} r_t;

		struct {
			unsigned func : 6;
			unsigned fd : 5;
            unsigned fs : 5;
            unsigned ft : 5;
            unsigned fmt : 5;
            unsigned op : 6;
		} fr_t;
	
		struct {
			signed s_imd : 16;
			unsigned cache : 2;
			unsigned func : 3;
			unsigned base : 5;
			unsigned op : 6;
		} c_t;
	};

#endif

#endif /* __ASM__ */

/*
 * R4K CPU instruction opcode
 */

#define OPCODE_BREAKX(code)	   (0x0000000D+((code)<<16))
#define OPCODE_BREAK		   OPCODE_BREAKX(5)
#define OPCODE_LB              0x20
#define OPCODE_LBU             0x24
#define OPCODE_LDC1            0x35
#define OPCODE_LHU             0x25
#define OPCODE_LH              0x21
#define OPCODE_LWU             0x27
#define OPCODE_LW              0x23
#define OPCODE_LWC1            0x31
#define OPCODE_LWL             0x22
#define OPCODE_LWR             0x26
#define OPCODE_LD              0x37
#define OPCODE_LDL             0x1A
#define OPCODE_LDR             0x1B
#define OPCODE_LUI			   0x0f
#define OPCODE_LL			   0x30
#define OPCODE_SC			   0x38

#define OPCODE_SB              0x28
#define OPCODE_SDC1            0x3d
#define OPCODE_SDL             0x2C
#define OPCODE_SDR             0x2D
#define OPCODE_SH              0x29
#define OPCODE_SWL             0x2A
#define OPCODE_SW              0x2B
#define OPCODE_SWC1            0x39
#define OPCODE_SWR             0x2E
#define OPCODE_SD              0x3f

#define OPCODE_SPECIAL         0x00
#define OPCODE_JALR            0x09
#define OPCODE_JR              0x08

#define OPCODE_REGIMM          0x01
#define OPCODE_BGEZ            0x01
#define OPCODE_BGEZAL          0x11
#define OPCODE_BLTZ            0x00
#define OPCODE_BLTZAL          0x10
#define OPCODE_BGEZL           0x03
#define OPCODE_BGEZALL         0x13
#define OPCODE_BLTZL           0x02
#define OPCODE_BLTZALL         0x12

#define OPCODE_J               0x02
#define OPCODE_JAL             0x03
#define OPCODE_BEQ             0x04
#define OPCODE_BGTZ            0x07
#define OPCODE_BLEZ            0x06
#define OPCODE_BNE             0x05
#define OPCODE_BEQL            0x14
#define OPCODE_BGTZL           0x17
#define OPCODE_BLEZL           0x16
#define OPCODE_BNEL            0x15

#define OPCODE_JR_RA           0x03E00008
#define OPCODE_ADDIU           0x09
#define OPCODE_ADDU            0x21
#define OPCODE_ORI	       0x0d
#define OPCODE_OR	       0x25

#endif /* __MIPS_OPCODE_H_INCLUDED */

/* __SRCVERSION("opcode.h $Rev: 153052 $"); */
