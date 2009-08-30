/*
 * $QNXLicenseC:
 * Copyright 2007, 2008, QNX Software Systems. All Rights Reserved.
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

#include "externs.h"
#include <ppc/opcode.h>
#include <ppc/inline.h>
#include <ppc/400cpu.h>

/*
 * Handle various and sundry fault conditions
 */

#if 0
	// For some reason, this vaddr_to_paddr call got introduced in
	// revision 1.10 by hzhou with the cryptic comment "400 clock".
	// I don't know why it'd be needed, and it doesn't work for
	// physical addresses above the one-to-one mapping area of the
	// kernel. bstecher
#define GET_OPCODE(op,iar) \
	op.op_code = *(uint32_t *)memmgr.vaddr_to_paddr(aspaces_prp[KERNCPU], (void*)(iar))
#else
#define GET_OPCODE(op,iar) \
	(op.op_code = *(uint32_t *)(iar))
#endif

 /*
  * is_store - figure out if the instruction is a store or not
  */
int
is_store(unsigned ins) {

	switch(ins >> (31-5)) {
 	case 36:
 	case 37:
 	case 38:
 	case 39:
 	case 44:
 	case 45:
 	case 47:
 	case 52:
 	case 53:
 	case 54:
 	case 55:
 		return( 1 );
 	case 31:
 	    switch( (ins >> 1) & 0x3ff ) {
 		case 151:
 		case 183:
 		case 215:
 		case 247:
 		case 407:
 		case 438:
 		case 439:
 		case 661:
 		case 662:
 		case 663:
 		case 695:
 		case 725:
 		case 727:
 		case 918:
 		case 983:
 			return( 1 );
		default:
			break;
 		}
 		break;
	default:
		break;
 	}
 	return( 0 );
 }

static void
emulate_fault(THREAD *thp, CPU_REGISTERS *regs, unsigned sig) {
	usr_fault(sig, actives[KERNCPU], 0);
	__ker_exit();
}

const struct fault_handlers emulate_fault_handlers = {
	emulate_fault, 0
};

static void
memcpy_w(uint8_t *dst, const uint8_t *src) {
	*(dst + 3) = *(src + 3);
	*(dst + 2) = *(src + 2);
	*(dst + 1) = *(src + 1);
	*dst       = *src;
}

static void
memcpy_wr(uint8_t *dst, const uint8_t *src) {
	*(dst + 3) = *src;
	*(dst + 2) = *(src + 1);
	*(dst + 1) = *(src + 2);
	*dst       = *(src + 3);
}

static void
loadh(uint8_t *target, const uint8_t *source, uint8_t f_sign) {
#if defined(__BIGENDIAN__)
	if(f_sign && (0x80 & *source) ) {
		// sign negtive
		*target = 0xff;
		*(target + 1) = 0xff;
	} else {
		// unsign load or sign positive
		*target = 0;
		*(target + 1) = 0;
	};

	*(target + 2) = *source;
	*(target + 3) = *(source + 1);
#elif defined(__LITTLEENDIAN__)
	*target = *source;
	*(target + 1) = *(source + 1);
	if(f_sign && (0x80 & *target) ) {
		// sign negtive
		*(target + 2) = 0xff;
		*(target + 3) = 0xff;
	} else {
		// unsign load or sign positive
		*(target + 2) = 0;
		*(target + 3) = 0;
	}
#else
	#error ENDIAN Not defined for system
#endif
}

static void
loadh_r(uint8_t *target, const uint8_t *source) {
#if defined(__BIGENDIAN__)
	*target = 0;
	*(target + 1) = 0;
	*(target + 3) = *source;
	*(target + 2) = *(source + 1);
#elif defined(__LITTLEENDIAN__)
	*(target + 1) = *source;
	*target  	  = *(source + 1);
	*(target + 2) = 0;
	*(target + 3) = 0;
#else
	#error ENDIAN Not defined for system
#endif
}

static void
storeh(uint8_t *target, const uint8_t *source) {
#if defined(__BIGENDIAN__)
	*target  = *(source+2);
	*(target + 1) = *(source+3);
#elif defined(__LITTLEENDIAN__)
	*target = *source ;
	*(target + 1) = *(source + 1);
#else
	#error ENDIAN Not defined for system
#endif
}


static void
storeh_r(uint8_t *target, const uint8_t *source) {
#if defined(__BIGENDIAN__)
	*target  = *(source+3);
	*(target + 1) = *(source+2);
#elif defined(__LITTLEENDIAN__)
	*target = *(source + 1) ;
	*(target + 1) = *source;
#else
	#error ENDIAN Not defined for system
#endif
}

