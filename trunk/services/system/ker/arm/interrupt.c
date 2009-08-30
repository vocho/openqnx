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
#include "arm/mmu.h"

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

/*
 * The generated code uses the following calling conventions:
 *
 *	r4 - holds the interrupt level	(ARM_REG_LEVEL)
 *	r5 - holds the syspageptr		(ARM_REG_SYSPAGE)
 *	r6 - holds the intrinfo_entry	(ARM_REG_INTRINFO)
 *	r7 - holds the mask count		(ARM_REG_INTRMASK)
 *	r8 - holds the intr_level		(ARM_REG_INTRLEVEL)
 *
 * r10 - set to non-zero by exception entry code if we interrupted kernel mode
 */

#define	ARM_REG_LEVEL		4
#define	ARM_REG_SYSPAGE		5
#define	ARM_REG_INTRINFO	6
#define	ARM_REG_INTRMASK	7
#define	ARM_REG_INTRLEVEL	8


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
add_opcode(uint32_t opcode)
{
	add_code(&opcode, sizeof(opcode));
}

static void
gen_load(unsigned reg, int value)
{
	int			imm;
	uint32_t	op;

	/*
	 * Synthesize 32-bit value from 4 8-bit parts:
	 *
	 *	mov	reg, #(value & 0xff)
	 *	orr	reg, reg, #((value >> 8)  & 0xff)
	 *	orr	reg, reg, #((value >> 16) & 0xff)
	 *	orr	reg, reg, #((value >> 24) & 0xff)
	 */
	add_opcode(0xe3a00000 | (reg << 12) | (value & 0xff));

	op = 0xe3800000 | (reg << 12) | (reg << 16);
	if ((imm = (value >> 8) & 0xff) != 0)
			add_opcode(op | imm | 0xc00);
	if ((imm = (value >> 16) & 0xff) != 0)
			add_opcode(op | imm | 0x800);
	if ((imm = (value >> 24) & 0xff) != 0)
			add_opcode(op | imm | 0x400);
}

static void
gen_burst_loads(unsigned genflags, unsigned level, int cascaded_eoi)
{
	if (genflags & INTR_GENFLAG_LOAD_LEVEL) {
		gen_load(ARM_REG_LEVEL, level);
	}
	if (genflags & INTR_GENFLAG_LOAD_SYSPAGE) {
		gen_load(ARM_REG_SYSPAGE, (uintptr_t)_syspage_ptr);
	}
	if (genflags & INTR_GENFLAG_LOAD_INTRINFO) {
		gen_load(ARM_REG_INTRINFO, (uintptr_t)interrupt_level[level].info);
	}
	if (genflags & INTR_GENFLAG_LOAD_INTRMASK) {
		if (cascaded_eoi) {
			gen_load(ARM_REG_INTRMASK, 0);
		}
		else {
			uint8_t off = offsetof(INTRLEVEL, mask_count);

			/*
			 * ldrh	r7, [r8, #off]
			 *
			 * WARNING: ldrh is not implemented on ARM v3 processors
			 */
			add_opcode(0xe1d870b0 | ((off & 0xf0) << 4) | (off & 0xf));
		}
	}
#ifdef	FIXME_SMP
	/*
	 * FIXME: need to handle genflags & INTR_GENFLAG_LOAD_CPUNUM?
	 */
#endif
}

static void
gen_queue_load(unsigned level)
{
	unsigned	level_base = interrupt_level[level].level_base;

	/*
	 * set r8 to the INTRLEVEL for the base interrupt for this controller
	 * add r8, r8, r4, lsl #4
	 */
	gen_load(ARM_REG_INTRLEVEL, (uintptr_t)&interrupt_level[level_base]);
	add_opcode(0xe0888204);
}

static void
gen_cond_branch(int value, unsigned off)
{
	/*
	 * On entry, ARM_REG_LEVEL (r4) contains the level returned by id callout
 	 *
 	 */
 	if (value == -1) {
 		/*
 		 * Test if r4 < 0 (cheaper than testing if == -1)
 		 *
 		 * cmp r4, #0
 		 * bmi off
 		 */
		add_opcode(0xe3540000);
		/*
		 * Convert off into pc-relative branch offset
		 */
		off = (((off - intrentry_off) - 8) >> 2) & 0x00ffffff;
		add_opcode(0x4a000000 | off);
 	}
 	else {
 		/*
 		 * Test if r4 == value
 		 *
		 * WARNING: we assume valid interrupt levels are 0-255 to allow us
		 *          to use a single instruction with immediate value.
		 *
		 *	cmp	r4, #(value & 0xff)
		 *	beq	off
		 */
		add_opcode(0xe3540000 | (value & 0xff));
		/*
		 * Convert off into pc-relative branch offset
		 */
		off = (((off - intrentry_off) - 8) >> 2) & 0x00ffffff;
		add_opcode(0x0a000000 | off);
	}
}

