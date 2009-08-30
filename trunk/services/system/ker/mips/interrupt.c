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
 * interrupt.c
 *	C level dispatch of interrupt delivery
 *
 */
#include "externs.h"
#include <mips/opcode.h>

extern void 	*r4k_exception_table[];

/*
 * Load up GP register with the value saved by InterruptAttach and
 * invoke routine. Restore the kernel's GP on exit.
 * Additionally, we load t9 ($25) with the handler's address, in case it
 * was compiled PIC...
 */
// static inline const struct sigevent *
// intr_invoke( const struct sigevent *(*handler)(void *), void *area, uint32_t gp ) {
#define intr_invoke( handler, area, id, gp ) 	\
			({									\
				const struct sigevent	*ev;	\
												\
				asm volatile (					\
				"	.set noreorder;"			\
				"	move $28,%4 ;"				\
				"	move $25,%1 ;"				\
				"	move $4,%2	;"				\
				"	jalr %1		;"				\
				"	 move $5,%3	;"				\
				"	move %0,$2	;"				\
				"	la $28,_gp	;"				\
				"	.set reorder;"				\
				: "=r" (ev)						\
				: "r" (handler), "r" (area), "r" (id), "r" ((uint32_t)gp) \
				: "memory"						\
				);								\
				ev;								\
			})