static uint8_t *
addr_FPU(uint32_t num) {
	THREAD			*act = actives[KERNCPU];
	FPU_REGISTERS 	*fpudata;
	extern void fpusave_alloc(void);

	lock_kernel();
	if(act->fpudata==NULL) {
    	if(fpuemul) {
			if((act->fpudata = object_alloc(NULL, &fpu_souls)) == NULL) {
				usr_fault(SIGFPE + (FPE_NOMEM*256) + (FLTNOFPU*65536),act,KIP(act));
				return 0;
			}
		} else {
			fpusave_alloc();
		}
	}

	// Fetch the FPU regs out of the CPU
	fpudata = FPUDATA_PTR(act->fpudata);
#ifdef VARIANT_smp
	if(FPUDATA_INUSE(act->fpudata) && FPUDATA_CPU(act->fpudata) != KERNCPU) {
		SENDIPI(FPUDATA_CPU(act->fpudata), IPI_CONTEXT_SAVE);
		// We always come through here from user mode, so this is safe
		__ker_exit();
	}
#endif
	if(actives_fpu[KERNCPU] == act) {
		InterruptDisable();
		cpu_force_fpu_save(act);
		actives_fpu[KERNCPU] = NULL;
		InterruptEnable();
	}
	return(((uint8_t*)act->fpudata) + num*sizeof(ppcfloat));
}


/*
 * fix_alignment - handle a mis-aligned load/store
 *
 * 1. Operations could be emulated:
 * 1) load/store alignment exc for int/float
 * 2) load/store alignment exc for multiple regs or string
 * 3) *lmw or stmw crosses a segment or BAT boundary
 *
 * 2. Operations NOT emulated and cause exceptions
 * 1) direct-store segment related
 * 2) lwarx and stwcx.
 * 3) string load/store crosses a protection boundary
 * 4) ?? cache store exc in no-cache or write-through-cache, eg. dcbz.
 *
 * Misaligned single-precision floating point loads are handled by
 * copying the float operand into aligned storage, casting it to
 * double, and storing it to the FPU context.  Misaligned single
 * precision stores are handled in a similar manner: the FPU context
 * value is cast to a float in aligned storage, which is then copied
 * back to the unaligned operand in the user process.  Under GCC
 * 4.2.4 with -msoft-float, the cast operations result in calls to
 * __extendsfdf2() and__truncdfsf2() respectively, which are provided
 * by libgcc.a.  (These are implemented entirely as integer
 * operations, which precludes the possibility of them causing a
 * floating point exceptions themselves.)  Note that one consequence
 * of this approach is that storing a value which is out of range for
 * a float will provide, unsurprisingly, the same result as a cast
 * from double to float which could theoretically differ from the
 * hardware operation.
 */
