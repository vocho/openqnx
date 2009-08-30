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

#include "externs.h"


//
// ker_savefpu is a pointer to an area to save the kernel FPU state if an
// fpu-disabled exception occurs (that is, an interrupt that requires the FPU
// interrupts a kernel call or interrupt that is also using the FPU).
//
SH_FPU_REGISTERS*	ker_savefpu[PROCESSORS_MAX];


/*
	registers for int handling:
		r8	-- systempage_ptr (formal r11)
		r9	-- id
		r10	-- intr_level_entry
		r12	-- intrinfo_entry
		r14	-- mask count (eoi only)
		r0 - r7 is working regs, r8, r12, r14 loading depends on gen flags
	Care: DO NOT CONFLICT WITH KERNEL (ENTERKERNEL)
*/

void
interrupt(INTRLEVEL *ilp, unsigned was_inkernel, SH_CPU_REGISTERS* reg_context_p) {
	INTERRUPT				*isr;
	THREAD					*thp;
	PROCESS					*orig_prp;
	PROCESS					**prpp;
	int						save_state;
	struct cpupage_entry	*cpupage;
	uint32_t				interrupt_save;
	void					*save_tls;
	unsigned				cpu = RUNCPU;
	SH_FPU_REGISTERS		fpu_save_area;
	SH_FPU_REGISTERS*		old_ker_savefpu;

#ifdef DOING_INTERRUPT_PROBLEM_CHECKING
	if((ilp->mask_count > 0) || ((inkernel & INKERNEL_INTRMASK) > 20)) {
		DebugKDBreak();
	}
#endif

	// Careful... we can take another interrupt while we're running here.
	old_ker_savefpu = ker_savefpu[cpu];
	// We use the fpscr as an indication of whether the save area got used.
	// If it gets used, the fpscr value will be overwritten with non-zero.
	fpu_save_area.fpscr = 0;
	// If we were coming from a kernel call or interrupt, and we had the
	// FPU enabled, we need to be prepared to preserve our FPU context.
	if (was_inkernel && ((reg_context_p->sr&SH_SR_FD)==0) ) {
		// Tell anybody who takes an FPU-disabled exception where to find our save area
		ker_savefpu[cpu] = &fpu_save_area;
		// Disable the FPU so anybody who tries to use it will take an exception.
		DISABLE_FPU();
	}

	cpupage = cpupageptr[cpu];
	save_state = cpupage->state;
	cpupage->state = 1;
	interrupt_save = *__cpu_imask[cpu];
	*__cpu_imask[cpu] = get_sr() & SH_SR_IMASK;

	save_tls = cpupage->tls;
	cpupage->tls = &intr_tls;

#ifdef VARIANT_instr
	if(int_enter_enable_mask) {
		add_ktrace_int_enter(ilp);
	}
#endif

	prpp = &aspaces_prp[cpu];
	orig_prp = *prpp;
	for(isr = ilp->queue; isr != NULL; isr = isr->next) {
		thp = isr->thread;
#ifdef VARIANT_instr
		if(int_exit_enable_mask) {
			add_ktrace_int_handler_enter(ilp, isr);
		}
#endif
		if(isr->handler) {
			const struct sigevent	*ev;

			if(thp->aspace_prp && thp->aspace_prp != *prpp) {
				memmgr.aspace(thp->aspace_prp, prpp);
			}
			ev = isr->handler(isr->area, isr->id);
			if(ev != NULL) {
				intrevent_add(ev, AP_INTREVENT_FROM_IO(thp), isr);
			}
#ifdef VARIANT_instr
			if(int_exit_enable_mask) {
				add_ktrace_int_handler_exit(ilp, ev);
			}
#endif
		} else {
			(void) interrupt_mask(isr->level, isr);
			intrevent_add(isr->area, AP_INTREVENT_FROM_IO(thp), isr);
#ifdef VARIANT_instr
			if(int_exit_enable_mask) {
				add_ktrace_int_handler_exit(ilp, isr->area);
			}
#endif
		}
	}
#ifndef VARIANT_smp
	// If we're uni-core we don't need to switch back to the
	// user's address space as it will always be done in kernel.s
	if( get_inkernel() != 1) 
#endif
	{
		if (orig_prp && (orig_prp != *prpp)) {
			memmgr.aspace(orig_prp, prpp);
		}
	}
#ifdef VARIANT_instr
	if(int_exit_enable_mask) {
		add_ktrace_int_exit(ilp);
	}
#endif
	InterruptDisable();
	cpupage->tls = save_tls;
	cpupage->state = save_state;
	*__cpu_imask[cpu] = interrupt_save;

	// After we're done with the ISRs, check to see if anybody used the
	// FPU.  If they did, the FPU-disabled exception would have written the
	// FPU context to ker_savefpu and this would have caused a non-zero fpscr
	// value.
	if (fpu_save_area.fpscr!=0) {
		// If anybody did tweak the FPU, restore the saved state.
		restore_fpu_registers(&fpu_save_area);
		// Note that we disabled the FPU earlier.  If it had been enabled before this
		// interrupt, we'll re-enable it when we restore the SR along with all the
		// other registers from the saved context.
	}
	ker_savefpu[cpu] = old_ker_savefpu;
}