static void
gen_transfer(int call, uintptr_t dest)
{
	/*
	 * Calculate distance to dest
	 */
	int off = ((dest - (uintptr_t)intrentry_base) - intrentry_off) - 8;

	if (off < 0x02000000 && off >= -0x02000000) {
		/*
		 * Can perform relative branch/link
		 */
		add_opcode(0xea000000 | ((off >> 2) & 0xffffff) | (call << 24));
	}
	else {
		/*
		 * Need to perform jump/link to absolute location:
		 *
		 *		adr	lr, 1f
		 *		ldr	pc, 0f
		 *	0:	.word	value
		 *	1:
		 */
		if (call)
			add_opcode(0xe28fe004);
		add_opcode(0xe51ff004);
		add_opcode(dest);
	}
}

void
interrupt(INTRLEVEL *ilp)
{
	INTERRUPT				*isr;
	THREAD					*thp;
	PROCESS					*orig_prp;
	PROCESS					**prpp;
	int						save_state;
	void					*save_tls;
	struct cpupage_entry	*cpupage;
	int						cpu = RUNCPU;

	cpupage = cpupageptr[cpu];
	save_state = cpupage->state;
	cpupage->state = 1;

	save_tls = cpupage->tls;
	cpupage->tls = &intr_tls;

#ifdef VARIANT_instr
	if(int_enter_enable_mask) {
		add_ktrace_int_enter(ilp);
	}
#endif
	InterruptEnable();

	prpp = &aspaces_prp[cpu];
	orig_prp = *prpp;
	for (isr = ilp->queue; isr != NULL; isr = isr->next) {
		thp = isr->thread;
#ifdef VARIANT_instr
		if(int_exit_enable_mask) {
			add_ktrace_int_handler_enter(ilp, isr);
		}
#endif
		if (isr->handler) {
			const struct sigevent	*ev;

			if (thp->aspace_prp && thp->aspace_prp != *prpp) {
				memmgr.aspace(thp->aspace_prp, prpp);
			}

			if ((ev = isr->handler(isr->area, isr->id)) != NULL) {
				intrevent_add(ev, AP_INTREVENT_FROM_IO(thp), isr);
			}
#ifdef VARIANT_instr
			if(int_exit_enable_mask) {
				add_ktrace_int_handler_exit(ilp, ev);
			}
#endif
		}
		else {
			(void) interrupt_mask(isr->level, isr);
			intrevent_add(isr->area, AP_INTREVENT_FROM_IO(thp), isr);
#ifdef VARIANT_instr
			if(int_exit_enable_mask) {
				add_ktrace_int_handler_exit(ilp, isr->area);
			}
#endif
		}
	} 
	if (orig_prp && (orig_prp != *prpp)) {
		memmgr.aspace(orig_prp, prpp);
	}
	else if (orig_prp == 0) {
		 *prpp = orig_prp; 
	}
#ifdef VARIANT_instr
	if(int_exit_enable_mask) {
		add_ktrace_int_exit(ilp);
	}
#endif
	InterruptDisable();
	cpupage->state = save_state;
	cpupage->tls = save_tls;
}

#define POST_GENFLAGS	INTR_GENFLAG_LOAD_LEVEL

static void
gen_burst(struct __intrgen_data *burst, unsigned level, int cascaded_eoi)
{
	gen_burst_loads(burst->genflags & ~POST_GENFLAGS, level, cascaded_eoi);
	add_code(burst->rtn, burst->size);
	gen_burst_loads(burst->genflags & POST_GENFLAGS, level, cascaded_eoi);
}