int
fix_alignment(CPU_REGISTERS *ctx, uintptr_t reference) {
	union ppc_instr	op;
	uint16_t 		f_update, opcode1, opcode2;
	uint32_t 		*ptr_reg, wtmp, wtmp2;
	unsigned		count;
	uint8_t			*tmp;
	int				i;

	if(!WITHIN_BOUNDRY(reference, reference, aspaces_prp[KERNCPU]->boundry_addr)) {
		 return MAKE_SIGCODE(SIGBUS, BUS_ADRALN, FLTACCESS);
	}

	GET_OPCODE(op, ctx->iar);

	opcode1 = op.d_t.op;
	f_update = opcode1 & PPCOPCODE_LSUPDATE_MASK;
	ptr_reg = ctx->gpr + op.d_t.ds;
	SET_XFER_HANDLER(&emulate_fault_handlers);

	unlock_kernel();
	switch(opcode1) {
	case PPCOPCODE_LHA:
	case PPCOPCODE_LHAU:
		loadh((uint8_t*)&wtmp, (uint8_t*)reference, 1);
		lock_kernel();
		*ptr_reg = wtmp;
		break;
	case PPCOPCODE_LHZ:
	case PPCOPCODE_LHZU:
		loadh((uint8_t*)&wtmp, (uint8_t*)reference, 0);
		lock_kernel();
		*ptr_reg = wtmp;
		break;
	case PPCOPCODE_LWZ:
	case PPCOPCODE_LWZU:
		memcpy_w((uint8_t*)&wtmp, (uint8_t*)reference);
		lock_kernel();
		*ptr_reg = wtmp;
		break;
	case PPCOPCODE_STH:
	case PPCOPCODE_STHU:
		storeh((uint8_t*)reference, (uint8_t*)(uintptr_t)ptr_reg);
		lock_kernel();
		break;
	case PPCOPCODE_STW:
	case PPCOPCODE_STWU:
		memcpy_w((uint8_t*)reference, (uint8_t*)ptr_reg);
		lock_kernel();
		break;
	case PPCOPCODE_XFORM:
		// X Form instructions
		opcode2 = op.x_t.xo;
		f_update = opcode2 & PPCXOCODE_LSUPDATE_MASK;
		switch(opcode2) {
		case PPCXOCODE_LHAX:
		case PPCXOCODE_LHAUX:
			loadh((uint8_t*)&wtmp, (uint8_t*)reference, 1);
			lock_kernel();
			*ptr_reg = wtmp;
			break;
		case PPCXOCODE_LHZX:
		case PPCXOCODE_LHZUX:
			loadh((uint8_t*)&wtmp, (uint8_t*)reference, 0);
			lock_kernel();
			*ptr_reg = wtmp;
			break;
		case PPCXOCODE_LWZX:
		case PPCXOCODE_LWZUX:
			memcpy_w((uint8_t*)&wtmp, (uint8_t*)reference);
			lock_kernel();
			*ptr_reg = wtmp;
			break;
		case PPCXOCODE_STHX:
		case PPCXOCODE_STHUX:
			storeh((uint8_t*)reference, (uint8_t*)(uintptr_t)ptr_reg);
			lock_kernel();
			break;
		case PPCXOCODE_STWX:
		case PPCXOCODE_STWUX:
			memcpy_w((uint8_t*)reference, (uint8_t*)ptr_reg);
			lock_kernel();
			break;
		case PPCXOCODE_LHBRX:
			loadh_r((uint8_t*)&wtmp, (uint8_t*)reference);
			lock_kernel();
			*ptr_reg = wtmp;
			break;
		case PPCXOCODE_LWBRX:
			memcpy_wr((uint8_t*)&wtmp, (uint8_t*)reference);
			lock_kernel();
			*ptr_reg = wtmp;
			break;
		case PPCXOCODE_STHBRX:
			storeh_r((uint8_t*)reference, (uint8_t*)ptr_reg);
			lock_kernel();
			break;
		case PPCXOCODE_STWBRX:
			memcpy_wr((uint8_t*)reference, (uint8_t*)ptr_reg);
			lock_kernel();
			break;
		case PPCXOCODE_LFDX:
		case PPCXOCODE_LFDUX:
			ptr_reg = (uint32_t*)addr_FPU(op.x_t.ds);
			unlock_kernel();
			memcpy_w((uint8_t*)&wtmp, (uint8_t*)reference);
			memcpy_w((uint8_t*)&wtmp2, (uint8_t*)(reference+4));
			lock_kernel();
			*ptr_reg++ = wtmp;
			*ptr_reg = wtmp2;
			break;
		case PPCXOCODE_STFDX:
		case PPCXOCODE_STFDUX:
			ptr_reg = (uint32_t*)addr_FPU(op.x_t.ds);
			unlock_kernel();
			memcpy_w((uint8_t*)reference, (uint8_t*)(ptr_reg++));
			memcpy_w((uint8_t*)(reference+4), (uint8_t*)ptr_reg);
			lock_kernel();
			break;
		case PPCXOCODE_LFSX:
		case PPCXOCODE_LFSUX:
			{
				float value;
				ptr_reg = (uint32_t *) addr_FPU(op.x_t.ds);
				unlock_kernel();
				memcpy_w((uint8_t *) & value, (const uint8_t *) reference);
				lock_kernel();
				*(double *)ptr_reg = value;
			}
			break;
		case PPCXOCODE_STFSX:
		case PPCXOCODE_STFSUX:
			{
				float value;
				ptr_reg = (uint32_t *) addr_FPU(op.x_t.ds);
				value = *(const double *)ptr_reg;
				unlock_kernel();
				memcpy_w((uint8_t *) reference, (const uint8_t *) & value);
				lock_kernel();
			}
			break;
		case PPCXOCODE_LSWI:
			count = op.x_t.b;
			if(count == 0) count = 32;
			goto do_lsw;
		case PPCXOCODE_LSWX:
			count = ctx->xer & 0x7f;
			if(count == 0) break;
do_lsw:
			unlock_kernel();

			// Copy into a temp area first to get all the
			// memory faults out of the way (max of 128 bytes).
			tmp = _alloca(count + 3);
			memcpy(tmp, (void *)reference, count);

			//Round up to a multiple of four.
			while((count & 3) != 0) {
				tmp[count++] = 0;
			}

			// Do this locked so we don't have to worry about being
			// preempted after we start creaming register values.
			lock_kernel();

			i = 24;
			do {
				*ptr_reg = (*ptr_reg & ~(0xff << i)) | (*tmp << i);
				i -= 8;
				if(i < 0) {
					i = 24;
					++ptr_reg;
					if(ptr_reg > &ctx->gpr[31]) {
						ptr_reg = &ctx->gpr[0];
					}
				}
				++tmp;
			} while(--count > 0);
			break;
		case PPCXOCODE_STSWI:
			count = op.x_t.b;
			if(count == 0) count = 32;
			goto do_stsw;
		case PPCXOCODE_STSWX:
			count = ctx->xer & 0x7f;
			if(count == 0) break;
do_stsw:
			unlock_kernel();
			i = 24;
			do {
				*(uint8_t *)reference = *ptr_reg >> i;
				i -= 8;
				if(i < 0) {
					i = 24;
					++ptr_reg;
					if(ptr_reg > &ctx->gpr[31]) {
						ptr_reg = &ctx->gpr[0];
					}
				}
				++reference;
			} while(--count > 0);
			break;
		default:
			SET_XFER_HANDLER(NULL);
			return MAKE_SIGCODE(SIGBUS, BUS_ADRALN, FLTACCESS);
		}
		break;
	case PPCOPCODE_LFD:
	case PPCOPCODE_LFDU:
		ptr_reg = (uint32_t*)addr_FPU(op.d_t.ds);
		unlock_kernel();
		memcpy_w((uint8_t*)&wtmp, (uint8_t*)reference);
		memcpy_w((uint8_t*)&wtmp2, (uint8_t*)(reference+4));
		lock_kernel();
		*ptr_reg++ = wtmp;
		*ptr_reg = wtmp2;
		break;
	case PPCOPCODE_STFD:
	case PPCOPCODE_STFDU:
		ptr_reg = (uint32_t*)addr_FPU(op.d_t.ds);
		unlock_kernel();
		memcpy_w((uint8_t*)reference, (uint8_t*)(ptr_reg++));
		memcpy_w((uint8_t*)(reference+4), (uint8_t*)ptr_reg);
		lock_kernel();
		break;
	case PPCOPCODE_LFS:
	case PPCOPCODE_LFSU:
		{
			float value;
			ptr_reg = (uint32_t *) addr_FPU(op.d_t.ds);
			unlock_kernel();
			memcpy_w((uint8_t *) & value, (const uint8_t *) reference);
			lock_kernel();
			*(double *)ptr_reg = value;
		}
		break;
	case PPCOPCODE_STFS:
	case PPCOPCODE_STFSU:
		{
			float value;
			ptr_reg = (uint32_t *) addr_FPU(op.d_t.ds);
			value = *(const double *)ptr_reg;
			unlock_kernel();
			memcpy_w((uint8_t *) reference, (const uint8_t *) & value);
			lock_kernel();
		}
		break;
	case PPCOPCODE_STMW:
	case PPCOPCODE_LMW:
	default:
		SET_XFER_HANDLER(NULL);
		return MAKE_SIGCODE(SIGBUS, BUS_ADRALN, FLTACCESS);
	}

	// for update instructions, update its index reg
	lock_kernel();

	if(f_update) {
		ptr_reg = ctx->gpr + op.d_t.a;
		*ptr_reg = reference;
	}

	SET_XFER_HANDLER(NULL);

	// advance PC
	ctx->iar += sizeof(union ppc_instr);

	return(0);
}