void
interrupt(INTRLEVEL *ilp) {
	INTERRUPT				*isr;
	THREAD					*thp;
	PROCESS					*orig_prp;
	PROCESS					**prpp;
	int						save_state;
	struct cpupage_entry	*cpupage;
	void					*save_tls;

#ifdef DOING_INTERRUPT_PROBLEM_CHECKING
	if((ilp->mask_count > 0)
	 ||((get_inkernel() & INKERNEL_INTRMASK) > 20)
	 ||((get_inkernel() & INKERNEL_INTRMASK) == 0)) {
		DebugKDBreak();
	}
#endif
	cpupage = cpupageptr[RUNCPU];
	save_state = cpupage->state;
	cpupage->state = 1;
	save_tls = cpupage->tls;
	cpupage->tls = &intr_tls;
	__cpu_membarrier();

#ifdef VARIANT_instr
	if(int_enter_enable_mask) {
		add_ktrace_int_enter(ilp);
	}
#endif
	InterruptEnable();

	if ( interrupt_hook ) {
		interrupt_hook(ilp);
	}

	prpp = &aspaces_prp[RUNCPU];
	orig_prp = *prpp;
	for(isr = ilp->queue; isr != NULL; isr = isr->next) {
		thp = isr->thread;
#ifdef VARIANT_instr
		if(int_exit_enable_mask) {
			add_ktrace_int_handler_enter(ilp, isr);
		}
#endif
		if(isr->handler) {
			const struct sigevent	*evlocal;

			if(thp->aspace_prp && thp->aspace_prp != *prpp) {
				memmgr.aspace(thp->aspace_prp, prpp);
			}
			evlocal = intr_invoke(isr->handler, isr->area, isr->id, isr->cpu.gp);
			if(evlocal != NULL) {
				intrevent_add(evlocal, AP_INTREVENT_FROM_IO(thp), isr);
			}
#ifdef VARIANT_instr
			if(int_exit_enable_mask) {
				add_ktrace_int_handler_exit(ilp, evlocal);
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
	if(orig_prp && (orig_prp != *prpp)) {
		memmgr.aspace(orig_prp, prpp);
	}
#ifdef VARIANT_instr
	if(int_exit_enable_mask) {
		add_ktrace_int_exit(ilp);
	}
#endif
	InterruptDisable();
	cpupage->tls = save_tls;
	cpupage->state = save_state;
	__cpu_membarrier();
}

struct level_label {
	struct level_label	*combine;
	struct level_label	*cascade;
	int					primary_level;
	unsigned			off;
	unsigned			id_off;
	unsigned			ipi_off;
	unsigned			done_off;
	int					secondary;
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
		if(primary != -1) labels[primary].cascade = &labels[level];
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

static void
add_code(const void *start, unsigned size) {
	unsigned	new;

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
add_opcode(uint32_t opcode) {
	add_code(&opcode, sizeof(opcode));
}

#define MK_OPCODE(opcode, r1, r2, immed) \
		(((opcode)<<26)+((r1)<<21)+((r2)<<16)+(immed))
	
static void
gen_load(unsigned reg, uintptr_t value) {
	unsigned upper;
	unsigned lower;

	lower = value & 0xffff;
	upper = value >> 16;
	if(upper == 0) {
		add_opcode(MK_OPCODE(OPCODE_ORI, 0, reg, lower));
	} else {
		add_opcode(MK_OPCODE(OPCODE_LUI, 0, reg, upper));
		if(lower != 0) {
			add_opcode(MK_OPCODE(OPCODE_ORI, reg, reg, lower));
		}
	}
}

static void
gen_burst_loads(unsigned genflags, unsigned level, int cascade_eoi) {
	if(genflags & INTR_GENFLAG_LOAD_LEVEL) {
		gen_load(MIPS_REG_S0, level);
	}
	if(genflags & INTR_GENFLAG_LOAD_SYSPAGE) {
		gen_load(MIPS_REG_S2, (uintptr_t)_syspage_ptr);
	}
	if(genflags & INTR_GENFLAG_LOAD_INTRINFO) {
		gen_load(MIPS_REG_S3, (uintptr_t)interrupt_level[level].info);
	}
	if(genflags & INTR_GENFLAG_LOAD_INTRMASK) {
		if(cascade_eoi) {
			gen_load(MIPS_REG_S4, 0);
		} else {
			add_opcode(MK_OPCODE(OPCODE_LHU, MIPS_REG_S1, MIPS_REG_S4, offsetof(INTRLEVEL, mask_count)));
		}
	}
	if(genflags & INTR_GENFLAG_LOAD_CPUNUM) {
		if(NUM_PROCESSORS == 1) {
			gen_load(MIPS_REG_S5, 0);
		} else {
			add_opcode(0x4035a000); // dmfc0 s5,CP0_XCONTEXT
			add_opcode(0x00000000);	// nop
			add_opcode(0x0015ae7e);	// dshrl s5,64-7
		}
	}
}

static void
gen_queue_load(unsigned level) {
	unsigned	level_base = interrupt_level[level].level_base;

	/* SLL instruction */
	add_opcode((LOG2_SIZEOF_INTRLEVEL << 6)
			+ (MIPS_REG_AT << 11)
			+ (MIPS_REG_S0 << 16));
	gen_load(MIPS_REG_S1, (uintptr_t)&interrupt_level[level_base]);
	/* ADD instruction */
	add_opcode((0x20)
			+ (MIPS_REG_S1 << 11)
			+ (MIPS_REG_S1 << 16)
			+ (MIPS_REG_AT << 21));
}

static void
gen_cond_branch(int value, unsigned off) {
	uint32_t	opcode[3];

	opcode[2] = 0; /* branch delay nop */
	if(value < 0) {
		off = (((off - intrentry_off) - 4) >> 2) & 0xffff;
		opcode[1] = MK_OPCODE(OPCODE_REGIMM,MIPS_REG_S0,OPCODE_BLTZ, off);
		add_code(&opcode[1], sizeof(opcode[1])*2);
	} else {
		opcode[0] = MK_OPCODE(OPCODE_ORI, MIPS_REG_ZERO, MIPS_REG_AT, value);
		off = (((off - intrentry_off) - 8) >> 2) & 0xffff;
		opcode[1] = MK_OPCODE(OPCODE_BEQ,MIPS_REG_S0,MIPS_REG_AT, off);
		add_code(opcode, sizeof(opcode));
	}
}

static void
gen_transfer(int call, uintptr_t dest) {
	uint32_t	opcode[2];

	dest >>= 2;
	dest &= 0x03ffffff;

	opcode[0] = call ? MK_OPCODE(OPCODE_JAL,0,0,dest) : MK_OPCODE(OPCODE_J,0,0,dest);
	opcode[1] = 0; /* branch delay NOP */
	add_code(opcode, sizeof(opcode));
}

#define POST_GENFLAGS	INTR_GENFLAG_LOAD_LEVEL

static void
gen_burst(struct __intrgen_data *burst, unsigned level, int cascade_eoi) {
	gen_burst_loads(burst->genflags & ~POST_GENFLAGS, level, cascade_eoi);
	add_code(burst->rtn, burst->size);
	gen_burst_loads(burst->genflags & POST_GENFLAGS, level, cascade_eoi);
}

static void
gen_intr(struct level_label *labels, unsigned level, struct intrinfo_entry *iip) {
	unsigned			i;
	struct level_label	*combine;
	struct level_label	*llp;
	unsigned			eoi_off;
	int					loop_check;
	unsigned			id_off;
	extern uint8_t		intr_entry_start[];
	extern uint8_t		intr_entry_end[];
	extern uint8_t		intr_process_queue[];
	extern uint8_t		intr_done[];
	extern uint8_t		intr_done_chk_fault[];
#if defined(VARIANT_smp)
	extern uint8_t		intr_process_ipi[];
	int					have_ipi = 0;
#endif

	llp = &labels[level];
	if(!llp->secondary) {
		if(llp->primary_level < 0) {
			if(intrentry_base != NULL) {
				/*
				 * Make interrupts go to our generated code...
				 *
				 * We could go through the general exception handler,
				 * or the optimized interrupt handling point depending
				 * on which chip we're running on. Set both places and
				 * let the hardware sort it out :-).
				 */
				uint32_t		*intr_code;
				void			*start;
				
			   	start = &intrentry_base[intrentry_off];

				r4k_exception_table[iip->cpu_intr_base] = start;
				intr_code = (void *)MIPS_EXCV_INTERRUPT;
				intr_code[0] |= ((uintptr_t)start) >> 16;
				intr_code[1] |= ((uintptr_t)start) & 0xffff;
			 }
			 add_code(intr_entry_start, intr_entry_end - intr_entry_start);
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
#if defined(VARIANT_smp)
		if(  (NUM_PROCESSORS > 1)
		  && (interrupt_level[level+i].config & INTR_CONFIG_FLAG_IPI)) {
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
	eoi_off = intrentry_off;
	gen_burst(&iip->eoi, level, 0);
	id_off = ~0;
	loop_check = level;
	do {
		if(interrupt_level[loop_check].info->eoi.genflags & INTR_GENFLAG_ID_LOOP) {
			id_off = labels[loop_check].id_off;
		}
		loop_check = labels[loop_check].primary_level;
	} while(loop_check >= 0);
	if(id_off != ~0) {
		gen_transfer(0, (uintptr_t)intrentry_base + id_off);
	}
	llp->done_off = intrentry_off;
	if(iip->flags & INTR_FLAG_CPU_FAULT) {
		gen_transfer(0, (uintptr_t)intr_done_chk_fault);
	} else {
		gen_transfer(0, (uintptr_t)intr_done);
	}
#if defined(VARIANT_smp)
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
	CacheControl(intrentry_base, intrentry_off, MS_INVALIDATE_ICACHE);
	CacheControl((void *)MIPS_EXCV_INTERRUPT, 3*sizeof(uint32_t), MS_INVALIDATE_ICACHE);
	_sfree(labels, lsize);
}

__SRCVERSION("interrupt.c $Rev: 154853 $");