static void
gen_intr(struct level_label *labels, unsigned level, struct intrinfo_entry *iip)
{
	unsigned			i;
	struct level_label	*combine;
	struct level_label	*llp;
	int					loop_check;
	unsigned			id_off;
	extern uint8_t		intr_entry_start[];
	extern uint8_t		intr_entry_end[];
	extern uint8_t		intr_process_queue[];
	extern uint8_t		intr_done[];
#if defined(VARIANT_smp)
	extern uint8_t		intr_process_ipi[];
	int					have_ipi = 0;
	uintptr_t			eoi_off;
#endif

	llp = &labels[level];
	
	if (!llp->secondary) {
		if (llp->primary_level < 0) {
			if (intrentry_base != NULL) {
				/*
				 * Plumb the entry point into the IRQ jump vector
				 */
				unsigned	vecp = 0x00000038;

				if (arm_mmu_getcr() & ARM_MMU_CR_X) {
					/*
					 * Vectors are remapped to 0xffff0000
					 */
					vecp += 0xffff0000;
				}
				*(unsigned *)vecp = (unsigned)&intrentry_base[intrentry_off];
			}
			add_code(intr_entry_start, intr_entry_end - intr_entry_start);
		}
		else {
			/*
			 * cascaded
			 */
 			struct intrinfo_entry *iip2 = interrupt_level[llp->primary_level].info;

			if (!(iip2->flags & INTR_FLAG_CASCADE_IMPLICIT_EOI)) {
				gen_burst(&iip2->eoi, llp->primary_level, 1);
			}
		}
	}
	llp->id_off = intrentry_off;
	gen_burst(&iip->id, level, 0);
	for (i = 0; i < iip->num_vectors; ++i) {
		struct level_label	*cascade = llp[i].cascade;

		if (cascade != NULL) {
			gen_cond_branch(i, cascade->off);
		}
#if defined(VARIANT_smp)
		if ((NUM_PROCESSORS > 1) && (interrupt_level[level+i].config & INTR_CONFIG_FLAG_IPI)) {
			gen_cond_branch(i, llp->ipi_off);
			have_ipi = 1;
		}
#endif
	}
	combine = llp->combine;
	if (combine != NULL) {
		gen_cond_branch(-1, combine->off);
	}
	else if (!(iip->id.genflags & INTR_GENFLAG_LOAD_LEVEL)) {
		gen_cond_branch(-1, llp->done_off);
	}
	gen_queue_load(level);
	gen_transfer(1, (uintptr_t)intr_process_queue);
#if defined(VARIANT_smp)
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
	if(id_off != ~0) {
		gen_transfer(0, (uintptr_t)intrentry_base + id_off);
	}
	llp->done_off = intrentry_off;
	if (iip->flags & INTR_FLAG_CPU_FAULT) {
#ifdef	FIXME
		gen_transfer(0, (uintptr_t)intr_done_chk_fault);
#else
		crash();
#endif
	}
	else {
		gen_transfer(0, (uintptr_t)intr_done);
	}
#ifdef	VARIANT_smp
	if (have_ipi) {
		llp->ipi_off = intrentry_off;
		gen_queue_load(level);
		gen_transfer(1, (uintptr_t)intr_process_ipi);
		gen_transfer(0, (uintptr_t)intrentry_base + eoi_off);
	}
#endif
}

/*
 * -------------------------------------------------------------------------
 * cpu independent routines
 *
 * For the moment, each port provides its own copy of the following code.
 * Later, this should be moved to a cpu-independent place.
 * -------------------------------------------------------------------------
 */

static void
calculate_combines(struct level_label *labels)
{
	unsigned			i;
	int					level;
	int					primary;
	struct level_label	*next;

	/*
	 * Figure out the combined interrupt links
	 * (things that share the same CPU interrupt or cascade vector).
	 */
	for (i = 0; i < intrinfo_num; ++i) {
		struct intrinfo_entry	*iip = &intrinfoptr[i];

		next = NULL;
		level = get_interrupt_level(NULL, iip->vector_base);

		primary = get_interrupt_level(NULL, iip->cascade_vector);
		labels[level].primary_level = primary;
		if (primary != -1)
			labels[primary].cascade = &labels[level];

		if (!labels[level].secondary) {
			unsigned	j;

			for (j = intrinfo_num - 1; j > i; --j) {
				struct intrinfo_entry	*iip2 = &intrinfoptr[j];
				int		combine = 0;

				if (iip2->cascade_vector == iip->cascade_vector) {
					if(iip->cascade_vector != _NTO_INTR_SPARE) {
						combine = 1;
					} else if(iip2->cpu_intr_base == iip->cpu_intr_base) {
						combine = 1;
					}
				}

				if (combine) {
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

void
cpu_interrupt_init(unsigned num_levels)
{
	struct intrinfo_entry	*iip;
	unsigned				i;
	struct level_label		*labels;
	unsigned				lsize;

	lsize = num_levels * sizeof(*labels);
	labels = _scalloc(lsize);

	calculate_combines(labels);

	for (;;) {
		for (i = 0, iip = intrinfoptr; i < intrinfo_num; ++i, ++iip) {
			unsigned level = get_interrupt_level(NULL, iip->vector_base);

			if (labels[level].off != intrentry_off) {
				/* phase error ... generate everything again */
				labels[level].off = intrentry_off;
				if (intrentry_base != NULL) {
					_sfree(intrentry_base, intrentry_size);
					intrentry_base = NULL;
				}
			}
			gen_intr(labels, level, iip);
		}
		if (intrentry_base != NULL)
			break;

		intrentry_size = intrentry_off;
		intrentry_base = _smalloc(intrentry_size);
		intrentry_off = 0;
	}

	/*
	 * Clean data cache if necessary
	 */
	_sfree(labels, lsize);
	CacheControl(&intrentry_base[0], intrentry_size, MS_SYNC);
}

__SRCVERSION("interrupt.c $Rev: 203119 $");