/*
 * Emulate just the load and store float
 */
static int
emulate_lfd_stfd(CPU_REGISTERS *ctx, union ppc_instr *op, uint32_t ref, int store) {
	uint32_t 	*ptr_reg, wtmp, wtmp2;

	if(!WITHIN_BOUNDRY(ref, ref, aspaces_prp[KERNCPU]->boundry_addr))
		 return MAKE_SIGCODE(SIGBUS, BUS_ADRALN, FLTACCESS);

	SET_XFER_HANDLER(&emulate_fault_handlers);
	ptr_reg = (uint32_t*)addr_FPU(op->d_t.ds);
	unlock_kernel();
	if(store) {
		memcpy_w((uint8_t*)ref, (uint8_t*)(ptr_reg++));
		memcpy_w((uint8_t*)(ref+4), (uint8_t*)ptr_reg);
		lock_kernel();
	} else {
		memcpy_w((uint8_t*)&wtmp, (uint8_t*)ref);
		memcpy_w((uint8_t*)&wtmp2, (uint8_t*)(ref+4));
		lock_kernel();
		*ptr_reg++ = wtmp;
		*ptr_reg = wtmp2;
	}
	SET_XFER_HANDLER(NULL);
	ctx->iar += sizeof(union ppc_instr);

	return 0;
}