struct level_label {
	struct level_label	*combine;
	struct level_label	*cascade;
	int					primary_level;
	unsigned			off;
	unsigned			id_off;
	unsigned			done_off;
	int					secondary;
	unsigned			ipi_off;
};

static uint8_t		*intrentry_base;
static uintptr_t	intrentry_off;
static unsigned		intrentry_size;

static void
calculate_combines(struct level_label *labels) {
	unsigned			i;
	int					level;
	int					primary;
	struct level_label	*next;

	/*
	 * Figure out the combined interrupt links
	 * (things that share the same CPU interrupt or cascade vector).
	 */
	for(i = 0; i < intrinfo_num; ++i) {
		struct intrinfo_entry	*iip = &intrinfoptr[i];

		next = NULL;
		level = get_interrupt_level(NULL, iip->vector_base);
		primary = get_interrupt_level(NULL, iip->cascade_vector);
		labels[level].primary_level = primary;
		if(primary != -1)
			if(labels[primary].cascade == 0) labels[primary].cascade = &labels[level];
		if(!labels[level].secondary) {
			unsigned	j;

			for(j = intrinfo_num - 1; j > i; --j) {
				struct intrinfo_entry	*iip2 = &intrinfoptr[j];
				int		combine = 0;

				if(iip2->cascade_vector == iip->cascade_vector) {
					if(iip->cascade_vector != _NTO_INTR_SPARE) {
						combine = 1;
					} else if(iip2->cpu_intr_base == iip->cpu_intr_base) {
						combine = 1;
					}
				}

				if(combine) {
					unsigned	slevel;

					slevel = get_interrupt_level(NULL, iip2->vector_base);

					labels[slevel].secondary = 1;
					labels[slevel].combine = next;
					next = &labels[slevel];
				}
			} 
			labels[level].combine = next;
		}
	}
}

static void add_opcode(uint16_t opcode);

static void
add_code(const void *start, unsigned size, int align) {
	unsigned	new;

	/* Need to make sure they are on the same align */
	/* currently, assume align 1 is always OK; Only deal with align 2 */
	if(align == 2) {
		if(((uintptr_t)&intrentry_base[intrentry_off])&0x3) {
			//pad out with a NOP.
			//Won't infinitely recurse because opcode are align==1
			add_opcode(0x0009);
		}
	}

	new = intrentry_off + size;
	if(intrentry_base != NULL) {
		if(new <= intrentry_size) {
			memcpy(&intrentry_base[intrentry_off], start, size);
		} else {
			_sfree(intrentry_base, intrentry_size);
			intrentry_base = NULL;
		}
	}
	intrentry_off = new;
}

static void
add_opcode(uint16_t opcode) {
	add_code(&opcode, sizeof(opcode), 1);
}

static void
add_data(uint32_t data) {
	add_code(&data, sizeof(data), 2);
}

