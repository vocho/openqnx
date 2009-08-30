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


/*
 * Load up short data registers with the value saved by InterruptAttach and
 * invoke routine.
 */
#define intr_invoke( handler, area, id, s1, s2 ) 	\
			({								\
				const struct sigevent	*ev;	\
												\
				asm volatile (					\
				"	mtlr %1		;"				\
				"	mr	%%r13,%4;"				\
				"	mr	%%r2,%5 ;"				\
				"	mr	%%r3,%2 ;"				\
				"	mr	%%r4,%3 ;"				\
				"	blrl		;"				\
				"	lis %%r13,_SDA_BASE_@ha;"	\
				"	addi %%r13,%%r13,_SDA_BASE_@l;"\
				"	lis %%r2,_SDA2_BASE_@ha;"	\
				"	addi %%r2,%%r2,_SDA2_BASE_@l;"\
				"	mr %0,%%r3;"				\
				: "=r" (ev)						\
				: "r" (handler), "r" (area), "r" (id), "r" (s1), "r" (s2) \
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
	void					*save_tls;
	struct cpupage_entry	*cpupage;
	int						cpu = RUNCPU;

#ifdef DOING_INTERRUPT_PROBLEM_CHECKING
	if((ilp->mask_count > 0) || ((inkernel & INKERNEL_INTRMASK) > 20)) {
		DebugKDBreak();
	}
#endif
	cpupage = cpupageptr[cpu];
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
			const struct sigevent	*evlocal;

			if(thp->aspace_prp && thp->aspace_prp != *prpp) {
				memmgr.aspace(thp->aspace_prp, prpp);
			}
			evlocal = intr_invoke(isr->handler, isr->area, isr->id, isr->cpu.gpr13, isr->cpu.gpr2);
			if(evlocal != NULL) {
				intrevent_add(evlocal, AP_INTREVENT_FROM_IO(thp), isr);
			}
#ifdef VARIANT_instr
			if(int_exit_enable_mask) {
				add_ktrace_int_handler_exit(ilp, evlocal);
			}
#endif
		} else {
			(void)interrupt_mask(isr->level, isr);
			intrevent_add(isr->area, AP_INTREVENT_FROM_IO(thp), isr);
#ifdef VARIANT_instr
			if(int_exit_enable_mask) {
				add_ktrace_int_handler_exit(ilp, isr->area);
			}
#endif
		}
	} 
	// no need to restore aspace if from user space as ker_exit will do it
#if defined(VARIANT_smp)
	{
//hzhou originally had the following 'if', but _NTO_ITF_MSG_DELIVERY is
//only defined if SMP_MSGOPT is enabled, which isn't true for the PPC.
//
//I'm not sure it's a valid check anyway, since on the X86 there were
//paths back through intr_done in kernel.S that didn't properly reset the
//aspace, even if we were only handling a simple interrupt (in SMP). Can't
//remember what they were off-hand, but I remember fixing a bug dealing
//with that. bstecher
//	if((get_inkernel() != 1) || (actives[RUNCPU]->internal_flags & _NTO_ITF_MSG_DELIVERY)) {
#else
	if(get_inkernel() != 1) {
#endif
		if(orig_prp && (orig_prp != *prpp)) {
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
	__cpu_membarrier();
}

struct level_label {
	struct level_label	*combine;
	struct level_label	*cascade;
	int					primary_level;
	unsigned			off;
	unsigned			id_off;
	unsigned			done_off;
	unsigned			ipi_off;
	int					secondary;
	uintptr_t			chain;
	void				*intrinfo;
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
	if(value < 0x8000) {
		add_opcode(MK_OPCODE(14, reg, 0, value));	/* addi reg,0,val */
	} else {
		add_opcode(MK_OPCODE(15, reg, 0, value >> 16));	/* addis reg,0,val@h */
		add_opcode(MK_OPCODE(24, reg, reg, value & 0xffff)); /* ori reg,reg,val@l */
	}
}

static void
gen_burst_loads(unsigned genflags, unsigned level, int cascaded_eoi) {
	if(genflags & INTR_GENFLAG_LOAD_INTRMASK) {
		// Have to load this first, since a LOAD_CPUNUM will trash R15
		if(cascaded_eoi) {
			gen_load(18, 0);
		} else {
			/* lhz %r18, mask_count(%r15) */
			add_opcode(MK_OPCODE((unsigned)40, 18, 15, offsetof(INTRLEVEL, mask_count)));
		}
	}
	if(genflags & INTR_GENFLAG_LOAD_LEVEL) {
		gen_load(14, level);
	}
	if(genflags & INTR_GENFLAG_LOAD_CPUNUM) {
		if(NUM_PROCESSORS == 1) {
			gen_load(15, 0);
		} else {
//			add_opcode(0x7dfffaa6); // mfspr %r15,PPC700_SPR_PIR
			add_opcode(0x7df342a6); // mfsprg %r15,3
		}
	}
	if(genflags & INTR_GENFLAG_LOAD_SYSPAGE) {
		gen_load(16, (uintptr_t)_syspage_ptr);
	}
	if(genflags & INTR_GENFLAG_LOAD_INTRINFO) {
		gen_load(17, (uintptr_t)interrupt_level[level].info);
	}
}

static void
gen_queue_load(unsigned level) {
	struct interrupt_level	*llp = &interrupt_level[level];
	struct intrinfo_entry	*iip = llp->info;
	uintptr_t				level_base;

	level_base = (uintptr_t)&interrupt_level[llp->level_base];

	if((iip->num_vectors == 1) && (iip->id.genflags & INTR_GENFLAG_NOGLITCH)) {
		gen_load(15, level_base);
	} else {
		add_opcode(0x55c02036); /* slwi %r0,%r14,4 */
		gen_load(15, level_base);
		add_opcode(0x7def0214); /* add %r15,%r15,%r0 */
	}
}

static void
gen_cond_branch(int value, unsigned off) {
	uint32_t	opcode[2];

	/* cmp immediate */
	opcode[0] = MK_OPCODE(11,0,14,value & 0xffff);
	off = ((off - intrentry_off) - 4) & 0xfffc;
	/* branch conditional */
	opcode[1] = 0x41820000UL | off;
	add_code(opcode, sizeof(opcode));
}

static void
gen_transfer(int call, uintptr_t dest) {
	uint32_t	opcode;
	uint32_t	sign;

	opcode = dest;
	if(call) opcode |= 0x01;
	sign = dest & 0xfe000000;
	if((sign == 0) || (sign == 0xfe000000)) {
		/* can branch absolute */
		opcode |= 0x02;
	} else {
		opcode -= (uint32_t)&intrentry_base[intrentry_off];
		sign = opcode & 0xfe000000;
		if((sign != 0) && (sign != 0xfe000000)) {
			// Can't reach target with a relative branch:
			// 	lis		%r11,dest >> 16
			//	ori		%r11,dest & 0xffff
			//	mtlr	%r11
			//	blr[l]

			gen_load(11, dest);
			add_opcode(0x7d6803a6); 
			add_opcode(0x4e800020 | (opcode & 0x1));
			return;
		}
	}
	opcode &= ~0xfc000000;
	opcode |= 0x48000000;
	add_opcode(opcode);
}

#define POST_GENFLAGS	INTR_GENFLAG_LOAD_LEVEL

static void
gen_burst(struct __intrgen_data *burst, unsigned level, int cascaded_eoi) {
	void		*start;
	unsigned	size;

	gen_burst_loads(burst->genflags & ~POST_GENFLAGS, level, cascaded_eoi);
	start = burst->rtn;
	size = burst->size;
//KLUDGE: backwards compatability.
	if(burst->genflags & INTR_GENFLAG_LOAD_CPUNUM) {
		if(*(uint32_t *)start == 0x7dfffaa6) {
			// The first instruction is "mfspr %15,1023". Don't include
			// it in the generated code, since the CPUNUM has already
			// been loaded by the INTR_GENFLAG_LOAD_CPUNUM.
			// Eventually we can get rid of this, once all the
			// kernels and startups have been updated.
			start = (uint8_t *)start + sizeof(uint32_t);
			size -= sizeof(uint32_t);
		}
	}
//END KLUDGE
	add_code(start, size);
	gen_burst_loads(burst->genflags & POST_GENFLAGS, level, cascaded_eoi);
}

static void
gen_intr(struct level_label *labels, unsigned level, struct intrinfo_entry *iip) {
	unsigned			i;
	struct level_label	*combine;
	struct level_label	*llp;
	int					loop_check;
	uintptr_t			eoi_off;
	uintptr_t			id_off;
	uintptr_t			chain;
	extern uint8_t		intr_process_queue[];
	extern uint8_t		intr_transfer_exc[];
	extern uint8_t		intr_done_chk_fault[];
	extern uint8_t		intr_done[];
#if defined(VARIANT_smp)
	extern uint8_t		intr_process_ipi[];
	int					have_ipi = 0;
#endif

	chain = 0;
	llp = &labels[level];
	if(!llp->secondary) {
		if(llp->primary_level < 0) {
			/*
			 * Make interrupts go to our generated code...
			 */
			chain = trap_chain_addr(iip->cpu_intr_base);
			if(chain != 0) llp->chain = chain;
			exc_intr_install(iip, &intrentry_base[intrentry_off], &llp->intrinfo);
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
	} else if(!(iip->id.genflags & (INTR_GENFLAG_LOAD_LEVEL|INTR_GENFLAG_NOGLITCH))) {
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
	if(id_off != ~0U) {
		gen_transfer(0, (uintptr_t)intrentry_base + id_off);
	}
	if(llp->chain == 0) {
		llp->done_off = intrentry_off;
	}
	if(iip->flags & INTR_FLAG_CPU_FAULT) {
		gen_transfer(0, (uintptr_t)intr_done_chk_fault);
	} else {
		gen_transfer(0, (uintptr_t)intr_done);
	}
	if(llp->chain != 0) {
		llp->done_off = intrentry_off;
		gen_transfer(1, (uintptr_t)intr_transfer_exc);
		gen_transfer(0, llp->chain);
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
	for(i = 0; i < intrentry_size; i += 4) {
		icache_flush((uintptr_t)&intrentry_base[i]);
	}
	_sfree(labels, lsize);
}

__SRCVERSION("interrupt.c $Rev: 199396 $");