int
fpu_emulation(CPU_REGISTERS *ctx) {
	union ppc_instr 	op;
	uint16_t 			opcode1, opcode2;
	uint32_t 			ref;
	int 				ret, next = 0;

next:
	ret = MAKE_SIGCODE(SIGILL, ILL_ILLOPC, FLTILL);

	GET_OPCODE(op, ctx->iar);
	opcode1 = op.d_t.op;

	switch(opcode1) {
	// LFD and STFD are by far the most common case
	case PPCOPCODE_LFD:
	case PPCOPCODE_STFD:
		ref = ctx->gpr[op.d_t.a] + op.d_t.d;
		ret = emulate_lfd_stfd(ctx, &op, ref, opcode1 == PPCOPCODE_STFD);
		break;
	case PPCOPCODE_LFDU:
	case PPCOPCODE_STFDU:
		ret = fix_alignment(ctx, ctx->gpr[op.d_t.a] + op.d_t.d);
		break;

	case PPCOPCODE_XFORM:
		//X Form instructions
		opcode2 = op.x_t.xo;
		switch(opcode2) {
		case PPCXOCODE_LFDX:
		case PPCXOCODE_LFDUX:
		case PPCXOCODE_STFDX:
		case PPCXOCODE_STFDUX:
			if(op.xl_t.bi_a == 0) {
				ret = fix_alignment(ctx, ctx->gpr[op.xl_t.b]);
			} else {
				ret = fix_alignment(ctx, ctx->gpr[op.xl_t.bi_a] + ctx->gpr[op.xl_t.b]);
			}
			break;
		default:
			break;
		}
		break;

	default:
		break;
	}
	if(ret == 0) {
		next = 1;
		goto next;
	}

	return next ? 0 : ret;

}

/* cpu specific instruction emulation */
int
instr_emulation(CPU_REGISTERS *ctx) {
	union ppc_instr	op;
	uint16_t 		opcode1, opcode2;
	int 			ret;

	GET_OPCODE(op, ctx->iar);

	ret = MAKE_SIGCODE(SIGILL, ILL_ILLOPC, FLTILL);

	opcode1 = op.d_t.op;

	switch(opcode1) {
	case PPCOPCODE_XFORM:
		//X Form instructions
		opcode2 = op.x_t.xo;
		switch(opcode2) {
		case PPCXOCODE_MFTB:
#if defined(VARIANT_400)
	#define SPR_TBL	PPC400_SPR_TBLO
	#define SPR_TBH	PPC400_SPR_TBHI
#elif defined(VARIANT_booke)
	#define SPR_TBL	PPC_SPR_TBL
	#define SPR_TBH	PPC_SPR_TBU
#endif
#if defined(SPR_TBL)
			{
				uint32_t *ptr_reg = ctx->gpr + op.xfx_t.ds;

				// for emulate move from timer base in ppc 400/booke
				if(op.xfx_t.spr == 0x188) {
					// tmb low
					lock_kernel();
					*ptr_reg = get_spr(SPR_TBL);
					ctx->iar += 4;
					ret = 0;
				} else if(op.xfx_t.spr == 0x1a8) {
					// tmb high
					lock_kernel();
					*ptr_reg = get_spr(SPR_TBH);
					ctx->iar += 4;
					ret = 0;
				}

			}
#endif
			break;

#if defined(VARIANT_booke)
		// For Motorola E500 core.... :-(.
		case PPCXOCODE_LSWI:
		case PPCXOCODE_STSWI:
			if(op.xl_t.bi_a == 0) {
				ret = fix_alignment(ctx, 0);
			} else {
				ret = fix_alignment(ctx, ctx->gpr[op.xl_t.bi_a]);
			}
			break;

		case PPCXOCODE_LSWX:
		case PPCXOCODE_STSWX:
			if(op.xl_t.bi_a == 0) {
				ret = fix_alignment(ctx, ctx->gpr[op.xl_t.b]);
			} else {
				ret = fix_alignment(ctx, ctx->gpr[op.xl_t.bi_a] + ctx->gpr[op.xl_t.b]);
			}
			break;
#endif
		default:
			break;
		}
		break;
	default:
		break;
	}

	return ret;
}

__SRCVERSION("fault.c $Rev: 209153 $");