#define MK_MOV_PC_L(disp,rn)	(0xd000 + disp + (rn<<8))
#define MK_MOV_IMM(imm,rn)		(0xe000 + imm + (rn<<8))
static void
gen_load(unsigned reg, uintptr_t value) {
	if( (value <= 0x7f ) || (value >= 0xffffff80) ) {
		add_opcode(MK_MOV_IMM((value&0xff), reg));
	} else {
		if(((uintptr_t)&intrentry_base[intrentry_off]) & 0x3) {
			/* 2byte boundary */
			/* mov.l @(1,pc),reg */
			add_opcode(MK_MOV_PC_L(1, reg));
			/* bra pc+8 */
			add_opcode(0xa002);
			/* one nops */
			add_opcode(0x0009);
			/* .long value */
			add_data(value);
		} else {
			/* 4byte boundary */
			/* mov.l @(1,pc),reg */
			add_opcode(MK_MOV_PC_L(1, reg));
			/* nop */
			add_opcode(0x0009);
			/* bra pc+8 */
			add_opcode(0xa002);
			/* nop */
			add_opcode(0x0009);
			/* .long value */
			add_data(value);
		}
	}
}

static void
gen_burst_loads(unsigned genflags, unsigned level, int cascaded_eoi) {
	if(genflags & INTR_GENFLAG_LOAD_LEVEL) {
		gen_load(9, level);
	}
	if(genflags & INTR_GENFLAG_LOAD_SYSPAGE) {
		gen_load(8, (uintptr_t)_syspage_ptr);
	}
	if(genflags & INTR_GENFLAG_LOAD_INTRINFO) {
		gen_load(12, (uintptr_t)interrupt_level[level].info);
	}
	if(genflags & INTR_GENFLAG_LOAD_INTRMASK) {
		if(cascaded_eoi) {
			gen_load(14, 0);
		} else {
			/* mov	#mask_count, r0 */
			add_opcode(MK_MOV_IMM(offsetof(INTRLEVEL, mask_count), 0));
			/* mov.w @(r0,r10),r14 */
			add_opcode(0x0ead);
		}
	}
}

static void
gen_queue_load(unsigned level) {
	unsigned	level_base = interrupt_level[level].level_base;

	/*	mov r9,r0 */
	add_opcode(0x6093);
	/*  two shll2 r0 */
	/* the size of intr_level_entry is 16 */
	add_opcode(0x4008);
	add_opcode(0x4008);
	gen_load(4, (uintptr_t)&interrupt_level[level_base]);
	/*	add r0,r4 */
	add_opcode(0x340c);
	/* mov r4,r10 */
	add_opcode(0x6a43);
}

static void
gen_cond_branch(int value, int	off) {


	/* cmp immediate */
	/* mov.l value,r0 */
	/* cmp/eq r0,r9 */
	gen_load(0, value);
	add_opcode(0x3900);

	off = ((off - intrentry_off) - 4) - 4;
	/* bf the next (not equal) */
	if(off < 128) {
		/* gen_load is 2 bytes */
		/* bf 8 */
		add_opcode(0x8b02);	
	} else if(((uint32_t)&intrentry_base[intrentry_off]+2) & 0x3) {
		/* gen_load is 10 bytes */
		/* bf 16 */
		add_opcode(0x8b06);
		off = off - 8;
	} else {
		/* gen_load is 12 bytes */
		/* bf 18 */
		add_opcode(0x8b07);
		off = off - 10;
	}

	/* mov.l off,r0 */
	gen_load(0, off);
	/* braf r0 */
	add_opcode(0x0023);
	/* nop */
	add_opcode(0x0009);
}

static void
gen_transfer(int call, uintptr_t dest) {

	/* mov.l dest,r0 */
	gen_load(0, dest);

	if(call) {
		/* jsr @r0 */
		/* nop */
		add_opcode(0x400b);
		add_opcode(0x0009);
	} else {
		/* jmp @r0 */
		/* nop */
		add_opcode(0x402b);
		add_opcode(0x0009);
	}
}

#define POST_GENFLAGS	INTR_GENFLAG_LOAD_LEVEL

static void
gen_burst(struct __intrgen_data *burst, unsigned level, int cascaded_eoi) {
	gen_burst_loads(burst->genflags & ~POST_GENFLAGS, level, cascaded_eoi);
	/* Need to make sure they are on the same align */
	add_code(burst->rtn, burst->size, 2);
	gen_burst_loads(burst->genflags & POST_GENFLAGS, level, cascaded_eoi);
}

static void
gen_intr(struct level_label *labels, unsigned level, struct intrinfo_entry *iip) {
	unsigned			i;
	struct level_label	*combine;
	struct level_label	*llp;
	int					loop_check;
	unsigned			id_off;
	extern uint8_t		intr_entry_start[];
	extern uint8_t		intr_entry_end[];
	extern uint8_t		intr_process_queue[];
	extern uint8_t		intr_done_chk_fault[];
	extern uint8_t		intr_done[];
#ifdef	VARIANT_smp
	extern uint8_t		intr_process_ipi[];
	int					have_ipi = 0;
	uintptr_t			eoi_off;
#endif

	llp = &labels[level];
	if(!llp->secondary) {
		if(llp->primary_level < 0) {
			/*
			 * Make interrupts go to our generated code...
			 */
			set_trap(iip->cpu_intr_base, (void (*)())&intrentry_base[intrentry_off], 0x00);
			add_code(intr_entry_start, intr_entry_end - intr_entry_start, 2);
		} else {
			/* cascaded */
 			struct intrinfo_entry *iip2 = interrupt_level[llp->primary_level].info;

			if(!(iip2->flags & INTR_FLAG_CASCADE_IMPLICIT_EOI)) {
				gen_burst(&iip2->eoi, llp->primary_level, 1);
			}
		}
	}
	llp->id_off = intrentry_off;
	gen_burst(&iip->id, level, 0);
	for(i = 0; i < iip->num_vectors; ++i) {
		struct level_label	*cascade = llp[i].cascade;

		if(cascade != NULL) {
			gen_cond_branch(i, cascade->off);
		}
#ifdef	VARIANT_smp
		if((NUM_PROCESSORS > 1) && (interrupt_level[level+i].config & INTR_CONFIG_FLAG_IPI)) {
			gen_cond_branch(i, llp->ipi_off);
			have_ipi = 1;
		}
#endif
	}
	combine = llp->combine;
	if(combine != NULL) {
		gen_cond_branch(-1, combine->off);
	} else if(!(iip->id.genflags & INTR_GENFLAG_LOAD_LEVEL)) {
		gen_cond_branch(-1, llp->done_off);
	}
	gen_queue_load(level);
	gen_transfer(1, (uintptr_t)intr_process_queue);
#ifdef	VARIANT_smp
	eoi_off = intrentry_off;
#endif
	gen_burst(&iip->eoi, level, 0);
	id_off = ~0;
	loop_check = level;
	do {
		if(interrupt_level[loop_check].info->eoi.genflags & INTR_GENFLAG_ID_LOOP) {
			id_off = labels[loop_check].id_off;
		}
		loop_check = labels[loop_check].primary_level;
	} while(loop_check >= 0);
	if(id_off != ~0U) {
		gen_transfer(0, (uintptr_t)intrentry_base + id_off);
	}
	llp->done_off = intrentry_off;
	if(iip->flags & INTR_FLAG_CPU_FAULT) {
		gen_transfer(0, (uintptr_t)intr_done_chk_fault);
	} else {
		gen_transfer(0, (uintptr_t)intr_done);
	}
#ifdef	VARIANT_smp
	if(have_ipi) {
		llp->ipi_off = intrentry_off;
		gen_queue_load(level);
		gen_transfer(1, (uintptr_t)intr_process_ipi);
		gen_transfer(0, (uintptr_t)intrentry_base + eoi_off);
	}
#endif
}

void
cpu_interrupt_init(unsigned num_levels) {
	struct intrinfo_entry	*iip;
	unsigned				i;
	struct level_label		*labels;
	unsigned				lsize;

	lsize = num_levels * sizeof(*labels);
	labels = _scalloc(lsize);

	calculate_combines(labels);

	for(;;) {
		for(i = 0, iip = intrinfoptr; i < intrinfo_num; ++i, ++iip) {
			unsigned level = get_interrupt_level(NULL, iip->vector_base);

			if(labels[level].off != intrentry_off) {
				/* phase error ... generate everything again */
				labels[level].off = intrentry_off;
				if(intrentry_base != NULL) {
					_sfree(intrentry_base, intrentry_size);
					intrentry_base = NULL;
				}
			}
			gen_intr(labels, level, iip);
		}
		if(intrentry_base != NULL) break;
		intrentry_size = intrentry_off;
		intrentry_base = _smalloc(intrentry_size);
		intrentry_off = 0;
	}
	for(i = 0; i < intrentry_size; i += 4) {
		icache_flush((uintptr_t)&intrentry_base[i]);
	}
	_sfree(labels, lsize);
}

__SRCVERSION("interrupt.c $Rev: 199396 $");
